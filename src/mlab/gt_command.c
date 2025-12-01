/*********************************************************************
 **                                                                  **
 **                           gt_command.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_command.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Intrinsic.h>

#define GT_COMMAND_C
#include "gt_command.h"

#include "gt_sim.h"
#include "console.h"
#include "gt_measure.h"
#include "gt_console_db.h"
#include "gt_command_panel.h"
#include "console_side_com.h"
#include "telop.h"

extern char cmdl_command_buff[];
extern int cmdl_command_buff_pos;
extern int cmdl_reading_string;
extern int cmdl_parse();
extern void cmdl_file_reset();


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define null_out(var) \
  if (var) free(var); \
  var = NULL


/**********************************************************************
 **                                                                  **
 **                     define global variables                      **
 **                                                                  **
 **********************************************************************/

int some_command_completed = 0;

int executing_step_list = FALSE;

bool die_after_warning = false;
extern int auto_run;

/**********************************************************************
 **                                                                  **
 **                             typedefs                             **
 **                                                                  **
 **********************************************************************/


typedef struct gt_Incomplete_command {
                     gt_Command *command;
   struct gt_Incomplete_command *next;
} gt_Incomplete_command;


/**********************************************************************
 **                                                                  **
 **                         local variables                          **
 **                                                                  **
 **********************************************************************/

static char indent[80];

static gt_Step *step_list = NULL;
static gt_Step *current_step = NULL;
static gt_Step *immediate_step = NULL;

static int immediate_command_mode = FALSE;

static int executed_first_step = FALSE;
static int abort_execution = FALSE;
static int stop_execution = FALSE;

static int continue_with_next_step = FALSE;
/* Cue from [Single Step] button to go on to the next step when executing
   the step list. */

static gt_Step *executing_single_step = NULL;
/* NULL if we are not executing a single step.  Points to the step being
   executed if a single step is being executed.  Should be NULL when
   automatically executing step list. */

static gt_Incomplete_command *incomplete_commands = NULL;


/* local variables for the various command clauses */
static float start_x_offset;
static float start_y_offset;

static gt_Formation formation;
static gt_Technique technique;

static float speed;

static char *phase_line;          /* NULL means no phase line was given */
static char *phase_line_date;
static char *phase_line_time;
static char *acknowledge_message;
static int wait_at_phase_line;

static int completion_clause_given;
static char *completion_message;
static int freeze_on_completion;

static int until_clause_given;
static char *until_time_str;
static char *until_date_str;



/**********************************************************************
 **                                                                  **
 **                     declare local functions                      **
 **                                                                  **
 **********************************************************************/

static int add_start_command(char *unit, char *location);
static int add_movement_command(char *unit, gt_Behavior behavior,
				char *location);
static int add_occupy_command(char *unit, char *location);
static int add_stop_command(char *unit);
static int add_set_command(gt_Parameter parameter, int val);
static int add_print_command(gt_Print print_what);
static int add_quit_command();

static gt_Step *setup_step();

static time_t compute_time(char *date, char *time);
static int seconds_from_time(char *time_str);
static int month_length(int month, int year);
static int execute_step(gt_Step *step, int wait);
int execute_step_list_workproc();
static void free_step_list(gt_Step *step_list);
static void start_immediate_command();

static void free_incomplete_commands();
static void add_incomplete_command(gt_Command *cmd);
static void remove_incomplete_command(int robot_id);
static int occupy_command_deletable(gt_Command *cmd);
static gt_Step *find_relevant_step(gt_Command *cmd);

static void print_subunit(gt_Unit *unit);

static gt_Step *new_step(char *name, gt_Command *cmd, gt_Step *next,
			 gt_Step *next_step, gt_Step *previous_step);
static void free_step(gt_Step *step);


/**********************************************************************
 **                                                                  **
 **      implement global functions to construct command lists       **
 **                                                                  **
 **********************************************************************/


/**********************************************************************
 **         gt_start_step_list         **
 ****************************************/

void gt_start_step_list()
{
   /* free the old command list */
   free_step_list(step_list);
   step_list = NULL;

   /* make sure the current step is NULL */
   current_step = NULL;

   /* Note that we are constructing a command list */
   immediate_command_mode = FALSE;
}




/**********************************************************************
 **           gt_start_step            **
 ****************************************/

void gt_start_step(char *label)
{
   gt_Step *step, *target;

   /* create a new step */
   step = new_step(label, NULL, NULL, NULL, NULL);

   /* figure out where to put it (the end of the list) */
   if (step_list == NULL) {
      step_list = step;
      }
   else {
      target = step_list;
      while (target->next_step)
	 target = target->next_step;
      target->next_step = step;
      step->previous_step = target;
      }

   /* in any case, point to the new step */
   current_step = step;
}



/**********************************************************************
 **          gt_start_command          **
 ****************************************/

void gt_start_command()
{
  /* set the default offsets for the START command */
  start_x_offset = 0.0;
  start_y_offset = 0.0;

   /* set the defaults for the movement */
  formation = NO_FORMATION;
  technique = TRAVELING;

  /* set the default speed */
  speed = 0.0;

  /* defaults for phase lines */
  null_out(phase_line);
  null_out(phase_line_date);
  null_out(phase_line_time);
  null_out(acknowledge_message);
  wait_at_phase_line = FALSE;

  /* defaults for the completion clause */
  completion_clause_given = FALSE;
  null_out(completion_message);
  freeze_on_completion = FALSE;

  /* defaults for the OCCUPY "UNTIL" clause */
  until_clause_given = FALSE;
  null_out(until_time_str);
  null_out(until_date_str);
}


/**********************************************************************
 **           gt_new_command           **
 ****************************************/

gt_Command *gt_new_command()
{
   gt_Command *cmd;

   /* malloc it first */
   if ((cmd = (gt_Command *)malloc(sizeof(gt_Command))) == NULL)
      return NULL;

   /* initialize everything */

   cmd->num_path_points = 0;
   cmd->path = NULL;

   cmd->unit_size = 0;
   cmd->unit = NULL;

   cmd->num_pl_points = 0;
   cmd->phase_line = NULL;

   cmd->goal.x = 0.0;
   cmd->goal.y = 0.0;
   cmd->behavior = NO_BEHAVIOR;
   cmd->formation = NO_FORMATION;
   cmd->technique = NO_TECHNIQUE;
   cmd->speed = 0.0;
   cmd->width = 0.0;
   cmd->phase_line_given = FALSE;
   cmd->time = 0;
   cmd->phase_line_ack_msg = strdup("");
   cmd->wait_at_phase_line = FALSE;
   cmd->completion_given = FALSE;
   cmd->completion_msg = strdup("");
   cmd->freeze_on_completion = FALSE;
   cmd->until_timeout_given = FALSE;
   cmd->robot_id = 0;
   cmd->seq_num = 0;

   return cmd;
}


/**********************************************************************
 **           gt_copy_command           **
 ****************************************/

gt_Command *gt_copy_command(gt_Command *org)
{
   gt_Command *cmd;
   int         i;

   /* malloc it first */
   if ((cmd = (gt_Command *)malloc(sizeof(gt_Command))) == NULL)
      return NULL;

   /* copy the easy stuff first */
   cmd->goal = org->goal;
   cmd->behavior = org->behavior;
   cmd->formation = org->formation;
   cmd->technique = org->technique;
   cmd->speed = org->speed;
   cmd->width = org->width;
   cmd->phase_line_given = org->phase_line_given;
   cmd->time = org->time;
   cmd->wait_at_phase_line = org->wait_at_phase_line;
   cmd->completion_given = org->completion_given;
   cmd->freeze_on_completion = org->freeze_on_completion;
   cmd->until_timeout_given = org->until_timeout_given;
   cmd->robot_id = org->robot_id;
   cmd->seq_num = org->seq_num;

   /* now do the strings */
   if( org->phase_line_ack_msg == NULL )
      cmd->phase_line_ack_msg = NULL;
   else
      cmd->phase_line_ack_msg = strdup(org->phase_line_ack_msg);

   if( org->completion_msg == NULL )
      cmd->completion_msg = NULL;
   else
      cmd->completion_msg = strdup(org->completion_msg);

   /* now do the arrays */
   cmd->unit_size = org->unit_size;
   if( cmd->unit_size == 0 )
      cmd->unit = NULL;
   else
   {
      cmd->unit = (int *)malloc(cmd->unit_size * sizeof(int));
      for (i=0; i<cmd->unit_size; i++)
         cmd->unit[i] = org->unit[i];
   }

   cmd->num_pl_points = org->num_pl_points;
   if( cmd->num_pl_points == 0 )
      cmd->phase_line = NULL;
   else
   {
      cmd->phase_line =
	 (gt_Point *)malloc(cmd->num_pl_points*sizeof(gt_Point));
      for (i=0; i<cmd->num_pl_points; i++)
         cmd->phase_line[i] = org->phase_line[i];
   }

   cmd->num_path_points = org->num_path_points;
   if( cmd->num_path_points == 0 )
      cmd->path = NULL;
   else
   {
      cmd->path = (gt_Point *)malloc(cmd->num_path_points*sizeof(gt_Point));
      for (i=0; i < cmd->num_path_points; i++)
         cmd->path[i] = org->path[i];
   }

   return cmd;
}


/**********************************************************************
 **          gt_free_command           **
 ****************************************/

void gt_free_command(gt_Command *cmd)
{
   /* if it is already NULL, just return */
   if (cmd == NULL)
      return;

   /* free all the strings first */
   null_out(cmd->phase_line_ack_msg);
   null_out(cmd->completion_msg);

   /* delete the arrays */
   if ((cmd->num_path_points != 0) && (cmd->path != NULL))
      free(cmd->path);
   if ((cmd->unit_size != 0) && (cmd->unit != NULL))
      free(cmd->unit);
   if ((cmd->num_pl_points != 0) && (cmd->phase_line != NULL))
      free(cmd->phase_line);

   /* now free the command itself */
   free(cmd);
}



/**********************************************************************
 **         gt_specify_offsets         **
 ****************************************/

void gt_specify_offsets(float dx, float dy)
{
   start_x_offset = dx;
   start_y_offset = dy;
}



/**********************************************************************
 **        gt_specify_formation        **
 ****************************************/

void gt_specify_formation(gt_Formation form)
{
   formation = form;
}


/**********************************************************************
 **        gt_specify_technique        **
 ****************************************/

void gt_specify_technique(gt_Technique tech)
{
   technique = tech;
}



/**********************************************************************
 **          gt_specify_speed          **
 ****************************************/

void gt_specify_speed(float v)
{
   speed = v;
}



/**********************************************************************
 **       gt_specify_phase_line        **
 ****************************************/

void gt_specify_phase_line(char *name, char *date, char *time)
{
   phase_line = strdup(name);
   if (date)
      phase_line_date = strdup(date);
   if (time)
      phase_line_time = strdup(time);
}


/**********************************************************************
 **     gt_specify_phase_line_msg      **
 ****************************************/
void gt_specify_phase_line_msg(char *ack_msg)
{
   if (ack_msg)
      acknowledge_message = strdup(ack_msg);
}


/**********************************************************************
 **     gt_specify_phase_line_wait     **
 ****************************************/

void gt_specify_phase_line_wait(int wait_at_line)
{
   wait_at_phase_line = wait_at_line;
}


/**********************************************************************
 **       gt_specify_completion        **
 ****************************************/

void gt_specify_completion(char *msg, int freeze)
{
  completion_clause_given = TRUE;
  if (msg)
    completion_message = strdup(msg);
  freeze_on_completion = freeze;
}


/**********************************************************************
 **          gt_specify_until          **
 ****************************************/

void gt_specify_until(char *date_str,
		      char *time_str)
{
   until_clause_given = TRUE;
   if (date_str)
      until_date_str = strdup(date_str);
   if (time_str)
      until_time_str = strdup(time_str);
}




/**********************************************************************
 **      gt_add_command_to_step        **
 ****************************************/

int gt_add_command_to_step(char *unit,
			   gt_Behavior behavior,
			   char *location)
{
  /* add this command to the step */
  switch (behavior) {

   case START:
      return add_start_command(unit, location);
      break;

   case MOVETO:
   case TELEOPERATE:
   case SWEEPTO:
   case FOLLOW:
      return add_movement_command(unit, behavior, location);
      break;

   case OCCUPY:
      return add_occupy_command(unit, location);
      break;

   case STOP:
      return add_stop_command(unit);
      break;

   case QUIT:
      return add_quit_command();
      break;

   case SET:
      return add_set_command((gt_Parameter)atoi(unit), (int)location);
      break;

   case PRINT:
      return add_print_command((gt_Print)atoi(unit));
      break;

   default:
      fprintf(stderr,
	      "gt_add_command_to_step: Illegal command (behavior=%d)\n",
	      behavior);
      break;
      }

   return GT_FAILURE;
}



/**********************************************************************
 **                                                                  **
 **          implement global functions to execute commands          **
 **                                                                  **
 **********************************************************************/



/**********************************************************************
 **     gt_abort_command_execution     **
 ****************************************/

void gt_abort_command_execution()
{
   if (executing_step_list) {
      abort_execution = TRUE;
      if (paused)
	 gt_toggle_pause_execution();
      }
   else
   {
      if ( debug )
      {
          fprintf( stderr, "gt_abort_command_execution - no execution list\n");
      }

      clear_robots();
      gt_rewind_step_list();
   }
}


/**********************************************************************
 **     gt_stop_command_execution      **
 ****************************************/

/* NOTE: Although this function is implemented and the code necessary to
         implement it is in the work procedure, neither is currently used.
	 It was intended for a [Stop] button on the command interface panel
	 which was not implemented for various reasons.  It is left here in
	 case it might be desired in the future.
 */

void gt_stop_command_execution()
{
   if (executing_step_list) {
      stop_execution = TRUE;
      }
   else
      warn_user("No commands are executing!");
}



/**********************************************************************
 **         gt_backup_step_list        **
 ****************************************/

void gt_backup_step_list()
{
   if (current_step) {
      if (current_step->previous_step == NULL) {
	 warn_user("Warning: Now at the beginning of the command list.\n\
         Unable to back up any further!");
	 }
      else {
	 /* normal case */

	 /* if we are executing, back up and execute the previous step */
	 if (executing_step_list) {
	    /* Automatic execution */
	    free_incomplete_commands();
	    some_command_completed = 0;
	    if (immediate_step) {
	       free_step_list(immediate_step);
	       immediate_step = NULL;
	       }
	    current_step = current_step->previous_step;
	    gt_display_step(current_step, EXECUTING_STEP,
			    current_step->next_step);
	    execute_step(current_step, TRUE);
	    executing_single_step = NULL;
	    }
	 else {
	    /* Non-automatic execution */
	    if (immediate_step) {
	       current_step = current_step->previous_step;
	       gt_display_step(immediate_step, EXECUTING_STEP, current_step);
	       }
	    else if (executing_single_step) {
	       current_step = current_step->previous_step;
	       gt_display_step(executing_single_step, EXECUTING_STEP,
			       current_step->next_step);
	       execute_step(current_step, TRUE);
	       }
	    else {
	       /* Nothing is executing */
	       current_step = current_step->previous_step;
	       gt_display_step(NULL, NO_STEP, current_step);
	       }
	    }
	 }
      }
   else
      warn_user("Warning: There is no command list to backup!");
}



/**********************************************************************
 **         gt_forward_step_list       **
 ****************************************/

void gt_forward_step_list()
{
   if (current_step) {
      if (current_step->next_step == NULL) {
	 warn_user("Warning: Now at the end of the command list.\n\
         Unable to move any further forward!");
	 }
      else {
	 /* normal case */

	 /* if we are executing, execute the new step */
	 if (executing_step_list) {
	    /* Automatic execution */
	    free_incomplete_commands();
	    some_command_completed = 0;
	    if (immediate_step) {
	       free_step_list(immediate_step);
	       immediate_step = NULL;
	       }
	    current_step = current_step->next_step;
	    gt_display_step(current_step, EXECUTING_STEP,
			    current_step->next_step);
	    execute_step(current_step, TRUE);
	    executing_single_step = NULL;
	    }
	 else {
	    /* Non-automatic execution */
	    if (immediate_step) {
	       current_step = current_step->next_step;
	       gt_display_step(immediate_step, EXECUTING_STEP, current_step);
	       }
	    else if (executing_single_step) {
	       current_step = current_step->next_step;
	       gt_display_step(executing_single_step, EXECUTING_STEP,
			       current_step->next_step);
	       execute_step(current_step, TRUE);
	       }
	    else {
	       /* Nothing is executing */
	       current_step = current_step->next_step;
	       gt_display_step(NULL, NO_STEP, current_step);
	       }
	    }
	 }
      }
   else
      warn_user("Warning: There is no command list to forward!");
}



/**********************************************************************
 **         gt_clear_step_list         **
 ****************************************/

void gt_clear_step_list()
{
   if ((current_step != NULL) || (step_list != NULL)) {
      free_step_list(step_list);
      step_list = NULL;
      current_step = NULL;
      if (immediate_step) {
	 free_step_list(immediate_step);
	 immediate_step = NULL;
	 }
      executing_single_step = NULL;
      gt_display_step(NULL, NO_STEP, NULL);
      }
}



/**********************************************************************
 **        gt_execute_step_list        **
 ****************************************/

void gt_execute_step_list()
{
   /* check for possible problems */
   if (current_step == NULL) {
      warn_user("Error: There is no current step to execute!\n\
       (Try loading commands or rewinding the command list.)");
      return;
      }

   /* start the workproc to automatically execute the commands */
   if (immediate_step) {
      free_step_list(immediate_step);
      immediate_step = NULL;
      }
   executing_step_list = TRUE;
   executed_first_step = FALSE;
   abort_execution = FALSE;
   stop_execution = FALSE;
   free_incomplete_commands();
   add_workproc((XtWorkProc)execute_step_list_workproc, NULL);
}



/**********************************************************************
 **        gt_execute_next_step        **
 ****************************************/

void gt_execute_next_step()
{
   if (current_step) {
      if (executing_step_list) {

	 /* We are automatically executing the step list, so cue the work
	    procedure execute_step_list_workproc() to go on to the next
	    step. */

	 continue_with_next_step = TRUE;
	 }
      else {

	 /* No automatic execution, do a manual single step here */

	 /* Clear up any immediate commands */
	 if (immediate_step) {
	    free_step_list(immediate_step);
	    immediate_step = NULL;
	    }

	 /* If already executing a step manually, position to the next step */
	 if (executing_single_step) {
	    current_step = current_step->next_step;
	    executing_single_step = NULL;
	    }

	 /* Display the step and then execute it */
	 gt_display_step(current_step, EXECUTING_STEP,
			 current_step->next_step);
	 if (execute_step(current_step, FALSE) == GT_FAILURE) {
	    gt_display_step(current_step, FAILED_STEP,
			    current_step->next_step);
	    executing_single_step = NULL;
	    }
	 else
	    executing_single_step = current_step;
	 }
      }
   else {
      warn_user("Error: There is no current step to execute!\n\
       (Try loading commands or rewinding the command list.)");
      }
}



/**********************************************************************
 **      gt_execute_user_command       **
 ****************************************/

void gt_execute_user_command(char *cmd)
{
   int result;

   /* initialize the immediate command */
   start_immediate_command();

   /* parse this command */
   strcpy(cmdl_command_buff, "IMMEDIATE COMMAND:");
   strcat(cmdl_command_buff, cmd);
   cmdl_command_buff_pos = 0;
   cmdl_reading_string = TRUE;
   result = cmdl_parse();
   cmdl_file_reset();
   cmdl_reading_string = FALSE;

   /* If there was an error, complain */
   if (result != 0) {
      warn_user("Error: Failure executing user command.");
      return;
      }

   /* Execute this immediate command.
      Note that some commands may have an immediate effect and do not create
      a command structure.  Therefore check for the command before executing
      it! */
   if (immediate_step->command) {
      if (executing_step_list) {
	 if ((immediate_step->command->behavior != SET) &&
	     (immediate_step->command->behavior != PRINT)) {
	    /* We are executing steps, and the new command will
	       affect the robots, so clean up first */
	    free_incomplete_commands();
	    some_command_completed = 0;
	    gt_display_step(immediate_step, EXECUTING_STEP,
			    current_step->next_step);
	    execute_step(immediate_step, TRUE);
	    executing_single_step = NULL;
	    }
	 else
	    execute_step(immediate_step, FALSE);
	 }
      else {
	 /* otherwise, just execute the command */
	 current_step = current_step->next_step;
	 gt_display_step(immediate_step, EXECUTING_STEP, current_step);
	 execute_step(immediate_step, FALSE);
	 executing_single_step = NULL;
	 }
      }
}



/**********************************************************************
 **         gt_rewind_step_list        **
 ****************************************/

void gt_rewind_step_list()
{
   if (executing_step_list) {
      warn_user(
	 "Warning: Cannot rewind commands while commands are executing!");
      }
   else {
      current_step = step_list;
      gt_display_step(NULL, NO_STEP, current_step);
      }
}



/**********************************************************************
 **     gt_toggle_pause_execution      **
 ****************************************/

void gt_toggle_pause_execution()
{
   paused = !paused;
   update_pause_button();

   send_robots_pause_state(paused);
}

/**********************************************************************
 **     gt_toggle_feedback      **
 ****************************************/

void gt_toggle_feedback()
{
    gStopRobotFeedback = !gStopRobotFeedback;

    update_feedback_button();

    send_robots_feedback_state(gStopRobotFeedback);
}

/**********************************************************************
 **                                                                  **
 **               implement global functions for units               **
 **                                                                  **
 **********************************************************************/

/**********************************************************************
 **            gt_new_unit             **
 ****************************************/

gt_Unit *gt_new_unit(
    int id,
    char *name,
    gt_Robot type,
    gt_Unit *children,
    gt_Unit *next)
{
    /* Create a new "unit" using the specified data.
       It is up to the user to "free" it. */
    gt_Unit *tmp = (gt_Unit *) malloc(sizeof(gt_Unit));


    if (tmp)
    {
        tmp->id = id;

        if (name)
        {
            tmp->name = strdup(name);
        }
        else
        {
            tmp->name = NULL;
        }

        tmp->type = type;
        tmp->children = children;
        tmp->next = next;

        return tmp;
    }
    else
    {
        fprintf(stderr, "Ran out of memory creating a new unit (%s)\n", name);
        return NULL;
    }
}


/**********************************************************************
 **          gt_print_unit             **
 ****************************************/

void gt_print_unit(gt_Unit *unit)
{
   printf("\nUNIT %s ", unit->name);
   strcpy(indent, "   ");
   print_subunit(unit);
   printf("\n\n");
}



/**********************************************************************
 **                                                                  **
 **                    implement local functions                     **
 **                                                                  **
 **********************************************************************/

/**********************************************************************
 **        add_movement_command        **
 ****************************************/

static int add_movement_command(char *unit,
                                gt_Behavior behavior,
                                char *location)
{
    gt_Step *step;
    gt_Command *cmd;
    gt_Measure *measure;

    /* copy special fields to step holder */
    if ((step = setup_step()) == NULL) {
        warn_userf("Error: Failure setting up step for add_movement_command\n\
       (UNIT %s %s %s)", unit, gt_behavior_name[behavior], location);
        return GT_FAILURE;
    }
    step->unit = strdup(unit);
    step->location = strdup(location);
    step->phase_line_name = NULL;

    /* get the location measure */
    measure = gt_get_db_measure(location);
    if (measure == NULL) {
        warn_userf("Error: Failure adding movement command (UNIT %s %s %s):\n\
       Unable to retrieve '%s'.", unit, gt_behavior_name[behavior], location, location);
        return GT_FAILURE;
    }

    /* create the command */
    if ((cmd = gt_new_command()) == NULL) {
        fprintf(stderr,
                "Error adding %s command: Out of memory creating new command!",
                gt_behavior_name[behavior]);
        return GT_FAILURE;
    }

    /* copy the location data into the command */
    switch (behavior) {

    case MOVETO:
    case TELEOPERATE:
    case SWEEPTO:      /* ??? Will probably need more for this later */

        /* copy in the goal */
        gt_center_of_measure(measure, &(cmd->goal));
        break;

    case FOLLOW:

        /* Make sure we have a polyline feature to follow */
        if (measure->data_type != POLYLINE) {
            warn_userf("Error: Failure adding FOLLOW command:\n\
       measure '%s' is not a line feature!",
                       location);
            return GT_FAILURE;
        }

        /* copy the path */
        {
            int i, size;
            gt_Point *pts;

            /* create the array for the path */
            size = measure->data.num_points*sizeof(gt_Point);
            if ((pts = (gt_Point *)malloc(size)) == NULL) {
                fprintf(stderr, "Error adding FOLLOW command: ");
                fprintf(stderr, "Out of memory creating path array!");
                return GT_FAILURE;
            }

            /* copy in the points */
            cmd->num_path_points = measure->data.num_points;
            cmd->path = pts;
            for (i=0; i < (int)(measure->data.num_points); i++) {
                cmd->path[i].x = measure->data.point[i].x;
                cmd->path[i].y = measure->data.point[i].y;
            }
        }
        break;

    default:
        fprintf(stderr, "add_movement_command: ");
        fprintf(stderr, "Error, %s is not a movement behavior for unit %s!\n",
                gt_behavior_name[behavior], unit);
        return GT_FAILURE;
        break;
    }

    /* copy the common information */
    cmd->behavior = behavior;
    cmd->formation = formation;
    cmd->technique = technique;
    cmd->speed = speed;

    /* deal with the phase line, if present */
    if (phase_line) {

        gt_Measure *phase_line_measure;
        int i;

        cmd->phase_line_given = TRUE;
        step->phase_line_name = strdup(phase_line);

        /* take care of the desired arrival time */
        if (phase_line_date || phase_line_time) {
            if ((cmd->time =
                 compute_time(phase_line_date, phase_line_time)) == 0) {
                warn_userf(
                    "Error: Failure parsing phase line date/time string for\n\
       add_movement_command (UNIT %s %s %s)", unit,
                    gt_behavior_name[behavior], location);
                return GT_FAILURE;
            }
        }

        /* get the points for the phase line */
        phase_line_measure = gt_get_db_measure(phase_line);
        if (phase_line_measure == NULL) {
            warn_userf("Error: Failure adding movement command (UNIT %s %s %s):\n\
       Unable to retrieve phase line '%s'.", unit, gt_behavior_name[behavior],
                       location, phase_line);
            return GT_FAILURE;
        }
        cmd->num_pl_points = phase_line_measure->data.num_points;
        cmd->phase_line =
            (gt_Point *)malloc(phase_line_measure->data.num_points *
                               sizeof(gt_Point));
        for (i = 0; i < (int)(phase_line_measure->data.num_points); i++)
            cmd->phase_line[i] = phase_line_measure->data.point[i];

        /* save the other phase line info */
        if (acknowledge_message)
            cmd->phase_line_ack_msg = strdup(acknowledge_message);
        cmd->wait_at_phase_line = wait_at_phase_line;

    }

    /* deal with the on-completion info */
    if (completion_clause_given) {
        cmd->completion_given = TRUE;
        if (completion_message)
            cmd->completion_msg = strdup(completion_message);
        cmd->freeze_on_completion = freeze_on_completion;
    }

    /* install the command */
    step->command = cmd;

    return GT_SUCCESS;
}




/**********************************************************************
 **         add_occupy_command         **
 ****************************************/

static int add_occupy_command(char *unit,
			      char *location)
{
   gt_Step *step;
   gt_Command *cmd;
   gt_Measure *measure;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      warn_userf("Error: Failure setting up step for add_occupy_command\n\
       (UNIT %s OCCUPY %s)", unit, location);
      return GT_FAILURE;
      }
   step->unit = strdup(unit);
   step->location = strdup(location);
   step->phase_line_name = NULL;

   /* get the location measure */
   measure = gt_get_db_measure(location);
   if (measure == NULL) {
      warn_userf("Error: Failure adding occupy command (UNIT %s OCCUPY %s):\n\
       Unable to retrieve location '%s'.", unit, location, location);
      return GT_FAILURE;
      }

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr, "Error adding OCCUPY command: Out of memory!");
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = OCCUPY;
   cmd->formation = formation;
   cmd->technique = technique;

   /* copy in the goal */
   gt_center_of_measure(measure, &(cmd->goal));

   /* deal with the "until" info */
   if (until_clause_given) {
      cmd->until_timeout_given = TRUE;
      if (until_time_str[0] == '-') {
	 /* occupy until a timeout occurs (flag this with negative seconds) */
	 cmd->time = -seconds_from_time(until_time_str);
	 }
      else {
	 /* occupy until a set time */
	 if ((cmd->time = compute_time(until_date_str, until_time_str)) == 0) {
	    warn_userf("Error: Failure parsing UNTIL date/time string for\n\
       add_occupy_command (UNIT %s OCCUPY %s)", unit, location);
	    return GT_FAILURE;
	    }
	 }
      }

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;
}




/**********************************************************************
 **         add_start_command          **
 ****************************************/

static int add_start_command(char *unit,
			     char *location)
{
   gt_Step *step;
   gt_Command *cmd;
   gt_Measure *measure;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      warn_userf("Error: Failure setting up step for add_start_command\n\
       (UNIT %s START %s)", unit, location);
      return GT_FAILURE;
      }
   step->unit = strdup(unit);
   step->location = strdup(location);
   step->phase_line_name = NULL;

   /* get the location measure */
   measure = gt_get_db_measure(location);
   if (measure == NULL) {
      warn_userf("Error: Failure adding START command (UNIT %s START %s):\n\
       Unable to retrieve location '%s'.", unit, location, location);
      return GT_FAILURE;
      }

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr,
	 "Error adding START command: Out of memory creating new command!");
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = START;
   cmd->formation = formation;
   cmd->technique = technique;

   /* copy in the goal */
   gt_center_of_measure(measure, &(cmd->goal));

   /* save the offsets */
   if ((cmd->path = (gt_Point *)malloc(sizeof(gt_Point))) == NULL) {
      fprintf(stderr,
	 "Error adding START command: Out of memory creating offsets!");
      return GT_FAILURE;
      }
   cmd->num_path_points = 1;
   cmd->path[0].x = start_x_offset;
   cmd->path[0].y = start_y_offset;

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;
}



/**********************************************************************
 **          add_stop_command          **
 ****************************************/

static int add_stop_command(char *unit)
{
   gt_Step *step;
   gt_Command *cmd;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      warn_userf("Error: Failure setting up step for add_stop_command\n\
       (UNIT %s STOP)", unit);
      return GT_FAILURE;
      }
   step->unit = strdup(unit);
   step->location = NULL;
   step->phase_line_name = NULL;

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr,
	 "Error adding STOP command: Out of memory creating new command!");
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = STOP;

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;
}


/**********************************************************************
 **          add_quit_command          **
 ****************************************/

static int add_quit_command()
{
   gt_Step *step;
   gt_Command *cmd;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      warn_userf("Error setting up step for add_quit_command");
      return GT_FAILURE;
      }
   step->unit = NULL;
   step->location = NULL;
   step->phase_line_name = NULL;

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr,
	 "Error adding QUIT command: Out of memory creating new command!");
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = QUIT;

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;

}


/**********************************************************************
 **          add_set_command           **
 ****************************************/

static int add_set_command(gt_Parameter parameter, int val)
{
   gt_Step *step;
   gt_Command *cmd;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      switch (gt_parameter_type[parameter]) {

      case BOOLEAN_PARAMETER:
	 warn_userf("Error: Failure setting up step for add_set_command\n\
       (SET %s %s)", gt_parameter_name[parameter], (val ? "ON" : "OFF"));
	 break;

      case NUMBER_PARAMETER:
	 warn_userf("Error: Failure setting up step for add_set_command\n\
       (SET %s %1.8g)", gt_parameter_name[parameter], *((double *)val));
	 break;

      default:
	 warn_userf(
	    "Error: Failure setting up step for add_set_command\n\
       Type error: SET %s (type=%d)",
	    gt_parameter_name[parameter], gt_parameter_type[parameter]);
	 break;
	 }
      return GT_FAILURE;
      }
   step->unit = NULL;
   step->location = NULL;
   step->phase_line_name = NULL;

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr,
	 "Error adding SET %s command: Out of memory creating new command!",
	 gt_parameter_name[parameter]);
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = SET;
   step->parameter_to_set = parameter;
   switch (gt_parameter_type[parameter]) {
   case BOOLEAN_PARAMETER:
      step->parameter_value.i = val;
      break;
   case NUMBER_PARAMETER:
      step->parameter_value.d = *((float*)val);
      break;
   default:
      warn_userf("Error interpreting parameter type for %s (type=%d)",
		 gt_parameter_name[parameter], gt_parameter_type[parameter]);
      return GT_FAILURE;
      break;
      }

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;
}


/**********************************************************************
 **          add_print_command           **
 ****************************************/

static int add_print_command(gt_Print print_what)
{
   gt_Step *step;
   gt_Command *cmd;

   /* copy special fields to step holder */
   if ((step = setup_step()) == NULL) {
      warn_userf(
	 "Error setting up step for add_print_command\n      (PRINT %s)",
	 gt_print_name[print_what]);
      return GT_FAILURE;
      }
   step->unit = NULL;
   step->location = NULL;
   step->phase_line_name = NULL;

   /* create the command */
   if ((cmd = gt_new_command()) == NULL) {
      fprintf(stderr,
	 "Error adding PRINT %s command: Out of memory creating new command!",
	  gt_print_name[print_what]);
      return GT_FAILURE;
      }

   /* install the information */
   cmd->behavior = PRINT;
   step->print_what = print_what;

   /* install the command */
   step->command = cmd;

   return GT_SUCCESS;
}



/**********************************************************************
 **            compute_time            **
 ****************************************/

static time_t compute_time(char *date_str, char *time_str)
{
   time_t now_t;
   struct tm tm;
   struct tm *now;
   int month, day, year, hour, min, sec;
   //int m, yday;
   int relative;
   char tmstr[20];

   /* first get the current time */
   now_t = time(0);
   now = localtime(&now_t);

   /* check to make sure the date is in the right format */
   if (date_str) {
      if ((strlen(date_str) == 8))
	 if ((date_str[2] != '-') || (date_str[5] != '-')) {
	    warn_userf("Error: Format for date string (%s) is incorrect.\n\
       (Format should be 'mm-dd-yy'.)", date_str);
	    return 0;
	    }
      else if ((strlen(date_str) == 7))
	 if ((date_str[1] != '-') || (date_str[4] != '-')) {
	    warn_userf("Error: Format for date string (%s) is incorrect.\n\
       (Format should be 'mm-dd-yy'.)", date_str);
	    return 0;
	    }
      else {
	 warn_userf("Error: Format for date string (%s) is incorrect.\n\
       (Format should be 'mm-dd-yy'.)", date_str);
	 return 0;
         }
      }

   /* parse the date for modifications */
   if (date_str) {
      sscanf(date_str, "%2d-%2d-%2d", &month, &day, &year);
      }
   else {
      // If no date given, then use today
      month = now->tm_mon + 1; // Month is 1 based, tm_mon is 0 based
      day = now->tm_mday;
      year = now->tm_year;
      }

   /* see if it is a relative time string */
   if (time_str[0] == '+') {
      relative = TRUE;
      strcpy(tmstr, &(time_str[1]));
      }
   else {
      relative = FALSE;
      strcpy(tmstr, time_str);
      }

   /* insert a leading '0' if one is missing */
   if  (tmstr[1] == ':') {
      char tmp[20];
      strcpy (tmp, tmstr);
      strcpy(tmstr, "0");
      strcat(tmstr, tmp);
      }

   /* parse the time string */
   if (strlen(tmstr) == 8) {
      sscanf(tmstr, "%d:%d:%d", &hour, &min, &sec);
      }
   else if (strlen(tmstr) == 5) {
      sscanf(tmstr, "%d:%d", &hour, &min);
      sec = 0;
      }
   else if (strchr(tmstr, ':') == NULL) {
      int n;
      sscanf(tmstr, "%d", &n);
      hour = n / 60;
      min = n % 60;
      sec = 0;
      }
   else {
      warn_userf("Error in format of time string (%s)", time_str);
      return 0;
      }

   /* Modify the time if it is a relative time string */
   if (relative) {
      hour += now->tm_hour;
      min += now->tm_min;
      sec += now->tm_sec;
      if (sec > 59) {
	 min += sec / 60;
	 sec = sec % 60;
         }
      if (min > 59) {
	 hour += min / 60;
	 min = min % 60;
         }
      if (hour > 23) {
	 day += hour / 24;
	 hour = hour % 24;
         }
      if (day > month_length(month-1,year)) {
	 day = day % month_length(month-1,year);
	 month += 1;  /* NB: Will break if enough relative time is given to
	                     push this over more than one month. */
         }
      if (month > 12) {
	 year += 1;
         month = 1;
         }
      }

   /* modify the time structure */
   tm.tm_sec = sec;
   tm.tm_min = min;
   tm.tm_hour = hour;
   tm.tm_mday = day;
   tm.tm_mon = month - 1;
   tm.tm_year = year;
   tm.tm_isdst = now->tm_isdst;     /* <- Who knows what this should be! ?? */

#ifdef linux
   return mktime(&tm);

#else // sparc

   tm.tm_zone = now->tm_zone;
   tm.tm_gmtoff = now->tm_gmtoff;

   /* compute the "year day" */
   yday = 0;
   for (m=1; m<month; m++)
      yday += month_length(m-1,year);
   yday += day;

   tm.tm_yday = yday;

   return timelocal(&tm);
#endif
}




/**********************************************************************
 **         seconds_from_time          **
 ****************************************/

static int seconds_from_time(char *time_str)
{
   char tmstr[20];
   int hour, min, sec;

   /* strip of the leading minus, if present */
   if (time_str[0] == '-')
      strcpy(tmstr, &(time_str[1]));
   else
      strcpy(tmstr, time_str);

   /* insert a leading '0' if one is missing */
   if  (tmstr[1] == ':') {
      char tmp[20];
      strcpy (tmp, tmstr);
      strcpy(tmstr, "0");
      strcat(tmstr, tmp);
      }

   /* parse the time string */
   if (strlen(tmstr) == 8) {
      sscanf(tmstr, "%d:%d:%d", &hour, &min, &sec);
      }
   else if (strlen(tmstr) == 5) {
      sscanf(tmstr, "%d:%d", &min, &sec);
      hour = 0;
      }
   else if (strchr(tmstr, ':') == NULL) {
      sscanf(tmstr, "%d", &sec);
      hour = 0;
      min = 0;
      }
   else {
      warn_userf("Error in format of time string (%s)", time_str);
      return 0;
      }

   /* always return a positive number of seconds */
   return (3600*hour + 60*min + sec);
}



/**********************************************************************
 **            execute_step            **
 ****************************************/

static int execute_step(gt_Step *step,
			int wait)
{
   /* if the "wait" flag is set, set things up to
      wait for the completion of this step. */

   gt_Step *sibling;
   gt_Unit_members *members;
   gt_Command *cmd;
   float gx, gy, save_gx, save_gy;
   int i;

   /* Execute this step */
   sibling = step;
   while (sibling) {

      switch (sibling->command->behavior) {

      case NO_BEHAVIOR:
	 break;


      case SET:

	 switch (sibling->parameter_to_set) {

	  case SHOW_TRAILS:
	     set_show_trails(sibling->parameter_value.i);
	     break;

	  case SHOW_REPELLING_OBSTACLES:
	     set_show_repelling_obstacles(sibling->parameter_value.i);
	     break;

 	  case SHOW_MOVEMENT_VECTORS:
	     set_show_movement_vectors(sibling->parameter_value.i);
	     break;

	  case DEBUG_ROBOTS:
	     set_debug_robots(sibling->parameter_value.i);
	     break;

	  case DEBUG_SIMULATOR:
	     set_debug_simulator(sibling->parameter_value.i);
	     break;

	  case DEBUG_SCHEDULER:
	     set_debug_scheduler(sibling->parameter_value.i);
	     break;

	  case CYCLE_DURATION:
	     set_cycle_duration(sibling->parameter_value.d);
	     break;

	  default:
	     break;
	  }

	  break;


      case PRINT:

	 switch (sibling->print_what) {

	  case CONSOLE_DB:
	     gt_print_db();
	     break;

 	  case ROBOT_INFO:
	     gt_print_robot_info();
	     break;

	  default:
	     break;
	  }

	 break;


      case QUIT:
         if ( debug )
         {
             fprintf( stderr, "execute_step: QUIT - clearing robots\n");
         }

	 clear_robots();
	 exit(0);
	 break;

      default:

	 /* make sure we have somewhere to send it!  */
	 if (sibling->unit == NULL) {
	    warn_userf("Error executing step %s, command has no unit!",
		       step->name);
	    return GT_FAILURE;
	    }

	 /* get the list of unit members to send this command to */
	 members =
	    (gt_Unit_members *)gt_get_db_entry(sibling->unit, UNIT_MEMBERS);
	 if (members == NULL) {
	    warn_userf("Error: Failure executing step %s, \n\
       Unable to find members for unit %s!",
		       step->name, sibling->unit);
	    return GT_FAILURE;
	    }

	 /* get the command */
	 cmd = sibling->command;

	 /* Create the unit member list to send with each command */
	 cmd->unit_size = members->num_robots;
	 if (cmd->unit)
 	    free(cmd->unit);
	 cmd->unit = (int *)malloc(members->num_robots * sizeof(int));
	 for (i=0; i<members->num_robots; i++)
	    cmd->unit[i] = members->robot[i].id;

	 /* send the command to each robot in the unit */
	 gx = cmd->goal.x;
	 gy = cmd->goal.y;
	 for (i=0; i<members->num_robots; i++) {

	    /* Make sure the command gets to the right robot */
	    cmd->robot_id = members->robot[i].id;

	    /* if it is a start command, increment locations */
	    if (cmd->behavior == START) {
	       save_gx = cmd->goal.x;
	       save_gy = cmd->goal.y;
	       cmd->goal.x = gx;
	       cmd->goal.y = gy;

	       /* Warn the user if we place a robot inside an obstacle */
	       if (overlap_obstacle(gx, gy))
		   warn_userf("Error: Robot placed inside an obstacle\n\
       (at x=%1.9g, y=%1.9g).  This may make it\n\
       impossible to complete the mission.", gx, gy);
	       }

	    /* play it safe and note that the command hasn't been done yet!
	       (Since it hasn't been sent, any existing completion messages
	       should be deleted so the command won't abort.) */
	    gt_delete(cmd->robot_id, RSP_MSG);

	    /* send command to robots */
	    if (send_robot_command(cmd, members->robot[i].type) != GT_SUCCESS)
	    {
	       warn_userf("Error: Failure executing robot command\n\
       (UNIT %s %s %s for robot_id=%d),\n\
       The robot executable has probably died.", sibling->unit,
			  gt_behavior_name[cmd->behavior], sibling->location,
			  cmd->robot_id);

               // If we are autorunning, then an error starting a robot is fatal
	       if (cmd->behavior == START && auto_run )
	       {
		  die_after_warning = true;
		  if ( debug )
		  {
		      fprintf( stderr, "execute_step - error starting robot: exiting and clearing robots\n");
		  }
		  clear_robots();
		  break;
	       }

	    }

	    /* keep track of commands that require waiting for completion */
	    if (wait) {
	       add_incomplete_command(cmd);
	       }

	    /* if it is a start command, increment locations */
	    if (cmd->behavior == START) {
	       cmd->goal.x = save_gx;
	       cmd->goal.y = save_gy;
	       gx += cmd->path[0].x;
	       gy += cmd->path[0].y;
	       }
	    }

	 /* If it was a teleoperate command, pop up the interface */
	 if (cmd->behavior == TELEOPERATE) {
#ifdef TELOP
	       int *robot_id = (int *)malloc(members->num_robots*sizeof(int));
	       /* Copy the robot IDs to send with the telop command */
	       for (i=0; i<members->num_robots; i++)
		  robot_id[i] = members->robot[i].id;

	       /* Teleoperate! */
	       if (telop_num_robots == 0)
		  gt_popup_telop_interface(step->unit, members->num_robots,
					   robot_id);
	       else {
		  warn_userf("Error: Unable to activate teleoperation!\nAlready teleoperating unit '%s'", telop_unit_name);
		  }

	       free(robot_id);
#else
	       warn_userf("Error: Unable to execute TELEOPERATION command!\nTeleoperation code is disabled.");
#endif
	    }
         }

       sibling = sibling->next;
       }

    return GT_SUCCESS;
}


/**********************************************************************
 **     execute_step_list_workproc     **
 ****************************************/

int
execute_step_list_workproc()
{
   int cmd_done;

   /* check to see if execution should be aborted */
   if (abort_execution) {
      free_incomplete_commands();
      some_command_completed = 0;
      executing_step_list = FALSE;
      gt_rewind_step_list();
      { /* tell all the robots to stop */
	 gt_Command *cmd = gt_new_command();
	 cmd->robot_id = -1;  /* all robots */
	 cmd->behavior = STOP;
	 send_robot_command(cmd, 0);
	 gt_free_command(cmd);
      }
      return TRUE;
      }

   /* check to see if execution should be stopped */
   if (stop_execution) {  /* See note at gt_stop_command_execution */
      /* clear the current incomplete step */
      free_incomplete_commands();
      some_command_completed = 0;
      executing_step_list = FALSE;

      /* set up to execute this step again */
      gt_display_step(NULL, NO_STEP, current_step);
      stop_execution = FALSE;
      return FALSE;
      }

   /* on the first time through, execute the current step */
   if (!executed_first_step) {
      executed_first_step = TRUE;
      if (immediate_step) {
	 free_step_list(immediate_step);
	 immediate_step = NULL;
	 }
      if (executing_single_step) {
	 current_step = current_step->next_step;
	 executing_single_step = NULL;
	 }
      gt_display_step(current_step, EXECUTING_STEP, current_step->next_step);
      execute_step(current_step, TRUE);
      return FALSE;
      }

   /* Force completion of this step and go on.  (User pressed [Single Step]
      button while automatically executing step list) */
   if (continue_with_next_step)
   {
      /* clear the current incomplete step */
      free_incomplete_commands();
      some_command_completed = 0;
      continue_with_next_step = FALSE;

      /* execute the next step */
      current_step = current_step->next_step;
      if (immediate_step)
      {
	 free_step_list(immediate_step);
	 immediate_step = NULL;
      }
      if( current_step )
      {
         gt_display_step(current_step, EXECUTING_STEP, current_step->next_step);
         execute_step(current_step, TRUE); /* Test result for proper exec? */
      }
      executing_single_step = NULL;
      return FALSE;
   }

   /* check for completed commands */
   if ((incomplete_commands != NULL) && some_command_completed)
   {
      gt_Incomplete_command *inc_cmd;
      char *result;

      /* check the console database to remove the completed commands */
      inc_cmd = incomplete_commands;
      cmd_done = FALSE;
      while (inc_cmd)
      {
	 /* get the value for the key and remove it from the database */
	 result = gt_get_value(inc_cmd->command->robot_id, RSP_MSG);

	 if (result != NULL)
	 {
	    if (strncmp(result, DONE_MSG, strlen(DONE_MSG)) == 0)
	    {
	       if (debug)
		  fprintf(stderr, "Robot %d completed %s command\n",
			  inc_cmd->command->robot_id,
			  gt_behavior_name[inc_cmd->command->behavior]);

	       // NOTE: INVALIDATES inc_cmd!
	       remove_incomplete_command(inc_cmd->command->robot_id);
	       inc_cmd = NULL;

               cmd_done = TRUE;
	    }
	    else if (debug)
	    {
	       fprintf(stderr,
		       "Command (UNIT ? %s) for robot_id=%d status='%s'\n",
		       gt_behavior_name[inc_cmd->command->behavior],
		       inc_cmd->command->robot_id, result);
	    }

            /* need to free the value string */
	    free(result);
         }
	 else if ((inc_cmd->command->behavior == OCCUPY) &&
		  (inc_cmd->command->phase_line_given))
	 {

	    /* We've got an unremoved OCCUPY command that was completed
	       earlier.  The proceed dialog box has been popped up.
	       Try to remove it again.  (Note the reused variable:
	       phase_line_given -- means a dialog box has been popped up.)
	     */

	    // NOTE: INVALIDATES inc_cmd!
	    remove_incomplete_command(inc_cmd->command->robot_id);
	    inc_cmd = NULL;

	    cmd_done = TRUE;
	 }

         /* Stop when first command finishes */
	 if( cmd_done )
	    break;

         /* check the next command */
	 inc_cmd = inc_cmd->next;
      }

      /* if we've finished this step, start another! */
      if (incomplete_commands == NULL) {
	 if (debug)
	    fprintf(stderr,"Current Step Complete\n");
	 if (current_step->next_step == NULL) {
	    /* we finished executing the commands!
	       (Reset things and kill this workproc) */
	    executing_step_list = FALSE;
	    gt_rewind_step_list();
	    return TRUE;
	    }
	 else {
	    /* execute the next step */
	    current_step = current_step->next_step;
	    if (immediate_step) {
	       free_step_list(immediate_step);
	       immediate_step = NULL;
	       }
	    gt_display_step(current_step, EXECUTING_STEP,
			    current_step->next_step);
	    execute_step(current_step, TRUE);
	    executing_single_step = NULL;
	    }
         }
      }

   return FALSE;
}





/**********************************************************************
 **           free_step_list           **
 ****************************************/

static void free_step_list(gt_Step *step_list)
{
   gt_Step *step, *next_step, *cmd, *next_cmd;

   /* if it is already NULL, just return */
   if (step_list == NULL)
      return;

   /* free the storage for each step */
   step = step_list;
   while (step) {
      /* free the storage for all the commands for this step */
      cmd = step;
      next_step = step->next_step;
      while (cmd) {
	 next_cmd = cmd->next;
	 free_step(cmd);   /* Note that this frees step_list too! */
	 cmd = next_cmd;
	 }
      step = next_step;
      }
}


/**********************************************************************
 **            month_length            **
 ****************************************/

static int month_length(int month, int year)
{
  /* Return the length of this month in days.
     NOTE WELL: January=0, Feb=1, etc */

   int length;

   /* Do the old rhyme! */
   switch (month) {
   case 1:            /* February */
#ifdef linux
     if( year == 1996 || year == 2004 || year == 2008 )
#else
     if (dysize(year) == 366)
#endif
       length = 29;
     else
       length = 28;
     break;

   case 3:	      /* April */
   case 5:	      /* June */
   case 8:	      /* September */
   case 10:	      /* November */
     length = 30;
     break;

   default:	      /* All the rest! */
     length = 31;
     break;
   }

   return length;
}



/**********************************************************************
 **             setup_step             **
 ****************************************/

static gt_Step *setup_step()
{
   /* Set up the step for the add_* commands */

   gt_Step *step;

   /* Set up to install the new command */
   if (immediate_command_mode) {

      /* make sure we have somewhere to put it */
      if (immediate_step == NULL)
	 start_immediate_command();

      /* figure out where this command goes */
      step = immediate_step;
      }
   else {

      /* make sure we have something to work with */
      if (current_step == NULL) {
	 fprintf(stderr, "setup_step: Error!  No current step\n");
	 return NULL;
         }

      /* figure out where this command goes */
      step = current_step;
      }

   /* find the last sibling in this step */
   while (step->next)
      step = step->next;

   /* if this step has a command already, append a new sibling */
   if (step->command) {
      step->next = new_step(NULL, NULL, NULL, NULL, NULL);
      step = step->next;
      }

   return step;
}



/**********************************************************************
 **       start_immediate_command      **
 ****************************************/

void start_immediate_command()
{
   /* free the old immediate command */
   if (immediate_step) {
      free_step_list(immediate_step);
      immediate_step = NULL;
      }

   /* Note that we are constructing an immediate command */
   immediate_command_mode = TRUE;

   /* Create the step */
   immediate_step = new_step("", NULL, NULL, NULL, NULL);
}


/**********************************************************************
 **       add_incomplete_command       **
 ****************************************/

static void add_incomplete_command(gt_Command *cmd)
{
   gt_Incomplete_command *inc_cmd;

   /* find the end of the list and add a holder for the command */
   if (incomplete_commands == NULL) {
      incomplete_commands =
         (gt_Incomplete_command *)malloc(sizeof(gt_Incomplete_command));
      inc_cmd = incomplete_commands;
      }
   else {
      inc_cmd = incomplete_commands;
      while (inc_cmd->next)
	 inc_cmd = inc_cmd->next;
      inc_cmd->next =
	 (gt_Incomplete_command *)malloc(sizeof(gt_Incomplete_command));
      inc_cmd = inc_cmd->next;
      }

   /* add the command
    * DCM: make a copy since the caller may change their source and we only
    *      keep a pointer to it.
    */
   inc_cmd->command = gt_copy_command(cmd);

   inc_cmd->next = NULL;
}



/**********************************************************************
 **         find_relevant_step         **
 ****************************************/

static gt_Step *find_relevant_step(gt_Command *cmd)
{
   /* find the step (sibling holder) for this command in the current_step.
      Note that this is the only way to get the unit and location name
      strings. */

   gt_Step *step;
   int i;

   /* scan through the substeps in the current_step */
   step = current_step;
   while (step) {

      /* see if the commands have the same behavior */
      if (step->command->behavior == cmd->behavior)
	 /* scan through the unit list and see if this robot is part of unit */
	 for (i=0; i<step->command->unit_size; i++)
	    /* if the robot is in this unit, return this step */
	    if (step->command->unit[i] == cmd->robot_id)
	       return step;

      step = step->next;
      }

   fprintf(stderr, "Error: find_relevant_step failed\n");
   return NULL;
}



/**********************************************************************
 **      free_incomplete_commands      **
 ****************************************/

static void free_incomplete_commands()
{
   gt_Incomplete_command *inc_cmd;

   while (incomplete_commands) {
      inc_cmd = incomplete_commands;
      incomplete_commands = incomplete_commands->next;
      gt_free_command(inc_cmd->command);
      free(inc_cmd);
      }
}



/**********************************************************************
 **      occupy_command_deletable      **
 ****************************************/

static int occupy_command_deletable(gt_Command *cmd)
{
   /* Determine if this command can be deleted from the incomplete_commands
      list.  Usually they can, but OCCUPY commands waiting on response from
      the dialog box cannot, until the user responds. */

   gt_Incomplete_command *inc_cmd;
   gt_Step *step;
   int i;

   /* Scan the list of incomplete commands to see if there are any other
      OCCUPY commands for this unit.  If there are, this command can be
      deleted.  The last one can deal with notifying the user through the
      proceed dialog box. */

   inc_cmd = incomplete_commands;
   while (inc_cmd) {

      /* if this command is an OCCUPY command, check it */
      if ((inc_cmd->command->behavior == OCCUPY) &&
	  (inc_cmd->command->robot_id != cmd->robot_id)) {
	 for (i=0; i<cmd->unit_size; i++) {
	    /* scan through the unit to see if this command belongs to unit */
	    if (inc_cmd->command->robot_id == cmd->unit[i]) {
	       /* found another incomplete OCCUPY command for this unit */
	       return TRUE;
	       }
	    }
	 }

      inc_cmd = inc_cmd->next;
      }

   /* There are no other incomplete occupy commands for this unit.
      See if a proceed dialog has already been popped up.

      NOTE the reused variables:
         phase_line_given -- means a dialog box has been popped up
         wait_at_phase_line -- a flag which is TRUE when the dialog
	                       box has been dismissed, and -1 if the
			       user asked to ABORT.
    */

   if (cmd->phase_line_given) {
      /* the dialog box was popped up, check the completion flag */
      if (cmd->wait_at_phase_line == 0) {
	 /* No user response yet, so this occupy is not complete
	    therefore don't delete the command. */
	 return FALSE;
	 }
      else if (cmd->wait_at_phase_line == -1) {
	 /* The user asked to abort */
	 abort_execution = TRUE;
	 return TRUE;
	 }
      else {
	 /* normal [okay] completion, so this command can now be deleted */
	 return TRUE;
	 }
      }
   else {
      /* The dialog box hasn't been popped up yet, so do it. */
      step = find_relevant_step(cmd);
      cmd->wait_at_phase_line = 0;
      get_proceed_clearance(&(cmd->wait_at_phase_line),
			    cmd->until_timeout_given, cmd->time,
			    "Unit \"%s\" is occupying %s.  Proceed?",
			    step->unit, step->location);
      cmd->phase_line_given = TRUE;
      return FALSE;
      }

   return FALSE;
}




/**********************************************************************
 **     remove_incomplete_command      **
 ****************************************/

static void remove_incomplete_command(int robot_id)
{
   gt_Incomplete_command *prev_inc_cmd, *inc_cmd;

   /* check for an empty list of incomplete commands */
   if (incomplete_commands == NULL) {
      warn_userf("Error: Attempt to remove incomplete command for\n\
       robot_id=%d from empty incomplete command list!");
      return;
      }

   /* see if the command is the first one */
   inc_cmd = incomplete_commands;
   if (inc_cmd->command->robot_id == robot_id)
   {

      /* found it! now delete it */
      if (inc_cmd->command->behavior == OCCUPY) {
	 if (occupy_command_deletable(inc_cmd->command)) {
	    incomplete_commands = incomplete_commands->next;
	    gt_free_command(inc_cmd->command);
	    free(inc_cmd);
	    some_command_completed--;
	    }
	 }
      else {
	 incomplete_commands = incomplete_commands->next;
	 gt_free_command(inc_cmd->command);
	 free(inc_cmd);
	 some_command_completed--;
	 }
      return;
   }

   /* It isn't the first one, so scan the rest of the list for the command */
   prev_inc_cmd = inc_cmd;
   inc_cmd = incomplete_commands->next;
   while (inc_cmd)
   {
      if (inc_cmd->command->robot_id == robot_id)
      {
	 /* found it! now delete it */
	 if (inc_cmd->command->behavior == OCCUPY) {
	    if (occupy_command_deletable(inc_cmd->command)) {
	       prev_inc_cmd->next = inc_cmd->next;
	       gt_free_command(inc_cmd->command);
	       free(inc_cmd);
	       some_command_completed--;
	       }
	    }
	 else {
	    prev_inc_cmd->next = inc_cmd->next;
	    gt_free_command(inc_cmd->command);
	    free(inc_cmd);
	    some_command_completed--;
	    }
	 return;
      }

      prev_inc_cmd = inc_cmd;
      inc_cmd = inc_cmd->next;
   }

   /* check for an empty list of incomplete commands */
   warn_userf("Error: Attempt to remove incomplete command\n\
       for robot_id=%d which does not exist!");
}



/**********************************************************************
 **           print_subunit            **
 ****************************************/

static void print_subunit(gt_Unit *unit)
{
   gt_Unit *sibling;

   if (unit == 0)
      return;

   if (unit->type == UNIT) {
      sibling = unit;
      while (sibling) {
	 printf("\n%s", indent);
	 if (sibling->name)
	    printf("%s",sibling->name);
	 else
	    printf("?");
	 strcat(indent, "   ");
	 print_subunit(sibling->children);
	 indent[strlen(indent)-3] = '\0';
	 sibling = sibling->next;
	 }
      }
   else {
      sibling = unit;
      while (sibling) {
	 if (sibling->name)
	    printf(" (%s %s-%d)", sibling->name,
		   gt_robot_info[sibling->type].name, sibling->id);
	 else
	    printf(" %s-%d", gt_robot_info[sibling->type].name, sibling->id);
	 sibling = sibling->next;
	 }
      }
}



/**********************************************************************
 **             free_step              **
 ****************************************/

static void free_step(gt_Step *step)
{
   /* if it is already NULL, just return */
   if (step == NULL)
      return;

   /* free the strings */
   null_out(step->name);
   null_out(step->unit);
   null_out(step->location);
   null_out(step->phase_line_name);

   /* free the associate command */
   gt_free_command(step->command);
   step->command = NULL;

   /* free the step itself */
   free(step);
}


/**********************************************************************
 **              new_step              **
 ****************************************/

static gt_Step *new_step(char *name,
			 gt_Command *cmd,
			 gt_Step *next,
			 gt_Step *next_step,
			 gt_Step *previous_step)
{
   gt_Step *step;

   /* create the new step */
   if ((step = (gt_Step *)malloc(sizeof(gt_Step))) == NULL) {
      fprintf(stderr,"new_step: Error, ran out of memory creating new step\n");
      return NULL;
      }

   /* copy the name and install it */
   if (name == NULL)
      step->name = NULL;
   else {
      if ((step->name = strdup(name)) == NULL) {
	 fprintf(stderr, "new_step: Error, ran out of memory ");
	 fprintf(stderr, "copying name for new step\n");
	 return NULL;
	 }
      }

   /* install the other specified values */
   step->command = cmd;
   step->next = next;
   step->next_step = next_step;
   step->previous_step = previous_step;

   /* set defaults on the rest */
   step->unit = NULL;
   step->location = NULL;
   step->phase_line_name = NULL;
   step->parameter_to_set = NO_PARAMETER;
   step->parameter_value.i = FALSE;

   return step;
}


/**********************************************************************
 * $Log: gt_command.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.3  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/07/31 03:40:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.100  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.99  2002/07/18 17:13:29  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.98  2000/10/16 19:39:40  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.97  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.96  1996/02/29  01:53:18  doug
 * *** empty log message ***
 *
 * Revision 1.95  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.94  1996/02/25  18:01:17  doug
 * tried to fix a bug which allowed single stepping off the end of the list
 *
 * Revision 1.93  1995/07/06  20:33:53  jmc
 * Fixed things so that the telop stuff can be disabled.
 *
 * Revision 1.92  1995/06/16  16:54:35  jmc
 * Fixed free_step_list to avoid freeing a block twice.
 *
 * Revision 1.91  1995/06/14  17:16:42  jmc
 * Minor cosmetic cleanup.
 *
 * Revision 1.90  1995/05/19  14:25:46  jmc
 * Fixed a problem with gt_backup_step_list and gt_forward_step_list
 * to make them work correctly while single stepping.
 *
 * Revision 1.89  1995/05/18  21:18:45  jmc
 * Added call to update_pause_button() to gt_toggle_pause_execution()
 * to make sure that the button label stays syncronized.
 *
 * Revision 1.88  1995/05/08  20:13:27  jmc
 * Added some code to keep TELOP from being invoked when it was
 * already being used to control a unit.
 *
 * Revision 1.87  1995/05/08  16:06:04  jmc
 * Improved an error message.
 *
 * Revision 1.86  1995/05/03  19:20:43  jmc
 * Added include of telop.h.  Added code to the command dispatch that
 * pops up the telop interface to send the additional unit stuff that
 * it needs.
 *
 * Revision 1.85  1995/04/26  18:54:12  jmc
 * Added Teleoperate stuff.  Modified execute_step to pop up the
 * teleoperation interface when executing that behavior.
 *
 * Revision 1.84  1995/04/20  17:46:34  jmc
 * Corrected spelling.
 *
 * Revision 1.83  1995/04/19  21:53:32  jmc
 * Fixed an insignificant problem that was pointed out by the latest
 * g++ compiler in -Wall mode.
 *
 * Revision 1.82  1995/04/19  18:34:08  jmc
 * Improved comments here and there.
 *
 * Revision 1.81  1995/04/17  18:54:57  jmc
 * Improved comments here and there.  Reworked many warn_userf
 * messages to make them display a little more nicely.
 *
 * Revision 1.80  1995/04/17  15:33:05  jmc
 * Fixed a bug that prevented the [Single Step] command followed by
 * [Execute Commands] from working properly.  Improved some comments
 * on related variables.
 *
 * Revision 1.79  1995/04/03  18:14:15  jmc
 * Added copyright notice.
 *
 * Revision 1.78  1995/03/30  19:54:19  doug
 * removed gt_std include
 *
 * Revision 1.77  1995/02/15  19:26:49  jmc
 * Several modifications to handle the new SWEEPTO command.
 *
 * Revision 1.76  1995/02/13  20:43:58  jmc
 * Added default case to a couple switch statements.
 *
 * Revision 1.75  1995/02/13  18:51:58  jmc
 * Modified several functions to convert parameter usage to allow
 * parameters to be either boolean or numbers (float point).
 *
 * Revision 1.74  1995/02/02  21:36:44  jmc
 * When aborting, if paused, unpause so ABORT will take effect.
 *
 * Revision 1.73  1995/02/02  16:39:03  jmc
 * Added SHOW_MOVEMENT_VECTORS case to execute_step.
 *
 * Revision 1.72  1995/01/30  15:45:20  doug
 * porting to linux
 *
 * Revision 1.71  1995/01/26  21:26:59  jmc
 * Narrowed some error messages to avoid cropping problems on some
 * systems.
 *
 * Revision 1.70  1994/11/21  17:57:47  jmc
 * Removed interface-type checks from gt_execute_step_list and moved
 * them to the calling function in gt_command_panel.c.
 *
 * Revision 1.69  1994/11/15  21:50:25  jmc
 * Fixed gt_clear_step_list to clear up the immediate_step and
 * executing_single_step to clear all execution things.
 *
 * Revision 1.68  1994/11/14  20:06:38  jmc
 * Fixed a small problem that my previous bullet-proofing raised.
 *
 * Revision 1.67  1994/11/14  15:28:23  jmc
 * Numerous modifications to make the next step window work.  This
 * includes more careful recording of what is executing (immediate
 * command, single step command, or automatic execution) as well as
 * various bug fixes.
 *
 * Revision 1.66  1994/11/11  21:30:14  jmc
 * Minor readability hacks.
 *
 * Revision 1.65  1994/11/11  15:47:30  jmc
 * Made a couple of print statements to print only in debug mode to
 * try and quieten program operation.
 *
 * Revision 1.64  1994/11/08  18:38:37  jmc
 * Minor fixes to gt_execute_user_command to make it handle immediate
 * commands better when the system is executing commands.
 *
 * Revision 1.63  1994/11/07  21:55:51  jmc
 * Modified gt_forward_step_list and gt_backward_step_list to work
 * properly if things are executing.
 *
 * Revision 1.62  1994/11/07  21:33:11  jmc
 * Added a warning message if a robot is placed inside an obstacle.
 *
 * Revision 1.61  1994/11/04  15:28:03  jmc
 * Added case DEBUG_SCHEDULER: to execute_step.
 *
 * Revision 1.60  1994/11/03  23:32:43  jmc
 * Added a few checks for NULL current_step.  Check for the return
 * status of execute_step when single stepping.  Need to do it
 * everywhere.  Added some stuff to stop execution.  Work in progress.
 * Added SHOW_REPELLING_OBSTACLES case to the execute set command
 * code.
 *
 * Revision 1.59  1994/10/31  20:25:20  jmc
 * Fixed a typo.
 *
 * Revision 1.58  1994/10/31  19:50:50  jmc
 * Changed a couple of print on debug statements to print all the
 * time for debugging purposes.
 *
 * Revision 1.57  1994/10/31  17:17:51  jmc
 * Fixed manual command entry while executing a step list: Now it
 * (should) abort the current step, execute the manual step, and then
 * continue with the next step.
 *
 * Revision 1.56  1994/10/31  16:39:32  jmc
 * Changed warn_userf message for out-of-memory conditions to
 * fprintf(stderr,...) commands since it is unlikely that warn_userf
 * would work with no memory.  Converted some informational fprintfs
 * to print only in debug mode.
 *
 * Revision 1.55  1994/10/28  21:08:21  jmc
 * Changes because of changes in gt_Command (deleted until_msg,
 * converted until_given to until_timeout_given).  Changed
 * gt_specify_until to use the new scheme.
 *
 * Revision 1.54  1994/10/27  15:47:09  jmc
 * Modified execute_next_step and execute_step_list_workproc for
 * [Single Step] to work during executing a step list.
 *
 * Revision 1.53  1994/10/25  16:24:30  jmc
 * Changed wait argument in gt_specify_phase_line_wait to
 * wait_at_line to avoid conflict with some global.
 *
 * Revision 1.52  1994/10/25  16:07:41  jmc
 * Added PRINT to gt_add_command_to_step and changed the order of SET
 * and QUIT in the same.  Added function add_print_command.  Added
 * PRINT case to execute_step.  Modified send_robot_command to send
 * robot type.  Modified several references to the obsolete
 * gt_robot_name to use the robot info database (gt_robot_info).
 * Modified includes to fix various problems.
 *
 * Revision 1.51  1994/10/18  22:38:48  jmc
 * Converted executing_step_list from local to global.  Removed
 * warning not to invoke this (when executing_step_list) from
 * gt_abort_command_execution and moved it to gt_command_panel.c.
 *
 * Revision 1.50  1994/10/18  20:20:14  jmc
 * Added include of stdlib.h to quiet warnings about exit().  Added
 * the global function gt_clear_step_list so that clear_... functions
 * can use it.  Added code to the abort parts of the execute workproc
 * to rewind the command list.
 *
 * Revision 1.49  1994/10/12  14:36:47  jmc
 * Minor tweak to get_proceed_clearance message.
 *
 * Revision 1.48  1994/10/12  01:01:58  jmc
 * Various modifications to get the OCCUPY command to pop up the
 * proceed dialog box.  Modified functions execute_step_list_workproc
 * and remove_incomplete_command.  Added functions find_relevant_step
 * and occupy_command_deletable.  Reordered a couple of functions.
 *
 * Revision 1.47  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.46  1994/10/10  15:05:07  jmc
 * Minor tweaks to gt_new_command and gt_copy_command.
 *
 * Revision 1.45  1994/10/06  20:03:58  jmc
 * Added free_step, gt_free_command.  Changed a number of statements
 * to use them.  Changed gt_dup_command to gt_copy_command.  Added
 * code to delete preexisting completion messages when sending a
 * corresponding command to a robot.
 *
 * Revision 1.44  1994/10/06  18:26:46  jmc
 * Fixed gt_dup_command to copy the goal too.
 *
 * Revision 1.43  1994/10/06  18:23:08  doug
 * made command_completed a counter and not a boolean
 *
 * Revision 1.42  1994/10/05  22:29:11  doug
 * working on executive
 *
 * Revision 1.41  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.40  1994/10/03  21:13:00  doug
 * Switched to tcx
 *
 * Revision 1.39  1994/09/06  14:26:59  jmc
 * Removed obsolete command->command_id data member from
 * gt_new_command.
 *
 * Revision 1.38  1994/09/06  14:10:22  jmc
 * Updated debugging printouts.
 *
 * Revision 1.37  1994/09/03  17:06:25  jmc
 * Added executed_first_step (instead of reusing executing_step_list)
 * so that clicking twice on [Execute Commands] quickly does not
 * botch things up.  Added warning if send_robot_command is
 * unsuccessful.  Converted to DONE_MSG.  Other minor fixes.
 *
 * Revision 1.36  1994/09/03  00:28:37  jmc
 * Added code for automatic execution of step lists.
 * Added work procedure to execute step list.
 * Added code to deal with list of incomplete_commands.
 *
 * Revision 1.35  1994/08/31  21:44:20  jmc
 * Fixed minor problem in gt_execute_user_command to prevent
 * execution of immediate commands which don't install any command
 * (but do something immediately).
 *
 * Revision 1.34  1994/08/30  14:21:23  jmc
 * Added some code to add_movement_command to copy the phase_line
 * points to the gt_Command structure.  Added a couple lines to
 * gt_new_command to initialize the phase_line structure in newly
 * created gt_Command structures.
 *
 * Revision 1.33  1994/08/29  17:46:00  jmc
 * Added add_set_command and add_quit_command to allow adding SET and
 * QUIT commands to steps.  Minor tweaks to the other add_* commands
 * to remove unnecessary statements.  Modified execute_command to
 * execute SET and QUIT commands (and added a switch to be more
 * general).
 *
 * Revision 1.32  1994/08/25  20:30:24  jmc
 * Added code to execute_step to create the unit member to send with
 * each command.
 *
 * Revision 1.31  1994/08/23  15:35:02  jmc
 * Removed the "pausing_execution" variable in favor of the already
 * existing "paused" variable.
 *
 * Revision 1.30  1994/08/18  20:38:06  jmc
 * Fixed an oversight from the previous modifications.  Now each
 * add_*_command function installs the peripheral data into the
 * command structure correctly.
 *
 * Revision 1.29  1994/08/18  19:23:48  doug
 * Converted SUCCESS/FAILURE to GT_SUCCESS/GT_FAILURE
 *
 * Revision 1.28  1994/08/18  18:40:17  doug
 * console links
 *
 * Revision 1.27  1994/08/17  22:57:21  doug
 * fixing rpc code
 *
 * Revision 1.26  1994/08/17  20:45:10  jmc
 * Extensively modified the [gt_]add_command_* functions and added
 * new gt_specify_* functions to deal with the new way of optional
 * information in the main commands (in the gt_command_yac.y).  Added
 * static local variables for the optional command information and
 * functions to set them (gt_start_command and gt_specify_*).  Made
 * modifications to deal with the additional information in the
 * PHASE-LINE, ON-COMPLETION, and UNTIL clauses.  Note that
 * gt_Command and gt_Step have been changed and some of the
 * modifications are to deal with that change.  Added a new
 * gt_new_command function to allocate new commands and set all the
 * defaults for the command data slots.  Deleted the gt_print_*
 * commands.
 *
 * Revision 1.25  1994/08/16  16:00:06  jmc
 * Changed command_list to step_list.
 *
 * Revision 1.24  1994/08/12  15:37:32  jmc
 * Added speed to gt_add_command_to_step and add_movement_command.
 * Modified gt_print_command to print the speed, if non-zero.
 *
 * Revision 1.23  1994/08/11  13:45:27  jmc
 * Added error checking in add_movement_command to make sure that the
 * "location" for a FOLLOW command is a POLYLINE feature.  It doesn't
 * make sense to follow anything else.
 *
 * Revision 1.22  1994/08/05  17:39:31  jmc
 * Trivial change: Modified an old comment.
 *
 * Revision 1.21  1994/08/05  17:20:40  jmc
 * Modified print_command to use the new format for DATE TIME for the
 * phase line spec.
 *
 * Revision 1.20  1994/08/05  17:16:00  jmc
 * Modified compute_time function to be more robust, missing leading
 * zeros, and fix a few minor bugs.  Added month_length function to
 * clean up some code duplication.
 *
 * Revision 1.19  1994/08/04  15:39:29  jmc
 * Consolidated duplicated code in add_*_command functions into new
 * function setup_step.  Converted several warning messages to
 * warn_userf.
 *
 * Revision 1.18  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.17  1994/08/03  17:19:02  jmc
 * Removed unused variables from gt_execute_next_step.  Fixed return
 * values in execute_step.
 *
 * Revision 1.16  1994/08/03  13:31:24  jmc
 * Converted external gt_add_command_to_step and internal
 * 'add_*(command)' functions to return int FAILURE/SUCCESS for
 * better error handling.
 *
 * Revision 1.15  1994/08/02  20:59:33  jmc
 * Modified gt_new_unit to return NULL immediately if malloc returns
 * no memory.
 *
 * Revision 1.14  1994/07/29  14:18:44  jmc
 * Modified gt_start_step to update previous_step.  Got
 * gt_forward_command_list and gt_backup_command_list working.
 *
 * Revision 1.13  1994/07/28  22:07:24  jmc
 * Added pausing_execution variable.  Changed stop_execution to
 * abort_execution.  Added backup/forward_command_list and
 * toggle_pause_execution stubs.  Modified uses of display_step to
 * indicate the status of the step.
 *
 * Revision 1.12  1994/07/26  16:03:50  jmc
 * Changed explicit "extern void warn_user" to include "gt_sim.h".
 *
 * Revision 1.11  1994/07/25  16:49:00  jmc
 * Converted many of the error messages over to use the warn_user function
 * (which pops up an error dialog box).
 *
 * Revision 1.10  1994/07/22  17:12:28  jmc
 * Numerous modifications to allow manual entry of commands to be
 * executed immediately.  Added function execute_step; modified
 * gt_execute_next_step to use it; set up gt_execute_user_command to
 * use it.  Added variables and functions for dealing with the
 * immediate commands.  Hacked add_*_command routines to add commands
 * to current_step or immediate_step depending on the mode.
 *
 * Revision 1.9  1994/07/20  19:41:44  jmc
 * Added printing of robot ID to print_subunit
 * (and therefore to gt_print_unit).
 *
 * Revision 1.8  1994/07/20  14:39:29  jmc
 * Changed "destination" to "location".
 *
 * Revision 1.7  1994/07/20  14:33:31  jmc
 * Changed add_start_command to save the starting location string.
 *
 * Revision 1.6  1994/07/19  21:26:08  jmc
 * Removed unused variables for clean compiling.
 *
 * Revision 1.5  1994/07/19  16:58:22  jmc
 * Added #define GT_COMMAND_C to allow header file to compile strings into
 * the object file for this file and not make copies everywhere.
 *
 * Revision 1.4  1994/07/15  19:08:06  jmc
 * Fixed a typo on line 993 (== instead of =)
 *
 * Revision 1.3  1994/07/12  19:14:31  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:39:19  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
