/**********************************************************************
 **                                                                  **
 **                              module.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Generic HServer module                                          **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: module.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef MODULE_H
#define MODULE_H

#include "pthread.h"
#include <sys/time.h>
#include <string>
#include <vector>

#include "statusbar.h"
#include "Watchdog.h"

using std::string;
using std::vector;

class Module {

protected:
    Module **self_;
    StatusbarSpinner *statusbarSpinner_;
    Watchdog *watchdog_;
    FILE *logfile_;
    vector<double> freqBuffer_;
    string logfileName_;
    char *statusStr;
    char *rstr;
    double shFrequency_; // Shared
    double lastReadingTime_;
    double nextLogTime_;
    double lastLoggedTime_;
    int statusLine;
    int statusLineLen;
    int report_level;
    int reportLevelLine;
    int freqIndex_;
    bool freqBufferFilled_;

    static pthread_mutex_t frequencyMutex_;

    const string NAME_;

    static const string EMPTY_STRING_;
    static const int NUM_FREQUENCY_SAMPLES_;

    void compFrequency_(void);
    void getFrequency_(double &frequency);
    void manageLog_(pthread_t threadID);

    virtual void updateStatusBar_(void) = 0;
    virtual void logData_(double logTime) {};


public:
    Module(Module **self, string name);
    string getName(void);
    int reportLevel() {return report_level;};
    void setReportLevel(int level);

    virtual ~Module();
};

inline string Module::getName(void)
{
    return NAME_;
}

#endif

/**********************************************************************
# $Log: module.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.2  2000/09/19 03:45:31  endo
# RCS log added.
#
#
#**********************************************************************/
