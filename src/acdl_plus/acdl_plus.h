/**********************************************************************
 **                                                                  **
 **                             acdl_plus.h                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for ACDLPlus class.                       **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef ACDL_PLUS_H
#define ACDL_PLUS_H

/* $Id: acdl_plus.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <string>

using std::string;

class ACDLPlus {

protected:
    static const string EMPTY_STRING_;
    static const string ACDL2CDL_DEFAULT_ROBOT_NAME_;

    static const string ACDL_STRING_MOVEMENT_;
    static const string ACDL_STRING_VEHICLE_;
    static const string ACDL_STRING_BIND_ARCH_;
    static const string ACDL_STRING_AGENT_NAME_;
    static const string ACDL_STRING_TASK_INDEX_;
    static const string ACDL_STRING_TASK_DESC_;
    static const string ACDL_STRING_TRIGGERING_TASK_INDEX_;
    static const string ACDL_STRING_CMD_;
    static const string ACDL_STRING_PARM_;
    static const string ACDL_STRING_PARM_NAME_;
    static const string ACDL_STRING_PARM_VALUE_;
    static const string ACDL_STRING_IF_;
    static const string ACDL_STRING_GOTO_;
    static const string ACDL_STRING_INSTANCE_;
    static const string ACDL_STRING_INSTANCE_INDEX_;
    static const string ACDL_STRING_INSTANCE_NAME_;
    static const string ACDL_STRING_START_;
    static const string ACDL_STRING_END_;
    static const string ACDL_STRING_MAX_VELOCITY_;
    static const string ACDL_STRING_BASE_VELOCITY_;
    static const string ACDL_DEFAULT_CONFIG_FILE_;
    static const int ACDL_TASK_INDEX_BASE_;


    static string readText_(string inputFileName);
    static bool dumpText_(string textString, string outputFileName);

public:
    ACDLPlus(void);
    ~ACDLPlus(void);

    static string acdlp2cimxmlText(string acdlpText);
    static string acdlp2cmdlText(string acdlpText);
    static string acdlp2cmdliText(string acdlpText);
    static string acdlp2cdlText(string acdlpText);
    static string cimxml2acdlpText(string cimxmlText);
    static string cmdl2acdlpText(string cmdlText);
    static string cmdli2acdlpText(string cmdliText);
    static string cdl2acdlpText(string cdlText);
    static bool acdlp2cimxml(string inputFileName, string outputFileName);
    static bool acdlp2cmdl(string inputFileName, string outputFileName);
    static bool acdlp2cmdli(string inputFileName, string outputFileName);
    static bool acdlp2cdl(string inputFileName, string outputFileName);
    static bool cimxml2acdlp(string inputFileName, string outputFileName);
    static bool cmdl2acdlp(string inputFileName, string outputFileName);
    static bool cmdli2acdlp(string inputFileName, string outputFileName);
    static bool cdl2acdlp(string inputFileName, string outputFileName);
};

#endif
/**********************************************************************
 * $Log: acdl_plus.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2006/09/04 00:35:15  nadeem
 * *** empty log message ***
 *
 * Revision 1.2  2006/08/02 21:38:05  endo
 * acdlp2cdl() implemented.
 *
 * Revision 1.1  2006/07/26 18:05:46  endo
 * ACDLPlus class added.
 *
 **********************************************************************/
