/**********************************************************************
 **                                                                  **
 **                              fred.h                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Robot emulation for HServer                                     **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: fred.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef FRED_H
#define FRED_H

#include <pthread.h>

#include "HServerTypes.h"
#include "robot.h"
#include "sensors.h"
#include "robot_config.h"

class Fred : public Robot {

protected:
    RobotPartXyt* partXyt_;
    pthread_t moverThread_;

    static const int MAIN_THREAD_USLEEP_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

    void addSensors_(void);
    void updateStatusBar_(void);
    void moveFred_(HSSpeed_t speed);
    void moverLoop_(void);
    void controlRobot_(void);
    void readDeadReckoningData_(void);

    static void* startMoverThread_(void *fredInstance);

public:
    Fred(Robot** a);
    ~Fred(void);
    void control(void);
};    

#endif

/**********************************************************************
# $Log: fred.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.5  2001/05/29 22:35:46  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.4  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
