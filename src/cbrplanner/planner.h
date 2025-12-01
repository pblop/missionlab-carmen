/**********************************************************************
 **                                                                  **
 **                              planner.h                           **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef PLANNER_H
#define PLANNER_H

/* $Id: planner.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <string>

#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "repair_plan_types.h"
#include "repair_plan.h"
#include "cnp_server_manager.h"

using std::string;

class Planner {

    typedef enum {
        DEPLOY_BY_GOTO,
        DEPLOY_BY_LOCALIZE
    };

protected:
    Planner **self_;
    RepairPlan *repairPlan_;
    CNP_Server_Manager *cnpServerManager_;
    CBRPlanner_CNPStatus_t currentCNPStatus_;
    vector<MissionMemoryManager::MissionPlan_t> currentMissionPlans_;
    int cnpMode_;

    static const string EMPTY_STRING_;
    static const string STRING_EMPTY_ARCHITECTURE_;
    static const string STRING_START_STATE_;
    static const string STRING_START_STATE_INDEX_;
    static const string STRING_START_STATE_DESC_;
    static const string STRING_IMMEDIATE_;
    static const string STRING_START_SUB_MISSION_;
    static const string STRING_START_SUB_MISSION_INDEX_;
    static const string STRING_START_SUB_MISSION_DESC_;
    static const string STRING_SUB_MISSION_NAME_;
    static const string STRING_GOAL_LOCATION_;
    static const string STRING_MOVE_TO_LOCATION_GAIN_;
    static const string STRING_WANDER_GAIN_;
    static const string STRING_AVOID_OBSTACLE_GAIN_;
    static const string STRING_AVOID_OBSTACLE_SPHERE_;
    static const string STRING_AVOID_OBSTACLE_SAFETY_MARGIN_;
    static const string STRING_SUB_MISSION_READY_;
    static const string STRING_SUB_MISSION_READY_INDEX_;
    static const string STRING_SUB_MISSION_DEPLOYMENT_;
    static const string STRING_DEPLOY_FIRST_;
    static const string STRING_EXECUTE_IMMEDIATELY_;
    static const string STRING_GOAL_TOLERANCE_;
    static const string STRING_DEPLOYMENT_METHOD_;
    static const string STRING_DEPLOY_BY_GOTO_;
    static const string STRING_DEPLOY_BY_LOCALIZE_;
    static const string STRING_TARGET_LATITUDE_;
    static const string STRING_TARGET_LONGITUDE_;
    static const string STRING_TIME_WINDOW_;
    static const string STRING_COLON_;
    static const string STRING_PLUS_;
    static const string STRING_SLASH_;
    static const string STRING_NA_;
    static const string FEATURE_NAME_MAX_VELOCITY_;
    static const string FEATURE_NAME_AGGRESSIVENESS_;
    static const string FEATURE_NAME_NUM_ROBOTS_;
    static const string FEATURE_NAME_NO_COMM_ZONE_;
    static const string FEATURE_NAME_WEATHER_ZONE_;
    static const string FEATURE_NAME_NO_FLY_ZONE_;
    static const string FEATURE_NAME_START_X_;
    static const string FEATURE_NAME_START_Y_;
    static const string FEATURE_NAME_START_LATITUDE_;
    static const string FEATURE_NAME_START_LONGITUDE_;
    static const string FEATURE_NAME_TASK_NAME_;
    static const string FEATURE_NAME_USE_LANDMARK_;
    static const string FEATURE_NAME_USE_LANDMARK2_;
    static const string FEATURE_NAME_LOCALIZATION_;
    static const string FEATURE_NAME_LOCALIZATION2_;
    static const string FEATURE_NAME_ENVIRONMENT_;
    static const string FEATURE_NAME_ENVIRONMENT2_;
    static const string FEATURE_NAME_ROBOT_ID_;
    static const string FEATURE_NAME_MISSION_TIME_;
    static const string FEATURE_VALUE_USEMAPS_;
    static const string FEATURE_VALUE_USEMAPS2_;
    static const string FEATURE_VALUE_INDOOR_;
    static const string FEATURE_VALUE_INDOOR2_;
    static const string CONSTRAINT_NAME_VEHICLE_TYPES_;
    static const string CONSTRAINT_VALUE_UUV_;
    static const string CONSTRAINT_VALUE_UGV_;
    static const string CONSTRAINT_VALUE_USV_;
    static const string CONSTRAINT_VALUE_UAV_;
    static const string MISSION_TIME_FORMAT_;
    static const char POLYGON_ENCODER_KEY_XY_START_;
    static const char POLYGON_ENCODER_KEY_XY_END_;
    static const char POLYGON_ENCODER_KEY_LATLON_START_;
    static const char POLYGON_ENCODER_KEY_LATLON_END_;
    static const char POLYGON_ENCODER_KEY_POINT_SEPARATOR_;
    static const char POLYGON_ENCODER_KEY_POINTS_SEPARATOR_;
    static const char POLYGON_ENCODER_KEY_POLY_SEPARATOR_;
    static const double DEFAULT_MOVE_TO_LOCATION_GAIN_;
    static const double DEFAULT_WANDER_GAIN_;
    static const double DEFAULT_INDOOR_AVOID_OBSTACLE_GAIN_;
    static const double DEFAULT_OUTDOOR_AVOID_OBSTACLE_GAIN_;
    static const double DEFAULT_INDOOR_AVOID_OBSTACLE_SPHERE_;
    static const double DEFAULT_OUTDOOR_AVOID_OBSTACLE_SPHERE_;
    static const double DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_;
    static const double DEFAULT_GOAL_TOLERANCE_;
    static const long DEFAULT_TIME_WINDOW_RANGE_SEC_;
    static const int MAX_CNP_PLANS_;
    static const int INVALID_ROBOT_ID_;
    static const bool FORCE_SUB_MISSION_READY_DEPLOY_FIRST_;
    static const bool USE_STATIC_GOAL_TOLERANCE_;
    static const bool ADAPT_GEOGRAPHIC_COORDINATE_;
    static const bool ADAPT_TIME_WINDOW_;

    MissionMemoryManager::MissionPlanList_t separateSubMissions_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        MemoryManager::Features_t features,
        vector<MemoryManager::Features_t> *featuresList);
    MissionMemoryManager::Coordinates_t extractCoordinates_(
        MemoryManager::Features_t features);
    MissionMemoryManager::GeoCoordinates_t extractGeoCoordinates_(
        MemoryManager::Features_t features);
    MissionMemoryManager::MissionPlan_t addStartSubMissionState_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        string taskName,
        MissionMemoryManager::Coordinates_t coordinates,
        bool useMaps,
        bool indoor,
        double maxVel,
        int deploymentMethod);
    MissionMemoryManager::MissionPlan_t addStartState_(
        MissionMemoryManager::MissionPlan_t missionPlan);
    MemoryManager::Features_t renameTaskInFeatures_(
        MemoryManager::Features_t features,
        string taskName);
    MemoryManager::Features_t filterNonIndexFeatures_(
        MemoryManager::Features_t features);
    MemoryManager::Features_t setNumRobots_(
        MemoryManager::Features_t features,
        int numRobots);
    MemoryManager::Features_t sortFeatures_(
        MemoryManager::Features_t features);
    CBRPlanner_PlanOperator_t createStartOperator_(void);
    CBRPlanner_PlanOperator_t createStartSubMissionOperator_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        string taskName,
        MissionMemoryManager::Coordinates_t coordinates,
        bool useMaps,
        bool indoor,
        double maxVel,
        int deploymentMethod);
    vector<MemoryManager::Features_t> extractSubFeatures_(
        MemoryManager::Features_t features);
    vector<MissionMemoryManager::MissionPlan_t> sortMissionPlans_(
        vector<MissionMemoryManager::MissionPlan_t> missionPlans);

    vector<MissionMemoryManager::MissionPlan_t> appendMissionPlans_(
        vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans,
        vector<MissionMemoryManager::MissionPlan_t> missionPlans,
        int trackingNumber);
    vector<MissionMemoryManager::MissionPlan_t> updateTargetLatLon_(
        vector<MissionMemoryManager::MissionPlan_t> missionPlans,
        MissionMemoryManager::GeoCoordinates_t geoCoordinates);
    vector<MissionMemoryManager::MissionPlan_t> updateTimeWindow_(
        vector<MissionMemoryManager::MissionPlan_t> missionPlans,
        string missionTimeString);
    MissionMemoryManager::MissionPlan_t adjustNumRobots_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        int numRobots);
    vector<int> mergeDataIndexLists_(
        vector<int> dataIndexListA,
        vector<int> dataIndexListB);
    vector<MissionMemoryManager::MissionPlan_t> mergeMissionPlans_(
        vector<MissionMemoryManager::MissionPlan_t> missionPlansA,
        vector<MissionMemoryManager::MissionPlan_t> missionPlansB);
    vector<int> trimRedundantDataIndex_(vector<int> dataIndexList);
    string getTaskNameInFeatures_(MemoryManager::Features_t features);
    string extractTaskNameFromStartSubMission_(
        CBRPlanner_PlanOperator_t startSubMissionOperator);
    string extractValueContent_(string value, bool withoutQuote);
    string extractMaxVelocityValue_(
        MemoryManager::Features_t features);
    string extractAggressivenessValue_(
        MemoryManager::Features_t features);
    string extractTaskName_(MemoryManager::Features_t features);
    string extractRobotVehicleType_(
        int robotID,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    string extractMissionTime_(MemoryManager::Features_t features);
    int extractNumRobots_(MemoryManager::Features_t features);
    int extractRobotID_(MemoryManager::Features_t features);
    void clearCurrentMissionPlans_(void);
    bool extractUseMaps_(MemoryManager::Features_t features);
    bool extractIndoor_(MemoryManager::Features_t features);
    void filterBySpecialZones_(
        vector<MissionMemoryManager::MissionPlan_t>* bigMissionPlans,
        MemoryManager::Features_t features,
        vector<string> zones);
    bool pointInPolygon_(
        MissionMemoryManager::Coordinates_t point,
        MissionMemoryManager::PointList_t polygon);
    void extractAllTargetGeoLocations_(
        MissionMemoryManager::MissionPlan_t missionPlan, 
        MissionMemoryManager::PointList_t* geoPoints);
    bool extractAllPolygons_(
        string polygonString,
        MissionMemoryManager::PolygonSet_t* geoPolySet,
        MissionMemoryManager::PolygonSet_t* polySet);

public:
    Planner(void);
    Planner(Planner **self);
    ~Planner(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
    MissionMemoryManager::MissionPlan_t reverseRobotOrder(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
    MissionMemoryManager::MissionPlan_t mergeMissions(
        vector<MissionMemoryManager::MissionPlan_t> missionplans,
        int *numRobots,
        bool *fixed);
    RepairQuestion_t getRepairQuestion(
        int lastSelectedOption);
    RepairQuestion_t getRepairFollowUpQuestion(void);
    CBRPlanner_CNPStatus_t getCurrentCNPStatus(void);
    vector<MissionMemoryManager::MissionPlan_t> getMissionPlans(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission);
    vector<MissionMemoryManager::MissionPlan_t> getMissionPlansCBROnly(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission);
    vector<MissionMemoryManager::MissionPlan_t> getMissionPlansPremissionCNP(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission,
        vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    vector<MissionMemoryManager::MissionPlan_t> getMissionPlansRuntimeCNP(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    vector<int> saveNewMissionPlan(
        MissionMemoryManager::MissionPlan_t missionPlan,
        MemoryManager::Features_t features,
        bool hasStartSubMission);
    vector<CBRPlanner_RobotIDs_t> getCurrentRobotIDList(void);
    int toggleCNP(void);
    void setCNPMode(int cnpMode);
    void putRepairFollowUpQuestionToggleAnswer(int selectedOption);
    void putRepairFollowUpQuestionAddMapPointAnswer(double x, double y);
    void putRepairFollowUpQuestionModMapPointAnswer(
        double oldX,
        double oldY,
        double newX,
        double newY,
        double closeRange);
    void putRepairFollowUpQuestionDelMapPointAnswer(
        double x,
        double y,
        double closeRange);
    void updateOffendingState(OffendingState_t offendingState);
};

inline RepairQuestion_t Planner::getRepairQuestion(
    int lastSelectedOption)
{
    return (repairPlan_->getRepairQuestion(lastSelectedOption));
}

inline RepairQuestion_t Planner::getRepairFollowUpQuestion(void)
{
    return (repairPlan_->getRepairFollowUpQuestion());
}

inline void Planner::putRepairFollowUpQuestionToggleAnswer(int selectedOption)
{
    repairPlan_->putRepairFollowUpQuestionToggleAnswer(selectedOption);
}

inline void Planner::putRepairFollowUpQuestionAddMapPointAnswer(double x, double y)
{
    repairPlan_->putRepairFollowUpQuestionAddMapPointAnswer(x, y);
}

inline void Planner::putRepairFollowUpQuestionModMapPointAnswer(
    double oldX,
    double oldY,
    double newX,
    double newY,
    double closeRange)
{
    repairPlan_->putRepairFollowUpQuestionModMapPointAnswer(
        oldX, oldY, newX, newY, closeRange);
}

inline void Planner::putRepairFollowUpQuestionDelMapPointAnswer(
    double x,
    double y,
    double closeRange)
{
    repairPlan_->putRepairFollowUpQuestionDelMapPointAnswer(x, y, closeRange);
}

inline void Planner::updateOffendingState(
    OffendingState_t offendingState)
{
    repairPlan_->updateOffendingState(offendingState);
}

inline CBRPlanner_CNPStatus_t Planner::getCurrentCNPStatus(void)
{
    return currentCNPStatus_;
}

#endif
/**********************************************************************
 * $Log: planner.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.6  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.5  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.4  2007/09/18 22:34:23  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.3  2007/01/30 15:12:42  nadeem
 * Added code to filter the cases based on No Fly, No Comm and Weather Zones
 *
 * Revision 1.2  2006/09/26 18:30:59  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.12  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.11  2006/02/19 17:52:00  endo
 * Experiment related modifications
 *
 * Revision 1.10  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.9  2005/09/22 21:43:50  endo
 * CNP version can now suggest multiple solutions.
 *
 * Revision 1.8  2005/08/12 21:48:31  endo
 * More improvements for August demo.
 *
 * Revision 1.7  2005/07/31 03:41:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.6  2005/07/27 20:40:17  endo
 * 3D visualization improved.
 *
 * Revision 1.5  2005/07/16 08:49:22  endo
 * CBR-CNP integration
 *
 * Revision 1.4  2005/06/23 22:09:05  endo
 * Adding the mechanism to talk to CNP.
 *
 * Revision 1.3  2005/05/18 21:22:23  endo
 * AuRA.naval added.
 *
 * Revision 1.2  2005/02/07 19:53:44  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
