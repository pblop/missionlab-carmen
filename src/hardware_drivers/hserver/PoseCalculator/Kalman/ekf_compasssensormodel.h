#ifndef EKF_COMPASSSENSORMODEL_H
#define EKF_COMPASSSENSORMODEL_H

#include "ekf_compasssensormodel.h"

class EKF_CompassSensorModel : public KF_SensorModel {
 public:
  EKF_CompassSensorModel(char *name, mxn_Matrix &var);
  ~EKF_CompassSensorModel(void);
  mxn_Matrix Get_Model(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ModelJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_Measurement(void);
  void Set_Measurement(double val);
  mxn_Matrix Get_Measurement(mxn_Matrix &state);
  

 private:
  double CompassVal;

};

#endif
 
