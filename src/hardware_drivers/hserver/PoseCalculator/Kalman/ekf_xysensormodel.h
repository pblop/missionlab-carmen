#ifndef EKF_XYSENSORMODEL_H
#define EKF_XYSENSORMODEL_H

#include "ekf_xysensormodel.h"

class EKF_XYSensorModel : public KF_SensorModel {
 public:
  EKF_XYSensorModel(char *name, mxn_Matrix &var);
  ~EKF_XYSensorModel(void);
  mxn_Matrix Get_Model(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ModelJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel);
  mxn_Matrix Get_Measurement(void);
  mxn_Matrix Get_Measurement(mxn_Matrix &state);
  void Set_Measurement(double x, double y);

 private:

  double XVal;
  double YVal;
};

#endif

    

  
