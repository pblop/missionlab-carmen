#ifndef IPC_CLIENT_H
#define IPC_CLIENT_H

#include <ipc.h>

#define IPC_SEND_MIN_TIME 0.10
#define IPC_CONNECTION_ERROR	0
#define IPC_ALREADY_CONNECTED	1
#define IPC_CONNECTION_SUCCESS	2

void ipc_send_localize_initialize(double x, double y, double theta);
void ipc_send_simulator_set_truepose(double x, double y, double theta);
void ipc_send_base_odometry(double x, double y, double theta, double tv, double rv);
void ipc_send_base_sonar(int numSonars, float *readings, int lenExtraBytes, float *extra);
void ipc_send_localize_globalpos(double x, double y, double theta, double odomX, double odomY, double odomTheta);
void ipc_send_base_velocity(double tv, double rv);
const char *ipc_preferred_server_name(const char *server = NULL);
int ipc_connect_preferred_server(const char *server = NULL);
void ipc_hook_message(char *name, char *newName, char *format);


#define CARMEN_LASER_LASER_FMT  "{int,{int,double,double,double,double,double,int},int,<float:3>,int,<float:5>,double,string}"
#define CARMEN_LASER_LASER1_NAME "carmen_laser_laser1"
#define CARMEN_LASER_FRONTLASER_NAME CARMEN_LASER_LASER1_NAME
#define CARMEN_LASER_FRONTLASER_FMT  CARMEN_LASER_LASER_FMT

/** supported laser types     **/
typedef enum {
	SICK_LMS                  = 0,
	SICK_PLS                  = 1,
	HOKUYO_URG                = 2,
	SIMULATED_LASER           = 3,
	SICK_S300                 = 4,
	HOKUYO_UTM                = 5,
	UNKNOWN_PROXIMITY_SENSOR  = 99
} carmen_laser_laser_type_t;

/** Possible remission values **/
typedef enum {
	REMISSION_NONE       = 0,
	REMISSION_DIRECT     = 1,
	REMISSION_NORMALIZED = 2
} carmen_laser_remission_type_t;

/** The laser message of the laser module (rawlaser) **/
typedef struct {
  carmen_laser_laser_type_t  laser_type;  /**< what kind of laser is this **/
  double start_angle;                     /**< angle of the first beam relative **/
                                          /**< to to the center of the laser **/
  double fov;                             /**< field of view of the laser **/
  double angular_resolution;              /**< angular resolution of the laser **/
  double maximum_range;                   /**< the maximum valid range of a measurement  **/
  double accuracy;                        /**< error in the range measurements **/
  carmen_laser_remission_type_t remission_mode;  /* if and what kind of remission values are used */

} carmen_laser_laser_config_t;


typedef struct {
  int id;
  carmen_laser_laser_config_t config;
  int num_readings;
  float *range;
  int num_remissions;
  float *remission;
  double timestamp;
  char *host;
} carmen_laser_laser_message;

typedef struct {
  double x;
  double y;
  double theta;
} carmen_point_t, *carmen_point_p;

/** Sonar measurement message **/
typedef struct {
int num_sonars;
double cone_angle;               /**< width of sonar cone **/
double *ranges;
carmen_point_t *sonar_offsets;
carmen_point_t robot_pose;
double tv, rv;
double timestamp;
char *host;                         /**< The host from which this message was sent **/
} carmen_robot_sonar_message;

#define      CARMEN_ROBOT_SONAR_NAME            "robot_sonar"
#define      CARMEN_ROBOT_SONAR_FMT             "{int,double,<double:1>,<{double,double,double}:1>,{double,double,double},double,double,double,string}"

typedef struct {
  double distance;
  double theta;
  double timestamp;
  char *host;               /**< The host from which this message was sent **/
} carmen_robot_vector_move_message;

#define      CARMEN_ROBOT_VECTOR_MOVE_NAME         "robot_vector_move"
#define      CARMEN_ROBOT_VECTOR_MOVE_FMT          "{double,double,double,string}"

typedef struct {
  int num_sonars;
  double cone_angle;                    //width of sonar cone
  double *range;
  carmen_point_t* sonar_offsets;
  double timestamp;
  char *host;
} carmen_base_sonar_message;

#define      MISSIONLAB_BASE_SONAR_NAME      "missionlab_base_sonar"
#define      CARMEN_BASE_SONAR_NAME          "carmen_base_sonar"
#define      CARMEN_BASE_SONAR_FMT           "{int,double,<double:1>,<{double,double,double}:1>,double,string}"

typedef struct {
  carmen_point_t pose;
  double timestamp;
  char *host;
} carmen_simulator_set_truepose_message;

#define CARMEN_SIMULATOR_SET_TRUEPOSE_NAME "carmen_simulator_set_truepose"
#define CARMEN_SIMULATOR_SET_TRUEPOSE_FMT  "{{double,double,double},double,string}"

typedef struct {
   double x, y, theta;
   double tv, rv;
   double acceleration;
   double timestamp;
   char *host;
} carmen_base_odometry_message;

#define      MISSIONLAB_BASE_ODOMETRY_NAME   "missionlab_base_odometry"
#define      CARMEN_BASE_ODOMETRY_NAME       "carmen_base_odometry"
#define      CARMEN_BASE_ODOMETRY_FMT        "{double,double,double,double,double,double,double,string}"

typedef struct {
  carmen_point_t globalpos, globalpos_std;
  carmen_point_t odometrypos;
  double globalpos_xy_cov;
  int converged;
  double timestamp;
  char *host;
} carmen_localize_globalpos_message;

#define MISSIONLAB_LOCALIZE_GLOBALPOS_NAME "missionlab_localize_globalpos"
#define CARMEN_LOCALIZE_GLOBALPOS_NAME "carmen_localize_globalpos"
#define CARMEN_LOCALIZE_GLOBALPOS_FMT  "{{double,double,double},{double,double,double},{double,double,double},double,int,double,string}"

typedef struct {
  carmen_point_t truepose;
  carmen_point_t odometrypose;
  double timestamp;
  char *host;
} carmen_simulator_truepos_message;

#define      CARMEN_SIMULATOR_TRUEPOS_NAME       "carmen_simulator_truepos"
#define      CARMEN_SIMULATOR_TRUEPOS_FMT        "{{double,double,double},{double,double,double},double,string}"


typedef struct {
  double tv, rv;
  double timestamp;
  char *host;
} carmen_base_velocity_message;

#define      MISSIONLAB_BASE_VELOCITY_NAME   "missionlab_base_velocity"
#define      CARMEN_BASE_VELOCITY_NAME       "carmen_base_velocity"
#define      CARMEN_BASE_VELOCITY_FMT        "{double,double,double,string}"


#define CARMEN_INITIALIZE_UNIFORM     1
#define CARMEN_INITIALIZE_GAUSSIAN    2

typedef struct {
  int distribution;
  int num_modes;
  carmen_point_t *mean, *std;
  double timestamp;
  char *host;
} carmen_localize_initialize_message;

#define CARMEN_LOCALIZE_INITIALIZE_NAME  "carmen_localize_initialize"
#define CARMEN_LOCALIZE_INITIALIZE_FMT   "{int,int,<{double,double,double}:2>,<{double,double,double}:2>,double,string}"

typedef struct {
  double timestamp;
  char *host;
} carmen_default_message;

#define CARMEN_DEFAULT_MESSAGE_FMT "{double,string}"

#define CARMEN_NAVIGATOR_STOP_NAME             "carmen_navigator_stop"
typedef carmen_default_message carmen_navigator_stop_message;
#define CARMEN_NAVIGATOR_GO_NAME               "carmen_navigator_go"
typedef carmen_default_message carmen_navigator_go_message;

typedef enum{CARMEN_NAVIGATOR_GOAL_REACHED_v,
	       CARMEN_NAVIGATOR_USER_STOPPED_v,
	       CARMEN_NAVIGATOR_UNKNOWN_v} carmen_navigator_reason_t;

  /** This message is published by the navigator whenever it stops trying to move
      towards the current goal.
  */

typedef struct {
  carmen_navigator_reason_t reason; /**< Reasons are: that the goal was
				       reached, a user stopped autonomous mode
				       (by publishing a stop message) or due
				       to some unknown failure. */
  double timestamp;
  char *host;
} carmen_navigator_autonomous_stopped_message;

#define      CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_NAME "carmen_navigator_autonomous_stopped"
#define      CARMEN_NAVIGATOR_AUTONOMOUS_STOPPED_FMT "{int,double,string}"


typedef struct {
  double x, y;           /**< It is assumed that (x, y) is in the reference frame of
                              the current map. */
  double timestamp;
  char *host;
} carmen_navigator_set_goal_message;

#define      CARMEN_NAVIGATOR_SET_GOAL_NAME         "carmen_navigator_set_goal"
#define      CARMEN_NAVIGATOR_SET_GOAL_FMT          "{double,double,double,string}"

typedef struct {
  char *message_name;
  char *new_message_name;
  double timestamp;
  char *host;               /**< The host from which this message was sent **/
} carmen_global_hook_message;
#define     CARMEN_GLOBAL_HOOK_MSG_NAME     "carmen_global_hook_msg"
#define     CARMEN_GLOBAL_HOOK_MSG_FMT      "{string, string, double, string}"

typedef enum {CARMEN_PARAM_OK, CARMEN_PARAM_NOT_FOUND, CARMEN_PARAM_NOT_INT,
	      CARMEN_PARAM_NOT_DOUBLE, CARMEN_PARAM_NOT_ONOFF,
	      CARMEN_PARAM_NOT_FILE, CARMEN_PARAM_FILE_ERR} carmen_param_status_t;

typedef struct {
  char *module_name;
  char *variable_name;
  double timestamp;
  char *host;
} carmen_param_query_message;

typedef struct {
  char *module_name;                /**< The queried variable's module */
  char *variable_name;              /**< The queried variable's name */
  double value;                     /**< The queried variable's value, if it
				       can be parsed as a double. */
  int expert;                       /**< 1 if this variable was labelled as an
				       "expert" variable, 0 otherwise. */
  carmen_param_status_t status;     /**< If status is not CARMEN_PARAM_OK, all
				       previous fields are not defined. */
  double timestamp;
  char *host;
} carmen_param_response_double_message;

#define CARMEN_PARAM_QUERY_DOUBLE_NAME    "carmen_param_query_double"

#endif
