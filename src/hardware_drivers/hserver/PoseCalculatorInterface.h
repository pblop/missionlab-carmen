/**********************************************************************
 **                                                                  **
 **                       PoseCalculatorInterface.h                  **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PoseCalculatorInterface.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef POSE_CALCULATOR_INTERFACE_H
#define POSE_CALCULATOR_INTERFACE_H

#include "HServerTypes.h"
#include "PoseCalculatorTypes.h"

class Robot;
class Gyro;
class Gps;
class Compass;

class PoseCalcInterface {

public:
    virtual int addRobot(Robot *robot) = 0;
    virtual int addGyro(Gyro *gyro) = 0;
    virtual int addGps(Gps *gps) = 0;
    virtual int addCompass(Compass *compass) = 0;
    virtual void initPose(HSPose_t pose) = 0;
    virtual void resetPose(void) = 0;
    virtual void setPose(HSPose_t pose) = 0;
    virtual void getPose(HSPose_t &pose) = 0;
    virtual void getLocation(HSLocation_t &loc) = 0;
    virtual void getRotation(HSRotation_t &rot) = 0;
    virtual void getVelocity(HSVelocity_t &vel) = 0;
    virtual void updateModulePose(int moduleID, HSPose_t pose) = 0;
    virtual void updateModuleLocation(int moduleID, HSLocation_t loc) = 0;
    virtual void updateModuleRotation(int moduleID, HSRotation_t rot) = 0;
    virtual void disconnectModule(int moduleID) = 0;
    virtual void getReferenceTimes(double &locRefTime, double &rotRefTime) = 0;
    virtual void getLocReferenceTime(double &refTime) = 0;
    virtual void getRotReferenceTime(double &refTime) = 0;
    virtual void control(void) = 0;
};

extern PoseCalcInterface *gPoseCalc;

#endif

/**********************************************************************
# $Log: PoseCalculatorInterface.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
#**********************************************************************/
