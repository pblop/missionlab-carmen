/**********************************************************************
 **                                                                  **
 **                         PoseCalculator.h                         **
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

/* $Id: PoseCalculator.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSE_CALCULATOR_H
#define POSE_CALCULATOR_H

#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <vector>
#include <string>

#include "sensors.h"
#include "robot.h"
#include "gps.h"
#include "compass.h"
#include "gyro.h"
#include "PoseCalculatorTypes.h"
#include "PoseCalculatorInterface.h"
#include "PCModuleInterface.h"
#include "PCSensoryDataBusInterface.h"
#include "PCSensorFuserInterface.h"
#include "PCSituationalContextInterface.h"
#include "PCCoordinatorInterface.h"

using std::vector;

// This macro takes a degree (d) and change its value to be inside
// the lower bound (l) and the higher bound (h).
#define POSECALC_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}
#define POSECALC_RAD2DEG(val) ((val)*180.0/M_PI)
#define POSECALC_DEG2RAD(val) ((val)*M_PI/180.0)

class PoseCalc : public PoseCalcInterface, public Module {

protected:
    PoseCalcSituationalContextInterface *sensorySitCon_;
    PoseCalcSituationalContextInterface *envSitCon_;
    PoseCalcSensoryDataBusInterface *sensoryDataBus_;
    PoseCalcCoordinatorInterface *shCoordinator_; // Shared
    Sensor *sensor_;
    HSPose_t shPoseCalcPose_; // Shared
    HSPose_t shPoseCalcLastPose_; // Shared
    vector<PoseCalcModuleInterface *> shPoseCalcModuleLocList_; // Shared
    vector<PoseCalcModuleInterface *> shPoseCalcModuleRotList_; // Shared
    vector<PoseCalcSensorFuserInterface *> shSensorFusers_; // Shared
    vector<int> shPoseCalcModuleLocIDTable_; // Shared
    vector<int> shPoseCalcModuleRotIDTable_; // Shared
    pthread_t mainThread_;
    double shLocRefTime_; // Shared
    double shRotRefTime_; // Shared
    float shXYT_[3]; // Shared
    int shSensorFuserType_; // Shared
    int shPoseCalcModuleID_; // Shared
    int shSelectedSensorLoc_; // Shared
    int shSelectedSensorRot_; // Shared
    int shVarianceScheme_; // Shared
    bool shBlockUpdate_;
    bool shPoseIniting_;

    static pthread_mutex_t poseMutex_;
    static pthread_mutex_t poseCalcModuleListMutex_;
    static pthread_mutex_t poseCalcModuleIDMutex_;
    static pthread_mutex_t selectedSensorMutex_;
    static pthread_mutex_t sensorFusersMutex_;
    static pthread_mutex_t blockUpdateMutex_;
    static pthread_mutex_t poseInitingMutex_;
    static pthread_mutex_t refTimeMutex_;
    static pthread_mutex_t coordinatorMutex_;
    static pthread_mutex_t sensorFuserTypeMutex_;
    static pthread_mutex_t varianceSchemeMutex_;

    const PoseCalcConstants_t POSECALC_CONSTANTS_;

    static const string EMPTY_STRING_;
    static const int MAIN_THREAD_USLEEP_;
    static const int WAIT_BLOCK_UPDATE_RELEASE_USLEEP_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int DEFAULT_COORDINATOR_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;
    static const bool FEEDBACK_POSE_TO_MODULES_;
    
    void setupSensorySituationalContext_(void);
    void setupEnvironmentalSituationalContext_(void);
    void setupSensoryDataBus_(void);
    void setupSensorFusers_(void);
    void setupCoordinator_(void);
    void setupOutput_(void);
    void mainLoop_(void);
    void updatePose_(void);
    void saveFusedPose_(PoseCalcFusedPose_t fusedPose);
    void updateStatusBar_(void);
    void setSelectedSensor_(int sensorType, int poseType);
    void setSelectedSensors_(int sensorTypeLoc, int sensorTypeRot);
    void getSelectedSensors_(int &sensorTypeLoc, int &sensorTypeRot);
    void saveSelectedSensors_(int sensorTypeLoc, int sensorTypeRot);
    void logData_(double logTime);
    void updateModuleLocation_(int moduleID, HSLocation_t loc);
    void updateModuleRotation_(int moduleID, HSRotation_t rot);
    void setBlockUpdate_(bool blockUpdate);
    void waitForBlockUpdateRelease_(void);
    void setPoseIniting_(bool poseIniting);
    void setLocReferenceTime_(double refTime);
    void setRotReferenceTime_(double refTime);
    void setReferenceTimes_(double locRefTime, double rotRefTime);
    void setSensorFuserType_(int fuserType);
    void getSensorFuserType_(int &fuserType);
    void setVarianceScheme_(int varianceScheme);
    void getVarianceScheme_(int &varianceScheme);
    bool poseIniting_(void);
    bool blockUpdate_(void);

    static void* startMainThread_(void* poseCalcInstance);


public:
    static const double MIN_CONFIDENCE_LEVEL;
    static const double MAX_CONFIDENCE_LEVEL;

    PoseCalc(
        PoseCalcConstants_t poseCalcConstants,
        int sensorFuserType,
        int varianceScheme);
    ~PoseCalc(void);

    int addRobot(Robot *robot);
    int addGyro(Gyro *gyro);
    int addGps(Gps *gps);
    int addCompass(Compass *compass);
    int getDebugReportLevel(void);
    void initPose(HSPose_t pose);
    void resetPose(void);
    void setPose(HSPose_t pose);
    void getPose(HSPose_t &pose);
    void getLastPose(HSPose_t &lastPose);
    void setLocation(HSLocation_t loc);
    void getLocation(HSLocation_t &loc);
    void setRotation(HSRotation_t rot);
    void getRotation(HSRotation_t &rot);
    void getVelocity(HSVelocity_t &vel);
    void updateModulePose(int moduleID, HSPose_t pose);
    void updateModuleLocation(int moduleID, HSLocation_t loc);
    void updateModuleRotation(int moduleID, HSRotation_t rot);
    void disconnectModule(int moduleID);
    void getLocReferenceTime(double &refTime);
    void getRotReferenceTime(double &refTime);
    void getReferenceTimes(double &locRefTime, double &rotRefTime);
    void control(void);
};

#endif

/**********************************************************************
# $Log: PoseCalculator.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:19  endo
# New PoseCalc integrated.
#
#**********************************************************************/
