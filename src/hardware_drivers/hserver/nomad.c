/**********************************************************************
 **                                                                  **
 **                               nomad.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Nomad 150/200 robot driver for HServer                          **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: nomad.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "message.h"
#include "statusbar.h"
#include "ndirect_hs.h"
#include "nomad.h"
#include "robot_config.h"
#include "sensors.h"
#include "Watchdog.h"


int translation_accel = 100; // default accel .1 inch/s/s
int translation_speed = 100; // default speed .1 inch/s
int steer_accel       = 450; // default steer accel .1 deg/s/s
int steer_speed       = 450; // default steer speed .1 deg/s
int turret_accel      = 450; // default accel .1 deg/s/s
int turret_speed      = 450; // default speed .1 deg/s


pthread_cond_t nomad_cond;
pthread_mutex_t nomad_cond_mutex;

double watchdog_speed;

pthread_mutex_t Nomad::nomadMutex_ = PTHREAD_MUTEX_INITIALIZER;

const int Nomad::DEFAULT_TIMEOUT_SEC_ = 2;
const int Nomad::READER_LOOP_USLEEP_ = 1000;
const int Nomad::SKIP_STATUSBAR_UPDATE_ = 10;
const int Nomad::MAX_DRIVE_SPEED_ = 4500; // .1 deg/sec
const int Nomad::MAX_STEER_SPEED_ = 100; // .1 in/sec
const int Nomad::WATCHDOG_CHECK_INTERVAL_SEC_ = 2;

void Nomad::updateStatusBar_()
{
    HSPose_t pose;
    char buf[100];
    statusStr[0] = 0;

    strcat(statusStr, "Nomad: ");
    
    getPose(pose);

    sprintf(
        buf,
        "%c x: %.2f y: %.2f t: %.2f ",
        statusbarSpinner_->getStatus(),
        pose.loc.x,
        pose.loc.y,
        pose.rot.yaw);

    strcat(statusStr, buf);

    if (nomadType_ == EnNomadType_200)
    {
        sprintf(buf, " cpu: %.2fV motor: %.2fV ", voltages[0], voltages[1]);
        strcat(statusStr, buf);
    }	

    if (mode == NOMAD_MODE_ZEROING)
    {
        strcat(statusStr, "... zeroing");
    }
    else if (mode == NOMAD_MODE_COMPASS)
    {
        strcat(statusStr, "... calibrating compass");
    }    

    statusbar->update(statusLine);
}


int Nomad::compassActivate_(void)
{
    int rtn;
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Nomad compass activate");
    }
    pthread_mutex_lock(&nomadMutex_);
    rtn = conf_cp(1);
    pthread_mutex_unlock(&nomadMutex_);
    if (!rtn)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("Error activating compass");
        }
        return -1;
    }
    return 0;
}

int Nomad::compassCalibrate_(void)
{
    int rtn;

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Nomad compass calibrate");
    }

    if (nomadType_ != EnNomadType_200)
    {
        return -1;
    }

    mode = NOMAD_MODE_COMPASS;
    updateStatusBar_();    

    pthread_mutex_lock(&nomadMutex_);
    rtn = conf_cp(2);
    pthread_mutex_unlock(&nomadMutex_);

    if (!rtn)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("Error calibrating compass");
        } 
        rtn = -1;
    }
    else
    {
        rtn = 0;
    }

    mode = NOMAD_MODE_NORMAL;
    updateStatusBar_();    

    return rtn;
}

void Nomad::setSonarSpeed_(int speed)
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Nomad set sonar speed: %d", speed);
    }
    sonar_speed = speed;
    conf_sn(sonar_speed, sonar_order);
}

void Nomad::sonarStart(void)
{
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Nomad sonar start");
    }
    conf_sn(sonar_speed, sonar_order);
}

void Nomad::sonarStop(void)
{
    int	order[16] = { 255 };
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Nomad sonar stop");
    }
    conf_sn(0, order);
}

void Nomad::controlRobot_(void)
{
    HSSpeed_t controlSpeed;
    int driveCmd;
    int steerCmd;

    getControlSpeed_(controlSpeed);

    // Drive command is in .1 in/s. 
    driveCmd = (int)(ROBOT_M2IN(controlSpeed.driveSpeed.value)*10.0);
    driveCmd = (driveCmd > MAX_DRIVE_SPEED_)? MAX_DRIVE_SPEED_ : driveCmd;
    driveCmd = (driveCmd < (-MAX_DRIVE_SPEED_))? (-MAX_DRIVE_SPEED_) : driveCmd;

    // Steer command is in .1 deg/s.
    steerCmd = (int)((controlSpeed.steerSpeed.value)*10.0);
    steerCmd = (steerCmd > MAX_STEER_SPEED_)? MAX_STEER_SPEED_ : steerCmd;
    steerCmd = (steerCmd < (-MAX_STEER_SPEED_))? (-MAX_STEER_SPEED_) : steerCmd;

    pthread_mutex_lock(&nomadMutex_);
    mv(MV_VM, driveCmd, MV_IGNORE, MV_IGNORE, MV_IGNORE, MV_IGNORE);
    mv(MV_IGNORE, MV_IGNORE, MV_VM, steerCmd, MV_VM, steerCmd);
    pthread_mutex_unlock(&nomadMutex_);
}

/* Test robot connection */
int Nomad::ping_(bool output)
{
    int rtn;

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        if (output)
        {
            printTextWindow("Nomad ping");
        }
    }

    pthread_mutex_lock(&nomadMutex_);
    rtn = mv(MV_IGNORE, MV_IGNORE, MV_IGNORE, MV_IGNORE, MV_IGNORE, MV_IGNORE);
    pthread_mutex_unlock(&nomadMutex_);

    if (output)
    {
        if (rtn)
        {
            printTextWindow("Nomad ping sucessful");
        }
        else
        {
            printTextWindow("Nomad ping unsucessful");
        }
    }
    return rtn;
}

int Nomad::updateCompass_(void)
{
    int rtn;
    pthread_mutex_lock(&nomadMutex_);
    rtn = get_cp();
    pthread_mutex_unlock(&nomadMutex_);
    if (!rtn)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("Error updating compass");
        }
        return !rtn;
    }
    compass[0] = ((float) gNomadState[STATE_COMPASS]) / 10.0;
    return !rtn;
}

int Nomad::updateSonar_(void)
{
    int i, rtn;
    pthread_mutex_lock(&nomadMutex_);
    rtn = get_sn();
    pthread_mutex_unlock(&nomadMutex_);
    for(i = 0; i < 16; i++)
    {
        sonar[i] = ((float) gNomadState[i + STATE_SONAR_0]) * .0254 + radius; 
    }
    return !rtn;
}

int Nomad::updateIR_(void)
{
    int i, rtn;
    pthread_mutex_lock(&nomadMutex_);
    rtn = get_ir();
    pthread_mutex_unlock(&nomadMutex_);
    for(i = 0; i < 16; i++)
    {
        if (gNomadState[i + STATE_IR_0] == 15)
        {
            ir[i] = 50.0;
        }
        else
        {
            ir[i] = ((float) gNomadState[i + STATE_IR_0]) * .0254 + radius; 
        }
    }
    return !rtn;
}

int Nomad::zero_(void)
{
    int rtn;

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Nomad zero");
    }

    mode = NOMAD_MODE_ZEROING;

    updateStatusBar_();

    pthread_mutex_lock(&nomadMutex_);
    rtn = zr();
    pthread_mutex_unlock(&nomadMutex_);

    if (!rtn)
    {
        if (report_level >= HS_REP_LEV_ERR)
        {
            printTextWindow("Error zeroing nomad");
        }
        rtn = -1;
    }
    else
    {
        rtn = 0;
    }

    mode = NOMAD_MODE_NORMAL;

    updateStatusBar_();

    return rtn;
}

void Nomad::initRobot(void)
{
    //resetPose_();
    zero_();
}

void Nomad::setInternalXYT_(double x, double y, double t)
{
    long lx, ly;
    int steering, turret;
    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Nomad set xyt: %f %f %f", x, y, t);
    }

    lx = (long) (x * 393.70079);  // meters to .1 inches
    ly = (long) (y * 393.70079);  // meters to .1 inches
    steering = turret = (int) (t * 10);	// degrees to .1 degrees

    pthread_mutex_lock(&nomadMutex_);
    dp(lx, ly);
    da(steering, turret);
    pthread_mutex_unlock(&nomadMutex_);
}

void Nomad::turnAngle_(double degrees)
{
    int turnCmd;

    if (report_level >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Nomad turn: %f", degrees);
    }

    turnCmd = (int)(degrees * 10.0); // in .1 deg.

    pthread_mutex_lock(&nomadMutex_);
    mv(MV_IGNORE, MV_IGNORE, MV_PR, turnCmd, MV_PR, turnCmd);
    pthread_mutex_unlock(&nomadMutex_);
}

void Nomad::moveDistance_(double meterDistance)
{
    int moveCmd;

    moveCmd = (int)(ROBOT_M2IN(meterDistance)*10.0); // in .1 inch.

    pthread_mutex_lock(&nomadMutex_);
    mv(MV_PR, moveCmd, MV_IGNORE, MV_IGNORE, MV_IGNORE, MV_IGNORE);
    pthread_mutex_unlock(&nomadMutex_);
}

void Nomad::updateVoltages_(void)
{
    pthread_mutex_lock(&nomadMutex_);
    voltages[0] = voltCpuGet();
    voltages[1] = voltMotorGet();
    battery_level  = voltages[1]; 
    pthread_mutex_unlock(&nomadMutex_);
}	    

int Nomad::updateLocation_(void)
{
    HSPose_t pose;

    int rtn;
    pthread_mutex_lock(&nomadMutex_);
    rtn = get_rc();
    rtn = get_rv();
    pthread_mutex_unlock(&nomadMutex_);

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    pose.loc.x = ROBOT_IN2M((double)(gNomadState[STATE_CONF_X]))*0.1;
    pose.loc.y = ROBOT_IN2M((double)(gNomadState[STATE_CONF_Y]))*0.1;
    pose.rot.yaw = (double)(gNomadState[STATE_CONF_STEER])*0.1;
    ROBOT_CRAMPDEG(pose.rot.yaw, 0.0, 360);
    pose.extra.time = getCurrentEpochTime();
    setPose_(pose);

    arm[0] = gNomadState[STATE_CONF_TURRET] * 0.1;

    return !rtn;
}

int Nomad::setupNomad_(void)
{
    int irorder[16], firerate, i;
    int	order[16] = { 0, 2, 15, 1, 14, 3, 13, 4, 12, 5, 11, 6, 10, 7, 9, 8 };
    unsigned char timeout;

    mode = NOMAD_MODE_NORMAL;
    updateStatusBar_();

    for(i = 0; i < 16; i++)
    {
        irorder[i] = order[i];
        sonar_order[i] = order[i];
    }
    firerate = 0;
    pthread_mutex_lock(&nomadMutex_);
    conf_sn(firerate, sonar_order);
    sonarStop();

    if (nomadType_ == EnNomadType_200)
    {
        conf_ir(2, irorder);
    }

    ac(translation_accel, steer_accel, turret_accel);
    sp(translation_speed, steer_speed, turret_speed);
    timeout = (unsigned char)DEFAULT_TIMEOUT_SEC_;
    conf_tm(timeout); 
    pthread_mutex_unlock(&nomadMutex_);

    if (nomadType_ == EnNomadType_200)
    {
        compassActivate_();
    }

    updateVoltages_();
    translation_speed = steer_speed = turret_speed = 0;
    mode = NOMAD_MODE_NORMAL;
    updateStatusBar_();

    return 0;
}

void *Nomad::startReaderThread_(void* nomadInstance)
{
    ((Nomad*)nomadInstance)->readerLoop_();
    return NULL;
}

void Nomad::readerLoop_(void)
{
    int count = 0;

    readerRunning = true;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Synchronize the 
        pthread_cond_broadcast(&nomad_cond);

        // Read the data from the robot.
        updateLocation_();
        updateSonar_();
        updateVoltages_();

        if (nomadType_ == EnNomadType_200)
        {
            updateIR_();
            updateCompass_();
        }

        // Compute the control speed.
        compControlSpeed_();

        // Control the robot.
        controlRobot_();

        // Check for the pthread termination again.
        pthread_testcancel();

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

        // Tell watchdog that the robot is still alive.
        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        usleep(READER_LOOP_USLEEP_);
    }
}

void Nomad::addSensors_(void)
{
    int i;

    for(i = 0; i < NOMAD_NUM_SONAR; i++)
    {
        sonar_angle[i] = float(i) * (360.0 / NOMAD_NUM_SONAR);
    }

    partXyt = new RobotPartXyt();

    partSonar = new RobotPartSonar(NOMAD_NUM_SONAR, sonar_angle);
    sensorSonar = new Sensor(SENSOR_SONAR, 0, NULL, NOMAD_NUM_SONAR, sonar_angle, NOMAD_NUM_SONAR, sonar);
    for(i = 0; i < NOMAD_NUM_SONAR; i++)
    {
        sonar[i] = 0.0;
    }

    partArm = new RobotPartArm();
    sensorArm = new Sensor(SENSOR_ARM, 0, NULL, 1, arm);
    arm[0] = 0.0;

    if (nomadType_ == EnNomadType_200)
    {
        partIr = new RobotPartIr(NOMAD_NUM_IR, sonar_angle);
        sensorIr = new Sensor(SENSOR_IR, 0, NULL, NOMAD_NUM_IR, sonar_angle, NOMAD_NUM_IR, ir);
        for(i = 0; i < NOMAD_NUM_IR; i++)
        {
            ir[i] = 0.0;
        }
        partCompass = new RobotPartCompass();
        sensorCompass = new Sensor(SENSOR_COMPASS, 0, NULL, 1, compass);
        compass[0] = 0.0;
    }
}


void Nomad::connectNomad_(void)
{
    int rtn;

    if (nomadType_ == EnNomadType_150)
    {
        sprintf(rstr, "Connecting to nomad 150 over %s...", strPortString.c_str());
        printTextWindow(rstr, false);
        nomad_open_serial(strPortString.c_str(), 38400);

        rtn = ping_(false);

        if (!rtn)
        {
            printTextWindow("failed");
            if (report_level>=HS_REP_LEV_ERR)
            {
                printTextWindow("error: Nomad open serial port");
            }

            mode = NOMAD_MODE_NORMAL; // Ernest- have to set mode before calling updateStatusBar_();
            updateStatusBar_();
            redrawWindows();
            delete this;
        }
    }
    else
    {
        printfTextWindow("Connecting to nomad 200 on %s...", strHost.c_str());
        printTextWindow(rstr, false);
        rtn = connect_robot(strHost.c_str());

        if (rtn)
        {
            printTextWindow("failed");
            if (report_level>=HS_REP_LEV_ERR)
            {
                printfTextWindow("error: Nomad connect robot %d", rtn);
            }
            delete this;
        }
    }
    printTextWindow("connected");

    mode = NOMAD_MODE_ZEROING;
    updateStatusBar_();
}


Nomad::~Nomad(void)
{
    if (readerRunning)
    {
        pthread_cancel(reader);
        pthread_join(reader, NULL);
        sonarStop();
        stop();
        disconnect_robot();
        delete sensorSonar;
        delete sensorArm;
        delete partArm;
        delete partXyt;
        delete partSonar;
        if (nomadType_ == EnNomadType_200)
        {
            delete sensorCompass;
            delete sensorIr;
            delete partCompass;
            delete partIr;
        }
        printTextWindow("Nomad disconnected");
    }
}

Nomad::Nomad(
    Robot** a,
    int nomadType,
    const string& strPortString, 
    const string& strHost) :
    Robot(a, HS_ROBOT_TYPE_NOMAD, HS_MODULE_NAME_ROBOT_NOMAD),
    nomadType_(nomadType),
    sonar_on(false),
    readerRunning(false)
{
    this->strHost = strHost;
    this->strPortString = strPortString;

    sonar_speed = 0;
    radius = 0.24;

    addSensors_();

    connectNomad_();
    setupNomad_();

    pthread_cond_init(&nomad_cond, NULL);
    pthread_mutex_init(&nomad_cond_mutex, NULL);

    pthread_create(&reader, NULL, &startReaderThread_, (void*)this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), reader);
    }

    readerRunning = true;

    initRobot();
}


void Nomad::control()
{
    bool bDone = false;
    int msgData[2];

    msgData[0] = sonar_on;
    msgData[1] = sonar_speed;

    messageDrawWindow(EnMessageType_NOMAD_CONTROL, EnMessageErrType_NONE, msgData);
    redrawWindows();
    do
    {
        int c = getch();

        switch(c) {

        case 'd':
            messageHide();
            delete this;
            return;
            break;
        case 'r':
            refreshScreen();
            break;
        case 'z':
            zero_();
            break;
        case 'c':
            compassCalibrate_();
            break;
        case 's':
            sonar_on = !sonar_on;
            msgData[0] = sonar_on;
            messageDrawWindow(EnMessageType_NOMAD_CONTROL, EnMessageErrType_NONE, msgData);
            if (sonar_on)
            {
                sonarStart();
            }
            else
            {
                sonarStop();
            }
            break;
        case '-':
        case '_':
            sonar_speed--;
            if (sonar_speed < 0)
            {
                sonar_speed = 0;
            }
            msgData[1] = sonar_speed;
            messageDrawWindow(EnMessageType_NOMAD_CONTROL, EnMessageErrType_NONE, msgData);
            setSonarSpeed_(sonar_speed);
     	    break;
        case '+':
        case '=':
            sonar_speed++;
            msgData[1] = sonar_speed;
            messageDrawWindow(EnMessageType_NOMAD_CONTROL, EnMessageErrType_NONE, msgData);
            setSonarSpeed_(sonar_speed);
            break;
        case 'p':
            ping_(true);
            break;
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
# $Log: nomad.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:07  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.9  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.8  2003/04/02 22:23:22  zkira
# Battery level for telop GUI
#
# Revision 1.7  2002/01/16 22:17:17  ernest
# Added WatchDog function; fixed other misc. bugs.
#
# Revision 1.6  2001/05/29 22:35:57  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.5  2001/03/23 21:31:44  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 23:22:16  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/09/19 03:47:14  endo
# RCS log added.
#
#
#**********************************************************************/
