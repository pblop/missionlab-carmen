/**********************************************************************
 **                                                                  **
 **                         OutElemente.h                            **
 **                                                                  **
 ** Contains the definition of an element of the Output              **
 ** Vector produced by the CBR                                       **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: OutElement.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBR_OUTELEMENT_
#define _CBR_OUTELEMENT_

//the root
class CCBROutElement {
  //data
public:
  //String Identifier of the element
  char strStringId[100];
  //value of the element
  OUTVALUE Value;
  //the type of the output value
  enum OUTVALUE_TYPE nValueType;
  //whether it is adaptable or not
  bool bAdapt;

  //constructors and desctructors
public:
  CCBROutElement() {
    strcpy(strStringId, "EMPTY");
    bAdapt = false;
  };

  //functions and operators
public:

  //apply the output to the robot
  void Apply() {
    char strTemp[100];
    switch(nValueType) {
    case FLOAT_VALUE_TYPE:
      ::Apply(strStringId, Float2Ascii(Value.fValue, 4, strTemp));
      break;
    case INT_VALUE_TYPE:
      sprintf(strTemp, "%d", Value.nValue);
      ::Apply(strStringId, strTemp);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown Action Type\n");
      break;
    };
  };

  void WriteOut(FILE* fCaseFile) {
    fprintf(fCaseFile, "%s - StringID\n", strStringId);
    fprintf(fCaseFile, "%d - Value Type\n", nValueType);
    switch(nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fCaseFile, "%f - Value\n", Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fCaseFile, "%d - Value\n", Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown Action Type\n");
      break;
    };
    fprintf(fCaseFile, "%d - Adapt\n", bAdapt?1:0);
  }

  void ReadIn(FILE* fCaseFile) {
    char strTemp[1000];
    int nTemp;
    fscanf(fCaseFile, "%s%[^\n]\n", strStringId, strTemp);
    fscanf(fCaseFile, "%d%[^\n]\n", (int*)&nValueType, strTemp);
    switch(nValueType) {
    case FLOAT_VALUE_TYPE:
      fscanf(fCaseFile, "%lf%[^\n]\n", &Value.fValue, strTemp);
      break;
    case INT_VALUE_TYPE:
      fscanf(fCaseFile, "%d%[^\n]\n", &Value.nValue, strTemp);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown Action Type\n");
      break;
    };
    fscanf(fCaseFile, "%d%[^\n]\n", &nTemp, strTemp);
    bAdapt = (nTemp == 1);
  }

  void WriteValueOut(FILE* fCaseFile) {
    switch(nValueType) {
    case FLOAT_VALUE_TYPE:
      fprintf(fCaseFile, "%f - Value\n", Value.fValue);
      break;
    case INT_VALUE_TYPE:
      fprintf(fCaseFile, "%d - Value\n", Value.nValue);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown Action Type\n");
      break;
    };
  }

  void ReadValueIn(FILE* fCaseFile) {
    char strTemp[1000];
    switch(nValueType) {
    case FLOAT_VALUE_TYPE:
      fscanf(fCaseFile, "%lf%[^\n]\n", &Value.fValue, strTemp);
      break;
    case INT_VALUE_TYPE:
      fscanf(fCaseFile, "%d%[^\n]\n", &Value.nValue, strTemp);
      break;
    default:
      fprintf(stderr, "ERROR: Unknown Action Type\n");
      break;
    };
  }




};


#endif


/**********************************************************************
 * $Log: OutElement.h,v $
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
