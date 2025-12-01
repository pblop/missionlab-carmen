/**********************************************************************
 **                                                                  **
 **                       PCModuleFilsters.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCModuleFilters.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unistd.h>

#include "PCModuleFilters.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for PoseCalcModuleFilterNull
//-----------------------------------------------------------------------
PoseCalcModuleFilterNull::PoseCalcModuleFilterNull(void)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcModuleFilterNull
//-----------------------------------------------------------------------
PoseCalcModuleFilterNull::~PoseCalcModuleFilterNull(void)
{
}

//-----------------------------------------------------------------------
// This function filters the input value.
//-----------------------------------------------------------------------
void PoseCalcModuleFilterNull::filter(PoseCalcPoseInfo_t &poseInfo)
{
    // Just return the input data..
}

/**********************************************************************
# $Log: PCModuleFilters.c,v $
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
