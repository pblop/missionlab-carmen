/**********************************************************************
 **                                                                  **
 **                      Kalman.c                                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: Kalman.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "Kalman.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculator.h"
#include "./Kalman/mxn_matrix.h"

#define EKFLOCALIZE_DEG2RAD(val) ((val)*M_PI/180.0)
#define D2R(val) ((val)*M_PI/180.0)

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern double getCurrentEpochTime(void);
extern void printfTextWindow(const char* arg, ...);
extern bool SilentMode;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for EKF
//-----------------------------------------------------------------------
Kalman::Kalman(
    PoseCalcSensoryDataBusInterface *sensoryDataBus) :
    PoseCalcSensorFuser(sensoryDataBus,POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER) {
  
  localizeEKF = new EKF_Localize();
  EKFInit = false;

  OdOffsetX = 0;
  OdOffsetY = 0;
  OdOffsetTheta = 0;
  LastX = 0;
  LastY = 0;
  LastTheta = 0;
  // ENDO
  LastThetaOd = 0;
  LastThetaGyro = 0;

  initialize();
}

//-----------------------------------------------------------------------
// Destructor for Kalman
//-----------------------------------------------------------------------
Kalman::~Kalman(void) {

  delete localizeEKF;
}

//-----------------------------------------------------------------------
// This function returns the output of the fused sensor information.
//-----------------------------------------------------------------------
PoseCalcFusedPose_t Kalman::getOutput(void) {
    PoseCalcFusedPose_t fusedPose;
    vector<PoseCalcPoseInfo_t> poseData;
    int i;
    float newx = 0, newy = 0, newtheta;
    float dx, dy, dtheta;
    bool GotOd = false;
    mxn_Matrix state;
    mxn_Matrix pastState;
    mxn_Matrix rot(2,2);
    mxn_Matrix od(2,1);
    mxn_Matrix roted(2,1);
    double variancerot, varianceloc;
    

    // Get the latest location and rotation data from the sensory bus.
    poseData = sensoryDataBus_->getLatestPoseData(FUSER_TYPE_);
    
    // Initialize data.
    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.extra.validPose = false;
    fusedPose.extra.time = getCurrentEpochTime();
    

    // Get the location with the highest confidence. Do not use the data if
    // it was already read previously.
    for (i = 0; i < (int)(poseData.size()); i++) {

        // first we update based on odometry;
        switch(poseData[i].sensorClass) {
        case POSECALC_SENSOR_CLASS_ROBOTSHAFT:
            newx = poseData[i].rawPose.loc.x;
            newy = poseData[i].rawPose.loc.y;
            newtheta = poseData[i].rawPose.rot.yaw;

            if (newtheta < 0) {
                newtheta += 360;
            }   
            if (newtheta > 360) {
                newtheta -= 360;
            }       

            dx = newx - LastX;
            dy = newy - LastY;
            // ENDO
            //dtheta = newtheta - LastTheta;
            dtheta = newtheta - LastThetaOd;

            rot[0][0] = cos(D2R(-OdOffsetTheta));
            rot[0][1] = sin(D2R(-OdOffsetTheta));
            rot[1][0] = -sin(D2R(-OdOffsetTheta));
            rot[1][1] = cos(D2R(-OdOffsetTheta));
            od[0][0] = dx;
            od[1][0] = dy;
	
            roted = rot * od;
	
            dx = roted[0][0];
            dy = roted[1][0];
            // ENDO
            //dtheta = newtheta - LastTheta;
            variancerot = poseData[i].variance.rotVariance.yaw;
            varianceloc = poseData[i].variance.locVariance.x;

            // ENDO
            LastThetaOd = newtheta;

            GotOd = true;

            break;
	
        default:
            break;
        }

    }
/*
    // get the gyro reading if available
    for (i = 0; i < (int)(poseData.size()); i++) {
     
        switch(poseData[i].sensorClass) {
        case POSECALC_SENSOR_CLASS_GYRO:
            newtheta = poseData[i].pose.rot.yaw;
            // ENDO
            //dtheta = newtheta - LastTheta;
            dtheta = newtheta - LastThetaGyro;
            variancerot = poseData[i].variance.rotVariance.yaw;

            // ENDO
            LastThetaGyro = newtheta;
	
            break;
	
        default:
            break;
        }
      
    }
*/  
    if (!GotOd) {
        return fusedPose;
    }
    // update the motion model
    pastState = localizeEKF->Get_State();
    localizeEKF->Update_MM(
        dx,
        dy,
        dtheta,
        varianceloc,
        POSECALC_DEG2RAD(variancerot));
    
    LastX = newx;
    LastY = newy;
    LastTheta = newtheta;

        
    // now go through and update all the sensors
    for (i = 0; i < (int)(poseData.size()); i++) {
        switch(poseData[i].sensorClass) {
        case POSECALC_SENSOR_CLASS_COMPASS:
        case POSECALC_SENSOR_CLASS_GPS:
        case POSECALC_SENSOR_CLASS_GYRO:
#ifdef LOG_POSE_DATA
        	fprintf(stderr, "KALMAN_SENSORS: Type=%d - X=%f Y=%f Theta=%f - locVariance=%f rotVariance=%f\n", poseData[i].sensorClass,
        			poseData[i].rawPose.loc.x, poseData[i].rawPose.loc.y, poseData[i].rawPose.rot.yaw,
        			poseData[i].variance.locVariance.x, poseData[i].variance.rotVariance.yaw);
#endif
            localizeEKF->Update_S(poseData[i], poseData[i].variance);
            break;
            
        default:
            break;
        }
    }

    fusedPose.extra.validPose = true;
    state = localizeEKF->Get_State();
    fusedPose.pose.loc.x = state[0][0];
    fusedPose.pose.loc.y = state[1][0];
    fusedPose.pose.rot.yaw = state[2][0];
    LastPose = fusedPose;

    // ENDO
    //OdOffsetTheta = fusedPose.pose.rot.yaw - LastTheta;
    OdOffsetTheta = fusedPose.pose.rot.yaw - LastThetaOd;

    if (OdOffsetTheta < 0) {
        OdOffsetTheta += 360;
    }

    if (OdOffsetTheta > 360) {
        OdOffsetTheta -= 360;
    }
    
    LastThetaGyro = fusedPose.pose.rot.yaw; // ENDO

#ifdef LOG_POSE_DATA
    fprintf(stderr, "KALMAN: pastStateX=%f pastStateY=%f pastStateTheta=%f - deltaX=%f deltaY=%f deltaTheta=%f - newStateX=%f newStateY=%f newStateTheta=%f - time=%f\n",
    				pastState[0][0], pastState[1][0], pastState[2][0], dx, dy, dtheta, fusedPose.pose.loc.x, fusedPose.pose.loc.y, fusedPose.pose.rot.yaw, fusedPose.extra.time);
#endif
    return fusedPose;
}

//-----------------------------------------------------------------------
// Reinits the EKF with current sensor information
//-----------------------------------------------------------------------
PoseCalcFusedPose_t Kalman::initialize(void) {
    PoseCalcFusedPose_t fusedPose;
    vector<PoseCalcLocationInfo_t> locData;
    vector<PoseCalcRotationInfo_t> rotData;
    double maxConfidenceLevel;
    int i;

    // Get the latest location and rotation data from the sensory bus.
    locData = sensoryDataBus_->getLatestLocationData(FUSER_TYPE_);
    rotData = sensoryDataBus_->getLatestRotationData(FUSER_TYPE_);

    // Initialize data.
    memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    fusedPose.fuserType = FUSER_TYPE_;
    fusedPose.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    fusedPose.extra.validPose = false;

    // Get the location with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(locData.size()); i++)
    {
        if ((locData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.loc = locData[i].loc;
            fusedPose.confidence.locConfidence = locData[i].confidence;
            fusedPose.variance.locVariance = locData[i].variance;
            fusedPose.extra.locSensorType = locData[i].sensorType;
            fusedPose.extra.validLoc = true;

            maxConfidenceLevel = locData[i].confidence.value;
        }
    }

    // Get the rotation with the highest confidence.
    maxConfidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;

    for (i = 0; i < (int)(rotData.size()); i++)
    {
        if ((rotData[i].confidence.value) > maxConfidenceLevel)
        {
            fusedPose.pose.rot = rotData[i].rot;
            fusedPose.confidence.rotConfidence = rotData[i].confidence;
            fusedPose.variance.rotVariance = rotData[i].variance;
            fusedPose.extra.rotSensorType = rotData[i].sensorType;
            fusedPose.extra.validRot = true;

            maxConfidenceLevel = rotData[i].confidence.value;
        }
    }

    if ((fusedPose.extra.validLoc) && (fusedPose.extra.validRot))
    {
        fusedPose.extra.validPose = true;
    }

    OdOffsetX = fusedPose.pose.loc.x - LastX;
    OdOffsetY = fusedPose.pose.loc.y - LastY;
    // ENDO
    //OdOffsetTheta = fusedPose.pose.rot.yaw - LastTheta;
    OdOffsetTheta = fusedPose.pose.rot.yaw - LastThetaOd;

    if (OdOffsetTheta < 0) {
      OdOffsetTheta += 360;
    }

    if (OdOffsetTheta > 360) {
      OdOffsetTheta -= 360;
    }

    //printfTextWindow("Kalman Init: fusedPose.pose.rot.yaw = %f  LastTheta = %f\n", fusedPose.pose.rot.yaw, LastTheta);

    //LastX = fusedPose.pose.loc.x;
    //LastY = fusedPose.pose.loc.y;
    //LastTheta = fusedPose.pose.rot.yaw;
    //LastPose = fusedPose;

    localizeEKF->Init(fusedPose);

//    GyroOffset = fusedPose.pose.rot.yaw;
    LastThetaGyro = fusedPose.pose.rot.yaw; // ENDO

    return fusedPose;
}

void Kalman::initialize(HSPose_t pose) {
    PoseCalcFusedPose_t initpose;

    memset((PoseCalcFusedPose_t *)&initpose, 0x0, sizeof(PoseCalcFusedPose_t));
    /*

    initpose.pose = pose;

    OdOffsetX = pose.loc.x - LastX;
    OdOffsetY = pose.loc.y - LastY;
    OdOffsetTheta = pose.rot.yaw - LastTheta;

    LastX = pose.loc.x;
    LastY = pose.loc.y;
    LastTheta = pose.rot.yaw;
    LastPose = initpose;

    localizeEKF->Init(initpose);
    */
}

/**********************************************************************
# $Log: Kalman.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:15  endo
# New PoseCalc integrated.
#
#**********************************************************************/
