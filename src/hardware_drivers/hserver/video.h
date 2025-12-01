/**********************************************************************
 **                                                                  **
 **                              video.h                             **
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

/* $Id: video.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef VIDEO_H
#define VIDEO_H

#define VIDEO_DEFAULT_HEIGHT 120
#define VIDEO_DEFAULT_WIDTH  160

#ifdef LINUX_VIDEODEV_PRESENT
	#include <linux/videodev.h>
#else
	#include <libv4l1-videodev.h>
#endif

#include <string>
#include "module.h"
#include "sensors.h"
#include "robot_config.h"

class Video : public Module
{
  protected:
    Sensor *sensorVideo;
    RobotPartVideo *partVideo;
    struct video_capability vid_cap;
    struct video_channel    vid_chan;
    struct video_picture    vid_pict;
    struct video_mmap       vid_mm;
    struct video_audio      vid_aud;
    string strPortString;
    int fd;
    int channel;
    int spinner;
    bool streaming,capturing,uploading,upload_send;
    char upload_name[ 100 ];
    unsigned char* buf;
    int height, width;
    void addSensor();
    pthread_t capturer;
    pthread_cond_t upload_cond;
    pthread_mutex_t upload_cond_mutex;
    int initV4Linux( int height, int width );
    void setChannel( int channel );
    void capture();
    void setSize( int height, int width );
    void updateStatusBar_(void);
  public:
    void capturer_thread();
    void frameUpload( int channel, char* name, bool send,
                      IPCommunicator* comm, IPMessage* msg );
    void control();
    Video( Video** a, const string& strPortString, int height = VIDEO_DEFAULT_HEIGHT,
           int width = VIDEO_DEFAULT_WIDTH );
    ~Video();
};

extern Video* video;

void framegrabberConnect();

#endif

/**********************************************************************
# $Log: video.h,v $
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
# Revision 1.4  2001/03/23 21:32:51  blee
# altered to use a config file
#
# Revision 1.3  2000/09/19 03:51:29  endo
# RCS log added.
#
#
#**********************************************************************/
