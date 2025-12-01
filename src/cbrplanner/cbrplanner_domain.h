/**********************************************************************
 **                                                                  **
 **                       cbrplanner_domain.h                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains the structures for CBRPlanner domains.       **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CBRPLANNER_DOMAIN_H
#define CBRPLANNER_DOMAIN_H

/* $Id: cbrplanner_domain.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string>

using std::string;
using std::vector;

typedef struct CBRPlanner_FSANames_t
{
    vector<string> taskNames;
    vector<string> triggerNames;
};

// Added by NADEEM - 02/09/07
typedef struct CBRPlanner_ACDLExtraData_t 
{
  string key;
  int format;
  int numDataEntries;
  vector<string> dataEntries;
};

typedef struct CBRPlanner_ACDLMissionPlan_t
{
    int type;
    int rating; // Obsolete. To be replaced by CBRPlanner_ACDLExtraData_t.
    int numRobots;
    vector<int> dataIndexList;
    int dataSize;
    char *data;
    vector<CBRPlanner_ACDLExtraData_t> extraDataList;
};

typedef struct CBRPlanner_Parameter_t
{
    string name;
    string value;
};

typedef vector<CBRPlanner_Parameter_t> CBRPlanner_Parameters_t;

typedef struct CBRPlanner_Precondition_t
{
    string name;
};

typedef vector<CBRPlanner_Precondition_t> CBRPlanner_Preconditions_t;

typedef struct CBRPlanner_Effect_t
{
    string name;
    string index;
    CBRPlanner_Parameters_t parameters;
};

typedef vector<CBRPlanner_Effect_t> CBRPlanner_Effects_t;

typedef struct CBRPlanner_PlanOperator_t
{
    string name;
    string index;
    string description;
    CBRPlanner_Parameters_t parameters;
    CBRPlanner_Preconditions_t preconditions;
    CBRPlanner_Effects_t effects;
};

typedef vector<CBRPlanner_PlanOperator_t> CBRPlanner_PlanSequence_t;

#endif
/**********************************************************************
 * $Log: cbrplanner_domain.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.2  2007/02/11 22:40:38  nadeem
 * Added the code to send the META-DATA from the cbrplanner to cfgedit_cbrclient
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/02/07 20:03:52  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:28:03  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
