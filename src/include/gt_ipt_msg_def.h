/****************************************************************************
*                                                                           *
*                      gt_ipt_msg_def.h                                     *
*                                                                           *
*                                                                           *
*  Declare MSG_TYPE array for ipt communication between console & robots    *
*                                                                           *
*  Copyright 1995 - 2006, Georgia Tech Research Corporation                 *
*  Atlanta, Georgia  30332-0415                                             *
*  ALL RIGHTS RESERVED, See file COPYRIGHT for details.                     *
****************************************************************************/

/* $Id: gt_ipt_msg_def.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef GT_IPT_MSG_DEF_H
#define GT_IPT_MSG_DEF_H

#include "ipt/ipt.h"
#include "gt_message_types.h"

char* DefaultConsoleName = "Console";
char* ComVersion  = "$Revision: 1.1.1.1 $";

/************************************/
/* name some formatters we will use */
/************************************/

char* TYP_Point = "Point";
#define FRM_Point "{float, float, float}"
/*
struct REC_Point
{
    double x;
    double y;
    double z;
};
*/

char* TYP_ObjectReading = "ObjectReading";
#define FRM_ObjectReading "{string, int, int, float, float, float, float, float, boolean}"
struct REC_ObjectReading
{
    char* object_color;
    int   id;
    int   object_shape;
    float x1;
    float y1;
    float r;
    float x2;
    float y2;
    bool  ismovable;
};

char* TYP_SensorReading = "SensorReading";
#define FRM_SensorReading "{int,int,float,float}"
struct REC_SensorReading
{
    int type;
    int color;
    float angle;
    float reading;
};

char* TYP_Sensor = "Sensor";
//#define FRM_Sensor "{int, int, string, int, <float: 2>, int, <float: 2> }"
//#define FRM_Sensor "{int, int, <float: 2> }"
/*struct REC_Sensor
{
    int type;
    int num_readings;
    float* reading;
};*/

#define FRM_Sensor "{int, int, int, <float: 3>, <float: 2> }"
struct REC_Sensor
{
    int type;
    int num_readings;
    int num_angles;
    float* angle;
    float* reading;
};



char* MSG_ObtenerSensores = "ObtenerSensores";
#define FRM_ObtenerSensores "int"
typedef int REC_ObtenerSensores;

char* MSG_DevolverSensores = "DevolverSensores";
//#define FRM_DevolverSensores "{int, int, <{int, int, string, int, <float: 2>, int, <float: 2>}: 2>}"
#define FRM_DevolverSensores "{int, int, <Sensor: 2>}"
//#define FRM_DevolverSensores "{int, int, int}"
struct REC_DevolverSensores
{
    int robotId;
    int numLecturas;
    REC_Sensor* sensor;
    //int temp;
};


/* Resets the console world */
/* For simulation only */
char *MSG_ResetWorld = "ResetWorld";
#define FRM_ResetWorld "{string,int}"
typedef struct REC_ResetWorld {
    char* new_overlay;
    int  load_new_map;
} REC_ResetWorld;

char* MSG_UpdateLMParams = "UpdateLMParams";
#define FRM_UpdateLMParams "{int}"
struct REC_UpdateLMParams
{
    int iRobotID;
};

char* MSG_StepTaken = "StepTaken";
#define FRM_StepTaken "{int,int}"
struct REC_StepTaken
{
    int iRobotID;
    int iStep;
};

/************************************************/
/* ipt messages sent from robots to the console */
/************************************************/

char* MSG_RobotBirth = "RobotBirth";
#define FRM_RobotBirth "{int, float, float, float, float, string, string}"
struct REC_RobotBirth
{
    int   robot_id;
    float start_x;
    float start_y;
    float start_z;
    float start_heading;
    char* port_name;
    char* com_version;
};

char* MSG_NotifyConfig = "NotifyConfig";
#define FRM_NotifyConfig "{int, int, int}"
struct REC_NotifyConfig
{
    int robot_id;
    int robot_type;
    int run_type;
};

char* MSG_RobotDeath = "RobotDeath";
#define FRM_RobotDeath "{int, string}"
struct REC_RobotDeath
{
    int   robot_id;
    char* message;
};

char* MSG_TerminateMission = "TerminateMission";
#define FRM_TerminateMission "{int}"
struct REC_TerminateMission
{
    int   robotID;
};

char* MSG_ReportedState = "ReportedState";
#define FRM_ReportedState "{string}"
struct REC_ReportedState
{
    char* message;
};

char* MSG_ReportedLogfile = "ReportedLogfile";
#define FRM_ReportedLogfile "{string}"
struct REC_ReportedLogfile
{
    char* logfileName;
};

char* MSG_Pickup = "Pickup";
#define FRM_Pickup "{int, int}"
struct REC_Pickup
{
    int robot_id;
    int object_id;
};

char* MSG_Probe = "Probe";
#define FRM_Probe "{int, int}"
struct REC_Probe
{
    int robot_id;
    int object_id;
};

char* MSG_Alert = "Alert";
#define FRM_Alert "{string}"
struct REC_Alert
{
    char* message;
};

char* MSG_PopUpTelop = "PopUpTelop";
#define FRM_PopUpTelop "{int, int}"
struct REC_PopUpTelop
{
    int popup;
    int robot_id;
};

char* MSG_AskToProceed = "AskToProceed";
#define FRM_AskToProceed "{int, int}"
struct REC_AskToProceed
{
    int ask;
    int robot_id;
};

char* MSG_ChangeColor = "ChangeColor";
#define FRM_ChangeColor "{int, int, string}"
struct REC_ChangeColor
{
    int   robot_id;
    int   object_id;
    char* new_color;
};

char *MSG_ChangeRobotColor = "ChangeRobotColor";
#define FRM_ChangeRobotColor "{int, string}"
typedef struct {
   int    robot_id;
   char  *new_color;
} REC_ChangeRobotColor;

char* MSG_DropInBasket = "DropInBasket";
#define FRM_DropInBasket "{int, int, int}"
struct REC_DropInBasket
{
    int robot_id;
    int object_id;
    int basket_id;
};

char* MSG_CheckVehicleType = "CheckVehicleType";
#define FRM_CheckVehicleType "{int, int}"
struct REC_CheckVehicleType
{
    int robotID;
    int objectID;
};

char* MSG_VehicleType = "VehicleType";
#define FRM_VehicleType "{int}"
struct REC_VehicleType
{
    int vehicleType;
};

char* MSG_CheckTargetUntrackable = "CheckTargetUntrackable";
#define FRM_CheckTargetUntrackable "{int, int}"
struct REC_CheckTargetUntrackable
{
    int robotID;
    int objectID;
};

char* MSG_TargetUntrackable = "TargetUntrackable";
#define FRM_TargetUntrackable "{boolean}"
struct REC_TargetUntrackable
{
    bool untrackable;
};

char* MSG_MoveRobot = "MoveRobot";
#define FRM_MoveRobot "{int, float, float, float}"
struct REC_MoveRobot
{
    int   robot_id;
    float x;
    float y;
    float z;
};

char* MSG_SpinRobot = "SpinRobot";
#define FRM_SpinRobot "{int, float}"
struct REC_SpinRobot
{
    int   robot_id;
    float w;
};

char* MSG_RobotLocation = "RobotLocation";
#define FRM_RobotLocation "{int, float, float, float, float}"
struct REC_RobotLocation
{
    int   robot_id;
    float x;
    float y;
    float z;
    float heading;
};

char* MSG_BatteryInfo = "BatteryInfo";
#define FRM_BatteryInfo "{int, int}"
struct REC_BatteryInfo
{
    int id;
    int level;
};


char* MSG_GpsPosition = "GpsPosition";
#define FRM_GpsPosition "{int, float, float, float, float, int}"                      //jh
struct REC_GpsPosition
{
    int   robot_id;
    float latitude;
    float longitude;
    float direction;
    float pingtime;
    int pingloss;
};

char* MSG_InquireMapLocation = "InquireMapLocation";
#define FRM_InquireMapLocation "{int, string}"
struct REC_InquireMapLocation
{
    int   robot_id;
    char* name;
};

char* MSG_SetMobilityType = "SetMobilityType";
#define FRM_SetMobilityType "{int, int}"
struct REC_SetMobilityType
{
    int robotID;
    int mobilityType;
};

char* MSG_GetMobility = "GetMobility";
#define FRM_GetMobility "{int}"
struct REC_GetMobility
{
    int robotID;
};

char* MSG_CurMobility = "CurMobility";
#define FRM_CurMobility "{int}"
struct REC_CurMobility
{
    int mobility;
};

char* MSG_GetElevation = "GetElevation";
#define FRM_GetElevation "{int}"
struct REC_GetElevation
{
    int robot_id;
};


char* MSG_CurElevation = "CurElevation";
#define FRM_CurElevation "{float}"
struct REC_CurElevation
{
    float elevation;
};


char* MSG_CancelTask = "CancelTask";
#define FRM_CancelTask "{int, int, int, int, string, boolean}"
struct REC_CancelTask {
    int TaskID;
    int Iteration;
    int RobotID;
    int Consensual;
    char *Info;
    bool IsReneged;
};

char* MSG_GetTask = "GetTask";
#define FRM_GetTask "{int}"
struct REC_GetTask {
    int robot_id;
};

char* MSG_GetTaskConstraints = "GetTaskConstraints";
#define FRM_GetTaskConstraints "{int}"
struct REC_GetTaskConstraints {
    int robotID;
};

char* MSG_GetAward = "GetAward";
#define FRM_GetAward "{int, int, int}"
struct REC_GetAward {
    int TaskID;
    int Iteration;
    int RobotID;
};

char* MSG_GetWater = "GetWater";
#define FRM_GetWater "{float, float}"
struct REC_GetWater {
    float tx;
    float ty;
};

char* MSG_CurWater = "CurWater";
#define FRM_CurWater "{int}"
struct REC_CurWater {
    int Water;
};



char *MSG_CurAward = "CurAward";
#define FRM_CurAward "{int, int, int}"
struct REC_CurAward {
    int TaskID;
    int Iteration;
    int RobotID;
};

char *MSG_CurTask = "CurTask";
#define FRM_CurTask "{int, int, string, string}"
struct REC_CurTask {
    int TaskID;
    int Iteration;
    char *TaskDesc;
    char *Restriction;
};

char *MSG_BidTask = "BidTask";
#define FRM_BidTask "{int, int, int, int, float}"
struct REC_BidTask {
    int TaskID;
    int Iteration;
    int RobotID;
    int TaskAccept;
    float Bid;
};

char *MSG_CNP_RobotConstraintsRequest = "RobotConstraintsRequest";
#define FRM_CNP_RobotConstraintsRequest "{int}"
struct REC_CNP_RobotConstraintsRequest {
    int robotID;
};

char *MSG_CNP_RobotConstraintsData = "CNP_RobotConstraintsData";
#define FRM_CNP_RobotConstraintsData "{int, int, <int: 2>, int, <char: 4>, <int: 2>, <int: 2>, int, <char: 8>}"
struct REC_CNP_RobotConstraintsData {
    int robotID;
    int numConstraints;
    int *nameSizes;
    int totalNameSize;
    char *nameData;
    int *types;
    int *valueSizes;
    int totalValueSize;
    char *valueData;
};

char *MSG_CNP_TaskConstraintsData = "CNP_TaskConstraintsData";
#define FRM_CNP_TaskConstraintsData "{int, <int: 1>, <int: 1>, int, <char: 4>, <int: 1>, int, <char: 7>, <int: 1>, <int: 1>, int, <char: 11>}"
struct REC_CNP_TaskConstraintsData {
    int numConstraints;
    int *taskInfoIDs;
    int *taskInfoNameSizes;
    int taskInfoTotalNameSize;
    char *taskInfoNameData;
    int *nameSizes;
    int totalNameSize;
    char *nameData;
    int *types;
    int *valueSizes;
    int totalValueSize;
    char *valueData;
};

char* MSG_GetPosition = "GetPosition";
#define FRM_GetPosition "{int}"
struct REC_GetPosition
{
    int robot_id;
};

char* MSG_GetCompassHeading = "GetCompassHeading";
#define FRM_GetCompassHeading "{int}"
struct REC_GetCompassHeading
{
    int heading;
};

char* MSG_MarkDoorway = "MarkDoorway";
#define FRM_MarkDoorway "{int, int}"
struct REC_MarkDoorway
{
    int robot_id;
    int mark_status;
};

char* MSG_DetectDoorway = "DetectDoorway";
#define FRM_DetectDoorway "{int, double, double, double}"
struct REC_DetectDoorway
{
    int    robot_id;
    double sensor_dir;
    double sensor_fov;
    double sensor_range;
};

char* MSG_DetectDoorwayReply = "DetectDoorwayReply";
#define FRM_DetectDoorwayReply "{int, <{float, float, float, float, double, int}: 1>}"

char* MSG_DetectHallway = "DetectHallway";
#define FRM_DetectHallway "{int, {{double, double, double}, double}}"
struct REC_DetectHallway
{
    int            robot_id;
    robot_position robot_loc;
};

char* MSG_DetectHallwayReply = "DetectHallwayReply";
#define FRM_DetectHallwayReply "{int, <{{double, double, double}, {double, double, double}, double}: 1>}"

char* MSG_DetectRoom = "DetectRoom";
#define FRM_DetectRoom "{int, {{double, double, double}, double}}"
struct REC_DetectRoom
{
    int            robot_id;
    robot_position robot_loc;
};

char* MSG_DetectRoomReply = "DetectRoomReply";
#define FRM_DetectRoomReply "{boolean}"

char* MSG_DetectObstacles = "DetectObstacles";
#define FRM_DetectObstacles "{int, float}"
struct REC_DetectObstacles
{
    int   robot_id;
    float max_dist;
};

char* MSG_DetectObjects = "DetectObjects";
#define FRM_DetectObjects "{int, float}"
struct REC_DetectObjects
{
    int   robot_id;
    float max_dist;
};

char* MSG_DetectRobots = "DetectRobots";
#define FRM_DetectRobots "{int, float}"
struct REC_DetectRobots
{
    int   robot_id;
    float max_dist;
};

char* MSG_GetRobotsPos = "GetRobotsPos";
#define FRM_GetRobotsPos "{int, double}"
struct REC_GetRobotsPos
{
    int   robot_id;
    double max_dist;
};


// reports from robot to console

char* MSG_PutState = "PutState";
#define FRM_PutState "{int, string, string}"
struct REC_PutState
{
    int   robot_id;
    char* key;
    char* value;
};


char* MSG_ObjectReadings = "ObjectReadings";
#define FRM_ObjectReadings "{int, <ObjectReading: 1>}"
struct REC_ObjectReadings
{
    int num_readings;
    REC_ObjectReading* readings;
};

// why can't use RobotLocation?
char* MSG_RobotPos = "RobotPos";
#define FRM_RobotPos "{float, float, float}"
struct REC_RobotPos
{
    float x;
    float y;
    float heading;
};

char* MSG_LaserReadings = "LaserReadings";
#define FRM_LaserReadings "{int, int, <float: 2>, <float: 2>}"
struct REC_LaserReadings
{
    int robot_id;
    int num_readings;
    float* readings;
    float* angles;
};

char* MSG_SensorReadings = "SensorReadings";
#define FRM_SensorReadings "{int, int, <SensorReading: 2>}"
struct REC_SensorReadings
{
    int robot_id;
    int num_readings;
    REC_SensorReading* reading;
};

char* MSG_PingSend = "PingSend";
#define FRM_PingSend "{int}"
struct REC_PingSend
{
    int check;
};

/****************************************************/
/* IPT messages sent from the console to a robot    */
/****************************************************/

char* MSG_Ack = "Ack";
#define FRM_Ack "{boolean}"
struct REC_Ack
{
    bool junk;
};

char* MSG_StepOneCycle = "StepOneCycle";
#define FRM_StepOneCycle "boolean"
typedef short REC_StepOneCycle;

char* MSG_RobotPause = "RobotPause";
#define FRM_RobotPause "boolean"
typedef short REC_RobotPause;

char* MSG_RobotFeedback = "RobotFeedback";
#define FRM_RobotFeedback "boolean"
typedef short REC_RobotFeedback;

char* MSG_RobotDebug = "RobotDebug";
#define FRM_RobotDebug "boolean"
typedef short REC_RobotDebug;

char* MSG_ReportState = "ReportState";
#define FRM_ReportState "boolean"
typedef short REC_ReportState;

char* MSG_RobotSuicide = "RobotSuicide";
#define FRM_RobotSuicide "int"
typedef int REC_RobotSuicide;

char* MSG_MapLocation = "MapLocation";
#define FRM_MapLocation "{boolean, Point}"

struct REC_MapLocation
{
    bool valid;
    gt_Point pos;
};

char* MSG_CurPosition = "CurXY";
#define FRM_CurPosition "{float, float, float}"

char* MSG_ObstacleList = "ObstacleList";
#define FRM_ObstacleList "{int, <Point: 1>, <float: 1>}"
struct REC_ObstacleList
{
    int num_obs;
    gt_Point* obs;
    float* radius;
};

char* MSG_ObjectList = "ObjectList";
#define FRM_ObjectList "{int, <ObjectReading: 1>}"
struct REC_ObjectList
{
    int num_objects;
    REC_ObjectReading* object;
};

char* MSG_CurRobots = "CurRobots";
#define FRM_CurRobots "{int, <{int, {float, float, float}, double}: 1>}"
struct REC_CurRobots
{
    int num_robots;
    gt_Id_and_v* robots;
};


char* MSG_RobotsPosReply = "RobotsPosReply";
#define FRM_RobotsPosReply "{int, <{int, double, double, double, double}: 1>}"
struct REC_RobotsPosReply
{
    int numRobots;
    robot_pos_id_t* robots;
};


char* MSG_NewCommand = "NewCommand";
#define FRM_NewCommand "{int, <{float, float, float}: 1>, int, <int: 3>, int, <{float, float, float}: 5>, {float, float, float}, int, int, int, float, float, int, int, string, int, int, string, int, int, int, int}"
struct REC_NewCommand
{
    int           num_path_points;
    gt_Point*     path;

    int           unit_size;
    int*          unit;

    int           num_pl_points;
    gt_Point*     phase_line;

    gt_Point 	  goal;
    gt_Behavior   behavior;
    gt_Formation  formation;
    gt_Technique  technique;
    float         speed;
    float         width;
    int           phase_line_given;
    int           time;   // Seconds since epoch (see 'man 3 time').
                          // 0 means time is not a factor (no phase line).
                          // Negative means seconds for timeout.
    char*         phase_line_ack_msg;
    int           wait_at_phase_line;
    int           completion_given;
    char*         completion_msg;
    int           freeze_on_completion;
    int           until_timeout_given;
    int           robot_id;
    int           seq_num;    // unique command sequence number

};
// Note the above is a duplicate of gt_Command defined in gt_command.h


char* MSG_NewState = "NewState";
#define FRM_NewState "{string, string}"
struct REC_NewState
{
    char* key;
    char* value;
};


char* MSG_Declare_iptClient = "Declare_iptClient";
#define FRM_Declare_iptClient "{int, string}"
struct REC_Declare_iptClient
{
    int server_id;
    char* server_name;
};

char* MSG_AddObject = "AddObject";
#define FRM_AddObject "{int, string, float, float, float, int}"
struct REC_AddObject
{
    int   robot_id;
    char* color;
    float x;
    float y;
    float diameter;
    int   style;
};

char* MSG_AddPassagePoint = "AddPassagePoint";
#define FRM_AddPassagePoint "{int, float, float, int, <char: 4>}"
struct REC_AddPassagePoint
{
    int robotID;
    float x;
    float y;
    int labelSize;
    char *label;
};

char* MSG_DisplayBitmapIcon = "DisplayBitmapIcon";
#define FRM_DisplayBitmapIcon "{int, float, float, int, <char: 4>, int, <char: 6>, int, <char: 8>, int, <char: 10>, boolean}"
struct REC_DisplayBitmapIcon
{
    int robotID;
    float x;
    float y;
    int bitmapNameSize;
    char *bitmapName;
    int labelSize;
    char *label;
    int enforcedFgColorSize;
    char *enforcedFgColor;
    int enforcedBgColorSize;
    char *enforcedBgColor;
};

char* MSG_PingBack = "PingBack";
#define FRM_PingBack "{int}"
struct REC_PingBack
{
    int check;
};

char* MSG_DrawingVectorField = "DrawingVectorField";
#define FRM_DrawingVectorField "int"
typedef int REC_DrawingVectorField;

char* MSG_DoneDrawingVectorField = "DoneDrawingVectorField";
#define FRM_DoneDrawingVectorField "int"
typedef int REC_DoneDrawingVectorField;

char* MSG_ReportSensorReadings = "ReportSensorReadings";
#define FRM_ReportSensorReadings "int"
typedef int REC_ReportSensorReadings;


/****************************************************/
/* IPT mensajes mandados de robot a robot           */
/****************************************************/

char* MSG_ConocerRobots = "ConocerRobots";
#define FRM_ConocerRobots "{string}"
typedef char *REC_ConocerRobots;

char* MSG_TeSigo = "TeSigo";
#define FRM_TeSigo "{string}"
typedef char *REC_TeSigo;

char * MSG_PosicionLider = "PosicionLider";
#define FRM_PosicionLider "{{double, double, double}, float}"
struct REC_PosicionLider
{
    Vector posicion;
    float heading;
};

char* MSG_CambioEstado = "CambioEstado";
#define FRM_CambioEstado "{int}"
typedef int REC_CambioEstado;

char* MSG_PosicionDeseadaSolicitud = "PosicionDeseadaSolicitud";
#define FRM_PosicionDeseadaSolicitud "{int}"
typedef int REC_PosicionDeseadaSolicitud;

char* MSG_PosicionDeseadaContestacion = "PosicionDeseadaContestacion";
#define FRM_PosicionDeseadaContestacion "{double, double, double}"
typedef Vector REC_PosicionDeseadaContestacion;

char* MSG_GetPosicionLider = "GetPosicionLider";
#define FRM_GetPosicionLider "{int}"
typedef int REC_GetPosicionLider;


char* MSG_GetSignalStrength = "MSG_GetSignalStrength";
#define FRM_GetSignalStrength "{int, float}"
struct REC_GetSignalStrength
{
	int robotId;
	float filter;
};

char* MSG_SignalStrength = "MSG_SignalStrength";
#define FRM_SignalStrength "{float}"
struct REC_SignalStrength
{
	float value;
};

/***********************************/
/* array declaring all ipt formats */

IPFormatSpec formatArray[] =
{
    { TYP_Point,                FRM_Point },
    { TYP_ObjectReading,        FRM_ObjectReading },
    { TYP_SensorReading,        FRM_SensorReading },
	{ TYP_Sensor,		FRM_Sensor },//Added by teny y alber
    { NULL,                     NULL }
};


/*******************************************************************/
// array declaring all ipt messages
IPMessageSpec messageArray[] = {

    // IPT messages sent from robots to the console or vice versa.
    { MSG_RobotBirth,             FRM_RobotBirth },
    { MSG_NotifyConfig,           FRM_NotifyConfig },
    { MSG_RobotDeath,             FRM_RobotDeath },
    { MSG_TerminateMission,       FRM_TerminateMission },
    { MSG_Pickup,                 FRM_Pickup },
    { MSG_CheckVehicleType,       FRM_CheckVehicleType },
    { MSG_VehicleType,            FRM_VehicleType },
    { MSG_CheckTargetUntrackable, FRM_CheckTargetUntrackable },
    { MSG_TargetUntrackable,      FRM_TargetUntrackable },
    { MSG_Probe,                  FRM_Probe },
    { MSG_Alert,                  FRM_Alert },
    { MSG_PopUpTelop,             FRM_PopUpTelop },
    { MSG_AskToProceed,           FRM_AskToProceed },
    { MSG_ChangeColor,            FRM_ChangeColor },
    { MSG_ChangeRobotColor,       FRM_ChangeRobotColor },
    { MSG_DropInBasket,           FRM_DropInBasket },
    { MSG_MoveRobot,              FRM_MoveRobot },
    { MSG_SpinRobot,              FRM_SpinRobot },
    { MSG_RobotLocation,          FRM_RobotLocation },
    { MSG_GpsPosition,            FRM_GpsPosition },
    { MSG_BatteryInfo,            FRM_BatteryInfo },
    { MSG_InquireMapLocation,     FRM_InquireMapLocation },
    { MSG_DetectDoorway,          FRM_DetectDoorway},
    { MSG_DetectHallway,          FRM_DetectHallway},
    { MSG_DetectRoom,             FRM_DetectRoom},
    { MSG_MarkDoorway,            FRM_MarkDoorway},
    { MSG_GetPosition,            FRM_GetPosition },
    { MSG_SetMobilityType,        FRM_SetMobilityType },
    { MSG_GetMobility,            FRM_GetMobility},
    { MSG_CurMobility,            FRM_CurMobility},
    { MSG_GetElevation,           FRM_GetElevation },
    { MSG_GetTask,                FRM_GetTask },
    { MSG_GetTaskConstraints,     FRM_GetTaskConstraints },
    { MSG_CurTask,                FRM_CurTask },
    { MSG_BidTask,                FRM_BidTask },
    { MSG_GetAward,               FRM_GetAward },
    { MSG_CurAward,               FRM_CurAward },
    { MSG_CurWater,               FRM_CurWater },
    { MSG_CancelTask,             FRM_CancelTask },
    { MSG_GetWater,               FRM_GetWater },
    { MSG_CurElevation,           FRM_CurElevation },
    { MSG_GetCompassHeading,      FRM_GetCompassHeading },
    { MSG_DetectObstacles,        FRM_DetectObstacles },
    { MSG_DetectObjects,          FRM_DetectObjects },
    { MSG_DetectRobots,           FRM_DetectRobots },
    { MSG_GetRobotsPos,           FRM_GetRobotsPos },
    { MSG_PutState,               FRM_PutState },
    { MSG_ObjectReadings,         FRM_ObjectReadings },
    { MSG_RobotPos,               FRM_RobotPos },
    { MSG_LaserReadings,          FRM_LaserReadings },
    { MSG_ReportedState,          FRM_ReportedState },
    { MSG_ReportedLogfile,        FRM_ReportedLogfile },
    { MSG_PingSend,               FRM_PingSend},
    { MSG_Ack,                    FRM_Ack },
    { MSG_StepOneCycle,           FRM_StepOneCycle },
    { MSG_RobotPause,             FRM_RobotPause },
    { MSG_RobotFeedback,          FRM_RobotFeedback },
    { MSG_RobotSuicide,           FRM_RobotSuicide },
    { MSG_RobotDebug,             FRM_RobotDebug },
    { MSG_ReportState,            FRM_ReportState },
    { MSG_MapLocation,            FRM_MapLocation },
    { MSG_CurPosition,            FRM_CurPosition },
    { MSG_ObstacleList,           FRM_ObstacleList },
    { MSG_ObjectList,             FRM_ObjectList },
    { MSG_CurRobots,              FRM_CurRobots },
    { MSG_RobotsPosReply,         FRM_RobotsPosReply },
    { MSG_NewCommand,             FRM_NewCommand },
    { MSG_NewState,               FRM_NewState },
    { MSG_Declare_iptClient,      FRM_Declare_iptClient },
    { MSG_AddObject,              FRM_AddObject },
    { MSG_AddPassagePoint,        FRM_AddPassagePoint},
    { MSG_DisplayBitmapIcon,      FRM_DisplayBitmapIcon},
    { MSG_DetectDoorwayReply,     FRM_DetectDoorwayReply},
    { MSG_DetectHallwayReply,     FRM_DetectHallwayReply},
    { MSG_DetectRoomReply,        FRM_DetectRoomReply},
    { MSG_SensorReadings,         FRM_SensorReadings},
    { MSG_PingBack,               FRM_PingBack},
    { MSG_UpdateLMParams,         FRM_UpdateLMParams},
    { MSG_StepTaken,              FRM_StepTaken},
    { MSG_DrawingVectorField,     FRM_DrawingVectorField },
    { MSG_DoneDrawingVectorField, FRM_DoneDrawingVectorField },
    { MSG_ReportSensorReadings,   FRM_ReportSensorReadings },
    { MSG_ResetWorld,             FRM_ResetWorld },
    { MSG_CNP_RobotConstraintsRequest, FRM_CNP_RobotConstraintsRequest },
    { MSG_CNP_RobotConstraintsData, FRM_CNP_RobotConstraintsData },
    { MSG_CNP_TaskConstraintsData, FRM_CNP_TaskConstraintsData },
    { MSG_ConocerRobots,          FRM_ConocerRobots },
    { MSG_TeSigo,                 FRM_TeSigo },
    { MSG_PosicionLider,          FRM_PosicionLider },
    { MSG_CambioEstado,           FRM_CambioEstado },
    { MSG_PosicionDeseadaSolicitud, FRM_PosicionDeseadaSolicitud },
    { MSG_PosicionDeseadaContestacion, FRM_PosicionDeseadaContestacion },
    { MSG_GetPosicionLider,       FRM_GetPosicionLider },
    { MSG_ObtenerSensores,	  FRM_ObtenerSensores},
    { MSG_DevolverSensores,       FRM_DevolverSensores},
    { MSG_GetSignalStrength,       FRM_GetSignalStrength},
    { MSG_SignalStrength,       FRM_SignalStrength},
    { NULL,                       NULL }
};

#endif  // GT_IPT_MSG_DEF_H

/**********************************************************************
 * $Log: gt_ipt_msg_def.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.9  2006/06/08 22:33:55  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.8  2006/05/02 04:12:21  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.7  2006/02/19 17:56:41  endo
 * Experiment related modifications
 *
 * Revision 1.6  2006/01/10 06:21:59  endo
 * AO-FNC Type-I check-in.
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
 * Revision 1.2  2005/02/07 23:14:04  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:47  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.20  2003/04/02 23:07:32  zkira
 * Added GPS and battery messages
 *
 * Revision 1.19  2002/10/31 21:28:00  ebeowulf
 * Added messaging and header for Reset World code.
 *
 * Revision 1.18  2002/10/31 20:56:31  ebeowulf
 * Fixed typo in ChangeRobotColor code.
 *
 * Revision 1.17  2002/10/31 20:00:00  ebeowulf
 * added change robot color function and message
 *
 * Revision 1.16  2002/07/02 20:36:27  blee
 * added messages so mlab could draw vector fields
 *
 * Revision 1.15  2002/04/04 18:35:28  blee
 * Added MSG_StepTaken stuff.
 *
 * Revision 1.14  2000/12/12 22:44:15  blee
 * altered MSG_LaserReadings stuff
 *
 * Revision 1.13  2000/11/10 23:29:06  blee
 * added UpdateLMParams message info (and in the message table)
 *
 * Revision 1.12  2000/08/19 00:57:26  endo
 * MSG_PingSend, MSG_PingBack added.
 *
 * Revision 1.11  2000/08/15 20:52:34  endo
 * MSG_AskToProceed added.
 *
 * Revision 1.10  2000/07/02 01:18:24  conrad
 * added sensor messages
 *
 * Revision 1.9  2000/06/13 16:44:35  endo
 * MSG_Declare_iptClient added.
 *
 * Revision 1.8  2000/05/19 21:39:11  endo
 * Ipt message handling for PopUpTelop added.
 *
 * Revision 1.7  2000/04/25 08:51:28  jdiaz
 * added start_heading field to robot_birth message
 *
 * Revision 1.6  2000/03/30 01:42:17  endo
 * MSG_SpinRobot FRM_SpinRobot REC_SpinRobot added.
 *
 * Revision 1.5  2000/02/28 23:54:30  jdiaz
 * messages to mark and unmark a doorway
 *
 * Revision 1.4  2000/02/18 02:02:00  endo
 * REC_Alert, Msg_Alert, FRM_Alert added for the
 * Alert state.
 *
 * Revision 1.3  2000/02/07 05:20:30  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.2  1999/12/18 09:18:55  endo
 * ReportedState (robot -> mlab)
 * ReportState (mlab -> robot)
 *
 **********************************************************************/
