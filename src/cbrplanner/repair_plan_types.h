/**********************************************************************
 **                                                                  **
 **                      repair_plan_types.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_plan_types.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef REPAIR_PLAN_TYPES_H
#define REPAIR_PLAN_TYPES_H

#include <vector>
#include <string>

using std::vector;
using std::string;

// If you alter this, make sure to update mission_design_types.h
// as well.
typedef enum RepairQuestionStatus_t {
    NO_SOLUTION,
    HAS_SOLUTION,
    NORMAL_QUESTION
};

// If you alter this, make sure to update mission_design_types.h
// as well.
typedef enum RepairQuestionType_t {
    NO_QUESTION,
    TOGGLE,
    ADD_MAP_POINT,
    MOD_MAP_POINT,
    DEL_MAP_POINT,
    STATE_NAME
};

// If you alter this, make sure to update mission_design_types.h
// as well.
typedef struct RepairQuestion_t {
    int status;
    int type;
    string question;
    vector<string> options;
};

typedef struct OffendingState_t
{
    string description;
};


#endif

/**********************************************************************
 * $Log: repair_plan_types.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 19:53:46  endo
 * Mods from usability-2004
 *
 **********************************************************************/
