/**********************************************************************
 **                                                                  **
 **                         gt_console_db.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1995, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_console_db.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_CONSOLE_DB_H
#define GT_CONSOLE_DB_H

#include "gt_measure.h"
#include "gt_command.h"
#include "console.h"

//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
typedef enum {
    STRING,
    MEASURE,
    UNIT_MEMBERS
} gt_db_Entry_type;

#ifdef GT_CONSOLE_DB_C
char *gt_db_entry_type_name[] = {
    "String", "Measure", "Unit-Members"
};
#else
extern char *gt_db_entry_type_name[];
#endif


// types related to robot information
typedef struct gt_Robot_parameters {
    char *key;
    char *value;
} gt_Robot_parameters;

typedef struct gt_Robot_parameters_list {
    char *key;
    char *value;
    struct gt_Robot_parameters_list *next;
} gt_Robot_parameters_list;

typedef struct gt_Robot_info {
    char *name;
    char *executable;
    char *host;
    char *color;
    int desired_robot_id;
    int num_params;
    gt_Robot_parameters *param;
} gt_Robot_info;


#define MAX_NUM_ROBOT_TYPES 20

extern int num_robot_types;

// This is the array of information about each type of robot
extern gt_Robot_info gt_robot_info[MAX_NUM_ROBOT_TYPES];



// types for unit lists
typedef struct gt_Unit_member_info {
    int id;
    gt_Robot type;
} gt_Unit_member_info;



// This definition of "robot[1]" is used so  that unit member lists can
// be stored as a contiguous chunk of data in the console database.
typedef struct gt_Unit_members {
    int num_robots;

    //First entry, allocate the struct big enough for all members
    gt_Unit_member_info robot[1]; 

} gt_Unit_members;

//-----------------------------------------------------------------------
// Global functions
//-----------------------------------------------------------------------
void gt_clear_db_measures(void);
int gt_add_assembly_area(char *name, gt_Point_list *pt, float diameter);
int gt_add_attack_position(char *name, gt_Point_list *pt, float diameter);
int gt_add_axis(char *name, gt_Point_list *pt, float width);
int gt_add_battle_position(char *name, gt_Point_list *pt, float diameter);
int gt_add_boundary(char *name, gt_Point_list *pt);
int gt_add_gap(char *name, gt_Point_list *pt, float width);
int gt_add_ldlc(char *name, gt_Point_list *pt);
int gt_add_new_robot_type(
    char *name,
    char *executable,
    char *host,
    char *color,
    gt_Robot_parameters_list *params);
int gt_add_mission_name(char *name);
int gt_add_objective(char *name, gt_Point_list *pt, float diameter);
int gt_add_overlay(char *filename);
int gt_load_waypoint_file(char *filename);
int gt_add_passage_point(char *name, float x, float y, float diameter);
int gt_add_attractor(float x, float y, float diameter);
int gt_add_basket(float x, float y, float diameter);
int gt_add_flag(float x, float y, float diaginal);
int gt_add_phase_line(char *name, gt_Point_list *pt);
int gt_add_scenario_label(char *name);
int gt_add_site_name(char *name);
int gt_add_starting_point(char *name, float x, float y);
int gt_add_unit(gt_Unit *unit);
int gt_add_wall(char *name, gt_Point_list *pt, double height = 1);
int gt_add_room(char *name, gt_Point_list *pt);
int gt_add_door(char *name, gt_Point_list *pt);
int gt_add_hallway(
    char *name,
    gt_Point_list *centerline, 
    double width,
    gt_Point_list *extents);
int gt_add_task(void);
int gt_update_db_entry(
    char *key,
    gt_db_Entry_type type,
    void *value,
    unsigned value_size /* num bytes */ );
int gt_delete_db_entry(char *key);
void *gt_get_db_entry(char *key, gt_db_Entry_type type);
gt_Measure *gt_get_db_measure(char *name);
void gt_clear_db(void);
void gt_draw_db(void);
void gt_print_db(void);
void gt_initialize_robot_info(void);
void gt_print_robot_info(void);
gt_Robot_parameters_list *gt_new_robot_parameters_list(
    char *key,
    char *value,
    gt_Robot_parameters_list *next);
void gt_free_robot_parameters_list(gt_Robot_parameters_list *params);
gt_Robot gt_robot_type(char *name);
int gt_add_object(
    float x,
    float y,
    float diameter,
    const char *color,
    OBJ_STYLES style);
int gtGetStartPlace(gt_Point *startPlace);
int load_new_map(char *filename);

#endif // GT_CONSOLE_DB_H

/**********************************************************************
 * $Log: gt_console_db.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.3  2005/07/31 03:40:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.22  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.21  2000/07/07 18:21:07  endo
 * gt_load_waypoint_file() added.
 *
 * Revision 1.20  2000/02/07 05:15:45  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.19  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.18  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.17  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.16  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.15  1996/02/29  01:53:18  doug
 * *** empty log message ***
 *
 * Revision 1.14  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.13  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.12  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.11  1995/04/14  21:04:52  jmc
 * Added a diameter argument to gt_add_assembly_area.
 *
 * Revision 1.10  1995/04/03  20:17:10  jmc
 * Added copyright notice.
 *
 * Revision 1.9  1994/10/28  00:33:46  jmc
 * Added a function prototype for gt_draw_db.
 *
 * Revision 1.8  1994/10/25  20:21:16  jmc
 * Modified gt_add_attack_position, gt_add_battle_position,
 * gt_add_objective, and gt_add_passage_point to take and additional
 * argument for the diameter (used when only a point is given for the
 * center of these measures.
 *
 * Revision 1.7  1994/10/25  15:06:05  jmc
 * Added #ifndef to prevent multiple inclusion problems.
 * Added types for robot info database and robot parameter lists, and
 * function prototypes for related functions.
 *
 * Revision 1.6  1994/08/23  19:14:16  jmc
 * Added a comment to gt_Unit_members.
 *
 * Revision 1.5  1994/08/04  18:50:43  jmc
 * Added width to gt_add_axis and gt_add_gap function prototypes.
 *
 * Revision 1.4  1994/07/26  20:11:52  jmc
 * Modified all the gt_add* routines return an int FAILURE/SUCCESS
 * value.
 *
 * Revision 1.3  1994/07/19  21:15:00  jmc
 * Modified to allow GT_CONSOLE_DB_C to determine whether the global strings will
 * be declared or defined here.
 *
 * Revision 1.2  1994/07/12  19:16:48  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
