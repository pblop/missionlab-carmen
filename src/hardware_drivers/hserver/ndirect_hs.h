/*
 * Nclient_hs.h
 *
 * Interface file for direct connections to the robot or for
 * connections to Nserver.
 * 
 * Copyright 1993,94,95, Nomadic Technologies, Inc.
 *
 *
 * Modified for hserver by William Halliburton
 *
 */

#ifndef _HOST_CLIENT_NCLIENT_H_
#define _HOST_CLIENT_NCLIENT_H_

/* constants */

#ifndef FALSE
#define FALSE			0
#endif
#ifndef TRUE
#define TRUE			1
#endif
#ifndef NULL
#define NULL                    0
#endif
#define MAX_VERTICES     10
#define NUM_STATE        45
#define NUM_MASK         44 
#define NUM_LASER        482 
#define BUFSIZE          4096
#define MAX_USER_BUF     0xFFFF

/* the number of sonars and infrareds */

#define SONARS           16
#define INFRAREDS        16

/*
 * The following defines allow you to access the State vector in a 
 * more readable way.
 */

#define STATE_SIM_SPEED            0

#define STATE_IR_0                 1
#define STATE_IR_1                 2
#define STATE_IR_2                 3
#define STATE_IR_3                 4
#define STATE_IR_4                 5
#define STATE_IR_5                 6
#define STATE_IR_6                 7
#define STATE_IR_7                 8
#define STATE_IR_8                 9
#define STATE_IR_9                 10
#define STATE_IR_10                11
#define STATE_IR_11                12
#define STATE_IR_12                13
#define STATE_IR_13                14
#define STATE_IR_14                15
#define STATE_IR_15                16

#define STATE_SONAR_0              17
#define STATE_SONAR_1              18
#define STATE_SONAR_2              19
#define STATE_SONAR_3              20
#define STATE_SONAR_4              21
#define STATE_SONAR_5              22
#define STATE_SONAR_6              23
#define STATE_SONAR_7              24
#define STATE_SONAR_8              25
#define STATE_SONAR_9              26
#define STATE_SONAR_10             27
#define STATE_SONAR_11             28
#define STATE_SONAR_12             29
#define STATE_SONAR_13             30
#define STATE_SONAR_14             31
#define STATE_SONAR_15             32
 
#define STATE_BUMPER               33
#define STATE_CONF_X               34
#define STATE_CONF_Y               35
#define STATE_CONF_STEER           36
#define STATE_CONF_TURRET          37
#define STATE_VEL_TRANS            38
#define STATE_VEL_STEER            39
#define STATE_VEL_TURRET           40
#define STATE_MOTOR_STATUS         41
#define STATE_LASER                42
#define STATE_COMPASS              43
#define STATE_ERROR                44

/*
 * The following defines allow you to access the Smask vector in a 
 * more readable way.
 */

#define SMASK_POS_DATA             0

#define SMASK_IR_1                 1
#define SMASK_IR_2                 2
#define SMASK_IR_3                 3
#define SMASK_IR_4                 4
#define SMASK_IR_5                 5
#define SMASK_IR_6                 6
#define SMASK_IR_7                 7
#define SMASK_IR_8                 8
#define SMASK_IR_9                 9
#define SMASK_IR_10                10
#define SMASK_IR_11                11
#define SMASK_IR_12                12
#define SMASK_IR_13                13
#define SMASK_IR_14                14
#define SMASK_IR_15                15
#define SMASK_IR_16                16

#define SMASK_SONAR_1              17
#define SMASK_SONAR_2              18
#define SMASK_SONAR_3              19
#define SMASK_SONAR_4              20
#define SMASK_SONAR_5              21
#define SMASK_SONAR_6              22
#define SMASK_SONAR_7              23
#define SMASK_SONAR_8              24
#define SMASK_SONAR_9              25
#define SMASK_SONAR_10             26
#define SMASK_SONAR_11             27
#define SMASK_SONAR_12             28
#define SMASK_SONAR_13             29
#define SMASK_SONAR_14             30
#define SMASK_SONAR_15             31
#define SMASK_SONAR_16             32
 
#define SMASK_BUMPER               33
#define SMASK_CONF_X               34
#define SMASK_CONF_Y               35
#define SMASK_CONF_STEER           36
#define SMASK_CONF_TURRET          37
#define SMASK_VEL_TRANS            38
#define SMASK_VEL_STEER            39
#define SMASK_VEL_TURRET           40
#define SMASK_RESERVED             41
#define SMASK_LASER                42
#define SMASK_COMPASS              43

/*
 * These defines are used for specifying the control modes in the
 * robot motion command 'mv'. If MV_IGNORE is specified for an axis
 * the current motion command for it will remain active. Specifying
 * MV_VM or MV_PR will select velocity and position control as in 
 * the vm and pr robot motion commands 
 */

#define MV_IGNORE 0
#define MV_VM     1 /* velocity mode */
#define MV_PR     2 /* position relative mode */
#define MV_LP     3 /* limp mode */
#define MV_AC     4 /* set acceleration for vm, pr, pa modes*/
#define MV_SP     5 /* set velocity for pr, pa modes */
#define MV_PA     6 /* position absolute mode */
#define MV_TQ     7 /* torque mode */
#define MV_MT     8 /* set maximum torque for vm, pr, pa, tq modes */


/*
 * zeroing modes for arm
 */

#define ZR_CHECK         1
#define ZR_ORIENT        2
#define ZR_NO_N_GRIPPER  4
/* 
 * user packet constants for arm 
 */

#define ARM_ZR 40
#define ARM_WS 41
#define ARM_MV 42


/*
 * For requesting the PosData the following defines should be used.
 * Each sensor has a bit, if it is set the pos-data is attached
 * when the sensory data is returned.
 */

#define POS_NONE          ( 0 << 0 )
#define POS_INFRARED      ( 1 << 0 )
#define POS_SONAR         ( 1 << 1 )
#define POS_BUMPER        ( 1 << 2 )
#define POS_LASER         ( 1 << 3 )
#define POS_COMPASS       ( 1 << 4 )

/* 
 * these macros enable the user to determine if the pos-attachment
 * is requested for a specific sensor. 1 is returned if the 
 * attachment is requested, 0 otherwise
 * 
 * Note that the function posDataCheck() is called (see below)
 */

#define POS_INFRARED_P  ( ( (posDataCheck()) & POS_INFRARED ) ? 1 : 0 )
#define POS_SONAR_P     ( ( (posDataCheck()) & POS_SONAR    ) ? 1 : 0 )
#define POS_BUMPER_P    ( ( (posDataCheck()) & POS_BUMPER   ) ? 1 : 0 )
#define POS_LASER_P     ( ( (posDataCheck()) & POS_LASER    ) ? 1 : 0 )
#define POS_COMPASS_P   ( ( (posDataCheck()) & POS_COMPASS  ) ? 1 : 0 )

/*
 * The user will be able to call a function that fills out a 
 * list of position data for a specific sensor reading. 
 * To access the sensors in that structure the following defines 
 * should be used. They should also be used if data for a single
 * infrared sensor / sonar is requested.
 */

#define POS_IR_1             0
#define POS_IR_2             1
#define POS_IR_3             2
#define POS_IR_4             3
#define POS_IR_5             4
#define POS_IR_6             5
#define POS_IR_7             6 
#define POS_IR_8             7
#define POS_IR_9             8
#define POS_IR_10            9
#define POS_IR_11           10
#define POS_IR_12           11
#define POS_IR_13           12
#define POS_IR_14           13
#define POS_IR_15           14
#define POS_IR_16           15

#define POS_SONAR_1          0
#define POS_SONAR_2          1
#define POS_SONAR_3          2
#define POS_SONAR_4          3
#define POS_SONAR_5          4
#define POS_SONAR_6          5 
#define POS_SONAR_7          6  
#define POS_SONAR_8          7
#define POS_SONAR_9          8
#define POS_SONAR_10         9
#define POS_SONAR_11        10
#define POS_SONAR_12        11
#define POS_SONAR_13        12
#define POS_SONAR_14        13
#define POS_SONAR_15        14
#define POS_SONAR_16        15


/* Define the length of the user buffer (Maximal short).
 * Due to Protocol bytes, the effective length is 65526 
 */
#define USER_BUFFER_LENGTH	0xFFFF


/********************
 *                  *
 * Type definitions *
 *                  *
 ********************/

/*
 * The following type definitions are used for the PosData.
 * PosData is an information packet that is attached to 
 * each sensor reading, if requested. Note that the use of 
 * PosData could cause compatibility problems when different
 * releases of the software are used on the robot and on the
 * server side. 
 *
 * The information packet can be used to determine how up-to-date
 * a sensory reading is. It contains the configuration of the robot.
 * This is the most updated configuration at the time of the sensor
 * reading. However, it is possible that the sensory reading
 * was taken after the integration of the coniguration.
 * To determine the interval that has passed two timestamps are in-
 * cluded in this information package: a timestamp for the computation
 * of the configuration and another timestamp for the actual capturing
 * of the senor reading.
 *
 * The timestamps are in milliseconds of the internal clock of the 
 * board that handles the sensors (Intellisys 100 sensor-board).
 */

/*
 * TimeData contains the current time of the Intellisys 100 
 * in milliseconds
 */

typedef unsigned long TimeData;

/*
 * ConfigData is where the i486 writes the current configuration
 * of the robot, so that the Intellisys 100 can attach current
 * integration values to the sensor readings.
 * It is also used inside of the Pos data.
 */

typedef struct _ConfigData
{
    /* the configuration of the robot */
    long          configX;
    long          configY;
    long          configSteer;
    long          configTurret;

    /* the velocities of the robot*/
    long          velTrans;
    long          velSteer;
    long          velTurret;

    /* time of integration in milliseconds (Intellisys 100 time) */
    TimeData      timeStamp;

} ConfigData;


/* 
 * PosData contains information that is attached to a sensor
 * reading in order to determine how recent it is.
 */

typedef struct _PosData
{
    /* the configuration of the robot at the time of the reading */
    ConfigData config;

    /* the time of the sensing in milliseconds (Intellisys 100 time) */
    TimeData   timeStamp;

} PosData;

/* these type definitions are for user defined package processing */

typedef union
{
  char            bytes[8];
  double          data;
} double_union;

typedef union
{
  char	        bytes[4];
  short		words[2];
  long	        data;
} long_union;

typedef union
{
  unsigned char	bytes[2];
  short		data;
} short_union;

typedef union
{
  unsigned char   bytes[2];
  unsigned short  data;
} ushort_union;

typedef union
{
  unsigned char	bytes[4];
  unsigned short	words[2];
  unsigned long	data;
} ulong_union;

struct request_struct
{
  short type;
  unsigned short size;
  long  mesg[USER_BUFFER_LENGTH];
};

struct reply_struct
{
  short type;
  unsigned short size;
  long  mesg[USER_BUFFER_LENGTH];
};


extern int CONN_TYPE;
extern int ROBOT_TCP_PORT;
extern long gNomadState[NUM_STATE];


/*****************************
 *                           *
 * Robot Interface Functions *
 *                           *
 *****************************/

/*
 * connect_robot - requests the server to connect to the robot
 *                 with id = robot_id. In order to talk to the server,
 *                 the SERVER_MACHINE_NAME and SERV_TCP_PORT must be
 *                 set properly. If a robot with robot_id exists,
 *                 a connection is established with that robot. If
 *                 no robot exists with robot_id, no connection is
 *                 established.
 *
 * parameters:
 *    long robot_id -- robot's id. In this multiple robot version, in order
 *                     to connect to a robot, you must know it's id.
 */

void disconnect_robot();
int connect_robot( const char* robot_hostname );
int nomad_open_serial( const char* szPortString, unsigned short baud );

/*
 * ac - sets accelerations of the robot. Currently it has no effect in 
 *      simulation mode.
 *
 * parameters:
 *    int t_ac, s_ac, r_ac -- the translation, steering, and turret
 *                            accelerations. t_ac is in 1/10 inch/sec^2
 *                            s_ac and r_ac are in 1/10 degree/sec^2.
 */
int ac(int t_ac, int s_ac, int r_ac);

/*
 * sp - sets speeds of the robot, this function will not cause the robot to
 *      move. However, the set speed will be used when executing a pr()
 *      or a pa().
 *
 * parameters:
 *    int t_sp, s_sp, r_sp -- the translation, steering, and turret
 *                            speeds. t_sp is in 1/10 inch/sec and
 *                            s_sp and r_sp are in 1/10 degree/sec.
 */
int sp(int t_sp, int s_sp, int r_sp);

/*
 * pr - moves the motors of the robot by a relative distance, using the speeds
 *      set by sp(). The three parameters specify the relative distances for
 *      the three motors: translation, steering, and turret. All the three
 *      motors move concurrently if the speeds are not set to zero and the 
 *      distances to be traveled are non-zero. Depending on the timeout 
 *      period set (by function conf_tm(timeout)), the motion may 
 *      terminate before the robot has moved the specified distances
 *
 * parameters:
 *    int t_pr, s_pr, r_pr -- the specified relative distances of the
 *                            translation, steering, and turret motors.
 *                            t_pr is in 1/10 inch and s_pr and r_pr are
 *                            in 1/10 degrees.
 */
int pr(int t_pr, int s_pr, int r_pr);

/*
 * vm - velocity mode, command the robot to move at translational
 *      velocity = tv, steering velocity = sv, and rotational velocity =
 *      rv. The robot will continue to move at these velocities until
 *      either it receives another command or this command has been
 *      timeout (in which case it will stop its motion).
 *
 * parameters: 
 *    int t_vm, s_vm, r_vm -- the desired translation, steering, and turret
 *                            velocities. tv is in 1/10 inch/sec and
 *                            sv and rv are in 1/10 degree/sec.
 */
int vm(int t_vm, int s_vm, int r_vm);

/*
 * mv - move, send a generalized motion command to the robot.
 *      For each of the three axis (translation, steering, and
 *      turret) a motion mode (t_mode, s_mode, r_mode) can be 
 *      specified (using the values MV_IGNORE, MV_AC, MV_SP,
 *      MV_LP, MV_VM, and MV_PR defined above):
 *
 *         MV_IGNORE : the argument for this axis is ignored
 *                     and the axis's motion will remain 
 *                     unchanged.
 *         MV_AC :     the argument for this axis specifies
 *                     an acceleration value that will be used
 *                     during motion commands.
 *         MV_SP :     the argument for this axis specifies
 *                     a speed value that will be used during
 *                     position relative (PR) commands.
 *         MV_LP :     the arguemnt for this axis is ignored
 *                     but the motor is turned off.
 *         MV_VM :     the argument for this axis specifies
 *                     a velocity and the axis will be moved
 *                     with this velocity until a new motion
 *                     command is issued (vm,pr,mv) or 
 *                     recieves a timeout.
 *         MV_PR :     the argument for this axis specifies
 *                     a position and the axis will be moved
 *                     to this position, unless this command
 *                     is overwritten by another (vm,pr,mv).
 *
 * parameters: 
 *    int t_mode - the desired mode for the tranlation axis
 *    int t_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 *    int s_mode - the desired mode for the steering axis
 *    int s_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 *    int r_mode - the desired mode for the turret axis
 *    int r_mv   - the value for that axis, velocity or position,
 *                 depending on t_mode
 */
int mv(int t_mode, int t_mv, int s_mode, int s_mv, int r_mode, int r_mv);

/*
 * ct - send the sensor mask, Smask, to the robot. You must first change
 *      the global variable Smask to the desired communication mask before
 *      calling this function. 
 */
int ct(void);

/*
 * gs - get the current state of the robot according to the mask (of 
 *      the communication channel)
 */
int gs(void);

/*
 * st - stops the robot (the robot holds its current position)
 */
int st(void);

/*
 * lp - set motor limp (the robot may not hold its position).
 */
int lp(void);

/*
 * tk - sends the character stream, talk_string, to the voice synthesizer
 *      to make the robot talk.
 *
 * parameters:
 *    char *talk_string -- the string to be sent to the synthesizer.
 */
int tk(char *talk_string);

/*
 * dp - define the current position of the robot as (x,y)
 * 
 * parameters:
 *    int x, y -- the position to set the robot to.
 */
int dp(int x, int y);

/*
 * zr - zeroing the robot, align steering and turret with bumper zero.
 *      The position, steering and turret angles are all set to zero.
 *      This function returns when the zeroing process has completed.
 */
int zr(void);

/*
 * conf_ir - configure infrared sensor system.
 *
 * parameters: 
 *    int history -- specifies the percentage dependency of the current 
 *                   returned reading on the previous returned reading.
 *                   It should be set between 0 and 10: 0 = no dependency 
 *                   10 = full dependency, i.e. the reading will not change
 *    int order[16] --  specifies the firing sequence of the infrared 
 *                      (#0 .. #15). You can terminate the order list by a 
 *                      "255". For example, if you want to use only the 
 *                      front three infrared sensors then set order[0]=0,
 *                      order[1]=1, order[2]=15, order[3]=255 (terminator).
 */
int conf_ir(int history, int order[16]);

/*
 * conf_sn - configure sonar sensor system.
 *
 * parameters:
 *    int rate -- specifies the firing rate of the sonar in 4 milli-seconds 
 *                interval; 
 *    int order[16] -- specifies the firing sequence of the sonar (#0 .. #15).
 *                     You can terminate the order list by a "255". For 
 *                     example, if you want to use only the front three 
 *                     sensors, then set order[0]=0, order[1]=1, order[2]=15, 
 *                     order[3]=255 (terminator).
 */
int conf_sn(int rate, int order[16]);

/*
 * conf_cp - configure compass system.
 * 
 * parameters:
 *    int mode -- specifies compass on/off: 0 = off ; 1 = on; 2 = calibrate.
 *                When you call conf_cp (2), the robot will rotate slowly 360
 *                degrees. You must wait till the robot stops rotating before
 *                issuing another command to the robot (takes ~3 minutes).
 */
int conf_cp(int mode);


/*
 * conf_tm - sets the timeout period of the robot in seconds. If the
 *           robot has not received a command from the host computer 
 *           for more than the timeout period, it will abort its  
 *           current motion 
 *  
 * parameters: 
 *    unsigned int timeout -- timeout period in seconds. If it is 0, there
 *                            will be no timeout on the robot.
 */
int conf_tm(unsigned char timeout);

/*
 * get_ir - get infrared data, independent of mask. However, only 
 *          the active infrared sensor readings are valid. It updates
 *          the State vector.
 */
int get_ir(void);

/*
 * get_sn - get sonar data, independent of mask. However, only 
 *          the active sonar sensor readings are valid. It updates
 *          the State vector.
 */
int get_sn(void);

/*
 * get_rc - get robot configuration data (x, y, th, tu), independent of 
 *          mask. It updates the State vector.
 */
int get_rc(void);

/*
 * get_rv - get robot velocities (translation, steering, and turret) data,
 *          independent of mask. It updates the State vector.
 */
int get_rv(void);

/*
 * get_ra - get robot acceleration (translation, steering, and turret) data,
 *          independent of mask. It updates the State vector.
 */
int get_ra(void);

/*
 * get_cp - get compass data, independent of mask. However, the
 *          data is valid only if the compass is on. It updates the
 *          State vector.
 */
int get_cp(void);

/*
 * get_bp - get bumper data, independent of mask. It updates the State
 *          vector.
 */
int get_bp(void);

/*
 * get_sg - get laser data line mode, independent of mask. It updates
 *          the laser vector.
 */
int get_sg(void);

/*
 * da - define the current steering angle of the robot to be th
 *      and the current turret angle of the robot to be tu.
 * 
 * parameters:
 *    int th, tu -- the steering and turret orientations to set the
 *                  robot to.
 */
int da(int th, int tu);

/*
 * ws - waits for stop of motors of the robot. This function is intended  
 *      to be used in conjunction with pr() and pa() to detect the desired
 *      motion has finished
 *
 * parameters:
 *    unsigned char t_ws, s_ws, r_ws -- These three parameters specify 
 *                                      which axis or combination of axis 
 *                                      (translation, steering, and turret) 
 *                                      to wait. 
 *    unsigned char timeout -- specifies how long to wait before timing out 
 *                             (return without stopping the robot).
 */
int ws(unsigned char t_ws, unsigned char s_ws,
       unsigned char r_ws, unsigned char timeout);



/*******************************
 *                             *
 * Miscellaneous robot control *
 *                             *
 *******************************/

/*
 * init_mask - initialize the sensor mask, Smask.
 */
void init_mask(void);

/*
 * init_sensors - initialize the sensor mask, Smask, and send it to the
 *                robot. It has no effect on the sensors 
 */
int init_sensors(void);

/*
 * place_robot - places the robot at configuration (x, y, th, tu). 
 *               In simulation mode, it will place both the Encoder-robot
 *               and the Actual-robot at this configuration. In real robot
 *               mode, it will call dp(x, y) and da(th, tu).
 * 
 * parameters:
 *    int x, y -- x-y position of the desired robot configuration
 *    int th, tu -- the steering and turret orientation of the robot
 *                  desired configuration
 */
int place_robot(int x, int y, int th, int tu);

/*
 * special_request - sends a special request (stored in user_send_buffer) 
 *                   to the robot and waits for the robot's response (which
 *                   will be stored in user_receive_buffer). 
 * 
 * parameters:
 *    unsigned char *user_send_buffer -- stores data to be sent to the robot
 *                                       Should be a pointer to an array of
 *                                       1024 elements
 *    unsigned char *user_receive_buffer -- stores data received from the robot
 *                                          Should be a pointer to an array of 
 *                                          1024 elements
 */
int special_request(unsigned char *user_send_buffer,
		    unsigned char *user_receive_buffer);

/*******************************************
 *                                         *
 * The following are helper functions for  *
 * developing user defined host <-> robot  *
 * communication                           *
 *                                         *
 *******************************************/

/*
 *  init_receive_buffer - sets the index to 4 which is the point
 *  at which data should begin to be extracted
 * 
 *  parameters:
 *     unsigned short *index -- is the buffer index
 */
int init_receive_buffer(unsigned short *index);

/*
 *  extract_receive_buffer_header - extracts the header information:
 *  length, serial_number, and packettype from the beginning of the
 *  receive buffer.
 *
 *  parameters:
 *     short *length -- is the returns the number of chars in the buffer
 *
 *     unsigned char *serial_number -- returns the serial number to be
 *                                     assigned to the packet
 *     unsigned char *packet_type -- returns the type number to be
 *                                   assigned to the packet
 *     unsigned char *buffer -- is the receive buffer
 */
int extract_receive_buffer_header(unsigned short *length, 
				  unsigned char *serial_number, 
				  unsigned char *packet_type, 
				  unsigned char *buffer);

/*
 *  init_send_buffer - sets the index to 4 which is the point
 *  at which data should be inserted
 *
 *  parameters:
 *     unsigned short *index -- is the buffer index
 */
int init_send_buffer(unsigned short *index);

/*
 *  stuff_send_buffer_header - loads the header information,
 *  length,serial_number, and packettype into the beginning of the
 *  buffer.  It should be called after the data has been stuffed,
 *  i.e. index represents the length of the packet.
 *
 *  parameters:
 *     int index -- is the buffer index which holds the number of chars
 *                  in the buffer
 *     unsigned char serial_number -- holds the serial number to be
 *                                    assigned to the packet
 *     unsigned char packet_type -- holds the type number to be
 *	                           assigned to the packet
 *
 *     unsigned char *buffer -- is the send buffer
 */
int stuff_send_buffer_header(unsigned short index, unsigned char serial_number, 
			     unsigned char packet_type, unsigned char *buffer);

/*
 *  stuffchar -  stuffs a 1 byte char into the send buffer
 *
 *  parameters:
 *     signed char data -- is the char to be stuffed
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *                              to reflect the bytes stuffed into the buffer
 */
int stuffchar(signed char data, unsigned char *buffer, unsigned short *index);

/*
 *  stuff2byteint - stuffs a short int(2 bytes) into the send buffer
 *
 *  parameters:
 *     signed int data -- is the value which will be split apart and stuffed
 *	                  bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff2byteint(signed short data,
		  unsigned char *buffer, unsigned short *index);

/*
 *  stuff4byteint - stuffs a long int(4 bytes) into the send buffer
 *
 *  parameters:
 *     signed long data -- is the value which will be split apart and stuffed
 *	                   bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff4byteint(signed long data,
		  unsigned char *buffer, unsigned short *index);

/*
 *  stuffuchar -  stuffs an unsigned char into the send buffer
 *
 *  parameters:
 *     unsigned char data -- is the char to be stuffed
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *                              to reflect the bytes stuffed into the buffer
 */
int stuffuchar(unsigned char data,
	       unsigned char *buffer, unsigned short *index);

/*
 *  stuff2byteuint - stuffs an unsigned short int(2 bytes) into the send buffer
 *
 *  parameters:
 *     unsigned short data -- is the value which will be split apart and 
 *                            stuffed bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff2byteuint(unsigned short data, unsigned char *buffer, unsigned short *index);

/*
 *  stuff4byteuint - stuffs an unsigned long int(4 bytes) into the send buffer
 *
 *  parameters:
 *     unsigned long data -- is the value which will be split apart and stuffed
 *	                     bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuff4byteuint(unsigned long data, unsigned char *buffer, unsigned short *index);

/*
 *  stuffdouble - stuffs a double(8 bytes) into the send buffer
 *
 *  parameters:
 *     double data -- is the value which will be split apart and stuffed
 *	              bytewise into the send buffer
 *     unsigned char *buffer -- is the send buffer
 *     unsigned short *index -- is the buffer index which will be incremented
 *	                        to reflect the bytes stuffed into the buffer
 */
int stuffdouble(double data, unsigned char *buffer, unsigned short *index);

/*
 *  extractchar -  extracts a char from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed char extractchar(unsigned char *buffer, unsigned short *index);

/*
 *  extract2byteint -  extracts a short int(2 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed short extract2byteint(unsigned char *buffer, unsigned short *index);

/*
 *  extract4byteint -  extracts a long int(4 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
signed long extract4byteint(unsigned char *buffer, unsigned short *index);

/*
 *  extractuchar -  extracts an unsigned char from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned char extractuchar(unsigned char *buffer, unsigned short *index);

/*
 *  extract2byteuint -  extracts an unsigned short int(2 bytes) from the 
 *                      receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned short extract2byteuint(unsigned char *buffer, unsigned short *index);

/*
 *  extract4byteuint -  extracts an unsigned long int(4 bytes) from the 
 *                      receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
unsigned long extract4byteuint(unsigned char *buffer, unsigned short *index);

/*
 *  extractdouble -  extracts a double(8 bytes) from the receive buffer
 *
 *  parameters:
 *     unsigned char *buffer -- is the receive buffer which holds the data
 *     unsigned short *index -- is the receive buffer index which will be
 *                              incremented to reflect the position of the
 *                              next piece of data to be extracted
 */
double extractdouble(unsigned char *buffer, unsigned short *index);


/*************************************************
 *                                               *
 * Functions for the attachment of position      *
 * data to sensory readings.                     *
 *                                               *
 *************************************************/

/***************
 * FUNCTION:     posDataRequest
 * PURPOSE:      request position information for sensors
 * ARGUMENTS:    int posRequest : 
 *               The argument of this function specifies the sensors 
 *               for which the position information (PosData) should 
 *               be attached to the sensory reading.
 *               Its value is obtained by ORing the desired defines. 
 * EXAMPLE:      To attach PosData to sonars and laser:
 *               posDataRequest ( POS_SONAR | POS_LASER );
 * ALGORITHM:    currently sets the global variable Smask[0] and
 *               then calls ct() to transmit the change to the server
 * RETURN:       TRUE if the argument was correct, else FALSE
 * SIDE EFFECT:  Smask[0]
 * CALLS:        
 * CALLED BY:    
 ***************/
int posDataRequest ( int posRequest );


/***************
 * FUNCTION:     posDataCheck
 * PURPOSE:      return the sensors for which the PosData attachment
 *               is currently requested. 
 * ARGUMENTS:    None
 * ALGORITHM:    returns the usedSmask that is not globally accessible
 *               (is set by ct() to be the value of Smask[0])
 * RETURN:       int, see posDataRequest
 *               the macros POS_*_P can be used to examine the value
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posDataCheck ( void );


/***************
 * FUNCTION:     posInfraredRingGet
 * PURPOSE:      copy the PosData for all infrareds to accessible memory
 * ARGUMENTS:    PosData posData [INFRAREDS] :
 *               an array of PosData structures that is filled with 
 *               PosData. The position information for each infrared
 *               containts the configuration of the robot at the time 
 *               of the sensory reading and a timestamp for the 
 *               configuration and the senosry reading .
 * ALGORITHM:    copies blocks of memory
 * RETURN:       int, return always TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posInfraredRingGet ( PosData posData[INFRAREDS] );


/***************
 * FUNCTION:     posInfraredGet
 * PURPOSE:      copy the PosData for a specific infrared to accessible 
 *               memory
 * ARGUMENTS:    PosData *posData : the memory location that the information
 *                                  will be copied to 
 *               int infraredNumber : the number of the infrared
 * ALGORITHM:    copies block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posInfraredGet     ( PosData *posData , int infraredNumber );


/***************
 * FUNCTION:     posSonarRingGet
 * PURPOSE:      copy the PosData for all sonars to accessible memory
 * ARGUMENTS:    PosData posData [SONARS] :
 *               an array of PosData structures that is filled with 
 *               PosData. The position information for each sonar
 *               containts the configuration of the robot at the time 
 *               of the sensory reading and a timestamp for the 
 *               configuration and the senosry reading .
 * ALGORITHM:    copies blocks of memory
 * RETURN:       int, return always TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posSonarRingGet    ( PosData posData[SONARS] );


/***************
 * FUNCTION:     posSonarGet
 * PURPOSE:      copy the PosData for a specific sonar to accessible memory
 * ARGUMENTS:    PosData *posData : the memory location that the information
 *                                  will be copied to 
 *                        int sonarNumber : the number of the sonar 
 * ALGORITHM:    copies block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 ***************/
int posSonarGet        ( PosData *posData , int sonarNumber );


/***************
 * FUNCTION:     posBumperGet
 * PURPOSE:      copy PosData for the bumper to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The bumper differs from other sensors in that the 
 *               posData is only updated after one of the bumper sensors 
 *               change its value from zero to one. This means that the 
 *               posData for the bumper always contains the position and 
 *               timeStamps of the latest hit, or undefined information 
 *               if the bumper was not hit yet.
 ***************/
int posBumperGet       ( PosData *posData );


/***************
 * FUNCTION:     posLaserGet
 * PURPOSE:      copy PosData for the laser to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The laser is updated at a frequency of 30Hz.
 ***************/
int posLaserGet        ( PosData *posData );


/***************
 * FUNCTION:     posCompassGet
 * PURPOSE:      copy PosData for the compass to accessible memory
 * ARGUMENTS:    PosData *posData : where the data is copied to 
 * ALGORITHM:    copies a block of memory
 * RETURN:       int, always returns TRUE
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The compass is updated ad a frequency of 10Hz.
 ***************/
int posCompassGet      ( PosData *posData );


/***************
 * FUNCTION:     posTimeGet
 * PURPOSE:      get the PosData time (Intellisys 100) in milliseconds
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       int 
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         The resolution of this timer is 16.4 milliseconds;
 *               the timer starts out at zero when the system is 
 *               turned on and will flow over after 49 days.
 ***************/
int posTimeGet         ( void );


/*************************************************
 *                                               *
 * Functions to determine the charge level       *
 * of the batteries for the cpu and the motors.  *
 *                                               *
 *************************************************/

/***************
 * FUNCTION:     voltCpuGet
 * PURPOSE:      get the voltage of the power supply for the CPU
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       float (the voltage in volt)
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         This should never drop below 10.8 volts.
 ***************/
float voltCpuGet       ( void );


/***************
 * FUNCTION:     voltMotorGet
 * PURPOSE:      get the voltage of the power supply for the motors
 * ARGUMENTS:    None
 * ALGORITHM:    ---
 * RETURN:       float (the voltage in volt)
 * SIDE EFFECT:  
 * CALLS:        
 * CALLED BY:    
 * NOTE:         This should never drop below 10.8 volts.
 *               Returns average of the two motor batteries.
 ***************/
float voltMotorGet     ( void );

void nomad_flush_buffer();
#endif /* _HOST_CLIENT_NCLIENT_H_ */
