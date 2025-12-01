/**********************************************************************
 **                                                                  **
 **                    GTechBehaviorAdviseMethod.h                   **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CommBehaviorAdviseMethod.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef GTECH_BEHAVIOR_ADVISE_METHOD_H
#define GTECH_BEHAVIOR_ADVISE_METHOD_H

typedef enum BehaviorCoordinatorAdviseMethod_t {
    CSB_ADVISE_GREEDY,
    CSB_ADVISE_AVERAGE,
    CSB_ADVISE_USE_COMM_RECOVERY,
    CSB_ADVISE_USE_COMM_PRESERVE,
    CSB_ADVISE_USE_INTERNALIZED_PLAN
};

#endif

/**********************************************************************
 * $Log: CommBehaviorAdviseMethod.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.1  2004/09/15 06:27:30  endo
 * Functions related to SetAdviseMethod added.
 *
 **********************************************************************/

