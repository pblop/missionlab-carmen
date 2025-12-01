/**********************************************************************
 **                                                                  **
 **                              atrvjr.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  RWI ATRV-Jr robot control for HServer                           **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: atrvjr.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef ATRVJR_H
#define ATRVJR_H

#include "mobilitycomponents_i.h"
#include "mobilitydata_i.h"
#include "mobilitygeometry_i.h"
#include "mobilityactuator_i.h"
#include "mobilityutil.h"
#include <pthread.h>
#include <unistd.h>
#include <math.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "message.h"
#include "robot_config.h"
#include "sensors.h"
#include "statusbar.h"
#include "robot.h"
#include "sensors.h"

// ATRV-JR has 17 sonar sensors.
#define ATRVJR_NUM_SONAR 17

// Angles of the sonar sensors. 0 is the forward direction. Counter clockwise.
#define ATRVJR_RANGE_ANGLE {180,90,90,75,60,45,30,15,0,-15,-30,-45,-60,-75,-90,-90,-180}

// Distances of the sonar sensors from the center of the robot. In meters.
#define ATRVJR_RANGE_LOC {0.32,0.20,0.20,0.25,0.28,0.32,0.32,0.32,0.32,0.32,0.32,0.32,0.28,0.25,0.20,0.20,0.32}

// Usually, the ATRV-Jr sonars can detect about 4.0 meters
#define ATRVJR_SONAR_MAX_RANGE 3.75

class ATRVJr : public Robot {

protected:
    MobilityGeometry::SegmentState_var moSonarState_;
    MobilityGeometry::PointState_var moBumperState_;
    MobilityActuator::ActuatorState_var moControlCommandState_;
    MobilityActuator::ActuatorState_var moLocationState_;
    MobilityData::PowerManagementState_var moPowerState_;
    HSPose_t shAtrvjrPoseOffset_; // Shared
    HSPose_t shLatestDeadReckoning_; // Shared
    Sensor* sensorSonar_;
    RobotPartXyt* partXyt_;
    RobotPartSonar* partSonar_;
    pthread_t readerThread_;
    float atrvjr_sonar[ATRVJR_NUM_SONAR];
    float atrvjr_range_angle[ATRVJR_NUM_SONAR];
    float atrvjr_range_loc[ATRVJR_NUM_SONAR];
    double atrvjr_battery[1];

    static pthread_mutex_t atrvjrPoseOffsetMutex_;
    static pthread_mutex_t latestDeadReckoningMutex_;

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_THREAD_USLEEP_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

    void initializeMobility_(void);
    void checkBumper_(void);
    void readSonarRanges_(void);
    void readPowerData_(void);
    void readDeadReckoningData_(void);
    void addSensors_(void);
    void updateStatusBar_(void);
    void updatePoseFromAtrvjrOffset_(void);
    void setAtrvjrPose_(HSPose_t pose);
    void setAtrvjrPoseOffset_(HSPose_t poseOffset);
    void getAtrvjrPoseOffset_(HSPose_t &poseOffset);
    void setLatestDeadReckoning_(HSPose_t pose);
    void getLatestDeadReckoning_(HSPose_t &pose);
    void readerLoop_(void);
    void controlRobot_(void);
    void atrvjrStop_(void);

    static void *startReaderThread_(void *atrvjrInstance);

public:
    ATRVJr(Robot** a);
    ~ATRVJr(void);
    void control(void);
};


#endif

/**********************************************************************
# $Log: atrvjr.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2002/01/16 21:44:27  ernest
# Added variables to support WatchDog function
#
# Revision 1.2  2001/05/29 22:34:20  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.1  2000/11/13 20:09:23  endo
# Initial revision
#
#
#**********************************************************************/
