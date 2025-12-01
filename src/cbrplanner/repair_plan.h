/**********************************************************************
 **                                                                  **
 **                         repair_plan.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_plan.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef REPAIR_PLAN_H
#define REPAIR_PLAN_H

#include <vector>
#include <string>

#include "windows.h"
#include "repair_plan_types.h"
#include "repair_plan_solution.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "repair_memory_manager.h"

using std::vector;
using std::string;

class RepairPlanSolution;

class RepairPlan {

protected:
    typedef struct ProblemOption_t {
        string value;
    };

    typedef struct Problem_t {
        int id;
        string name;
        string description;
        int type;
        vector<ProblemOption_t> options;
    };

    typedef struct RepairRule_t {
        vector<Problem_t> problems;
        int solutionNumber;
    };

    typedef struct Solution_t {
        int number;
        RepairPlanSolution *solution;
    };

protected:
    RepairPlan **self_;
    RepairMemoryManager *repairMemoryManager_;
    RepairPlanSolution *currentSolution_;
    OffendingState_t offendingState_;
    MissionMemoryManager::Coordinates_t offendingStateCoordinate_;
    vector<Problem_t> problemList_;
    vector<RepairRule_t> repairRuleList_;
    vector<ProblemOption_t> lastOptions_;
    vector<Solution_t *> solutionList_;
    double coordinateCloseRange_;

    static const string RC_FILENAME_;
    static const string RC_STRING_PROBLEM_KEY_;
    static const string RC_STRING_PROBLEM_BEGIN_;
    static const string RC_STRING_PROBLEM_END_;
    static const string RC_STRING_PROBLEM_ID_;
    static const string RC_STRING_PROBLEM_NAME_;
    static const string RC_STRING_PROBLEM_DESC_;
    static const string RC_STRING_PROBLEM_TYPE_;
    static const string RC_STRING_PROBLEM_OPTION_;
    static const string RC_STRING_REPAIR_KEY_;
    static const string RC_STRING_REPAIR_BEGIN_;
    static const string RC_STRING_REPAIR_END_;
    static const string RC_STRING_REPAIR_PROBLEMS_;
    static const string RC_STRING_REPAIR_SOLUTION_NUMBER_;
    static const string EMPTY_STRING_;
    static const string STRING_GOTO_;
    static const string STRING_GOAL_LOCATION_;
    static const string STRING_START_SUB_MISSION_;
    static const string STRING_SUB_MISSION_NAME_;
    static const string STRING_PARM_WAYPOINTS_TASK_;
    static const int SOLUTION_NUMBER_NONE_;
    static const double DEFAULT_FEATURE_WEIGHT_VALUE_;
    static const double DEFAULT_COORDINATE_CLOSE_RANGE_;
    static const bool DEFAULT_FEATURE_NONINDEX_VALUE_;

    Problem_t getProblemFromID_(int id);
    RepairPlanSolution *getSolutionFromNumber_(int number);
    OffendingState_t searchOffendingWaypointState_(
        MissionMemoryManager::MissionPlan_t missionplan,
        MissionMemoryManager::Coordinates_t offendingStateCoordinate);
    MissionMemoryManager::Coordinates_t extractCoordinate_(string value);
    vector<Problem_t> nums2problems_(string numsBuf);
    string extractValueContent_(string value, bool withoutQuote);
    bool loadRCFile_(void);
    bool fscanfQuotedWords_(FILE *stream, char *buf);
    bool fscanfBracketedWords_(FILE *stream, char *buf);
    bool coordinateCloseEnough_(
        MissionMemoryManager::Coordinates_t pointA,
        MissionMemoryManager::Coordinates_t pointB);
    void dumpProblemList_(void);
    void dumpRepairRuleList_(void);
    void saveRepairRule_(void);
    void prepareSolutions_(void);
    void updateCoordinateCloseRange_(double range);

public:
    RepairPlan(void);
    RepairPlan(RepairPlan **self);
    ~RepairPlan(void);
    RepairQuestion_t getRepairQuestion(int lastSelectedOption);
    RepairQuestion_t getRepairFollowUpQuestion(void);
    MissionMemoryManager::MissionPlan_t repairMissionPlan(
        MissionMemoryManager::MissionPlan_t missionplan,
        bool *fixed);
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

inline void RepairPlan::updateOffendingState(OffendingState_t offendingState)
{
    offendingState_ = offendingState;
}

inline void RepairPlan::updateCoordinateCloseRange_(double range)
{
    coordinateCloseRange_ = range;
}

inline bool RepairPlan::coordinateCloseEnough_(
        MissionMemoryManager::Coordinates_t pointA,
        MissionMemoryManager::Coordinates_t pointB)
{
    if (sqrt(pow((pointA.x - pointB.x),2) + pow((pointA.y - pointB.y),2)) >
        coordinateCloseRange_)
    {
        return false;
    }

    return true;
}

#endif

/**********************************************************************
 * $Log: repair_plan.h,v $
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
