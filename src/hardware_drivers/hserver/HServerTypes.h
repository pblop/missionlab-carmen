/**********************************************************************
 **                                                                  **
 **                          HServerTypes.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: HServerTypes.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef HSERVER_TYPES_H
#define HSERVER_TYPES_H

#include <string>

using std::string;

static const double HS_EPSILON  = 0.000001;

static const int HS_POSE_EXTRADATA_SIZE  = 1024;

typedef struct HSPoseExtra_t {
    double time;
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct HSLocation_t {
    double x;
    double y;
    double z;
    HSPoseExtra_t extra;
};

typedef struct HSRotation_t {
    double yaw;
    double pitch;
    double roll;
    HSPoseExtra_t extra;
};

typedef struct HSPose_t {
    HSLocation_t loc;
    HSRotation_t rot;
    HSPoseExtra_t extra;
};

typedef HSLocation_t HSTransVelocity_t;

typedef HSRotation_t HSAngularVelocity_t;

typedef struct HSVelocityExtra_t {
    double time;
    bool backward;
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct HSVelocity_t {
    HSTransVelocity_t transVel;
    HSAngularVelocity_t angVel;
    HSVelocityExtra_t extra;
};

typedef struct HSSpeedExtra_t {
    double time;
    char data[HS_POSE_EXTRADATA_SIZE];
};

typedef struct HSDriveSpeed_t {
    double value;
    HSSpeedExtra_t extra;
};

typedef struct HSSteerSpeed_t {
    double value;
    HSSpeedExtra_t extra;
};

typedef struct HSArmSpeed_t {
    double value;
    HSSpeedExtra_t extra;
};

typedef struct HSSpeed_t {
    HSDriveSpeed_t driveSpeed;
    HSSteerSpeed_t steerSpeed;
    HSArmSpeed_t armSpeed;
    HSSpeedExtra_t extra;
};

const string HS_MODULE_NAME_APM = "Apm";
const string HS_MODULE_NAME_CAMERA = "Camera";
const string HS_MODULE_NAME_COGNACHROME = "Cognachrome";
const string HS_MODULE_NAME_COMPASS_3DMG = "Compass3DMG";
const string HS_MODULE_NAME_COMPASS_KVHC100 = "CompassKVHC100";
const string HS_MODULE_NAME_COMPASS_CAN = "CompassCAN";
const string HS_MODULE_NAME_GPS = "Gps";
const string HS_MODULE_NAME_GYRO_DMUVGX = "GyroDMUVGX";
const string HS_MODULE_NAME_GYRO_CAN = "GyroCAN";
const string HS_MODULE_NAME_JBOX = "JBox";
const string HS_MODULE_NAME_IPT = "IPT";
const string HS_MODULE_NAME_ROBOT_AMIGOBOT = "Amigobot";
const string HS_MODULE_NAME_ROBOT_ROBOTLASER = "RobotLaser";
const string HS_MODULE_NAME_ROBOT_ROOMBA = "Roomba";
const string HS_MODULE_NAME_ROBOT_ROOMBA560 = "Roomba560";
const string HS_MODULE_NAME_ROBOT_CARRETILLA = "Carretilla";
const string HS_MODULE_NAME_ROBOT_CARRETILLA_SIMULACION = "CarretillaSimulacion";
const string HS_MODULE_NAME_ROBOT_CARRETILLA_GAZEBO = "CarretillaGazebo";
const string HS_MODULE_NAME_WEBCAM = "webCam";
const string HS_MODULE_NAME_ROBOT_ATRVJR = "ATRVJr";
const string HS_MODULE_NAME_ROBOT_FRED = "Fred";
const string HS_MODULE_NAME_ROBOT_NOMAD = "Nomad";
const string HS_MODULE_NAME_ROBOT_PIONEER = "Pioneer";
const string HS_MODULE_NAME_ROBOT_PIONEER2 = "Pioneer2";
const string HS_MODULE_NAME_ROBOT_POSECALC = "PoseCalc";
const string HS_MODULE_NAME_ROBOT_URBAN = "Urban";
const string HS_MODULE_NAME_LASER_SICK = "LaserSICK";
const string HS_MODULE_NAME_VIDEO = "Video";
const string HS_MODULE_NAME_XWINDOW = "Xwindow";
const string HS_MODULE_NAME_ROBOT_CARMEN = "Carmen";


typedef enum HSRobotTypes_t {
    HS_ROBOT_TYPE_AMIGOBOT,
    HS_ROBOT_TYPE_ROBOTLASER,
	HS_ROBOT_TYPE_ROOMBA,
	HS_ROBOT_TYPE_ROOMBA560,
    HS_ROBOT_TYPE_CARRETILLA,
    HS_ROBOT_TYPE_CARRETILLA_SIMULACION,
    HS_ROBOT_TYPE_CARRETILLA_GAZEBO,
    HS_ROBOT_TYPE_ATRVJR,
    HS_ROBOT_TYPE_FRED,
    HS_ROBOT_TYPE_NOMAD,
    HS_ROBOT_TYPE_PIONEER,
    HS_ROBOT_TYPE_PIONEER2,
    HS_ROBOT_TYPE_URBAN,
    HS_ROBOT_TYPE_CARMEN
};

typedef struct HSDateString_t {
    string year;
    string month;
    string day;
    string hour;
    string min;
    string sec;
};

#endif

/**********************************************************************
# $Log: HServerTypes.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
#**********************************************************************/
