#ifndef KF_SENSORMODEL_H
#define KF_SENSORMODEL_H

#include <stdlib.h>
#include "mxn_matrix.h"

class KF_SensorModel {
 public:
  KF_SensorModel(char *name, mxn_Matrix &var);
  virtual ~KF_SensorModel(void);
  char *Get_Name(void);
  virtual mxn_Matrix Get_Model(double dtime, mxn_Matrix &statemodel) = 0;
  virtual mxn_Matrix Get_Measurement(void) = 0;
  virtual mxn_Matrix Get_Measurement(mxn_Matrix &state) = 0;
  virtual mxn_Matrix Get_ModelJacobian(double dtime, mxn_Matrix &statemodel) = 0 ;
  virtual mxn_Matrix Get_ErrorJacobian(double dtime, mxn_Matrix &statemodel) = 0;
  mxn_Matrix *Get_Variance(void);
  void Set_Variance(mxn_Matrix var);


 protected:
  char *Name;
  mxn_Matrix Variance;
  double Time;
};


#endif
