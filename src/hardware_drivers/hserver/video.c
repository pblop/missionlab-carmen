/**********************************************************************
 **                                                                  **
 **                              video.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Video framegrabber control for HServer                          **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: video.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "HServerTypes.h"
#include "hserver.h"
#include "message.h"
#include "statusbar.h"
#include "video.h"
#include "xwindow.h"
#include "robot_config.h"
#include "sensors.h"

Video* video = NULL;

void Video::updateStatusBar_(void)
{
    char buf[ 100 ];
    statusStr[ 0 ] = 0;
  
    strcat( statusStr, "Video: " );

    sprintf( buf, "%c Chan %d", channel, statusbarSpinner_->getStatus() );

    strcat( statusStr, buf );
    if ( streaming )
    {
        strcat( statusStr, " Streaming" );
    }
    if ( capturing )
    {
        strcat( statusStr, " Capturing" );
    }
    statusbar->update( statusLine );
}

int Video::initV4Linux( int height, int width )
{
    int i;

    this->height = height;
    this->width = width;

    fd = open( strPortString.c_str(), O_RDWR );
    if ( fd <= 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video device open error" );
        }
        return -1;
    }
  
    if ( ioctl( fd, VIDIOCGCAP, &vid_cap ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video VIDIOCGCAP error" );
        }
        return -1;
    }
  
    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        printfTextWindow( "Video Capture Device Name: %s", vid_cap.name );
        printfTextWindow( "   channels %d  audios %d  width %d %d height %d %d", 
                          vid_cap.channels, vid_cap.audios, vid_cap.minwidth, vid_cap.maxwidth, 
                          vid_cap.minheight, vid_cap.maxheight );
        rstr[ 0 ] = 0;
        if ( vid_cap.type & VID_TYPE_CAPTURE )
        {
            strcat( rstr, "Can capture, " );
        }
        if ( vid_cap.type & VID_TYPE_TUNER )
        {
            strcat( rstr, "Can tune, " );
        }
        if ( vid_cap.type & VID_TYPE_TELETEXT )
        {
            strcat( rstr, "Does teletext, " );
        }
        if ( vid_cap.type & VID_TYPE_OVERLAY )
        {
            strcat( rstr, "Overlay onto frame buffer, " );
        }
        if ( vid_cap.type & VID_TYPE_CLIPPING )
        {
            strcat( rstr, "Can clip, " );
        }
        if ( vid_cap.type & VID_TYPE_FRAMERAM )
        {
            strcat( rstr, "Uses frame buffer memory, " );
        }
        if ( vid_cap.type & VID_TYPE_SCALES )
        {
            strcat( rstr, "Scalable, " );
        }
        if ( vid_cap.type & VID_TYPE_MONOCHROME )
        {
            strcat( rstr, "Monchrome, " );
        }
        if ( vid_cap.type & VID_TYPE_SUBCAPTURE )
        {
            strcat( rstr, "Capture subareas." );
        }
        printTextWindow( rstr );
    }

    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        for( i=0;i<vid_cap.channels;i++ )
        {
            vid_chan.channel = i;
            if( ioctl( fd, VIDIOCGCHAN, &vid_chan ) < 0 )
            {
                if ( report_level >= HS_REP_LEV_ERR )
                {
                    printTextWindow( "Video VIDIOCGCHAN error" );
                }
                return -1;
            }
            printfTextWindow( "Video Source ( %d ) Name : %s  tuners: %d  flags: %d  norm %d", 
                              i, vid_chan.name, vid_chan.tuners, vid_chan.flags, vid_chan.norm );
        }
    }
  
    buf = (unsigned char*) mmap( 0, height * width * 3, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if (buf == MAP_FAILED)
    {
        buf = NULL;
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video mmap error" );
        }
        return -1;
    }
  
    vid_chan.channel = channel;
    vid_chan.norm = 1;

    if( ioctl( fd, VIDIOCSCHAN, &vid_chan ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video VIDIOCSCHAN error" );
        }
        return -1;
    }

    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        if( ioctl( fd, VIDIOCGPICT, &vid_pict ) < 0 )
        {
            if ( report_level >= HS_REP_LEV_ERR )
            {
                printTextWindow( "Video VIDIOCGPICT error" );
            }
            return -1;
        }
        printfTextWindow( "Picture bight %d hue %d color %d contrast %d "
                          "whiteness %d depth %d palette %d\n", 
                          vid_pict.brightness, vid_pict.hue, vid_pict.colour, vid_pict.contrast, 
                          vid_pict.whiteness, vid_pict.depth, vid_pict.palette );
    }
  
    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        vid_aud.audio = 0;
        if( ioctl( fd, VIDIOCGAUDIO, &vid_aud ) < 0 )
        {
            if ( report_level >= HS_REP_LEV_ERR )
            {
                printTextWindow( "Video VIDIOCGAUDIO error" );
            }
            return -1;
        }
        printfTextWindow( "Audio volume %d bass %d treble %d flags %d name %s "
                          "mode %d balance %d step %d\n", 
                          vid_aud.volume, vid_aud.bass, vid_aud.treble, vid_aud.flags, vid_aud.name, 
                          vid_aud.mode, vid_aud.balance, vid_aud.step );
    }

    vid_mm.frame = 0;
    vid_mm.height = height;
    vid_mm.width = width;
    vid_mm.format = VIDEO_PALETTE_RGB24;

    if( ioctl( fd, VIDIOCMCAPTURE, &vid_mm ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video VIDIOCMCAPTURE error" );
        }
        return -1;
    }

    if( ioctl( fd, VIDIOCSYNC, &vid_mm.frame ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video VIDIOSYNC error" );
        }
        return -1;
    }

    return 0;
}
  

void Video::setChannel( int channel )
{
    vid_chan.channel = channel;
    vid_chan.norm = 1;
  
    this->channel = channel;

    if( ioctl( fd, VIDIOCSCHAN, &vid_chan ) < 0 )
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video VIDIOCSCHAN error" );
        }
        return;
    }
}

static void start_thread( void *video_instance )
{
    ( (Video*) video_instance )->capturer_thread();
}

void Video::capturer_thread()
{
    FILE* ofile;
    int i, j;
    bool uploading;

    capturing = true;
    updateStatusBar_();

    do
    {
        uploading = this->uploading;
        if( ioctl( fd, VIDIOCMCAPTURE, &vid_mm ) < 0 )
        {
            if ( report_level >= HS_REP_LEV_ERR )
            {
                printTextWindow( "Video VIDIOCMCAPTURE error" );
            }
            return;
        }

        if( ioctl( fd, VIDIOCSYNC, &vid_mm.frame ) < 0 )
        {
            if ( report_level >= HS_REP_LEV_ERR )
            {
                printTextWindow( "Video VIDIOSYNC error" );
            }
            return;
        }

        if ( xwindow != NULL )
        {
            xwindow->putRGB( 0, 0, width, height, buf );
        }
        if ( ( xwindow == NULL ) || ( uploading && !upload_send ) )
        {
            if ( uploading )
            {
                ofile = fopen( upload_name, "w" );
                chmod( upload_name, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH );
            }
            else
            {
                ofile = fopen( "./video_cap.ppm", "w" );
            }
            fprintf( ofile, "P6\n %d %d\n 255\n", width, height );
            fflush( ofile );
	
            for( i = 0; i < height; i++ )
            {
                for( j = 0; j < width; j++ )
                {
                    fwrite( buf + ( i * width + j ) * 3 + 2, 1, 1, ofile );
                    fwrite( buf + ( i * width + j ) * 3 + 1, 1, 1, ofile );
                    fwrite( buf + ( i * width + j ) * 3,     1, 1, ofile );
                }
            }
            fclose( ofile );
        }

        updateStatusBar_();
        if ( uploading )
        {
            this->uploading = false;
            pthread_cond_broadcast( &upload_cond );
        }
    } while ( streaming );
    capturing = false;
    updateStatusBar_();
    pthread_exit( 0 );
}

void Video::capture()
{
    pthread_attr_t attr;
    pthread_attr_init( &attr );
    pthread_attr_setdetachstate( &attr, PTHREAD_CREATE_DETACHED );
    pthread_create( &capturer, &attr, (void* (*)(void*)) &start_thread, (void*) this );
}

void Video::setSize( int height, int width )
{
    munmap( (char*) buf, height * width * 3 );
    buf = (unsigned char*) mmap( 0, height * width * 3, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
    if(buf == MAP_FAILED)
    {
        if ( report_level >= HS_REP_LEV_ERR )
        {
            printTextWindow( "Video mmap error" );
        }
        return;
    }
    this->width = width;
    this->height = height;
    vid_mm.height = height;
    vid_mm.width = width;
    printfTextWindow( "Vidion capture resolution %dx%d", width, height );
}

void Video::frameUpload( int channel, char* name, bool send, IPCommunicator* comm, IPMessage* msg )
{
    REC_VideoFrame reply;
    if ( report_level>=HS_REP_LEV_DEBUG )
    {
        printfTextWindow( "video frame upload  name: %s  channel %d", name, channel );
    }

    strcpy( upload_name, name );
    uploading = true;
    if ( send )
    {
        upload_send = true;
    }
    if ( !capturing )
    {
        capture();
    }
    pthread_mutex_lock( &upload_cond_mutex );
    pthread_cond_wait( &upload_cond, &upload_cond_mutex );
    pthread_mutex_unlock( &upload_cond_mutex );   
    if ( !send )
    {
        printTextWindow( "Video frame upload saved" );
    }
    else
    {
        reply.width = width;
        reply.height = height;
        reply.size = height * width * 3;
        reply.frame = (char*) buf;
        comm->Reply( msg, MSG_VideoFrame, &reply );
    }
}
 
void Video::control()
{
    bool bDone = false;
    int msgData[ 1 ];

    msgData[ 0 ] = streaming;

    messageDrawWindow( EnMessageType_VIDEO_CONTROL, EnMessageErrType_NONE, msgData );
    do
    {
        int c = getch();
        switch( c )
        {
        case 'd':
            delete this;
            return;
            break;
        case 'r':
            refreshScreen();
            break;
        case 'x':
        case 'Q':
        case KEY_ESC:
            bDone = true;
            break;
        case 'g':
            capture();
            break;
        case 's':
            streaming = !streaming;
            updateStatusBar_();
            msgData[ 0 ] = streaming;
            messageDrawWindow( EnMessageType_VIDEO_CONTROL, EnMessageErrType_NONE, msgData );
            break;
        case 'a':
        case 'b':
        case 'c':
            setChannel( c-'a' );
            updateStatusBar_();
            break;
        case '!':
            setSize( 120, 160 );
            break;
        case '@':
            setSize( 240, 320 );
            break;
        case '#':
            setSize( 360, 480 );
            break;
        case '$':
            setSize( 480, 640 );
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
    } while ( !bDone );
    messageHide();
}

void Video::addSensor()
{
    partVideo = new RobotPartVideo( 3 );
    sensorVideo = new Sensor( SENSOR_VIDEO );
}

Video::~Video()
{
    if ( buf != NULL )
    {
        munmap( (char*) buf, height * width * 3 );
    }
    if ( fd > 0 )
    {
        close( fd );
    }
    if ( sensorVideo != NULL )
    {
        delete sensorVideo;
    }
    if ( partVideo != NULL )
    {
        delete partVideo;
    }
    printTextWindow( "Video disconnected" );
}

Video::Video(
    Video** a,
    const string& strPortString,
    int height, 
    int width) : 
    Module((Module**)a, HS_MODULE_NAME_VIDEO)
{
    this->strPortString = strPortString;
    fd = -1;
    buf = NULL;
    sensorVideo = NULL;
    partVideo = NULL;
    channel = 0;
    streaming = false;
    printTextWindow( "Connecting to framegrabber... ", false );
    if ( initV4Linux( height, width ) )
    {
        printTextWindow( "failed" );
        delete this;
        return;
    }
    printTextWindow( "connected" );
    addSensor();
    updateStatusBar_();
    pthread_cond_init( &upload_cond, NULL );
    pthread_mutex_init( &upload_cond_mutex, NULL );
}

void framegrabberConnect()
{
    unsigned const int uPORT_ECHO_LINE = 4;

    // get the connection port from the user
    string strPort = messageGetString( EnMessageType_FRAMEGRABBER_PORT, uPORT_ECHO_LINE );

    new Video( &video, strPort );
}

/**********************************************************************
# $Log: video.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 06:01:03  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:43  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.6  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.5  2001/03/23 21:32:46  blee
# altered to use a config file
#
# Revision 1.4  2000/12/12 23:23:24  blee
# Changed #defines for the user interface to enumerated types.
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
