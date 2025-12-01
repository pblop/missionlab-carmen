/**********************************************************************
 **                                                                  **
 **                       mission_design_types.h                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_design_types.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef MISSION_DESIGN_TYPES_H
#define MISSION_DESIGN_TYPES_H

#include <string>
#include <vector>

using std::string;
using std::vector;

// This should be same as the one difined in cbrplanner/repair_plan_types.h
typedef enum MMDRepairQuestionStatus_t {
    MMDRQS_NO_SOLUTION,
    MMDRQS_HAS_SOLUTION,
    MMDRQS_NORMAL_QUESTION
};

// This should be same as the one difined in cbrplanner/repair_plan_types.h
typedef enum MMDRepairQuestionType_t {
    MMDRQS_NO_QUESTION,
    MMDRQS_TOGGLE,
    MMDRQS_ADD_MAP_POINT,
    MMDRQS_MOD_MAP_POINT,
    MMDRQS_DEL_MAP_POINT,
    MMDRQS_STATE_NAME
};

// This should be same as the one difined in cbrplanner/repair_plan_types.h
typedef struct MMDRepairQuestion_t {
    int status;
    int type;
    string question;
    vector<string> options;
};

#endif

/**********************************************************************
 * $Log: mission_design_types.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 23:12:36  endo
 * Mods from usability-2004
 *
 **********************************************************************/
