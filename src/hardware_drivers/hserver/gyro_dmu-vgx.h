/**********************************************************************
 **                                                                  **
 **                           gyro_dmu-vgx.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Corssbow DMU-VGX Gyroscope driver for HServer                   **
 **                                                                  **
 **  Copyright 2001 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
/* $Id: gyro_dmu-vgx.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef GYRO_DMU_VGX_H
#define GYRO_DMU_VGX_H

#include "mobilitycomponents_i.h"
#include "mobilitydata_i.h"
#include "mobilitygeometry_i.h"
#include "mobilityactuator_i.h"
#include "mobilityutil.h"
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
#include "gyro.h"

using std::string;

class GyroDMUVGX : public Gyro {

protected:
    mbyClientHelper *pHelper_;
    CORBA::Object_ptr ptempObj_;
    MobilityActuator::ActuatorState_var pMbyGyroState_;
    MobilityActuator::ActuatorData *pMbyGyroData_;

    static const string DMUSERVER_NAME_;
    static const int READER_THREAD_USLEEP_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;
    static const double INIT_CALIB_FACTOR_YAW_;
    static const double INIT_CALIB_FACTOR_PITCH_;
    static const double INIT_CALIB_FACTOR_ROLL_;
    static const double INIT_OFFSET_YAW_;
    static const double INIT_OFFSET_PITCH_;
    static const double INIT_OFFSET_ROLL_;

    GyroData_t readGyroData_(void);
    int initializeMobility_(void);
    void updateStatusBar_(void);

public:
    GyroDMUVGX(Gyro** a);
    ~GyroDMUVGX();
    void control(void);
};

#endif

/**********************************************************************
# $Log: gyro_dmu-vgx.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2001/05/18 17:54:52  blee
# Initial revision
#
#
#**********************************************************************/
