/**********************************************************************
 **                                                                  **
 **                             atrvjr.c                             **
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

/* $Id: atrvjr.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#define USE_ATRVJR
#include "HServerTypes.h"
#include "atrvjr.h"
#include "Watchdog.h"
#include "LogManager.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
void exit_hserver(void);

//-----------------------------------------------------------------------
// Mutexes
//-----------------------------------------------------------------------
pthread_mutex_t ATRVJr::atrvjrPoseOffsetMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ATRVJr::latestDeadReckoningMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int ATRVJr::SKIP_STATUSBAR_UPDATE_ = 10;
const int ATRVJr::READER_THREAD_USLEEP_ = 1000;
const int ATRVJr::WATCHDOG_CHECK_INTERVAL_SEC_ = 2;

//-----------------------------------------------------------------------
// Contructor
//-----------------------------------------------------------------------
ATRVJr::ATRVJr(Robot** a) : 
    Robot(a, HS_ROBOT_TYPE_ATRVJR, HS_MODULE_NAME_ROBOT_ATRVJR)
{
    HSPose_t initPose;
    double curTime;

    memset((HSPose_t *)&shAtrvjrPoseOffset_, 0x0, sizeof(HSPose_t));

    // Set the sonar angles.
    float tmp_angle[ATRVJR_NUM_SONAR] = ATRVJR_RANGE_ANGLE;
    memcpy(atrvjr_range_angle, tmp_angle, sizeof(float)*ATRVJR_NUM_SONAR);

    // Set the sonar locations.
    float tmp_loc[ATRVJR_NUM_SONAR] = ATRVJR_RANGE_LOC;
    memcpy(atrvjr_range_loc, tmp_loc, sizeof(float)*ATRVJR_NUM_SONAR);

    initializeMobility_();
 
    pthread_create(&readerThread_, NULL, &startReaderThread_, (void*)this);

    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(
            NAME_,
            WATCHDOG_CHECK_INTERVAL_SEC_,
            getpid(),
            readerThread_);
    }

    // Reset the initial position.
    memset((HSPose_t *)&initPose, 0x0, sizeof(HSPose_t));
    curTime = getCurrentEpochTime();
    initPose.extra.time = curTime;
    initPose.loc.extra.time = curTime;
    initPose.rot.extra.time = curTime;
    setAtrvjrPose_(initPose);

    addSensors_();
    updateStatusBar_();
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
ATRVJr::~ATRVJr(void)
{
    atrvjrStop_();

    pthread_cancel(readerThread_);
    pthread_join(readerThread_, NULL);
    delete sensorSonar_;

    printTextWindow("ATRV-Jr disconnected");
}

//-----------------------------------------------------------------------
// This function starts the reader thread.
//-----------------------------------------------------------------------
void *ATRVJr::startReaderThread_(void *atrvjrInstance)
{
    ((ATRVJr*) atrvjrInstance)->readerLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function sets the offset between the raw ATRV-Jr data and the
// current robot pose.
//-----------------------------------------------------------------------
void ATRVJr::setAtrvjrPoseOffset_(HSPose_t poseOffset)
{
    pthread_mutex_lock(&atrvjrPoseOffsetMutex_);
    shAtrvjrPoseOffset_ = poseOffset;
    pthread_mutex_unlock(&atrvjrPoseOffsetMutex_);
}

//-----------------------------------------------------------------------
// This function returns the offset between the raw ATRV-Jr data and the
// current robot pose.
//-----------------------------------------------------------------------
void ATRVJr::getAtrvjrPoseOffset_(HSPose_t &poseOffset)
{
    pthread_mutex_lock(&atrvjrPoseOffsetMutex_);
    poseOffset = shAtrvjrPoseOffset_;
    pthread_mutex_unlock(&atrvjrPoseOffsetMutex_);
}

//-----------------------------------------------------------------------
// This function saves the latest dead-reckoning data.
//-----------------------------------------------------------------------
void ATRVJr::setLatestDeadReckoning_(HSPose_t pose)
{
    pthread_mutex_lock(&latestDeadReckoningMutex_);
    shLatestDeadReckoning_ = pose;
    pthread_mutex_unlock(&latestDeadReckoningMutex_);
}

//-----------------------------------------------------------------------
// This function returns the saved latest dead-reckoning data.
//-----------------------------------------------------------------------
void ATRVJr::getLatestDeadReckoning_(HSPose_t &pose)
{
    pthread_mutex_lock(&latestDeadReckoningMutex_);
    pose = shLatestDeadReckoning_;
    pthread_mutex_unlock(&latestDeadReckoningMutex_);
}

//-----------------------------------------------------------------------
// This function updates the status bar.
//-----------------------------------------------------------------------
void ATRVJr::updateStatusBar_(void)
{
    HSPose_t pose;
    double frequency;
    char buf[100];
    const bool DISPLAY_FREQUENCY = true;
  
    statusStr[0] = '\0';

    strcat(statusStr, "ATRV-Jr: ");

    getPose(pose);

    sprintf(
        buf,
        "%c %.2fV  x: %.2f  y: %.2f  t: %.2f",
        statusbarSpinner_->getStatus(),
        atrvjr_battery[0],
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
void ATRVJr::initializeMobility_(void)
{
    CORBA::Object_ptr corbaObj;
    mbyClientHelper* moHelper;
    int argc;
    char* argv[4];
    char* robotName = "ATRVJr";
    char pathName[255];

    argv[0] = "hserver";
    argv[1] = "-robot";
    argv[2] = "ATRVJr";
    argv[3] = 0;
    argc = 3;

    moHelper = new mbyClientHelper(argc, argv);

    // Build a pathname to the component we want to use to get sonar data.
    sprintf(pathName, "%s/Sonar/Segment", robotName); // Use robot name arg.

    // Locate the component we want.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we want from the object we found
    moSonarState_ = MobilityGeometry::SegmentState::_narrow(corbaObj); //,env);
    
    // Build a pathname to the component we want to use to get bumper data.
    sprintf(pathName, "%s/Contact/Point", robotName); // Use robot name arg.

    // Locate the component we want.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we want from the object we found
    moBumperState_ = MobilityGeometry::PointState::_narrow(corbaObj); //,env);
    
    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Drive/Command", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moControlCommandState_ = MobilityActuator::ActuatorState::_duplicate(
        MobilityActuator::ActuatorState::_narrow(corbaObj));

    // Build pathname to the component we want to use to drive the robot.
    sprintf(pathName, "%s/Drive/State", robotName); // Use robot name arg.

    // Locate object within robot.
    corbaObj = moHelper->find_object(pathName);

    // Request the interface we need from the object we found.
    moLocationState_ = MobilityActuator::ActuatorState::_duplicate(
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
void ATRVJr::controlRobot_(void)
{
    MobilityActuator::ActuatorData moControlCommand;
    HSSpeed_t controlSpeed;

    getControlSpeed_(controlSpeed);

    // We'll send two axes of command. Axis[0] == translate, Axis[1] == rotate.
    moControlCommand.velocity.length(2);
    moControlCommand.velocity[0] = (float)(controlSpeed.driveSpeed.value);
    moControlCommand.velocity[1] = (float)(ROBOT_DEG2RAD(controlSpeed.steerSpeed.value));
    moControlCommandState_->new_sample(moControlCommand, 0);
}

//-----------------------------------------------------------------------
// This function stops ATRV-Jr directly (as opposed to changing the
// "desired speed" in robot.c).
//-----------------------------------------------------------------------
void ATRVJr::atrvjrStop_(void)
{
    MobilityActuator::ActuatorData moControlCommand;

    moControlCommand.velocity.length(2);
    moControlCommand.velocity[0] = 0;
    moControlCommand.velocity[1] = 0;
    moControlCommandState_->new_sample(moControlCommand, 0);
}

//-----------------------------------------------------------------------
// This function sets the ATRV-Jr pose.
//-----------------------------------------------------------------------
void ATRVJr::setAtrvjrPose_(HSPose_t pose)
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
    
    setAtrvjrPoseOffset_(poseOffset);

    updatePoseFromAtrvjrOffset_();

    delete moLocationStatus;
    moLocationStatus = NULL;
}


//-----------------------------------------------------------------------
// This function retrieves the sonar information from mobility.
//-----------------------------------------------------------------------
void ATRVJr::readSonarRanges_(void)
{
    MobilityGeometry::SegmentData_var moSonarData;
    unsigned int i;

    moSonarData = moSonarState_->get_sample(0);
  
    for (i = 0; i < moSonarData->org.length(); i++)
    {
        // Compute segment lengths.
        atrvjr_sonar[i] = sqrt(
            (moSonarData->org[i].x - moSonarData->end[i].x)*
            (moSonarData->org[i].x - moSonarData->end[i].x)+
            (moSonarData->org[i].y - moSonarData->end[i].y)*
            (moSonarData->org[i].y - moSonarData->end[i].y));

        // If the distance being read was bigger than sonar's detectable range, mark it as
        // no reading. Otherwise, adjust the readings by considering the width of the robot.
        if (atrvjr_sonar[i] > ATRVJR_SONAR_MAX_RANGE)
        {
            atrvjr_sonar[i] = SONAR_READING_OUTSIDE_RANGE;
        }
        else
        {
            atrvjr_sonar[i] += atrvjr_range_loc[i];
        }
    }
}

//-----------------------------------------------------------------------
// This function checks for the bumper and halt the robot if there is
// a contact.
//-----------------------------------------------------------------------
void ATRVJr::checkBumper_(void)
{
    MobilityGeometry::Point3Data_var moBumberData = NULL;
    int i;
    bool bumped = false;

    moBumberData = moBumperState_->get_sample(0);

    for (i = 0; i < (int)(moBumberData->point.length()); i++)
    {
        bumped = (bool)(moBumberData->point[i].flags);

        if (bumped)
        {
            printfTextWindow("ATRV-Jr: Bumped. Halting the robot.\n");
            atrvjrStop_();
            exit_hserver();
        }
    }
}

//-----------------------------------------------------------------------
// This function updates the battery information.
//-----------------------------------------------------------------------
void ATRVJr::readPowerData_(void)
{
    MobilityData::PowerManagementStatus *moPowerStatus = NULL;

    moPowerStatus = moPowerState_->get_sample(0);
    atrvjr_battery[0] = moPowerStatus->SourceVoltage[0];
    battery_level = atrvjr_battery[0] * 10;

    setBatteryVoltage_((double)(atrvjr_battery[0]));

    delete moPowerStatus;
    moPowerStatus = NULL;
}

//-----------------------------------------------------------------------
// This function updates the robot pose from the offset.
//-----------------------------------------------------------------------
void ATRVJr::updatePoseFromAtrvjrOffset_(void)
{
    HSPose_t pose;
    HSPose_t poseOffset;
    double x, y, angle;

    getLatestDeadReckoning_(pose);
    getAtrvjrPoseOffset_(poseOffset);

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
void ATRVJr::readDeadReckoningData_(void)
{
    HSPose_t pose;
    MobilityActuator::ActuatorData *moLocationStatus = NULL;
    double x, y, t;
    double curTime;

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));

    moLocationStatus = moLocationState_->get_sample(0);
    curTime = getCurrentEpochTime();

    pose.loc.x = (double)(moLocationStatus->position[0]);
    pose.loc.y = (double)(moLocationStatus->position[1]);
    pose.rot.yaw = (double)(moLocationStatus->position[2]);
    pose.rot.yaw = ROBOT_RAD2DEG(pose.rot.yaw);
    ROBOT_CRAMPDEG(pose.rot.yaw, 0.0, 360.0);
    pose.extra.time = curTime;
    pose.loc.extra.time = curTime;
    pose.rot.extra.time = curTime;

    setLatestDeadReckoning_(pose);

    updatePoseFromAtrvjrOffset_();

    delete moLocationStatus;
    moLocationStatus = NULL;
}

//-----------------------------------------------------------------------
// This function loops around to aquire sensory data from ATRV-Jr and
// send the control commands to it.
//-----------------------------------------------------------------------
void ATRVJr::readerLoop_(void)
{
    int count = 0;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Check to see if the robot is hitting anything.
        checkBumper_();

        // Read the data from the robot.
        readPowerData_();
        readDeadReckoningData_();
        readSonarRanges_();

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

        // Update the status bar if necessary.
        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }
        count++;

        // Log the data if necessary.
        if (gLogManager != NULL)
        {
            manageLog_(readerThread_);
        }

        // Tell watchdog that the robot is still alive.
        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        usleep(READER_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function adds the sensors.
//-----------------------------------------------------------------------
void ATRVJr::addSensors_(void)
{
    partXyt_ = new RobotPartXyt();

    partSonar_ = new RobotPartSonar(
        ATRVJR_NUM_SONAR,
        atrvjr_range_angle);

    sensorSonar_ = new Sensor(
        SENSOR_SONAR,
        0,
        NULL,
        ATRVJR_NUM_SONAR, 
        atrvjr_range_angle,
        ATRVJR_NUM_SONAR,
        atrvjr_sonar);

    memset((float *)atrvjr_sonar, 0x0, sizeof(float)*ATRVJR_NUM_SONAR);
}

//-----------------------------------------------------------------------
// This function sets up the user interface for the robot option.
//-----------------------------------------------------------------------
void ATRVJr::control(void)
{
    int c;
    int done = 0;

    messageDrawWindow(EnMessageType_ATRVJR_CONTROL, EnMessageErrType_NONE);
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
# $Log: atrvjr.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.5  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.4  2004/05/13 22:43:22  endo
# Angular velocity reduced. Memory Leak fixed.
#
# Revision 1.3  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.2  2004/04/09 18:36:59  endo
# Used memcpy to copy the array.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.6  2003/04/02 22:20:19  zkira
# Battery level for telop GUI
#
# Revision 1.5  2002/01/16 21:40:57  ernest
# Added WatchDog function -Ernest
#
# Revision 1.4  2001/05/29 22:34:10  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.3  2001/03/08 23:32:51  blee
# Fixed a bug so that GPS works and fixed a compiler error.
#
# Revision 1.2  2000/12/12 23:25:17  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.1  2000/11/13 20:09:23  endo
# Initial revision
#
#
#**********************************************************************/
