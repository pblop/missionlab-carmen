/**********************************************************************
 **                                                                  **
 **                    repair_plan_solution.cc                       **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_plan_solution.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <string>

#include "cbrplanner.h"
#include "repair_plan_solution.h"
#include "file_utils.h"
#include "windows.h"
#include "debugger.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string RepairPlanSolution::EMPTY_STRING_ = "";
const string RepairPlanSolution::STRING_START_ = "Start";
const string RepairPlanSolution::STRING_GOTO_ = "GoTo";
const string RepairPlanSolution::STRING_GOTO_INDEX_ = "$Repair_GoTo";
const string RepairPlanSolution::STRING_GOAL_LOCATION_ = "%Goal_Location";
const string RepairPlanSolution::STRING_MOVE_TO_LOCATION_GAIN_ = "%move_to_location_gain";
const string RepairPlanSolution::STRING_WANDER_GAIN_ = "%wander_gain";
const string RepairPlanSolution::STRING_AVOID_OBSTACLE_GAIN_ = "%avoid_obstacle_gain";
const string RepairPlanSolution::STRING_AVOID_OBSTACLE_SPHERE_ = "%avoid_obstacle_sphere";
const string RepairPlanSolution::STRING_AVOID_OBSTACLE_SAFETY_MARGIN_ = "%avoid_obstacle_safety_margin";
const string RepairPlanSolution::STRING_IMMEDIATE_ = "Immediate";
const string RepairPlanSolution::STRING_AT_GOAL_ = "AtGoal";
const string RepairPlanSolution::STRING_GOAL_TOLERANCE_ = "%Goal_Tolerance";
const string RepairPlanSolution::STRING_START_SUB_MISSION_ = "StartSubMission";
const string RepairPlanSolution::STRING_START_SUB_MISSION_INDEX_ = "$StartSubMission";
const string RepairPlanSolution::STRING_SUB_MISSION_NAME_ = "%sub_mission_name";
const string RepairPlanSolution::STRING_SUB_MISSION_READY_ = "SubMissionReady";
const string RepairPlanSolution::STRING_SUB_MISSION_DEPLOYMENT_ = "%sub_mission_deployment";
const string RepairPlanSolution::STRING_DEPLOY_FIRST_ = "{DEPLOY_FIRST}";
const string RepairPlanSolution::STRING_EXECUTE_IMMEDIATELY_ = "{EXECUTE_IMMEDIATELY}";
const string RepairPlanSolution::STRING_ABOUTFACE_ = "AboutFace";
const string RepairPlanSolution::STRING_ABOUTFACE_INDEX_ = "$Repair_AboutFace";
const string RepairPlanSolution::STRING_ABOUTFACE_COMPLETED_ = "AboutFaceCompleted";
const string RepairPlanSolution::STRING_PROCEED_ALONG_HALLWAY_ = "ProceedAlongHallway";
const string RepairPlanSolution::STRING_AT_END_OF_HALL_ = "AtEndOfHall";
const string RepairPlanSolution::STRING_HOSTAGE_ = "Hostage";
const string RepairPlanSolution::STRING_STAY_WITH_ = "StayWith";
const string RepairPlanSolution::STRING_STAY_WITH_INDEX_ = "$StayWith";
const string RepairPlanSolution::STRING_OBJECTS_ = "%Objects";
const string RepairPlanSolution::STRING_MOVE_TO_OBJECT_GAIN_ = "%move_to_object_gain";
const double RepairPlanSolution::DEFAULT_MOVE_TO_LOCATION_GAIN_ = 1.0;
const double RepairPlanSolution::DEFAULT_WANDER_GAIN_ = 0.0;
const double RepairPlanSolution::DEFAULT_INDOOR_AVOID_OBSTACLE_GAIN_ = 0.3;
const double RepairPlanSolution::DEFAULT_OUTDOOR_AVOID_OBSTACLE_GAIN_ = 1.0;
const double RepairPlanSolution::DEFAULT_INDOOR_AVOID_OBSTACLE_SPHERE_ = 0.7;
const double RepairPlanSolution::DEFAULT_OUTDOOR_AVOID_OBSTACLE_SPHERE_ = 2.2;
const double RepairPlanSolution::DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_ = 0.3;
// ENDO
//const double RepairPlanSolution::DEFAULT_GOAL_TOLERANCE_ = 0.5;
const double RepairPlanSolution::DEFAULT_GOAL_TOLERANCE_ = 50.0;
const double RepairPlanSolution::DEFAULT_MOVE_TO_OBJECT_GAIN_ = 1.0;

//=======================================================================
// Constructor for RepairPlanSolution
//=======================================================================
RepairPlanSolution::RepairPlanSolution(void)
{
    currentRepairFollowUpQuestionID_ = 0;
    offendingStateIsKnown_ = false;
}

//-----------------------------------------------------------------------
// Destructor for RepairPlanSolution
//-----------------------------------------------------------------------
RepairPlanSolution::~RepairPlanSolution(void)
{
}

//-----------------------------------------------------------------------
//  This function adds GoTo state before the sepcified state.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::addGoToState_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    OffendingState_t offendingState,
    MissionMemoryManager::Coordinates_t coordinates,
    bool indoor,
    bool avoidAboutFaceState,
    bool *added)
{
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t gotoOperator;
    CBRPlanner_Precondition_t precondition;
    const CBRPlanner_Preconditions_t EMPTY_PRECONDITIONS;
    int i, numOperators;

    *added = false;

    if ((int)(missionPlan.plans.size()) == 0)
    {
        return missionPlan;
    }

    // Currently, it only adds to the first robot.
    if (offendingState.description == STRING_START_)
    {
        gotoOperator = createGoToOperator_(
            EMPTY_PRECONDITIONS,
            coordinates,
            indoor);

        if ((int)(missionPlan.plans[0].size()) > 0)
        {
            gotoOperator.effects[0].index = missionPlan.plans[0][0].index;
        }

        plan.push_back(gotoOperator);
        *added = true;
    }

    for (i = 0; i < (int)(missionPlan.plans[0].size()); i++)
    {
        if ((offendingState.description == missionPlan.plans[0][i].description) &&
            (!(*added)))
        {
            if ((avoidAboutFaceState) &&
                (missionPlan.plans[0][i].name == STRING_ABOUTFACE_) &&
                (i > 0))
            {
                // Special case. If the offending state is AboutFace,
                // add GoTo in the previous state.
                plan.pop_back();
                i--;
            }

            gotoOperator = createGoToOperator_(
                missionPlan.plans[0][i].preconditions,
                coordinates,
                indoor);

            numOperators = plan.size();
            if (numOperators > 0)
            {
                plan[numOperators-1].effects[0].index = gotoOperator.index;
            }

            gotoOperator.effects[0].index = missionPlan.plans[0][i].index;
            plan.push_back(gotoOperator);
            *added = true;
        }

        plan.push_back(missionPlan.plans[0][i]);
    }

    missionPlan.plans[0] = plan;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function adds StartSubMission state in the beginning
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::addStartSubMissionState_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    string taskName,
    OffendingState_t offendingState,
    MissionMemoryManager::Coordinates_t coordinates,
    bool useMaps,
    bool indoor,
    bool *added)
{
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t startSubMissionOperator;
    const CBRPlanner_Preconditions_t EMPTY_PRECONDITIONS;
    int i, numOperators;

    // Currently, it only adds to the first robot.
    if ((int)(missionPlan.plans.size()) == 0)
    {
        return missionPlan;
    }

    if (offendingState.description == STRING_START_)
    {
        startSubMissionOperator = createStartSubMissionOperator_(
            taskName,
            EMPTY_PRECONDITIONS,
            coordinates,
            useMaps,
            indoor);

        if ((int)(missionPlan.plans[0].size()) > 0)
        {
            startSubMissionOperator.effects[0].index = missionPlan.plans[0][0].index;
        }

        plan.push_back(startSubMissionOperator);
        *added = true;
    }


    for (i = 0; i < (int)(missionPlan.plans[0].size()); i++)
    {
        if (offendingState.description == missionPlan.plans[0][i].description)
        {
            startSubMissionOperator = createStartSubMissionOperator_(
                taskName,
                missionPlan.plans[0][i].preconditions,
                coordinates,
                useMaps,
                indoor);

            numOperators = plan.size();
            if (numOperators > 0)
            {
                plan[numOperators-1].effects[0].index = startSubMissionOperator.index;
            }

            startSubMissionOperator.effects[0].index = missionPlan.plans[0][i].index;
            plan.push_back(startSubMissionOperator);
            *added = true;
        }

        plan.push_back(missionPlan.plans[0][i]);
    }

    missionPlan.plans[0] = plan;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function deletes a specified state and its subsequent trigger.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::deleteState_(
    MissionMemoryManager::MissionPlan_t missionplan,
    OffendingState_t offendingState,
    bool *deleted)
{
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t thisOperator, lastOperator;
    CBRPlanner_PlanOperator_t offendingOperator;
    int i, j, k, index, lastIndex = -1;
    bool offendingOperatorFound = false, preconditionCopied = false;
    bool connected = false;
    const bool DEBUG = true;

    if ((int)(missionplan.plans.size()) == 0)
    {
        return missionplan;
    }

    if (offendingState.description == STRING_START_)
    {
        // Start state cannot be deleted.
        return missionplan;
    }

    // Currently, it only deletes the state in the first robot.

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("DELETE STATE ORIGINAL:\n");
        gDebugger->dumpPlanSequence(missionplan.plans[0]);
    }

    for (i = 0; i < (int)(missionplan.plans[0].size()); i++)
    {
        thisOperator = missionplan.plans[0][i];

        if (offendingState.description == thisOperator.description)
        {
            offendingOperator = thisOperator;
            offendingOperatorFound = true;
            continue;
        }

        if (offendingOperatorFound && (!preconditionCopied))
        {
            if (i == 0)
            {
                // Error.
                return missionplan;
            }
            else
            {
                // First, eliminated the triggers that connect between the
                // offending state and next state (thisOperator).
                for (j = 0; j < (int)(offendingOperator.effects.size()); j++)
                {
                    connected = false;
                    for (k = 0; k < (int)(thisOperator.preconditions.size()); k++)
                    {
                        if (offendingOperator.effects[j].name ==
                            thisOperator.preconditions[k].name)
                        {
                            connected = true;
                            index = k;
                            break;
                        }
                    }

                    if (connected)
                    {
                        // Delete the trigger (precondition).
                        thisOperator.preconditions[index] = 
                            thisOperator.preconditions.back();
                        thisOperator.preconditions.pop_back();
                    }
                }

                if (lastIndex >= 0)
                {
                    // Now, copy the triggers that were connected between the
                    // previous state and the offending state.
                    for (j = 0; j < (int)(lastOperator.effects.size()); j++)
                    {
                        connected = false;
                        for (k = 0; k < (int)(offendingOperator.preconditions.size()); k++)
                        {
                            if (lastOperator.effects[j].name ==
                                offendingOperator.preconditions[k].name)
                            {
                                plan[lastIndex].effects[j].index = thisOperator.index;
                                connected = true;
                                index = k;
                                break;
                            }
                        }

                        if (connected)
                        {
                            // Copy the trigger.
                            thisOperator.preconditions.push_back(
                                offendingOperator.preconditions[index]);

                            // Delete the entry from the array.
                            offendingOperator.preconditions[index] =
                                offendingOperator.preconditions.back();
                            offendingOperator.preconditions.pop_back();
                        }
                    }
                }
            }
            preconditionCopied = true;
            *deleted = true;
        }

        lastIndex = plan.size();
        plan.push_back(thisOperator);
        lastOperator = thisOperator;
    }

    missionplan.plans[0] = plan;

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("DELETE STATE RESULT:\n");
        gDebugger->dumpPlanSequence(missionplan.plans[0]);
    }

    return missionplan;
}

//-----------------------------------------------------------------------
// This function modifies the GoalLocation of the offending state to
// the specified coordinate.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::updateGoalLocation_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    OffendingState_t offendingState,
    MissionMemoryManager::Coordinates_t coordinates,
    bool *fixed)
{
    CBRPlanner_Parameter_t parameter;
    CBRPlanner_Effect_t effect;
    char coordBuf[1024];
    int i, j, k;

    *fixed = false;

    if ((int)(missionPlan.plans.size()) == 0)
    {
        return missionPlan;
    }

    sprintf(coordBuf, "{%.2f,+%.2f}", coordinates.x, coordinates.y);

    // Currently, it only updates for the first robot.
    for (i = 0; i < (int)(missionPlan.plans[0].size()); i++)
    {
        if (offendingState.description == missionPlan.plans[0][i].description)
        {
            for (j = 0; j < (int)(missionPlan.plans[0][i].parameters.size()); j++)
            {
                parameter = missionPlan.plans[0][i].parameters[j];

                if (parameter.name == STRING_GOAL_LOCATION_)
                {
                    missionPlan.plans[0][i].parameters[j].value = coordBuf;
                    *fixed = true;
                    break;
                }
            }

            for (k = 0; k < (int)(missionPlan.plans[0][i].effects.size()); k++)
            {
                effect = missionPlan.plans[0][i].effects[k];

                for (j = 0; j < (int)(effect.parameters.size()); j++)
                {
                    parameter = missionPlan.plans[0][i].effects[k].parameters[j];

                    if (parameter.name == STRING_GOAL_LOCATION_)
                    {
                        missionPlan.plans[0][i].effects[k].parameters[j].value = 
                            coordBuf;
                        break;
                    }
                }
            }
        }
    }

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function creates GoTo operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t RepairPlanSolution::createGoToOperator_(
    CBRPlanner_Preconditions_t preconditions,
    MissionMemoryManager::Coordinates_t coordinates,
    bool indoor)
{
    CBRPlanner_PlanOperator_t gotoOperator;
    CBRPlanner_Parameter_t parameter;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    char buf[1024];
    static int gotoOperatorIndex = 0;

    gotoOperator.name = STRING_GOTO_;
    sprintf(
        buf,
        "%s%d",
        STRING_GOTO_INDEX_.c_str(),
        gotoOperatorIndex++);
    gotoOperator.index = buf;

    parameter.name = STRING_GOAL_LOCATION_;
    sprintf(buf, "{%.2f,+%.2f}", coordinates.x, coordinates.y);
    parameter.value = buf;
    gotoOperator.parameters.push_back(parameter);

    parameter.name = STRING_MOVE_TO_LOCATION_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_MOVE_TO_LOCATION_GAIN_);
    parameter.value = buf;
    gotoOperator.parameters.push_back(parameter);

    parameter.name = STRING_WANDER_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_WANDER_GAIN_);
    parameter.value = buf;
    gotoOperator.parameters.push_back(parameter);

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
    gotoOperator.parameters.push_back(parameter);

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
    gotoOperator.parameters.push_back(parameter);

    parameter.name = STRING_AVOID_OBSTACLE_SAFETY_MARGIN_;
    sprintf(buf, "{%.2f}", DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_);
    parameter.value = buf;
    gotoOperator.parameters.push_back(parameter);

    if ((int)(preconditions.size()) > 0)
    {
        gotoOperator.preconditions = preconditions;
    }
    else
    {
        precondition.name = STRING_IMMEDIATE_;
        gotoOperator.preconditions.push_back(precondition);
    }

    effect.name = STRING_AT_GOAL_;
    parameter.name = STRING_GOAL_TOLERANCE_;
    sprintf(buf, "{%.2f}", DEFAULT_GOAL_TOLERANCE_);
    parameter.value = buf;
    effect.parameters.push_back(parameter);

    parameter.name = STRING_GOAL_LOCATION_;
    sprintf(buf, "{%.2f,+%.2f}", coordinates.x, coordinates.y);
    parameter.value = buf;
    effect.parameters.push_back(parameter);

    gotoOperator.effects.push_back(effect);

    return gotoOperator;
}

//-----------------------------------------------------------------------
//  This function creates StartSubMission operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t RepairPlanSolution::createStartSubMissionOperator_(
    string taskName,
    CBRPlanner_Preconditions_t preconditions,
    MissionMemoryManager::Coordinates_t coordinates,
    bool useMaps,
    bool indoor)
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

    if ((int)(preconditions.size()) > 0)
    {
        startSubMissionOperator.preconditions = preconditions;
    }
    else
    {
        precondition.name = STRING_IMMEDIATE_;
        startSubMissionOperator.preconditions.push_back(precondition);
    }

    effect.name = STRING_SUB_MISSION_READY_;
    parameter.name = STRING_SUB_MISSION_DEPLOYMENT_;
    if (useMaps)
    {
        parameter.value = STRING_DEPLOY_FIRST_;
    }
    else
    {
        parameter.value = STRING_EXECUTE_IMMEDIATELY_;
    }
    effect.parameters.push_back(parameter);

    parameter.name = STRING_GOAL_TOLERANCE_;
    sprintf(buf, "{%.2f}", DEFAULT_GOAL_TOLERANCE_);
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
//  This function adds AboutFace state before the sepcified state.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::addAboutFaceState_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    OffendingState_t offendingState,
    bool addToProceedAlongHallwayOnly,
    bool *added)
{
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t aboutFaceOperator;
    CBRPlanner_Precondition_t precondition;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Parameter_t parameter;
    const CBRPlanner_Preconditions_t EMPTY_PRECONDITIONS;
    int i, index;
    char buf[1024];

    *added = false;

    if ((int)(missionPlan.plans.size()) == 0)
    {
        return missionPlan;
    }

    // Currently, it only adds to the first robot.
    for (i = 0; i < (int)(missionPlan.plans[0].size()); i++)
    {
        plan.push_back(missionPlan.plans[0][i]);
        index = plan.size() - 1;

        if ((offendingState.description == missionPlan.plans[0][i].description) &&
            (!(*added)))
        {
            if ((addToProceedAlongHallwayOnly) &&
                (missionPlan.plans[0][i].name != STRING_PROCEED_ALONG_HALLWAY_))
            {
                return missionPlan;
            }

            if (missionPlan.plans[0][i].name == STRING_START_)
            {
                // It cannot be added to Start state. Add it to next one.
                plan.push_back(missionPlan.plans[0][i]);
                i++;

                if (i > (int)(missionPlan.plans[0].size()))
                {
                    return missionPlan;
                }
            }

            aboutFaceOperator = createAboutFaceOperator_();

            effect.name = STRING_AT_END_OF_HALL_;
            effect.index = aboutFaceOperator.index;
            parameter.name = STRING_GOAL_TOLERANCE_;
            sprintf(buf, "{%.2f}", DEFAULT_GOAL_TOLERANCE_);
            parameter.value = buf;
            effect.parameters.push_back(parameter);
            plan[index].effects.push_back(effect);

            aboutFaceOperator.effects[0].index = plan[index].index;
            plan.push_back(aboutFaceOperator);
            *added = true;
        }

    }

    missionPlan.plans[0] = plan;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function creates AboutFace operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t RepairPlanSolution::createAboutFaceOperator_(void)
{
    CBRPlanner_PlanOperator_t aboutFaceOperator;
    CBRPlanner_Parameter_t parameter;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    char buf[1024];
    static int aboutFaceOperatorIndex = 0;

    aboutFaceOperator.name = STRING_ABOUTFACE_;
    sprintf(
        buf,
        "%s%d",
        STRING_ABOUTFACE_INDEX_.c_str(),
        aboutFaceOperatorIndex++);
    aboutFaceOperator.index = buf;

    precondition.name = STRING_AT_END_OF_HALL_;
    aboutFaceOperator.preconditions.push_back(precondition);
 
    effect.name = STRING_ABOUTFACE_COMPLETED_;
    aboutFaceOperator.effects.push_back(effect);

    return aboutFaceOperator;
}

//-----------------------------------------------------------------------
// This function replace the offending state with StayWith[tagetObject].
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlanSolution::replaceWithStayWithState_(
    MissionMemoryManager::MissionPlan_t missionPlan,
    OffendingState_t offendingState,
    string targetObjectString,
    bool indoor,
    bool *replaced)
{
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_PlanOperator_t stayWithOperator;
    int i, numOperators;

    *replaced = false;

    if ((int)(missionPlan.plans.size()) == 0)
    {
        return missionPlan;
    }

    // Currently, it only updates for the first robot.
    for (i = 0; i < (int)(missionPlan.plans[0].size()); i++)
    {
        if ((offendingState.description == missionPlan.plans[0][i].description) &&
            (!(*replaced)))
        {
            if (missionPlan.plans[0][i].name == STRING_START_)
            {
                // Start state cannot be replaced.
                return missionPlan;
            }

            stayWithOperator = createStayWithOperator_(
                missionPlan.plans[0][i].preconditions,
                targetObjectString,
                indoor);

            numOperators = plan.size();
            if (numOperators > 0)
            {
                plan[numOperators-1].effects[0].index = stayWithOperator.index;
            }

            stayWithOperator.effects = missionPlan.plans[0][i].effects;

            plan.push_back(stayWithOperator);
            *replaced = true;
        }
        else
        {
            plan.push_back(missionPlan.plans[0][i]);
        }
    }

    missionPlan.plans[0] = plan;

    return missionPlan;
}

//-----------------------------------------------------------------------
//  This function creates StayWith operator.
//-----------------------------------------------------------------------
CBRPlanner_PlanOperator_t RepairPlanSolution::createStayWithOperator_(
    CBRPlanner_Preconditions_t preconditions,
    string targetObjectString,
    bool indoor)
{
    CBRPlanner_PlanOperator_t stayWithOperator;
    CBRPlanner_Parameter_t parameter;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    char buf[1024];
    static int stayWithOperatorIndex = 0;

    stayWithOperator.name = STRING_STAY_WITH_;
    sprintf(
        buf,
        "%s%d",
        STRING_STAY_WITH_INDEX_.c_str(),
        stayWithOperatorIndex++);
    stayWithOperator.index = buf;

    parameter.name = STRING_OBJECTS_;
    sprintf(buf, "{%s}", targetObjectString.c_str());
    parameter.value = buf;
    stayWithOperator.parameters.push_back(parameter);

    parameter.name = STRING_MOVE_TO_OBJECT_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_MOVE_TO_OBJECT_GAIN_);
    parameter.value = buf;
    stayWithOperator.parameters.push_back(parameter);

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
    stayWithOperator.parameters.push_back(parameter);

    parameter.name = STRING_WANDER_GAIN_;
    sprintf(buf, "{%.2f}", DEFAULT_WANDER_GAIN_);
    parameter.value = buf;
    stayWithOperator.parameters.push_back(parameter);

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
    stayWithOperator.parameters.push_back(parameter);

    parameter.name = STRING_AVOID_OBSTACLE_SAFETY_MARGIN_;
    sprintf(buf, "{%.2f}", DEFAULT_AVOID_OBSTACLE_SAFETY_MARGIN_);
    parameter.value = buf;
    stayWithOperator.parameters.push_back(parameter);

    stayWithOperator.preconditions = preconditions;

    return stayWithOperator;
}

//-----------------------------------------------------------------------
// This function gets a followup question for this solution.
//-----------------------------------------------------------------------
RepairQuestion_t RepairPlanSolution::getRepairFollowUpQuestion_(void)
{
    RepairQuestion_t repairQuestion;

    repairQuestion.type = NO_QUESTION;
    repairQuestion.status = NORMAL_QUESTION;

    if (currentRepairFollowUpQuestionID_ < (int)(repairFollowUpQuestions_.size()))
    {
        repairQuestion = repairFollowUpQuestions_[currentRepairFollowUpQuestionID_];
        currentRepairFollowUpQuestionID_++;
    }  

    return repairQuestion;
}

//-----------------------------------------------------------------------
// Reset parameters for the follow up questions.
//-----------------------------------------------------------------------
void RepairPlanSolution::resetFollowUpQuestions(void)
{
    currentRepairFollowUpQuestionID_ = 0;
}

//=======================================================================
// Constructor and destructor for RPS_EnterBuilding
//=======================================================================
RPS_EnterBuilding::RPS_EnterBuilding(void) : RepairPlanSolution ()
{
}

//-----------------------------------------------------------------------
// Destructor for RPS_EnterBuilding
//-----------------------------------------------------------------------
RPS_EnterBuilding::~RPS_EnterBuilding(void)
{
}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_EnterBuilding::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = addGoToState_(
        missionplan,
        offendingState_,
        targetPoint_,
        true, // i.e., indoor
        true, // about AboutFace
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_EnterRoom
//=======================================================================
RPS_EnterRoom::RPS_EnterRoom(void) : RepairPlanSolution () {}
RPS_EnterRoom::~RPS_EnterRoom(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_EnterRoom::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    /*
    missionplan = addGoToState_(
        missionplan,
        offendingState_,
        targetPoint_,
        true, // i.e., indoor
        true, // about AboutFace
        fixed);
    */

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_AddIndoorPoint
//=======================================================================
RPS_AddIndoorPoint::RPS_AddIndoorPoint(void) : RepairPlanSolution () {}
RPS_AddIndoorPoint::~RPS_AddIndoorPoint(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_AddIndoorPoint::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = addGoToState_(
        missionplan,
        offendingState_,
        targetPoint_,
        true, // i.e., indoor
        true, // about AboutFace
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_AddOutdoorPoint
//=======================================================================
RPS_AddOutdoorPoint::RPS_AddOutdoorPoint(void) : RepairPlanSolution () {}
RPS_AddOutdoorPoint::~RPS_AddOutdoorPoint(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_AddOutdoorPoint::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = addGoToState_(
        missionplan,
        offendingState_,
        targetPoint_,
        false, // i.e., outdoor
        true, // about AboutFace
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_MovePoint
//=======================================================================
RPS_MovePoint::RPS_MovePoint(void) : RepairPlanSolution () {}
RPS_MovePoint::~RPS_MovePoint(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_MovePoint::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = updateGoalLocation_(
        missionplan,
        offendingState_,
        targetPoint_,
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_DeletePoint
//=======================================================================
RPS_DeletePoint::RPS_DeletePoint(void) : RepairPlanSolution () {}
RPS_DeletePoint::~RPS_DeletePoint(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_DeletePoint::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = deleteState_(
        missionplan,
        offendingState_,
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_AddTurnAroundHallway
//=======================================================================
RPS_AddTurnAroundHallway::RPS_AddTurnAroundHallway(void) : RepairPlanSolution () {}
RPS_AddTurnAroundHallway::~RPS_AddTurnAroundHallway(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_AddTurnAroundHallway::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = addAboutFaceState_(
        missionplan,
        offendingState_,
        true, // Only if offending state is ProceedAlongHallway
        fixed);

    return missionplan;
}

//=======================================================================
// Constructor and destructor for RPS_ReplaceWithStayWithHostage
//=======================================================================
RPS_ReplaceWithStayWithHostage::RPS_ReplaceWithStayWithHostage(void) : RepairPlanSolution () {}
RPS_ReplaceWithStayWithHostage::~RPS_ReplaceWithStayWithHostage(void) {}

//-----------------------------------------------------------------------
// This function applies the solution.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RPS_ReplaceWithStayWithHostage::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    missionplan = replaceWithStayWithState_(
        missionplan,
        offendingState_,
        STRING_HOSTAGE_,
        true, // Indoor 
        fixed);

    return missionplan;
}

/**********************************************************************
 * $Log: repair_plan_solution.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/06/28 14:03:46  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 19:53:45  endo
 * Mods from usability-2004
 *
 **********************************************************************/
