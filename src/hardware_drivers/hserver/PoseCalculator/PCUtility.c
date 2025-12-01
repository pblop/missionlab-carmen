/**********************************************************************
 **                                                                  **
 **                             PCUtility.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCUtility.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "PCUtility.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string PoseCalcUtility::EMPTY_STRING_ = "";
extern void printfTextWindow(const char* arg, ...);

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// This function returns the sensor name based on the specified type.
//-----------------------------------------------------------------------
string PoseCalcUtility::sensorType2Name(int sensorType)
{
    string name = EMPTY_STRING_;

    switch (sensorType) {

    case POSECALC_SENSOR_TYPE_COMPASS_3DMG:
        name = HS_MODULE_NAME_COMPASS_3DMG;
        break;

    case POSECALC_SENSOR_TYPE_COMPASS_KVHC100:
        name = HS_MODULE_NAME_COMPASS_KVHC100;
        break;

    case POSECALC_SENSOR_TYPE_COMPASS_CAN:
    	name = HS_MODULE_NAME_COMPASS_CAN;
    	break;

    case POSECALC_SENSOR_TYPE_GPS:
        name = HS_MODULE_NAME_GPS;
        break;

    case POSECALC_SENSOR_TYPE_GYRO_DMUVGX:
        name = HS_MODULE_NAME_GYRO_DMUVGX;
        break;

    case POSECALC_SENSOR_TYPE_GYRO_CAN:
        name = HS_MODULE_NAME_GYRO_CAN;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_AMIGOBOT:
        name = HS_MODULE_NAME_ROBOT_AMIGOBOT;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROBOTLASER:
        name = HS_MODULE_NAME_ROBOT_ROBOTLASER;
        break;

	case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA:
        name = HS_MODULE_NAME_ROBOT_ROOMBA;
        break;

	case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA560:
        name = HS_MODULE_NAME_ROBOT_ROOMBA560;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA:
        name = HS_MODULE_NAME_ROBOT_CARRETILLA;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_SIMULACION:
        name = HS_MODULE_NAME_ROBOT_CARRETILLA_SIMULACION;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_GAZEBO:
        name = HS_MODULE_NAME_ROBOT_CARRETILLA_GAZEBO;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ATRVJR:
        name = HS_MODULE_NAME_ROBOT_ATRVJR;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_FRED:
        name = HS_MODULE_NAME_ROBOT_FRED;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_NOMAD:
        name = HS_MODULE_NAME_ROBOT_NOMAD;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER:
        name = HS_MODULE_NAME_ROBOT_PIONEER;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER2:
        name = HS_MODULE_NAME_ROBOT_PIONEER2;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_URBAN:
        name = HS_MODULE_NAME_ROBOT_URBAN;
        break;

    case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARMEN:
            name = HS_MODULE_NAME_ROBOT_CARMEN;
            break;
    }

    return name;
}

//-----------------------------------------------------------------------
// This function returns the sensor type based on the specified name.
//-----------------------------------------------------------------------
int PoseCalcUtility::name2SensorType(string name)
{
    int sensorType = POSECALC_SENSOR_TYPE_NONE;

    if (name == HS_MODULE_NAME_COMPASS_3DMG)
    {
        sensorType = POSECALC_SENSOR_TYPE_COMPASS_3DMG;
    }
    else if (name == HS_MODULE_NAME_COMPASS_KVHC100)
    {
        sensorType = POSECALC_SENSOR_TYPE_COMPASS_KVHC100;
    }
    else if (name == HS_MODULE_NAME_COMPASS_CAN)
    {
        sensorType = POSECALC_SENSOR_TYPE_COMPASS_CAN;
    }
    else if (name == HS_MODULE_NAME_GPS)
    {
        sensorType = POSECALC_SENSOR_TYPE_GPS;
    }
    else if (name == HS_MODULE_NAME_GYRO_DMUVGX)
    {
        sensorType = POSECALC_SENSOR_TYPE_GYRO_DMUVGX;
    }
    else if (name == HS_MODULE_NAME_GYRO_CAN)
    {
        sensorType = POSECALC_SENSOR_TYPE_GYRO_CAN;
    }
    else if (name == HS_MODULE_NAME_ROBOT_AMIGOBOT)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_AMIGOBOT;
    }
    else if (name == HS_MODULE_NAME_ROBOT_ROBOTLASER)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROBOTLASER;
    }
	else if (name == HS_MODULE_NAME_ROBOT_ROOMBA)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA;
    }
	else if (name == HS_MODULE_NAME_ROBOT_ROOMBA560)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA560;
    }
    else if (name == HS_MODULE_NAME_ROBOT_CARRETILLA)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA;
    }
    else if (name == HS_MODULE_NAME_ROBOT_CARRETILLA_SIMULACION)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_SIMULACION;
    }
    else if (name == HS_MODULE_NAME_ROBOT_CARRETILLA_GAZEBO)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_GAZEBO;
    }
    else if (name == HS_MODULE_NAME_ROBOT_ATRVJR)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_ATRVJR;
    }
    else if (name == HS_MODULE_NAME_ROBOT_FRED)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_FRED;
    }
    else if (name == HS_MODULE_NAME_ROBOT_NOMAD)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_NOMAD;
    }
    else if (name == HS_MODULE_NAME_ROBOT_PIONEER)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER;
    }
    else if (name == HS_MODULE_NAME_ROBOT_PIONEER2)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER2;
    }
    else if (name == HS_MODULE_NAME_ROBOT_URBAN)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_URBAN;
    }
    else if (name == HS_MODULE_NAME_ROBOT_CARMEN)
    {
        sensorType = POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARMEN;
    }
    else
    {
        sensorType = POSECALC_SENSOR_TYPE_NONE;
    }

    return sensorType;
}

//-----------------------------------------------------------------------
// This function extracts the location info from the pose info.
//-----------------------------------------------------------------------
PoseCalcLocationInfo_t PoseCalcUtility::extractLocationInfoFromPoseInfo(
    PoseCalcPoseInfo_t poseInfo)
{
    PoseCalcLocationInfo_t locInfo;

    locInfo.sensorClass = poseInfo.sensorClass;
    locInfo.sensorType = poseInfo.sensorType;
    locInfo.refTime = poseInfo.locRefTime;
    locInfo.loc = poseInfo.pose.loc;
    locInfo.rawLoc = poseInfo.rawPose.loc;
    locInfo.confidence = poseInfo.confidence.locConfidence;
    locInfo.variance = poseInfo.variance.locVariance;
    locInfo.extra = poseInfo.extra;

    return locInfo;
}

//-----------------------------------------------------------------------
// This function extracts the rotation info from the pose info.
//-----------------------------------------------------------------------
PoseCalcRotationInfo_t PoseCalcUtility::extractRotationInfoFromPoseInfo(
    PoseCalcPoseInfo_t poseInfo)
{
    PoseCalcRotationInfo_t rotInfo;

    rotInfo.sensorClass = poseInfo.sensorClass;
    rotInfo.sensorType = poseInfo.sensorType;
    rotInfo.refTime = poseInfo.rotRefTime;
    rotInfo.rot = poseInfo.pose.rot;
    rotInfo.rawRot = poseInfo.rawPose.rot;
    rotInfo.confidence = poseInfo.confidence.rotConfidence;
    rotInfo.variance = poseInfo.variance.rotVariance;
    rotInfo.extra = poseInfo.extra;

    return rotInfo;
}

/**********************************************************************
# $Log: PCUtility.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:16  endo
# New PoseCalc integrated.
#
#**********************************************************************/
