#ifndef KF_PROCESSMODEL_H
#define KF_PROCESSMODEL_H

#include "mxn_matrix.h"

class KF_ProcessModel {
 public:
  KF_ProcessModel(char *name, mxn_Matrix &var, double initval);
  virtual ~KF_ProcessModel(void);
  char *Get_Name(void);
  double Get_Xk(void);
  virtual mxn_Matrix Update(double dtime) = 0;
  virtual mxn_Matrix Update(double dtime, mxn_Matrix &state, mxn_Matrix &control) = 0;
  mxn_Matrix *Get_Variance(void);
  void Set_Variance(mxn_Matrix &var);
  virtual mxn_Matrix Get_ErrorJacobian(void) = 0;
  virtual mxn_Matrix Get_ProcessJacobian(void) = 0;

 protected:
  char *Name;

  mxn_Matrix Variance;

  double Xk;
  double Xk_1;

  double Time;
};


#endif
