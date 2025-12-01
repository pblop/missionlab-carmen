#include <string.h>
#include <stdio.h>
#include <math.h>
#include "kf_processmodel.h"



#define D2R(val) ((val)*M_PI/180.0)

KF_ProcessModel::KF_ProcessModel(char *name, mxn_Matrix &var, double initval) {

  Name = strdup(name);
  Xk = 0.0;
  Xk_1 = initval;
  Variance = var;
  Time = 0.0;
}

KF_ProcessModel::~KF_ProcessModel(void) {

  // nothing to see here!
}


char *KF_ProcessModel::Get_Name(void) {
  
  return Name;
}

double KF_ProcessModel::Get_Xk(void) {
 
  return Xk;
}

mxn_Matrix *KF_ProcessModel::Get_Variance(void) {
  
  return &Variance;
}

void KF_ProcessModel::Set_Variance(mxn_Matrix &var) {

  Variance = var;
}
