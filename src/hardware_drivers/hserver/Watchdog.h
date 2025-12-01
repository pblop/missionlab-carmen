/**********************************************************************
 **                                                                  **
 **                            Watchdog.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: Watchdog.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <string>

#include "HServerTypes.h"

using std::vector;
using std::string;

class Watchdog {

protected:
    FILE *debugFile_;
    pthread_t checkThread_;
    string debugFilename_;
    double shUpdateTime_; // Shared
    double lastCheckedUpdateTime_;
    int numNoUpdate_;

    const HSDateString_t DATE_STRING_;
    const pthread_t MODULE_THREAD_;
    const string MODULE_NAME_;
    const int CHECK_INTERVAL_SEC_;
    const int PID_;

    static pthread_mutex_t updateTimeMutex_;

    static const int MAX_NONUPDATE_;

    void createDebugFilename_(void);
    void createDateString_(void);
    void checkLoop_(void);
    void checkModuleUpdate_(void);
    void printDebugMessage_(string msg);

    static void* startCheckThread_(void* watchdogInstance);
    static void* startAlertThread_(void* alertMessage);

public:
    Watchdog(
        string moduleName,
        int checkIntervalSec,
        int pid,
        pthread_t moduleThread);
    ~Watchdog(void);
    void notifyUpdate(void);
};

extern bool gWatchdogEnabled;

#endif

/**********************************************************************
# $Log: Watchdog.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:28  endo
# New PoseCalc integrated.
#
#**********************************************************************/
