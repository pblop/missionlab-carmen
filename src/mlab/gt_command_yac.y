%{
/**********************************************************************
 **                                                                  **
 **                         gt_command_yac.y                         **
 **                                                                  **
 **                                                                  **
 **         Parser for the command description language (cmdl)       **
 **                                                                  **
 **         See the MissionLab user's manual for details.            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2008 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_command_yac.y,v 1.2 2008/07/16 21:16:09 endo Exp $ */

extern "C" {
#include <stdarg.h>
#include <malloc.h>
#include <math.h>
}
#include "console.h"
#include "gt_sim.h"
#include "gt_world.h"
#include "gt_scale.h"
#include "gt_command.h"
#include "gt_console_db.h"
#include "3d_vis_wid.h"
#include "gt_simulation.h"
#include "mlab_cnp.h"

extern void clear_robots(void);

extern void cmdl_error(char *);
extern void cmdl_errorf(const char *, ...);
extern int cmdl_lex();
int cmdl_parse();

int next_robot_id = 1;           /* running id counter for next robot created */
int robot_id;

/* Some error messages */

static char *new_robot_name_error_string = 
"*** Error in new robot name ***\n\
   (You may be trying to redefine the robot name.)\n\
   Correct usage:\n\n\
      NEW-ROBOT robot-name robot-executable\n\
                [host] [(robot parameter list)]";

static char *new_robot_executable_error_string =
"*** Error in new robot executable name ***\n\
   (Try putting it in quotes.)\n\
   Correct usage:\n\n\
      NEW-ROBOT robot-name robot-executable\n\
                [host] [(robot parameter list)]";


%}

%union {               int  boolean;
                     float  fval;
                      char *strval;
                  gt_Robot  robot;
                   gt_Unit *unit;
  gt_Robot_parameters_list *robot_parameters;
              gt_Formation  formation;
              gt_Technique  technique;
}

%token MISSION_NAME SCENARIO OVERLAY MISSION_AREA LOAD_MAP SP
%token UNIT_TOKEN NEW_ROBOT
%token START_COMMANDS IMMEDIATE_COMMAND
%token START_TOKEN MOVETO_TOKEN TELEOPERATE_TOKEN SWEEPTO_TOKEN OCCUPY_TOKEN
%token FOLLOW_TOKEN STOP_TOKEN QUIT_TOKEN SET_TOKEN PRINT_TOKEN
%token CONSOLE_DB_TOKEN ROBOT_INFO_TOKEN CREATE_OBSTACLES OBSTACLE_TOKEN
%token FORMATION TECHNIQUE AND UNTIL MOUSE WAIT FREEZE TIMEOUT
%token PHASE_LINE_TOKEN ON_COMPLETION ACKNOWLEDGE MESSAGE SPEED
%token SHOW_TRAILS_TOKEN SHOW_REP_OBST_TOKEN SHOW_MOV_VECT_TOKEN
%token SEED_TOKEN OBSTACLE_COVERAGE_TOKEN ZOOM_FACTOR_TOKEN
%token SCALE_ROBOTS_TOKEN ROBOT_LENGTH_TOKEN SHOW_COLORMAP_TOKEN
%token MIN_OBSTACLE_RADIUS_TOKEN MAX_OBSTACLE_RADIUS_TOKEN CYCLE_DURATION_TOKEN
%token DEBUG_ROBOTS_TOKEN DEBUG_SIMULATOR_TOKEN DEBUG_SCHEDULER_TOKEN
%token HEIGHTMAP INJECTTRACK_TOKEN RED_TOKEN YELLOW_TOKEN
%token USE_MILITARY_STYLE_TOKEN
%token <boolean> BOOLEAN
%token <strval> QSTRING IDENTIFIER FILENAME DATE TIME STEP_NUMBER
%token <fval> NUMBER
%token <formation> FORMATION_TYPE
%token <technique> TECHNIQUE_TYPE
%token <robot> ROBOT_TYPE

%type <strval> name robot_executable_name robot_parameter_name
%type <unit> unit_list unit robot_list robot
%type <robot_parameters> robot_parameter robot_parameter_list
%%

/* NOTE: "statements" are executed immediately and are never entered into a
         command list.  "commands" are entered into a command list (which may
         or may not be executed immediately). */


file:      info_part commands_part
         | IMMEDIATE_COMMAND composite_immediate_command
         ;

info_part:
           info_statement
         | info_part info_statement
         ;

info_statement:
           mission_name_statement
         | scenario_statement
         | overlay_statement
	 | mission_area_statement
	 | load_map_statement
         | starting_point_statement
         | robot_definition_statement
         | unit_description_statement
         | set_parameter_statement
         | populate_statement
         | obstacle_statement
         | print_statement
         ;

mission_name_statement:
           MISSION_NAME name
                { 
                   gt_add_mission_name($2);
                   free($2);
                }
         ;

scenario_statement:
           SCENARIO name
                { 
                   gt_add_scenario_label($2);
                   free($2);
                }
         ;

overlay_statement:
           OVERLAY FILENAME
                { 
                   if (gt_add_overlay($2) != GT_SUCCESS) {
                      cmdl_errorf("Error loading overlay file\n   '%s'",$2);
                      YYABORT;
                      }
                   free($2);
                }
         | OVERLAY name
                {
                   if (gt_add_overlay($2) != GT_SUCCESS) {
                      cmdl_errorf("Error loading overlay file named\n   '%s'",$2);
                      YYABORT;
                      }
                   free($2);
                }
         ;

mission_area_statement:
           MISSION_AREA NUMBER NUMBER
                {
		   if ($2 < 0.0) {
		      cmdl_errorf("Mission-Area width must be positive (%1.8g)", $2);
		      YYABORT;
		      }
		   else if ($3 < 0.0) {
		      cmdl_errorf("Mission-Area height must be positive (%1.8g)", $3);
		      YYABORT;
		      }
		   else {
		      set_mission_area_size($2, $3);
		      }
		}
	 ;


/*heightmap_statement:
           HEIGHTMAP name NUMBER NUMBER NUMBER NUMBER
                { if (!load_heightmap($2, (int)$3, (int)$4), (int)$5, (int)$6) {
                     cmdl_errorf("Error: Unable to load heightmap %s", $2);
                     YYABORT;
                  }
                };
*/

load_map_statement:
           LOAD_MAP name
                {
		   if (!load_map(gMapInfo, $2, drawing_area)) {
		      cmdl_errorf("Error: Unable to LOAD-MAP %s", $2);
		      YYABORT;
		      }
		}
         ;

starting_point_statement:
           SP name NUMBER NUMBER
                {
                   if (gt_add_starting_point($2, $3+origin_x, $4+origin_y) != GT_SUCCESS) {
                      cmdl_errorf("Error creating SP %s %f %f",$2,$3,$4);
                      YYABORT;
                      }
                   free($2);
                }
         ;



	   /* NEW_ROBOT name_of_robot executable [color [host[(parms)]]] */

robot_definition_statement:
           NEW_ROBOT name robot_executable_name
                {
                   if (gt_add_new_robot_type($2, $3, NULL, NULL, NULL) != GT_SUCCESS){
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   free($2);
                   free($3);
                }
         | NEW_ROBOT name robot_executable_name name
                {
                   if (gt_add_new_robot_type($2, $3, NULL, $4, NULL) != GT_SUCCESS) {
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   free($2);
                   free($3);
                   free($4);
                }
         | NEW_ROBOT name robot_executable_name name name
                {

                   if (gt_add_new_robot_type($2, $3, $5, $4, NULL) != GT_SUCCESS) {
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   free($2);
                   free($3);
                   free($4);
                }
         | NEW_ROBOT name robot_executable_name '(' robot_parameter_list ')'
                { 
                   if (gt_add_new_robot_type($2, $3, NULL, NULL, $5) != GT_SUCCESS) {
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   gt_free_robot_parameters_list($5);
                   free($2);
                   free($3);
                   free($5);
                }
         | NEW_ROBOT name robot_executable_name name '(' robot_parameter_list ')'
                {
                   if (gt_add_new_robot_type($2, $3, NULL, $4, $6) != GT_SUCCESS) {
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   gt_free_robot_parameters_list($6);
                   free($2);
                   free($3);
                   free($4);
                }
         | NEW_ROBOT name robot_executable_name name name '(' robot_parameter_list ')'
                {
                   if (gt_add_new_robot_type($2, $3, $5, $4, $7) != GT_SUCCESS) {
                      cmdl_errorf("Error creating new robot type '%s'",$2);
                      YYABORT;
                      }
                   gt_free_robot_parameters_list($7);
                   free($2);
                   free($3);
                   free($4);
                   free($5);
                }
         | NEW_ROBOT error
                {
                   cmdl_errorf(new_robot_name_error_string);
                   YYABORT;
                }
         | NEW_ROBOT name error
                {
                   cmdl_errorf(new_robot_executable_error_string);
                   YYABORT;
                }
         ;


robot_executable_name:
           name
         | ROBOT_TYPE
                {
                   /* The user has specified an executable name which happens
                      to correspond to the name of a system or user-defined
                      robot.  Return the appropriate string for the name.
                    */
                   char *rob;
                   switch ($1) {
                   case UNIT:
                      rob = (char *)malloc(5);
                      strcpy(rob, "unit");
                      break;
                   case ROBOT:
                      rob = (char *)malloc(6);
                      strcpy(rob, "robot");
                      break;
                   default:
                      if ($1 < num_robot_types) {
                         rob = (char *)malloc(strlen(gt_robot_info[$1].name)+1);
                         strcpy(rob, gt_robot_info[$1].name);
                         }
                      else {
                         cmdl_errorf("Error in robot executable name (robot_type=%d)", $1);
                         YYABORT;
                         }
                      break;
                      }
                   $$ = rob;
                }
         ;


robot_parameter_list:
           robot_parameter
                {
                   $$ = $1;
                }
         | robot_parameter ',' robot_parameter_list
                {
                   gt_Robot_parameters_list *p = $1;
                   while (p->next)
                      p = p->next;
                   p->next = $3;
                   $$ = $1;
                }
         ;

robot_parameter:
           robot_parameter_name '=' name
                { 
                   if (($$ = gt_new_robot_parameters_list($1, $3, NULL)) == NULL) {
                      cmdl_errorf("Error adding robot parameter: %s=%s", $1, $3);
                      YYABORT;
                      }
                   free($1);
                   free($3);
                }
         | robot_parameter_name '=' BOOLEAN
                {
                   char val[30];
                   sprintf(val, "%d", $3);
                   if (($$ = gt_new_robot_parameters_list($1, val, NULL)) == NULL) {
                      cmdl_errorf("Error adding robot parameter: %s=%s",
                                  $1, $3 ? "Yes" : "No");
                      YYABORT;
                      }
                   free($1);
                }
         | robot_parameter_name '=' NUMBER
                {
                   char val[30];
                   sprintf(val, "%1.8g", $3);
                   if (($$ = gt_new_robot_parameters_list($1, val, NULL)) == NULL) {
                      cmdl_errorf("Error adding robot parameter: %s=%f", $1, $3);
                      YYABORT;
                      }
                   free($1);
                }
         ;

robot_parameter_name:
           name
         | TIMEOUT
                {
                   char *str;
                   str = (char *)malloc(8);
                   strcpy(str, "timeout");
                   $$ = str;
                }
         ;


unit_description_statement:
           UNIT_TOKEN '<' name '>' robot_list
                { 
                   gt_Unit *u = gt_new_unit(0,$3,UNIT,$5,0);
                   if ((u == NULL) || (gt_add_unit(u) != GT_SUCCESS)) {
                      cmdl_errorf("Error creating UNIT %s",$3);
                      YYABORT;
                      }
                   free($3);
                }
         | UNIT_TOKEN '<' name '>' unit_list
                {
                   gt_Unit *u = gt_new_unit(0,$3,UNIT,$5,0);
                   if ((u == NULL) || (gt_add_unit(u) != GT_SUCCESS)) {
                      cmdl_errorf("Error creating UNIT %s",$3);
                      YYABORT;
                      }
                   free($3);
                }
         ;



unit_list:
           '(' unit ')'
                { 
                   $$ = $2;
                }
         | unit_list '(' unit ')'
                {
                   gt_Unit *u = $1;
                   while (u->next)
                      u = u->next;
                   u->next = $3;
                   $$ = $1;
                }
         ;


unit:
           robot_list
                { 
                   if (($$ = gt_new_unit(0,0,UNIT,$1,0)) == NULL) {
                      cmdl_errorf("Error creating robot list");
                      YYABORT;
                      }
                }
         | '<' name '>' robot_list
                {
                   if (($$ = gt_new_unit(0,$2,UNIT,$4,0)) == NULL) {
                      cmdl_errorf("Error creating robot list %s",$2);
                      YYABORT;
                      }
                   free($2);
                }
         | '<' name '>' unit_list
                {
                   if (($$ = gt_new_unit(0,$2,UNIT,$4,0)) == NULL) {
                      cmdl_errorf("Error creating unit list %s",$2);
                      YYABORT;
                      }
                   free($2);
                }
         ;


robot_list:
           robot
                { 
                   $$ = $1;
                }
         | robot_list robot
                {
                   gt_Unit *u = $1;
                   while (u->next)
                      u = u->next;
                   u->next = $2;
                   $$ = $1;
                }
         ;


robot:     ROBOT_TYPE
                {
                    robot_id = gt_robot_info[$1].desired_robot_id;

                    if (robot_id > next_robot_id)
                    {
                        next_robot_id = robot_id;
                    }
                    else if (robot_id < 0)
                    {
                        robot_id = next_robot_id;
                    }

                    if (($$ = gt_new_unit(robot_id,0,$1,0,0)) == NULL)
                    {
                      cmdl_errorf("Error adding robot %s",
                                  gt_robot_info[$1].name);
                      YYABORT;
                    }
                    else
                    {
                        next_robot_id++;
                    }
                }
         | name 
                {
                    gt_Robot type = gt_robot_type($1);

                    if (type != GT_FAILURE)
                    {
                        robot_id = gt_robot_info[type].desired_robot_id;

                        if (robot_id > next_robot_id)
                        {
                            next_robot_id = robot_id;
                        }
                        else if (robot_id < 0)
                        {
                            robot_id = next_robot_id;
                        }

                        if (($$ = gt_new_unit(robot_id,0,type,0,0)) == NULL)
                        {
                            cmdl_errorf("Error adding robot %s",
                                        gt_robot_info[type].name);
                            YYABORT;
                        }
                        else
                        {
                            next_robot_id++;
                        }
                    }
                    else
                    {
                        cmdl_errorf("Error: '%s' is not the name of a known type of robot", $1);
                        YYABORT;
                    }
                }
         ;


set_parameter_statement:
           SET_TOKEN SEED_TOKEN NUMBER              { seed = nint($3);           }
         | SET_TOKEN USE_MILITARY_STYLE_TOKEN       { use_military_style(TRUE);  }
         | SET_TOKEN USE_MILITARY_STYLE_TOKEN BOOLEAN { use_military_style($3);  }
         | SET_TOKEN SHOW_TRAILS_TOKEN              { set_show_trails(TRUE);     }
         | SET_TOKEN SHOW_TRAILS_TOKEN BOOLEAN      { set_show_trails($3);       }
         | SET_TOKEN SHOW_REP_OBST_TOKEN            { set_show_repelling_obstacles(TRUE); }
         | SET_TOKEN SHOW_REP_OBST_TOKEN BOOLEAN    { set_show_repelling_obstacles($3);   }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN            { set_show_movement_vectors(TRUE);    }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN BOOLEAN    { set_show_movement_vectors($3);      }
         | SET_TOKEN SHOW_COLORMAP_TOKEN            { set_show_colormap(TRUE);   }
         | SET_TOKEN SHOW_COLORMAP_TOKEN BOOLEAN    { set_show_colormap($3);     }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN             { set_debug_robots(TRUE);    }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN BOOLEAN     { set_debug_robots($3);      }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN          { set_debug_simulator(TRUE); }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN BOOLEAN  { set_debug_simulator($3);   }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN          { set_debug_scheduler(TRUE); }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN BOOLEAN  { set_debug_scheduler($3);   }
         | SET_TOKEN SCALE_ROBOTS_TOKEN             { set_scale_robots(TRUE);    }
         | SET_TOKEN SCALE_ROBOTS_TOKEN BOOLEAN     { set_scale_robots($3);      }
         | SET_TOKEN ROBOT_LENGTH_TOKEN NUMBER
                {
                   if (!set_robot_length((double)$3)) {
                      cmdl_errorf("Error in SET ROBOT-LENGTH %1.8g statement", $3);
                      YYABORT;
                      }
                }
         | SET_TOKEN OBSTACLE_COVERAGE_TOKEN NUMBER
                {
                   if (!set_obstacle_coverage((double)$3)) {
                      cmdl_errorf("Error in SET OBSTACLE-COVERAGE %1.8g", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN MIN_OBSTACLE_RADIUS_TOKEN NUMBER
                {
		   if (!set_min_obstacle_radius((double)$3)) {
                      cmdl_errorf("Error in SET MIN-OBSTACLE-RADIUS %1.8g statement", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN MAX_OBSTACLE_RADIUS_TOKEN NUMBER 
                {
		   if (!set_max_obstacle_radius((double)$3)) {
                      cmdl_errorf("Error in SET MAX-OBSTACLE-RADIUS %1.8g statement", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN ZOOM_FACTOR_TOKEN NUMBER
                {
		   if (!set_zoom_factor((double)$3/100.0)) {
		      cmdl_errorf("Error in SET ZOOM-FACTOR %1.8g statement", $3);
		      YYABORT;
		      }
		}
         | SET_TOKEN CYCLE_DURATION_TOKEN NUMBER
                {
		   if (!set_cycle_duration((double)$3)) {
                      cmdl_errorf("Error in SET CYCLE-DURATION %1.8g statement", $3);
                      YYABORT;
		      }
                }
         ;


populate_statement:
           CREATE_OBSTACLES
                {
                   create_obstacles(TRUE);
                }
         ;

obstacle_statement:
           OBSTACLE_TOKEN NUMBER NUMBER NUMBER
                {
                   if (gt_add_obstacle($2+origin_x, $3+origin_y, $4) !=  GT_SUCCESS) {
                      cmdl_errorf("Unable to add Obstacle (x=%1.8g, y=%1.8g, r=%1.8g).", 
                                  $2, $3, $4);
                      YYABORT;
                      }
                }
         ;

print_statement:
           PRINT_TOKEN CONSOLE_DB_TOKEN             { gt_print_db(); }
         | PRINT_TOKEN ROBOT_INFO_TOKEN             { gt_print_robot_info(); }
         ;


commands_part:
           START_COMMANDS { gt_start_step_list(); } step_list
         ;

step_list:
           step
         | step_list step
         ;

step:      STEP_NUMBER { gt_start_step($1); } composite_command
                {
                   free($1);
                }
         ;

composite_command:
           command
         | composite_command AND command
         ;

command:
           start_command
         | moveto_command
         | teleoperate_command
         | sweepto_command
         | follow_command
         | occupy_command
         | stop_command
         | quit_command
         | set_parameter_command
         | print_command
         ;


composite_immediate_command:
           immediate_command 
         | composite_immediate_command AND immediate_command
         ;

immediate_command:
           start_command
         | moveto_command
         | teleoperate_command
         | sweepto_command
         | follow_command
         | occupy_command
         | stop_command
         | quit_command
         | set_parameter_statement
         | populate_statement
         | obstacle_statement
         | print_statement
         | unit_description_statement
         | cnp_command_statement
         ;


start_command:
           UNIT_TOKEN name START_TOKEN name {gt_start_command();} opt_start_clause
                {
                   if (gt_add_command_to_step($2, START, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s START %s",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

cnp_command_statement:
	INJECTTRACK_TOKEN RED_TOKEN
    {
		//inject_track_task(1);

        if (gMlabCNP != NULL)
        {
//            gMlabCNP->injectTrackTask(CNP_TRACK_RED);
        }
    }
	| INJECTTRACK_TOKEN YELLOW_TOKEN
      {
          //inject_track_task(2);

          if (gMlabCNP != NULL)
          {
 //             gMlabCNP->injectTrackTask(CNP_TRACK_YELLOW);
          }
      }
	;


moveto_command:
           UNIT_TOKEN name MOVETO_TOKEN name {gt_start_command();} opt_moveto_clause_list
                {
                   if (gt_add_command_to_step($2, MOVETO, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s MOVETO %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

teleoperate_command:
           UNIT_TOKEN name TELEOPERATE_TOKEN name {gt_start_command();} opt_teleoperate_clause_list
                {
                   if (gt_add_command_to_step($2, TELEOPERATE, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s TELEOPERATE %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

sweepto_command:
           UNIT_TOKEN name SWEEPTO_TOKEN name {gt_start_command();} opt_sweepto_clause_list
                {
                   if (gt_add_command_to_step($2, SWEEPTO, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s SWEEPTO %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

follow_command:
           UNIT_TOKEN name FOLLOW_TOKEN name {gt_start_command();} opt_follow_clause_list
                {
                   if (gt_add_command_to_step($2, FOLLOW, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s FOLLOW %s",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

occupy_command:
           UNIT_TOKEN name OCCUPY_TOKEN name {gt_start_command();} opt_occupy_clause_list
                {
                   if (gt_add_command_to_step($2, OCCUPY, $4) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s OCCUPY %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

stop_command:
           UNIT_TOKEN name STOP_TOKEN {gt_start_command();}
                {
                   if (gt_add_command_to_step($2, STOP, 0) != GT_SUCCESS) {
                      cmdl_errorf("Error adding command: UNIT %s STOP",$2);
                      YYABORT;
                      }
                   free($2);
                }
         ;


quit_command:
           QUIT_TOKEN
                { 
                   if (gt_add_command_to_step(NULL, QUIT, NULL) != GT_SUCCESS) {
                      cmdl_errorf("Error adding QUIT command");
                      YYABORT;
                      }
                }
         ;


set_parameter_command:
           SET_TOKEN SHOW_TRAILS_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_TRAILS, SET, (char *)TRUE)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET SHOW-TRAILS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_TRAILS_TOKEN BOOLEAN
                { 
                   if (gt_add_command_to_step((char *)SHOW_TRAILS,SET,(char *)$3)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET SHOW-TRAILS %s command",
                                  ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }
 

         | SET_TOKEN USE_MILITARY_STYLE_TOKEN
                { 
                   if (gt_add_command_to_step((char *)USE_MILITARY_STYLE, SET, (char *)TRUE)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET USE-MILITARY-STYLE TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN USE_MILITARY_STYLE_TOKEN BOOLEAN
                { 
                   if (gt_add_command_to_step((char *)USE_MILITARY_STYLE,SET,(char *)$3)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET USE-MILITARY-STYLE %s command",
                                  ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }
 

         | SET_TOKEN SHOW_REP_OBST_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_REPELLING_OBSTACLES,
                                              SET, (char *)TRUE) != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET HIGHLIGHT-REPELLING-OBSTACLES\
TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_REP_OBST_TOKEN BOOLEAN
                { 
                   if (gt_add_command_to_step((char *)SHOW_REPELLING_OBSTACLES,
                                              SET,(char *)$3) != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET HIGHLIGHT-REPELLING-OBSTACLES\
%s command", ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }

         | SET_TOKEN SHOW_MOV_VECT_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_MOVEMENT_VECTORS,
                                              SET, (char *)TRUE) != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET SHOW-MOVEMENT-VECTORS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN BOOLEAN
                { 
                   if (gt_add_command_to_step((char *)SHOW_MOVEMENT_VECTORS,
                                              SET,(char *)$3) != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET SHOW-MOVEMENT-VECTORS %s command",
                                  ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }

         | SET_TOKEN DEBUG_ROBOTS_TOKEN 
                { 
                   if (gt_add_command_to_step((char *)DEBUG_ROBOTS,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG-ROBOTS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN BOOLEAN
                { 
                   if (gt_add_command_to_step((char *)DEBUG_ROBOTS,SET,(char *)$3) 
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG-ROBOTS %s command",
                                  ($3 ? "ON" : "OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SIMULATOR,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG_SIMULATOR TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN BOOLEAN  
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SIMULATOR,SET,(char *)$3)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG-SIMULATOR %s command",
                                  ($3 ? "ON":"OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SCHEDULER,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG_SCHEDULER TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN BOOLEAN  
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SCHEDULER,SET,(char *)$3)
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET DEBUG-SCHEDULER %s command",
                                  ($3 ? "ON":"OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN CYCLE_DURATION_TOKEN NUMBER
                {
                   if ($3 < MIN_CYCLE_DURATION) {
                      cmdl_errorf("Cycle duration value, %1.8g, is less than\n\
 the minimum value of %1.8g seconds", $3, MIN_CYCLE_DURATION);
                      YYABORT;
                      }
                   if ($3 > MAX_CYCLE_DURATION) {
                      cmdl_errorf("Cycle duration value, %1.8g, is greater than\n\
 the maximum value of %1.8g seconds", $3, MAX_CYCLE_DURATION);
                      YYABORT;
                      }
                   if (gt_add_command_to_step((char *)CYCLE_DURATION,SET,(char *)&($3))
                       != GT_SUCCESS) {
                      cmdl_errorf("Error adding SET CYCLE-DURATION %1.8g command",$3);
                      YYABORT;
                      }
                }
         ;


print_command: 
           PRINT_TOKEN CONSOLE_DB_TOKEN
                {
                   if (gt_add_command_to_step((char *)CONSOLE_DB, PRINT, NULL)
                       != GT_SUCCESS) {
                      cmdl_error("Error adding PRINT CONSOLE-DATABASE command");
                      YYABORT;
                      }
                }
         | PRINT_TOKEN ROBOT_INFO_TOKEN
                {
                   if (gt_add_command_to_step((char *)ROBOT_INFO, PRINT, NULL)
                       != GT_SUCCESS) {
                      cmdl_error("Error adding PRINT ROBOT-INFO command");
                      YYABORT;
                      }
                }
         ;



opt_start_clause:
           /* empty */
         | NUMBER NUMBER  { gt_specify_offsets($1,$2); }
         ;


opt_moveto_clause_list:
           /* empty */
         | opt_moveto_clause_list opt_moveto_clause
         ;

opt_moveto_clause:
           formation_clause
         | technique_clause
         | speed_clause
         | phase_line_clause
         | on_completion_clause
         ;



opt_teleoperate_clause_list:
           /* empty */
         | opt_teleoperate_clause_list opt_teleoperate_clause
         ;

opt_teleoperate_clause:
           formation_clause
         | speed_clause
         | on_completion_clause
         ;


opt_sweepto_clause_list:
           /* empty */
         | opt_sweepto_clause_list opt_sweepto_clause
         ;

opt_sweepto_clause:
           formation_clause
         | technique_clause
         | speed_clause
         | phase_line_clause
         | on_completion_clause
         ;


opt_follow_clause_list:
           /* empty */
         | opt_follow_clause_list opt_follow_clause
         ;

opt_follow_clause:
           formation_clause 
         | technique_clause 
         | speed_clause
         | phase_line_clause
         | on_completion_clause
         ;

opt_occupy_clause_list:
           /* empty */
         | opt_occupy_clause_list opt_occupy_clause
         ;

opt_occupy_clause:
           formation_clause
         | until_clause
         ;


formation_clause:  
           FORMATION '=' FORMATION_TYPE { gt_specify_formation($3); }
         | FORMATION FORMATION_TYPE     { gt_specify_formation($2); }
         | FORMATION_TYPE               { gt_specify_formation($1); }
         ;

technique_clause:
           TECHNIQUE '=' TECHNIQUE_TYPE { gt_specify_technique($3); }
         | TECHNIQUE TECHNIQUE_TYPE     { gt_specify_technique($2); }
         | TECHNIQUE_TYPE               { gt_specify_technique($1); }
         ;


speed_clause:
           SPEED NUMBER       { gt_specify_speed($2); }
         | SPEED '=' NUMBER   { gt_specify_speed($3); }
         ;

phase_line_clause:
           phase_line_clause_start
         | phase_line_clause_start ACKNOWLEDGE name
                { 
                   gt_specify_phase_line_msg($3);
                   free($3);
                }
         | phase_line_clause_start ACKNOWLEDGE name WAIT
                { 
                   gt_specify_phase_line_msg($3);
                   gt_specify_phase_line_wait(TRUE);
                   free($3);
                }
         ;

phase_line_clause_start:
           PHASE_LINE_TOKEN name
                {
                   gt_specify_phase_line($2, NULL, NULL);
                   free($2);
                }
         | PHASE_LINE_TOKEN name DATE TIME
                {
                   gt_specify_phase_line($2, $3,   $4);
                   free($2);
                   free($3);
                   free($4);
                }
         | PHASE_LINE_TOKEN name TIME DATE
                {
                   gt_specify_phase_line($2, $4,   $3);
                   free($2); 
                   free($3);
                   free($4);
                }
         | PHASE_LINE_TOKEN name TIME
                {
                   gt_specify_phase_line($2, NULL, $3);
                   free($2);
                   free($3);
                }
         ;

on_completion_clause:
           ON_COMPLETION MESSAGE name
                { 
                   gt_specify_completion($3, FALSE);
                   free($3);
                } 
         | ON_COMPLETION MESSAGE name FREEZE
                { 
                   gt_specify_completion($3, TRUE);
                   free($3);
                }
         ;

until_clause:
           UNTIL TIME
                { 
                   gt_specify_until(NULL, $2);
                   free($2);
                }
         | UNTIL TIME DATE
                { 
                   gt_specify_until($3, $2);
                   free($2);
                   free($3);
                }
         | UNTIL DATE TIME
                { 
                   gt_specify_until($2, $3);
                   free($2);
                   free($3);
                }
         | UNTIL NUMBER
                {
                   char time_str[20];
                   sprintf(time_str, "%d", nint($2));
                   gt_specify_until(NULL, time_str);
                }
         | UNTIL TIMEOUT TIME
                {
                   if ($3[0] == '+') {
                      $3[0] = '-';
                      gt_specify_until(NULL, $3);
                      free($3);
                      }
                   else {
                      char wait_time[20];
                      sprintf(wait_time, "-%s", $3);
                      gt_specify_until(NULL, wait_time);
                      free($3);
                      }
                }
         | UNTIL TIMEOUT NUMBER
                {
                   char wait_time[20];
                   sprintf(wait_time, "-%d", nint($3));
                   gt_specify_until(NULL, wait_time);
                }
         ;


name:      IDENTIFIER
         | QSTRING
         ;

%%



/**********************************************************************
 * $Log: gt_command_yac.y,v $
 * Revision 1.2  2008/07/16 21:16:09  endo
 * Military style is now a toggle option.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.8  2006/07/11 10:07:49  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.7  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.6  2006/01/12 20:32:44  pulam
 * cnp cleanup
 *
 * Revision 1.5  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.4  2005/07/31 03:40:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.3  2005/04/08 01:56:03  pulam
 * addition of cnp behaviors
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.54  1997/02/14  15:49:30  zchen
 * *** empty log message ***
 *
 * Revision 1.53  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.52  1995/06/14  17:15:33  jmc
 * Added immediate statements for SET SHOW-COLORMAP statement.
 *
 * Revision 1.51  1995/06/12  17:56:10  jmc
 * Minor cleanup.
 *
 * Revision 1.50  1995/06/12  16:18:14  jmc
 * Added MISSION-AREA and LOAD-MAP statements.
 *
 * Revision 1.49  1995/05/06  00:43:37  jmc
 * Changed the grammar for SET METERS-PER-PIXEL to SET ZOOM-FACTOR.
 *
 * Revision 1.48  1995/05/04  22:05:22  jmc
 * Added include for gt_scale.h since all the scale related functions
 * moved to it.
 *
 * Revision 1.47  1995/05/04  19:59:26  jmc
 * Various fixes to accomodate the different style of map scaling.
 * Changed grammar for SET MAGNIFY-ROBOTS to SET SCALE-ROBOTS, and
 * SET ROBOT-MAGNIFICATION to SET ROBOT-LENGTH.  Converted range
 * checking to use the involved set_* function for several
 * set_*_statement grammars.  Moved the range checking to where the
 * set_* functions were defined.
 *
 * Revision 1.46  1995/05/02  17:57:19  jmc
 * Converted the SET METERS_PER_PIXEL to use the return value of the
 * function, rather than test the parameter itself.
 *
 * Revision 1.45  1995/04/27  19:42:11  jmc
 * Minor tweaks to SET METERS_PER_PIXEL grammar location.
 *
 * Revision 1.44  1995/04/26  18:58:13  jmc
 * Added grammer for the TELEOPERATE behavior.
 *
 * Revision 1.43  1995/04/26  11:44:41  jmc
 * Made the error strings static.
 *
 * Revision 1.42  1995/04/25  22:16:04  jmc
 * Added separate definitions of robot_executable_name and
 * robot_parameter_name to make their parsing more flexible and to
 * deal with some special cases.  Added error messages to a couple
 * typical errors in robot_definition_statement.
 *
 * Revision 1.41  1995/04/24  16:44:02  jmc
 * Added name=BOOLEAN grammar to robot_parameter so that BOOLEANs can
 * be specified textually.
 *
 * Revision 1.40  1995/04/20  17:59:31  jmc
 * Corrected spelling.
 *
 * Revision 1.39  1995/04/20  16:58:34  jmc
 * Modified error messages to put filenames on fresh lines
 * since some filenames are really long.
 *
 * Revision 1.38  1995/04/14  17:28:04  jmc
 * Renamed POPULATE_OBSTACLES to CREATE_OBSTACLES and
 * populate_world() to create_obstacles().
 *
 * Revision 1.37  1995/04/03  19:33:30  jmc
 * Added copyright notice.
 *
 * Revision 1.36  1995/02/15  19:24:08  jmc
 * Added grammar for SWEEPTO command.
 *
 * Revision 1.35  1995/02/14  16:35:19  jmc
 * Tweaked an error message.
 *
 * Revision 1.34  1995/02/13  18:53:17  jmc
 * Added SET CYCLE-DURATION to the commands.  This required changes
 * in several other files.
 *
 * Revision 1.33  1995/02/02  22:50:31  jmc
 * Tweaked some of the error messages.
 *
 * Revision 1.32  1995/02/02  22:25:19  jmc
 * Added grammar for SET CYCLE-DURATION statements.
 *
 * Revision 1.31  1995/02/02  16:39:53  jmc
 * Added grammar for SET SHOW-MOVEMENT-VECTORS statements and
 * commands.
 *
 * Revision 1.30  1995/01/27  21:55:23  jmc
 * Added an explanatory comment right before the grammar definitions.
 *
 * Revision 1.29  1994/11/10  23:30:27  jmc
 * Added grammar to set "magnify robots" and "robot magnification"
 * from statements (but not commands).
 *
 * Revision 1.28  1994/11/07  20:10:45  jmc
 * Small mods to make sure that everything uses the origin correctly.
 *
 * Revision 1.27  1994/11/07  17:51:01  jmc
 * Added an obstacle creation command to preamble and immediate mode.
 *
 * Revision 1.26  1994/11/04  15:25:10  jmc
 * Added grammar for SET DEBUG-SCHEDULER in statements and commands.
 *
 * Revision 1.25  1994/11/03  18:45:25  jmc
 * Added grammar for SET HIGHLIGHT-REPELLING-OBSTACLES in statements
 * and commands.
 *
 * Revision 1.24  1994/10/28  18:07:51  jmc
 * Modified the syntax of OCCUPY UNTIL to specify either an "until"
 * time (OCCUPY UNTIL 12:00) or by an "until" timeout (OCCUPY UNTIL
 * TIMEOUT 12:00).
 *
 * Revision 1.23  1994/10/27  21:39:15  jmc
 * Added several SET commands for SYSTEM-SEED, OBSTACLE-COVERAGE,
 * METERS-PER-PIXEL, MIN-OBSTACLE-RADIUS, and MAX-OBSTACLE-RADIUS.
 * Added the POPULATE-OBSTACLES command.  These are all world
 * configuration things that had been overlooked.  Note that these
 * are for set statments only---not commands to be saved in step
 * lists.
 *
 * Revision 1.22  1994/10/26  14:22:53  jmc
 * Changed robot_id to next_robot_id to avoid name conflicts with
 * other modules.
 *
 * Revision 1.21  1994/10/25  14:51:10  jmc
 * Changed the syntax for robots from fixed to definable via a new
 * command NEW-ROBOT.  Changed syntax of unit lists to solve a
 * problem introduced by the NEW-ROBOT commands.  Now ALL unit names
 * must be surrounded by angle brackets, eg: UNIT <Zebra>.  Added
 * PRINT (console-db|robot-info) commands to immediate mode, command
 * preamble, and step lists.  Cleaned up indenting.  Added code to
 * free strings previously allocated by strdup in the lexer.
 *
 * Revision 1.20  1994/09/03  13:51:22  jmc
 * Fixed minor problem with ordering of #includes.
 *
 * Revision 1.19  1994/08/31  21:50:09  jmc
 * Added composite_immediate_command and immediate_command to make
 * immediate commands more flexible.  Added
 * unit_description_statement to immediate command.
 *
 * Revision 1.18  1994/08/29  17:55:03  jmc
 * Added grammar for parsing SET and QUIT commands.  Had to make a
 * separate set_parameter_command (as distinguished from the
 * previously implemented set_parameter_statement) in order to allow
 * addition of SET commands to the step list.
 *
 * Revision 1.17  1994/08/26  16:00:51  jmc
 * Added grammar for setting show-trails, debug-simulator, and
 * debug-robots parameter values.
 *
 * Revision 1.16  1994/08/18  20:39:21  jmc
 * Moved the gt_start_command() to right before the optional clauses.
 * Modified the opt_occupy_clause_list to allow formation clauses.
 * (This used to work but it got overlooked in the previous
 * modifications.)
 *
 * Revision 1.15  1994/08/18  19:25:14  doug
 * Converted SUCCESS/FAILURE to GT_SUCCESS/GT_FAILURE
 *
 * Revision 1.14  1994/08/17  20:37:49  jmc
 * Extensively modified the grammar to make optional-clause-lists on
 * the end of the main command definitions.  This required using
 * gt_specify_* functions to deal with the optional information.
 *
 * Revision 1.13  1994/08/16  15:59:47  jmc
 * Changed command_list to step_list.
 *
 * Revision 1.12  1994/08/12  15:27:03  jmc
 * Added an optional speed clause to moveto_statement and
 * follow_statement.
 *
 * Revision 1.11  1994/08/11  20:49:28  jmc
 * Modified the syntax of the OCCUPY command until_clause from
 * UNTIL-KEY "str" to UNTIL MESSAGE "msg".  If the "msg" part is left
 * out, it defaults to "Go".
 *
 * Revision 1.10  1994/08/05  17:12:46  jmc
 * Changed syntax of DATE:TIME to DATE TIME.  Made order of date and
 * time reversible.
 *
 * Revision 1.9  1994/08/03  13:37:39  jmc
 * Added better error handling/aborting to add_* calls.
 *
 * Revision 1.8  1994/07/26  21:01:55  jmc
 * Modified overlay sections to abort if there are problems reading
 * the overlay file.
 *
 * Revision 1.7  1994/07/22  17:17:22  jmc
 * Minor modifications to allow manual entry of commands to be
 * executed immediately.  Added immediate command token and syntax
 * under "file:".
 *
 * Revision 1.6  1994/07/20  20:08:38  jmc
 * Removed some debugging print statements.
 *
 * Revision 1.5  1994/07/20  19:44:40  jmc
 * Improved grammar for units to make it more robust (hopefully).
 *
 * Revision 1.4  1994/07/12  19:14:54  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.3  1994/07/12  19:03:43  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:21:45  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
