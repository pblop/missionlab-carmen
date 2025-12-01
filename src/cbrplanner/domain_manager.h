/**********************************************************************
 **                                                                  **
 **                           domain_manager.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef DOMAIN_MANAGER_H
#define DOMAIN_MANAGER_H

/* $Id: domain_manager.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string>

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"

using std::vector;
using std::string;

class DomainManager {

protected:
    DomainManager **self_;

    static const string EMPTY_STRING_;
    static const string ACDL_STRING_BIND_ARCH_;
    static const string ACDL_STRING_INSTANCE_INDEX_;
    static const string ACDL_STRING_INSTANCE_NAME_;
    static const string ACDL_STRING_INSTANCE_;
    static const string ACDL_STRING_TASK_INDEX_;
    static const string ACDL_STRING_TASK_DESC_;
    static const string ACDL_STRING_AGENT_NAME_;
    static const string ACDL_STRING_TRIGGERING_TASK_INDEX_;
    static const string ACDL_STRING_PARM_NAME_;
    static const string ACDL_STRING_PARM_VALUE_;
    static const string ACDL_STRING_IF_;
    static const string ACDL_STRING_START_;
    static const string ACDL_STRING_GOTO_;
    static const string ACDL_STRING_END_;
    static const string ACDL_STRING_MOVEMENT_;
    static const string ACDL_STRING_STARTTASK_INDEX_;
    static const string ACDL_STRING_MAX_VELOCITY_;
    static const string ACDL_STRING_BASE_VELOCITY_;
    static const string ACDL_STRING_ROBOT_ID_;
    static const string ACDL_STRING_START_SUB_MISSION_;
    static const int DEFAULT_ROBOT_ID_;

    void addEffect_(
        vector<CBRPlanner_PlanOperator_t> *plan,
        string operatorIndex,
        CBRPlanner_Effect_t effect);
    void addPrecondition_(
        vector<CBRPlanner_PlanOperator_t> *plan,
        string operatorIndex,
        CBRPlanner_Precondition_t precondition);
    void setAsFirstOperator_(
        vector<CBRPlanner_PlanOperator_t> *planSequence,
        string operatorIndex);
    vector<CBRPlanner_PlanOperator_t> addOperatorToPlanSequence_(
        vector<CBRPlanner_PlanOperator_t> planSequence,
        CBRPlanner_PlanOperator_t planOperator);
    bool hasAgentInMissionPlan_(
        CBRPlanner_ACDLMissionPlan_t acdlPlan,
        string agentName);

public:
    DomainManager(void);
    DomainManager(DomainManager **self);
    ~DomainManager(void);

    void updateFSANames(CBRPlanner_FSANames_t fsaNames);
    CBRPlanner_ACDLMissionPlan_t missionplan2acdl(
        MissionMemoryManager::MissionPlan_t missionplan);
    MissionMemoryManager::MissionPlan_t acdl2missionplan(
        CBRPlanner_ACDLMissionPlan_t acdlPlan);
    bool hasStartSubMision(CBRPlanner_ACDLMissionPlan_t acdlPlan);
};

inline bool DomainManager::hasStartSubMision(
    CBRPlanner_ACDLMissionPlan_t acdlPlan)
{
    return (hasAgentInMissionPlan_(acdlPlan, ACDL_STRING_START_SUB_MISSION_));
}

#endif
/**********************************************************************
 * $Log: domain_manager.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/08/24 22:25:21  endo
 * State ordering problem fixed.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2005/07/31 03:41:41  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/02/07 19:53:43  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
