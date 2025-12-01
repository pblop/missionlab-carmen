/**********************************************************************
 **                                                                  **
 **                      repair_memory_manager.h                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef REPAIR_MEMORY_MANAGER_H
#define REPAIR_MEMORY_MANAGER_H

/* $Id: repair_memory_manager.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string>

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "memory_manager.h"

using std::vector;
using std::string;

class RepairMemoryManager : public MemoryManager {

    typedef struct RepairPlan_t {
        int solutionNumber;
        int dataIndex;
    };

protected:
    vector<RepairPlan_t> repairPlanList_;

public:
    RepairMemoryManager(void);
    RepairMemoryManager(RepairMemoryManager **self);
    ~RepairMemoryManager(void);
    int saveNewRepairPlan(int solutionNumber, Features_t features);
    int getSolutionNumber(int dataIndex);
};

#endif
/**********************************************************************
 * $Log: repair_memory_manager.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 19:53:45  endo
 * Mods from usability-2004
 *
 **********************************************************************/
