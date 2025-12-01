/**********************************************************************
 **                                                                  **
 **                         PCSensoryDataBus.h                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSensoryDataBus.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_SENSORY_DATA_BUS_H
#define POSECALC_SENSORY_DATA_BUS_H

#include <pthread.h>
#include <vector>

#include <string.h>

#include "PoseCalculatorTypes.h"
#include "PCSensoryDataBusInterface.h"
#include "PoseCalculatorInterface.h"

using std::vector;

//-----------------------------------------------------------------------
// Implementation of PoseCalcSensoryDataBus
//-----------------------------------------------------------------------

class PoseCalcSensoryDataBus : public PoseCalcSensoryDataBusInterface {

protected:
    PoseCalcInterface *poseCalc_;
    vector<PoseCalcSensoryData_t> shSensoryDataList_; // Shared
    bool shSesnoryDataListReady_; // Shared
    bool shCheckRead_; // Shared
    bool shFilterReadData_; // Shared

    static pthread_mutex_t sensoryDataListMutex_;
    static pthread_mutex_t filterReadDataMutex_;

    static const double WAIT_DATA_SEC_;
    static const int SENSORY_BUFFER_SIZE_;
    static const int WAIT_DATA_USLEEP_;
    static const int WAIT_DATA_WARN_SEC_;

    void saveLocationInfo_(PoseCalcLocationInfo_t locInfo);
    void saveRotationInfo_(PoseCalcRotationInfo_t rotInfo);
    bool filterReadData_(void);

public:
    PoseCalcSensoryDataBus(PoseCalcInterface *poseCalc);
    vector<PoseCalcSensoryData_t> getAllSensoryData(int fuserType);
    vector<PoseCalcLocationInfo_t> getLatestLocationData(int fuserType);
    vector<PoseCalcRotationInfo_t> getLatestRotationData(int fuserType);
    vector<PoseCalcPoseInfo_t> getLatestPoseData(int fuserType);
    void initialize(void);
    void savePoseInfo(PoseCalcPoseInfo_t poseInfo);
    void waitForData(void);
    void setFilterReadData(bool filterReadData);

    virtual ~PoseCalcSensoryDataBus(void);
};

#endif

/**********************************************************************
# $Log: PCSensoryDataBus.h,v $
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
