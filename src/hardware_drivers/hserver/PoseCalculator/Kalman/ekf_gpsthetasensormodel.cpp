#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <math.h>
#include "mxn_matrix.h"
#include "kf_sensormodel.h"
#include "ekf_gpsthetasensormodel.h"

#define D2R(val) ((val)*M_PI/180.0)

EKF_GpsThetaSensorModel::EKF_GpsThetaSensorModel(char *name, mxn_Matrix &var) : KF_SensorModel(name, var) {
}

EKF_GpsThetaSensorModel::~EKF_GpsThetaSensorModel(void) {

  free(Name);
}

void EKF_GpsThetaSensorModel::Set_Measurement(double val) {
  GpsThetaVal = D2R(val);
  printf("Setting GpsThetaVal: %f\n", GpsThetaVal);
}

mxn_Matrix EKF_GpsThetaSensorModel::Get_Measurement(void) {
  mxn_Matrix tempm(1,1);
  //tempm[0][0] = 0.0;
  //tempm[1][0] = 0.0;
  tempm[0][0] = GpsThetaVal;

  return tempm;
}

mxn_Matrix EKF_GpsThetaSensorModel::Get_Measurement(mxn_Matrix &state) {
  mxn_Matrix tempm(1,1);
  //tempm[0][0] = 0.0;
  //tempm[1][0] = 0.0;
  printf("GpsThetaVal: %f\n", GpsThetaVal);
  tempm[0][0] = GpsThetaVal;
  return tempm;
}



mxn_Matrix EKF_GpsThetaSensorModel::Get_Model(double dtime, mxn_Matrix &statemodel) {
  
  mxn_Matrix promod(1,1);
  promod[0][0] = 1.0;

  return promod;
}

mxn_Matrix EKF_GpsThetaSensorModel::Get_ModelJacobian(double dtime, mxn_Matrix &statemodel) {
  mxn_Matrix jac(1,3);
  jac[0][0] = 0.0;
  jac[0][1] = 0.0;
  jac[0][2] = 1.0;
  //jac[1][0] = 0.0;
  //jac[1][1] = 1.0;
  //jac[1][2] = 0.0;
  //jac[2][0] = 0.0;
  //jac[2][1] = 0.0;
  //jac[2][2] = 1.0;
  
  return jac;
}

mxn_Matrix EKF_GpsThetaSensorModel::Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel) {
 
  mxn_Matrix jac(3,3);
  jac[0][0] = 0.0;
  jac[0][1] = 0.0;
  jac[0][2] = 0.0;
  jac[1][0] = 0.0;
  jac[1][1] = 0.0;
  jac[1][2] = 0.0;
  jac[2][0] = 0.0;
  jac[2][1] = 0.0;
  jac[2][2] = 1.0;

  return jac;
}

  



