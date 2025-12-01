/**********************************************************************
 **                                                                  **
 **                            LogManager.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: LogManager.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <string>

using std::string;

class LogManager {

protected:
    string logfilePrefix_;
    pthread_t logTimerThread_;
    double shNextLogTime_;

    static pthread_mutex_t nextLogTimeMutex_;

    static const string EMPTY_STRING_;
    static const double LOG_TIME_INTERVAL_SEC_;
    static const int LOG_TIMER_THREAD_USLEEP_;

    void makeLogfilePrefix_(void);
    void logTimerLoop_(void);
    void checkForLogTime_(void);

    static void* startLogTimerThread_(void* logManagerInstance);

public:
    LogManager(void);
    ~LogManager(void);
    string getLogfilePrefix(void);
    double getNextLogTime(void);
};

inline string LogManager::getLogfilePrefix(void)
{
    return logfilePrefix_;
}

extern LogManager *gLogManager;

#endif

/**********************************************************************
# $Log: LogManager.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
#**********************************************************************/
