/**********************************************************************
 **                                                                  **
 **                              ipt_names.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  HServer IPT message names                                       **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ipt_names.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include "hserver_ipt.h"

char *MSG_ModuleId = "ModuleId";
char *MSG_InitConnection = "InitConnection";
char *TYP_SensorReadingT = "SensorReadingT" ;
char *TYP_RobotPartT = "RobotPartT";
char *MSG_RobotSetupInfo = "RobotSetupInfo";
char *MSG_SensorUpdate = "SensorUpdate";
char *MSG_BatteryUpdate = "BatteryUpdate";
char *MSG_SetPinza = "SetPinza";
char *MSG_GetPinza = "GetPinza";
char *MSG_MotoresAspirar = "MotoresAspirar";
char *MSG_AdvancedTelop = "AdvancedTelop";
char *MSG_GetCentralServer = "GetCentralServer";

char *TYP_JboxReadingT = "JboxReadingT";
char *MSG_JboxUpdate = "JboxUpdate";

char *MSG_SimpleCommand = "SimpleCommand";
char *MSG_DriveCommand = "DriveCommand";
char *MSG_SteerCommand = "SteerCommand";
char *MSG_SteerTowardDriveCommand = "SteerTowardDriveCommand";
char *MSG_SetXYT = "SetXYT";
char *MSG_InitRobot = "InitRobot";
char *MSG_MoveCamera = "MoveCamera";
char *MSG_SetCameraTrackerMode = "SetCameraTrackerMode";
char *MSG_FrameUpload = "FrameUpload";
char *MSG_VideoFrame = "VideoFrame";
char *MSG_GpsRTCM = "GpsRTCM";
char *MSG_LaserScan = "LaserScan";

IPFormatSpec iptHclientFormatArray[] = {
    { TYP_SensorReadingT, FRM_SensorReadingT },
    { TYP_RobotPartT, FRM_RobotPartT},
    { TYP_JboxReadingT, FRM_JboxReadingT },
    { NULL, NULL}
};

IPMessageSpec iptHclientMessageArray[] = {
    { MSG_InitConnection,            FRM_InitConnection },
    { MSG_RobotSetupInfo,            FRM_RobotSetupInfo },
    { MSG_SensorUpdate,              FRM_SensorUpdate },
    { MSG_BatteryUpdate,             FRM_BatteryUpdate },
    { MSG_SimpleCommand,             FRM_SimpleCommand },
    { MSG_DriveCommand,              FRM_DriveCommand },
    { MSG_SteerCommand,              FRM_SteerCommand },
    { MSG_SteerTowardDriveCommand,   FRM_SteerTowardDriveCommand },
    { MSG_SetXYT,                    FRM_SetXYT },
    { MSG_InitRobot,                 FRM_InitRobot },
    { MSG_MoveCamera,                FRM_MoveCamera},
    { MSG_SetCameraTrackerMode,      FRM_SetCameraTrackerMode},
    { MSG_FrameUpload,               FRM_FrameUpload},
    { MSG_JboxUpdate,                FRM_JboxUpdate },
    { MSG_VideoFrame,                FRM_VideoFrame},
	{ MSG_SetPinza,                  FRM_SetPinza },
	{ MSG_GetPinza,                  FRM_GetPinza },
	{ MSG_MotoresAspirar,			 FRM_MotoresAspirar },
	{ MSG_AdvancedTelop,			 FRM_AdvancedTelop },
	{ MSG_GetCentralServer,          FRM_GetCentralServer },
    { NULL, NULL}
};

IPMessageSpec iptGpsMessageArray[] = {
    { MSG_GpsRTCM, FRM_GpsRTCM },
    { NULL, NULL}
};

IPMessageSpec iptHandlerMessageArray[] = {
    { MSG_ModuleId, FRM_ModuleId },
    { NULL, NULL}
};

IPMessageSpec iptLaserMessageArray[] = {
    { MSG_LaserScan,  FRM_LaserScan},
    { NULL, NULL}
};

/**********************************************************************
# $Log: ipt_names.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:41:06  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2003/04/02 22:30:38  zkira
# Added battery update messages
#
# Revision 1.2  2000/09/19 03:14:21  endo
# RCS log added.
#
#
#**********************************************************************/
