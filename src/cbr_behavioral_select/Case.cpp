/**********************************************************************
 **                                                                  **
 **                            Case.cpp                              **
 **                                                                  **
 ** Contains the implemetation of the CBR Case Class                 **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Case.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include "Headers.h"

//==============constructors and destructors=======================

CCBRCase::CCBRCase() {
  nCaseID = TEMPLATE_CASE_ID;
  strcpy(strCaseName, TEMPLATE_CASE_NAME);
  nCaseType = NO_CASE_TYPE;
  fHeurThresh = 0;
  fImprov = 0;
  fCaseSuccess = 0;
}

CCBRCase::~CCBRCase() {}





//==============functions and operators============================
//Write out the case

bool CCBRCase::WriteOut(FILE* fCaseFile) {
  int i = 0;
  //write the case type
  //(the type has to be first since it is used by ShortTerm Memory)
  fprintf(fCaseFile, "%d - CaseType\n", nCaseType);
  //write the case number
  fprintf(fCaseFile, "%d - Case ID\n", nCaseID);
  //write the case name
  fprintf(fCaseFile, "%s - CaseName\n", strCaseName);

  //write out the Features Vector
  fprintf(fCaseFile, "%d - Features Vector Size\n", (int)FeaturesVector.size());
  for(i = 0; i < (int)FeaturesVector.size(); i++)
    FeaturesVector[i].WriteOut(fCaseFile);

  //write out the OutputVector
  fprintf(fCaseFile, "%d - Output Vector Size\n", OutputVector.size());
  for(i = 0; i < (int)OutputVector.size(); i++) {
    OutputVector[i].WriteOut(fCaseFile);
    //write out the value of the corresponding adapt element
    fprintf(fCaseFile, "Adaptation:\n");
    if(AdaptV.size() == OutputVector.size()) {
      //check that the adaptation vector is valid
      AdaptV[i].WriteValueOut(fCaseFile);
    } else {
      //the adaptation vector is invalid - mostlikely
      //the case was never adapted. Then make the adaptation
      //direction as 0
      CCBROutElement TempElem = OutputVector[i];
      switch(TempElem.nValueType) {
      case FLOAT_VALUE_TYPE:
        TempElem.Value.fValue = 0;
        break;
      case INT_VALUE_TYPE:
        TempElem.Value.nValue = 0;
        break;
      default:
        fprintf(stderr, "ERROR: Unknown Value Type\n");
        break;
      };
      TempElem.WriteValueOut(fCaseFile);
    }


  }

  //write the case success
  fprintf(fCaseFile, "%f - CaseSuccess\n", fCaseSuccess);

  //write the heuristics threshold
  fprintf(fCaseFile, "%f - Heuristics Threshold\n", fHeurThresh);

  //write the improvement
  fprintf(fCaseFile, "%f - Improvement\n", fImprov);

  return true;

}

//read in the case
bool CCBRCase::ReadIn(FILE* fCaseFile) {
  char strTemp[1000];
  int nTemp, i=0;
  structFeature Feature;
  CCBROutElement OutElement;

  //read the case type
  fscanf(fCaseFile, "%d%[^\n]\n", (int*)&nCaseType, strTemp);
  //read the case number
  fscanf(fCaseFile, "%d%[^\n]\n", &nCaseID, strTemp);
  //read the case name
  fscanf(fCaseFile, "%s%[^\n]\n", strCaseName, strTemp);

  //read in the Features Vector
  fscanf(fCaseFile, "%d%[^\n]\n", &nTemp, strTemp);
  //initialize the feature indices array
  for(i = 0; i < FEATURE_SIZE; i++)
    FeaturesIndices[i] = FEATURE_NOT_PRESENT;

  //nTemp is the size of the features vector
  for(i = 0; i < nTemp; i++) {
    Feature.ReadIn(fCaseFile);
    FeaturesVector.push_back(Feature);
    //set the index of the feature in the vector
    if(FeaturesIndices[Feature.nID] == FEATURE_NOT_PRESENT)
      FeaturesIndices[Feature.nID] = FeaturesVector.size()-1;
  }

  //read in the OutputVector
  fscanf(fCaseFile, "%d%[^\n]\n", &nTemp, strTemp);
  for(i = 0; i < nTemp; i++) {
    OutElement.ReadIn(fCaseFile);
    OutputVector.push_back(OutElement);
    //read in the adaptation element
    fscanf(fCaseFile, "%[^\n]\n", strTemp);
    OutElement.ReadValueIn(fCaseFile);
    AdaptV.push_back(OutElement);
  }

  //read the case success
  fscanf(fCaseFile, "%lf%[^\n]\n", &fCaseSuccess, strTemp);

  //read the case success
  fscanf(fCaseFile, "%lf%[^\n]\n", &fHeurThresh, strTemp);

  //read the case improvement
  fscanf(fCaseFile, "%lf%[^\n]\n", &fImprov, strTemp);

  return true;
}



//sets the success of the case
void CCBRCase::SetCaseSuccess(double fNewCaseSuccess) {
  fCaseSuccess = fNewCaseSuccess;
  //limit the case success to its range
  fCaseSuccess = __min(POSITIVE_CASE_SUCCESS, fCaseSuccess);
  fCaseSuccess = __max(NEGATIVE_CASE_SUCCESS, fCaseSuccess);
}



//Read in the case
bool CCBRMoveCase::ReadIn(FILE* fCaseFile) {

  //read in all the variables from the file
  if(!CCBRCase::ReadIn(fCaseFile))
    return false;

  return true;

}


//Write out the case
bool CCBRMoveCase::WriteOut(FILE* fCaseFile) {

  //write all the variables into the file
  if(CCBRCase::WriteOut(fCaseFile) == false)
    return false;

  return true;

}

//Apply the case
bool CCBRMoveCase::Apply() {
  //iterate through all the elements of the output vector
  //and apply each one of them
  for(int i = 0; i < (int)OutputVector.size(); i++) {
    OutputVector[i].Apply();
  }

  return true;

}


//returns the address of the output value specified by the string id.
OUTVALUE* CCBRCase::GetOutValueAddress(char strOutValID[]) {
  for(unsigned int i = 0; i < OutputVector.size(); i++) {
    if(!strcmp(strOutValID, OutputVector[i].strStringId))
      return &(OutputVector[i].Value);
  }

  fprintf(stderr, "ERROR: Could not find the outvalue specified by %s\n",
          strOutValID);

  return NULL;
}



/**********************************************************************
 * $Log: Case.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:25  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/05 23:36:22  kaess
 * Initial revision
 *
 **********************************************************************/
