/**********************************************************************
 **                                                                  **
 **                       mission_expert_types.h                     **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_expert_types.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#ifndef MISSION_EXPERT_TYPES_H
#define MISSION_EXPERT_TYPES_H

#include <string>
#include <vector>

using std::string;
using std::vector;

typedef struct MExpPoint2D_t
{
    double x, y; 
};

typedef struct MExpLatLon_t
{
    double latitude, longitude; 
};

typedef enum MExpFeatureOptionType_t
{
    MEXP_FEATURE_OPTION_TOGGLE,
    MEXP_FEATURE_OPTION_SLIDER1,
    MEXP_FEATURE_OPTION_SLIDER10,
    MEXP_FEATURE_OPTION_SLIDER100,
    MEXP_FEATURE_OPTION_POLYGONS,
    MEXP_FEATURE_OPTION_STRINGS
};

// If a new element is added, make sure to specify its default value
// in CBRWizard::clearFeatureOption_().
typedef struct MExpFeatureOption_t
{
    string value;
};

// Make sure this match with MEXP_FEATURE_TYPE below.
typedef enum MExpFeatureType_t
{
    MEXP_FEATURE_GLOBAL,
    MEXP_FEATURE_LOCAL,
    MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT,
    MEXP_FEATURE_TASK_CONSTRAINT,
    MEXP_FEATURE_ROBOT_CONSTRAINT,
    MEXP_FEATURE_RUNTIME_CNP_MISSION_MANAGER,
    MEXP_FEATURE_RUNTIME_CNP_BIDDER,
    MEXP_FEATURE_RUNTIME_CNP_BOTH,
    NUM_MEXP_FEATURE_TYPES
};

// Make sure this match with MExpFeatureType_t above.
const string MEXP_FEATURE_TYPE[NUM_MEXP_FEATURE_TYPES] = 
{
    "MEXP_FEATURE_GLOBAL",
    "MEXP_FEATURE_LOCAL",
    "MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT",
    "MEXP_FEATURE_TASK_CONSTRAINT",
    "MEXP_FEATURE_ROBOT_CONSTRAINT",
    "MEXP_FEATURE_RUNTIME_CNP_MISSION_MANAGER",
    "MEXP_FEATURE_RUNTIME_CNP_BIDDER",
    "MEXP_FEATURE_RUNTIME_CNP_BOTH"
};

// If a new element is added, make sure to specify its default value
// in CBRWizard::clearFeature_().
typedef struct MExpFeature_t
{
    int id;
    int type;
    bool nonIndex;
    bool hide;
    bool allowDisable;
    double weight;
    string name;
    int optionType;
    vector<MExpFeatureOption_t> options;
    float selectedOption; // Value '-1' means disabled (when allowDisable = true).
};

typedef struct MExpMissionTask_t
{
    int number;
    int id;
    int maxUsage; // Value '0' means infinity (MEXP_USAGE_INFINITY).
    string name;
    string buttonName;
    vector<MExpFeature_t> localFeatures;
    MExpPoint2D_t position;
    MExpLatLon_t geo;
};

typedef struct MExpConstraints_t
{
    int id;
    string name;
    vector<MExpFeature_t> constraints;
};

typedef MExpConstraints_t MExpRobotConstraints_t;

typedef MExpConstraints_t MExpTaskConstraints_t;

typedef struct MExpRobotIDs_t
{
    vector<int> robotIDs;
};

// Make sure to match this enum with CNP_OutputStatus_t in
// cnp_server_manager.h
typedef enum MExpCNPStatusType_t
{
    MEXP_CNP_OUTPUT_NORMAL, // Regular one-to-one mapping
    MEXP_CNP_OUTPUT_EXCESSIVE_ROBOT, // Some robot did not get a job
    MEXP_CNP_OUTPUT_INCOMPLETE_MAPPING, // Some job not assigned, lack of adequate robots
    MEXP_CNP_OUTPUT_EXCESSIVE_ROBOT_AND_INCOMPLETE_MAPPING,
    MEXP_CNP_OUTPUT_UNEXPECTED_ERROR
};

typedef struct MExpCNPStatus_t
{
    int status;
    string errorMsg;
};

typedef enum MExpRunMode_t
{
    MEXP_RUNMODE_NONE,
    MEXP_RUNMODE_NEW,
    MEXP_RUNMODE_VIEW,
    MEXP_RUNMODE_REPLAY
};

typedef enum MExpMetadataSpecialText_t
{
    MEXP_METADATA_SPECIAL_TEXT_LINEBREAK,
    MEXP_METADATA_SPECIAL_TEXT_QUOTATION,
    MEXP_METADATA_SPECIAL_TEXT_SPACE,
    NUM_MEXP_METADATA_SPECIAL_TEXTS
};

typedef struct MExpMetadataSpecialTextConversion_t
{
    string specialText;
    string convertedText;
};

const MExpMetadataSpecialTextConversion_t MEXP_METADATA_SPECIAL_TEXT_CONVERSION[NUM_MEXP_METADATA_SPECIAL_TEXTS] =
{
    {"\n", "<br>"}, // MEXP_METADATA_SPECIAL_TEXT_LINEBREAK
    {"\"", "&quot;"}, // MEXP_METADATA_SPECIAL_TEXT_QUOTATION
    {" ", "&nbsp;"} // MEXP_METADATA_SPECIAL_TEXT_SPACE
};

typedef enum MExpMetadataRating_t
{
    MEXP_METADATA_RATING_STEALTH,
    MEXP_METADATA_RATING_EFFICIENCY,
    MEXP_METADATA_RATING_PRESERVATION,
    MEXP_METADATA_RATING_DURATION,
    NUM_MEXP_METADATA_RATINGS
};

typedef struct MExpMetadata_t
{
    int creationTimeSec;
    int numUsage;
    double relevance;
    double ratings[NUM_MEXP_METADATA_RATINGS];
    vector<string> userComments;
};

typedef struct MExpRetrievedMission_t
{
    int index;
    int rating; // To be obsolete.
    int numRobots;
    vector<int> dataIndexList;
    string acdlSolution;
    MExpMetadata_t metadata;
    bool feedbackSent;
    bool usageCounterUpdated;
    bool newUnsavedMission;

    static bool isAscendingIndex(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.index) <= (b.index));
    }

    static bool isDescendingIndex(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.index) > (b.index));
    }

    static bool isAscendingRelevance(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.relevance) <= (b.metadata.relevance));
    }

    static bool isDescendingRelevance(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.relevance) > (b.metadata.relevance));
    }

    static bool isAscendingRatingStealth(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_STEALTH]) <= (b.metadata.ratings[MEXP_METADATA_RATING_STEALTH]));
    }

    static bool isDescendingRatingStealth(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_STEALTH]) > (b.metadata.ratings[MEXP_METADATA_RATING_STEALTH]));
    }

    static bool isAscendingRatingEfficiency(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_EFFICIENCY]) <= (b.metadata.ratings[MEXP_METADATA_RATING_EFFICIENCY]));
    }

    static bool isDescendingRatingEfficiency(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_EFFICIENCY]) > (b.metadata.ratings[MEXP_METADATA_RATING_EFFICIENCY]));
    }

    static bool isAscendingRatingPreservation(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_PRESERVATION]) <= (b.metadata.ratings[MEXP_METADATA_RATING_PRESERVATION]));
    }

    static bool isDescendingRatingPreservation(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_PRESERVATION]) > (b.metadata.ratings[MEXP_METADATA_RATING_PRESERVATION]));
    }

    static bool isAscendingRatingDuration(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_DURATION]) <= (b.metadata.ratings[MEXP_METADATA_RATING_DURATION]));
    }

    static bool isDescendingRatingDuration(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.ratings[MEXP_METADATA_RATING_DURATION]) > (b.metadata.ratings[MEXP_METADATA_RATING_DURATION]));
    }

    static bool isAscendingNumUsage(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.numUsage) <= (b.metadata.numUsage));
    }

    static bool isDescendingNumUsage(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.numUsage) > (b.metadata.numUsage));
    }

    static bool isAscendingNumUserComments(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.userComments.size()) <= (b.metadata.userComments.size()));
    }

    static bool isDescendingNumUserComments(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.metadata.userComments.size()) > (b.metadata.userComments.size()));
    }

    static bool isAscendingNumRobots(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.numRobots) <= (b.numRobots));
    }

    static bool isDescendingNumRobots(const MExpRetrievedMission_t &a, const MExpRetrievedMission_t &b)
    {
        return ((a.numRobots) > (b.numRobots));
    }
};

typedef struct MExpRetrievedMissionSummary_t
{
    vector<int> ratings; 
    vector<string> fsaSummaries;
};

typedef struct MExpLogfileInfo_t
{
    string name;
};

typedef struct MExpStateInfo_t {
    string fsaName;
    string stateName;
};

#endif

/**********************************************************************
 * $Log: mission_expert_types.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.11  2007/09/29 23:45:06  endo
 * Global feature can be now disabled.
 *
 * Revision 1.10  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.9  2007/09/18 22:36:12  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.8  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.7  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.6  2007/08/04 23:52:55  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.5  2007/07/17 21:53:51  endo
 * Meta data sorting function added.
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
 * Revision 1.9  2006/05/15 01:23:28  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.8  2006/01/30 02:47:28  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.7  2005/08/12 22:42:59  endo
 * More improvements for August demo.
 *
 * Revision 1.6  2005/08/12 21:49:59  endo
 * More improvements for August demo.
 *
 * Revision 1.5  2005/08/09 19:12:44  endo
 * Things improved for the August demo.
 *
 * Revision 1.4  2005/07/31 03:39:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.3  2005/07/16 08:47:49  endo
 * CBR-CNP integration
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:35  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2003/04/06 15:21:55  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/06 08:51:21  endo
 * Initial revision
 *
 **********************************************************************/
