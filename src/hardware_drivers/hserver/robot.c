/**********************************************************************
 **                                                                  **
 **                              robot.c                             **
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

/* $Id: robot.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <pthread.h>
#include <math.h>
#include <string>
#include <algorithm>
#include <numeric>

#include "message.h"
#include "robot.h"
#include "hserver.h"
#include "PoseCalculatorInterface.h"
#include "sensors.h"
#include "ipc_client.h"

using std::string;


// bound degrees to -180..+180
#define deg_range_pm_180(t) {(t)=fmod((double)(t),360); if((t)>180) (t) -= 360; if((t)<=-180) (t) += 360;}

// bound degrees to [0.. 360)
#define deg_range_pm_360(t) {(t)=fmod((double)(t),360); if((t)<0) (t) += 360;}

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Mutexes
//-----------------------------------------------------------------------
pthread_mutex_t Robot::poseMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::velocityMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::controlSpeedMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::desiredRotationMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::desiredSpeedMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::minPointTurnAngleMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::reversableDriveMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Robot::batteryVoltageMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const double Robot::DEFAULT_MIN_POINT_TURN_ANGLE_ = 180.0;
const double Robot::DEFAULT_MAX_STEER_SPEED_ = 90.0;
const double Robot::MAX_STEER_SPEED_ANGLE_ = 90.0;
const double Robot::MAX_NO_COMMAND_TIME_SEC_ = 2.0;
const double Robot::MIN_STEER_SPEED_ = 6.0;
const double Robot::STOPPING_SPEED_THRESHOLD_ = 0.1;
const int Robot::NUM_VELOCITY_SAMPLES_ = 10;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
Robot::Robot(Robot** a, int robotType, string name) :
   Module((Module**) a, name),
   shBatteryVoltage_(0),
   shMinPointTurnAngle_(DEFAULT_MIN_POINT_TURN_ANGLE_),
   maxSteerSpeed_(DEFAULT_MAX_STEER_SPEED_),
   robotType_(robotType),
   poseCalcID_(-1),
   velIndex_(0),
   shReversableDrive_(false),
   velBufferFilled_(false)
{
    battery_level = -1;
    memset((HSPose_t *)&shPose_, 0x0, sizeof(HSPose_t));
    memset((HSPose_t *)&shLastPose_, 0x0, sizeof(HSPose_t));
    memset((HSVelocity_t *)&shVelocity_, 0x0, sizeof(HSVelocity_t));
    memset((HSSpeed_t *)&shControlSpeed_, 0x0, sizeof(HSSpeed_t));
    memset((HSSpeed_t *)&shDesiredSpeed_, 0x0, sizeof(HSSpeed_t));
    memset((HSRotation_t *)&shDesiredRotation_, 0x0, sizeof(HSRotation_t));
    memset((VelocityBuffers_t *)&velBuffers_, 0x0, sizeof(VelocityBuffers_t));

    aspirar = false;
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
Robot::~Robot(void)
{
    stop();

    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->disconnectModule(poseCalcID_);
        poseCalcID_ = -1;
    }
}

//-----------------------------------------------------------------------
// This function initializes the robot. It is a virtual function which
// can be overwritten by a derived robot.
//-----------------------------------------------------------------------
void Robot::initRobot(void)
{
    //resetPose_();
}

//-----------------------------------------------------------------------
// This function returns the current robot pose.
//-----------------------------------------------------------------------
void Robot::getPose(HSPose_t &pose)
{
    pthread_mutex_lock(&poseMutex_);
    pose = shPose_;
    pthread_mutex_unlock(&poseMutex_);
}

//-----------------------------------------------------------------------
// This function sets the pose to be the specified value.
//-----------------------------------------------------------------------
void Robot::setPose_(HSPose_t pose)
{
    pthread_mutex_lock(&poseMutex_);
    shLastPose_ = shPose_;
    shPose_ = pose;

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
    	ipc_send_base_odometry(pose.loc.x,
			pose.loc.y,
			pose.rot.yaw * M_PI / 180,
			shDesiredSpeed_.driveSpeed.value,
			shDesiredSpeed_.steerSpeed.value * M_PI / 180);
    }

    pthread_mutex_unlock(&poseMutex_);

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
		for(int i = 0; i < sensors->numSensors(); i++)
		{
			if(sensors->latest_sensor[i]->getType() == SENSOR_SONAR)
			{
				int numReadings;
				float *readings;
				sensors->latest_sensor[i]->getReadingInfo(readings, numReadings);
				ipc_send_base_sonar(numReadings, readings,
						sensors->latest_sensor[i]->getLenExtra(), (float *)sensors->latest_sensor[i]->getExtra());
				delete readings;
			}
		}
    }

    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->updateModulePose(poseCalcID_, pose);
    }
}

//-----------------------------------------------------------------------
// This function updates the pose based on the specified increments.
//-----------------------------------------------------------------------
void Robot::advancePose_(HSPose_t deltaPose)
{
    HSPose_t pose;
    double curTime;

    curTime = getCurrentEpochTime();



    pthread_mutex_lock(&poseMutex_);

    shLastPose_ = shPose_;
    pose = shPose_;
    pose.loc.x += deltaPose.loc.x;
    pose.loc.y += deltaPose.loc.y;
    pose.loc.z += deltaPose.loc.z;
    pose.rot.yaw += deltaPose.rot.yaw;
    deg_range_pm_360(pose.rot.yaw);
    pose.rot.pitch += deltaPose.rot.pitch;
    deg_range_pm_360(pose.rot.pitch);
    pose.rot.roll += deltaPose.rot.roll;
    deg_range_pm_360(pose.rot.roll);
    pose.loc.extra.time = curTime;
    pose.rot.extra.time = curTime;
    pose.extra.time = curTime;
    shPose_ = pose;

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
    	ipc_send_base_odometry(pose.loc.x,
			pose.loc.y,
			pose.rot.yaw * M_PI / 180,
			shDesiredSpeed_.driveSpeed.value,
			shDesiredSpeed_.steerSpeed.value * M_PI / 180);
    }

    pthread_mutex_unlock(&poseMutex_);

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
		for(int i = 0; i < sensors->numSensors(); i++)
		{
			if(sensors->latest_sensor[i]->getType() == SENSOR_SONAR)
			{
				int numReadings;
				float *readings;
				sensors->latest_sensor[i]->getReadingInfo(readings, numReadings);
				ipc_send_base_sonar(numReadings, readings,
						sensors->latest_sensor[i]->getLenExtra(), (float *)sensors->latest_sensor[i]->getExtra());
				delete readings;
			}
		}
    }

    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
#ifdef LOG_POSE_DATA
    	fprintf(stderr, "ROBOT: deltaX=%f deltaY=%f deltaTheta=%f -> X=%f Y=%f Theta=%f - time=%f\n", deltaPose.loc.x, deltaPose.loc.y, deltaPose.rot.yaw,
    			pose.loc.x, pose.loc.y, pose.rot.yaw, curTime);
#endif
        gPoseCalc->updateModulePose(poseCalcID_, pose);
    }
}

//-----------------------------------------------------------------------
// This function updates the pose based on the specified speed and delta
// time (considered in 2D).
//-----------------------------------------------------------------------
void Robot::advancePose2D_(HSSpeed_t speed, double deltaTimeSec)
{
    HSPose_t pose;
    double dist, angle, dx, dy;
    double curTime;

    dist  = speed.driveSpeed.value*deltaTimeSec;
    angle = speed.steerSpeed.value*deltaTimeSec;
    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&poseMutex_);
    shLastPose_ = shPose_;
    pose = shPose_;

    dy = sin(ROBOT_DEG2RAD(pose.rot.yaw+(angle/2.0)))*dist;
    dx = cos(ROBOT_DEG2RAD(pose.rot.yaw+(angle/2.0)))*dist;

    pose.loc.x += dx;
    pose.loc.y += dy;
    pose.rot.yaw += angle;
    deg_range_pm_360(pose.rot.yaw);
    pose.loc.extra.time = curTime;
    pose.rot.extra.time = curTime;
    pose.extra.time = curTime;

    shPose_ = pose;

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
    	ipc_send_base_odometry(pose.loc.x,
			pose.loc.y,
			pose.rot.yaw * M_PI / 180,
			shDesiredSpeed_.driveSpeed.value,
			shDesiredSpeed_.steerSpeed.value * M_PI / 180);
    }

    pthread_mutex_unlock(&poseMutex_);

    if(robotType_ != HS_ROBOT_TYPE_CARMEN)
    {
		for(int i = 0; i < sensors->numSensors(); i++)
		{
			if(sensors->latest_sensor[i]->getType() == SENSOR_SONAR)
			{
				int numReadings;
				float *readings;
				sensors->latest_sensor[i]->getReadingInfo(readings, numReadings);
				ipc_send_base_sonar(numReadings, readings,
						sensors->latest_sensor[i]->getLenExtra(), (float *)sensors->latest_sensor[i]->getExtra());
				delete readings;
			}
		}
    }

    if ((gPoseCalc != NULL) && (poseCalcID_ >= 0))
    {
        gPoseCalc->updateModulePose(poseCalcID_, pose);
    }
}

//-----------------------------------------------------------------------
// This function sets the pose to be zero.
//-----------------------------------------------------------------------
void Robot::resetPose_(void)
{
    HSPose_t pose;
    double curTime;

    curTime = getCurrentEpochTime();

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    pose.extra.time = curTime;
    pose.loc.extra.time = curTime;
    pose.loc.extra.time = curTime;

    setPose_(pose);
}

//-----------------------------------------------------------------------
// This function sets the desired rotation of the robot with respect to
// the world coordinate (Note: East = 0).
//-----------------------------------------------------------------------
void Robot::setDesiredRotation(HSRotation_t rot)
{
    pthread_mutex_lock(&desiredRotationMutex_);
    shDesiredRotation_ = rot;
    pthread_mutex_unlock(&desiredRotationMutex_);
}

//-----------------------------------------------------------------------
// This function returns the desired rotation of the robot with respect to
// the world coordinate (Note: East = 0).
//-----------------------------------------------------------------------
void Robot::getDesiredRotation_(HSRotation_t &rot)
{
    pthread_mutex_lock(&desiredRotationMutex_);
    rot = shDesiredRotation_;
    pthread_mutex_unlock(&desiredRotationMutex_);
}

//-----------------------------------------------------------------------
// This function sets the desired speed. This speed may not be the exact
// speed that would be used to control the robot, depending on
// compControlSpeed_() below.
//-----------------------------------------------------------------------
void Robot::setDesiredSpeed(HSSpeed_t speed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&desiredSpeedMutex_);
    shDesiredSpeed_ = speed;
    pthread_mutex_unlock(&desiredSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the desired drive speed. This speed may not be
// the exact speed that would be used to control the robot, depending
// on compControlSpeed_() below.
//-----------------------------------------------------------------------
void Robot::setDesiredDriveSpeed(double driveSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&desiredSpeedMutex_);
    shDesiredSpeed_.driveSpeed.value = driveSpeed;
    shDesiredSpeed_.driveSpeed.extra.time = curTime;
    shDesiredSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&desiredSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the desired steer speed. This speed may not be
// the exact speed that would be used to control the robot, depending
// on compControlSpeed_() below.
//-----------------------------------------------------------------------
void Robot::setDesiredSteerSpeed(double steerSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&desiredSpeedMutex_);
    shDesiredSpeed_.steerSpeed.value = steerSpeed;
    shDesiredSpeed_.steerSpeed.extra.time = curTime;
    shDesiredSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&desiredSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function sets the desired arm speed. This speed may not be
// the exact speed that would be used to control the robot, depending
// on compControlSpeed_() below.
//-----------------------------------------------------------------------
void Robot::setDesiredArmSpeed(double armSpeed)
{
    double curTime;

    curTime = getCurrentEpochTime();

    pthread_mutex_lock(&desiredSpeedMutex_);
    shDesiredSpeed_.armSpeed.value = armSpeed;
    shDesiredSpeed_.armSpeed.extra.time = curTime;
    shDesiredSpeed_.extra.time = curTime;
    pthread_mutex_unlock(&desiredSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function returns the desired speed. This speed may not be
// the exact speed that would be used to control the robot, depending
// on compControlSpeed_() below.
//-----------------------------------------------------------------------
void Robot::getDesiredSpeed_(HSSpeed_t &desiredSpeed)
{
    pthread_mutex_lock(&desiredSpeedMutex_);
    desiredSpeed = shDesiredSpeed_;
    pthread_mutex_unlock(&desiredSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function computes the input speed of the robot that would be
// commanded.
//-----------------------------------------------------------------------
void Robot::compControlSpeed_(void)
{
    HSSpeed_t desiredSpeed;
    HSRotation_t currentRot;
    HSRotation_t desiredRot;
    double curTime;
    double minPointTurnAngle, desiredHeading, angle, factor, sign;
    bool reversable = false;

    if (gPoseCalc == NULL)
    {
        return;
    }

    curTime = getCurrentEpochTime();
    //setReversableDrive(false);
    getReversableDrive_(reversable);
    getMinimumPointTurnAngle_(minPointTurnAngle);
    getDesiredSpeed_(desiredSpeed);
    getDesiredRotation_(desiredRot);


    // If both desired rotation and desired speed was specified long
    // time ago. Set the control speed (both driving and steering) to
    // be zero. If the desired rotation is requested after desired
    // steering speed is specified, compute the control steering speed
    // based on the rotation.
    if ((desiredRot.extra.time < (curTime - MAX_NO_COMMAND_TIME_SEC_)) &&
        (desiredSpeed.extra.time < (curTime - MAX_NO_COMMAND_TIME_SEC_)))
    {
        desiredSpeed.driveSpeed.value = 0;
        desiredSpeed.steerSpeed.value = 0;
    }
    else if (desiredRot.extra.time > (desiredSpeed.steerSpeed.extra.time))
    {
			gPoseCalc->getRotation(currentRot);
			desiredHeading = desiredRot.yaw - currentRot.yaw;
			deg_range_pm_180(desiredHeading);

			if (reversable)
			{
				if (fabs(desiredHeading) > 90.0)
				{
					desiredHeading += 180.0;
					desiredSpeed.driveSpeed.value *= -1.0;
				}
			}

			deg_range_pm_180(desiredHeading);

			if (fabs(desiredHeading) >= minPointTurnAngle)
			{
				desiredSpeed.driveSpeed.value = 0;
			}

			// Determine the steering speed based on the sine function of the
			// angle from the desired heading.
			sign = ((desiredHeading >= 0)||(desiredHeading<-175))? 1.0 : -1.0;
			angle = fabs(desiredHeading);
			angle = (angle > MAX_STEER_SPEED_ANGLE_)? MAX_STEER_SPEED_ANGLE_ : angle;
			factor = sin(0.5*M_PI*angle/MAX_STEER_SPEED_ANGLE_);
			desiredSpeed.steerSpeed.value = sign * factor* maxSteerSpeed_;

			if (fabs(desiredSpeed.steerSpeed.value) < MIN_STEER_SPEED_)
			{
				if (fabs(angle) >= minPointTurnAngle)
				{
					desiredSpeed.driveSpeed.value = 0;
					printfTextWindow("Warning: Minimum point-turn angle (%f) too small.\n", minPointTurnAngle);
				}
				else
				{
					desiredSpeed.steerSpeed.value = 0.0;
				}
			}
    }
    else
    {
        // Use the desired speed as the control speed.
    }
    pthread_mutex_lock(&controlSpeedMutex_);
    shControlSpeed_ = desiredSpeed;
    pthread_mutex_unlock(&controlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function returns the input speed of the robot that would be
// commanded.
//-----------------------------------------------------------------------
void Robot::getControlSpeed_(HSSpeed_t &speed)
{
    pthread_mutex_lock(&controlSpeedMutex_);
    speed = shControlSpeed_;
    pthread_mutex_unlock(&controlSpeedMutex_);
}

//-----------------------------------------------------------------------
// This function enables (or disables) the revesable mode of the robot.
//-----------------------------------------------------------------------
void Robot::setReversableDrive(bool reversable)
{
    pthread_mutex_lock(&reversableDriveMutex_);
    shReversableDrive_ = reversable;
    pthread_mutex_unlock(&reversableDriveMutex_);
}

//-----------------------------------------------------------------------
// This function returns the status of the revesable mode of the robot.
//-----------------------------------------------------------------------
void Robot::getReversableDrive_(bool &reversable)
{
    pthread_mutex_lock(&reversableDriveMutex_);
    reversable = shReversableDrive_;
    pthread_mutex_unlock(&reversableDriveMutex_);
}

//-----------------------------------------------------------------------
// This function sets the minimum angle that forces the robot to make
// a point turn.
//-----------------------------------------------------------------------
void Robot::setMinimumPointTurnAngle(double angle)
{
    pthread_mutex_lock(&minPointTurnAngleMutex_);
    shMinPointTurnAngle_ = angle;
    pthread_mutex_unlock(&minPointTurnAngleMutex_);
}

//-----------------------------------------------------------------------
// This function returns the minimum angle that forces the robot to make
// a point turn.
//-----------------------------------------------------------------------
void Robot::getMinimumPointTurnAngle_(double &angle)
{
    pthread_mutex_lock(&minPointTurnAngleMutex_);
    angle = shMinPointTurnAngle_;
    pthread_mutex_unlock(&minPointTurnAngleMutex_);
}

//-----------------------------------------------------------------------
// This function stops the robot by setting the drive and steer speeds
// to be zero.
//-----------------------------------------------------------------------
void Robot::stop(void)
{
    setDesiredDriveSpeed(0);
    setDesiredSteerSpeed(0);
    setDesiredArmSpeed(0);
}

//-----------------------------------------------------------------------
// This function computes the velocity of the robot.
//-----------------------------------------------------------------------
void Robot::updateVelocity_(void)
{
    HSPose_t pose, lastPose, delta;
    HSVelocity_t aveVel;
    double angle, diffAngle;
    double frequency;
    double curTime;
    double sum;
    double transSpeed;

    curTime = getCurrentEpochTime();

    memset((HSVelocity_t *)&aveVel, 0x0, sizeof(HSVelocity_t));
    aveVel.extra.time = curTime;
    aveVel.transVel.extra.time = curTime;
    aveVel.angVel.extra.time = curTime;

    // Get the frequency.
    getFrequency_(frequency);

    if (frequency == 0)
    {
        // The velocity not yet ready to be computed.
        return;
    }

    if (!(velBuffers_.initialized))
    {
        velBuffers_.xBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.yBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.zBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.yawBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.pitchBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.rollBuffer.resize(NUM_VELOCITY_SAMPLES_);
        velBuffers_.initialized = true;
        velIndex_ = 0;
        velBufferFilled_ = false;
    }

    // Copy the current and previous poses.
    pthread_mutex_lock(&poseMutex_);
    pose = shPose_;
    lastPose = shLastPose_;
    pthread_mutex_unlock(&poseMutex_);

    // Get the differences.
    delta.loc.x = pose.loc.x - lastPose.loc.x;
    delta.loc.y = pose.loc.y - lastPose.loc.y;
    delta.loc.z = pose.loc.z - lastPose.loc.z;
    delta.rot.yaw = pose.rot.yaw - lastPose.rot.yaw;
    delta.rot.pitch = pose.rot.pitch - lastPose.rot.pitch;
    delta.rot.roll = pose.rot.roll - lastPose.rot.roll;

    // Assumption: No robot will turn more than 180 degree per cycle.
    // This assumption is necessary as there is no way to tell whether
    // the robot turned 270 degree or - 90 degree by just comparing
    // the yaw value.
    deg_range_pm_180(delta.rot.yaw);
    deg_range_pm_180(delta.rot.pitch);
    deg_range_pm_180(delta.rot.roll);

    // Compute the current velocity.
    velBuffers_.xBuffer[velIndex_] = (delta.loc.x)*frequency;
    velBuffers_.yBuffer[velIndex_] = (delta.loc.y)*frequency;
    velBuffers_.zBuffer[velIndex_] = (delta.loc.z)*frequency;
    velBuffers_.yawBuffer[velIndex_] = (delta.rot.yaw)*frequency;
    velBuffers_.pitchBuffer[velIndex_] = (delta.rot.pitch)*frequency;
    velBuffers_.rollBuffer[velIndex_] = (delta.rot.roll)*frequency;
    velIndex_++;

    if (velIndex_ >= NUM_VELOCITY_SAMPLES_)
    {
        velBufferFilled_ = true;
        velIndex_ = 0;
    }

    // Compute the average velocity over the buffers.
    if (velBufferFilled_)
    {
        sum = accumulate(
            velBuffers_.xBuffer.begin(),
            velBuffers_.xBuffer.end(),
            0.0);
        aveVel.transVel.x = sum/(double)NUM_VELOCITY_SAMPLES_;

        sum = accumulate(
            velBuffers_.yBuffer.begin(),
            velBuffers_.yBuffer.end(),
            0.0);
        aveVel.transVel.y = sum/(double)NUM_VELOCITY_SAMPLES_;

        sum = accumulate(
            velBuffers_.zBuffer.begin(),
            velBuffers_.zBuffer.end(),
            0.0);
        aveVel.transVel.z = sum/(double)NUM_VELOCITY_SAMPLES_;

        sum = accumulate(
            velBuffers_.yawBuffer.begin(),
            velBuffers_.yawBuffer.end(),
            0.0);
        aveVel.angVel.yaw = sum/(double)NUM_VELOCITY_SAMPLES_;

        sum = accumulate(
            velBuffers_.pitchBuffer.begin(),
            velBuffers_.pitchBuffer.end(),
            0.0);
        aveVel.angVel.pitch = sum/(double)NUM_VELOCITY_SAMPLES_;

        sum = accumulate(
            velBuffers_.rollBuffer.begin(),
            velBuffers_.rollBuffer.end(),
            0.0);
        aveVel.angVel.roll = sum/(double)NUM_VELOCITY_SAMPLES_;
    }
    else
    {
        sum = accumulate(
            velBuffers_.xBuffer.begin(),
            (velBuffers_.xBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.transVel.x = sum/(double)velIndex_;

        sum = accumulate(
            velBuffers_.yBuffer.begin(),
            (velBuffers_.yBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.transVel.y = sum/(double)velIndex_;

        sum = accumulate(
            velBuffers_.zBuffer.begin(),
            (velBuffers_.zBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.transVel.z = sum/(double)velIndex_;

        sum = accumulate(
            velBuffers_.yawBuffer.begin(),
            (velBuffers_.yawBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.angVel.yaw = sum/(double)velIndex_;

        sum = accumulate(
            velBuffers_.pitchBuffer.begin(),
            (velBuffers_.pitchBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.angVel.pitch = sum/(double)velIndex_;

        sum = accumulate(
            velBuffers_.rollBuffer.begin(),
            (velBuffers_.rollBuffer.begin()) + (velIndex_-1),
            0.0);
        aveVel.angVel.roll = sum/(double)velIndex_;
    }

    // Now, check for the backwardness. First, get the angle that the
    // system moved.
    transSpeed = sqrt(pow(aveVel.transVel.x, 2)+pow(aveVel.transVel.y, 2));
    if (transSpeed > STOPPING_SPEED_THRESHOLD_)
    {
        angle = ROBOT_RAD2DEG(atan2(aveVel.transVel.y, aveVel.transVel.x));

        // Get the difference between those two.
        deg_range_pm_360(pose.rot.yaw);
        deg_range_pm_360(angle);
        diffAngle =  angle - pose.rot.yaw;
        deg_range_pm_180(diffAngle);

        // It is moving backward if the difference is more than 90 degrees.
        aveVel.extra.backward = (fabs(diffAngle) > 90.0)? true : false;
    }

    setVelocity(aveVel);
}

//-----------------------------------------------------------------------
// This function updates the velocity info.
//-----------------------------------------------------------------------
void Robot::setVelocity(HSVelocity_t vel)
{
    pthread_mutex_lock(&velocityMutex_);
    shVelocity_ = vel;
    pthread_mutex_unlock(&velocityMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the velocity info.
//-----------------------------------------------------------------------
void Robot::getVelocity(HSVelocity_t &vel)
{
    pthread_mutex_lock(&velocityMutex_);
    vel = shVelocity_;
    pthread_mutex_unlock(&velocityMutex_);
}

//-----------------------------------------------------------------------
// This function sets the battery voltage.
//-----------------------------------------------------------------------
void Robot::setBatteryVoltage_(double batteryVoltage)
{
    pthread_mutex_lock(&batteryVoltageMutex_);
    shBatteryVoltage_ = batteryVoltage;
    pthread_mutex_unlock(&batteryVoltageMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the battery voltage.
//-----------------------------------------------------------------------
void Robot::getBatteryVoltage_(double &batteryVoltage)
{
    pthread_mutex_lock(&batteryVoltageMutex_);
    batteryVoltage = shBatteryVoltage_;
    pthread_mutex_unlock(&batteryVoltageMutex_);
}

//-----------------------------------------------------------------------
// This function logs the data.
//-----------------------------------------------------------------------
void Robot::logData_(double logTime)
{
    HSPose_t pose;
    HSSpeed_t speed;
    double batteryVoltage;

    if (logfile_ != NULL)
    {
        getPose(pose);
        getControlSpeed_(speed);
        getBatteryVoltage_(batteryVoltage);

        fprintf(
            logfile_,
            "%f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %f %f %f %.2f %.2f %.2f %f\n",
            logTime,
            batteryVoltage,
            pose.loc.x,
            pose.loc.y,
            pose.loc.z,
            pose.rot.yaw,
            pose.rot.pitch,
            pose.rot.roll,
            pose.extra.time,
            pose.loc.extra.time,
            pose.rot.extra.time,
            speed.driveSpeed.value,
            speed.steerSpeed.value,
            speed.armSpeed.value,
            speed.extra.time);

        fflush(logfile_);
    }
}

void Robot::setMotores(int val){
}

bool Robot::getAspirar(){
    return aspirar;
}

/**********************************************************************
# $Log: robot.c,v $
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
# Revision 1.9  2003/04/02 22:25:20  zkira
# Added battery level variable to class
#
# Revision 1.8  2002/01/16 22:29:42  ernest
# Added flag  m_bposition_set.
#
# Revision 1.7  2001/12/22 16:35:24  endo
# RH 7.1 porting.
#
# $Log: robot.c,v $
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
# Revision 1.9  2003/04/02 22:25:20  zkira
# Added battery level variable to class
#
# Revision 1.8  2002/01/16 22:29:42  ernest
# Added flag  m_bposition_set.
#
# Revision 1.6  2001/05/29 22:35:15  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.5  2001/03/23 21:32:03  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 23:29:29  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
