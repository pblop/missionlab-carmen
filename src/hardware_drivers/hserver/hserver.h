/**********************************************************************
 **                                                                  **
 **                             hserver.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Hardware Server                                                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: hserver.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef HSERVER_H
#define HSERVER_H

#include <curses.h>
#include <pthread.h>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "HServerTypes.h"

using std::vector;

#define HSERVER_VERSION "7.0.00"

#define SONAR_READING_OUTSIDE_RANGE 0

typedef unsigned char byte_t;
typedef unsigned short ushort;


// simple commands
#define HCLIENT_GET_ROBOT_SETUP_CMD           0x01
#define HCLIENT_RANGE_STOP_CMD                0x02
#define HCLIENT_RANGE_START_CMD               0x03
#define HCLIENT_CLOSE_ROBOT_CMD               0x05
#define HCLIENT_STOP_CMD                      0x06
#define HCLIENT_GET_SENSOR_READINGS_CMD       0x07
#define HCLIENT_GET_BATTERY_READINGS_CMD      0x08
#define HCLIENT_GET_JBOX_READINGS_CMD         0x09
#define HCLIENT_GET_PINZA_ALTURA_CMD		  0x10
#define HCLIENT_MOTORES_ASPIRAR_CMD		      0x11
#define HCLIENT_GET_CENTRAL_SERVER_CMD		  0x12

// sensors and masks
#define SENSOR_SICKLMS200_MASK      0x0001
#define SENSOR_SONAR_MASK           0x0002
#define SENSOR_IR_MASK              0x0004
#define SENSOR_XYT_MASK             0x0008
#define SENSOR_PANTILT_CAMERA_MASK  0x0010
#define SENSOR_COGNACHROME_MASK     0x0020
#define SENSOR_ARM_MASK             0x0040
#define SENSOR_VIDEO_MASK           0x0080
#define SENSOR_CAMERA_FOCUS_MASK    0x0100
#define SENSOR_COMPASS_MASK         0x0200
#define SENSOR_GPS_MASK             0x0400
#define SENSOR_ROOMBA_MASK          0x1000
#define MOTOR_ASPIRAR_MASK          0x2000
#define WEBCAM_MASK                 0x4000
#define SENSOR_GENERIC_MASK			0x8000

#define SENSOR_SICKLMS200           0
#define SENSOR_SONAR                1
#define SENSOR_IR                   2
#define SENSOR_XYT                  3
#define SENSOR_PANTILT_CAMERA       4
#define SENSOR_COGNACHROME          5
#define SENSOR_ARM                  6
#define SENSOR_VIDEO                7
#define SENSOR_CAMERA_FOCUS         8
#define SENSOR_COMPASS              9
#define SENSOR_GPS                  10
#define SENSOR_ROOMBA               12
#define MOTOR_ASPIRAR               13
#define WEBCAM                      14
#define SENSOR_GENERIC				15


// new sensors and masks
#define SENSOR_JBOX_MASK    0x0800
#define SENSOR_JBOX         11

#define SENSOR_DANGER_COLOR         3
#define SENSOR_SICKLMS200_COLOR     0


// robot setup and sensors
typedef struct
{
    int type;
    int color;
    float angle;
    float reading;
} sensor_reading_t;

typedef struct
{
    float angle;
    float fov_x;
    float fov_y;
} info_pantilt_camera_t;

typedef struct
{
    float angle;
    float fov;
    float range;
    int num_readings;
} info_laser_t;

typedef struct
{
    int number;
    float fov;
    float range;
    //float* angle;
} info_sonar_t;


typedef struct
{
    int number;
    float fov;
    float range;
} info_roomba_t;

typedef struct
{
    int nPixel_x;
    int nPixel_y;
} info_webCam_t;

typedef struct
{
    int num_sensor;
} info_sensor_generic_t;

typedef struct
{
    int number;
    float fov;
    float range;
    //float* angle;
} info_ir_t;

typedef struct
{
    int num_channels;
    int max_per_channel;
} info_cog_t;

typedef struct
{
    int num_channels;
} info_video_t;

typedef struct
{
    float scale;
} info_xyt_t;

typedef struct{
     int power;
}info_motor_aspirador;

typedef struct
{
    int num_per_channel[ 3 ];
} extra_sensor_cog_t;

typedef struct
{
    int bufnum;
} extra_laser_t;

typedef struct
{
    int type;
    int len_info;
    char* info;
} robot_part_t;

typedef struct
{
    char* name;
    int num_parts;
    robot_part_t* part;
} robot_setup_t;

typedef struct
{
    int type;
    int len_extra;
    char* extra;
    int num_angles;
    float* angle;
    int num_readings;
    float* reading;
} sensor_t;


typedef enum
{
    GPS_LATITUDE,
    GPS_LONGITUDE,
    GPS_ALTITUDE,
    GPS_HEADING,
    GPS_PITCH,
    GPS_ROLL,
    GPS_TIME,
    GPS_NUM_ELEMENTS // Keep this in the end of the list.

} gps_elements_t;

typedef struct
{
    int id;
    int numHops;
    int signalStrength;
    double gpsReading[GPS_NUM_ELEMENTS];
}  info_jbox_t;

typedef vector<info_jbox_t> info_jbox_list_t;

// window sizes and locations
#define HS_HELP_COL                  30
#define HS_HELP_ROW                  25
#define HS_HELP_X                    3
#define HS_HELP_Y                    2
#define HS_CONSOLE_PACK_NUMBER_COL   34
#define HS_CONSOLE_PING_RATE         100
#define HS_STATUS_X                  1
#define HS_STATUS_Y                  3

// report levels
#define HS_REP_LEV_NONE     0
#define HS_REP_LEV_ERR      1
#define HS_REP_LEV_NORMAL   2
#define HS_REP_LEV_DEBUG    3

#define HS_BUF_SIZE 1024

#define KEY_ESC 27

extern WINDOW* termBordWindow;
extern WINDOW* termWindow;
extern pthread_mutex_t screenMutex;

extern int mainPanelX, mainPanelY;
extern int screenX, screenY;

extern pthread_mutex_t screenMutex;

extern int statusbarDevideLine;

// PU Silent Window
extern bool SilentMode;


void printfTextWindow(const char* szFormat, ...);
void printTextWindow(char *str, int nl = true, int filter = false, int clear = false);
void printTerminalWindow(char *str, int x, int y);
int setupSerial(int* fd, const char* port, long baud, int report_errors, char* caller,
                 int timeout, int nonblock = false);
void refreshScreen(void);
void drawTermWindow(void);
void redrawWindows(void);
void resizeWindows(bool erase = true);
void flushSerialLine(int fd);
void exit_hserver(void);

double getCurrentEpochTime(void);
HSDateString_t getCurrentDateString(void);

#endif

/**********************************************************************
# $Log: hserver.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.5  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.4  2004/04/17 12:20:35  endo
# Intalling JBox Sensor Update.
#
# Revision 1.3  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.2  2004/04/06 02:07:40  pulam
# Added code for Silent Mode
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.21  2003/06/19 20:39:16  endo
# 6.0.01
#
# Revision 1.20  2003/04/06 13:41:02  endo
# 6.0.00
#
# Revision 1.19  2003/04/06 10:45:37  endo
# 5.0.09
#
# Revision 1.18  2003/04/03 23:07:43  endo
# 5.0.08
#
# Revision 1.17  2003/04/02 23:04:58  zkira
# Added HCLIENT_GET_BATTERY_READINGS
#
# Revision 1.16  2002/11/03 20:44:39  endo
# 5.0.07
#
# Revision 1.15  2002/09/26 14:37:46  endo
# 5.0.06
#
# Revision 1.14  2002/08/22 14:43:52  endo
# 5.0.05
#
# Revision 1.13  2002/07/18 17:18:18  endo
# 5.0.04
#
# Revision 1.12  2002/07/04 23:31:27  endo
# v5.0.03
#
# Revision 1.11  2002/04/01 22:03:23  endo
# 5.0.02
#
# Revision 1.10  2002/01/31 10:55:49  endo
# Version 5.0.01
#
# Revision 1.9  2002/01/16 21:48:10  ernest
# Added WatchDog functionality
#
# Revision 1.8  2001/05/29 22:36:35  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.7  2001/03/23 21:17:52  blee
# changed setupSerial prototype; made the char* a const char*
#
# Revision 1.6  2000/12/12 22:56:19  blee
# Added prototype for printfTextWindow().
#
# Revision 1.5  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.4  2000/10/16 20:03:53  endo
# steerToward() bug fixed. READING_OUTSIDE_RANGE added for the
# sonar reading.
#
# Revision 1.3  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
