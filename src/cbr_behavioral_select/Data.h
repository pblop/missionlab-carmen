/**********************************************************************
 **                                                                  **
 **                             Data.h                               **
 **                                                                  **
 ** Contains the declaration of other CBR related data types         **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Data.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBR_DATA_
#define _CBR_DATA_

//#define VERBOSE
//#define CREATE_FEAT_DEBUG_FILE
//#define CREATE_DEBUG_FILES

#define REAL_ROBOT 0

//configuration defines
#define MAXNUMOBSTACLES  150   //the maximum limit on the obstacles
//#define MAXNUMOBSTACLES  600

//real vs. simulation
#if REAL_ROBOT == 0   //simulation

#define SENSOR_THRESH 50    //the distance beyond which no obstacles are detected
//#define SENSOR_THRESH 100

#define TIME_UNITS              5  //TODO - was 10   //case time is given in these units
//safety margin used to add on top of the obstacle radius
#define OBS_SAFETY_MARGIN       0.5
//The low threshold for case similarity
#define CASESIMLOW_THRESH       0.6
//The high threshold for case similarity
#define CASESIMHIGH_THRESH      0.85
//The threshold for the realtive similarity in the new case decision tree
#define CASESIMDIFF_THRESH      0.3
//Maximum velocity of a robot (length over 1 cycle)
#define MAXVEL                  0.1
//The long window size for the long term relative motion
#define LONGTERM_LONGWINSIZE  600   //200     //in cycles
//The short window size for the long term relative motion
#define LONGTERM_SHORTWINSIZE  40    //40    //in cycles
//the max limit for the time threshold multiply
#define TIME_MPY_THRESH         5.0

#else                 //real robot

//Maximum velocity of a robot (length over 1 cycle)
#define MAXVEL                  0.1 //mk020612 0.05
#define ROBOT_RADIUS            0.3
//#define SENSOR_THRESH        10  //the distance beyond which no obstacles are detected
#define SENSOR_THRESH           5  //the distance beyond which no obstacles are detected
#define TIME_UNITS              5  //case time is given in these units
//safety margin used to add on top of the obstacle radius
//#define OBS_SAFETY_MARGIN       0.0
#define OBS_SAFETY_MARGIN       1.5 //mk021102: needs to correspond to outdoor safety margin
//The low threshold for case similarity
#define CASESIMLOW_THRESH       0.6
//The high threshold for case similarity
#define CASESIMHIGH_THRESH      0.9
//The threshold for the realtive similarity in the new case decision tree
#define CASESIMDIFF_THRESH      0.1
//The long window size for the long term relative motion
#define LONGTERM_LONGWINSIZE  200     //in cycles
//The short window size for the long term relative motion
#define LONGTERM_SHORTWINSIZE  40      //in cycles
//the max limit for the time threshold multiply
#define TIME_MPY_THRESH         3.0

#endif


//Filename defines
#define CASEFILENAME   "cbr_behavioral_select.cases"

#define DEBUGFILENAME  "CBR.debug"
#define DEBUG1FILENAME "CBRFeat.debug" // this file needed for test scripts!!! mk
#define DEBUG2FILENAME "CBRLearn.debug"

//Misc. defines
#ifdef WINCOMPILER
#define PI 3.14159265
#endif

//the states the CBR Controller can be in
enum CBRCONTROLLER_STATE {INIT_STATE=0, FIRST_NEW_TASK_STATE, NEW_TASK_STATE,
                          CASE_NOT_FOUND_STATE, CASE_FOUND_STATE};

//the output value types
enum OUTVALUE_TYPE {FLOAT_VALUE_TYPE=0, BOOL_VALUE_TYPE, INT_VALUE_TYPE};

//the types of the cases
enum CASE_TYPE {NO_CASE_TYPE=0, MOVETO_CASE_TYPE};

//the maximum allowed case ID
//(meaning the bound on the number of cases that EVER existed in the library)
#define MAX_ALLOWED_CASEID  10000
//the maximum number of cases to maintain in the library
#define LIBRARYSIZE  10
//the maximum number of similar cases in the library
#define NUMOFMATCHINGCASES  3

//the values for the case successes
#define NEGATIVE_CASE_SUCCESS -1.0  //negative success
#define UNKNOWN_CASE_SUCCESS   0.0   //unknown or neutral
#define NEUTRAL_CASE_SUCCESS   UNKNOWN_CASE_SUCCESS
#define POSITIVE_CASE_SUCCESS  1.0   //positive success

//The number of regions used for the obstacle density feature
#define NUMOFREGIONS      4
//The decay constant with which the length of the region's decays
//TODO - set up the decay based on going into the negative direction
//rather that on the index of the interval
#define REGLENGTHDECAY    1.0
//The density of a region that is considered to be critical(not
//allowing to pass through
#define REGDENSITYTHRESH  0.9

//The long window size for the short term relative motion
#define SHORTTERM_LONGWINSIZE    20  //50    //in cycles
//The short window size for the short term relative motion
#define SHORTTERM_SHORTWINSIZE    5  //10    //in cycles
//The decay in the Long Window average position computations
//for the short term relative motion
//The position will reach 0.9 of a new position in
//SHORTTERM_LONGWINSIZE/2 cycles given the new pos
//differs from old two times
#define SHORTTERM_LONGWINDECAY    pow((1-0.9)/(1-0.5),1.0/(SHORTTERM_LONGWINSIZE/2.0))
//The decay in the Short Window average position computations
//for the short term relative motion
#define SHORTTERM_SHORTWINDECAY   pow((1-0.9)/(1-0.5),1.0/(SHORTTERM_SHORTWINSIZE/2.0))
//The normalization factor for the velocity based on the short term window
#if REAL_ROBOT==1   //debug-TODO- fix, should be the same
#define SHORTTERM_NORM ((SHORTTERM_LONGWINSIZE - SHORTTERM_SHORTWINSIZE)/4.0);
#else
#define SHORTTERM_NORM ((SHORTTERM_LONGWINSIZE - SHORTTERM_SHORTWINSIZE)/2.0);
#endif

//The time similarity defines
//The weight of the SHORTTERMRELMOTION similarity
#define SHORTTERMRELMT_SIM_WEIGHT  0.001
//The weight of the long term motion similarity
#define LONGTERMRELMT_SIM_WEIGHT   0.1
//The sum of all the weights above
#define TIMESIM_WEIGHTSUM (SHORTTERMRELMT_SIM_WEIGHT + LONGTERMRELMT_SIM_WEIGHT)

//The limits for spatial and temporal similarity based selections
//for each of them the probability that a case will go through
//with its (BestCaseSimwithEnv - CaseSimwithEnv) > XXX_2STD is about 0.05
//The distibution is normal
#define SPATIALSIM_2STD   0.05
#define TEMPORALSIM_2STD  0.5
//At the third selection stage an interval is computed with length based on gaussian dstn.
//If (MaxBestness-Bestness) = THIRDSEL_2STD then length is about 0.01
#define THIRDSEL_2STD   0.2

//In the third selection stage the weights for each of the dimensions of the selection
#define SELECT_SUCCESS_WEIGHT     0.8
#define SELECT_TEMPORALSIM_WEIGHT 0.1
#define SELECT_SPATIALSIM_WEIGHT  0.1

//weights for computation of overall similarity
#define OVERALLSIM_TEMPORALSIM_WEIGHT 0.1
#define OVERALLSIM_SPATIALSIM_WEIGHT  0.9

//thresholds for the NeedCreateNewCase decision procedure
#define CREATECASE_SUCCESS_HIGH  0.8
#define CREATECASE_SIM_HIGH      0.95
#define CREATECASE_SIM_LOW       0.85

//Adaptation function configuration
//weights for the interpolation with positive success cases
#define ADAPT_POS_NEWCASE_WEIGHT  0.10
#define ADAPT_POS_OLDCASE_WEIGHT  0.01
//weights for the subtraction of the negative success cases
#define ADAPT_NEG_NEWCASE_WEIGHT  0.05
#define ADAPT_NEG_OLDCASE_WEIGHT  0.005

//positive and negative success cases selection for adaptation
//thresholds for selection
#define POSNEGSUCCESS_SPATSIM_THRESH      0.9
#define POSNEGSUCCESS_TEMPORALSIM_THRESH  0.1
#define POSSUCCESS_THRESH                 0.5
#define NEGSUCCESS_THRESH                -0.5




//The decay in the Long Window average position computations
//for the long term relative motion
//The position will reach 0.9 of a new position in
//LONGTERM_LONGWINSIZE/2 cycles given the new pos
//differs from old two times
#define LONGTERM_LONGWINDECAY   pow((1-0.9)/(1-0.5),1.0/(LONGTERM_LONGWINSIZE/2.0))
//The decay in the Short Window average position computations
//for the long term relative motion
#define LONGTERM_SHORTWINDECAY    pow((1-0.9)/(1-0.5),1.0/(LONGTERM_SHORTWINSIZE/2.0))
//The normalization factor for the velocity based on the short term window
#if REAL_ROBOT == 1    //debug-TODO- fix, should be the same
#define LONGTERM_NORM ((LONGTERM_LONGWINSIZE - LONGTERM_SHORTWINSIZE)/4.0);
#else
#define LONGTERM_NORM ((LONGTERM_LONGWINSIZE - LONGTERM_SHORTWINSIZE)/2.0);
#endif



//the Names of features
#define GOALDISTFEATURE_NAME        "Goal_Distance"
#define REGDENSFEATURE_NAME         "Region_Density"
#define REGDISTFEATURE_NAME         "Region_Distance"
#define SHORTTERMRELMOTFEATURE_NAME "ShortTerm_Motion"
#define LONGTERMRELMOTFEATURE_NAME  "LongTerm_Motion"
#define CASETIMEFEATURE_NAME        "Case_Time"

//The IDS of features
enum FEATUREID  {GOALDIST_FEATURE=0,
                 REG_FEATURE, //includes all the Reg Densities and Reg distances
                 SHORTTERMRELMOT_FEATURE,
                 LONGTERMRELMOT_FEATURE,
                 CASETIME_FEATURE,
                 //should be the last one
                 FEATURE_SIZE
                };

//The feature index is not present
#define FEATURE_NOT_PRESENT -1

//The case selction decision tree
#if REAL_ROBOT==1
//The hangover time
#define CASETIME_THRESH         5  //cycles/10
//The long term motion threshold
#define LONGTERMMOT_THRESH      0.1
//The short term motion threshold
#define SHORTTERMMOT_THRESH     0.1
//The low threshold for the short term motion
#define SHORTTERMMOT_LOW_THRESH 0.05
#else
//The hangover time
#define CASETIME_THRESH         5  //cycles/10
//The long term motion threshold
#define LONGTERMMOT_THRESH      0.2
//The short term motion threshold
#define SHORTTERMMOT_THRESH     0.2
//The low threshold for the short term motion
#define SHORTTERMMOT_LOW_THRESH 0.05
#endif

//define a lower bound on noise when there is no shortterm movement
#define NOISE_SHORTTERM_LOWERBOUND 0.1


//case adaptation parameters
//the highest max limit on the multiplier for the noise
#define NOISE_HIGH_MPY_THRESH 10.0
//the mid max limit on the multiplier for the noise
#define NOISE_MID_MPY_THRESH   5.0
//the lowest max limit on the multiplier for the noise
#define NOISE_LOW_MPY_THRESH   2.0



//Misc



//FC Smoothing coefficients
#if REAL_ROBOT == 1
#define FC_DECAYTIME  2       //decay very fast - toward intraversible
#define FC_ATTACKTIME 10   // 15   //attack slower - toward fully traversable
#else
#define FC_DECAYTIME    1     //decay very fast - toward intraversible
#define FC_ATTACKTIME   1     //attack slower - toward fully traversable
#endif

//the FC will reach the new value +/- 10% in FC_DECAYTIME/FC_ATTACKTIME given
//that it is 2 times smaller/larger than the old value
#define FC_SMOOTH_DECAYCOEFF  pow((1-0.9)/(1-0.5),1.0/FC_DECAYTIME)
#define FC_SMOOTH_ATTACKCOEFF pow((1-0.9)/(1-0.5),1.0/FC_ATTACKTIME)


//the maximum distance to be considered while matching cases
#define D_TRESH 7   //TODO - adjust
//the minimum distance to be considered
#define D_MIN_THRESH 0.01
//weight vector defines
//Increasing it, increases weight of the front regions
//(this is by how much AT MOST the front region should be
//different for case X to be possibly still more similar
//than another case that is exact in the front region
//but is fully dissimlar in the rest of the regions)
#define WEIGHTDELTA_THRESH  0.05
//minimal importance to the backward regions(the value itself
//is not important but rather the value above)
#define WEIGHT_BACKWARDREGION 0.1

//Learning defines
//the length of the window in terms of cases to propogate back reward
#define LEARN_WINDOW        2     //TODO - was 3
#define LEARN_REWARD_INIT  -0.05  //TODO - was -0.005
#define LEARN_GOALDISTDELTA 0.5   //TODO - was 1.0
#define LEARN_POS_DELTA     0.05
#define LEARN_NEG_DELTA     0.05  //TODO - was 0.005
#define LEARN_GOALDIST_NEGTHRESH  0.1
#define LEARN_REWARD_MAX    0.15
#define LEARN_REWARD_MIN   -0.15  //TODO - was -0.05
#define LEARN_HEUR_DELTA    0.9   //TODO - was 0.95
#define LEARN_HEUR_SMOOTH         0.99
#define CASE_IMPROV_SMOOTH_COEFF  0.5

//after how many case improvements the reward starts increasing proportional to it
#define CASE_IMPROVEMENT_THRESH 4.0

//the magnitude of noise for adaptation
#define LEARN_DELTA_MAGNITUDE 0.5

//feature projection
#define GOAL_TRAVERSE_CHECK 0 //TODO - think about it
#define PRWR0    0.0008
#define PRWR1   -0.3305
#define PRWR2    0.2411
#define PRWR3   -0.0142
#define PRWSTMV -0.9124

#define PR_THRESH -0.8

//=============================Used by Cases structures============
#define TEMPLATE_CASE_ID 0
#define TEMPLATE_CASE_NAME "TEMPLATE_CASE"

struct structFeature {
  char strName[100];       //The name of the feature
  //all the parameters are normalized from 0 to 1. If some of them are not applicable to a
  //particular feature then they should be set up as 1.
  double fDegofMembership; //The degree of how much the feature is present
  double fValue;           //The value of the feature
  double fProb;            //The probability of the feature being present
  double fImportDegree;    //The Importance of the feature
  FEATUREID nID;           //The ID of the feature
  void WriteOut(FILE* fCaseFile) {
    fprintf(fCaseFile, "%s - Name\n", strName);
    fprintf(fCaseFile, "%f - Value\n", fValue);
    fprintf(fCaseFile, "%f - Prob\n", fProb);
    fprintf(fCaseFile, "%f - ImportDegree\n", fImportDegree);
    fprintf(fCaseFile, "%f - Membership Degree\n", fDegofMembership);
    fprintf(fCaseFile, "%d - Feature ID\n", nID);
  };
  void ReadIn(FILE* fCaseFile) {
    char strTemp[1000];
    fscanf(fCaseFile, "%s%[^\n]\n", strName, strTemp);
    fscanf(fCaseFile, "%lf%[^\n]\n", &fValue, strTemp);
    fscanf(fCaseFile, "%lf%[^\n]\n", &fProb, strTemp);
    fscanf(fCaseFile, "%lf%[^\n]\n", &fImportDegree, strTemp);
    fscanf(fCaseFile, "%lf%[^\n]\n", &fDegofMembership, strTemp);
    fscanf(fCaseFile, "%d%[^\n]\n", (int*)&nID, strTemp);
  };

};

struct HISTORY {
  int nTime;
  int nCount;
};

//union for the output element value type
union OUTVALUE{
  double fValue;
  bool bValue;
  int nValue;
};

//structure for learning data
struct LEARNING {
  double fHeuristic;    //the heuristic based on which reward is assigned
  double fReward;       //the scalar reward
  int    nCaseID;       //the ID of the case to reward
  double fHeurThresh;   //the threshold that decides between no and positive progress
  int nTimeApplied;     //Time that the case was applied
  bool bDelayedReinf;   //whether to use delayed reinforcement or not for the case
  double fEndHeuristic; //the heuristic value after the case was applied
  bool bApplied;        //whether the knowledge from the case was applied
}
;

//maximum allowed time to use one case without re-application
#define MAXAPPLYTIME  10 //TODO - was 5


//macro definitions
#define ERROR_EPS 0.0000001
#define IS_ZERO(x) (fabs(x) < ERROR_EPS)

#ifndef WINCOMPILER
#define __min(x,y)  (x < y ? x : y)
#define __max(x,y)  (x > y ? x : y)
#endif


#endif


/**********************************************************************
 * $Log: Data.h,v $
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
