/**********************************************************************
 **                                                                  **
 **                             Watchdog.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: Watchdog.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "hserver.h"
#include "Watchdog.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void printfTextWindow(const char* arg, ...);
extern void exit_hserver(void);

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const int Watchdog::MAX_NONUPDATE_ = 5;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t Watchdog::updateTimeMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
Watchdog::Watchdog(
    string moduleName,
    int checkIntervalSec,
    int pid,
    pthread_t moduleThread) :
    debugFile_(NULL),
    lastCheckedUpdateTime_(0),
    numNoUpdate_(0),
    DATE_STRING_(getCurrentDateString()),
    MODULE_THREAD_(moduleThread),
    MODULE_NAME_(moduleName),
    CHECK_INTERVAL_SEC_(checkIntervalSec),
    PID_(pid)
{
    // Create the output file name.
    createDebugFilename_();

    // Assign the current time for now.
    shUpdateTime_ = getCurrentEpochTime();

    // Start the check thread.
    pthread_create(&checkThread_, NULL, &startCheckThread_, (void*)this);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Watchdog::~Watchdog(void)
{
    pthread_cancel(checkThread_);
    pthread_join(checkThread_, NULL );
}

//-----------------------------------------------------------------------
// This function starts the check thread.
//-----------------------------------------------------------------------
void* Watchdog::startCheckThread_(void* watchdogInstance)
{
    ((Watchdog *)watchdogInstance)->checkLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function alerts a message in a separate thread, avoiding the
// main checkThread getting stuck at printfTextWindow().
//-----------------------------------------------------------------------
void* Watchdog::startAlertThread_(void* alertMessage)
{
    if (alertMessage != NULL)
    {
        printfTextWindow("%s", (char *)alertMessage);
    }

    return NULL;
}

//-----------------------------------------------------------------------
// This function constantly checks the status of the module.
//-----------------------------------------------------------------------
void Watchdog::checkLoop_(void)
{
    while (true)
    {
        // Sleep first.
        sleep(CHECK_INTERVAL_SEC_);
        
        // Make sure termination was not requested.
        pthread_testcancel();

        checkModuleUpdate_();
    }
}

//-----------------------------------------------------------------------
// This function checks to see if the module is still running.
//-----------------------------------------------------------------------
void Watchdog::checkModuleUpdate_(void)
{
    string bufString;
    double updateTime;
    char buf[1024];

    pthread_mutex_lock(&updateTimeMutex_);
    updateTime = shUpdateTime_;
    pthread_mutex_unlock(&updateTimeMutex_);

    if (updateTime == lastCheckedUpdateTime_)
    {
        numNoUpdate_++;

        sprintf(
            buf,
            "Watchdog: Warning. %s (pid %d [thread %d]) not updated for %d sec.\n",
            MODULE_NAME_.c_str(),
            PID_,
            (int)MODULE_THREAD_,
            (CHECK_INTERVAL_SEC_*numNoUpdate_));
        bufString = buf;
        printDebugMessage_(bufString);
    }
    else
    {
        lastCheckedUpdateTime_ = updateTime;
        numNoUpdate_ = 0;
    }
    
    if (numNoUpdate_ > MAX_NONUPDATE_)
    {
        sprintf(
            buf,
            "Watchdog: Aborting HServer. %s (pid %d [thread %d]) failed.\n",
            MODULE_NAME_.c_str(),
            PID_,
            (int)MODULE_THREAD_);
        bufString = buf;
        printDebugMessage_(bufString);
        exit_hserver();
    }
}

//-----------------------------------------------------------------------
// This function notifies Watchdog that the module is still running.
//-----------------------------------------------------------------------
void Watchdog::notifyUpdate(void)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&updateTimeMutex_);
    shUpdateTime_ = curTime;
    pthread_mutex_unlock(&updateTimeMutex_);
}

//-----------------------------------------------------------------------
// This function outputs the message to both the HServer screen and the
// output file.
//-----------------------------------------------------------------------
void Watchdog::printDebugMessage_(string msg)
{
    pthread_t alertThread;
    char errorMsg[1024];

    if (debugFile_ == NULL)
    {
        debugFile_ = fopen(debugFilename_.c_str(), "a");

        if (debugFile_ == NULL)
        {
            sprintf(
                errorMsg,
                "Watchdog: Warning. %s could not be opened.\n",
                debugFilename_.c_str());

            // Alert a message in a separate thread, avoiding the main checkThread
            // getting stuck at printfTextWindow().
            pthread_create(&alertThread, NULL, &startAlertThread_, (void*)errorMsg);
            return;
        }

        fprintf(debugFile_, "%s/%s/%s - %s:%s:%s\n",
                DATE_STRING_.year.c_str(),
                DATE_STRING_.month.c_str(),
                DATE_STRING_.day.c_str(),
                DATE_STRING_.hour.c_str(),
                DATE_STRING_.min.c_str(),
                DATE_STRING_.sec.c_str());
    }

    fprintf(debugFile_, "%s", msg.c_str());
    fflush(debugFile_);

    // Alert a message in a separate thread, avoiding the main checkThread
    // getting stuck at printfTextWindow().
    pthread_create(&alertThread, NULL, &startAlertThread_, (void*)(msg.c_str()));
}

//-----------------------------------------------------------------------
// This function creates the output file name.
//-----------------------------------------------------------------------
void Watchdog::createDebugFilename_(void)
{
    char buf[1024];

    debugFilename_ = "HServer-Watchdog:";
    debugFilename_ += MODULE_NAME_;
    sprintf(buf, ":%d", (int)MODULE_THREAD_);
    debugFilename_ += buf;
    debugFilename_ += ".log";
}

/**********************************************************************
# $Log: Watchdog.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:41:08  endo
# New PoseCalc integrated.
#
#**********************************************************************/
