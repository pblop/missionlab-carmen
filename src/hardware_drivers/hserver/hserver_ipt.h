/**********************************************************************
 **                                                                  **
 **                           hserver_ipt.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  HServer IPT messages                                            **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: hserver_ipt.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef HSERVER_IPT_H
#define HSERVER_IPT_H

#include "ipt/ipt.h"
#include "ipt/message.h"

#define MODULE_NOT_CONNECTED   0
#define MODULE_UNKNOWN         1
#define MODULE_ID_GPS          2
#define MODULE_ID_LASER        3

extern char *MSG_InitConnection;
#define FRM_InitConnection "string"
typedef struct REC_InitConnection {
    char *name;
} REC_InitConnection;


extern char *TYP_SensorReadingT;
#define FRM_SensorReadingT "{int,int,<char: 2>,int,<float: 4>,int,<float: 6>}"
typedef struct REC_SensorReadingT {
    int type;
    int len_extra;
    char *extra;
    int num_angles;
    float *angle;
    int num_readings;
    float *reading;
} REC_SensorReadingT;

extern char *TYP_RobotPartT;
#define FRM_RobotPartT "{int,int,<char: 2>}"
typedef struct REC_RobotPartT {
    int type;
    int len_info;
    char *info;
} REC_RobotPartT;

extern char *MSG_RobotSetupInfo;
#define FRM_RobotSetupInfo "{string,int,<RobotPartT: 2>}"
typedef struct REC_RobotSetupInfo {
    char *name;
    int num_parts;
    REC_RobotPartT *part;
} REC_RobotSetupInfo;

extern char *MSG_BatteryUpdate;
#define FRM_BatteryUpdate "{int}"
typedef struct REC_BatteryUpdate {
  int battery_level;
} REC_BatteryUpdate;

extern char *TYP_JboxReadingT;
#define FRM_JboxReadingT "{int,int,int,int,<double: 4>}"
typedef struct REC_JboxReadingT
{
    int id;
    int num_hops;
    int signal_strength;
    int num_readings;
    double *gps_reading;
} REC_JboxReadingT;

extern char *MSG_JboxUpdate;
#define FRM_JboxUpdate "{int,<JboxReadingT: 1>}"
typedef struct REC_JboxUpdate
{
    int neighbor_num;
    REC_JboxReadingT *jbox_readingt;
} REC_JboxUpdate;

extern char *MSG_SensorUpdate;
#define FRM_SensorUpdate "{int,<SensorReadingT: 1>}"
typedef struct REC_SensorUpdate {
    int num_sensors;
    REC_SensorReadingT *sensor;
} REC_SensorUpdate;

extern char *MSG_SimpleCommand;
#define FRM_SimpleCommand "{int}"
typedef struct REC_SimpleCommand {
    int command;
} REC_SimpleCommand;

extern char *MSG_DriveCommand;
#define FRM_DriveCommand "{float}"
typedef struct REC_DriveCommand {
    float vel;  //m per sec
} REC_DriveCommand;

extern char *MSG_SteerCommand;
#define FRM_SteerCommand "{float}"
typedef struct REC_SteerCommand {
    float avel;  //deg per sec
} REC_SteerCommand;

extern char *MSG_SteerTowardDriveCommand;
#define FRM_SteerTowardDriveCommand "{float,float,float,int}"
typedef struct REC_SteerTowardDriveCommand {
    float theta;  //angle
    float speed;  //m per sec
    float drive_wait_angle; // deg
    int use_reverse;
} REC_SteerTowardDriveCommand;

extern char *MSG_SetXYT;
#define FRM_SetXYT "{float,float,float}"
typedef struct REC_SetXYT {
    float x;
    float y;
    float t;
} REC_SetXYT;

extern char *MSG_InitRobot;
#define FRM_InitRobot "{double,double,double}"
typedef struct REC_InitRobot {
    double x;
    double y;
    double heading;
} REC_InitRobot;

extern char *MSG_MoveCamera;
#define FRM_MoveCamera "{float,float}"
typedef struct REC_MoveCamera {
    float pan;
    float tilt;
} REC_MoveCamera;

extern char *MSG_SetCameraTrackerMode;
#define FRM_SetCameraTrackerMode "{int}"
typedef struct REC_SetCameraTrackerMode {
    int mode;
} REC_SetCameraTrackerMode;

extern char *MSG_FrameUpload;
#define FRM_FrameUpload "{int,int,string}"
typedef struct REC_FrameUpload {
    int channel;
    int send;
    char *name;
} REC_FrameUpload;

extern char  *MSG_VideoFrame;
#define FRM_VideoFrame "{int,int,int,<char: 3>}"
typedef struct REC_VideoFrame {
    int width;
    int height;
    int size;
    char *frame;
} REC_VideoFrame;

extern char *MSG_GpsRTCM;
#define FRM_GpsRTCM "{int,<char:1>}"
typedef struct REC_GpsRTCM {
    int size;
    char *data;
} REC_GpsRTCM;

extern char *MSG_ModuleId;
#define FRM_ModuleId "{int}"
typedef struct REC_ModuleId {
    int id;
} REC_ModuleId;

extern char *MSG_LaserScan;
#define FRM_LaserScan "{int,int,<int :1>}"
typedef struct REC_LaserScan {
    int size;
    int min;
    unsigned int *scan;
} REC_LaserScan;


extern IPFormatSpec iptHclientFormatArray[];

extern IPMessageSpec iptHclientMessageArray[];

extern IPMessageSpec iptGpsMessageArray[];

extern IPMessageSpec iptHandlerMessageArray[];

extern IPMessageSpec iptLaserMessageArray[];

extern IPMessageSpec iptMazeMessageArray[];

struct IPMsgCallbackHandlerSpec
{
    char* msg_name;
    IPHandlerCallback *callback;
    int context;
};

extern char *MSG_GetPinza;
#define FRM_GetPinza "{float}"
typedef struct REC_GetPinza {
  float altura;   //m
} REC_GetPinza;

extern char *MSG_SetPinza;
#define FRM_SetPinza "{float}"
typedef struct REC_SetPinza {
    float altura;  //m
} REC_SetPinza;

extern char *MSG_MotoresAspirar;
#define FRM_MotoresAspirar "{int}"
typedef struct REC_MotoresAspirar {
    int power;  //Activa o desactiva los motores de limpieza
} REC_MotoresAspirar;

extern char *MSG_AdvancedTelop;
#define FRM_AdvancedTelop "{int, int, int, int, int}"
typedef struct REC_AdvancedTelop {
	int joy_x;
	int joy_y;
	int slider1;
	int slider2;
	int joy_buttons;
} REC_AdvancedTelop;

extern char *MSG_GetCentralServer;
#define FRM_GetCentralServer "{string}"
typedef struct REC_GetCentralServer {
  char *url;
} REC_GetCentralServer;

#endif

/**********************************************************************
# $Log: hserver_ipt.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.3  2004/04/17 12:20:35  endo
# Intalling JBox Sensor Update.
#
# Revision 1.2  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.4  2003/04/02 22:18:22  zkira
# Added BatteryUpdate message
#
# Revision 1.3  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
