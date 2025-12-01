/**********************************************************************
 **                                                                  **
 **                              module.c                            **
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

/* $Id: module.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include <numeric>
#include <string>

#include "module.h"
#include "statusbar.h"
#include "report_level.h"
#include "hserver.h"
#include "LogManager.h"

using std::string;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string Module::EMPTY_STRING_ = "";
const int Module::NUM_FREQUENCY_SAMPLES_ = 100;

//-----------------------------------------------------------------------
// Mutexes
//-----------------------------------------------------------------------
pthread_mutex_t Module::frequencyMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
Module::Module(Module **a, string name) :
    self_(a),
    statusbarSpinner_(new StatusbarSpinner()),
    watchdog_(NULL),
    logfile_(NULL),
    logfileName_(EMPTY_STRING_),
    shFrequency_(0),
    lastReadingTime_(0),
    nextLogTime_(0),
    lastLoggedTime_(0),
    freqIndex_(0),
    freqBufferFilled_(false),
    NAME_(name)
{
    if ( a != NULL )
    {
        *a = this;
    }

    statusLineLen = 500;
    statusStr = new char[statusLineLen];
    statusStr[0] = '\0';
    statusLine = statusbar->add(statusStr);
    reportLevelLine = reportlevel->add((char *)(name.c_str()),&report_level);
    rstr = new char[200];
    report_level = HS_REP_LEV_NORMAL;
    freqBuffer_.resize(NUM_FREQUENCY_SAMPLES_);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Module::~Module(void)
{
    if (watchdog_ != NULL)
    {
        delete watchdog_;
        watchdog_ = NULL;
    }


    if (self_ != NULL)
    {
        *self_ = NULL;
    }

    statusbar->remove(statusLine);
    reportlevel->remove(reportLevelLine);

    delete [] statusStr;
    delete [] rstr;

    if (statusbarSpinner_ != NULL)
    {
        delete statusbarSpinner_;
        statusbarSpinner_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function sets the report level to be the specified value.
//-----------------------------------------------------------------------
void Module::setReportLevel(int level)
{
    report_level = level;
}

//-----------------------------------------------------------------------
// This function computes the frequency of a loop. It has to be called
// in the loop once per cycle.
//-----------------------------------------------------------------------
void Module::compFrequency_(void)
{
    double frequency, aveFrequency;
    double absTime;
    double sum;

    absTime = getCurrentEpochTime();

    if (lastReadingTime_ == 0)
    {
        lastReadingTime_ = absTime;
        return;
    }
    else
    {
        frequency = 1.0/(absTime - lastReadingTime_);
    }

    freqBuffer_[freqIndex_] = frequency;
    freqIndex_++;

    if (freqIndex_ >= NUM_FREQUENCY_SAMPLES_)
    {
        freqBufferFilled_ = true;
        freqIndex_ = 0;
    }

    if (freqBufferFilled_)
    {
        sum = accumulate(freqBuffer_.begin(), freqBuffer_.end(), 0.0);
        aveFrequency = sum/(double)NUM_FREQUENCY_SAMPLES_;
    }
    else
    {
        sum = accumulate(
            freqBuffer_.begin(),
            (freqBuffer_.begin()) + (freqIndex_-1),
            0.0);
        aveFrequency = sum/(double)freqIndex_;
    }

    lastReadingTime_ = absTime;

    pthread_mutex_lock(&frequencyMutex_);
    shFrequency_ = aveFrequency;
    pthread_mutex_unlock(&frequencyMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the frequency value.
//-----------------------------------------------------------------------
void Module::getFrequency_(double &frequency)
{
    pthread_mutex_lock(&frequencyMutex_);
    frequency = shFrequency_;
    pthread_mutex_unlock(&frequencyMutex_);
}

//-----------------------------------------------------------------------
// This function checks the upcoming log time and saves a log if it is
// the time to do so.
//-----------------------------------------------------------------------
void Module::manageLog_(pthread_t threadID)
{
    string logfilePrefix;
    double curTime;
    char buf[1024];

    if (gLogManager != NULL)
    {
        if (nextLogTime_ == 0)
        {
            // Create the logfile name.
            logfilePrefix = gLogManager->getLogfilePrefix();
            sprintf(
                buf,
                "%s-%s:%ld.log",
                logfilePrefix.c_str(),
                NAME_.c_str(),
                threadID);
            logfileName_ = buf;

            logfile_ = fopen(logfileName_.c_str(), "w");

            if (logfile_ == NULL)
            {
                printfTextWindow(
                    "Module: Warning. Logfile %s could not be opened.\n",
                    logfileName_.c_str());
            }

            // Get the first log time.
            nextLogTime_ = gLogManager->getNextLogTime();

        }

        // Get the current time.
        curTime = getCurrentEpochTime();

        if (curTime >= nextLogTime_)
        {
            if (lastLoggedTime_ != nextLogTime_)
            {
                // If it is the time, log the data.
                logData_(nextLogTime_);

                lastLoggedTime_ = nextLogTime_;
            }

            // Check for the next log time.
            nextLogTime_ = gLogManager->getNextLogTime();
        }
    }
}

/**********************************************************************
# $Log: module.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:07  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2002/01/16 22:13:33  ernest
# Fixed null termination of statusStr.
#
# Revision 1.3  2000/12/12 23:20:41  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.2  2000/09/19 03:45:31  endo
# RCS log added.
#
#
#**********************************************************************/
