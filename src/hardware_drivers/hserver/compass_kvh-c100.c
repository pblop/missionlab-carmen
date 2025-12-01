/**********************************************************************
 **                                                                  **
 **                           compass_kvh-c100.c                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  KVH C100 Compass driver for HServer                             **
 **                                                                  **
 **  Note: HServer reads compass data from mobility. mobility has to **
 **        be installed in the system.                               **
 **                                                                  **
 **  Copyright 2001 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: compass_kvh-c100.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include "HServerTypes.h"
#include "compass.h"
#include "compass_kvh-c100.h"
#include "hserver.h"
#include "Watchdog.h"

const char *CompassKVHC100::COMPASSSERVER_NAME_ = "KVH C100";
const int CompassKVHC100::READER_THREAD_USLEEP_ = 1000;
const int CompassKVHC100::SKIP_STATUSBAR_UPDATE_ = 10;
const int CompassKVHC100::WATCHDOG_CHECK_INTERVAL_SEC_ = 3;

// Constructor for the CompassKVHC100 class.
CompassKVHC100::CompassKVHC100(Compass** a)
    : Compass((Compass**) a, TYPE_KVHC100, HS_MODULE_NAME_COMPASS_KVHC100)
{
    int rtn;

    if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("Connecting to Compass KVH C100... ", false);
    }

    strcat(statusStr, "Compass KVH C100: [connecting]");
    statusbar->update(statusLine);

    readerThreadUSleep_ = READER_THREAD_USLEEP_;
    skipStatusBarUpdate_ = SKIP_STATUSBAR_UPDATE_;

    // Initialize the mobility.
    rtn = initializeMobility_();

    refreshScreen();

    if (rtn)
    {
        if (report_level > HS_REP_LEV_NONE)
        {
            printTextWindow("failed", true);
        }

        delete this;
        return;
    }

    // Start reading the COMPASS KVH C100 data.
    pthread_create(&readerThread_, NULL, &startMainThread_, (void*)this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), readerThread_);
    }

    while (!firstDataReceived_)
    {
        usleep(readerThreadUSleep_);
    }

    if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow( "connected", true );
    }

    updateStatusBar_();
}

// Destructor for the CompassKVHC100 class.
CompassKVHC100::~CompassKVHC100()
{
    if (readerThreadIsUp_)
    {
        pthread_cancel(readerThread_);
        pthread_join(readerThread_, NULL );
    }

    printTextWindow( "Compass KVH C100 disconnected" );
}

// This function updates the status bar.
void CompassKVHC100::updateStatusBar_()
{
    double heading = 0;
    char buf[100];
  
    statusStr[0] = 0;

    strcat(statusStr, "Compass KVH C100: ");

    getHeading(heading);
  
    sprintf(buf, "%c heading: %.1f [deg]", statusbarSpinner_->getStatus(), heading);
    strcat(statusStr, buf);
    statusbar->update(statusLine);
}

// This function sets up the interface between HServer and mobility. 
int CompassKVHC100::initializeMobility_(void)
{
    int argc;
    char* argv[4];
    char* compassName = strdup(COMPASSSERVER_NAME_);
    char pathName[255];

    argv[0] = "HServer";
    argv[1] = "COMPASS";
    argv[2] = "KVHC100";
    argv[3] = 0;
    argc = 3;

    pHelper_ = new mbyClientHelper(argc, argv);

    sprintf(pathName, "%s/Compass", compassName);

    ptempObj_ = pHelper_->find_object(pathName);

    pMbyCompassState_ = FVectorState_i::_duplicate(
        FVectorState_i::_narrow(ptempObj_));

    return 0;
}  

// This function grabs the Compass KVH C100 data from mobility
Compass::CompassData_t CompassKVHC100::readCompassData_(void)
{
    MobilityData::FVectorData  *mbyCompassData;
    CompassData_t compassData;
    struct timeval timeVal;
    struct timezone timeZone;

    // Reset the data.
    memset((HSRotation_t *)&(compassData.rot), 0x0, sizeof(HSRotation_t));

    // Put the time stamp.
    compassData.rot.extra.time = getCurrentEpochTime();

    // Grab the latest data set from mobility.
    mbyCompassData = pMbyCompassState_->get_sample(0); 

    if (mbyCompassData != NULL)
    {
        // Retrieve the heading data.
        compassData.rot.yaw = mbyCompassData->data[0];
        COMPASS_CRAMPDEG((compassData.rot.yaw), 0.0, 360.0);
 
        // Label it as a valid.
        compassData.validData = true;
    }
    else
    {
        compassData.validData = false;
    }

    delete mbyCompassData;
    mbyCompassData = NULL;

    return compassData;
}

void CompassKVHC100::control(void)
{
    int c;
    bool bDone = false;

    messageDrawWindow(
        EnMessageType_COMPASS_KVH_C100_CONTROL,
        EnMessageErrType_NONE);

    redrawWindows();

    do
    {
        c = getch();

        switch(c) {

        case 'd':
            messageHide();
            delete this;
            return;
        case 'x':
        case 'Q':
        case KEY_ESC:
            bDone = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey(c);
            break;
        }
    } while (!bDone);

    messageHide();
}

/**********************************************************************
# $Log: compass_kvh-c100.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.3  2004/05/13 22:47:34  endo
# Memory leak fixed.
#
# Revision 1.2  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2001/05/18 17:55:02  blee
# Initial revision
#
#**********************************************************************/

