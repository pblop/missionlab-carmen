/**********************************************************************
 **                                                                  **
 **                              urban.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  RWI Urban Robot control for HServer                             **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: urban.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef URBAN_H
#define URBAN_H

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

// Urnan has 10 sonar sensors.
#define URBAN_NUM_SONAR 10

// Angles of the sonar sensors. 0 is the forward direction. Counter clockwise.
#define URBAN_RANGE_ANGLE {0,15,75,90,115,-115,-90,-75,-15,0}

// Distances of the sonar sensors from the center of the robot. In meters.
#define URBAN_RANGE_LOC {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0}

// Usually, the Urban sonars can detect about 4.0 meters
#define URBAN_SONAR_MAX_RANGE 3.75

class Urban : public Robot {

protected:
    MobilityGeometry::SegmentState_var moSonarState_;
    MobilityActuator::ActuatorState_var moControlCommandState_;
    MobilityActuator::ActuatorState_var moArmCommandState_;
    MobilityActuator::ActuatorState_var moLocationState_;
    MobilityActuator::ActuatorState_var moArmState_;
    MobilityData::PowerManagementState_var moPowerState_;
    HSPose_t shUrbanPoseOffset_; // Shared
    HSPose_t shLatestDeadReckoning_; // Shared
    Sensor* sensorSonar_;
    Sensor* sensorArm_;
    RobotPartXyt* partXyt_;
    RobotPartSonar* partSonar_;
    RobotPartArm* partArm_;
    pthread_t readerThread_;
    float urban_sonar[URBAN_NUM_SONAR];
    float urban_range_angle[URBAN_NUM_SONAR];
    float urban_range_loc[URBAN_NUM_SONAR];
    float urban_arm[1];
    double urban_battery[1];

    static pthread_mutex_t urbanPoseOffsetMutex_;
    static pthread_mutex_t latestDeadReckoningMutex_;

    static const int SKIP_STATUSBAR_UPDATE_;
    static const int READER_THREAD_USLEEP_;

    void initializeMobility_(void);
    void readSonarRanges_(void);
    void readPowerData_(void);
    void readDeadReckoningData_(void);
    void readArmData_(void);
    void addSensors_(void);
    void updateStatusBar_(void);
    void updatePoseFromUrbanOffset_(void);
    void setUrbanPose_(HSPose_t pose);
    void setUrbanPoseOffset_(HSPose_t poseOffset);
    void getUrbanPoseOffset_(HSPose_t &poseOffset);
    void setLatestDeadReckoning_(HSPose_t pose);
    void getLatestDeadReckoning_(HSPose_t &pose);
    void readerLoop_(void);
    void controlRobot_(void);
    void urbanStop_(void);

    static void *startReaderThread_(void *urbanInstance);

public:
    Urban(Robot** a);
    ~Urban(void);
    void control(void);
};


#endif

/**********************************************************************
# $Log: urban.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:28  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2001/05/29 22:36:19  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
