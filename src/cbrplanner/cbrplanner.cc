/**********************************************************************
 **                                                                  **
 **                            cbrplanner.cc                         **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module instantiates DomainManager, MemoryManager, and      **
 **  Planner (sub-)modules within its class to server as a CBR       **
 **  Planner.                                                        **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cbrplanner.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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

#include "cbrplanner.h"
#include "cbrplanner_domain.h"
#include "domain_manager.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "planner.h"
#include "windows.h"
#include "cnp_types.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const struct timespec CBRPlanner::MAINLOOP_SLEEPTIME_NSEC_ = {0, 10000000}; // 10 msec.
const int CBRPlanner::MAINLOOP_USLEEPTIME_ = 10000; // 10 msec.

//-----------------------------------------------------------------------
// Constructor for CBRPlanner class.
//-----------------------------------------------------------------------
CBRPlanner::CBRPlanner(CBRPlanner **cbrplannerInstance) : self_(cbrplannerInstance)
{
    if (cbrplannerInstance != NULL)
    {
        *cbrplannerInstance = this;
    }

    domainManager_ = NULL;
    missionMemoryManager_ = NULL;
    planner_ = NULL;
    startSubMissionOperatorIndex_ = 0;
}

//-----------------------------------------------------------------------
// Distructor for CBRPlanner class.
//-----------------------------------------------------------------------
CBRPlanner::~CBRPlanner(void)
{
    if (planner_ != NULL)
    {
        delete planner_;
    }

    if (missionMemoryManager_ != NULL)
    {
        delete missionMemoryManager_;
    }

    if (domainManager_ != NULL)
    {
        delete domainManager_;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//---------------------------------------------------------------------
// This function makes the program to just loop until terminated by
// the user.
//---------------------------------------------------------------------
void CBRPlanner::mainLoop(void)
{
    while (true)
    {
        //usleep(MAINLOOP_USLEEPTIME_);
        nanosleep(&MAINLOOP_SLEEPTIME_NSEC_, NULL);
    }
}

//---------------------------------------------------------------------
// This function starts all the subsystems within the CBR Planner.
//---------------------------------------------------------------------
void CBRPlanner::startSubsystems(void)
{
    new DomainManager(&domainManager_);
    new MissionMemoryManager(&missionMemoryManager_);
    new Planner(&planner_);
}

//-----------------------------------------------------------------------
// This function will save a new mission plan into its memory.
//-----------------------------------------------------------------------
vector<int> CBRPlanner::saveNewMissionPlan(
    CBRPlanner_ACDLMissionPlan_t acdlPlan,
    MemoryManager::Features_t features)
{
    MissionMemoryManager::MissionPlan_t missionplan;
    vector<int> dataIndexList;
    bool hasStartSubMission;

    // Convert the ACDL represented mision plan into a format that
    // MemoryManager can understand.
    missionplan = domainManager_->acdl2missionplan(acdlPlan);
    hasStartSubMission = domainManager_->hasStartSubMision(acdlPlan);

    // Save the plan into the memory along with the feature.
    dataIndexList = planner_->saveNewMissionPlan(
        missionplan,
        features,
        hasStartSubMission);

    return dataIndexList;
}

//-----------------------------------------------------------------------
// This function updates the FSA list of domain manager.
//-----------------------------------------------------------------------
void CBRPlanner::updateFSANames(CBRPlanner_FSANames_t fsaNames)
{
    domainManager_->updateFSANames(fsaNames);
}

//-----------------------------------------------------------------------
// This function will get a mission plan from Planner, and convert it in
// ACDL (CNP diabled).
//-----------------------------------------------------------------------
vector<CBRPlanner_ACDLMissionPlan_t> CBRPlanner::getACDLMissionPlans(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission)
{
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans;
    vector<MemoryManager::TaskConstraints_t> emptyTaskConstraintsList;
    vector<MemoryManager::RobotConstraints_t> emptyRobotConstraintsList;

    acdlPlans = getACDLMissionPlans_(
        features,
        maxRating,
        addStartSubMission,
        CNP_MODE_DISABLED,
        emptyTaskConstraintsList,
        emptyRobotConstraintsList);

    return acdlPlans;
}

//-----------------------------------------------------------------------
// This function will get a mission plan from Planner, and convert it in
// ACDL via premission-CNP.
//-----------------------------------------------------------------------
vector<CBRPlanner_ACDLMissionPlan_t> CBRPlanner::getACDLMissionPlansPremissionCNP(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission,
    vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans;

    acdlPlans = getACDLMissionPlans_(
        features,
        maxRating,
        addStartSubMission,
        CNP_MODE_PREMISSION,
        taskConstraintsList,
        robotConstraintsList);

    return acdlPlans;
}

//-----------------------------------------------------------------------
// This function will get a mission plan from Planner, and convert it in
// ACDL for runtime-CNP.
//-----------------------------------------------------------------------
vector<CBRPlanner_ACDLMissionPlan_t> CBRPlanner::getACDLMissionPlansRuntimeCNP(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans;
    vector<MemoryManager::TaskConstraints_t> emptyTaskConstraintsList;

    acdlPlans = getACDLMissionPlans_(
        features,
        maxRating,
        addStartSubMission,
        CNP_MODE_RUNTIME,
        emptyTaskConstraintsList,
        robotConstraintsList);

    return acdlPlans;
}

//-----------------------------------------------------------------------
// This function will get a mission plan from Planner, and convert it in
// ACDL.
//-----------------------------------------------------------------------
vector<CBRPlanner_ACDLMissionPlan_t> CBRPlanner::getACDLMissionPlans_(
    MemoryManager::Features_t features,
    int maxRating,
    bool addStartSubMission,
    int cnpMode,
    vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList)
{
    CBRPlanner_ACDLMissionPlan_t acdlPlan;
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans;
    vector<MissionMemoryManager::MissionPlan_t> missionplans;
    int i;

    resetStartSubMissionOperatorIndex_();

    switch (cnpMode) {

    case CNP_MODE_DISABLED:
        // Get a mision plan from the Planner module, based on the feature.
        missionplans = planner_->getMissionPlansCBROnly(
            features,
            maxRating,
            addStartSubMission);
        break;

    case CNP_MODE_PREMISSION:
        // Get a mision plan from the Planner module, based on the feature
        // and CNP constraints
        missionplans = planner_->getMissionPlansPremissionCNP(
            features,
            maxRating,
            addStartSubMission,
            taskConstraintsList,
            robotConstraintsList);
        break;

    case CNP_MODE_RUNTIME:
        // Get a mision plan from the Planner module, based on the feature.
        missionplans = planner_->getMissionPlansRuntimeCNP(
            features,
            maxRating,
            addStartSubMission,
            robotConstraintsList);
        break;

    case CNP_MODE_PREMISSION_AND_RUNTIME:
    default:
        gWindows->printfPlannerWindow(
            "Warning: Unsupported CNP mode = %d.\n",
            cnpMode);

        // Use CBR only.
        missionplans = planner_->getMissionPlansCBROnly(
            features,
            maxRating,
            addStartSubMission);
        break;
    }

    for (i = 0; i < ((int)(missionplans.size())); i++)
    {
        // Convert the mission plan into an ACDL mission plan.
        acdlPlan = domainManager_->missionplan2acdl(missionplans[i]);
        acdlPlans.push_back(acdlPlan);
    }

    return acdlPlans;
}

//-----------------------------------------------------------------------
// This function will send the mission plan to Planner in order to fix
// it. Once it is fixed, this function will send it back to Communicator.
//-----------------------------------------------------------------------
CBRPlanner_ACDLMissionPlan_t CBRPlanner::repairMissionPlan(
    CBRPlanner_ACDLMissionPlan_t acdlPlan,
    bool *fixed)
{
    MissionMemoryManager::MissionPlan_t missionplan;

    // Convert the ACDL represented mision plan into a format that
    // Planner can understand.
    missionplan = domainManager_->acdl2missionplan(acdlPlan);

    // Repair it.
    missionplan = planner_->repairMissionPlan(missionplan, fixed);

    if (*fixed)
    {
        // Convert back to the ACDL representation.
        acdlPlan = domainManager_->missionplan2acdl(missionplan);
    }

    return acdlPlan;
}

//-----------------------------------------------------------------------
// This function returns the current subMissioOperatorIndex_ value.
// If specified, it increase its value.
//-----------------------------------------------------------------------
unsigned int CBRPlanner::getStartSubMissionOperatorIndex(bool increment)
{
    unsigned int value;

    value = startSubMissionOperatorIndex_;

    if (increment)
    {
        startSubMissionOperatorIndex_++;
    }

    return value;
}

//-----------------------------------------------------------------------
// This function reverses the order of the robots in the mission
//-----------------------------------------------------------------------
string CBRPlanner::reverseRobotOrder(string acdlMission)
{
    CBRPlanner_ACDLMissionPlan_t acdlPlan;
    MissionMemoryManager::MissionPlan_t missionplan;
    string newACDLMission;
    bool fixed = false;

    // Convert to a mission plan.
    acdlPlan.dataSize = acdlMission.size();
    acdlPlan.data = new char[acdlPlan.dataSize+1];
    memcpy(acdlPlan.data, acdlMission.c_str(), acdlPlan.dataSize);
    acdlPlan.data[acdlPlan.dataSize] = '\0';
    missionplan = domainManager_->acdl2missionplan(acdlPlan);
    delete [] acdlPlan.data;
    acdlPlan.data = NULL;

    missionplan = planner_->reverseRobotOrder(missionplan, &fixed);

    if (!fixed)
    {
        gWindows->printfPlannerWindow(
            "Error: CBRPlanner::reverseRobotOrder(). Failed to reverse the robot order.\n");
        return acdlMission;
    }

    acdlPlan = domainManager_->missionplan2acdl(missionplan);

    if (acdlPlan.data == NULL)
    {
        gWindows->printfPlannerWindow(
            "Error: CBRPlanner::reverseRobotOrder(). Failed to convert the mission to ACDL.\n");
        return acdlMission;
    }

    newACDLMission = acdlPlan.data;

    return newACDLMission;
}


//-----------------------------------------------------------------------
// This function merges multiple missions into one.
//-----------------------------------------------------------------------
string CBRPlanner::mergeMissions(vector<string> acdlMissions, int *numRobots)
{
    CBRPlanner_ACDLMissionPlan_t acdlPlan;
    MissionMemoryManager::MissionPlan_t missionplan;
    vector<MissionMemoryManager::MissionPlan_t> missionplans;
    string acdlMission, newACDLMission;
    int i;
    bool fixed = false;

    for (i = 0; i < (int)(acdlMissions.size()); i++)
    {
        // Convert to a mission plan.
        acdlMission = acdlMissions[i];
        acdlPlan.dataSize = acdlMission.size();
        acdlPlan.data = new char[acdlPlan.dataSize+1];
        memcpy(acdlPlan.data, acdlMission.c_str(), acdlPlan.dataSize);
        acdlPlan.data[acdlPlan.dataSize] = '\0';
        missionplan = domainManager_->acdl2missionplan(acdlPlan);
        missionplans.push_back(missionplan);
        delete [] acdlPlan.data;
        acdlPlan.data = NULL;
    }

    missionplan = planner_->mergeMissions(missionplans, numRobots, &fixed);

    if (!fixed)
    {
        gWindows->printfPlannerWindow(
            "Error: CBRPlanner::mergeMissions(). Failed to merge the missions.\n");
        return acdlMission;
    }

    acdlPlan = domainManager_->missionplan2acdl(missionplan);

    if (acdlPlan.data == NULL)
    {
        gWindows->printfPlannerWindow(
            "Error: CBRPlanner::mergeMissions(). Failed to convert the mission to ACDL.\n");
        return acdlMission;
    }

    newACDLMission = acdlPlan.data;

    return newACDLMission;
}


/**********************************************************************
 * $Log: cbrplanner.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
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
 * Revision 1.6  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.5  2006/02/19 17:51:59  endo
 * Experiment related modifications
 *
 * Revision 1.4  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.3  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.2  2005/02/07 19:53:43  endo
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
