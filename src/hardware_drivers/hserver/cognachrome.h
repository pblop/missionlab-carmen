/**********************************************************************
 **                                                                  **
 **                           cognachrome.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Cognachrome vision system interface for HServer                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cognachrome.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef COGNACHROME_H_
#define COGNACHROME_H_

#include <pthread.h>
#include <string>
#include "module.h"
#include "sensors.h"
#include "robot_config.h"

using std::string;

#define CHROME_MAX_X 250.0
#define CHROME_MAX_Y 250.0
#define COG_STREAM_BREAK 0xFE
#define COG_LEX_PROMPT           1
#define COG_LEX_ADDING           2
#define COG_LEX_SAVING           3
#define COG_LEX_RETRAINING       4
#define COG_LEX_PROTOCOL         5 
#define COG_LEX_PROTOCOL_HEADER  6
#define COG_LEX_PROTOCOL_ABC     7
#define COG_LEX_TRACKING         8
#define COG_LEX_CHANNEL          9
#define COG_LEX_NUM_OBJECTS      10




class Cognachrome: public Module
{
  protected:
    int trackStatusLine;
    char *trackStatusStr;
    Sensor *sensorCog;
    RobotPartCognachrome *partCog;
    extra_sensor_cog_t extra_sensor_cog;
    int fd;
    int stream_num;
    bool baudUp,connected;
    bool threads_running;
    pthread_t reader,sender;
    int max_objects,max_objects_per_color,num_colors;
    float *angle,*reading;
    int *colorobjects,*numobjects,*maxnumobjects,*sizeobjects;
    string port_str;
    int mode,filter,channel;
    int total_receive,total_transmit;
    int min_blob_size;
    bool isTracking,crosshairs;
    pthread_mutex_t sendQueMutex,statusMutex;
    int send_que_max;
    char** sendQue;
    unsigned int sendQueFirst;
    unsigned int sendQueLast;
    int pendingSends;
    static const int INITIAL_OBJECT_SIZE;
    void useTpuvisCommand( int rtn, unsigned char* text, int len );
    int sendCog();
    int readCog();
    void setupTpuvis();
    void setProtocolString();
    void sendSaphiraPacket( int fd, char* str, int len );
    int calcSaphiraChksum( byte_t* ptr );
    int tpuvisCommand( char command );
    int setFilter( int filter );
    int setChannel( int channel );
    void stopTracking();
    void startTracking();
    void sendStream( int stream, char* str, int nobreak = false );
    void updateSensor();
    void addSensor();
    int postSend( char* msg );
    void updateStatusBar_(void);

  public:
    int lexRead();
    int largestObject( int color, int* x,int* y,int* z ); // color = -1 for all
    void reader_thread();
    void sender_thread();
    void control();
    Cognachrome( Cognachrome** a, const string& strPortString );
    ~Cognachrome();
};

extern Cognachrome* cognachrome;

void cognachromeConnect();

#endif

/**********************************************************************
# $Log: cognachrome.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.5  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.4  2001/03/23 21:28:51  blee
# altered to use a config file
#
# Revision 1.3  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
