#ifndef GT_SIMULATION_H
#define GT_SIMULATION_H
/****************************************************************************
*                                                                           *
*                                                                           *
*                      Doug MacKenzie                                       *
*                                                                           *
*  header - define global stuff                                             *
*                                                                           *
*  Copyright 1995 - 2006, Georgia Tech Research Corporation                 *
*  Atlanta, Georgia  30332-0415                                             *
*  ALL RIGHTS RESERVED, See file COPYRIGHT for details.                     *
*                                                                           *
****************************************************************************/

/* $Id: gt_simulation.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */


#include <sys/time.h>
#include <vector>
#include <string>

#include "gt_robot_type.h"
#include "gt_com.h"
#include "gt_std.h"
#include "gt_std_types.h"
#include "gt_message_types.h"
#include "lineutils.h"
#include "CNPConstraint.h"
#include "overlay_zones.h"

using std::vector;
using std::string;

// ENDO - gcc 3.4: The following moved to gt_std.h
/*
struct gt_Point_array
{
    int num_points;
    gt_Point *point;
};

struct gt_Point_list
{
    float  x;
    float  y;
    gt_Point_list* next;
};

struct robot_position
{
    Vector v;
    double heading;
};

struct gps_position
{
    double latitude;
    double longitude;
    double direction;
    double pingtime;
    int pingloss;
};

// ENDO - gcc 3.4: Moved to lineutils.h
struct room_t
{
    lineRec2d_t* walls;
    room_t* next;
};

struct door_t
{
    lineRec2d_t* doorway;
    int marked;
    door_t* next;
};

struct hallwayRec_t
{
    Point start;
    Point end;
    double width;
    lineRec2d_t *extents;
    hallwayRec_t* next;
};

struct type_pry {
    int status;  // 0: successful
                 // 1: serial port not open
                 // 2: reading corrupted or no reading
    double shaft_heading;
    double pitch;          // 0 = level, 90 = front up
    double roll;           // 0 = level, 90 = banked right
    double yaw;            // 0 = east, 90 = north
};
*/

typedef enum CSB_NodeType_t
{
    CSB_NODE_SELF,
    CSB_NODE_OTHER
};

typedef struct CSB_CommInfo_t
{
    int id;
    int numHops;
    double signalStrength;
    int nodeType;
    double heading;
    double latitude;
    double longitude;
    double x;
    double y;
    double z;
};

typedef vector<CSB_CommInfo_t> CSB_CommInfoList_t;

typedef struct CSB_Advise_t
{
    double x;
    double y;
    double z;
    double confidence;
};

#define ASM_PT	1
#define BB_PT	2
#define HEADING_INVALID 999

#define SAFETY_MARGIN_MSG   "SAFETY_MARGIN" // return safety margin
#define RSP_MSG             "EXEC_STATE"    // return executive state
#define ROBOT_TYPE_MSG      "robot_type"
#define RUN_TYPE_MSG        "run_type"
#define TIME_OUT_MSG        "timeout"
#define COMPASS_TYPE_MSG    "compass_type"

#define DONE_MSG            "Command Finished"
#define EXECUTING_MSG       "Executing Command"
#define ERROR_MSG           "Error Encountered"
#define UNKNOWN_MSG         "Unknown msg state"

/*
enum RobotType { HOLONOMIC, HSERVER };
const char* const ROBOT_TYPE_NAMES[] = { "HOLONOMIC", "HSERVER" };
extern RobotType robot_type;

const char* const RUN_TYPE_NAMES[] = { "SIMULATION", "REAL" };
enum RunType { SIMULATION, REAL };
*/

enum COMPASS_TYPES { NOCOMPASS, C100, DGSSS };
const char* const COMPASS_TYPE_NAMES[] = { "NOCOMPASS", "C100", "DGSSS" };

/*
// This should match with Robothandler::RobotType_t in
// robothandler.h.
typedef enum RobotMobilityType_t {
    UNKNOWN_ROBOT_MOBILITY_TYPE = -1,
    UGV = 0,
    URV,
    UUV,
    USV,
    UAV,
    SLUAVU,
    SLUAVL,
    NUM_ROBOT_MOBILITY_TYPES // Keep this at the end of the list.
};
*/

Vector compute_dock_force_on_robot( Vector robotpos, Vector dockpos,
                                    double dockxy, double controlled_radius,
                                    double cone_angle, double ballistic_gain,
                                    double controlled_gain );

void exec_log_robot_data( Vector v, robot_position rp, int state_num );
void exec_set_turn_mode( int mode );
void exec_spin_robot( float ang_velocity, float slippageFactor );
int exec_new_robot( int request_id, Vector v );
void exec_move_robot( Vector v );
void exec_update_console_lm_params();
robot_position exec_get_position();
gps_position exec_get_gps(); //trw lilia
void exec_get_battery_info(); //trw zk
obs_array exec_detect_obstacles( float max_dist );
void halt( char* msg );
void exec_terminate_mission(void);
void open_simulation_link( char* thisrv_host, int our_port,
                           char* server_host, int server_port,
                           float start_x, float start_y, int request_id );
void usage( char* name );
void init( int argc, char** argv );
RobotReadings exec_detect_robots( float max_dist );
vector<GTRobotPosID_t> exec_get_sim_robots_pos(double maxDist);
int exec_get_last_key_press();
Vector exec_get_last_button_press();
int exec_get_robot_state( int robot_id );
void exec_put_robot_state( int state );
void exec_pickup_object( int object_id );
void exec_probe_object( int object_id );
int exec_check_vehicle_type(int objectID);
bool exec_check_target_untrackable(int objectID);
void exec_alert( char* alertsubject, char* alertmessage, bool use_email, char* mailrecipnt, bool attach_image );
int exec_send_email( char* mailrecipnt, char sbj[ 256 ], char* bodyfile );
void exec_popup_telop_interface();
void exec_change_object_color( int object_id, char* new_color );
void exec_change_robot_color( char* new_color );
void exec_drop_object_in_basket( int object_id, int basket_id );
void exec_write( Vector loc, int cmd, char text[ 16 ] );
void exec_put_console_state( char* parm, char* val );

// update the local database key with the new value and then
// send the change to the console, with instructions for it to
// broadcast the update to all robots.
void exec_broadcast_value( char* key, char* value, int via );

void exec_reset_world( char* new_overlay, int load_new_map );
void exec_add_object( char* color, int style, Vector pos, double diameter );
void exec_set_compass_on();
void exec_set_compass_off();
void exec_setxy( double x, double y );
void exec_setxyTheta( double x, double y, double theta );
int exec_get_compass_heading();
void exec_update_compass_window();
bool exec_detect_doorway( double hallway_width, double robot_length, int look_for, double max_dist );
bool exec_detect_unmarked_doorway( double hallway_width, double robot_length, int look_for, double max_dist );

void exec_mark_nearest_doorway();
void exec_unmark_nearest_doorway();
bool exec_detect_nearest_doorway( Vector* pt1, Vector* pt2, int* newtime );
bool exec_detect_nearest_unmarked_doorway( Vector* pt1, Vector* pt2 );
int exec_detect_hallway( robot_position robot_loc, Vector* start, Vector* end, double* width, int* timestamp );
int exec_detect_alternate_hallway_info_at_random( robot_position robot_loc, int seed, Vector* pt1,
                                                  Vector* pt2, double* width, double* dx, double* dy );
bool exec_detect_in_hallway( robot_position robot_loc );
bool exec_detect_alternate_hallway( robot_position robot_loc );
bool exec_detect_in_room( robot_position robot_loc );
int correctside( double xx1, double yy1, double xx2, double yy2, double xx3, double yy3 );
raw_sonar_array exec_get_raw_sonar();

ObjectList  exec_detect_objects( float max_dist );
void exec_move_camera( Vector direction  );
Vector exec_get_camera_heading();
gt_Command exec_get_command();
void end_of_cycle();
Vector exec_get_map_position( char* name );
type_pry  exec_get_pry();

int exec_detect_sound();             // sound stuff
void exec_talk( char* talkmessage ); // talk stuff

char *get_state( const char* key );
char *return_state( char* key );
int put_state( char* key, char *value );
void exec_set_camera_tracker_mode(int mode);
void exec_set_mobility_type(int mobilityType);
float exec_get_elevation(void);

bool exec_cnp_got_a_bidding_task(void);
bool exec_cnp_is_task_changed(void);
void exec_cnp_bid_on_task(void);
bool exec_cnp_is_auction_ended(void);
bool exec_cnp_is_auction_failed(void);
bool exec_cnp_is_task_lost(void);
bool exec_cnp_is_task_won(void);
bool exec_cnp_won_task_is(string cnpTaskName);
Vector exec_cnp_get_track_pos(void);
void exec_cnp_cancel_task(bool isReneged, string msg);
bool exec_cnp_task_completion_notified(
    bool selectWonCNPTask,
    string specificCNPTaskName);
bool exec_cnp_task_reneging_notified(
    bool selectWonCNPTask,
    string specificCNPTaskName);
void exec_send_cnp_task_constraints(
    vector<struct CNP_TaskConstraint> cnpTaskConstraints);
void exec_notify_cnp_task_completed(void);
void exec_notify_cnp_task_reneged(void);
void exec_notify_cnp_won_task_started(void);
void exec_receive_cnp_robot_constraints(void);

// SLUAV PU
robot_position exec_get_robot_position(int rid);


int get_state_vector( char* context, char *name, Vector* retval );
int get_state_double( char* context, char *name, double* retval );
int put_state_vector( char* context, char *name, Vector value );
int put_state_double( char* context, char *name, double value );
void exec_report_current_state( char* msg );
void exec_ask_to_proceed();
int exec_ping_console( double* ping_time );
void exec_notify_config(void);
CSB_CommInfoList_t exec_get_csb_comm_info(void);
CSB_Advise_t exec_get_csb_advise(int method);
void exec_init_csb(void);
bool exec_check_csb_initialized(void);
void exec_update_csb_sensor_data(void);
void exec_update_csb_carretilla(void);
void exec_set_signal_threshold_low(float threshold);
void exec_set_signal_threshold_high(float threshold);
float exec_get_signal_level_carretilla(void);
int exec_get_general_battery_level_carretilla();
int exec_get_servo_battery_level_carretilla();


void exec_mover_roomba(Vector v);

Vector exec_calcular_vector_roomba(Vector v, double location_gain, double obstacle_gain, double sphere);

bool pertenece(double valor, double centro , double margen);

void exec_get_sensor_roomba(int *num_lecturas, float *lecturas);

void exec_activar_motores_limpieza(bool);

void enviar_broadCast(int);
void escucho_broadCast(bool);
void enviarPosicion(robot_position);
robot_position posicion_lider(void);
float modulo(Vector , Vector);
void enviar_cambio_estado(int);
bool cambio_estado(void);
void poner_cambio_estado(bool);
int get_num_esclavos(void);
void decrementar_num_esclavos(void);
void incrementar_num_esclavos(void);
void almacenar_posicion(Vector);
Vector get_posicion_objetivo(void); // pregunta al lider la posicion a la que debe ir
Vector posicion_objetivo(void); // devuelve la posicion objetivo. Antes se debe ejecutar get_posicion_objetivo
bool hay_lider(void);
void sigo_lider(void);
void set_primera(int);
int get_primera(void);
void set_inicial_nueva_tarea(int val);
int get_inicial_nueva_tarea(void);
Vector posicion_ant_lider(void);
void set_posicion_ant_lider(Vector v);
Vector posicion_habitacion(void);
obs_array  exec_detect_obstacles_mio(float max_dist);
void exec_send_advanced_telop(int joy_x, int joy_y, int slider1, int slider2, int joy_buttons);
void exec_carmen_navigate(double x, double y);
void exec_read_carmen_advise();
void exec_init_carmen_navigate();

extern int debug;

extern int report_current_state;
extern int log_robot_data;
extern unsigned long current_cycle;
extern int save_learning_momentum_weights;
extern int save_sensor_readings;

extern vector<WeatherZone *> WeatherZones;
extern vector<NoFlyZone *> NoFlyZones;
extern vector<NoCommZone *> NoCommZones;

#endif  // GT_SIMULATION_H



/**********************************************************************
 * $Log: gt_simulation.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.14  2006/06/08 22:33:55  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.13  2006/05/14 06:23:36  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.12  2006/05/06 04:52:33  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.11  2006/05/02 04:12:21  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.10  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.9  2006/02/19 17:56:41  endo
 * Experiment related modifications
 *
 * Revision 1.8  2006/01/10 06:22:00  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.7  2005/07/27 20:37:53  endo
 * 3D visualization improved.
 *
 * Revision 1.6  2005/07/23 05:57:14  endo
 * 3D visualization improved.
 *
 * Revision 1.5  2005/04/08 01:55:56  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:33  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:53  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:47:33  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.94  2003/04/02 23:08:41  zkira
 * Added exec_get_gps and exec_get_battery functions
 *
 * Revision 1.93  2002/10/31 21:28:00  ebeowulf
 * Added messaging and header for Reset World code.
 *
 * Revision 1.92  2002/10/31 20:00:00  ebeowulf
 * added change robot color function and message
 *
 * Revision 1.91  2002/07/02 20:29:55  blee
 * changed ROBOT_TYPE and RUN_TYPE to RobotType and RunType
 *
 * Revision 1.90  2001/12/23 20:30:32  endo
 * RH 7.1 porting.
 *
 * Revision 1.89  2001/01/24 23:05:31  saho
 * Updated  exec_setxy() to take double instead of float. Bug fix.
 *
 * Revision 1.88  2000/11/10 23:27:54  blee
 * added: save_learning_momentum_weights, save_sensor_readings,
 * and exec_update_console_lm_params
 *
 * Revision 1.87  2000/09/19 10:04:41  endo
 * exec_setxyTheta() bug fixed.
 *
 * Revision 1.86  2000/08/19 00:57:49  endo
 * exec_ping_console added.
 *
 * Revision 1.85  2000/08/15 20:51:48  endo
 * exec_ask_to_proceed added.
 *
 * Revision 1.84  2000/08/12 20:11:33  saho
 * Fixes for exec_detect_hallway.
 *
 * Revision 1.83  2000/07/02 02:42:46  conrad
 * *** empty log message ***
 *
 * Revision 1.82  2000/07/02 01:15:27  conrad
 * added hserver support
 *
 * Revision 1.81  2000/06/13 16:46:33  endo
 * exec_broadcast_value modified to take "int via".
 *
 * Revision 1.80  2000/05/19 21:38:19  endo
 * exec_popup_telop_interface added.
 *
 * Revision 1.79  2000/04/19 21:27:56  jdiaz
 * added headers for the new put_state and get_state methods
 *
 * Revision 1.78  2000/04/13 22:29:37  endo
 * Checked in for Doug.
 *
 * Revision 1.77  2000/03/30 15:53:03  saho
 * Added exec_talk().
 *
 * Revision 1.76  2000/03/22 02:31:37  saho
 * Added a declaration for void exec_setxyTheta(double x, double y, double theta);
 * and deleted setxy() which was no nlonger present in robot_side_com.c .
 *
 * Revision 1.75  2000/02/29 21:04:00  saho
 * Added declaration for  exec_detect_sound(void);
 *
 * Revision 1.74  2000/02/28 23:51:06  jdiaz
 * detect unmarked doorway and mark doorway
 *
 * Revision 1.73  2000/02/18 22:40:35  conrad
 * added nomad200
 *
 * Revision 1.72  2000/02/18 02:03:34  endo
 * exec_alert, exec_send_email added.
 *
 * Revision 1.71  2000/02/07 05:21:04  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.70  1999/12/18 09:23:19  endo
 * exec_log_robot_data() added.
 * exec_report_current_state() added.
 *
 * Revision 1.69  1999/12/16 22:59:41  mjcramer
 * rh6 port
 *
 * Revision 1.62  1999/08/11 21:47:26  jdiaz
 * added correctside function definition
 *
 * Revision 1.61  1999/07/03 01:07:05  mjcramer
 * Added camera heading stuff
 *
 * Revision 1.60  1999/06/29 02:45:42  mjcramer
 * Added camera stuff
 *
 * Revision 1.59  1999/06/28 20:15:59  endo
 * setxy added.
 *
 * Revision 1.58  1999/06/23 18:10:43  jdiaz
 * added exec_setxy
 *
 * Revision 1.57  1999/06/11 20:19:31  conrad
 * added URBAN
 *
 * Revision 1.56  1999/06/07 03:02:49  jdiaz
 * added declaration for exec_get_raw_sonar
 *
 * Revision 1.55  1999/05/31 01:09:17  conrad
 * *** empty log message ***
 *
 * Revision 1.54  1999/05/17 16:57:51  jdiaz
 * support for reading and displaying the current compass reading.
 *
 * Revision 1.53  1999/04/27 09:41:41  conrad
 * Added support for kyle.
 *
 * Revision 1.52  1998/11/22 21:09:42  bas
 * Added PIONEERAT to simulation.
 *
 * Revision 1.51  1997/03/08 04:57:13  zchen
 * *** empty log message ***
 *
 * Revision 1.50  1996/12/17  19:13:14  tucker
 * added NOMAD150 types
 *
 * Revision 1.49  1996/10/04  21:18:26  doug
 * fixes for linux
 *
 * Revision 1.49  1996/10/01 13:02:43  doug
 * got linux version working
 *
 * Revision 1.48  1996/05/02 20:14:33  kali
 * changed pry_type to type_pry because it was causing problems with the
 * CNL compiler.
 *
 * Revision 1.47  1996/04/30  22:06:34  kali
 * fixed typo
 *
 * Revision 1.46  1996/04/30  19:09:03  kali
 * added shaft_heading field to pry_type
 *
 * Revision 1.45  1996/04/30  19:04:00  kali
 * *** empty log message ***
 *
 * Revision 1.44  1996/04/30  18:02:44  kali
 * added declaration of exec_getpry function
 *
 * Revision 1.43  1996/04/30  17:20:37  kali
 * added enum COMPASS_TYPES
 *
 * Revision 1.42  1996/04/30  17:08:34  kali
 * added pry_type and COMPASS_TYPE_MSG
 *
 * Revision 1.41  1996/03/09  01:09:46  doug
 * *** empty log message ***
 *
 * Revision 1.40  1996/02/28  03:55:12  doug
 * *** empty log message ***
 *
 * Revision 1.39  1996/02/27  22:13:43  doug
 * *** empty log message ***
 *
 * Revision 1.38  1996/02/21  15:44:14  doug
 * *** empty log message ***
 *
 * Revision 1.37  1996/02/01  19:55:18  doug
 * added current_cycle as a global
 *
 * Revision 1.36  1995/11/08  16:51:45  doug
 * *** empty log message ***
 *
 * Revision 1.35  1995/10/18  14:01:14  doug
 * *** empty log message ***
 *
 * Revision 1.34  1995/10/12  20:02:06  doug
 * add pickup funcion
 *
 * Revision 1.33  1995/10/03  21:23:09  doug
 * added prototype for get_objects call
 *
 * Revision 1.32  1995/04/14  18:25:19  jmc
 * Changed include of doug.h to gt_std.h and
 * dougs_types.h to gt_std_types.h.
 *
 * Revision 1.31  1995/03/31  22:08:35  jmc
 * Added a copyright notice.
 *
 * Revision 1.30  1995/01/13  16:52:15  doug
 * added UNICYCLE_HUMMER as robot type
 *
 * Revision 1.29  1995/01/10  19:32:13  doug
 * added RUN_TYPES def
 *
 * Revision 1.28  1995/01/10  16:58:25  doug
 * make run_type a seperate list and add HUMMER
 *
 * Revision 1.27  1994/11/22  21:48:08  doug
 * added heading parm to detect_obs call
 *
 * Revision 1.26  1994/11/18  00:37:05  doug
 * added configuration messages for the denning robots
 *
 * Revision 1.25  1994/11/17  23:44:22  doug
 * added array of names to match robot types
 *
 * Revision 1.24  1994/11/16  15:12:37  doug
 * added robot_type enum and extern
 *
 * Revision 1.23  1994/11/08  14:52:44  doug
 * added SAFETY_MARGIN_MSG
 *
 * Revision 1.22  1994/11/04  03:30:45  doug
 * added end_of_cycle call
 *
 * Revision 1.21  1994/11/02  21:53:27  doug
 * fixed type on exec_get_obs_list
 *
 * Revision 1.20  1994/11/01  23:45:47  doug
 * *** empty log message ***
 *
 * Revision 1.19  1994/10/25  15:24:52  jmc
 * Minor readability hacks.
 *
 * Revision 1.18  1994/10/19  21:29:18  doug
 * added return_state
 *
 * Revision 1.17  1994/10/19  14:24:24  doug
 * *** empty log message ***
 *
 * Revision 1.16  1994/10/04  19:41:44  doug
 * fixed halt prototype
 *
 * Revision 1.15  1994/10/03  19:03:51  doug
 * added get_state, put_state prototypes
 *
 * Revision 1.14  1994/09/28  20:21:57  doug
 * Now use tcx instead of rpc
 *
 * Revision 1.13  1994/09/03  13:33:06  doug
 * Added defines for messages returned from robots
 *
 * Revision 1.12  1994/08/22  16:38:33  doug
 * Added RSP_MSG define
 *
 * Revision 1.11  1994/08/22  15:52:39  doug
 * Removed command packet code since now in rpc header
 *
 * Revision 1.10  1994/08/19  17:08:54  doug
 * rpc is working
 *
 * Revision 1.9  1994/08/18  20:04:56  doug
 * moved gt_std.h doug.h
 *
 * Revision 1.8  1994/08/18  19:33:43  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.7  1994/08/17  22:55:09  doug
 * added link to rpc include
 *
 * Revision 1.6  1994/08/16  19:23:00  jmc
 * Added type for gt_Point_array.  Updated definition of gt_Command
 * to add more phase line info, on-completion info, and "until" info.
 *
 * Revision 1.5  1994/08/12  14:36:46  jmc
 * Added speed to gt_Command and fixed_gt_Command.
 *
 * Revision 1.4  1994/08/11  19:36:25  doug
 * new_robot now returns the id that was assigned.
 *
 * Revision 1.3  1994/07/14  13:40:58  jmc
 * Added extern "C"'s on several function prototypes
 *
 * Revision 1.2  1994/07/14  13:20:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
