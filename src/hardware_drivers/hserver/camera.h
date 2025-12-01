/**********************************************************************
 **                                                                  **
 **                              camera.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Camera control for HServer                                      **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: camera.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef CAMERA_H_
#define CAMERA_H_

#include <pthread.h>
#include <string>
#include "module.h"
#include "sensors.h"
#include "robot_config.h"

using std::string;

#define TRACKER_MODE_NONE            0
#define TRACKER_MODE_CENTER          1
#define TRACKER_MODE_SWEEP_X         2
#define TRACKER_MODE_SWEEP_Y         3
#define TRACKER_MODE_SWEEP_XY        4
#define TRACKER_MODE_LARGEST_OBJECT  5
#define TRACKER_MODE_LARGEST_A       6
#define TRACKER_MODE_LARGEST_B       7
#define TRACKER_MODE_LARGEST_C       8
#define TRACKER_MODE_CLOSEST_LASER   9
#define TRACKER_MODE_SWEEP_X_HIGH    10
#define TRACKER_MODE_SWEEP_X_LOW     11
#define TRACKER_MODE_SWEEP_Y_LEFT    12
#define TRACKER_MODE_SWEEP_Y_RIGHT   13

typedef struct
{
    unsigned char* msg;
    int inq;
} camMsg_t;
#define MAX_CAM_MESSAGES 20

// Measured                 Dec              Hex                   By spec 

#define PAN_ABS_POS_MIN    -860              // 0xFCA4            -880 = 0xFC90    
#define PAN_ABS_POS_MAX     862              // 0x035E             880 = 0x0370   
#define TILT_ABS_POS_MIN   -281              // 0xFEE7            -300 = 0xFED4
#define TILT_ABS_POS_MAX    283              // 0x011B             300 = 0x012C

#define PAN_REL_POS_MIN   -1722              //                  -1760
#define PAN_REL_POS_MAX    1722              //                   1760
#define TILT_REL_POS_MIN   -564              //                   -600
#define TILT_REL_POS_MAX    564              //                    600

#define CAMERA_FOV_X 48.0
#define CAMERA_FOV_Y 36.0

#define FOCUS_POS_NEAR     0x9FFF
#define FOCUS_POS_FAR      0x1000

#define CAMERA_MAX_PAN_SPEED  0x18
#define CAMERA_MAX_TILT_SPEED 0x14

#define MAX_PAN 0x370
#define MAX_PAN_DEG 100
#define PAN_TO_DEG  -((float)MAX_PAN_DEG / (float)MAX_PAN )
#define MAX_TILT_DEG 25
#define MAX_TILT 0x12C
#define TILT_TO_DEG ( (float)MAX_TILT_DEG / (float)MAX_TILT )

class Camera: public Module
{
  protected:
    Sensor* sensorCamera;
    RobotPart* partCamera;
    camMsg_t camQ[ MAX_CAM_MESSAGES ];
    pthread_mutex_t camQMutex;
    int qTop, qBot, qEmpty, qNext;
    camMsg_t moveMsg;
    int haveMove;
    int fd;
    int sockets[ 2 ];
    int currentJob;
    int inqJob;
    int numjobs;
    int trackerMode;
    int goalTilt,goalPan,goalZoom;
    bool powerOn,md_mode;
    int pan,tilt,zoom,focus;
    bool threads_running;
    pthread_t reader,position,tracker;
    float updateSpeed;
    float current_readings[ 4 ];
    int writeMsg( unsigned char* msg, int len );
    int sendCommand( unsigned char* msg,int inq = false );
    void zoomDirect( int zoom );
    int readMsg( unsigned char* msg );
    void processReply( int command, byte_t* msg );
    void runCamQ();
    void decodeMessage( unsigned char* msg );
    void getPacket();
    bool setAttr( int fd ); 
    void addSensor();
    void updateStatusBar_(void);

  public:
    int getPan() { return pan; };
    void reader_thread();
    void tracker_thread();
    void position_thread();
    void setTrackerMode( int mode );
    void moveAbs( int pan, int tilt );
    void moveDeg( float pan, float tilt );
    void power( bool on );
    void MDModeOnOff( bool on );
    int panTilt( int pan, int tilt, int pan_speed = CAMERA_MAX_PAN_SPEED, 
                 int tilt_speed = CAMERA_MAX_TILT_SPEED );
    void zoomTo( int zoom );
    void zoomInc( int zoom );
    void control();
    Camera( Camera** a, const string& strPort );
    ~Camera();
};

extern Camera *camera;

void cameraConnect();

#endif

/**********************************************************************
# $Log: camera.h,v $
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
# Revision 1.4  2001/03/23 21:27:40  blee
# altered to use a config file
#
# Revision 1.3  2000/09/19 03:07:52  endo
# More sweep mode added.
#
#
#**********************************************************************/
