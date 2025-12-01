/**********************************************************************
 **                                                                  **
 **                               nomad.h                            **
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

/* $Id: nomad.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef NOMAD_H
#define NOMAD_H

#include <pthread.h>
#include <string>
#include <sys/time.h>
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"

using std::string;

#define NOMAD_MAX_VEL  100
#define NOMAD_MAX_RVEL 4500
#define NOMAD_MIN_ACC  40
#define NOMAD_MAX_ACC  320

#define NOMAD_NUM_SONAR 16
#define NOMAD_NUM_IR    16

#define NOMAD_MODE_NORMAL  0
#define NOMAD_MODE_ZEROING 1
#define NOMAD_MODE_COMPASS 2

#define NOMAD_MAX_IR 15 // inches

#define NOMAD_WAIT 20   // seconds

class Nomad : public Robot {

public:
    enum EnNomadType
    {
        EnNomadType_150,
        EnNomadType_200
    };

protected:
    Sensor* sensorSonar;
    Sensor* sensorIr;
    Sensor* sensorArm;
    Sensor* sensorCompass;
    RobotPartSonar* partSonar;
    RobotPartIr* partIr;
    RobotPartXyt* partXyt;
    RobotPartArm* partArm;
    RobotPartCompass* partCompass;
    string strPortString;
    string strHost;
    float sonar_angle[NOMAD_NUM_SONAR];
    float sonar[NOMAD_NUM_SONAR];
    int sonar_order[NOMAD_NUM_SONAR];
    int sonar_speed; // To Do: Protect this data with a mutex.
    int nomadType_;
    float ir[NOMAD_NUM_IR];
    float arm[1];
    float compass[1];
    double voltages[2];
    int mode;
    double radius;
    pthread_mutex_t nomad_watchdog_mutex;
    pthread_t reader;
    bool sonar_on;
    bool readerRunning;

    static pthread_mutex_t nomadMutex_;

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_LOOP_USLEEP_;
    static const int MAX_DRIVE_SPEED_;
    static const int MAX_STEER_SPEED_;
    static const int DEFAULT_TIMEOUT_SEC_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

    int setupNomad_(void);
    int updateCompass_(void);
    int updateSonar_(void);
    int updateIR_(void);
    int updateLocation_(void);
    int compassActivate_(void);
    int compassCalibrate_(void);
    int ping_(bool output);
    int zero_(void);
    void connectNomad_(void);
    void setSonarSpeed_(int speed);
    void updateVoltages_(void);
    void moveDistance_(double meterDistance);
    void turnAngle_(double degrees);
    void setInternalXYT_(double xi, double yi, double theta);
    void updateStatusBar_(void);
    void controlRobot_(void);
    void readerLoop_(void);
    void addSensors_(void);

    static void *startReaderThread_(void* nomadInstance);


public:
    Nomad(
        Robot** a,
        int nomadType,
        const string& strPortString, 
        const string& strHost);
    ~Nomad(void);
    void sonarStart(void);
    void sonarStop(void);
    void initRobot(void);
    void control(void);
};    

#endif

/**********************************************************************
# $Log: nomad.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.7  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.6  2002/01/16 22:19:11  ernest
# Added WatchDog function
#
# Revision 1.5  2001/05/29 22:36:02  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.4  2001/03/23 21:31:53  blee
# altered to use a config file
#
# Revision 1.3  2000/09/19 03:47:14  endo
# RCS log added.
#
#
#**********************************************************************/
