/**********************************************************************
 **                                                                  **
 **                             gyro.h                               **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
/* $Id: gyro.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef GYRO_H
#define GYRO_H

#include <sys/time.h>
#include <pthread.h>

#include "HServerTypes.h"
#include "module.h"

// This macro takes a degree (d) and change its value to be inside
// the lower bound (l) and the higher bound (h).
#define GYRO_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}

class Gyro : public Module {

public:
    typedef enum GyroType_t {
        TYPE_DMUVGX,
        TYPE_CAN
    };

protected:
    typedef struct GyroData_t {
        HSRotation_t rot;
        HSAngularVelocity_t angVel;
    };

    pthread_t readerThread_;
    HSRotation_t shGyroRot_; // Shared
    HSAngularVelocity_t shAngularVelocity_; // Shared
    HSRotation_t gyroRotCalibFactor_;
    HSRotation_t gyroRotOffset_;
    int type_;
    int readerThreadUSleep_;
    int skipStatusBarUpdate_;
    int poseCalcID_;
    bool firstDataReceived_;
    bool readerThreadIsUp_;

    static pthread_mutex_t rotationMutex_;
    static pthread_mutex_t angularVelocityMutex_;

    HSRotation_t correctRotation_(HSRotation_t rot);
    HSAngularVelocity_t correctAngularVelocity_(HSAngularVelocity_t gyroAngVel);
    void updateGyroData_(void);
    void setAngleYaw_(double yaw);
    void setAnglePitch_(double pitch);
    void setAngleRoll_(double roll);
    void setAngularVelocity_(HSAngularVelocity_t angVel);
    void resetAngles_(void);
    void readerLoop_(void);
    void logData_(double logTime);

    static void* startMainThread_(void* gyroInstance);

    virtual GyroData_t readGyroData_(void) = 0;
    virtual void updateStatusBar_(void) = 0;

public:
    Gyro(void);
    Gyro(Gyro** a, int type, string name);
    void getRotation(HSRotation_t &rot);
    void getAngularVelocity(HSAngularVelocity_t &angVel);
    void savePoseCalcID(int id);

    virtual ~Gyro(void);
    virtual void control(void) = 0;
};

inline void Gyro::savePoseCalcID(int id)
{
    poseCalcID_ = id;
}

extern Gyro* gGyro;

#endif

/**********************************************************************
# $Log: gyro.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2001/05/18 17:54:52  blee
# Initial revision
#
#
#**********************************************************************/
