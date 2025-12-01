/**********************************************************************
 **                                                                  **
 **                            compass.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: compass.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <unistd.h>
#include <string>
#include <numeric>

#include "compass.h"
#include "hserver.h"
#include "PoseCalculatorInterface.h"
#include "Watchdog.h"
#include "LogManager.h"

using std::string;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
// Pthread mutex to for HServer angles.
pthread_mutex_t Compass::rotationMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Compass::angularVelocityMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int Compass::NUM_ANGULAR_VELOCITY_SAMPLES_ = 10;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
// Constructor for the Compass class.
Compass::Compass(Compass** a, int type, string name) :
    Module((Module**) a, name),
    type_(type),
    poseCalcID_(-1),
    angVelIndex_(0),
    firstDataReceived_(false),
    readerThreadIsUp_(false),
    angVelBufferFilled_(false)
{
    memset((HSRotation_t *)&shCompassRot_, 0x0, sizeof(HSRotation_t));
    memset((HSRotation_t *)&shLastCompassRot_, 0x0, sizeof(HSRotation_t));
    memset(
        (HSAngularVelocity_t *)&shAngularVelocity_,
        0x0,
        sizeof(HSAngularVelocity_t));

    angVelYawBuffer_.resize(NUM_ANGULAR_VELOCITY_SAMPLES_);
    angVelPitchBuffer_.resize(NUM_ANGULAR_VELOCITY_SAMPLES_);
    angVelRollBuffer_.resize(NUM_ANGULAR_VELOCITY_SAMPLES_);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Compass::~Compass(void)
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
void* Compass::startMainThread_(void* compassInstance)
{
    ((Compass*)compassInstance)->readerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to get the compass.
//-----------------------------------------------------------------------
void Compass::readerLoop_(void)
{
    int count = 0;

    readerThreadIsUp_ = true;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Get the data from the compass.
        updateCompassData_();

        // Compute the frequency of the loop.
        compFrequency_();

        // Compute the angular velocity.
        updateAngularVelocity_();

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

        // Tell watchdog that compass is still alive.
        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        // Sleep a bit to reduce the CPU load.
        usleep(readerThreadUSleep_);
    }
}

//-----------------------------------------------------------------------
// This function updates the compass data by reading it
// from mobility.
//-----------------------------------------------------------------------
void Compass::updateCompassData_(void)
{
    CompassData_t rawCompassData;

    // Read it from the device
    rawCompassData = readCompassData_();
    
    if (rawCompassData.validData)
    {
        // Save the value.
        pthread_mutex_lock(&rotationMutex_);
        shLastCompassRot_ = shCompassRot_;
        shCompassRot_ = rawCompassData.rot;
        pthread_mutex_unlock(&rotationMutex_);

        if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
        {
#ifdef LOG_POSE_DATA
        	fprintf(stderr, "COMPASS: Rot=%f - time=%f\n", rawCompassData.rot, rawCompassData.rot.extra.time);
#endif
            gPoseCalc->updateModuleRotation(poseCalcID_, rawCompassData.rot);
        }

        if (!firstDataReceived_)
        {
            firstDataReceived_ = true;
        }
    }
}

//-----------------------------------------------------------------------
// This function returns the current compass heading.
//-----------------------------------------------------------------------
void Compass::getHeading(double& heading )
{
    pthread_mutex_lock(&rotationMutex_);
    heading = shCompassRot_.yaw;
    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function returns the current compass heading.
//-----------------------------------------------------------------------
void Compass::getRotation(HSRotation_t &rot)
{
    pthread_mutex_lock(&rotationMutex_);
    rot = shCompassRot_;
    pthread_mutex_unlock(&rotationMutex_);
}

//-----------------------------------------------------------------------
// This function computes the velocity of the robot.
//-----------------------------------------------------------------------
void Compass::updateAngularVelocity_(void)
{
    HSRotation_t dRot;
    HSAngularVelocity_t angVel, aveAngVel;
    double frequency, sum;

    memset((HSAngularVelocity_t *)&aveAngVel, 0x0, sizeof(HSAngularVelocity_t));

    getFrequency_(frequency);

    if (frequency == 0)
    {
        // The angular velocity not yet ready to be computed.
        return;
    }

    pthread_mutex_lock(&rotationMutex_);
    dRot.yaw = shCompassRot_.yaw - shLastCompassRot_.yaw;
    dRot.pitch = shCompassRot_.pitch - shLastCompassRot_.pitch;
    dRot.roll = shCompassRot_.roll - shLastCompassRot_.roll;
    aveAngVel.extra.time = shCompassRot_.extra.time;
    pthread_mutex_unlock(&rotationMutex_);

    // Assumption: No robot will turn more than 180 degree per cycle.
    // This assumption is necessary as there is no way to tell whether
    // the robot turned 270 degree or - 90 degree by just comparing
    // the theta value.
    COMPASS_CRAMPDEG(dRot.yaw, -180.0, 180.0);
    COMPASS_CRAMPDEG(dRot.pitch, -180.0, 180.0);
    COMPASS_CRAMPDEG(dRot.roll, -180.0, 180.0);

    angVel.yaw = dRot.yaw*frequency;
    angVel.pitch = dRot.pitch*frequency;
    angVel.roll = dRot.roll*frequency;

    angVelYawBuffer_[angVelIndex_] = angVel.yaw;
    angVelPitchBuffer_[angVelIndex_] = angVel.pitch;
    angVelRollBuffer_[angVelIndex_] = angVel.roll;
    angVelIndex_++;

    if (angVelIndex_ >= NUM_ANGULAR_VELOCITY_SAMPLES_)
    {
        angVelBufferFilled_ = true;
        angVelIndex_ = 0;
    }

    if (angVelBufferFilled_)
    {
        sum = accumulate(angVelYawBuffer_.begin(), angVelYawBuffer_.end(), 0.0);
        aveAngVel.yaw = sum/(double)NUM_ANGULAR_VELOCITY_SAMPLES_;

        sum = accumulate(angVelPitchBuffer_.begin(), angVelPitchBuffer_.end(), 0.0);
        aveAngVel.pitch = sum/(double)NUM_ANGULAR_VELOCITY_SAMPLES_;

        sum = accumulate(angVelRollBuffer_.begin(), angVelRollBuffer_.end(), 0.0);
        aveAngVel.roll = sum/(double)NUM_ANGULAR_VELOCITY_SAMPLES_;
    }
    else
    {
        sum = accumulate(
            angVelYawBuffer_.begin(),
            (angVelYawBuffer_.begin()) + (angVelIndex_-1),
            0.0);
        aveAngVel.yaw = sum/(double)angVelIndex_;

        sum = accumulate(
            angVelPitchBuffer_.begin(),
            (angVelPitchBuffer_.begin()) + (angVelIndex_-1),
            0.0);
        aveAngVel.pitch = sum/(double)angVelIndex_;

        sum = accumulate(
            angVelRollBuffer_.begin(),
            (angVelRollBuffer_.begin()) + (angVelIndex_-1),
            0.0);
        aveAngVel.roll = sum/(double)angVelIndex_;
    }

    setAngularVelocity_(aveAngVel);
}

//-----------------------------------------------------------------------
// This function updates the angular velocity.
//-----------------------------------------------------------------------
void Compass::setAngularVelocity_(HSAngularVelocity_t angVel)
{
    pthread_mutex_lock(&angularVelocityMutex_);
    shAngularVelocity_ = angVel;
    pthread_mutex_unlock(&angularVelocityMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the anagular velocity.
//-----------------------------------------------------------------------
void Compass::getAngularVelocity(HSAngularVelocity_t &angVel)
{
    pthread_mutex_lock(&angularVelocityMutex_);
    angVel = shAngularVelocity_;
    pthread_mutex_unlock(&angularVelocityMutex_);
}

//-----------------------------------------------------------------------
// This function logs the data.
//-----------------------------------------------------------------------
void Compass::logData_(double logTime)
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

/**********************************************************************
# $Log: compass.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/05/13 23:11:24  endo
# Frequency info added.
#
# Revision 1.1  2004/04/24 11:20:53  endo
# 3DM-G added.
#
***********************************************************************/
