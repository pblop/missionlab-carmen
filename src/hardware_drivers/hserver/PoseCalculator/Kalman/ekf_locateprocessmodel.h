#ifndef EKF_LOCATEPROCESSMODEL_H
#define EKF_LOCATEPROCESSMODEL_H

#include "kf_processmodel.h"

class EKF_LocateProcessModel : public KF_ProcessModel {
 public:
  EKF_LocateProcessModel(char *name, mxn_Matrix &var, double initval);
  ~EKF_LocateProcessModel(void);
  mxn_Matrix Update(double dtime);
  mxn_Matrix Update(double dtime, mxn_Matrix &state, mxn_Matrix &control);
  mxn_Matrix Get_ErrorJacobian(void);
  mxn_Matrix Get_ProcessJacobian(void);

 private:
  mxn_Matrix ProcessJacobian;
};

#endif

    

  
