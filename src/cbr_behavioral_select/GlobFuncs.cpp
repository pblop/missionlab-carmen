/**********************************************************************
 **                                                                  **
 **                          GlobFuncs.cc                            **
 **                                                                  **
 **                                                                  **
 **  Copyright 1999, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************
**
** The file contains some of the global functions used across
** different classes
**
**
** created by: mlikhach 
***********************************************************************/

#include "Headers.h"




//creates a case of the specified type and returns a pointer to it
CCBRCase* CreateCaseofType(CASE_TYPE nCaseType) {
  CCBRCase* pCase;

  switch(nCaseType) {
  case MOVETO_CASE_TYPE:
    pCase = new CCBRMoveCase;
    break;
  default:
    fprintf(stderr, "ERROR: creating a case of invalid type\n");
    return NULL;
  };

  return pCase;
}
