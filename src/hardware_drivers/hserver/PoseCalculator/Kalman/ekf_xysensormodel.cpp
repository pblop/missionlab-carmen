#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "mxn_matrix.h"
#include "kf_sensormodel.h"
#include "ekf_xysensormodel.h"

EKF_XYSensorModel::EKF_XYSensorModel(char *name, mxn_Matrix &var) : KF_SensorModel(name, var) {
}

EKF_XYSensorModel::~EKF_XYSensorModel(void) {

  free(Name);
}

void EKF_XYSensorModel::Set_Measurement(double x, double y) {
  XVal = x;
  YVal = y;
}

mxn_Matrix EKF_XYSensorModel::Get_Measurement(void) {
  mxn_Matrix tempm(3,1);
  tempm[0][0] = 0.0;
  tempm[1][0] = 0.0;
  tempm[2][0] = 0.0;  // read sensor data


  return tempm;
}

mxn_Matrix EKF_XYSensorModel::Get_Measurement(mxn_Matrix &state) {
  mxn_Matrix tempm(2,1);

  tempm[0][0] = XVal;
  tempm[1][0] = YVal;

  return tempm;
}


mxn_Matrix EKF_XYSensorModel::Get_Model(double dtime, mxn_Matrix &statemodel) {
  
  mxn_Matrix promod(1,1);
  promod[0][0] = 1.0;

  return promod;
}

mxn_Matrix EKF_XYSensorModel::Get_ModelJacobian(double dtime, mxn_Matrix &statemodel) {
  mxn_Matrix jac(2,3);
  jac[0][0] = 1.0;
  jac[0][1] = 0.0;
  jac[0][2] = 0.0;
  jac[1][0] = 0.0;
  jac[1][1] = 1.0;
  jac[1][2] = 0.0;
  
  return jac;
}

mxn_Matrix EKF_XYSensorModel::Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel) {
 
  mxn_Matrix jac(3,3);
  jac[0][0] = 1.0;
  jac[1][0] = 0.0;
  jac[2][0] = 0.0;
  jac[0][1] = 0.0;
  jac[1][1] = 1.0;
  jac[2][1] = 0.0;
  jac[0][2] = 0.0;
  jac[1][2] = 0.0;
  jac[2][2] = 0.0;

  return jac;
} 
  



