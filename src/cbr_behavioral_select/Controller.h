/**********************************************************************
 **                                                                  **
 **                         Controller.h                             **
 **                                                                  **
 ** Contains the definition of the CBR Controller Class              **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Controller.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBR_CONTROLLER_
#define _CBR_CONTROLLER_


class CCBRCase;
class CCBRShortTermMemory;
class CCBROutElement;

class CCBRController {

  //data
private:
  //current state
  enum CBRCONTROLLER_STATE nState;
  //The list of currently loaded cases(local library)
  CCBRShortTermMemory* pShortTermMemory;
  //The file containing the cases
  FILE* fCaseFile;
  //the vector of current environment features
  vector<structFeature> CurrentFeaturesVector;
  //the vector of indices of the environment features above
  int CurrentFeaturesIndices[FEATURE_SIZE];
  //the Fc vector of the current environment
  double fCurrEnvFc[NUMOFREGIONS];
  //FC vector with the old values for the smoothing of the above
  double fOldEnvFc[NUMOFREGIONS];
  //The weight vector for the Fc vector
  double fWeightV[NUMOFREGIONS];
  //the vector of all the possible output elements(actions)
  vector<CCBROutElement> FullOutputVector;
  //the limits of the above
  vector<CCBROutElement> OutputUpperLimitVector;
  vector<CCBROutElement> OutputLowerLimitVector;
  //The Number of cases in the whole library
  int nCaseNum;
  //some history on the current case
  HISTORY CaseHistory;
  //minimum time(except for another branch of decision tree) to apply the case
  int nTimeThresh;
  //the vector of learning structures for LearnWindow of cases in the past
  //the 0th is the current case, the LearnWindow is the oldest case
  vector<LEARNING> LearningV;

public:
  //Current Case
  CCBRCase* pCurrentCase;

  //constructors & destructors
public:
  CCBRController ();
  ~CCBRController();


  //functions and operators
public:
  //initialize the controller
  bool Initialize();
  //Identify features step
  bool IdentifyFeatures(structRobotPos RobotPos, structObstaclePos ObstaclesPosArray[],
                        int nNumofObstacles, PT GoalPos);
  //Select and Adapt case step
  bool SelectandAdaptCase();
  //Apply a case step
  bool ApplyCurrentCase();
  //Resets the state for the new task
  //and the learning on the previous case (learning 2 step)or queueing it up for the later.
  bool NewTaskReset();
  //Returns true if it the Controller started a new task
  bool IsNewTask();
  //Data Operations functions
  CBRCONTROLLER_STATE GetState() {
    return nState;
  };
  //Save the whole library
  void SaveLibrary();
  //adds the current case to the library
  bool AddCurrentCase();
  //reset function on a new case
  bool ResetwithNewCase();
  //print some info about the old case
  void PrintOldCaseDebug();


  //utility functions
private:
  //generates a default case - if nothing else fits
  CCBRCase* CreateDefaultCase();
  //sets the output vector space and its limits
  void SetOutputVectorSpace();
  void SetOutputVectorUpperLimit();
  void SetOutputVectorLowerLimit();
  //Get the most similar case to the current environment
  CCBRCase* GetBestMatchCase(double&  fSimDegree);
  //Case swapping choice decision tree
  bool NeedSwapCase(CCBRCase* pNewCase, double fSimDegree);
  //Computes Fc vector
  bool ComputeFc(vector<structFeature>* pFeaturesVector,
                 int FeaturesIndices[], double fFc[]);
  //Returns the degree of similarity between two Fc vectors
  double ComputeSimDegree(double fFc1[], double fFc2[]);
  //set the weight vector
  bool SetWeightVector();
  //Adapt the current case
  bool AdaptCurrentCase(bool bNewCase, vector<CCBRCase*>& PosSuccessCases,
                        vector<CCBRCase*>& NegSuccessCases);
  //Computes the similarity with the environment in time domain
  double ComputeTimeSimwithCurrEnv(CCBRCase* pCase);
  bool SmoothFC(double fNewFC[], double fOldFC[]);
  //case selection functions
  bool TemporalSelection(vector<CCBRCase*> &MatchingCasePtrs);
  bool SpatialSelection(vector<CCBRCase*> &MatchingCasePtrs);
  CCBRCase* BestCaseSelection(vector<CCBRCase*> &MatchingCasePtrs, double&  fRetSimDegree);
  //returns true if new case need to be created in the library, false - otherwise
  bool NeedCreateCase(CCBRCase* pCase, int nNumofMatchedCases);
  //get positive and negative successes cases similar to the current environment
  bool GetPosandNegSuccessCases(vector<CCBRCase*>& PosSuccessCases,
                                vector<CCBRCase*>& NegSuccessCases,  int& nSimCasesNum);
  //adds a new learning structure for the current case and learn on the oldest one
  //and then remove it
  bool AddNewApplyRemoveOldLearn();
  //apply the learned success to the case the indexed structure corresponds to
  bool ApplyLearnedKnowledge(int nLearnStructIndex);
  //based on heuristic propogate the reward back to the stored learning structures
  bool PropogateLearning();


  //returns a case with output vector scaled by the scalar
  bool MpyOutputVCase(CCBRCase* pCase, double fScalar, CCBRCase* pOutCase);
  //returns a case with output vector consisting of a sum of two scaled vector
  bool AddandMpyOutputVCase(CCBRCase* pCase1, double fScalar1,
                            CCBRCase* pCase2, double fScalar2, CCBRCase* pOutCase);
  //returns the length of the output vector for a case
  double LengthOutputVCase(CCBRCase* pCase);
  //returns the dot product of two output vectors of two cases
  double DotOutputVCase(CCBRCase* pCase1, CCBRCase* pCase2);
  //sets case output value to random delta where each value is less thay fDeltaMag except integer values are 1 or -1
  void SetCasetoRandDelta(CCBRCase* pCase, double fDeltaMag);
  //limits Output Vector for a case
  void LimitOutputV(CCBRCase* pCase);
  //is goal traversable
  bool IsGoalTraversable();
  //copy adaptable elements
  void CopyAdaptElements(CCBRCase* pSourceCase, CCBRCase* pDestCase);
  //create a new name for the given case using its case id and its old name
  void CreateNewName(CCBRCase* pCase);
  void ResetDerivedCase(CCBRCase* pCase);
  int GetCurrentCaseTime();
  double GetCurrentfD();
  void UpdateCurrentLearning();
  void ProcessCaseRepetitions(int nNewCaseID);
  int GetCumulativeCaseApplicationTime(int nLearnStructIndex);

  void PrintNewCaseCreated();
  void PrintLearn(LEARNING LearnStruct);
  void LimitGainsforSafety(CCBRCase* pCase);
};




//global functions
void PrintCase(CCBRCase* pCase);


#endif


/**********************************************************************
 * $Log: Controller.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:26  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/05 23:36:22  kaess
 * Initial revision
 *
 **********************************************************************/
