/**********************************************************************
 **                                                                  **
 **                           cognachrome.c                          **
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

/* $Id: cognachrome.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>

#include "HServerTypes.h"
#include "hserver.h"
#include "cognachrome.h"
#include "message.h"
#include "camera.h"
#include "robot_config.h"
#include "hserver.h"
#include "sensors.h"

Cognachrome *cognachrome = NULL;

#define COG_TRACKING_START 0xFF

#define SAPHIRA_VISIONIO_COM  0x64
#define SAPHIRA_VISIONIOpac   0x80
#define SAPHIRA_ARGSTR 0x2B

#define COG_BUF_SIZE 1024

#define COG_MODE_NONE          0
#define COG_MODE_RETRAINING    1
#define COG_MODE_ADDING        2
#define COG_MODE_SAVING        3
#define COG_MODE_RESTORING     4
#define COG_MODE_SENDING       5
#define COG_MODE_INITIALIZING  6
#define COG_MODE_TRACKING      7

#define COG_DEFAULT_FILTER   2

const int Cognachrome::INITIAL_OBJECT_SIZE = 10;

int tpuvislex( char** text, int* len );
void tpuvisrestart( FILE* input_file );

static char channel_protocol[ 4 ][ 20 ] = 
{
    "\\377",
    "a%wC%wR%wN",
    "b%wC%wR%wN",
    "c%wC%wR%wN"
};

void Cognachrome::updateStatusBar_(void)
{
    pthread_cleanup_push( (void(*)(void*)) pthread_mutex_unlock, (void *) &statusMutex );
    pthread_mutex_lock( &statusMutex );
    char buf[ 100 ];
    int i, j;
    statusStr[ 0 ] = 0;
    trackStatusStr[ 0 ] = 0;
    strcat( statusStr, "Cognachrome: " );
    if (!connected)
    {
        strcat( statusStr, "Not connected " );
        statusbar->update(statusLine);
    } 
    else 
    { 

        sprintf(
            buf,
            "%c Channel %d  F %d  S %d  R %d  T %d  ",
            statusbarSpinner_->getStatus(),
            channel,
            filter, 
            pendingSends,
            total_receive,
            total_transmit);

        strcat(statusStr,buf);
        switch(mode){
        case COG_MODE_NONE:
            break;
        case COG_MODE_RETRAINING:
            strcat( statusStr, "Retraining  " );
            break;
        case COG_MODE_ADDING:
            strcat( statusStr, "Adding  " );
            break;
        case COG_MODE_SAVING:
            strcat( statusStr, "Saving  " );
            break;
        case COG_MODE_RESTORING:
            strcat( statusStr, "Restoring  " );
            break;
        case COG_MODE_SENDING:
            strcat( statusStr, "Sending  " );
            break;    
        case COG_MODE_TRACKING:
            break;
        case COG_MODE_INITIALIZING:
            break;    
        }
  
        statusbar->update( statusLine );
  
        if ( isTracking )
        {
            trackStatusStr[ 0 ] = 0;
            strcat( trackStatusStr, "Tracking:  " );
            for( i = 0; i < 3; i++ )
            {
                if ( numobjects[ i ] )
                {
                    if ( i == 0 )
                    {
                        strcat( trackStatusStr, "a " );
                    }
                    else if ( i == 1 )
                    {
                        strcat( trackStatusStr, "b " );
                    }
                    else if ( i == 2 )
                    {
                        strcat( trackStatusStr, "c " );
                    }
                    for( j = 0; j < numobjects[ i ]; j++ )
                    {
                        sprintf( buf, "(%d,%d,%d) ", colorobjects[ i * max_objects * 3 + j * 3 ], 
                                 colorobjects[ i * max_objects * 3 + j * 3 + 1 ], 
                                 colorobjects[ i * max_objects * 3 + j * 3 + 2 ] );
                        strcat( trackStatusStr,buf);
                    }
                }
            }
            trackStatusStr[ screenX - 2 ] = 0;
            statusbar->update( trackStatusLine );
        }
    }
    pthread_cleanup_pop( 1 );
}



int Cognachrome::largestObject( int color, int* x, int* y, int* z)
{
    int i;
    int maxsize = 0;
    int objcolor;

    if ( color == -1)
    {
        for( i = 0; i < 3; i++ )
        {
            if ( numobjects[ i ] > 0 )
            {
                if ( maxsize < colorobjects[ i * max_objects * 3 + 2 ] )
                {
                    objcolor = i;
                    maxsize = colorobjects[ i * max_objects * 3 + 2 ];
                }
            }
        }
        if ( maxsize == 0 )
        {
            return -1;
        }
        *x = colorobjects[ objcolor *max_objects * 3 ];
        *y = colorobjects[ objcolor *max_objects *3 + 1 ];
        *z = colorobjects[ objcolor *max_objects *3 + 2 ];
        return 0;
    }
    if ( numobjects[ color ] == 0 )
    {
        return -1;
    }
    *x = colorobjects[ color * max_objects * 3 ];
    *y = colorobjects[ color * max_objects * 3 + 1 ];
    *z = colorobjects[ color * max_objects * 3 + 2 ];
    return 0;
}

/* Add message to sens que */
int Cognachrome::postSend( char* msg )
{
    pthread_cleanup_push( (void(*)(void*)) pthread_mutex_unlock, (void*) &sendQueMutex );
    pthread_mutex_lock( &sendQueMutex );
    sendQue[ sendQueLast ] = strdup( msg );
    sendQueLast = ( sendQueLast + 1 ) % send_que_max;
    pendingSends++;
    pthread_cleanup_pop( 1 );
    updateStatusBar_();
    return 0;
}  

/* Update angles and ranges for readings */
void Cognachrome::updateSensor()
{
    int i, j, idx;
    int x, y, size;
    float range;
    float fangle = 0;


    idx = 0;
    for( i = 0; i < 3; i++ )
    {
        extra_sensor_cog.num_per_channel[ i ] = 0;
        for(j = 0; j < numobjects[ i ]; j++ )
        {
            x = colorobjects[ i * max_objects * 3 + j * 3 ];
            y = colorobjects[ i * max_objects * 3 + j * 3 + 1 ];
            size = colorobjects[i*max_objects * 3 + j * 3 + 2 ];
            if ( size < min_blob_size )
            {
                continue;
            }
            range = sizeobjects[ i ] / sqrt( size );
            if ( camera != NULL)
            {
                fangle = camera->getPan() * PAN_TO_DEG;
            }
            else
            {
                fangle = 0;
            }
            fangle += ( ( CHROME_MAX_X / 2.0 ) - x ) / ( CHROME_MAX_X / 2.0 ) * ( CAMERA_FOV_X / 2.0 );
            angle[ idx ] = fangle;
            reading[ idx ] = range;
            extra_sensor_cog.num_per_channel[ i ] += 1;
            idx++;
        }
    }

    sensorCog->setAngleInfo( angle, idx );
    sensorCog->setReadingInfo( reading, idx );
}
    
/* Send str to particular cog stream */
void Cognachrome::sendStream( int stream, char* str, int nobreak )
{
    static byte_t buf[ COG_BUF_SIZE ];

    buf[ 0 ] = COG_STREAM_BREAK;
    buf[ 1 ] = stream;
    strcpy( (char*) &buf[ 2 ], str );
    if ( nobreak)
    {
        write( fd, &buf[ 2 ],strlen( str ) );
    }
    else
    {
        write( fd, buf, strlen( str ) + 2 );
    }
}

void Cognachrome::startTracking()
{
    postSend( "r" );
    isTracking = true;
    updateStatusBar_();
}

void Cognachrome::stopTracking()
{
    sendStream( 3, "q" );
    isTracking = false;
    updateStatusBar_();
}

int Cognachrome::setChannel( int channel )
{
    char buf[ 10 ];
    char chanc;

    switch( channel )
    {
    case 0:
        chanc = 'a';
        break;
    case 1:
        chanc = 'b';
        break;
    case 2:
        chanc = 'c';
        break;
    }

    sprintf( buf, "a%c", chanc );
    sendStream( 3, buf );
    return 0;
}

int Cognachrome::setFilter( int filter )
{
    char buf[ 10 ];
    char chanc;

    chanc = filter + '0';
    sprintf( buf, "f%c\n", chanc );
    sendStream( 3, buf );
    return 0;
}

/* Send to tpuvis */
int Cognachrome::tpuvisCommand( char command )
{
    char buf[ 2 ] = { 0, 0 };
    buf[ 0 ] = command;
    sendStream( 3, buf );
    return 0;
}  
 
int Cognachrome::calcSaphiraChksum( byte_t* ptr )
{
    int n;
    int c = 0;
    n = *(ptr++);
    n -= 2;
    while ( n > 1 )
    {
        c += ( *(ptr) << 8 ) | *( ptr + 1 );
        c = c & 0xffff;
        n -= 2;
        ptr += 2;
    }
    if ( n > 0 )
    {
        c = c ^ (int) *(ptr++);
    }
    return c;
}

void Cognachrome::sendSaphiraPacket( int fd, char* str, int len )
{
    byte_t pack[ 255 ];
    int i, chk;

    pack[ 0 ] = 0xfa;
    pack[ 1 ] = 0xfb;
    pack[ 2 ] = 5 + len;
    pack[ 3 ] = SAPHIRA_VISIONIO_COM;
    pack[ 4 ] = SAPHIRA_ARGSTR;
    pack[ 5 ] = len;
    for( i = 0; i < len; i++ )
    {
        pack[ 6 + i ] = str[ i ];
    }
    chk = calcSaphiraChksum( &pack[ 2 ] );
    pack[ 6 + len ] = (chk & 0x0000ff00) >> 8;
    pack[ 7 + len ] = chk & 0x000000ff;
    write( fd, pack, pack[ 2 ] + 3 );
}

void Cognachrome::setProtocolString()
{
    postSend( "ph" );
    postSend( "pa" );
    postSend( "pb" );
    postSend( "pc" );
    postSend( "n" );
}

void Cognachrome::setupTpuvis()
{
    setFilter( filter );
    setProtocolString();
    startTracking();
}

int Cognachrome::readCog()
{
    static byte_t buf[ COG_BUF_SIZE ];
    int cnum, i;
    byte_t sbuf[ 2 ];
    static int numcalls = 0;
    int maxcalls = 30;
    int rtn;

    if ( !baudUp )
    {
        numcalls++;
        if ( numcalls > maxcalls )
        {
            close( fd );
            rtn = setupSerial( &fd, port_str.c_str(), B38400, HS_REP_LEV_ERR, "Cog", 1 );
            if ( rtn )
            {
                return -1;
            }
            sendStream( 3, "q" );
            baudUp = true;
        }
    }
    cnum = read( fd, buf, COG_BUF_SIZE );
    if ( cnum == -1 )
    {
        printTextWindow( "Cog Error read" );
        cnum = 0;
    }
    if ( cnum==0 )
    {
        return 0;
    }
    buf[ cnum ] = 0;
    for( i = 0; i < cnum; i++ )
    {
        if ( buf[ i ] == COG_STREAM_BREAK )
        {
            stream_num = buf[ i + 1 ];
            if ( !baudUp )
            {
                if ( stream_num == 0 )
                {
                    sbuf[ 0 ] = '\n';
                    write( fd, sbuf, 1 );
                    sendStream( 0, "baud 38400\n" );
                }
            } 
            if ( baudUp && !connected )
            {
                connected = true;
                printTextWindow( "Cognachrome connected at 38400 baud" );
                setupTpuvis();
            }
            i++;
        }
    }
    return cnum;
}

int Cognachrome::lexRead()
{
    int cnum;
    int c;

    do {
        cnum = read( fd, &c, 1 );
        if ( cnum == -1 )
        {
            printTextWindow( "CogLexRead read error" );
            cnum = 0;
        }
        if ( cnum == 0 )
        {
            //printTextWindow( "CogLexRead read empty error" );
        }
    } while ( !cnum );
    return c;
}

int Cognachrome::sendCog()
{
    int len;
    pthread_cleanup_push( (void(*)(void*)) pthread_mutex_unlock, (void *) &sendQueMutex );
    pthread_mutex_lock( &sendQueMutex );
    sendStream( 3, sendQue[ sendQueFirst ] );
    len = strlen( sendQue[ sendQueFirst ] );
    free( sendQue[ sendQueFirst ] );
    pendingSends--;
    pthread_cleanup_pop( 1 );
    updateStatusBar_();
    sendQueFirst = ( sendQueFirst + 1 ) % send_que_max;
    if ( isTracking )
    {
        mode = COG_MODE_TRACKING;
    }
    else
    {
        mode = COG_MODE_SENDING;
    }
    return len;
}

// Handle output of lexer in tpuvis.yy
void Cognachrome::useTpuvisCommand( int rtn, unsigned char* text, int len )
{
    char buf[ 100 ];
    int i, index, color;
    static struct timeval oldtime, nowtime;
    static int firsttime = true;
    static long timediff = 0;

    if ( firsttime )
    {
        firsttime = false;
        gettimeofday( &oldtime, NULL );
    }
    gettimeofday( &nowtime, NULL );
    timediff = ( nowtime.tv_sec - oldtime.tv_sec ) * 1000000 + 
        ( nowtime.tv_usec - oldtime.tv_usec );
    //sprintf( rstr, "time %ld %ld %ld %ld %ld", nowtime.tv_sec, nowtime.tv_usec, 
    //         oldtime.tv_sec, oldtime.tv_usec, timediff );
    //printTextWindow(rstr);
    switch ( rtn )
    {
    case COG_LEX_ADDING:
        mode = COG_MODE_ADDING;
        updateStatusBar_();
        break;
    case COG_LEX_SAVING:
        mode = COG_MODE_SAVING;
        updateStatusBar_();
        break;
    case COG_LEX_PROMPT:
        mode = COG_MODE_NONE;
        channel = text[ 8 ] - 'a';
        updateStatusBar_();
        break;
    case COG_LEX_RETRAINING:
        mode = COG_MODE_RETRAINING;
        updateStatusBar_();
        break;
    case COG_LEX_PROTOCOL_HEADER:
        sprintf( buf, "%s\n", channel_protocol[ 0 ] );
        sendStream( 3, buf );
        break;
    case COG_LEX_PROTOCOL_ABC:
        sprintf( buf, "%s\n", channel_protocol[ text[ len - 2 ] - 'A' + 1 ] );
        sendStream( 3, buf );
        break;
    case COG_LEX_TRACKING:
        oldtime = nowtime;
        for( i = 0; i < 3; i++ )
        {
            numobjects[i]=0;
        }
        if ( len > 4 )
        {
            for( i = 0; i < ( len - 1 ) / 7; i++ )
            {
                color = text[ 1 + i * 7 ] - 'a';
                index = 1 + i * 7;
                colorobjects[ color * max_objects * 3 + numobjects[ color ] * 3 ] = 
                    text[ index + 2 ] + text[ index + 1 ] * 254;
                colorobjects[ color * max_objects * 3 + numobjects[ color ] * 3 + 1 ] = 
                    text[ index + 4 ] + text[ index + 3 ] * 254;
                colorobjects[ color * max_objects * 3 + numobjects[ color ] * 3 + 2 ] = 
                    text[ index + 6 ] + text[ index + 5 ] * 254;
                numobjects[ color ]++;
            }
        }
        updateSensor();
        break;
    case COG_LEX_CHANNEL:
        stream_num = text[ 1 ];
        break;
    case COG_LEX_NUM_OBJECTS:
        if ( text[ 1 ] == 'i' )
        {
            sendStream( 3, "\n" );
            break;
        }
        sprintf( buf, "%d\n", maxnumobjects[ text[ 29 ] - 'A' ] ); 
        sendStream( 3, buf );
    }
}

static void start_reader_thread( void* cog_instance )
{
    ( (Cognachrome*) cog_instance)->reader_thread();
}

// Thread to handle all reads to the cog
void Cognachrome::reader_thread()
{
    int rtn,len;
    char* text;
    int count = 10;

    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );

    while( true )
    {
        pthread_testcancel();
        if ( !connected )
        {
            rtn = readCog();
            pthread_testcancel();
            if ( !rtn )
            {
                continue;
            }
            total_receive += rtn;
            updateStatusBar_();
        }
        else
        {
            rtn = tpuvislex( &text, &len );
            pthread_testcancel();
            useTpuvisCommand( rtn, (unsigned char*) text, len );
            total_receive += len;
            if ( rtn == COG_LEX_CHANNEL );//statusbarCogUpdate();
            else if ( count-- == 0 )
            {
                count = 10;
                updateStatusBar_();
            }
        }
    }
}

static void start_sender_thread( void* cog_instance )
{
    ( (Cognachrome*) cog_instance)->sender_thread();
}


/* Thread to send from send que */
void Cognachrome::sender_thread()
{
    int rtn;
    pthread_setcanceltype( PTHREAD_CANCEL_ASYNCHRONOUS, NULL );
    while( true )
    {
        usleep( 10000 );
        pthread_testcancel();
        if ( ( sendQueFirst == sendQueLast ) || ( mode != COG_MODE_NONE ) )
        {
            continue;
        }
        rtn = sendCog();
        total_transmit += rtn;
        if ( connected && rtn )
        {
            updateStatusBar_();
        }
    }
}

void Cognachrome::addSensor()
{
    partCog = new RobotPartCognachrome( 3, 3 );
    sensorCog = new Sensor( SENSOR_COGNACHROME, sizeof( extra_sensor_cog_t ), 
                            (char*) &extra_sensor_cog, 0, angle, 0, reading );
}


void Cognachrome::control()
{
    int msgData[ 10 ];
    int c, i;
    bool done = false;

    for( i = 0; i < 3; i++ )
    {
        msgData[ i ] = sizeobjects[ i ];
    }

    messageDrawWindow( EnMessageType_COG_CONTROL, EnMessageErrType_NONE, msgData );

    do
    {
        c = wgetch( msgWindow );
        switch( c )
        {
        case 'x':
        case 'Q':
        case KEY_ESC:
            done = true;
            break;
        case 'd':
            messageHide();
            delete this;
            return;
            break;
        case 'a':
            setChannel( 0 );
            break;
        case 'b':
            setChannel( 1 );
            break;
        case 'c':
            setChannel( 2 );
            break;
        case 't':
            tpuvisCommand( 's' );
            break;
        case 'r':
            refreshScreen();
            break;
        case 'i':
            tpuvisCommand( 'j' );
            break;
        case '>':
        case '<':
        case 'S':
            tpuvisCommand( c );
            break;
        case 'R':
            tpuvisCommand( c );
            mode = COG_MODE_RESTORING;
            updateStatusBar_();
            break;
        case '+':
        case '=':
            filter += 1;
            filter %= 4;
            setFilter( filter );
            updateStatusBar_();
            break;
        case '-':
            filter -= 1;
            if ( filter == -1 )
            {
                filter = 3;
            }
            setFilter( filter );
            updateStatusBar_();
            break;
        case 'h':
            crosshairs = !crosshairs;
            if ( crosshairs )
            {
                tpuvisCommand( '+' );
            }
            else
            {
                tpuvisCommand( '-' );
            }
            break;
        case 'z':
            isTracking = !isTracking;
            if ( isTracking )
            {
                startTracking();
            }
            else
            {
                stopTracking();
            }
            break;
        case 'p':
            setProtocolString();
            break;
        case '[':
            sizeobjects[ channel ]--;
            for( i = 0; i < 3; i++ )
            {
                msgData[ i ] = sizeobjects[ i ];
            }
            messageDrawWindow( EnMessageType_COG_CONTROL, EnMessageErrType_NONE, msgData );
            break;
        case ']':
            sizeobjects[ channel ]++;
            for( i = 0; i < 3; i++ )
            {
                msgData[ i ] = sizeobjects[ i ];
            }
            messageDrawWindow( EnMessageType_COG_CONTROL, EnMessageErrType_NONE, msgData);
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
  
    hide_panel( msgPanel );
    return;
}



Cognachrome::~Cognachrome()
{
    if ( threads_running )
    {
        pthread_cancel( reader );
        pthread_join( reader, NULL );
        pthread_cancel( sender );
        pthread_join( sender, NULL );
    }
    if ( fd )
    {
        close(fd);
    }
    tpuvisrestart( NULL );
    delete sensorCog;
    delete partCog;
    delete [] angle;
    delete [] reading;
    delete [] sendQue;
    delete [] colorobjects;
    delete [] numobjects;
    delete [] maxnumobjects;
    statusbar->remove( trackStatusLine );
    delete [] trackStatusStr;
    printTextWindow( "Cognachrome disconnected" );
}

Cognachrome::Cognachrome( Cognachrome** a, const string& strPortString ) : 
    Module((Module**)a, HS_MODULE_NAME_COGNACHROME)
{
    int rtn, i;
    pthread_mutex_init( &sendQueMutex,NULL );
    pthread_mutex_init( &statusMutex,NULL );
    fd = 0;
    trackStatusStr = new char[ statusLineLen ];
    trackStatusStr[ 0 ] = 0;
    trackStatusLine = statusbar->add( trackStatusStr );
    connected = false;
    sendQueFirst = sendQueLast = pendingSends = 0;
    min_blob_size = 6;
    max_objects = 5;
    max_objects_per_color = 3;
    num_colors = 3;
    send_que_max = 10;
    sendQue = new char*[ send_que_max ];
    angle = new float[ max_objects * max_objects_per_color ];
    reading = new float[ max_objects * max_objects_per_color ];
    colorobjects = new int[ num_colors * max_objects * 3 ];
    sizeobjects = new int[ num_colors ];
    for( i = 0; i < num_colors; i++ )
    {
        sizeobjects[i] = INITIAL_OBJECT_SIZE;
    }

    numobjects = new int[ num_colors ];
    for( i = 0; i < num_colors; i++ )
    {
        numobjects[i] = 0;
    }
    maxnumobjects = new int[ num_colors ];
    for (i = 0; i < num_colors; i++)
    { 
	maxnumobjects[i] = max_objects_per_color;
    }

    stream_num = -1;
    channel = 2;
    mode = COG_MODE_INITIALIZING;
    filter = COG_DEFAULT_FILTER;
    total_receive = total_transmit = 0;
    isTracking = crosshairs = false;
    baudUp = false;
    threads_running = true;
    port_str = strPortString;
    updateStatusBar_();
    rtn = setupSerial( &fd, port_str.c_str(), B9600, HS_REP_LEV_ERR, "Cog", 1 );
    if ( rtn )
    {
        printTextWindow( "cog failed" );
        delete this;
        return;
    };
    sendSaphiraPacket( fd, "q", 1 );
    sendSaphiraPacket( fd, "q", 1 );
    pthread_create( &reader, NULL, (void * (*)(void *))*start_reader_thread, (void*) this );
    pthread_create( &sender, NULL, (void * (*)(void *))*start_sender_thread, (void*) this );
    threads_running = true;
    addSensor();
    updateStatusBar_();
}

void cognachromeConnect()
{
    unsigned const int uPORT_ECHO_LINE = 5;

    // get the connection port from the user
    string strPort = messageGetString( EnMessageType_COG_PORT, uPORT_ECHO_LINE );

    new Cognachrome( &cognachrome, strPort );
}

/**********************************************************************
# $Log: cognachrome.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:41:05  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.7  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.6  2001/12/22 16:35:24  endo
# RH 7.1 porting.
#
# Revision 1.5  2001/03/23 21:28:40  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 22:50:59  blee
# Changed updateSensor() to alter the setting of the angle and
# reading info. Also changed some #defines to enumerated types
# for hserver's user interface.
#
# Revision 1.3  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
