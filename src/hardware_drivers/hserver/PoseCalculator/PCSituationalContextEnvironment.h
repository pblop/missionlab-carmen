/**********************************************************************
 **                                                                  **
 **                 PCSituationalContextEnvironment.h                **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: PCSituationalContextEnvironment.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef POSECALC_SITUATIONAL_CONTEXT_ENVIRONMENT_H
#define POSECALC_SITUATIONAL_CONTEXT_ENVIRONMENT_H

#include "PCSituationalContextInterface.h"
#include "PoseCalculatorTypes.h"

class PoseCalcSituationalContextEnvironment : 
    public PoseCalcSituationalContextInterface {

public:
    PoseCalcSituationalContextEnvironment(void);
    void applySituation(PoseCalcGenericData_t &data);

    virtual ~PoseCalcSituationalContextEnvironment(void);
};

#endif

/**********************************************************************
# $Log: PCSituationalContextEnvironment.h,v $
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
