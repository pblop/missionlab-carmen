/**********************************************************************
 **                                                                  **
 **                              fred.c                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Robot emulation for HServer                                     **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: fred.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "sensors.h"
#include "robot_config.h"
#include "message.h"
#include "statusbar.h"
#include "fred.h"
#include "Watchdog.h"
#include "LogManager.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Mutexes
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int Fred::MAIN_THREAD_USLEEP_ = 100000;
const int Fred::SKIP_STATUSBAR_UPDATE_ = 10;
const int Fred::WATCHDOG_CHECK_INTERVAL_SEC_ = 2;

//-----------------------------------------------------------------------
// Contructor
//-----------------------------------------------------------------------
Fred::Fred(Robot** a) : 
    Robot(a, HS_ROBOT_TYPE_FRED, HS_MODULE_NAME_ROBOT_FRED)
{
    resetPose_();
    addSensors_();
    updateStatusBar_();

    pthread_create(&moverThread_, NULL, &startMoverThread_, (void*)this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), moverThread_);
    }

    refreshScreen();
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Fred::~Fred(void)
{
    pthread_cancel(moverThread_);
    pthread_join(moverThread_, NULL);
    delete partXyt_;
}

//-----------------------------------------------------------------------
// This function updates the status bar.
//-----------------------------------------------------------------------
void Fred::updateStatusBar_(void)
{
    HSPose_t pose;
    char buf[ 100 ];
    double frequency = 0;
    const bool DISPLAY_FREQUENCY = true;

    statusStr[ 0 ] = 0;
    strcat(statusStr, "Fred: ");

    getPose(pose);

    sprintf(
        buf,
        "%c x: %.2f  y: %.2f  t: %.2f",
        statusbarSpinner_->getStatus(),
        pose.loc.x,
        pose.loc.y,
        pose.rot.yaw);

    if (DISPLAY_FREQUENCY)
    {
        getFrequency_(frequency);
        sprintf(buf, "%s : %.2f Hz", buf, frequency);
    }

    strcat(statusStr, buf);
    statusbar->update(statusLine);
}

//-----------------------------------------------------------------------
// This function starts the mover thread.
//-----------------------------------------------------------------------
void *Fred::startMoverThread_(void *fredInstance)
{
    ((Fred*) fredInstance)->moverLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to move Fred.
//-----------------------------------------------------------------------
void Fred::moverLoop_(void)
{
    int count = 0;
    
    setMinimumPointTurnAngle(15);

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Compute the control speed.
        compControlSpeed_();

        // Control the robot.
        controlRobot_();

        // Compute the frequency.
        compFrequency_();

        // Compute the velocity.
        updateVelocity_();

        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        if (gLogManager != NULL)
        {
            manageLog_(moverThread_);
        }

        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        usleep(MAIN_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function moves the robot.
//-----------------------------------------------------------------------
void Fred::moveFred_(HSSpeed_t speed)
{
    double dt, curTime;
    static double lastTime = 0;

    curTime = getCurrentEpochTime();

    if (lastTime == 0)
    {
        lastTime = curTime;
        return;
    }

    dt = curTime - lastTime;

    advancePose2D_(speed, dt);

    lastTime = curTime;
}

//-----------------------------------------------------------------------
// This function controls the robot with the control speed.
//-----------------------------------------------------------------------
void Fred::controlRobot_(void)
{
    HSSpeed_t controlSpeed;

    getControlSpeed_(controlSpeed);

    moveFred_(controlSpeed);
}

//-----------------------------------------------------------------------
// This function adds the sensors.
//-----------------------------------------------------------------------
void Fred::addSensors_(void)
{
    partXyt_ = new RobotPartXyt();
}

//-----------------------------------------------------------------------
// This function sets up the user interface for the robot option.
//-----------------------------------------------------------------------
void Fred::control(void)
{
    bool done = false;
    int msgData[2];

    msgData[0] = 0; // "speed_factor" is obsolete.
    msgData[1] = 0; // "angular_speed_factor" is obsolete.

    messageDrawWindow(EnMessageType_FRED_CONTROL, EnMessageErrType_NONE, msgData);

    do
    {
        int c = getch();

        switch(c) {

        case 'd':
            messageHide();
            delete this;
            return;
            break;

        case 'x':
        case 'Q':
        case KEY_ESC:
            done = true;
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
    } while (!done);

    messageHide();
}

/**********************************************************************
# $Log: fred.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.7  2001/12/25 22:54:53  endo
# *** empty log message ***
#
# Revision 1.6  2001/05/29 22:35:41  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.5  2000/12/12 23:23:38  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.4  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
