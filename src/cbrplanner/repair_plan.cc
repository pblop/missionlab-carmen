/**********************************************************************
 **                                                                  **
 **                          repair_plan.cc                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_plan.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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

#include "repair_plan.h"
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
// RC file name.
const string RepairPlan::RC_FILENAME_ = ".repair_planrc";

// RC file vocabulary
const string RepairPlan::RC_STRING_PROBLEM_KEY_ = "<PROBLEM";
const string RepairPlan::RC_STRING_PROBLEM_BEGIN_ = "BEGIN>";
const string RepairPlan::RC_STRING_PROBLEM_END_ = "END>";
const string RepairPlan::RC_STRING_PROBLEM_ID_ = "PROBLEM-ID";
const string RepairPlan::RC_STRING_PROBLEM_NAME_ = "PROBLEM-NAME";
const string RepairPlan::RC_STRING_PROBLEM_DESC_ = "PROBLEM-DESC";
const string RepairPlan::RC_STRING_PROBLEM_TYPE_ = "PROBLEM-TYPE";
const string RepairPlan::RC_STRING_PROBLEM_OPTION_ = "PROBLEM-OPTION";
const string RepairPlan::RC_STRING_REPAIR_KEY_ = "<REPAIR";
const string RepairPlan::RC_STRING_REPAIR_BEGIN_ = "BEGIN>";
const string RepairPlan::RC_STRING_REPAIR_END_ = "END>";
const string RepairPlan::RC_STRING_REPAIR_PROBLEMS_ = "REPAIR-PROBLEMS";
const string RepairPlan::RC_STRING_REPAIR_SOLUTION_NUMBER_ = "REPAIR-SOLUTION-NUMBER";

// Other strings.
const string RepairPlan::EMPTY_STRING_ = "";
const string RepairPlan::STRING_GOTO_ = "GoTo";
const string RepairPlan::STRING_GOAL_LOCATION_ = "%Goal_Location";
const string RepairPlan::STRING_START_SUB_MISSION_ = "StartSubMission";
const string RepairPlan::STRING_SUB_MISSION_NAME_ = "%sub_mission_name";
const string RepairPlan::STRING_PARM_WAYPOINTS_TASK_ = "{\"WaypointsTask\"}";

// Others
const int RepairPlan::SOLUTION_NUMBER_NONE_ = -1;
const double RepairPlan::DEFAULT_FEATURE_WEIGHT_VALUE_ = 0.9;
const double RepairPlan::DEFAULT_COORDINATE_CLOSE_RANGE_ = 1.0;
const bool RepairPlan::DEFAULT_FEATURE_NONINDEX_VALUE_ = false;
//-----------------------------------------------------------------------
// Constructor for RepairPlan
//-----------------------------------------------------------------------
RepairPlan::RepairPlan(RepairPlan **repairPlanInstance) : self_(repairPlanInstance)
{
    if (repairPlanInstance != NULL)
    {
        *repairPlanInstance = this;
    }

    coordinateCloseRange_ = DEFAULT_COORDINATE_CLOSE_RANGE_;

    new RepairMemoryManager(&repairMemoryManager_);

    prepareSolutions_();

    if (loadRCFile_())
    {
        saveRepairRule_();
    }
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
RepairPlan::~RepairPlan(void)
{
    if (repairMemoryManager_ != NULL)
    {
        delete repairMemoryManager_;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function loads the problems from the RC file.
//-----------------------------------------------------------------------
bool RepairPlan::loadRCFile_(void)
{
    Problem_t problem;
    ProblemOption_t option;
    RepairRule_t repairRule;
    const Problem_t EMPTY_PROBLEM = {0, string(), string(), 0, vector<ProblemOption_t>()};
    const ProblemOption_t EMPTY_PROBLEM_OPTION = {string()};
    const RepairRule_t EMPTY_REPAIR_RULE = {vector<Problem_t>(), 0};
    FILE *rcFile = NULL;
    string rcFilename;
    string bufString1, bufString2;
    char buf1[1024], buf2[1024];
    const bool DEBUG = true;

    // First, find the RC file.
    rcFilename = find_file_in_envpath(RC_FILENAME_.c_str());

    if (rcFilename == EMPTY_STRING_)
    {
        gWindows->printfPlannerWindow(
            "Error: RepairPlan RC file \"%s\" could not be found.\n",
            RC_FILENAME_.c_str());
        return false;
    }

    // Load the RC file.
    rcFile = fopen(rcFilename.c_str(), "r");

    if (rcFile == NULL)
    {
        gWindows->printfPlannerWindow(
            "Error: RC file \"%s\" could not be opened.\n",
            RC_FILENAME_.c_str());
        return false;
    }

    // Clear the existing list.
    problemList_.clear();

    while (fscanf(rcFile, "%s", buf1) != EOF)
    {
        bufString1 = buf1;

        if (bufString1[0] == '#')
        {
            // It's a comment.
            while ((fgetc(rcFile) != '\n') && (!feof(rcFile)))
            {
                // Keep reading until next line.
            }
        }
        else if (bufString1 == RC_STRING_PROBLEM_KEY_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            bufString2 = buf2;

            if (bufString2 == RC_STRING_PROBLEM_BEGIN_)
            {
                problem = EMPTY_PROBLEM;
            }
            else if (bufString2 == RC_STRING_PROBLEM_END_)
            {
                problemList_.push_back(problem);
            }
        }
        else if (bufString1 == RC_STRING_PROBLEM_ID_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            problem.id = atoi(buf2);
        }
        else if (bufString1 == RC_STRING_PROBLEM_NAME_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            problem.name = buf2;
        }
        else if (bufString1 == RC_STRING_PROBLEM_DESC_)
        {
            if (fscanfQuotedWords_(rcFile, (char *)&buf2) == false)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            problem.description = buf2;
        }
        else if (bufString1 == RC_STRING_PROBLEM_TYPE_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            problem.type = atoi(buf2);
        }
        else if (bufString1 == RC_STRING_PROBLEM_OPTION_)
        {
            option = EMPTY_PROBLEM_OPTION;

            if (fscanfQuotedWords_(rcFile, (char *)&buf2) == false)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            option.value = buf2;
            problem.options.push_back(option);
        }
        else if (bufString1 == RC_STRING_REPAIR_KEY_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            bufString2 = buf2;

            if (bufString2 == RC_STRING_REPAIR_BEGIN_)
            {
                repairRule = EMPTY_REPAIR_RULE;
            }
            else if (bufString2 == RC_STRING_REPAIR_END_)
            {
                repairRuleList_.push_back(repairRule);
            }
        }
        else if (bufString1 == RC_STRING_REPAIR_PROBLEMS_)
        {
            if (fscanfBracketedWords_(rcFile, (char *)&buf2) == false)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            bufString2 = buf2;
            repairRule.problems = nums2problems_(bufString2);
        }
        else if (bufString1 == RC_STRING_REPAIR_SOLUTION_NUMBER_)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                gWindows->printfPlannerWindow(
                    "Warning: RC file \"%s\" corrupted after \"%s\".\n",
                    RC_FILENAME_.c_str(),
                    buf1);
                return false;
            }
            
            repairRule.solutionNumber = atoi(buf2);
        }
    }

    if (DEBUG)
    {
        dumpProblemList_();
        dumpRepairRuleList_();
    }

    return true;
}

//-----------------------------------------------------------------------
// This function reads words that comes with "". It expects the first
// and last chars to be '"'.
//-----------------------------------------------------------------------
bool RepairPlan::fscanfQuotedWords_(FILE *stream, char *buf)
{
    char localBuf[1024];
    char c;
    string localBufString;
    int localBufSize, index;

    if (fscanf(stream, "%s", localBuf) == EOF)
    {
        return false;
    }

    // Make sure the first char is ".
    if (localBuf[0] != '"')
    {
        return false;
    }

    localBufString = localBuf;
    localBufSize = localBufString.size();

    if (localBufString[localBufSize-1] == '"')
    {
        // The last " is already found.
        memcpy(buf, localBuf+1, localBufSize-2);
        buf[localBufSize-2] = '\0';
        return true;
    }

    memcpy(buf, localBuf+1, localBufSize-1);
    index = localBufSize-1;

    while (true)
    {
        c = fgetc(stream);

        switch (c) {

        case EOF:
        case '\n':
            return false;

        case '"':
            buf[index] = '\0';
            return true;

        default:
            buf[index] = c;
            index++;
            break;
        }
    }

    return false;
}

//-----------------------------------------------------------------------
// This function reads words that comes with []. It expects the first
// and last chars to be '['.
//-----------------------------------------------------------------------
bool RepairPlan::fscanfBracketedWords_(FILE *stream, char *buf)
{
    char localBuf[1024];
    char c;
    string localBufString;
    int localBufSize, index;

    if (fscanf(stream, "%s", localBuf) == EOF)
    {
        return false;
    }

    // Make sure the first char is ".
    if (localBuf[0] != '[')
    {
        return false;
    }

    localBufString = localBuf;
    localBufSize = localBufString.size();

    if (localBufString[localBufSize-1] == ']')
    {
        // The last " is already found.
        memcpy(buf, localBuf+1, localBufSize-2);
        buf[localBufSize-2] = '\0';
        return true;
    }

    memcpy(buf, localBuf+1, localBufSize-1);
    index = localBufSize-1;

    while (true)
    {
        c = fgetc(stream);

        switch (c) {

        case EOF:
        case '\n':
            return false;

        case ']':
            buf[index] = '\0';
            return true;

        default:
            buf[index] = c;
            index++;
            break;
        }
    }

    return false;
}

//-----------------------------------------------------------------------
// This function converts the list of the number [* *:* ...] to a
// set of problems.
//-----------------------------------------------------------------------
vector<RepairPlan::Problem_t> RepairPlan::nums2problems_(string numsBuf)
{
    Problem_t problem;
    vector<Problem_t> problems;
    vector<string> chunks;
    vector<ProblemOption_t> options;
    string localBuf;
    string::size_type index;
    int i, bufSize, problemID, optionID;

    // First, break the string into chunks of * or *:*.
    while (true)
    {
        bufSize = numsBuf.size();

        if (bufSize == 0)
        {
            // It's an empty string.
            break;
        }

        index = numsBuf.find(' ');

        if (index == string::npos)
        {
            // It reached the end before hitting a space.
            chunks.push_back(numsBuf);
            break;
        }

        // Found the space. Get the chunk before the space.
        localBuf = numsBuf.substr(0, index);

        if ((int)(localBuf.size()) > 0)
        {
            // Add this chunk.
            chunks.push_back(localBuf);
        }
        else
        {
            // Sequences of spaces. Do nothing.
        }

        numsBuf = numsBuf.substr(index+1, bufSize-index-1);
    }

    if ((int)(chunks.size()) == 0)
    {
        // It's empty.
        return problems;
    }

    for (i = 0; i < (int)(chunks.size()); i++)
    {
        bufSize = chunks[i].size();
        optionID = -1;
        options.clear();

        // Look for to see if option number is specified.
        index = chunks[i].find(':');

        if (index == string::npos)
        {
            // It reached the end before hitting ':'.
            problemID = atoi(chunks[i].c_str());
        }
        else
        {
            // Seprate the problem ID and the option ID.
            localBuf = chunks[i].substr(0, index);
            problemID = atoi(localBuf.c_str());
            localBuf = chunks[i].substr(index+1, bufSize-index-1);
            optionID = atoi(localBuf.c_str());
        }
        
        problem = getProblemFromID_(problemID);
        options = problem.options;
        problem.options.clear();


        if ((problem.type == TOGGLE) && 
            (optionID >= 0) && 
            (optionID < (int)(options.size())))
        {
            problem.options.push_back(options[optionID]);
        }

        problems.push_back(problem);
    }

    return problems;
}

//-----------------------------------------------------------------------
// This function finds the stored problem from the ID.
//-----------------------------------------------------------------------
RepairPlan::Problem_t RepairPlan::getProblemFromID_(int id)
{
    const Problem_t EMPTY_PROBLEM = {0, string(), string(), 0, vector<ProblemOption_t>()};
    int i;

    for (i = 0; i < (int)(problemList_.size()); i++)
    {
        if (problemList_[i].id == id)
        {
            return problemList_[i];
            
        }
    }

    return EMPTY_PROBLEM;
}

//-----------------------------------------------------------------------
// This function prints out the problems into the debug file.
//-----------------------------------------------------------------------
void RepairPlan::dumpProblemList_(void)
{
    int i, j;

    gDebugger->printfLine();
    gDebugger->printfDebug("PROBLEM LIST:\n");
    gDebugger->printfDebug("\n");

    for (i = 0; i < (int)(problemList_.size()); i++)
    {
        gDebugger->printfDebug(
            "%s %d\n",
            RC_STRING_PROBLEM_ID_.c_str(),
            problemList_[i].id);

        gDebugger->printfDebug(
            "%s %s\n",
            RC_STRING_PROBLEM_NAME_.c_str(),
            problemList_[i].name.c_str());

        gDebugger->printfDebug(
            "%s \"%s\"\n",
            RC_STRING_PROBLEM_DESC_.c_str(),
            problemList_[i].description.c_str());

        gDebugger->printfDebug(
            "%s %d\n",
            RC_STRING_PROBLEM_TYPE_.c_str(),
            problemList_[i].type);

        for (j = 0; j < (int)(problemList_[i].options.size()); j++)
        {
            gDebugger->printfDebug(
                "%s \"%s\"\n",
                RC_STRING_PROBLEM_OPTION_.c_str(),
                problemList_[i].options[j].value.c_str());
        }

        gDebugger->printfDebug("\n");
    }
}

//-----------------------------------------------------------------------
// This function prints out the repair rules into the debug file.
//-----------------------------------------------------------------------
void RepairPlan::dumpRepairRuleList_(void)
{
    int i, j, k, optionSize;

    gDebugger->printfLine();
    gDebugger->printfDebug("REPAIR RULE LIST:\n");
    gDebugger->printfDebug("\n");

    for (i = 0; i < (int)(repairRuleList_.size()); i++)
    {
        for (j = 0; j < (int)(repairRuleList_[i].problems.size()); j++)
        {
            gDebugger->printfDebug(
                "%s %s\n",
                RC_STRING_PROBLEM_NAME_.c_str(),
                repairRuleList_[i].problems[j].name.c_str());
            
            optionSize = repairRuleList_[i].problems[j].options.size();

            for (k = 0; k < optionSize; k++)
            {
                gDebugger->printfDebug(
                    "%s \"%s\"\n",
                    RC_STRING_PROBLEM_OPTION_.c_str(),
                    repairRuleList_[i].problems[j].options[k].value.c_str());
            }
        }

        gDebugger->printfDebug(
            "%s %d\n",
            RC_STRING_REPAIR_SOLUTION_NUMBER_.c_str(),
            repairRuleList_[i].solutionNumber);

        gDebugger->printfDebug("\n");
    }
}

//-----------------------------------------------------------------------
// This function saves the repair rule in the memory.
//-----------------------------------------------------------------------
void RepairPlan::saveRepairRule_(void)
{
    MemoryManager::Features_t features;
    MemoryManager::Feature_t feature;
    vector<Problem_t> problems;
    int i, j;

    for (i = 0; i < (int)(repairRuleList_.size()); i++)
    {
        features.clear();
        problems = repairRuleList_[i].problems;

        for (j = 0; j < (int)(problems.size()); j++)
        {
            feature.name = problems[j].description;

            if ((int)(problems[j].options.size() > 0))
            {
                if ((int)(problems[j].options.size() > 1))
                {
                    gWindows->printfPlannerWindow(
                        "Warning: Error in a repair rule.\n");
                }

                feature.value = problems[j].options[0].value;
            }
            else
            {
                feature.value = EMPTY_STRING_;
            }

            feature.nonIndex = DEFAULT_FEATURE_NONINDEX_VALUE_;
            feature.weight = DEFAULT_FEATURE_WEIGHT_VALUE_;
            features.push_back(feature);
        }

        repairMemoryManager_->saveNewRepairPlan(
            repairRuleList_[i].solutionNumber,
            features);
    }
}

//-----------------------------------------------------------------------
// This function prepares the list of the repair solutions.
//-----------------------------------------------------------------------
void RepairPlan::prepareSolutions_(void)
{
    Solution_t *solution = NULL;
    RepairQuestion_t repairQuestion;

    currentSolution_ = NULL;

    // Solution No. 1: RPS_EnterBuilding().
    solution = new Solution_t;
    solution->number = 1;
    solution->solution = new RPS_EnterBuilding();
    solution->solution->setOffendingStateAsKnown();
    repairQuestion.type = ADD_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please click on the entry of the building (inside)\nwhere you want the robot to enter.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    repairQuestion.type = STATE_NAME;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = EMPTY_STRING_;
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 2: RPS_EnterRoom().
    solution = new Solution_t;
    solution->number = 2;
    solution->solution = new RPS_EnterRoom();
    solution->solution->setOffendingStateAsKnown();
    repairQuestion.type = ADD_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please click on the room where you want\nthe robot to enter.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    repairQuestion.type = STATE_NAME;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = EMPTY_STRING_;
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 3: RPS_AddIndoorPoint().
    solution = new Solution_t;
    solution->number = 3;
    solution->solution = new RPS_AddIndoorPoint();
    solution->solution->setOffendingStateAsKnown();
    repairQuestion.type = ADD_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please click on the indoor point where you want\nthe robot to reach.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    repairQuestion.type = STATE_NAME;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = EMPTY_STRING_;
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 4: RPS_AddOutdoorPoint().
    solution = new Solution_t;
    solution->number = 4;
    solution->solution = new RPS_AddOutdoorPoint();
    solution->solution->setOffendingStateAsKnown();
    repairQuestion.type = ADD_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please click on the outdoor point where you want\nthe robot to reach.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    repairQuestion.type = STATE_NAME;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = EMPTY_STRING_;
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 5: RPS_MovePoint().
    solution = new Solution_t;
    solution->number = 5;
    solution->solution = new RPS_MovePoint();
    solution->solution->setOffendingStateAsUnknown();
    repairQuestion.type = MOD_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please drag the waypoint or via point that\nyou want to adjust.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 6: RPS_DeletePoint().
    solution = new Solution_t;
    solution->number = 6;
    solution->solution = new RPS_DeletePoint();
    solution->solution->setOffendingStateAsUnknown();
    repairQuestion.type = DEL_MAP_POINT;
    repairQuestion.status = NORMAL_QUESTION;
    repairQuestion.question = "Please click on the waypoint or via point that\nyou want to delete.";
    solution->solution->addRepairFollowUpQuestion(repairQuestion);
    solutionList_.push_back(solution);

    // Solution No. 7: RPS_AddTurnAroundHallway()
    solution = new Solution_t;
    solution->number = 7;
    solution->solution = new RPS_AddTurnAroundHallway();
    solution->solution->setOffendingStateAsKnown();
    solutionList_.push_back(solution);

    // Solution No. 8: RPS_ReplaceWithStayWithHostage()
    solution = new Solution_t;
    solution->number = 8;
    solution->solution = new RPS_ReplaceWithStayWithHostage();
    solution->solution->setOffendingStateAsKnown();
    solutionList_.push_back(solution);
}

//-----------------------------------------------------------------------
// This function finds the solution from solution number
//-----------------------------------------------------------------------
RepairPlanSolution *RepairPlan::getSolutionFromNumber_(int number)
{
    int i;

    for (i = 0; i < (int)(solutionList_.size()); i++)
    {
        if (solutionList_[i]->number == number)
        {
            return solutionList_[i]->solution;
        }
    }

    return NULL;
}

//-----------------------------------------------------------------------
// This function gets the repair question based on the composition of the
// decision tree.
//-----------------------------------------------------------------------
RepairQuestion_t RepairPlan::getRepairQuestion(int lastSelectedOption)
{
    MemoryManager::DecisionTreeNode_t *currentNode = NULL;
    RepairQuestion_t repairQuestion;
    ProblemOption_t lastOption; 
    string optionValue;
    int i, solutionNumber;

    repairQuestion.status = NO_SOLUTION;
    repairQuestion.type = TOGGLE;
    currentSolution_ = NULL;

    if ((lastSelectedOption == -1) ||
        ((int)(lastOptions_.size()) <= lastSelectedOption))
    {
        currentNode = repairMemoryManager_->getCurrentNode(EMPTY_STRING_);
    }
    else
    {
        currentNode = repairMemoryManager_->getCurrentNode(
            lastOptions_[lastSelectedOption].value);
    }

    lastOptions_.clear();

    if (currentNode == NULL)
    {
        gWindows->printfPlannerWindow(
            "Warning: Error finding a repair plan.\n");
        return repairQuestion;
    }

    if (currentNode->isLeafNode)
    {
        solutionNumber = repairMemoryManager_->getSolutionNumber(
            currentNode->data.indexes[0]);

        if (solutionNumber > 0)
        {
            repairQuestion.status = HAS_SOLUTION;
            currentSolution_ = getSolutionFromNumber_(solutionNumber);
            currentSolution_->resetFollowUpQuestions();
        }
        else
        {
            // Note: 0 (or less ) is considered to be no solution.
            repairQuestion.status = NO_SOLUTION;
        }
    }
    else
    {
        repairQuestion.status = NORMAL_QUESTION;
        repairQuestion.question = currentNode->name;

        for (i = 0; i < (int)(currentNode->edges.size()); i++)
        {
            optionValue = currentNode->edges[i].value;
            repairQuestion.options.push_back(optionValue);
            lastOption.value = optionValue;
            lastOptions_.push_back(lastOption);
        }
    }

    return repairQuestion;
}

//-----------------------------------------------------------------------
// This function gets the follow up question for the solution of the
// repair plan.
//-----------------------------------------------------------------------
RepairQuestion_t RepairPlan::getRepairFollowUpQuestion(void)
{
    RepairQuestion_t repairQuestion;

    repairQuestion.type = NO_QUESTION;
    repairQuestion.status = NO_SOLUTION;

    if (currentSolution_ != NULL)
    {
        repairQuestion = currentSolution_->getRepairFollowUpQuestion_();
    }

    return repairQuestion;
}

//-----------------------------------------------------------------------
// This function puts the answer to the follow up question of the repair
// plan.
//-----------------------------------------------------------------------
void RepairPlan::putRepairFollowUpQuestionToggleAnswer(int selectedOption)
{
    // Currently, not yet implemented.
}

//-----------------------------------------------------------------------
// This function puts the answer to the follow up question of the repair
// plan.
//-----------------------------------------------------------------------
void RepairPlan::putRepairFollowUpQuestionAddMapPointAnswer(double x, double y)
{
    if (currentSolution_ == NULL)
    {
        gWindows->printfPlannerWindow("Error: Solution not available.\n");
        return;
    }

    currentSolution_->saveTargetPoint(x, y);
}

//-----------------------------------------------------------------------
// This function puts the answer to the follow up question of the repair
// plan.
//-----------------------------------------------------------------------
void RepairPlan::putRepairFollowUpQuestionModMapPointAnswer(
        double oldX,
        double oldY,
        double newX,
        double newY,
        double closeRange)
{
    if (currentSolution_ == NULL)
    {
        gWindows->printfPlannerWindow("Error: Solution not available.\n");
        return;
    }

    offendingStateCoordinate_.x = oldX;
    offendingStateCoordinate_.y = oldY;
    currentSolution_->saveTargetPoint(newX, newY);
    updateCoordinateCloseRange_(closeRange);
}

//-----------------------------------------------------------------------
// This function puts the answer to the follow up question of the repair
// plan.
//-----------------------------------------------------------------------
void RepairPlan::putRepairFollowUpQuestionDelMapPointAnswer(
    double x,
    double y,
    double closeRange)
{
    if (currentSolution_ == NULL)
    {
        gWindows->printfPlannerWindow("Error: Solution not available.\n");
        return;
    }

    offendingStateCoordinate_.x = x;
    offendingStateCoordinate_.y = y;
    updateCoordinateCloseRange_(closeRange);
}

//-----------------------------------------------------------------------
// This function repairs the mission plan.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t RepairPlan::repairMissionPlan(
    MissionMemoryManager::MissionPlan_t missionplan,
    bool *fixed)
{
    *fixed = false;

    if (currentSolution_ == NULL)
    {
        gWindows->printfPlannerWindow("Error: Solution not available.\n");
        return missionplan;
    }

    if (!(currentSolution_->offendingStateIsKnown()))
    {
        // Find the offending state based on the waypoint coordinate
        // rather than asking the user to specify using the replay.
        offendingState_ = searchOffendingWaypointState_(
            missionplan,
            offendingStateCoordinate_);
    }
    currentSolution_->updateOffendingState(offendingState_);
    missionplan = currentSolution_->repairMissionPlan(missionplan, fixed);
    //currentSolution_->resetFollowUpQuestions();

    return missionplan;
}

//-----------------------------------------------------------------------
// This function searches offending state (StartSubTask - Waypoints, or
// GoTo) from a given coordinate of it.
//-----------------------------------------------------------------------
OffendingState_t  RepairPlan::searchOffendingWaypointState_(
    MissionMemoryManager::MissionPlan_t missionplan,
    MissionMemoryManager::Coordinates_t offendingStateCoordinate)
{
    OffendingState_t offendingState;
    MissionMemoryManager::Coordinates_t waypointCoordinate;
    CBRPlanner_Parameter_t parameter;
    int i, j, index = 0;
    bool isWaypoint = false;
    bool offendingStateFound = false;

    if ((int)(missionplan.plans.size()) == 0)
    {
        return offendingState;
    }

    // Currently, it only searches in the first robot.
    for (i = 0; i < (int)(missionplan.plans[0].size()); i++)
    {
        index = 0;
        isWaypoint = false;

        if (missionplan.plans[0][i].name == STRING_GOTO_)
        {
            isWaypoint = true;
        }
        else if (missionplan.plans[0][i].name == STRING_START_SUB_MISSION_)
        {

            for (j = 0; j < (int)(missionplan.plans[0][i].parameters.size()); j++)
            {
                parameter = missionplan.plans[0][i].parameters[j];

                if (parameter.name == STRING_SUB_MISSION_NAME_)
                {
                    if (parameter.value == STRING_PARM_WAYPOINTS_TASK_)
                    {
                        isWaypoint = true;
                        index = j+1;
                    }

                    break;
                }
            }
        }

        if (isWaypoint)
        {
            for (j = index; j < (int)(missionplan.plans[0][i].parameters.size()); j++)
            {
                parameter = missionplan.plans[0][i].parameters[j];

                if (parameter.name == STRING_GOAL_LOCATION_)
                {
                    waypointCoordinate = extractCoordinate_(parameter.value);

                    if (coordinateCloseEnough_(
                            offendingStateCoordinate,
                            waypointCoordinate))
                    {
                        offendingState.description = (missionplan.plans[0][i].description);
                        offendingStateFound = true;
                    }

                    break;
                }
            }
        }

        if (offendingStateFound)
        {
            break;
        }
    }

    return offendingState;
}

//-----------------------------------------------------------------------
// This function extracts the task name from StartSubMission state.
//-----------------------------------------------------------------------
MissionMemoryManager::Coordinates_t RepairPlan::extractCoordinate_(string value)
{
    MissionMemoryManager::Coordinates_t coordinate;
    string xString, yString;
    string::size_type index;
    int stringSize;

    value = extractValueContent_(value, true);
    index = value.find(",");

    if (index != string::npos)
    {
        stringSize = value.size();
        xString = value.substr(0, index);
        yString = value.substr(index+1, stringSize-index-1);

        coordinate.x = atof(xString.c_str());
        coordinate.y = atof(yString.c_str());
    }

    return coordinate;
}

//-----------------------------------------------------------------------
// This function extracts the value inside '{' and '}'.
//-----------------------------------------------------------------------
string RepairPlan::extractValueContent_(string value, bool withoutQuote)
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

/**********************************************************************
 * $Log: repair_plan.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
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
