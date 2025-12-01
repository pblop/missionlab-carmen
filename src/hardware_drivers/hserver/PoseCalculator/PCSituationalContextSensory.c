/**********************************************************************
 **                                                                  **
 **                    PCSituationalContextSensory.c                 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSituationalContextSensory.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "PCSituationalContextSensory.h"
#include "PoseCalculatorTypes.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void printfTextWindow(const char* arg, ...);

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for PoseCalcSituationalContextSensory
//-----------------------------------------------------------------------
PoseCalcSituationalContextSensory::PoseCalcSituationalContextSensory(void)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcSituationalContextSensory
//-----------------------------------------------------------------------
PoseCalcSituationalContextSensory::~PoseCalcSituationalContextSensory(void)
{
}

//-----------------------------------------------------------------------
// This function returns the situational value.
//-----------------------------------------------------------------------
double PoseCalcSituationalContextSensory::getSituationalValue_(
    int sensorType,
    int poseType)
{
    double situationalValue = 0;

    switch (poseType) {

    case POSECALC_POSE_TYPE_LOCATION:

        switch (sensorType) {

        case POSECALC_SENSOR_TYPE_GPS:
            situationalValue = 0.9;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_AMIGOBOT:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROBOTLASER:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA560:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA:
            situationalValue = 0.8;
            break;

       case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_SIMULACION:
            situationalValue = 0.8;
            break;

       case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_GAZEBO:
            situationalValue = 0.8;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ATRVJR:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_FRED:
            situationalValue = 0.6;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_NOMAD:
            situationalValue = 0.6;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER2:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_URBAN:
            situationalValue = 0.4;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARMEN:
            situationalValue = 0.5;
            break;
        }

        break;

    case POSECALC_POSE_TYPE_ROTATION:

        switch (sensorType) {

        case POSECALC_SENSOR_TYPE_COMPASS_3DMG:
            //situationalValue = 0.9;
            situationalValue = 0.9;
            break;

        case POSECALC_SENSOR_TYPE_COMPASS_KVHC100:
            situationalValue = 0.7;
            break;

        case POSECALC_SENSOR_TYPE_COMPASS_CAN:
            situationalValue = 0.8;
            break;

        case POSECALC_SENSOR_TYPE_GPS:
            //situationalValue = 0.8;
            situationalValue = 0.9;
            break;

        case POSECALC_SENSOR_TYPE_GYRO_DMUVGX:
            situationalValue = 0.75;
            break;

        case POSECALC_SENSOR_TYPE_GYRO_CAN:
            situationalValue = 0.75;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_AMIGOBOT:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROBOTLASER:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA:
            situationalValue = 0.5;
            break;

		case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ROOMBA560:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA:
            situationalValue = 0.8;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_SIMULACION:
            situationalValue = 0.8;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARRETILLA_GAZEBO:
            situationalValue = 0.8;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_ATRVJR:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_FRED:
            situationalValue = 0.6;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_NOMAD:
            situationalValue = 0.6;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_PIONEER2:
            situationalValue = 0.5;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_URBAN:
            situationalValue = 0.4;
            break;

        case POSECALC_SENSOR_TYPE_ROBOTSHAFT_CARMEN:
            situationalValue = 0.5;
            break;
        }

        break;
    }

    return situationalValue;
}

//-----------------------------------------------------------------------
// This function applies the situational value to the confidence.
//-----------------------------------------------------------------------
void PoseCalcSituationalContextSensory::applySituation(PoseCalcGenericData_t &data)
{
    PoseCalcPoseInfo_t *poseInfo = NULL;
    double situationalValue = 0;

    poseInfo = (PoseCalcPoseInfo_t *)(data.content);

    situationalValue = getSituationalValue_(
        poseInfo->sensorType,
        poseInfo->poseType);

    switch (poseInfo->poseType) {

    case POSECALC_POSE_TYPE_LOCATION:
        (poseInfo->confidence.locConfidence.value) *= situationalValue;
        break;

    case POSECALC_POSE_TYPE_ROTATION:
        (poseInfo->confidence.rotConfidence.value) *= situationalValue;
        break;
    }
}

/**********************************************************************
# $Log: PCSituationalContextSensory.c,v $
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
