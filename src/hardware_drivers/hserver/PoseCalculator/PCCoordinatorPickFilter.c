/**********************************************************************
 **                                                                  **
 **                     PCCoordinatorPickFilter.c                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCCoordinatorPickFilter.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "PCCoordinatorPickFilter.h"
#include "PoseCalculatorTypes.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for PoseCalcCoordinatorPickFilter
//-----------------------------------------------------------------------
PoseCalcCoordinatorPickFilter::PoseCalcCoordinatorPickFilter(int fuserType) :
    fuserType_(fuserType)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcCoordinatorPickFilter
//-----------------------------------------------------------------------
PoseCalcCoordinatorPickFilter::~PoseCalcCoordinatorPickFilter(void)
{
}

//-----------------------------------------------------------------------
// This function returns the output of the fused sensor information.
//-----------------------------------------------------------------------
PoseCalcFusedPose_t PoseCalcCoordinatorPickFilter::coordinate(
    vector<PoseCalcFusedPose_t> fusedPoseList)
{
    PoseCalcFusedPose_t fusedPose;
    int i;
    bool filterFound = false;
    
    for (i = 0; i < (int)(fusedPoseList.size()); i++)
    {
        if (fusedPoseList[i].fuserType == fuserType_)
        {
            fusedPose = fusedPoseList[i];
            filterFound = true;
            break;
        }
    }

    if (!filterFound)
    {
        // Specified filter was not found.
        memset((PoseCalcFusedPose_t *)&fusedPose, 0x0, sizeof(PoseCalcFusedPose_t));
    }

    return fusedPose;
}

/**********************************************************************
# $Log: PCCoordinatorPickFilter.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:53  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.1  2004/09/10 19:59:16  endo
# New PoseCalc integrated.
#
#**********************************************************************/
