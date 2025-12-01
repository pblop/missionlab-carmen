/**********************************************************************
 **                                                                  **
 **                         compass_kvh-c100.c                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  KVH C100 Compass driver for HServer                             **
 **                                                                  **
 **  Copyright 2001 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
/* $Id: compass_kvh-c100.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef COMPASS_KVH_C100_H
#define COMPASS_KVH_C100_H

#include "mobilitycomponents_i.h"
#include "mobilitydata_i.h"
#include "mobilitygeometry_i.h"
#include "mobilityactuator_i.h"
#include "mobilityutil.h"
#include "MobilityData.hh"
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include "hserver.h"
#include "message.h"
#include "robot_config.h"
#include "sensors.h"
#include "statusbar.h"
#include "robot.h"
#include "sensors.h"


class CompassKVHC100 : public Compass {

protected:
    MobilityData::FVectorState *pMbyCompassState_;
    mbyClientHelper *pHelper_;
    CORBA::Object_ptr ptempObj_;

    static const char *COMPASSSERVER_NAME_;
    static const int READER_THREAD_USLEEP_;
    static const int SKIP_STATUSBAR_UPDATE_;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

    CompassData_t readCompassData_(void);
    int initializeMobility_(void);
    void updateStatusBar_(void);
    
public:
    CompassKVHC100(Compass** a);
    ~CompassKVHC100();
    void control(void);
};


#endif

/**********************************************************************
# $Log: compass_kvh-c100.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/24 11:20:53  endo
# 3DM-G added.
#
# Revision 1.1.1.1  2004/02/27 22:30:51  endo
# Initial MARS 2020 revision
#
# Revision 1.1  2001/05/18 17:55:10  blee
# Initial revision
#
#
#**********************************************************************/
