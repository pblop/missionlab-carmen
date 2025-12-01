/**********************************************************************
 **                                                                  **
 **                         PoseCalculator.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  James Brian Lee                                    **
 **  Revised by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2004, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PoseCalculator.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <algorithm>
#include <numeric>
#include <sys/resource.h>


#include "HServerTypes.h"
#include "hserver.h"
#include "Watchdog.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculator.h"
#include "PCModuleInterface.h"
#include "PCModules.h"
#include "PCSensoryDataBusInterface.h"
#include "PCSensoryDataBus.h"
#include "PCSensorFuserInterface.h"
#include "Kalman.h"
#include "MaximumConfidence.h"
#include "ParticleFilter.h"
#include "PCSituationalContextInterface.h"
#include "PCSituationalContextSensory.h"
#include "PCSituationalContextEnvironment.h"
#include "PCCoordinatorInterface.h"
#include "PCCoordinatorPickFilter.h"
#include "PCUtility.h"
#include "LogManager.h"
#include "message.h"
#include "ipc_client.h"

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void exit_hserver(void);
extern bool SilentMode;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
// public
const double PoseCalc::MIN_CONFIDENCE_LEVEL = 0.0;
const double PoseCalc::MAX_CONFIDENCE_LEVEL = 1.0;

// protected
const string PoseCalc::EMPTY_STRING_ = "";
//const int PoseCalc::MAIN_THREAD_USLEEP_ = 20000;
const int PoseCalc::MAIN_THREAD_USLEEP_ = 100000;
//const int PoseCalc::WAIT_BLOCK_UPDATE_RELEASE_USLEEP_ = 500;
const int PoseCalc::WAIT_BLOCK_UPDATE_RELEASE_USLEEP_ = 10000;
const int PoseCalc::SKIP_STATUSBAR_UPDATE_ = 10;
const int PoseCalc::DEFAULT_COORDINATOR_ = POSECALC_COORDINATOR_TYPE_PICK_FILTER;
const int PoseCalc::WATCHDOG_CHECK_INTERVAL_SEC_ = 5;
const bool PoseCalc::FEEDBACK_POSE_TO_MODULES_ = true;

//-----------------------------------------------------------------------
// Mutex
//-----------------------------------------------------------------------
pthread_mutex_t PoseCalc::poseMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::poseCalcModuleListMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::poseCalcModuleIDMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::selectedSensorMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::sensorFusersMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::blockUpdateMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::poseInitingMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::refTimeMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::coordinatorMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::sensorFuserTypeMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t PoseCalc::varianceSchemeMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
PoseCalc::PoseCalc(
    PoseCalcConstants_t poseCalcConstants,
    int sensorFuserType,
    int varianceScheme) :
    Module(NULL, HS_MODULE_NAME_ROBOT_POSECALC),
    sensorySitCon_(NULL),
    envSitCon_(NULL),
    sensoryDataBus_(NULL),
    shCoordinator_(NULL),
    sensor_(NULL),
    shLocRefTime_(0),
    shRotRefTime_(0),
    shSensorFuserType_(sensorFuserType),
    shPoseCalcModuleID_(0),
    shSelectedSensorLoc_(POSECALC_SENSOR_TYPE_NONE),
    shSelectedSensorRot_(POSECALC_SENSOR_TYPE_NONE),
    shVarianceScheme_(varianceScheme),
    shBlockUpdate_(false),
    shPoseIniting_(false),
    POSECALC_CONSTANTS_(poseCalcConstants)
{
    memset(shXYT_, 0x0, sizeof(float)*3);
    memset(&shPoseCalcPose_, 0x0, sizeof(HSPose_t));
    memset(&shPoseCalcLastPose_, 0x0, sizeof(HSPose_t));

    // Setup the sensory situational context.
    setupSensorySituationalContext_();

    // Setup the sensory data bus.
    setupSensoryDataBus_();

    // Setup the sensor fusers.
    setupSensorFusers_();

    // Setup the environmentsituational context.
    setupEnvironmentalSituationalContext_();

    // Setup the cooridantor.
    setupCoordinator_();

    // Setup the output (i.e., the sensor class to have the final
    // pose to be available in the common sensor repository.)
    setupOutput_();

    // Start the main thread.
    pthread_create(&mainThread_, NULL, &startMainThread_, (void*)this);

    // Start watchdog if enabled.
    if (gWatchdogEnabled)
    {
        watchdog_ = new Watchdog(NAME_, WATCHDOG_CHECK_INTERVAL_SEC_, getpid(), mainThread_);
    }

    refreshScreen();
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
PoseCalc::~PoseCalc(void)
{
    int i;

    pthread_cancel(mainThread_);
    pthread_join(mainThread_, NULL );

    pthread_mutex_lock(&sensorFusersMutex_);

    for (i = 0; i < (int)(shSensorFusers_.size()); i++)
    {
        delete (shSensorFusers_[i]);
        (shSensorFusers_[i]) = NULL;
    }
    shSensorFusers_.clear();

    pthread_mutex_unlock(&sensorFusersMutex_);

    pthread_mutex_lock(&poseCalcModuleListMutex_);

    for (i = 0; i < (int)(shPoseCalcModuleLocList_.size()); i++)
    {
        (shPoseCalcModuleLocList_[i])->die();
        (shPoseCalcModuleLocList_[i]) = NULL;
    }
    shPoseCalcModuleLocList_.clear();

    for (i = 0; i < (int)(shPoseCalcModuleRotList_.size()); i++)
    {
        (shPoseCalcModuleRotList_[i])->die();
        (shPoseCalcModuleRotList_[i]) = NULL;
    }
    shPoseCalcModuleRotList_.clear();

    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    if (sensor_ != NULL)
    {
        delete sensor_;
        sensor_ = NULL;
    }

    pthread_mutex_lock(&coordinatorMutex_);

    if (shCoordinator_ != NULL)
    {
        delete shCoordinator_;
        shCoordinator_ = NULL;
    }

    pthread_mutex_unlock(&coordinatorMutex_);

    if (sensoryDataBus_ != NULL)
    {
        delete sensoryDataBus_;
        sensoryDataBus_ = NULL;
    }

    if (sensorySitCon_ != NULL)
    {
        delete sensorySitCon_;
        sensorySitCon_ = NULL;
    }

    if (envSitCon_ != NULL)
    {
        delete envSitCon_;
        envSitCon_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function sets up the sensory situational context.
//-----------------------------------------------------------------------
void PoseCalc::setupSensorySituationalContext_(void)
{
    PoseCalcSituationalContextSensory *sensorySitCon = NULL;

    sensorySitCon = new PoseCalcSituationalContextSensory();
    sensorySitCon_ = sensorySitCon;
}

//-----------------------------------------------------------------------
// This function sets up the environmental situational context.
//-----------------------------------------------------------------------
void PoseCalc::setupEnvironmentalSituationalContext_(void)
{
    PoseCalcSituationalContextEnvironment *envSitCon = NULL;

    envSitCon = new PoseCalcSituationalContextEnvironment();
    envSitCon_ = envSitCon;
}

//-----------------------------------------------------------------------
// This function sets up the sensory data bus.
//-----------------------------------------------------------------------
void PoseCalc::setupSensoryDataBus_(void)
{
    //int varianceScheme;

    PoseCalcSensoryDataBus *sensoryDataBus = NULL;

    sensoryDataBus = new PoseCalcSensoryDataBus(this);
    sensoryDataBus_ = sensoryDataBus;

    /*
    // If the dynamic variance scheme is used, do not let the sensory
    // bus to send the data to the fusers if the data were already read
    // by them. On the other, if the variance scheme is static, let
    // them read as many times as they want.
    getVarianceScheme_(varianceScheme);

    switch (varianceScheme) {

    case POSECALC_VARIANCE_SCHEME_DYNAMIC:
        sensoryDataBus_->setFilterReadData(true);
        break;

    case POSECALC_VARIANCE_SCHEME_STATIC:
        sensoryDataBus_->setFilterReadData(false);
        break;
    }
    */
}

//-----------------------------------------------------------------------
// This function sets up the sensor fusers.
//-----------------------------------------------------------------------
void PoseCalc::setupSensorFusers_(void)
{
    PoseCalcSensorFuserInterface *sensorFuser = NULL;
    MaximumConfidence *maxconf = NULL;
    Kalman *ekf = NULL;
    ParticleFilter *pfilter = NULL;

    pthread_mutex_lock(&sensorFusersMutex_);

    // Setup Maximum Confidence
    maxconf = new MaximumConfidence(sensoryDataBus_);
    sensorFuser = maxconf;
    shSensorFusers_.push_back(sensorFuser);

    // Setup Extended Kalman Filter
    ekf = new Kalman(sensoryDataBus_);
    sensorFuser = ekf;
    shSensorFusers_.push_back(sensorFuser);


    // Setup Particle Filter
    pfilter = new ParticleFilter(sensoryDataBus_);
    sensorFuser = pfilter;
    shSensorFusers_.push_back(sensorFuser);


    pthread_mutex_unlock(&sensorFusersMutex_);
}

//-----------------------------------------------------------------------
// This function sets the current fuser type.
//-----------------------------------------------------------------------
void PoseCalc::setSensorFuserType_(int fuserType)
{
    pthread_mutex_lock(&sensorFuserTypeMutex_);
    shSensorFuserType_ = fuserType;
    pthread_mutex_unlock(&sensorFuserTypeMutex_);
}

//-----------------------------------------------------------------------
// This function returns the current fuser type.
//-----------------------------------------------------------------------
void PoseCalc::getSensorFuserType_(int &fuserType)
{
    pthread_mutex_lock(&sensorFuserTypeMutex_);
    fuserType = shSensorFuserType_;
    pthread_mutex_unlock(&sensorFuserTypeMutex_);
}

//-----------------------------------------------------------------------
// This function sets the current variance scheme.
//-----------------------------------------------------------------------
void PoseCalc::setVarianceScheme_(int varianceScheme)
{
    pthread_mutex_lock(&varianceSchemeMutex_);
    shVarianceScheme_ = varianceScheme;
    pthread_mutex_unlock(&varianceSchemeMutex_);

    /*
    // If the dynamic variance scheme is used, do not let the sensory
    // bus to send the data to the fusers if the data were already read
    // by them. On the other, if the variance scheme is static, let
    // them read as many times as they want.
    switch (varianceScheme) {

    case POSECALC_VARIANCE_SCHEME_DYNAMIC:
        sensoryDataBus_->setFilterReadData(true);
        break;

    case POSECALC_VARIANCE_SCHEME_STATIC:
        sensoryDataBus_->setFilterReadData(false);
        break;
    }
    */
}

//-----------------------------------------------------------------------
// This function returns the current variance scheme.
//-----------------------------------------------------------------------
void PoseCalc::getVarianceScheme_(int &varianceScheme)
{
    pthread_mutex_lock(&varianceSchemeMutex_);
    varianceScheme = shVarianceScheme_;
    pthread_mutex_unlock(&varianceSchemeMutex_);
}

//-----------------------------------------------------------------------
// This function sets up the coordinator.
//-----------------------------------------------------------------------
void PoseCalc::setupCoordinator_(void)
{
    PoseCalcCoordinatorPickFilter *coordinatorPickFilter = NULL;
    int fuserType;

    getSensorFuserType_(fuserType);

    switch (DEFAULT_COORDINATOR_) {

    case POSECALC_COORDINATOR_TYPE_PICK_FILTER:
        coordinatorPickFilter = new PoseCalcCoordinatorPickFilter(fuserType);

        pthread_mutex_lock(&coordinatorMutex_);
        shCoordinator_ = coordinatorPickFilter;
        pthread_mutex_unlock(&coordinatorMutex_);
        break;

    default:
        printfTextWindow("PoseCalc: Unknown coordinator.\n");
        exit_hserver();
        break;
    }
}

//-----------------------------------------------------------------------
// This function sets up the Sensor class to have the pose available in
// the common sensor repository.
//-----------------------------------------------------------------------
void PoseCalc::setupOutput_(void)
{
    // Setup the sensor class in order to pass the pose data.
    sensor_ = new Sensor(
        SENSOR_XYT,
        0,
        NULL,
        0,
        NULL,
        3,
        shXYT_,
        NULL,
        &poseMutex_);
}

//-----------------------------------------------------------------------
// This function starts the main thread.
//-----------------------------------------------------------------------
void* PoseCalc::startMainThread_(void* poseCalcInstance)
{
    ((PoseCalc*)poseCalcInstance)->mainLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function constantly computes the pose.
//-----------------------------------------------------------------------
void PoseCalc::mainLoop_(void)
{
    int count = 0;
    HSPose_t pose;

    struct rlimit rlp;

    rlp.rlim_cur = 131072;
    rlp.rlim_max = 131072;
//     if(setrlimit(RLIMIT_STACK, &rlp) != 0){
//         printfTextWindow("Errror al modificar el tamño del stak --> %d .\n", rlp.rlim_cur);
//     }
    getrlimit(RLIMIT_STACK, &rlp);
//     printfTextWindow("Valor actual %d y valor maximo %d .\n", rlp.rlim_cur, rlp.rlim_max);

    while (true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Compute the pose.
        updatePose_();

        // Compute the frequency of the loop.
        compFrequency_();

#ifdef LOG_POSE_DATA
        getPose(pose);
        fprintf(
             stderr,
             "POSECALCULATOR: X=%f Y=%f Z=%f poseTime=%f locTime=%f rotTime=%f\n",
             pose.loc.x,
             pose.loc.y,
             pose.rot.yaw,
             pose.extra.time,
             pose.loc.extra.time,
             pose.rot.extra.time);
#endif

        // Update the status bar.
        if (count > SKIP_STATUSBAR_UPDATE_)
        {
            updateStatusBar_();
            count = 0;
        }

        count++;

        // Log the data if necessary.
        if (gLogManager != NULL)
        {
            manageLog_(mainThread_);
        }

        // Tell watch dog that PoseCalc is still alive.
        if (watchdog_ != NULL)
        {
            watchdog_->notifyUpdate();
        }

        usleep(MAIN_THREAD_USLEEP_);
    }
}

//-----------------------------------------------------------------------
// This function computes the pose in a MissionLab coordinate by fusing
// the poses from different sensors using different fusers, and
// coordinate the fused poses (coming from the different fusers).
//-----------------------------------------------------------------------
void PoseCalc::updatePose_(void)
{
    PoseCalcFusedPose_t fusedPose;
    PoseCalcGenericData_t data;
    vector<PoseCalcFusedPose_t> fusedPoseList;
    int i;

    // If initializing the pose, don't update the pose.
    if (poseIniting_())
    {
        return;
    }

    // Do not let PCModules to update the pose since they would
    // be outdated by the time when the fusedPose (below) is
    // computed.
    setBlockUpdate_(true);

    pthread_mutex_lock(&sensorFusersMutex_);

    for (i = 0; i < (int)(shSensorFusers_.size()); i++)
    {
        // Run the filter.
        fusedPose = (shSensorFusers_[i])->getOutput();

        // Apply the environmental situation.
        data.content = (void *)&fusedPose;
        envSitCon_->applySituation(data);

        // Save the data.
        fusedPoseList.push_back(fusedPose);
    }

    pthread_mutex_unlock(&sensorFusersMutex_);

    // Coordinate the poses that were computed by different fusers.
    pthread_mutex_lock(&coordinatorMutex_);
    fusedPose = shCoordinator_->coordinate(fusedPoseList);
    pthread_mutex_unlock(&coordinatorMutex_);

    // Save the pose (if valid).
    saveFusedPose_(fusedPose);

    // Let the PCModules to resume their work.
    setBlockUpdate_(false);

    // Save the sensor type if Maximum Confidence is used as a fuser.
    if (fusedPose.fuserType == POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE)
    {
        saveSelectedSensors_(
            fusedPose.extra.locSensorType,
            fusedPose.extra.rotSensorType);
    }
}

//-----------------------------------------------------------------------
// This function saves the pose, location, or rotation info if the fused
// pose is valid.
//-----------------------------------------------------------------------
void PoseCalc::saveFusedPose_(PoseCalcFusedPose_t fusedPose)
{
    // If initializing the pose, don't update the pose.
    if (poseIniting_())
    {
        return;
    }

    if (fusedPose.extra.validPose)
    {
        setPose(fusedPose.pose);
    }
    else if (fusedPose.extra.validLoc)
    {
        setLocation(fusedPose.pose.loc);
    }
    else if (fusedPose.extra.validRot)
    {
        setRotation(fusedPose.pose.rot);
    }
}

//-----------------------------------------------------------------------
// This function saves the selected sensors if they are valid sensors.
// It is a wrapper for setSelectedSensors(), adding the check for the
// validity.
//-----------------------------------------------------------------------
void PoseCalc::saveSelectedSensors_(int sensorTypeLoc, int sensorTypeRot)
{
    int sLoc, sRot;

    getSelectedSensors_(sLoc, sRot);

    if (sensorTypeLoc != POSECALC_POSE_TYPE_NONE)
    {
        sLoc = sensorTypeLoc;
    }

    if (sensorTypeRot != POSECALC_POSE_TYPE_NONE)
    {
        sRot = sensorTypeRot;
    }

    setSelectedSensors_(sLoc, sRot);
}

//-----------------------------------------------------------------------
// This function computes the velocity of the robot.
//-----------------------------------------------------------------------
void PoseCalc::getVelocity(HSVelocity_t &vel)
{
    if (gRobot != NULL)
    {
        // Use robot's velocity if available.
        gRobot->getVelocity(vel);
    }

    if (gGyro != NULL)
    {
        // Use Gyro's angular velocity if available (and even if the
        // robot's angVel is available.
        gGyro->getAngularVelocity(vel.angVel);
    }
}

//-----------------------------------------------------------------------
// This function updates the status bar.
//-----------------------------------------------------------------------
void PoseCalc::updateStatusBar_(void)
{
    HSPose_t pose;
    string selectedSensorNameLoc, selectedSensorNameRot;
    char buf[250];
    double frequency = 0;
    int fuserType, varianceScheme;
    int selectedSensorLoc, selectedSensorRot;
    const bool DISPLAY_FREQUENCY = true;

    statusStr[0] = '\0';

    strcat(statusStr, "Pose: ");

    getPose(pose);

    sprintf(
        buf,
        "%c x: %.2f  y: %.2f  yaw: %.2f",
        statusbarSpinner_->getStatus(),
        pose.loc.x,
        pose.loc.y,
        pose.rot.yaw);

    getSensorFuserType_(fuserType);

    switch (fuserType) {

    case POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE:
        getSelectedSensors_(selectedSensorLoc, selectedSensorRot);
        selectedSensorNameLoc = PoseCalcUtility::sensorType2Name(selectedSensorLoc);
        selectedSensorNameRot = PoseCalcUtility::sensorType2Name(selectedSensorRot);

        if ((selectedSensorNameLoc != EMPTY_STRING_) &&
            (selectedSensorNameRot != EMPTY_STRING_))
        {
            sprintf(
                buf,
                "%s [maxconf  loc: %s  rot: %s]",
                buf,
                selectedSensorNameLoc.c_str(),
                selectedSensorNameRot.c_str());
        }
        else if (selectedSensorNameLoc != EMPTY_STRING_)
        {
            sprintf(
                buf,
                "%s [maxconf  loc: %s]",
                buf,
                selectedSensorNameLoc.c_str());
        }
        else if (selectedSensorNameRot != EMPTY_STRING_)
        {
            sprintf(
                buf,
                "%s [maxconf  rot: %s]",
                buf,
                selectedSensorNameRot.c_str());
        }
        break;

    case POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER:
        sprintf(buf, "%s [ekf]", buf);
        break;

    case POSECALC_FUSER_TYPE_PARTICLE_FILTER:
        sprintf(buf, "%s [pfilter]", buf);
        break;
    }

    getVarianceScheme_(varianceScheme);

    switch (varianceScheme) {

    case POSECALC_VARIANCE_SCHEME_DYNAMIC:
        sprintf(buf, "%s [dynamic var]", buf);
        break;

    case POSECALC_VARIANCE_SCHEME_STATIC:
        sprintf(buf, "%s [static var]", buf);
        break;
    }

    if (DISPLAY_FREQUENCY)
    {
        getFrequency_(frequency);
        sprintf(buf, "%s : %.2f Hz", buf, frequency);
    }

    strcat(statusStr, buf);
    statusbar->update(statusLine);
}

//-----------------------------------------------------------------------
// This function updates the sensor values for both pose and heading.
//-----------------------------------------------------------------------
void PoseCalc::updateModulePose(int moduleID, HSPose_t pose)
{
    waitForBlockUpdateRelease_();

    updateModuleLocation_(moduleID, pose.loc);
    updateModuleRotation_(moduleID, pose.rot);
}

//-----------------------------------------------------------------------
// This function updates the sensor values for the location.
//-----------------------------------------------------------------------
void PoseCalc::updateModuleLocation(int moduleID, HSLocation_t loc)
{
    waitForBlockUpdateRelease_();

    updateModuleLocation_(moduleID, loc);
}

//-----------------------------------------------------------------------
// This function updates the sensor values for the heading.
//-----------------------------------------------------------------------
void PoseCalc::updateModuleRotation(int moduleID, HSRotation_t rot)
{
    waitForBlockUpdateRelease_();

    updateModuleRotation_(moduleID, rot);
}

//-----------------------------------------------------------------------
// This function updates the sensor values for the location.
//-----------------------------------------------------------------------
void PoseCalc::updateModuleLocation_(int moduleID, HSLocation_t loc)
{
    PoseCalcPoseInfo_t poseInfo;
    PoseCalcGenericData_t data;
    PoseCalcModuleInterface *moduleLoc = NULL;
    vector<int>::iterator iter;
    double refTime;
    int indexLoc;
    int varianceScheme;
    bool moduleFound = false;

    getLocReferenceTime(refTime);
    getVarianceScheme_(varianceScheme);

    memset((PoseCalcPoseInfo_t *)&poseInfo, 0x0, sizeof(PoseCalcPoseInfo_t));
    poseInfo.pose.loc = loc;
    poseInfo.pose.extra = loc.extra;
    poseInfo.rawPose.loc = loc;
    poseInfo.rawPose.extra = loc.extra;
    poseInfo.locRefTime = refTime;

    pthread_mutex_lock(&poseCalcModuleListMutex_);

    iter = find(
        shPoseCalcModuleLocIDTable_.begin(),
        shPoseCalcModuleLocIDTable_.end(),
        moduleID);

    if (iter != shPoseCalcModuleLocIDTable_.end())
    {
        indexLoc = iter - shPoseCalcModuleLocIDTable_.begin();
        moduleLoc = shPoseCalcModuleLocList_[indexLoc];
        moduleFound = true;
    }

    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    if (moduleFound && (moduleLoc != NULL))
    {
        // Filter, transform, and grade the pose.
        moduleLoc->managePose(poseInfo, varianceScheme);

        // Apply the situation.
        data.content = (void *)&poseInfo;
        sensorySitCon_->applySituation(data);

        // Save in the sensory data bus.
        sensoryDataBus_->savePoseInfo(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function updates the sensor values for the heading.
//-----------------------------------------------------------------------
void PoseCalc::updateModuleRotation_(int moduleID, HSRotation_t rot)
{
    PoseCalcPoseInfo_t poseInfo;
    PoseCalcGenericData_t data;
    PoseCalcModuleInterface *moduleRot = NULL;
    vector<int>::iterator iter;
    double refTime;
    int indexRot;
    int varianceScheme;
    bool moduleFound = false;

    getRotReferenceTime(refTime);
    getVarianceScheme_(varianceScheme);

    memset((PoseCalcPoseInfo_t *)&poseInfo, 0x0, sizeof(PoseCalcPoseInfo_t));
    poseInfo.pose.rot = rot;
    poseInfo.pose.extra = rot.extra;
    poseInfo.rawPose.rot = rot;
    poseInfo.rawPose.extra = rot.extra;
    poseInfo.rotRefTime = refTime;

    pthread_mutex_lock(&poseCalcModuleListMutex_);

    iter = find(
        shPoseCalcModuleRotIDTable_.begin(),
        shPoseCalcModuleRotIDTable_.end(),
        moduleID);

    if (iter != shPoseCalcModuleRotIDTable_.end())
    {
        indexRot = iter - shPoseCalcModuleRotIDTable_.begin();
        moduleRot = shPoseCalcModuleRotList_[indexRot];
        moduleFound = true;
    }

    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    if (moduleFound && (moduleRot != NULL))
    {
        // Filter, transform, and grade the pose.
        moduleRot->managePose(poseInfo, varianceScheme);

        // Apply the situation.
        data.content = (void *)&poseInfo;
        sensorySitCon_->applySituation(data);

        // Save in the sensory data bus.
        sensoryDataBus_->savePoseInfo(poseInfo);
    }
}

//-----------------------------------------------------------------------
// This function adds the robot shaft-encoder module.
//-----------------------------------------------------------------------
int PoseCalc::addRobot(Robot *robot)
{
    PoseCalcModuleInterface *moduleLoc = NULL;
    PoseCalcModuleInterface *moduleRot = NULL;
    PoseCalcModuleRobotShaftLoc *moduleRobotShaftLoc = NULL;
    PoseCalcModuleRobotShaftRot *moduleRobotShaftRot = NULL;
    int moduleID, sensorType;

    pthread_mutex_lock(&poseCalcModuleIDMutex_);
    moduleID = shPoseCalcModuleID_;
    shPoseCalcModuleID_++;
    pthread_mutex_unlock(&poseCalcModuleIDMutex_);

    sensorType = PoseCalcUtility::name2SensorType(robot->getName());

    // Create the Loc module.
    moduleRobotShaftLoc = new PoseCalcModuleRobotShaftLoc(
        moduleID,
        sensorType,
        this,
        robot);
    moduleLoc = moduleRobotShaftLoc;

    // Create the Rot module.
    moduleRobotShaftRot = new PoseCalcModuleRobotShaftRot(
        moduleID,
        sensorType,
        this,
        robot);
    moduleRot = moduleRobotShaftRot;

    // Save the module.
    pthread_mutex_lock(&poseCalcModuleListMutex_);
    shPoseCalcModuleLocList_.push_back(moduleLoc);
    shPoseCalcModuleLocIDTable_.push_back(moduleID);
    shPoseCalcModuleRotList_.push_back(moduleRot);
    shPoseCalcModuleRotIDTable_.push_back(moduleID);
    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    return moduleID;
}

//-----------------------------------------------------------------------
// This function sets up the GPS.
//-----------------------------------------------------------------------
int PoseCalc::addGps(Gps *gps)
{
    PoseCalcModuleInterface *moduleLoc = NULL;
    PoseCalcModuleInterface *moduleRot = NULL;
    PoseCalcModuleGpsLoc *moduleGpsLoc = NULL;
    PoseCalcModuleGpsRot *moduleGpsRot = NULL;
    int moduleID, sensorType;

    pthread_mutex_lock(&poseCalcModuleIDMutex_);
    moduleID = shPoseCalcModuleID_;
    shPoseCalcModuleID_++;
    pthread_mutex_unlock(&poseCalcModuleIDMutex_);

    sensorType = PoseCalcUtility::name2SensorType(gps->getName());

    // Create the Loc module.
    moduleGpsLoc = new PoseCalcModuleGpsLoc(
        moduleID,
        sensorType,
        this,
        gps);
    moduleLoc = moduleGpsLoc;

    // Create the Rot module.
    moduleGpsRot = new PoseCalcModuleGpsRot(
        moduleID,
        sensorType,
        this,
        gps,
        POSECALC_CONSTANTS_.gps2MlabHeadingFacor,
        POSECALC_CONSTANTS_.gps2MlabHeadingOffset,
        POSECALC_CONSTANTS_.gpsMinValidTransSpeed4Heading,
        POSECALC_CONSTANTS_.gpsMaxValidAngSpeed4Heading);
    moduleRot = moduleGpsRot;

    // Save the module.
    pthread_mutex_lock(&poseCalcModuleListMutex_);
    shPoseCalcModuleLocList_.push_back(moduleLoc);
    shPoseCalcModuleLocIDTable_.push_back(moduleID);
    shPoseCalcModuleRotList_.push_back(moduleRot);
    shPoseCalcModuleRotIDTable_.push_back(moduleID);
    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    return moduleID;
}

//-----------------------------------------------------------------------
// This function sets up the gyro.
//-----------------------------------------------------------------------
int PoseCalc::addGyro(Gyro *gyro)
{
    PoseCalcModuleInterface *moduleRot = NULL;
    PoseCalcModuleGyroRot *moduleGyroRot = NULL;
    int moduleID, sensorType;

    pthread_mutex_lock(&poseCalcModuleIDMutex_);
    moduleID = shPoseCalcModuleID_;
    shPoseCalcModuleID_++;
    pthread_mutex_unlock(&poseCalcModuleIDMutex_);

    sensorType = PoseCalcUtility::name2SensorType(gyro->getName());

    // Create the Rot module.
    moduleGyroRot = new PoseCalcModuleGyroRot(
        moduleID,
        sensorType,
        this,
        gyro);
    moduleRot = moduleGyroRot;

    // Save the module.
    pthread_mutex_lock(&poseCalcModuleListMutex_);
    shPoseCalcModuleRotList_.push_back(moduleRot);
    shPoseCalcModuleRotIDTable_.push_back(moduleID);
    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    return moduleID;
}

//-----------------------------------------------------------------------
// This function sets up the compass.
//-----------------------------------------------------------------------
int PoseCalc::addCompass(Compass *compass)
{
    PoseCalcModuleInterface *moduleRot = NULL;
    PoseCalcModuleCompassRot *moduleCompassRot = NULL;
    int moduleID, sensorType;

    pthread_mutex_lock(&poseCalcModuleIDMutex_);
    moduleID = shPoseCalcModuleID_;
    shPoseCalcModuleID_++;
    pthread_mutex_unlock(&poseCalcModuleIDMutex_);

    sensorType = PoseCalcUtility::name2SensorType(compass->getName());

    // Create the Rot module.
    moduleCompassRot = new PoseCalcModuleCompassRot(
        moduleID,
        sensorType,
        this,
        compass,
        POSECALC_CONSTANTS_.compass2MlabHeadingFacor,
        POSECALC_CONSTANTS_.compass2MlabHeadingOffset,
        POSECALC_CONSTANTS_.compassMaxValidAngSpeed4Heading);
    moduleRot = moduleCompassRot;

    // Save the module.
    pthread_mutex_lock(&poseCalcModuleListMutex_);
    shPoseCalcModuleRotList_.push_back(moduleRot);
    shPoseCalcModuleRotIDTable_.push_back(moduleID);
    pthread_mutex_unlock(&poseCalcModuleListMutex_);

    return moduleID;
}

//-----------------------------------------------------------------------
// This function initilizes the Pose Calculator, and sets the pose to
// be the specified value. (This function will get called when a new
// robot mission starts.)
//-----------------------------------------------------------------------
void PoseCalc::initPose(HSPose_t pose)
{
    PoseCalcFusedPose_t fusedPose;
    PoseCalcGenericData_t data;
    vector<PoseCalcFusedPose_t> fusedPoseList;
    int currentVairanceScheme;
    int i;

    setPoseIniting_(true);
    pthread_mutex_lock(&sensorFusersMutex_);

    // Initialize the sensory bus.
    sensoryDataBus_->initialize();

    // Reset the current pose with this value.
    setPose(pose);

	ipc_send_simulator_set_truepose(pose.loc.x,
		pose.loc.y, pose.rot.yaw * M_PI / 180);

	ipc_send_localize_initialize(pose.loc.x,
		pose.loc.y, pose.rot.yaw * M_PI / 180);

    // During the initialization, use the dynamic variance scheme.
    getVarianceScheme_(currentVairanceScheme);
    setVarianceScheme_(POSECALC_VARIANCE_SCHEME_DYNAMIC);

    // Make sure that buffer is filled.
    setBlockUpdate_(false);
    sensoryDataBus_->waitForData();
    setBlockUpdate_(true);

    for (i = 0; i < (int)(shSensorFusers_.size()); i++)
    {
        // Run the filter.
        fusedPose = (shSensorFusers_[i])->initialize();

        // Apply the environmental situation.
        data.content = (void *)&fusedPose;
        envSitCon_->applySituation(data);

        // Save the data.
        fusedPoseList.push_back(fusedPose);
    }

    pthread_mutex_unlock(&sensorFusersMutex_);

    // Coordinate the poses that were computed by different fusers.
    pthread_mutex_lock(&coordinatorMutex_);
    fusedPose = shCoordinator_->coordinate(fusedPoseList);
    pthread_mutex_unlock(&coordinatorMutex_);

    // Save for what it is.
    setPose(fusedPose.pose);

    // Put back the original variance scheme.
    setVarianceScheme_(currentVairanceScheme);

    // Let the PCModules to resume their work.
    setBlockUpdate_(false);
    setPoseIniting_(false);

    // Wait for the buffer to be filled.
    sensoryDataBus_->waitForData();

    // Save the sensor type if Maximum Confidence is used as a fuser.
    if (fusedPose.fuserType == POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE)
    {
        saveSelectedSensors_(
            fusedPose.extra.locSensorType,
            fusedPose.extra.rotSensorType);
    }
}

//-----------------------------------------------------------------------
// This function resets the pose to zero.
//-----------------------------------------------------------------------
void PoseCalc::resetPose(void)
{
    HSPose_t pose;
    double curTime;

    curTime = getCurrentEpochTime();

    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    pose.extra.time = curTime;
    pose.loc.extra.time = curTime;
    pose.rot.extra.time = curTime;

    initPose(pose);
}

//-----------------------------------------------------------------------
// This function sets the pose.
//-----------------------------------------------------------------------
void PoseCalc::setPose(HSPose_t pose)
{
    double curTime;
    int i;

    // Use the current epoch time as a reference time.
    curTime = getCurrentEpochTime();
    setReferenceTimes_(curTime, curTime);

    pthread_mutex_lock(&poseMutex_);

    // Remember the last value.
    shPoseCalcLastPose_ = shPoseCalcPose_;

    // Copy the current value.
    shPoseCalcPose_ = pose;

    // To Do: XYT will be obsolete.
    // Update the values for the Sensor class.
    shXYT_[0] = pose.loc.x;
    shXYT_[1] = pose.loc.y;
    shXYT_[2] = pose.rot.yaw;

	HSPose_t odometryPose;

	gRobot->getPose(odometryPose);

	ipc_send_localize_globalpos(pose.loc.x,	pose.loc.y,
					pose.rot.yaw * M_PI / 180,
					odometryPose.loc.x,
					odometryPose.loc.y,
					odometryPose.rot.yaw * M_PI / 180);

	if((gRobot->getRobotType() == HS_ROBOT_TYPE_FRED) ||
		(gRobot->getRobotType() == HS_ROBOT_TYPE_CARRETILLA_SIMULACION))
	{
		ipc_send_simulator_set_truepose(pose.loc.x,	pose.loc.y,
				pose.rot.yaw * M_PI / 180);
	}

    pthread_mutex_unlock(&poseMutex_);

    if (FEEDBACK_POSE_TO_MODULES_ || poseIniting_())
    {
        pthread_mutex_lock(&poseCalcModuleListMutex_);

        // Tell location modules to use this pose.
        for (i = 0; i < (int)(shPoseCalcModuleLocList_.size()); i++)
        {
            (shPoseCalcModuleLocList_[i])->setPose(pose);
        }

        // Tell rotation modules to use this pose.
        for (i = 0; i < (int)(shPoseCalcModuleRotList_.size()); i++)
        {
            (shPoseCalcModuleRotList_[i])->setPose(pose);
        }

        pthread_mutex_unlock(&poseCalcModuleListMutex_);
    }
}

//-----------------------------------------------------------------------
// This function retrieves the pose.
//-----------------------------------------------------------------------
void PoseCalc::getPose(HSPose_t &pose)
{
    pthread_mutex_lock(&poseMutex_);
    pose = shPoseCalcPose_;
    pthread_mutex_unlock(&poseMutex_);
}

//-----------------------------------------------------------------------
// This function updates the location.
//-----------------------------------------------------------------------
void PoseCalc::setLocation(HSLocation_t loc)
{
    HSPose_t pose;
    double curTime;
    int i;

    // Use the current epoch time as a reference time.
    curTime = getCurrentEpochTime();
    setLocReferenceTime_(curTime);

    // Copy the location.
    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    pose.loc = loc;

    pthread_mutex_lock(&poseMutex_);

    // Remember the last value.
    shPoseCalcLastPose_.loc = shPoseCalcPose_.loc;
    shPoseCalcLastPose_.extra = shPoseCalcPose_.loc.extra;

    // Copy the current value.
    shPoseCalcPose_.loc = pose.loc;

    // To Do: XYT will be obsolete.
    // Update the values for the Sensor class.
    shXYT_[0] = pose.loc.x;
    shXYT_[1] = pose.loc.y;

	HSPose_t odometryPose;

	gRobot->getPose(odometryPose);

	ipc_send_localize_globalpos(pose.loc.x,	pose.loc.y,
					pose.rot.yaw * M_PI / 180,
					odometryPose.loc.x,
					odometryPose.loc.y,
					odometryPose.rot.yaw * M_PI / 180);

	if((gRobot->getRobotType() == HS_ROBOT_TYPE_FRED) ||
		(gRobot->getRobotType() == HS_ROBOT_TYPE_CARRETILLA_SIMULACION))
	{
		ipc_send_simulator_set_truepose(pose.loc.x,	pose.loc.y,
				pose.rot.yaw * M_PI / 180);
	}

    pthread_mutex_unlock(&poseMutex_);


    if (FEEDBACK_POSE_TO_MODULES_ || poseIniting_())
    {
        pthread_mutex_lock(&poseCalcModuleListMutex_);

        // Tell location modules to use this pose.
        for (i = 0; i < (int)(shPoseCalcModuleLocList_.size()); i++)
        {
            (shPoseCalcModuleLocList_[i])->setPose(pose);
        }

        pthread_mutex_unlock(&poseCalcModuleListMutex_);
    }
}

//-----------------------------------------------------------------------
// This function retrieves the location.
//-----------------------------------------------------------------------
void PoseCalc::getLocation(HSLocation_t &loc)
{
    pthread_mutex_lock(&poseMutex_);
    loc = shPoseCalcPose_.loc;
    pthread_mutex_unlock(&poseMutex_);
}

//-----------------------------------------------------------------------
// This function updates the rotation.
//-----------------------------------------------------------------------
void PoseCalc::setRotation(HSRotation_t rot)
{
    HSPose_t pose;
    double curTime;
    int i;

    // Use the current epoch time as a reference time.
    curTime = getCurrentEpochTime();
    setRotReferenceTime_(curTime);

    // Copy the rotation.
    memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
    pose.rot = rot;

    pthread_mutex_lock(&poseMutex_);

    // Remember the last value.
    shPoseCalcLastPose_.rot = shPoseCalcPose_.rot;

    // Copy the current value.
    shPoseCalcPose_.rot = pose.rot;
    shPoseCalcPose_.extra = pose.rot.extra;

    // To Do: XYT will be obsolete.
    // Update the values for the Sensor class.
    shXYT_[2] = pose.rot.yaw;

	HSPose_t odometryPose;

	gRobot->getPose(odometryPose);

	ipc_send_localize_globalpos(pose.loc.x,	pose.loc.y,
					pose.rot.yaw * M_PI / 180,
					odometryPose.loc.x,
					odometryPose.loc.y,
					odometryPose.rot.yaw * M_PI / 180);

	if((gRobot->getRobotType() == HS_ROBOT_TYPE_FRED) ||
		(gRobot->getRobotType() == HS_ROBOT_TYPE_CARRETILLA_SIMULACION))
	{
		ipc_send_simulator_set_truepose(pose.loc.x,	pose.loc.y,
				pose.rot.yaw * M_PI / 180);
	}

    pthread_mutex_unlock(&poseMutex_);

    if (FEEDBACK_POSE_TO_MODULES_ || poseIniting_())
    {
        pthread_mutex_lock(&poseCalcModuleListMutex_);

        // Tell rotation modules to use this pose.
        for (i = 0; i < (int)(shPoseCalcModuleRotList_.size()); i++)
        {
            (shPoseCalcModuleRotList_[i])->setPose(pose);
        }

        pthread_mutex_unlock(&poseCalcModuleListMutex_);
    }
}

//-----------------------------------------------------------------------
// This function retrieves the rotation info.
//-----------------------------------------------------------------------
void PoseCalc::getRotation(HSRotation_t &rot)
{
    pthread_mutex_lock(&poseMutex_);
    rot = shPoseCalcPose_.rot;
    pthread_mutex_unlock(&poseMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the previous pose info.
//-----------------------------------------------------------------------
void PoseCalc::getLastPose(HSPose_t &lastPose)
{
    pthread_mutex_lock(&poseMutex_);
    lastPose = shPoseCalcLastPose_;
    pthread_mutex_unlock(&poseMutex_);
}

//-----------------------------------------------------------------------
// This function updates the selected module names.
//-----------------------------------------------------------------------
void PoseCalc::setSelectedSensors_(int sensorTypeLoc, int sensorTypeRot)
{
    pthread_mutex_lock(&selectedSensorMutex_);
    shSelectedSensorLoc_ = sensorTypeLoc;
    shSelectedSensorRot_ = sensorTypeRot;
    pthread_mutex_unlock(&selectedSensorMutex_);
}

//-----------------------------------------------------------------------
// This function updates the selected module names.
//-----------------------------------------------------------------------
void PoseCalc::setSelectedSensor_(int sensorType, int poseType)
{
    pthread_mutex_lock(&selectedSensorMutex_);

    switch (poseType){

    case POSECALC_POSE_TYPE_LOCATION:
      shSelectedSensorLoc_ = sensorType;
      break;

    case POSECALC_POSE_TYPE_ROTATION:
      shSelectedSensorRot_ = sensorType;
      break;
    }

    pthread_mutex_unlock(&selectedSensorMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves the selected module names.
//-----------------------------------------------------------------------
void PoseCalc::getSelectedSensors_(int &sensorTypeLoc, int &sensorTypeRot)
{
    pthread_mutex_lock(&selectedSensorMutex_);
    sensorTypeLoc = shSelectedSensorLoc_;
    sensorTypeRot = shSelectedSensorRot_;
    pthread_mutex_unlock(&selectedSensorMutex_);
}

//-----------------------------------------------------------------------
// This function removes the specified module from the list.
//-----------------------------------------------------------------------
void PoseCalc::disconnectModule(int moduleID)
{
    vector<PoseCalcModuleInterface *>::iterator moduleIter;
    vector<int>::iterator idIter;
    int index;

    pthread_mutex_lock(&poseCalcModuleListMutex_);

    idIter = find(
        shPoseCalcModuleLocIDTable_.begin(),
        shPoseCalcModuleLocIDTable_.end(),
        moduleID);

    if (idIter != shPoseCalcModuleLocIDTable_.end())
    {
        // Remember the index.
        index = idIter - shPoseCalcModuleLocIDTable_.begin();
        moduleIter = shPoseCalcModuleLocList_.begin() + index;

        // Erase the entry from the table.
        shPoseCalcModuleLocIDTable_.erase(idIter);

        // Erase the module from the list.
        (shPoseCalcModuleLocList_[index])->die();
        shPoseCalcModuleLocList_[index] = NULL;
        shPoseCalcModuleLocList_.erase(moduleIter);
    }

    idIter = find(
        shPoseCalcModuleRotIDTable_.begin(),
        shPoseCalcModuleRotIDTable_.end(),
        moduleID);

    if (idIter != shPoseCalcModuleRotIDTable_.end())
    {
        // Remember the index.
        index = idIter - shPoseCalcModuleRotIDTable_.begin();
        moduleIter = shPoseCalcModuleRotList_.begin() + index;

        // Erase the entry from the table.
        shPoseCalcModuleRotIDTable_.erase(idIter);

        // Erase the module from the list.
        (shPoseCalcModuleRotList_[index])->die();
        shPoseCalcModuleRotList_[index] = NULL;
        shPoseCalcModuleRotList_.erase(moduleIter);
    }

    pthread_mutex_unlock(&poseCalcModuleListMutex_);
}

//-----------------------------------------------------------------------
// This function logs the data.
//-----------------------------------------------------------------------
void PoseCalc::logData_(double logTime)
{
    HSPose_t pose;
    int fuserType;

    if (logfile_ != NULL)
    {
        getPose(pose);
        getSensorFuserType_(fuserType);

        fprintf(
            logfile_,
            "%f %d %.2f %.2f %.2f %.2f %.2f %.2f %f %f %f\n",
            logTime,
            fuserType,
            pose.loc.x,
            pose.loc.y,
            pose.loc.z,
            pose.rot.yaw,
            pose.rot.pitch,
            pose.rot.roll,
            pose.extra.time,
            pose.loc.extra.time,
            pose.rot.extra.time);

        fflush(logfile_);
    }
}

//-----------------------------------------------------------------------
// This function sets the flag of shBlockUpdate_.
//-----------------------------------------------------------------------
void PoseCalc::setBlockUpdate_(bool blockUpdate)
{
    pthread_mutex_lock(&blockUpdateMutex_);
    shBlockUpdate_ = blockUpdate;
    pthread_mutex_unlock(&blockUpdateMutex_);
}

//-----------------------------------------------------------------------
// This function waits for the flag shBlockUpdate being released.
//-----------------------------------------------------------------------
bool PoseCalc::blockUpdate_(void)
{
    bool blockUpdate = false;

    pthread_mutex_lock(&blockUpdateMutex_);
    blockUpdate = shBlockUpdate_;
    pthread_mutex_unlock(&blockUpdateMutex_);

    return blockUpdate;
}

//-----------------------------------------------------------------------
// This function waits for the flag shBlockUpdate being released.
//-----------------------------------------------------------------------
void PoseCalc::waitForBlockUpdateRelease_(void)
{
    while (true)
    {
        if (!blockUpdate_())
        {
            return;
        }

        usleep(WAIT_BLOCK_UPDATE_RELEASE_USLEEP_);

        // Make sure termination was not requested.
        pthread_testcancel();
    }
}

//-----------------------------------------------------------------------
// This function sets the flag of shPoseIniting_.
//-----------------------------------------------------------------------
void PoseCalc::setPoseIniting_(bool poseIniting)
{
    pthread_mutex_lock(&poseInitingMutex_);
    shPoseIniting_ = poseIniting;
    pthread_mutex_unlock(&poseInitingMutex_);
}

//-----------------------------------------------------------------------
// This function returns the flag of shPoseIniting_.
//-----------------------------------------------------------------------
bool PoseCalc::poseIniting_(void)
{
    bool poseIniting = false;

    pthread_mutex_lock(&poseInitingMutex_);
    poseIniting = shPoseIniting_;
    pthread_mutex_unlock(&poseInitingMutex_);

    return poseIniting;
}

//-----------------------------------------------------------------------
// This function sets the reference times for the location.
//-----------------------------------------------------------------------
void PoseCalc::setLocReferenceTime_(double refTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    shLocRefTime_ = refTime;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function returns the reference times for the location.
//-----------------------------------------------------------------------
void PoseCalc::getLocReferenceTime(double &refTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    refTime = shLocRefTime_;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function sets the reference times for the rotation.
//-----------------------------------------------------------------------
void PoseCalc::setRotReferenceTime_(double refTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    shRotRefTime_ = refTime;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function returns the reference times for the rotation.
//-----------------------------------------------------------------------
void PoseCalc::getRotReferenceTime(double &refTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    refTime = shRotRefTime_;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function returns the reference times for both location and
// rotation.
//-----------------------------------------------------------------------
void PoseCalc::setReferenceTimes_(double locRefTime, double rotRefTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    shLocRefTime_ = locRefTime;
    shRotRefTime_ = rotRefTime;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function returns the reference times for both location and
// rotation.
//-----------------------------------------------------------------------
void PoseCalc::getReferenceTimes(double &locRefTime, double &rotRefTime)
{
    pthread_mutex_lock(&refTimeMutex_);
    locRefTime = shLocRefTime_;
    rotRefTime = shRotRefTime_;
    pthread_mutex_unlock(&refTimeMutex_);
}

//-----------------------------------------------------------------------
// This function sets up the user interface.
//-----------------------------------------------------------------------
void PoseCalc::control(void)
{
    PoseCalcCoordinatorPickFilter *coordinatorPickFilter = NULL;
    EnMessageErrType msgNum;
    int mainCh, fuserCh, varCh;
    int fuserType, varianceScheme;
    bool done = false;

    messageDrawWindow(EnMessageType_POSECALC_CONTROL, EnMessageErrType_NONE);

    while (!done)
    {
        mainCh = getch();

        switch(mainCh) {

        case 'f':
            // Ask the user to pick a filter
            messageDrawWindow(
                EnMessageType_POSECALC_FUSER_TYPE,
                EnMessageErrType_NONE);

            msgNum = EnMessageErrType_NONE;
            fuserType = POSECALC_FUSER_TYPE_NONE;
            fuserCh = getch();

            switch(fuserCh) {

            case '1':
                fuserType = POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE;
                break;

            case '2':
                fuserType = POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER;
                break;

            case '3':
                fuserType = POSECALC_FUSER_TYPE_PARTICLE_FILTER;
                break;

            case 'c':
                break;

            case 'x':
                done = true;
                break;

            default:
                msgNum = EnMessageErrType_POSECALC_FUSER_TYPE;
                break;
            };

            if (fuserType != POSECALC_FUSER_TYPE_NONE)
            {
                // Filter chosen. Re-instantiate the coordinator with this fuser.
                coordinatorPickFilter = new PoseCalcCoordinatorPickFilter(fuserType);

                pthread_mutex_lock(&coordinatorMutex_);

                if (shCoordinator_ != NULL)
                {
                    delete shCoordinator_;
                }

                shCoordinator_ = coordinatorPickFilter;

                pthread_mutex_unlock(&coordinatorMutex_);

                // Remember this filter type.
                setSensorFuserType_(fuserType);

                done = true;
            }

            messageDrawWindow(EnMessageType_POSECALC_CONTROL, msgNum);
            break;

        case 'v':
            // Ask the user to pick a varianceScheme
            messageDrawWindow(
                EnMessageType_POSECALC_VARIANCE_SCHEME,
                EnMessageErrType_NONE);

            msgNum = EnMessageErrType_NONE;
            varianceScheme = POSECALC_VARIANCE_SCHEME_NONE;
            varCh = getch();

            switch(varCh) {

            case '1':
                varianceScheme = POSECALC_VARIANCE_SCHEME_DYNAMIC;
                break;

            case '2':
                varianceScheme = POSECALC_VARIANCE_SCHEME_STATIC;
                break;

            case 'c':
                break;

            case 'x':
                done = true;
                break;

            default:
                msgNum = EnMessageErrType_POSECALC_VARIANCE_SCHEME;
                break;
            };

            if (varianceScheme != POSECALC_VARIANCE_SCHEME_NONE)
            {
                setVarianceScheme_(varianceScheme);
                done = true;
            }

            messageDrawWindow(EnMessageType_POSECALC_CONTROL, msgNum);
            break;

        case 'r':
            resetPose();
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
            messageMovePanelKey(mainCh);
            break;
        }
    }

    messageHide();
}

//-----------------------------------------------------------------------
// This function prints out the debugging message that is called inside
// ParticleFilter.
//-----------------------------------------------------------------------
void ParticleFilterPrintf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printfTextWindow("ParticleFilter: %s", buf);
}

//-----------------------------------------------------------------------
// This function prints out the debugging message that is called inside
// Kalman.
//-----------------------------------------------------------------------
void KalmanPrintf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printfTextWindow("Kalman: %s", buf);
}

/**********************************************************************
# $Log: PoseCalculator.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:17  endo
# New PoseCalc integrated.
#
# Revision 1.9  2004/05/13 23:08:33  endo
# It can now log the latitude and longitude info.
#
# Revision 1.8  2004/05/13 23:04:59  endo
# Some bugs found during the Ft. Benning trip (May, 2004) fixed.
#
# Revision 1.7  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.6  2004/04/21 17:15:45  mpowers
# Rotated gps and compass headings so East is 0 and North is 270.
#
# Revision 1.5  2004/04/19 08:49:45  endo
# Particle filter fixed. Visualization option added.
#
# Revision 1.4  2004/04/17 12:20:34  endo
# Intalling JBox Sensor Update.
#
# Revision 1.3  2004/04/15 21:51:14  mpowers
# Added visualization for particle filter.
#
# Revision 1.2  2004/04/09 13:42:51  mpowers
# added support for particle filter for localization sensor fusion
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
#**********************************************************************/
