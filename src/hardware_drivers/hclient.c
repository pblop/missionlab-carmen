/**********************************************************************
 **                                                                  **
 **                              hclient.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Hardware client. Connects to hserver                            **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: hclient.c,v 1.1.1.1 2006/07/12 13:37:55 endo Exp $ */

#include <stdio.h>
#include <termio.h>
#include <math.h>
#include <sys/file.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <linux/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ipt/ipt.h"
#include "ipt/message.h"

#include "hserver/hserver.h"
#include "hserver/hserver_ipt.h"
#include "hclient.h"
#include "robot_side_com.h"
#include "hserver/ipc_client.h"

const int NUM_GPS_READINGS = 7;

extern int     robot_id;

robot_setup_t *rs = NULL;

int sensor_setup_link[MAX_SENSORS];
int num_sensors;
sensor_t sensor[MAX_SENSORS];

char hclient_ipt_name[100];
char hserver_ipt_name[100];
#define carmen_ipt_name "carmen"

char *ipt_server_name;
char ipt_name[100];

IPCommunicator *hc_communicator;
IPConnection *hserver_connection;

/* Print robot setup to stdout */
int robotsetupPrintStdout(REC_RobotSetupInfo *rs){
    int i;
    info_pantilt_camera_t *info_pantilt;
    info_laser_t *info_laser;
    info_sonar_t *info_sonar;
    info_ir_t *info_ir;
    info_cog_t *info_cog;
    info_video_t *info_video;
    info_xyt_t *info_xyt;

	//Added by teny y alber
	info_sensor_generic_t *info_sensor_generic;

    printf("Robot Setup\n");
    printf("name: %s\n",rs->name);
    printf("number of parts: %d\n",rs->num_parts);
    for(i=0;i<rs->num_parts;i++){
        switch(rs->part[i].type){
        case SENSOR_PANTILT_CAMERA:
            info_pantilt = (info_pantilt_camera_t*)rs->part[i].info;
            printf("%d.  pan/tilt camera   angle: %.2f  fov x: %.2f y: %.2f\n",i,
                   info_pantilt->angle,info_pantilt->fov_x,info_pantilt->fov_y);
            break;
        case SENSOR_XYT:
            info_xyt = (info_xyt_t*)rs->part[i].info;
            printf("%d.  xyt  scale: %f\n",i,info_xyt->scale);
            break;
        case SENSOR_ARM:
            printf("%d.  arm\n",i);
            break;
        case SENSOR_SICKLMS200:
            info_laser = (info_laser_t*)rs->part[i].info;
            printf("%d.  laser  num readings: %d angle: %.2f  fov %.2f\n",i,
                   info_laser->num_readings,info_laser->angle,info_laser->fov);
            break;
        case SENSOR_SONAR:
            info_sonar = (info_sonar_t*)rs->part[i].info;
            printf("%d.  sonar  number: %d  fov: %.2f  range: %.2f\n",i,
                   info_sonar->number,info_sonar->fov,info_sonar->range);
            break;
        case SENSOR_IR:
            info_ir = (info_ir_t*)rs->part[i].info;
            printf("%d.  ir  number: %d  fov: %.2f  range: %.2f\n",i,
                   info_ir->number,info_ir->fov,info_ir->range);
            break;
        case SENSOR_COGNACHROME:
            info_cog = (info_cog_t*)rs->part[i].info;
            printf("%d.  cognachrome  num channels: %d  max per channel: %d\n",i,
                   info_cog->num_channels,info_cog->max_per_channel);
            break;
        case SENSOR_VIDEO:
            info_video = (info_video_t*)rs->part[i].info;
            printf("%d.  video  num channels: %d\n",i,info_video->num_channels);
            break;
        case SENSOR_COMPASS:
            printf("%d.  compass\n",i);
            break;
        case SENSOR_GPS:
            printf("%d.  gps\n",i);
            break;
		//Added by teny y alber
		case SENSOR_GENERIC:
			info_sensor_generic = (info_sensor_generic_t*)rs->part[i].info;
			 printf("%d.  generic sensor  number: %d\n",i,info_sensor_generic->num_sensor);
			break;
        default:
            printf("%d.  Unknown sensor type\n",i);
        }
    }
    return(0);
}

/* Send simple query to hserver */
int hclient_send_simple_query(int command){
    REC_SimpleCommand rec,*rtn;
    int rtn_cmd;

    rec.command = command;
    IPMessage *msg = hc_communicator->Query( hserver_connection, MSG_SimpleCommand,
                                             &rec, MSG_SimpleCommand );
    rtn = (REC_SimpleCommand*)msg->FormattedData();
    rtn_cmd = rtn->command;
    delete msg;
    return(rtn_cmd);
}

/* Send simple command to hserver */
void hclient_send_simple_command(int command){
    REC_SimpleCommand rec;

    rec.command = command;
    hc_communicator->SendMessage( hserver_connection, MSG_SimpleCommand, &rec );
}

int hclient_frame_upload(int channel,char *name){
    REC_FrameUpload rec;
    REC_SimpleCommand *rtn;
    REC_VideoFrame *vrtn;
    int rtn_cmd,i,j;
    FILE *ofile;
    rec.channel = channel;
    rec.name = name;
    rec.send = 1;
    if (!rec.send) {
        IPMessage *msg = hc_communicator->Query( hserver_connection, MSG_FrameUpload,
                                                 &rec, MSG_SimpleCommand);
        rtn = (REC_SimpleCommand*)msg->FormattedData();
        rtn_cmd = rtn->command;
        delete msg;
    } else {
        IPMessage *msg = hc_communicator->Query( hserver_connection, MSG_FrameUpload,
                                                 &rec, MSG_VideoFrame );
        vrtn = (REC_VideoFrame*)msg->FormattedData();
        printf("got video frame h %d w %d\n",vrtn->height,vrtn->width);
        ofile = fopen(name,"w");
        fprintf(ofile,"P6\n %d %d\n 255\n",vrtn->width,vrtn->height);
        fflush(ofile);

        for(i=0;i<vrtn->height;i++)
            for(j=0;j<vrtn->width;j++){
                fwrite(vrtn->frame + (i * vrtn->width + j)*3+2, 1, 1, ofile);
                fwrite(vrtn->frame + (i * vrtn->width + j)*3+1, 1, 1, ofile);
                fwrite(vrtn->frame + (i * vrtn->width + j)*3  , 1, 1, ofile);
            }
        fclose(ofile);
        delete msg;
    }
    return(rtn_cmd);
}

/* Retrieve robot setup from hserver */
void hclient_get_robot_setup(){
    REC_SimpleCommand rec;
    int i,type;
    info_cog_t *info_cog;
    info_laser_t *info_laser;
    info_sonar_t *info_sonar;
    info_ir_t *info_ir;

	//Added teny y alber
	info_sensor_generic_t *info_sensor_generic;

    if (rs) delete rs;
    rec.command = HCLIENT_GET_ROBOT_SETUP_CMD;
    IPMessage *msg = hc_communicator->Query( hserver_connection, MSG_SimpleCommand,
                                             &rec, MSG_RobotSetupInfo );
    REC_RobotSetupInfo *robot_setup = (REC_RobotSetupInfo*)msg->FormattedData(1);
    robotsetupPrintStdout(robot_setup);
    rs = (robot_setup_t*) robot_setup;
    for(i=0;i<rs->num_parts;i++){
        type = rs->part[i].type;
        sensor_setup_link[type] = i;
        if (sensor[type].type == -1) {
            sensor[type].type = type;
            sensor[type].len_extra = 0;
            sensor[type].num_angles = 0;
            sensor[type].num_readings = 0;

            switch(type){
            case SENSOR_SONAR:
                info_sonar = (info_sonar_t*)rs->part[i].info;
                sensor[type].num_readings = info_sonar->number;
                sensor[type].num_angles = info_sonar->number;
                sensor[type].len_extra = sizeof(float)*info_sonar->number*6;
                break;
            case SENSOR_IR:
                info_ir = (info_ir_t*)rs->part[i].info;
                sensor[type].num_readings = info_ir->number;
                sensor[type].num_angles = info_ir->number;
                break;
            case SENSOR_PANTILT_CAMERA:
                sensor[type].num_angles = 4;
                break;
            case SENSOR_SICKLMS200:
                info_laser = (info_laser_t*)rs->part[i].info;
                sensor[type].len_extra = sizeof(extra_laser_t);
                sensor[type].num_angles = info_laser->num_readings;
                sensor[type].num_readings = info_laser->num_readings;
                break;
            case SENSOR_XYT:
                sensor[type].num_readings = 3;
                break;
            case SENSOR_COGNACHROME:
                info_cog = (info_cog_t*)rs->part[i].info;
                sensor[type].len_extra = sizeof(extra_sensor_cog_t);
                sensor[type].num_angles = info_cog->num_channels*info_cog->max_per_channel;
                sensor[type].num_readings = info_cog->num_channels*info_cog->max_per_channel;
                break;
            case SENSOR_ARM:
                sensor[type].num_angles = 1;
                break;
            case SENSOR_GPS:
                sensor[type].num_readings = NUM_GPS_READINGS;
                break;
            case SENSOR_COMPASS:
                sensor[type].num_angles = 1;
                break;
			//Added by teny y alber
			case SENSOR_GENERIC:
				info_sensor_generic = (info_sensor_generic_t*)rs->part[i].info;
				sensor[type].num_readings = info_sensor_generic->num_sensor;
				break;
            default:
                fprintf(stderr,"Unknown robot setup sensor type: %d\n",type);
            }
            if (sensor[type].len_extra)
            {
                sensor[type].extra = (char*) malloc( sensor[type].len_extra );
            }
            if (sensor[type].num_angles)
            {
                sensor[type].angle = (float*)malloc( sizeof(float) * sensor[type].num_angles );
            }
            if (sensor[type].num_readings)
            {
                sensor[type].reading = (float*)malloc( sizeof(float) * sensor[type].num_readings );
            }
        } else fprintf(stderr,"***** error in get robot setup\n");
    }
    num_sensors = rs->num_parts;
    delete msg;
}

/* Initialize ipt connection with hserver */
int hclient_init_ipt_connection(void)
{
    REC_InitConnection rec;
    rec.name = hclient_ipt_name;
    if(!hc_communicator->SendMessage(hserver_connection,MSG_InitConnection,&rec))
    {
    	fprintf(stderr, "FATAL ERROR: CONNECTION TO HSERVER NAMED \"%s\" AT %s FAILED. USE -r <%s> OPTION IN HSERVER.\n", hserver_ipt_name, ipt_server_name, hserver_ipt_name);
        return -1;
    }
    return 0;
}

void hclientUpdateRobotSetup(IPCommunicator *comm, IPMessage *msg, void *hndData)
{
    fprintf(stderr,"*** !@# robot setup\n");
}

void hclientUpdateSensorsHandler(IPCommunicator *comm, IPMessage *msg, void *hndData)
{
    fprintf(stderr,"*** !@# sensors\n");
}

/* Setup ipt */
int hclientSetupIpt(void)
{
    unsigned int i;
    IPMessageType* msg_type;

    if (ipt_server_name == NULL){
        ipt_server_name = getenv("IPTHOST");
    }
    if (ipt_server_name == NULL)
        ipt_server_name = getenv("HOST");
    if (ipt_server_name == NULL){
        fprintf(stderr,"Unable to determine iptserver host from IPTHOST or HOST, using 127.0.0.1\n");
        ipt_server_name = "127.0.0.1";
    }

    hc_communicator = IPCommunicator::Instance(hclient_ipt_name,ipt_server_name);
    if(!hc_communicator)
    {
    	fprintf(stderr, "FATAL ERROR: CONNECTION TO HSERVER AT %s FAILED. CHECK IF HSERVER IS RUNNING AND CONNECTED TO %s\n", ipt_server_name);
    	return -1;
    }

    hserver_connection = hc_communicator->Connect(hserver_ipt_name,IPT_REQUIRED);
    if(!hserver_connection)
    {
    	fprintf(stderr, "FATAL ERROR: CONNECTION TO HSERVER NAMED \"%s\" AT %s FAILED. USE -r <%s> OPTION IN HSERVER.\n", hserver_ipt_name, ipt_server_name, hserver_ipt_name);
    	return -1;
    }

    hc_communicator->RegisterNamedFormatters(iptHclientFormatArray);
    hc_communicator->RegisterMessages(iptHclientMessageArray);

    IPMsgHandlerSpec hndArray[] = {
        { MSG_RobotSetupInfo, hclientUpdateRobotSetup, IPT_HNDL_ALL, NULL},
        { MSG_SensorUpdate, hclientUpdateSensorsHandler, IPT_HNDL_ALL,NULL}
    };

    for(i=0;i<(sizeof(hndArray)/sizeof(IPMsgHandlerSpec));i++) {
        msg_type = communicator->LookupMessage( hndArray[i].msg_name );
        communicator->RegisterHandler( msg_type, hndArray[i].callback,
                                       hndArray[i].data, hndArray[i].context );
    }

    return(0);
}


/* Return side ranges from laser. Used for door trigger */
int hclient_get_side_ranges(float *left,float *right){

    if (sensor[SENSOR_SICKLMS200].type != SENSOR_SICKLMS200) {
        fprintf(stderr,"No laser information\n");
        return(-1);
    }

    *right = sensor[SENSOR_SICKLMS200].reading[0];
    *left = sensor[SENSOR_SICKLMS200].reading[360];

    return(0);
}

/* Set robot speed */
int hclient_drive(float speed)  // m per sec
{
    REC_DriveCommand rec;

    rec.vel = speed;
    hc_communicator->SendMessage(hserver_connection,MSG_DriveCommand,&rec);

    return(0);
}

/* Set robot angular speed */
int hclient_steer(float avel)
{
    REC_SteerCommand rec;

    rec.avel = avel;
    hc_communicator->SendMessage(hserver_connection,MSG_SteerCommand,&rec);

    return(0);
}


/* Steer toward theta and when within drive_wait_angle start to drive at speed */
int hclient_steer_toward_and_drive(
    float speed,
    float theta,
    float drive_wait_angle,
    int use_reverse)
{
    REC_SteerTowardDriveCommand rec;

    rec.theta = theta;
    rec.speed = speed;
    rec.drive_wait_angle = drive_wait_angle;
    rec.use_reverse = use_reverse;
    hc_communicator->SendMessage(hserver_connection,MSG_SteerTowardDriveCommand,&rec);
    return(0);
}


/* Stop robot motion */
int hclient_stop(void)
{
    hclient_send_simple_command(HCLIENT_STOP_CMD);
    return(0);
}


/* Start sonar */
int hclient_range_start()
{
    hclient_send_simple_command(HCLIENT_RANGE_START_CMD);
    return(0);
}

/* Stop sonar */
int hclient_range_stop(void)
{
    hclient_send_simple_command(HCLIENT_RANGE_STOP_CMD);
    return(0);
}

/* Set robot location */
int hclient_setxyt(float x, float y, float theta){
    REC_SetXYT rec;

    rec.x = x;
    rec.y = y;
    rec.t = theta;
    hc_communicator->SendMessage(hserver_connection,MSG_SetXYT,&rec);

    return 0;
}


int hclient_move_camera( int pandegree, int tiltdegree ) {
    REC_MoveCamera rec;

    rec.pan = pandegree;
    rec.tilt = tiltdegree;
    hc_communicator->SendMessage(hserver_connection,MSG_MoveCamera,&rec);
    return 0;
}

/* Pan/tilt camera to location */
int hclient_set_camera_tracker_mode(int mode){
    static int last_mode = 0;

    if (mode == last_mode) return 0;
    last_mode = mode;

    REC_SetCameraTrackerMode rec;

    rec.mode = mode;
    hc_communicator->SendMessage(hserver_connection,MSG_SetCameraTrackerMode,&rec);
    return 0;
}

void hclientInitVariables(){
    int i;
    for (i=0;i<MAX_SENSORS;i++){
        sensor[i].type = -1;
        sensor_setup_link[i] = -1;
    }
}

/* Close robot connection */
int hclient_close_robot(void)
{
    hclient_send_simple_command(HCLIENT_CLOSE_ROBOT_CMD);
    return 0;
}

/* Return robot location */
int hclient_getxyt(double *x,double *y,double *t){
    if (sensor[SENSOR_XYT].type != SENSOR_XYT) {
        fprintf(stderr,"Hclient Error: no xyt\n");
        return(-1);
    }
    if (sensor[SENSOR_XYT].num_readings != 3) {
        fprintf(stderr,"Hclient Error: wrong number of xyt readings\n");
        return(-1);
    }
    *x = sensor[SENSOR_XYT].reading[0];
    *y = sensor[SENSOR_XYT].reading[1];
    *t = sensor[SENSOR_XYT].reading[2];

    return(0);
}

/* Return current battery readings */
int hclient_get_battery(int *level) {
    REC_SimpleCommand rec;
    REC_BatteryUpdate *current_battery;

    rec.command = HCLIENT_GET_BATTERY_READINGS_CMD;
    IPMessage *msg = hc_communicator->Query(hserver_connection,MSG_SimpleCommand,&rec,MSG_BatteryUpdate);
    current_battery = (REC_BatteryUpdate*)msg->FormattedData();

    *level = current_battery->battery_level;

    delete msg;
    return(0);
}

// Return current Jbox info
int hclient_get_jbox_info(info_jbox_list_t *jboxInfoList)
{
    info_jbox_t jboxInfo;
    REC_SimpleCommand rec;
    REC_JboxUpdate *current_jbox;
    int i, j, numNeighbors;
    const bool DUMP_READINGS = false;

    rec.command = HCLIENT_GET_JBOX_READINGS_CMD;
    IPMessage *msg = hc_communicator->Query(
        hserver_connection,
        MSG_SimpleCommand,
        &rec,
        MSG_JboxUpdate);

    current_jbox = (REC_JboxUpdate*)msg->FormattedData();

    numNeighbors = current_jbox->neighbor_num;

    for (i = 0; i < numNeighbors; i++)
    {
        jboxInfo.id = current_jbox->jbox_readingt[i].id;
        jboxInfo.numHops = current_jbox->jbox_readingt[i].num_hops;
        jboxInfo.signalStrength = current_jbox->jbox_readingt[i].signal_strength;

        memcpy(
            jboxInfo.gpsReading,
            current_jbox->jbox_readingt[i].gps_reading,
            sizeof(double)*GPS_NUM_ELEMENTS);

        jboxInfoList->push_back(jboxInfo);

        if (DUMP_READINGS)
        {
            printf(
                "\n%d. ID: %d, Hops: %d, Signal strength: %d\n",
                i,
                jboxInfo.id,
                jboxInfo.numHops,
                jboxInfo.signalStrength);

            printf("GPS:");
            for (j = 0; j < GPS_NUM_ELEMENTS; j++)
            {
                printf(" %f ", jboxInfo.gpsReading[j]);
            }
            printf("\n");
        }
    }

    delete msg;

    return (0);
}


/* Return current GPS position*/
int hclient_get_gps(double *lat,double *lon,double *dir){

    // Get data from sensor array, make sure it's valid
    if (sensor[SENSOR_GPS].type != SENSOR_GPS) {

        *lat = 0.0;
        *lon = 0.0;
        *dir = 0.0;
        return(0);
    }
    if (sensor[SENSOR_GPS].num_readings != NUM_GPS_READINGS) {
        fprintf(stderr,"Hclient Error: wrong number of gps readings\n");
        return(-1);
    }

    *lat = sensor[SENSOR_GPS].reading[0];
    *lon = sensor[SENSOR_GPS].reading[1];
    *dir = sensor[SENSOR_GPS].reading[3];

    // jh
    // return   < 0 -> error
    //          ==0 -> simulated value.
    //          > 0 -> real gps
    return(1);
}

/* Copy sensor packet into current sensors */
void hclientUpdateSensors(REC_SensorUpdate *cs){
    int i;
    int type;

    for(i=0;i<cs->num_sensors;i++){
        type = cs->sensor[i].type;


        if (sensor[type].type == -1)
		{
			printf("Warning: in update sensors: Valor type :%d\n",type);

			//Add hot-plugged sensors, experimental
			sensor[type].num_readings = cs->sensor[i].num_readings;
			sensor[type].num_angles = cs->sensor[i].num_angles;
			sensor[type].len_extra = cs->sensor[i].len_extra;

			if (sensor[type].len_extra)
			{
				sensor[type].extra = (char*) malloc( sensor[type].len_extra );
			}
			if (sensor[type].num_angles)
			{
				sensor[type].angle = (float*)malloc( sizeof(float) * sensor[type].num_angles );
			}
			if (sensor[type].num_readings)
			{
				sensor[type].reading = (float*)malloc( sizeof(float) * sensor[type].num_readings );
			}
		}
        //else {
            sensor[type].type = cs->sensor[i].type;
            sensor[type].len_extra = cs->sensor[i].len_extra;
            if (sensor[type].len_extra) {
                memcpy( sensor[type].extra, cs->sensor[i].extra,
                        sensor[type].len_extra );
            }
            sensor[type].num_angles = cs->sensor[i].num_angles;
            if (sensor[type].num_angles) {
                memcpy( sensor[type].angle, cs->sensor[i].angle,
                        sizeof(float) * sensor[type].num_angles);
            }
            sensor[type].num_readings = cs->sensor[i].num_readings;
            if (sensor[type].num_readings) {

				memcpy( sensor[type].reading, cs->sensor[i].reading,
                        sizeof(float) * sensor[type].num_readings );
            }
        //}
    }
}

/* Return new sensor readings from hserver */
int hclient_get_sensor_readings(int mask,int *num,sensor_t **sensorPtr)
{
    REC_SimpleCommand rec;
    REC_SensorUpdate *current_sensors;

    rec.command = HCLIENT_GET_SENSOR_READINGS_CMD;
    IPMessage *msg = hc_communicator->Query(hserver_connection,MSG_SimpleCommand,&rec,MSG_SensorUpdate);
    current_sensors = (REC_SensorUpdate*)msg->FormattedData();
    hclientUpdateSensors(current_sensors);
    delete msg;
    *num = num_sensors;
    *sensorPtr = sensor;
    return(0);
}

// ENDO
/*
// Initialize robot location. Will home nomads
int hclient_init_xyt(void)
{
    REC_SimpleCommand rec;

    rec.command = HCLIENT_INIT_XYT_CMD;
    IPMessage *msg = hc_communicator->Query(hserver_connection,MSG_SimpleCommand,&rec,MSG_SimpleCommand);
    delete msg;
    return(0);
}
*/
// This function will initializes robot, and sets its current position to
// be the specified value.
int hclient_init_robot(double x, double y, double heading)
{
    REC_InitRobot rec;

    rec.x = x;
    rec.y = y;
    rec.heading = heading;

    // Use Query (instead of SendMessage) as to make sure that
    // the robot is initialized.
    IPMessage *msg = hc_communicator->Query(
        hserver_connection,
        MSG_InitRobot,
        &rec,
        MSG_SimpleCommand);

    delete msg;
    return(0);
}

/* Update sensor readings from hserver */
int hclient_update_sensors(){
    sensor_t *sensor;
    int num_sensors;
    hclient_get_sensor_readings(0xFFFF,&num_sensors,&sensor);
    return(0);
}

/* Open robot connection. Name is the hserver name */
int hclient_open_robot(char *name,bool multipleHservers)
{
    int rtn;

    fprintf(stderr,"Connecting to hserver %s\n",name);

    if (multipleHservers) {
        sprintf(hserver_ipt_name,"%s:control",name);
    } else {
        sprintf(hserver_ipt_name,"hserver_%s",name);
    }
    sprintf(hclient_ipt_name,"hclient_%s",name);
    hclientInitVariables();
    rtn = hclientSetupIpt();
    if (rtn) return(-1);

    rtn = hclient_init_ipt_connection();
    if (rtn) return(-1);

    hclient_get_robot_setup();
    hclient_update_sensors();
    hclient_send_simple_command(HCLIENT_RANGE_STOP_CMD);
    // ENDO
    //hclient_init_xyt();
    return 0;
}

/* Return obstacles from latest sensor reading */
int hclient_get_obstacles(
    int mask,
    float danger_range,
    sensor_reading_t** reading,
    bool adjust)
{
    int i,j,idx,total_readings,rsidx;
    double x,y,t;
    float *sp,st,sd;

    hclient_getxyt(&x,&y,&t);

    total_readings = 0;

    for(i=0;i<MAX_SENSORS;i++){
        switch(sensor[i].type){
        case SENSOR_SICKLMS200:
            if (!(mask&SENSOR_SICKLMS200_MASK)) break;
            total_readings += sensor[i].num_readings;
            break;
        case SENSOR_COGNACHROME:
            if (!(mask&SENSOR_COGNACHROME_MASK)) break;
            total_readings += sensor[i].num_readings;
            break;
        case SENSOR_SONAR:
            if (!(mask&SENSOR_SONAR_MASK)) break;
            for (j = 0; j<sensor[i].num_readings; j++)
            {
                if (sensor[i].reading[j] != (float)SONAR_READING_OUTSIDE_RANGE) ++total_readings;
            }
            break;
        case SENSOR_GENERIC:
            if (!(mask&SENSOR_GENERIC_MASK)) break;
            total_readings += sensor[i].num_readings;
            break;
        }
    }

    if (total_readings) {
        *reading = (sensor_reading_t*)malloc(sizeof(sensor_reading_t)*total_readings);
    } else {
        *reading = NULL;
    }

    idx = 0;
    for(i=0;i<MAX_SENSORS;i++){
        switch(sensor[i].type){
        case SENSOR_SICKLMS200:
            if (!(mask&SENSOR_SICKLMS200_MASK)) break;
            for(j=0;j<sensor[SENSOR_SICKLMS200].num_readings;j++){
                (*reading)[idx].type = SENSOR_SICKLMS200;
                (*reading)[idx].angle = sensor[SENSOR_SICKLMS200].angle[j];
                (*reading)[idx].reading = sensor[SENSOR_SICKLMS200].reading[j];
                if ((*reading)[idx].reading < danger_range) {
                    (*reading)[idx].color = SENSOR_DANGER_COLOR;
                } else {
                    (*reading)[idx].color = SENSOR_SICKLMS200_COLOR;
                }
                idx++;
            }
            break;
        case SENSOR_COGNACHROME:
            if (!(mask&SENSOR_COGNACHROME_MASK)) break;
            extra_sensor_cog_t *ex_cog;
            ex_cog =  (extra_sensor_cog_t*)(sensor[SENSOR_COGNACHROME].extra);
            for(j=0;j<sensor[SENSOR_COGNACHROME].num_readings;j++){
                (*reading)[idx].type = SENSOR_COGNACHROME;
                (*reading)[idx].angle = sensor[SENSOR_COGNACHROME].angle[j];
                (*reading)[idx].reading = sensor[SENSOR_COGNACHROME].reading[j];
                if (j<(ex_cog->num_per_channel[0]))
                    (*reading)[idx].color = 0;
                else if (j<(ex_cog->num_per_channel[0]+ex_cog->num_per_channel[1]))
                    (*reading)[idx].color = 1;
                else (*reading)[idx].color = 2;
                idx++;
            }
            break;
        case SENSOR_SONAR:
            if (!(mask&SENSOR_SONAR_MASK)) break;
            rsidx = sensor_setup_link[SENSOR_SONAR];
            for(j=0;j<sensor[SENSOR_SONAR].num_readings;j++)
            {
                if (sensor[SENSOR_SONAR].reading[j] != (float)SONAR_READING_OUTSIDE_RANGE)
                {
                    // If "adjust" is true, the obstacle location will be adjusted for each time
                    // cycle by considering how much the robot moved. However, this part should
                    // be reviewed since some values are hard coded -- endo
                    //if (adjust)
                    /*if (adjust && sensor[SENSOR_SONAR].len_extra)
                    {
                        sp = (float*)(sensor[SENSOR_SONAR].extra);
                        st = *(sp+j*3+2);
                        sd = st-t;
                    }
                    else sd = 0;*/

                    (*reading)[idx].type = SENSOR_SONAR;
                    (*reading)[idx].angle = sensor[SENSOR_SONAR].angle[j];//+sd;
                    (*reading)[idx].reading = sensor[SENSOR_SONAR].reading[j];
                    (*reading)[idx].color = 0;
                    idx++;
                }
            }
            break;
        case SENSOR_GENERIC:
        	if (!(mask&SENSOR_GENERIC_MASK)) break;
        	for(j=0;j<sensor[SENSOR_GENERIC].num_readings;j++)
        	{
                (*reading)[idx].type = SENSOR_GENERIC;
                (*reading)[idx].angle = 0;
                (*reading)[idx].reading = sensor[SENSOR_GENERIC].reading[j];
                (*reading)[idx].color = 0;
                idx++;
        	}
        	break;
        }
    }
    return(total_readings);
}

/* Return array of latest laser reading */
int hclient_laser_read(float *reading){
    int i;
    int num_readings;

    if (sensor[SENSOR_SICKLMS200].type != SENSOR_SICKLMS200) {
        fprintf(stderr,"No laser sensor");
        return(-1);
    }

    num_readings = sensor[SENSOR_SICKLMS200].num_readings;
    for(i=0;i<sensor[SENSOR_SICKLMS200].num_readings;i++){
        reading[i] = sensor[SENSOR_SICKLMS200].reading[i];
    }
    return(num_readings);
}

/*Obtiene la altura a la que se encuentra la pinza*/
int hclient_get_pinza(float *altura){

	REC_SimpleCommand rec;
    REC_GetPinza *pinza;

    rec.command = HCLIENT_GET_PINZA_ALTURA_CMD;
    IPMessage *msg = hc_communicator->Query(hserver_connection,MSG_SimpleCommand,&rec,MSG_GetPinza);
    pinza = (REC_GetPinza*)msg->FormattedData();

    *altura = pinza->altura;

    delete msg;
    return(0);
}

/*Establece la altura a la que se debe situar la pinza*/
int hclient_set_pinza(float pos){

	REC_SetPinza rec;

    rec.altura = pos;
    hc_communicator->SendMessage(hserver_connection,MSG_SetPinza,&rec);

    return(0);
}

/*Activa los motores de limpieza*/
int hclient_motores_aspirar(bool power)
{
	REC_MotoresAspirar rec;

 	rec.power = (int)power;
    hc_communicator->SendMessage(hserver_connection,MSG_MotoresAspirar,&rec);

    return (0);
}

void hclient_send_advanced_telop(int joy_x, int joy_y, int slider1, int slider2, int joy_buttons)
{
	REC_AdvancedTelop rec;
	rec.joy_x = joy_x;
	rec.joy_y = joy_y;
	rec.slider1 = slider1;
	rec.slider2 = slider2;
	rec.joy_buttons = joy_buttons;
    hc_communicator->SendMessage(hserver_connection,MSG_AdvancedTelop,&rec);
}

const char *hclient_get_central_server()
{
	REC_SimpleCommand rec;
	REC_GetCentralServer *centralServer;
	const char *url;

    rec.command = HCLIENT_GET_CENTRAL_SERVER_CMD;
    IPMessage *msg = hc_communicator->Query(hserver_connection,MSG_SimpleCommand,&rec,MSG_GetCentralServer);
    centralServer = (REC_GetCentralServer*)msg->FormattedData();

    url = strdup(centralServer->url);

    delete msg;
    return url;
}

/**********************************************************************
# $Log: hclient.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:55  endo
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
# Revision 1.13  2003/06/19 21:18:53  endo
# *** empty log message ***
#
# Revision 1.12  2003/06/19 20:51:31  endo
# *** empty log message ***
#
# Revision 1.11  2003/04/06 13:20:20  endo
# gcc 3.1.1
#
# Revision 1.10  2003/04/02 22:14:17  zkira
# GPS and battery information for telop GUI
#
# Revision 1.9  2000/12/12 22:45:09  blee
# Changed the case for SENSOR_SICKLMS200 in hclient_get_robot_setup()
# to include angle counts, and changed the case for SENSOR_SICKLMS200
# in hclient_get_obstacles() to get the angles.
#
# Revision 1.8  2000/11/13 21:07:45  endo
# *** empty log message ***
#
# Revision 1.7  2000/11/13 20:16:04  endo
# If the sonar reading is outside its range, it will be filtered.
#
# Revision 1.6  2000/09/19 03:52:33  endo
# Sonar bug fixed.
#
#
#**********************************************************************/
