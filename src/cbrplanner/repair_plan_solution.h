/**********************************************************************
 **                                                                  **
 **                     repair_plan_solution.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_plan_solution.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef REPAIR_PLAN_SOLUTION_H
#define REPAIR_PLAN_SOLUTION_H

#include <vector>
#include <string>

#include "repair_plan_types.h"
#include "repair_plan.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "repair_memory_manager.h"

using std::vector;
using std::string;

class RepairPlanSolution {

protected:
    vector<RepairQuestion_t> repairFollowUpQuestions_;
    MissionMemoryManager::Coordinates_t targetPoint_;
    OffendingState_t offendingState_;
    int currentRepairFollowUpQuestionID_;
    bool offendingStateIsKnown_;

    static const string EMPTY_STRING_;
    static const string STRING_START_;
    static const string STRING_GOTO_;
    static const string STRING_GOTO_INDEX_;
    static const string STRING_GOAL_LOCATION_;
    static const string STRING_MOVE_TO_LOCATION_GAIN_;
    static const string STRING_WANDER_GAIN_;
    static const string STRING_AVOID_OBSTACLE_GAIN_;
    static const string STRING_AVOID_OBSTACLE_SPHERE_;
    static const string STRING_AVOID_OBSTACLE_SAFETY_MARGIN_;
    static const string STRING_IMMEDIATE_;
    static const string STRING_AT_GOAL_;
    static const string STRING_GOAL_TOLERANCE_;
    static const string STRING_START_SUB_MISSION_;
    static const string STRING_SUB_MISSION_NAME_;
    static const string STRING_SUB_MISSION_READY_;
    static const string STRING_SUB_MISSION_DEPLOYMENT_;
    static const string STRING_DEPLOY_FIRST_;
    static const string STRING_EXECUTE_IMMEDIATELY_;
    static const string STRING_ABOUTFACE_;
    static const string STRING_ABOUTFACE_INDEX_;
    static const string STRING_ABOUTFACE_COMPLETED_;
    static const string STRING_START_SUB_MISSION_INDEX_;
    static const string STRING_PROCEED_ALONG_HALLWAY_;
    static const string STRING_AT_END_OF_HALL_;
    static const string STRING_HOSTAGE_;
    static const string STRING_STAY_WITH_;
    static const string STRING_STAY_WITH_INDEX_;
    static const string STRING_OBJECTS_;
    static const string STRING_MOVE_TO_OBJECT_GAIN_;
    static const double DEFAULT_MOVE_TO_LOCATION_GAIN_;
    static const double DEFAULT_WANDER_GAIN_;
    static const double DEFAULT_INDOOR_AVOID_OBSTACLE_GAIN_;
    static const double DEFAULT_OUTDOOR_AVOID_OBSTACLE_GAIN_;
    static const double DEFAULT_INDOOR_AVOID_OBSTACLE_SPHERE_;
    static const double DEFAULT_OUTDOOR_AVOID_OBSTACLE_SPHERE_;
    static const double DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_;
    static const double DEFAULT_GOAL_TOLERANCE_;
    static const double DEFAULT_MOVE_TO_OBJECT_GAIN_;

    MissionMemoryManager::MissionPlan_t addGoToState_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        OffendingState_t offendingState,
        MissionMemoryManager::Coordinates_t coordinates,
        bool indoor,
        bool avoidAboutFaceState,
        bool *added);
    MissionMemoryManager::MissionPlan_t addStartSubMissionState_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        string taskName,
        OffendingState_t offendingState,
        MissionMemoryManager::Coordinates_t coordinates,
        bool useMaps,
        bool indoor,
        bool *added);
    MissionMemoryManager::MissionPlan_t updateGoalLocation_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        OffendingState_t offendingState,
        MissionMemoryManager::Coordinates_t coordinates,
        bool *fixed);
    MissionMemoryManager::MissionPlan_t deleteState_(
        MissionMemoryManager::MissionPlan_t missionplan,
        OffendingState_t offendingState,
        bool *deleted);
    MissionMemoryManager::MissionPlan_t addAboutFaceState_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        OffendingState_t offendingState,
        bool addToProceedAlongHallwayOnly,
        bool *added);
    MissionMemoryManager::MissionPlan_t replaceWithStayWithState_(
        MissionMemoryManager::MissionPlan_t missionPlan,
        OffendingState_t offendingState,
        string targetObjectString,
        bool indoor,
        bool *replaced);
    CBRPlanner_PlanOperator_t createGoToOperator_(
        CBRPlanner_Preconditions_t preconditions,
        MissionMemoryManager::Coordinates_t coordinates,
        bool indoor);
    CBRPlanner_PlanOperator_t createStartSubMissionOperator_(
        string taskName,
        CBRPlanner_Preconditions_t preconditions,
        MissionMemoryManager::Coordinates_t coordinates,
        bool useMaps,
        bool indoor);
    CBRPlanner_PlanOperator_t createAboutFaceOperator_(void);
    CBRPlanner_PlanOperator_t createStayWithOperator_(
        CBRPlanner_Preconditions_t preconditions,
        string targetObjectString,
        bool indoor);

public:
    RepairPlanSolution(void);
    virtual ~RepairPlanSolution(void);
    virtual MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
    RepairQuestion_t getRepairFollowUpQuestion_(void);
    bool offendingStateIsKnown(void);
    void addRepairFollowUpQuestion(RepairQuestion_t followUpQuestion);
    void saveTargetPoint(double x, double y);
    void updateOffendingState(OffendingState_t offendingState);
    void resetFollowUpQuestions(void);
    void setOffendingStateAsKnown(void);
    void setOffendingStateAsUnknown(void);
};

inline MissionMemoryManager::MissionPlan_t RepairPlanSolution::repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,    
        bool *fixed)
{
    *fixed = false;
    return missionplan;
}

inline bool RepairPlanSolution::offendingStateIsKnown(void)
{
    return offendingStateIsKnown_;
}

inline void RepairPlanSolution::addRepairFollowUpQuestion(
    RepairQuestion_t repairQuestion)
{
    repairFollowUpQuestions_.push_back(repairQuestion);
}

inline void RepairPlanSolution::saveTargetPoint(double x, double y)
{
    targetPoint_.x = x;
    targetPoint_.y = y;
}

inline void RepairPlanSolution::updateOffendingState(
    OffendingState_t offendingState)
{
    offendingState_ = offendingState;
}

inline void RepairPlanSolution::setOffendingStateAsKnown(void)
{
    offendingStateIsKnown_ = true;
}

inline void RepairPlanSolution::setOffendingStateAsUnknown(void)
{
    offendingStateIsKnown_ = false;
}

class RPS_EnterBuilding : public RepairPlanSolution {

public:
    RPS_EnterBuilding(void);
    ~RPS_EnterBuilding(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_EnterRoom : public RepairPlanSolution {

public:
    RPS_EnterRoom(void);
    ~RPS_EnterRoom(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_AddIndoorPoint : public RepairPlanSolution {

public:
    RPS_AddIndoorPoint(void);
    ~RPS_AddIndoorPoint(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_AddOutdoorPoint : public RepairPlanSolution {

public:
    RPS_AddOutdoorPoint(void);
    ~RPS_AddOutdoorPoint(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_MovePoint : public RepairPlanSolution {

public:
    RPS_MovePoint(void);
    ~RPS_MovePoint(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_DeletePoint : public RepairPlanSolution {

public:
    RPS_DeletePoint(void);
    ~RPS_DeletePoint(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_AddTurnAroundHallway : public RepairPlanSolution {

public:
    RPS_AddTurnAroundHallway(void);
    ~RPS_AddTurnAroundHallway(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

class RPS_ReplaceWithStayWithHostage : public RepairPlanSolution {

public:
    RPS_ReplaceWithStayWithHostage(void);
    ~RPS_ReplaceWithStayWithHostage(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
};

#endif

/**********************************************************************
 * $Log: repair_plan_solution.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1  2005/02/07 19:53:45  endo
 * Mods from usability-2004
 *
 **********************************************************************/
