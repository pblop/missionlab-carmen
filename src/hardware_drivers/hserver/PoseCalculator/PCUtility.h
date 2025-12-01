/**********************************************************************
 **                                                                  **
 **                            PCUtility.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCUtility.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_UTILITY_H
#define POSECALC_UTILITY_H

#include <string>

#include "HServerTypes.h"
#include "PoseCalculatorTypes.h"

using std::string;

class PoseCalcUtility {

    static const string EMPTY_STRING_;
    static const string STRING_UNKNOWN_;

public:
    PoseCalcUtility(void) {};
    ~PoseCalcUtility(void) {};

    static string sensorType2Name(int sensorType);
    static int name2SensorType(string name);
    static PoseCalcLocationInfo_t extractLocationInfoFromPoseInfo(
        PoseCalcPoseInfo_t poseInfo);
    static PoseCalcRotationInfo_t extractRotationInfoFromPoseInfo(
        PoseCalcPoseInfo_t poseInfo);

};

#endif

/**********************************************************************
# $Log: PCUtility.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:19  endo
# New PoseCalc integrated.
#
#**********************************************************************/
