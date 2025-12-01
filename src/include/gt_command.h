/**********************************************************************
 **                                                                  **
 **                           gt_command.h                           **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: gt_command.h,v 1.2 2008/07/16 21:16:37 endo Exp $ */

#ifndef GT_COMMAND_H
#define GT_COMMAND_H

#include <sys/time.h>

#include "gt_measure.h"

/**********************************************************************
 **                                                                  **
 **                              types                               **
 **                                                                  **
 **********************************************************************/

#ifdef GT_COMMAND_C

/* NOTE: The corresponding types for the following three id arrays are
         defined in mlab/gt_tcx_type_def.h */

   char *gt_behavior_name[] = {
      "No-Behavor", "Start", "MoveTo", "Teleoperate", "SweepTo",
      "Follow", "Occupy", "Stop", "Quit", "Set", "Print"
   };

   char *gt_formation_name[] = {
      "No-Formation", "Line", "Line-left", "Line-right", 
      "Wedge", "Wedge-left", "Wedge-right",
      "Diamond", "Column", "Sweep"
   };   

   char *gt_technique_name[] = {
      "No-Technique", "Traveling", "Traveling-Overwatch", "Bounding-Overwatch"
   };

#else

   extern char *gt_behavior_name[];
   extern char *gt_formation_name[];
   extern char *gt_technique_name[];

#endif  /* GT_COMMAND_C */



typedef int gt_Robot;

/* Note that the following values are predefined for gt_Robot:
     0 is for a UNIT (with no executable)
     1 is for the vanilla robot with the executable name "robot" */

const gt_Robot UNIT = 0;
const gt_Robot ROBOT = 1;


/**********************************************************************
 **                                                                  **
 **                    definitions for parameters                    **
 **                                                                  **
 **********************************************************************/

typedef enum {
   NO_TYPE,
   BOOLEAN_PARAMETER,
   NUMBER_PARAMETER
} gt_Parameter_types;


typedef enum {
   NO_PARAMETER,

   SHOW_TRAILS,
   SHOW_REPELLING_OBSTACLES,
   SHOW_MOVEMENT_VECTORS,
   USE_MILITARY_STYLE,

   DEBUG_ROBOTS,
   DEBUG_SIMULATOR,
   DEBUG_SCHEDULER,

   CYCLE_DURATION
} gt_Parameter;


#ifdef GT_COMMAND_C

   char *gt_parameter_name[] = {
      "No-Parameter",

      "Show-Robot-Trails",
      "Highlight-Repelling-Obstacles",
      "Show-Movement-Vectors", 
      "Use-Military-Style",

      "Debug-Robots",
      "Debug-Simulator",
      "Debug-Scheduler",

      "Cycle-Duration"
   };

   gt_Parameter_types gt_parameter_type[] =
   {
      NO_TYPE,

      BOOLEAN_PARAMETER,
      BOOLEAN_PARAMETER,
      BOOLEAN_PARAMETER,
      BOOLEAN_PARAMETER,

      BOOLEAN_PARAMETER,
      BOOLEAN_PARAMETER,
      BOOLEAN_PARAMETER,

      NUMBER_PARAMETER
   };

#else

   extern char *gt_parameter_name[];
   extern gt_Parameter_types gt_parameter_type[];

#endif  /* GT_COMMAND_C */




typedef enum {
   PRINT_NOTHING,
   CONSOLE_DB,
   ROBOT_INFO
} gt_Print;

#ifdef GT_COMMAND_C

   char *gt_print_name[] = {
      "Print-Nothing",
      "Console-Database",
      "Robot-Info"
   };

#else

   extern char *gt_print_name[];

#endif  /* GT_COMMAND_C */



typedef struct gt_Unit {
              int  id;
             char *name;
	 gt_Robot  type;
   struct gt_Unit *children;  /* subordinate units */
   struct gt_Unit *next;      /* next peer unit */
} gt_Unit;


typedef union {
   int i;
   double d;
} gt_Parameter_union;


typedef struct gt_Step {
		  char *name;
		  char *unit;
		  char *location;
		  char *phase_line_name;
          gt_Parameter  parameter_to_set;
    gt_Parameter_union  parameter_value;
	      gt_Print  print_what;
	    gt_Command *command;
	struct gt_Step *next;           /* the next command in this step */
	struct gt_Step *next_step;
	struct gt_Step *previous_step;
} gt_Step;


/**********************************************************************
 **                                                                  **
 **                     declare global variables                     **
 **                                                                  **
 **********************************************************************/

extern int some_command_completed;

extern int executing_step_list;


/**********************************************************************
 **                                                                  **
 **                     declare global functions                     **
 **                                                                  **
 **********************************************************************/

/* Command related functions */

void gt_start_step_list();
void gt_start_step(char *label);
void gt_start_command();

gt_Command *gt_new_command();
gt_Command *gt_copy_command(gt_Command *org);
void gt_free_command(gt_Command *cmd);

int gt_add_command_to_step(char *unit,
			   gt_Behavior behavior,
			   char *location);

void gt_specify_offsets(float dx, float dy);
void gt_specify_formation(gt_Formation form);
void gt_specify_technique(gt_Technique tech);
void gt_specify_speed(float v);
void gt_specify_phase_line(char *name, char *date, char *time);
void gt_specify_phase_line_msg(char *ack_msg);
void gt_specify_phase_line_wait(int wait_at_line);
void gt_specify_completion(char *msg, int freeze);
void gt_specify_until(char *date_str, char *time_str);

void gt_abort_command_execution();
void gt_stop_command_execution();
void gt_backup_step_list();
void gt_clear_step_list();
void gt_execute_step_list();
void gt_execute_next_step();
void gt_execute_user_command(char *str);
void gt_forward_step_list();
void gt_rewind_step_list();
void gt_toggle_pause_execution();
void gt_toggle_feedback(void);


/* Unit related functions */

void gt_print_unit(gt_Unit *unit);

gt_Unit *gt_new_unit(int id, char *name, gt_Robot type,
		     gt_Unit *children, gt_Unit *next);


#endif  /* GT_COMMAND_H */



/**********************************************************************
 * $Log: gt_command.h,v $
 * Revision 1.2  2008/07/16 21:16:37  endo
 * Military style is now a toggle option.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/11 06:42:25  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.30  1995/04/26  17:52:51  jmc
 * Added the Teleoperate behavior name to the list.
 *
 * Revision 1.29  1995/03/31  22:06:26  jmc
 * Added a copyright notice.
 *
 * Revision 1.28  1995/02/15  16:29:48  jmc
 * Updated gt_behavior_name and gt_formation_name to reflect the new
 * values in the corresponding types gt_Behavior and gt_Formation.
 *
 * Revision 1.27  1995/02/13  18:49:34  jmc
 * Reorganized ifdefs to keep all the parameter definitions together.
 * Added a new type (gt_Parameter_types) to allow parameters to be
 * either boolean or numbers.  This was necessary to add numerical
 * paramters that can be part of a command script.
 *
 * Revision 1.26  1995/02/02  15:35:19  jmc
 * Added the Show-Movement-Vectors parameter to the parameter
 * enumeration (gt_Parameter) and the parameter name array
 * (gt_parameter_name).
 *
 * Revision 1.25  1994/11/04  15:27:04  jmc
 * Added DEBUG_SCHEDULER to gt_Parameter and suitable string to
 * gt_parameter_name.
 *
 * Revision 1.24  1994/11/03  18:46:48  jmc
 * Added SHOW_REPELLING_OBSTACLES to gt_Parameter for SET commands.
 *
 * Revision 1.23  1994/11/03  17:12:33  jmc
 * Added a function prototype for gt_stop_command_execution.
 *
 * Revision 1.22  1994/10/28  20:17:15  jmc
 * Changed gt_specify_until to take a date and time string instead of
 * a message because of the changed semantics of the OCCUPY UNTIL
 * command.
 *
 * Revision 1.21  1994/10/25  16:23:00  jmc
 * Changed wait argument in gt_specify_phase_line_wait to
 * wait_at_line to avoid conflict with some global.
 *
 * Revision 1.20  1994/10/25  15:58:19  jmc
 * Added "Print" to gt_behavior_name and switched the order of "Set"
 * and "Quit" in the same.  Removed gt_robot_name since this has been
 * superseded by the robot info database gt_robot_info which is now
 * declared here.   Added a "print_what" entry to the step so as not
 * to add it to gt_Command.
 *
 * Revision 1.19  1994/10/18  22:36:47  jmc
 * Declared executing_step_list as a global variable.
 *
 * Revision 1.18  1994/10/18  20:20:57  jmc
 * Added function prototype for gt_clear_step_list.
 *
 * Revision 1.17  1994/10/06  19:58:54  jmc
 * Added prototypes for gt_copy_command and gt_free_command.
 *
 * Revision 1.16  1994/09/03  00:24:30  jmc
 * Added declaration for global variable some_command_completed.
 *
 * Revision 1.15  1994/08/29  17:22:53  jmc
 * Added gt_Parameter and gt_parameter_name and added new data
 * members parameter_to_set and parameter_value to gt_Step to deal
 * with SET commands.
 *
 * Revision 1.14  1994/08/29  15:57:47  jmc
 * Added name-strings for new behaviors.  Added new name-string array
 * for gt_Parameter.
 *
 * Revision 1.13  1994/08/23  15:35:25  jmc
 * Removed the "pausing_execution" variable in favor of the already
 * existing "paused" variable.
 *
 * Revision 1.12  1994/08/17  20:33:55  jmc
 * Modified gt_Step to remove the "until" field since it has been
 * moved into gt_Command (in gt_simulation.h).  Modified the
 * [gt_]add_command_* functions prototypes.  Added new gt_specify_*
 * functions prototypes.  See gt_command.c for more details.  Deleted
 * the gt_print_* commands.
 *
 * Revision 1.11  1994/08/16  15:59:10  jmc
 * Changed command_list to step_list.
 *
 * Revision 1.10  1994/08/12  14:15:02  jmc
 * Added speed to gt_add_command_to_step.
 *
 * Revision 1.9  1994/08/11  20:53:52  jmc
 * Removed an obsolete comment.
 *
 * Revision 1.8  1994/08/03  13:19:08  jmc
 * Changed return type of gt_add_command_to_step from void to int to
 * improve error checking.
 *
 * Revision 1.7  1994/07/29  13:48:06  jmc
 * Added previous_step to gt_Step to allow easy backing up the
 * command list.
 *
 * Revision 1.6  1994/07/28  21:58:56  jmc
 * Added function prototypes for new command_list positioning
 * commands.  Also changed stop_command to abort_command.
 *
 * Revision 1.5  1994/07/20  17:27:25  jmc
 * Changed HUMMER to HMMVW.
 *
 * Revision 1.4  1994/07/20  14:38:47  jmc
 * Changed "destination" to "location".
 *
 * Revision 1.3  1994/07/19  17:02:09  jmc
 * Modified to allow GT_COMMAND_C to determine whether the global strings will be
 * declared or defined here.
 *
 * Revision 1.2  1994/07/14  13:20:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
