/**********************************************************************
 **                                                                  **
 **                      PoseCalculatorTypes.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PoseCalculatorTypes.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef POSE_CALCULATOR_TYPES_H
#define POSE_CALCULATOR_TYPES_H

#include <string>
#include <vector>

#include "HServerTypes.h"

using std::string;
using std::vector;

typedef enum PoseCalcPoseType_t {
    POSECALC_POSE_TYPE_NONE,
    POSECALC_POSE_TYPE_LOCATION,
    POSECALC_POSE_TYPE_ROTATION,
    POSECALC_POSE_TYPE_LOCATION_AND_ROTATION
};

typedef enum PoseCalcFuserTypes_t {
    POSECALC_FUSER_TYPE_NONE,
    POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE,
    POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER,
    POSECALC_FUSER_TYPE_PARTICLE_FILTER,
    NUM_POSECALC_FUSER_TYPES
};

typedef enum PoseCalcSensorType_t {
    POSECALC_SENSOR_TYPE_NONE,
    POSECALC_SENSOR_TYPE_COMPASS_3DMG,
    POSECALC_SENSOR_TYPE_COMPASS_KVHC100,
    POSECALC_SENSOR_TYPE_COMPASS_CAN,
    POSECALC_SENSOR_TYPE_GPS,
    POSECALC_SENSOR_TYPE_GYRO_DMUVGX,
    POSECALC_SENSOR_TYPE_GYRO_CAN,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_AMIGOBOT,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROBOTLASER,
	POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA,
	POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA560,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_SIMULACION,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_GAZEBO,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_ATRVJR,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_FRED,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_NOMAD,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER2,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_URBAN,
    POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARMEN
};

typedef enum PoseCalcSensorClass_t {
    POSECALC_SENSOR_CLASS_NONE,
    POSECALC_SENSOR_CLASS_COMPASS,
    POSECALC_SENSOR_CLASS_GPS,
    POSECALC_SENSOR_CLASS_GYRO,
    POSECALC_SENSOR_CLASS_ROBOTSHAFT
};

typedef enum PoseCalcCoordinatorTypes_t {
    POSECALC_COORDINATOR_TYPE_NONE,
    POSECALC_COORDINATOR_TYPE_PICK_FILTER
};

typedef enum PoseCalcVarianceScheme_t {
    POSECALC_VARIANCE_SCHEME_NONE,
    POSECALC_VARIANCE_SCHEME_DYNAMIC,
    POSECALC_VARIANCE_SCHEME_STATIC
};

typedef struct PoseCalcConstants_t {
    double compass2MlabHeadingFacor;
    double compass2MlabHeadingOffset;
    double compassMaxValidAngSpeed4Heading;
    double gps2MlabHeadingFacor;
    double gps2MlabHeadingOffset;
    double gpsMinValidTransSpeed4Heading;
    double gpsMaxValidAngSpeed4Heading;
};

typedef struct PoseCalcGenericData_t {
    void *content;
};

typedef struct PoseCalcLocationConfidence_t {
    double value;
};

typedef struct PoseCalcRotationConfidence_t {
    double value;
};

typedef struct PoseCalcPoseConfidence_t {
    PoseCalcLocationConfidence_t locConfidence;
    PoseCalcRotationConfidence_t rotConfidence;
};

typedef HSLocation_t PoseCalcLocationVariance_t;
typedef HSRotation_t PoseCalcRotationVariance_t;

typedef struct PoseCalcPoseVariance_t {
    PoseCalcLocationVariance_t locVariance;
    PoseCalcRotationVariance_t rotVariance;
};

typedef struct PoseCalcPoseInfoExtra_t {
    double time;
    bool read[NUM_POSECALC_FUSER_TYPES];
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct PoseCalcLocationInfo_t {
    int sensorClass;
    int sensorType;
    double refTime;
    HSLocation_t loc;
    HSLocation_t rawLoc;
    PoseCalcLocationConfidence_t confidence;
    PoseCalcLocationVariance_t variance;
    PoseCalcPoseInfoExtra_t extra;
};

typedef struct PoseCalcRotationInfo_t {
    int sensorClass;
    int sensorType;
    double refTime;
    HSRotation_t rot;
    HSRotation_t rawRot;
    PoseCalcRotationConfidence_t confidence;
    PoseCalcRotationVariance_t variance;
    PoseCalcPoseInfoExtra_t extra;
};

typedef struct PoseCalcPoseInfo_t {
    int sensorClass;
    int sensorType;
    int poseType;
    double locRefTime;
    double rotRefTime;
    HSPose_t pose;
    HSPose_t rawPose;
    PoseCalcPoseConfidence_t confidence;
    PoseCalcPoseVariance_t variance;
    PoseCalcPoseInfoExtra_t extra;
};

typedef struct PoseCalcSensoryDataExtra_t {
    double time;
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct PoseCalcSensoryData_t {
    int sensorClass;
    int sensorType;
    vector<PoseCalcLocationInfo_t> locData;
    vector<PoseCalcRotationInfo_t> rotData;
    int latestLocIndex;
    int latestRotIndex;
    PoseCalcSensoryDataExtra_t extra;
};


typedef struct PoseCalcFusedPoseExtra_t {
    double time;
    int locSensorType;
    int rotSensorType;
    bool validPose;
    bool validLoc;
    bool validRot;
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct PoseCalcFusedPose_t {
    int fuserType;
    HSPose_t pose;
    PoseCalcPoseConfidence_t confidence;
    PoseCalcPoseVariance_t variance;
    PoseCalcFusedPoseExtra_t extra;
};

#endif

/**********************************************************************
# $Log: PoseCalculatorTypes.h,v $
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
