/**********************************************************************
 **                                                                  **
 **                      mission_memory_manager.h                    **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MISSION_MEMORY_MANAGER_H
#define MISSION_MEMORY_MANAGER_H

/* $Id: mission_memory_manager.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string>
#include <string.h>	//Old compilation error: strncpy not defined

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "memory_manager.h"

using std::string;

class MissionMemoryManager : public MemoryManager {

public:
    typedef struct Coordinates_t
    {
        double x;
        double y;
    };

    typedef struct GeoCoordinates_t
    {
        double latitude;
        double longitude;
    };

    typedef vector<Coordinates_t> PointList_t;

    typedef vector<PointList_t> PolygonSet_t;

    typedef struct MissionPlan_t
    {
        int type;
        int rating; // Obsolete. To be replaced by CBRPlanner_ExtraData_t.
        string archType;
        vector<CBRPlanner_PlanSequence_t> plans; // Size of this vector indicates # of robots
        vector<string> maxVelValues; // Size of this vector indicates # of robots also
        vector<string> baseVelValues; // Size of this vector indicates # of robots also
        vector<int> robotIDs; // Size of this vector indicates # of robots also
        vector<int> dataIndexList;
        vector<CBRPlanner_ExtraData_t> extraDataList; // Meta-Data about the MissionPlan
    };

    typedef vector<MissionPlan_t> MissionPlanList_t;

protected:
    MissionPlanList_t missionPlanList_;
    string cbrlibraryName_;
    
    static const string CBRLIBRARY_EXTENSION_;
    static const string CBL_STRING_DATA_;
    static const string CBL_STRING_META_DATA_;
    static const string CBL_STRING_META_DATA_KEY_;
    static const string CBL_STRING_META_DATA_VALUE_;
    static const string CBL_STRING_TYPE_;
    static const string CBL_STRING_RATING_;
    static const string CBL_STRING_ARCHTYPE_;
    static const string CBL_STRING_PLAN_;
    static const string CBL_STRING_OPERATOR_;
    static const string CBL_STRING_OPERATOR_NAME_;
    static const string CBL_STRING_OPERATOR_INDEX_;
    static const string CBL_STRING_OPERATOR_DESC_;
    static const string CBL_STRING_OPERATOR_PARM_;
    static const string CBL_STRING_OPERATOR_PARM_NAME_;
    static const string CBL_STRING_OPERATOR_PARM_VALUE_;
    static const string CBL_STRING_OPERATOR_PRECONDITION_;
    static const string CBL_STRING_OPERATOR_PRECONDITION_NAME_;
    static const string CBL_STRING_OPERATOR_EFFECT_;
    static const string CBL_STRING_OPERATOR_EFFECT_NAME_;
    static const string CBL_STRING_OPERATOR_EFFECT_INDEX_;
    static const string CBL_STRING_OPERATOR_EFFECT_PARM_;
    static const string CBL_STRING_OPERATOR_EFFECT_PARM_NAME_;
    static const string CBL_STRING_OPERATOR_EFFECT_PARM_VALUE_;
    static const string CBL_STRING_DECISIONTREE_;
    static const string CBL_STRING_NODE_;
    static const string CBL_STRING_NODE_NAME_;
    static const string CBL_STRING_NODE_LEAF_;
    static const string CBL_STRING_NODE_EDGE_;
    static const string CBL_STRING_NODE_EDGE_PARENT_;
    static const string CBL_STRING_NODE_EDGE_VALUE_;
    static const string CBL_STRING_NODE_EDGE_NEXTNODE_;
    static const string CBL_STRING_NODE_DATA_;
    static const string CBL_STRING_NODE_DATA_INDEXES_;
    static const string CBL_STRING_NODE_DATA_INDEX_;
    static const string CBL_STRING_NODE_DATA_STATUSES_;
    static const string CBL_STRING_NODE_DATA_STATUS_;
    static const string CBL_STRING_NODE_DATA_OTHERFEATURE_;
    static const string CBL_STRING_NODE_DATA_OTHERFEATURE_NAME_;
    static const string CBL_STRING_NODE_DATA_OTHERFEATURE_VALUE_;
    static const string CBL_STRING_START_;
    static const string CBL_STRING_END_;

    MissionPlan_t loadMissionPlan_(FILE *in);
    DecisionTree_t *loadDecisionTree_(FILE *in);
    DecisionTreeData_t loadNodeData_(FILE *in);
    DecisionTreeNode_t *loadNode_(FILE *in);
    string askCBRLibraryName_(void);
    string addCBRLibraryExtension_(string cbrlibraryName);
    string createCBRLibraryNamePrefix_(void);
    bool saveVerify_(string cbrlibraryName);
    bool loadVerify_(string cbrlibraryName);
    bool hasCBRLibraryExtension_(string cbrlibraryName);
    bool confirmClear_(void);
    void saveCBRLibrary_(string cbrlibraryName);
    void loadCBRLibrary_(FILE *in);
    void checkSaveCBRLibrary(void);
    void writeMissionPlans_(FILE *out);
    void writeDecisionTree_(FILE *out, DecisionTree_t *decisionTree);
    void writeNode_(FILE *out, DecisionTreeNode_t *node);
    void writeNodeData_(FILE *out, DecisionTreeData_t data);

public:
    MissionMemoryManager(void);
    MissionMemoryManager(MissionMemoryManager **self);
    ~MissionMemoryManager(void);

    vector<MissionPlan_t> getSavedMissionPlans(
        Features_t features,
        int maxRating,
        bool disableBacktrack = false);
    string getCBRLibraryName(void);
    int saveNewMissionPlan(MissionPlan_t missionplan, Features_t features);
    bool confirmClear(void);
    void loadCBRLibrary(void);
    void loadCBRLibrary(string cbrlibraryName);
    void saveCBRLibrary(void);
    void saveCBRLibrary(string cbrlibraryName);
    void confirmSaveCBRLibrary(void);
    void saveExtraDataList(
        vector<int> dataIndexList,
        vector<CBRPlanner_ExtraData_t> extraDataList);
    void updateMissionUsageCounter(vector<int> dataIndexList);
};

inline bool MissionMemoryManager::confirmClear(void)
{
    return (confirmClear_());
}

inline string MissionMemoryManager::getCBRLibraryName(void)
{
    return cbrlibraryName_;
}

#endif
/**********************************************************************
 * $Log: mission_memory_manager.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/09/29 23:45:36  endo
 * Global feature can be now disabled.
 *
 * Revision 1.7  2007/08/10 15:15:39  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.6  2007/08/06 22:08:47  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.5  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.4  2007/02/11 22:40:38  nadeem
 * Added the code to send the META-DATA from the cbrplanner to cfgedit_cbrclient
 *
 * Revision 1.3  2007/01/30 15:57:38  nadeem
 * Added data structures for Point and PointList
 *
 * Revision 1.2  2006/09/26 18:30:59  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2006/02/19 17:52:00  endo
 * Experiment related modifications
 *
 * Revision 1.3  2005/07/31 03:41:41  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/05/18 21:22:22  endo
 * AuRA.naval added.
 *
 * Revision 1.1  2005/02/07 19:53:44  endo
 * Mods from usability-2004
 *
 **********************************************************************/
