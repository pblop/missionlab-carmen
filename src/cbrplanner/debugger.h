/**********************************************************************
 **                                                                  **
 **                              debugger.h                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef DEBUGGER_H
#define DEBUGGER_H

/* $Id: debugger.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <stdio.h>
#include <string>

#include "cbrplanner_domain.h"

using std::string;

//-----------------------------------------------------------------------
// Debugger class
//-----------------------------------------------------------------------
class Debugger {

public:
    typedef enum OutputType_t {
	DEBUG_FILE,
	DEBUG_STDOUT
    };

protected:
    Debugger **self_;
    string debugFilename_;
    FILE *debugFile_;
    FILE *out_;
    int outputType_;
    bool debug_;

    static const string DEBUG_FILENAME_EXTENSION_;
    static const string EMPTY_STRING_;
    
    string createDebugFilenamePrefix_(void);
    void setDefaultDebugFilename_(void);
    void openDebugFile_(void);

public:
    Debugger(void);
    Debugger(Debugger **self);
    ~Debugger(void);
    bool debugEnabled(void);
    void printfDebug(const char *format, ...);
    void printfDebug(int outputType, const char *format, ...);
    void printfLine(void);
    void setOutputType(int outputType);
    void enableDebug(void);
    void enableDebug(int outputType);
    void disableDebug(void);
    void dumpPlanSequence(CBRPlanner_PlanSequence_t planSequence);
};

inline bool Debugger::debugEnabled(void)
{
    return debug_;
}

inline void Debugger::enableDebug(void)
{
    debug_ = true;
}

inline void Debugger::disableDebug(void)
{
    debug_ = false;
}

extern Debugger *gDebugger;

#endif
/**********************************************************************
 * $Log: debugger.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.2  2005/02/07 20:08:18  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
