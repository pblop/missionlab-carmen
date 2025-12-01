/**********************************************************************
 **                                                                  **
 **                      cbrplanner_protocol.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This header file contains constants and structures for the      **
 **  socket communication between CBRServer and CBRClient.           **
 **                                                                  **
 **  Copyright 2003 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CBRPLANNER_PROTOCOL_H
#define CBRPLANNER_PROTOCOL_H

/* $Id: cbrplanner_protocol.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <string>
#include <vector>

using std::string;
using std::vector;

#define CBRPCOM_MAX_BUFSIZE (4096)

#define CBRPCOM_SYNC_BYTE1 (0x01)
#define CBRPCOM_SYNC_BYTE2 (0x02)

#define CBRPCOM_CMD_CLIENTINIT (0x11)
#define CBRPCOM_CMD_CLIENTEXIT (0x12)
#define CBRPCOM_CMD_FSANAMES (0x13)
#define CBRPCOM_CMD_MISSIONPLAN_SAVE (0x14)
#define CBRPCOM_CMD_MISSIONPLAN_RETRIEVE (0x15)
#define CBRPCOM_CMD_MISSIONPLAN_FIX (0x16)
#define CBRPCOM_CMD_SELECTED_MISSION (0x17)
#define CBRPCOM_CMD_POSITIVE_FEEDBACK (0x18)
#define CBRPCOM_CMD_NEGATIVE_FEEDBACK (0x19)
#define CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE (0x1A)
#define CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE (0x1B)
#define CBRPCOM_CMD_LOGFILE_INFO_SAVE (0x1C)
#define CBRPCOM_CMD_POLLING_DATA (0x1D)
#define CBRPCOM_CMD_REPORT_CURRENT_STATE (0x1E)
#define CBRPCOM_CMD_REPAIR_Q_RETRIEVE (0x1F)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_RETRIEVE (0x20)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_TOGGLE_REPLY (0x21)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_ADD_MAP_POINT_REPLY (0x22)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_MOD_MAP_POINT_REPLY (0x23)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_DEL_MAP_POINT_REPLY (0x24)
#define CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_STATENAME_REPLY (0x25)
#define CBRPCOM_CMD_HIGHLIGHTED_STATE (0x26)
#define CBRPCOM_CMD_LOGFILE_INFO_ACKNOWLEDGE (0x27)
#define CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_PREMISSION (0x28)
#define CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_RUNTIME (0x29)
#define CBRPCOM_CMD_ROBOT_ID_LIST_RETRIEVE (0x2A)
#define CBRPCOM_CMD_CNP_STATUS_RETRIEVE (0x2B)
#define CBRPCOM_CMD_EXTRADATA_LIST (0x2C)
#define CBRPCOM_CMD_USAGE_COUNTER_UPDATE (0x2D)
#define CBRPCOM_CMD_CHECK_CBR_LIBRARY_FILE_SAVE (0x2E)
#define CBRPCOM_CMD_SAVE_CBR_LIBRARY_FILE (0x2F)
#define CBRPCOM_CMD_REVERSE_ROBOT_ORDER (0x30)
#define CBRPCOM_CMD_MERGE_MISSIONS (0x31)

typedef enum CBPlanner_FormatType_t
{
    CBRPLANNER_FORMAT_STRING,
    CBRPLANNER_FORMAT_INT,
    CBRPLANNER_FORMAT_DOUBLE,
    CBRPLANNER_FORMAT_BOOLEAN,
    CBRPLANNER_FORMAT_NA
};

typedef enum CBRPlanner_ClientType_t
{
    CBRPLANNER_CLIENT_UNKNOWN,
    CBRPLANNER_CLIENT_CFGEDIT,
    CBRPLANNER_CLIENT_MLAB_MMD,
    CBRPLANNER_CLIENT_MLAB_MPB,
    CBRPLANNER_CLIENT_ROBOT_EXE,
    NUM_CBRPLANNER_CLIENT_TYPES
};

const string CBRPLANNER_CLIENT_TYPES_STRING[NUM_CBRPLANNER_CLIENT_TYPES] = 
{
    "CBRPLANNER_CLIENT_UNKNOWN",
    "CBRPLANNER_CLIENT_CFGEDIT",
    "CBRPLANNER_CLIENT_MLAB_MMD",
    "CBRPLANNER_CLIENT_MLAB_MPB",
    "CBRPLANNER_CLIENT_ROBOT_EXE"
};

typedef enum CBRPlanner_MissionPlanType_t
{
    CBRPLANNER_MISSIONPLAN_EMPTY,
    CBRPLANNER_MISSIONPLAN_NEW,
    CBRPLANNER_MISSIONPLAN_OLD,
    CBRPLANNER_MISSIONPLAN_CONSTRUCTED,
    CBRPLANNER_MISSIONPLAN_REPAIRED
};

typedef struct CBRPlanner_ClientInfo_t
{
    int type;
    int pid;
    string name;
};

typedef enum CBRPlanner_ExtraDataKeyType_t
{
    // If you add a new key, make sure to add the corresponding
    // string in CBRPLANNER_EXTRADATA_KEYS[] and format in
    // CBRPLANNER_EXTRADATA_FORMATS below.
    CBRPLANNER_EXTRADATA_KEY_CREATION_TIME,
    CBRPLANNER_EXTRADATA_KEY_NUM_USAGE,
    CBRPLANNER_EXTRADATA_KEY_COMPUTED_RELEVANCE,
    CBRPLANNER_EXTRADATA_KEY_STEALTH_RATINGS,
    CBRPLANNER_EXTRADATA_KEY_EFFICIENCY_RATINGS,
    CBRPLANNER_EXTRADATA_KEY_PRESERVATION_RATINGS,
    CBRPLANNER_EXTRADATA_KEY_DURATION_RATINGS,
    CBRPLANNER_EXTRADATA_KEY_USER_COMMENTS,
    NUM_CBRPLANNER_EXTRADATA_KEYS
};

const string CBRPLANNER_EXTRADATA_KEYS[NUM_CBRPLANNER_EXTRADATA_KEYS] =
{
    "CreationTime", // Seconds since "00:00:00 1970-01-01 UTC". (From "date '+%s'".)
    "NumUsage", // Number of times used.
    "ComputedRelevance", // Computed rating.
    "StealthRatings", // User specified ratings. (list)
    "EfficiencyRatings", // User specified ratings. (list)
    "PreservationRatings", // User specified ratings. (list)
    "DurationRatings", // User specified ratings. (list)
    "UserComments" // User comments. (list)
};

const int CBRPLANNER_EXTRADATA_FORMATS[NUM_CBRPLANNER_EXTRADATA_KEYS] =
{
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_CREATION_TIME
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_NUM_USAGE
    CBRPLANNER_FORMAT_DOUBLE, // CBRPLANNER_EXTRADATA_KEY_COMPUTED_RELEVANCE
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_STEALTH_RATINGS
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_EFFICIENCY_RATINGS
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_PRESERVATION_RATINGS
    CBRPLANNER_FORMAT_INT, // CBRPLANNER_EXTRADATA_KEY_DURATION_RATINGS
    CBRPLANNER_FORMAT_STRING // CBRPLANNER_EXTRADATA_KEY_USER_COMMENTS
};

typedef struct CBRPlanner_ExtraData_t
{
    string key; // CBRPLANNER_EXTRADATA_KEYS[].
    int format; // CBRPLANNER_EXTRADATA_FORMATS[].
    int numDataEntries; // Size of the vector below.
    vector<string> dataEntries; // List of the (extra) data.
};

typedef struct CBRPlanner_MissionPlan_t
{
    int type;
    int rating; // Obsolete. To be replaced by CBRPlanner_ExtraData_t.
    int numRobots;
    vector<int> dataIndexList;
    int dataSize;
    char *data;
    vector<CBRPlanner_ExtraData_t> extraDataList;
};

typedef struct CBRPlanner_Feature_t
{
    string name;
    string value;
    bool nonIndex;
    double weight;
    int formatType;
};

typedef struct CBRPlanner_Constraint_t
{
    string name;
    string value;
    int id;
    int formatType;
};

typedef struct CBRPlanner_Constraints_t
{
    int id;
    string name;
    vector<CBRPlanner_Constraint_t> constraints;
};

typedef CBRPlanner_Constraints_t CBRPlanner_TaskConstraints_t;

typedef CBRPlanner_Constraints_t CBRPlanner_RobotConstraints_t;

typedef struct CBRPlanner_OverlayInfo_t
{
    string name;
};

typedef struct CBRPlanner_LogfileInfo_t
{
    string name;
};

// Make sure this structure matches with MlabPlayBack::StateInfo_t in
// gt_playback.h.
typedef struct CBRPlanner_StateInfo_t
{
    string fsaName;
    string stateName;
};

typedef struct CBRPlanner_RobotIDs_t
{
    vector<int> robotIDs;
};

typedef struct CBRPlanner_CNPStatus_t
{
    int status;
    string errorMsg;
};

#endif
/**********************************************************************
 * $Log: cbrplanner_protocol.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
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
 * Revision 1.4  2007/02/12 16:49:51  endo
 * UserRatings divided into more specific categories
 *
 * Revision 1.3  2007/02/02 22:06:47  endo
 * CreationTime added to the extra data keys.
 *
 * Revision 1.2  2007/02/02 20:17:31  endo
 * CBRPlanner_ExtraData_t added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.7  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.6  2005/08/12 21:48:29  endo
 * More improvements for August demo.
 *
 * Revision 1.5  2005/07/31 03:41:40  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.4  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.3  2005/06/23 22:09:04  endo
 * Adding the mechanism to talk to CNP.
 *
 * Revision 1.2  2005/02/07 19:53:43  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:26:49  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
