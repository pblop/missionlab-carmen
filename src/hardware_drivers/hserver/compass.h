/**********************************************************************
 **                                                                  **
 **                            compass.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: compass.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef COMPASS_H
#define COMPASS_H

#include <sys/time.h>
#include <pthread.h>
#include <vector>
#include <string>

#include "HServerTypes.h"
#include "module.h"

using std::vector;
using std::string;

// This macro takes a degree (d) and change its value to be inside
// the lower bound (l) and the higher bound (h).
#define COMPASS_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}


class Compass : public Module {

public:
    typedef enum CompassType_t {
        TYPE_KVHC100,
        TYPE_3DMG,
        TYPE_CAN
    };

protected:
    typedef struct CompassData_t {
        HSRotation_t rot;
        bool validData;
    };

    HSRotation_t shCompassRot_; // Shared
    HSRotation_t shLastCompassRot_; // Shared
    HSAngularVelocity_t shAngularVelocity_; // Shared
    pthread_t readerThread_;
    vector<double> angVelYawBuffer_;
    vector<double> angVelPitchBuffer_;
    vector<double> angVelRollBuffer_;
    int type_;
    int readerThreadUSleep_;
    int skipStatusBarUpdate_;
    int poseCalcID_;
    int angVelIndex_;
    bool firstDataReceived_;
    bool readerThreadIsUp_;
    bool angVelBufferFilled_;

    static pthread_mutex_t rotationMutex_;
    static pthread_mutex_t angularVelocityMutex_;

    static const int NUM_ANGULAR_VELOCITY_SAMPLES_;

    void updateCompassData_(void);
    void updateAngularVelocity_(void);
    void setAngularVelocity_(HSAngularVelocity_t angVel);
    void readerLoop_(void);
    void logData_(double logTime);

    static void* startMainThread_(void* compassInstance);

    virtual CompassData_t readCompassData_(void) = 0;
    virtual void updateStatusBar_(void) = 0;

public:
    Compass(void);
    Compass(Compass** a, int type, string name);
    void getRotation(HSRotation_t &rot);
    void getHeading(double &heading);
    void getAngularVelocity(HSAngularVelocity_t &angVel);
    void savePoseCalcID(int id);
    int getType(void);

    virtual ~Compass(void);
    virtual void control(void) = 0;
};


inline int Compass::getType(void)
{
    return type_;
}

inline void Compass::savePoseCalcID(int id)
{
    poseCalcID_ = id;
}

extern Compass* gCompass;

#endif

/**********************************************************************
# $Log: compass.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/05/13 23:11:24  endo
# Frequency info added.
#
# Revision 1.1  2004/04/24 11:20:53  endo
# 3DM-G added.
#
#**********************************************************************/
