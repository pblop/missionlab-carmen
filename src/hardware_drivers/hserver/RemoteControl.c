/**********************************************************************
 **                                                                  **
 **                          RemoteControl.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by: William Halliburton                                 **
 **  Revised by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: RemoteControl.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <pthread.h>
#include <panel.h>
#include <stdlib.h>
#include <unistd.h>

#include "RemoteControl.h"
#include "hserver.h"
#include "robot.h"
#include "camera.h"
#include "message.h"
#include "console.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t RemoteControl::robotControlSpeedMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int RemoteControl::CONTROLLER_THREAD_USLEEP_ = 10000;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
RemoteControl::RemoteControl(void) : 
    controllerIsUp_(false)
{
    memset((HSSpeed_t *)&shRobotControlSpeed_, 0x0, sizeof(HSSpeed_t));
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
RemoteControl::~RemoteControl(void)
{
    if (controllerIsUp_)
    {
        pthread_cancel(controllerThread_);
        pthread_join(controllerThread_, NULL);
    }
}

//-----------------------------------------------------------------------
// This function starts the remote control thread.
//-----------------------------------------------------------------------
void* RemoteControl::startControllerThread_(void* controllerInstance)
{
    ((RemoteControl *)controllerInstance)->controllerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function constantly sends commands to the robot and/or camera.
//-----------------------------------------------------------------------
void RemoteControl::controllerLoop_(void)
{
    while (true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Send the necessary commands to the robot.
        sendRobotControlCommands_();
        
        // Make sure termination was not requested again.
        pthread_testcancel();

        usleep(CONTROLLER_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function sends the control commands to the robot.
//-----------------------------------------------------------------------
void RemoteControl::sendRobotControlCommands_(void)
{
    HSSpeed_t controlSpeed;
    double curTime;

    if (gRobot == NULL)
    {
        return;
    }

    // Get the control speed.
    getRobotControlSpeed_(controlSpeed);

    // Assign a new time stamp.
    curTime = getCurrentEpochTime();
    controlSpeed.driveSpeed.extra.time = curTime;
    controlSpeed.steerSpeed.extra.time = curTime;
    controlSpeed.extra.time = curTime;

    // Send off the command.
    gRobot->setDesiredSpeed(controlSpeed);
}

//-----------------------------------------------------------------------
// This function starts the controller thread and gets key inputs.
//-----------------------------------------------------------------------
void RemoteControl::runRemoteControl(void)
{
    gDisableConsoleRobotControl = true;

    // Reset the control speed of the robot.
    resetRobotControlSpeed_();

    // Start the controller  thread.
    pthread_create(&controllerThread_, NULL, &startControllerThread_, (void*)this);
    controllerIsUp_ = true;

    // Process the keyboad inputs from the user.
    processKeyInputs_();

    // Finish the controller  thread.
    pthread_cancel(controllerThread_);
    pthread_join(controllerThread_, NULL);

    controllerIsUp_ = false;
    gDisableConsoleRobotControl = false;
}

//-----------------------------------------------------------------------
// This function processes the keyboad inputs from the user.
//-----------------------------------------------------------------------
void RemoteControl::processKeyInputs_(void)
{
    bool done = false;
    int velocity = 0;
    double uvelocity = 0;
    int rvelocity = 0;
    double urvelocity = 0;
    double armvelocity = 0;
    int cameraInc = 4;
    int robotInc = 16;
    int msgData[7];
    int pan = 0;
    int tilt = 0;
    int refresh = 0;
    int pandegree,tiltdegree;

    msgData[0] = cameraInc;
    msgData[1] = robotInc;
    msgData[2] = velocity;
    msgData[3] = rvelocity;
    msgData[4] = (int)(armvelocity);
    msgData[5] = false; // teleport [Obsolete]
    msgData[6] = 4; // teleport inc [Obsolete]

    show_panel(msgPanel);
    redrawWindows();

    messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
    pandegree = int(pan * PAN_TO_DEG);
    tiltdegree = int(tilt * TILT_TO_DEG);
    do
    {
        int c = getch();

        switch(c){

        case 'w':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            tilt += cameraInc;
            tiltdegree = int(tilt * TILT_TO_DEG);
            if (tilt > TILT_ABS_POS_MAX)
            {
                tilt = TILT_ABS_POS_MAX;
            }
            camera->moveAbs(pan,tilt);
            break;

        case 's':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            tilt -= cameraInc;
            tiltdegree = int(tilt * TILT_TO_DEG);
            if (tilt < TILT_ABS_POS_MIN)
            {
                tilt = TILT_ABS_POS_MIN;
            }
            camera->moveAbs(pan,tilt);
            break;

        case 'a':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            pan -= cameraInc;
            pandegree = int(pan * PAN_TO_DEG);
            if (pan > PAN_ABS_POS_MAX)
            {
                tilt = PAN_ABS_POS_MAX;
            }
            camera->moveAbs(pan,tilt);
            break;

        case 'd':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            pan += cameraInc;
            pandegree = int(pan * PAN_TO_DEG);
            if (pan < PAN_ABS_POS_MIN)
            {
                tilt = PAN_ABS_POS_MIN;
            }
            camera->moveAbs(pan,tilt);
            break;

        case '0':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            pan = tilt = 0;
            pandegree = tiltdegree = 0;
            camera->moveAbs(0,0);
            break;

        case 'c':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            cameraInc = cameraInc / 2;
            if (cameraInc == 0)
            {
                cameraInc = 1;
            }
            msgData[0] = cameraInc; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'e':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            cameraInc = cameraInc * 2;
            msgData[0] = cameraInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'r':
            refreshScreen();
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            break;

        case 'q':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            camera->zoomInc(cameraInc*10);
            break;

        case 'z':
            if (camera == NULL)
            {
                printfTextWindow("Warning: Camera is not connected.\n");
                break;
            }
            camera->zoomInc(cameraInc*10);
            break;

        case 'Q':
        case 'x':
            done = true;
            break;

        case 'i':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            velocity += robotInc;
            setRobotControlDriveSpeed_((double)velocity/1000.0);
            msgData[2] = velocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'k':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            velocity -= robotInc;
            setRobotControlDriveSpeed_((double)velocity/1000.0);
            msgData[2] = velocity ; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'j':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            rvelocity += robotInc;
            setRobotControlSteerSpeed_((double)rvelocity);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'l':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            rvelocity -= robotInc;
            setRobotControlSteerSpeed_((double)rvelocity);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case ' ':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            velocity = rvelocity = 0;
            uvelocity = urvelocity = 0;
            armvelocity = 0;
            resetRobotControlSpeed_();
            
            msgData[2] = velocity; 
            msgData[3] = rvelocity; 
            msgData[4] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'o':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            rvelocity = 0;
            setRobotControlSteerSpeed_(0);
            msgData[3] = rvelocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'u':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            velocity = 0;
            setRobotControlDriveSpeed_(0);
            msgData[2] = velocity; 
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'p':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            robotInc = robotInc * 2;
            msgData[1] = robotInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case ';':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            robotInc = robotInc / 2;
            if (robotInc == 0)
            {
                robotInc = 1;
            }
            msgData[1] = robotInc;
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'n':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            armvelocity += ((double)robotInc);
            setRobotControlArmSpeed_(armvelocity);
            msgData[4] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case 'm':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            armvelocity -= ((double)robotInc);
            setRobotControlArmSpeed_(armvelocity);
            msgData[4] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
            break;

        case ',':
            if (gRobot == NULL)
            {
                printfTextWindow("Warning: Robot is not connected.\n");
                break;
            }
            armvelocity = 0;
            setRobotControlArmSpeed_(armvelocity);
            msgData[4] = (int)(armvelocity);
            messageDrawWindow(EnMessageType_REMOTE,EnMessageErrType_NONE,msgData);
            refresh = true;
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
            messageMovePanelKey(c);
            break;

        default:
            printfTextWindow("key %c", (char)c);
            break;
        }

        if (refresh)
        {
            pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void*) &screenMutex);
            pthread_mutex_lock(&screenMutex);
            wrefresh(msgWindow);
            pthread_cleanup_pop(1);
            refresh = false;
        }
    } while (!done);

    hide_panel(msgPanel);
    redrawWindows();
}

//-----------------------------------------------------------------------
// This function resets the speed of the robot.
//-----------------------------------------------------------------------
void RemoteControl::resetRobotControlSpeed_(void)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&robotControlSpeedMutex_);
    memset((HSSpeed_t *)&shRobotControlSpeed_, 0x0, sizeof(HSSpeed_t));
    shRobotControlSpeed_.driveSpeed.extra.time = curTime;
    shRobotControlSpeed_.steerSpeed.extra.time = curTime;
    shRobotControlSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&robotControlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the drive speed of the robot. 
//-----------------------------------------------------------------------
void RemoteControl::setRobotControlDriveSpeed_(double driveSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&robotControlSpeedMutex_);
    shRobotControlSpeed_.driveSpeed.value = driveSpeed;
    shRobotControlSpeed_.driveSpeed.extra.time = curTime;
    shRobotControlSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&robotControlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the steer speed of the robot. 
//-----------------------------------------------------------------------
void RemoteControl::setRobotControlSteerSpeed_(double steerSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&robotControlSpeedMutex_);
    shRobotControlSpeed_.steerSpeed.value = steerSpeed;
    shRobotControlSpeed_.steerSpeed.extra.time = curTime;
    shRobotControlSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&robotControlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the arm speed of the robot. 
//-----------------------------------------------------------------------
void RemoteControl::setRobotControlArmSpeed_(double armSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&robotControlSpeedMutex_);
    shRobotControlSpeed_.armSpeed.value = armSpeed;
    shRobotControlSpeed_.armSpeed.extra.time = curTime;
    shRobotControlSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&robotControlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function returns the speed of the robot. 
//-----------------------------------------------------------------------
void RemoteControl::getRobotControlSpeed_(HSSpeed_t &speed)
{
    pthread_mutex_lock(&robotControlSpeedMutex_);
    speed = shRobotControlSpeed_;
    pthread_mutex_unlock(&robotControlSpeedMutex_);
}

/**********************************************************************
# $Log: RemoteControl.c,v $
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
