/**********************************************************************
 **                                                                  **
 **                           domain_manager.cc                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module will tranlate the representation of the mission     **
 **  that CfgEdit understands to the representation of the mission   **
 **  that MemoryManager understand (or vice versa).                  **
 **                                                                  **
 **  Copyright 2003 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: domain_manager.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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
#include <iostream>
#include <fstream>

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "domain_manager.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "windows.h"
#include "debugger.h"

using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string DomainManager::EMPTY_STRING_ = "";
const string DomainManager::ACDL_STRING_BIND_ARCH_ = "BIND_ARCH";
const string DomainManager::ACDL_STRING_INSTANCE_INDEX_ = "INSTANCE_INDEX";
const string DomainManager::ACDL_STRING_INSTANCE_NAME_ = "INSTANCE_NAME";
const string DomainManager::ACDL_STRING_INSTANCE_ = "INSTANCE";
const string DomainManager::ACDL_STRING_TASK_INDEX_ = "TASK_INDEX";
const string DomainManager::ACDL_STRING_TASK_DESC_ = "TASK_DESC";
const string DomainManager::ACDL_STRING_AGENT_NAME_ = "AGENT_NAME";
const string DomainManager::ACDL_STRING_TRIGGERING_TASK_INDEX_ = "TRIGGERING_TASK_INDEX";
const string DomainManager::ACDL_STRING_PARM_NAME_ = "PARM_NAME";
const string DomainManager::ACDL_STRING_PARM_VALUE_ = "PARM_VALUE";
const string DomainManager::ACDL_STRING_IF_ = "IF";
const string DomainManager::ACDL_STRING_START_ = "START";
const string DomainManager::ACDL_STRING_GOTO_ = "GOTO";
const string DomainManager::ACDL_STRING_END_ = "END";
const string DomainManager::ACDL_STRING_MAX_VELOCITY_ = "MAX_VELOCITY";
const string DomainManager::ACDL_STRING_BASE_VELOCITY_ = "BASE_VELOCITY";
const string DomainManager::ACDL_STRING_MOVEMENT_ = "movement";
const string DomainManager::ACDL_STRING_STARTTASK_INDEX_ = "Start";
const string DomainManager::ACDL_STRING_START_SUB_MISSION_ = "StartSubMission";

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for DomainManager class.
//-----------------------------------------------------------------------
DomainManager::DomainManager(DomainManager **domainManagerInstance) : self_(domainManagerInstance)
{
    if (domainManagerInstance != NULL)
    {
        *domainManagerInstance = this;
    }
}

//-----------------------------------------------------------------------
// Distructor for DomainManager class.
//-----------------------------------------------------------------------
DomainManager::~DomainManager(void)
{
    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function updates the FSA list of domain manager.
//-----------------------------------------------------------------------
void DomainManager::updateFSANames(CBRPlanner_FSANames_t fsaNames)
{
    // Not yet supported.
}

//-----------------------------------------------------------------------
// This function converts a mision plan into in ACDL.
//-----------------------------------------------------------------------
CBRPlanner_ACDLMissionPlan_t DomainManager::missionplan2acdl(MissionMemoryManager::MissionPlan_t missionplan)
{
    CBRPlanner_ACDLMissionPlan_t acdlPlan;
    CBRPlanner_PlanSequence_t plan;
    FILE *tmpFile = NULL;
    char tmpFilename[256];
    int i, j, k, p, planSize, effectsSize, parameterSize;
    int fd;
    const bool DEBUG = true;
    CBRPlanner_ACDLExtraData_t extraData;
    
    // Copy the type.
    acdlPlan.type = missionplan.type;

    // Copy the rating.
    acdlPlan.rating = missionplan.rating;

    // Copy the number of robots.
    acdlPlan.numRobots = missionplan.plans.size();

    // Copy the extra Data
    for (i=0; i < (int)(missionplan.extraDataList.size()); i++)
    {
        extraData.key = missionplan.extraDataList[i].key;
        extraData.format = missionplan.extraDataList[i].format;
        extraData.numDataEntries = missionplan.extraDataList[i].numDataEntries;

        extraData.dataEntries.clear();
        for (j=0; j< extraData.numDataEntries; j++)
        {
            extraData.dataEntries.push_back(missionplan.extraDataList[i].dataEntries[j]);
        }
        acdlPlan.extraDataList.push_back(extraData);
    }

    // Copy the data index list.
    acdlPlan.dataIndexList = missionplan.dataIndexList;

    // First, create a temporary file to work with.
    sprintf(tmpFilename, "/tmp/%s-cbrplanner-missionplan2acdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpFilename);
    unlink(tmpFilename);
    tmpFile = fopen(tmpFilename, "w");

    fprintf(
        tmpFile,
        "%s %s\n\n",
        ACDL_STRING_BIND_ARCH_.c_str(),
        missionplan.archType.c_str());

    for (p = 0; p < (int)(missionplan.plans.size()); p++)
    {
        fprintf(
            tmpFile,
            "%s %s\n\n",
            ACDL_STRING_MAX_VELOCITY_.c_str(),
            missionplan.maxVelValues[p].c_str());

        fprintf(
            tmpFile,
            "%s %s\n\n",
            ACDL_STRING_BASE_VELOCITY_.c_str(),
            missionplan.baseVelValues[p].c_str());

        plan = missionplan.plans[p];

        fprintf(
            tmpFile,
            "%s %s\n\n",
            ACDL_STRING_INSTANCE_NAME_.c_str(),
            ACDL_STRING_MOVEMENT_.c_str());

        fprintf(
            tmpFile,
            "%s %s\n\n",
            ACDL_STRING_INSTANCE_.c_str(),
            ACDL_STRING_START_.c_str());

        planSize = plan.size();
        for (i = 0; i < planSize; i++)
        {
            // Dump the tasks.
            fprintf(
                tmpFile,
                "%s %s\n",
                ACDL_STRING_TASK_INDEX_.c_str(),
                plan[i].index.c_str());

            if (plan[i].description != EMPTY_STRING_)
            {
                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_TASK_DESC_.c_str(),
                    plan[i].description.c_str());
            }

            parameterSize = plan[i].parameters.size();
            for (j = 0; j < parameterSize; j++)
            {
                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_PARM_NAME_.c_str(),
                    plan[i].parameters[j].name.c_str());

                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_PARM_VALUE_.c_str(),
                    plan[i].parameters[j].value.c_str());
            }

            fprintf(
                tmpFile,
                "%s %s\n",
                ACDL_STRING_AGENT_NAME_.c_str(),
                plan[i].name.c_str());
        }
    
        for (i = 0; i < planSize; i++)
        {
            // Dump the triggers.
            effectsSize = plan[i].effects.size();
            if (effectsSize == 0)
            {
                // Don't write trigger if there is no effect.
                continue;
            }

            for (j = 0; j < effectsSize; j++)
            {
                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_TRIGGERING_TASK_INDEX_.c_str(),
                    plan[i].index.c_str());

                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_IF_.c_str(),
                    ACDL_STRING_START_.c_str());
	
                parameterSize = plan[i].effects[j].parameters.size();
                for (k = 0; k < parameterSize; k++)
                {
                    fprintf(
                        tmpFile,
                        "%s %s\n",
                        ACDL_STRING_PARM_NAME_.c_str(),
                        plan[i].effects[j].parameters[k].name.c_str());

                    fprintf(
                        tmpFile,
                        "%s %s\n",
                        ACDL_STRING_PARM_VALUE_.c_str(),
                        plan[i].effects[j].parameters[k].value.c_str());
                }

                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_AGENT_NAME_.c_str(),
                    plan[i].effects[j].name.c_str());

                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_GOTO_.c_str(),
                    plan[i].effects[j].index.c_str());

                fprintf(
                    tmpFile,
                    "%s %s\n",
                    ACDL_STRING_IF_.c_str(),
                    ACDL_STRING_END_.c_str());
            }
        }

        fprintf(
            tmpFile,
            "\n%s %s\n",
            ACDL_STRING_INSTANCE_.c_str(),
            ACDL_STRING_END_.c_str());
    }

    fclose(tmpFile);

    // Next, read the file and save the code as a data.
    ifstream tmpACDLReadFile(tmpFilename);

    // Find the data size.
    tmpACDLReadFile.seekg (0, ios::end);
    acdlPlan.dataSize = tmpACDLReadFile.tellg();
    tmpACDLReadFile.seekg (0, ios::beg);

    // To append NULL at the end.
    acdlPlan.dataSize++; 

    // Read in the data.
    acdlPlan.data = new char[acdlPlan.dataSize];
    tmpACDLReadFile.read (acdlPlan.data, acdlPlan.dataSize-1);
    
    // Append a null, to make it a char string.
    acdlPlan.data[acdlPlan.dataSize-1] = '\0';

    tmpACDLReadFile.close();
    unlink(tmpFilename);

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("%s\n", acdlPlan.data);
        gDebugger->printfLine();
    }

    return acdlPlan;
}

//-----------------------------------------------------------------------
// This function converts an ACDL represented mision plan into a format
// that MemoryManager can understand.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t DomainManager::acdl2missionplan(CBRPlanner_ACDLMissionPlan_t acdlPlan)
{
    MissionMemoryManager::MissionPlan_t missionplan;
    CBRPlanner_PlanOperator_t planOperator;
    CBRPlanner_PlanSequence_t planSequence;
    char buf1[1024], buf2[1024], buf3[1024], buf4[1024], buf5[1024], buf6[1024];
    char tmpFilename[256];
    FILE *tmpFile = NULL;
    string bufString1, bufString2, bufString3, bufString4, bufString5, bufString6;
    string operatorIndex, triggerName;
    string maxVelValue = EMPTY_STRING_, baseVelValue = EMPTY_STRING_;
    CBRPlanner_Effect_t effect;
    CBRPlanner_Precondition_t precondition;
    CBRPlanner_Parameter_t parameter;
    int fd;
    int robotID = 0;
    const bool DEBUG = true;

    // Copy the type.
    missionplan.type = acdlPlan.type;
    
    // Copy the rating.
    missionplan.rating = acdlPlan.rating;

    // Copy the data index list.
    missionplan.dataIndexList = acdlPlan.dataIndexList;

    // To dump the "char*" based data in a file (and then process the
    // data using fscanf), create a temporary file.
    sprintf(tmpFilename, "/tmp/%s-cbrplanner-acdl2missionplan-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpFilename);
    unlink(tmpFilename);

    // Dump the data.
    tmpFile = fopen(tmpFilename, "w");
    fprintf(tmpFile, "%s", acdlPlan.data);
    fclose(tmpFile);

    // Now, open up the file, again, for reading.
    tmpFile = fopen(tmpFilename, "r");

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("ACDL2MISSIONPLAN:\n");
    }

    // Process the data.
    while (fscanf(tmpFile, "%s %s", buf1, buf2) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if (bufString1 == ACDL_STRING_BIND_ARCH_)
        {
            // The architecture name is found.
            missionplan.archType = bufString2;

            if (DEBUG)
            {
                gDebugger->printfLine();
                gDebugger->printfDebug(
                    "Arch Type: %s\n",
                    missionplan.archType.c_str());
            }
        }
        else if (bufString1 == ACDL_STRING_MAX_VELOCITY_)
        {
            // The max velocity is found.
            //missionplan.maxVelValue = bufString2;
            maxVelValue = bufString2;

            if (DEBUG)
            {
                gDebugger->printfLine();
                gDebugger->printfDebug(
                    "Max Velocity: %s\n",
                    //missionplan.maxVelValue.c_str());
                    maxVelValue.c_str());
            }
        }
        else if (bufString1 == ACDL_STRING_BASE_VELOCITY_)
        {
            // The base velocity is found.
            //missionplan.baseVelValue = bufString2;
            baseVelValue = bufString2;

            if (DEBUG)
            {
                gDebugger->printfLine();
                gDebugger->printfDebug(
                    "Base Velocity: %s\n",
                    //missionplan.baseVelValue.c_str());
                    baseVelValue.c_str());
            }
        }
        else if ((bufString1 == ACDL_STRING_INSTANCE_NAME_) &&
                 (bufString2 == ACDL_STRING_MOVEMENT_))
        {
            planSequence.clear();

            // Now, read about FSA.
            while (fscanf(tmpFile, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;

                if (bufString3 == ACDL_STRING_TASK_INDEX_)
                {
                    // Task index $AN_xx (or Start) is found.
                    planOperator.index = bufString4;
                }
                else if (bufString3 == ACDL_STRING_TASK_DESC_)
                {
                    planOperator.description = bufString4;
                }
                else if (bufString3 == ACDL_STRING_AGENT_NAME_)
                {
                    // Task name is found. Save in as a segment of the plan (if
                    // it is not a start state.
                    planOperator.name = bufString4;

                    if (planOperator.index != ACDL_STRING_STARTTASK_INDEX_)
                    {
                        planSequence = addOperatorToPlanSequence_(planSequence, planOperator);
                    }
                    planOperator.parameters.clear();
                }
                else if (bufString3 == ACDL_STRING_PARM_NAME_)
                {
                    parameter.name = bufString4;
                }
                else if (bufString3 == ACDL_STRING_PARM_VALUE_)
                {
                    parameter.value = bufString4;
                    planOperator.parameters.push_back(parameter);
                }
                else if (bufString3 == ACDL_STRING_TRIGGERING_TASK_INDEX_)
                {
                    // The trigger will be added to this task.
                    operatorIndex = bufString4;

                    while (fscanf(tmpFile, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == ACDL_STRING_AGENT_NAME_)
                        {
                            // Remember this trigger name.
                            triggerName = bufString6;
                            effect.name = triggerName;
                        }
                        else if (bufString5 == ACDL_STRING_PARM_NAME_)
                        {
                            parameter.name = bufString6;
                        }
                        else if (bufString5 == ACDL_STRING_PARM_VALUE_)
                        {
                            parameter.value = bufString6;
                            effect.parameters.push_back(parameter);
                        }
                        else if (bufString5 == ACDL_STRING_GOTO_)
                        {
                            effect.index = bufString6;

                            if (operatorIndex != ACDL_STRING_STARTTASK_INDEX_)
                            {
                                addEffect_(&planSequence, operatorIndex, effect);
                            }
                            else
                            {
                                setAsFirstOperator_(&planSequence, bufString6);
                            }

                            effect.parameters.clear();

                            precondition.name = triggerName;
                            addPrecondition_(&planSequence, bufString6, precondition);
                        }
                        else if ((bufString5 == ACDL_STRING_IF_) &&
                                 (bufString6 == ACDL_STRING_END_))
                        {
                            break;
                        }
                    }
                }
                else if ((bufString3 == ACDL_STRING_INSTANCE_) &&
                         (bufString4 == ACDL_STRING_END_))
                {
                    if (DEBUG)
                    {
                        gDebugger->dumpPlanSequence(planSequence);
                    }

                    missionplan.plans.push_back(planSequence);
                    missionplan.maxVelValues.push_back(maxVelValue);
                    missionplan.baseVelValues.push_back(baseVelValue);
                    missionplan.robotIDs.push_back(robotID);
                    break;
                }
            }
        }
    }

    fclose(tmpFile);
    unlink(tmpFilename);

    return missionplan;
}

//-----------------------------------------------------------------------
// This function will add an effect to the specified operator in the plan.
//-----------------------------------------------------------------------
void DomainManager::addEffect_(
    vector<CBRPlanner_PlanOperator_t> *planSequence,
    string operatorIndex,
    CBRPlanner_Effect_t effect)
{
    int i, planSize;

    planSize = planSequence->size();

    for (i = 0; i < planSize; i++)
    {
        if ((*planSequence)[i].index == operatorIndex)
        {
            (*planSequence)[i].effects.push_back(effect);
            return;
        }
    }
}

//-----------------------------------------------------------------------
// This function will add a precondition to the specified operator in the
// plan.
//-----------------------------------------------------------------------
void DomainManager::addPrecondition_(
    vector<CBRPlanner_PlanOperator_t> *planSequence,
    string operatorIndex,
    CBRPlanner_Precondition_t precondition)
{
    int i, planSize;

    planSize = planSequence->size();

    for (i = 0; i < planSize; i++)
    {
        if ((*planSequence)[i].index == operatorIndex)
        {
            (*planSequence)[i].preconditions.push_back(precondition);
            return;
        }
    }
}

//-----------------------------------------------------------------------
// This function will set the specified opeator to be the first state.
//-----------------------------------------------------------------------
void DomainManager::setAsFirstOperator_(
    vector<CBRPlanner_PlanOperator_t> *planSequence,
    string operatorIndex)
{
    vector<CBRPlanner_PlanOperator_t> newSequence;
    CBRPlanner_PlanOperator_t firstOperator;
    int i, planSize;
    bool operatorFound = false;

    planSize = planSequence->size();

    for (i = 0; i < planSize; i++)
    {
        if ((*planSequence)[i].index == operatorIndex)
        {
            firstOperator = (*planSequence)[i];
            operatorFound = true;
            break;
        }
    }

    if (!operatorFound)
    {
        gWindows->printfPlannerWindow(
            "Error: DomainManager::setAsFirstOperator_(). Unknown operator index '%s'\n",
            operatorIndex.c_str());
        return;
    }

    newSequence.push_back(firstOperator);

    for (i = 0; i < planSize; i++)
    {
        if ((*planSequence)[i].index != operatorIndex)
        {
            newSequence.push_back((*planSequence)[i]);
        }
    }

    *planSequence = newSequence;
}

//-----------------------------------------------------------------------
// This function will add an operator to a mission plan.
//-----------------------------------------------------------------------
vector<CBRPlanner_PlanOperator_t> DomainManager::addOperatorToPlanSequence_(
    vector<CBRPlanner_PlanOperator_t> planSequence,
    CBRPlanner_PlanOperator_t planOperator)
{
    vector<CBRPlanner_PlanOperator_t> newPlanSequence;

    newPlanSequence = planSequence;
    newPlanSequence.push_back(planOperator);

    return newPlanSequence;
}

//-----------------------------------------------------------------------
// This function checks to see if the mission (ACDL) contains a specified
// agent.
//-----------------------------------------------------------------------
bool DomainManager::hasAgentInMissionPlan_(CBRPlanner_ACDLMissionPlan_t acdlPlan, string agentName)
{
    FILE *tmpACDLFile = NULL;
    char buf1[1024], buf2[1024];
    char tmpFilename[256];
    string bufString1, bufString2;
    int fd;
    bool found = false;
    
    // To dump the "char*" based data in a file (and then process the
    // data using fscanf), create a temporary file.
    sprintf(tmpFilename, "/tmp/%s-cbrplanner-hasAgentInMissionPlan-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpFilename);
    unlink(tmpFilename);

    // Dump the data.
    tmpACDLFile = fopen(tmpFilename, "w");
    fprintf(tmpACDLFile, "%s", acdlPlan.data);
    fclose(tmpACDLFile);

    // Now, open up the file, again, for reading.
    tmpACDLFile = fopen(tmpFilename, "r");

    // Process the data.
    while (fscanf(tmpACDLFile, "%s", buf1) != EOF)
    {
        bufString1 = buf1;

        if (bufString1 == ACDL_STRING_AGENT_NAME_)
        {
            if (fscanf(tmpACDLFile, "%s", buf2) != EOF)
            {
                bufString2 = buf2;

                if (bufString2 == agentName)
                {
                    found = true;
                    break;
                }
            }
        }
    }

    fclose(tmpACDLFile);
    unlink(tmpFilename);

    return found;
}

/**********************************************************************
 * $Log: domain_manager.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.4  2007/08/24 22:25:21  endo
 * State ordering problem fixed.
 *
 * Revision 1.3  2007/08/09 19:21:39  endo
 * Fix to clean up a temporary file.
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
 * Revision 1.6  2006/02/19 17:52:00  endo
 * Experiment related modifications
 *
 * Revision 1.5  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.4  2005/07/31 03:41:41  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.3  2005/07/27 20:40:17  endo
 * 3D visualization improved.
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
