#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "kf_processmodel.h"
#include "kalman_filter.h"

// Constructor: Inits process models to NULL
KalmanFilter::KalmanFilter(void) {

  // initialize all our process models to NULL
  StateModel_Pro = NULL;
  ControlModel_Pro = NULL;
  SensorModel_Pro = NULL;
  Num_Sensors = 0;
}

// Deconstructor
KalmanFilter::~KalmanFilter(void) {
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

// Add a new state process model to the filter
void KalmanFilter::Add_StateModel(KF_ProcessModel *processmodel) {
  
  // if we already have a model delete it
  if (StateModel_Pro != NULL) {
    delete StateModel_Pro;
  }

  // and get its variance matrix
  StateModel_Pro = processmodel;
  StateVariance = *(StateModel_Pro->Get_Variance());
}


// Add a new control process model to the filter
void KalmanFilter::Add_ControlModel(KF_ProcessModel *processmodel) {
  
  // check if we have to delete the old one
  if (ControlModel_Pro != NULL) {
    delete ControlModel_Pro;
  }

  ControlModel_Pro = processmodel;
}

// Add a sensor process model to the filter,  it will return a sensor it number used for updating
int KalmanFilter::Add_SensorModel(KF_SensorModel *sensormodel) {
  int cnt;
  KF_SensorModel **tempsens;

  // make room for new sensor
  tempsens = new KF_SensorModel *[Num_Sensors +1];
  if (!tempsens) {
    printf("KalmanFilter:Add_SensorModel: Out of memory!\n");
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
void KalmanFilter::Add_Measurement(int sensornum) {
  mxn_Matrix sensepro;
  mxn_Matrix sensevar;
  mxn_Matrix sensemeas;
  mxn_Matrix tempmat;

  mxn_Matrix tempest;
  mxn_Matrix tempvar;

  if ((sensornum < 0) || (sensornum >= Num_Sensors)) {
    printf("KalmanFilter: Add_Measurement: Trying to update nonexisting sensors\n");
    return;
  }

  sensepro = SensorModel_Pro[sensornum]->Get_Model(0, StateEstimate_k1);
  sensevar = *(SensorModel_Pro[sensornum]->Get_Variance());
  sensemeas = SensorModel_Pro[sensornum]->Get_Measurement();

  printf("Got Measurement:\n");
  sensemeas.Print_Matrix();
  
  // calculate the kalman gain
  // Kk = Pk * H.t * (H * Pk * H.t + R) ^ -1
  mxn_Matrix sensevarcovar = sensevar * sensevar.Transpose();
  Kalman_Gain = (StateEstCert_k1 * sensepro.Transpose()) * ((sensepro * StateEstCert_k1 * sensepro.Transpose() + sensevarcovar).Inverse_GJ());



  // update the state estimate with the sensor reading
  // Xk = Xk + Kk * (Zk - H * Xk)
  tempest = StateEstimate_k1 + (Kalman_Gain * (sensemeas - (sensepro * StateEstimate_k1)));
  StateEstimate_k1 = tempest;

  // update the error covariance
  // Pk = (I - Kk * H) Pk
  tempmat = Kalman_Gain * sensepro;
  tempvar = mxn_Matrix((tempmat.Identity() - tempmat) * StateEstCert_k1);
  StateEstCert_k1 = tempvar;

  printf("New Est:\n");
  StateEstimate_k1.Print_Matrix();
  printf("\n\n");
	    
}


// Set the initial state and certainty estimates
void KalmanFilter::Add_InitialStateEstimate(mxn_Matrix &stateest, mxn_Matrix &certest) {

  StateEstimate_k = stateest;
  StateEstimate_k1 = stateest;
  
  StateEstCert_k = certest * certest.Transpose();
  StateEstCert_k1 = StateEstCert_k;
}

  
// Update the filter based on the time and possible control input
void KalmanFilter::Update(double dtime, mxn_Matrix &controlinput) {

  ControlState = controlinput;

  Update_State(dtime);
  printf("New State\n");
  StateEstimate_k1.Print_Matrix();
  printf("\n");

  Update_ErrorCovar();
  printf("New_Cert:\n");
  StateEstCert_k1.Print_Matrix();
  printf("\n\n");
}
  

// Update the state estimate base on time  
void KalmanFilter::Update_State(double dtime) {

  // store the old state estimate
  StateEstimate_k = StateEstimate_k1;


  // update the state model (process model)
  StateModel = StateModel_Pro->Update(dtime);

  // Now update appropriatly based on a possible control model
  if (ControlModel_Pro != NULL) {
    ControlModel = ControlModel_Pro->Update(dtime);
    StateEstimate_k1 = (StateModel * StateEstimate_k) + (ControlModel * ControlState);
  }
  else {
    StateEstimate_k1 = StateModel * StateEstimate_k;
  }
}


// updates the error covariance
void KalmanFilter::Update_ErrorCovar(void) {
  
  StateEstCert_k = StateEstCert_k1;
  StateEstCert_k.Print_Matrix();
  StateEstCert_k1 = ((StateModel * StateEstCert_k * StateModel.Transpose()) + (StateVariance * StateVariance.Transpose()));

}
