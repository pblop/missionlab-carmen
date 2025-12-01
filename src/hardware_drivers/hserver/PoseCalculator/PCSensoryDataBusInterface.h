/**********************************************************************
 **                                                                  **
 **                     SensoryDataBusInterface.h                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSensoryDataBusInterface.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_SENSORY_DATA_BUS_INTERFACE_H
#define POSECALC_SENSORY_DATA_BUS_INTERFACE_H

#include <vector>

#include "PoseCalculatorTypes.h"

using std::vector;

//-----------------------------------------------------------------------
// Abstract class for the PoseCalcSensoryDataBus
//-----------------------------------------------------------------------

class PoseCalcSensoryDataBusInterface {

public:
    virtual vector<PoseCalcSensoryData_t> getAllSensoryData(int fuserType) = 0;
    virtual vector<PoseCalcLocationInfo_t> getLatestLocationData(int fuserType) = 0;
    virtual vector<PoseCalcRotationInfo_t> getLatestRotationData(int fuserType) = 0;
    virtual vector<PoseCalcPoseInfo_t> getLatestPoseData(int fuserType) = 0;
    virtual void savePoseInfo(PoseCalcPoseInfo_t poseInfo) = 0;
    virtual void initialize(void) = 0;
    virtual void waitForData(void) = 0;
    virtual void setFilterReadData(bool filterReadData) = 0;
};

#endif

/**********************************************************************
# $Log: PCSensoryDataBusInterface.h,v $
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
