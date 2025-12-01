#ifndef ROBOT2SIM_H
#define ROBOT2SIM_H
/**********************************************************************
 **                                                                  **
 **  robot2sim.h                                                     **
 **                                                                  **
 **  externs for the functions that rpc calls on server side         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: robot2sim.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: robot2sim.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:24:20  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.6  2004/05/11 19:34:35  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.5  2003/06/30 21:11:04  doug
// Support altitiude
//
// Revision 1.4  1998/06/18 20:13:09  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/12 00:59:25  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/11 12:00:24  doug
// working
//
// Revision 1.1  1997/12/10 13:18:31  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "rpc_defs.h"

namespace sara
{
/*********************************************************************/
// Functions called by the RPC dispatcher
// These are defined in simserver/src/common/simulation.cc
/*********************************************************************/

// First function called by a robot to attach to the simulator.
// The robot sends a unique key (mykey) to distinquish duplicate requests.
// The robot sends the type of robot it wishes to simulate
// The robot's ID is returned
int *attach_robot(int mykey, ROBOT_TYPES robot_type,
        double startX, double startY, double startZ, double startT);

// Detach a robot from the simulator
// The robot passes in its robot_id.
// Deletes the robot state info record
// Returns true on success
int *detach_robot(int robot_id);

// move_robot
// Turn the robot to direction 'steer' and set velocity to 'speed'.
// Use the cycle time to determine how far it moves.
// Apply the vehicle's kinematic constraints to constrain the movement. 
int *move_robot(int robot_id, double steer, double Hmps, double Vmps);

// Return the robot's current location and heading
ROBOT_XYT *get_xyzt(int robot_id);

// Return the robot's current sensor readings
SENSOR_READINGS *get_sensor_readings(int robot_id, double range);
}

/*********************************************************************/
#endif
