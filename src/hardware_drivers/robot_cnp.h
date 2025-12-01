/**********************************************************************
 **                                                                  **
 **                           robot_cnp.h                            **
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
#ifndef ROBOT_CNP_H
#define ROBOT_CNP_H

/* $Id: robot_cnp.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <list>
#include <string>
#include "CNPConstraint.h"
#include "AbstractCNPTask.h"
#include "gt_simulation.h"

using std::list;
using std::string;

typedef struct RobotCNP_TaskInfo_t {
    int taskID;
    int iteration;
    string taskDesc;
    string restriction;
};

typedef enum RobotCNP_BiddingStatus_t {
    ROBOTCNP_BIDDING_NONE,
    ROBOTCNP_BIDDING_PENDING,
    ROBOTCNP_BIDDING_WON,
    ROBOTCNP_BIDDING_LOST
};


struct CNPRobotData {
  robot_position Pos;
  float Velocity;
  bool Stealth;

  robot_position OtherPos;
  robot_position OtherVelocity;
};


class RobotCNP {

protected:
    RobotCNP_TaskInfo_t currentBiddingTaskInfo_;
    RobotCNP_TaskInfo_t currentExecutingTaskInfo_;
    RobotCNP_TaskInfo_t wonTaskInfo_;
    AbstractCNPTask *currentBiddingTaskInstance_;
    list<CNP_RobotConstraint> robotConstraints_;
    list<CNP_TaskConstraint> currentBiddingTaskConstraints_;
    int currentBiddingStatus_;

    void resetWonTask_(void);

    static const string EMPTY_STRING_;
    static const string STRING_EOD_TASK_;
    static const string STRING_INSPECT_TASK_;
    static const string STRING_INTERCEPT_TASK_;
    static const string STRING_OBSERVE_TASK_;
    static const string STRING_TRACK_TASK_;
    static const string STRING_NONE_;

public:
    RobotCNP(void);
    ~RobotCNP(void);
    RobotCNP_TaskInfo_t getCurrentBiddingTaskInfo(void);
    RobotCNP_TaskInfo_t getCurrentExecutingTaskInfo(void);
    RobotCNP_TaskInfo_t getWonTaskInfo(void);

    list<CNP_TaskConstraint> getCurrentBiddingTaskConstraints(void);
    double compTaskBid(CNPRobotData &rd);
    void saveRobotConstraints(list<CNP_RobotConstraint> robotConstraints);
    void addRobotConstraints(CNP_RobotConstraint robotConstraint);
    void updateRobotConstraints(list<CNP_RobotConstraint> updatedRobotConstraints);
    bool setCurrentBiddingTask(
        RobotCNP_TaskInfo_t taskInfo,
        list<CNP_TaskConstraint> taskConstraints);
    void resetCurrentBiddingTask(int biddingStatus);
    void resetCurrentExecutingTask(void);
    void setWonTaskExecuted(void);
    bool biddingLost(void);
    bool biddingWon(void);
};


inline RobotCNP_TaskInfo_t RobotCNP::getCurrentBiddingTaskInfo(void)
{
    return currentBiddingTaskInfo_;
}

inline RobotCNP_TaskInfo_t RobotCNP::getCurrentExecutingTaskInfo(void)
{
    return currentExecutingTaskInfo_;
}

inline RobotCNP_TaskInfo_t RobotCNP::getWonTaskInfo(void)
{
    return wonTaskInfo_;
}

inline void RobotCNP::saveRobotConstraints(list<CNP_RobotConstraint> robotConstraints)
{
    robotConstraints_ = robotConstraints;
}

inline bool RobotCNP::biddingLost(void)
{
  return (currentBiddingStatus_ == ROBOTCNP_BIDDING_LOST);
}

inline bool RobotCNP::biddingWon(void)
{
    return (currentBiddingStatus_ == ROBOTCNP_BIDDING_WON);
}

extern RobotCNP *gRobotCNP;

#endif
/**********************************************************************
 * $Log: robot_cnp.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:56  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/05/15 06:26:46  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.5  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.4  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.3  2006/02/20 22:19:29  pulam
 * Bug fix involving broadcast of task completion
 *
 * Revision 1.2  2006/02/19 23:51:53  pulam
 * Changes made for experiments
 *
 * Revision 1.1  2006/01/10 06:09:02  endo
 * AO-FNC Type-I check-in.
 *
 **********************************************************************/
