/**********************************************************************
 **                                                                  **
 **                          ekf_localize.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ekf_localize.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef EKF_LOCALIZE_H
#define EKF_LOCALIZE_H

#include "extend_kalman_filter.h"
#include "ekf_locateprocessmodel.h"
#include "ekf_compasssensormodel.h"
#include "ekf_gyrosensormodel.h"
#include "ekf_gpsthetasensormodel.h"
#include "ekf_xysensormodel.h"
#include "PoseCalculatorTypes.h"


//#define EPSILON    0.001
                          
class EKF_Localize {
 public:
  EKF_Localize(void);
  ~EKF_Localize(void);
  void Init(PoseCalcFusedPose_t pos);
  void Update_MM(double dx, double dy, double dt, double varxy, double vart);
  void Update_S(PoseCalcPoseInfo_t &sensor, PoseCalcPoseVariance_t variance);
  mxn_Matrix Get_State(void);

 private:
    int GyroOffset;
  ExtendKalmanFilter *EKF;
  EKF_LocateProcessModel *Promod;
  EKF_CompassSensorModel *ThetaSensor;
  EKF_XYSensorModel *XYSensor;
  int Sensor_Index[2];

};

#endif

/**********************************************************************
# $Log: ekf_localize.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
