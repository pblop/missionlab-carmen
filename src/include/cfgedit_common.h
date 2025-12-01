/**********************************************************************
 **                                                                  **
 **                     cfgedit_common.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: cfgedit_common.h,v 1.1.1.1 2008/07/14 16:44:20 endo Exp $ */


#ifndef CFGEDIT_COMMON_H
#define CFGEDIT_COMMON_H

#define RC_FILENAME             ".cfgeditrc"
#define CFLAGS_NAME             "cflags"
#define LDFLAGS_NAME            "ldflags"

#define BACKUP_FILES_NAME       "backup_files"
#define CDL_LIBRARIES           "CDL_libraries"
#define CNL_LIBRARIES           "CNL_libraries"
#define CNL_SOURCES             "CNL_sources"
#define CNL_LIBS_FLAG           "CNL_LIBS"
#define OVERLAY_FILES           "MapOverlays"

#define USER_PRIVILEGES_NAME    "user_privileges"
#define VISIBLE_NAMES_NAME      "restrict_names"

typedef unsigned long ObjectClasses;
typedef unsigned long OneObjectClass;
#define AnyObject ((ObjectClasses)(-1))

#if defined(WANT_OBJECT_DEFS)
// Mapping of bit number (position in the array) to an objects name and color
static struct {
    char* name;
    char* color;
} object_defs[] = {
    {"Mines",           "orange"},
    {"Enemy Robots",    "red"},
    {"Flags",           "purple"},
    {"EOD Areas",       "green"},
    {"Rocks",           "black"},
    {"Trees/Shrubs",    "DarkSeaGreen"},
    {"Home Base",  	    "white"},
    {"Unknown objects", "brown"},
    {"Hiding Places",   "lightblue"},
    {"Banana",          "yellow"},
    {"Frienly Robots",  "blue"},
    {NULL,  NULL}
};

#endif

// Matched pairs of states and triggers are defined in this
// array.
#if defined(USE_MATCHED_STATE_AND_TRIGGER)
static struct {
    char* state;
    char* trigger;
} matched_state_trigger[] = {
    {"AboutFace",              "AboutFaceCompleted"},
    {"Alert",                  "Alerted"},
    {"AssistedGoTo",           "AtGoal"},
    {"CNP_BidOnTask",          "CNP_IsAuctionEnded"},
    {"EnterRoom",              "InRoom"},
    {"EnterAlternateHallway",  "InAlternateHallway"},
    {"GoTo",                   "AtGoal"},
    {"GoTo_CBR",               "AtGoal"},
    {"GoToOutdoorNavigation",  "AtGoal"},
    {"GoToOutdoor_CBR",        "AtGoal"},
    {"GoTo_LM",                "AtGoal"},
    {"GoThroughDoor",          "ThroughDoorway"},
    {"GoTo_Dstar",             "AtGoal"},
	{"ICARUS_MO_AGM",          "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Communications","ICARUS_Objective_Completed"},
	{"ICARUS_MO_EOIR",         "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Loiter",       "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Mad",          "ICARUS_Objective_Completed"},
	{"ICARUS_MO_PGB",          "ICARUS_Objective_Completed"},
	{"ICARUS_MO_SARImage",     "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Search",       "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Steerpoint",   "ICARUS_Objective_Completed"},
	{"ICARUS_MO_Track",        "ICARUS_Objective_Completed"},
	{"ICARUS_MO_UAVSearch",    "ICARUS_Objective_Completed"},
	{"ICARUS_MO_UUVEo",        "ICARUS_Objective_Completed"},
	{"ICARUS_MO_UUVSearch",    "ICARUS_Objective_Completed"},
    {"LeaveRoom",              "InHallway"},
    {"LookFor",                "Detect"},
    {"MarkDoorway",            "MarkedDoorway"},
    {"MoveAhead",              "MovedDistance"},
    {"MoveAway",               "AwayFrom"},
    {"MoveInFormation",        "AtGoalInFormation"},
    {"MoveToward",             "Near"},
    {"MoveToward_LM",          "Near"},
    {"MoveToward_NotifiedObject", "NearNotifiedObject"},
    {"NotifyRobots",           "MessageSent"},
    {"PickUp",                 "Holding"},
    {"ProceedAlongHallway",    "AtDoorway"},
    {"Spin",                   "HasTurned"}, 
    {"StartSubMission",        "SubMissionReady"},
    {"Telop",                  "TelopComplete"}, 
    {"TestObject",             "TestPositive"}, 
    {"UnmarkDoorway",          "UnmarkedDoorway"},
    {NULL,  NULL}
};
#endif

#endif


/**********************************************************************
 * $Log: cfgedit_common.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/08/17 17:58:12  alanwags
 * header values for get color
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/02/19 17:56:41  endo
 * Experiment related modifications
 *
 * Revision 1.2  2006/01/10 06:21:59  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.29  2003/04/06 09:29:03  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.28  2003/04/06 07:46:46  endo
 * Checked in for Ananth. GoTo_Dstar and AtGoal pair added.
 *
 * Revision 1.27  2003/04/05 23:55:35  kaess
 * *** empty log message ***
 *
 * Revision 1.26  2000/11/13 21:06:54  blee
 * added default trigger for MoveToward_LM
 *
 * Revision 1.25  2000/11/13 20:51:31  blee
 * added default trigger for GoTo_LM
 *
 * Revision 1.24  2000/08/15 22:11:02  endo
 * GoThroughDoor matched with ThroughDoorway.
 *
 * Revision 1.23  2000/05/19 21:49:53  endo
 * AssistedGoTo and AtGoal added in matched_state_trigger[].
 *
 * Revision 1.22  2000/04/25 07:20:36  endo
 * More state-trigger matches were added.
 *
 * Revision 1.21  2000/04/16 15:42:39  endo
 * New pairs of state/trigger added.
 *
 * Revision 1.20  2000/03/30 01:17:39  endo
 * New object colors added. New pairs for state-trigger
 * added.
 *
 * Revision 1.19  2000/02/18 02:50:48  endo
 * Made FSA to match Alert - Alerted, and EnteRoom - InRoom.
 *
 * Revision 1.18  2000/01/22 20:05:38  endo
 * This modification will allow cfgedit FSA
 * to choose the defalut trigger based on
 * the state the trigger is originated
 * from. For example, the defalut trigger
 * of GoTo state will be AtGoal.
 *
 * Revision 1.17  1999/10/25 20:27:29  endo
 * rolled back to this version.
 *
 * Revision 1.15  1996/05/02 22:59:13  doug
 * *** empty log message ***
 *
 * Revision 1.14  1996/04/11  03:58:34  doug
 * *** empty log message ***
 *
 * Revision 1.13  1996/03/08  00:48:05  doug
 * *** empty log message ***
 *
 * Revision 1.12  1996/03/04  22:52:24  doug
 * *** empty log message ***
 *
 * Revision 1.11  1996/03/01  00:48:07  doug
 * *** empty log message ***
 *
 * Revision 1.10  1996/02/28  03:55:12  doug
 * *** empty log message ***
 *
 * Revision 1.9  1996/02/27  22:13:43  doug
 * *** empty log message ***
 *
 * Revision 1.8  1996/02/27  05:09:05  doug
 * *** empty log message ***
 *
 * Revision 1.7  1996/02/26  05:02:51  doug
 * *** empty log message ***
 *
 * Revision 1.6  1996/02/18  00:04:17  doug
 * *** empty log message ***
 *
 * Revision 1.5  1996/01/17  18:47:16  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/11/21  23:12:58  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/10/31  15:19:16  doug
 * moved symbol names here from symtab.h
 *
 * Revision 1.2  1995/10/30  22:28:06  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/10/30  22:26:25  doug
 * Initial revision
 *
 **********************************************************************/
