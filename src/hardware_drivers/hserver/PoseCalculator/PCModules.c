/**********************************************************************
 **                                                                  **
 **                            PCModules.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCModules.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <math.h>

#include "hserver.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculator.h"
#include "PCModules.h"
#include "PCModuleFilterInterface.h"
#include "PCModuleFilters.h"
#include "PCUtility.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define POSECALCMODULE_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}
#define POSECALCMODULE_RAD2DEG(val) ((val)*180.0/M_PI)
#define POSECALCMODULE_DEG2RAD(val) ((val)*M_PI/180.0)

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void printfTextWindow(const char* arg, ...);

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string PoseCalcModule::EMPTY_STRING_ = "";

const double PoseCalcModuleRobotShaftLoc::MAXIMUM_STOPPING_TRANS_SPEED_ = 0.1;
const double PoseCalcModuleRobotShaftLoc::MAXIMUM_STOPPING_ANGULAR_SPEED_ = 5.0;
const double PoseCalcModuleRobotShaftLoc::MOVING_LOC_VARIANCE_ = 0.01;
const double PoseCalcModuleRobotShaftLoc::STATIC_LOC_VARIANCE_ = 0.01;
const double PoseCalcModuleRobotShaftLoc::STOPPING_LOC_VARIANCE_ = 0.0001;
const double PoseCalcModuleRobotShaftLoc::WORST_ROT_VARIANCE_ = 129600.0;

const double PoseCalcModuleRobotShaftRot::MAXIMUM_STOPPING_TRANS_SPEED_ = 0.1;
const double PoseCalcModuleRobotShaftRot::MAXIMUM_STOPPING_ANGULAR_SPEED_ = 5.0;
//const double PoseCalcModuleRobotShaftRot::MOVING_ROT_VARIANCE_ = 6.0;
//const double PoseCalcModuleRobotShaftRot::STOPPING_ROT_VARIANCE_ = 6.0;
const double PoseCalcModuleRobotShaftRot::MOVING_ROT_VARIANCE_ = 1.0;
const double PoseCalcModuleRobotShaftRot::STATIC_ROT_VARIANCE_ = 1.0;
const double PoseCalcModuleRobotShaftRot::STOPPING_ROT_VARIANCE_ = 0.01;
const double PoseCalcModuleRobotShaftRot::WORST_LOC_VARIANCE_ = 1000000.0;

const double PoseCalcModuleGyroRot::MAXIMUM_STOPPING_TRANS_SPEED_ = 0.1;
const double PoseCalcModuleGyroRot::MAXIMUM_STOPPING_ANGULAR_SPEED_ = 5.0;
//const double PoseCalcModuleGyroRot::MOVING_ROT_VARIANCE_ = 3.0;
//const double PoseCalcModuleGyroRot::STOPPING_ROT_VARIANCE_ = 3.0;
const double PoseCalcModuleGyroRot::MOVING_ROT_VARIANCE_ = 1.0;
const double PoseCalcModuleGyroRot::STOPPING_ROT_VARIANCE_ = 1.0;
//const double PoseCalcModuleGyroRot::MOVING_ROT_VARIANCE_ = 0.5;
const double PoseCalcModuleGyroRot::STATIC_ROT_VARIANCE_ = 1.0;
//const double PoseCalcModuleGyroRot::STOPPING_ROT_VARIANCE_ = 0.005;
const double PoseCalcModuleGyroRot::WORST_LOC_VARIANCE_ = 1000000.0;
//const double PoseCalcModuleGpsLoc::RT20_0_LOC_VARIANCE_ = 0.04;
//const double PoseCalcModuleGpsLoc::RT20_1_LOC_VARIANCE_ = 0.09;
const double PoseCalcModuleGpsLoc::RT20_0_LOC_VARIANCE_ = 0.2;
const double PoseCalcModuleGpsLoc::RT20_1_LOC_VARIANCE_ = 0.3;
const double PoseCalcModuleGpsLoc::RT20_100_LOC_VARIANCE_ = 0.001;
const double PoseCalcModuleGpsLoc::STATIC_LOC_VARIANCE_ = 0.25;
const double PoseCalcModuleGpsLoc::WORST_LOC_VARIANCE_ = 1000000.0;
const double PoseCalcModuleGpsLoc::WORST_ROT_VARIANCE_ = 129600.0;

//const double PoseCalcModuleGpsRot::RT20_0_ROT_VARIANCE_ = 3.0;
//const double PoseCalcModuleGpsRot::RT20_1_ROT_VARIANCE_ = 6.0;
const double PoseCalcModuleGpsRot::RT20_0_ROT_VARIANCE_ = 1.0;
const double PoseCalcModuleGpsRot::RT20_1_ROT_VARIANCE_ = 4.0;
const double PoseCalcModuleGpsRot::RT20_100_ROT_VARIANCE_ = 0.01;
const double PoseCalcModuleGpsRot::STATIC_ROT_VARIANCE_ = 3600.0;
const double PoseCalcModuleGpsRot::WORST_LOC_VARIANCE_ = 1000000.0;
const double PoseCalcModuleGpsRot::WORST_ROT_VARIANCE_ = 129600.0;
const long int PoseCalcModuleGpsRot::MIN_VALID_ANGULAR_SPEED_SAMPLES_ = 3;
const long int PoseCalcModuleGpsRot::MIN_VALID_TRANS_SPEED_SAMPLES_ = 3;

//const double PoseCalcModuleCompassRot::BEST_ROT_VARIANCE_ = 18.0;
const double PoseCalcModuleCompassRot::BEST_ROT_VARIANCE_ = 360.0;
//const double PoseCalcModuleCompassRot::STATIC_ROT_VARIANCE_ = 18.0;
const double PoseCalcModuleCompassRot::STATIC_ROT_VARIANCE_ = 8100.0;
const double PoseCalcModuleCompassRot::WORST_LOC_VARIANCE_ = 1000000.0;
const double PoseCalcModuleCompassRot::WORST_ROT_VARIANCE_ = 129600.0;
const long int PoseCalcModuleCompassRot::MIN_VALID_ANGULAR_SPEED_SAMPLES_ = 200;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t PoseCalcModule::modulePoseMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for PoseCalcModule
//-----------------------------------------------------------------------
PoseCalcModule::PoseCalcModule(
    int id,
    int sensorClass,
    int sensorType,
    int poseType,
    PoseCalcInterface *poseCalc) :
    poseCalc_(poseCalc),
    moduleFilter_(NULL),
    ID_(id),
    SENSOR_CLASS_(sensorClass),
    SENSOR_TYPE_(sensorType),
    POSE_TYPE_(poseType)
{
    // Initialize the variables.
    memset((HSPose_t *)&shModulePose_, 0x0, sizeof(HSPose_t));

    // Set the filter.
    setFilter_();
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModule
//-----------------------------------------------------------------------
PoseCalcModule::~PoseCalcModule(void)
{
    if (moduleFilter_ != NULL)
    {
        delete moduleFilter_;
        moduleFilter_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function manages the incoming data to have it fixed its
// coordinate, and assign some its confidence value.
//-----------------------------------------------------------------------
void PoseCalcModule::managePose(PoseCalcPoseInfo_t &poseInfo, int varianceScheme)
{
    // Specify the sensor etc.
    poseInfo.sensorClass = SENSOR_CLASS_;
    poseInfo.sensorType = SENSOR_TYPE_;
    poseInfo.poseType = POSE_TYPE_;

    // Filter the incoming data.
    filter_(poseInfo);

    // Fix its coordinate system.
    transform_(poseInfo);

    // Assign the confidence value.
    grade_(poseInfo, varianceScheme);

}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated location.
//-----------------------------------------------------------------------
void PoseCalcModule::grade_(PoseCalcPoseInfo_t &poseInfo, int varianceScheme)
{
    switch (varianceScheme) {

    case POSECALC_VARIANCE_SCHEME_DYNAMIC:
        gradeDynamic_(poseInfo);
        break;

    case POSECALC_VARIANCE_SCHEME_STATIC:
        gradeStatic_(poseInfo);
        break;
    }
}

//-----------------------------------------------------------------------
// This function sets the pose to specified one.
//-----------------------------------------------------------------------
void PoseCalcModule::setPose(HSPose_t pose)
{
    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_ = pose;
    pthread_mutex_unlock(&modulePoseMutex_);
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleRobotShaftLoc
//-----------------------------------------------------------------------
PoseCalcModuleRobotShaftLoc::PoseCalcModuleRobotShaftLoc(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Robot *robot) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_ROBOTSHAFT,
        sensorType,
        POSECALC_POSE_TYPE_LOCATION,
        poseCalc),
    robot_(robot)
{
    memset((HSLocation_t *)&lastInputLoc_, 0x0, sizeof(HSLocation_t));
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleRobotShaftLoc
//-----------------------------------------------------------------------
PoseCalcModuleRobotShaftLoc::~PoseCalcModuleRobotShaftLoc(void)
{
}

//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftLoc::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftLoc::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// location.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftLoc::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    HSPose_t pose;
    HSLocation_t inputLoc;
    double dxLocal, dyLocal, dxPoseCalc, dyPoseCalc;
    double dist, angle, diffAngle, yaw;

    // Get the location
    inputLoc = poseInfo.pose.loc;

    // We cannot simply add dx and dy since the X-Y coordinate
    // for this module is most likely different from the X-Y
    // coordiante that PoseCalc uses. Using the local dx and dy,
    // we determine how much the robot traveled (according to
    // this module), then we project it to the PoseCalc coordinate.

    // Compute the absolute distance that the robot moved.
    dxLocal = inputLoc.x - lastInputLoc_.x;
    dyLocal = inputLoc.y - lastInputLoc_.y;
    dist = sqrt(pow(dxLocal, 2) + pow(dyLocal, 2));

    // Check the difference between the direction that robot moved
    // and the robot heading.
    robot_->getPose(pose);

    POSECALCMODULE_CRAMPDEG(pose.rot.yaw, 0.0, 360.0);
    angle = POSECALCMODULE_RAD2DEG(atan2(dyLocal, dxLocal));
    POSECALCMODULE_CRAMPDEG(angle, 0.0, 360.0);
    diffAngle =  angle - pose.rot.yaw;
    POSECALCMODULE_CRAMPDEG(diffAngle, -180.0, 180.0);

    // Project the distance.
    poseCalc_->getPose(pose);
    yaw = pose.rot.yaw + diffAngle;
    POSECALCMODULE_CRAMPDEG(yaw, 0.0, 360.0);
    dxPoseCalc = dist*cos(POSECALCMODULE_DEG2RAD(yaw));
    dyPoseCalc = dist*sin(POSECALCMODULE_DEG2RAD(yaw));

    // Update the location, and save the value.
    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_.loc.x += dxPoseCalc;
    shModulePose_.loc.y += dyPoseCalc;
    shModulePose_.loc.extra = inputLoc.extra;
    poseInfo.pose = shModulePose_;
    pthread_mutex_unlock(&modulePoseMutex_);

    // Note: This function is only the place where the value of lastInputLoc_
    // should be altered (except in the constructor). Otherwise, it has to be
    // protected by mutex.
    lastInputLoc_ = inputLoc;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated location.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftLoc::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double transSpeed, angSpeed;

    poseInfo.confidence.locConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;

    poseCalc_->getVelocity(vel);
    transSpeed = sqrt(pow(vel.transVel.x, 2) + pow(vel.transVel.y, 2));
    angSpeed = fabs(vel.angVel.yaw);

    if ((transSpeed <= MAXIMUM_STOPPING_TRANS_SPEED_) &&
        (angSpeed <= MAXIMUM_STOPPING_ANGULAR_SPEED_))
    {
        poseInfo.variance.locVariance.x = STOPPING_LOC_VARIANCE_;
        poseInfo.variance.locVariance.y = STOPPING_LOC_VARIANCE_;
        poseInfo.variance.locVariance.z = STOPPING_LOC_VARIANCE_;
    }
    else
    {
        poseInfo.variance.locVariance.x = MOVING_LOC_VARIANCE_;
        poseInfo.variance.locVariance.y = MOVING_LOC_VARIANCE_;
        poseInfo.variance.locVariance.z = MOVING_LOC_VARIANCE_;
    }

    poseInfo.variance.rotVariance.yaw = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = WORST_ROT_VARIANCE_;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated location.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftLoc::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.variance.locVariance.x = STATIC_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = STATIC_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = STATIC_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = WORST_ROT_VARIANCE_;
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleRobotShaftRot
//-----------------------------------------------------------------------
PoseCalcModuleRobotShaftRot::PoseCalcModuleRobotShaftRot(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Robot *robot) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_ROBOTSHAFT,
        sensorType,
        POSECALC_POSE_TYPE_ROTATION,
        poseCalc),
    robot_(robot)
{
    memset((HSRotation_t *)&lastInputRot_, 0x0, sizeof(HSRotation_t));
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleRobotShaftRot
//-----------------------------------------------------------------------
PoseCalcModuleRobotShaftRot::~PoseCalcModuleRobotShaftRot(void)
{
}

//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftRot::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftRot::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftRot::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    HSRotation_t inputRot;
    double deltaYaw;

    // Get the rotation.
    inputRot = poseInfo.pose.rot;

    // Calculate the increment from the last time.
    deltaYaw = inputRot.yaw - lastInputRot_.yaw;

    // Add the increment to this pose.
    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_.rot.yaw += deltaYaw;
    POSECALCMODULE_CRAMPDEG((shModulePose_.rot.yaw),0.0,360.0);
    shModulePose_.rot.extra = inputRot.extra;
    poseInfo.pose = shModulePose_;
    pthread_mutex_unlock(&modulePoseMutex_);

    // Note: This function is only the place where the value of lastInputRot_
    // should be altered (except in the constructor). Otherwise, it has to be
    // protected by mutex.
    lastInputRot_ = inputRot;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftRot::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double transSpeed, angSpeed;

    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;

    poseCalc_->getVelocity(vel);
    transSpeed = sqrt(pow(vel.transVel.x, 2) + pow(vel.transVel.y, 2));
    angSpeed = fabs(vel.angVel.yaw);

    if ((transSpeed <= MAXIMUM_STOPPING_TRANS_SPEED_) &&
        (angSpeed <= MAXIMUM_STOPPING_ANGULAR_SPEED_))
    {
        poseInfo.variance.rotVariance.yaw = STOPPING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.pitch = STOPPING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.roll = STOPPING_ROT_VARIANCE_;
    }
    else
    {
        poseInfo.variance.rotVariance.yaw = MOVING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.pitch = MOVING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.roll = MOVING_ROT_VARIANCE_;
    }

    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleRobotShaftRot::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.variance.rotVariance.yaw = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = STATIC_ROT_VARIANCE_;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleGyroRot
//-----------------------------------------------------------------------
PoseCalcModuleGyroRot::PoseCalcModuleGyroRot(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Gyro *gyro) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_GYRO,
        sensorType,
        POSECALC_POSE_TYPE_ROTATION,
        poseCalc),
    gyro_(gyro)
{
    memset((HSRotation_t *)&lastInputRot_, 0x0, sizeof(HSRotation_t));
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleGyroRot
//-----------------------------------------------------------------------
PoseCalcModuleGyroRot::~PoseCalcModuleGyroRot(void)
{
}

//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGyroRot::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGyroRot::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleGyroRot::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    HSRotation_t inputRot;
    HSRotation_t delta;

    // Get the rotation.
    inputRot = poseInfo.pose.rot;

    // Get the difference
    delta = inputRot;
    delta.yaw -= lastInputRot_.yaw;
    delta.pitch -= lastInputRot_.pitch;
    delta.roll -= lastInputRot_.roll;

    pthread_mutex_lock(&modulePoseMutex_);

    shModulePose_.rot.yaw += delta.yaw;
    POSECALCMODULE_CRAMPDEG((shModulePose_.rot.yaw),0.0,360.0);
    shModulePose_.rot.pitch += delta.pitch;
    POSECALCMODULE_CRAMPDEG((shModulePose_.rot.pitch),0.0,360.0);
    shModulePose_.rot.roll += delta.roll;
    POSECALCMODULE_CRAMPDEG((shModulePose_.rot.roll),0.0,360.0);
    shModulePose_.rot.extra = inputRot.extra;
    poseInfo.pose = shModulePose_;
    pthread_mutex_unlock(&modulePoseMutex_);

    // Note: This function is only the place where the value of lastInputRot_
    // should be altered (except in the constructor). Otherwise, it has to be
    // protected by mutex.
    lastInputRot_ = inputRot;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleGyroRot::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double transSpeed, angSpeed;

    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;

    poseCalc_->getVelocity(vel);
    transSpeed = sqrt(pow(vel.transVel.x, 2) + pow(vel.transVel.y, 2));
    angSpeed = fabs(vel.angVel.yaw);

    if ((transSpeed <= MAXIMUM_STOPPING_TRANS_SPEED_) &&
        (angSpeed <= MAXIMUM_STOPPING_ANGULAR_SPEED_))
    {
        poseInfo.variance.rotVariance.yaw = STOPPING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.pitch = STOPPING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.roll = STOPPING_ROT_VARIANCE_;
    }
    else
    {
        poseInfo.variance.rotVariance.yaw = MOVING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.pitch = MOVING_ROT_VARIANCE_;
        poseInfo.variance.rotVariance.roll = MOVING_ROT_VARIANCE_;
    }

    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleGyroRot::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.variance.rotVariance.yaw = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = STATIC_ROT_VARIANCE_;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleGpsLoc
//-----------------------------------------------------------------------
PoseCalcModuleGpsLoc::PoseCalcModuleGpsLoc(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Gps *gps) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_GPS,
        sensorType,
        POSECALC_POSE_TYPE_LOCATION,
        poseCalc),
    gps_(gps)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleGpsLoc
//-----------------------------------------------------------------------
PoseCalcModuleGpsLoc::~PoseCalcModuleGpsLoc(void)
{
}

//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsLoc::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsLoc::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// location.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsLoc::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    // Update the location, and save the value.
    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_.loc = poseInfo.pose.loc;
    pthread_mutex_unlock(&modulePoseMutex_);
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated location.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsLoc::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    double confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
    double locVariance = WORST_LOC_VARIANCE_;
    int rt20Status;

    rt20Status = gps_->getRT20Status();

    switch (rt20Status) {

    case 0:
        confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
        locVariance = RT20_0_LOC_VARIANCE_;
        break;

    case 1:
        confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
        locVariance = RT20_1_LOC_VARIANCE_;
        break;

    case 100:
    	confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
    	locVariance = RT20_100_LOC_VARIANCE_;
    	break;

    default:
        confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
        locVariance = WORST_LOC_VARIANCE_;
        break;
    }

    poseInfo.confidence.locConfidence.value = confidenceLevel;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.variance.locVariance.x = locVariance;
    poseInfo.variance.locVariance.y = locVariance;
    poseInfo.variance.locVariance.z = locVariance;
    poseInfo.variance.rotVariance.yaw = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = WORST_ROT_VARIANCE_;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated location.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsLoc::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.variance.locVariance.x = STATIC_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = STATIC_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = STATIC_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = WORST_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = WORST_ROT_VARIANCE_;
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleGpsRot
//-----------------------------------------------------------------------
PoseCalcModuleGpsRot::PoseCalcModuleGpsRot(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Gps *gps,
    double headingFactor,
    double headingOffset,
    double minValidTransSpeed,
    double maxValidAngSpeed) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_GPS,
        sensorType,
        POSECALC_POSE_TYPE_ROTATION,
        poseCalc),
    gps_(gps),
    numValidTransSpeed_(0),
    numValidAngularSpeed_(-1),
    HEADING_FACTOR_(headingFactor),
    HEADING_OFFSET_(headingOffset),
    MIN_VALID_TRANS_SPEED_(minValidTransSpeed),
    MAX_VALID_ANGULAR_SPEED_(maxValidAngSpeed)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleGpsRot
//-----------------------------------------------------------------------
PoseCalcModuleGpsRot::~PoseCalcModuleGpsRot(void)
{
}

//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsRot::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsRot::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// location.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsRot::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double heading;

    // Comput the new heading.
    heading = poseInfo.pose.rot.yaw;
    heading *= HEADING_FACTOR_;
    heading += HEADING_OFFSET_;

    // Check for backwardness.
    poseCalc_->getVelocity(vel);
    if (vel.extra.backward)
    {
        heading += 180.0;
    }

    POSECALCMODULE_CRAMPDEG(heading,0.0,360.0);

    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_.rot.yaw = heading;
    shModulePose_.rot.extra = poseInfo.pose.rot.extra;
    poseInfo.pose = shModulePose_;
    pthread_mutex_unlock(&modulePoseMutex_);
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsRot::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double transSpeed, angSpeed;
    double confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
    double rotVariance = WORST_ROT_VARIANCE_;
    int rt20Status;
    bool hasValidAngSpeed = false;
    bool hasValidTransSpeed = false;

    // To compute the confidence level, first get both translational and
    // angular speed of the robot. The robot has to be moving at least
    // with cetain translational speed, and it should not be turning.
    poseCalc_->getVelocity(vel);
    transSpeed = sqrt(pow(vel.transVel.x, 2) + pow(vel.transVel.y, 2));
    angSpeed = fabs(vel.angVel.yaw);

    // Check the validity of the translational speed.
    if (transSpeed >= MIN_VALID_TRANS_SPEED_)
    {
        numValidTransSpeed_++;

        // Make sure it wasn't noise.
        if (numValidTransSpeed_ > MIN_VALID_TRANS_SPEED_SAMPLES_)
        {
            hasValidTransSpeed = true;
        }
    }
    else
    {
        numValidTransSpeed_ = 0;
    }

    // Check the validity of the translational speed.
    if (angSpeed < MAX_VALID_ANGULAR_SPEED_)
    {
        if (numValidAngularSpeed_ == -1)
        {
            // Assumption: When HServer started, the robot has never been
            // moved.
            hasValidAngSpeed = true;
        }
        else
        {
            numValidAngularSpeed_++;

            // Make sure it wasn't noise.
            if (numValidAngularSpeed_ > MIN_VALID_ANGULAR_SPEED_SAMPLES_)
            {
                hasValidAngSpeed = true;
            }
        }
    }
    else
    {
        numValidAngularSpeed_ = 0;
    }

    if (hasValidTransSpeed && hasValidAngSpeed)
    {
        rt20Status = gps_->getRT20Status();

        switch (rt20Status) {

        case 0:
            confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
            rotVariance = RT20_0_ROT_VARIANCE_;
            break;

        case 1:
            confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
            rotVariance = RT20_1_ROT_VARIANCE_;
            break;

        case 100:
			confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
			rotVariance = RT20_100_ROT_VARIANCE_;
			break;

        default:
            confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
            rotVariance = WORST_ROT_VARIANCE_;
            break;
        }
    }
    else
    {
        // The robot is not moving (translationally) fast enough and/or
        // it is currently turning.
        confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
        rotVariance = WORST_ROT_VARIANCE_;
    }

    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = confidenceLevel;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = rotVariance;
    poseInfo.variance.rotVariance.pitch = rotVariance;
    poseInfo.variance.rotVariance.roll = rotVariance;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleGpsRot::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    //poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = STATIC_ROT_VARIANCE_;
}

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleCompassRot
//-----------------------------------------------------------------------
PoseCalcModuleCompassRot::PoseCalcModuleCompassRot(
    int id,
    int sensorType,
    PoseCalcInterface *poseCalc,
    Compass *compass,
    double headingFactor,
    double headingOffset,
    double maxValidAngSpeed) :
    PoseCalcModule(
        id,
        POSECALC_SENSOR_CLASS_COMPASS,
        sensorType,
        POSECALC_POSE_TYPE_ROTATION,
        poseCalc),
    compass_(compass),
    numValidAngularSpeed_(-1),
    HEADING_FACTOR_(headingFactor),
    HEADING_OFFSET_(headingOffset),
    MAX_VALID_ANGULAR_SPEED_(maxValidAngSpeed)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleCompassRot
//-----------------------------------------------------------------------
PoseCalcModuleCompassRot::~PoseCalcModuleCompassRot(void)
{
}


//-----------------------------------------------------------------------
// This functions creates and sets the filter to filter incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleCompassRot::setFilter_(void)
{
    PoseCalcModuleFilterNull *moduleFilter = NULL;

    moduleFilter = new PoseCalcModuleFilterNull();
    moduleFilter_ = moduleFilter;
}

//-----------------------------------------------------------------------
// This function filters the incoming data.
//-----------------------------------------------------------------------
void PoseCalcModuleCompassRot::filter_(PoseCalcPoseInfo_t &poseInfo)
{
    if (moduleFilter_ != NULL)
    {
        moduleFilter_->filter(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function transforms the input sensor readings into the MissionLab
// location.
//-----------------------------------------------------------------------
void PoseCalcModuleCompassRot::transform_(PoseCalcPoseInfo_t &poseInfo)
{
    double heading;

    // Comput the new heading.
    heading = poseInfo.pose.rot.yaw;
    heading *= HEADING_FACTOR_;
    heading += HEADING_OFFSET_;
    POSECALCMODULE_CRAMPDEG(heading,0.0,360.0);

    pthread_mutex_lock(&modulePoseMutex_);
    shModulePose_.rot.yaw = heading;
    shModulePose_.rot.extra = poseInfo.pose.rot.extra;
    poseInfo.pose = shModulePose_;
    pthread_mutex_unlock(&modulePoseMutex_);
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleCompassRot::gradeDynamic_(PoseCalcPoseInfo_t &poseInfo)
{
    HSVelocity_t vel;
    double angSpeed;
    double confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
    double rotVariance = WORST_ROT_VARIANCE_;
    bool hasValidAngSpeed = false;

    // To compute the confidence level, the angular speed of the robot
    // needs to be calculated. The robot should not be turning.
    poseCalc_->getVelocity(vel);
    angSpeed = fabs(vel.angVel.yaw);

    if (angSpeed < MAX_VALID_ANGULAR_SPEED_)
    {
        if (numValidAngularSpeed_ == -1)
        {
            // Assumption: When HServer started, the robot has never been
            // moved.
            hasValidAngSpeed = true;
        }
        else
        {
            numValidAngularSpeed_++;

            // Make sure it wasn't noise.
            if (numValidAngularSpeed_ > MIN_VALID_ANGULAR_SPEED_SAMPLES_)
            {
                hasValidAngSpeed = true;
            }
        }
    }
    else
    {
        numValidAngularSpeed_ = 0;
    }

    if (hasValidAngSpeed)
    {
        confidenceLevel = PoseCalc::MAX_CONFIDENCE_LEVEL;
        rotVariance = BEST_ROT_VARIANCE_;
    }
    else
    {
        // The robot is currently turning.
        confidenceLevel = PoseCalc::MIN_CONFIDENCE_LEVEL;
        rotVariance = WORST_ROT_VARIANCE_;
    }

    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = confidenceLevel;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = rotVariance;
    poseInfo.variance.rotVariance.pitch = rotVariance;
    poseInfo.variance.rotVariance.roll = rotVariance;
}

//-----------------------------------------------------------------------
// This function grades the quality of the calculated rotation.
//-----------------------------------------------------------------------
void PoseCalcModuleCompassRot::gradeStatic_(PoseCalcPoseInfo_t &poseInfo)
{
    poseInfo.confidence.locConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    //poseInfo.confidence.rotConfidence.value = PoseCalc::MAX_CONFIDENCE_LEVEL;
    poseInfo.confidence.rotConfidence.value = PoseCalc::MIN_CONFIDENCE_LEVEL;
    poseInfo.variance.locVariance.x = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.y = WORST_LOC_VARIANCE_;
    poseInfo.variance.locVariance.z = WORST_LOC_VARIANCE_;
    poseInfo.variance.rotVariance.yaw = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.pitch = STATIC_ROT_VARIANCE_;
    poseInfo.variance.rotVariance.roll = STATIC_ROT_VARIANCE_;
}

/**********************************************************************
# $Log: PCModules.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:16  endo
# New PoseCalc integrated.
#
#**********************************************************************/
