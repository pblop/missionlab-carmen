/**********************************************************************
 **                                                                  **
 **                     repair_memory_manager.cc                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This modules deals with storing and retrieving of repair rules. **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: repair_memory_manager.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>

#include "cbrplanner_domain.h"
#include "memory_manager.h"
#include "repair_memory_manager.h"
#include "windows.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for RepairMemoryManager class.
//-----------------------------------------------------------------------
RepairMemoryManager::RepairMemoryManager(RepairMemoryManager **repairMemoryManagerInstance) :
    MemoryManager((MemoryManager **)repairMemoryManagerInstance)
{
}

//-----------------------------------------------------------------------
// Distructor for RepairMemoryManager class.
//-----------------------------------------------------------------------
RepairMemoryManager::~RepairMemoryManager(void)
{
}

//-----------------------------------------------------------------------
// This function will save a new repair plan into its memory (decision
// tree).
//-----------------------------------------------------------------------
int RepairMemoryManager::saveNewRepairPlan(int solutionNumber, Features_t features)
{
    RepairPlan_t repairplan;
    int index;
    const bool REPORT = false;

    // Save the case.
    repairplan.solutionNumber = solutionNumber;
    index = repairPlanList_.size();
    repairplan.dataIndex = index;
    repairPlanList_.push_back(repairplan);
    memoryChanged_ = true;

    addNewDataToDecisionTree_(&decisionTree_, features, index);

    if (REPORT)
    {
        // Provide the feedback to the user.
        gWindows->printfPlannerWindow(
            "New case (ID %d) added to the repair memory.\n",
            index);
    }

    return index;
}

//-----------------------------------------------------------------------
// This function returns the solution numebr in the repair plan list
// based on the given data index.
//-----------------------------------------------------------------------
int RepairMemoryManager::getSolutionNumber(int dataIndex)
{
    int i;

    for (i = 0; i < (int)(repairPlanList_.size()); i++)
    {
        if (repairPlanList_[i].dataIndex == dataIndex)
        {
            return repairPlanList_[i].solutionNumber;
        }
    }

    return -1;
}

/**********************************************************************
 * $Log: repair_memory_manager.cc,v $
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
