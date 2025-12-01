#ifndef EKF_GYROSENSORMODEL_H
#define EKF_GYROSENSORMODEL_H

#include "ekf_compasssensormodel.h"

class EKF_GyroSensorModel : public KF_SensorModel {
 public:
  EKF_GyroSensorModel(char *name, mxn_Matrix &var);
  ~EKF_GyroSensorModel(void);
  mxn_Matrix Get_Model(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ModelJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_Measurement(void);
  void Set_Measurement(double val);
  mxn_Matrix Get_Measurement(mxn_Matrix &state);
  

 private:
  double GyroVal;

};

#endif
 
