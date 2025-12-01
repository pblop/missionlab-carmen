/**********************************************************************
 **                                                                  **
 **                  PoseCalculatorFilterInterface.h                 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCModuleFilterInterface.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_MODULE_FILTER_INTERFACE_H
#define POSECALC_MODULE_FILTER_INTERFACE_H

#include "PoseCalculatorTypes.h"

class PoseCalcModuleFilterInterface {

public:
    virtual void filter(PoseCalcPoseInfo_t &poseInfo) = 0;
};

#endif

/**********************************************************************
# $Log: PCModuleFilterInterface.h,v $
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
