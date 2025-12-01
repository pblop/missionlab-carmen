/**********************************************************************
 **                                                                  **
 **                        cfgedit_cbrclient.cc                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the CfgEditCBRClient class,    **
 **  whose job is to act as an interface between CfgEdit and         **
 **  CBRServer.                                                      **
 **                                                                  **
 **  Copyright 2003 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cfgedit_cbrclient.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/un.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#include "cfgedit_cbrclient.h"
#include "cbrplanner_protocol.h"
#include "mission_expert.h"
#include "mission_expert_types.h"
#include "globals.h"
#include "EventLogging.h"
#include "string_utils.h"
#include "cnp_types.h"

using std::string;
using std::vector;
using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
// Feature file related constants.
const string CfgEditCBRClient::STRING_TASK_ = "Task";
const string CfgEditCBRClient::STRING_START_X_ = "StartXCoord";
const string CfgEditCBRClient::STRING_START_Y_ = "StartYCoord";
const string CfgEditCBRClient::STRING_START_LATITUDE_ = "StartLatitude";
const string CfgEditCBRClient::STRING_START_LONGITUDE_ = "StartLongitude";
const string CfgEditCBRClient::STRING_ROBOT_CONSTRAINTS_ = "RobotConstraints";
const char CfgEditCBRClient::CHAR_AMPERSAND_ = '&';
const char CfgEditCBRClient::CHAR_DOUBLE_QUOTE_ = '\"';

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for CfgEditCBRClient class.
//-----------------------------------------------------------------------
CfgEditCBRClient::CfgEditCBRClient(const symbol_table<rc_chain> &rcTable) :
    CBRClient(rcTable)
{
    clientInfo_.type = CBRPLANNER_CLIENT_CFGEDIT;
    clientInfo_.pid = getpid();
    clientInfo_.name = "CfgEdit";
}

//-----------------------------------------------------------------------
// Distructor for MlabCBRClient class.
//-----------------------------------------------------------------------
CfgEditCBRClient::~CfgEditCBRClient(void)
{
}

//-----------------------------------------------------------------------
// This function uploads the FSA (tasks and triggers) names to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::saveFSANames(config_fsa_lists_t fsaLists)
{
    pthread_mutex_lock(&socketMutex_);
    sendFSANamesToServer_(fsaLists);
    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function saves the new mission plan into the CBRServer.
//-----------------------------------------------------------------------
vector<int> CfgEditCBRClient::saveMissionPlan(
    vector<MExpFeature_t> globalFeatures_,
    MExpMissionTask_t task,
    double taskWeight,
    string solutionFileName)
{
    CBRPlanner_ExtraData_t extraData;
    MExpFeature_t globalFeature;
    MExpFeature_t localFeature;
    CBRPlanner_MissionPlan_t missionplan;
    CBRPlanner_Feature_t feature;
    vector<CBRPlanner_Feature_t> features;
    vector<int> dataIndexList;
    vector<CBRPlanner_ExtraData_t> extraDataList;
    string value;
    time_t curTime;
    char buf[1024];
    int i, j, k;
    const bool SEND_CREATION_TIME = true;

    // Describe the mission plan type.
    missionplan.type = CBRPLANNER_MISSIONPLAN_NEW;

    // Describe the mission plan rating.
    missionplan.rating = 0;

    // Open the solution file to read.
    ifstream istr(solutionFileName.c_str());

    if (istr.bad()) 
    {
        fprintf
            (stderr,
             "Error(cfgedit): CfgEditCBRClient::saveMissionPlan(). file %s not readable",
             solutionFileName.c_str());
        return dataIndexList;
    }

    istr.seekg (0, ios::end);
    missionplan.dataSize = istr.tellg();
    istr.seekg (0, ios::beg);

    // To append NULL at the end.
    missionplan.dataSize++; 

    // Read in the solution.
    missionplan.data = new char[missionplan.dataSize];
    istr.read (missionplan.data, missionplan.dataSize-1);

    // Append a null, to make it a char string.
    missionplan.data[missionplan.dataSize-1] = CHAR_NULL_;

    // Close the solution file.
    istr.close();

    // Number of features.
    i = 0;

    // Save the global features.
    for (j = 0; j<(int)globalFeatures_.size(); j++)
    {
        if (globalFeature.selectedOption < 0)
        {
            // Disabled. Skip it.
            continue;
        }

        globalFeature = globalFeatures_[j];
        feature.name = globalFeature.name;
        feature.nonIndex = globalFeature.nonIndex;
        feature.weight = globalFeature.weight;

        switch (globalFeature.optionType) {
	    
        case MEXP_FEATURE_OPTION_TOGGLE:
            for (k = 0; k < ((int)(globalFeature.options.size())); k++)
            {
                if (globalFeature.selectedOption == k)
                {
                    feature.value = globalFeature.options[k].value;
                    features.push_back(feature);
                    break;
                }
            }
            break;

        case MEXP_FEATURE_OPTION_SLIDER1:
            sprintf(buf, "%.2f", globalFeature.selectedOption);
            feature.value = buf;
            features.push_back(feature);
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
            sprintf(buf, "%d", (int)(globalFeature.selectedOption));
            feature.value = buf;
            features.push_back(feature);
            break;

        case MEXP_FEATURE_OPTION_POLYGONS:
            feature.value = EMPTY_STRING_;
            for (k = 0; k < ((int)(globalFeature.options.size())); k++)
            {
                if (globalFeature.selectedOption == k)
                {
                    feature.value += globalFeature.options[k].value;
                    break;
                }
            }

            features.push_back(feature);
            break;

        case MEXP_FEATURE_OPTION_STRINGS:
            feature.value = EMPTY_STRING_;
            for (k = 0; k < ((int)(globalFeature.options.size())); k++)
            {
                if (globalFeature.selectedOption == k)
                {
                    feature.value += globalFeature.options[k].value;
                    break;
                }
            }

            features.push_back(feature);
            break;
        }
    }

    // Save the task name.
    feature.name = STRING_TASK_;
    feature.value = task.name;
    feature.nonIndex = false;
    feature.weight = taskWeight;
    features.push_back(feature);

    // Save other local features.
    for (j = 0; j < ((int)(task.localFeatures.size())); j++)
    {
        localFeature = task.localFeatures[j];
        feature.name = localFeature.name;
        feature.nonIndex = localFeature.nonIndex;
        feature.weight = localFeature.weight;

        switch (localFeature.optionType) {
	    
        case MEXP_FEATURE_OPTION_TOGGLE:
            for (k = 0; k < ((int)(localFeature.options.size())); k++)
            {
                if (localFeature.selectedOption == k)
                {
                    feature.value = localFeature.options[k].value;
                    features.push_back(feature);
                    break;
                }
            }
            break;

        case MEXP_FEATURE_OPTION_SLIDER1:
            sprintf(buf, "%.2f", localFeature.selectedOption);
            feature.value = buf;
            features.push_back(feature);
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
            sprintf(buf, "%d", (int)(localFeature.selectedOption));
            feature.value = buf;
            features.push_back(feature);
            break;
        }
    }
    
    dataIndexList = sendMissionPlanToServer_(missionplan, features);

    if (SEND_CREATION_TIME)
    {
        // Send the creation time as the metadata.
        extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_CREATION_TIME];
        extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_CREATION_TIME];
        extraData.numDataEntries = 1;
        time(&curTime);
        sprintf(buf, "%d", (int)curTime);
        value = buf;
        extraData.dataEntries.push_back(value);

        extraDataList.push_back(extraData);
        sendExtraDataListToServer_(dataIndexList, extraDataList);
    }

    return dataIndexList;
}

//-----------------------------------------------------------------------
// This function retrieve a mission planfrom the CBRServer based on the
// features defined in the specified feature file.
//-----------------------------------------------------------------------
vector<MExpRetrievedMission_t> CfgEditCBRClient::cbrplannerMissionsPlan2RetrievedMissions_(
    vector<CBRPlanner_MissionPlan_t> missionPlans)
{
    vector<MExpRetrievedMission_t> retrievedMissions;
    MExpRetrievedMission_t retrievedMission;
    const MExpRetrievedMission_t EMPTY_RETRIEVED_MISSION = {0, 0, 0, vector<int>(), string(), {0, 0, 0, {}, vector<string>()}, 0, 0, 0};
    int i;
    
    for (i = 0; i < (int)(missionPlans.size()); i++)
    {
        retrievedMission = EMPTY_RETRIEVED_MISSION;

        if (cbrplannerMissionPlan2RetrievedMission_(
                missionPlans[i],
                &retrievedMission))
        {
            retrievedMission.index = retrievedMissions.size();
            retrievedMissions.push_back(retrievedMission);
        }
    }

    return retrievedMissions;
}

//-----------------------------------------------------------------------
// This function converts vector<CBRPlanner_ExtraData_t> to MExpMetadata_t.
//-----------------------------------------------------------------------
bool CfgEditCBRClient::cbrplannerExtraDataList2Metadata_(
    vector<CBRPlanner_ExtraData_t> extraDataList,
    MExpMetadata_t *metadata)
{
    CBRPlanner_ExtraData_t extraData;
    string comment;
    int totalRating;
    int i, j;
    const double INVALID_RELEVANCE = -1.0;
    const int INVALID_RATING = -1;

    if (metadata == NULL)
    {
        return false;
    }

    metadata->relevance = INVALID_RELEVANCE;
    metadata->creationTimeSec = 0;
    metadata->numUsage = 0;

    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        metadata->ratings[i] = INVALID_RATING;
    }


    for (i = 0; i < (int)(extraDataList.size()); i++)
    {
        extraData = extraDataList[i];

        if ((extraData.key) ==
            CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_CREATION_TIME])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                metadata->creationTimeSec = atoi(extraData.dataEntries[0].c_str());
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_NUM_USAGE])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                metadata->numUsage = atoi(extraData.dataEntries[0].c_str());
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_COMPUTED_RELEVANCE])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                metadata->relevance = atof(extraData.dataEntries[0].c_str());
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_STEALTH_RATINGS])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                totalRating = 0;

                for (j = 0; j < (int)(extraData.dataEntries.size()); j++)
                {
                    totalRating += atoi(extraData.dataEntries[j].c_str());
                }

                metadata->ratings[MEXP_METADATA_RATING_STEALTH] = 
                    ((double)totalRating)/((double)(extraData.dataEntries.size()));
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_EFFICIENCY_RATINGS])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                totalRating = 0;

                for (j = 0; j < (int)(extraData.dataEntries.size()); j++)
                {
                    totalRating += atoi(extraData.dataEntries[j].c_str());
                }

                metadata->ratings[MEXP_METADATA_RATING_EFFICIENCY] = 
                    ((double)totalRating)/((double)(extraData.dataEntries.size()));
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_PRESERVATION_RATINGS])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                totalRating = 0;

                for (j = 0; j < (int)(extraData.dataEntries.size()); j++)
                {
                    totalRating += atoi(extraData.dataEntries[j].c_str());
                }

                metadata->ratings[MEXP_METADATA_RATING_PRESERVATION] = 
                    ((double)totalRating)/((double)(extraData.dataEntries.size()));
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_DURATION_RATINGS])
        {
            if ((extraData.dataEntries.size()) > 0)
            {
                totalRating = 0;

                for (j = 0; j < (int)(extraData.dataEntries.size()); j++)
                {
                    totalRating += atoi(extraData.dataEntries[j].c_str());
                }

                metadata->ratings[MEXP_METADATA_RATING_DURATION] = 
                    ((double)totalRating)/((double)(extraData.dataEntries.size()));
            }
        }
        else if ((extraData.key) ==
                 CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_USER_COMMENTS])
        {
            for (j = 0; j < (int)(extraData.dataEntries.size()); j++)
            {
                comment = extraData.dataEntries[j].c_str();
                metadata->userComments.push_back(comment);
            }
        }
    }

    return true;
}

//-----------------------------------------------------------------------
// This function converts CBRPlanner_MissionPlan to MExpRetrievedMission_t.
//-----------------------------------------------------------------------
bool CfgEditCBRClient::cbrplannerMissionPlan2RetrievedMission_(
    CBRPlanner_MissionPlan_t missionPlan,
    MExpRetrievedMission_t *retrievedMission)
{
    const MExpRetrievedMission_t EMPTY_RETRIEVED_MISSION = {0, 0, 0, vector<int>(), string(), {0, 0, 0, {}, vector<string>()}, 0, 0, 0};
    const bool SUPPORT_LEGACY_RATING = true;

    if (retrievedMission == NULL)
    {
        return false;
    }

    // Reset the data first.
    *retrievedMission = EMPTY_RETRIEVED_MISSION;

    if (SUPPORT_LEGACY_RATING)
    {
        retrievedMission->rating = missionPlan.rating;
    }

    retrievedMission->numRobots = missionPlan.numRobots;
    retrievedMission->dataIndexList = missionPlan.dataIndexList;
    retrievedMission->acdlSolution = missionPlan.data;

    cbrplannerExtraDataList2Metadata_(
        missionPlan.extraDataList,
        &(retrievedMission->metadata));

    retrievedMission->feedbackSent = false;
    retrievedMission->usageCounterUpdated = false;
    retrievedMission->newUnsavedMission = false;

    return true;
}

//-----------------------------------------------------------------------
// This function retrieve a mission planfrom the CBRServer based on the
// features defined in the specified feature file.
//-----------------------------------------------------------------------
vector<MExpRetrievedMission_t> CfgEditCBRClient::retrieveMissionPlan(
    string featureFileName,
    vector<MExpMissionTask_t> defaultTaskList,
    vector<MExpFeature_t> defaultGlobalFeatures,
    vector<MExpFeature_t> defaultLocalFeatures,
    double taskWeight,
    int maxRating,
    bool addStartSubMission,
    string *overlayFileName,
    int *cnpMode,
    vector<MExpRobotIDs_t> *robotIDList,
    MExpCNPStatus_t *cnpStatus,
    bool *reload)
{
    typedef enum ConstraintType_t {
        CONSTRAINT_UNKNOWN,
        CONSTRAINT_TASK,
        CONSTRAINT_ROBOT
    };

    MExpRetrievedMission_t retrievedMission;
    CBRPlanner_Feature_t feature, globalFeature;
    CBRPlanner_Constraint_t constraint;
    CBRPlanner_Constraints_t genericConstraints;
    CBRPlanner_CNPStatus_t retrievedCNPStatus;
    vector<CBRPlanner_MissionPlan_t> missionplans;
    vector<CBRPlanner_Feature_t> features;
    vector<MExpRetrievedMission_t> retrievedMissions;
    vector<CBRPlanner_TaskConstraints_t> taskConstraintsList;
    vector<CBRPlanner_RobotConstraints_t> robotConstraintsList;
    vector<CBRPlanner_RobotIDs_t> retrievedRobotIDList;
    string bufString1, bufString2, valueString;
    FILE *featureFile;
    const MExpRetrievedMission_t EMPTY_RETRIEVED_MISSION = {0, 0, 0, vector<int>(), string(), {0, 0, 0, {}, vector<string>()}, 0, 0, 0};
    const vector<MExpRetrievedMission_t> EMPTY_RETRIEVED_MISSIONS;
    float selectedOption;
    char buf[1024], buf1[1024], buf2[1024], buf3[1024];
    int i, j;
    int taskId;
    int numOptions, optionType, optionNum;
    int numConstraints, constraintType = CONSTRAINT_UNKNOWN;
    int listSize, numIDs;
    int localFeatureId, globalFeatureId;
    int cnpModeInFile;
    bool dataExists = false, hadError = false;
    
    featureFile = fopen(featureFileName.c_str(), "r");
    if (featureFile == NULL)
    {
        return EMPTY_RETRIEVED_MISSIONS;
    }

    while(!feof(featureFile))
    {
        if (fscanf(featureFile, "%s", buf1) == EOF)
        {
            break;
        }

        bufString1 = buf1;

        if (bufString1 ==  MEXP_STRING_OVERLAY)
        {
            // Save the overlay name specified.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            *overlayFileName = buf2;
        }
        else if (bufString1 ==  MEXP_STRING_CNP)
        {
            // Check the mode of CNP
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            bufString2 = buf2;

            if (isdigit(bufString2[0]))
            {
                cnpModeInFile = atoi(buf2);
            }
            else
            {
                for (i = 0; i < NUM_CNP_MODES; i++)
                {
                    if (bufString2 == CNP_MODE_STRING[i])
                    {
                        cnpModeInFile = i;
                        break;
                    }
                }
            }

            if (cnpModeInFile != *cnpMode)
            {
                *cnpMode = cnpModeInFile;
                *reload = true;
                fclose(featureFile);
                return EMPTY_RETRIEVED_MISSIONS;
            }

            *reload = false;

        }
        else if (bufString1 == MEXP_STRING_GLOBALFEATURE)
        {
            // Build a list of the global preferences.  
            // The task entries will append them to each feature list.

            // Get the preference ID (e.g., NumberOfRobots, Time, etc.)
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            globalFeatureId = atoi(buf2);

            globalFeature.name = defaultGlobalFeatures[globalFeatureId].name;
            globalFeature.nonIndex = defaultGlobalFeatures[globalFeatureId].nonIndex;
            globalFeature.weight = defaultGlobalFeatures[globalFeatureId].weight;

            // Get the selected option.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            selectedOption = atof(buf2);

            // Get the number of options for the global preference.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            numOptions = atoi(buf2);

            // Get the option type for the global preference.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            optionType = atoi(buf2);

            switch (optionType) {

            case MEXP_FEATURE_OPTION_POLYGONS:
            case MEXP_FEATURE_OPTION_STRINGS:

                valueString = EMPTY_STRING_;

                // Look for the polygon points (or string) store in the option value.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                // (Skip the featureName.)
                
                // Parse though the options
                for (i = 0; i < numOptions; i++)
                {
                    if (i != 0)
                    {
                        valueString += CHAR_AMPERSAND_;
                    }

                    // Get the option string
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        break;
                    }
                    bufString2 = buf2;

                    if (bufString2 != MEXP_STRING_OPTION)
                    {
                        hadError = true;
                        break;
                    }

                    // Get the option number
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        break;
                    }
                    // (Skip the option number.)

                    // Get the option value
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        break;
                    }
                    bufString2 = buf2;

                    // Remove '"'
                    valueString += removeCharInString(
                        bufString2,
                        CHAR_DOUBLE_QUOTE_);
                }
                break;
            }

            if (selectedOption >= 0)
            {
                switch (optionType) {

                case MEXP_FEATURE_OPTION_TOGGLE:
                    globalFeature.value = defaultGlobalFeatures[globalFeatureId].options[(int)selectedOption].value;
                    globalFeature.formatType = CBRPLANNER_FORMAT_STRING;
                    break;
			
                case MEXP_FEATURE_OPTION_SLIDER1:
                    sprintf(buf, "%.2f", selectedOption);
                    globalFeature.value = buf;
                    globalFeature.formatType = CBRPLANNER_FORMAT_DOUBLE;
                    break;

                case MEXP_FEATURE_OPTION_SLIDER100:
                case MEXP_FEATURE_OPTION_SLIDER10:
                    sprintf(buf, "%d", (int)selectedOption);
                    globalFeature.value = buf;
                    globalFeature.formatType = CBRPLANNER_FORMAT_INT;
                    break;

                case MEXP_FEATURE_OPTION_POLYGONS:
                    globalFeature.value = valueString;
                    globalFeature.formatType = CBRPLANNER_FORMAT_STRING;
                    break;

                case MEXP_FEATURE_OPTION_STRINGS:
                    globalFeature.value = valueString;
                    globalFeature.formatType = CBRPLANNER_FORMAT_STRING;
                    break;
                }

                features.push_back(globalFeature);
            }
        }
        else if (bufString1 == MEXP_STRING_TASK)
        {
            feature.name = STRING_TASK_;
            feature.nonIndex = false;
            feature.weight = taskWeight;
            feature.formatType = CBRPLANNER_FORMAT_STRING;

            // Get the task ID (e.g., Biohazrd, EOD, etc.)
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            taskId = atoi(buf2);

            for (i = 0; i < ((int)(defaultTaskList.size())); i++)
            {
                if (defaultTaskList[i].id == taskId)
                {
                    feature.value = defaultTaskList[i].name;
                    break;
                }
            }
	    
            features.push_back(feature);

            dataExists = true;
        }
        else if (bufString1 == MEXP_STRING_COORDINATE)
        {
            feature.nonIndex = true;
            feature.weight = 0.0;
            feature.formatType = CBRPLANNER_FORMAT_DOUBLE;

            // Get the coordinate.
            if (fscanf(featureFile, "%s %s", buf2, buf3) == EOF)
            {
                hadError = true;
                break;
            }

            feature.name = STRING_START_X_;
            feature.value = buf2;
            features.push_back(feature);

            feature.name = STRING_START_Y_;
            feature.value = buf3;
            features.push_back(feature);

            dataExists = true;
        }
        else if (bufString1 == MEXP_STRING_GEO_COORDINATE)
        {
            feature.nonIndex = true;
            feature.weight = 0.0;
            feature.formatType = CBRPLANNER_FORMAT_DOUBLE;

            // Get the coordinate.
            if (fscanf(featureFile, "%s %s", buf2, buf3) == EOF)
            {
                hadError = true;
                break;
            }

            feature.name = STRING_START_LATITUDE_;
            feature.value = buf2;
            features.push_back(feature);

            feature.name = STRING_START_LONGITUDE_;
            feature.value = buf3;
            features.push_back(feature);

            dataExists = true;
        }
        else if (bufString1 == MEXP_STRING_FEATURE)
        {
            // Get the feature ID.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            localFeatureId = atoi(buf2);

            // Get the feature option being selected.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            selectedOption = atof(buf2);

            for (i = 0; i < ((int)(defaultLocalFeatures.size())); i++)
            {
                if (defaultLocalFeatures[i].id == localFeatureId)
                {
                    feature.name = defaultLocalFeatures[i].name;
                    feature.nonIndex = defaultLocalFeatures[i].nonIndex;
                    feature.weight = defaultLocalFeatures[i].weight;

                    switch (defaultLocalFeatures[i].optionType) {
			
                    case MEXP_FEATURE_OPTION_TOGGLE:
                        feature.value = defaultLocalFeatures[i].options[(int)selectedOption].value;
                        feature.formatType = CBRPLANNER_FORMAT_STRING;
                        break;

                    case MEXP_FEATURE_OPTION_SLIDER1:
                        sprintf(buf, "%.2f", selectedOption);
                        feature.value = buf;
                        feature.formatType = CBRPLANNER_FORMAT_DOUBLE;
                        break;

                    case MEXP_FEATURE_OPTION_SLIDER100:
                    case MEXP_FEATURE_OPTION_SLIDER10:
                        sprintf(buf, "%d", (int)selectedOption);
                        feature.value = buf;
                        feature.formatType = CBRPLANNER_FORMAT_INT;
                        break;
                    }

                    features.push_back(feature);
                    break;
                }
            }

            dataExists = true;
        }
        else if ((bufString1 == MEXP_STRING_TASK_CONSTRAINTS) ||
                 (bufString1 == MEXP_STRING_ROBOT_CONSTRAINTS))
        {
            if (bufString1 == MEXP_STRING_TASK_CONSTRAINTS)
            {
                constraintType = CONSTRAINT_TASK;
            }
            else if (bufString1 == MEXP_STRING_ROBOT_CONSTRAINTS)
            {
                constraintType = CONSTRAINT_ROBOT;
            }

            genericConstraints.constraints.clear();

            // Get the task/robot ID
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            genericConstraints.id = atoi(buf2);

            // Get the number of constraints
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            numConstraints = atoi(buf2);

            // Get the name of the task/robot
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            bufString2 = buf2;
            // Remove '"'
            bufString2 = removeCharInString(bufString2, CHAR_DOUBLE_QUOTE_);
            genericConstraints.name = bufString2;

            for (i = 0; i < numConstraints; i++)
            {
                // Make sure it is the constraint first.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                bufString2 = buf2;

                if ((bufString2 != MEXP_STRING_TASK_CONSTRAINT) &&
                    (bufString2 != MEXP_STRING_ROBOT_CONSTRAINT))
                {
                    hadError = true;
                    break;
                }

                // Get the constraint ID.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                constraint.id = atoi(buf2);

                // Get the option being selected.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                selectedOption = atof(buf2);

                // Get the number of options.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                numOptions = atoi(buf2);

                // Get the option types.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                optionType = atoi(buf2);

                // Get the constraint name.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    break;
                }
                bufString2 = buf2;
                // Remove '"'
                bufString2 = removeCharInString(bufString2, CHAR_DOUBLE_QUOTE_);
                constraint.name = bufString2;

                switch (optionType) {

                case MEXP_FEATURE_OPTION_SLIDER1:
                    sprintf(buf, "%.2f", selectedOption);
                    constraint.value = buf;
                    constraint.formatType = CBRPLANNER_FORMAT_DOUBLE;
                    break;

                case MEXP_FEATURE_OPTION_SLIDER100:
                case MEXP_FEATURE_OPTION_SLIDER10:
                    sprintf(buf, "%d", (int)selectedOption);
                    constraint.value = buf;
                    constraint.formatType = CBRPLANNER_FORMAT_INT;
                    break;

                case MEXP_FEATURE_OPTION_TOGGLE:
                    // Parse though the options until it finds the selected option.
                    for (j = 0; j < numOptions; j++)
                    {
                        // Get the option string
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            break;
                        }
                        bufString2 = buf2;

                        if (bufString2 != MEXP_STRING_OPTION)
                        {
                            hadError = true;
                            break;
                        }

                        // Get the option number
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            break;
                        }
                        optionNum = atoi(buf2);

                        // Get the option value
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            break;
                        }

                        if (optionNum == ((int)selectedOption))
                        {
                            bufString2 = buf2;
                            bufString2 = removeCharInString(bufString2, CHAR_DOUBLE_QUOTE_);
                            constraint.value = bufString2;
                        }
                    }

                    constraint.formatType = CBRPLANNER_FORMAT_STRING;
                    break;

                default:
                    // Unknown option type.
                    hadError = true;
                    break;
                }

                if (hadError)
                {
                    break;
                }

                genericConstraints.constraints.push_back(constraint);
            }

            switch (constraintType) {

            case CONSTRAINT_TASK:
                taskConstraintsList.push_back(
                    (CBRPlanner_TaskConstraints_t)genericConstraints);
                break;

            case CONSTRAINT_ROBOT:
                robotConstraintsList.push_back(
                    (CBRPlanner_RobotConstraints_t)genericConstraints);
                break;
            }
        }
    }

    fclose(featureFile);

    if ((!dataExists)||(hadError))
    {
        return EMPTY_RETRIEVED_MISSIONS;
    }

    // Get the mission plan data from the server.
    pthread_mutex_lock(&socketMutex_);
    gEventLogging->start("CfgEditCBRClient Retrieving mission plan from CBRServer");

    switch (*cnpMode) {

    case CNP_MODE_DISABLED:
        missionplans = retrieveMissionPlansFromServer_(
            features,
            maxRating,
            addStartSubMission);
        break;

    case CNP_MODE_PREMISSION:
        missionplans = retrieveMissionPlansFromServerPremissionCNP_(
            features,
            maxRating,
            addStartSubMission,
            taskConstraintsList,
            robotConstraintsList);
        retrievedRobotIDList = retrieveCurrentRobotIDListFromServer_();
        retrievedCNPStatus = retrieveCurrentCNPStatusFromServer_();
        listSize = retrievedRobotIDList.size();
        robotIDList->resize(listSize);

        for (i = 0; i < listSize; i++)
        {
            numIDs = retrievedRobotIDList[i].robotIDs.size();
            (*robotIDList)[i].robotIDs.resize(numIDs);

            for (j = 0; j < numIDs; j++)
            {
                (*robotIDList)[i].robotIDs[j] = retrievedRobotIDList[i].robotIDs[j];
            }
        }

        cnpStatus->status = retrievedCNPStatus.status;
        cnpStatus->errorMsg = retrievedCNPStatus.errorMsg;
        break;

    case CNP_MODE_RUNTIME:
        missionplans = retrieveMissionPlansFromServerRuntimeCNP_(
            features,
            maxRating,
            addStartSubMission,
            robotConstraintsList);
        retrievedRobotIDList = retrieveCurrentRobotIDListFromServer_();
        listSize = retrievedRobotIDList.size();
        robotIDList->resize(listSize);

        for (i = 0; i < listSize; i++)
        {
            numIDs = retrievedRobotIDList[i].robotIDs.size();
            (*robotIDList)[i].robotIDs.resize(numIDs);

            for (j = 0; j < numIDs; j++)
            {
                (*robotIDList)[i].robotIDs[j] = retrievedRobotIDList[i].robotIDs[j];
            }
        }
        break;

    case CNP_MODE_PREMISSION_AND_RUNTIME:
    default:
        fprintf
            (stderr,
             "Warning: CfgEditCBRClient::retrieveMissionPlan(). Unknown CNP mode.\a\n");
        missionplans = retrieveMissionPlansFromServer_(
            features,
            maxRating,
            addStartSubMission);
        break;
    }

    gEventLogging->end("CfgEditCBRClient Retrieving mission plan from CBRServer");
    pthread_mutex_unlock(&socketMutex_);

    retrievedMissions = cbrplannerMissionsPlan2RetrievedMissions_(missionplans);

    return retrievedMissions;
}

//-----------------------------------------------------------------------
// This function asks the CBRServer to fix the mission plan.
//-----------------------------------------------------------------------
vector<MExpRetrievedMission_t> CfgEditCBRClient::fixMissionPlan(string missionPlanFileName)
{
    CBRPlanner_MissionPlan_t missionplan;
    CBRPlanner_Feature_t feature;
    MExpRetrievedMission_t retrievedMission;
    vector<MExpRetrievedMission_t> retrievedMissions;
    bool fixed = false;

    // Describe the mission plan type.
    missionplan.type = CBRPLANNER_MISSIONPLAN_NEW;

    // Describe the mission plan rating.
    missionplan.rating = 0;

    // Open the solution file to read.
    ifstream istr(missionPlanFileName.c_str());

    if (istr.bad()) 
    {
        fprintf
            (stderr,
             "Error: CfgEditCBRClient::saveMissionPlan(). file %s not readable",
             missionPlanFileName.c_str());
        return retrievedMissions;
    }

    istr.seekg (0, ios::end);
    missionplan.dataSize = istr.tellg();
    istr.seekg (0, ios::beg);

    // To append NULL at the end.
    missionplan.dataSize++; 

    // Read in the solution.
    missionplan.data = new char[missionplan.dataSize];
    istr.read (missionplan.data, missionplan.dataSize-1);

    // Append a null, to make it a char string.
    missionplan.data[missionplan.dataSize-1] = CHAR_NULL_;

    // Close the solution file.
    istr.close();

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that the mission plan needs to be fixed.
    sendCommandToServer_(CBRPCOM_CMD_MISSIONPLAN_FIX);

    // Next, send the mission plan. The mission plan data is composed of the type flag,
    // rating, data size, and the data itselt.
    writeInteger_(shSocketFd_, missionplan.type);
    writeInteger_(shSocketFd_, missionplan.rating);
    writeInteger_(shSocketFd_, missionplan.dataSize);
    write(shSocketFd_, missionplan.data, missionplan.dataSize);

    // Clear the data.
    delete [] missionplan.data;
    missionplan.data = NULL;

    // Now retrieve the mission plan from the server. Make repairment was successful.
    fixed = readBoolean_(shSocketFd_);

    if (fixed)
    {
        missionplan = retrieveMissionPlanFromServer_();
    }

    pthread_mutex_unlock(&socketMutex_);

    if (!fixed)
    {
        // It's an empty mission.
        return retrievedMissions;
    }

    cbrplannerMissionPlan2RetrievedMission_(missionplan, &retrievedMission);
    retrievedMissions.push_back(retrievedMission);

    return retrievedMissions;
}

//-----------------------------------------------------------------------
// This function asks the CBRServer to reverse the order of the robots in
// the ACDL mission.
//-----------------------------------------------------------------------
string CfgEditCBRClient::reverseRobotOrder(string acdlMission)
{
    string newACDLMission;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that the robot order
    // should be reversed.
    sendCommandToServer_(CBRPCOM_CMD_REVERSE_ROBOT_ORDER);

    // Send the ACDL data.
    writeString_(shSocketFd_, acdlMission);

    // Read the new ACDL data.
    newACDLMission = readString_(shSocketFd_);

    pthread_mutex_unlock(&socketMutex_);

    return newACDLMission;
}

//-----------------------------------------------------------------------
// This function merges multiple ACDL missions into one.
//-----------------------------------------------------------------------
string CfgEditCBRClient::getMergedAbstractedMissionPlan(
    vector<string> acdlMissions,
    int *numRobots)
{
    string acdlMission;
    int i, numMissions;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that the robot order
    // should be reversed.
    sendCommandToServer_(CBRPCOM_CMD_MERGE_MISSIONS);

    // Send the number of the missions.
    numMissions = acdlMissions.size();
    writeInteger_(shSocketFd_, numMissions);

    for (i = 0; i < numMissions; i++)
    {
        // Send the ACDL data.
        writeString_(shSocketFd_, acdlMissions[i]);
    }

    // Read the number of the robots.
    *numRobots = readInteger_(shSocketFd_);

    // Read the new ACDL data.
    acdlMission = readString_(shSocketFd_);

    pthread_mutex_unlock(&socketMutex_);

    return acdlMission;
}

//-----------------------------------------------------------------------
// This function sends a mission plan to the server.
//-----------------------------------------------------------------------
vector<int> CfgEditCBRClient::sendMissionPlanToServer_(
    CBRPlanner_MissionPlan_t missionplan,
    vector<CBRPlanner_Feature_t> features)
{
    int numFeatures, nameSize, valueSize, dataIndexListSize;
    int i, dataIndex;
    int formatType;
    vector<int> dataIndexList;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that the mission plan will be sent.
    sendCommandToServer_(CBRPCOM_CMD_MISSIONPLAN_SAVE);

    // Next, send the mission plan. The mission plan data is composed of the type flag,
    // rating, data size, and the data itselt.
    writeInteger_(shSocketFd_, missionplan.type);
    writeInteger_(shSocketFd_, missionplan.rating);
    writeInteger_(shSocketFd_, missionplan.dataSize);
    write(shSocketFd_, missionplan.data, missionplan.dataSize);

    // Send the number of features
    numFeatures = features.size();
    writeInteger_(shSocketFd_, numFeatures);

    for (i = 0; i < numFeatures; i++)
    {
        // Send the length of the name
        nameSize = features[i].name.size();
        writeInteger_(shSocketFd_, nameSize);

        // Send the name
        write(shSocketFd_, features[i].name.c_str(), nameSize);
	
        // Send the length of the value
        valueSize = features[i].value.size();
        writeInteger_(shSocketFd_, valueSize);

        // Send the value
        write(shSocketFd_, features[i].value.c_str(), valueSize);

        // Send the nonIndex flag.
        writeBoolean_(shSocketFd_, features[i].nonIndex);

        // Send the weight.
        writeDouble_(shSocketFd_, features[i].weight);

        // Send the format type
        formatType = features[i].formatType;
        writeInteger_(shSocketFd_, formatType);
    }

    // Send zero as the number of robots
    writeInteger_(shSocketFd_, 0);

    // Get the new list of data indexes.
    dataIndexListSize = readInteger_(shSocketFd_);

    for (i = 0; i < dataIndexListSize; i++)
    {
        dataIndex = readInteger_(shSocketFd_);
        dataIndexList.push_back(dataIndex);
    }

    pthread_mutex_unlock(&socketMutex_);

    return dataIndexList;
}

//-----------------------------------------------------------------------
// This function retrieves mission plans afer sending features to the server.
//-----------------------------------------------------------------------
vector<CBRPlanner_MissionPlan_t> CfgEditCBRClient::retrieveMissionPlansFromServer_(
    vector<CBRPlanner_Feature_t> features,
    int maxRating,
    bool addStartSubMission)
{
    vector<CBRPlanner_MissionPlan_t> missionplans;

    // First, send a flag to notify the server that the mission plan is going
    // to be retrieved.
    sendCommandToServer_(CBRPCOM_CMD_MISSIONPLAN_RETRIEVE);

    // Send the features
    sendFeaturesToServer_(features);

    // Send the maximum suitability rating
    write(shSocketFd_, &maxRating, sizeof(int));

    // Send the addStartSubMission flag.
    write(shSocketFd_, &addStartSubMission, sizeof(bool));

    // Now retrieve the mission plan from the server.
    missionplans = receiveMissionPlansFromServer_();

    return missionplans;
}

//-----------------------------------------------------------------------
// This function retrieves mission plans afer sending features to the server,
// with premission-CNP enabled.
//-----------------------------------------------------------------------
vector<CBRPlanner_MissionPlan_t> CfgEditCBRClient::retrieveMissionPlansFromServerPremissionCNP_(
    vector<CBRPlanner_Feature_t> features,
    int maxRating,
    bool addStartSubMission,
    vector<CBRPlanner_TaskConstraints_t> taskConstraintsList,
    vector<CBRPlanner_RobotConstraints_t> robotConstraintsList)
{
    vector<CBRPlanner_MissionPlan_t> missionplans;
    vector<CBRPlanner_Constraints_t> genericConstraintsList;

    // First, send a flag to notify the server that the mission plan is going
    // to be retrieved.
    sendCommandToServer_(CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_PREMISSION);

    // Send the features
    sendFeaturesToServer_(features);

    // Send the maximum suitability rating
    write(shSocketFd_, &maxRating, sizeof(int));

    // Send the addStartSubMission flag.
    write(shSocketFd_, &addStartSubMission, sizeof(bool));

    // Send the task constraints.
    genericConstraintsList = (vector<CBRPlanner_Constraints_t>)taskConstraintsList;
    sendConstraintsToServer_(genericConstraintsList);

    // Send the robot constraints.
    genericConstraintsList = (vector<CBRPlanner_Constraints_t>)robotConstraintsList;
    sendConstraintsToServer_(genericConstraintsList);

    // Now retrieve the mission plan from the server.
    missionplans = receiveMissionPlansFromServer_();

    return missionplans;
}

//-----------------------------------------------------------------------
// This function retrieves mission plans afer sending features to the server.
//-----------------------------------------------------------------------
vector<CBRPlanner_MissionPlan_t> CfgEditCBRClient::retrieveMissionPlansFromServerRuntimeCNP_(
    vector<CBRPlanner_Feature_t> features,
    int maxRating,
    bool addStartSubMission,
    vector<CBRPlanner_RobotConstraints_t> robotConstraintsList)
{
    vector<CBRPlanner_MissionPlan_t> missionplans;
    vector<CBRPlanner_Constraints_t> genericConstraintsList;

    // First, send a flag to notify the server that the mission plan is going
    // to be retrieved.
    sendCommandToServer_(CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_RUNTIME);

    // Send the features
    sendFeaturesToServer_(features);

    // Send the maximum suitability rating
    write(shSocketFd_, &maxRating, sizeof(int));

    // Send the addStartSubMission flag.
    write(shSocketFd_, &addStartSubMission, sizeof(bool));

    // Send the robot constraints.
    genericConstraintsList = (vector<CBRPlanner_Constraints_t>)robotConstraintsList;
    sendConstraintsToServer_(genericConstraintsList);

    // Now retrieve the mission plan from the server.
    missionplans = receiveMissionPlansFromServer_();

    return missionplans;
}

//-----------------------------------------------------------------------
// This function retrieves mission plans
//-----------------------------------------------------------------------
vector<CBRPlanner_MissionPlan_t> CfgEditCBRClient::receiveMissionPlansFromServer_(void)
{
    CBRPlanner_MissionPlan_t missionplan;
    vector<CBRPlanner_MissionPlan_t> missionplans;
    int i, numPlans;
    const bool DEBUG_RECEIVE_MISSION_PLANS_FROM_SERVER = false;

    // Now retrieve the mission plan from the server. Get the number of the plans.
    numPlans = readInteger_(shSocketFd_);

    for (i = 0; i < numPlans; i++)
    {
        missionplan = retrieveMissionPlanFromServer_();

        if (DEBUG_RECEIVE_MISSION_PLANS_FROM_SERVER)
        {
            fprintf(stderr, "Received Plan (%d)\n", i);
            fprintf(stderr, "%s\n\n", missionplan.data);
        }

        missionplans.push_back(missionplan);
    }

    return missionplans;
}

//-----------------------------------------------------------------------
// This function retrieves Robot ID lists.
//-----------------------------------------------------------------------
vector<CBRPlanner_RobotIDs_t> CfgEditCBRClient::retrieveCurrentRobotIDListFromServer_(void)
{
    vector<CBRPlanner_RobotIDs_t> robotIDList;
    int i, j, listSize, numIDs, id;
    const bool DEBUG = false;

    // First, send the flag
    sendCommandToServer_(CBRPCOM_CMD_ROBOT_ID_LIST_RETRIEVE);

    listSize = readInteger_(shSocketFd_);
    robotIDList.resize(listSize);

    for (i = 0; i < listSize; i++)
    {
        numIDs = readInteger_(shSocketFd_);
        robotIDList[i].robotIDs.resize(numIDs);

        for (j = 0; j < numIDs; j++)
        {
            id = readInteger_(shSocketFd_);
            robotIDList[i].robotIDs[j] = id;
        }
    }

    if (DEBUG)
    {
        fprintf(stderr, "Received Robot IDs\n");

        for (i = 0; i < (int)(robotIDList.size()); i++)
        {
            fprintf(stderr, "LIST(%d)\n", i);

            for (j = 0; j < (int)(robotIDList[i].robotIDs.size()); j++)
            {
                fprintf(stderr, "%d. Robot ID: %d\n", j, robotIDList[i].robotIDs[j]);
            }
        }
    }

    return robotIDList;
}

//-----------------------------------------------------------------------
// This function retrieves CNP status and error messages.
//-----------------------------------------------------------------------
CBRPlanner_CNPStatus_t CfgEditCBRClient::retrieveCurrentCNPStatusFromServer_(void)
{
    CBRPlanner_CNPStatus_t cnpStatus;
    string errorMsg = EMPTY_STRING_, debugMsg;
    int status, dataSize;
    char *data = NULL;
    const bool DEBUG = false;

    // First, send the flag
    sendCommandToServer_(CBRPCOM_CMD_CNP_STATUS_RETRIEVE);

    // Get the status
    status = readInteger_(shSocketFd_);

    // Get the message size.
    dataSize = readInteger_(shSocketFd_);

    // Get the message (if it has a content).
    if (dataSize > 0)
    {
        data = new char[dataSize+1];
        readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);

        errorMsg = data;
        errorMsg[dataSize] = CHAR_NULL_;
        delete [] data;
        data = NULL;
    }

    cnpStatus.status = status;
    cnpStatus.errorMsg = errorMsg;

    if (DEBUG)
    {
        fprintf(stderr, "Retreived CNP Status: %d\n", cnpStatus.status);

        if (cnpStatus.errorMsg == EMPTY_STRING_)
        {
            debugMsg = "(empty)";
        }
        else
        {
            debugMsg = cnpStatus.errorMsg;
        }

        fprintf(stderr, "Error Message: %s\n", debugMsg.c_str());
    }

    return cnpStatus;
}

//-----------------------------------------------------------------------
// This function sends features to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendFeaturesToServer_(vector<CBRPlanner_Feature_t> features)
{
    int numFeatures, nameSize, valueSize;
    int i;
    int formatType;
    double weight;
    bool nonIndex = false;

    // Send the number of features
    numFeatures = features.size();
    writeInteger_(shSocketFd_, numFeatures);

    for (i = 0; i < numFeatures; i++)
    {
        // Send the length of the name
        nameSize = features[i].name.size();
        writeInteger_(shSocketFd_, nameSize);

        // Send the name
        write(shSocketFd_, features[i].name.c_str(), nameSize);
	
        // Send the length of the value
        valueSize = features[i].value.size();
        writeInteger_(shSocketFd_, valueSize);

        // Send the value
        write(shSocketFd_, features[i].value.c_str(), valueSize);

        // Send the nonIndex flag.
        nonIndex = features[i].nonIndex;
        writeBoolean_(shSocketFd_, nonIndex);

        // Send the weight.
        weight = features[i].weight;
        write(shSocketFd_, &weight, sizeof(double));

        // Send the format type
        formatType = features[i].formatType;
        writeInteger_(shSocketFd_, formatType);
    }
}

//-----------------------------------------------------------------------
// This function sends constraints to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendConstraintsToServer_(vector<CBRPlanner_Constraints_t> constraintsList)
{
    CBRPlanner_Constraint_t constraint;
    int listSize, numConstraints, nameSize, valueSize;
    int i, j;
    int id;
    int formatType;

    // Send the size of the list
    listSize = constraintsList.size();
    writeInteger_(shSocketFd_, listSize);

    for (i = 0; i < listSize; i++)
    {
        // Send the ID of the task/robot
        id = constraintsList[i].id;
        writeInteger_(shSocketFd_, id);

        // Send the length of the name
        nameSize = constraintsList[i].name.size();
        writeInteger_(shSocketFd_, nameSize);

        // Send the name
        write(shSocketFd_, constraintsList[i].name.c_str(), nameSize);
	
        // Send the number of constraints
        numConstraints = constraintsList[i].constraints.size();
        writeInteger_(shSocketFd_, numConstraints);

        for (j = 0; j < numConstraints; j++)
        {
            constraint = constraintsList[i].constraints[j];

            // Send the length of the name
            nameSize = constraint.name.size();
            writeInteger_(shSocketFd_, nameSize);

            // Send the name
            write(shSocketFd_, constraint.name.c_str(), nameSize);
	
            // Send the length of the value
            valueSize = constraint.value.size();
            writeInteger_(shSocketFd_, valueSize);

            // Send the value
            write(shSocketFd_, constraint.value.c_str(), valueSize);

            // Send the ID
            id = constraint.id;
            writeInteger_(shSocketFd_, id);

            // Send the format type
            formatType = constraint.formatType;
            writeInteger_(shSocketFd_, formatType);
        }
    }
}

//-----------------------------------------------------------------------
// This function checks to see if the CBR library file needs to be saved
// and retrieves the name of the file.
//-----------------------------------------------------------------------
bool CfgEditCBRClient::checkCBRLibrarySave(string *cblFileName)
{
    char *buf = NULL;
    int filenameSize;
    bool needSave = false;

    pthread_mutex_lock(&socketMutex_);

    // First, send the flag
    sendCommandToServer_(CBRPCOM_CMD_CHECK_CBR_LIBRARY_FILE_SAVE);

    // Get the boolean value.
    needSave = readBoolean_(shSocketFd_);

    // Get the filename. The size of the name first.
    filenameSize = readInteger_(shSocketFd_);
    buf = new char[filenameSize+1];

    // Read the value.
    readNBytes_(shSocketFd_, (unsigned char *)buf, filenameSize);
    buf[filenameSize] = CHAR_NULL_;
    *cblFileName = buf;
    delete[] buf;
    buf = NULL;

    pthread_mutex_unlock(&socketMutex_);

    return needSave;
}

//-----------------------------------------------------------------------
// This function commands the server to save the cbr library in a file.
//-----------------------------------------------------------------------
void CfgEditCBRClient::saveCBRLibraryFile(string filename)
{
    int filenameSize;

    pthread_mutex_lock(&socketMutex_);

    // First, send the flag
    sendCommandToServer_(CBRPCOM_CMD_SAVE_CBR_LIBRARY_FILE);

    // Send the size of the name first.
    filenameSize = filename.size();
    writeInteger_(shSocketFd_, filenameSize);

    // Send the value.
    write(shSocketFd_, filename.c_str(), filenameSize);

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function updates the usage counter in the CBR library.
//-----------------------------------------------------------------------
void CfgEditCBRClient::updateMissionUsageCounter(vector<int> dataIndexList)
{
    int i, indexSize;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that the usage counter
    // should be updated.
    sendCommandToServer_(CBRPCOM_CMD_USAGE_COUNTER_UPDATE);

    // Send the case index, but first the number of the indexes.
    indexSize = dataIndexList.size();
    writeInteger_(shSocketFd_, indexSize);

    for (i = 0; i < indexSize; i++)
    {
        // Send the case index.
        writeInteger_(shSocketFd_, dataIndexList[i]);
    }

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends metadata to the server and attach it as the case's
// metadata.
//-----------------------------------------------------------------------
void CfgEditCBRClient::saveMetadata(vector<int> dataIndexList, MExpMetadata_t metadata)
{
    CBRPlanner_ExtraData_t extraData;
    vector<CBRPlanner_ExtraData_t> extraDataList;
    string value;
    char buf[1024];
    int i;
    bool invalidCase = false;

    extraData.numDataEntries = 1;

    // Check creation time.
    if (metadata.creationTimeSec > 0)
    {
        extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_CREATION_TIME];
        extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_CREATION_TIME];
        sprintf(buf, "%d", metadata.creationTimeSec);
        value = buf;
        extraData.dataEntries.push_back(value);
        extraDataList.push_back(extraData);
        extraData.dataEntries.clear();
    }

    // Check ratings.
    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        if (metadata.ratings[i] > 0)
        {
            invalidCase = false;

            switch (i) {

            case MEXP_METADATA_RATING_STEALTH:
                extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_STEALTH_RATINGS];
                extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_STEALTH_RATINGS];
                break;

            case MEXP_METADATA_RATING_EFFICIENCY:
                extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_EFFICIENCY_RATINGS];
                extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_EFFICIENCY_RATINGS];
                break;

            case MEXP_METADATA_RATING_PRESERVATION:
                extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_PRESERVATION_RATINGS];
                extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_PRESERVATION_RATINGS];
                break;

            case MEXP_METADATA_RATING_DURATION:
                extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_DURATION_RATINGS];
                extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_DURATION_RATINGS];
                break;

            default:
                invalidCase = true;
                break;
            }

            if (!invalidCase)
            {
                sprintf(buf, "%.0f", metadata.ratings[i]);
                value = buf;
                extraData.dataEntries.push_back(value);
                extraDataList.push_back(extraData);
            }

            extraData.dataEntries.clear();
        }
    }

    // Check user comment.
    for (i = 0; i < (int)(metadata.userComments.size()); i++)
    {
        if (metadata.userComments[i] != EMPTY_STRING_)
        {
            extraData.key = CBRPLANNER_EXTRADATA_KEYS[CBRPLANNER_EXTRADATA_KEY_USER_COMMENTS];
            extraData.format = CBRPLANNER_EXTRADATA_FORMATS[CBRPLANNER_EXTRADATA_KEY_USER_COMMENTS];
            extraData.dataEntries.push_back(metadata.userComments[i]);
            extraDataList.push_back(extraData);
            extraData.dataEntries.clear();
        }
    }

    sendExtraDataListToServer_(dataIndexList, extraDataList);
}

//-----------------------------------------------------------------------
// This function sends extra data to the server and attach it as
// the case's metadata.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendExtraDataListToServer_(
    vector<int> dataIndexList,
    vector<CBRPlanner_ExtraData_t> extraDataList)
{
    CBRPlanner_ExtraData_t extraData;
    int indexSize, listSize, keySize, valueSize;
    int i, j;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag notify the server that fsa lists will be sent.
    sendCommandToServer_(CBRPCOM_CMD_EXTRADATA_LIST);

    // Send the case index, but first the number of the indexes.
    indexSize = dataIndexList.size();
    writeInteger_(shSocketFd_, indexSize);

    for (i = 0; i < indexSize; i++)
    {
        // Send the case index.
        writeInteger_(shSocketFd_, dataIndexList[i]);
    }

    // Send the size of the list
    listSize = extraDataList.size();
    writeInteger_(shSocketFd_, listSize);

    for (i = 0; i < listSize; i++)
    {
        extraData = extraDataList[i];

        // Send the size of the key
        keySize = extraData.key.size();
        writeInteger_(shSocketFd_, keySize);

        // Send the key.
        write(shSocketFd_, extraData.key.c_str(), keySize);

        // Send the format of the data.
        writeInteger_(shSocketFd_, extraData.format);

        // Send the number of values.
        writeInteger_(shSocketFd_, extraData.numDataEntries);

        for (j = 0; j < extraData.numDataEntries; j++)
        {
            // Send the size of the value.
            valueSize = extraData.dataEntries[j].size();
            writeInteger_(shSocketFd_, valueSize);

            // Send the value.
            write(shSocketFd_, extraData.dataEntries[j].c_str(), valueSize);
        }
    }

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves a mission plan afer sending features to the server.
//-----------------------------------------------------------------------
CBRPlanner_MissionPlan_t CfgEditCBRClient::retrieveMissionPlanFromServer_(void)
{
    CBRPlanner_MissionPlan_t missionplan;
    int i, j, dataIndexListSize;
    int dataIndex, extraDataSize, keySize, valueSize;
	char *buf = NULL;
	CBRPlanner_ExtraData_t extraData;
	string metaValue;


    // Receive the mission type.
    missionplan.type = readInteger_(shSocketFd_);

    // Receive the rating.
    missionplan.rating = readInteger_(shSocketFd_);

	// Get the number of meta data fields
	extraDataSize = readInteger_(shSocketFd_);

	// Receive each peice of meta-data.
	for (i = 0; i < extraDataSize; i++)
	{
        // Get the size of the ith key.
        keySize = readInteger_(shSocketFd_);
        buf = new char[keySize+1];

        // Read the ith key.
        readNBytes_(shSocketFd_, (unsigned char *)buf, keySize);
        buf[keySize] = CHAR_NULL_;
        extraData.key = buf;
        delete[] buf;
        buf = NULL;

        // Read the format of the data.
        extraData.format = readInteger_(shSocketFd_);
	  
        // Read the number of values for this key.
        extraData.numDataEntries = readInteger_(shSocketFd_);
        extraData.dataEntries.clear();

        for (j = 0; j < extraData.numDataEntries; j++)
        {
            // Read the size of the value.
            valueSize = readInteger_(shSocketFd_);
            buf = new char[valueSize+1];
	  
            // Read the value.
            readNBytes_(shSocketFd_, (unsigned char *)buf, valueSize);
            buf[valueSize] = CHAR_NULL_;
            metaValue = buf;
            delete[] buf;
            buf = NULL;

            extraData.dataEntries.push_back(metaValue);
        }
	  
        // Add the latest extraData to the vector
        missionplan.extraDataList.push_back(extraData);
	}	  

    // Receive the list of the data indexes.
    missionplan.dataIndexList.clear();
    dataIndexListSize = readInteger_(shSocketFd_);

    for (i = 0; i < dataIndexListSize; i++)
    {
        dataIndex = readInteger_(shSocketFd_);
        missionplan.dataIndexList.push_back(dataIndex);
    }

    // Receive the number of robots.
    missionplan.numRobots = readInteger_(shSocketFd_);

    // Receive the data.
    missionplan.dataSize = readInteger_(shSocketFd_);
    missionplan.data = new char[missionplan.dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)(missionplan.data), missionplan.dataSize);
    missionplan.data[missionplan.dataSize] = CHAR_NULL_;

    return missionplan;
}

//-----------------------------------------------------------------------
// This function sends the FSA (tasks and triggers) names to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendFSANamesToServer_(config_fsa_lists_t fsaLists)
{
    int i, numTasks, numTriggers, nameSize;

    // First, send a flag notify the server that fsa lists will be sent.
    sendCommandToServer_(CBRPCOM_CMD_FSANAMES);

    // Send the number of tasks.
    numTasks = fsaLists.taskList.size();
    write(shSocketFd_, &numTasks, sizeof(int));

    for (i = 0; i < numTasks; i++)
    {
        // Send the length of the task name.
        nameSize = fsaLists.taskList[i].name.size();
        write(shSocketFd_, &nameSize, sizeof(int));

        // Send the name of the task
        write(shSocketFd_, fsaLists.taskList[i].name.c_str(), nameSize);
    }

    // Send the number of triggers.
    numTriggers = fsaLists.triggerList.size();
    write(shSocketFd_, &numTriggers, sizeof(int));

    for (i = 0; i < numTriggers; i++)
    {
        // Send the length of the trigger name.
        nameSize = fsaLists.triggerList[i].name.size();
        write(shSocketFd_, &nameSize, sizeof(int));

        // Send the name of the trigger
        write(shSocketFd_, fsaLists.triggerList[i].name.c_str(), nameSize);
    }
}

//-----------------------------------------------------------------------
// This function sends a positive feedback (regarding the latest mission)
// to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::savePositiveFeedback(vector<int> dataIndexList)
{
    sendFeedbackToServer_(CBRPCOM_CMD_POSITIVE_FEEDBACK, dataIndexList);
}

//-----------------------------------------------------------------------
// This function sends a negative feedback (regarding the latest mission)
// to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::saveNegativeFeedback(vector<int> dataIndexList)
{
    sendFeedbackToServer_(CBRPCOM_CMD_NEGATIVE_FEEDBACK, dataIndexList);
}

//-----------------------------------------------------------------------
// This function sends a feedback (regarding the latest mission) to the
// server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendFeedbackToServer_(const unsigned char cmd, vector<int> dataIndexList)
{
    int i, dataIndexListSize;

    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(cmd);

    dataIndexListSize = dataIndexList.size();
    write(shSocketFd_, &dataIndexListSize, sizeof(int));

    for (i = 0; i < dataIndexListSize; i++)
    {
        write(shSocketFd_, &(dataIndexList[i]), sizeof(int));
    }

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves logfile info from the server.
//-----------------------------------------------------------------------
MExpLogfileInfo_t CfgEditCBRClient::retrieveLogfileInfoFromServer_(void)
{
    MExpLogfileInfo_t logfileInfo;
    int dataSize;
    char *data = NULL;

    dataSize = readInteger_(shSocketFd_);
    data = new char[dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);

    logfileInfo.name = data;
    logfileInfo.name[dataSize] = CHAR_NULL_;
    delete [] data;
    data = NULL;

    return logfileInfo;
}

//-----------------------------------------------------------------------
// This function retrieves state info from the server.
//-----------------------------------------------------------------------
    vector<MExpStateInfo_t> CfgEditCBRClient::retrieveStateInfoFromServer_(void)
    {
        MExpStateInfo_t stateInfo;
        vector<MExpStateInfo_t> stateInfoList;
        int i, dataSize, numStateInfo, nameSize;
        char *data, *dataStart = NULL, *nameBuf = NULL;

        dataSize = readInteger_(shSocketFd_);
        data = new char[dataSize];
        dataStart = data;
        readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);

        // Copy the size of the list.
        memcpy(&numStateInfo, data, sizeof(int));
        data += sizeof(int);

        for (i = 0; i < numStateInfo; i++)
        {
            // Copy the size of the FSA name.
            memcpy(&nameSize, data, sizeof(int));
            data += sizeof(int);

            // Copy the FSA name itself.
            nameBuf = new char[nameSize+1];
            memcpy(nameBuf, data, nameSize);
            data += nameSize;
            nameBuf[nameSize] = CHAR_NULL_;
            stateInfo.fsaName = nameBuf;
            delete nameBuf;
            nameBuf = NULL;

            // Copy the size of the state name.
            memcpy(&nameSize, data, sizeof(int));
            data += sizeof(int);

            // Copy the state name itself.
            nameBuf = new char[nameSize+1];
            memcpy(nameBuf, data, nameSize);
            data += nameSize;
            nameBuf[nameSize] = CHAR_NULL_;
            stateInfo.stateName = nameBuf;
            delete nameBuf;
            nameBuf = NULL;

            stateInfoList.push_back(stateInfo);
        }

        data = dataStart;
        delete [] data;
        data = NULL;

        return stateInfoList;
    }

//-----------------------------------------------------------------------
// This function sends logfile info to the server.
//-----------------------------------------------------------------------
void CfgEditCBRClient::sendLogfileInfoToServer_(
    vector<MExpLogfileInfo_t> logfileInfoList)
{
    char *dummyData;
    int i, numLogfileInfoList, dataSize;

    // Read a dummy data first.
    dataSize = readInteger_(shSocketFd_);
    dummyData = new char[dataSize];
    readNBytes_(shSocketFd_, (unsigned char *)dummyData, dataSize);
    delete [] dummyData;

    sendCommandToServer_(CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE);

    numLogfileInfoList = logfileInfoList.size();
    writeInteger_(shSocketFd_, numLogfileInfoList);

    for (i = 0; i < numLogfileInfoList; i++)
    {
        dataSize = logfileInfoList[i].name.size();
        writeInteger_(shSocketFd_, dataSize);
        write(shSocketFd_, logfileInfoList[i].name.c_str(), dataSize);
    }
}

//-----------------------------------------------------------------------
// This function sends overlay info to the server.
//-----------------------------------------------------------------------
    void CfgEditCBRClient::sendOverlayInfoToServer_(string overlayFileName)
    {
        char *dummyData;
        int dataSize;

        // Read a dummy data first.
        dataSize = readInteger_(shSocketFd_);
        dummyData = new char[dataSize];
        readNBytes_(shSocketFd_, (unsigned char *)dummyData, dataSize);
        delete [] dummyData;

        sendCommandToServer_(CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE);

        dataSize = overlayFileName.size();
        writeInteger_(shSocketFd_, dataSize);
        write(shSocketFd_, overlayFileName.c_str(), dataSize);
    }

//-----------------------------------------------------------------------
// This function read and process the server data.
//-----------------------------------------------------------------------
    void CfgEditCBRClient::readAndProcessServerData_(void)
    {
        MExpLogfileInfo_t logfileInfo;
        vector<MExpStateInfo_t> stateInfoList;
        vector<MExpLogfileInfo_t> logfileInfoList;
        string overlayFileName;
        unsigned char oneByteBuf[1];
        const bool DEBUG_READ_AND_PROCESS_SERVER_DATA = false;

        readNBytes_(shSocketFd_, (unsigned char *)&oneByteBuf, 1);

        if (DEBUG_READ_AND_PROCESS_SERVER_DATA)
        {
            fprintf(
                stderr,
                "CfgEditCBRClient::readAndProcessServerData_(): Byte [%x] receieved.\n",
                oneByteBuf[0]);
        }

        switch (oneByteBuf[0]) {

        case CBRPCOM_CMD_LOGFILE_INFO_SAVE:
            logfileInfo = retrieveLogfileInfoFromServer_();
            if (gMExp != NULL)
            {
                gMExp->saveLogfileInfo(logfileInfo);
            }
            break;

        case CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE:
            if (gMExp != NULL)
            {
                logfileInfoList  = gMExp->getLogfileInfoList();
            }
            sendLogfileInfoToServer_(logfileInfoList);
            break;

        case CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE:
            if (gMExp != NULL)
            {
                overlayFileName  = gMExp->getOverlayFileName();
            }
            sendOverlayInfoToServer_(overlayFileName);
            break;

        case CBRPCOM_CMD_REPORT_CURRENT_STATE:
            stateInfoList = retrieveStateInfoFromServer_();
            if (gMExp != NULL)
            {
                gMExp->highlightStates(stateInfoList);
            }
            break;

        default:
            break;
        }
    }

//-----------------------------------------------------------------------
// This function sends the index of the highlighted state.
//-----------------------------------------------------------------------
    void CfgEditCBRClient::saveHighlightedStateInfo(MExpStateInfo_t highlightedStateInfo)
    {
        CBRPlanner_StateInfo_t *stateInfo = NULL;

        stateInfo = (CBRPlanner_StateInfo_t *)&highlightedStateInfo;

        pthread_mutex_lock(&socketMutex_);
        sendCommandToServer_(CBRPCOM_CMD_HIGHLIGHTED_STATE);
        sendStateInfo_(*stateInfo);
        pthread_mutex_unlock(&socketMutex_);
    }

/**********************************************************************
 * $Log: cfgedit_cbrclient.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.17  2007/09/29 23:45:06  endo
 * Global feature can be now disabled.
 *
 * Revision 1.16  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.15  2007/09/18 22:36:11  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.14  2007/08/10 15:14:59  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.13  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.12  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.11  2007/08/04 23:52:54  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.10  2007/07/17 21:53:51  endo
 * Meta data sorting function added.
 *
 * Revision 1.9  2007/06/28 14:01:08  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.8  2007/02/14 17:30:10  endo
 * Minor bug fix for displaying meta-data.
 *
 * Revision 1.7  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.6  2007/02/12 11:35:53  nadeem
 * Modified to code to ensure received strings are always null terminated.
 *
 * Revision 1.5  2007/02/11 22:40:38  nadeem
 * Added the code to send the META-DATA from the cbrplanner to cfgedit_cbrclient
 *
 * Revision 1.4  2007/01/29 15:08:35  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.3  2007/01/28 21:05:30  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.2  2006/09/26 18:30:27  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.8  2006/02/19 17:52:51  endo
 * Experiment related modifications
 *
 * Revision 1.7  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.6  2006/01/30 02:47:27  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.5  2005/08/12 21:49:58  endo
 * More improvements for August demo.
 *
 * Revision 1.4  2005/08/09 19:12:43  endo
 * Things improved for the August demo.
 *
 * Revision 1.3  2005/07/31 03:39:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/07/16 08:47:49  endo
 * CBR-CNP integration
 *
 * Revision 1.1  2005/02/07 22:25:25  endo
 * Mods for usability-2004
 *
 * Revision 1.2  2003/04/06 08:48:14  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/13 01:08:36  endo
 * Initial revision
 *
 **********************************************************************/
