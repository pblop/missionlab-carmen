/**********************************************************************
 **                                                                  **
 **                     PCCoordinatorInterface.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCCoordinatorInterface.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_COORDINATOR_INTERFACE_H
#define POSECALC_COORDINATOR_INTERFACE_H

#include <vector>

#include "PoseCalculatorTypes.h"

using std::vector;

//-----------------------------------------------------------------------
// Abstract class for PoseCalcCoordinator
//-----------------------------------------------------------------------

class PoseCalcCoordinatorInterface {

public:
    virtual PoseCalcFusedPose_t coordinate(
        vector<PoseCalcFusedPose_t> fusedPoseList) = 0;
};

#endif

/**********************************************************************
# $Log: PCCoordinatorInterface.h,v $
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
