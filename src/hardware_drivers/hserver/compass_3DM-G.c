/**********************************************************************
 **                                                                  **
 **                         compass_3DM-G.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Alexander Stoytchev and Yoichiro Endo               **
 **                                                                  **
 **  MicroSensor 3DM-G driver for HServer                            **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: compass_3DM-G.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include "HServerTypes.h"
#include "compass.h"
#include "compass_3DM-G.h"
#include "3DM-G/m3dmgAdapter.h"
#include "Watchdog.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int Compass3DMG::BAUD_RATE_ = 38400;
const int Compass3DMG::READER_THREAD_USLEEP_ = 5000;
const int Compass3DMG::SKIP_STATUSBAR_UPDATE_ = 10;
const int Compass3DMG::WATCHDOG_CHECK_INTERVAL_SEC_ = 3;
const bool Compass3DMG::USE_GYRO_STABILIZED_VALUE_ = true;

//-----------------------------------------------------------------------
// Constructor for the Compass3DMG class.
//-----------------------------------------------------------------------
Compass3DMG::Compass3DMG(Compass** a, string portString)
    : Compass( (Compass**) a, TYPE_3DMG, HS_MODULE_NAME_COMPASS_3DMG)
{
    int rtn;

    portString_ = portString;

    if (report_level > HS_REP_LEV_NONE)
    {
        printTextWindow("Connecting to Compass 3DM-G... ", false);
    }

    strcat(statusStr, "Compass 3DM-G: [connecting]");
    statusbar->update(statusLine);

    deviceNum3DMG_ = -1;
    readerThreadUSleep_ = READER_THREAD_USLEEP_;
    skipStatusBarUpdate_ = SKIP_STATUSBAR_UPDATE_;

    // Open the port and connect to the device.
    rtn = initialize3DMG_();

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

    // Start reading data.
    pthread_create( &readerThread_, NULL, &startMainThread_, (void*) this );

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

//-----------------------------------------------------------------------
// Destructor for the Compass3DMG class.
//-----------------------------------------------------------------------
Compass3DMG::~Compass3DMG(void)
{
    if (readerThreadIsUp_)
    {
        pthread_cancel(readerThread_);
        pthread_join(readerThread_, NULL );
    }

    if (deviceNum3DMG_ > 0)
    {
        m3dmg_closeDevice(deviceNum3DMG_);
    }

    printTextWindow( "Compass 3DM-G disconnected" );
}

//-----------------------------------------------------------------------
// This function updates the status bar.
//-----------------------------------------------------------------------
void Compass3DMG::updateStatusBar_(void)
{
    double heading = 0;
    char buf[100];
    double frequency = 0;
    const bool DISPLAY_FREQUENCY = true;
  
    statusStr[0] = 0;

    strcat(statusStr, "Compass 3DM-G: ");

    getHeading(heading);

    sprintf(buf, "%c heading: %.1f [deg]", statusbarSpinner_->getStatus(), heading);

    if (DISPLAY_FREQUENCY)
    {
        getFrequency_(frequency);
        sprintf(buf, "%s : %.2f Hz", buf, frequency);
    }

    strcat(statusStr, buf);
    statusbar->update(statusLine);
}

//-----------------------------------------------------------------------
// Connect to the physical 3DM-G device. 
//-----------------------------------------------------------------------
int Compass3DMG::initialize3DMG_(void) 
{
    int portNum3DMG;

    // Open the serial/rocket port.
    portNum3DMG = m3dmg_openPort(portString_, BAUD_RATE_, 8, 0, 1);

    if (portNum3DMG < 0)
    {
        //printfTextWindow("Compass 3DM-G: Failed to open %s.\n", portString_.c_str());
        return -1;
    }

    // Map the device
    deviceNum3DMG_ = m3dmg_mapDevice(1, portNum3DMG);

    if (deviceNum3DMG_ <= 0)
    {
        //printfTextWindow("Compass 3DM-G: Failed to map the device.\n");
        return -1;
    }

    return 0;
}

//-----------------------------------------------------------------------
// This function grabs the Compass data
//-----------------------------------------------------------------------
Compass3DMG::CompassData_t Compass3DMG::readCompassData_(void)
{
    CompassData_t compassData;
    float roll = 0;
    float pitch = 0;
    float yaw = 0;
    int rtn;
 
    // Reset the data.
    memset((HSRotation_t *)&(compassData.rot), 0x0, sizeof(HSRotation_t));

    // Put the time stamp.
    compassData.rot.extra.time = getCurrentEpochTime();

    // Put the time stamp.

    if (USE_GYRO_STABILIZED_VALUE_)
    {
        // Euler angles - gyro-stabilized
        rtn = m3dmg_getEulerAngles(deviceNum3DMG_, &pitch, &roll, &yaw, M3D_STABILIZED);
    }
    else
    {
        // Euler angles - instantaneous
        rtn = m3dmg_getEulerAngles(deviceNum3DMG_, &pitch, &roll, &yaw, M3D_INSTANT);
    }

    if (rtn < 0)
    {
        printfTextWindow("Compass3DMG: Failing to read the compass data.\n");
        compassData.validData = false;
    }
    else
    {
        compassData.rot.yaw = yaw;
        COMPASS_CRAMPDEG((compassData.rot.yaw), 0.0, 360.0);
        compassData.rot.pitch = pitch;
        COMPASS_CRAMPDEG((compassData.rot.pitch), 0.0, 360.0);
        compassData.rot.roll = roll;
        COMPASS_CRAMPDEG((compassData.rot.roll), 0.0, 360.0);
        compassData.validData = true;

        if (report_level >= HS_REP_LEV_DEBUG)
        {
            printfTextWindow(
                "Compass3DMG Debug: Yaw/Pitch/Roll <%.2f, %.2f, %.2f>",
                yaw, pitch, roll);
        }
    }

    return compassData;
}

//-----------------------------------------------------------------------
// This function gets called when the user types 'K'.
//-----------------------------------------------------------------------
void Compass3DMG::control(void)
{
    int c;
    bool bDone = false;

    messageDrawWindow(
        EnMessageType_COMPASS_3DMG_CONTROL,
        EnMessageErrType_NONE);
    redrawWindows();

    do
    {
        c = getch();
        switch( c ) {

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

//-----------------------------------------------------------------------
// For debugging inside the 3DM-G code.
//-----------------------------------------------------------------------
void Compass3DMG_Printf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printfTextWindow("Compass 3DM-G: %s", buf);
}

/**********************************************************************
# $Log: compass_3DM-G.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/05/13 23:14:01  endo
# Frequency info added.
#
# Revision 1.1  2004/04/24 11:20:53  endo
# 3DM-G added.
#
***********************************************************************/
