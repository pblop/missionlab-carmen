/**********************************************************************
 **                                                                  **
 **                      MaximumConfidence.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: MaximumConfidence.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef MAXIMUM_CONFIDENCE_H
#define MAXIMUM_CONFIDENCE_H

#include "PoseCalculatorTypes.h"
#include "PCSensorFuser.h"
#include "PCSensoryDataBusInterface.h"

#include <string.h>

//-----------------------------------------------------------------------
// Generic PoseCalcSensorFuser class.
//-----------------------------------------------------------------------

class MaximumConfidence : public PoseCalcSensorFuser {

protected:

public:
    MaximumConfidence(PoseCalcSensoryDataBusInterface *sensoryDataBus);

    ~MaximumConfidence(void);
    PoseCalcFusedPose_t getOutput(void);
    PoseCalcFusedPose_t initialize(void);
};

#endif

/**********************************************************************
# $Log: MaximumConfidence.h,v $
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
