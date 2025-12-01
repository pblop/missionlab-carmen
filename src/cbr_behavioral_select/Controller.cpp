/**********************************************************************
 **                                                                  **
 **                         Controller.cpp                           **
 **                                                                  **
 ** Contains the implementation of the CBR Controller Class          **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003 - 2006 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Controller.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include "Headers.h"

#define MK_DEBUG

#ifdef MK_DEBUG
long dCt = 0;
FILE* fDebugMKFile;
#define DEBUGMKFILENAME "DecisionLog.debug"
#endif

#ifdef CREATE_DEBUG_FILES
FILE* fDebugFile;
FILE* fDebug2File;

FILE* fStuck = fopen("Stuck.dat", "a");
FILE* fMove = fopen("Move.dat", "a");
#endif

#ifdef CREATE_FEAT_DEBUG_FILE
FILE* fDebug1File;
#endif


//================constructors and desctructors====================
CCBRController::CCBRController () {
  nState = INIT_STATE;	//initial state
  pCurrentCase = NULL;
  pShortTermMemory = new CCBRShortTermMemory;
  nCaseNum = 0;

#ifdef MK_DEBUG
  fDebugMKFile = fopen(DEBUGMKFILENAME, "w");
  if(fDebugMKFile == NULL) {
    fprintf(stderr, "ERROR: could not open file %s\n", DEBUGMKFILENAME);
    exit(1);
  }
#endif

#ifdef CREATE_DEBUG_FILES
  fDebugFile = fopen(DEBUGFILENAME, "w");
  if(fDebugFile == NULL) {
    fprintf(stderr, "ERROR: could not open file %s\n", DEBUGFILENAME);
    exit(1);
  }
  fDebug2File = fopen(DEBUG2FILENAME, "w");
  if(fDebug2File == NULL) {
    fprintf(stderr, "ERROR: could not open file %s\n", DEBUG2FILENAME);
    exit(1);
  }
#endif
#ifdef CREATE_FEAT_DEBUG_FILE
  fDebug1File = fopen(DEBUG1FILENAME, "w");
  if(fDebug1File == NULL) {
    fprintf(stderr, "ERROR: could not open file %s\n", DEBUG1FILENAME);
    exit(1);
  }
#endif
  
}


CCBRController::~CCBRController() {

  if(pCurrentCase != NULL ) {
    delete pCurrentCase;
    pCurrentCase = NULL;
  }
  delete pShortTermMemory;
  pShortTermMemory = NULL;

}

//=================================================================


//=================functions and operations========================
//Initialization function - should be called only once per creation
//of the CBRController

bool CCBRController::Initialize() {
  int i = 0;
  long randseed = time(NULL);

  //the state is first new task state
  nState = FIRST_NEW_TASK_STATE;

  //set the output vector space
  SetOutputVectorSpace();
  SetOutputVectorUpperLimit();
  SetOutputVectorLowerLimit();


  //Set the weight vector
  if(SetWeightVector() == false)
    return false;

  //set the FC vector
  for(i = 0; i < NUMOFREGIONS; i++) {
    fCurrEnvFc[i] = 1.0;    //fully traversable
    fOldEnvFc[i] = 1.0;     //fully traversable
  }

  //reset the history structure

  CaseHistory.nTime = 0;
  CaseHistory.nCount = 0;

  //initialize random number generator
  srand((unsigned)randseed);


  return true;
}



//Identify features step in the CBR algorithm
//Note: all the readings are relative to the robot position
bool CCBRController::IdentifyFeatures(structRobotPos RobotPos, structObstaclePos ObstaclesPosArray[],
                                      int nNumofObstacles, PT GoalPos) {
  structFeature Feature;
  int nFeatureIndex = 0;

  //First reset the vector of current features
  CurrentFeaturesVector.clear();
  for(unsigned int i = 0; i < FEATURE_SIZE; i++)
    CurrentFeaturesIndices[i] = FEATURE_NOT_PRESENT;

  //first, compute the Distance to the goal feature
  Feature = ComputeGoalDistFeature(GoalPos);

  //add the feature to the vector of features
  CurrentFeaturesVector.push_back(Feature);
  CurrentFeaturesIndices[GOALDIST_FEATURE] = nFeatureIndex++;

  //compute the obstacle density and the first obstacle distance
  //for each angular region around the robot and these feature
  //to the vector of features
  ComputeObstacleDensityApprox(ObstaclesPosArray,
                               nNumofObstacles, GoalPos, CurrentFeaturesVector);
  CurrentFeaturesIndices[REG_FEATURE] = nFeatureIndex;
  nFeatureIndex += NUMOFREGIONS*2;

  //Compute Short-Term Relative Motion feature
  Feature = ComputeShortTermRelMotionFeature(RobotPos);

  //add the feature to the vector of features
  CurrentFeaturesVector.push_back(Feature);
  CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE] = nFeatureIndex++;

  //Compute Long-Term Relative Motion feature
  Feature = ComputeLongTermRelMotionFeature(RobotPos);

  //add the feature to the vector of features
  CurrentFeaturesVector.push_back(Feature);
  CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE] = nFeatureIndex++;

  //Compute Case Time feature
  Feature = ComputeCaseTimeFeature(&CaseHistory);

  //add the feature to the vector of features
  CurrentFeaturesVector.push_back(Feature);
  CurrentFeaturesIndices[CASETIME_FEATURE] = nFeatureIndex++;

  for(unsigned int i = 0; i < CurrentFeaturesVector.size(); i++) {
#ifdef VERBOSE
    printf("feature name = %s\n", CurrentFeaturesVector[i].strName);
    printf("feature value = %f\n", CurrentFeaturesVector[i].fValue);
#endif
#ifdef CREATE_FEAT_DEBUG_FILE
    fprintf(fDebug1File, "<%li>\n", dCt++);
    fprintf(fDebug1File, "feature name = %s\n", CurrentFeaturesVector[i].strName);
    fprintf(fDebug1File, "feature value = %f\n", CurrentFeaturesVector[i].fValue);
#endif
    
  }

  //based on the features compute the Fc vector
  if(ComputeFc(&CurrentFeaturesVector, CurrentFeaturesIndices,
               fCurrEnvFc) == false) {
    fprintf(stderr, "ERROR: Could not compute the environment FC\n");
    return false;
  }

  //run the Current Environment FC vector through the smoothing filter
  if(SmoothFC(fCurrEnvFc, fOldEnvFc) == false) {
    fprintf(stderr, "ERROR: Could not smooth out the EnvFC vector\n");
    return false;
  }

  //store the current FC for the next time
  copyFC(fCurrEnvFc, fOldEnvFc);

#ifdef CREATE_FEAT_DEBUG_FILE
  //print into the debug file
  for(unsigned int k = 0; k < NUMOFREGIONS; k++)
    fprintf(fDebug1File, "fCurrEnvFC[%d]=%f", k, fCurrEnvFc[k]);
  fprintf(fDebug1File, "\n");
#endif
  
  return true;
}



//Select a case step
bool CCBRController::SelectandAdaptCase() {
  CCBRCase* pNewCase = NULL;
  vector<CCBRCase*> PosSuccessCases, NegSuccessCases;
  bool bCreateNewCase;
  int nSimCasesNum;

  //make sure that old current case is NULL if no case was selected
  if(pCurrentCase != NULL && nState == CASE_NOT_FOUND_STATE) {
    fprintf(stderr, "ERROR: The old current case was not deleted\n");
    return false;
  }

  //if there are no high level features are identified
  //then use the default case
  if(CurrentFeaturesVector.size() == 0) {
    fprintf(stderr, "ERROR: No features specified\n");
    return false;
  }

  //Select the best matching case
  double fSimDegree=0;

  //Get the best case
  if((pNewCase = GetBestMatchCase(fSimDegree)) == NULL) {
    fprintf(stderr, "ERROR: Could not find the best match\n");
    return false;
  }

  //limit the fSimDegree
  fSimDegree = __min(1.0, fSimDegree);
  fSimDegree = __max(0.0, fSimDegree);

  //Go through the decision tree and decide whether
  //to swap the current case with the new one if there
  //was an old case at all
  bool bApplyCase;
  if(nState == CASE_FOUND_STATE) {
    if(NeedSwapCase(pNewCase,fSimDegree) == true) {
#ifdef CREATE_DEBUG_FILES
      fprintf(fDebug2File, "*********************\n");
#endif

      //print debug info
      PrintOldCaseDebug();

      //Update the last learning structure
      UpdateCurrentLearning();

      //Do the learning on the old case
      PropogateLearning();

      //change the current case onto the new one
      delete pCurrentCase;
      pCurrentCase = NULL;

      //set the flag
      bApplyCase = true;
    } else
      bApplyCase = false;
  } else
    bApplyCase = true;

  //if a new
  if(bApplyCase) {
#ifdef CREATE_DEBUG_FILES
    fprintf(fDebug2File, "Current Environment:");
    for(int k = 0; k < (unsigned int)NUMOFREGIONS; k++)
      fprintf(fDebug2File, "FC[%d]=%f ", k, fCurrEnvFc[k]);
    fprintf(fDebug2File, "\nNew case to be applied:\n");
#endif
    PrintCase(pNewCase);

    pCurrentCase = CreateCaseofType(pNewCase->GetCaseType());
    *pCurrentCase = *pNewCase;

    //Reset some of the variables for the controller for new case
    ResetwithNewCase();

    //Get similar to environment cases with very positive success and very negative success
    //and the number of all similar cases
    if(!GetPosandNegSuccessCases(PosSuccessCases, NegSuccessCases, nSimCasesNum))
      return false;

    //if this is to be a new case in the library then add it
    bCreateNewCase = NeedCreateCase(pCurrentCase, nSimCasesNum);
    fflush(fDebugMKFile); //mk
    
    if(bCreateNewCase) {
      //assign a new ID to the current Case
      pCurrentCase->SetCaseID(pShortTermMemory->GetFreeCaseID());

      //change the name for the case
      CreateNewName(pCurrentCase);

      //change the input feature vector onto environment
      pCurrentCase->FeaturesVector = CurrentFeaturesVector;

      //reset some of the variables of the new derived case
      ResetDerivedCase(pCurrentCase);

      //add current case to the library
      AddCurrentCase();

    }

    //Add New Learning Structure and apply knowledge from the old learning structure
    AddNewApplyRemoveOldLearn();

    //Adapt the Current Case
    if(AdaptCurrentCase(bCreateNewCase, PosSuccessCases, NegSuccessCases) == false)
      return false;

  }

  //the case is found now
  nState = CASE_FOUND_STATE;

  return true;
}


//Apply a case step
bool CCBRController::ApplyCurrentCase() {
  if(nState == CASE_NOT_FOUND_STATE || pCurrentCase == NULL)
    return false;

#ifdef VERBOSE
  printf("Applied Case Name: %s\n", pCurrentCase->GetCaseName());
#endif
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "<%li>\n", dCt++);
  fprintf(fDebug1File, "Applied Case Name: %s\n", pCurrentCase->GetCaseName());
#endif

  if(pCurrentCase->Apply() == false)
    return false;

  return true;
}


//Returns true if it the Controller started a new task
bool CCBRController::IsNewTask() {
  //for now we do not take care of the transition to the second task.
  return (nState == NEW_TASK_STATE || nState == FIRST_NEW_TASK_STATE);
}


//Resets the state for the new task.
//Also does the learning on the previous case or queue it up for the later.
//return true if the operations are successful
bool CCBRController::NewTaskReset() {
  if(nState == NEW_TASK_STATE) {

    //reset the learning vector
    LearningV.clear();

    //Save the short term memory from the previous task - TODO

    //delete the current case if there is one
    if(pCurrentCase != NULL) {
      delete pCurrentCase;
      pCurrentCase = NULL;
    }
  }

  //===========Load a ShortTerm Memory=======================
  //open the case file
  if((fCaseFile = fopen(CASEFILENAME, "r")) == NULL) {
    fprintf(stderr, "ERROR: Could not open the case file - %s\n", CASEFILENAME);
    return false;
  }

  if(pShortTermMemory->Load(fCaseFile) == false)
    return false;

  //set the Number of cases - for now it is equal to the number of cases in the
  //short term memeory
  nCaseNum = pShortTermMemory->GetNumCases();

  //close the case file
  fclose(fCaseFile);
  //===========================================================

  //change state
  nState = CASE_NOT_FOUND_STATE;

  return true;
}


//Save the CBR library
void CCBRController::SaveLibrary() {
  //check that the library is modifiable
  if(!pShortTermMemory->IsModifiable()) {
    fprintf(stderr, "WARNING: Library is not modifiable - cases are not saved\n");
    return;
  }

  //open the case file
  if((fCaseFile = fopen(CASEFILENAME, "w")) == NULL) {
    fprintf(stderr, "ERROR: Could not open the case file - %s\n", CASEFILENAME);
    return;
  }

  //Save the short term memory
  pShortTermMemory->Save(fCaseFile);

  //close the case file
  fclose(fCaseFile);

}


CCBRCase* CCBRController::CreateDefaultCase() {
  //for now it is a Move case
  CCBRCase* pCase = CreateCaseofType(MOVETO_CASE_TYPE);

  //copy the current features
  pCase->FeaturesVector = CurrentFeaturesVector;
  //copy the current feature indices
  memcpy(pCase->FeaturesIndices, CurrentFeaturesIndices,
         sizeof(CurrentFeaturesIndices));
  //copy the full output vector space with their default values
  pCase->OutputVector = FullOutputVector;
  //set the id of the case
  pCase->SetCaseID(nCaseNum+1);
  //Set the name of the case
  pCase->SetCaseName("DEFAULT*1");
  //Set the case success for now to unknown = neutral
  pCase->SetCaseSuccess(UNKNOWN_CASE_SUCCESS);
  //set the case type
  pCase->SetCaseType(MOVETO_CASE_TYPE);

  return pCase;

}

void CCBRController::SetCasetoRandDelta(CCBRCase* pCase, double fDeltaMag) {
  double fRandNum;
  unsigned int i;

  //copy the current features
  pCase->FeaturesVector = CurrentFeaturesVector;
  //copy the current feature indices
  memcpy(pCase->FeaturesIndices, CurrentFeaturesIndices,
         sizeof(CurrentFeaturesIndices));
  //set the id of the case
  pCase->SetCaseID(nCaseNum+1);
  //Set the name of the case
  pCase->SetCaseName("DEFAULT");
  //Set the case success for now to unknown = neutral
  pCase->SetCaseSuccess(UNKNOWN_CASE_SUCCESS);
  //set the case type
  pCase->SetCaseType(MOVETO_CASE_TYPE);

  //this is the only important part here
  pCase->OutputVector = FullOutputVector;

  for(i = 0; i < pCase->OutputVector.size(); i++) {
    //adapt only the allowed dimensions.
    if(pCase->OutputVector[i].bAdapt) {
      fRandNum = (((double)rand())/RAND_MAX - 0.5)* fDeltaMag;
      if(pCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
        pCase->OutputVector[i].Value.fValue = fRandNum;
      else if(pCase->OutputVector[i].nValueType == INT_VALUE_TYPE) {
        pCase->OutputVector[i].Value.nValue = (fRandNum > 0)?-1:1;
      }
    } else {
      if(pCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
        pCase->OutputVector[i].Value.fValue = 0;
      else if(pCase->OutputVector[i].nValueType == INT_VALUE_TYPE)
        pCase->OutputVector[i].Value.nValue = 0;
    }
  }
}


void CCBRController::SetOutputVectorSpace() {
  CCBROutElement OutElement;

  //create the first action
  strcpy(OutElement.strStringId, MOVETOGOAL_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = MOVETOGOAL_GAIN_DEFAULT;
  OutElement.bAdapt = MOVETOGOAL_GAIN_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the second action
  strcpy(OutElement.strStringId, NOISE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = NOISE_GAIN_DEFAULT;
  OutElement.bAdapt = NOISE_GAIN_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the third action
  strcpy(OutElement.strStringId, NOISE_PERSISTANCE);
  OutElement.nValueType = INT_VALUE_TYPE;
  OutElement.Value.nValue = NOISE_PERSISTANCE_DEFAULT;
  OutElement.bAdapt = NOISE_PERSISTANCE_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the fourth action
  strcpy(OutElement.strStringId, OBSTACLE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_GAIN_DEFAULT;
  OutElement.bAdapt = OBSTACLE_GAIN_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, OBSTACLE_SPHERE);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_SPHERE_DEFAULT;
  OutElement.bAdapt = OBSTACLE_SPHERE_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the sixth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_X);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_X_DEFAULT;
  OutElement.bAdapt = BIAS_VECTOR_X_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_Y);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_Y_DEFAULT;
  OutElement.bAdapt = BIAS_VECTOR_Y_ADAPT;
  FullOutputVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_GAIN_DEFAULT;
  OutElement.bAdapt = BIAS_VECTOR_GAIN_ADAPT;
  FullOutputVector.push_back(OutElement);
}

void CCBRController::SetOutputVectorUpperLimit() {
  CCBROutElement OutElement;
  //create the first action
  strcpy(OutElement.strStringId, MOVETOGOAL_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = MOVETOGOAL_GAIN_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the second action
  strcpy(OutElement.strStringId, NOISE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = NOISE_GAIN_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the third action
  strcpy(OutElement.strStringId, NOISE_PERSISTANCE);
  OutElement.nValueType = INT_VALUE_TYPE;
  OutElement.Value.nValue = NOISE_PERSISTANCE_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the fourth action
  strcpy(OutElement.strStringId, OBSTACLE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_GAIN_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, OBSTACLE_SPHERE);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_SPHERE_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the sixth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_X);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_X_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_Y);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_Y_UPPER;
  OutputUpperLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_GAIN_UPPER;
  OutputUpperLimitVector.push_back(OutElement);
}

void CCBRController::SetOutputVectorLowerLimit() {
  CCBROutElement OutElement;

  //create the first action
  strcpy(OutElement.strStringId, MOVETOGOAL_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = MOVETOGOAL_GAIN_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the second action
  strcpy(OutElement.strStringId, NOISE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = NOISE_GAIN_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the third action
  strcpy(OutElement.strStringId, NOISE_PERSISTANCE);
  OutElement.nValueType = INT_VALUE_TYPE;
  OutElement.Value.nValue = NOISE_PERSISTANCE_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the fourth action
  strcpy(OutElement.strStringId, OBSTACLE_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_GAIN_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, OBSTACLE_SPHERE);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = OBSTACLE_SPHERE_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the sixth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_X);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_X_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_Y);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_Y_LOWER;
  OutputLowerLimitVector.push_back(OutElement);

  //create the fifth action
  strcpy(OutElement.strStringId, BIAS_VECTOR_GAIN);
  OutElement.nValueType = FLOAT_VALUE_TYPE;
  OutElement.Value.fValue = BIAS_VECTOR_GAIN_LOWER;
  OutputLowerLimitVector.push_back(OutElement);
}

void CCBRController::LimitOutputV(CCBRCase* pCase) {
  unsigned int i;

  for(i = 0; i < pCase->OutputVector.size(); i++) {
    if(pCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE) {
      pCase->OutputVector[i].Value.fValue =
        __min(OutputUpperLimitVector[i].Value.fValue,
              pCase->OutputVector[i].Value.fValue);
      pCase->OutputVector[i].Value.fValue =
        __max(OutputLowerLimitVector[i].Value.fValue,
              pCase->OutputVector[i].Value.fValue);
    } else if(pCase->OutputVector[i].nValueType == INT_VALUE_TYPE) {
      pCase->OutputVector[i].Value.nValue =
        __min(OutputUpperLimitVector[i].Value.nValue,
              pCase->OutputVector[i].Value.nValue);
      pCase->OutputVector[i].Value.nValue =
        __max(OutputLowerLimitVector[i].Value.nValue,
              pCase->OutputVector[i].Value.nValue);

    }

  }
}


//adds the current case to the library
bool CCBRController::AddCurrentCase() {
  //saves the currently running case into the short term memory
  if(pCurrentCase != NULL) {
    if(!pShortTermMemory->AddCase(pCurrentCase))
      return false;
    nCaseNum++;
  } else
    return false;

  PrintNewCaseCreated();

  return true;
}


//return true if the current case should be changed
//onto the new one, otherwise - false
//Case swapping decision tree
bool CCBRController::NeedSwapCase(CCBRCase* pNewCase, double fSimDegree) {
  int nTemp;
  double fCurrCaseFc[NUMOFREGIONS];
  double fNewCaseFc[NUMOFREGIONS];
  double fCurrCaseSimDegree, fNewCaseSimDegree;

  //first, make sure that the suggested case is not the same as current
  //if(pNewCase->GetCaseID() == pCurrentCase->GetCaseID())
  //	return false;	//leave old case

  //----------------------------extract features------------------
  nTemp = CurrentFeaturesIndices[CASETIME_FEATURE];
  double fCaseTime =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  //---------------------------------------------------------------



  //----------compute the similarity of the current case------------
  //Compute Fc for current case
  if(ComputeFc(&pCurrentCase->FeaturesVector,
               pCurrentCase->FeaturesIndices,
               fCurrCaseFc) == false) {
    fprintf(stderr, "Could not compute the Current Case FC\n");
    return false;
  }
  //print into the debug file
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "<%li>\n", dCt++);
  fprintf(fDebug1File, "FC for the currently applied case:\n");
  for(int k = 0; k < NUMOFREGIONS; k++)
    fprintf(fDebug1File, "fCurrCaseFC[%d]=%f", k, fCurrCaseFc[k]);
  fprintf(fDebug1File, "\n");
#endif

  //now based on the Fc of environment and
  //the Fc of the current case compute the degree of
  //similarity of the current case
  fCurrCaseSimDegree = ComputeSimDegree(fCurrEnvFc, fCurrCaseFc);

#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "Env SimDegree with the current case = %f\n",
          fCurrCaseSimDegree);
#endif
  //----------------------------------------------------------------

  //----------compute the similarity of the new case------------
  //Compute Fc for new case
  if(ComputeFc(&pNewCase->FeaturesVector,
               pNewCase->FeaturesIndices,
               fNewCaseFc) == false) {
    fprintf(stderr, "Could not compute the New Case FC\n");
    return false;
  }
  //now based on the Fc of environment and
  //the Fc of the New case compute the degree of
  //similarity of the New case
  fNewCaseSimDegree = ComputeSimDegree(fCurrEnvFc, fNewCaseFc);
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "Env SimDegree with the new case = %f\n",
          fNewCaseSimDegree);
#endif
  //----------------------------------------------------------------


  //-------------decision tree-----------------------------------
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "The Time current case was applied=%f\n",
          fCaseTime);
  fprintf(fDebug1File, "LontTermMotion=%f, ShortTermMotion=%f\n",
          fLongTermRelMt, fShortTermRelMt);
#endif
  
  //check case time application to the maximum allowed case time
  if(fCaseTime > MAXAPPLYTIME)
    return true;

  //check the time case was applied
  if(fCaseTime <= nTimeThresh)
    //check the similarity of the case with environment
    if(fCurrCaseSimDegree > CASESIMLOW_THRESH &&
        (fNewCaseSimDegree-fCurrCaseSimDegree) < CASESIMDIFF_THRESH)
      return false;	//same case

  //check the long term relative motion
  if(fLongTermRelMt > LONGTERMMOT_THRESH) {
    //check short term movement against the low threshold
    if(fShortTermRelMt < SHORTTERMMOT_LOW_THRESH)
      return true; //new new case

    //check the similarity of the case with environment
    if(fCurrCaseSimDegree > CASESIMLOW_THRESH &&
        (fNewCaseSimDegree-fCurrCaseSimDegree) < CASESIMDIFF_THRESH)
      return false;	//same case
    else
      return true;	//new case
  }
  //no long term movement

  else {
    //check short term movement
    if(fShortTermRelMt > SHORTTERMMOT_THRESH) {
      //no long term, but some short term movement
      //check the case similarity with the environment
      if(fCurrCaseSimDegree > CASESIMHIGH_THRESH)
        return false;	//same case
      else
        return true;	//new case
    }
    else {
      //no long and no short term movement
      return true;	//new case
    }
  }

}



bool CCBRController::ComputeFc(vector<structFeature>* pFeaturesVector,
                               int FeaturesIndices[], double fFc[]) {
  int nTemp;

  //--------Get the distance to the goal---------------
  //Its index
  nTemp = FeaturesIndices[GOALDIST_FEATURE];
  //make sure that it is valid
  if(nTemp == FEATURE_NOT_PRESENT) {
    fprintf(stderr, "ERROR: No Goal Distance Feature is found\n");
    return false;
  }
  double fD = (*pFeaturesVector)[nTemp].fValue;
  //limit it by the threshold
  fD = __min(fD, D_TRESH);
  fD = __max(fD, D_MIN_THRESH);
  //---------------------------------------------------

  //--------Get the start of the Region features-------
  int nRegFeatStart = FeaturesIndices[REG_FEATURE];
  //make sure it is valid
  if(nRegFeatStart == FEATURE_NOT_PRESENT) {
    fprintf(stderr, "ERROR: No Region Features are found\n");
    return false;
  }
  //--------------------------------------------------

  //find Fc for each region
  double fd, fCurrFc, fR, fG;
  //---Process First Region-----------------------------
  //Compute d
  fd = fD;
  //Get R(distance) of the region
  fR = (*pFeaturesVector)[nRegFeatStart+1].fValue;
  //Get density of the region
  fG = (*pFeaturesVector)[nRegFeatStart].fValue;
  //Compute Fc for the region
  if(fR > fd)
    fCurrFc = 1;
  else
    fCurrFc = 1 - (fd - fR)/fD*fG;
  //save the Fc
  fFc[0] = fCurrFc;
  //----------------------------------------------------


  //-----Process now all the rest of the regions--------
  //fR for left and right regions
  double fRleft = fR;
  double fRright = fR;
  //angle between the regions
  //TODO - change for the non-uniform distribution
  double fInvCosQ = cos(2*PI/NUMOFREGIONS);
  if(!IS_ZERO(fInvCosQ))
    fInvCosQ = 1/fInvCosQ;
  else
    fInvCosQ = 100000.00;

  int nRegProcNum = 0;
  int nLeftRegIndex = 1;
  int nRightRegIndex = NUMOFREGIONS-1;
  int nRegNumtoProcess = 0;
  //process all the regions except for the last one
  //if it lies half on positive, half on negative  side
  //from the Y axis
  //They are processed in couples - two regions that
  //are mirrors of each other relative to the X axis
  //The number of regions to process:
  if(NUMOFREGIONS%2 == 0)
    nRegNumtoProcess = NUMOFREGIONS-2;
  else
    nRegNumtoProcess = NUMOFREGIONS-1;
  while(nRegProcNum < nRegNumtoProcess) {
    //first process the left(if going in Increasing X axis) region
    //Compute d = min(D, R(of previous region)/Cos(Q))
    fd = __min(fD, (fRleft+D_MIN_THRESH)*fInvCosQ);
    //Get R(distance) of the region
    fR = (*pFeaturesVector)[nRegFeatStart+nLeftRegIndex*2+1].fValue;
    //Get density of the region
    fG = (*pFeaturesVector)[nRegFeatStart+nLeftRegIndex*2].fValue;
    //Compute Fc for the region
    if(fR > fd)
      fCurrFc = 1;
    else
      fCurrFc = 1 - (fd - fR)/fD*fG;
    //save the Fc
    fFc[nLeftRegIndex] = fCurrFc;
    //save the R for the next left region
    fRleft = fR;
    //increment the left region index
    nLeftRegIndex++;

    //now process the right region
    //Compute d = min(D, R(of previous region)/Cos(Q))
    fd = __min(fD, (fRright+D_MIN_THRESH)*fInvCosQ);
    //Get R(distance) of the region
    fR = (*pFeaturesVector)[nRegFeatStart+nRightRegIndex*2+1].fValue;
    //Get density of the region
    fG = (*pFeaturesVector)[nRegFeatStart+nRightRegIndex*2].fValue;
    //Compute Fc for the region
    if(fR > fd)
      fCurrFc = 1;
    else
      fCurrFc = 1 - (fd - fR)/fD*fG;
    //save the Fc
    fFc[nRightRegIndex] = fCurrFc;
    //save the R for the next rigth region
    fRright = fR;
    //increment the right region index
    nRightRegIndex--;

    //increment the total counter
    nRegProcNum += 2;
  }

  //Process the last region if there is one
  //mirror of the first region relative to Y axis
  if(nRegProcNum < NUMOFREGIONS-1) {
    //Compute d = min(D, R(of previous region)/Cos(Q))
    fd = __min(fD, (fRleft+D_MIN_THRESH)*fInvCosQ);
    //Get R(distance) of the region
    fR = (*pFeaturesVector)[nRegFeatStart+nLeftRegIndex*2+1].fValue;
    //Get density of the region
    fG = (*pFeaturesVector)[nRegFeatStart+nLeftRegIndex*2].fValue;
    //Compute Fc for the region
    if(fR > fd)
      fCurrFc = 1;
    else
      fCurrFc = 1 - (fd - fR)/fD*fG;
    //save the Fc
    fFc[nLeftRegIndex] = fCurrFc;
  }

  return true;
}


//Returns the degree of similarity between two Fc vectors
//it is actually weighted distance:
//S = 1 - Sum(Wi*(Fc1i - Fc2i)^2)/Sum(Wi)
double CCBRController::ComputeSimDegree(double fFc1[], double fFc2[]) {
  double fSimDegree = 0.0;
  double fWSum = 0.0;

  //Compute S = Sum(Wi*(Fc1i - Fc2i)^2);
  for(int i = 0; i < NUMOFREGIONS; i++) {
    fSimDegree = fSimDegree + fWeightV[i]*
                 pow((fFc1[i]-fFc2[i]),2.0);
    fWSum = fWSum + fWeightV[i];
  }
  //S = Sum(Wi*(Fc1i - Fc2i)^2)/Sum(Wi)
  fSimDegree = fSimDegree/fWSum;
  //S = 1 - Sum(Wi*(Fc1i - Fc2i)^2)/Sum(Wi)
  fSimDegree = 1 - fSimDegree;

  return fSimDegree;
}


//Sets up the weight vector
bool CCBRController::SetWeightVector() {
  int nRightIndex = 0;
  int nLeftIndex = 0;
  double fWRightSum = 0, fWLeftSum = 0;
  int i = 0;


  //set all the region weights looking backward
  //set the last region weight if even number of regions
  if(NUMOFREGIONS%2 == 0) {
    fWeightV[NUMOFREGIONS/2] = WEIGHT_BACKWARDREGION;
    nRightIndex = NUMOFREGIONS/2+1;
    nLeftIndex = NUMOFREGIONS/2-1;
    fWRightSum += WEIGHT_BACKWARDREGION;
    fWLeftSum+= WEIGHT_BACKWARDREGION;
  } else {
    nRightIndex = NUMOFREGIONS/2+1;
    nLeftIndex = NUMOFREGIONS/2;
  }

  //Now RightIndex points toward the region looking backward
  //and located to the right of X axis going positive direction
  //Similar is the left index

  //set all the rest of the backward regions
  for(i = 0; i < (NUMOFREGIONS-nRightIndex)/2; i++) {
    fWeightV[nRightIndex+i] = WEIGHT_BACKWARDREGION;
    fWRightSum += WEIGHT_BACKWARDREGION;
  }
  nRightIndex += i;
  for(i = 0; i < nLeftIndex/2; i++) {
    fWeightV[nLeftIndex-i] = WEIGHT_BACKWARDREGION;
    fWLeftSum+= WEIGHT_BACKWARDREGION;
  }
  nLeftIndex -= i;

  //Set all the forward regions on the right
  for(i = 0; i < NUMOFREGIONS-nRightIndex; i++) {
    if(fWRightSum == 0) {
      fprintf(stderr, "ERROR: Could not set region weight vector\n");
      return false;
    }
    double fTemp = fWRightSum/WEIGHTDELTA_THRESH;
    fWeightV[nRightIndex+i] = fTemp;
    fWRightSum = fWRightSum + fTemp;
  }
  //Set all the forward regions on the left
  for(i = 0; i < nLeftIndex; i++) {
    if(fWLeftSum == 0) {
      fprintf(stderr, "ERROR: Could not set region weight vector\n");
      return false;
    }
    double fTemp = fWLeftSum/WEIGHTDELTA_THRESH;
    fWeightV[nLeftIndex-i] = fTemp;
    fWLeftSum = fWLeftSum + fTemp;
  }
  //set the very first region
  fWeightV[0] = (fWLeftSum + fWRightSum)/WEIGHTDELTA_THRESH;
  //take care of one region case
  if(fWeightV[0] == 0)
    fWeightV[0] = 1.0;

  return true;

}


bool CCBRController::ResetwithNewCase() {
  //reset the history
  CaseHistory.nTime = 0;
  CaseHistory.nCount = 0;

  //Case Time Threshold
  nTimeThresh = CASETIME_THRESH;

  return  true;
}



//Print some info about the case that was just changed
void CCBRController::PrintOldCaseDebug() {

#ifdef CREATE_DEBUG_FILES
  unsigned int i, k;

  fprintf(fDebugFile, "-------------Change of Case---------------\n");
  int nTemp = CurrentFeaturesIndices[CASETIME_FEATURE];
  double fCaseTime =  CurrentFeaturesVector[nTemp].fValue;
  fprintf(fDebugFile, "Case Name: %s (ID = %d) \n", pCurrentCase->GetCaseName(),
          pCurrentCase->GetCaseID());
  fprintf(fDebugFile, "Case was applied for %f time units\n", fCaseTime);
  nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  fprintf(fDebugFile, "Current Short-Term Motion; %f\n", fShortTermRelMt);
  nTemp = CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  fprintf(fDebugFile, "Current Long-Term Motion; %f\n", fLongTermRelMt);
  fprintf(fDebugFile, "Case had Time Thresh: %d\n", nTimeThresh);
  fprintf(fDebugFile, "Case Success: %f\n", pCurrentCase->GetCaseSuccess());
  fprintf(fDebugFile, "Case Improvement: %f\n", pCurrentCase->fImprov);
  fprintf(fDebugFile, "Heur Threshold: %f\n", pCurrentCase->fHeurThresh);
  for(k = 0; k < (unsigned int)NUMOFREGIONS; k++)
    fprintf(fDebugFile, "fCurrEnvFC[%d]=%f", k, fCurrEnvFc[k]);
  fprintf(fDebugFile, "\n");

  for(i = 0; i < pCurrentCase->OutputVector.size(); i++) {
    fprintf(fDebugFile, "%s = ", pCurrentCase->OutputVector[i].strStringId);
    switch(pCurrentCase->OutputVector[i].nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fDebugFile, "%f\n", pCurrentCase->OutputVector[i].Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fDebugFile, "%d\n", pCurrentCase->OutputVector[i].Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: wrong format for Output element\n");
      return;
    };
  }

  fprintf(fDebugFile, "Adaptation Vector used:\n");
  for(i = 0; i < pCurrentCase->AdaptV.size(); i++) {
    fprintf(fDebugFile, "%s = ", pCurrentCase->AdaptV[i].strStringId);
    switch(pCurrentCase->OutputVector[i].nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fDebugFile, "%f\n", pCurrentCase->AdaptV[i].Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fDebugFile, "%d\n", pCurrentCase->AdaptV[i].Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: wrong format for Output element\n");
      return;
    };
  }



  //save the file
  fflush(fDebugFile);


  //save features for Matlab FIsher linear discriminant
  if(fLongTermRelMt > 0.95 && fShortTermRelMt > 0.5) {
    for(k = 0; k < (unsigned int)NUMOFREGIONS; k++)
      fprintf(fMove, "%f ", fCurrEnvFc[k]);
    fprintf(fMove, "%f\n", fShortTermRelMt);
    fflush(fMove);
  } else if(fLongTermRelMt < 0.5 || fShortTermRelMt < 0.1) {
    for(k = 0; k < (unsigned int)NUMOFREGIONS; k++)
      fprintf(fStuck, "%f ", fCurrEnvFc[k]);
    fprintf(fStuck, "%f\n", fShortTermRelMt);
    fflush(fStuck);
  }
#endif

}



double CCBRController::ComputeTimeSimwithCurrEnv(CCBRCase* pCase) {
  double fSim = 0;

  //----------------------------extract features------------------
  int nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMtEnv =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMtEnv =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = pCase->FeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  pCase->FeaturesVector[nTemp].fValue;
  nTemp = pCase->FeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  pCase->FeaturesVector[nTemp].fValue;
  //---------------------------------------------------------------

  fSim = SHORTTERMRELMT_SIM_WEIGHT*pow(fShortTermRelMtEnv-fShortTermRelMt,2.0) +
         LONGTERMRELMT_SIM_WEIGHT*pow(fLongTermRelMtEnv-fLongTermRelMt,2.0);

  //normalize the similarity
  fSim = 1.0 - fSim/TIMESIM_WEIGHTSUM;

#ifdef VERBOSE
  printf("LongTermEnv=%f, ShortTermEnv=%f, LongTerm=%f, ShortTerm=%f, fSim=%f\n",
         fLongTermRelMtEnv, fShortTermRelMtEnv, fLongTermRelMt, fShortTermRelMt, fSim);
#endif
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "<%li>\n", dCt++);
  fprintf(fDebug1File, "LongTermEnv=%f, ShortTermEnv=%f, LongTerm=%f, ShortTerm=%f, fSim=%f\n",
          fLongTermRelMtEnv, fShortTermRelMtEnv, fLongTermRelMt, fShortTermRelMt, fSim);
#endif
  
  return fSim;

}

//Asymmetric smoothing of the FC vector
bool CCBRController::SmoothFC(double fNewFC[], double fOldFC[]) {
  int i = 0;
  double fD;

  //iterate through all the elements of the vector
  for(i = 0; i < NUMOFREGIONS; i++) {
    //depending on whether it attacks or decays set the smoothing coefficient
    if(fNewFC[i] > fOldFC[i])
      fD = FC_SMOOTH_ATTACKCOEFF;  //traversability increases
    else
      fD = FC_SMOOTH_DECAYCOEFF;

    //filtering itself
    fNewFC[i] = (1.0-fD)*fNewFC[i] + fD*fOldFC[i];
  }


  return true;
}


//Return the most similar case to the current environment
CCBRCase* CCBRController::GetBestMatchCase(double&  fSimDegree) {
  vector<CCBRCase*> MatchingCasePtrs;
  CCBRCase* pRetCase;

  //if the library is empty, then the best match is a default case
  if(pShortTermMemory->GetNumCases() == 0) {
    //Using the default case
#ifdef VERBOSE
    printf("Creating a default case\n");
#endif
    pRetCase = CreateDefaultCase();
    pShortTermMemory->AddCase(pRetCase);
    nCaseNum++;
    delete pRetCase;
  }

  //Spatial Selection Stage
  if(!SpatialSelection(MatchingCasePtrs)) {
    fprintf(stderr, "ERROR in spatial selection stage\n");
    return NULL;
  }

  //Temporal Selection Stage
  if(!TemporalSelection(MatchingCasePtrs)) {
    fprintf(stderr, "ERROR in temporal selection stage\n");
    return NULL;
  }

  //Third - random selection of the best match
  if((pRetCase = BestCaseSelection(MatchingCasePtrs, fSimDegree)) == NULL) {
    fprintf(stderr, "ERROR at third selection stage\n");
    return NULL;
  }

#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "<%li>\n", dCt++);
  fprintf(fDebug1File, "The Best Match Case: %s\n", pRetCase->GetCaseName());
#endif
  
  //return the case
  return pRetCase;

}

//spatial selection of cases
bool CCBRController::SpatialSelection(vector<CCBRCase*> &MatchingCasePtrs) {
  double fTemp;
  double fCurrFc[NUMOFREGIONS];
  int i = 0;


  //Iterate through all the cases in the library and find maximum SimDegree
  double fMaxSimDegree = 0; // no match at all
  for(i = 0; i < pShortTermMemory->GetNumCases(); i++) {


#if GOAL_TRAVERSE_CHECK && 0 //TODO - remove at all
    //first two are dedicated to goal traversable situation whereas
    //the last two are dedicated to goal intraversable - TODO
    if(IsGoalTraversable() == true && i > 1)
      break;
    else if (IsGoalTraversable() == false && i <= 1)
      continue;
#endif

    //get a new case from the library
    CCBRCase* pCase = pShortTermMemory->GetCase(i);



    //Compute Fc for it
    if(ComputeFc(&pCase->FeaturesVector,
                 pCase->FeaturesIndices,
                 fCurrFc) == false) {
      fprintf(stderr, "Could not compute the library case FC\n");
      return false;
    }

    //now based on the Fc of environment and
    //the Fc of the case compute the degree of similarity
    fTemp = ComputeSimDegree(fCurrEnvFc, fCurrFc);

    //keep the maximum
    if(fTemp > fMaxSimDegree)
      fMaxSimDegree = fTemp;
  }

  //iterate again and select all the cases that are within some
  //probabilistic interval from MaxSimDegree
  for(i = 0; i < pShortTermMemory->GetNumCases(); i++) {
    //get a new case from the library
    CCBRCase* pCase = pShortTermMemory->GetCase(i);

#if GOAL_TRAVERSE_CHECK && 0 //TODO - remove at all
    //first two are dedicated to goal traversable situation whereas
    //the last two are dedicated to goal intraversable - TODO
    if(IsGoalTraversable() == true && i > 1)
      break;
    else if (IsGoalTraversable() == false && i <= 1)
      continue;
#endif

    //Compute Fc for it
    if(ComputeFc(&pCase->FeaturesVector,
                 pCase->FeaturesIndices,
                 fCurrFc) == false) {
      fprintf(stderr, "Could not compute the library case FC\n");
      return false;
    }

    //now based on the Fc of environment and
    //the Fc of the case compute the degree of similarity
    fTemp = ComputeSimDegree(fCurrEnvFc, fCurrFc);

#ifdef CREATE_FEAT_DEBUG_FILE
    fprintf(fDebug1File, "<%li>\n", dCt++);
    fprintf(fDebug1File, "case: %s\n", pCase->GetCaseName());
    for(int k = 0; k < NUMOFREGIONS; k++)
      fprintf(fDebug1File, "FC[%d]=%f ", k, fCurrFc[k]);
    fprintf(fDebug1File, "SimDegree with env %f\n", fTemp);
    fprintf(fDebug1File, "\n");
#endif
    
    //see if the case can be selected
    fTemp = fMaxSimDegree - fTemp;	//the difference between best case and this one

    //get gaussian distributed random number
    float fThresh = fabs((float) (gaussianRandom()) );
    //multiply by our probabilistic limit to set a corresponding variance
    fThresh = SPATIALSIM_2STD/2.0*fThresh;
    //does the case go through
    if(fTemp <= fThresh) {
      MatchingCasePtrs.push_back(pCase);
#ifdef CREATE_FEAT_DEBUG_FILE
      fprintf(fDebug1File, "case %s selected into spatially matching set\n",
              pCase->GetCaseName());
#endif
    }
  }

  //make sure something was selected
  if(MatchingCasePtrs.size() == 0)
    return false;

  return true;

}

//Temporal selection of cases
bool CCBRController::TemporalSelection(vector<CCBRCase*> &MatchingCasePtrs) {
  vector<CCBRCase*> NextMatchCasePtrs;
  double fMaxSimDegree = 0;
  unsigned int i;

  //compute the best temporal similarity
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    //get a new case from the library
    CCBRCase* pCase = MatchingCasePtrs[i];

    //compute temporal similarity
    double fTemp = ComputeTimeSimwithCurrEnv(pCase);

    //keep the maximum
    if(fTemp > fMaxSimDegree)
      fMaxSimDegree = fTemp;
  }

  //iterate again and select all the cases that are within some
  //probabilistic interval from MaxSimDegree
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    //get a new case from the library
    CCBRCase* pCase = MatchingCasePtrs[i];

    //compute temporal similarity
    double fTemp = ComputeTimeSimwithCurrEnv(pCase);

#ifdef CREATE_FEAT_DEBUG_FILE
    fprintf(fDebug1File, "<%li>\n", dCt++);
    fprintf(fDebug1File, "case: %s, temporal simdegree = %f\n", pCase->GetCaseName(), fTemp);
#endif
    
    //see if the case can be selected
    fTemp = fMaxSimDegree - fTemp;	//the difference between best case and this one

    //get gaussian distributed random number
    float fThresh = fabs( ((float) gaussianRandom()) );
    //multiply by our probabilistic limit to set a corresponding variance
    fThresh = TEMPORALSIM_2STD/2.0*fThresh;
    //does the case go through
    if(fTemp <= fThresh) {
      NextMatchCasePtrs.push_back(pCase);
#ifdef CREATE_FEAT_DEBUG_FILE
      fprintf(fDebug1File, "case %s selected into temporally matching set\n",
              pCase->GetCaseName());
#endif
    }
  }

  //make sure something was selected
  if(NextMatchCasePtrs.size() == 0)
    return false;
  else
    //copy into the vector to be returned
    MatchingCasePtrs = NextMatchCasePtrs;

  return true;
}


//randomly selects a best case with a bias toward most successful case
//with normal distribution
CCBRCase* CCBRController::BestCaseSelection(vector<CCBRCase*> &MatchingCasePtrs, double&  fRetSimDegree) {
  double fMaxBestness = -1.0;
  double* fSelIntervals = new double[MatchingCasePtrs.size()];
  double* fBestnesses = new double[MatchingCasePtrs.size()];
  double* fSpatSims = new double[MatchingCasePtrs.size()];
  double fCurrFc[NUMOFREGIONS];
  unsigned int i;

  //compute the largest Case Bestness measure
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    double fSuccess, fTemporalSim, fSpatialSim;
    //get a new case from the library
    CCBRCase* pCase = MatchingCasePtrs[i];

    //get the success of the case
    fSuccess = pCase->GetCaseSuccess();

    //compute temporal similarity
    fTemporalSim = ComputeTimeSimwithCurrEnv(pCase);

    //Compute Fc for the case
    if(ComputeFc(&pCase->FeaturesVector,
                 pCase->FeaturesIndices,
                 fCurrFc) == false) {
      fprintf(stderr, "ERROR: Could not compute the library case FC\n");
      return NULL;
    }

    //now based on the Fc of environment and
    //the Fc of the case compute the degree of similarity
    fSpatialSim = ComputeSimDegree(fCurrEnvFc, fCurrFc);

    //remember the spatial similarity for each case
    fSpatSims[i] = fSpatialSim;

    //compute bestness of the case
    fBestnesses[i] = SELECT_SUCCESS_WEIGHT*fSuccess +
                     SELECT_TEMPORALSIM_WEIGHT*fTemporalSim + SELECT_SPATIALSIM_WEIGHT*fSpatialSim;

    //keep the maximum
    if(fBestnesses[i] > fMaxBestness)
      fMaxBestness = fBestnesses[i];
  }


  //iterate again and compute intervals on (0,1) for each case
  //the length of the interval is proportional to fMaxBestness-fBestness
  double fSumSelIntervals = 0;
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    //fBestnesses[i] is on [-1, 1]
    //Get the interval
    fSelIntervals[i] = exp(-pow((fMaxBestness-fBestnesses[i]), 2.0)/(2.0*pow(THIRDSEL_2STD/2.0, 2.0)));

    //sum of the intervals
    fSumSelIntervals += fSelIntervals[i];
  }

  //normalize all the intervals
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    fSelIntervals[i] = fSelIntervals[i]/fSumSelIntervals;
  }

  //Generate a uniformly drawn number on [0,1]
  double fRanNum = ((double)rand())/RAND_MAX;

  //see to which interval the number belongs
  double fEnd = 0;
  int nSelInd = -1;
  for(i = 0; i < MatchingCasePtrs.size(); i++) {
    //the running sum of the intervals
    fEnd += fSelIntervals[i];
    //if the number belongs to this interval then we have identified the case
    if(fRanNum < fEnd) {
      nSelInd = i;
      break;
    }
  }

  //make sure something was selected
  if(nSelInd == -1) {
    fprintf(stderr, "ERROR: Case was not selected at the third selection stage\n");
    return NULL;
  }

  //return the spatial similarity of the best case
  fRetSimDegree = fSpatSims[nSelInd];

  //cleanup
  delete [] fSelIntervals;
  delete [] fBestnesses;
  delete [] fSpatSims;

  //return the selected case
  return MatchingCasePtrs[nSelInd];
}

//decides whether the input case need to be created as new one or kept as old one
bool CCBRController::NeedCreateCase(CCBRCase* pCase, int nNumofMatchedCases) {
  double fSuccess, fTemporalSim, fSpatialSim, fOverallSim;
  double fCurrFc[NUMOFREGIONS];

  //limit the maximum number of cases
  if(pShortTermMemory->GetNumCases() >= LIBRARYSIZE || nNumofMatchedCases > NUMOFMATCHINGCASES) {
#ifdef MK_DEBUG
    fprintf(fDebugMKFile, "<%li>\n", dCt++);
    fprintf(fDebugMKFile, "(1) ADAPT: Over Limit\n");
    fprintf(fDebugMKFile, "    num: %i, overall num: %i\n", nNumofMatchedCases, pShortTermMemory->GetNumCases());
    fprintf(fDebugMKFile, "------------------\n");
#endif
    return false;
  }


  //get the success of the case
  fSuccess = pCase->GetCaseSuccess();

  //compute temporal similarity
  fTemporalSim = ComputeTimeSimwithCurrEnv(pCase);

  //Compute Fc for the case
  if(ComputeFc(&pCase->FeaturesVector,
               pCase->FeaturesIndices,
               fCurrFc) == false) {
    fprintf(stderr, "Could not compute the library case FC\n");
#ifdef MK_DEBUG
    fprintf(fDebugMKFile, "<%li>\n", dCt++);
    fprintf(fDebugMKFile, "(2) ADAPT: Could not compute FC\n");
    fprintf(fDebugMKFile, "    temporal: %f, success: %f, num: %i\n", fTemporalSim, fSuccess, nNumofMatchedCases);
    fprintf(fDebugMKFile, "------------------\n");
#endif
    return false;
  }

#ifdef MK_DEBUG
  int k;
  
  //case ID
  fprintf(fDebugMKFile, "<%li>\n", dCt++);
  fprintf(fDebugMKFile, "  Case ID   = %d\n", pCase->GetCaseID());
  fprintf(fDebugMKFile, "  Case Name = %s\n", pCase->GetCaseName());

  //traversability for the case
  for(k = 0; k < NUMOFREGIONS; k++)
    fprintf(fDebugMKFile, "  fCurrFC[%d]=%f", k, fCurrFc[k]);
  fprintf(fDebugMKFile, "\n");
  
  //traversability for the environment
  for(k = 0; k < NUMOFREGIONS; k++)
    fprintf(fDebugMKFile, "  fCurrEnvFC[%d]=%f", k, fCurrEnvFc[k]);
  fprintf(fDebugMKFile, "\n");
#endif

  //now based on the Fc of environment and
  //the Fc of the case compute the degree of similarity
  fSpatialSim = ComputeSimDegree(fCurrEnvFc, fCurrFc);

  //combined similarity
  fOverallSim = OVERALLSIM_TEMPORALSIM_WEIGHT*fTemporalSim +
    OVERALLSIM_SPATIALSIM_WEIGHT*fSpatialSim;

#ifdef MK_DEBUG
  //print out Overall Similarity (after it is computed)
  fprintf(fDebugMKFile, "  fOverallSim=%f\n", fOverallSim);
#endif

  //decision itself
  if(fSuccess > CREATECASE_SUCCESS_HIGH) {
    //the case has a high success rate
    if(fOverallSim > CREATECASE_SIM_HIGH) {
      //adapt this one if similarity is also high
#ifdef MK_DEBUG
      fprintf(fDebugMKFile, "(3) ADAPT: high success, high similarity\n");
      fprintf(fDebugMKFile, "    spatial: %f, temporal: %f, success: %f, num: %i\n", fSpatialSim, fTemporalSim, fSuccess, nNumofMatchedCases);
      fprintf(fDebugMKFile, "------------------\n");
#endif
      return false;	//adapt this case
    } else {
      //create a new one if similarity is low in order
      //not to re-adapt case on a wrong environment
#ifdef MK_DEBUG
      fprintf(fDebugMKFile, "(4) CREATE: high success, low similarity\n");
      fprintf(fDebugMKFile, "    spatial: %f, temporal: %f, success: %f, num: %i\n", fSpatialSim, fTemporalSim, fSuccess, nNumofMatchedCases);
      fprintf(fDebugMKFile, "------------------\n");
#endif
      return true;	//create a new case
    }
  } else {
    //the case has a low success rate
    if(fOverallSim > CREATECASE_SIM_LOW && nNumofMatchedCases > 1) { //mk021016, was 1, back to 1
      //adapt this case if the similarity is relatively high
      //and there is at most one more similar case
#ifdef MK_DEBUG
      fprintf(fDebugMKFile, "(5) ADAPT: low success, more cases\n");
      fprintf(fDebugMKFile, "    spatial: %f, temporal: %f, success: %f, num: %i\n", fSpatialSim, fTemporalSim, fSuccess, nNumofMatchedCases);
      fprintf(fDebugMKFile, "------------------\n");
#endif
      return false;	//adapt this case
    } else {
      //create new case
#ifdef MK_DEBUG
      fprintf(fDebugMKFile, "(6) CREATE: low success, no other cases around\n");
      fprintf(fDebugMKFile, "    spatial: %f, temporal: %f, success: %f, num: %i\n", fSpatialSim, fTemporalSim, fSuccess, nNumofMatchedCases);
      fprintf(fDebugMKFile, "------------------\n");
#endif
      return true;	//create a new case
    }
  }
}



#if 0	//non-learning adaptation process
//Adapt a case step

bool CCBRController::AdaptCurrentCase() {
  //adapt time to apply the case
  int nTemp = CurrentFeaturesIndices[CASETIME_FEATURE];
  double fCaseTime =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  CurrentFeaturesVector[nTemp].fValue;

  //get the time the current case suggest to apply
  nTemp = pCurrentCase->FeaturesIndices[CASETIME_FEATURE];
  nTimeThresh = (int)pCurrentCase->FeaturesVector[nTemp].fValue;

  /*
  //vary the Minimum case time application if case thrashing is suspected
  if(fCaseTime < 2*CASETIME_THRESH)
  nTimeThresh += 2.0*CASETIME_THRESH-fCaseTime + CASETIME_THRESH;
  else
  nTimeThresh = CASETIME_THRESH;
  */

  //increase noise if relative movement is very small
  double* pNoiseGain = &(pCurrentCase->GetOutValueAddress(NOISE_GAIN))->fValue;
  if(fLongTermRelMt < LONGTERMMOT_THRESH && fShortTermRelMt < SHORTTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_HIGH_MPY_THRESH,
                                      4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));
    /*debug - TODO - redo for simulations*/
    /*nTimeThresh += CASETIME_THRESH*__min(TIME_MPY_THRESH,
      4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));*/
    nTimeThresh += nTimeThresh*__min(TIME_MPY_THRESH,
                                     4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));

  } else if(fLongTermRelMt < LONGTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_MID_MPY_THRESH,
                                      4*LONGTERMMOT_THRESH/fLongTermRelMt);
    /*debug - TODO - redo for simulations*/
    /*nTimeThresh += CASETIME_THRESH*__min(TIME_MPY_THRESH,
      4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));*/
    nTimeThresh += nTimeThresh*__min(TIME_MPY_THRESH,				     4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));

  } else if(fShortTermRelMt < SHORTTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_LOW_MPY_THRESH,
                                      4*SHORTTERMMOT_THRESH/fShortTermRelMt);
  }


  return true;
}
#endif


//Adapt a case step
bool CCBRController::AdaptCurrentCase(bool bNewCase, vector<CCBRCase*>& PosSuccessCases,
                                      vector<CCBRCase*>& NegSuccessCases) {
  double fCurrentCaseW, fInterpCaseW;
  //  double fNegCaseW;
  unsigned int i;
  CCBRCase* pCaseTemp = CreateCaseofType(pCurrentCase->GetCaseType());

  //assign weights for the interpolation based on whether it is a new case or not
  if(bNewCase)
    fInterpCaseW = ADAPT_POS_NEWCASE_WEIGHT;
  else
    fInterpCaseW = ADAPT_POS_OLDCASE_WEIGHT;
  fCurrentCaseW = 1 - fInterpCaseW;

  //Get the total success of positive cases
  double fNorm = pCurrentCase->GetCaseSuccess()*fCurrentCaseW+0.5;
  for(i = 0; i < PosSuccessCases.size(); i++) {
    CCBRCase* pCase	= PosSuccessCases[i];
    fNorm += pCase->GetCaseSuccess()*fInterpCaseW;
  }

  /*
  	//do the interpolation with positive cases if there are any
  	if(PosSuccessCases.size() > 0)
  	{
  		//normalize the weights
  		fCurrentCaseW = fCurrentCaseW/fNorm;
  		fInterpCaseW = fInterpCaseW/fNorm;
  		//interpolate with the positive success cases
  		//CurrentCase = a1*CurrentCase + a2*Case1 +  ... + ak*Casek
  		 MpyOutputVCase(pCurrentCase, fCurrentCaseW*pCurrentCase->GetCaseSuccess()+0.5, pCurrentCase);
  		for(i = 0; i < PosSuccessCases.size(); i++)
  		{
  			CCBRCase* pCase	= PosSuccessCases[i];
  			AddandMpyOutputVCase(pCurrentCase, 1.0, pCase, pCase->GetCaseSuccess()*fInterpCaseW, 
  				pCurrentCase);
  		}
  	}
   
  	//assign weights for the negative component subtraction based on whether 
  	//it is a new case or not
  	if(bNewCase)
  		fNegCaseW = ADAPT_NEG_NEWCASE_WEIGHT;
  	else
  		fNegCaseW = ADAPT_NEG_OLDCASE_WEIGHT;
   
  	if(NegSuccessCases.size() > 0)
  	{
  		//subtract out the negative component
  		double fOutVLen = LengthOutputVCase(pCurrentCase);
  		for(i = 0; i < NegSuccessCases.size(); i++)
  		{
  			CCBRCase* pCase	= NegSuccessCases[i];
  			double fTemp = DotOutputVCase(pCurrentCase, pCase);	//X*Xn
  			fTemp = fTemp/pow(LengthOutputVCase(pCase), 2.0);	//X*Xn/|Xn|^2
  			fTemp = fTemp*pCase->GetCaseSuccess()*fNegCaseW;	//note: success is negative
  			AddandMpyOutputVCase(pCurrentCase, 1.0, pCase, fTemp, pCurrentCase);
  		}
  		//normalize back the vector
  		MpyOutputVCase(pCurrentCase, fOutVLen/LengthOutputVCase(pCurrentCase), pCurrentCase);
  	}
   
  */
  //---------add noise to the adaptation-----------------------
  //generate random direction vector
  CCBRCase* pRandCase = CreateCaseofType(pCurrentCase->GetCaseType());
  CCBRCase* pAdaptCase = CreateCaseofType(pCurrentCase->GetCaseType());
  SetCasetoRandDelta(pRandCase, LEARN_DELTA_MAGNITUDE*(1-0.95*pCurrentCase->GetCaseSuccess()));

  //initialize new adaptation vector with random vector
  *pAdaptCase = *pRandCase;

  //check the success of the latest adaptation of the case
  if(pCurrentCase->AdaptV.size() == 0)
    ;
  //no adaptation history - use the generated random vector
  else if(pCurrentCase->fImprov <= 0) {
    //last adaptation resulted in negative case performance change
    //interpolate between random vector and opposite of last adaptation vector
    pAdaptCase->OutputVector = pCurrentCase->AdaptV;
    AddandMpyOutputVCase(pRandCase, 0.8, pAdaptCase, -0.2, pAdaptCase);
  } else {
    //positive performance change of the last adaptation
    //continue with this adaptation
    pAdaptCase->OutputVector = pCurrentCase->AdaptV;

    //avoid local minima with NoiseGain = 0 and small MoveToGoal
    double* pNoiseGain = &(pCurrentCase->GetOutValueAddress(NOISE_GAIN))->fValue;
    double* pGoalGain = &(pCurrentCase->GetOutValueAddress(MOVETOGOAL_GAIN))->fValue;
    if(*pNoiseGain < ERROR_EPS)
      *pGoalGain = __min(*pGoalGain+0.1, MOVETOGOAL_GAIN_UPPER);
  }
  //remember the new adaptation vector
  pCurrentCase->AdaptV = pAdaptCase->OutputVector;



#if GOAL_TRAVERSE_CHECK
  double fAdaptNoiseGain = (pAdaptCase->GetOutValueAddress(NOISE_GAIN))->fValue;
  double fCurrentNoiseGain0 = (pCurrentCase->GetOutValueAddress(NOISE_GAIN))->fValue;

  double* pNoiseGain = &(pAdaptCase->GetOutValueAddress(NOISE_GAIN))->fValue;
  if(IsGoalTraversable()) {
    *pNoiseGain = 0.9*(*pNoiseGain);
#ifdef VERBOSE
    printf("Goal Traversable: decreasinging gain\n");
#endif
  } else {
    *pNoiseGain = 1.1*(*pNoiseGain) + 0.05;
#ifdef VERBOSE
    printf("Goal Intraversable: increasing gain\n");
#endif
  }
#endif

  //do the adaptation
  AddandMpyOutputVCase(pCurrentCase, 1.0, pAdaptCase, 1.0, pCaseTemp);
  CopyAdaptElements(pCaseTemp, pCurrentCase);

  delete pRandCase;
  delete pAdaptCase;

  //regular adaptation procedure
  int nTemp = CurrentFeaturesIndices[CASETIME_FEATURE];
  //  double fCaseTime =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  CurrentFeaturesVector[nTemp].fValue;
  nTemp = CurrentFeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  CurrentFeaturesVector[nTemp].fValue;

  //get the time the current case suggest to apply
  nTemp = pCurrentCase->FeaturesIndices[CASETIME_FEATURE];
  nTimeThresh = (int)pCurrentCase->FeaturesVector[nTemp].fValue;

  //increase noise if relative movement is very small
  double* pNoiseGain = &(pCurrentCase->GetOutValueAddress(NOISE_GAIN))->fValue;
  int* pNoisePers = &(pCurrentCase->GetOutValueAddress(NOISE_PERSISTANCE))->nValue;
  if(fLongTermRelMt < LONGTERMMOT_THRESH && fShortTermRelMt < SHORTTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_HIGH_MPY_THRESH,
                                      4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt));
    *pNoiseGain = __max(NOISE_SHORTTERM_LOWERBOUND, *pNoiseGain);
    *pNoisePers = (int)((*pNoisePers)*__min(NOISE_HIGH_MPY_THRESH,
                                      4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt)));
    nTimeThresh += (int)((double)nTimeThresh*__min(TIME_MPY_THRESH,
                                     4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt)));
  } else if(fLongTermRelMt < LONGTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_MID_MPY_THRESH,
                                      4*LONGTERMMOT_THRESH/fLongTermRelMt);
    *pNoisePers = (int)((double)(*pNoisePers)*__min(NOISE_MID_MPY_THRESH,
                                      4*LONGTERMMOT_THRESH/fLongTermRelMt));
    nTimeThresh += (int)((double)nTimeThresh*__min(TIME_MPY_THRESH,
                                     4*(LONGTERMMOT_THRESH+SHORTTERMMOT_THRESH)/(fLongTermRelMt+fShortTermRelMt)));
  } else if(fShortTermRelMt < SHORTTERMMOT_THRESH) {
    *pNoiseGain = (*pNoiseGain)*__min(NOISE_LOW_MPY_THRESH,
                                      4*SHORTTERMMOT_THRESH/fShortTermRelMt);
    *pNoiseGain = __max(NOISE_SHORTTERM_LOWERBOUND, *pNoiseGain);
    *pNoisePers = (int)((double)(*pNoisePers)*__min(NOISE_LOW_MPY_THRESH,
                                      4*SHORTTERMMOT_THRESH/fShortTermRelMt));
  }

  //limit the dimensions of the output vector
  LimitOutputV(pCurrentCase);

  //limit the gains for the safety of the robot
  LimitGainsforSafety(pCurrentCase);

  //update the case in the library
  CCBRCase *pLibCase = pShortTermMemory->GetCasebyID(pCurrentCase->GetCaseID());
  if(pLibCase == NULL) {
    fprintf(stderr, "ERROR: Current Case does not have a corresponding case in the library\n");
    exit(1);
  }
  pLibCase->OutputVector = pCurrentCase->OutputVector;
  pLibCase->AdaptV = pCurrentCase->AdaptV;

  return true;
}


bool CCBRController::GetPosandNegSuccessCases(vector<CCBRCase*>& PosSuccessCases,
    vector<CCBRCase*>& NegSuccessCases, int& nSimCasesNum) {
  double fSpatSim, fTemporalSim, fSuccess;
  int i;
  double fCurrFc[NUMOFREGIONS];

  //iterate again and select all the cases that are within some
  //probabilistic interval from MaxSimDegree
  nSimCasesNum = 0;
  for(i = 0; i < pShortTermMemory->GetNumCases(); i++) {
    //get a new case from the library
    CCBRCase* pCase = pShortTermMemory->GetCase(i);
    //Compute Fc for it
    if(ComputeFc(&pCase->FeaturesVector,
                 pCase->FeaturesIndices,
                 fCurrFc) == false) {
      fprintf(stderr, "Could not compute the library case FC\n");
      return false;
    }

    //now based on the Fc of environment and
    //the Fc of the case compute the degree of similarity
    fSpatSim = ComputeSimDegree(fCurrEnvFc, fCurrFc);

    if(fSpatSim < POSNEGSUCCESS_SPATSIM_THRESH)
      continue;

    //compute temporal similarity
    fTemporalSim = ComputeTimeSimwithCurrEnv(pCase);

    if(fTemporalSim < POSNEGSUCCESS_TEMPORALSIM_THRESH)
      continue;

    //new matching case is found
    nSimCasesNum++;

    //get the success of the case
    fSuccess = pCase->GetCaseSuccess();

    //add the case to either positive or negative cases based on its success
    if(fSuccess > POSSUCCESS_THRESH)
      PosSuccessCases.push_back(pCase);
    else if (fSuccess < NEGSUCCESS_THRESH)
      NegSuccessCases.push_back(pCase);
  }

  return true;

}

//returns a case with output vector scaled by the scalar
bool CCBRController::MpyOutputVCase(CCBRCase* pCase, double fScalar, CCBRCase* pOutCase) {
  *pOutCase = *pCase;
  unsigned int i;

  for(i = 0; i < pOutCase->OutputVector.size(); i++) {
    if(pOutCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.fValue = fScalar*pOutCase->OutputVector[i].Value.fValue;
    else if(pOutCase->OutputVector[i].nValueType == INT_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.nValue = pOutCase->OutputVector[i].Value.nValue;
    else if(pOutCase->OutputVector[i].nValueType == BOOL_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.bValue = pOutCase->OutputVector[i].Value.bValue;
    else {
      fprintf(stderr, "ERROR: unknown type of output value in MpyOutputVCase\n");
      exit(1);
    }
  }

  return true;
}


//returns a case with output vector consisting of a sum of two scaled vector
bool CCBRController::AddandMpyOutputVCase(CCBRCase* pCase1, double fScalar1,
    CCBRCase* pCase2, double fScalar2, CCBRCase* pOutCase) {
  unsigned int i;

  CCBRCase* pCaseTemp = CreateCaseofType(pCase2->GetCaseType());
  MpyOutputVCase(pCase1, fScalar1, pOutCase);
  MpyOutputVCase(pCase2, fScalar2, pCaseTemp);

  for(i = 0; i < pOutCase->OutputVector.size(); i++) {
    if(pOutCase->OutputVector[i].nValueType != pCaseTemp->OutputVector[i].nValueType) {
      fprintf(stderr, "ERROR: output value types don't match in addition\n");
      exit(1);
    }

    if(pOutCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.fValue += pCaseTemp->OutputVector[i].Value.fValue;
    else if(pOutCase->OutputVector[i].nValueType == INT_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.nValue += pCaseTemp->OutputVector[i].Value.nValue;
    else if(pOutCase->OutputVector[i].nValueType == BOOL_VALUE_TYPE)
      pOutCase->OutputVector[i].Value.bValue = pOutCase->OutputVector[i].Value.bValue ||
          pCaseTemp->OutputVector[i].Value.bValue ;
    else {
      fprintf(stderr, "ERROR: unknown type of output value in MpyOutputVCase\n");
      exit(1);
    }
  }

  delete pCaseTemp;

  return true;

}

//returns the length of the Output Vector
double CCBRController::LengthOutputVCase(CCBRCase* pCase) {
  double fLength = 0;
  unsigned int i;

  for(i = 0; i < pCase->OutputVector.size(); i++) {
    if(pCase->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
      fLength += pCase->OutputVector[i].Value.fValue;
    else if(pCase->OutputVector[i].nValueType == INT_VALUE_TYPE)
      fLength += pCase->OutputVector[i].Value.nValue;
    else if(pCase->OutputVector[i].nValueType == BOOL_VALUE_TYPE)
      fLength += (int)pCase->OutputVector[i].Value.bValue ;
    else {
      fprintf(stderr, "ERROR: unknown type of output value in MpyOutputVCase\n");
      exit(1);
    }
  }
  return fLength;
}


//returns the dot product of two output vectors of two cases
double CCBRController::DotOutputVCase(CCBRCase* pCase1, CCBRCase* pCase2) {
  double fDot = 0;
  unsigned i;

  for(i = 0; i < pCase1->OutputVector.size(); i++) {
    if(pCase1->OutputVector[i].nValueType != pCase2->OutputVector[i].nValueType) {
      fprintf(stderr, "ERROR: output value types don't match in dot product\n");
      exit(1);
    }

    if(pCase1->OutputVector[i].nValueType == FLOAT_VALUE_TYPE)
      fDot += pCase1->OutputVector[i].Value.fValue*pCase2->OutputVector[i].Value.fValue;
    else if(pCase1->OutputVector[i].nValueType == INT_VALUE_TYPE)
      fDot += pCase1->OutputVector[i].Value.nValue*pCase2->OutputVector[i].Value.nValue;
    else if(pCase1->OutputVector[i].nValueType == BOOL_VALUE_TYPE)
      fDot += pCase1->OutputVector[i].Value.bValue && pCase2->OutputVector[i].Value.bValue ;
    else {
      fprintf(stderr, "ERROR: unknown type of output value in MpyOutputVCase\n");
      exit(1);
    }
  }

  return fDot;
}


//adds a new learning structure for the current case
bool CCBRController::AddNewApplyRemoveOldLearn() {
  LEARNING NewLearnStruct;

  //--------Get the distance to the goal---------------
  double fD = GetCurrentfD();

  //--------Get the start of the Region features-------
  int nRegFeatStart = CurrentFeaturesIndices[REG_FEATURE];
  //make sure it is valid
  if(nRegFeatStart == FEATURE_NOT_PRESENT) {
    fprintf(stderr, "ERROR: No Region Features are found\n");
    return false;
  }
  //Get R(distance) of the region
  double fR = CurrentFeaturesVector[nRegFeatStart+1].fValue;
  fR = __min(fD, fR);
  fR = __min(D_TRESH, fR);

  //initialize the learning structure
  NewLearnStruct.fHeuristic = fD;        //heuristic is distance to the goal
  NewLearnStruct.fReward = LEARN_REWARD_INIT;
  NewLearnStruct.nCaseID = pCurrentCase->GetCaseID();
  NewLearnStruct.nTimeApplied = 0;
  NewLearnStruct.bDelayedReinf = false;
  NewLearnStruct.fEndHeuristic = fD;
  NewLearnStruct.bApplied = false;
  //NewLearnStruct.fHeurThresh = fR; - TODO - change back
  NewLearnStruct.fHeurThresh = pCurrentCase->fHeurThresh;

#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "****New Learn:******\n");
#endif
  PrintLearn(NewLearnStruct);

  //process case repetitions in learning vector
  ProcessCaseRepetitions(NewLearnStruct.nCaseID);

  //is the learning vector is full?
  if(LearningV.size() == LEARN_WINDOW) {
    //then we need to remove the oldest case in learning vector
    //apply the learned knowledge in it
    //to the corresponding case if it was not applied already
    if(!LearningV[LearningV.size()-1].bApplied) {
      if(!ApplyLearnedKnowledge(LearningV.size()-1)) {
#ifdef VERBOSE
        printf("The learning was skipped for the case with ID = %d\n",
               LearningV[LearningV.size()-1].nCaseID);
#endif
      }
    }
    //and remove this element
    LearningV.pop_back();
  }

  //add to the head of the learning vector the current case learning structure
  LearningV.insert(LearningV.begin(), NewLearnStruct);

  return true;
}

//apply the learned success to the case the structure corresponds to
bool CCBRController::ApplyLearnedKnowledge(int nLearnStructIndex) {
#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "***Apply Knowledge for %d Learn****\n", nLearnStructIndex);
#endif
  
  //get the structure that needs to be processed
  LEARNING* pLearnStruct = &LearningV[nLearnStructIndex];

  //get the case that it corresponds to
  CCBRCase* pCase = pShortTermMemory->GetCasebyID(pLearnStruct->nCaseID);
  //if there is no such case then return false
  if(pCase == NULL)
    return false;

  //-----Case Improvement Update---------
  //get the current heuristic value and time
  double fD = pLearnStruct->fEndHeuristic;
  int nOverallTime = pLearnStruct->nTimeApplied;
  //if reinforcement is delayed then use the current value of goal distance
  //rather than the one that was the result of the direct case application
  //and set the overall time to the time since the case first was applied
  //until now
  if(pLearnStruct->bDelayedReinf) {
    fD = GetCurrentfD();
    nOverallTime = GetCumulativeCaseApplicationTime(nLearnStructIndex);
  }

  //debug-learn double fCasePerf = pLearnStruct->fHeuristic - fD;
  //debug-learn double fCaseImprov = fCasePerf - pLearnStruct->fHeurThresh;
  double fCasePerf = (pLearnStruct->fHeuristic - fD)/nOverallTime;
  double fCaseImprov = fCasePerf - LEARN_HEUR_DELTA*pLearnStruct->fHeurThresh;

  //maximum tracker for the heuristic threshold
  if(fCasePerf > pLearnStruct->fHeurThresh)
    pCase->fHeurThresh = fCasePerf;
  else
    pCase->fHeurThresh = LEARN_HEUR_SMOOTH*pCase->fHeurThresh +
                         (1-LEARN_HEUR_SMOOTH)*fCasePerf;

  //smooth the case improvement
  if(fCaseImprov >= 0 && fCasePerf > ERROR_EPS) {
    /*
    //positive improvement
    if(pCase->fImprov < 0)
    pCase->fImprov = __max(CASE_IMPROV_SMOOTH_COEFF*fCaseImprov, ERROR_EPS);
    else
    pCase->fImprov = 
    (1-CASE_IMPROV_SMOOTH_COEFF)*pCase->fImprov
    + CASE_IMPROV_SMOOTH_COEFF*fCaseImprov;
    */
    pCase->fImprov = __max(1, pCase->fImprov+1);
  } else {
    /*
    //negative improvement
    pCase->fImprov = 
    (1-CASE_IMPROV_SMOOTH_COEFF)*pCase->fImprov
    + CASE_IMPROV_SMOOTH_COEFF*fCaseImprov;
    */
    pCase->fImprov = pCase->fImprov-1;
  }
  //bound from above
  pCase->fImprov = __min(100, pCase->fImprov);
  //------------------------------------

  //---------------Success Update-----------------
  double fReward = pLearnStruct->fReward;

#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "before scaling: fReward=%f ", fReward);
#endif
  
  //make reward proportional to consistency in case improvement if reward is positive
  if(pCase->fImprov > CASE_IMPROVEMENT_THRESH && fReward > 0)
    fReward = fReward*pCase->fImprov/CASE_IMPROVEMENT_THRESH;
#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "after scaling: fReward=%f \n", fReward);
#endif

  double fNewSuccess = pCase->GetCaseSuccess() + fReward;

  //limit the success
  fNewSuccess = __min(1.0, fNewSuccess);
  fNewSuccess = __max(-1.0, fNewSuccess);

  //filter the success
  //TODO - make smooth coefficient as define
  fNewSuccess = 0.9*pCase->GetCaseSuccess() + 0.1*fNewSuccess;

  //set the new success for the case
  pCase->SetCaseSuccess(fNewSuccess);
  //-----------------------------------------------

#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "Caseperf=%f CaseImprov=%f fReward=%f fSuccess=%f \n",
          fCasePerf, fCaseImprov, fReward, fNewSuccess);
#endif

  //save the library
  SaveLibrary();

  return true;
}

//based on heuristic propogate the reward back to the stored learning structures
bool CCBRController::PropogateLearning() {
  unsigned int i;

  //get the current value of the heuristic - goal distance
  double fD = GetCurrentfD();

  //iterate through LEARN_WINDOW learning structures
  //at the head of the list is the most recent case.
  int nOverallTime = 0;
  for(i = 0; i < __min(LearningV.size(), LEARN_WINDOW); i++) {
    LEARNING* pLearnStruct = &LearningV[i];

    //update the overall time since the learning structure was applied
    nOverallTime += pLearnStruct->nTimeApplied;

    //debug-learn double fCasePerf = pLearnStruct->fHeuristic - fD;
    //debug-learn double fCaseImprov = fCasePerf - pLearnStruct->fHeurThresh;
    double fCasePerf = (pLearnStruct->fHeuristic - fD)/nOverallTime;
    //    double fCaseImprov = fCasePerf - pLearnStruct->fHeurThresh;

    /*
    //decide whether this case needs learning horizon of more than itself
    if(pLearnStruct->fHeuristic < fD)
  {
    //robot had to go back at some point
    pLearnStruct->bDelayedReinf = true;
  }
    */

    //do reward processing of only cases that need delayed reinforcement or were just used
    if(i == 0 || pLearnStruct->bDelayedReinf) {

#ifdef MK_DEBUG
      double oldReward = pLearnStruct->fReward;
#endif
      
      //if positive progress
      if( (fCasePerf > LEARN_HEUR_DELTA*pLearnStruct->fHeurThresh && !pLearnStruct->bDelayedReinf)
          || (pLearnStruct->fHeuristic - fD > LEARN_GOALDISTDELTA && pLearnStruct->bDelayedReinf))
        //debug-learn && fCasePerf > LEARN_GOALDISTDELTA)
      {
        //bound from below
        pLearnStruct->fReward = __max(pLearnStruct->fReward, 0);

        //increase the reward
        /*	              pLearnStruct->fReward += LEARN_POS_DELTA*fCaseImprov*
           (LEARN_WINDOW - i); */
        pLearnStruct->fReward += LEARN_POS_DELTA;

      }
      //if negative progress
      //debug-learn else if(fD - pLearnStruct->fHeuristic > LEARN_GOALDIST_NEGTHRESH)
      else if(fCasePerf < 0) {
        //bound from above
        pLearnStruct->fReward = __min(pLearnStruct->fReward, LEARN_REWARD_INIT);

        //increase the reward
        /*		      pLearnStruct->fReward -= LEARN_NEG_DELTA*(fD - pLearnStruct->fHeuristic - LEARN_GOALDIST_NEGTHRESH)*
           (i+1); */

        pLearnStruct->fReward -= LEARN_NEG_DELTA;

      }
      //positive progress but not that strong
      else {
        if(i == 0)
          //penalize if no strong progress due to this case application
          pLearnStruct->fReward = LEARN_REWARD_INIT;
        else
          //decrease reward if it is due to the next case application
          pLearnStruct->fReward -= LEARN_NEG_DELTA;
      }

      //limit the reward value
      pLearnStruct->fReward = __min(LEARN_REWARD_MAX, pLearnStruct->fReward);
      pLearnStruct->fReward = __max(LEARN_REWARD_MIN, pLearnStruct->fReward);
#ifdef MK_DEBUG
      if (pLearnStruct->bDelayedReinf) {
        fprintf(fDebugMKFile, "<%li>\n", dCt++);
        CCBRCase* pCase = pShortTermMemory->GetCasebyID(pLearnStruct->nCaseID);
        fprintf(fDebugMKFile, "Delayed reward for case %s:\n",pCase->GetCaseName());
        fprintf(fDebugMKFile, "  New success value: %f\n", pLearnStruct->fReward);
        fprintf(fDebugMKFile, "  Increment was: %f\n", pLearnStruct->fReward - oldReward);
      }
#endif
    }

#if 0
    //update case improvement when reinforcement is to used
    if( (i == 0 && !pLearnStruct->bDelayedReinf) ||
        (i == LEARN_WINDOW-1 && pLearnStruct->bDelayedReinf)) {
      //set a new case performance threshold
      CCBRCase* pCase = pShortTermMemory->GetCasebyID(pLearnStruct->nCaseID);
      if(pCase == NULL) {
        fprintf(stderr, "ERROR: Case not found in propogate learning\n");
        exit(1);
      }

      //maximum tracker for the case improvement
      if(fCaseImprov > 0)
        pCase->fHeurThresh = fCasePerf;
      else
        pCase->fHeurThresh = LEARN_HEUR_SMOOTH*pCase->fHeurThresh +
                             (1-LEARN_HEUR_SMOOTH)*fCasePerf;

      //smooth the case improvement
      if(fCaseImprov > 0) {
        //positive improvement
        if(pCase->fImprov < 0)
          pCase->fImprov = CASE_IMPROV_SMOOTH_COEFF*fCaseImprov;
        else
          pCase->fImprov =
            (1-CASE_IMPROV_SMOOTH_COEFF)*pCase->fImprov
            + CASE_IMPROV_SMOOTH_COEFF*fCaseImprov;
      } else {
        //negative improvement
        pCase->fImprov =
          (1-CASE_IMPROV_SMOOTH_COEFF)*pCase->fImprov
          + CASE_IMPROV_SMOOTH_COEFF*fCaseImprov;
      }
    }
#endif
#ifdef CREATE_DEBUG_FILES
    fprintf(fDebug2File, "***LearnStruct[%d]:***\n", i);
#endif
    PrintLearn(*pLearnStruct);
  }

  return true;
}


//returns true if current robot situation is goal traversable
//based on Fisher Linear Discriminant suggested feature projection
bool CCBRController::IsGoalTraversable() {
  double fValue = 0;

  //Assume that there are 4 regions
  fValue += fCurrEnvFc[0]*PRWR0;
  fValue += fCurrEnvFc[1]*PRWR1;
  fValue += fCurrEnvFc[2]*PRWR2;
  fValue += fCurrEnvFc[3]*PRWR3;

  //add shortterm movement
  int nTemp = CurrentFeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  CurrentFeaturesVector[nTemp].fValue;

  fValue += fShortTermRelMt*PRWSTMV;

#ifdef CREATE_DEBUG_FILES
  fprintf(fDebugFile, "ProjectValue = %f\n", fValue);
#endif

  //threshold
  if(fValue > PR_THRESH)
    return false;
  else
    return true;
}


void CCBRController::CopyAdaptElements(CCBRCase* pSourceCase, CCBRCase* pDestCase) {

  for(unsigned int i = 0; i < pSourceCase->OutputVector.size(); i++) {
    if(pSourceCase->OutputVector[i].bAdapt)
      pDestCase->OutputVector[i] = pSourceCase->OutputVector[i];
  }

}

void CCBRController::PrintNewCaseCreated() {
#ifdef CREATE_DEBUG_FILES
  int i;

  fprintf(fDebug2File, "-------------New Case Created---------------\n");
  int nTemp;
  fprintf(fDebug2File, "Case Name: %s (ID = %d) \n",
          pCurrentCase->GetCaseName(), pCurrentCase->GetCaseID());
  nTemp = pCurrentCase->FeaturesIndices[SHORTTERMRELMOT_FEATURE];
  double fShortTermRelMt =  pCurrentCase->FeaturesVector[nTemp].fValue;
  nTemp = pCurrentCase->FeaturesIndices[LONGTERMRELMOT_FEATURE];
  double fLongTermRelMt =  pCurrentCase->FeaturesVector[nTemp].fValue;
  fprintf(fDebug2File, "Short-Term: %f Long-Term: %f\n",
          fShortTermRelMt, fLongTermRelMt);
  fprintf(fDebug2File, "Case Success: %f\n", pCurrentCase->GetCaseSuccess());
  fprintf(fDebug2File, "Case Improvement: %f\n", pCurrentCase->fImprov);
  fprintf(fDebug2File, "Heur Threshold: %f\n", pCurrentCase->fHeurThresh);
  for(i = 0; i < pCurrentCase->OutputVector.size(); i++) {
    fprintf(fDebug2File, "%s = ", pCurrentCase->OutputVector[i].strStringId);
    switch(pCurrentCase->OutputVector[i].nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fDebug2File, "%f\n", pCurrentCase->OutputVector[i].Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fDebug2File, "%d\n", pCurrentCase->OutputVector[i].Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: wrong format for Output element\n");
      return;
    };
  }
  fprintf(fDebug2File, "----------------------------------------------------\n");

  //save the file
  fflush(fDebug2File);
#endif
}

//asterisk is used to separate the name of the case from the ID that is also in the
//name of the case in order to make all the names unique
void CCBRController::CreateNewName(CCBRCase* pCase) {
  char sTemp[200];
  char *sTemp1;

  strcpy(sTemp, pCase->GetCaseName());
  sTemp1 = strchr(sTemp, '*');
  //if asterisk is in the name of the case
  if(sTemp1 != NULL)
    sTemp1[1] = '\0';
  //otherwise
  else {
    //add an asterisk
    strcat(sTemp, "*");
  }
  sprintf(sTemp, "%s%d", sTemp, pCase->GetCaseID());
  pCase->SetCaseName(sTemp);

}




void PrintCase(CCBRCase* pCase) {
#ifdef CREATE_DEBUG_FILES
  unsigned int i, k;

  fprintf(fDebug2File, "+++++++Case Name: %s+++++++\n", pCase->GetCaseName());
  for(i = 0; i <  pCase->FeaturesVector.size(); i++) {
    fprintf(fDebug2File, "Feature Name: %s, Value: %f\n",
            pCase->FeaturesVector[i].strName, pCase->FeaturesVector[i].fValue);
  }
  fprintf(fDebug2File, "Case Success: %f; ", pCase->GetCaseSuccess());
  fprintf(fDebug2File, "Case Improv: %f; ", pCase->fImprov);
  fprintf(fDebug2File, "Heur Thresh: %f\n", pCase->fHeurThresh);

  for(i = 0; i < pCase->OutputVector.size(); i++) {
    fprintf(fDebug2File, "%s = ", pCase->OutputVector[i].strStringId);
    switch(pCase->OutputVector[i].nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fDebug2File, "%f\n", pCase->OutputVector[i].Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fDebug2File, "%d\n", pCase->OutputVector[i].Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: wrong format for Output element\n");
      return;
    };
  }

  fprintf(fDebug2File, "Adaptation Vector:\n");
  for(i = 0; i < pCase->AdaptV.size(); i++) {
    fprintf(fDebug2File, "%s = ", pCase->AdaptV[i].strStringId);
    switch(pCase->OutputVector[i].nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fDebug2File, "%f\n", pCase->AdaptV[i].Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fDebug2File, "%d\n", pCase->AdaptV[i].Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: wrong format for Output element\n");
      return;
    };
  }
  fprintf(fDebug2File, "++++++++++++++++++++++++++++++++++++++++\n");

  //save the file
  fflush(fDebug2File);
#endif

}


void CCBRController::ResetDerivedCase(CCBRCase* pCase) {
  pCase->SetCaseSuccess(0);
  pCase->fImprov = 0;
  pCase->fHeurThresh = 0;
}


void CCBRController::UpdateCurrentLearning() {
  //make sure that we have some learning structures
  if(LearningV.size() == 0)
    return;

  LEARNING* pLearnStruct = &LearningV[0];

  //update the time that case was applied
  pLearnStruct->nTimeApplied = GetCurrentCaseTime();

  //update the heuristics advancement
  pLearnStruct->fEndHeuristic = GetCurrentfD();

  //decide whether this case needs learning horizon of more than itself
  if(pLearnStruct->fHeuristic <= pLearnStruct->fEndHeuristic) {
    //robot had to go back at some point
    pLearnStruct->bDelayedReinf = true;
#ifdef MK_DEBUG
    fprintf(fDebugMKFile, "<%li>\n", dCt++);
    fprintf(fDebugMKFile, "reinforcement delayed for case %s\n", pCurrentCase->GetCaseName());
#endif

  }


}


int CCBRController::GetCurrentCaseTime() {
  int nTime = CaseHistory.nCount;
  nTime += CaseHistory.nTime*TIME_UNITS;

  return nTime;
}

//return the current distance to the goal
double CCBRController::GetCurrentfD() {
  int nTemp = CurrentFeaturesIndices[GOALDIST_FEATURE];
  //make sure that it is valid
  if(nTemp == FEATURE_NOT_PRESENT) {
    fprintf(stderr, "ERROR: No Goal Distance Feature is found\n");
    return false;
  }
  return CurrentFeaturesVector[nTemp].fValue;
}

void CCBRController::ProcessCaseRepetitions(int nNewCaseID) {
  //iterate through all the cases currently under learning and see if any of
  //them are the same as CurrentCase
  for(unsigned int i = 0; i < __min(LearningV.size(), LEARN_WINDOW); i++) {
    LEARNING* pLearnStruct = &LearningV[i];

    if(pLearnStruct->nCaseID == nNewCaseID && !pLearnStruct->bApplied) {
      //the case is repeated by current case and was not applied - apply the knowledge
      if(!ApplyLearnedKnowledge(i)) {
#ifdef VERBOSE
        printf("The learning was skipped for the case with ID = %d\n",
               LearningV[LearningV.size()-1].nCaseID);
#endif
      }

      //the case was applied
      pLearnStruct->bApplied = true;
    }
  }
}

int CCBRController::GetCumulativeCaseApplicationTime(int nLearnStructIndex) {
  int i;
  int nOverallTime = 0;

  //iterate through all the cases from the most recent one to the
  //one we are interested in and add up their application times
  for(i = 0; i <= nLearnStructIndex; i++) {
    nOverallTime += LearningV[i].nTimeApplied;
  }

  return nOverallTime;
}

void CCBRController::PrintLearn(LEARNING LearnStruct) {
#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug2File, "CaseID=%d fHeur=%f fEndHeur=%f nTime=%d fHeurThresh=%f fReward=%f bDelayed=%d bApplied=%d\n",
          LearnStruct.nCaseID, LearnStruct.fHeuristic, LearnStruct.fEndHeuristic,
          LearnStruct.nTimeApplied, LearnStruct.fHeurThresh,
          LearnStruct.fReward, (int)LearnStruct.bDelayedReinf, (int)LearnStruct.bApplied);
#endif
}

void CCBRController::LimitGainsforSafety(CCBRCase* pCase) {
  double* pNoiseGain = &(pCase->GetOutValueAddress(NOISE_GAIN))->fValue;
  double* pObstacleGain = &(pCase->GetOutValueAddress(OBSTACLE_GAIN))->fValue;
  double* pMoveToGoalGain = &(pCase->GetOutValueAddress(MOVETOGOAL_GAIN))->fValue;

  if(*pObstacleGain < *pNoiseGain + *pMoveToGoalGain)
    *pObstacleGain = *pNoiseGain + *pMoveToGoalGain;

}



/**********************************************************************
 * $Log: Controller.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/04 18:42:16  endo
 * gasdev() replaced by GaussianRandom().
 *
 * Revision 1.1.1.1  2005/02/06 23:00:25  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/05 23:36:22  kaess
 * Initial revision
 *
 **********************************************************************/
