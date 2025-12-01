#ifndef ROBOT2CON_H
#define ROBOT2CON_H
/**********************************************************************
 **                                                                  **
 **  robot2con.h                                                     **
 **                                                                  **
 **  externs for the functions that rpc calls on server side to talk **
 **  to the console.                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: robot2con.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: robot2con.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:22:47  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.11  2004/05/11 19:34:36  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.10  1998/05/12 18:00:26  doug
* made ping return ping_info rec
*
* Revision 1.9  1998/05/12 15:31:27  doug
* added locking
*
* Revision 1.8  1998/05/04 20:38:17  doug
* converted pause, single step, and resume to return an int
*
* Revision 1.7  1998/03/19 15:11:51  doug
* can download FSAs
*
* Revision 1.6  1998/03/17 10:54:28  doug
* adding rpc commands
*
* Revision 1.5  1998/02/23 15:53:37  doug
* added list_prims
*
* Revision 1.4  1998/02/18 14:56:01  doug
* *** empty log message ***
*
* Revision 1.3  1997/12/15 11:46:37  doug
* added pause, singlestep and resume
*
* Revision 1.2  1997/12/13 23:20:56  doug
* *** empty log message ***
*
* Revision 1.1  1997/12/11 15:55:48  doug
* Initial revision
*
*
**********************************************************************/

#include "r2c_defs.h"

namespace sara
{

/*********************************************************************/
/**@name The RPC functions available to the operator console */
//@{
/**
Reply to a broadcast RPC ping call.
The ping_robot function is called by the operator console as an
RPC broadcast function to find out what robots are active.

Returns: The type of robot responding.

Note: The network address of the robot is also implicitly returned.
*/
//PING_INFO *ping_robot();





/**
Attach to robot to get detailed info.
The operator console must attach to the robot before it can
ask for detailed information, such as the FSA and current
status.

The caller should pass a unique key to allow discarding duplicate
requests in case the reply packet is lost.

Returns: A handle used in subsequent commands.
*/
int *attach_to_robot(int mykey);





/**
Detach from the robot.
When the caller no longer requires detailed access to the robot they
should detach from the robot to allow it to remove overhead associated
with maintaining the detailed information.

Returns: true if successful, false if the opcon_id is invalid.
*/
int *detach_from_robot(int opcon_id);




/**
Get the public robot status.
This function is called to get the robot status publically available,
without needing to attach to the robot.
This status includes the current location and heading of the robot.

Returns: The location record.
*/
//ROBOT_STATUS *get_robot_status();




/**
Pause the robot.
This command sets a flag causing the robot to pause at the
end of the current cycle.
It will halt execution of the mission as well as its movement until
either a single step or resume command is received.

Returns: true.
*/
//int *robot_pause();




/**
Single step the robot.
Cause a paused robot to execute a single hardware cycle of the current mission.
The robot will likely physically move a small amount before again halting.
This command has no effect if the robot is not in the paused state.

Returns: true.
*/
//int *robot_single_step();




/**
Resume the robot.
This command is used to resume execution of a mission which was previously
paused.  It has no effect if the robot was not in the paused state.

Returns: true.
*/
//int *robot_resume();




/**
List the current mission FSA.
This command returns a copy of the current mission FSA.
The FSA is output in ASCII text using the following format:
\begin{verbatim}
  state at 100,100 name "StateName1" action "ActionName1";
  state at 200,100 name "StateName2" action "ActionName2";

  transition at 150,100 from "CurrentState" to "NewState" when "Trigger";
  transition at 150,100 from "CurrentState" to "NewState" when "Trigger";

  start state "StateName";
\end{verbatim}

All lines are terminated by newlines (not carrage returns).
The list of states will be terminated by a blank line and
the list of transitions will be terminated by a blank line.

Returns: The FSA.
*/
//char **get_fsa();




/**
Download a new mission FSA.
This command replaces the robot's current mission FSA with this new one.
The FSA is transmitted in free-form ASCII text using the same format as
get_fsa with the addition of the active state flag to tell the robot
where to start executing the mission if a state other than the start
state is appropriate.  For example:
\begin{verbatim}
  state at 100,100 name "StateName1" action "ActionName1";
  state at 200,100 name "StateName2" action "ActionName2";
  transition at 150,100 from "CurrentState" to "NewState" when "Trigger";
  transition at 150,100 from "CurrentState" to "NewState" when "Trigger";
  start state "StateName";
  active state "StateName";
\end{verbatim}

The key is the lock handle returned by a valid lock_fsa call.

Returns: 1 on success, 0 if the FSA is invalid, -1 if key is invalid.
*/
int *put_fsa(char *new_fsa, int key);



/**
Locks the FSA for editing by the caller.

The caller should pass a unique sequence number to allow discarding duplicate
requests in case the reply packet is lost.

Note: The lock will expire after period set by the robot.

Returns: A key used in put_fsa and unlock_fsa calls or 0 on failure.
*/
int *lock_fsa(int seqnum);



/**
Unlocks the FSA to allow others access.

key is the handle returned by a previous lock_fsa.

Returns: 	1 on success,
		0 if no lock is present (lock timed out?),
	       -1 if invalid key.
*/
int *unlock_fsa(int key);



/**
List the available primitives.
This command returns an ASCII string listing the available trigger and action
primitives available on this particular robot.
*/
//char **list_prims();




/**
Get the private FSA state information.
Returns the active FSA state and values of triggers for transitions leaving
the active state.
*/
//char **get_state_info();

//@}

/*********************************************************************/
}
#endif
