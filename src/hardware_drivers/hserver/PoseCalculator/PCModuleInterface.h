/**********************************************************************
 **                                                                  **
 **                        PCModulesInterface.h                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCModuleInterface.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_MODULE_INTERFACE_H
#define POSECALC_MODULE_INTERFACE_H

#include "PoseCalculatorTypes.h"

//-----------------------------------------------------------------------
// Abstract Class
//-----------------------------------------------------------------------
class PoseCalcModuleInterface {

public:
    virtual void managePose(PoseCalcPoseInfo_t &poseInfo, int varianceScheme) = 0;
    virtual void setPose(HSPose_t pose) = 0;
    virtual void die(void) = 0;
};

#endif

/**********************************************************************
# $Log: PCModuleInterface.h,v $
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
