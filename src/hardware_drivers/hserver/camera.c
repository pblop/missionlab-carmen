/**********************************************************************
 **                                                                  **
 **                              camera.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Camera control for HServer                                      **
 **                                                                  **
 **  Some code take from Vladimir Kravtchenko's SONY EVI D30/D31     **
 **  Interface Code, Version 1.3                                     **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: camera.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "camera.h"
#include "message.h"
#include "cognachrome.h"
#include "statusbar.h"
#include "laser.h"
#include "robot_config.h"
#include "sensors.h"

Camera* camera = NULL;

#define MAX_PAN_DEG 100
#define MAX_TILT_DEG 25
#define MAX_PAN 0x370
#define MAX_TILT 0x12C
#define MAX_PAN_SPEED 0x18
#define MAX_PAN_DEG_PER_SEC 80
#define MAX_TILT_SPEED 0x14
#define MAX_TILT_DEG_PER_SEC 50
#define DEG_TO_PAN -( MAX_PAN / MAX_PAN_DEG )
#define PAN_TO_DEG  -((float)MAX_PAN_DEG / (float)MAX_PAN )
#define DEG_TO_TILT ( MAX_TILT / MAX_TILT_DEG )
#define TILT_TO_DEG ( (float)MAX_TILT_DEG / (float)MAX_TILT )
#define MAX_ZOOM 1023

#define VISCA_PACKET_MAX   16                // Max packet length 16 byte

#define ZOOM_POS_MIN       0x0000            // 0
#define ZOOM_POS_MAX       0x03FF            // 1023

#define FOCUS_POS_NEAR     0x9FFF
#define FOCUS_POS_FAR      0x1000

#define JOB_IDLE          0
#define JOB_MOVE          1
#define JOB_GETPANTILT    2
#define JOB_GETZOOM       3
#define JOB_HOME          4
#define JOB_BACKLIGHTOFF  5
#define JOB_IFCLEAR       6
#define JOB_SETADDRESS    7
#define JOB_ZOOM          8
#define JOB_POWERON       9
#define JOB_POWEROFF      10
#define JOB_FOCUSINQ      11
#define JOB_MDMODEON      12
#define JOB_MDMODEOFF     13

static char jobNames[][20] = {"idle","move","get pan/tilt","get zoom","home",
  "back light off","ifclear","set address","zoom","power on","power off","focus inq",
  "md mode on","md mode off"};

#define RTS	0x004
#define CTS	0x020
#define DSR	0x100

#define LINE_CHECK_FLAG (CTS | DSR | RTS)

static unsigned char CamPanTiltPosInq[] = { JOB_GETPANTILT,5, 1, 0x81, 0x09, 0x06, 0x12, 0xFF };
//static unsigned char CamZoomPosInq[]    = { JOB_GETZOOM,5, 1, 0x81, 0x09, 0x04, 0x47, 0xFF };
static unsigned char CamPTDHome[]       = { JOB_HOME,5, 2, 0x81, 0x01, 0x06, 0x04, 0xFF };
static unsigned char CamBackLightOff[]  = { JOB_BACKLIGHTOFF,6, 2, 0x81, 0x01, 0x04, 0x33, 0x03, 0xFF };
static unsigned char CamIFClearB[]      = { JOB_IFCLEAR,5, 1, 0x88, 0x01, 0x00, 0x01, 0xFF };
static unsigned char CamSetAddressB[]   = { JOB_SETADDRESS,4, 1, 0x88, 0x30, 0x01, 0xFF };
static unsigned char CamPTDAbsPos[]     = { JOB_MOVE,15, 2, 0x81, 0x01, 0x06, 0x02, 0x00, 0x00, 
                                            0,0,0,0, 0,0,0,0, 0xFF };
static unsigned char CamZoomDirect[]    = { JOB_ZOOM,9, 1, 0x81, 0x01, 0x04, 0x47, 0x00, 0x00, 
                                            0x00, 0x00, 0xFF };
static unsigned char CamPowerOn[]       = { JOB_POWERON,6, 2, 0x81, 0x01, 0x04, 0x00, 0x02, 0xFF };
static unsigned char CamPowerOff[]      = { JOB_POWEROFF,6, 2, 0x81, 0x01, 0x04, 0x00, 0x03, 0xFF };
static unsigned char CamFocusPosInq[]   = { JOB_FOCUSINQ,5, 1, 0x81, 0x09, 0x04, 0x48, 0xFF };
static unsigned char CamMDModeOn[]      = { JOB_MDMODEON,6, 2, 0x81, 0x01, 0x07, 0x08, 0x02, 0xFF };
static unsigned char CamMDModeOff[]     = { JOB_MDMODEOFF,6, 2, 0x81, 0x01, 0x07, 0x08, 0x03, 0xFF };

#define CAM_PAN   0 
#define CAM_TILT  1
#define CAM_ZOOM  2 
#define CAM_FOCUS 3

void Camera::updateStatusBar_(void)
{
    char buf[ 100 ];
  
    statusStr[ 0 ] = '\0';

    strcat( statusStr, "Camera: " );

    sprintf( buf, "%c %.0f up/sec p: %.1f[%d](%d) t: %.1f[%d](%d) z: %d(%d) foc: %d ",
        statusbarSpinner_->getStatus(),
             updateSpeed, pan * PAN_TO_DEG, pan, goalPan, tilt * TILT_TO_DEG, tilt, 
             goalTilt, zoom, goalZoom, focus );

    switch( trackerMode ) {

    case TRACKER_MODE_NONE:          
        break;
    case TRACKER_MODE_CENTER:
        strcat( buf, "center" );
        break;
    case TRACKER_MODE_SWEEP_X:
        strcat( buf, "sweep x (center)" );
        break;
    case TRACKER_MODE_SWEEP_X_HIGH:
        strcat( buf, "sweep x (high)" );
        break;
    case TRACKER_MODE_SWEEP_X_LOW:
        strcat( buf, "sweep x (low)" );
        break;
    case TRACKER_MODE_SWEEP_Y:
        strcat( buf, "sweep y (center)" );
        break;
    case TRACKER_MODE_SWEEP_Y_LEFT:
        strcat( buf, "sweep y (left)" );
        break;
    case TRACKER_MODE_SWEEP_Y_RIGHT:
        strcat( buf, "sweep y (right)" );
        break;
    case TRACKER_MODE_SWEEP_XY:
        strcat( buf, "speep xy" );
        break;
    case TRACKER_MODE_LARGEST_OBJECT:
        strcat( buf, "largest" );
        break;
    case TRACKER_MODE_CLOSEST_LASER:
        strcat( buf, "laser" );
        break;
    }
    strcat( statusStr, buf );
    statusbar->update( statusLine );
}

// send message to camera
int Camera::writeMsg( unsigned char* msg, int len )
{
    int     rc, lineStatus, lineChk;
    struct  termios *term_st;

    // Check receiver is still there via LINE_CHECK_FLAG

    rc = ioctl( fd, TIOCMGET, &lineChk );

    lineStatus = lineChk & LINE_CHECK_FLAG;

    if ( lineStatus != LINE_CHECK_FLAG )
    {
        //fprintf(stderr,"Lost connection, line status: " );
        //fprintf(stderr,"%o\n", lineStatus);
        return FALSE;
    }

    rc = write( fd, msg, (int) len );          // Send Visca message

    if ( rc == -1 )
    {
        fprintf( stderr, "Error writing to link\n" );
        perror( "Write link" );

        /* Flush the link */

        term_st = (struct termios*) malloc( sizeof( struct termios ) );
        rc = tcgetattr( fd, term_st );
        if ( rc == -1 )
        {
            fprintf( stderr, "Error getting attr for flush\n" );
            perror( "tcgetattr error" );
        }
        else
        {
            term_st->c_lflag = ( FLUSHO | term_st->c_lflag );
            rc = tcsetattr( fd, TCSANOW, term_st );
            if ( rc == -1 )
            {
                fprintf( stderr, "Error flushing link\n" );
                perror( "tcsetattr error" );
            }
        }
        free( term_st );
        return FALSE;
    }

    return TRUE;
}


/* send command from que to camera */
int Camera::sendCommand( unsigned char* msg, int inq )
{
    int rtn = 0;
    if ( numjobs == 2 )
    {
        if ( msg == CamPTDAbsPos )
        {
            moveMsg.msg = msg;
            moveMsg.inq = inq;
            haveMove = true;
        }
        else
        {
            pthread_cleanup_push( (void(*)(void*)) pthread_mutex_unlock, (void *) &camQMutex );
            pthread_mutex_lock( &camQMutex );
            if ( ( qTop == qBot ) && ( !qEmpty ) )
            {
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    //printTextWindow("Camera send que full" );
                    rtn = -1;
                }
            }
            else
            {
                camQ[ qTop ].msg = msg;
                camQ[ qTop ].inq = inq;
                qTop++;
                if ( qTop == MAX_CAM_MESSAGES )
                {
                    qTop = 0;
                }
                qEmpty = false;
            }
            pthread_cleanup_pop( 1 );
        }
        return rtn;
    }
    if (!inq)
    {
        currentJob = msg[ 0 ];
    }
    else
    {
        inqJob = msg[ 0 ];
    }
    rtn = writeMsg( &msg[ 3 ], msg[ 1 ] );
    if ( !rtn )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Camera Error send command" );
            return -1;
        }
    }
    numjobs++;
    return 0;
}


static void SplitShort ( byte_t *src, byte_t *dest ) 
{
    dest[ 0 ] = src[ 1 ] >> 4;
    dest[ 1 ] = src[ 1 ] & 0x0F;
    dest[ 2 ] = src[ 0 ] >> 4;
    dest[ 3 ] = src[ 0 ] & 0x0F;
}

/* Absolution position */
int Camera::panTilt(
    int pan,
    int tilt,
    int pan_speed, 
    int tilt_speed)
{
    int rtn;
    ushort span,stilt;
    span = (ushort) pan;
    stilt = (ushort) tilt;
    CamPTDAbsPos[ 7 ] = (byte_t) pan_speed;
    CamPTDAbsPos[ 8 ] = (byte_t) tilt_speed;
    SplitShort( (byte_t*) &span, &CamPTDAbsPos[ 9 ] );
    SplitShort( (byte_t*) &stilt, &CamPTDAbsPos[ 13 ] );
    rtn = sendCommand( CamPTDAbsPos );
    return rtn;
}

void Camera::setTrackerMode( int mode )
{
    if ( mode != trackerMode )
    {
        panTilt( pan, tilt ); // ???
    }
    trackerMode = mode;
}

// Absolute position
void Camera::moveAbs( int pan, int tilt )
{
    if ( pan < PAN_ABS_POS_MIN )
    {
        pan = PAN_ABS_POS_MIN;
    }
    else if ( pan > PAN_ABS_POS_MAX )
    {
        pan = PAN_ABS_POS_MAX;
    }
    if ( tilt < TILT_ABS_POS_MIN )
    {
        tilt = TILT_ABS_POS_MIN;
    }
    else if ( tilt > TILT_ABS_POS_MAX )
    {
        tilt = TILT_ABS_POS_MAX;
    }
    panTilt( pan, tilt );
    goalTilt = tilt;
    goalPan = pan;
}

void Camera::power( bool on )
{
    if ( on )
    {
        sendCommand( CamPowerOn );
    }
    else
    {
        sendCommand( CamPowerOff );
    }
}

void Camera::MDModeOnOff( bool on )
{
    if ( on )
    {
        sendCommand(CamMDModeOn);
    }
    else
    {
        sendCommand(CamMDModeOff);
    }
}

void Camera::zoomDirect( int zoom )
{
    ushort szoom = (ushort) zoom;

    SplitShort( (byte_t*) &szoom, &CamZoomDirect[ 7 ] );
    sendCommand( CamZoomDirect );
}

void Camera::zoomTo( int zoom )
{
    if ( zoom < ZOOM_POS_MIN )
    {
        zoom = ZOOM_POS_MIN;
    }
    else if ( zoom > ZOOM_POS_MAX )
    {
        zoom = ZOOM_POS_MAX;
    }
    zoomDirect( zoom );
    goalZoom = zoom;
}

void Camera::zoomInc( int zoom )
{
    zoom = goalZoom + zoom;
    if ( zoom < ZOOM_POS_MIN )
    {
        zoom = ZOOM_POS_MIN;
    }
    else if ( zoom > ZOOM_POS_MAX )
    {
        zoom = ZOOM_POS_MAX;
    }
    zoomDirect( zoom );
    goalZoom = zoom;
}

// Read message from camera
int Camera::readMsg( unsigned char* msg )
{
    int rc, acc = 0, flag = TRUE;

    // Read complete Visca command character
    // by character up to 0xFF which can appear
    // only at the end !
    while ( flag )
    {	
        pthread_testcancel();
        rc = read( fd, &msg[ acc ], 1 );
        pthread_testcancel();
	
        if ( rc > 0 )
        {
            if ( msg[ acc++ ] == 0xFF)
            {
                flag = FALSE;
            }
        }
        else if ( rc < 0 )    // if read error occured
        {
            acc  = 0;
            flag = FALSE;
        }
    }
    return acc;
}

void Camera::processReply( int command, byte_t* msg )
{
    char buf[ 100 ];
    static struct timeval oldtime, nowtime;  
    static int firsttime = true;
    long timediff;

    if ( firsttime )
    {
        firsttime = false;
        gettimeofday( &oldtime, NULL );
    }
    gettimeofday( &nowtime, NULL );
    timediff = ( nowtime.tv_sec - oldtime.tv_sec ) * 1000000 + 
               ( nowtime.tv_usec - oldtime.tv_usec );
    numjobs--;
    switch( command )
    {
    case JOB_GETZOOM:
        zoom = msg[ 2 ] << 12 | msg[ 3 ] << 8 | msg[ 4 ] << 4 | msg[ 5 ];
        //sprintf(buf,"Camera zoom %d ",zoom);
        //printTextWindow(buf);
        current_readings[ CAM_ZOOM ] = (float) zoom;
        break;
    case JOB_GETPANTILT:
        oldtime = nowtime;
        updateSpeed = 1000000 / timediff;
        if ( report_level >= HS_REP_LEV_DEBUG )
        {
            strcat( buf, "info " );
        }
        pan = msg[ 2 ] << 12 | msg[ 3 ] << 8 | msg[ 4 ] << 4 | msg[ 5 ];
        if ( pan & 0x00008000 )
        {
            pan = pan | 0xFFFF0000;
        }
        tilt = msg[ 6 ] << 12 | msg[ 7 ] << 8 | msg[ 8 ] << 4 | msg[ 9 ];
        if ( tilt & 0x00008000 )
        {
            tilt=tilt | 0xFFFF0000;
        }
        if ( report_level >= HS_REP_LEV_DEBUG )
        {
            printfTextWindow( "Camera pan tilt pan %d tilt %d\n", pan, tilt );
        }
        current_readings[ CAM_PAN  ] = (float) pan;
        current_readings[ CAM_TILT ] = (float) tilt;
        break;
    case JOB_FOCUSINQ:
        focus = msg[ 2 ] << 12 | msg[ 3 ] << 8 | msg[ 4 ] << 4 | msg[ 5 ];
        current_readings[ CAM_FOCUS ] = (float) focus;
        break;
    default:
        printfTextWindow( "Camera unknown reply %d", command );
        break;
    }
}

void Camera::runCamQ()
{
    if ( !haveMove && qEmpty )
    {
        return;
    }
    if ( haveMove )
    {
        haveMove = false;
        sendCommand( moveMsg.msg, moveMsg.inq );
    }
    else
    {
        sendCommand( camQ[ qBot ].msg, camQ[ qBot ].inq );
        pthread_cleanup_push( (void(*)(void*)) pthread_mutex_unlock, (void *) &camQMutex );
        pthread_mutex_lock( &camQMutex );
        qBot++;
        if ( qBot == MAX_CAM_MESSAGES )
        {
            qBot = 0;
        }
        if ( qBot == qTop )
        {
            qEmpty = true;
        }
        pthread_cleanup_pop( 1 );
    }
}

void Camera::decodeMessage( unsigned char* msg )
{
    int sock, camID, i;
    char buf[ 100 ], buf2[ 100 ];

    buf[ 0 ] = 0;
    i = 0;
    sock = msg[ 1 ] & 0x0F;
    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        sprintf( buf, "Camera Msg: " );
        while( msg[ i ] != 0xFF )
        {
            sprintf( buf2, "%2x ", msg[ i++ ] );
            strcat( buf, buf2 );
        }
        printTextWindow( buf );
    }

    if ( msg[ 0 ] > 0x80 )
    {
        if ( msg[ 0 ] == 0x88 )
        {
            camID = 0;
        }
        else
        {
            camID = ( msg[ 0 ] - 0x80 ) >> 4;
        }
        //if (report_level >= HS_REP_LEV_DEBUG) {
        //  sprintf(buf,"Camera [%d]  ",camID);
        //  printTextWindow(buf,false);
        //  buf[0]=0;
        //}
        if ( msg[ 1 ] == 0x01 )
        {
            if ( report_level >= HS_REP_LEV_DEBUG )
            {
                printTextWindow("If_Clear" );
            }
            numjobs = 0;
        }
        if ( msg[ 1 ] == 0x60 )
        {
            if ( report_level >= HS_REP_LEV_ERR )
            {
                strcat( buf, "Error : " );
            }
            switch( msg[ 2 ] )
            {
            case 0x02: 
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    strcat( buf, "syntax error " );
                }
                break;
            case 0x03: 
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    strcat( buf, "command buffer full " );
                }
                break;
            case 0x04: 
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    strcat( buf, "command cancel " );
                }
                break;
            case 0x05: 
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    strcat( buf, "no sockets " );
                }
                break;
            case 0x41: 
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    strcat( buf, "command not executable " );
                }
                break;
            default:
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    sprintf( buf2, "unknown %2x ", msg[ 2 ] );
                    strcat( buf, buf2 );
                }
                break;
            }
            if ( report_level >= HS_REP_LEV_ERR )
            {
                printTextWindow(buf);
            }
        }
        else if ( msg[ 1 ] == 0x50 )
        {
            runCamQ();
            processReply( inqJob, msg );
        }
        else if ( ( msg[ 1 ] > 0x40 ) && ( msg[ 1 ] < 0x50 ) )
        {
            sockets[ sock ] = currentJob;
            if ( report_level >= HS_REP_LEV_DEBUG )
            {
                sprintf( buf2, "command acknowledged %d %s", sock, jobNames[ sockets[ sock ] ] );
                strcat( buf, buf2 );
                printTextWindow( buf );
            }
        } else if ( (msg[ 1 ] > 0x50 ) && ( msg[ 1 ] < 0x60 ) )
        {
            runCamQ();
            if ( report_level >= HS_REP_LEV_DEBUG )
            {
                sprintf( buf2, "command completed %d %s ", sock, jobNames[ sockets[ sock ] ] );
                strcat( buf, buf2 );
                printTextWindow( buf );
            }
            sockets[ sock ] = JOB_IDLE;
            numjobs--;
        }
    }
}

void Camera::getPacket()
{
    unsigned char buf[ 40 ];
    int num;
    num = readMsg( buf );
    decodeMessage( buf );
}

void Camera::moveDeg( float pan, float tilt )    // needs work
{
    int pan_i, tilt_i;
    int pan_deg, tilt_deg;
    static int pan_deg_old = 0, tilt_deg_old = 0;
    pan_deg = (int) pan;
    tilt_deg = (int) tilt;
    if ( ( pan_deg == pan_deg_old ) && ( tilt_deg == tilt_deg_old ) )
    {
        return;
    }
    pan_deg_old = pan_deg;
    tilt_deg_old = tilt_deg;
    
    pan_i = pan_deg * DEG_TO_PAN;
    tilt_i = tilt_deg * DEG_TO_TILT;
    if ( pan_i < PAN_ABS_POS_MIN )
    {
        pan_i = PAN_ABS_POS_MIN;
    }
    else if ( pan_i > PAN_ABS_POS_MAX )
    {
        pan_i = PAN_ABS_POS_MAX;
    }
    if ( tilt_i < TILT_ABS_POS_MIN )
    {
        tilt_i = TILT_ABS_POS_MIN;
    }
    else if ( tilt_i > TILT_ABS_POS_MAX )
    {
        tilt_i = TILT_ABS_POS_MAX;
    }
    panTilt( pan_i, tilt_i );
    goalTilt = tilt_i;
    goalPan = pan_i;
}

static void start_reader_thread( void* camera_instance )
{
    ( (Camera*) camera_instance)->reader_thread();
}

/* Thread to read all repies from camera and send from send que */
void Camera::reader_thread()
{
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    while( true )
    {
        pthread_testcancel();
        getPacket();
        updateStatusBar_();
    }
}

static void start_position_thread( void* camera_instance )
{
    ( (Camera*) camera_instance)->position_thread();
}

/* Thread to place position inqueries on send que */
void Camera::position_thread()
{
    while( true )
    { 
        pthread_testcancel();
        sendCommand( CamPanTiltPosInq, true );
        usleep( 50000 );
        //sendCommand(CamZoomPosInq,true);
        //usleep(50000);
        sendCommand( CamFocusPosInq, true );
        usleep( 50000 );
    }
}

static void start_tracker_thread( void* camera_instance )
{
    ( (Camera*) camera_instance)->tracker_thread();
}

/* Thread that does all camera tracking work */
void Camera::tracker_thread()
{
    int pan_deg, tilt_deg, pan_to, tilt_to;
    int state = 0, numstates = 2;
    int accuracy = 250;
    int dtilt, dpan;
    int last_pan_to = 0, last_tilt_to = 0;
    int rtn;
    int objx, objy, objz, objcolor, maxsize;
    float dx, dy;
    float dxmod = 0.8, dymod = 0.7;

    while( true )
    {
        pthread_testcancel();
        usleep( 100000 );
        pan_deg = (int) ( pan * PAN_TO_DEG );
        tilt_deg = (int) ( tilt * TILT_TO_DEG );
        switch ( trackerMode )
        {
        case TRACKER_MODE_NONE:
            continue;
            break;
        case TRACKER_MODE_CENTER:
            pan_to = 0;
            tilt_to = 0;
            break;
        case TRACKER_MODE_SWEEP_X:
            // camera swings left and right looking straight ahead
            if ( state % 2 )
            {
                pan_to = PAN_ABS_POS_MIN;
            }
            else
            {
                pan_to = PAN_ABS_POS_MAX;
            }
            tilt_to = 0;
            numstates = 2;
            accuracy = 250;
            break;
        case TRACKER_MODE_SWEEP_X_HIGH:
            // camera swings left and right looking up
            if ( state % 2 )
            {
                pan_to = PAN_ABS_POS_MIN;
            }
            else
            {
                pan_to = PAN_ABS_POS_MAX;
            }
            tilt_to = TILT_ABS_POS_MAX;
            numstates = 2;
            accuracy = 250;
            break;
        case TRACKER_MODE_SWEEP_X_LOW:
            // camera swings left and right looking down
            if ( state % 2 )
            {
                pan_to = PAN_ABS_POS_MIN;
            }
            else
            {
                pan_to = PAN_ABS_POS_MAX;
            }
            tilt_to = TILT_ABS_POS_MIN;
            numstates = 2;
            accuracy = 250;
            break;
        case TRACKER_MODE_SWEEP_Y:
            // camera swings up and down facing center
            if ( state % 2 )
            {
                tilt_to = TILT_ABS_POS_MIN;
            }
            else
            {
                tilt_to = TILT_ABS_POS_MAX;
            }
            pan_to = 0;
            numstates = 2;
            accuracy = 50;
            break;
        case TRACKER_MODE_SWEEP_Y_LEFT:
            // camera swings up and down facing left
            if ( state % 2 )
            {
                tilt_to = TILT_ABS_POS_MIN;
            }
            else
            {
                tilt_to = TILT_ABS_POS_MAX;
            }
            pan_to = PAN_ABS_POS_MIN;
            numstates = 2;
            accuracy = 50;
            break;
        case TRACKER_MODE_SWEEP_Y_RIGHT:
            // camera swings up and down facing right
            if ( state % 2 )
            {
                tilt_to = TILT_ABS_POS_MIN;
            }
            else
            {
                tilt_to = TILT_ABS_POS_MAX;
            }
            pan_to = PAN_ABS_POS_MAX;
            numstates = 2;
            accuracy = 50;
            break;
        case TRACKER_MODE_SWEEP_XY:
            switch ( state )
            {
            case 0: 
                pan_to = PAN_ABS_POS_MIN;
                tilt_to = 0;
                break;
            case 1:
                pan_to = PAN_ABS_POS_MAX;
                tilt_to = 0;
                break;
            case 2:
                pan_to = PAN_ABS_POS_MAX;
                tilt_to = TILT_ABS_POS_MAX;
                break;
            case 3:
                pan_to = PAN_ABS_POS_MIN;
                tilt_to = TILT_ABS_POS_MAX;
                break;
            default:
                pan_to = 0;
                tilt_to = 0;
                break;
            }
            numstates = 4;
            accuracy = 250;
            break;
        case TRACKER_MODE_LARGEST_OBJECT:
        case TRACKER_MODE_LARGEST_A:
        case TRACKER_MODE_LARGEST_B:
        case TRACKER_MODE_LARGEST_C:
            if ( cognachrome == NULL )
            {
                break;
            }
            maxsize = 0;
            if ( trackerMode == TRACKER_MODE_LARGEST_OBJECT )
            {
                rtn = cognachrome->largestObject( -1, &objx, &objy, &objz );
                if ( !rtn )
                {
                    maxsize = objz; 
                }
            }
            else
            {
                objcolor = trackerMode - TRACKER_MODE_LARGEST_A;
                rtn = cognachrome->largestObject( objcolor, &objx, &objy, &objz );
                if ( !rtn )
                {
                    maxsize = objz; 
                }
            }
            if ( maxsize == 0 )
            {
                pan_to = 0;
                tilt_to = 0;
            }
            else
            {
                dx = CHROME_MAX_X / 2.0 -objx;
                dy = CHROME_MAX_Y / 2.0 -objy;
                dx *= ( CAMERA_FOV_X / CHROME_MAX_X );
                dy *= ( CAMERA_FOV_Y / CHROME_MAX_Y );
                dx *= dxmod;
                dy *= dymod;
                pan_to = pan + (int) ( dx * DEG_TO_PAN );
                tilt_to = tilt + (int)( dy * DEG_TO_TILT );
                accuracy = 25;
            }
            break;
        case TRACKER_MODE_CLOSEST_LASER:
            if ( Laser::m_clLaserList.size() > 0 )
            {
                float fAngle;
                int iMinReading;
                Laser::OverallMinReading( iMinReading, fAngle );
                pan_to = (int) fAngle * DEG_TO_PAN;
                if ( pan_to > PAN_ABS_POS_MAX )
                {
                    pan_to = PAN_ABS_POS_MAX;
                }
                if ( pan_to < PAN_ABS_POS_MIN )
                {
                    pan_to = PAN_ABS_POS_MIN;
                }
                tilt_to = 0;
                accuracy = 10;
            }
            break;
        }
    
        dpan = pan_to - pan;
        dtilt = tilt_to - tilt;

        goalTilt = tilt_to;
        goalPan = pan_to;

        if ( ( abs( dpan ) < accuracy ) && ( abs( dtilt ) < accuracy ) )
        {
            state = ( state + 1 ) % numstates;
            continue;
        }
        else
        {
            rtn = panTilt( pan_to, tilt_to );
            if ( !rtn )
            {
                last_pan_to = pan_to;
                last_tilt_to = tilt_to;
            }
        }
    }
}

// setup camera serial port
bool Camera::setAttr( int fd )
{
    struct  termios* term_st;
    int     rc;
  
    if ( fd > 0 )
    {
        term_st = (struct termios*) malloc ( sizeof( struct termios ) );
        rc = tcgetattr( fd, term_st );

        if ( rc == -1 )
        {
            //fprintf(stderr,"Error getting attr\n" );
            //perror ("tcgetattr error" );
            return FALSE;
        }

        /* Setting mode to Raw */

        term_st->c_iflag &= ~( IGNBRK | BRKINT | PARMRK | ISTRIP
                               | INLCR | IGNCR | ICRNL | IXON);
        term_st->c_oflag &= ~( OPOST | ONLCR );
        term_st->c_lflag &= ~( ECHO | ECHONL | ICANON | ISIG | IEXTEN );
       
        // set MIN and TIME for non-canonical mode input processing
        // MIN = 0 and TIME > 0 such that reads complete as soon
        // as a character is read, or the timer expires. TIME is
        // units of 0.10 seconds

        term_st->c_cc[ VMIN ] = 0;
        term_st->c_cc[ VTIME ] = 10;  /* equals 0.5 second */

        // set speed to 9600 baud, 8 databits, even parity enabled
        // 1 stop bit 1 start bit

        term_st->c_cflag = ( ~B300 & term_st->c_cflag );
        term_st->c_cflag = ( B9600 | term_st->c_cflag );


        rc = tcsetattr( fd, TCSANOW, term_st );
        if ( rc == -1 )
        {
            fprintf( stderr, "Error setting attr\n" );
            perror ( "tcsetattr error" );
            return FALSE;
        }
        else
        {       
            rc = tcgetattr( fd, term_st );  // Set terminal attributes
        }

        free( term_st );

        return TRUE;

    }

    return FALSE;
}

void Camera::addSensor()
{
    partCamera = new RobotPartPanTilt( 0, CAMERA_FOV_X, CAMERA_FOV_Y );
    sensorCamera = new Sensor( SENSOR_PANTILT_CAMERA, 0, NULL, 4, current_readings );
}
 
Camera::Camera( Camera** a, const string& strPort ) : 
    Module( (Module**) a, HS_MODULE_NAME_CAMERA)
{
    int i;

    qTop = 0;
    qBot = 0;
    qEmpty = true;
    haveMove = false;
    numjobs = 0;
    powerOn = true;
    md_mode = false;
    updateSpeed = 0;
    trackerMode = TRACKER_MODE_NONE;
    threads_running = false;
    sensorCamera = NULL;
    partCamera = NULL;

    pthread_mutex_init( &camQMutex, NULL );

    printfTextWindow( "camera serial port: %s", strPort.c_str() );
    printTextWindow( "Testing camera connection... ", false );
    if ( ( fd = open( strPort.c_str(), O_RDWR , 0 ) ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printfTextWindow( "Camera Error opening serial port %s", strPort.c_str() );
        }
        delete this;
        return;
    }
    setAttr( fd );
    sendCommand( CamIFClearB );
    sendCommand( CamSetAddressB );
    for( i = 0; i < 4; i++ )
    {
        current_readings[ i ] = 0.0;
    }
    sendCommand( CamPTDHome );
    sendCommand( CamBackLightOff ); 
    threads_running = true;
    pthread_create( &reader, NULL,(void * (*)(void *)) &start_reader_thread,(void*) this );
    pthread_create( &position, NULL,(void * (*)(void *)) &start_position_thread,(void*) this );
    pthread_create( &tracker, NULL,(void * (*)(void *)) &start_tracker_thread,(void*) this );
    printTextWindow( "connected" );
    updateStatusBar_();
    addSensor();
}

Camera::~Camera(){
    if ( threads_running )
    {
        pthread_cancel( position );
        pthread_join( position,NULL );
        pthread_cancel( tracker );
        pthread_join( tracker,NULL );
        pthread_cancel( reader );
        pthread_join( reader, NULL );
        pthread_mutex_destroy( &camQMutex );
        close( fd );
    }
    delete sensorCamera;
    delete partCamera;
    printTextWindow( "Camera disconnected" );
}


void Camera::control()
{
    int c, ct, cs;
    int done = 0, done_track_select = 0;
    int msgData[ 1 ];

    msgData[ 0 ] = powerOn;

    messageDrawWindow( EnMessageType_CAMERA_CONTROL, EnMessageErrType_NONE, msgData );
    do
    {
        c = getch();
        switch( c )
        {
        case 'd':
            messageHide();
            delete this;
            return;
            break;
        case 't':
            done_track_select = 0;
            while( !done_track_select )
            {
                ct = messageGetChar( EnMessageFilterType_TRACKING, EnMessageType_TRACKING, 
                                     EnMessageErrType_TRACKING, NULL, false );
                if ( ct == 'a' )
                {
                    trackerMode = TRACKER_MODE_CLOSEST_LASER;
                    done_track_select = 1;
                }
                else
                {
                    trackerMode = ct-'1';
                }

                // If the users specified TRACKER_MODE_SWEEP_X, 
                // ask them what angle (high, center, low) the
                // camera should be.
                switch ( trackerMode )
                {
                case TRACKER_MODE_SWEEP_X:
                    cs = messageGetChar( EnMessageFilterType_CAMERA_SWEEP_X, 
                                         EnMessageType_CAMERA_SWEEP_X, 
                                         EnMessageErrType_CAMERA_SWEEP_X, NULL, false );
                    messageDrawWindow( EnMessageType_TRACKING, EnMessageErrType_TRACKING, msgData );
                    switch ( cs )
                    {
                    case 'h':
                        trackerMode = TRACKER_MODE_SWEEP_X_HIGH;
                        break;
                    case 'c':
                        trackerMode = TRACKER_MODE_SWEEP_X;
                        break;
                    case 'l':
                        trackerMode = TRACKER_MODE_SWEEP_X_LOW;
                        break;
                    }
                    break;

                    // If the users specified TRACKER_MODE_SWEEP_Y, 
                    // ask them what angle (left, center, right) the
                    // camera should be.
                case TRACKER_MODE_SWEEP_Y:
                    cs = messageGetChar( EnMessageFilterType_CAMERA_SWEEP_Y, 
                                         EnMessageType_CAMERA_SWEEP_Y, 
                                         EnMessageErrType_CAMERA_SWEEP_Y, NULL, false );
                    messageDrawWindow( EnMessageType_TRACKING, EnMessageErrType_TRACKING, msgData );
                    switch ( cs )
                    {
                    case 'l':
                        trackerMode = TRACKER_MODE_SWEEP_Y_LEFT;
                        break;
                    case 'c':
                        trackerMode = TRACKER_MODE_SWEEP_Y;
                        break;
                    case 'r':
                        trackerMode = TRACKER_MODE_SWEEP_Y_RIGHT;
                        break;
                    }
                    break;
                default:
                    done_track_select = 1;
                    break;
                }
            }
            messageDrawWindow( EnMessageType_CAMERA_CONTROL, EnMessageErrType_NONE, msgData );
            break;
        case 'r':
            refreshScreen();
            break;
        case 'c':
            panTilt( 0, 0 );
            break;
        case 'p':
            powerOn = !powerOn;
            msgData[ 0 ] = powerOn;
            power( powerOn );
            messageDrawWindow( EnMessageType_CAMERA_CONTROL, EnMessageErrType_NONE, msgData );
            break;
        case 'm':
            md_mode = !md_mode;
            MDModeOnOff( md_mode );
            break;
        case 'x':
        case 'Q':
        case KEY_ESC:
            done = true;
            break;
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case KEY_UP:
        case KEY_DOWN:
        case KEY_LEFT:
        case KEY_RIGHT:
            messageMovePanelKey( c );
            break;
        }
    } while ( !done );
    messageHide();
}

void cameraConnect()
{
    unsigned const int uPORT_ECHO_LINE = 4;

    // get the connection port from the user
    string strPort = messageGetString( EnMessageType_LASER, uPORT_ECHO_LINE );

    new Camera( &camera, strPort );
}
  
/**********************************************************************
# $Log: camera.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.6  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.5  2001/03/23 21:27:12  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 22:48:09  blee
# Changed the case for TRACKER_MODE_CLOSEST_LASER in tracker_thread()
# to access the laser differently. Also changed some #defines to
# enumerated types for hserver's user interface.
#
# Revision 1.3  2000/09/19 03:07:52  endo
# More sweep mode added.
#
#
#**********************************************************************/
