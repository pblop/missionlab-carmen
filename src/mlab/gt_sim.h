/**********************************************************************
 **                                                                  **
 **                             gt_sim.h                             **
 **                                                                  **
 **                                                                  **
 ** Written by: Douglas C. MacKenzie                                 **
 **                                                                  **
 ** Copyright 1995 - 2005 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_sim.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_SIM_H
#define GT_SIM_H

#include <vector>

#include "gt_message_types.h"
#include "gt_world.h"
#include "gt_command.h"
#include "ipt/ipt.h"
#include "gt_std.h"
#include "robot.hpp"

using std::vector;

int gt_simulation_loop(void);
void gt_init_simulation(char* ipt_home);

extern int debug;
extern bool gAutomaticExecution;

extern int report_current_state;
extern int log_robot_data;
extern int report_state_messages;

extern bool paused;
extern bool gStopRobotFeedback;
extern bool waypoint_design_mode;
extern int seed;
extern int robot_debug_messages;
extern int debug_scheduler;
extern int highlight_repelling_obstacles;
extern int show_movement_vectors;
extern int show_obstacles;
extern int erase_obstacles;
extern int laser_linear_mode;
extern int laser_normal_mode;
extern int laser_connected_mode;
extern int sonar_arc_mode;
extern int sonar_point_mode;
extern int LOS_mode;
extern int uf;
extern int show_colors;
extern int learning_momentum_enabled;
extern int save_learning_momentum_weights;
extern int save_sonar_readings;

struct T_robot_state
{
    int            robot_id;
    int            alive;
    int            robot_type;
    int            run_type;
    Vector         loc;
    Vector         lastLoc;
    double         heading;
    int            color;
    int            is_up;
    int            v_up;
    bool           drawing_vector_field;
    double         steer_angle;
    Vector         old_v;
    IPConnection*  com_link;
    bool           step_taken;
    bool           sensors_received;
    T_robot_state* next;
    robot*         object_ptr;
    bool           LOS_not;
    int            pcount;
    int            mobility_type;
    bool           vis;
    char *           col;
};

T_robot_state* add_new_robot(int request_id);

int start_robot(
    char *machine,
    char *file,
    int robot_id,
    int robot_debug,
    int report_state,
    double start_x,
    double start_y,
    char *color);
void Pick_new_map(void);
int GetFirstRobotID(void);
int GetNextRobotID(int iID);
bool GetRobotLocation(int iID, Vector& suLocation);
bool SetRobotLocation(int iID, const Vector& suLocation);
bool SetDrawingVectorField(int iID, bool bDrawing);
bool SetRunType(int iID, int newRunType, int& oldRunType);
bool SetRunType(int iID, int newRunType);
bool ResetStepTaken(int iID);
bool WaitForRobotStep(int iID);
bool ResetSensorsReceived(int iID);
bool WaitForSensors(int iID);
void RestoreReadingsFromBackup(void);

void clear_robots(void);
int send_robot_command(gt_Command* cmd, gt_Robot type);
IPConnection *sim_get_com_link(int robot_id);
int sim_robot_birth(int request_id, gt_Point start_loc, float heading, IPConnection* com_link);
void sim_notify_config(int robot_id, int robot_type, int run_type);
void sim_move_robot(int robot_id, gt_Point dist);
void sim_spin_robot(int robot_id, double ang_vel);
void sim_robot_location(int robot_id, gt_Point position, double theta);
void sim_robot_location(int robot_id, gt_Point position, double theta, GC *gc);
robot_position sim_get_position(int robot_id);
char* sim_get_console_state(int robot_id, char* parm);
void sim_put_console_state(int robot_id, char* parm, char* val);
Obs_List sim_detect_obstacles(int robot_id, double max_dist);

DoorwayList sim_detect_doorway(int robot_id, double sensorDir,
                                double sensorFoV, double sensorRange);
void sim_markunmark_nearest_doorway(int robot_id, int mark_status);
HallwayList sim_detect_hallway(int robot_id, robot_position loc);
bool sim_detect_in_room(int robot_id, robot_position loc);
void gt_init_simulation(char* tcx_home);
RobotReadings sim_detect_robots(int robot_id, double max_dist);
RobotReadings sim_get_robot_pos(int robot_id, double max_dist);
void gt_init_simulation(char* ipt_home);
int gt_simulation_loop(void);
void delete_robot_rec(int robot_id);
int find_first_robot_rec(void);
int find_next_robot_rec(void);
void erase_robots(void);
void draw_robots(void);
void draw_robot(struct T_robot_state* bot);  // need the "struct" keyword here
void erase_robot(struct T_robot_state* bot); // need the "struct" keyword here

void clear_objects(void);

void erase_halo(int robot_id);
void erase_halos(void);
void erase_movement_vectors(void);
void sim_report_obstacles(int robot_id, int cnt, gt_Point* obs);
void sim_report_laser(int robot_id, int num_readings, float* readings, float* angles);
ObjectList sim_detect_objects(int robot_id, double max_dist);
void sim_pickup(int robot_id, int object_id);
void sim_probe(int robot_id, int object_id);
int sim_check_vehicle_type(int robotID, int objectID);
bool sim_check_target_trackable(int robotID, int objectID);
void sim_alert(char* msg);
void sim_drop_in_basket(int robot_id, int object_id, int basket_id);
void sim_change_color(int robot_id, int object_id, char* new_color);
void sim_change_robot_color(int robot_id, char* new_color);
void sim_report_sensors(int robot_id, int cnt, SensorReading* reading);
void sim_set_mobility_type(int robotID, int mobilityType);
int sim_get_mobility(int robotID);
float sim_get_elevation(int robot_id);
//CNP_RequestMess sim_cnp_got_a_task(int roboid);
//void inject_track_task(int tt);
//vector<struct CNP_RobotConstraint> sim_get_init_cnp_robot_constraints(int robotID);
void quit_mlab(void);
void call_quit_mlab(void);


#include "shape.hpp"
#include "list.hpp"
extern GTList<shape*> object_list; // List of all features in the environment

#endif  // GT_SIM_H


/**********************************************************************
 * $Log: gt_sim.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/08/10 04:41:06  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.2  2007/02/08 19:40:43  pulam
 * Added handling of zones (no-fly, weather)
 * Fixed up some of the icarus objectives to add turn limiting
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.14  2006/07/12 06:16:42  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.13  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.12  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.11  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.10  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.9  2006/02/19 17:58:00  endo
 * Experiment related modifications
 *
 * Revision 1.8  2006/02/14 02:28:20  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.7  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.6  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.5  2005/04/08 01:56:03  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:37  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:56  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.75  2003/06/19 20:35:15  endo
 * playback feature added.
 *
 * Revision 1.74  2002/10/31 20:05:29  ebeowulf
 * Added the ability to change the color of a robot.
 *
 * Revision 1.73  2002/07/02 20:41:50  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.72  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.71  2000/12/12 22:43:30  blee
 * Altered sim_report_laser() prototype.
 *
 * Revision 1.70  2000/11/20 19:33:54  sgorbiss
 * Add visualization of line-of-sight communication
 *
 * Revision 1.69  2000/11/10 23:38:53  blee
 * added flags for learning momentum and saving of sensor readings
 *
 * Revision 1.68  2000/07/07 18:25:17  endo
 * extern int waypoint_design_mode; added.
 * void quit_mlab(void); added.
 *
 * Revision 1.67  2000/07/02 00:56:41  conrad
 * rechecked in after goahead
 *
 * Revision 1.65  2000/06/27 22:18:28  conrad
 * added sensor display variables
 *
 * Revision 1.64  2000/04/25 09:53:51  jdiaz
 * added parm to sim_birth
 *
 * Revision 1.63  2000/03/30 01:46:11  endo
 * sim_spin_robot added.
 *
 * Revision 1.62  2000/02/28 23:41:41  jdiaz
 * mark doorway function header added
 *
 * Revision 1.61  2000/02/18 01:52:04  endo
 * sim_alert added.
 *
 * Revision 1.60  2000/02/07 05:12:42  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.59  1999/12/18 09:38:45  endo
 * Data logging and Report current state capabilities
 * added.
 *
 * Revision 1.58  1999/12/16 22:48:25  mjcramer
 * RH6 port
 *
 * Revision 1.54  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.53  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.52  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.51  1996/03/01  00:47:09  doug
 * *** empty log message ***
 *
 * Revision 1.50  1996/02/22  00:56:22  doug
 * *** empty log message ***
 *
 * Revision 1.49  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.48  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.47  1995/10/12  20:13:34  doug
 * Added pickup function
 *
 * Revision 1.46  1995/10/11  20:36:33  doug
 * *** empty log message ***
 *
 * Revision 1.45  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.44  1995/04/27  18:40:37  jmc
 * Removed declaration for clear_map().  Moved it to console.h.
 *
 * Revision 1.43  1995/04/26  22:02:40  doug
 * Converted from tcx to ipt.  Keep your fingers crossed!
 *
 * Revision 1.42  1995/04/20  18:20:17  jmc
 * Corrected spelling.
 *
 * Revision 1.41  1995/04/19  20:17:05  jmc
 * Moved some declarations from here to gt_world.h since gt_world.c
 * is where they are defined.
 *
 * Revision 1.40  1995/04/14  18:25:40  jmc
 * Changed include of doug.h to gt_std.h.
 *
 * Revision 1.39  1995/04/14  15:26:32  jmc
 * Moved clear_world and populate_world to gt_world.h (and renamed
 * them to clear_obstacles and create_obstacles, respectively).
 *
 * Revision 1.38  1995/04/10  16:30:56  jmc
 * Moved some declarations to where they belonged in console.h.
 *
 * Revision 1.37  1995/04/06  19:40:47  jmc
 * Removed WINDOW_HEIGHT and WINDOW_WIDTH.  This is now handled in
 * console.c via the fallback resources so people can easily override
 * them to make mlab whatever size is desired.
 *
 * Revision 1.36  1995/04/03  20:29:28  jmc
 * Added copyright notice.
 *
 * Revision 1.35  1995/02/02  15:39:42  jmc
 * Changed show_movement_vector to show_movement_vectors.  (Made it
 * plural.)
 *
 * Revision 1.34  1995/01/11  21:17:49  doug
 * added new show vector option
 *
 * Revision 1.33  1995/01/10  19:32:58  doug
 * add notify_config msg
 *
 * Revision 1.32  1994/11/29  17:31:06  doug
 * support small worlds
 *
 * Revision 1.31  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.30  1994/11/11  16:32:23  jmc
 * Renamed world_clear to clear_world.
 *
 * Revision 1.29  1994/11/09  21:05:03  doug
 * move robot size to gt_world.c
 *
 * Revision 1.28  1994/11/08  17:14:09  doug
 * added red_xorGC
 *
 * Revision 1.27  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.26  1994/11/03  20:19:16  doug
 * added options button to turn on cthreads scheduler messages
 *
 * Revision 1.25  1994/11/03  18:38:07  jmc
 * Changed draw_halos to highlight_repelling_obstacles.
 *
 * Revision 1.24  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.23  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.22  1994/10/31  18:38:29  jmc
 * Changed meter_per_pixel to meters_per_pixel.
 *
 * Revision 1.21  1994/10/28  00:48:36  jmc
 * Added function prototype for populate_world.
 *
 * Revision 1.20  1994/10/27  18:27:43  jmc
 * Made the robots a little bigger.
 *
 * Revision 1.19  1994/10/27  14:52:04  jmc
 * Changed playing field from 750m to 1000m square.  Tweaked robot
 * size.
 *
 * Revision 1.18  1994/10/26  23:18:15  doug
 * added slider bars and converted to meters
 *
 * Revision 1.17  1994/10/25  15:23:17  jmc
 * Modified the function prototype for send_robot_command.  Minor
 * hack to include files.
 *
 * Revision 1.16  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.15  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.14  1994/10/04  22:21:42  doug
 * Getting execute to work
 *
 * Revision 1.13  1994/10/03  21:14:10  doug
 * *** empty log message ***
 *
 * Revision 1.12  1994/09/03  17:01:48  doug
 * *** empty log message ***
 *
 * Revision 1.11  1994/08/22  16:26:24  doug
 * Added extern for robot_debug_messages
 *
 * Revision 1.10  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.9  1994/08/18  18:40:36  doug
 * console links
 *
 * Revision 1.8  1994/08/04  14:53:40  jmc
 * Removed an unnecessary include <stdarg.h> that I forgot in
 * the previous edit.
 *
 * Revision 1.7  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.6  1994/08/02  20:55:30  jmc
 * Added function prototype for warn_userf (with printf style
 * arguments).
 *
 * Revision 1.5  1994/07/26  16:13:29  jmc
 * Fixed order problem in include files.
 *
 * Revision 1.4  1994/07/22  21:23:39  jmc
 * Added header for warn_user.  Implemented in console.c.
 *
 * Revision 1.3  1994/07/18  19:36:20  jmc
 * Moved gt_configuration_popup to gt_world.h (and renamed it).
 *
 * Revision 1.2  1994/07/14  13:26:37  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
