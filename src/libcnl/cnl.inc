/**********************************************************************
 **                                                                  **
 **                             cnl.inc                              **
 **                                                                  **
 **                                                                  **
 **  Written by: Douglas C. MacKenzie                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research                     **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cnl.inc,v 1.1.1.1 2006/07/12 13:37:59 endo Exp $ */


#ifndef CNL_INC
#define CNL_INC

init
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <string>
#include <unistd.h>

#include "gt_simulation.h"
#include "cnl.h"
#include "gt_std_types.h"
#include "gt_std.h"
#include "FSA_status.h"
#include "qlearn.h"

using std::cerr;
using std::string;


iend


/* Declare the types that CNL will use */
type <vector_array,"{int, <{double, double, double}:1>}">;
type <obs_array,"{int, <{{double, double, double}, double}:1>}">;
type <raw_sonar_array,"{int, <{double}:1>}">;
type <Vector,"{double, double, double}">;
type <Robots,"{int, [int:10], [{double, double, double}:10]}">;
type <robot_position,"{{double, double, double}, double}">;
type <gt_Formation,"int">;
type <gt_Unit_Members,"{int, [int:64]}">;
type <CNLString_t,"string">;
type <bool,"boolean">;
type <boolean,"boolean">;
type <gt_Command,"{int, <{float, float}: 1>, int, <int: 3>, int, <{float, float}: 5>, {float, float}, int, int, int, float, float, int, int, string, int, int, string, int, int, int, int}">;
type <ObjectList,"{int, <{int, int, int, float, float, float, float, float}: 1>}">;
type <ObjectReading,"{int, int, int, float, float, float, float, float}">;
type <ObjectClasses,"int">;
type <OneObjectClass,"int">;
type <type_pry,"int,double,double,double,double">;
type <Reinforcer, "int, int, double">;

/********************************************************/

external procedure Vector MOVE_TO_GOAL_DSTAR with
   Vector 	  goal_rel_loc;
   obs_array      obstacles;
   robot_position pos;
   double         gridsize;
   double         length;
   double         width;
   double         ao_val;
   double         goal_gain;
   double         persistence;
   double         angle_dev;
   Vector         ao_vec;
pend


external procedure void DRIVE with
   Vector v;
   double max_vel;
   double base_vel;
   double cautious_vel;
   bool cautious_mode;
pend

external procedure void DRIVE_ROOMBA with
   Vector v;
   double max_vel;
   double base_vel;
   double cautious_vel;
   bool cautious_mode;
pend

external procedure int GET_COMPASS_HEADING with
pend

external procedure raw_sonar_array RAW_ULTRASONICS with
pend

external procedure bool DOORWAY_ULTRASOUND with
	double robot_length;
	double hallway_width;
	int look_for;
	int unmarked;
	double max_dist;
pend

external procedure Vector MARK_DOORWAY with
pend

external procedure bool MARKED_DOORWAY with
pend

external procedure Vector UNMARK_DOORWAY with
pend

external procedure bool UNMARKED_DOORWAY with
pend

external procedure Vector VECTOR_COMPASS_DIFFERENTIAL with
   double Desired_heading;
   int Compass_heading;
   double Tolerance;
pend

external procedure Vector STAY_ON_PATH with
   Vector Start;
   Vector End;
   double Path_Width;
pend

external procedure void DRIVE_W_SPIN with
   Vector v;
   double max_vel;
   double base_vel;
   double cautious_vel;
   bool cautious_mode;
pend

external procedure void MOVE_ROBOT with
   Vector v;
   double max_vel;
   double base_vel;
   double cautious_vel;
   bool cautious_mode;
pend

external procedure Vector MOVE_AHEAD with
   double       heading;
pend

external procedure Vector MOVE_TO_GOAL with
   Vector goal_rel_loc;
   double success_radius;
pend

external procedure Vector MOVE_TO with
   Vector goal_rel_loc;
   bool have_a_goal;
pend

external procedure Vector IR_A_ROOMBA with
    Vector goal_rel_loc;
    double location_gain;
	double obstacle_gain;
	double sphere;
pend

external procedure Vector IR_POSICION_OBJETIVO with
    Vector obstaculo;
pend

external procedure Vector ASPIRAR with
pend

external procedure Vector ENTRAR_HABITACION with
    Vector entrar;
    Vector ostaculos;
    Vector teleop;
    double pesoEntrar;
    double pesoObstaculos;
    double pesoTeleop;
pend

external procedure Vector GUIAR_ROBOT with
    int num;
    Vector pasillo;
pend

external procedure Vector GUIAR with
    Vector moverPasillo;
    //Vector obstaculos;
    Vector estarEnPasillo;
pend


external procedure Vector SEGUIR_LIDER with
    Vector obstaculo;
pend

//external procedure Vector SEGUIR_LIDER with
//   obs_array      obstacles;
//   robot_position pos;
//   double         gridsize;
//   double         length;
//   double         width;
//   double         ao_val;
//   double         goal_gain;
//   double         persistence;
//   double         angle_dev;
//   Vector         ao_vec;
//pend

external procedure Vector IR_POSICON_ANT_LIDER with
   obs_array      obstacles;
   robot_position pos;
   double         gridsize;
   double         length;
   double         width;
   double         ao_val;
   double         goal_gain;
   double         persistence;
   double         angle_dev;
   Vector         ao_vec;
pend

external procedure Vector ASPIRAR560 with
pend

external procedure Vector FOLLOW_CSB_ADVISE with
   int method;
pend

external procedure Vector GROUSAL_FOLLOW_CSB_ADVISE with
   int method;
pend

external procedure Vector INIT_CSB with
pend

external procedure Vector UPDATE_CSB_SENSOR_DATA with
pend

external procedure bool COMM_SIGNAL_STRONG_ENOUGH with
   double threshold;
pend

external procedure Vector CARMEN_NAVIGATE with
   Vector goal;
pend

external procedure bool GROUSAL_COMM_SIGNAL_STRONG_ENOUGH with
   double threshold;
pend

external procedure bool GROUSAL_SERVO_BATTERY_CHARGED with
   double threshold;
pend

external procedure bool GROUSAL_GENERAL_BATTERY_CHARGED with
   double threshold;
pend

external procedure bool CSB_CONFIDENCE_HIGH_ENOUGH with
   int method;
   double threshold;
pend

external procedure robot_position SHAFTENCODERS with
pend

external procedure Vector  SET_GLOBAL_TRIGGER_FLAG with
 CNLString_t  key;
pend

external procedure Vector  SET_TRIGGER_FLAG with
 CNLString_t  key;
pend

external procedure bool IS_TRIGGER_FLAG_SET with
 CNLString_t  key;
pend

external procedure Vector DETECT_GOAL_WITH_SHAFT_ENCODERS with
   Vector goal;
   Vector robot_loc;
pend

external procedure int GET_ROBOT_STATE with
   int robot_id;
pend

external procedure int V_IS_ZERO with
   Vector v;
pend

external procedure Vector LAST_BUTTON_PRESS with
pend

external procedure void PUT_ROBOT_STATE with
   int state;
pend

external procedure Vector GLOBAL_TO_EGOCENTRIC with
   Vector global;
   Vector robot_location;
   double robot_heading;
pend

external procedure Vector POS_ANT_LIDER_REL_LOC with
  robot_position pos;
pend

external procedure bool IN_ROOM with
pend

external procedure bool IN_HALLWAY with
pend

external procedure bool IS_TARGET_UNTRACKABLE with
   ObjectReading object;
pend

external procedure bool CNP_IS_TASK_WON with
pend

external procedure bool CNP_WON_TASK_IS with
   CNLString_t taskName;
pend

external procedure bool CNP_IS_TASK_CHANGED with
pend

external procedure bool CNP_IS_TASK_LOST with
pend

external procedure bool CNP_GOT_A_TASK with
pend

external procedure Vector CNP_BID_ON_TASK with
pend

// Obsolete: Use CNP_NOTIFY_TASK_COMPLETED instad.
//external procedure Vector CNP_RENEG_TASK with
//pend

external procedure Vector CNP_TRACK with
pend

external procedure Vector CNP_INJECT_TASK with
   int taskID;
   CNLString_t taskName;
   CNLString_t constraintsString;
pend

external procedure bool CNP_IS_AUCTION_ENDED with
pend

external procedure bool CNP_IS_AUCTION_FAILED with
pend

external procedure Vector CNP_NOTIFY_TASK_COMPLETED with
    bool reneged;
pend

external procedure Vector CNP_NOTIFY_WON_TASK_STARTED with
pend

external procedure bool CNP_TASK_COMPLETION_NOTIFIED with
   int cnpTaskSelection;
   CNLString_t specificTaskName;
   bool reneged;
pend

external procedure Vector CNP_SAVE_TARGET_LOCATION with
   ObjectReading object;
pend

external procedure Vector CNP_SAVE_TARGET_VELOCITY with
   ObjectReading object;
pend

external procedure Vector CNP_SAVE_TARGET_VEHICLE_TYPE with
   ObjectReading object;
pend

external procedure Vector ENTER_ROOM with
   double distance;
   int unmarked;
pend

external procedure Vector ENTER_DOORWAY with
   double distance;
   int unmarked;
pend

external procedure bool THROUGH_DOORWAY with
   int unmarked;
pend

external procedure Vector LEAVE_ROOM with
pend

external procedure Vector ENTER_ALTERNATE_HALLWAY with
double distance;
pend

external procedure bool DETECT_ALTERNATE_HALLWAY with
pend

external procedure Vector DOCK with
   robot_position robotloc; Vector dockpos;
   double dockxy; double controlled_radius;
   double cone_angle; double ballistic_gain;
   double controlled_gain;
pend

external procedure Vector SUM_WEIGHTED_VECTORS with
   list Vector v;
   list double gain;
   CNLString_t name;
pend

external procedure Vector COOP with
   list Vector members;
   list double weight;
pend

external procedure Vector GET_XY with
   robot_position cur_pos;
pend

external procedure double GET_HEADING with
   robot_position cur_pos;
pend

external procedure robot_position GET_LOCATION with
pend

external procedure Robots DETECT_ROBOTS with
   double max_dist;
pend

external procedure Vector NOISE with
   int    persistence;
   double robot_heading;
pend

external procedure int LAST_KEY_PRESS with
pend

external procedure Vector PROBE with
   double       sphere;
   double       safety_margin;
   obs_array    readings;
pend

external procedure Vector ALERT with
   CNLString_t  alertsubject;
   CNLString_t  alertmessage;
   bool    use_email;
   CNLString_t  mail_recipient;
   bool    attach_image;
pend

external procedure bool ALERTED_ONCE with
   bool    value;
pend

external procedure Vector NOTIFY with
   CNLString_t  notifymessage;
pend

external procedure Vector NOTIFY_ROBOTS with
   CNLString_t  notifymessage;
pend

external procedure bool NOTIFIED_ONCE with
   CNLString_t  notifymessage;
pend

external procedure Vector NOTIFY_ROBOTS_OBJECT_LOCATION with
   ObjectReading object;
pend

external procedure bool IS_OBJECT_LOCATION_NOTIFIED with
pend

external procedure Vector NOTIFIED_OBJECT_LOCATION with
pend

external procedure Vector START_SUB_MISSION with
   CNLString_t sub_mission_name;
   int deployment_method;
   Vector goal_rel_loc;
pend

external procedure bool SUB_MISSION_READY with
   int sub_mission_deployment;
   boolean at_goal_trigger;
pend

external procedure bool IS_NOT_PROGRESSING with
   Vector       desired_location;
   double       patient;
pend

external procedure obs_array DETECT_OBSTACLES with
   double max_sensor_range;
pend

external procedure obs_array ULTRASONICS with
   double max_sensor_range;
   double heading;
pend

external procedure Vector AVOID_STATIC_OBSTACLES with
   double       sphere;
   double       safety_margin;
   obs_array    readings;
pend

external procedure Vector AVOID_ROBOT with
   double  sphere;
   double  min_range;
   Robots  readings;
pend

external procedure Vector DETECT_CENTER_OF_MASS with
   Robots   readings;
pend


external procedure double DETECT_FORM_HEADING with
   Vector  relative_goal;
   Vector  telop_input;
   double  telop_mag;
   double  default_trigger;
   double  robot_heading;
pend


external procedure Vector DETECT_FORM_POSITION with
   double	    spacing;
   gt_Unit_Members  members;
   Vector	    unit_center_relative;
   Vector	    leader_center_relative;
   double	    formation_heading;
   Vector           robot_pos;
   double  	    robot_heading;
   gt_Formation	    formation_type;
   int		    formation_ref;
pend

external procedure Vector COMPUTE_FORM_POSITION with
	double		spacing;
	int	        unit_size;
	Vector		unit_center_relative;
	double      formation_heading;
	Vector          robot_location;
	double          robot_heading;
	CNLString_t          formation_name;
pend


external procedure Vector GET_HALLWAY_INFO_VECTOR with
  CNLString_t  key;
  int Direction;
pend

external procedure double GET_HALLWAY_INFO_DOUBLE with
  CNLString_t  key;
pend

external procedure bool DETECT_IN_POSITION with
   Vector	offset;
   double	slop;
pend


external procedure Vector DETECT_LEADER_CENTER with
   double		spacing;
   double		form_heading;
   gt_Formation	form_type;
   Robots		readings;
   gt_Unit_Members	members;
pend

external procedure Vector DETECT_UNIT_CENTER with
   Robots		readings;
   gt_Unit_Members	members;
pend


external procedure Vector DETECT_UNIT_CENTER_GLOBAL with
   Robots	readings;
   Vector	own_pos;
pend


external procedure double DATABASE_DOUBLE with
   CNLString_t  key;
   double  initial;
pend

external procedure bool SET_DATABASE_DOUBLE with
   CNLString_t  key;
   double  value;
pend

external procedure int DATABASE_INT with
   CNLString_t  key;
   int     initial;
pend

external procedure bool DATABASE_BOOLEAN with
   CNLString_t  key;
   bool     initial;
pend

// This CNL procedure (in tandam with UPDATE_DISTRIBUTED_FLAG) maintains a
// boolean value, distributed between all the robots and the missionlab console.
external procedure bool TEST_DISTRIBUTED_FLAG with
   CNLString_t  key;
	bool    default_value;
pend

// To simplify using on a transition, UPDATE_DISTRIBUTED_FLAG outputs true.
external procedure bool UPDATE_DISTRIBUTED_FLAG with
   CNLString_t  key;
	bool    value;
pend

external procedure Vector SWIRL_STATIC_OBSTACLES with
   double	sphere;
   double	safety_margin;
   obs_array    readings;
   Vector       goal_rel_loc;
pend

external procedure obs_array REMOVE_ROBOTS with
   obs_array mixed;
   Robots robots;
pend

external procedure Vector MAINTAIN_FORM with
 Vector  offset;                 /* vector to proper position */
 double  dead_zone_radius;       /* distance of allowed slop */
 double  saturation_length;      /* spring saturates at dzr + sl */
pend

external procedure Vector SUBTRACT_VECTORS with
   Vector A;
   Vector B;
pend

external procedure Vector MDL_coordinator with
   Vector moveto;
   Vector teleop;
   Vector occupy;
   bool moveto_done;
   bool teleop_done;
   bool occupy_done;
pend

external procedure bool AND with
   bool a;
   bool b;
pend

external procedure bool OR with
   bool a;
   bool b;
pend

external procedure bool NOR with
   bool a;
   bool b;
pend

external procedure bool NOT with
   bool a;
pend

external procedure Vector TELOP with
   int telop_mode;
   double robot_heading;
   double joystick_x;
   double joystick_y;
   double joystick2_x;
   double joystick2_y;
   double slider_1;
   double slider_2;   
   int joy_buttons;
pend

external procedure Vector GROUSAL_TELOP_CSB with
   int telop_mode;
   double robot_heading;
   double joystick_x;
   double joystick_y;
   double joystick2_x;
   double joystick2_y;
   double slider_1;
   double slider_2;   
   int joy_buttons;
pend

external procedure Vector POPUP_TELOP_INTERFACE with
pend

external procedure Vector SOUND with
   double robot_heading;
	double sound_direction_x;
	double sound_direction_y;
pend

external procedure bool DETECT_SOUND with
   double volume_threshold;
pend

external procedure bool DETECT_SOUND_MOTIVATED with
   double volume_threshold;
   double curiousity_threshold;
   double curiousity_level;
pend

external procedure Vector MOTIVATIONAL_UPDATE with
   double anger_level;
   double fear_level;
   double hunger_level;
   double curiousity_level;
pend

external procedure Vector CHANGE_MOTIVATIONAL_VECTOR with
   double        anger_change;			// Updates (increments/decrements)
   double        fear_change;
   double        hunger_change;
   double        curiousity_change;
   double        anger_level;		// The global anger level.
   double        fear_level;
   double 	     hunger_level;
   double 	     curiousity_level;
pend

external procedure Vector TALK with
   CNLString_t        talkmessage;        // (user types in)
pend


external procedure Vector EXTRACT_GOAL with
pend

external procedure ObjectList DETECT_OBJECTS with
   double max_sensor_range;
pend

external procedure ObjectList FILTER_OBJECT_CLASSES with
   CNLString_t        desired_color;
   ObjectList    full_list;
pend

external procedure ObjectList FILTER_OBJECTS with
   CNLString_t        desired_color;
   ObjectList    full_list;
pend

external procedure bool IS_AN_OBJECT with
   ObjectList    object_list;
pend

external procedure bool IS_AN_OBJECT_MOTIVATED with
   ObjectList    object_list;
   double        anger_lower;
   double        anger_upper;
   double        fear_lower;
   double        fear_upper;
   double        hunger_lower;
   double        hunger_upper;
   double        curiousity_lower;
   double        curiousity_upper;
   double        anger_level;
   double        fear_level;
   double    hunger_level;
   double    curiousity_level;
pend


external procedure ObjectReading CLOSEST_OBJECT with
   ObjectList    object_list;
pend

external procedure Vector CLOSEST_OBSTACLE with
   obs_array    obstacle_list;
pend

external procedure bool IS_AT_GOAL with
   bool         have_a_goal;
   Vector 	goal_rel_loc;
   double 	success_radius;
pend

external procedure bool ESTOY_OBJETIVO with
   bool         have_a_goal;
   double   success_radius;
pend

external procedure bool ESTOY_POS_ANT_LIDER with
   bool         have_a_goal;
   double   success_radius;
pend


external procedure bool FIN_ASPIRAR with
    double tiempo_limpieza;
pend

external procedure bool NUEVA_TAREA with
pend

external procedure bool SKIP_GOAL with
   bool         have_a_goal;
pend

external procedure Vector PICKUP_OBJECT with
   ObjectReading object;
pend

external procedure Vector OBJECT_LOCATION with
   ObjectReading   object;
pend

external procedure Vector AVOID_OBJECTS with
   double       sphere;
   double       safety_margin;
   ObjectList   objlist;
pend

external procedure Vector AVOID_OBSTACLES with
  double 	sphere;
  double	safety_margin;
  obs_array	readings;
pend


external procedure Vector AVOID_OBSTACLES_MEM with
   double	sphere;
   double	safety_margin;
   double       decay_rate;
   obs_array    readings;
   robot_position cur_pos;
pend

external procedure bool CONSTANT with
   bool  value;
pend

external procedure Vector DEBUG_PROC with
pend

external procedure Vector DEBUG_SHOW_INPUT with
   CNLString_t msg;
   Vector input;
pend

external procedure Vector HALT with
pend

external procedure Vector TERMINATE_MISSION with
pend

external procedure Vector NULL_MS with
pend

external procedure Vector NOOP with
pend

external procedure Vector DROP_IN_BASKET with
   ObjectReading basket;
pend

external procedure bool FIRST_TIME with
pend

external procedure Vector MOVE_TO_FREE_SPACE with
   double       horizon;
   obs_array    readings;
pend

external procedure Vector MARK_OBJECT with
   ObjectReading      object;
   OneObjectClass  new_class;
pend

external procedure bool WAIT with
   double   delay_time;
pend

external procedure Vector WAIT_FOR_PROCEED with
pend

external procedure bool ISFACING with
   double   desired_heading;
   double   accuracy;
   double robot_heading;
pend

external procedure bool HAS_TURNED with
   double   desired_turn_angle;
   double   robot_heading;
   double   off_set;
pend

external procedure bool MOVED_DISTANCE with
   double   desired_distance;
   robot_position position;
pend

external procedure Vector SPIN with
   double angular_velocity;
   double slippageFactor;
pend

external procedure Vector PROJECTED_GOAL with
   Vector robot_location;
   double robot_heading;
   double project_angle;
   double distance;
pend

external procedure Vector SETXY with
	double x;
	double y;
pend

external procedure Vector SETXYTHETA with
   Vector new_robot_location;
   double new_theta;
pend

external procedure ObjectList FILTER_OBJECTS_BY_CLASS with
   ObjectList    full_list;

   // If false, pass objects only of the indicated types
   // If true, pass objects only if NOT the indicated types
   bool          remove_these;

   // Classes of objects
   ObjectClasses classes;
pend

external procedure ObjectList ECHO_OBJECTS with
   ObjectList    full_list;
pend

external procedure Vector COMPUTE_UNIT_CENTER with
   ObjectList           robots;
pend

external procedure boolean PROBE_MOVABLE with
   ObjectReading    object;
pend

external procedure Vector PROBE_OBJECT with
   ObjectReading object;
pend

external procedure Vector ADD_OBJECT with
   CNLString_t        color;
   int           style;         // STYLE_FIXED, STYLE_MOVABLE, STYLE_CONTAINER
   Vector        place;
   double        diameter;
pend

external procedure bool IS_VALID_OBJECT with
   ObjectReading  object;
pend

external procedure Vector MAP_LOCATION with
   CNLString_t   feature_name;
pend

external procedure bool EQUAL_INT with
   int a;
   int b;
pend

external procedure Vector DROP_OBJECT with
pend

external procedure type_pry GET_PRY with
pend

external procedure double GET_YAW with
   type_pry pry;
pend

external procedure Vector GET_UPHILL with
   type_pry pry;
pend

external procedure Vector SCALE_VECTOR with
   Vector v;
   double multiplier;
pend

external procedure Vector DEBUG_SHOW_INPUT_PRY with
   type_pry input;
pend

external procedure int COUNT_OBJECTS with
   ObjectList   objlist;
pend

external procedure int SUM with
   list int values;
pend

external procedure double SUM_DOUBLE with
   list double values;
pend


external procedure void MOTIVATIONS with
   Vector v;
pend

external procedure void PANTILT with
   Vector look_loc;
pend

external procedure Vector LOOK_TO with
   double look_theta;
   double look_psi;
pend

external procedure bool IS_LOOKING_AT_DEGREE with
	int lookfor;
	double desired_theta;
	double delta_theta;
	double desired_psi;
	double delta_psi;
pend

external procedure Vector SET_CAMERA_TRACKING_MODE with
	 int mode;
pend

external procedure Vector SET_MOBILITY_TYPE with
	 int mobilityType;
pend

external procedure Vector MNAV_OBSTACLES with
  double        ulim;
  Vector        f;
  Vector        t;
  Vector        g;
  double        u;
pend

external procedure Vector MNAV_TO_GOAL with
  double        ulim;
  Vector        f;
  Vector        t;
  Vector        g;
  double        u;
pend

external procedure bool MUST_FOLLOW with
  Vector        g;
  Vector        f;
pend

external procedure bool MUST_LEAVE with
  double        alpha;
  Vector        g;
  Vector        f;
pend

external procedure Vector FREE_US_PATH with
  Vector        goal_rel_loc;
  obs_array     readings;
  robot_position cur_pos;
  bool          progress;
  int		state_sharing;
pend

external procedure bool IS_A_FROBOT with
   ObjectList    object_list;
   robot_position cur_pos;
pend

external procedure Vector INTERCEPT with
    ObjectList objlist;
pend

external procedure Vector DATABASE_VEC with
   CNLString_t  key;
   Vector  new_vector;
pend

external procedure Vector NOTIFY_GOALS with
   Vector  goal_loc;
pend

external procedure Vector SET_CID with
   int  cid;
pend

external procedure Vector AVOID_PAST with
  int   past_horizon;
  int   past_mark;
  robot_position cur_pos;
  int   past_gridsize;
  int   world_to_past_scale;
  double overlap_ratio;

pend


external procedure Vector COMPUTE_REP with
  double        sphere;
  obs_array     readings;
pend

external procedure Vector COMPUTE_F with
  Vector        rep_force;
pend

external procedure Vector COMPUTE_T with
  Vector        rep_force;
pend

external procedure Vector COMPUTE_G with
  Vector        goal_rel_loc;
  bool          have_a_goal;
pend

external procedure double COMPUTE_U with
  Vector        rep_force;
pend

external procedure double COMPUTE_ULIM with
  double        alpha;
  double        beta;
  double        umax;
  double        umin;
pend

external procedure double COMPUTE_A with
   Vector g;
   Vector f;
pend

external procedure double COMPUTE_B with
   Vector g;
pend

external procedure Vector SET_TARGET with
  Vector        goal_loc;
pend

external procedure Vector CURRENT_TARGET with
   Vector  new_vector;
pend

external procedure bool IS_NOT_PROGRESSING_MEM with
   Vector       desired_location;
   double       patient;
pend

external procedure Vector CHANGE_ROBOT_COLOR with
   CNLString_t        color;
pend

external procedure Vector RESET_WORLD with
   CNLString_t        new_overlay;
   int	         load_new_overlay;
pend


external procedure int CBRGetCase with
	Vector goal_rel_loc;
pend

external procedure Vector CBRAgent with
	int CBR_RetCode;
	Vector COOPVector;
pend

external procedure Vector MakeUnitVector with
	double XComp;
	double YComp;
pend

external procedure Vector HeadRelUnitV with
	Vector Goal_Location;
	robot_position Robot_Pos;
	double XComp;
	double YComp;
pend


external procedure Vector AERIAL_GLOBAL_TO_EGOCENTRIC with
   Vector global;
   Vector robot_location;
   double robot_heading;
   double goal_altitude;
pend

external procedure Vector AERIAL_MOVE_TO_GOAL with
   Vector goal_rel_loc;
   double success_radius;
pend

external procedure Vector AERIAL_AVOID_OBSTACLES with
  double        sphere;
  double        safety_margin;
  obs_array     readings;
pend

external procedure Vector AERIAL_AVOID_GROUND with
  robot_position  robopos;
  double          ground_avoidance_range;
  double          ground_avoidance_threshold;
pend

external procedure Vector AERIAL_MAINTAIN_ALTITUDE with
  robot_position  robopos;
  double          desired_altitude;
  double          dead_zone;
pend

external procedure Vector AERIAL_NOISE with
  int persistence;
  double robot_heading;
pend

external procedure Vector AERIAL_AVOID_ROBOT with
   double  sphere;
   double  min_range;
pend

external procedure int STRING_TO_INT with
   CNLString_t inputString;
pend

external procedure bool STRING_TO_BOOL with
   CNLString_t inputString;
pend

external procedure double STRING_TO_DOUBLE with
   CNLString_t inputString;
pend

external procedure float STRING_TO_FLOAT with
   CNLString_t inputString;
pend

external procedure Vector SLUAV_TRANSPORT with
   int robotid;
pend

external procedure Vector CMDLi with
   CNLString_t envFilename;
   CNLString_t cmdlFilename;
   Vector moveto;
   Vector recon;
   bool moveto_done;
   bool recon_done;
pend

#endif



/**********************************************************************
 * $Log: cnl.inc,v $
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.14  2006/07/02 22:17:29  endo
 * CMDLi compiling error fix.
 *
 * Revision 1.13  2006/07/02 21:46:24  endo
 * CMDLi from MARS 2020 integrated into AO-FNC repository.
 *
 * Revision 1.12  2006/05/06 04:26:03  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.11  2006/05/02 04:17:18  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.10  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.9  2006/03/01 09:27:39  endo
 * Check-in for Type-I Intercept Experiment.
 *
 * Revision 1.8  2006/02/19 22:41:33  endo
 * CNP_SAVE_TARGET_LOCATION stuff.
 *
 * Revision 1.7  2006/02/19 17:54:53  endo
 * Experiment related modifications
 *
 * Revision 1.6  2006/01/10 06:16:04  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.5  2005/05/18 21:19:52  endo
 * AuRA.naval added.
 *
 * Revision 1.4  2005/04/08 01:56:00  pulam
 * addition of cnp behaviors
 *
 * Revision 1.3  2005/03/27 01:19:35  pulam
 * addition of uxv behaviors
 *
 * Revision 1.2  2005/03/23 07:36:54  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:01  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.124  2003/04/06 12:04:03  endo
 * gcc 3.1.1
 *
 * Revision 1.123  2003/04/06 09:25:40  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.122  2003/04/06 07:44:36  endo
 * Checked in for Ananth. DSTAR added.
 *
 * Revision 1.121  2003/04/06 00:15:07  kaess
 * *** empty log message ***
 *
 * Revision 1.120  2003/04/02 22:41:34  zkira
 * Added SKIP_GOAL
 *
 * Revision 1.119  2002/11/08 16:16:43  endo
 * Rolled back to cancel ARBITRATE.
 *
 * Revision 1.118  2002/11/07 22:08:50  blee
 * Added ARBITRATE operator.
 *
 * Revision 1.117  2002/10/31 21:15:08  ebeowulf
 * Added the RESET_WORLD behavior
 *
 * Revision 1.116  2002/10/31 15:02:40  ebeowulf
 * Added the CHANGE_ROBOT_COLOR behavior for simulation.
 *
 * Revision 1.115  2002/10/25 15:20:09  ebeowulf
 * removed the QLEARN cnl function.
 *
 * Revision 1.114  2002/10/25 01:38:55  kaess
 * - swirl stripped off unnecessary code and parameters,
 * infinite repulsion within safety margin added
 * - "robot-too-cautious" bug fixed: robot should be in
 * cautious mode iff inside safety margin, which is the dase
 * when COOP returns an infinit vector - infinity needed to be changed
 *
 * Revision 1.113  2002/01/13 01:43:35  endo
 * SET_DATABASE_DOUBLE added.
 *
 * Revision 1.112  2001/10/18 17:42:30  ebeowulf
 * Added a timeout variable to QLEARN
 *
 * Revision 1.111  2001/09/24 21:30:18  ebeowulf
 * Added QLEARN operator, and Reinforcer type
 *
 * Revision 1.110  2001/01/24 23:27:03  saho
 * Added definitions for MOTIVATIONS,  MOTIVATIONAL_UPDATE, and
 * CHANGE_MOTIVATIONAL_VECTOR.
 *
 * Revision 1.109  2000/12/02 21:46:37  sgorbiss
 * Added cnl code for line-of-sight navigation
 * MicroNavigation and Avoid The Past
 *
 * Revision 1.108  2000/08/15 20:51:15  endo
 * WAIT_FOR_PROCEED added.
 *
 * Revision 1.107  2000/08/12 22:04:11  saho
 * Minor change to DOORWAY_ULTRASOUND.
 *
 * Revision 1.106  2000/07/13 19:21:39  conrad
 * added camera tracking mode
 *
 * Revision 1.105  2000/06/18 01:24:20  endo
 * MOVE_AHEAD no longer accepts "amount".
 *
 * Revision 1.104  2000/05/19 21:43:06  endo
 * POPUP_TELOP_INTERFACE and IS_NOT_PROGRESSING added.
 *
 * Revision 1.103  2000/04/25 07:14:48  endo
 * NOTIFY_ROBOTS was added.
 *
 * Revision 1.102  2000/04/19 21:17:31  jdiaz
 * added cnl proc headers
 *
 * Revision 1.101  2000/04/16 15:18:26  endo
 * PROJECTED_GOAL SUM_DOUBLE ALERT ALERTED_ONCE NOTIFY
 * NOTIFIED added/modified.
 *
 * Revision 1.100  2000/04/13 22:30:57  endo
 * Checked in for Doug.
 * TEST_DISTRIBUTED_FLAG added.
 *
 * Revision 1.99  2000/03/30 15:49:54  saho
 * Added TALK.cnl.
 *
 * Revision 1.98  2000/03/30 01:31:11  endo
 * off_set added to HAS_TURNED.
 *
 * Revision 1.97  2000/03/24 16:55:07  saho
 * Added IS_AN_OBJECT_MOTIVATED and DETECT_SOUND_MOTIVATED.
 *
 * Revision 1.96  2000/02/29 20:41:23  saho
 * Added definitions for SOUND and DETECT_SOUND.
 *
 * Revision 1.95  2000/02/29 00:13:59  jdiaz
 * protos for mark and unmark doorway
 *
 * Revision 1.94  2000/02/18 02:39:25  endo
 * ALERT ALERTED_ONCE added.
 * int delay -> double delay_time
 *
 * Revision 1.93  2000/02/07 05:37:26  jdiaz
 * added some function defs
 *
 * Revision 1.92  2000/01/24 18:26:17  ellenber
 * Fixed Objects/Obstacles conflict.  Now works with Obstacles.
 *
 * Revision 1.91  1999/12/18 09:54:44  endo
 * Made it to include "FSA_status.h".
 *
 * Revision 1.90  1999/12/16 23:09:56  mjcramer
 * rh6 port
 *
 * Revision 1.84  1999/08/11 21:42:08  jdiaz
 * added defintion for STAY_ON_PATH
 *
 * Revision 1.83  1999/07/03 01:04:52  mjcramer
 * Added a "kill Endo" virus
 * Seriously, I added more camera stuff
 *
 * Revision 1.82  1999/06/29 02:16:21  mjcramer
 * Added camererera control
 *
 * Revision 1.81  1999/06/28 20:06:08  endo
 * SETXY and SETXYTHETA added.
 *
 * Revision 1.80  1999/06/11 15:48:23  conrad
 * *** empty log message ***
 *
 * Revision 1.79  1999/06/07 04:02:43  jdiaz
 * function declarations for RAW_ULTASONICS, HAS_TURNED, MOVED_DISTANCE, DOORWAY_ULTRASOUND
 *
 * Revision 1.78  1999/05/31 02:35:17  conrad
 * *** empty log message ***
 *
 * Revision 1.77  1997/02/25 02:54:25  zchen
 * *** empty log message ***
 *
# Revision 1.76  1996/06/17  22:58:23  doug
# *** empty log message ***
#
# Revision 1.75  1996/06/06  19:55:51  kali
# *** empty log message ***
#
# Revision 1.74  1996/06/05  20:27:37  doug
# added SCALE_VECTOR
#
# Revision 1.73  1996/06/02  20:55:30  doug
# *** empty log message ***
#
# Revision 1.72  1996/05/02  20:37:21  kali
# changed pry_type to type_pry to avoid errors with cnl compiler
#
# Revision 1.71  1996/05/02  15:51:25  kali
# *** empty log message ***
#
# Revision 1.70  1996/05/02  14:51:03  kali
# *** empty log message ***
#
# Revision 1.69  1996/04/30  22:00:05  doug
# added get_pry type and extern
#
# Revision 1.68  1996/03/03  23:58:41  doug
# *** empty log message ***
#
# Revision 1.67  1996/03/01  00:48:21  doug
# *** empty log message ***
#
# Revision 1.66  1996/02/28  03:54:13  doug
# *** empty log message ***
#
# Revision 1.65  1996/02/27  05:00:40  doug
# *** empty log message ***
#
# Revision 1.64  1996/02/26  05:01:53  doug
# *** empty log message ***
#
# Revision 1.63  1996/02/25  01:14:00  doug
# *** empty log message ***
#
# Revision 1.62  1996/02/21  15:44:22  doug
# *** empty log message ***
#
# Revision 1.61  1996/02/19  21:58:25  doug
# added new style probe
#
# Revision 1.60  1996/02/18  00:04:39  doug
# *** empty log message ***
#
# Revision 1.59  1996/02/08  19:22:52  doug
# *** empty log message ***
#
# Revision 1.58  1996/01/31  02:08:05  doug
# *** empty log message ***
#
# Revision 1.57  1996/01/17  18:45:38  doug
# *** empty log message ***
#
# Revision 1.56  1995/11/08  16:50:38  doug
# *** empty log message ***
#
# Revision 1.55  1995/11/04  23:43:11  doug
# *** empty log message ***
#
# Revision 1.54  1995/11/02  21:15:08  doug
# *** empty log message ***
#
#*********************************************************************/
