/**********************************************************************
 **                                                                  **
 **                              robot.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Generic robot class for HServer                                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: robot.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef ROBOT_H
#define ROBOT_H

#include <pthread.h>
#include <string.h>

#include "HServerTypes.h"
#include "module.h"
#include "hserver_ipt.h"

#define ROBOT_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}
#define ROBOT_RAD2DEG(val) ((val)*180.0/M_PI)
#define ROBOT_DEG2RAD(val) ((val)*M_PI/180.0)
#define ROBOT_M2MM(val) ((val)*1000.0)
#define ROBOT_M2IN(val) ((val)*39.3700787)
#define ROBOT_IN2M(val) ((val)*0.0254)

using std::string;

class Robot : public Module {

    typedef struct VelocityBuffers_t {
        vector<double> xBuffer;
        vector<double> yBuffer;
        vector<double> zBuffer;
        vector<double> yawBuffer;
        vector<double> pitchBuffer;
        vector<double> rollBuffer;
        bool initialized;
    };

protected:
    HSPose_t shPose_; // Shared
    HSPose_t shLastPose_; // Shared
    HSSpeed_t shControlSpeed_; // Shared
    HSSpeed_t shDesiredSpeed_; // Shared
    HSRotation_t shDesiredRotation_; // Shared
    HSVelocity_t shVelocity_; // Shared
    VelocityBuffers_t velBuffers_;
    double shBatteryVoltage_; // Shared
    double shMinPointTurnAngle_; // Shared
    double battery_level; // To be obsolete
    double maxSteerSpeed_;
    int robotType_;
    int poseCalcID_;
    int velIndex_;
    bool shReversableDrive_; // Shared
    bool velBufferFilled_;
    bool aspirar;

    static pthread_mutex_t poseMutex_;
    static pthread_mutex_t velocityMutex_;
    static pthread_mutex_t controlSpeedMutex_;
    static pthread_mutex_t desiredSpeedMutex_;
    static pthread_mutex_t desiredRotationMutex_;
    static pthread_mutex_t minPointTurnAngleMutex_;
    static pthread_mutex_t reversableDriveMutex_;
    static pthread_mutex_t batteryVoltageMutex_;

    static const double DEFAULT_MIN_POINT_TURN_ANGLE_;
    static const double DEFAULT_MAX_STEER_SPEED_;
    static const double MAX_STEER_SPEED_ANGLE_;
    static const double MAX_NO_COMMAND_TIME_SEC_;
    static const double MIN_STEER_SPEED_;
    static const double STOPPING_SPEED_THRESHOLD_;
    static const int NUM_VELOCITY_SAMPLES_;

    void setPose_(HSPose_t pose);
    void resetPose_(void);
    void advancePose_(HSPose_t deltaPose);
    void advancePose2D_(HSSpeed_t speed, double deltaTimeSec);
    void compControlSpeed_(void);
    void getDesiredSpeed_(HSSpeed_t &desiredSpeed);
    void getDesiredRotation_(HSRotation_t &rot);
    void getReversableDrive_(bool &reversable);
    void getMinimumPointTurnAngle_(double &angle);
    void updateVelocity_(void);
    void logData_(double logTime);
    void setBatteryVoltage_(double batteryVoltage);
    void getBatteryVoltage_(double &batteryVoltage);

    virtual void updateStatusBar_(void) = 0;
    virtual void controlRobot_(void) = 0;

public:
    void getControlSpeed_(HSSpeed_t &speed);
    Robot(void);
    Robot(Robot** a, int robotType, string name);
    double getBattery(void) {return battery_level;}
    void savePoseCalcID(int id);
    void getPose(HSPose_t &pose);
    void setDesiredRotation(HSRotation_t rot);
    void setDesiredSpeed(HSSpeed_t speed);
    void setDesiredDriveSpeed(double driveSpeed);
    void setDesiredSteerSpeed(double steerSpeed);
    void setDesiredArmSpeed(double armSpeed);
    void setReversableDrive(bool reversable);
    void setMinimumPointTurnAngle(double angle);
    void stop(void);
    void setVelocity(HSVelocity_t vel);
    void getVelocity(HSVelocity_t &vel);
    void setMotores(int);
    bool getAspirar();
    int getRobotType(){ return robotType_;}

    virtual ~Robot(void);
    virtual void sonarStart(void) {};
    virtual void sonarStop(void) {};
    virtual void advancedTelop(REC_AdvancedTelop *telop) {};
    virtual void initRobot(void);
    virtual void control(void) = 0;
};

inline void Robot::savePoseCalcID(int id)
{
    poseCalcID_ = id;
}

extern Robot *gRobot;

#endif

/**********************************************************************
# $Log: robot.h,v $
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
# Revision 1.8  2003/04/26 06:48:31  ebeowulf
# Added the Pioneer2-dxe controller.
#
# Revision 1.7  2003/04/02 22:26:24  zkira
# Added battery level variable to class
#
# Revision 1.6  2002/02/18 13:48:21  endo
# AmigoBot added.
#
# Revision 1.5  2002/01/16 22:33:33  ernest
# Added
# m_bposition_set flag.
#
# Revision 1.4  2001/05/29 22:35:28  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.3  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
