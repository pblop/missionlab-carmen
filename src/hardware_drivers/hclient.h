/**********************************************************************
 **                                                                  **
 **                              hclient.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  HServer external references for robot_side_com.c                **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: hclient.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef HCLIENT_H
#define HCLIENT_H

#include "hserver/hserver.h"

//Definicion del numero maximo de sensores de la carretilla.
const int MAX_SENSORS = 22;

// Stop robot motion
int hclient_stop(void);

// Start sonar
int hclient_range_start(void);

// Stop sonar
int hclient_range_stop(void);

// Set robot speed
int hclient_drive(float speed); // meter per second

// Set robot angular speed
int hclient_steer(float avel); // degrees per second

// Set robot location
int hclient_setxyt(float x,float y,float theta); // meters, meters, degrees

// Close robot connection
int hclient_close_robot(void);

// Open robot connection. Name is the hserver name
int hclient_open_robot(char *name,bool multipleHservers);

// Steer toward theta and when within drive_wait_angle start to drive at speed
int hclient_steer_toward_and_drive(float speed,float theta,float drive_wait_angle = 15.0,int use_reverse = true);

// Return robot location
int hclient_getxyt(double *x,double *y,double *t);

// Return battery reading
int hclient_get_battery(int *level);

// Return Jbox reading
int hclient_get_jbox_info(info_jbox_list_t *jboxInfoList);

// Return GPS
int hclient_get_gps(double *lat,double *lon,double *dir);

// Pan/tilt camera to location
int hclient_move_camera( int pandegree, int tiltdegree );

// Set camera tracking mode.
int hclient_set_camera_tracker_mode(int mode);

// Return side ranges from laser. Used for door trigger
int hclient_get_side_ranges(float *left,float *right);

// Return new sensor readings from hserver
int hclient_get_sensor_readings(int mask,int *num,sensor_t **sensorPtr);

// Return obstacles from latest sensor reading
int hclient_get_obstacles(int mask,float danger_range,sensor_reading_t **reading,bool adjust = false);

// Return array of latest laser reading
int hclient_laser_read(float *reading);

// Update sensor readings from hserver
int hclient_update_sensors();

// Capture fram and save to file
int hclient_frame_upload(int channel,char *name);

// Initialize robot, and set its current position to be the specified value
int hclient_init_robot(double x, double y, double heading);

//Devuelve la altura a la que se encuentra la pinza
int hclient_get_pinza(float * altura);

//Indica la altura a la que se debe situar la pinza.
int hclient_set_pinza(float pos);

//Activa o desactivo los motores de aspirar.
int hclient_motores_aspirar(bool power);

void hclient_send_advanced_telop(int joy_x, int joy_y, int slider1, int slider2, int joy_buttons);
const char *hclient_get_central_server();

#endif

/**********************************************************************
# $Log: hclient.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/06 00:55:59  endo
# PoseCalculator from MARS 2020 migrated
#
# Revision 1.4  2004/09/10 19:30:57  endo
# New PoseCalc integrated.
#
# Revision 1.3  2004/04/17 12:21:47  endo
# Intalling JBox Sensor Update.
#
# Revision 1.2  2004/04/13 19:36:53  yangchen
# JBox feature added.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.5  2003/04/02 22:16:30  zkira
# GPS and battery information for telop GUI
#
# Revision 1.4  2000/09/19 03:52:33  endo
# Sonar bug fixed.
#
#
#**********************************************************************/
