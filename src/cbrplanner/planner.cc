/**********************************************************************
 **                                                                  **
 **                              planner.cc                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module builds a mission based by adjusting ballpark        **
 **  solutions suggested by MemoryManager.                           **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: planner.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>
#include <string>
#include <vector>

#include "cbrplanner_domain.h"
#include "planner.h"
#include "cbrplanner.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "repair_plan.h"
#include "windows.h"
#include "debugger.h"
#include "cnp_server_manager.h"
#include "cnp_types.h"
#include "cnp_constants.h"

using std::string;
using std::vector;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string Planner::EMPTY_STRING_ = "";
const string Planner::STRING_EMPTY_ARCHITECTURE_ = "free";
const string Planner::STRING_START_STATE_ = "Stop";
const string Planner::STRING_START_STATE_INDEX_ = "Start";
const string Planner::STRING_START_STATE_DESC_ = "Start";
const string Planner::STRING_IMMEDIATE_ = "Immediate";
const string Planner::STRING_START_SUB_MISSION_ = "StartSubMission";
const string Planner::STRING_START_SUB_MISSION_INDEX_ = "$StartSubMission";
const string Planner::STRING_SUB_MISSION_NAME_ = "%sub_mission_name";
const string Planner::STRING_GOAL_LOCATION_ = "%Goal_Location";
const string Planner::STRING_MOVE_TO_LOCATION_GAIN_ = "%move_to_location_gain";
const string Planner::STRING_WANDER_GAIN_ = "%wander_gain";
const string Planner::STRING_AVOID_OBSTACLE_GAIN_ = "%avoid_obstacle_gain";
const string Planner::STRING_AVOID_OBSTACLE_SPHERE_ = "%avoid_obstacle_sphere";
const string Planner::STRING_AVOID_OBSTACLE_SAFETY_MARGIN_ = "%avoid_obstacle_safety_margin";
const string Planner::STRING_SUB_MISSION_READY_ = "SubMissionReady";
const string Planner::STRING_SUB_MISSION_DEPLOYMENT_ = "%sub_mission_deployment";
const string Planner::STRING_DEPLOY_FIRST_ = "{DEPLOY_FIRST}";
const string Planner::STRING_EXECUTE_IMMEDIATELY_ = "{EXECUTE_IMMEDIATELY}";
const string Planner::STRING_GOAL_TOLERANCE_ = "%Goal_Tolerance";
const string Planner::STRING_DEPLOYMENT_METHOD_ = "%deployment_method";
const string Planner::STRING_DEPLOY_BY_GOTO_ = "{DEPLOY_BY_GOTO}";
const string Planner::STRING_DEPLOY_BY_LOCALIZE_ = "{DEPLOY_BY_LOCALIZE}";
const string Planner::STRING_TARGET_LATITUDE_ = "%target_latitude";
const string Planner::STRING_TARGET_LONGITUDE_ = "%target_longitude";
const string Planner::STRING_TIME_WINDOW_ = "%time_window";
const string Planner::STRING_COLON_ = ":";
const string Planner::STRING_PLUS_ = "+";
const string Planner::STRING_SLASH_ = "/";
const string Planner::STRING_NA_ = "N/A";
const string Planner::FEATURE_NAME_MAX_VELOCITY_ = "MaxVelocity";
const string Planner::FEATURE_NAME_AGGRESSIVENESS_ = "Aggressiveness";
const string Planner::FEATURE_NAME_NUM_ROBOTS_ = "NumberOfRobots";
const string Planner::FEATURE_NAME_NO_FLY_ZONE_ = "NoFlyZone";
const string Planner::FEATURE_NAME_NO_COMM_ZONE_ = "NoCommZone";
const string Planner::FEATURE_NAME_WEATHER_ZONE_ = "WeatherZone";
const string Planner::FEATURE_NAME_START_X_ = "StartXCoord";
const string Planner::FEATURE_NAME_START_Y_ = "StartYCoord";
const string Planner::FEATURE_NAME_START_LATITUDE_ = "StartLatitude";
const string Planner::FEATURE_NAME_START_LONGITUDE_ = "StartLongitude";
const string Planner::FEATURE_NAME_TASK_NAME_ = "Task";
const string Planner::FEATURE_NAME_LOCALIZATION_ = "Localization";
const string Planner::FEATURE_NAME_LOCALIZATION2_ = "LOCALIZATION";
const string Planner::FEATURE_NAME_ENVIRONMENT_ = "Environment";
const string Planner::FEATURE_NAME_ENVIRONMENT2_ = "ENVIRONMENT";
const string Planner::FEATURE_NAME_ROBOT_ID_ = "ROBOT_ID";
const string Planner::FEATURE_NAME_MISSION_TIME_ = "MissionTime";
const string Planner::FEATURE_VALUE_USEMAPS_ = "UseMaps";
const string Planner::FEATURE_VALUE_USEMAPS2_ = "USEMAPS";
const string Planner::FEATURE_VALUE_INDOOR_ = "Indoor";
const string Planner::FEATURE_VALUE_INDOOR2_ = "INDOOR";
const string Planner::CONSTRAINT_NAME_VEHICLE_TYPES_ = "VEHICLE_TYPES";
const string Planner::CONSTRAINT_VALUE_UUV_ = "UUV";
const string Planner::CONSTRAINT_VALUE_UGV_ = "UGV";
const string Planner::CONSTRAINT_VALUE_USV_ = "USV";
const string Planner::CONSTRAINT_VALUE_UAV_ = "UAV";
const string Planner::MISSION_TIME_FORMAT_ = "%H:%M:%S+%m/%d/%Y";
const char Planner::POLYGON_ENCODER_KEY_XY_START_ = '<';
const char Planner::POLYGON_ENCODER_KEY_XY_END_ = '>';
const char Planner::POLYGON_ENCODER_KEY_LATLON_START_ = '[';
const char Planner::POLYGON_ENCODER_KEY_LATLON_END_ = ']';
const char Planner::POLYGON_ENCODER_KEY_POINT_SEPARATOR_ = ',';
const char Planner::POLYGON_ENCODER_KEY_POINTS_SEPARATOR_ = ';';
const char Planner::POLYGON_ENCODER_KEY_POLY_SEPARATOR_ = '&';
const double Planner::DEFAULT_MOVE_TO_LOCATION_GAIN_ = 1.0;
const double Planner::DEFAULT_WANDER_GAIN_ = 0.0;
const double Planner::DEFAULT_INDOOR_AVOID_OBSTACLE_GAIN_ = 0.3;
const double Planner::DEFAULT_OUTDOOR_AVOID_OBSTACLE_GAIN_ = 1.0;
const double Planner::DEFAULT_INDOOR_AVOID_OBSTACLE_SPHERE_ = 0.7;
const double Planner::DEFAULT_OUTDOOR_AVOID_OBSTACLE_SPHERE_ = 2.2;
const double Planner::DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_ = 0.3;
const double Planner::DEFAULT_GOAL_TOLERANCE_ = 0.5;
const long Planner::DEFAULT_TIME_WINDOW_RANGE_SEC_ = 86400;
const int Planner::MAX_CNP_PLANS_ = 3;
const int Planner::INVALID_ROBOT_ID_ = -1;
const bool Planner::FORCE_SUB_MISSION_READY_DEPLOY_FIRST_ = true;
const bool Planner::USE_STATIC_GOAL_TOLERANCE_ = false;
const bool Planner::ADAPT_GEOGRAPHIC_COORDINATE_ = false;
const bool Planner::ADAPT_TIME_WINDOW_ = true;

//-----------------------------------------------------------------------
// Constructor for Planner class.
//-----------------------------------------------------------------------
Planner::Planner(
    Planner **plannerInstance) : 
    self_(plannerInstance),
    cnpServerManager_(NULL),
    cnpMode_(CNP_MODE_DISABLED)
{
    if (plannerInstance != NULL)
    {
        *plannerInstance = this;
    }

    clearCurrentMissionPlans_();

    currentCNPStatus_.status = CNP_OUTPUT_NORMAL;
    currentCNPStatus_.errorMsg = EMPTY_STRING_;

    new RepairPlan(&repairPlan_);
}

//-----------------------------------------------------------------------
// Distructor for Planner class.
//-----------------------------------------------------------------------
Planner::~Planner(void)
{
    if (repairPlan_ != NULL)
    {
        delete repairPlan_;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
//  This function resets the current mission plan.
//-----------------------------------------------------------------------
void Planner::clearCurrentMissionPlans_(void)
{
    int i;

    for (i = 0; i < (int)(currentMissionPlans_.size()); i++)
    {
        if ((int)(currentMissionPlans_[i].plans.size()) > 0)
        {
            currentMissionPlans_[i].plans.clear();
        }

        if ((int)(currentMissionPlans_[i].robotIDs.size()) > 0)
        {
            currentMissionPlans_[i].robotIDs.clear();
        }
    }

    currentMissionPlans_.clear();
}

//-----------------------------------------------------------------------
//  This function returns the RobotIDs in the current mission plan sets.
//-----------------------------------------------------------------------
vector<CBRPlanner_RobotIDs_t> Planner::getCurrentRobotIDList(void)
{
    vector<CBRPlanner_RobotIDs_t> robotIDList;
    int i, j, numMissions;

    numMissions = currentMissionPlans_.size();

    robotIDList.resize(numMissions);

    for (i = 0; i < numMissions; i++)
    {
        for (j = 0; j < (int)(currentMissionPlans_[i].robotIDs.size()); j++)
        {
            robotIDList[i].robotIDs.push_back(currentMissionPlans_[i].robotIDs[j]);
        }
    }

    return robotIDList;
}

//-----------------------------------------------------------------------
//  This function is just a wrapper for getMissionPlansCBROnly().
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::getMissionPlans(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission)
{
    vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans;

    bigMissionPlans = getMissionPlansCBROnly(features, maxRating, addStartSubMission);

    return bigMissionPlans;
}

//-----------------------------------------------------------------------
//  This function creates a misssion plan.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::getMissionPlansPremissionCNP(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission,
    vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    vector<MissionMemoryManager::MissionPlan_t> missionPlans;
    vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans;
    vector<MissionMemoryManager::MissionPlan_t> curMissionPlans;
    vector<MemoryManager::Features_t> subFeaturesList;
    vector<MemoryManager::Features_t> subFeaturesListCopy;
    vector<MemoryManager::Constraint_t> constraints;
    vector<CBRPlanner_PlanSequence_t> plans; // Size of this vector indicates # of robots
    vector <string> zones;
    list<CNP_TaskInfo>::const_iterator iterTI;
    list<CNP_TaskConstraint>::const_iterator iterTC;
    list<CNP_RobotConstraint>::const_iterator iterRC;
    string maxVelValue, baseVelValue, aggressivenessValue, taskName, vehicleTypeValue;
    CNP_InputBundle cnpInputBundle;
    CNP_OutputBundle cnpOutputBundle;
    CNP_TaskConstraint taskConstraint;
    CNP_RobotConstraint robotConstraint;
	CNP_TaskInfo taskInfo;
    CNPRobotTaskMapping robotTaskMapping;
    CNPRobotTaskMapping::const_iterator iterTM;
    CBRPlanner_PlanSequence_t planSequence; // Single robot. Possibly multiple tasks.
    MemoryManager::Features_t filteredFeatures;
    MissionMemoryManager::MissionPlan_t missionPlan;
    MissionMemoryManager::Coordinates_t coordinates;
    MissionMemoryManager::GeoCoordinates_t geoCoordinates;
    char buf[1024];
    double maxVel, baseVel, aggressiveness;
    int i, j, numRobots, robotID, numTasks, taskID, curRobotID;
    int numMissionPlans;
    int trackingNumber = 0;
    bool useMaps, indoor;
    const bool DEBUG = true;
    
    clearCurrentMissionPlans_();

    // Make sure premission-CNP is enabled.
    setCNPMode(CNP_MODE_PREMISSION);

    // Next, get the base velocity from the global features through
    // "aggressiveness".
    aggressivenessValue = extractAggressivenessValue_(features);
    aggressiveness = atof(aggressivenessValue.c_str());
    baseVel = maxVel * aggressiveness;
    sprintf(buf, "%.2f", baseVel);
    baseVelValue = buf;

    // Decompose the features with respect to sub-missions.
    subFeaturesList = extractSubFeatures_(features);
    subFeaturesListCopy = subFeaturesList;

    // Check the number of tasks
    numTasks = subFeaturesList.size();
    if (numTasks != (int)(taskConstraintsList.size()))
    {
        gWindows->printfPlannerWindow(
            "Error: Incorrect number of tasks detected in getMissionPlansPremissionCNP().\n");
        gWindows->printfPlannerWindow(
            " - Number of Tasks: %d (CBR) vs %d (CNP)\n",
            numTasks,
            (int)(taskConstraintsList.size()));

        return bigMissionPlans;
    }

    // Bundle up the input for CNP. First, task names and constraints.
    for (i = 0; i < numTasks; i++)
    {
        // For CNP, assign the index as the ID (rather than actual Task ID).
	    taskInfo.id = i;
        taskInfo.name = taskConstraintsList[i].name;
		cnpInputBundle.ListOfCNPTasks.push_back(taskInfo);		

        taskConstraint.taskInfo = taskInfo;

        constraints = taskConstraintsList[i].constraints;

        for (j = 0; j < (int)(constraints.size()); j++)
        {
            taskConstraint.strConstraintName = constraints[j].name;

            switch (constraints[j].formatType) {

            case MemoryManager::FORMAT_STRING:
                taskConstraint.constraintType = CNP_STRING;
                break;

            case MemoryManager::FORMAT_INT:
                taskConstraint.constraintType = CNP_INT;
                break;

            case MemoryManager::FORMAT_DOUBLE:
                taskConstraint.constraintType = CNP_DOUBLE;
                break;

            case MemoryManager::FORMAT_BOOLEAN:
                taskConstraint.constraintType = CNP_BOOLEAN;
                break;

            case MemoryManager::FORMAT_NA:
                taskConstraint.constraintType = CNP_NA;
                break;
            }

            taskConstraint.strConstraintValue = constraints[j].value;

            cnpInputBundle.ListOfCNPTaskConstraints.push_back(taskConstraint);
        }
    }

    // Get the number of robots from the global features (and check the validity).
    numRobots = extractNumRobots_(features);

    if (numRobots < 0)
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::getMissionPlansPremissionCNP(). Number of robots could not be determined.\n");

        return bigMissionPlans;
    }
    else if (numRobots != (int)(robotConstraintsList.size()))
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::getMissionPlansPremissionCNP(). Incorrect number of robots detected.\n");
        gWindows->printfPlannerWindow(
            " - Number of Robots: %d (CBR) vs %d (CNP)\n",
            numRobots,
            (int)(robotConstraintsList.size()));

        return bigMissionPlans;
    }

    // Bundle up the robot constraints for CNP.
    for (i = 0; i < numRobots; i++)
    {
        robotConstraint.iRobotID = robotConstraintsList[i].id;

        constraints = robotConstraintsList[i].constraints;

        for (j = 0; j < (int)(constraints.size()); j++)
        {
            robotConstraint.strConstraintName = constraints[j].name;

            switch (constraints[j].formatType) {

            case MemoryManager::FORMAT_STRING:
                robotConstraint.constraintType = CNP_STRING;
                break;

            case MemoryManager::FORMAT_INT:
                robotConstraint.constraintType = CNP_INT;
                break;

            case MemoryManager::FORMAT_DOUBLE:
                robotConstraint.constraintType = CNP_DOUBLE;
                break;

            case MemoryManager::FORMAT_BOOLEAN:
                robotConstraint.constraintType = CNP_BOOLEAN;
                break;

            case MemoryManager::FORMAT_NA:
                robotConstraint.constraintType = CNP_NA;
                break;
            }

            robotConstraint.strConstraintValue = constraints[j].value;

            cnpInputBundle.ListOfCNPRobotConstraints.push_back(robotConstraint);
        }
    }

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug(
            "BUNDLED TASK INFO (Size %d):\n",
            cnpInputBundle.ListOfCNPTasks.size());

        for (iterTI = cnpInputBundle.ListOfCNPTasks.begin();
             iterTI != cnpInputBundle.ListOfCNPTasks.end();
             iterTI++)
        {
            gDebugger->printfDebug(
                "%d %s\n",
                iterTI->id,
                iterTI->name.c_str());
        }

        gDebugger->printfLine();
        gDebugger->printfDebug(
            "BUNDLED TASK CONSTRAINTS (Size %d):\n",
            cnpInputBundle.ListOfCNPTaskConstraints.size());

        for (iterTC = cnpInputBundle.ListOfCNPTaskConstraints.begin();
             iterTC != cnpInputBundle.ListOfCNPTaskConstraints.end();
             iterTC++)
        {
            gDebugger->printfDebug(
                "%d %s %s %s\n",
                iterTC->taskInfo.id,
                iterTC->taskInfo.name.c_str(),
                iterTC->strConstraintName.c_str(),
                iterTC->strConstraintValue.c_str());
        }

        gDebugger->printfLine();
        gDebugger->printfDebug(
            "BUNDLED ROBOT CONSTRAINTS (Size %d):\n",
            cnpInputBundle.ListOfCNPRobotConstraints.size());

        for (iterRC = cnpInputBundle.ListOfCNPRobotConstraints.begin();
             iterRC != cnpInputBundle.ListOfCNPRobotConstraints.end();
             iterRC++)
        {

            gDebugger->printfDebug(
                "%d %s %s\n",
                iterRC->iRobotID,
                iterRC->strConstraintName.c_str(),
                iterRC->strConstraintValue.c_str());
        }

        gDebugger->printfLine();
    }

    // Get the task-robot mapping from CNP.
    cnpOutputBundle = cnpServerManager_->PremissionCNPBidding(cnpInputBundle);

    // Copy the data.
    robotTaskMapping = cnpOutputBundle.robotTaskMapping;
    currentCNPStatus_.status = cnpOutputBundle.status;
    currentCNPStatus_.errorMsg = cnpOutputBundle.errorMsg;

    // Based on the mapping, retrieve the missions.
    curRobotID = -1;
    numRobots = 0;

    for (iterTM = robotTaskMapping.begin();
         iterTM != robotTaskMapping.end();
         iterTM++)
    {
        // Get the robot ID
        robotID = iterTM->first;
        
        // Get the task ID
        taskID = iterTM->second;

        // Get the coordinates and other info.
        coordinates = extractCoordinates_(subFeaturesList[taskID]);
        geoCoordinates = extractGeoCoordinates_(subFeaturesList[taskID]);
        taskName = extractTaskName_(subFeaturesList[taskID]);
        useMaps = extractUseMaps_(subFeaturesList[taskID]);
        indoor = extractIndoor_(subFeaturesList[taskID]);
        
        vehicleTypeValue = extractRobotVehicleType_(
            robotID,
            robotConstraintsList);

        if (vehicleTypeValue == CONSTRAINT_VALUE_UUV_)
        {
            maxVel = CNP_MAXVEL_UUV;
        }
        else if (vehicleTypeValue == CONSTRAINT_VALUE_UGV_)
        {
            maxVel = CNP_MAXVEL_UGV;
        }
        else if (vehicleTypeValue == CONSTRAINT_VALUE_USV_)
        {
            maxVel = CNP_MAXVEL_USV;
        }
        else if (vehicleTypeValue == CONSTRAINT_VALUE_UAV_)
        {
            maxVel = CNP_MAXVEL_UAV;
        }
        else
        {
            maxVel = CNP_MAXVEL_USV;
        }

        sprintf(buf, "%.2f", maxVel);
        maxVelValue = buf;
        baseVel = maxVel * aggressiveness;
        sprintf(buf, "%.2f", baseVel);
        baseVelValue = buf;

        // Remove all non-index features.
        filteredFeatures = filterNonIndexFeatures_(subFeaturesList[taskID]);

        // Get the mission plan for these "subfeatures".
        missionPlans = gCBRPlanner->missionMemoryManager()->getSavedMissionPlans(
            filteredFeatures,
            maxRating);

        if (ADAPT_GEOGRAPHIC_COORDINATE_)
        {
            missionPlans = updateTargetLatLon_(missionPlans, geoCoordinates);
        }

        // If there is more than one plan for this subfeature (i.e., sub-
        // mission), we face a serious computational problem as the number
        // of the robot increases. For now, we just take the first plan
        // that was returned from the memory manager per subfeature.
        numMissionPlans = missionPlans.size();

        for (i = 0; i < numMissionPlans; i++)
        {
            // Adjust the number of robots.
            missionPlans[i] = adjustNumRobots_(missionPlans[i], 1);

            // Put the "StartSubMisson" state in the beginning.
            if (addStartSubMission)
            {
                missionPlans[i] = addStartSubMissionState_(
                    missionPlans[i],
                    taskName,
                    coordinates,
                    useMaps,
                    indoor,
                    maxVel,
                    DEPLOY_BY_GOTO);
            }
        }

        if (curRobotID != robotID)
        {
            // It's a new robot.
            numRobots++;
            curRobotID = robotID;

            // Copy the mission.
            curMissionPlans = missionPlans;

            if (numRobots == 1)
            {
                // It's the first robot. Assign the basic parameters for the mission.
                bigMissionPlans = curMissionPlans;
            }

            while ((int)(bigMissionPlans.size()) < numMissionPlans)
            {
                // Douplicate the first mission if necessary.
                bigMissionPlans.push_back(bigMissionPlans[0]);
            }

            // Resize the list of all the missions.
            for (i = 0; i < numMissionPlans; i++)
            {
                bigMissionPlans[i].plans.resize(numRobots);
                bigMissionPlans[i].maxVelValues.resize(numRobots);
                bigMissionPlans[i].baseVelValues.resize(numRobots);
                bigMissionPlans[i].robotIDs.resize(numRobots);
            }
        }
        else
        {
            // It's the same robot as the previous iteration.
            // Append the plans to create a sequence of sub-missions.
            curMissionPlans = appendMissionPlans_(
                curMissionPlans,
                missionPlans,
                trackingNumber++);
        }

        for (i = 0; i < numMissionPlans; i++)
        {
            bigMissionPlans[i].plans[numRobots-1] = curMissionPlans[i].plans[0];
            bigMissionPlans[i].maxVelValues[numRobots-1] = maxVelValue;
            bigMissionPlans[i].baseVelValues[numRobots-1] = baseVelValue;
            bigMissionPlans[i].robotIDs[numRobots-1] = curRobotID;
        }
    }

    if ((int)(bigMissionPlans.size()) > MAX_CNP_PLANS_)
    {
        bigMissionPlans.resize(MAX_CNP_PLANS_);
    }

    // Filter cases based on "No Fly Zones" etc. before proceeding further.
    zones.push_back(FEATURE_NAME_NO_FLY_ZONE_);
    zones.push_back(FEATURE_NAME_NO_COMM_ZONE_);
    zones.push_back(FEATURE_NAME_WEATHER_ZONE_);
    filterBySpecialZones_(&bigMissionPlans, features, zones);

    // Straighten up some parameters.
    for (i = 0; i < (int)(bigMissionPlans.size()); i++)
    {
        bigMissionPlans[i] = addStartState_(bigMissionPlans[i]);

        if (bigMissionPlans[i].type == CBRPLANNER_MISSIONPLAN_EMPTY)
        {
            bigMissionPlans[i].archType = STRING_EMPTY_ARCHITECTURE_;
        }
        else
        {
            bigMissionPlans[i].type = CBRPLANNER_MISSIONPLAN_CONSTRUCTED;
        }
    }

    currentMissionPlans_ = bigMissionPlans;

    return bigMissionPlans;
}

//-----------------------------------------------------------------------
//  This function creates a misssion plan for runtime-CNP.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::getMissionPlansRuntimeCNP(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    vector<MissionMemoryManager::MissionPlan_t> missionPlans;
    vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans;
    string maxVelValue, baseVelValue, aggressivenessValue, taskName, vehicleTypeValue;
    MissionMemoryManager::MissionPlan_t missionPlanKeep;
    MissionMemoryManager::MissionPlan_t missionPlan;
    MissionMemoryManager::Coordinates_t coordinates;
    MissionMemoryManager::GeoCoordinates_t geoCoordinates;
    vector<MemoryManager::Features_t> subFeaturesList;
    vector<MemoryManager::Features_t> subFeaturesListCopy;
    vector <string> zones;
    char buf[1024];
    double maxVel, baseVel, aggressiveness;
    bool useMaps, indoor;
    int i, numRobots;
    int robotID;

    clearCurrentMissionPlans_();

    // Make sure runtime-CNP is enabled.
    setCNPMode(CNP_MODE_RUNTIME);

    // Next, get the base velocity from the global features through
    // "aggressiveness".
    aggressivenessValue = extractAggressivenessValue_(features);
    aggressiveness = atof(aggressivenessValue.c_str());

    // Get the number of robots from the global features.
    numRobots = extractNumRobots_(features);

    if (numRobots < 0)
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::getMissionPlansRuntimeCNP(). Number of robots could not be determined.\n");

        return bigMissionPlans;
    }
    else if (numRobots > (int)(robotConstraintsList.size()))
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::getMissionPlansRuntimeCNP(). Incorrect number of robots detected.\n");
        gWindows->printfPlannerWindow(
            " - Number of Robots: %d (Specified) vs %d (CNP Robot Constraints)\n",
            numRobots,
            (int)(robotConstraintsList.size()));

        return bigMissionPlans;
    }

    // Decompose the features with respect to sub-missions.
    subFeaturesList = extractSubFeatures_(features);
    subFeaturesListCopy = subFeaturesList;

    // The number of the robot should be same as the number of sub-missions.
    // One sub-mission per robot is assigned.
    if ((numRobots > 1) && (numRobots == (int)(subFeaturesList.size())))
    {
        for (i = 0; i < (int)(subFeaturesList.size()); i ++)
        {
            // Get the coordinates and other info.
            coordinates = extractCoordinates_(subFeaturesList[i]);
            geoCoordinates = extractGeoCoordinates_(subFeaturesList[i]);
            taskName = extractTaskName_(subFeaturesList[i]);
            useMaps = extractUseMaps_(subFeaturesList[i]);
            indoor = extractIndoor_(subFeaturesList[i]);
            robotID = extractRobotID_(subFeaturesList[i]);

            vehicleTypeValue = extractRobotVehicleType_(
                robotID,
                robotConstraintsList);

            if (vehicleTypeValue == CONSTRAINT_VALUE_UUV_)
            {
                maxVel = CNP_MAXVEL_UUV;
            }
            else if (vehicleTypeValue == CONSTRAINT_VALUE_UGV_)
            {
                maxVel = CNP_MAXVEL_UGV;
            }
            else if (vehicleTypeValue == CONSTRAINT_VALUE_USV_)
            {
                maxVel = CNP_MAXVEL_USV;
            }
            else if (vehicleTypeValue == CONSTRAINT_VALUE_UAV_)
            {
                maxVel = CNP_MAXVEL_UAV;
            }
            else
            {
                maxVel = CNP_MAXVEL_USV;
            }

            sprintf(buf, "%.2f", maxVel);
            maxVelValue = buf;
            baseVel = maxVel * aggressiveness;
            sprintf(buf, "%.2f", baseVel);
            baseVelValue = buf;

            // Remove all non-index features.
            subFeaturesList[i] = filterNonIndexFeatures_(subFeaturesList[i]);

            // Get the mission plan for these "subfeatures".
            missionPlans = gCBRPlanner->missionMemoryManager()->getSavedMissionPlans(
                subFeaturesList[i],
                maxRating);

            if (ADAPT_GEOGRAPHIC_COORDINATE_)
            {
                missionPlans = updateTargetLatLon_(missionPlans, geoCoordinates);
            }

            // If there is more than one plan for this subfeature (i.e., sub-
            // mission), we face a serious computational problem as the number
            // of the robot increases. For now, we just take the first plan
            // that was returned from the memory manager per subfeature.
            missionPlan = missionPlans[0];

            // Adjust the number of robots.
            missionPlan = adjustNumRobots_(missionPlan, 1);

            // Put the "StartSubMisson" state in the beginning.
            if (addStartSubMission)
            {
                missionPlan = addStartSubMissionState_(
                    missionPlan,
                    taskName,
                    coordinates,
                    useMaps,
                    indoor,
                    maxVel,
                    DEPLOY_BY_LOCALIZE);
            }

            if (i == 0)
            {
                missionPlanKeep = missionPlan;
                missionPlanKeep.plans.resize(numRobots);
                missionPlanKeep.maxVelValues.resize(numRobots);
                missionPlanKeep.baseVelValues.resize(numRobots);
                missionPlanKeep.robotIDs.resize(numRobots);
            }
            else
            {
                missionPlanKeep.plans[i] = missionPlan.plans[0];
            }

            missionPlanKeep.maxVelValues[i] = maxVelValue;
            missionPlanKeep.baseVelValues[i] = baseVelValue;
            missionPlanKeep.robotIDs[i] = robotID;
        }

        bigMissionPlans.push_back(missionPlanKeep);
    }
    else
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::getMissionPlansRuntimeCNP().\n");
        gWindows->printfPlannerWindow(
            " Inadequate number of robots.\n");
        return bigMissionPlans;
    }

    // Filter cases based on "No Fly Zones" etc. before proceeding further.
    zones.push_back(FEATURE_NAME_NO_FLY_ZONE_);
    zones.push_back(FEATURE_NAME_NO_COMM_ZONE_);
    zones.push_back(FEATURE_NAME_WEATHER_ZONE_);
    filterBySpecialZones_(&bigMissionPlans, features, zones);

    // Straighten up some parameters.
    for (i = 0; i < (int)(bigMissionPlans.size()); i++)
    {
        bigMissionPlans[i] = addStartState_(bigMissionPlans[i]);

        if (bigMissionPlans[i].type == CBRPLANNER_MISSIONPLAN_EMPTY)
        {
            bigMissionPlans[i].archType = STRING_EMPTY_ARCHITECTURE_;
        }
        else
        {
            bigMissionPlans[i].type = CBRPLANNER_MISSIONPLAN_CONSTRUCTED;
        }
    }

    bigMissionPlans = sortMissionPlans_(bigMissionPlans);

    currentMissionPlans_ = bigMissionPlans;

    return bigMissionPlans;
}

//-----------------------------------------------------------------------
// This method parses the string and extracts all the polygons encoded 
// in the given string. Code based on Endo's MlabMissionDesign::decodePolygon_
// method. For geo locations, x=longitude, y=latitude.   NADEEM - 01/28/07
//-----------------------------------------------------------------------
bool Planner::extractAllPolygons_(
    string polygonString,
    MissionMemoryManager::PolygonSet_t* polySet,
    MissionMemoryManager::PolygonSet_t* geoPolySet)
{
    MissionMemoryManager::Coordinates_t pos, geo;
    string bufString;
    string::size_type index1,index2;
    double x, y, lat, lon;
    MissionMemoryManager::PointList_t points, geoPoints; 
    string superString;

    if (polygonString.empty())
        return false;

    superString = polygonString;

    while(!superString.empty()) {
    
        // Find "<"
        index1 = superString.find(POLYGON_ENCODER_KEY_XY_START_);
    
        // Find "&"
        index2 = superString.find(POLYGON_ENCODER_KEY_POLY_SEPARATOR_);
    
        if (index1 == string::npos)
            return false;
    
        if (index2 == string::npos)
        {
            polygonString = superString.substr(index1, string::npos);	
            superString.clear();
        }
        else
        {
            // Get the string for the first poygon
            polygonString = superString.substr(index1, index2-index1);
	
            // Get the remaining string for rest of the polygons
            superString = superString.substr(index2+1, string::npos);
        }
    
        while (true)
        {
	
            // Find "<"
            index1 = polygonString.find(POLYGON_ENCODER_KEY_XY_START_);
    
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_XY_START_ not found.\n");
                return false;
            }
    
            polygonString = polygonString.substr(index1+1, string::npos);

            // Find ","
            index1 = polygonString.find(POLYGON_ENCODER_KEY_POINT_SEPARATOR_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_POINT_SEPARATOR_ not found.\n");
	    
                return false;
            }
	
            // Get x
            bufString = polygonString.substr(0, index1);
            x = atof(bufString.c_str());
	
            polygonString = polygonString.substr(index1+1, string::npos);
	
            // Find ">"
            index1 = polygonString.find(POLYGON_ENCODER_KEY_XY_END_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). (POLYGON_ENCODER_KEY_XY_END_ not found.\n");
                return false;
            }
	
            // Get y
            bufString = polygonString.substr(0, index1);
            y = atof(bufString.c_str());
	
            polygonString = polygonString.substr(index1+1, string::npos);
	
            // Find "["
            index1 = polygonString.find(POLYGON_ENCODER_KEY_LATLON_START_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_LATLON_START_ not found.\n");
                return false;
            }
	
            polygonString = polygonString.substr(index1+1, string::npos);
	
            // Find ","
            index1 = polygonString.find(POLYGON_ENCODER_KEY_POINT_SEPARATOR_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_POINT_SEPARATOR_ not found.\n");
                return false;
            }
	
            // Get lat
            bufString = polygonString.substr(0, index1);
            lat = atof(bufString.c_str());
	
            polygonString = polygonString.substr(index1+1, string::npos);
	
            // Find "]"
            index1 = polygonString.find(POLYGON_ENCODER_KEY_LATLON_END_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_LATLON_END_ not found.\n");
                return false;
            }
	
            // Get lon
            bufString = polygonString.substr(0, index1);
            lon = atof(bufString.c_str());
      
            polygonString = polygonString.substr(index1+1, string::npos);
      
            // Find ";"
            index1 = polygonString.find(POLYGON_ENCODER_KEY_POINTS_SEPARATOR_);
	
            if (index1 == string::npos)
            {
                fprintf(stderr, "Error: Planner::extractAllPolygons(). POLYGON_ENCODER_KEY_POINTS_SEPARATOR_ not found.\n");
                return false;
            }
	
            // Save the point.
            pos.x = x;
            pos.y = y;
            points.push_back(pos);
	
	
            geo.x = lon;
            geo.y = lat;
            geoPoints.push_back(geo);
	
	
            if (polygonString.size() <= 1)
            {
                // Done.
                break;
            }
	
            polygonString = polygonString.substr(index1+1, string::npos);
        }
        polySet->push_back(points);
        geoPolySet->push_back(geoPoints);
        points.clear();
    }
  
    return true;
}

//-----------------------------------------------------------------------
// This method will extract all the Longitude,Latitude pairs from 
// Missionplan_t structure into a vector of points. 
// x=longitude and y=latitude.  NADEEM - 01/28/07
//-----------------------------------------------------------------------
void Planner::extractAllTargetGeoLocations_(
    MissionMemoryManager::MissionPlan_t missionPlan, 
    MissionMemoryManager::PointList_t* geoPoints)
{
    bool lat_found, lon_found;
    MissionMemoryManager::Coordinates_t pos;

    geoPoints->clear();
    string val_str;

    // for each robot
    for (int i=0; i < (int)(missionPlan.plans.size()); i++)
    { 
        //for each task
        for (int j=0; j < (int)(missionPlan.plans[i].size()); j++)
        {
            lat_found = lon_found = false;
            //Go through all parameters
            for (int k=0; k < (int)(missionPlan.plans[i][j].parameters.size()); k++)
            {	      
                if (missionPlan.plans[i][j].parameters[k].name == STRING_TARGET_LONGITUDE_) {
                    val_str = missionPlan.plans[i][j].parameters[k].value;
                    val_str = val_str.substr(1, val_str.size()-2);
                    pos.x = atof(val_str.c_str());
                    lon_found = true;
                }

                if (missionPlan.plans[i][j].parameters[k].name == STRING_TARGET_LATITUDE_) {
                    val_str = missionPlan.plans[i][j].parameters[k].value;
                    val_str = val_str.substr(1, val_str.size()-2);
                    pos.y = atof(val_str.c_str());
                    lat_found = true;
                }
	      
                if (lat_found && lon_found) {
                    geoPoints->push_back(pos);
                    break;
                }
            }
        }
    }
  
}

//-----------------------------------------------------------------------
// This method takes in a point and a polygon and returns true if the
// point lies inside that polygon. NADEEM - 01/28/07
//-----------------------------------------------------------------------
bool Planner::pointInPolygon_(
    MissionMemoryManager::Coordinates_t point,
    MissionMemoryManager::PointList_t polygon)
{
    int i, j;
    bool res = false;

    for (i = 0, j = (int)(polygon.size())-1; i < (int)(polygon.size()); j = i++)
    {
        if ((((polygon[i].y <= point.y) && (point.y < polygon[j].y)) ||
             ((polygon[j].y <= point.y) && (point.y < polygon[i].y))) &&
            (point.x < (polygon[j].x - polygon[i].x) * 
             (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x))
        {
            res = !res;
        }
    }

    return res;
}

//-----------------------------------------------------------------------
// This function filters the set of returned missions based on "No-Fly
// Zones. NADEEM - 01/26/07
//-----------------------------------------------------------------------
void Planner::filterBySpecialZones_(
    vector<MissionMemoryManager::MissionPlan_t>* bigMissionPlans,
    MemoryManager::Features_t features,
    vector<string> zones)
{

    // Extract all the polygons from the feature list. Then for each mission 
    // search if any of the mission objectives is positioned there.

    int i,j, k, numCases, numErased;
    bool matched;
    MissionMemoryManager::PolygonSet_t polySet, geoPolySet;
    MissionMemoryManager::PointList_t geoPoints;
    vector<MissionMemoryManager::MissionPlan_t> missionPlans;

    // First grab all the polygons from the features list
    // We will get both the XY and as well as geographic coordinates
    for (i = 0; i < (int)(features.size()); i++)
    {
        for (j=0; j < (int)(zones.size()); j++) 
        {
            if (features[i].name == zones[j])
            {
                // feature matched. Now extract all the polygon points
                // For Geo Positions: x=longitude, y=latitude
                extractAllPolygons_(features[i].value, &polySet, &geoPolySet);
                break;
            }
        }
    }

    // Now go through each plan and extract all the target locations
    // Check whether any of these target locations fall into any 
    // of the Polygons we extracted above

    numCases = (int)(bigMissionPlans->size());
    numErased = 0;
    missionPlans = *bigMissionPlans;

    // for each plan
    for (i=0; i < numCases; i++)
    {
        extractAllTargetGeoLocations_(missionPlans[i], &geoPoints);

        matched = false;
        // Now test the points against the polygons
        //for each point
        for (j=0; j < (int)(geoPoints.size()); j++) {
            //for each polygon
            for (k=0; k < (int) (geoPolySet.size()); k++) {
                if (pointInPolygon_(geoPoints[j], geoPolySet[k]))
                {
                    vector<MissionMemoryManager::MissionPlan_t>::iterator it 
                        = bigMissionPlans->begin();
		
                    bigMissionPlans->erase(it+i-numErased);
                    numErased++;
                    matched = true;
                    break;
                }
            }
            if (matched) 
                break;
        }

    }
}

//-----------------------------------------------------------------------
//  This function creates a misssion plan.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::getMissionPlansCBROnly(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission)
{
    vector<MissionMemoryManager::MissionPlan_t> missionPlans;
    vector<MissionMemoryManager::MissionPlan_t> missionPlansTypeI;
    vector<MissionMemoryManager::MissionPlan_t> missionPlansTypeII;
    vector<MissionMemoryManager::MissionPlan_t> missionPlansTypeIII;
    vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans;
    vector <string> zones;
    string maxVelValue, baseVelValue, aggressivenessValue, taskName;
    string missionTimeString;
    MissionMemoryManager::MissionPlan_t missionPlanKeep;
    MissionMemoryManager::MissionPlan_t missionPlan;
    MissionMemoryManager::Coordinates_t coordinates;
    MissionMemoryManager::GeoCoordinates_t geoCoordinates;
    vector<MemoryManager::Features_t> subFeaturesList;
    vector<MemoryManager::Features_t> subFeaturesListCopy;
    char buf[1024];
    double maxVel, baseVel, aggressiveness;
    int i, j, k, n, numRobots;
    int trackingNumber = 0;
    bool useMaps, indoor, numRobotsNotSpecified = false;
    const int POSSIBLE_MAX_ROBOTS_WHEN_NOT_SPECIFIED = 100;

    clearCurrentMissionPlans_();

    // First, get the max velocity from the global features.
    maxVelValue = extractMaxVelocityValue_(features);
    maxVel = atof(maxVelValue.c_str());

    // Next, get the base velocity from the global features through
    // "aggressiveness".
    aggressivenessValue = extractAggressivenessValue_(features);
    aggressiveness = atof(aggressivenessValue.c_str());
    baseVel = maxVel * aggressiveness;
    sprintf(buf, "%.2f", baseVel);
    baseVelValue = buf;

    // Get the number of robots from the global features.
    numRobots = extractNumRobots_(features);
    numRobotsNotSpecified = (numRobots > 0)? false : true;

    // Get the mission time.
    missionTimeString = extractMissionTime_(features);

    // Decompose the features with respect to sub-missions.
    subFeaturesList = extractSubFeatures_(features);
    subFeaturesListCopy = subFeaturesList;

    if (!numRobotsNotSpecified)
    {
        // Type I Mission: Since the number of the robot is same as the number
        // of sub-missions, one sub-mission per robot is assigned.
        if ((numRobots > 1) && (numRobots == (int)(subFeaturesList.size())))
        {
            for (i = 0; i < (int)(subFeaturesList.size()); i ++)
            {
                // Get the coordinates and other info.
                coordinates = extractCoordinates_(subFeaturesList[i]);
                geoCoordinates = extractGeoCoordinates_(subFeaturesList[i]);
                taskName = extractTaskName_(subFeaturesList[i]);
                useMaps = extractUseMaps_(subFeaturesList[i]);
                indoor = extractIndoor_(subFeaturesList[i]);

                // Remove all non-index features.
                subFeaturesList[i] = filterNonIndexFeatures_(subFeaturesList[i]);

                // Get the mission plan for these "subfeatures".
                missionPlans = gCBRPlanner->missionMemoryManager()->getSavedMissionPlans(
                    subFeaturesList[i],
                    maxRating);

                if (ADAPT_GEOGRAPHIC_COORDINATE_)
                {
                    missionPlans = updateTargetLatLon_(missionPlans, geoCoordinates);
                }

                if ((ADAPT_TIME_WINDOW_) &&
                    (missionTimeString != EMPTY_STRING_) &&
                    (missionTimeString != STRING_NA_))
                {
                    missionPlans = updateTimeWindow_(missionPlans, missionTimeString);
                }

                // If there is more than one plan for this subfeature (i.e., sub-
                // mission), we face a serious computational problem as the number
                // of the robot increases. For now, we just take the first plan
                // that was returned from the memory manager per subfeature.
                missionPlan = missionPlans[0];

                // Adjust the number of robots.
                missionPlan = adjustNumRobots_(missionPlan, 1);

                // Put the "StartSubMisson" state in the beginning.
                if (addStartSubMission)
                {
                    missionPlan = addStartSubMissionState_(
                        missionPlan,
                        taskName,
                        coordinates,
                        useMaps,
                        indoor,
                        maxVel,
                        DEPLOY_BY_GOTO);
                }

                if (i == 0)
                {
                    missionPlanKeep = missionPlan;
                    missionPlanKeep.plans.resize(numRobots);
                    missionPlanKeep.maxVelValues.resize(numRobots);
                    missionPlanKeep.baseVelValues.resize(numRobots);
                    missionPlanKeep.robotIDs.resize(numRobots);
                }
                else
                {
                    missionPlanKeep.plans[i] = missionPlan.plans[0];
                }

                missionPlanKeep.maxVelValues[i] = maxVelValue;
                missionPlanKeep.baseVelValues[i] = baseVelValue;
                missionPlanKeep.robotIDs[i] = i+1;
            }

            missionPlansTypeI.push_back(missionPlanKeep);
        }

        // Type II Mission: Every robot has all sub-missions
        subFeaturesList = subFeaturesListCopy;
        for (i = 0; i < (int)(subFeaturesList.size()); i++)
        {
            // Get the coordinates and other info.
            coordinates = extractCoordinates_(subFeaturesList[i]);
            geoCoordinates = extractGeoCoordinates_(subFeaturesList[i]);
            taskName = extractTaskName_(subFeaturesList[i]);
            useMaps = extractUseMaps_(subFeaturesList[i]);
            indoor = extractIndoor_(subFeaturesList[i]);

            // Remove all non-index features.
            subFeaturesList[i] = filterNonIndexFeatures_(subFeaturesList[i]);

            // Get the mission plan for these "subfeatures".
            missionPlans = gCBRPlanner->missionMemoryManager()->getSavedMissionPlans(
                subFeaturesList[i],
                maxRating);

            if (ADAPT_GEOGRAPHIC_COORDINATE_)
            {
                missionPlans = updateTargetLatLon_(missionPlans, geoCoordinates);
            }

            if ((ADAPT_TIME_WINDOW_) &&
                (missionTimeString != EMPTY_STRING_) &&
                (missionTimeString != STRING_NA_))
            {
                missionPlans = updateTimeWindow_(missionPlans, missionTimeString);
            }

            for (j = 0; j < ((int)(missionPlans.size())); j++)
            {
                missionPlans[j] = adjustNumRobots_(missionPlans[j], numRobots);

                // Put the "StartSubMisson" state in the beginning.
                if (addStartSubMission)
                {
                    missionPlans[j] = addStartSubMissionState_(
                        missionPlans[j],
                        taskName,
                        coordinates,
                        useMaps,
                        indoor,
                        maxVel,
                        DEPLOY_BY_GOTO);
                }

                missionPlans[j].maxVelValues.resize(numRobots);
                missionPlans[j].baseVelValues.resize(numRobots);

                for (k = 0; k < numRobots; k++)
                {
                    missionPlans[j].maxVelValues[k] = maxVelValue;
                    missionPlans[j].baseVelValues[k] = baseVelValue;
                }
            }

            // Append the plans to create a sequence of sub-missions.
            missionPlansTypeII = appendMissionPlans_(
                missionPlansTypeII,
                missionPlans,
                trackingNumber++);
        }

        // Merge the two types of missions.
        bigMissionPlans = mergeMissionPlans_(missionPlansTypeI, missionPlansTypeII);
    }
    else
    {
        // Type III: Number of robots not specified. Incrementally check the possible
        // number of robots.
        for (n = 0; n < POSSIBLE_MAX_ROBOTS_WHEN_NOT_SPECIFIED; n++)
        {
            subFeaturesList = subFeaturesListCopy;
            missionPlansTypeIII.clear();

            for (i = 0; i < (int)(subFeaturesList.size()); i++)
            {
                // Get the coordinates and other info.
                coordinates = extractCoordinates_(subFeaturesList[i]);
                geoCoordinates = extractGeoCoordinates_(subFeaturesList[i]);
                taskName = extractTaskName_(subFeaturesList[i]);
                useMaps = extractUseMaps_(subFeaturesList[i]);
                indoor = extractIndoor_(subFeaturesList[i]);

                // Remove all non-index features.
                subFeaturesList[i] = filterNonIndexFeatures_(subFeaturesList[i]);

                // Set the number of robots.
                subFeaturesList[i] = setNumRobots_(subFeaturesList[i], n);

                // Get the mission plan for these "subfeatures".
                missionPlans = gCBRPlanner->missionMemoryManager()->getSavedMissionPlans(
                    subFeaturesList[i],
                    maxRating,
                    true); // Disable backtrack.

                if (missionPlans.size() == 0)
                {
                    continue;
                }

                if (ADAPT_GEOGRAPHIC_COORDINATE_)
                {
                    missionPlans = updateTargetLatLon_(missionPlans, geoCoordinates);
                }

                if ((ADAPT_TIME_WINDOW_) &&
                    (missionTimeString != EMPTY_STRING_) &&
                    (missionTimeString != STRING_NA_))
                {
                    missionPlans = updateTimeWindow_(missionPlans, missionTimeString);
                }

                for (j = 0; j < ((int)(missionPlans.size())); j++)
                {
                    // Put the "StartSubMisson" state in the beginning.
                    if (addStartSubMission)
                    {
                        missionPlans[j] = addStartSubMissionState_(
                            missionPlans[j],
                            taskName,
                            coordinates,
                            useMaps,
                            indoor,
                            maxVel,
                            DEPLOY_BY_GOTO);
                    }

                    missionPlans[j].maxVelValues.resize(n);
                    missionPlans[j].baseVelValues.resize(n);

                    for (k = 0; k < n; k++)
                    {
                        missionPlans[j].maxVelValues[k] = maxVelValue;
                        missionPlans[j].baseVelValues[k] = baseVelValue;
                    }
                }

                // Append the plans to create a sequence of sub-missions.
                missionPlansTypeIII = appendMissionPlans_(
                    missionPlansTypeIII,
                    missionPlans,
                    trackingNumber++);
            }

            bigMissionPlans = mergeMissionPlans_(bigMissionPlans, missionPlansTypeIII);
        }
    }

    // Filter cases based on "No Fly Zones" etc. before proceeding further.
    zones.push_back(FEATURE_NAME_NO_FLY_ZONE_);
    zones.push_back(FEATURE_NAME_NO_COMM_ZONE_);
    zones.push_back(FEATURE_NAME_WEATHER_ZONE_);
    filterBySpecialZones_(&bigMissionPlans, features, zones);

    // Straighten up some parameters.
    for (i = 0; i < (int)(bigMissionPlans.size()); i++)
    {
        bigMissionPlans[i] = addStartState_(bigMissionPlans[i]);

        if (bigMissionPlans[i].type == CBRPLANNER_MISSIONPLAN_EMPTY)
        {
            bigMissionPlans[i].archType = STRING_EMPTY_ARCHITECTURE_;
        }
        else
        {
            bigMissionPlans[i].type = CBRPLANNER_MISSIONPLAN_CONSTRUCTED;
        }
    }
    
    //Now sort the set of remaining cases
    bigMissionPlans = sortMissionPlans_(bigMissionPlans);

    currentMissionPlans_ = bigMissionPlans;

    return bigMissionPlans;
}

//-----------------------------------------------------------------------
// This function sorts the mission plans based on their ratings.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::sortMissionPlans_(
    vector<MissionMemoryManager::MissionPlan_t> missionPlans)
{
    vector<MissionMemoryManager::MissionPlan_t> newMissionPlans, tmpMissionPlans;
    int i, heighestRating, heighestMissionIndex;

    while ((int)(missionPlans.size()) > 0)
    {
        heighestRating = -1;
        heighestMissionIndex = -1;

        for (i = 0; i < (int)(missionPlans.size()); i++)
        {
            if (missionPlans[i].rating > heighestRating)
            {
                heighestMissionIndex = i;
                heighestRating = missionPlans[i].rating;
            }
        }

        if (heighestMissionIndex != -1)
        {
            tmpMissionPlans.clear();

            for (i = 0; i < (int)(missionPlans.size()); i++)
            {
                if (i == heighestMissionIndex)
                {
                    newMissionPlans.push_back(missionPlans[i]);
                }
                else
                {
                    tmpMissionPlans.push_back(missionPlans[i]);
                }
            }

            missionPlans = tmpMissionPlans;
        }
        else
        {
            break;
        }
    }

    return newMissionPlans;
}

//-----------------------------------------------------------------------
// This function adds one mission to another to create a seqeubce of
// sub-missions.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::appendMissionPlans_(
    vector<MissionMemoryManager::MissionPlan_t> bigMissionPlans,
    vector<MissionMemoryManager::MissionPlan_t> missionPlans,
    int trackingNumber)
{
    MissionMemoryManager::MissionPlan_t newBigMissionPlan;
    vector<MissionMemoryManager::MissionPlan_t> newBigMissionPlans;
    CBRPlanner_Precondition_t precondition;
    CBRPlanner_Effect_t effect;
    char bufIndex[1024];
    int i, j, k, l, m, numRobots, lastIndex;

    if ((int)(bigMissionPlans.size()) == 0)
    {
        return missionPlans;
    }

    if ((int)(missionPlans.size()) == 0)
    {
        return bigMissionPlans;
    }

    // bufIndex will be added to any index that would possibly conflict
    // with any existing index.
    sprintf(bufIndex, "%d", trackingNumber);

    for (i = 0; i < (int)(bigMissionPlans.size()); i++)
    {
        for (j = 0; j < (int)(missionPlans.size()); j++)
        {
            newBigMissionPlan = bigMissionPlans[i];

            // Take the average of two ratings.
            newBigMissionPlan.rating = (newBigMissionPlan.rating + missionPlans[j].rating)/2;

            // Append the list of the data index as well.
            newBigMissionPlan.dataIndexList = mergeDataIndexLists_(
                newBigMissionPlan.dataIndexList,
                missionPlans[j].dataIndexList);

            numRobots = newBigMissionPlan.plans.size();

            if (numRobots != (int)(missionPlans[j].plans.size()))
            {
                // Number of robots does not match. Do not append
                // this mission.
                continue;
            }

            for (k = 0; k < numRobots; k++)
            {
                lastIndex = newBigMissionPlan.plans[k].size() - 1;
		
                if ((lastIndex > 0) && ((int)(missionPlans[j].plans[k].size()) > 0))
                {
                    effect.name = STRING_IMMEDIATE_;
                    effect.index = missionPlans[j].plans[k][0].index;
                    effect.index += bufIndex;
                    newBigMissionPlan.plans[k][lastIndex].effects.push_back(effect);

                    precondition.name = STRING_IMMEDIATE_;
                    missionPlans[j].plans[k][0].preconditions.push_back(precondition);
                }
		
                for (l = 0; l < (int)(missionPlans[j].plans[k].size()); l++)
                {
                    missionPlans[j].plans[k][l].index += bufIndex;

                    for (m = 0; m < (int)(missionPlans[j].plans[k][l].effects.size()); m++)
                    {
                        missionPlans[j].plans[k][l].effects[m].index += bufIndex;
                    }
		    
                    newBigMissionPlan.plans[k].push_back(missionPlans[j].plans[k][l]);
                }
            }

            newBigMissionPlans.push_back(newBigMissionPlan);
        }
    }

    return newBigMissionPlans;
}

//-----------------------------------------------------------------------
// This function merges two lists of data indexes.
//-----------------------------------------------------------------------
vector<int> Planner::mergeDataIndexLists_(
    vector<int> dataIndexListA,
    vector<int> dataIndexListB)
{
    int i;

    for (i = 0; i < (int)(dataIndexListB.size()); i++)
    {
        dataIndexListA.push_back(dataIndexListB[i]);
    }

    dataIndexListA = trimRedundantDataIndex_(dataIndexListA);

    return dataIndexListA;
}

//-----------------------------------------------------------------------
// This function merges two mission plans.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::mergeMissionPlans_(
    vector<MissionMemoryManager::MissionPlan_t> missionPlansA,
    vector<MissionMemoryManager::MissionPlan_t> missionPlansB)
{
    int i;

    for (i = 0; i < (int)(missionPlansB.size()); i++)
    {
        missionPlansA.push_back(missionPlansB[i]);
    }

    return missionPlansA;
}

//-----------------------------------------------------------------------
// This function gets rid of redundant indexes in the list.
//-----------------------------------------------------------------------
vector<int> Planner::trimRedundantDataIndex_(vector<int> dataIndexList)
{
    int i, j;
    vector<int> newDataIndexList;
    bool dataExists;

    for (i = 0; i < (int)(dataIndexList.size()); i++)
    {
        dataExists = false;

        for (j = 0; j < (int)(newDataIndexList.size()); j++)
        {
            if (dataIndexList[i] == newDataIndexList[j])
            {
                dataExists = true;
                break;
            }
        }

        if (!dataExists)
        {
            newDataIndexList.push_back(dataIndexList[i]);
        }
    }

    return newDataIndexList;
}

//-----------------------------------------------------------------------
// This function extracts the maximum velocity from the features list.
//-----------------------------------------------------------------------
string Planner::extractMaxVelocityValue_(MemoryManager::Features_t features)
{
    string maxVelValue = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_MAX_VELOCITY_)
        {
            maxVelValue = features[i].value;
            break;
        }
    }

    return maxVelValue;
}

//-----------------------------------------------------------------------
// This function extracts the aggressiveness from the features list.
//-----------------------------------------------------------------------
string Planner::extractAggressivenessValue_(MemoryManager::Features_t features)
{
    string aggressivenessValue = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_AGGRESSIVENESS_)
        {
            aggressivenessValue = features[i].value;
            break;
        }
    }

    return aggressivenessValue;
}

//-----------------------------------------------------------------------
// This function extracts the number of robots from the features list.
//-----------------------------------------------------------------------
int Planner::extractNumRobots_(MemoryManager::Features_t features)
{
    int i, numRobots = -1;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_NUM_ROBOTS_)
        {
            numRobots = atoi(features[i].value.c_str());
            break;
        }
    }

    return numRobots;
}

//-----------------------------------------------------------------------
// This function extracts the mission time from the features list.
//-----------------------------------------------------------------------
string Planner::extractMissionTime_(MemoryManager::Features_t features)
{
    string missionTimeString = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_MISSION_TIME_)
        {
            missionTimeString = features[i].value.c_str();
            break;
        }
    }

    return missionTimeString;
}

//-----------------------------------------------------------------------
// This function extracts the robot ID from the features list.
//-----------------------------------------------------------------------
int Planner::extractRobotID_(MemoryManager::Features_t features)
{
    int robotID = INVALID_ROBOT_ID_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_ROBOT_ID_)
        {
            robotID = atoi(features[i].value.c_str());
            break;
        }
    }

    return robotID;
}

//-----------------------------------------------------------------------
// This function extracts the vehicle type from the robot constraints
// list
//-----------------------------------------------------------------------
string Planner::extractRobotVehicleType_(
    int robotID,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    vector<MemoryManager::Constraint_t> constraints;
    int i, j;

    for (i = 0; i < (int)(robotConstraintsList.size()); i++)
    {
        if ((robotConstraintsList[i].id) == robotID)
        {
            constraints = robotConstraintsList[i].constraints;

            for (j = 0; j < (int)(constraints.size()); j++)
            {
                if ((constraints[j].name) == CONSTRAINT_NAME_VEHICLE_TYPES_)
                {
                    return (constraints[j].value);
                }
            }
        }
    }

    return EMPTY_STRING_;
}

//-----------------------------------------------------------------------
// This function extracts sub-features (features that correspond to
// sub-missions).
//-----------------------------------------------------------------------
vector<MemoryManager::Features_t> Planner::extractSubFeatures_(MemoryManager::Features_t features)
{
    vector<MemoryManager::Features_t> subFeaturesList;
    MemoryManager::Features_t subFeatures, globalFeatures;
    int i, j;
    bool firstTaskFound = false;
    const bool DEBUG = true;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_TASK_NAME_)
        {
            if (firstTaskFound)
            {
                subFeaturesList.push_back(subFeatures);
            }
            else
            {
                firstTaskFound = true;
            }

            subFeatures = globalFeatures;
            subFeatures.push_back(features[i]);
        }
        else
        {
            if (firstTaskFound)
            {
                subFeatures.push_back(features[i]);
            }
            else
            {
                globalFeatures.push_back(features[i]);
            }
        }
    }

    if ((int)(subFeatures.size()) > 0)
    {
        subFeaturesList.push_back(subFeatures);
    }

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("EXTRACTED SUB-FEATURES:\n");

        for (i = 0; i < (int)(subFeaturesList.size()); i++)
        {
            gDebugger->printfLine();
            for (j = 0; j < (int)(subFeaturesList[i].size()); j++)
            {
                gDebugger->printfDebug("%s  ", subFeaturesList[i][j].name.c_str());
                gDebugger->printfDebug("%s  ", subFeaturesList[i][j].value.c_str());
                gDebugger->printfDebug("%d ", subFeaturesList[i][j].nonIndex);
                gDebugger->printfDebug("%.2f\n", subFeaturesList[i][j].weight);
            }
        }
        gDebugger->printfLine();
    }

    return subFeaturesList;
}

//-----------------------------------------------------------------------
// This function extracts the coordinates from the features list.
//-----------------------------------------------------------------------
MissionMemoryManager::Coordinates_t Planner::extractCoordinates_(MemoryManager::Features_t features)
{
    MissionMemoryManager::Coordinates_t coordinates;
    bool xFound = false, yFound = false;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_START_X_)
        {
            coordinates.x = atof(features[i].value.c_str());
            xFound = true;
        }
        else if (features[i].name == FEATURE_NAME_START_Y_)
        {
            coordinates.y = atof(features[i].value.c_str());
            yFound = true;
        }

        if ((xFound) && (yFound))
        {
            break;
        }
    }

    return coordinates;
}

//-----------------------------------------------------------------------
// This function extracts the geographic coordinates (lat/lon) from the
// features list.
//-----------------------------------------------------------------------
MissionMemoryManager::GeoCoordinates_t Planner::extractGeoCoordinates_(MemoryManager::Features_t features)
{
    MissionMemoryManager::GeoCoordinates_t geoCoordinates;
    bool latFound = false, lonFound = false;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_START_LATITUDE_)
        {
            geoCoordinates.latitude = atof(features[i].value.c_str());
            latFound = true;
        }
        else if (features[i].name == FEATURE_NAME_START_LONGITUDE_)
        {
            geoCoordinates.longitude = atof(features[i].value.c_str());
            lonFound = true;
        }

        if ((latFound) && (lonFound))
        {
            break;
        }
    }

    return geoCoordinates;
}

//-----------------------------------------------------------------------
// This function extracts the task name from the features list.
//-----------------------------------------------------------------------
string Planner::extractTaskName_(MemoryManager::Features_t features)
{
    string taskName = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_TASK_NAME_)
        {
            taskName = features[i].value;
            break;
        }
    }

    return taskName;
}

//-----------------------------------------------------------------------
// This function extracts the localization value from the features list,
// and checks whether maps should be used or not.
//-----------------------------------------------------------------------
bool Planner::extractUseMaps_(MemoryManager::Features_t features)
{
    bool useMaps = false;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if ((features[i].name == FEATURE_NAME_LOCALIZATION_) ||
            (features[i].name == FEATURE_NAME_LOCALIZATION2_))
        {
            if ((features[i].value == FEATURE_VALUE_USEMAPS_) ||
                (features[i].value == FEATURE_VALUE_USEMAPS2_))
            {
                useMaps = true;
            }

            break;
        }
    }

    return useMaps;
}

//-----------------------------------------------------------------------
// This function extracts the environment value from the features list,
// and checks whether the should be running indoor or not.
//-----------------------------------------------------------------------
bool Planner::extractIndoor_(MemoryManager::Features_t features)
{
    bool indoor = false;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if ((features[i].name == FEATURE_NAME_ENVIRONMENT_) ||
            (features[i].name == FEATURE_NAME_ENVIRONMENT2_))
        {
            if ((features[i].value == FEATURE_VALUE_INDOOR_) ||
                (features[i].value == FEATURE_VALUE_INDOOR2_))
            {
                indoor = true;
            }

            break;
        }
    }

    return indoor;
}

//-----------------------------------------------------------------------
// This function will sort the feature list based on the weights.
//-----------------------------------------------------------------------
MemoryManager::Features_t Planner::sortFeatures_(MemoryManager::Features_t features)
{
    MemoryManager::Features_t tmpFeatures, newFeatures;
    double heighestWeight = -1.0;
    int i, heighestFeatureIndex = -1;

    while ((int)(features.size()) > 0)
    {
        heighestWeight = -1.0;
        heighestFeatureIndex = -1;

        for (i = 0; i < (int)(features.size()); i++)
        {
            if (features[i].weight > heighestWeight)
            {
                heighestFeatureIndex = i;
                heighestWeight = features[i].weight;
            }
        }

        if (heighestFeatureIndex != -1)
        {
            tmpFeatures.clear();

            for (i = 0; i < (int)(features.size()); i++)
            {
                if (i == heighestFeatureIndex)
                {
                    newFeatures.push_back(features[i]);
                }
                else
                {
                    tmpFeatures.push_back(features[i]);
                }
            }

            features = tmpFeatures;
        }
        else
        {
            break;
        }
    }

    return newFeatures;
}

//-----------------------------------------------------------------------
// This function will remove all the nonIndex features from the list.
//-----------------------------------------------------------------------
MemoryManager::Features_t Planner::filterNonIndexFeatures_(MemoryManager::Features_t features)
{
    MemoryManager::Features_t filteredFeatures;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (!(features[i].nonIndex))
        {
            filteredFeatures.push_back(features[i]);
        }
    }

    return filteredFeatures;
}

//-----------------------------------------------------------------------
// This function will set the number of robots in terms of the feature
// "NumberOfRobots". If NumberOfRobots is not found, it adds a new
// feaure.
//-----------------------------------------------------------------------
MemoryManager::Features_t Planner::setNumRobots_(
    MemoryManager::Features_t features,
    int numRobots)
{
    MemoryManager::Feature_t feature;
    char valueString[1024];
    int i;
    bool numRobotsEntryFound = false;
    const double DEFAULT_WEIGHT = 1.0;

    sprintf(valueString, "%d", numRobots);

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_NUM_ROBOTS_)
        {
            features[i].value = valueString;
            numRobotsEntryFound = true;
            break;
        }
    }

    if (!numRobotsEntryFound)
    {
        feature.name = FEATURE_NAME_NUM_ROBOTS_;
        feature.value = valueString;
        feature.nonIndex = false;
        feature.weight = DEFAULT_WEIGHT;
        feature.formatType = MemoryManager::FORMAT_INT;
        features.push_back(feature);
    }

    return features;
}

//-----------------------------------------------------------------------
//  This function adds StartSubMission state in the beginning
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::addStartSubMissionState_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    string taskName,
    MissionMemoryManager::Coordinates_t coordinates,
    bool useMaps,
    bool indoor,
    double maxVel,
    int deploymentMethod)
{
    vector<CBRPlanner_PlanSequence_t> newPlans;
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t startSubMissionOperator;
    int i, j;

    startSubMissionOperator = createStartSubMissionOperator_(
        missionPlan,
        taskName,
        coordinates,
        useMaps,
        indoor,
        maxVel,
        deploymentMethod);

    for (i = 0; i < (int)(missionPlan.plans.size()); i++)
    {
        plan.clear();
        startSubMissionOperator.effects[0].index = missionPlan.plans[i][0].index;
        plan.push_back(startSubMissionOperator);

        for (j = 0; j < (int)(missionPlan.plans[i].size()); j++)
        {
            plan.push_back(missionPlan.plans[i][j]);
        }

        newPlans.push_back(plan);
    }

    missionPlan.plans = newPlans;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function adds Start state in the beginning
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::addStartState_(
    MissionMemoryManager::MissionPlan_t missionPlan)
{
    vector<CBRPlanner_PlanSequence_t> newPlans;
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t startOperator;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    int i, j;

    startOperator = createStartOperator_();
    effect.name = STRING_IMMEDIATE_;
    precondition.name = STRING_IMMEDIATE_;

    for (i = 0; i < (int)(missionPlan.plans.size()); i++)
    {
        plan.clear();
        plan.push_back(startOperator);

        if ((int)(missionPlan.plans[i].size()) > 0)
        {
            effect.index = missionPlan.plans[i][0].index;
            plan[0].effects.push_back(effect);

            missionPlan.plans[i][0].preconditions.push_back(precondition);

            for (j = 0; j < (int)(missionPlan.plans[i].size()); j++)
            {
                plan.push_back(missionPlan.plans[i][j]);
            }
        }
        newPlans.push_back(plan);
    }

    missionPlan.plans = newPlans;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function creates Start operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t Planner::createStartOperator_(void)
{
    CBRPlanner_PlanOperator_t startOperator;

    startOperator.name = STRING_START_STATE_;
    startOperator.index = STRING_START_STATE_INDEX_;
    startOperator.description = STRING_START_STATE_DESC_;

    return startOperator;}

//-----------------------------------------------------------------------
//  This function creates StartSubMission operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t Planner::createStartSubMissionOperator_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    string taskName,
    MissionMemoryManager::Coordinates_t coordinates,
    bool useMaps,
    bool indoor,
    double maxVel,
    int deploymentMethod)
{
    CBRPlanner_PlanOperator_t startSubMissionOperator;
    CBRPlanner_Parameter_t parameter;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    char buf[1024];

    startSubMissionOperator.name = STRING_START_SUB_MISSION_;
    sprintf(
        buf,
        "%s%d",
        STRING_START_SUB_MISSION_INDEX_.c_str(),
        gCBRPlanner->getStartSubMissionOperatorIndex(true));
    startSubMissionOperator.index = buf;
    startSubMissionOperator.description = buf;

    parameter.name = STRING_SUB_MISSION_NAME_;
    sprintf(buf, "{\"%s\"}", taskName.c_str());
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);
    
    parameter.name = STRING_DEPLOYMENT_METHOD_;
    switch (deploymentMethod) {
    case DEPLOY_BY_LOCALIZE:
        parameter.value = STRING_DEPLOY_BY_LOCALIZE_;
        break;

    case DEPLOY_BY_GOTO:
    default:
        parameter.value = STRING_DEPLOY_BY_GOTO_;
        break;
    }
    startSubMissionOperator.parameters.push_back(parameter);
    
    parameter.name = STRING_GOAL_LOCATION_;
    sprintf(buf, "{%.2f,+%.2f}", coordinates.x, coordinates.y);
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    parameter.name = STRING_MOVE_TO_LOCATION_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_MOVE_TO_LOCATION_GAIN_);
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    parameter.name = STRING_WANDER_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_WANDER_GAIN_);
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    parameter.name = STRING_AVOID_OBSTACLE_GAIN_;
    if (indoor)
    {
        sprintf(buf, "{%.2f}", DEFAULT_INDOOR_AVOID_OBSTACLE_GAIN_);
    }
    else
    {
        sprintf(buf, "{%.2f}", DEFAULT_OUTDOOR_AVOID_OBSTACLE_GAIN_);
    }
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    parameter.name = STRING_AVOID_OBSTACLE_SPHERE_;
    if (indoor)
    {
        sprintf(buf, "{%.2f}", DEFAULT_INDOOR_AVOID_OBSTACLE_SPHERE_);
    }
    else
    {
        sprintf(buf, "{%.2f}", DEFAULT_OUTDOOR_AVOID_OBSTACLE_SPHERE_);
    }
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    parameter.name = STRING_AVOID_OBSTACLE_SAFETY_MARGIN_;
    sprintf(buf, "{%.2f}", DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_);
    parameter.value = buf;
    startSubMissionOperator.parameters.push_back(parameter);

    precondition.name = STRING_IMMEDIATE_;
    startSubMissionOperator.preconditions.push_back(precondition);

    effect.name = STRING_SUB_MISSION_READY_;
    parameter.name = STRING_SUB_MISSION_DEPLOYMENT_;
    if (useMaps || FORCE_SUB_MISSION_READY_DEPLOY_FIRST_)
    {
        parameter.value = STRING_DEPLOY_FIRST_;
    }
    else
    {
        parameter.value = STRING_EXECUTE_IMMEDIATELY_;
    }
    effect.parameters.push_back(parameter);

    parameter.name = STRING_GOAL_TOLERANCE_;
    if (USE_STATIC_GOAL_TOLERANCE_)
    {
        sprintf(buf, "{%.2f}", DEFAULT_GOAL_TOLERANCE_);
    }
    else
    {
        // Use the max velocity * 1 sec as the goal tolerance.
        sprintf(buf, "{%.2f}", maxVel);
    }
    parameter.value = buf;
    effect.parameters.push_back(parameter);

    parameter.name = STRING_GOAL_LOCATION_;
    sprintf(buf, "{%.2f,+%.2f}", coordinates.x, coordinates.y);
    parameter.value = buf;
    effect.parameters.push_back(parameter);

    startSubMissionOperator.effects.push_back(effect);

    return startSubMissionOperator;
}

//-----------------------------------------------------------------------
// This function adjusts the number of robots in the mission plan.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::adjustNumRobots_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    int numRobots)
{
    CBRPlanner_PlanSequence_t plan;
    int biggestRobotID = 0;
    int i, j, k;
    char bufIndex[1024];

    // Duplicate existing robot if it has less than specified number of
    // robots.
    for (i = 0; i < numRobots; i++)
    {
        if ((int)(missionPlan.plans.size()) > i)
        {
            if ((int)(missionPlan.robotIDs.size()) > i)
            {
                if (biggestRobotID < missionPlan.robotIDs[i])
                {
                    biggestRobotID = missionPlan.robotIDs[i];
                }
            }
            else
            {
                biggestRobotID++;
                missionPlan.robotIDs.push_back(biggestRobotID);
            }
        }
        else
        {
            plan = missionPlan.plans[0];
            sprintf(bufIndex, "%d", i);


            for (j = 0; j < (int)(plan.size()); j++)
            {
                plan[j].index += bufIndex;

                for (k = 0; k < (int)(plan[j].effects.size()); k++)
                {
                    plan[j].effects[k].index += bufIndex;
                }
            }

            missionPlan.plans.push_back(plan);

            biggestRobotID++;
            missionPlan.robotIDs.push_back(biggestRobotID);
        }
    }

    // Delete excessive robot plans.
    while ((int)(missionPlan.plans.size()) > numRobots)
    {
        missionPlan.plans.pop_back();
    }

    while ((int)(missionPlan.robotIDs.size()) > numRobots)
    {
        missionPlan.robotIDs.pop_back();
    }

    return missionPlan;
}

//-----------------------------------------------------------------------
// This function saves new mission plan in the memory.
//-----------------------------------------------------------------------
vector<int> Planner::saveNewMissionPlan(
    MissionMemoryManager::MissionPlan_t missionPlan,
    MemoryManager::Features_t features,
    bool hasStartSubMission)
{
    MissionMemoryManager::MissionPlanList_t missionPlans;
    vector<MemoryManager::Features_t> featuresList;
    vector<int> dataIndexList;
    const vector<int> EMPTY_DATA_INDEX_LIST;
    vector<string> featureNameList;
    int i, dataIndex;

    if (hasStartSubMission)
    {
        missionPlans = separateSubMissions_(
            missionPlan,
            features,
            &featuresList);
    }
    else
    {
        missionPlans.push_back(missionPlan);
        featuresList.push_back(features);
    }

    if (missionPlans.size() != featuresList.size())
    {
        gWindows->printfPlannerWindow(
            "Warning: Inadequate mission plan. Not saved.\n");
        return EMPTY_DATA_INDEX_LIST;
    }

    for (i = 0; i < (int)(missionPlans.size()); i++)
    {
        features = filterNonIndexFeatures_(featuresList[i]);
        features = sortFeatures_(features);
        dataIndex = gCBRPlanner->missionMemoryManager()->saveNewMissionPlan(
            missionPlans[i],
            features);
        dataIndexList.push_back(dataIndex);
    }

    return dataIndexList;
}

//-----------------------------------------------------------------------
// This function separates sub-missions.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlanList_t Planner::separateSubMissions_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    MemoryManager::Features_t features,
    vector<MemoryManager::Features_t> *featuresList)
{
    MissionMemoryManager::MissionPlanList_t missionPlans;
    const MissionMemoryManager::MissionPlanList_t EMPTY_MISSION_PLANS;
    MissionMemoryManager::MissionPlan_t tmpMissionPlan;
    CBRPlanner_PlanSequence_t plan;
    string taskName;
    int i, j, lastIndex, missionCount;
    int robotID = 0, biggestRobotID = 0;
    bool firstStartSubMissionFound = false;
    
    tmpMissionPlan = missionPlan;
    featuresList->clear();

    for (i = 0; i < (int)(missionPlan.plans.size()); i++)
    {
        // Check for all the robots.
        firstStartSubMissionFound = false;
        tmpMissionPlan.plans.clear();
        tmpMissionPlan.robotIDs.clear();
        plan.clear();
        missionCount = 0;

        if ((int)(missionPlan.robotIDs.size()) > i)
        {
            robotID = missionPlan.robotIDs[i];

            if (robotID > biggestRobotID)
            {
                biggestRobotID = robotID;
            }
        }
        else
        {
            biggestRobotID++;
            robotID = biggestRobotID;
        }

        for (j = 0; j < (int)(missionPlan.plans[i].size()); j++)
        {
            // Check the plan sequence for each robot.
            if (missionPlan.plans[i][j].name == STRING_START_SUB_MISSION_)
            {
                if (firstStartSubMissionFound)
                {
                    plan[0].preconditions.clear();
                    lastIndex = (int)(plan.size()) - 1;
                    plan[lastIndex].effects.clear();

                    if (i == 0)
                    {
                        // Case when this is for the first robot.
                        tmpMissionPlan.plans.push_back(plan);
                        tmpMissionPlan.robotIDs.push_back(robotID);
                        missionPlans.push_back(tmpMissionPlan);

                        features = renameTaskInFeatures_(features, taskName);
                        featuresList->push_back(features);
                    }
                    else
                    {
                        // Case when this is for the second or higher robot.
                        if (((int)(missionPlans.size()) > missionCount) &&
                            ((int)(featuresList->size()) > missionCount))
                        {
                            if (getTaskNameInFeatures_((*featuresList)[missionCount]) != taskName)
                            {
                                gWindows->printfPlannerWindow(
                                    "Warning: Inadequate mission plan. Not saved.\n");
                                return EMPTY_MISSION_PLANS;
                            }

                            missionPlans[missionCount].plans.push_back(plan);
                            missionPlans[missionCount].robotIDs.push_back(robotID);
                        }
                        else
                        {
                            gWindows->printfPlannerWindow(
                                "Warning: Inadequate mission plan. Not saved.\n");
                            return EMPTY_MISSION_PLANS;
                        }
                    }

                    missionCount++;
                    plan.clear();
                    tmpMissionPlan.plans.clear();
                    tmpMissionPlan.robotIDs.clear();
                }
                else
                {
                    firstStartSubMissionFound = true;
                }

                taskName = extractTaskNameFromStartSubMission_(missionPlan.plans[i][j]);
            }
            else
            {
                if (firstStartSubMissionFound)
                {
                    plan.push_back(missionPlan.plans[i][j]);
                }
            }
        }

        if (firstStartSubMissionFound)
        {
            if (i == 0)
            {
                if ((int)(plan.size()) > 0)
                {
                    // Case when this is for the first robot.
                    tmpMissionPlan.plans.push_back(plan);
                    tmpMissionPlan.robotIDs.push_back(robotID);
                    missionPlans.push_back(tmpMissionPlan);

                    features = renameTaskInFeatures_(features, taskName);
                    featuresList->push_back(features);
                }
            }
            else
            {
                // Case when this is for the second or higher robot.
                if (((int)(missionPlans.size()) > missionCount) &&
                    ((int)(featuresList->size()) > missionCount))
                {
                    if (getTaskNameInFeatures_((*featuresList)[missionCount]) != taskName)
                    {
                        gWindows->printfPlannerWindow(
                            "Warning: Inadequate mission plan. Not saved.\n");
                        return EMPTY_MISSION_PLANS;
                    }

                    missionPlans[missionCount].plans.push_back(plan);
                    missionPlans[missionCount].robotIDs.push_back(robotID);
                }
                else
                {
                    gWindows->printfPlannerWindow(
                        "Warning: Inadequate mission plan. Not saved.\n");
                    return EMPTY_MISSION_PLANS;
                }
            }
        }
    }

    return missionPlans;
}

//-----------------------------------------------------------------------
// This function changes the name of the task in the feature.
//-----------------------------------------------------------------------
string Planner::getTaskNameInFeatures_(MemoryManager::Features_t features)
{
    string taskName = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_TASK_NAME_)
        {
            taskName = features[i].value;
        }
    }

    return taskName;
}

//-----------------------------------------------------------------------
// This function changes the name of the task in the feature.
//-----------------------------------------------------------------------
MemoryManager::Features_t Planner::renameTaskInFeatures_(
    MemoryManager::Features_t features,
    string taskName)
{
    int i;

    for (i = 0; i < (int)(features.size()); i++)
    {
        if (features[i].name == FEATURE_NAME_TASK_NAME_)
        {
            features[i].value = taskName;
        }
    }

    return features;
}

//-----------------------------------------------------------------------
// This function extracts the task name from StartSubMission state.
//-----------------------------------------------------------------------
string Planner::extractTaskNameFromStartSubMission_(
    CBRPlanner_PlanOperator_t startSubMissionOperator)
{
    int i;
    string taskName = EMPTY_STRING_;
    CBRPlanner_Parameters_t parameters;

    parameters = startSubMissionOperator.parameters;

    for (i = 0; i < (int)(parameters.size()); i++)
    {
        if (parameters[i].name == STRING_SUB_MISSION_NAME_)
        {
            taskName = extractValueContent_(parameters[i].value, true);
        }
    }

    return taskName;
}

//-----------------------------------------------------------------------
// This function extracts the task name from StartSubMission state.
//-----------------------------------------------------------------------
string Planner::extractValueContent_(string value, bool withoutQuote)
{
    string content;
    string::size_type index1, index2;


    index1 = value.find("{");
    index2 = value.find("}");

    if ((index1 != string::npos) && (index2 != string::npos))
    {
        index1++;
        content = value.substr(index1, index2-index1);

        if (withoutQuote)
        {
            index1 = content.find("\"");
            index2 = content.find("\"", index1+1);

            if ((index1 != string::npos) && (index1 != string::npos))
            {
                index1++;
                content = content.substr(index1, index2-index1);
            }
        }
    }

    return content;
}

//-----------------------------------------------------------------------
// This function fixes the mission plan.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    string archType;
    vector<string> maxVelValues, baseVelValues;
    vector<int> robotIDs;
    int numRobots, newNumRobots;

    // Remember variables.
    archType = missionplan.archType;
    numRobots = missionplan.plans.size();
    maxVelValues = missionplan.maxVelValues;
    baseVelValues = missionplan.baseVelValues;
    robotIDs = missionplan.robotIDs;

    // Ask RepairPlan to fix the plan.
    missionplan = repairPlan_->repairMissionPlan(missionplan, fixed);

    // Add the start state.
    missionplan = addStartState_(missionplan);

    // Specify the type.
    missionplan.type = CBRPLANNER_MISSIONPLAN_REPAIRED;

    // Check the number of robots.
    newNumRobots = missionplan.plans.size();

    if (numRobots != newNumRobots)
    {
        gWindows->printfPlannerWindow("Warning: Planner::repairMissionPlan().\n");
        gWindows->printfPlannerWindow(
            "The number of robots changed from %d to %d.\n",
            numRobots,
            newNumRobots);

        maxVelValues.resize(newNumRobots);
        baseVelValues.resize(newNumRobots);
        robotIDs.resize(newNumRobots);
    }

    // Restore the variables.
    missionplan.archType = archType;
    missionplan.maxVelValues = maxVelValues;
    missionplan.baseVelValues = baseVelValues;
    missionplan.robotIDs = robotIDs;

    return missionplan;
}

//-----------------------------------------------------------------------
// This function reverses the order of the robot in the mission
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::reverseRobotOrder(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    vector<CBRPlanner_PlanSequence_t> plans;
    vector<string> maxVelValues;
    vector<string> baseVelValues;
    vector<int> robotIDs;
    vector<int> dataIndexList;
    vector<CBRPlanner_ExtraData_t> extraDataList;

    plans = missionplan.plans;
    missionplan.plans.clear();

    maxVelValues = missionplan.maxVelValues;
    missionplan.maxVelValues.clear();

    baseVelValues = missionplan.baseVelValues;
    missionplan.baseVelValues.clear();

    robotIDs = missionplan.robotIDs;
    missionplan.robotIDs.clear();

    while (plans.size() > 0)
    {
        missionplan.plans.push_back(plans.back());
        plans.pop_back();

        missionplan.maxVelValues.push_back(maxVelValues.back());
        maxVelValues.pop_back();

        missionplan.baseVelValues.push_back(baseVelValues.back());
        baseVelValues.pop_back();

        missionplan.robotIDs.push_back(robotIDs.back());
        robotIDs.pop_back();
    }

    dataIndexList = missionplan.dataIndexList;
    missionplan.dataIndexList.clear();

    while (dataIndexList.size() > 0)
    {
        missionplan.dataIndexList.push_back(dataIndexList.back());
        dataIndexList.pop_back();
    }

    extraDataList = missionplan.extraDataList;
    missionplan.extraDataList.clear();

    while (extraDataList.size() > 0)
    {
        missionplan.extraDataList.push_back(extraDataList.back());
        extraDataList.pop_back();
    }

    missionplan = addStartState_(missionplan);

    *fixed = true;

    return missionplan;
}

//-----------------------------------------------------------------------
// This function merges multiple missions into one.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t Planner::mergeMissions(
    vector<MissionMemoryManager::MissionPlan_t> missionplans,
    int *numRobots,
    bool *fixed)
{
    MissionMemoryManager::MissionPlan_t mergedMissionPlan;
    MissionMemoryManager::MissionPlan_t missionplan;
    vector<CBRPlanner_PlanSequence_t> plans;
    vector<string> maxVelValues;
    vector<string> baseVelValues;
    vector<int> robotIDs;
    vector<int> dataIndexList;
    vector<CBRPlanner_ExtraData_t> extraDataList;
    int i, j, numMissions;

    *fixed = false;
    numMissions = missionplans.size();

    if (numMissions == 0)
    {
        gWindows->printfPlannerWindow("Warning: Planner::mergeMissions(). Input is empty.\n");
        return mergedMissionPlan;
    }

    mergedMissionPlan = missionplans[0];

    if (numMissions == 1)
    {
        mergedMissionPlan = addStartState_(mergedMissionPlan);
        *numRobots = mergedMissionPlan.plans.size();
        *fixed = true;
        return mergedMissionPlan;
    }

    for (i = 1; i < numMissions; i++) 
    {
        missionplan = missionplans[i];

        plans = missionplan.plans;
        maxVelValues = missionplan.maxVelValues;
        baseVelValues = missionplan.baseVelValues;
        robotIDs = missionplan.robotIDs;

        for (j = 0; j < (int)(plans.size()); j++)
        {
            mergedMissionPlan.plans.push_back(plans[j]);
            mergedMissionPlan.maxVelValues.push_back(maxVelValues[j]);
            mergedMissionPlan.baseVelValues.push_back(baseVelValues[j]);
            mergedMissionPlan.robotIDs.push_back(robotIDs[j]);
        }


        dataIndexList = missionplan.dataIndexList;

        for (j = 0; j < (int)(dataIndexList.size()); j++)
        {
            mergedMissionPlan.dataIndexList.push_back(dataIndexList[j]);
        }

        extraDataList = missionplan.extraDataList;

        for (j = 0; j < (int)(extraDataList.size()); j++)
        {
            mergedMissionPlan.extraDataList.push_back(extraDataList[j]);
        }
    }

    mergedMissionPlan = addStartState_(mergedMissionPlan);
    *numRobots = mergedMissionPlan.plans.size();
    *fixed = true;

    return mergedMissionPlan;
}

//-----------------------------------------------------------------------
// This function toggles the CNP mode.
//-----------------------------------------------------------------------
int Planner::toggleCNP(void)
{
    int cnpMode = cnpMode_;

    cnpMode++;

    if (cnpMode >= NUM_CNP_MODES)
    {
        cnpMode = CNP_MODE_DISABLED;
    }

    setCNPMode(cnpMode);

    return cnpMode;
}

//-----------------------------------------------------------------------
// This function sets the CNP mode.
//-----------------------------------------------------------------------
void Planner::setCNPMode(int cnpMode)
{
    bool printNewMode = false;

    if (cnpMode_ != cnpMode)
    {
        printNewMode = true;
    }

    cnpMode_ = cnpMode;

    switch (cnpMode_) {

    case CNP_MODE_PREMISSION:
        if (cnpServerManager_ == NULL)
        {
            cnpServerManager_ = new CNP_Server_Manager();
        }
        break;
    }

    if (printNewMode)
    {
        gWindows->printfPlannerWindow(
            "CNP Mode: %d [%s]\n",
            cnpMode_,
            CNP_MODE_STRING[cnpMode_].c_str());
    }
}

//-----------------------------------------------------------------------
// This function replaces the geographic coordinates (lat/lon) of a
// target.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::updateTargetLatLon_(
    vector<MissionMemoryManager::MissionPlan_t> missionPlans,
    MissionMemoryManager::GeoCoordinates_t geoCoordinates)
{
    vector<MissionMemoryManager::MissionPlan_t> newMissionPlans;
    MissionMemoryManager::MissionPlan_t missionPlan;
    CBRPlanner_Parameter_t parameter;
    char latBuf[1024], lonBuf[1024];
    int i, j, k, l;

    sprintf(latBuf, "{%.6f}", geoCoordinates.latitude);
    sprintf(lonBuf, "{%.6f}", geoCoordinates.longitude);

    for (i = 0; i < (int)(missionPlans.size()); i++)
    {
        missionPlan = missionPlans[i];

        for (j = 0; j < (int)(missionPlan.plans.size()); j++)
        {
            for (k = 0; k < (int)(missionPlan.plans[j].size()); k++)
            {
                for (l = 0; l < (int)(missionPlan.plans[j][k].parameters.size()); l++)
                {
                    parameter = missionPlan.plans[j][k].parameters[l];

                    if (parameter.name == STRING_TARGET_LATITUDE_)
                    {
                        missionPlan.plans[j][k].parameters[l].value = latBuf;
                    }
                    else if (parameter.name == STRING_TARGET_LONGITUDE_)
                    {
                        missionPlan.plans[j][k].parameters[l].value = lonBuf;
                    }
                }
            }
        }

        newMissionPlans.push_back(missionPlan);
    }

    return newMissionPlans;
}

//-----------------------------------------------------------------------
// This function replaces the time window in the mission.
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> Planner::updateTimeWindow_(
    vector<MissionMemoryManager::MissionPlan_t> missionPlans,
    string missionTimeString)
{
    struct tm tmMissionTime, *tmStart = NULL, *tmEnd = NULL;
    vector<MissionMemoryManager::MissionPlan_t> newMissionPlans;
    MissionMemoryManager::MissionPlan_t missionPlan;
    CBRPlanner_Parameter_t parameter;
    long startTime, endTime;
    char timeWindowValue[2048], startTimeBuf[512], endTimeBuf[512];
    int i, j, k, l;
    const int DAY_LIGHT_SAVING_FLAG = -1;

    // Time in "hh:mm:ss+MM/DD/YYYY" format. Convert it as epoch time..
    if (strptime(
            missionTimeString.c_str(),
            MISSION_TIME_FORMAT_.c_str(),
            &tmMissionTime) == NULL)
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::updateTimeWindow_(). Invalid mission time string [%s].\n",
            missionTimeString.c_str()); 

        return missionPlans;
    }

    tmMissionTime.tm_isdst = DAY_LIGHT_SAVING_FLAG;
    startTime = (long)mktime(&tmMissionTime);

    if (startTime < 0)
    {
        gWindows->printfPlannerWindow(
            "Error: Planner::updateTimeWindow_(). mktime() failed [%s].\n",
            missionTimeString.c_str()); 

        return missionPlans;
    }

    // Set up the string for the start time.
    tmStart = localtime((time_t *)&startTime);

    strftime(
        startTimeBuf,
        sizeof(startTimeBuf),
        MISSION_TIME_FORMAT_.c_str(),
        tmStart);

    // Calculate the end time.
    endTime = startTime + DEFAULT_TIME_WINDOW_RANGE_SEC_;

    // Set up the string for the end time.
    tmEnd = localtime((time_t *)&endTime);

    strftime(
        endTimeBuf,
        sizeof(endTimeBuf),
        MISSION_TIME_FORMAT_.c_str(),
        tmEnd);

    sprintf(
        timeWindowValue,
        "{\"%s\",\"%s\"}",
        startTimeBuf,
        endTimeBuf);

    for (i = 0; i < (int)(missionPlans.size()); i++)
    {
        missionPlan = missionPlans[i];

        for (j = 0; j < (int)(missionPlan.plans.size()); j++)
        {
            for (k = 0; k < (int)(missionPlan.plans[j].size()); k++)
            {
                for (l = 0; l < (int)(missionPlan.plans[j][k].parameters.size()); l++)
                {
                    parameter = missionPlan.plans[j][k].parameters[l];

                    if (parameter.name == STRING_TIME_WINDOW_)
                    {
                        missionPlan.plans[j][k].parameters[l].value = timeWindowValue;
                    }
                }
            }
        }

        newMissionPlans.push_back(missionPlan);
    }

    return newMissionPlans;
}

//-----------------------------------------------------------------------
// This function prints out the debugging message that is called inside
// CNP library.
//-----------------------------------------------------------------------
void CNPPrintf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    gWindows->printfPlannerWindow("CNP: %s", buf);
}

/**********************************************************************
 * $Log: planner.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.7  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.6  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.5  2007/09/18 22:34:23  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.4  2007/01/30 15:12:42  nadeem
 * Added code to filter the cases based on No Fly, No Comm and Weather Zones
 *
 * Revision 1.3  2006/10/23 22:17:32  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
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
 * Revision 1.16  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.15  2006/02/19 17:52:00  endo
 * Experiment related modifications
 *
 * Revision 1.14  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.13  2005/10/21 00:28:57  endo
 * Bug fix
 *
 * Revision 1.12  2005/09/22 21:43:50  endo
 * CNP version can now suggest multiple solutions.
 *
 * Revision 1.11  2005/08/12 21:48:30  endo
 * More improvements for August demo.
 *
 * Revision 1.10  2005/08/09 19:12:03  endo
 * Things improved for the August demo.
 *
 * Revision 1.9  2005/07/31 03:41:41  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.8  2005/07/27 20:40:17  endo
 * 3D visualization improved.
 *
 * Revision 1.7  2005/07/18 19:43:21  alanwags
 * Debugging TaskInfo added.
 *
 * Revision 1.6  2005/07/18 19:18:21  alanwags
 * CBR-CNP integration
 *
 * Revision 1.5  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.4  2005/06/23 22:09:05  endo
 * Adding the mechanism to talk to CNP.
 *
 * Revision 1.3  2005/05/18 21:22:22  endo
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
