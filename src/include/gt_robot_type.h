/**********************************************************************
 **                                                                  **
 **                          gt_robot_type.h                         **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef GT_ROBOT_TYPE_H
#define GT_ROBOT_TYPE_H

/* $Id: gt_robot_type.h,v 1.1.1.1 2008/07/14 16:44:20 endo Exp $ */

#include <string>

using std::string;

// If you add a new entry, make sure to add one to ROBOT_TYPE_NAMES
// below, too.
typedef enum RobotType_t {
    HOLONOMIC,
    NO_HOLONOMIC,
    HSERVER,
    NUM_ROBOT_TYPES // Keep this at the end of the list.
};

const string ROBOT_TYPE_NAMES[NUM_ROBOT_TYPES] = {
    "HOLONOMIC",
    "NO_HOLONOMIC",
    "HSERVER"
};

// If you add a new entry, make sure to add one to RUN_TYPE_NAMES
// below, too.
typedef enum RunType_t {
    SIMULATION,
    REAL,
    NUM_RUN_TYPES // Keep this at the end of the list.
};

const string RUN_TYPE_NAMES[NUM_RUN_TYPES] = {
    "SIMULATION",
    "REAL"
};

// If you add a new entry, make sure to add one to ROBOT_MOBILITY_TYPE_NAMES
// below, too.
typedef enum RobotMobilityType_t {
    UNKNOWN_ROBOT_MOBILITY_TYPE,
    UXV,
    UGV,
    UUV,
    USV,
    UAV,
    SLUAVU,
    SLUAVL,
    NUM_ROBOT_MOBILITY_TYPES // Keep this at the end of the list.
};

const string ROBOT_MOBILITY_TYPE_NAMES[NUM_ROBOT_MOBILITY_TYPES] = {
    "UNKNOWN_ROBOT_MOBILITY_TYPE",
    "UXV",
    "UGV",
    "UUV",
    "USV",
    "UAV",
    "SLUAVU",
    "SLUAVL"
};

extern RobotType_t gRobotType;


#endif
/**********************************************************************
 * $Log: gt_robot_type.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/05/02 04:12:21  endo
 * TrackTask improved for the experiment.
 *
 **********************************************************************/
