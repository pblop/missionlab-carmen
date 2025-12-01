/**********************************************************************
 **                                                                  **
 **                       ShortTermMemory.cpp                        **
 **                                                                  **
 ** Contains the implementation of the CBR Cases List Class          **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: ShortTermMemory.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include "Headers.h"

//===============constructors & destructors========================
CCBRShortTermMemory::~CCBRShortTermMemory() {
  if(nNumCases == 0)
    return;
  else {
    while(nNumCases > 0) {
      if(!DeleteCasefromHead()) {
        fprintf(stderr, "ERROR: Failed to destruct local library\n");
        exit(1);
      }
    }
  }
  //printf("Hi\n");
}



//==============functions and operations============================
//Load the Local Library

bool CCBRShortTermMemory::Load(FILE* fCaseFile) {
  //Read in the cases
  //at this point we are just reading in all the cases
  //at a later point we will read in only part of them in
  //order to keep a local library in memory
  if(ReadCases(fCaseFile) != true) {
    return false;
  }



  return true;
}

//Save the Local Library
bool CCBRShortTermMemory::Save(FILE* fCaseFile) {

  //Save the cases
  if(SaveCases(fCaseFile) != true) {
    return false;
  }



  return true;
}


//Read in the local library of cases
bool CCBRShortTermMemory::ReadCases(FILE* fCaseFile) {
  CCBRCase* pCase;
  int nNumofFileCases;
  char strTemp[1000];
  int nTemp;

  //read in the number of cases
  if(fscanf(fCaseFile, "%d%[^\n]\n", &nNumofFileCases, strTemp) == EOF) {
    return false;
  }

  //read in whether the library is modifiable
  if(fscanf(fCaseFile, "%d%[^\n]\n", &nTemp, strTemp) == EOF) {
    return false;
  }
  bModifiable = (nTemp == 1);

  //set the Number of Cases to 0 initially
  nNumCases = 0;

  for(int i = 0; i<nNumofFileCases; i++) {
    //read the case type
    CASE_TYPE nCaseType;
    fpos_t fPos;
    //get the current position in the file
    if(fgetpos(fCaseFile, &fPos) != 0) {
      fprintf(stderr, "ERROR: Could not handle case file\n");
      return false;
    }
    //read in the case type
    fscanf(fCaseFile, "%d", (int*)&nCaseType);
    //return the file position back
    if(fsetpos(fCaseFile, &fPos) != 0) {
      fprintf(stderr, "ERROR: Could not handle case file\n");
      return false;
    }

    //create new case of the read type
    pCase = CreateCaseofType(nCaseType);

    //read in the case
    if( pCase->ReadIn(fCaseFile) != true)
      return false;

    //add the case to the local library
    if(AddCase(pCase) != true)
      return false;

    delete pCase;
  }

#ifdef VERBOSE
  printf("ShortTerm Memory Size: %d cases\n", nNumCases);
#endif
  return true;
}

//save the local library of cases
bool CCBRShortTermMemory::SaveCases(FILE* fCaseFile) {
  CCBRCase* pCase;

  //rewind the file
  rewind(fCaseFile);

  fprintf(fCaseFile, "%d\n", nNumCases);
  fprintf(fCaseFile, "%d - modifiable library\n", bModifiable?1:0);
  for(int i = 0; i<nNumCases; i++) {
    pCase = GetCase(i);
    pCase->WriteOut(fCaseFile);
    fprintf(fCaseFile, "\n");
  }


  return true;
}



//Add a case to the end of the list
bool CCBRShortTermMemory::AddCasetoTail(structCBRCaseNode* pCaseNode) {
  //empty list
  if(pTailCaseNode == NULL) {
    pTailCaseNode = pCaseNode;
    pHeadCaseNode = pCaseNode;
  } else {
    pTailCaseNode->pNextCaseNode = pCaseNode;
    pTailCaseNode = pCaseNode;
  }

  //next node pointer
  pCaseNode->pNextCaseNode = NULL;

  //increment counter
  nNumCases++;

  return true;
}

//Add a case
bool CCBRShortTermMemory::AddCase(CCBRCase* pNewCase) {
  CCBRCase* pCase;

  //create the case
  pCase = CreateCaseofType(pNewCase->GetCaseType());
  *pCase = *pNewCase;

  structCBRCaseNode* pCaseNode;

  //create new CaseNode
  pCaseNode = new structCBRCaseNode;
  pCaseNode->pCase = pCase;

  //add the case to the local library
  if(AddCasetoTail(pCaseNode) != true)
    return false;

  return true;
}


//Delete a case from the head of the list
bool CCBRShortTermMemory::DeleteCasefromHead() {
  structCBRCaseNode* pCaseNode;

  //empty list
  if(pHeadCaseNode == NULL)
    return false;

  //get the case to delete
  pCaseNode = pHeadCaseNode;

  //setup the head pointer
  pHeadCaseNode = pCaseNode->pNextCaseNode;

  //if this was the last case
  if(pTailCaseNode == pCaseNode)
    pTailCaseNode = NULL;

  //delete the node
  delete pCaseNode->pCase;
  pCaseNode->pCase = NULL;
  delete pCaseNode;
  pCaseNode = NULL;

  //decrement counter
  nNumCases--;

  return true;
}


//Get a case by its index
CCBRCase* CCBRShortTermMemory::GetCase(int nCaseIndex) {
  structCBRCaseNode* pCaseNode = pHeadCaseNode;

  if(nCaseIndex >= nNumCases) {
    fprintf(stderr, "ERROR:invalid library index\n");
    exit(1);
  }


  for(int i = 0; i < nCaseIndex; i++)
    pCaseNode = pCaseNode->pNextCaseNode;


  return pCaseNode->pCase;


}


//returns an ID that can be used for a new case
//returns -1 if there is no such
int CCBRShortTermMemory::GetFreeCaseID() {
  int nRetID = 0;
  structCBRCaseNode* pCaseNode = pHeadCaseNode;

  for(int i = 0; i < nNumCases; i++) {
    //keep the return ID one more than the maximum ID in the library
    if(nRetID <= pCaseNode->pCase->GetCaseID())
      nRetID = pCaseNode->pCase->GetCaseID()+1;

    //make sure that the ID does not exceed the allowed maximu
    if(nRetID > MAX_ALLOWED_CASEID)
      return -1;

    //go the next case
    pCaseNode = pCaseNode->pNextCaseNode;
  }

  return nRetID;
}


//gets a case with the specified ID
//returns NULL if there is no such case
CCBRCase* CCBRShortTermMemory::GetCasebyID(int nCaseID) {
  structCBRCaseNode* pCaseNode = pHeadCaseNode;
  CCBRCase* pRetCase = NULL;

  for(int i = 0; i < nNumCases; i++) {
    if(nCaseID == pCaseNode->pCase->GetCaseID()) {
      pRetCase = pCaseNode->pCase;
      break;
    }

    //go the next case
    pCaseNode = pCaseNode->pNextCaseNode;
  }

  return pRetCase;
}


/**********************************************************************
 * $Log: ShortTermMemory.cpp,v $
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
