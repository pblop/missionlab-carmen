 /**********************************************************************
 **                                                                  **
 **                              urban.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **  Revised by:  Yoichiro Endo                                      **
 **                                                                  **
 **  RWI Urban Robot control for HServer                             **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: urban.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#define USE_URBAN
#include "HServerTypes.h"
#include "urban.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Mutexes
//-----------------------------------------------------------------------
pthread_mutex_t Urban::urbanPoseOffsetMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Urban::latestDeadReckoningMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int Urban::SKIP_STATUSBAR_UPDATE_ = 10;
const int Urban::READER_THREAD_USLEEP_ = 1000;

//-----------------------------------------------------------------------
// Contructor
//-----------------------------------------------------------------------
Urban::Urban(Robot** a) : 
    Robot(a, HS_ROBOT_TYPE_URBAN, HS_MODULE_NAME_ROBOT_URBAN)
{
    HSPose_t initPose;
    double curTime;

    memset((HSPose_t *)&shUrbanPoseOffset_, 0x0, sizeof(HSPose_t));

    // Set the sonar angles.
    float tmp_angle[URBAN_NUM_SONAR] = URBAN_RANGE_ANGLE;
    memcpy(urban_range_angle, tmp_angle, sizeof(float)*URBAN_NUM_SONAR);

    // Set the sonar locations.
    float tmp_loc[URBAN_NUM_SONAR] = URBAN_RANGE_LOC;
    memcpy(urban_range_loc, tmp_loc, sizeof(float)*URBAN_NUM_SONAR);

    initializeMobility_();
 
    pthread_create(&readerThread_, NULL, &startReaderThread_, (void*)this);

    // Reset the initial position.
    memset((HSPose_t *)&initPose, 0x0, sizeof(HSPose_t));
    curTime = getCurrentEpochTime();
    initPose.extra.time = curTime;
    initPose.loc.extra.time = curTime;
    initPose.rot.extra.time = curTime;
    setUrbanPose_(initPose);

    addSensors_();
    updateStatusBar_();
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Urban::~Urban(void)
{
    urbanStop_();

    pthread_cancel(readerThread_);
    pthread_join(readerThread_, NULL);
    delete sensorSonar_;
    delete sensorArm_;

    printTextWindow("Urban disconnected");
}

//-----------------------------------------------------------------------
// This function starts the reader thread.
//-----------------------------------------------------------------------
void *Urban::startReaderThread_(void *urbanInstance)
{
    ((Urban*) urbanInstance)->readerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function sets the offset between the raw Urban data and the
// current robot pose.
//-----------------------------------------------------------------------
void Urban::setUrbanPoseOffset_(HSPose_t poseOffset)
{
    pthread_mutex_lock(&urbanPoseOffsetMutex_);
    shUrbanPoseOffset_ = poseOffset;
    pthread_mutex_unlock(&urbanPoseOffsetMutex_);
}

//-----------------------------------------------------------------------
// This function returns the offset between the raw Urban data and the
// current robot pose.
//-----------------------------------------------------------------------
void Urban::getUrbanPoseOffset_(HSPose_t &poseOffset)
{
    pthread_mutex_lock(&urbanPoseOffsetMutex_);
    poseOffset = shUrbanPoseOffset_;
    pthread_mutex_unlock(&urbanPoseOffsetMutex_);
}

//-----------------------------------------------------------------------
// This function saves the latest dead-reckoning data.
//-----------------------------------------------------------------------
void Urban::setLatestDeadReckoning_(HSPose_t pose)
{
    pthread_mutex_lock(&latestDeadReckoningMutex_);
    shLatestDeadReckoning_ = pose;
    pthread_mutex_unlock(&latestDeadReckoningMutex_);
}

//-----------------------------------------------------------------------
// This function returns the saved latest dead-reckoning data.
//-----------------------------------------------------------------------
void Urban::getLatestDeadReckoning_(HSPose_t &pose)
{
    pthread_mutex_lock(&latestDeadReckoningMutex_);
    pose = shLatestDeadReckoning_;
    pthread_mutex_unlock(&latestDeadReckoningMutex_);
}

//-----------------------------------------------------------------------
// This function updates the status bar.
//-----------------------------------------------------------------------
void Urban::updateStatusBar_(void)
{
    HSPose_t pose;
    double frequency;
    char buf[100];
    const bool DISPLAY_FREQUENCY = true;
  
    statusStr[0] = 0;

    strcat(statusStr, "Urban: ");

    getPose(pose);

    sprintf(
        buf,
        "%c %.2fV  x: %.2f  y: %.2f  t: %.2f",
        statusbarSpinner_->getStatus(),
        urban_battery[0],
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
// This function initializes the mobility.
//-----------------------------------------------------------------------
void Urban::initializeMobility_(void)
{
    CORBA::Object_ptr corbaObj;
    mbyClientHelper* moHelper;
    int argc;
    char* argv[4];
    char* robotName = "Urban";
    char pathName[255];

    argv[0] = "hserver";
    argv[1] = "-robot";
    argv[2] = "Urban";
    argv[3] = 0;
    argc = 3;

    moHelper = new mbyClientHelper(argc, argv);

    // Build a pathname to the component we want to use to get sensor data.
    sprintf(pathName, "%s/Sonar/Segment", robotName); // Use robot name arg.

    // Locate the component we want.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we want from the object we found
    moSonarState_ = MobilityGeometry::SegmentState::_narrow(corbaObj); //,env);
    
    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Drive/Command", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moControlCommandState_ = MobilityActuator::ActuatorState::_duplicate(
        MobilityActuator::ActuatorState::_narrow(corbaObj));

    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Arm/Command", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moArmCommandState_ = MobilityActuator::ActuatorState::_duplicate(
		MobilityActuator::ActuatorState::_narrow(corbaObj));

    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Drive/State", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moLocationState_ = MobilityActuator::ActuatorState::_duplicate(
        MobilityActuator::ActuatorState::_narrow(corbaObj));

    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Arm/State", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moArmState_ = MobilityActuator::ActuatorState::_duplicate(
		MobilityActuator::ActuatorState::_narrow(corbaObj));

    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Power", robotName); // Use robot name arg.
  
    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);
  
    // Request the interface we need from the object we found.
    moPowerState_ = MobilityData::PowerManagementState::_duplicate(
        MobilityData::PowerManagementState::_narrow(corbaObj));
}  

//-----------------------------------------------------------------------
// This function controls the robot with the control speed.
//-----------------------------------------------------------------------
void Urban::controlRobot_(void)
{
    MobilityActuator::ActuatorData moControlCommand;
    MobilityActuator::ActuatorData moArmCommand;
    HSSpeed_t controlSpeed;

    getControlSpeed_(controlSpeed);

    // Drive and Steer.
    moControlCommand.velocity.length(2);
    moControlCommand.velocity[0] = (float)(controlSpeed.driveSpeed.value);
    moControlCommand.velocity[1] = (float)(ROBOT_DEG2RAD(controlSpeed.steerSpeed.value));
    moControlCommandState_->new_sample(moControlCommand, 0);

    // Arm
    moArmCommand.velocity.length(1);
    moArmCommand.velocity[0] = (float)(controlSpeed.armSpeed.value);
    moArmCommandState_->new_sample(moArmCommand, 0);  
}

//-----------------------------------------------------------------------
// This function stops Urby directly (as opposed to changing the
// "desired speed" in robot.c).
//-----------------------------------------------------------------------
void Urban::urbanStop_(void)
{
    MobilityActuator::ActuatorData moControlCommand;
    MobilityActuator::ActuatorData moArmCommand;

    moControlCommand.velocity.length(2);
    moControlCommand.velocity[0] = 0;
    moControlCommand.velocity[1] = 0;
    moControlCommandState_->new_sample(moControlCommand, 0);

    moArmCommand.velocity.length(1);
    moArmCommand.velocity[0] = 0;
    moArmCommandState_->new_sample(moArmCommand, 0);  
}


//-----------------------------------------------------------------------
// This function sets the Urban pose.
//-----------------------------------------------------------------------
void Urban::setUrbanPose_(HSPose_t pose)
{
    HSPose_t rawPose;
    HSPose_t poseOffset;
    MobilityActuator::ActuatorData *moLocationStatus = NULL;
    double angle, dx, dy;
    double curTime;

    // Reset the data.
    memset((HSPose_t *)&rawPose, 0x0, sizeof(HSPose_t));
    memset((HSPose_t *)&poseOffset, 0x0, sizeof(HSPose_t));

    // Get the raw pose.
    moLocationStatus = moLocationState_->get_sample(0);
    curTime = getCurrentEpochTime();
    rawPose.loc.x = (double)(moLocationStatus->position[0]);
    rawPose.loc.y = (double)(moLocationStatus->position[1]);
    rawPose.rot.yaw = (double)(moLocationStatus->position[2]);
    rawPose.rot.yaw = ROBOT_RAD2DEG(rawPose.rot.yaw);
    ROBOT_CRAMPDEG(rawPose.rot.yaw, 0.0, 360.0);
    rawPose.extra.time = curTime;
    rawPose.loc.extra.time = curTime;
    rawPose.rot.extra.time = curTime;

    // Compute the offset.
    poseOffset.rot.yaw = pose.rot.yaw - rawPose.rot.yaw;
    ROBOT_CRAMPDEG(poseOffset.rot.yaw, -180.0, 180.0);

    angle = poseOffset.rot.yaw;
    ROBOT_CRAMPDEG(angle, 0.0, 360.0);
    angle = ROBOT_DEG2RAD(angle);
    dx = (rawPose.loc.x*cos(angle)) - (rawPose.loc.y*sin(angle));
    dy = (rawPose.loc.x*sin(angle)) + (rawPose.loc.y*cos(angle));
    
    poseOffset.loc.x = pose.loc.x - dx;
    poseOffset.loc.y = pose.loc.y - dy;

    poseOffset.extra = rawPose.extra;
    poseOffset.loc.extra = rawPose.loc.extra;
    poseOffset.rot.extra = rawPose.rot.extra;
    
    setUrbanPoseOffset_(poseOffset);

    updatePoseFromUrbanOffset_();

    delete moLocationStatus;
    moLocationStatus = NULL;
}


//-----------------------------------------------------------------------
// This function reads the arm data.
//-----------------------------------------------------------------------
void Urban::readArmData_(void)
{
    MobilityActuator::ActuatorData *moArmStatus = NULL;
    double angle, dx, dy;

    // Get the raw pose.
    moArmStatus = moArmState_->get_sample(0);
    urban_arm[0] = moArmStatus->position[0];

    delete moArmStatus;
    moArmStatus = NULL;
}


//-----------------------------------------------------------------------
// This function retrieves the sonar information from mobility.
//-----------------------------------------------------------------------
void Urban::readSonarRanges_(void)
{
    MobilityGeometry::SegmentData_var moSonarData;
    unsigned int i;

    moSonarData = moSonarState_->get_sample(0);
  
    for (i = 0; i < moSonarData->org.length(); i++)
    {
        // Compute segment lengths.
        urban_sonar[i] = sqrt(
            (moSonarData->org[i].x - moSonarData->end[i].x)*
            (moSonarData->org[i].x - moSonarData->end[i].x)+
            (moSonarData->org[i].y - moSonarData->end[i].y)*
            (moSonarData->org[i].y - moSonarData->end[i].y));

        // If the distance being read was bigger than sonar's detectable range, mark it as
        // no reading. Otherwise, adjust the readings by considering the width of the robot.
        if (urban_sonar[i] > URBAN_SONAR_MAX_RANGE)
        {
            urban_sonar[i] = SONAR_READING_OUTSIDE_RANGE;
        }
        else
        {
            urban_sonar[i] += urban_range_loc[i];
        }
    }
}

//-----------------------------------------------------------------------
// This function updates the battery information.
//-----------------------------------------------------------------------
void Urban::readPowerData_(void)
{
    MobilityData::PowerManagementStatus *moPowerStatus = NULL;

    moPowerStatus = moPowerState_->get_sample(0);
    urban_battery[0] = moPowerStatus->SourceVoltage[0];
    battery_level = urban_battery[0] * 10;

    delete moPowerStatus;
    moPowerStatus = NULL;
}

//-----------------------------------------------------------------------
// This function updates the robot pose from the offset.
//-----------------------------------------------------------------------
void Urban::updatePoseFromUrbanOffset_(void)
{
    HSPose_t pose;
    HSPose_t poseOffset;
    double x, y, angle;

    getLatestDeadReckoning_(pose);
    getUrbanPoseOffset_(poseOffset);

    x = pose.loc.x;
    y = pose.loc.y;

    angle = poseOffset.rot.yaw;
    ROBOT_CRAMPDEG(angle, -180.0, 180.0);
    angle = ROBOT_DEG2RAD(angle);

    pose.loc.x = (x*cos(angle)) - (y*sin(angle)) + poseOffset.loc.x;
    pose.loc.y = (x*sin(angle)) + (y*cos(angle)) + poseOffset.loc.y;
    pose.rot.yaw += poseOffset.rot.yaw;
    ROBOT_CRAMPDEG(pose.rot.yaw, 0.0, 360.0);

    setPose_(pose);
}

//-----------------------------------------------------------------------
// This function updates the current postion of the robot.
//-----------------------------------------------------------------------
void Urban::readDeadReckoningData_(void)
{
    HSPose_t pose;
    MobilityActuator::ActuatorData *moLocationStatus = NULL;
    double x, y, t;
    double curTime;

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    curTime = getCurrentEpochTime();

    moLocationStatus = moLocationState_->get_sample(0);

    pose.loc.x = (double)(moLocationStatus->position[0]);
    pose.loc.y = (double)(moLocationStatus->position[1]);
    pose.rot.yaw = (double)(moLocationStatus->position[2]);
    pose.rot.yaw = ROBOT_RAD2DEG(pose.rot.yaw);
    ROBOT_CRAMPDEG(pose.rot.yaw, 0.0, 360.0);
    pose.extra.time = curTime;
    pose.loc.extra.time = curTime;
    pose.rot.extra.time = curTime;

    setLatestDeadReckoning_(pose);

    updatePoseFromUrbanOffset_();

    delete moLocationStatus;
    moLocationStatus = NULL;
}

//-----------------------------------------------------------------------
// This function loops around to aquire sensory data from Urban and
// send the control commands to it.
//-----------------------------------------------------------------------
void Urban::readerLoop_(void)
{
    int count = 0;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Read the data from the robot.
        readPowerData_();
        readDeadReckoningData_();
        readSonarRanges_();
        readArmData_();

        // Check for the pthread termination again.
        pthread_testcancel();

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

        usleep(READER_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function adds the sensors.
//-----------------------------------------------------------------------
void Urban::addSensors_(void)
{
    partXyt_ = new RobotPartXyt();

    partSonar_ = new RobotPartSonar(
        URBAN_NUM_SONAR,
        urban_range_angle);

    sensorSonar_ = new Sensor(
        SENSOR_SONAR,
        0,
        NULL,
        URBAN_NUM_SONAR, 
        urban_range_angle,
        URBAN_NUM_SONAR,
        urban_sonar);

    memset((float *)urban_sonar, 0x0, sizeof(float)*URBAN_NUM_SONAR);

    partArm_ = new RobotPartArm();
    sensorArm_ = new Sensor(SENSOR_ARM, 0, NULL, 1, urban_arm);
    urban_arm[0] = 0.0;
}

//-----------------------------------------------------------------------
// This function sets up the user interface for the robot option.
//-----------------------------------------------------------------------
void Urban::control(void)
{
    int c;
    int done = 0;

    messageDrawWindow(EnMessageType_URBAN_CONTROL, EnMessageErrType_NONE);
    redrawWindows();
    do
    {
        c = getch();

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
# $Log: urban.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:08  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.8  2003/04/02 22:28:10  zkira
# Added battery level variable to class
#
# Revision 1.7  2001/05/29 22:36:14  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.6  2000/12/12 23:24:45  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.5  2000/11/13 20:12:59  endo
# Minor compiler warning fix.
#
# Revision 1.4  2000/10/16 21:22:12  endo
# steerToward() adjusted. (Modified at Rockville, MD)
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
