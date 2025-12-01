/**********************************************************************
 **                                                                  **
 **                             message.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Message dialog for HServer                                      **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: message.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <curses.h>
#include <panel.h>

using std::string;

extern WINDOW* msgWindow;
extern PANEL* msgPanel;

// Window parameters
#define MESSAGE_WIN_COL                   40
#define MESSAGE_WIN_ROW                   23
#define MESSAGE_WIN_X                     35
#define MESSAGE_WIN_Y                     9

// Message types
enum EnMessageType
{
    EnMessageType_NONE,
    EnMessageType_INTRO,
    EnMessageType_CAMERA,
    EnMessageType_LASER,
    EnMessageType_REMOTE,
    EnMessageType_ROBOT,
    EnMessageType_PIONEER2,
    EnMessageType_PIONEER2_PORT,
    EnMessageType_PIONEER,
    EnMessageType_PIONEER_PORT,
    EnMessageType_AMIGOBOT,
    EnMessageType_AMIGOBOT_PORT,
    EnMessageType_ROBOTLASER,
    EnMessageType_ROBOTLASER_PORT,
	EnMessageType_ROOMBA,
    EnMessageType_ROOMBA_PORT,
	EnMessageType_ROOMBA560,
    EnMessageType_ROOMBA560_PORT,
    EnMessageType_CARMEN_PORT,
    EnMessageType_WEBCAM_PORT,
    EnMessageType_WEBCAM_CONTROL,
    EnMessageType_TERMINAL,
    EnMessageType_TERMINAL_BAUD,
    EnMessageType_COG,
    EnMessageType_COG_PORT,
    EnMessageType_COG_CONTROL,
    EnMessageType_URBAN_CONTROL,
    EnMessageType_CAMERA_CONTROL,
    EnMessageType_TRACKING,
    EnMessageType_FRAMEGRABBER_PORT,
    EnMessageType_VIDEO_CONTROL,
    EnMessageType_USAGE_CONTROL,
    EnMessageType_NOMAD_CONTROL,
    EnMessageType_NOMAD,
    EnMessageType_NOMAD_150_PORT,
    EnMessageType_NOMAD_200_HOST_NAME,
    EnMessageType_FRED_CONTROL,
    EnMessageType_CARRETILLA_CONTROL,
    EnMessageType_CARRETILLA_SIMULACION_CONTROL,
    EnMessageType_CARRETILLA_GAZEBO_CONTROL,
    EnMessageType_LASER_CONTROL,
    EnMessageType_MAZE_CONTROL,
    EnMessageType_CONSOLE_CONTROL,
    EnMessageType_MAZE_SIZE_SELECT,
    EnMessageType_MAZE_TYPE_SELECT,
    EnMessageType_MAZE_RES_SELECT,
    EnMessageType_ROBOT_TYPE,
    EnMessageType_COMPASS_TYPE,
    EnMessageType_GYRO_TYPE,
    EnMessageType_GPS_CONTROL,
    EnMessageType_GPS_SELECT,
    EnMessageType_JBOX_ID,
    EnMessageType_JBOX_CONTROL,
    EnMessageType_JBOX_SELECT,
    EnMessageType_JBOX_DISPLAY_NSS,
    EnMessageType_JBOX_DISPLAY_POS,
    EnMessageType_JBOX_DISPLAY_GPS,
    EnMessageType_JBOX_DISPLAY_HOPS,
    EnMessageType_MAZEDISPLAY_CONTROL,
    EnMessageType_SOUND_SELECT,
    EnMessageType_SOUND_CONTROL,
    EnMessageType_GPS_PORT,
    EnMessageType_X_CONTROL,
    EnMessageType_LASER_TYPE,
    EnMessageType_LASER_USER_ACTION,
    EnMessageType_LASER_ANGLE_OFFSET,
    EnMessageType_LASER_X_OFFSET,
    EnMessageType_LASER_Y_OFFSET,
    EnMessageType_ENTER_A_LASER_NAME,
    EnMessageType_CHOOSE_A_LASER_NAME,
    EnMessageType_CAMERA_SWEEP_X,
    EnMessageType_CAMERA_SWEEP_Y,
    EnMessageType_ATRVJR_CONTROL,
    EnMessageType_GYRO_DMU_VGX_CONTROL,
    EnMessageType_GYRO_CAN_CONTROL,
    EnMessageType_COMPASS_KVH_C100_CONTROL,
    EnMessageType_COMPASS_3DMG_CONTROL,
    EnMessageType_COMPASS_3DMG_PORT,
    EnMessageType_COMPASS_CAN_CONTROL,
    EnMessageType_POSECALC_CONTROL,
    EnMessageType_POSECALC_FUSER_TYPE,
    EnMessageType_POSECALC_VARIANCE_SCHEME
};

// Message error messages
enum EnMessageErrType
{
    EnMessageErrType_NONE,
    EnMessageErrType_PORT,
    EnMessageErrType_BAUD,
    EnMessageErrType_COG,
    EnMessageErrType_NOMAD_TYPE,
    EnMessageErrType_TRACKING,
    EnMessageErrType_MAZE_SIZE,
    EnMessageErrType_MAZE_TYPE,
    EnMessageErrType_MAZE_RES,
    EnMessageErrType_ROBOT_TYPE,
    EnMessageErrType_COMPASS_TYPE,
    EnMessageErrType_GYRO_TYPE,
    EnMessageErrType_GPS,
    EnMessageErrType_JBOX_NOTFOUND,
    EnMessageErrType_JBOX_NOTCONNECT,
    EnMessageErrType_LASER_TYPE,
    EnMessageErrType_GENERAL_INVALID,
    EnMessageErrType_INVALID_NUMBER_FORMAT,
    EnMessageErrType_CAMERA_SWEEP_X,
    EnMessageErrType_CAMERA_SWEEP_Y,
    EnMessageErrType_POSECALC_FUSER_TYPE,
    EnMessageErrType_POSECALC_VARIANCE_SCHEME
};

// Input filters
enum EnMessageFilterType
{
    EnMessageFilterType_DIGIT,
    EnMessageFilterType_BAUD,
    EnMessageFilterType_COG,
    EnMessageFilterType_TRACKING,
    EnMessageFilterType_NOMAD,
    EnMessageFilterType_MAZE_SIZE,
    EnMessageFilterType_MAZE_TYPE,
    EnMessageFilterType_MAZE_RES,
    EnMessageFilterType_ROBOT_TYPE,
    EnMessageFilterType_COMPASS_TYPE,
    EnMessageFilterType_GYRO_TYPE,
    EnMessageFilterType_GPS,

    // In the first menu for JBOX, only allow input as 'q','x' or 'd'
    EnMessageFilterType_JBOX,

    EnMessageFilterType_SOUND,
    EnMessageFilterType_LASER_TYPE,
    EnMessageFilterType_LASER_USER_ACTION,
    EnMessageFilterType_LASER_NAME,
    EnMessageFilterType_CAMERA_SWEEP_X,
    EnMessageFilterType_CAMERA_SWEEP_Y
};

extern PANEL* msgPanel;

extern int messagePanelX, messagePanelY;

void messageDrawWindow( EnMessageType msg, EnMessageErrType err, void* data = NULL );
char messageGetChar( EnMessageFilterType filter, EnMessageType msg, EnMessageErrType err,
                    void* data = NULL, bool hide = true, void* pvFilterData = NULL);
string messageGetString( EnMessageType enMsg, unsigned int uEchoLine,
                         void* pvData = NULL, bool bHide = true );
void messageInitialize();
void messageHide();
void messageShow();
void messageMovePanelKey( int c );
void messageMove();

#endif

/**********************************************************************
# $Log: message.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.5  2004/09/10 19:43:27  endo
# New PoseCalc integrated.
#
# Revision 1.4  2004/04/24 11:20:53  endo
# 3DM-G added.
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
# Revision 1.10  2003/04/26 06:48:31  ebeowulf
# Added the Pioneer2-dxe controller.
#
# Revision 1.9  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.8  2002/02/18 13:48:21  endo
# AmigoBot added.
#
# Revision 1.7  2001/05/29 22:37:40  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.6  2001/03/23 21:26:20  blee
# altered to use a config file and allow for a laser's physical
# offset on a robot
#
# Revision 1.5  2000/12/12 22:59:27  blee
# Changed #defines for the user interface to enumerated types.
# Added prototype for messageGetString().
# Changed prototype for messageGetChar().
#
# Revision 1.4  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.3  2000/09/19 03:45:31  endo
# RCS log added.
#
#
#**********************************************************************/
