/**********************************************************************
 **                                                                  **
 **                          robot_config.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Robot setup routines for HServer                                **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: robot_config.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string.h>
#include <stdlib.h>
#include <panel.h>
#include <sys/types.h>
#include <sys/socket.h>


#include "hserver.h"
#include "robot_config.h"
#include "sensors.h"


RobotConfiguration *robot_config = NULL;


RobotPart::~RobotPart(){
    robot_config->remove(index);
    if (info != NULL) // Ernest
	delete info;
}

RobotPart::RobotPart(int type, int len_info,char *info)
{
    this->type = type;
    this->len_info = len_info;
    this->info = info;
    index = robot_config->add(this);
}

RobotPartSonar::RobotPartSonar(
    int number,
    float *angle,
    float fov,
    float range) : RobotPart(SENSOR_SONAR)
{
    info_sonar_t *info;
    len_info = sizeof(info_sonar_t);
    info = new info_sonar_t;
    info->number = number;
    //info->angle = angle;
    info->fov = fov;
    info->range = range;
    this->info = (char *)info;
}


RobotPartRoomba::RobotPartRoomba(
    int number,
    float *angle,
    float fov,
    float range) : RobotPart(SENSOR_ROOMBA)
{
    info_roomba_t *info;
    len_info = sizeof(info_roomba_t);
    info = new info_roomba_t;
    info->number = number;
    info->fov = fov;
    info->range = range;
    this->info = (char *)info;
}


RobotPartMotoresAspirar::RobotPartMotoresAspirar() : RobotPart(MOTOR_ASPIRAR)
{
    info_motor_aspirador *info;
    len_info = sizeof(info_motor_aspirador);
    info = new info_motor_aspirador;
    info->power = false;
}

RobotPartWebCam::RobotPartWebCam(int nPixel_x, int nPixel_y) : RobotPart(WEBCAM){
    info_webCam_t *info;
    len_info = sizeof(info_webCam_t);
    info = new info_webCam_t;
    info->nPixel_x = nPixel_x;
    info->nPixel_y = nPixel_y;
}


RobotPartIr::RobotPartIr(
    int number,
    float *angle,
    float fov,
    float range) : RobotPart(SENSOR_IR)
{
    info_ir_t *info;
    len_info = sizeof(info_ir_t);
    info = new info_ir_t;
    info->number = number;
    //info->angle = angle;
    info->fov = fov;
    info->range = range;
    this->info = (char *)info;
}

RobotPartPanTilt::RobotPartPanTilt(float angle,float fov_x,float fov_y) : RobotPart(SENSOR_PANTILT_CAMERA) {
    info_pantilt_camera_t *info;
    len_info = sizeof(info_pantilt_camera_t);
    info = new info_pantilt_camera_t;
    info->angle = angle;
    info->fov_x = fov_x;
    info->fov_y = fov_y;
    this->info = (char *)info;
}

RobotPartVideo::RobotPartVideo(int num_channels) : RobotPart(SENSOR_VIDEO) {
    info_video_t *info;
    len_info = sizeof(info_video_t);
    info = new info_video_t;
    info->num_channels = num_channels;
    this->info = (char *)info;
}

RobotPartCognachrome::RobotPartCognachrome(int num_channels,int max_per_channel) : RobotPart(SENSOR_COGNACHROME) {
    info_cog_t *info;
    len_info = sizeof(info_cog_t);
    info = new info_cog_t;
    info->num_channels = num_channels;
    info->max_per_channel = max_per_channel;
    this->info = (char *)info;
}

RobotPartXyt::RobotPartXyt(float scale) : RobotPart(SENSOR_XYT)
{
    info_xyt_t *info;
    len_info = sizeof(info_xyt_t);
    info = new info_xyt_t;
    info->scale = scale;
    this->info = (char *)info;
}

RobotPartLaser::RobotPartLaser(int num_readings,float angle,float fov,float range) : RobotPart(SENSOR_SICKLMS200) {
    info_laser_t *info;
    len_info = sizeof(info_laser_t);
    info = new info_laser_t;
    info->num_readings = num_readings;
    info->angle = angle;
    info->fov = fov;
    info->range = range;
    this->info = (char *)info;
}

RobotPartSensorGeneric::RobotPartSensorGeneric(int num_sensor) : RobotPart(SENSOR_GENERIC)
{
    info_sensor_generic_t *info;
    len_info = sizeof(info_sensor_generic_t);
    info = new info_sensor_generic_t;
    info->num_sensor = num_sensor;
	this->info = (char *)info;
}

RobotPartArm::RobotPartArm() : RobotPart(SENSOR_ARM) {
}

RobotPartCompass::RobotPartCompass() : RobotPart(SENSOR_COMPASS) {
}

RobotPartGps::RobotPartGps() : RobotPart(SENSOR_GPS) {
}

int RobotConfiguration::add(RobotPart *part){
    int i;
    int idx = -1;
    if (num_parts == max_parts) return(-1);
    this->part[num_parts] = part;
    for(i=0;i<max_parts;i++){
	if (index[i] == -1){
	    idx = i;
	    break;
	}
    }
    index[idx] = num_parts;
    num_parts++;
    return(idx);
}

void RobotConfiguration::remove(int idx){
    int i,low;
    low = index[idx];
    index[idx] = -1;
    for(i=0;i<max_parts;i++){
	if (index[i]>low) index[i]--;
	if ((i>=low)&&(i<max_parts-1)) part[i] = part[i+1];
    }
    num_parts--;
}

void RobotConfiguration::fillIptMessage(REC_RobotSetupInfo *rec){
    int i;
    rec->name = name;
    rec->num_parts = num_parts;
    if (num_parts) rec->part = new REC_RobotPartT[num_parts];
    for(i=0;i<num_parts;i++){
	rec->part[i].type = part[i]->type;
	rec->part[i].len_info = part[i]->len_info;
	rec->part[i].info = part[i]->info;
    }
}


void RobotConfiguration::draw(){
    int line = 1;
    int i;
    info_pantilt_camera_t *info_pantilt;
    info_laser_t *info_laser;
    info_sonar_t *info_sonar;
	info_roomba_t *info_roomba;
    info_ir_t *info_ir;
    info_cog_t *info_cog;
    info_video_t *info_video;
    info_xyt_t *info_xyt;

    char buf[100];

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock(&screenMutex);
    werase(window);
    wmove(window,0,0);
    wborder(window,0,0,0,0,0,0,0,0);
    mvwaddstr(window,line++,1,"-- Robot Setup --");
    mvwaddstr(window,line++,1,"");
    sprintf(buf,"id: %d  name: %s",id,name);
    mvwaddstr(window,line++,1,buf);
    sprintf(buf,"sensors  number: %d",num_parts);
    mvwaddstr(window,line++,1,buf);
    for(i=0;i<num_parts;i++)
    {
	switch(part[i]->type){
	case SENSOR_PANTILT_CAMERA:
	    info_pantilt = (info_pantilt_camera_t*)part[i]->info;
	    sprintf(buf,"%d.  pan/tilt camera   angle: %.2f  fov x: %.2f y: %.2f",i,
		    info_pantilt->angle,info_pantilt->fov_x,info_pantilt->fov_y);
	    break;
	case SENSOR_XYT:
	    info_xyt = (info_xyt_t*)part[i]->info;
	    sprintf(buf,"%d.  xyt  scale: %f",i,info_xyt->scale);
	    break;
	case SENSOR_ARM:
	    sprintf(buf,"%d.  arm",i);
	    break;
	case SENSOR_SICKLMS200:
	    info_laser = (info_laser_t*)part[i]->info;
	    sprintf(buf,"%d.  laser  num readings: %d angle: %.2f  fov %.2f",i,
		    info_laser->num_readings,info_laser->angle,info_laser->fov);
	    break;
	case SENSOR_SONAR:
	    info_sonar = (info_sonar_t*)part[i]->info;
	    sprintf(buf,"%d.  sonar  number: %d  fov: %.2f  range: %.2f",i,
		    info_sonar->number,info_sonar->fov,info_sonar->range);
	    mvwaddstr(window,line++,1,buf);
		break;
	case SENSOR_ROOMBA:
	    info_roomba = (info_roomba_t*)part[i]->info;
	    sprintf(buf,"%d.  roomba  number: %d  fov: %.2f  range: %.2f",i,
		    info_roomba->number,info_roomba->fov,info_roomba->range);
	    mvwaddstr(window,line++,1,buf);
	    break;
	case SENSOR_IR:
	    info_ir = (info_ir_t*)part[i]->info;
	    sprintf(buf,"%d.  ir  number: %d  fov: %.2f  range: %.2f",i,
		    info_ir->number,info_ir->fov,info_ir->range);
	    break;
	case SENSOR_COGNACHROME:
	    info_cog = (info_cog_t*)part[i]->info;
	    sprintf(buf,"%d.  cognachrome  num channels: %d  max per channel: %d",i,
		    info_cog->num_channels,info_cog->max_per_channel);
	    break;
	case SENSOR_VIDEO:
	    info_video = (info_video_t*)part[i]->info;
	    sprintf(buf,"%d.  video  num channels: %d",i,info_video->num_channels);
	    break;
	case SENSOR_COMPASS:
	    sprintf(buf,"%d.  compass",i);
	    break;
	case SENSOR_GPS:
	    sprintf(buf,"%d.  gps",i);
	    break;
	default:
	    sprintf(buf,"%d.  Unknown sensor type",i);
	}
	mvwaddstr(window,line++,1,buf);
    }
    pthread_cleanup_pop(1);
}

void RobotConfiguration::windowInit(){
    panelX = SETUP_WIN_X;
    panelY = SETUP_WIN_Y;
    panelH = SETUP_WIN_ROW;
    panelW = SETUP_WIN_COL;
    if (panelH>screenY) panelH = screenY;
    if (panelW>screenX) panelW = screenX;
    if (panelX+panelW>screenX) panelX = screenX-panelW;
    if (panelY+panelH>screenY) panelY = screenY-panelH;
    window = newwin(panelH,panelW,panelY,panelX);
    panel = new_panel(window);
    draw();
}

void RobotConfiguration::capInt(int *i,int l,int u){
  if (*i<l) *i = l;
  if (*i>u) *i = u;
}

void RobotConfiguration::movePanel(int xdif,int ydif){
    panelX += xdif;
    panelY += ydif;
    capInt(&panelX,0,mainPanelX-SETUP_WIN_COL);
    capInt(&panelY,0,mainPanelY-SETUP_WIN_ROW);
    move_panel(panel,panelY,panelX);
    redrawWindows();
}

void RobotConfiguration::movePanelKey(int c){
    switch(c){
    case '1':
	movePanel(-1,1);
	break;
    case '2':
	movePanel(0,1);
	break;
    case '3':
	movePanel(1,1);
	break;
    case '4':
	movePanel(-1,0);
	break;
    case '5':
	break;
    case '6':
	movePanel(1,0);
	break;
    case '7':
	movePanel(-1,-1);
	break;
    case '8':
	movePanel(0,-1);
	break;
    case '9':
	movePanel(1,-1);
	break;
    }
}

int RobotConfiguration::runKey(int c){
    switch(c){
    case 'r':
	refreshScreen();
	break;
    case 'x':
    case 'Q':
	return(1);
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
	movePanelKey(c);
	break;
    }
    return(0);
}

void RobotConfiguration::show(){
    int c,done;

    done = false;
    show_panel(panel);
    draw();
    redrawWindows();
    do {
	c = getch();
	done = runKey(c);
    } while (!done);
    hide_panel(panel);
    redrawWindows();
    return;
}


RobotConfiguration::~RobotConfiguration(){
}

RobotConfiguration::RobotConfiguration(int max,int id,char *name){
    num_parts = 0;
    max_parts = max;
    part = new RobotPart*[max_parts];
    memset(part,-1,sizeof(RobotPart*)*max_parts); // Ernest - initialize as precaution
    index = new char[max_parts];
    memset(index,-1,sizeof(char)*max_parts);
    this->name = strdup(name);
    this->id = id;
    windowInit();
    hide_panel(panel);
}


/**********************************************************************
# $Log: robot_config.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 03:06:53  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.5  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.4  2002/01/16 22:36:50  ernest
# Initialize "part"; check for NULL ptr before deleting "info"
#
# Revision 1.3  2000/11/13 20:12:59  endo
# Minor compiler warning fix.
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
