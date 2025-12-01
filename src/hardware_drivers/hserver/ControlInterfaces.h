/**********************************************************************
 **                                                                  **
 **                        ContrilInterfaces.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: ControlInterfaces.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef CONTROLINTERFACES_H
#define CONTROLINTERFACES_H

#include <pthread.h>
#include "robot.h"
#include "gps.h"
#include "gps_default.h"
#include "jbox.h"
#include "compass.h"
#include "gyro.h"

extern bool g_bWatchDogEnable;

void robotConnect(Robot*& robot);
void compassConnect(Compass*& compass);
void gyroConnect(Gyro*& gyro);
void gpsConnect(
    Gps*& gps,
    double base_lat,
    double base_lon,
    double base_x,
    double base_y,
    double mperlat,
    double mperlon);
void controlGps(Gps* gps);
void controlJbox(Jbox* jbox);

#endif

/**********************************************************************
# $Log: ControlInterfaces.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
