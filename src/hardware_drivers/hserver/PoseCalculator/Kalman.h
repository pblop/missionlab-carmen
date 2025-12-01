/**********************************************************************
 **                                                                  **
 **                      Kalman.h                                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: Kalman.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef KALMAN_H
#define KALMAN_H

#include "PoseCalculatorTypes.h"
#include "PCSensorFuser.h"
#include "PCSensoryDataBusInterface.h"
#include "ekf_localize.h"

#include <string.h>

//-----------------------------------------------------------------------
// Generic PoseCalcSensorFuser class.
//-----------------------------------------------------------------------

class Kalman : public PoseCalcSensorFuser {

protected:

public:
    Kalman(PoseCalcSensoryDataBusInterface *sensoryDataBus);

    ~Kalman(void);
    PoseCalcFusedPose_t getOutput(void);
    PoseCalcFusedPose_t initialize(void);

    void initialize(HSPose_t pose);
    
 private:
    EKF_Localize *localizeEKF;
    bool EKFInit;

    double LastX;
    double LastY;
    double LastTheta;
    double LastThetaOd; // ENDO
    double LastThetaGyro; // ENDO
    PoseCalcFusedPose_t LastPose;
    double OdOffsetX;
    double OdOffsetY;
    double OdOffsetTheta;
    bool InitKalman;
    bool UseOdOffset;

};

#endif

/**********************************************************************
# $Log: Kalman.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:17  endo
# New PoseCalc integrated.
#
#**********************************************************************/
