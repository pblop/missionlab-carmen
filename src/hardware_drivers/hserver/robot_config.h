/**********************************************************************
 **                                                                  **
 **                          robot_config.h                          **
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

/* $Id: robot_config.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

#include <panel.h>
#include "hserver_ipt.h"

#define SETUP_WIN_COL                   60
#define SETUP_WIN_ROW                   40
#define SETUP_WIN_X                     8
#define SETUP_WIN_Y                     5

class RobotPart
{
    friend class RobotConfiguration;
  protected:
    int type;
    int len_info;
    char *info;
    int index;
  public:
    RobotPart(int type,int len_info = 0,char *info = NULL);
    ~RobotPart();
};


class RobotPartSonar : public RobotPart
{
  public:
    RobotPartSonar(int number,float *angle,float fov = 30.0,float range = 15.0);
};

class RobotPartRoomba : public RobotPart
{
  public:
    RobotPartRoomba(int number,float *angle,float fov = 30.0,float range = 0.10);
};

class RobotPartIr : public RobotPart
{
  public:
    RobotPartIr(int number,float *angle,float fov = 30.0,float range = 0.30);
};


class RobotPartPanTilt : public RobotPart
{
  public:
    RobotPartPanTilt(float angle,float fov_x,float fov_y);
};

class RobotPartVideo : public RobotPart
{
  public:
    RobotPartVideo(int num_channels);
};

class RobotPartCognachrome : public RobotPart
{
  public:
    RobotPartCognachrome(int num_channels,int max_per_channel);
};

class RobotPartXyt : public RobotPart
{
  public:
    RobotPartXyt(float scale = 1.0);
};

class RobotPartLaser : public RobotPart
{
  public:
    RobotPartLaser(int num_readings,float angle,float fov,float range);
};

class RobotPartArm : public RobotPart
{
  public:
    RobotPartArm();
};

class RobotPartCompass : public RobotPart
{
  public:
    RobotPartCompass();
};

class RobotPartGps : public RobotPart
{
  public:
    RobotPartGps();
};

class RobotPartMotoresAspirar : public RobotPart
{
  public:
    RobotPartMotoresAspirar();
};

class RobotPartWebCam : public RobotPart
{
  public:
    RobotPartWebCam(int nPixel_x, int nPixel_y);
};

class RobotPartSensorGeneric : public RobotPart
{
  public:
    RobotPartSensorGeneric(int num);
};


class RobotConfiguration
{
  private:
    int max_parts;
    char *index;
    int num_parts;
    RobotPart **part;
    char *name;
    int id;
    WINDOW *window;
    PANEL *panel;
    int panelX,panelY,panelW,panelH;
    void draw();
    void windowInit();
    void capInt(int *i,int l,int u);
    void movePanel(int xdif,int ydif);
    void movePanelKey(int c);
    int runKey(int c);
  public:
    void fillIptMessage(REC_RobotSetupInfo *rec);
    char *getName(){ return name; };
    int add(RobotPart *part);
    void remove(int idx);
    void show();
    RobotConfiguration(int max,int id,char *name);
    ~RobotConfiguration();
};

extern RobotConfiguration *robot_config;

#endif

/**********************************************************************
# $Log: robot_config.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.2  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
