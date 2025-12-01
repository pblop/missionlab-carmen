/**********************************************************************
 **                                                                  **
 **                     SensorFuserInterface.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSensorFuserInterface.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_SENSOR_FUSER_INTERFACE_H
#define POSECALC_SENSOR_FUSER_INTERFACE_H

#include "PoseCalculatorTypes.h"

//-----------------------------------------------------------------------
// Abstract class for the PoseCalcSensorFuser
//-----------------------------------------------------------------------

class PoseCalcSensorFuserInterface {

public:
    virtual PoseCalcFusedPose_t getOutput(void) = 0;
    virtual PoseCalcFusedPose_t initialize(void) = 0;
    virtual void die(void) = 0;
};

#endif

/**********************************************************************
# $Log: PCSensorFuserInterface.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:18  endo
# New PoseCalc integrated.
#
#**********************************************************************/
