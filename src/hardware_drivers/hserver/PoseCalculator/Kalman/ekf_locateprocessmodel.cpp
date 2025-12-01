#include <stdio.h>
#include <math.h>
#include "mxn_matrix.h"
#include "kf_processmodel.h"
#include "ekf_locateprocessmodel.h"

#define D2R(val) ((val)*M_PI/180.0)
extern void printfTextWindow(const char* arg, ...);
extern bool SilentMode;


EKF_LocateProcessModel::EKF_LocateProcessModel(char *name, mxn_Matrix &var, double initval) : KF_ProcessModel(name, var, initval) {
  // no!!! 
}

EKF_LocateProcessModel::~EKF_LocateProcessModel(void) {

  delete []Name;
}

mxn_Matrix EKF_LocateProcessModel::Update(double dtime, mxn_Matrix &state, mxn_Matrix &control) {
  double dist;
  mxn_Matrix sk1(3,1);
  mxn_Matrix rot(2,2);
  mxn_Matrix xy(2,1);
  mxn_Matrix roted(2,1);
  double radi;
  bool backward = false;

  radi = D2R(control[2][0]);
  dist = sqrt(control[0][0] * control[0][0] + control[1][0] * control[1][0]);


  
  

  if ((state[2][0] >= 0.0) && (state[2][0] <= M_PI / 2)) {

      if ((control[1][0] < -0.001) || (control[0][0] < -0.001)) {
          backward = true;
      } 
  }
  else if ((state[2][0] >= M_PI / 2) && (state[2][0] <= M_PI)) {
      if ((control[1][0] < -0.00001) || (control[0][0] > 0.00001)) {   
          backward = true;
      }
  }
  else if ((state[2][0] >= M_PI) && (state[2][0] <= 3 * M_PI / 2)) {
      if  ((control[0][0] > 0.00001) || (control[1][0] > 0.00001)) {
          backward = true;
      }
  }
  else {
      if ((control[0][0] < -0.00001) || (control[1][0] > 0.00001)) {
          backward = true;
      }     
  } 
  
  sk1[0][0] = state[0][0] + dist * cos(state[2][0] + radi / 2);
  sk1[1][0] = state[1][0] + dist * sin(state[2][0] + radi / 2);
  sk1[2][0] = state[2][0] + D2R(control[2][0]);
  if (sk1[2][0] < 0) {
    sk1[2][0] += 2 * M_PI;
  }
  else if (sk1[2][0] > 2 * M_PI) {
    sk1[2][0] -= 2 * M_PI;
  }  
  


  if (backward) {
      sk1[0][0] -= state[0][0];
      sk1[1][0] -= state[1][0];
      sk1[0][0] = sk1[0][0] * -1;
      sk1[1][0] = sk1[1][0] * -1;
      sk1[0][0] += state[0][0];
      sk1[1][0] += state[1][0];
  }





  mxn_Matrix jac(3,3);
  
  jac[0][0] = 1.0;
  jac[1][0] = 0.0;
  jac[2][0] = 0.0;
  jac[0][1] = 0.0;
  jac[1][1] = 1.0;
  jac[2][1] = 0.0;
  jac[0][2] = -dist * sin(state[2][0] + radi / 2);
  jac[1][2] = dist * cos(state[2][0] + radi / 2);
  jac[2][2] = 1.0;
  ProcessJacobian = jac;

  return sk1;
}

mxn_Matrix EKF_LocateProcessModel::Get_ErrorJacobian(void) {
  mxn_Matrix errj(3,3);

  errj[0][0] = 1.0;
  errj[1][0] = 0.0;
  errj[2][0] = 0.0;
  errj[0][1] = 0.0;
  errj[1][1] = 1.0;
  errj[2][1] = 0.0;
  errj[0][2] = 0.0;
  errj[1][2] = 0.0;
  errj[2][2] = 1.0;

  return errj;
}

mxn_Matrix EKF_LocateProcessModel::Get_ProcessJacobian(void) {
  return ProcessJacobian;
}	

mxn_Matrix EKF_LocateProcessModel::Update(double dtime) {

  mxn_Matrix promod(2,2);

  promod[0][0] = 1.0;
  promod[0][1] = dtime;
  promod[1][0] = 0.0;
  promod[1][1] = 1.0;

  return promod;
}

