/**********************************************************************
 **                                                                  **
 **                            PCModules.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCModules.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_MODULE_H
#define POSECALC_MODULE_H

#include <pthread.h>
#include <vector>
#include <string.h>

#include "gyro.h"
#include "robot.h"
#include "gps.h"
#include "compass.h"
#include "PoseCalculatorTypes.h"
#include "PCModuleInterface.h"
#include "PoseCalculatorInterface.h"
#include "PCModuleFilterInterface.h"

using std::vector;
using std::string;

//-----------------------------------------------------------------------
// Generic PoseCalcModule
//-----------------------------------------------------------------------
class PoseCalcModule : public PoseCalcModuleInterface {

protected:
    PoseCalcInterface *poseCalc_;
    PoseCalcModuleFilterInterface *moduleFilter_;
    HSPose_t shModulePose_; // Shared
    int id_;
    int sensorType_;

    const int ID_;
    const int SENSOR_CLASS_;
    const int SENSOR_TYPE_;
    const int POSE_TYPE_;

    static pthread_mutex_t modulePoseMutex_;

    static const string EMPTY_STRING_;

    void grade_(PoseCalcPoseInfo_t &poseInfo, int varianceScheme);

    virtual void setFilter_(void) {};
    virtual void filter_(PoseCalcPoseInfo_t &poseInfo) = 0;
    virtual void transform_(PoseCalcPoseInfo_t &poseInfo) = 0;
    virtual void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo) = 0;
    virtual void gradeStatic_(PoseCalcPoseInfo_t &poseInfo) = 0;

public:
    PoseCalcModule(
        int id,
        int sensorClass,
        int sensorType,
        int poseType,
        PoseCalcInterface *poseCalc);
    void managePose(PoseCalcPoseInfo_t &poseInfo, int varianceScheme);
    void setPose(HSPose_t pose);
    void die(void);

    virtual ~PoseCalcModule(void);
};

inline void PoseCalcModule::die(void)
{
    delete this;
}

//-----------------------------------------------------------------------
// Implementation of PoseCalcModule
//-----------------------------------------------------------------------
class PoseCalcModuleRobotShaftLoc : public PoseCalcModule {

protected:
    Robot *robot_;
    HSLocation_t lastInputLoc_;

    static const double MAXIMUM_STOPPING_TRANS_SPEED_;
    static const double MAXIMUM_STOPPING_ANGULAR_SPEED_;
    static const double MOVING_LOC_VARIANCE_;
    static const double STATIC_LOC_VARIANCE_;
    static const double STOPPING_LOC_VARIANCE_;
    static const double WORST_ROT_VARIANCE_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleRobotShaftLoc(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Robot *robot);
    ~PoseCalcModuleRobotShaftLoc(void);
};


class PoseCalcModuleRobotShaftRot : public PoseCalcModule {

protected:
    Robot *robot_;
    HSRotation_t lastInputRot_;

    static const double MAXIMUM_STOPPING_TRANS_SPEED_;
    static const double MAXIMUM_STOPPING_ANGULAR_SPEED_;
    static const double MOVING_ROT_VARIANCE_;
    static const double STATIC_ROT_VARIANCE_;
    static const double STOPPING_ROT_VARIANCE_;
    static const double WORST_LOC_VARIANCE_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleRobotShaftRot(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Robot *robot);
    ~PoseCalcModuleRobotShaftRot(void);
};

class PoseCalcModuleGyroRot : public PoseCalcModule {

protected:
    Gyro *gyro_;
    HSRotation_t lastInputRot_;

    static const double MAXIMUM_STOPPING_TRANS_SPEED_;
    static const double MAXIMUM_STOPPING_ANGULAR_SPEED_;
    static const double MOVING_ROT_VARIANCE_;
    static const double STATIC_ROT_VARIANCE_;
    static const double STOPPING_ROT_VARIANCE_;
    static const double WORST_LOC_VARIANCE_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleGyroRot(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Gyro *gyro);
    ~PoseCalcModuleGyroRot(void);
};

class PoseCalcModuleGpsLoc : public PoseCalcModule {

protected:
    Gps *gps_;

    static const double RT20_0_LOC_VARIANCE_;
    static const double RT20_1_LOC_VARIANCE_;
    static const double RT20_100_LOC_VARIANCE_;
    static const double STATIC_LOC_VARIANCE_;
    static const double WORST_LOC_VARIANCE_;
    static const double WORST_ROT_VARIANCE_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleGpsLoc(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Gps *gps);
    ~PoseCalcModuleGpsLoc(void);
};

class PoseCalcModuleGpsRot : public PoseCalcModule {

protected:
    Gps *gps_;
    long int numValidTransSpeed_;
    long int numValidAngularSpeed_;

    const double HEADING_FACTOR_;
    const double HEADING_OFFSET_;
    const double MIN_VALID_TRANS_SPEED_;
    const double MAX_VALID_ANGULAR_SPEED_;

    static const double RT20_0_ROT_VARIANCE_;
    static const double RT20_1_ROT_VARIANCE_;
    static const double RT20_100_ROT_VARIANCE_;
    static const double STATIC_ROT_VARIANCE_;
    static const double WORST_LOC_VARIANCE_;
    static const double WORST_ROT_VARIANCE_;
    static const long int MIN_VALID_TRANS_SPEED_SAMPLES_;
    static const long int MIN_VALID_ANGULAR_SPEED_SAMPLES_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleGpsRot(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Gps *gps,
        double headingFactor,
        double headingOffset,
        double minValidTransSpeed,
        double maxValidAngSpeed);
    ~PoseCalcModuleGpsRot(void);
};

class PoseCalcModuleCompassRot : public PoseCalcModule {

protected:
    Compass *compass_;
    long int numValidAngularSpeed_;

    const double HEADING_FACTOR_;
    const double HEADING_OFFSET_;
    //const double MIN_VALID_ANGULAR_SPEED_;
    const double MAX_VALID_ANGULAR_SPEED_;

    static const double BEST_ROT_VARIANCE_;
    static const double STATIC_ROT_VARIANCE_;
    static const double WORST_LOC_VARIANCE_;
    static const double WORST_ROT_VARIANCE_;
    static const long int MIN_VALID_ANGULAR_SPEED_SAMPLES_;

    void setFilter_(void);
    void filter_(PoseCalcPoseInfo_t &poseInfo);
    void transform_(PoseCalcPoseInfo_t &poseInfo);
    void gradeDynamic_(PoseCalcPoseInfo_t &poseInfo);
    void gradeStatic_(PoseCalcPoseInfo_t &poseInfo);

public:
    PoseCalcModuleCompassRot(
        int id,
        int sensorType,
        PoseCalcInterface *poseCalc,
        Compass *compass,
        double headingFactor,
        double headingOffset,
        double maxValidAngSpeed);
    ~PoseCalcModuleCompassRot(void);
};

#endif

/**********************************************************************
# $Log: PCModules.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:18  endo
# New PoseCalc integrated.
#
#**********************************************************************/
