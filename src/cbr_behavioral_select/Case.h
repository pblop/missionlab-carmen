/**********************************************************************
 **                                                                  **
 **                            Case.h                                **
 **                                                                  **
 ** Contains the definition of the CBR Case Class                    **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Case.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBR_CASE_
#define _CBR_CASE_

#include <string.h>	//Old compilation error: strcpy not defined

//the root case in the hierarchy of cases
class CCBRCase {

  //data
protected:
  //The Case ID
  int nCaseID;
  //The Case Name
  char strCaseName[100];
  //The Case Type
  CASE_TYPE nCaseType;


public:
  //the vector of features that identify the current case
  vector<structFeature> FeaturesVector;
  //the vector of indices of the environment features above
  int FeaturesIndices[FEATURE_SIZE];
  //the vector of the output elements of the case
  vector<CCBROutElement> OutputVector;
  //the current adaptation direction vector for output parameters
  vector<CCBROutElement> AdaptV;
  //the result of the case application
  //ranges between -1 and 1,
  //where -1 - negative effect, 0 - neutral(unknown), 1 - positive
  double fCaseSuccess;
  double fHeurThresh;	//TODO
  double fImprov;

  //constructors & destructors
public:
  CCBRCase();
  virtual ~CCBRCase();

  //functions and operators
public:
  //Read in the case from a file
  virtual bool ReadIn(FILE* fCaseFile);
  //write out the variables of the case
  virtual bool WriteOut(FILE* fCaseFile);
  //apply the case toward the robot
  virtual bool Apply()=0;
  //returns the ID of the Case
  int GetCaseID() {
    return nCaseID;
  };
  //returns the name of the case
  char *GetCaseName() {
    return strCaseName;
  };
  //sets the ID of the Case
  void SetCaseID(int nNewCaseID) {
    nCaseID = nNewCaseID;
  };
  //sets the name of the case
  void SetCaseName(char *strNewCaseName) {
    strcpy(strCaseName,strNewCaseName);
  };
  //sets the success of the case
  void SetCaseSuccess(double fNewCaseSuccess);
  //gets the success of the case
  double GetCaseSuccess() {
    return fCaseSuccess;
  };
  //sets the Case type
  void SetCaseType(CASE_TYPE nNewCaseType) {
    nCaseType = nNewCaseType;
  };
  //gets the Case type
  CASE_TYPE GetCaseType() {
    return nCaseType;
  };
  //get the address of the output value
  OUTVALUE* GetOutValueAddress(char strOutValID[]);




  //utility functions
private:



};


//MoveCase (Leaf case)
class CCBRMoveCase : public CCBRCase {
  //data
private:


public:

  //constructors & destructors
public:
CCBRMoveCase() : CCBRCase () {}
  ;


  //functions and operators
public:
  //Read in the case from a file
  bool ReadIn(FILE* fCaseFile);
  //write out the variables of the case
  bool WriteOut(FILE* fCaseFile);
  //Apply the case
  bool Apply();

  //utility functions
private:


};


#endif


/**********************************************************************
 * $Log: Case.h,v $
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
