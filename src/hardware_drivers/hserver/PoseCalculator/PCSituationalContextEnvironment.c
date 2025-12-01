/**********************************************************************
 **                                                                  **
 **                 PCSituationalContextEnvironment.c                **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSituationalContextEnvironment.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "PCSituationalContextEnvironment.h"
#include "PoseCalculatorTypes.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern void printfTextWindow(const char* arg, ...);

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for PoseCalcSituationalContextEnvironment
//-----------------------------------------------------------------------
PoseCalcSituationalContextEnvironment::PoseCalcSituationalContextEnvironment(void)
{
}

//-----------------------------------------------------------------------
// Destructor for PoseCalcSituationalContextEnvironment
//-----------------------------------------------------------------------
PoseCalcSituationalContextEnvironment::~PoseCalcSituationalContextEnvironment(void)
{
}

//-----------------------------------------------------------------------
// This function computes the situational value.
//-----------------------------------------------------------------------
void PoseCalcSituationalContextEnvironment::applySituation(PoseCalcGenericData_t &data)
{
    PoseCalcFusedPose_t *fusedPose = NULL;

    fusedPose = (PoseCalcFusedPose_t *)(data.content);

    if (fusedPose == NULL)
    {
        return;
    }

    switch (fusedPose->fuserType) {

    case POSECALC_FUSER_TYPE_MAXMUM_CONFIDENCE:
        // Alter the data content if necessary.
        break;

    case POSECALC_FUSER_TYPE_EXTENDED_KALMAN_FILTER:
        // Alter the data content if necessary.
        break;

    case POSECALC_FUSER_TYPE_PARTICLE_FILTER:
        // Alter the data content if necessary.
        break;
    }
}

/**********************************************************************
# $Log: PCSituationalContextEnvironment.c,v $
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
