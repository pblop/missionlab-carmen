/**********************************************************************
 **                                                                  **
 **                            LogManager.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: LogManager.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "LogManager.h"
#include "HServerTypes.h"
#include "hserver.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string LogManager::EMPTY_STRING_ = "";
const double LogManager::LOG_TIME_INTERVAL_SEC_ = 1.0;
const int LogManager::LOG_TIMER_THREAD_USLEEP_ = 1000;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t LogManager::nextLogTimeMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
LogManager::LogManager(void) : 
    logfilePrefix_(EMPTY_STRING_)
{
    // Create the prefix for the logfile.
    makeLogfilePrefix_();

    // Assign the first log time.
    shNextLogTime_ = getCurrentEpochTime() + LOG_TIME_INTERVAL_SEC_;

    // Start the check thread.
    pthread_create(&logTimerThread_, NULL, &startLogTimerThread_, (void*)this);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
LogManager::~LogManager(void)
{
    pthread_cancel(logTimerThread_);
    pthread_join(logTimerThread_, NULL );
}

//-----------------------------------------------------------------------
// This function starts the log timer thread.
//-----------------------------------------------------------------------
void* LogManager::startLogTimerThread_(void* logManagerInstance)
{
    ((LogManager *)logManagerInstance)->logTimerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to check for the next log time.
//-----------------------------------------------------------------------
void LogManager::logTimerLoop_(void)
{
    while (true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Update the log time if necessary.
        checkForLogTime_();

        // Sleep for a specified time.
        usleep(LOG_TIMER_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function updates the log time if necessary.
//-----------------------------------------------------------------------
void LogManager::checkForLogTime_(void)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&nextLogTimeMutex_);

    if (curTime >= shNextLogTime_)
    {
        shNextLogTime_ += LOG_TIME_INTERVAL_SEC_;
    }

    pthread_mutex_unlock(&nextLogTimeMutex_);
}

//-----------------------------------------------------------------------
// This function makes a logfile prefix
//-----------------------------------------------------------------------
void LogManager::makeLogfilePrefix_(void)
{
    HSDateString_t dateString;
    char buf[1024];

    dateString = getCurrentDateString();

    sprintf(
        buf,
        "HServer-Log-%s%s%s-%s:%s:%s",
        dateString.year.c_str(),
        dateString.month.c_str(),
        dateString.day.c_str(),
        dateString.hour.c_str(),
        dateString.min.c_str(),
        dateString.sec.c_str());

    logfilePrefix_ = buf;

    printfTextWindow(
        "Data logging enabled on %s/%s/%s - %s:%s:%s\n",
        dateString.year.c_str(),
        dateString.month.c_str(),
        dateString.day.c_str(),
        dateString.hour.c_str(),
        dateString.min.c_str(),
        dateString.sec.c_str());
}

//-----------------------------------------------------------------------
// This function makes a logfile prefix
//-----------------------------------------------------------------------
double LogManager::getNextLogTime(void)
{
    double nextLogTime;

    pthread_mutex_lock(&nextLogTimeMutex_);
    nextLogTime = shNextLogTime_;
    pthread_mutex_unlock(&nextLogTimeMutex_);

    return nextLogTime;
}

/**********************************************************************
# $Log: LogManager.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:41:07  endo
# New PoseCalc integrated.
#
#**********************************************************************/
