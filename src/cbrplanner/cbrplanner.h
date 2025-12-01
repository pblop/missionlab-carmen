/**********************************************************************
 **                                                                  **
 **                             cbrplanner.h                         **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CBRPLANNER_H
#define CBRPLANNER_H

/* $Id: cbrplanner.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <time.h>

#include <string>
#include <vector>

#include "domain_manager.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "planner.h"
#include "repair_plan_types.h"
#include "repair_plan.h"

class CBRPlanner {

protected:
    CBRPlanner **self_;
    DomainManager *domainManager_;
    MissionMemoryManager *missionMemoryManager_;
    Planner *planner_;
    unsigned int startSubMissionOperatorIndex_;

    static const struct timespec MAINLOOP_SLEEPTIME_NSEC_;
    static const int MAINLOOP_USLEEPTIME_;

    vector<CBRPlanner_ACDLMissionPlan_t> getACDLMissionPlans_(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission,
        int cnpMode,
        vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    void resetStartSubMissionOperatorIndex_(void);

public:
    CBRPlanner(void);
    CBRPlanner(CBRPlanner **self);
    ~CBRPlanner(void);

    MissionMemoryManager *missionMemoryManager(void);
    CBRPlanner_ACDLMissionPlan_t getACDLMissionPlan(
        MemoryManager::Features_t features);
    CBRPlanner_ACDLMissionPlan_t repairMissionPlan(
        CBRPlanner_ACDLMissionPlan_t acdlPlan,
        bool *fixed);
    RepairQuestion_t getRepairQuestion(
        int lastSelectedOption);
    RepairQuestion_t getRepairFollowUpQuestion(void);
    CBRPlanner_CNPStatus_t getCurrentCNPStatus(void);
    vector<int> saveNewMissionPlan(
        CBRPlanner_ACDLMissionPlan_t missionplan,
        MemoryManager::Features_t features);
    vector<CBRPlanner_ACDLMissionPlan_t> getACDLMissionPlans(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission);
    vector<CBRPlanner_ACDLMissionPlan_t> getACDLMissionPlansPremissionCNP(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission,
        vector<MemoryManager::TaskConstraints_t> taskConstraintsList,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    vector<CBRPlanner_ACDLMissionPlan_t> getACDLMissionPlansRuntimeCNP(
        MemoryManager::Features_t features,
        int maxRating,
        bool addStartSubMission,
        vector<MemoryManager::RobotConstraints_t> robotConstraintsList);
    vector<CBRPlanner_RobotIDs_t> getCurrentRobotIDList(void);
    string getCBRLibraryName(void);
    string mergeMissions(vector<string> acdlMissions, int *numRobots);
    string reverseRobotOrder(string acdlMission);
    int toggleCNP(void);
    unsigned int getStartSubMissionOperatorIndex(bool increment);
    bool confirmClear(void);
    bool memoryChanged(void);
    void startSubsystems(void);
    void mainLoop(void);
    void updateFSANames(CBRPlanner_FSANames_t fsaNames);
    void updateOverlayInfo(CBRPlanner_OverlayInfo_t overlayInfo);
    void loadCBRLibrary(string cbrlibraryName);
    void loadCBRLibrary(void);
    void saveCBRLibrary(void);
    void saveCBRLibrary(string cbrlibraryName);
    void saveExtraDataList(
        vector<int> dataIndexList,
        vector<CBRPlanner_ExtraData_t> extraDataList);
    void givePositiveFeedback(vector<int> dataIndexList);
    void giveNegativeFeedback(vector<int> dataIndexList);
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
    void updateMissionUsageCounter(vector<int> dataIndexList);
    void updateOffendingState(OffendingState_t offendingState);
};

inline MissionMemoryManager *CBRPlanner::missionMemoryManager(void)
{
    return missionMemoryManager_;
}

inline RepairQuestion_t CBRPlanner::getRepairQuestion(
    int lastSelectedOption)
{
    return (planner_->getRepairQuestion(lastSelectedOption));
}

inline RepairQuestion_t CBRPlanner::getRepairFollowUpQuestion(void)
{
    return (planner_->getRepairFollowUpQuestion());
}

inline CBRPlanner_CNPStatus_t CBRPlanner::getCurrentCNPStatus(void)
{
    return (planner_->getCurrentCNPStatus());
}

inline vector<CBRPlanner_RobotIDs_t> CBRPlanner::getCurrentRobotIDList(void)
{
    return (planner_->getCurrentRobotIDList());
}

inline void CBRPlanner::loadCBRLibrary(string cbrlibraryName)
{
    missionMemoryManager_->loadCBRLibrary(cbrlibraryName);
}

inline void CBRPlanner::loadCBRLibrary(void)
{
    missionMemoryManager_->loadCBRLibrary();
}

inline void CBRPlanner::saveCBRLibrary(void)
{
    missionMemoryManager_->saveCBRLibrary();
}

inline bool CBRPlanner::confirmClear(void)
{
    return (missionMemoryManager_->confirmClear());
}

inline bool CBRPlanner::memoryChanged(void)
{
    return (missionMemoryManager_->memoryChanged());
}

inline void CBRPlanner::givePositiveFeedback(vector<int> dataIndexList)
{
    missionMemoryManager_->givePositiveFeedback(dataIndexList);
}

inline void CBRPlanner::giveNegativeFeedback(vector<int> dataIndexList)
{
    missionMemoryManager_->giveNegativeFeedback(dataIndexList);
}

inline void CBRPlanner::saveExtraDataList(
    vector<int> dataIndexList,
    vector<CBRPlanner_ExtraData_t> extraDataList)
{
    missionMemoryManager_->saveExtraDataList(dataIndexList, extraDataList);
}

inline void CBRPlanner::updateMissionUsageCounter(vector<int> dataIndexList)
{
    missionMemoryManager_->updateMissionUsageCounter(dataIndexList);
}

inline void CBRPlanner::putRepairFollowUpQuestionToggleAnswer(int selectedOption)
{
    planner_->putRepairFollowUpQuestionToggleAnswer(selectedOption);
}

inline void CBRPlanner::putRepairFollowUpQuestionAddMapPointAnswer(double x, double y)
{
    planner_->putRepairFollowUpQuestionAddMapPointAnswer(x, y);
}

inline void CBRPlanner::putRepairFollowUpQuestionModMapPointAnswer(
    double oldX,
    double oldY,
    double newX,
    double newY,
    double closeRange)
{
    planner_->putRepairFollowUpQuestionModMapPointAnswer(
        oldX, oldY, newX, newY, closeRange);
}

inline void CBRPlanner::putRepairFollowUpQuestionDelMapPointAnswer(
    double x,
    double y,
    double closeRange)
{
    planner_->putRepairFollowUpQuestionDelMapPointAnswer(x, y, closeRange);
}

inline void CBRPlanner::updateOffendingState(OffendingState_t offendingState)
{
    planner_->updateOffendingState(offendingState);
}

inline void CBRPlanner::resetStartSubMissionOperatorIndex_(void)
{
    startSubMissionOperatorIndex_ = 0;
}

inline int CBRPlanner::toggleCNP(void)
{
    return (planner_->toggleCNP());
}

inline string CBRPlanner::getCBRLibraryName(void)
{
    return (missionMemoryManager_->getCBRLibraryName());
}

inline void CBRPlanner::saveCBRLibrary(string cbrlibraryName)
{
    missionMemoryManager_->saveCBRLibrary(cbrlibraryName);
}

extern CBRPlanner *gCBRPlanner;

#endif
/**********************************************************************
 * $Log: cbrplanner.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.6  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.5  2007/08/10 15:15:39  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.4  2007/08/06 22:08:47  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.3  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
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
 * Revision 1.9  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.8  2006/02/19 17:51:59  endo
 * Experiment related modifications
 *
 * Revision 1.7  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.6  2005/08/12 21:48:29  endo
 * More improvements for August demo.
 *
 * Revision 1.5  2005/07/31 03:41:39  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.4  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.3  2005/06/23 22:09:04  endo
 * Adding the mechanism to talk to CNP.
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
