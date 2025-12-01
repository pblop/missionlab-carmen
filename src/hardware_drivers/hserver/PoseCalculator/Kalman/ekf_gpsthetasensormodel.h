#ifndef EKF_GPSTHETASENSORMODEL_H
#define EKF_GPSTHETASENSORMODEL_H

#include "kf_sensormodel.h"

class EKF_GpsThetaSensorModel : public KF_SensorModel {
 public:
  EKF_GpsThetaSensorModel(char *name, mxn_Matrix &var);
  ~EKF_GpsThetaSensorModel(void);
  mxn_Matrix Get_Model(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ModelJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_Measurement(void);
  void Set_Measurement(double val);
  mxn_Matrix Get_Measurement(mxn_Matrix &state);
  

 private:
  double GpsThetaVal;

};

#endif
 
