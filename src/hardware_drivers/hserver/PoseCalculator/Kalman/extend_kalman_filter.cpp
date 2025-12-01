#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "kf_processmodel.h"
#include "extend_kalman_filter.h"

extern void printfTextWindow(const char* arg, ...);

// Constructor: Inits process models to NULL
ExtendKalmanFilter::ExtendKalmanFilter(void) {

  // initialize all our process models to NULL
  StateModel_Pro = NULL;
  ControlModel_Pro = NULL;
  SensorModel_Pro = NULL;
  Num_Sensors = 0;
}

// Deconstructor
ExtendKalmanFilter::~ExtendKalmanFilter(void) {
  int cnt;

  if (StateModel_Pro != NULL) {
    delete StateModel_Pro;
  }
  if (ControlModel_Pro != NULL) {
    delete ControlModel_Pro;
  }
  if (Num_Sensors != 0) {
    for (cnt = 0; cnt < Num_Sensors; cnt++) {
      delete SensorModel_Pro[cnt];
    }
    delete []SensorModel_Pro;
  }

}

mxn_Matrix ExtendKalmanFilter::Get_State(void) {
  return StateEstimate_k1;
}  

// Add a new state process model to the filter
void ExtendKalmanFilter::Add_StateModel(KF_ProcessModel *processmodel) {
  
  // if we already have a model delete it
  if (StateModel_Pro != NULL) {
    delete StateModel_Pro;
  }

  // and get its variance matrix
  StateModel_Pro = processmodel;
  StateVariance = *(StateModel_Pro->Get_Variance());
}


// Add a new control process model to the filter
void ExtendKalmanFilter::Add_ControlModel(KF_ProcessModel *processmodel) {
  
  // check if we have to delete the old one
  if (ControlModel_Pro != NULL) {
    delete ControlModel_Pro;
  }

  ControlModel_Pro = processmodel;
}

// Add a sensor process model to the filter,  it will return a sensor it number used for updating
int ExtendKalmanFilter::Add_SensorModel(KF_SensorModel *sensormodel) {
  int cnt;
  KF_SensorModel **tempsens;


  // make room for new sensor
  tempsens = new KF_SensorModel *[Num_Sensors +1];
  if (!tempsens) {
    printf("ExtendKalmanFilter:Add_SensorModel: Out of memory!\n");
    exit(1);
  }
  
  // copy over old sensors
  for (cnt = 0; cnt < Num_Sensors; cnt++) {
    tempsens[cnt] = SensorModel_Pro[cnt];
  }

  // add new sensor and delete aold sensors
  tempsens[Num_Sensors] = sensormodel;
  delete []SensorModel_Pro;
  SensorModel_Pro = tempsens;
  Num_Sensors++;

  return Num_Sensors - 1;
}


// Update the filter with a sensor measurement from sensor #  sensornum)
void ExtendKalmanFilter::Add_Measurement(int sensornum) {
  mxn_Matrix sensepro;
  mxn_Matrix sensevar;
  mxn_Matrix sensemeas;
  mxn_Matrix tempmat;
  mxn_Matrix senseerrjac;

  mxn_Matrix tempest;
  mxn_Matrix tempvar;
  mxn_Matrix aaa;


  if ((sensornum < 0) || (sensornum >= Num_Sensors)) {
    printf("ExtendKalmanFilter: Add_Measurement: Trying to update nonexisting sensors\n");
    return;
  }

  sensepro = SensorModel_Pro[sensornum]->Get_ModelJacobian(0, StateEstimate_k1);
 

  sensevar = *(SensorModel_Pro[sensornum]->Get_Variance());

  // printfTextWindow("svar: %f\n", sensevar[0][0]);

  senseerrjac = SensorModel_Pro[sensornum]->Get_ErrorJacobian(0, StateEstimate_k1);

  sensemeas = SensorModel_Pro[sensornum]->Get_Measurement(StateEstimate_k1);

  
  // calculate the kalman gain
  // Kk = Pk * H.t * (H * Pk * H.t + V * R * V.t) ^ -1
  mxn_Matrix sensevarcovar = (sensevar * sensevar.Transpose()).Diag();
//  if (sensevarcovar.N == 2) {
//      printfTextWindow("sensevarcovar: %f %f %f %f\n", sensevarcovar[0][0], sensevarcovar[1][0], sensevarcovar[0][1], sensevarcovar[1][1]);
//      mxn_Matrix ssss = ((sensepro * StateEstCert_k1 * sensepro.Transpose() + sensevarcovar).Inverse_GJ());
//      printfTextWindow("dddd: %f %f %f %f\n", ssss[0][0], ssss[1][0], ssss[0][1], ssss[1][1]);
//  } 

  Kalman_Gain = (StateEstCert_k1 * sensepro.Transpose()) * ((sensepro * StateEstCert_k1 * sensepro.Transpose() + sensevarcovar).Inverse_GJ());
  
//  printfTextWindow("KG: %f\n", Kalman_Gain[0][0]);


  // update the state estimate with the sensor reading
  // Xk = Xk + Kk * (Zk - H * Xk)
  tempest = StateEstimate_k1 + (Kalman_Gain * (sensemeas - (sensepro * StateEstimate_k1)));
  StateEstimate_k1 = tempest;

  // update the error covariance
  // Pk = (I - Kk * H) Pk
  tempmat = Kalman_Gain * sensepro;
  tempvar = mxn_Matrix((tempmat.Identity() - tempmat) * StateEstCert_k1);
  StateEstCert_k1 = tempvar.Diag();
}


// Set the initial state and certainty estimates
void ExtendKalmanFilter::Add_InitialStateEstimate(mxn_Matrix &stateest, mxn_Matrix &certest) {

  StateEstimate_k = stateest;
  StateEstimate_k1 = stateest;
  
  StateEstCert_k = (certest * certest.Transpose()).Diag();
  StateEstCert_k1 = StateEstCert_k;

}

  
// Update the filter based on the time and possible control input
void ExtendKalmanFilter::Update(double dtime, mxn_Matrix &controlinput) {
  
  ControlState = controlinput;

  Update_State(dtime); 

  Update_ErrorCovar();
}
  

// Update the state estimate base on time  
void ExtendKalmanFilter::Update_State(double dtime) {
  mxn_Matrix controlinput;

  // store the old state estimate
  StateEstimate_k = StateEstimate_k1;

  StateEstimate_k1 = StateModel_Pro->Update(dtime, StateEstimate_k, ControlState);
}


// updates the error covariance currently assumes error jacobian is all 1's
void ExtendKalmanFilter::Update_ErrorCovar(void) {
  mxn_Matrix errorjac;

  errorjac = StateModel_Pro->Get_ErrorJacobian();
  StateEstCert_k = StateEstCert_k1;

  StateEstCert_k1 = ((StateModel_Pro->Get_ProcessJacobian() * StateEstCert_k * StateModel_Pro->Get_ProcessJacobian().Transpose()) + (errorjac * (StateVariance * StateVariance.Transpose()) * errorjac.Transpose())).Diag();
}

