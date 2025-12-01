/**********************************************************************
 **                                                                  **
 **                              gyro.c                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gyro.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <unistd.h>
#include <string.h>

#include "gyro.h"
#include "PoseCalculatorInterface.h"
#include "Watchdog.h"
#include "LogManager.h"
#include "hserver.h"

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t Gyro::rotationMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Gyro::angularVelocityMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for the Gyro class.
//-----------------------------------------------------------------------
Gyro::Gyro(Gyro** a, int type, string name) :
    Module((Module**)a, name),
    type_(type),
    poseCalcID_(-1),
    firstDataReceived_(false),
    readerThreadIsUp_(false)
{
    memset((HSRotation_t *)&shGyroRot_, 0x0, sizeof(HSRotation_t));
    memset((HSAngularVelocity_t *)&shAngularVelocity_, 0x0, sizeof(HSAngularVelocity_t));
    resetAngles_();
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Gyro::~Gyro(void)
{
    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->disconnectModule(poseCalcID_);
        poseCalcID_ = -1;
    }
}

//-----------------------------------------------------------------------
// This function calls the reader thread.
//-----------------------------------------------------------------------
void* Gyro::startMainThread_(void* gyroInstance)
{
    ((Gyro*)gyroInstance)->readerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to get the gyros data.
//-----------------------------------------------------------------------
void Gyro::readerLoop_(void)
{
    int count = 0;

    readerThreadIsUp_ = true;

    while(TRUE)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Get the data from the gyro.
        updateGyroData_();

        // Compute the frequency.
        compFrequency_();

        // Check for the pthread termination again.
        pthread_testcancel();

        // Show updated data to the screen.
        if (count > skipStatusBarUpdate_)
        {
            updateStatusBar_();
            count = 0;
        }

        count++;

        // Log the data if necessary.
        if (gLogManager != NULL)
        {
            manageLog_(readerThread_);
        }

        // Tell watchdog that the robot is still alive.
        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        // Sleep a bit to reduce the CPU load.
        usleep(readerThreadUSleep_);
    }
}
//-----------------------------------------------------------------------
// This function updates the gyroscopic data by reading it
// from mobility.
//-----------------------------------------------------------------------
void Gyro::updateGyroData_(void)
{
    GyroData_t rawGyroData;
    HSRotation_t rot;
    HSAngularVelocity_t angVel;

    // Read the raw data from mobility.
    rawGyroData = readGyroData_();

    // Fix the rotation.
    rot = correctRotation_(rawGyroData.rot);

    // Save the rotation.
    pthread_mutex_lock(&rotationMutex_);
    shGyroRot_ = rot;
    pthread_mutex_unlock(&rotationMutex_);

    // Fix the angular velocity (scale/direction).
    angVel = correctAngularVelocity_(rawGyroData.angVel);

    // Save the angular velocity.
    pthread_mutex_lock(&angularVelocityMutex_);
    shAngularVelocity_ = angVel;
    pthread_mutex_unlock(&angularVelocityMutex_);

    // Report to PoseCalc.
    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->updateModuleRotation(poseCalcID_, rot);
    }

    if (!firstDataReceived_)
    {
        firstDataReceived_ = true;
    }
}

//-----------------------------------------------------------------------
// This function converts the gyro data value into HServer
// yaw, pitch, and roll angles in degree.
//-----------------------------------------------------------------------
HSRotation_t Gyro::correctRotation_(HSRotation_t gyroRot)
{
    HSRotation_t rot;

    rot = gyroRot;

    rot.yaw *= gyroRotCalibFactor_.yaw;
    rot.yaw += gyroRotOffset_.yaw;
    GYRO_CRAMPDEG(rot.yaw,-180.0,180.0);

    rot.pitch *= gyroRotCalibFactor_.pitch;
    rot.pitch += gyroRotOffset_.pitch;
    GYRO_CRAMPDEG(rot.pitch,-180.0,180.0);

    rot.roll *= gyroRotCalibFactor_.roll;
    rot.roll += gyroRotOffset_.roll;
    GYRO_CRAMPDEG(rot.roll,-180.0,180.0);

    return rot;
}

//-----------------------------------------------------------------------
// This function fixes the scale/direction of the velocity.
//-----------------------------------------------------------------------
HSAngularVelocity_t Gyro::correctAngularVelocity_(HSAngularVelocity_t gyroAngVel)
{
    HSAngularVelocity_t angVel;

    angVel = gyroAngVel;

    angVel.yaw *= gyroRotCalibFactor_.yaw;
    angVel.pitch *= gyroRotCalibFactor_.pitch;
    angVel.roll *= gyroRotCalibFactor_.roll;

    return angVel;
}

//-----------------------------------------------------------------------
// This function sets the Gyro angles to be the specified value.
//-----------------------------------------------------------------------
void Gyro::setAngleYaw_(double yaw)
{
    pthread_mutex_lock(&rotationMutex_);

    gyroRotOffset_.yaw += yaw - shGyroRot_.yaw;
    shGyroRot_.yaw = yaw;
    GYRO_CRAMPDEG(shGyroRot_.yaw,0.0,360.0);

    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function sets the Gyro angles to be the specified value.
//-----------------------------------------------------------------------
void Gyro::setAnglePitch_(double pitch)
{
    pthread_mutex_lock(&rotationMutex_);

    gyroRotOffset_.pitch += pitch - shGyroRot_.pitch;
    shGyroRot_.pitch = pitch;
    GYRO_CRAMPDEG(shGyroRot_.pitch,-180.0,180.0);

    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function sets the Gyro angles to be the specified value.
//-----------------------------------------------------------------------
void Gyro::setAngleRoll_(double roll)
{
    pthread_mutex_lock(&rotationMutex_);

    gyroRotOffset_.roll += roll - shGyroRot_.roll;
    shGyroRot_.roll = roll;
    GYRO_CRAMPDEG(shGyroRot_.roll,-180.0,180.0);

    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function resets all the angles.
//-----------------------------------------------------------------------
void Gyro::resetAngles_(void)
{
    setAngleYaw_(0);
    setAnglePitch_(0);
    setAngleRoll_(0);
}

//-----------------------------------------------------------------------
// This function returns yaw, pitch, and roll.
//-----------------------------------------------------------------------
void Gyro::getRotation(HSRotation_t &rot)
{
    pthread_mutex_lock(&rotationMutex_);
    rot = shGyroRot_;
    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function logs the data.
//-----------------------------------------------------------------------
void Gyro::logData_(double logTime)
{
    HSRotation_t rot;

    if (logfile_ != NULL)
    {
        getRotation(rot);

        fprintf(
            logfile_,
            "%f %.2f %.2f %.2f %f\n",
            logTime,
            rot.yaw,
            rot.pitch,
            rot.roll,
            rot.extra.time);

        fflush(logfile_);
    }
}

//-----------------------------------------------------------------------
// This function retrieves the anagular velocity.
//-----------------------------------------------------------------------
void Gyro::getAngularVelocity(HSAngularVelocity_t &angVel)
{
    pthread_mutex_lock(&angularVelocityMutex_);
    angVel = shAngularVelocity_;
    pthread_mutex_unlock(&angularVelocityMutex_);
}

/**********************************************************************
# $Log: gyro.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/05/13 23:15:11  endo
# Memory leak fixed.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2001/12/22 16:35:40  endo
# CALIB_FACTOR changed due to the change in the DMU server.
#
# Revision 1.1  2001/05/18 17:54:47  blee
# Initial revision
#
#**********************************************************************/
