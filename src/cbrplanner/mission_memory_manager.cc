/**********************************************************************
 **                                                                  **
 **                     mission_memory_manager.cc                    **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This modules deals with storing and retrieving of missions.     **
 **                                                                  **
 **  Copyright 2003 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_memory_manager.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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
#include <ctype.h>
#include <sys/types.h>

#include "cbrplanner_domain.h"
#include "memory_manager.h"
#include "mission_memory_manager.h"
#include "windows.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const string MissionMemoryManager::CBRLIBRARY_EXTENSION_ = "cbl";
const string MissionMemoryManager::CBL_STRING_DATA_ = "DATA";
const string MissionMemoryManager::CBL_STRING_META_DATA_ = "META-DATA";
const string MissionMemoryManager::CBL_STRING_META_DATA_KEY_ = "META-DATA-KEY";
const string MissionMemoryManager::CBL_STRING_META_DATA_VALUE_ = "META-DATA-VALUE";
const string MissionMemoryManager::CBL_STRING_TYPE_ = "TYPE";
const string MissionMemoryManager::CBL_STRING_RATING_ = "RATING";
const string MissionMemoryManager::CBL_STRING_ARCHTYPE_ = "ARCHTYPE";
const string MissionMemoryManager::CBL_STRING_PLAN_ = "PLAN";
const string MissionMemoryManager::CBL_STRING_OPERATOR_ = "OPERATOR";
const string MissionMemoryManager::CBL_STRING_OPERATOR_NAME_ = "OPERATOR-NAME";
const string MissionMemoryManager::CBL_STRING_OPERATOR_INDEX_ = "OPERATOR-INDEX";
const string MissionMemoryManager::CBL_STRING_OPERATOR_DESC_ = "OPERATOR-DESC";
const string MissionMemoryManager::CBL_STRING_OPERATOR_PARM_ = "OPERATOR-PARM";
const string MissionMemoryManager::CBL_STRING_OPERATOR_PARM_NAME_ = "OPERATOR-PARM-NAME";
const string MissionMemoryManager::CBL_STRING_OPERATOR_PARM_VALUE_ = "OPERATOR-PARM-VALUE";
const string MissionMemoryManager::CBL_STRING_OPERATOR_PRECONDITION_ = "OPERATOR-PRECONDITION";
const string MissionMemoryManager::CBL_STRING_OPERATOR_PRECONDITION_NAME_ = "OPERATOR-PRECONDITION-NAME";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_ = "OPERATOR-EFFECT";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_NAME_ = "OPERATOR-EFFECT-NAME";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_INDEX_ = "OPERATOR-EFFECT-INDEX";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_PARM_ = "OPERATOR-EFFECT-PARM";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_PARM_NAME_ = "OPERATOR-EFFECT-PARM-NAME";
const string MissionMemoryManager::CBL_STRING_OPERATOR_EFFECT_PARM_VALUE_ = "OPERATOR-EFFECT-PARM-VALUE";
const string MissionMemoryManager::CBL_STRING_DECISIONTREE_ = "DECISIONTREE";
const string MissionMemoryManager::CBL_STRING_NODE_ = "NODE";
const string MissionMemoryManager::CBL_STRING_NODE_NAME_ = "NODE-NAME";
const string MissionMemoryManager::CBL_STRING_NODE_LEAF_ = "NODE-LEAF";
const string MissionMemoryManager::CBL_STRING_NODE_EDGE_ = "NODE-EDGE";
const string MissionMemoryManager::CBL_STRING_NODE_EDGE_PARENT_ = "NODE-EDGE-PARENT";
const string MissionMemoryManager::CBL_STRING_NODE_EDGE_VALUE_ = "NODE-EDGE-VALUE";
const string MissionMemoryManager::CBL_STRING_NODE_EDGE_NEXTNODE_ = "NODE-EDGE-NEXTNODE";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_ = "NODE-DATA";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_INDEXES_ = "NODE-DATA-INDEXES";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_INDEX_ = "NODE-DATA-INDEX";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_STATUSES_ = "NODE-DATA-STATUSES";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_STATUS_ = "NODE-DATA-STATUS";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_OTHERFEATURE_ = "NODE-DATA-OTHERFEATURE";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_OTHERFEATURE_NAME_ = "NODE-DATA-OTHERFEATURE-NAME";
const string MissionMemoryManager::CBL_STRING_NODE_DATA_OTHERFEATURE_VALUE_ = "NODE-DATA-OTHERFEATURE-VALUE";
const string MissionMemoryManager::CBL_STRING_START_ = "START";
const string MissionMemoryManager::CBL_STRING_END_ = "END";

//-----------------------------------------------------------------------
// Constructor for MissionMemoryManager class.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionMemoryManager(MissionMemoryManager **missionMemoryManagerInstance) :
    MemoryManager((MemoryManager **)missionMemoryManagerInstance),
    cbrlibraryName_(EMPTY_STRING_)
{
}

//-----------------------------------------------------------------------
// Distructor for MissionMemoryManager class.
//-----------------------------------------------------------------------
MissionMemoryManager::~MissionMemoryManager(void)
{
}

//-----------------------------------------------------------------------
// This function will save a new mission plan into its memory (decision
// tree).
//-----------------------------------------------------------------------
int MissionMemoryManager::saveNewMissionPlan(MissionPlan_t missionplan, Features_t features)
{
    int index;

    // Save the case.
    index = missionPlanList_.size();
    missionplan.dataIndexList.push_back(index);
    missionPlanList_.push_back(missionplan);
    memoryChanged_ = true;

    // Provide the feedback to the user.
    gWindows->printfPlannerWindow(
        "New case (ID %d) added to the mission memory.\n",
        index);

    addNewDataToDecisionTree_(&decisionTree_, features, index);

    return index;
}

//-----------------------------------------------------------------------
// This function will get an saved mission plan into its memory (decision
// tree).
//-----------------------------------------------------------------------
vector<MissionMemoryManager::MissionPlan_t> MissionMemoryManager::getSavedMissionPlans(
    Features_t features,
    int maxRating,
    bool disableBacktrack)
{
    MissionPlan_t missionplan;
    vector<MissionPlan_t> missionplans;
    vector<int> indexes, ratings;
    int i;

    indexes = getSavedData_(
        decisionTree_,
        features,
        maxRating,
        &ratings,
        disableBacktrack);

    for (i = 0; i < (int)(indexes.size()); i++)
    {
        missionplan = missionPlanList_[indexes[i]];
        missionplan.type = CBRPLANNER_MISSIONPLAN_OLD;
        missionplan.rating = ratings[i];
        missionplans.push_back(missionplan);
    }

    return missionplans;
}

//-----------------------------------------------------------------------
// This function attaches the extra data to the case as a metadata.
//-----------------------------------------------------------------------
void MissionMemoryManager::saveExtraDataList(
    vector<int> dataIndexList,
    vector<CBRPlanner_ExtraData_t> extraDataList)
{
    CBRPlanner_ExtraData_t extraData;
    int i, j, k, l, m, n, numIndexes, numMissionPlans;
    int dataIndex, keyType;
    bool entryFound = false;

    numIndexes = dataIndexList.size();
    numMissionPlans = missionPlanList_.size();

    for (i = 0; i < numIndexes; i++)
    {
        dataIndex = dataIndexList[i];

        // Search the data.
        for (j = 0; j < numMissionPlans; j++)
        {
            for (k = 0; k < (int)(missionPlanList_[j].dataIndexList.size()); k++)
            {
                if (missionPlanList_[j].dataIndexList[k] == dataIndex)
                {
                    // Data found. Search the appropriate entry for each extra
                    // data.
                    for (l = 0; l < (int)(extraDataList.size()); l++)
                    {
                        extraData = extraDataList[l];
                        entryFound = false;

                        // Search the key type.
                        keyType = -1;
                        for (m = 0; m < NUM_CBRPLANNER_EXTRADATA_KEYS; m++)
                        {
                            if (CBRPLANNER_EXTRADATA_KEYS[m] == extraData.key)
                            {
                                keyType = m;
                                break;
                            }
                        }

                        // Search the entry in the list.
                        for (m = 0; m < (int)(missionPlanList_[j].extraDataList.size()); m++)
                        {
                            if (missionPlanList_[j].extraDataList[m].key == extraData.key)
                            {
                                // Entry found.
                                switch (keyType) {

                                case CBRPLANNER_EXTRADATA_KEY_CREATION_TIME:
                                    // Preserve the original creation time.
                                    if ((missionPlanList_[j].extraDataList[m].numDataEntries == 0) &&
                                        (extraData.numDataEntries > 0))
                                    {
                                        missionPlanList_[j].extraDataList[m].dataEntries.clear();
                                        missionPlanList_[j].extraDataList[m].dataEntries.push_back(
                                            extraData.dataEntries[0]);
                                        missionPlanList_[j].extraDataList[m].numDataEntries = 1;
                                    }
                                    break;

                                case CBRPLANNER_EXTRADATA_KEY_NUM_USAGE:
                                case CBRPLANNER_EXTRADATA_KEY_COMPUTED_RELEVANCE:
                                    // Retain only the latest value.
                                    if (extraData.numDataEntries > 0)
                                    {
                                        missionPlanList_[j].extraDataList[m].dataEntries.clear();
                                        missionPlanList_[j].extraDataList[m].dataEntries.push_back(
                                            extraData.dataEntries[0]);
                                        missionPlanList_[j].extraDataList[m].numDataEntries = 1;
                                    }
                                    break;

                                default:
                                    // Append.
                                    for (n = 0; n < extraData.numDataEntries; n++)
                                    {
                                        missionPlanList_[j].extraDataList[m].dataEntries.push_back(
                                            extraData.dataEntries[n]);
                                    }

                                    missionPlanList_[j].extraDataList[m].numDataEntries = 
                                        missionPlanList_[j].extraDataList[m].dataEntries.size();
                                    break;
                                }

                                entryFound = true;
                                memoryChanged_ = true;
                                break;
                            }
                        }

                        if (!entryFound)
                        {
                            // New entry.
                            missionPlanList_[j].extraDataList.push_back(extraData);
                            memoryChanged_ = true;
                        }
                    }

                    break;
                }
            }
        }
    } 
}

//-----------------------------------------------------------------------
// This function increments the usage counter in the case metadata.
//-----------------------------------------------------------------------
void MissionMemoryManager::updateMissionUsageCounter(vector<int> dataIndexList)
{
    CBRPlanner_ExtraData_t extraData;
    string value;
    char buf[10240];
    int i, j, k, l, numIndexes, numMissionPlans;
    int keyType;
    int dataIndex;
    int numUsage;
    bool entryFound = false;
    const int DEFAULT_NUM_USAGE = 1;

    // Create a default extra data.
    keyType = CBRPLANNER_EXTRADATA_KEY_NUM_USAGE;
    extraData.key = CBRPLANNER_EXTRADATA_KEYS[keyType];
    extraData.format = CBRPLANNER_EXTRADATA_FORMATS[keyType];
    extraData.numDataEntries = 1;

    // Search the list.
    numIndexes = dataIndexList.size();
    numMissionPlans = missionPlanList_.size();
    for (i = 0; i < numIndexes; i++)
    {
        dataIndex = dataIndexList[i];

        // Search the data.
        for (j = 0; j < numMissionPlans; j++)
        {
            for (k = 0; k < (int)(missionPlanList_[j].dataIndexList.size()); k++)
            {
                if (missionPlanList_[j].dataIndexList[k] == dataIndex)
                {
                    entryFound = false;
                    numUsage = DEFAULT_NUM_USAGE;

                    // Search the entry in the list.
                    for (l = 0; l < (int)(missionPlanList_[j].extraDataList.size()); l++)
                    {
                        if (missionPlanList_[j].extraDataList[l].key == extraData.key)
                        {
                            if (extraData.numDataEntries > 0)
                            {
                                value = missionPlanList_[j].extraDataList[l].dataEntries[0];

                                if ((value.size() > 0) && (isdigit(value.c_str()[0])))
                                {
                                    numUsage = atoi(value.c_str()) + 1;
                                }
                            }

                            sprintf(buf, "%d", numUsage);
                            value = buf;
                            extraData.dataEntries.clear();
                            extraData.dataEntries.push_back(value);
                            missionPlanList_[j].extraDataList[l] = extraData;

                            entryFound = true;
                            memoryChanged_ = true;
                            break;
                        }
                    }

                    if (!entryFound)
                    {
                        // New entry.
                        sprintf(buf, "%d", numUsage);
                        value = buf;
                        extraData.dataEntries.clear();
                        extraData.dataEntries.push_back(value);
                        missionPlanList_[j].extraDataList.push_back(extraData);
                        memoryChanged_ = true;
                    }

                    break;
                }
            }
        }
    } 
}

//-----------------------------------------------------------------------
// This function asks the CBR library name from the user.
//-----------------------------------------------------------------------
string MissionMemoryManager::askCBRLibraryName_(void)
{
    string cbrlibraryName;
    string title;

    title = "Enter the CBR library name.";

    gWindows->clearKeyboardWindow();
    gWindows->setKeyboardWindowTitle(title);
    gWindows->printfKeyboardWindow(": ");
    cbrlibraryName = gWindows->scanfKeyboardWindow();

    if (!hasCBRLibraryExtension_(cbrlibraryName))
    {
        cbrlibraryName = addCBRLibraryExtension_(cbrlibraryName);
    }

    return cbrlibraryName;
}

//-----------------------------------------------------------------------
// This function loads a CBR library.
//-----------------------------------------------------------------------
void MissionMemoryManager::loadCBRLibrary(string cbrlibraryName)
{
    FILE *in = NULL;

    in = fopen(cbrlibraryName.c_str(), "r");

    if (in == NULL)
    {
        // Error opening the file.
        gWindows->printfPlannerWindow(
            "Warning: CBR library \"%s\" could not be loaded.\n",
            cbrlibraryName.c_str());
        return;
    }

    if (memoryChanged_)
    {
        if (!confirmClear_())
        {
            gWindows->printfPlannerWindow("CBR library not loaded.\n");
            return;
        }
    }


    // Clear the memory first.
    missionPlanList_.clear();

    if (decisionTree_ != NULL)
    {
        delete decisionTree_;
        decisionTree_ = NULL;
    }

    // Load the library.
    loadCBRLibrary_(in);

    // Close the file.
    fclose(in);

    cbrlibraryName_ = cbrlibraryName;

    // Provide the feedbacks to the memory.
    gWindows->printfPlannerWindow(
        "CBR library \"%s\" loaded.\n",
        cbrlibraryName.c_str());

    gWindows->printfPlannerWindow(
        "%d case%s in the current memory.\n",
        (int)(missionPlanList_.size()),
        ((int)(missionPlanList_.size())>1)? "s" : "");
}

//-----------------------------------------------------------------------
// This function loads a CBR library.
//-----------------------------------------------------------------------
void MissionMemoryManager::loadCBRLibrary(void)
{
    string cbrlibraryName;

    cbrlibraryName = askCBRLibraryName_();

    if (!loadVerify_(cbrlibraryName))
    {
        // The user changed the mind.
        gWindows->printfPlannerWindow("Cases not loaded.\n");
        return;
    }

    loadCBRLibrary(cbrlibraryName);
}

//-----------------------------------------------------------------------
// This function loads a CBR library.
//-----------------------------------------------------------------------
void MissionMemoryManager::loadCBRLibrary_(FILE *in)
{
    char buf1[10240], buf2[10240];
    string bufString1, bufString2;
    MissionPlan_t missionPlan;
    int index;

    while (fscanf(in, "%s %s", buf1, buf2) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if ((bufString1 == CBL_STRING_DATA_) &&
            (bufString2 == CBL_STRING_START_))
        {
            missionPlan = loadMissionPlan_(in);
            index = missionPlanList_.size();
            missionPlan.dataIndexList.push_back(index);
            missionPlanList_.push_back(missionPlan);
        }
        else if ((bufString1 == CBL_STRING_DECISIONTREE_) &&
                 (bufString2 == CBL_STRING_START_))
        {
            decisionTree_ = loadDecisionTree_(in);
        }	
    }
}

//-----------------------------------------------------------------------
// This function loads a CBR library.
//-----------------------------------------------------------------------
MissionMemoryManager::MissionPlan_t MissionMemoryManager::loadMissionPlan_(FILE *in)
{
    char buf1[10240], buf2[10240], buf3[10240], buf4[10240];
    char buf5[10240], buf6[10240], buf7[10240], buf8[10240];
    char buf9[10240], buf10[10240];
    string bufString1, bufString2, bufString3, bufString4;
    string bufString5, bufString6, bufString7, bufString8;
    string bufString9, bufString10;
    MissionPlan_t missionplan;
    CBRPlanner_PlanOperator_t planOperator;
    CBRPlanner_Parameter_t parm, effectParm;
    CBRPlanner_Precondition_t precondition;
    CBRPlanner_Effect_t effect;
    CBRPlanner_PlanSequence_t plan;
    CBRPlanner_ExtraData_t extraData;
    int i;
    
    while (fscanf(in, "%s %s", buf1, buf2) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;
		
        if (bufString1 == CBL_STRING_TYPE_)
        {
            missionplan.type = atoi(buf2);
        }
        else if (bufString1 == CBL_STRING_ARCHTYPE_)
        {
            missionplan.archType = bufString2;
        }
        else if ((bufString1 == CBL_STRING_PLAN_) &&
                 (bufString2 == CBL_STRING_START_))
        {
            plan.clear();

            while (fscanf(in, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;
		
                if ((bufString3 == CBL_STRING_OPERATOR_) &&
                    (bufString4 == CBL_STRING_START_))
                {
                    planOperator.parameters.clear();
                    planOperator.preconditions.clear();
                    planOperator.effects.clear();

                    while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == CBL_STRING_OPERATOR_NAME_)
                        {
                            planOperator.name = bufString6;
                        }
                        else if (bufString5 == CBL_STRING_OPERATOR_INDEX_)
                        {
                            planOperator.index = bufString6;
                        }
                        else if (bufString5 == CBL_STRING_OPERATOR_DESC_)
                        {
                            planOperator.description = bufString6;
                        }
                        else if ((bufString5 == CBL_STRING_OPERATOR_PARM_) &&
                                 (bufString6 == CBL_STRING_START_))
                        {
                            while (fscanf(in, "%s %s", buf7, buf8) != EOF)
                            {
                                bufString7 = buf7;	
                                bufString8 = buf8;
				
                                if (bufString7 == CBL_STRING_OPERATOR_PARM_NAME_)
                                {
                                    parm.name = bufString8; 
                                }
                                else if (bufString7 == CBL_STRING_OPERATOR_PARM_VALUE_)
                                {
                                    parm.value = bufString8; 
                                }
                                else if ((bufString7 == CBL_STRING_OPERATOR_PARM_) &&
                                         (bufString8 == CBL_STRING_END_))
                                {
                                    planOperator.parameters.push_back(parm);
                                    break;
                                }
                            }
                        }
                        else if ((bufString5 == CBL_STRING_OPERATOR_PRECONDITION_) &&
                                 (bufString6 == CBL_STRING_START_))
                        {
                            while (fscanf(in, "%s %s", buf7, buf8) != EOF)
                            {
                                bufString7 = buf7;	
                                bufString8 = buf8;

                                if (bufString7 == CBL_STRING_OPERATOR_PRECONDITION_NAME_)
                                {
                                    precondition.name = bufString8; 
                                }
                                else if ((bufString7 == CBL_STRING_OPERATOR_PRECONDITION_) &&
                                         (bufString8 == CBL_STRING_END_))
                                {
                                    planOperator.preconditions.push_back(precondition);
                                    break;
                                }
                            }
                        }
                        else if ((bufString5 == CBL_STRING_OPERATOR_EFFECT_) &&
                                 (bufString6 == CBL_STRING_START_))
                        {
                            effect.parameters.clear();

                            while (fscanf(in, "%s %s", buf7, buf8) != EOF)
                            {
                                bufString7 = buf7;	
                                bufString8 = buf8;

                                if (bufString7 == CBL_STRING_OPERATOR_EFFECT_NAME_)
                                {
                                    effect.name = bufString8; 
                                }
                                else if (bufString7 == CBL_STRING_OPERATOR_EFFECT_INDEX_)
                                {
                                    effect.index = bufString8; 
                                }
                                else if ((bufString7 == CBL_STRING_OPERATOR_EFFECT_PARM_) &&
                                         (bufString8 == CBL_STRING_START_))
                                {
                                    while (fscanf(in, "%s %s", buf9, buf10) != EOF)
                                    {
                                        bufString9 = buf9;
                                        bufString10 = buf10;

                                        if (bufString9 == CBL_STRING_OPERATOR_EFFECT_PARM_NAME_)
                                        {
                                            effectParm.name = bufString10; 
                                        }
                                        else if (bufString9 == CBL_STRING_OPERATOR_EFFECT_PARM_VALUE_)
                                        {
                                            effectParm.value = bufString10; 
                                        }
                                        else if ((bufString9 == CBL_STRING_OPERATOR_EFFECT_PARM_) &&
                                                 (bufString10 == CBL_STRING_END_))
                                        {
                                            effect.parameters.push_back(effectParm);
                                            break;
                                        }
                                    }
                                }
                                else if ((bufString7 == CBL_STRING_OPERATOR_EFFECT_) &&
                                         (bufString8 == CBL_STRING_END_))
                                {
                                    planOperator.effects.push_back(effect);
                                    break;
                                }
                            }
                        }
                        else if ((bufString5 == CBL_STRING_OPERATOR_) &&
                                 (bufString6 == CBL_STRING_END_))
                        {
                            plan.push_back(planOperator);
                            break;
                        }
                    }
                }
                else if ((bufString3 == CBL_STRING_PLAN_) &&
                         (bufString4 == CBL_STRING_END_))
                {
                    missionplan.plans.push_back(plan);
                    break;
                }
            }
        }
        else if ((bufString1 == CBL_STRING_META_DATA_) &&
                 (bufString2 == CBL_STRING_START_))
        {
            if (fscanf(in, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;

                if (bufString3 == CBL_STRING_META_DATA_KEY_)
                {
                    // found a key, now get the index for this key
                    for (i=0; i < NUM_CBRPLANNER_EXTRADATA_KEYS; i++)
                    {
                        if (bufString4 == CBRPLANNER_EXTRADATA_KEYS[i])
                            break;
                    }
		 
                    if (i < NUM_CBRPLANNER_EXTRADATA_KEYS) 
                    {
                        extraData.key = CBRPLANNER_EXTRADATA_KEYS[i];
                        extraData.format = CBRPLANNER_EXTRADATA_FORMATS[i];		      

                        extraData.numDataEntries = 0;
                        extraData.dataEntries.clear();
		      
                        while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                        {
                            bufString5 = buf5;
                            bufString6 = buf6;

                            if (bufString5 == CBL_STRING_META_DATA_VALUE_)
                            {
                                extraData.dataEntries.push_back(bufString6);
                                extraData.numDataEntries = extraData.dataEntries.size();
                            }
                            else if ((bufString5 == CBL_STRING_META_DATA_) &&
                                     (bufString6 == CBL_STRING_END_))
                            {
                                break;
                            }
                        }

                        if (extraData.numDataEntries > 0)
                            missionplan.extraDataList.push_back(extraData);
                    }
		  
                }
                else if ((bufString1 == CBL_STRING_META_DATA_) &&
                         (bufString2 == CBL_STRING_END_))
                {
                    break;
                }
            }

        }
        else if ((bufString1 == CBL_STRING_DATA_) &&
                 (bufString2 == CBL_STRING_END_))
        {
            break;
        }
    }

    return missionplan;
}

//-----------------------------------------------------------------------
// This function loads a decision tree from a file.
//-----------------------------------------------------------------------
MemoryManager::DecisionTree_t *MissionMemoryManager::loadDecisionTree_(FILE *in)
{
    DecisionTree_t *decisionTree = NULL;

    decisionTree = loadNode_(in);

    return decisionTree;
}

//-----------------------------------------------------------------------
// This function loads a node from a file.
//-----------------------------------------------------------------------
MissionMemoryManager::DecisionTreeData_t MissionMemoryManager::loadNodeData_(FILE *in)
{
    char buf1[10240], buf2[10240], buf3[10240], buf4[10240];
    char buf5[10240], buf6[10240];
    string bufString1, bufString2, bufString3, bufString4;
    string bufString5, bufString6;
    DecisionTreeData_t data;
    Feature_t otherFeature;
    int index, status;
    bool dataFound = false;

    while ((fscanf(in, "%s %s", buf1, buf2) != EOF) && (!dataFound))
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if ((bufString1 == CBL_STRING_NODE_DATA_) &&
            (bufString2 == CBL_STRING_START_))
        {

            while (fscanf(in, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;

                if ((bufString3 == CBL_STRING_NODE_DATA_INDEXES_) &&
                    (bufString4 == CBL_STRING_START_))
                {
                    while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == CBL_STRING_NODE_DATA_INDEX_)
                        {
                            index = atoi(buf6);
                            data.indexes.push_back(index);
                        }
                        else if ((bufString5 == CBL_STRING_NODE_DATA_INDEXES_) &&
                                 (bufString6 == CBL_STRING_END_))
                        {
                            break;
                        }
                    }  
                }
                if ((bufString3 == CBL_STRING_NODE_DATA_STATUSES_) &&
                    (bufString4 == CBL_STRING_START_))
                {
                    while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == CBL_STRING_NODE_DATA_STATUS_)
                        {
                            status = atoi(buf6);
                            data.statuses.push_back(status);
                        }
                        else if ((bufString5 == CBL_STRING_NODE_DATA_STATUSES_) &&
                                 (bufString6 == CBL_STRING_END_))
                        {
                            break;
                        }
                    }  
                }
                else if ((bufString3 == CBL_STRING_NODE_DATA_OTHERFEATURE_) &&
                         (bufString4 == CBL_STRING_START_))
                {
                    while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;

                        if (bufString5 == CBL_STRING_NODE_DATA_OTHERFEATURE_NAME_)
                        {
                            otherFeature.name = bufString6;
                        }
                        else if (bufString5 == CBL_STRING_NODE_DATA_OTHERFEATURE_VALUE_)
                        {
                            otherFeature.value = bufString6;
                        }
                        else if ((bufString5 == CBL_STRING_NODE_DATA_OTHERFEATURE_) &&
                                 (bufString6 == CBL_STRING_END_))
                        {
                            data.otherFeatures.push_back(otherFeature);
                            break;
                        }
                    }

                }
                else if ((bufString3 == CBL_STRING_NODE_DATA_) &&
                         (bufString4 == CBL_STRING_END_))
                {
                    dataFound = true;
                    break;
                }
            }
        }
    }    

    return data;
}

//-----------------------------------------------------------------------
// This function loads a node from a file.
//-----------------------------------------------------------------------
MemoryManager::DecisionTreeNode_t *MissionMemoryManager::loadNode_(FILE *in)
{
    char buf1[10240], buf2[10240], buf3[10240], buf4[10240];
    char buf5[10240], buf6[10240];
    string bufString1, bufString2, bufString3, bufString4;
    string bufString5, bufString6;
    DecisionTreeNode_t *node = NULL;
    DecisionTreeEdge_t edge;

    while (fscanf(in, "%s %s", buf1, buf2) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if ((bufString1 == CBL_STRING_NODE_) &&
            (bufString2 == CBL_STRING_START_))
        {
            node = new DecisionTreeNode_t;

            while (fscanf(in, "%s %s", buf3, buf4) != EOF)
            {
                bufString3 = buf3;
                bufString4 = buf4;

                if (bufString3 == CBL_STRING_NODE_NAME_)
                {
                    node->name = bufString4;
                }
                else if (bufString3 == CBL_STRING_NODE_LEAF_)
                {
                    node->isLeafNode = atoi(buf4);

                    if (node->isLeafNode)
                    {
                        node->data = loadNodeData_(in);
                        return node;
                    }

                }
                else if ((bufString3 == CBL_STRING_NODE_EDGE_) &&
                         (bufString4 == CBL_STRING_START_))
                {
                    while (fscanf(in, "%s %s", buf5, buf6) != EOF)
                    {
                        bufString5 = buf5;
                        bufString6 = buf6;
			
                        if (bufString5 == CBL_STRING_NODE_EDGE_PARENT_)
                        {
                            edge.parentName = bufString6;
                        }
                        else if (bufString5 == CBL_STRING_NODE_EDGE_VALUE_)
                        {
                            edge.value = bufString6;
                        }
                        else if ((bufString5 == CBL_STRING_NODE_EDGE_NEXTNODE_) &&
                                 (bufString6 == CBL_STRING_START_))
                        {
                            edge.nextNode = (void *)(loadNode_(in));
                        }
                        else if ((bufString5 == CBL_STRING_NODE_EDGE_) &&
                                 (bufString6 == CBL_STRING_END_))
                        {
                            node->edges.push_back(edge);
                            break;
                        }
                    }
                }
                else if ((bufString3 == CBL_STRING_NODE_) &&
                         (bufString4 == CBL_STRING_END_))
                {
                    return node;
                }
            }
        }
        else if ((bufString1 == CBL_STRING_DECISIONTREE_) &&
                 (bufString2 == CBL_STRING_END_))
        {
            break;
        }
        else if ((bufString1 == CBL_STRING_NODE_EDGE_NEXTNODE_) &&
                 (bufString2 == CBL_STRING_END_))
        {
            break;
        }
    }

    return node;
}

//-----------------------------------------------------------------------
// This function saves the current cases into a library.
//-----------------------------------------------------------------------
void MissionMemoryManager::saveCBRLibrary(void)
{
    string cbrlibraryName;

    cbrlibraryName = askCBRLibraryName_();

    if (!saveVerify_(cbrlibraryName))
    {
        // The user changed the mind.
        gWindows->printfPlannerWindow("Cases not saved.\n");
        return;
    }

    saveCBRLibrary_(cbrlibraryName);
}

//-----------------------------------------------------------------------
// This function saves the current cases into a library.
//-----------------------------------------------------------------------
void MissionMemoryManager::saveCBRLibrary(string cbrlibraryName)
{
    if (!hasCBRLibraryExtension_(cbrlibraryName))
    {
        cbrlibraryName = addCBRLibraryExtension_(cbrlibraryName);
    }

    saveCBRLibrary_(cbrlibraryName);
}

//-----------------------------------------------------------------------
// This function saves the current cases into a library.
//-----------------------------------------------------------------------
void MissionMemoryManager::saveCBRLibrary_(string cbrlibraryName)
{
    FILE *out = NULL;

    out = fopen(cbrlibraryName.c_str(), "w");

    if (out == NULL)
    {
        // Error opening the file.
        gWindows->printfPlannerWindow(
            "Warning: \"%s\" could not be opend. Cases not saved.\n",
            cbrlibraryName.c_str());

        return;
    }

    // Write the data (i.e., mission plans).
    writeMissionPlans_(out);

    // Write the mission decision tree.
    writeDecisionTree_(out, decisionTree_);

    // Close the file.
    fclose(out);

    // Report the status.
    gWindows->printfPlannerWindow("Cases saved in \"%s\".\n", cbrlibraryName.c_str());

    cbrlibraryName_ = cbrlibraryName;
    memoryChanged_ = false;
}

//-----------------------------------------------------------------------
// This function saves the entire decision tree in a file.
//-----------------------------------------------------------------------
void MissionMemoryManager::writeDecisionTree_(FILE *out, DecisionTree_t *decisionTree)
{
    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_DECISIONTREE_.c_str(),
        CBL_STRING_START_.c_str());

    writeNode_(out, decisionTree);

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_DECISIONTREE_.c_str(),
        CBL_STRING_END_.c_str());

    fflush(out);
}

//-----------------------------------------------------------------------
// This function save data in a file. 
//-----------------------------------------------------------------------
void MissionMemoryManager::writeNodeData_(FILE *out, DecisionTreeData_t data)
{
    Feature_t otherFeature;
    int i, index, status;

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_.c_str(),
        CBL_STRING_START_.c_str());

    // Write indexes.
    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_INDEXES_.c_str(),
        CBL_STRING_START_.c_str());

    for (i = 0; i < (int)(data.indexes.size()); i++)
    {
        index = data.indexes[i];

        fprintf(
            out,
            "%s %d\n",
            CBL_STRING_NODE_DATA_INDEX_.c_str(),
            index);
    }

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_INDEXES_.c_str(),
        CBL_STRING_END_.c_str());

    // Write statuses.
    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_STATUSES_.c_str(),
        CBL_STRING_START_.c_str());

    for (i = 0; i < (int)(data.statuses.size()); i++)
    {
        status = data.statuses[i];

        fprintf(
            out,
            "%s %d\n",
            CBL_STRING_NODE_DATA_STATUS_.c_str(),
            status);
    }

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_STATUSES_.c_str(),
        CBL_STRING_END_.c_str());

    // Write features.
    for (i = 0; i < (int)(data.otherFeatures.size()); i++)
    {
        otherFeature = data.otherFeatures[i];

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_DATA_OTHERFEATURE_.c_str(),
            CBL_STRING_START_.c_str());

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_DATA_OTHERFEATURE_NAME_.c_str(),
            otherFeature.name.c_str());

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_DATA_OTHERFEATURE_VALUE_.c_str(),
            otherFeature.value.c_str());

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_DATA_OTHERFEATURE_.c_str(),
            CBL_STRING_END_.c_str());
    }

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_DATA_.c_str(),
        CBL_STRING_END_.c_str());
}

//-----------------------------------------------------------------------
// This function save a node in a file. 
//-----------------------------------------------------------------------
void MissionMemoryManager::writeNode_(FILE *out, DecisionTreeNode_t *node)
{
    int i;
    DecisionTreeEdge_t edge;
    DecisionTreeNode_t *nextNode;

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_.c_str(),
        CBL_STRING_START_.c_str());

    if (node->name != EMPTY_STRING_)
    {
        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_NAME_.c_str(),
            node->name.c_str());
    }

    fprintf(
        out,
        "%s %d\n",
        CBL_STRING_NODE_LEAF_.c_str(),
        node->isLeafNode);

    if (node->isLeafNode)
    {
        writeNodeData_(out, node->data);
        return;
    }

    for (i = 0; i < (int)(node->edges.size()); i ++)
    {
        edge = node->edges[i];
        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_EDGE_.c_str(),
            CBL_STRING_START_.c_str());

        if (edge.parentName != EMPTY_STRING_)
        {
            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_NODE_EDGE_PARENT_.c_str(),
                edge.parentName.c_str());
        }

        if (edge.value != EMPTY_STRING_)
        {
            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_NODE_EDGE_VALUE_.c_str(),
                edge.value.c_str());
        }

        nextNode = (DecisionTreeNode_t *)(edge.nextNode);

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_EDGE_NEXTNODE_.c_str(),
            CBL_STRING_START_.c_str());

        writeNode_(out, nextNode);

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_EDGE_NEXTNODE_.c_str(),
            CBL_STRING_END_.c_str());

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_NODE_EDGE_.c_str(),
            CBL_STRING_END_.c_str());
    }

    fprintf(
        out,
        "%s %s\n",
        CBL_STRING_NODE_.c_str(),
        CBL_STRING_END_.c_str());

    fflush(out);
}

//-----------------------------------------------------------------------
// This function save the entire decision tree.
//-----------------------------------------------------------------------
void MissionMemoryManager::writeMissionPlans_(FILE *out)
{
    int i, j, k, l, p;
    MissionPlan_t missionplan;
    CBRPlanner_PlanOperator_t planOperator;
    CBRPlanner_Parameter_t parm, effectParm;
    CBRPlanner_Precondition_t precondition;
    CBRPlanner_Effect_t effect;
    CBRPlanner_PlanSequence_t plan;

    for (i = 0; i < (int)(missionPlanList_.size()); i++)
    {
        missionplan = missionPlanList_[i];
        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_DATA_.c_str(),
            CBL_STRING_START_.c_str());

        fprintf(
            out,
            "%s %d\n",
            CBL_STRING_TYPE_.c_str(),
            missionplan.type);

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_ARCHTYPE_.c_str(),
            missionplan.archType.c_str());

        for (p = 0; p < (int)(missionplan.plans.size()); p++)
        {
            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_PLAN_.c_str(),
                CBL_STRING_START_.c_str());

            plan = missionplan.plans[p];

            for (j = 0; j < (int)(plan.size()); j++)
            {
                planOperator = plan[j];
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_OPERATOR_.c_str(),
                    CBL_STRING_START_.c_str());
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_OPERATOR_NAME_.c_str(),
                    planOperator.name.c_str());
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_OPERATOR_INDEX_.c_str(),
                    planOperator.index.c_str());
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_OPERATOR_DESC_.c_str(),
                    planOperator.description.c_str());

                for (k = 0; k < (int)(planOperator.parameters.size()); k++)
                {
                    parm = planOperator.parameters[k];
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PARM_.c_str(),
                        CBL_STRING_START_.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PARM_NAME_.c_str(),
                        parm.name.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PARM_VALUE_.c_str(),
                        parm.value.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PARM_.c_str(),
                        CBL_STRING_END_.c_str());
                }

                for (k = 0; k < (int)(planOperator.preconditions.size()); k++)
                {
                    precondition = planOperator.preconditions[k];
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PRECONDITION_.c_str(),
                        CBL_STRING_START_.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PRECONDITION_NAME_.c_str(),
                        precondition.name.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_PRECONDITION_.c_str(),
                        CBL_STRING_END_.c_str());
                }

                for (k = 0; k < (int)(planOperator.effects.size()); k++)
                {
                    effect = planOperator.effects[k];
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_EFFECT_.c_str(),
                        CBL_STRING_START_.c_str());
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_EFFECT_NAME_.c_str(),
                        effect.name.c_str());

                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_EFFECT_INDEX_.c_str(),
                        effect.index.c_str());

                    for (l = 0; l < (int)(effect.parameters.size()); l++)
                    {
                        effectParm = effect.parameters[l];
                        fprintf(
                            out,
                            "%s %s\n",
                            CBL_STRING_OPERATOR_EFFECT_PARM_.c_str(),
                            CBL_STRING_START_.c_str());
                        fprintf(
                            out,
                            "%s %s\n",
                            CBL_STRING_OPERATOR_EFFECT_PARM_NAME_.c_str(),
                            effectParm.name.c_str());
                        fprintf(
                            out,
                            "%s %s\n",
                            CBL_STRING_OPERATOR_EFFECT_PARM_VALUE_.c_str(),
                            effectParm.value.c_str());
                        fprintf(
                            out,
                            "%s %s\n",
                            CBL_STRING_OPERATOR_EFFECT_PARM_.c_str(),
                            CBL_STRING_END_.c_str());
                    }
                    fprintf(
                        out,
                        "%s %s\n",
                        CBL_STRING_OPERATOR_EFFECT_.c_str(),
                        CBL_STRING_END_.c_str());
                }
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_OPERATOR_.c_str(),
                    CBL_STRING_END_.c_str());
            }

            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_PLAN_.c_str(),
                CBL_STRING_END_.c_str());
        }

        // Now write out the META-DATA, if any
        for (j = 0; j < (int) (missionplan.extraDataList.size()); j++) 
        {
            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_META_DATA_.c_str(),
                CBL_STRING_START_.c_str());

            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_META_DATA_KEY_.c_str(),
                missionplan.extraDataList[j].key.c_str());

            for (k = 0; k < (int) (missionplan.extraDataList[j].dataEntries.size()); k++)
            {
                fprintf(
                    out,
                    "%s %s\n",
                    CBL_STRING_META_DATA_VALUE_.c_str(),
                    missionplan.extraDataList[j].dataEntries[k].c_str());
            }	      
	    
            fprintf(
                out,
                "%s %s\n",
                CBL_STRING_META_DATA_.c_str(),
                CBL_STRING_END_.c_str());
        }

        fprintf(
            out,
            "%s %s\n",
            CBL_STRING_DATA_.c_str(),
            CBL_STRING_END_.c_str());
    }

    fflush(out);
}

//-----------------------------------------------------------------------
// This function asks the user to verify the file name.
//-----------------------------------------------------------------------
bool MissionMemoryManager::saveVerify_(string cbrlibraryName)
{
    string title;
    char c;
    bool save = false;

    title = "Save to \"";
    title += cbrlibraryName;
    title += "\"?";

    gWindows->clearKeyboardWindow();
    gWindows->printfKeyboardWindow("s)ave,  ");
    gWindows->printfKeyboardWindow("c)ancel");
    gWindows->setKeyboardWindowTitle(title);

    c = getch();

    switch(c) {

    case 's':
    case 'S':
        save = true;
        break;
    }

    return save;
}

//-----------------------------------------------------------------------
// This function asks the user to verify the file name.
//-----------------------------------------------------------------------
bool MissionMemoryManager::loadVerify_(string cbrlibraryName)
{
    string title;
    char c;
    bool load = false;

    title = "Load \"";
    title += cbrlibraryName;
    title += "\"?";

    gWindows->clearKeyboardWindow();
    gWindows->printfKeyboardWindow("l)oad,  ");
    gWindows->printfKeyboardWindow("c)ancel");
    gWindows->setKeyboardWindowTitle(title);

    c = getch();

    switch(c) {

    case 'l':
    case 'L':
        load = true;
        break;
    }

    return load;
}

//-----------------------------------------------------------------------
// This function asks the user to confirm deletion of the current cases.
//-----------------------------------------------------------------------
bool MissionMemoryManager::confirmClear_(void)
{
    string title;
    char c;
    bool clear = false;

    title = "Discard all cases in the current memory?";

    gWindows->clearKeyboardWindow();
    gWindows->printfKeyboardWindow("d)iscard,  ");
    gWindows->printfKeyboardWindow("c)ancel");
    gWindows->setKeyboardWindowTitle(title);

    c = getch();

    switch(c) {

    case 'd':
    case 'D':
        clear = true;
        break;
    }

    return clear;
}

//-----------------------------------------------------------------------
// This function checks to see it has an appropriate extension for the
// library.
//-----------------------------------------------------------------------
bool MissionMemoryManager::hasCBRLibraryExtension_(string cbrlibraryName)
{
    int index, size, extensionSize;
    string extension, possibleExtension;

    extension =  STRING_PERIOD_ + CBRLIBRARY_EXTENSION_;

    size = cbrlibraryName.size();

    extensionSize = extension.size();

    index = size - extensionSize;

    if (index < 1)
    {
        return false;
    }

    possibleExtension = cbrlibraryName.substr(index, extensionSize);

    if (possibleExtension != extension)
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function adds an appropriate extension for the library.
//-----------------------------------------------------------------------
string MissionMemoryManager::addCBRLibraryExtension_(string cbrlibraryName)
{
    int size;
    string extension;

    size = cbrlibraryName.size();

    if (size == 0)
    {
        cbrlibraryName = createCBRLibraryNamePrefix_();
    }

    extension =  STRING_PERIOD_ + CBRLIBRARY_EXTENSION_;

    cbrlibraryName += extension;

    return cbrlibraryName;
}

//-----------------------------------------------------------------------
// This function sets the default CBR library name prefix.
//-----------------------------------------------------------------------
string MissionMemoryManager::createCBRLibraryNamePrefix_(void)
{
    time_t cur_time;
    tm local_time;
    string prefix;
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];
    char *userName = NULL, buf[10240];

    // Get the user name.
    userName = getenv("USER");

    // Get the current date and time.
    cur_time = time(NULL);
    strncpy(date,ctime(&cur_time),sizeof(date));
    local_time = *localtime(&cur_time);
    strftime(month, sizeof(month), "%m", &local_time);
    strftime(day, sizeof(day), "%d", &local_time);
    strftime(year, sizeof(year), "%Y", &local_time);
    strftime(hour, sizeof(hour), "%H", &local_time);
    strftime(min, sizeof(min), "%M", &local_time);
    strftime(sec, sizeof(sec), "%S", &local_time);

    // Create the prefix.
    sprintf(buf, "cbrserver-%s-%s%s%s-%s%s%s",
            userName,
            month,
            day,
            year,
            hour,
            min,
            sec);
    prefix = buf;

    return prefix;
}

/**********************************************************************
 * $Log: mission_memory_manager.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.7  2007/09/17 18:23:34  endo
 * Increasing the buffer memory for reading from a file.
 *
 * Revision 1.6  2007/08/10 15:15:39  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.5  2007/08/06 22:08:47  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.4  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.3  2007/08/02 16:33:32  endo
 * Metadata bug fix.
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
 * Revision 1.2  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1  2005/02/07 19:53:44  endo
 * Mods from usability-2004
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
