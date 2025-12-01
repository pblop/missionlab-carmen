/**********************************************************************
 **                                                                  **
 **                         compass_3DM-G.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Alexander Stoytchev                                 **
 **  Modified by: Yoichiro Endo                                      **
 **                                                                  **
 **  MicroSensor 3DM-G driver for HServer                            **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: compass_3DM-G.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef COMPASS_3DM_G_H
#define COMPASS_3DM_G_H

#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <string>

#include "hserver.h"
#include "message.h"
#include "robot_config.h"
#include "sensors.h"
#include "statusbar.h"
#include "robot.h"
#include "sensors.h"

using std::string;

class Compass3DMG : public Compass {

protected:
    string portString_;
    int deviceNum3DMG_;

    static const int BAUD_RATE_;
    static const int READER_THREAD_USLEEP_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;
    static const bool USE_GYRO_STABILIZED_VALUE_;

    CompassData_t readCompassData_(void);
    int initialize3DMG_(void);
    void updateStatusBar_(void);

public:

    Compass3DMG(Compass** a, string portString);
    ~Compass3DMG(void);
    void control(void);
};


extern Compass3DMG *gCompass3DMG;

#endif

/**********************************************************************
# $Log: compass_3DM-G.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:51  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.1  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2002/10/29 18:10:11  pulam
# First Revision V2020 Project
#
# Revision 1.1  2001/05/18 17:55:10  blee
# Initial revision
#
#
#**********************************************************************/
