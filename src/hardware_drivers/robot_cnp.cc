/**********************************************************************
 **                                                                  **
 **                           robot_cnp.cc                           **
 **                                                                  **
 **  Written by:       Yoichiro Endo                                 **
 **  Based on code by: Patrick Ulam                                  **
 **                                                                  **
 **  This class deals with runtime-CNP related functions within      **
 **  robot.                                                          **
 **                                                                  **
 **  Copyright 2005 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: robot_cnp.cc,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "robot_cnp.h"
#include "CNPObserveTask.h"
#include "CNPInterceptTask.h"
#include "CNPInspectTask.h"
#include "CNPEODTask.h"
#include "CNPTrackTask.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string RobotCNP::EMPTY_STRING_ = "";
const string RobotCNP::STRING_EOD_TASK_ = "EODTask";
const string RobotCNP::STRING_INSPECT_TASK_ = "InspectTask";
const string RobotCNP::STRING_INTERCEPT_TASK_ = "InterceptTask";
const string RobotCNP::STRING_OBSERVE_TASK_ = "ObserveTask";
const string RobotCNP::STRING_TRACK_TASK_ = "TrackTask";
const string RobotCNP::STRING_NONE_ = "None";

//-----------------------------------------------------------------------
// Constructor for RobotCNP class.
//-----------------------------------------------------------------------
RobotCNP::RobotCNP(void) :
    currentBiddingTaskInstance_(NULL),
    currentBiddingStatus_(ROBOTCNP_BIDDING_NONE)
{
    resetCurrentBiddingTask(ROBOTCNP_BIDDING_NONE);
    resetCurrentExecutingTask();
    resetWonTask_();
}

//-----------------------------------------------------------------------
// Distructor for RobotCBRClient class.
//-----------------------------------------------------------------------
RobotCNP::~RobotCNP(void)
{
}
//-----------------------------------------------------------------------
// This function calculates the bid on task.
//-----------------------------------------------------------------------
double RobotCNP::compTaskBid(CNPRobotData &rd)
{
    double bid = 0;

    // update robot constraints
    if (currentBiddingTaskInstance_ != NULL)
    {
        currentBiddingTaskInstance_->SetVehicleLocation(rd.Pos);
	//currentBiddingTaskInstance_->SetTargetVelocity(rd.OtherVelocity);
        bid = currentBiddingTaskInstance_->CalculateBid(robotConstraints_);
    }

    return bid;
}

//-----------------------------------------------------------------------
// This function adds an entry to the robot constraints while replaces
// the entry if it already exists
//-----------------------------------------------------------------------
void RobotCNP::addRobotConstraints(CNP_RobotConstraint robotConstraint)
{
    list<CNP_RobotConstraint>::iterator iterRC;
    bool entryFound = false;

    for (iterRC = robotConstraints_.begin();
         iterRC != robotConstraints_.end();
         iterRC++)
    {
        //printf("rc2: %s   %s\n", iterRC->strConstraintName.c_str(), iterRC->strConstraintValue.c_str());
        // Check to see if the entry exist.
        if ((iterRC->iRobotID == robotConstraint.iRobotID) &&
            (iterRC->strConstraintName == robotConstraint.strConstraintName))
        {
            // Found, replace it.
            *iterRC = robotConstraint;
            entryFound = true;
            break;
        }
    }

    // Not found. Add it to the list.
    if (!entryFound)
    {
        robotConstraints_.push_back(robotConstraint);
    }
}

//-----------------------------------------------------------------------
// This function updates the content of the robot constraints (if
// necessary).
//-----------------------------------------------------------------------
void RobotCNP::updateRobotConstraints(
    list<CNP_RobotConstraint> updatedRobotConstraints)
{
    list<CNP_RobotConstraint>::iterator iterRC;
    list<CNP_RobotConstraint>::iterator iterURC;
    bool entryFound = false;

    for (iterURC = updatedRobotConstraints.begin();
         iterURC != updatedRobotConstraints.end();
         iterURC++)
    {
        entryFound = false;
      


        // First, check to see if there is an entry.
        for (iterRC = robotConstraints_.begin();
             iterRC != robotConstraints_.end();
             iterRC++)
        {
            if ((iterURC->iRobotID == iterRC->iRobotID) &&
                (iterURC->strConstraintName == iterRC->strConstraintName))
            {
                // Found, replace it.
                *iterRC = *iterURC;
                entryFound = true;
                break;
            }
        }

        // Not found, add a new entry.
        if (!entryFound)
        {
            robotConstraints_.push_back(*iterURC);
        }
    }

}

//-----------------------------------------------------------------------
// This function sets the current bidding task and instantiate the instance
// of the task.
//-----------------------------------------------------------------------
bool RobotCNP::setCurrentBiddingTask(
    RobotCNP_TaskInfo_t taskInfo,
    list<CNP_TaskConstraint> taskConstraints)
{
    list<CNP_TaskConstraint>::iterator iterTC;
    const bool DEBUG_SET_CURRENT_BIDDING_TASK = false;

    currentBiddingTaskInfo_ = taskInfo;
    currentBiddingTaskConstraints_ = taskConstraints;

    if (currentBiddingTaskInstance_ != NULL)
    {
        delete currentBiddingTaskInstance_;
        currentBiddingTaskInstance_ = NULL;
    }

    resetWonTask_();

    if (currentBiddingTaskInfo_.taskDesc == STRING_EOD_TASK_)
    {
        currentBiddingTaskInstance_ = new CNPEODTask();
    }
    else if (currentBiddingTaskInfo_.taskDesc == STRING_INSPECT_TASK_)
    {
        currentBiddingTaskInstance_ = new CNPInspectTask();
    }
    else if (currentBiddingTaskInfo_.taskDesc == STRING_INTERCEPT_TASK_)
    {
        currentBiddingTaskInstance_ = new CNPInterceptTask();
    }
    else if (currentBiddingTaskInfo_.taskDesc == STRING_OBSERVE_TASK_)
    {
        currentBiddingTaskInstance_ = new CNPObserveTask();
    }
    else if (currentBiddingTaskInfo_.taskDesc == STRING_TRACK_TASK_)
    {
        currentBiddingTaskInstance_ = new CNPTrackTask();
    }
    else
    {
        fprintf(stderr, "Error: RobotCNP::setCurrentBiddingTask(). Unknown Task.\n");
        return false;
    }

    if (DEBUG_SET_CURRENT_BIDDING_TASK)
    {
        fprintf(stderr, "RobotCNP::setCurrentBiddingTask():\n");
    }

    for (iterTC = taskConstraints.begin();
         iterTC != taskConstraints.end();
         iterTC++)
    {
        if (DEBUG_SET_CURRENT_BIDDING_TASK)
        {
            fprintf(
                stderr,
                "  Adding {%s, %s} for [%s(%d)]\n",
                iterTC->strConstraintName.c_str(),
                iterTC->strConstraintValue.c_str(),
                iterTC->taskInfo.name.c_str(),
                iterTC->taskInfo.id);
        }

        currentBiddingTaskInstance_->AddTaskConstraint(*iterTC);
    }

    currentBiddingStatus_ = ROBOTCNP_BIDDING_PENDING;

    return true;
}

//-----------------------------------------------------------------------
// This function resets the current bidding task instance
//-----------------------------------------------------------------------
void RobotCNP::resetCurrentBiddingTask(int biddingStatus)
{
    if (currentBiddingTaskInstance_ != NULL)
    {
        //printf("NOT NULL\n");
        delete currentBiddingTaskInstance_;
        currentBiddingTaskInstance_ = NULL;
    }

    currentBiddingStatus_ = biddingStatus;

    switch (biddingStatus) {

    case ROBOTCNP_BIDDING_WON:
        wonTaskInfo_ = currentBiddingTaskInfo_;
        break;

    default:
        resetWonTask_();
        break;
    }

    currentBiddingTaskInfo_.taskID = -1;
    currentBiddingTaskInfo_.iteration = 0;
    currentBiddingTaskInfo_.taskDesc = STRING_NONE_;
    currentBiddingTaskInfo_.restriction = STRING_NONE_;
}

//-----------------------------------------------------------------------
// This function resets the current executing task info
//-----------------------------------------------------------------------
void RobotCNP::resetCurrentExecutingTask(void)
{
    currentExecutingTaskInfo_.taskID = -1;
    currentExecutingTaskInfo_.iteration = 0;
    currentExecutingTaskInfo_.taskDesc = STRING_NONE_;
    currentExecutingTaskInfo_.restriction = STRING_NONE_;
}

//-----------------------------------------------------------------------
// This function resets the won task info
//-----------------------------------------------------------------------
void RobotCNP::resetWonTask_(void)
{
    wonTaskInfo_.taskID = -1;
    wonTaskInfo_.iteration = 0;
    wonTaskInfo_.taskDesc = STRING_NONE_;
    wonTaskInfo_.restriction = STRING_NONE_;
}

//-----------------------------------------------------------------------
// This function sets the won task as the current executing task.
//-----------------------------------------------------------------------
void RobotCNP::setWonTaskExecuted(void)
{
    currentExecutingTaskInfo_ = wonTaskInfo_;
}

list<CNP_TaskConstraint> RobotCNP::getCurrentBiddingTaskConstraints(void) {
    return currentBiddingTaskConstraints_;
}




/**********************************************************************
 * $Log: robot_cnp.cc,v $
 * Revision 1.1.1.1  2006/07/12 13:37:56  endo
 * MissionLab 7.0
 *
 * Revision 1.9  2006/05/15 06:26:46  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.8  2006/05/02 04:20:39  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.7  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.6  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.5  2006/02/20 22:19:29  pulam
 * Bug fix involving broadcast of task completion
 *
 * Revision 1.4  2006/02/20 02:21:01  pulam
 * compTaskBid
 *
 * Revision 1.3  2006/02/19 23:51:53  pulam
 * Changes made for experiments
 *
 * Revision 1.2  2006/01/30 03:02:24  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1  2006/01/10 06:09:02  endo
 * AO-FNC Type-I check-in.
 *
 **********************************************************************/
