/**********************************************************************
 **                                                                  **
 **                         cfgedit_cbrclient.h                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for cbrclient.cc.                         **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CFGEDIT_CBRCLIENT_H
#define CFGEDIT_CBRCLIENT_H

/* $Id: cfgedit_cbrclient.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <string>
#include <vector>
#include "cbrclient.h"
#include "mission_expert_types.h"
#include "cbrplanner_protocol.h"
#include "configuration.hpp"

using std::string;
using std::vector;

class CfgEditCBRClient : public CBRClient {

protected:
    static const string STRING_TASK_;
    static const string STRING_START_X_;
    static const string STRING_START_Y_;
    static const string STRING_START_LATITUDE_;
    static const string STRING_START_LONGITUDE_;
    static const string STRING_ROBOT_CONSTRAINTS_;
    static const char CHAR_AMPERSAND_;
    static const char CHAR_DOUBLE_QUOTE_;

    MExpLogfileInfo_t retrieveLogfileInfoFromServer_(void);
    CBRPlanner_MissionPlan_t retrieveMissionPlanFromServer_(void);
    CBRPlanner_CNPStatus_t retrieveCurrentCNPStatusFromServer_(void);
    vector<int> sendMissionPlanToServer_(
        CBRPlanner_MissionPlan_t missionplan,
        vector<CBRPlanner_Feature_t> features);
    vector<CBRPlanner_MissionPlan_t> receiveMissionPlansFromServer_(void);
    vector<CBRPlanner_MissionPlan_t> retrieveMissionPlansFromServer_(
        vector<CBRPlanner_Feature_t> features,
        int maxRating,
        bool addStartSubMission);
    vector<CBRPlanner_MissionPlan_t> retrieveMissionPlansFromServerPremissionCNP_(
        vector<CBRPlanner_Feature_t> features,
        int maxRating,
        bool addStartSubMission,
        vector<CBRPlanner_TaskConstraints_t> taskConstraintsList,
        vector<CBRPlanner_RobotConstraints_t> robotConstraintsList);
    vector<CBRPlanner_MissionPlan_t> retrieveMissionPlansFromServerRuntimeCNP_(
        vector<CBRPlanner_Feature_t> features,
        int maxRating,
        bool addStartSubMission,
        vector<CBRPlanner_RobotConstraints_t> robotConstraintsList);
    vector<CBRPlanner_RobotIDs_t> retrieveCurrentRobotIDListFromServer_(void);
    vector<MExpRetrievedMission_t> cbrplannerMissionsPlan2RetrievedMissions_(
        vector<CBRPlanner_MissionPlan_t> missionPlans);
    void readAndProcessServerData_(void);
    void sendConstraintsToServer_(vector<CBRPlanner_Constraints_t> constraintsList);
    void sendExtraDataListToServer_(
        vector<int> dataIndexList,
        vector<CBRPlanner_ExtraData_t> extraDataList);
    void sendFeaturesToServer_(vector<CBRPlanner_Feature_t> features);
    void sendFeedbackToServer_(const unsigned char cmd, vector<int> dataIndexList);
    void sendFSANamesToServer_(config_fsa_lists_t fsaLists);
    void sendLogfileInfoToServer_(vector<MExpLogfileInfo_t> logfileList);
    void sendOverlayInfoToServer_(string overlayFileName);
    bool cbrplannerExtraDataList2Metadata_(
        vector<CBRPlanner_ExtraData_t> extraDataList,
        MExpMetadata_t *metadata);
    bool cbrplannerMissionPlan2RetrievedMission_(
        CBRPlanner_MissionPlan_t missionPlan,
        MExpRetrievedMission_t *retrievedMission);

public:
    CfgEditCBRClient(void);
    CfgEditCBRClient(const symbol_table<rc_chain> &rcTable);
    ~CfgEditCBRClient(void);
    vector<int> saveMissionPlan(
        vector<MExpFeature_t> globalFeatures_,
        MExpMissionTask_t task,
        double taskWeight,
        string solutionFileName);
    vector<MExpRetrievedMission_t> retrieveMissionPlan(
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
        bool *reload);
    vector<MExpRetrievedMission_t> fixMissionPlan(
        string missionPlanFileName);
    vector<MExpStateInfo_t> retrieveStateInfoFromServer_(void);
    string getMergedAbstractedMissionPlan(
        vector<string> acdlMissions,
        int *numRobots);
    string reverseRobotOrder(string acdlMission);
    void saveCBRLibraryFile(string filename);
    void saveFSANames(config_fsa_lists_t fsaLists);
    void savePositiveFeedback(vector<int> dataIndexList);
    void saveNegativeFeedback(vector<int> dataIndexList);
    void saveHighlightedStateInfo(MExpStateInfo_t highlightedStateInfo);
    void saveMetadata(vector<int> dataIndexList, MExpMetadata_t metadata);
    void updateMissionUsageCounter(vector<int> dataIndexList);
    bool checkCBRLibrarySave(string *cblFileName);
};

extern CfgEditCBRClient *gCfgEditCBRClient;

#endif
/**********************************************************************
 * $Log: cfgedit_cbrclient.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.9  2007/09/29 23:45:06  endo
 * Global feature can be now disabled.
 *
 * Revision 1.8  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.7  2007/08/10 15:14:59  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.6  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.5  2007/08/04 23:52:54  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.4  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
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
 * Revision 1.7  2006/02/19 17:52:51  endo
 * Experiment related modifications
 *
 * Revision 1.6  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.5  2006/01/30 02:47:27  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.4  2005/08/12 21:49:58  endo
 * More improvements for August demo.
 *
 * Revision 1.3  2005/07/31 03:39:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/07/16 08:47:49  endo
 * CBR-CNP integration
 *
 * Revision 1.1  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 **********************************************************************/
