#ifndef EXTEND_KALMAN_FILTER_H
#define EXTEND_KALMAN_FILTER_H

#include "mxn_matrix.h"
#include "kf_processmodel.h"
#include "kf_sensormodel.h"

/*
States wanted:

x,y,theta

*/

class ExtendKalmanFilter {
 public:
  ExtendKalmanFilter(void);
  ~ExtendKalmanFilter(void);
  void Add_StateModel(KF_ProcessModel *processmodel);
  void Add_ControlModel(KF_ProcessModel *processmodel);
  int Add_SensorModel(KF_SensorModel *sensormodel);
  void Add_InitialStateEstimate(mxn_Matrix &stateest, mxn_Matrix &certest);
  mxn_Matrix Get_State(void);

  void Update(double dtime, mxn_Matrix &controlinput);
  void Add_Measurement(int sensornum);

 private:
  void Update_State(double dtime);
  void Update_ErrorCovar(void);

  mxn_Matrix StateModel;
  mxn_Matrix StateVariance;
  
  mxn_Matrix ControlModel;
  mxn_Matrix ControlState;

  mxn_Matrix StateEstimate_k;
  mxn_Matrix StateEstimate_k1;
  mxn_Matrix StateEstCert_k;
  mxn_Matrix StateEstCert_k1;

  mxn_Matrix Kalman_Gain;
  

  mxn_Matrix EstErrorCovar_k1;

  KF_ProcessModel *StateModel_Pro;
  KF_ProcessModel *ControlModel_Pro;
  KF_SensorModel **SensorModel_Pro;


  int Num_Sensors;

  double TimeSteps;
};




#endif


