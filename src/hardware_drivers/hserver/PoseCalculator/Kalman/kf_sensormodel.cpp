#include <string.h>
#include <stdio.h>
#include "kf_sensormodel.h"

KF_SensorModel::KF_SensorModel(char *name, mxn_Matrix &var) {

  Name = strdup(name);
  Variance = var;
  Time = 0.0;
}

KF_SensorModel::~KF_SensorModel(void) {
  free(Name);
}


char *KF_SensorModel::Get_Name(void) {
  
  return Name;
}


mxn_Matrix *KF_SensorModel::Get_Variance(void) {
  
  return &Variance;
}

void KF_SensorModel::Set_Variance(mxn_Matrix var) {
  Variance = var;
}
