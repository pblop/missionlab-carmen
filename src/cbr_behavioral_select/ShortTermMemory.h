/**********************************************************************
 **                                                                  **
 **                      ShortTermMemory.h                           **
 **                                                                  **
 ** Contains the definition of the CBR Cases List Class              **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: ShortTermMemory.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#ifndef _CBR_SHORTTERMMEMORY_
#define _CBR_SHORTTERMMEMORY_

struct structCBRCaseNode {
  CCBRCase *pCase;
  structCBRCaseNode* pNextCaseNode;
};

class CCBRShortTermMemory {

  //data
private:
  structCBRCaseNode* pHeadCaseNode;
  structCBRCaseNode* pTailCaseNode;
  int nNumCases;
  bool bModifiable;


  //constructors & destructors
public:
  CCBRShortTermMemory() {
    pHeadCaseNode = NULL;
    pTailCaseNode = NULL;
    nNumCases = 0;
    bModifiable = true;
  };

  ~CCBRShortTermMemory();


  //functions and operators
public:
  //initialize the local library
  bool Load(FILE* fCaseFile);
  bool Save(FILE* fCaseFile);

  //get the number of cases in the short term memory
  int GetNumCases() {
    return nNumCases;
  };

  //gets a case indexed by the nCaseIndex
  CCBRCase* GetCase(int nCaseIndex);

  //gets a case with the specified ID
  //returns NULL if there is no such case
  CCBRCase* GetCasebyID(int nCaseID);

  //adds a case to the short teerm memory
  bool AddCase(CCBRCase* pNewCase);

  //returns an ID that can be used for a new case
  //returns -1 if there is no such
  int GetFreeCaseID();

  //returns true if library file can be modified
  //(as a result of learning)
  bool IsModifiable() {
    return bModifiable;
  };

  //utility functions
private:
  //Read in the library of cases
  bool ReadCases(FILE* fCaseFile);

  //Save the library of cases
  bool SaveCases(FILE* fCaseFile);

  //Add a case to the end of the list
  bool AddCasetoTail(structCBRCaseNode* pCaseNode);

  //Delete a case from the head of the list
  bool DeleteCasefromHead();

};


#endif


/**********************************************************************
 * $Log: ShortTermMemory.h,v $
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
