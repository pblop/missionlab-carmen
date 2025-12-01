/**********************************************************************
 **                                                                  **
 **                          ekf_localize.cpp                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ekf_localize.cpp,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdio.h>
#include "math.h"
#include "ekf_localize.h"

extern bool SilentMode;
bool stopprint;
extern void printfTextWindow(const char* arg, ...);

#define EKFLOCALIZE_DEG2RAD(val) ((val)*M_PI/180.0)
#define POSECALCMODULE_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}


EKF_Localize::EKF_Localize(void) {

  mxn_Matrix var(3,1);
  mxn_Matrix var2(2,1);
  mxn_Matrix var3(1,1);


  EKF = new ExtendKalmanFilter();
  var[0][0] = 0.5;
  var[1][0] = 0.5;
  var[2][0] = 0.1;
  Promod = new EKF_LocateProcessModel("LocationProcess", var, 0.0);


  // add the compass sensor model
  var3[0][0] = 0.2;
  ThetaSensor = new EKF_CompassSensorModel("CompassSensor", var3);

  // add the gps xy sensor model
  var2[0][0] = 1.0;
  var2[1][0] = 1.0;
  XYSensor = new EKF_XYSensorModel("GPSXYSensor", var2);
  EKF->Add_StateModel(Promod);
  Sensor_Index[0] = EKF->Add_SensorModel(ThetaSensor);
  Sensor_Index[1] = EKF->Add_SensorModel(XYSensor);

  GyroOffset = 0;

}

EKF_Localize::~EKF_Localize(void) {
  delete EKF;
}


void EKF_Localize::Update_MM(double dx, double dy, double dt, double varxy, double vart) {
  mxn_Matrix var(3,1);
  mxn_Matrix a;

  a = EKF->Get_State();

  var[0][0] = varxy;
  var[1][0] = varxy;
  var[2][0] = vart;

  Promod->Set_Variance(var);

  // now we update the motion model
  mxn_Matrix cont(3,1);
  cont[0][0] = dx;
  cont[1][0] = dy;
  cont[2][0] = dt;

  EKF->Update(0.1, cont);
  a = EKF->Get_State();
}


void EKF_Localize::Update_S(PoseCalcPoseInfo_t &sensor, PoseCalcPoseVariance_t variance) {
    //FILE *infile;
    mxn_Matrix var(1,1);
    mxn_Matrix var2(2,1);
    mxn_Matrix state(3,1);

    switch (sensor.sensorType) {
    case POSECALC_SENSOR_TYPE_COMPASS_3DMG:
    case POSECALC_SENSOR_TYPE_COMPASS_KVHC100:
    case POSECALC_SENSOR_TYPE_COMPASS_CAN:
        state = Get_State();
        while (sensor.pose.rot.yaw - state[2][0] > 180) {
            sensor.pose.rot.yaw -= 360;
        }
        while (sensor.pose.rot.yaw - state[2][0] <= -180) {
            sensor.pose.rot.yaw += 360;
        }


        var[0][0] = EKFLOCALIZE_DEG2RAD(variance.rotVariance.yaw);

        ThetaSensor->Set_Variance(var);
        ThetaSensor->Set_Measurement(sensor.pose.rot.yaw);
        EKF->Add_Measurement(0);
        break;

    case POSECALC_SENSOR_TYPE_GYRO_DMUVGX:
    case POSECALC_SENSOR_TYPE_GYRO_CAN:

        state = Get_State();
        sensor.rawPose.rot.yaw += GyroOffset;
        while (sensor.rawPose.rot.yaw - state[2][0] > 180) {
            sensor.rawPose.rot.yaw -= 360;
        }
        while (sensor.rawPose.rot.yaw - state[2][0] <= -180) {
            sensor.rawPose.rot.yaw += 360;
        }

        var[0][0] = EKFLOCALIZE_DEG2RAD(variance.rotVariance.yaw);

        ThetaSensor->Set_Variance(var);
        ThetaSensor->Set_Measurement(sensor.rawPose.rot.yaw);
        EKF->Add_Measurement(0);
        break;

    case POSECALC_SENSOR_TYPE_GPS:

        if (sensor.pose.rot.extra.time != 0.0) {

            state = Get_State();
            while (sensor.pose.rot.yaw - state[2][0] > 180) {
                sensor.pose.rot.yaw -= 360;
            }
            while (sensor.pose.rot.yaw - state[2][0] <= -180) {
                sensor.pose.rot.yaw += 360;
            }

            var[0][0] = EKFLOCALIZE_DEG2RAD(variance.rotVariance.yaw);
            ThetaSensor->Set_Variance(var);
            ThetaSensor->Set_Measurement(sensor.pose.rot.yaw);
            EKF->Add_Measurement(0);
        }

        if (sensor.pose.loc.extra.time != 0.0) {
            var2[0][0] = variance.locVariance.x;
            var2[1][0] = variance.locVariance.y;
            XYSensor->Set_Variance(var2);
            XYSensor->Set_Measurement(sensor.pose.loc.x, sensor.pose.loc.y);
            EKF->Add_Measurement(1);
        }
        break;

    default:
        break;
    }



}


// Get the current state estimate for the position
mxn_Matrix EKF_Localize::Get_State(void) {
  mxn_Matrix tempm = EKF->Get_State();
  tempm[2][0] = tempm[2][0] * 360.0 / (2.0 * M_PI);
  POSECALCMODULE_CRAMPDEG(tempm[2][0], 0, 360);

  return tempm;
}


// init the ekf with some hand picked variances and the current fused position
void EKF_Localize::Init(PoseCalcFusedPose_t pos) {
  mxn_Matrix loc(3,1);
  mxn_Matrix var(3,1);
  //static int cc = 0;

  loc[0][0] = pos.pose.loc.x;
  loc[1][0] = pos.pose.loc.y;
  loc[2][0] = pos.pose.rot.yaw * 2 * M_PI / 360.0;
  GyroOffset = (int)(pos.pose.rot.yaw);

  var[0][0] = 0.01;
  var[1][0] = 0.01;
  var[2][0] = 0.1;

  EKF->Add_InitialStateEstimate(loc, var);

}

/**********************************************************************
# $Log: ekf_localize.cpp,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
