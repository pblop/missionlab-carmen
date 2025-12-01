/**********************************************************************
 **                                                                  **
 **                            console.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Communication interface to robot-executables                    **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: console.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <math.h>

#include "PoseCalculatorInterface.h"
#include "hserver.h"
#include "camera.h"
#include "statusbar.h"
#include "robot_config.h"
#include "sensors.h"
#include "message.h"
#include "robot.h"
#include "ipt_handler.h"
#include "video.h"

#include "ipt/connection.h"
#include "hserver_ipt.h"
#include "ipc_client.h"
#include "jbox.h"

#include "roomba.h"
#include "ipc_client.h"

#define CONSOLE_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}

bool consoleConnected,useConsole;

bool gDisableConsoleRobotControl = false;

pthread_t console_reader;

IPConnection* console;

char consoleRstr[100];

IPCommunicator* communicator;

//int console_total_recieve = 0;

char hclient_ipt_name[100];
HSPose_t initialPose;

// Query for robot setup
void consoleReturnRobotSetup(IPCommunicator* comm, IPMessage* msg)
{
    REC_RobotSetupInfo rec;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    robot_config->fillIptMessage(&rec);

    comm->Reply(msg, MSG_RobotSetupInfo, &rec);
    iptHandler->saveNumBytesSent(sizeof(REC_RobotSetupInfo));

    if (rec.num_parts > 0)
    {
        delete [] rec.part;
    }
}

// Query for robot battery readings (if available)
void consoleReturnBatteryReadings(IPCommunicator* comm, IPMessage* msg)
{
    REC_BatteryUpdate rec;

    if (gRobot != NULL)
    {
      rec.battery_level = (int)(gRobot->getBattery());
    }
    else
    {
      rec.battery_level = -2;
    }

    comm->Reply(msg, MSG_BatteryUpdate, &rec);
    iptHandler->saveNumBytesSent(sizeof(REC_BatteryUpdate));
}

// Query for jbox readings (if available)
void consoleReturnJboxReadings(IPCommunicator* comm, IPMessage* msg)
{
    int i;
    REC_JboxUpdate rec;
    JBoxDataList_t Jbox_data_list;

    if (jbox != NULL)
    {
        Jbox_data_list = jbox->getJBoxDataList();

        rec.neighbor_num = Jbox_data_list.size();
        rec.jbox_readingt = (REC_JboxReadingT*)malloc(sizeof(REC_JboxReadingT)*rec.neighbor_num);

        for(i = 0; i < rec.neighbor_num; i++)
        {
            rec.jbox_readingt[i].id = Jbox_data_list[i].ID;
            rec.jbox_readingt[i].num_hops = Jbox_data_list[i].num_hops;
            rec.jbox_readingt[i].signal_strength = Jbox_data_list[i].signal_strength;
            rec.jbox_readingt[i].num_readings = GPS_NUM_ELEMENTS;
            rec.jbox_readingt[i].gps_reading = (double*)malloc(sizeof(double)*GPS_NUM_ELEMENTS);
            memcpy(
                rec.jbox_readingt[i].gps_reading,
                Jbox_data_list[i].gps_reading,
                sizeof(double)*GPS_NUM_ELEMENTS);
        }

        comm->Reply(msg, MSG_JboxUpdate, &rec);
        iptHandler->saveNumBytesSent(
            sizeof(REC_JboxUpdate) +
            ((sizeof(REC_JboxReadingT)+(sizeof(double)*GPS_NUM_ELEMENTS))*rec.neighbor_num));

        // clean up the copies
        for (i = 0; i < rec.neighbor_num; i++)
        {
            if (rec.jbox_readingt[i].gps_reading != NULL)
            {
                free(rec.jbox_readingt[i].gps_reading);
            }
        }
        free(rec.jbox_readingt);
    }
    else
    {
        rec.neighbor_num = 0;
        rec.jbox_readingt = NULL;

        comm->Reply(msg, MSG_JboxUpdate, &rec);
        iptHandler->saveNumBytesSent(sizeof(REC_JboxUpdate));
    }
}

// Query for robot sensor readings
void consoleReturnSensorReadings(IPCommunicator* comm, IPMessage* msg)
{

    int i;
    REC_SensorUpdate rec;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    rec.num_sensors = sensors->numSensors();
    rec.sensor = (REC_SensorReadingT*) malloc(sizeof(REC_SensorReadingT) * rec.num_sensors);
    for(i = 0; i < rec.num_sensors; i++)
    {
        rec.sensor[i].type = sensors->latest_sensor[i]->getType();
        rec.sensor[i].len_extra = sensors->latest_sensor[i]->getLenExtra();
        rec.sensor[i].extra = sensors->latest_sensor[i]->getExtra();
        sensors->latest_sensor[i]->getAngleInfo(rec.sensor[i].angle, rec.sensor[i].num_angles);
        sensors->latest_sensor[i]->getReadingInfo(rec.sensor[i].reading, rec.sensor[i].num_readings);

        // if space for angles or readings couldn't be allocated, set their counts to 0
        if (rec.sensor[i].angle == NULL)
        {
            rec.sensor[i].num_angles = 0;
        }
        if (rec.sensor[i].reading == NULL)
        {
            rec.sensor[i].num_readings = 0;
        }
    }

    comm->Reply(msg, MSG_SensorUpdate, &rec);
    iptHandler->saveNumBytesSent(
        sizeof(REC_SensorUpdate)+(sizeof(REC_SensorReadingT)*rec.num_sensors));

    // clean up the copies
    for (i = 0; i < rec.num_sensors; i++)
    {
        if (rec.sensor[i].angle != NULL)
        {
            delete [] rec.sensor[i].angle;
        }
        if (rec.sensor[i].reading != NULL)
        {
            delete [] rec.sensor[i].reading;
        }
    }
    free(rec.sensor);
}

void consoleRangeStop()
{
    if (gRobot != NULL)
    {
        gRobot->sonarStop();
    }
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Console: range stop");
    }
}

void consoleRangeStart()
{
    if (gRobot != NULL)
    {
        gRobot->sonarStart();
    }
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Console: range start");
    }
}

void consoleCloseRobot()
{
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Console: close robot");
    }
}

void consoleStopRobot()
{
    if (!gDisableConsoleRobotControl)
    {
        if (gRobot != NULL)
        {
            gRobot->stop();
        }
    }
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printTextWindow("Console: stop robot");
    }
}

void consoleDriveCommand(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DriveCommand *rec = NULL;
    double driveSpeed;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    if ((gDisableConsoleRobotControl) ||
        (gRobot == NULL) ||
        (gPoseCalc == NULL))
    {
        return;
    }

    rec = (REC_DriveCommand*) msg->FormattedData();

    driveSpeed = rec->vel;
    gRobot->setDesiredDriveSpeed(driveSpeed);

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: drive command %f", rec->vel);
    }
}

void consoleSteerCommand(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SteerCommand *rec = NULL;
    double steerSpeed;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    if ((gDisableConsoleRobotControl) ||
        (gRobot == NULL) ||
        (gPoseCalc == NULL))
    {
        return;
    }

    rec = (REC_SteerCommand*) msg->FormattedData();


    steerSpeed = rec->avel;
    gRobot->setDesiredSteerSpeed(steerSpeed);

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: steer command %f", rec->avel);
    }
}

void consoleSteerTowardDriveCommand(IPCommunicator *comm, IPMessage *msg, void *hndData)
{
    REC_SteerTowardDriveCommand* rec = NULL;
    HSRotation_t rot;
    double driveSpeed;
    double minPointTurnAngle;
    bool reversable = false;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    if ((gDisableConsoleRobotControl) ||
        (gRobot == NULL) ||
        (gPoseCalc == NULL))
    {
        return;
    }

    rec = (REC_SteerTowardDriveCommand*) msg->FormattedData();

	/*printfTextWindow(
            "Console: steer toward drive command %f %f %d\n",
            rec->theta,
            rec->speed,
            rec->use_reverse);*/

    reversable = rec->use_reverse;
    minPointTurnAngle = rec->drive_wait_angle;
    driveSpeed = rec->speed;
    gPoseCalc->getRotation(rot);
    CONSOLE_CRAMPDEG(rec->theta, -180.0, 180.0);
    rot.yaw += rec->theta;
    CONSOLE_CRAMPDEG(rot.yaw, 0.0, 360.0);
    rot.extra.time = getCurrentEpochTime();

    gRobot->setReversableDrive(reversable);
    gRobot->setMinimumPointTurnAngle(minPointTurnAngle);
    gRobot->setDesiredDriveSpeed(driveSpeed);
    gRobot->setDesiredRotation(rot);

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow(
            "Console: steer toward drive command %f %f %d",
            rec->theta,
            rec->speed,
            rec->use_reverse);
    }
}


void consoleFrameUpload(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_FrameUpload* rec = (REC_FrameUpload*) msg->FormattedData();
    REC_SimpleCommand reply;

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {

        printfTextWindow("Console: frame upload name: %s  channel: %d ",
                          rec->name, rec->channel);
    }

    if (video == NULL)
    {
        reply.command = -1;
        comm->Reply(msg, MSG_SimpleCommand, &reply);
        iptHandler->saveNumBytesSent(sizeof(REC_SimpleCommand));
    }

    video->frameUpload(rec->channel, rec->name, rec->send, comm, msg);
    if (!rec->send)
    {
        reply.command = 0;
        comm->Reply(msg, MSG_SimpleCommand, &reply);
        iptHandler->saveNumBytesSent(sizeof(REC_SimpleCommand));
    }
}

void consoleSetXYT(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SetXYT* rec;
    HSPose_t pose;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    rec = (REC_SetXYT*) msg->FormattedData();

    if (gPoseCalc != NULL)
    {
        memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));

        pose.loc.x = rec->x;
        pose.loc.y = rec->y;
        pose.rot.yaw = rec->t;
        pose.extra.time = getCurrentEpochTime();

        gPoseCalc->setPose(pose);
    }

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: Set xyt command x: %f y:%f t: %f", rec->x, rec->y, rec->t);
    }
}

void consoleSetPinzaAltura(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SetPinza *rec = NULL;
    float altura;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    if ((gDisableConsoleRobotControl) ||
        (gRobot == NULL) ||
        (gPoseCalc == NULL))
    {
        return;
    }

    rec = (REC_SetPinza*) msg->FormattedData();

    altura = rec->altura;
    //gRobot->setDesiredDriveSpeed(driveSpeed);

	printf("\n\nSet Pinza altura : %f\n\n", altura);

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: set pinza command %f", rec->altura);
    }
}


void consoleGetPinzaAltura(IPCommunicator* comm, IPMessage* msg)
{
    REC_GetPinza rec;

    if (gRobot != NULL)
    {
      //rec.altura = (float)(gRobot->getPinza());
		rec.altura = 1.0;
    }
    else
    {
      rec.altura = -2;
    }
	printf("Get Pinza altura : %f", rec.altura);
    comm->Reply(msg, MSG_GetPinza, &rec);
    iptHandler->saveNumBytesSent(sizeof(REC_GetPinza));
}


void consoleMotoresAspirar(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_MotoresAspirar *rec = NULL;
    char etiqueta[30];


    iptHandler->saveNumBytesReceived(msg->SizeData());

    if ((gDisableConsoleRobotControl) ||
        (gRobot == NULL) ||
        (gPoseCalc == NULL))
    {
        return;
    }

    rec = (REC_MotoresAspirar*) msg->FormattedData();


	if(rec->power == 1){
		if (gRobot->getAspirar() == true)
			((Roomba *)gRobot)->setMotores(1);
	}
	else{
		if (gRobot->getAspirar() == true)
			((Roomba *)gRobot)->setMotores(0);
	}

    sprintf(etiqueta,"%s motores", (rec->power == 1) ? "Activar" : "Desactivar");
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: %s\n", etiqueta);
    }
}

void consoleAdvancedTelop(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
	REC_AdvancedTelop *rec = NULL;

	iptHandler->saveNumBytesReceived(msg->SizeData());

	if(gRobot==NULL)
		return;

	rec = (REC_AdvancedTelop*) msg->FormattedData();
	gRobot->advancedTelop(rec);
}

void consoleGetCentralServer(IPCommunicator* comm, IPMessage* msg)
{
    REC_GetCentralServer rec;
    rec.url = (char *)ipc_preferred_server_name();
    comm->Reply(msg, MSG_GetCentralServer, &rec);
    iptHandler->saveNumBytesSent(sizeof(REC_GetCentralServer));
}

void consoleInitRobot(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_InitRobot *rec = NULL;
    REC_SimpleCommand reply;
    HSPose_t pose;
    double curTime;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    rec = (REC_InitRobot *) msg->FormattedData();

    // First, initialize the robot.
    if (gRobot != NULL)
    {
        gRobot->initRobot();
    }

    // Tell pose calculator to initialize the pose.
    if (gPoseCalc != NULL)
    {
        curTime = getCurrentEpochTime();

        memset((HSPose_t *)&pose, 0x0, sizeof(HSPose_t));
        pose.loc.x = rec->x;
        pose.loc.y = rec->y;
        pose.rot.yaw = rec->heading;
        pose.extra.time = curTime;
        pose.loc.extra.time = curTime;
        pose.rot.extra.time = curTime;
		initialPose = pose;

		gPoseCalc->initPose(pose);
    }

    // Send the acknowledgment to the client.
    reply.command = 0;
    comm->Reply(msg, MSG_SimpleCommand, &reply);
    iptHandler->saveNumBytesSent(sizeof(REC_SimpleCommand));
	//printfTextWindow("Console: Init Robot command x: %f y:%f heading: %f\n", rec->x, rec->y, rec->heading);
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: Init Robot command x: %f y:%f heading: %f", rec->x, rec->y, rec->heading);
    }
}

void consoleMoveCamera(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    iptHandler->saveNumBytesReceived(msg->SizeData());

    REC_MoveCamera *rec = (REC_MoveCamera*) msg->FormattedData();
    if (camera!=NULL)
    {
        camera->moveDeg(rec->pan, rec->tilt);
    }
}

void consoleSetCameraTrackerMode(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    iptHandler->saveNumBytesReceived(msg->SizeData());

    REC_SetCameraTrackerMode* rec = (REC_SetCameraTrackerMode*) msg->FormattedData();
    if (camera != NULL)
    {
        camera->setTrackerMode(rec->mode);
    }

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: Set camera tracker mode %d", rec->mode);
    }
}

char* consoleSimpleName(int command)
{
    switch(command)
    {
    case HCLIENT_GET_ROBOT_SETUP_CMD:
        return "HCLIENT_GET_ROBOT_SETUP_CMD";
        break;

    case HCLIENT_RANGE_STOP_CMD:
        return "HCLIENT_RANGE_STOP_CMD";
        break;

    case HCLIENT_RANGE_START_CMD:
        return "HCLIENT_RANGE_START_CMD";
        break;

    case HCLIENT_CLOSE_ROBOT_CMD:
        return "HCLIENT_CLOSE_ROBOT_CMD";
        break;

    case HCLIENT_STOP_CMD:
        return "HCLIENT_STOP_CMD";
        break;

    case HCLIENT_GET_SENSOR_READINGS_CMD:
        return "HCLIENT_GET_SENSOR_READINGS_CMD";
        break;

    case HCLIENT_GET_BATTERY_READINGS_CMD:         // trw zk
        return "HCLIENT_GET_BATTERY_READINGS_CMD";
        break;

    case HCLIENT_GET_JBOX_READINGS_CMD:
        return "HCLIENT_GET_JBOX_READINGS_CMD";
        break;

    default:
        return "unknown";
    }
}

void consoleHandleSimpleCommand(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SimpleCommand* rec = NULL;

    iptHandler->saveNumBytesReceived(msg->SizeData());

    rec = (REC_SimpleCommand*) msg->FormattedData();

    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Console: simple command %x: %s",
                          rec->command, consoleSimpleName(rec->command));
    }

    switch(rec->command) {

    case HCLIENT_GET_ROBOT_SETUP_CMD:
        consoleReturnRobotSetup(comm, msg);
        break;

    case HCLIENT_RANGE_STOP_CMD:
        consoleRangeStop();
        break;

    case HCLIENT_RANGE_START_CMD:
        consoleRangeStart();
        break;

    case HCLIENT_CLOSE_ROBOT_CMD:
        consoleCloseRobot();
        break;

    case HCLIENT_STOP_CMD:
        consoleStopRobot();
        break;

    case HCLIENT_GET_SENSOR_READINGS_CMD:
        consoleReturnSensorReadings(comm, msg);
        break;

    case HCLIENT_GET_BATTERY_READINGS_CMD:
        consoleReturnBatteryReadings(comm, msg);
        break;

    case HCLIENT_GET_JBOX_READINGS_CMD:
        consoleReturnJboxReadings(comm, msg);
        break;

	case HCLIENT_GET_PINZA_ALTURA_CMD:
        consoleGetPinzaAltura(comm, msg);
        break;
/*
	case HCLIENT_MOTORES_ASPIRAR_CMD:
        consoleMotoresAspirar(comm, msg);
        break;*/

	case HCLIENT_GET_CENTRAL_SERVER_CMD:
        consoleGetCentralServer(comm, msg);
        break;

    default:
        if (iptHandler->reportLevel() >= HS_REP_LEV_ERR)
        {
            printfTextWindow("Unknown simple command %x", rec->command);
        }
        break;
    }
    //statusbarConsoleUpdate();
}

void consoleHandleInitConnection(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    iptHandler->saveNumBytesReceived(msg->SizeData());

    REC_InitConnection* rec = (REC_InitConnection*) msg->FormattedData();
    strcpy(hclient_ipt_name, rec->name);
    console = communicator->Connect(hclient_ipt_name, IPT_REQUIRED);
    if (iptHandler->reportLevel() >= HS_REP_LEV_DEBUG)
    {
        printfTextWindow("Initialized ipt connection with %s on %s",
                 rec->name, console->Host());
    }
}

void consoleRegisterHandlers()
{
    int num_handlers;
    IPMsgHandlerSpec hndArray[] = {
        { MSG_SimpleCommand ,          consoleHandleSimpleCommand,     IPT_HNDL_STD, NULL },
        { MSG_InitConnection,          consoleHandleInitConnection,    IPT_HNDL_STD, NULL },
        { MSG_DriveCommand,            consoleDriveCommand,            IPT_HNDL_STD, NULL },
        { MSG_SteerCommand,            consoleSteerCommand,            IPT_HNDL_STD, NULL },
        { MSG_SteerTowardDriveCommand, consoleSteerTowardDriveCommand, IPT_HNDL_STD, NULL },
        { MSG_SetXYT,                  consoleSetXYT,                  IPT_HNDL_STD, NULL },
        { MSG_InitRobot,               consoleInitRobot,               IPT_HNDL_STD, NULL },
        { MSG_MoveCamera,              consoleMoveCamera,              IPT_HNDL_STD, NULL },
        { MSG_SetCameraTrackerMode,    consoleSetCameraTrackerMode,    IPT_HNDL_STD, NULL },
        { MSG_FrameUpload,             consoleFrameUpload,             IPT_HNDL_STD, NULL },
		{ MSG_SetPinza,                consoleSetPinzaAltura,          IPT_HNDL_STD, NULL },
		{ MSG_MotoresAspirar,          consoleMotoresAspirar,          IPT_HNDL_STD, NULL },
		{ MSG_AdvancedTelop ,          consoleAdvancedTelop,           IPT_HNDL_STD, NULL }
    };
    num_handlers = sizeof(hndArray)/sizeof(IPMsgHandlerSpec);
    iptHandler->registerHandlers(num_handlers, hndArray);
}


void consoleDisableHandlers()
{
    int num_handlers;
    char* hndNames[] = {
        MSG_SimpleCommand,
        MSG_InitConnection,
        MSG_DriveCommand,
        MSG_SteerCommand,
        MSG_SteerTowardDriveCommand,
        MSG_SetXYT,
        MSG_MoveCamera,
        MSG_SetCameraTrackerMode,
		MSG_SetPinza,
		MSG_MotoresAspirar,
		MSG_AdvancedTelop
    };
    num_handlers = sizeof(hndNames) / sizeof(IPMessageType*);
    iptHandler->disableHandlers(num_handlers, hndNames);
}


void consoleConnect()
{
    communicator = iptHandler->communicator();
    iptHandler->registerNamedFormatters(iptHclientFormatArray);
    iptHandler->registerMessages(iptHclientMessageArray);
    consoleRegisterHandlers();
    iptHandler->set_state(IPT_MOD_CONSOLE, true, false);
    printTextWindow("Hclient listening connected");
}

void consoleDisconnect()
{
    consoleDisableHandlers();
}


/**********************************************************************
# $Log: console.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.4  2004/09/10 19:41:05  endo
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
# Revision 1.7  2003/04/02 22:29:21  zkira
# Added consoleGetBatteryReadings and battery message cases
#
# Revision 1.6  2001/05/29 22:37:50  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.5  2000/12/12 22:52:29  blee
# Changed consoleReturnSensorReadings() to copy sensor readings before
# writing them.
#
# Revision 1.4  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
