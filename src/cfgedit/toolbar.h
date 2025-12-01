/**********************************************************************
 **                                                                  **
 **                             toolbar.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: toolbar.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef TOOLBAR_H
#define TOOLBAR_H

typedef enum SYSTEM_MODE { 
    MOVE_MODE, START_CONNECT_TRANSITION, CONNECTING_TRANSITION, 
    PLACE_STATE, EDIT_MODE, MOVE_UP, CUT, COPY, PASTE, DELETE, LINK,
    NEW_PRIMITIVE, NEW_AGENT, NEW_RBP, NEW_OBP, NEW_IBP, NEW_OPERATOR,
    MAKING_CONNECTION, PLACEMENT, DRAGGING, PUSHUP, DUPLICATE, UNDO,
    CVT_INPUT, IMPORT_LIB_FNC, NEW_TRIGGER, EXPAND_SUBTREES, GROUP_COMPONENT,
    ADD_TO_LIB_FNC, DELETE_FROM_LIB_FNC, WAYPOINT_DESIGN, START_OVER, 
    PLANNER_DESIGN, MEXP_NEW_MISSION, MEXP_VIEW_MAP_MISSION,
    MEXP_ADD_MISSION_TO_CBRLIB, MEXP_REPLAY_MISSION
};

extern SYSTEM_MODE system_mode;

/*-----------------------------------------------------------------------*/
// Object list
extern GTList<class screen_object *> objs;
inline int  num_objs() { return objs.len(); }
inline void remove_obj(screen_object *s) { objs.remove(s); }
inline void add_obj(screen_object *s) { objs.append(s); }

/*-----------------------------------------------------------------------*/
// Manage the selection list
extern Symbol *selected;       // The symbol selected on page

void save_cdl_for_replay(const char* event); 

bool are_selections();
bool is_selected(screen_object * s);
bool is_selected(Symbol *rec);
// Mark the object as selected, if possible.
// returns true if object is selected, false otherwise.
bool select(screen_object * s);

void unselect();
void forget_selections();

// Called to prompt and maybe delete the selected symbols
bool can_paste(bool show_warnings);
void dump_blackboard();

void objects_handle_events();
void objects_bypass_events();
void cancel_cb();

extern screen_object *cutTaskExitedTransition(screen_object *task_star);
extern void TaskExited_design(screen_object *task_star, Symbol *cur);
extern void updateTriggerFieldAutomatically(screen_object *t, screen_object *s);
/*-----------------------------------------------------------------------*/
// Manage the mode buttons
void set_mode(Widget w, SYSTEM_MODE new_mode);

/*-----------------------------------------------------------------------*/
//extern char *gWaypointOverlayName;

#endif


/**********************************************************************
 * $Log: toolbar.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/09/07 23:10:04  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 * Revision 1.2  2006/07/26 18:07:47  endo
 * ACDLPlus class added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:34  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.33  2002/05/06 17:59:14  endo
 * "Delete" functionality added in the edit menu.
 *
 * Revision 1.32  2002/01/12 23:23:16  endo
 * Mission Expert functionality added.
 *
 * Revision 1.31  2000/06/25 03:05:19  endo
 * updateTriggerFieldAutomatically added.
 *
 * Revision 1.30  2000/04/25 07:26:19  endo
 * TaskExited_design and its related functions added.
 *
 * Revision 1.29  2000/04/13 21:49:42  endo
 * Checked in for Doug.
 * This patch extends MissionLab to allow the user to import read-only
 * library code.  When the user attempts to change something which is
 * read-only, cfgedit pops up a dialog box and asks if it should import
 * the object so it can be edited.  If OK'd, it imports the object.
 *
 * This fixes the problem with FSA's (and other assemblages) not being
 * editable when they are loaded from the library.
 *
 * Revision 1.28  2000/03/31 17:04:50  endo
 * reset_waypoint_positioning() deleted since giNumWayPoints
 * was replaced with getNumberOfStatesInFSA.
 *
 * Revision 1.27  2000/03/23 20:14:38  endo
 * Made cfgedit to turn off or on path_planner based on
 * the flag specified in .cfgeditrc.
 *
 * Revision 1.26  2000/03/22 02:12:42  saho
 * Added code for the new Planner button.
 *
 * Revision 1.25  2000/03/20 20:37:33  endo
 * reset_waypoint_positioning() added.
 *
 * Revision 1.24  2000/03/13 23:58:35  endo
 * The "Start Over" button was added to CfgEdit.
 *
 * Revision 1.23  2000/02/13 20:46:47  sapan
 * *** empty log message ***
 *
 * Revision 1.22  2000/02/02 23:42:08  jdiaz
 * waypoints support
 *
 * Revision 1.21  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.20  1996/10/04  20:58:17  doug
 * changes to get to version 1.0c
 *
 * Revision 1.20  1996/10/01 13:00:11  doug
 * went to version 1.0c
 *
 * Revision 1.19  1996/06/02 16:25:59  doug
 * added ability to group nodes
 *
 * Revision 1.18  1996/03/05  22:55:37  doug
 * *** empty log message ***
 *
 * Revision 1.17  1996/02/28  03:56:24  doug
 * *** empty log message ***
 *
 * Revision 1.16  1996/02/26  05:01:33  doug
 * *** empty log message ***
 *
 * Revision 1.15  1996/02/20  22:52:24  doug
 * adding EditParms
 *
 * Revision 1.14  1996/02/20  20:00:22  doug
 * *** empty log message ***
 *
 * Revision 1.13  1996/02/19  21:57:05  doug
 * library components and permissions now work
 *
 * Revision 1.12  1996/02/16  00:07:18  doug
 * *** empty log message ***
 *
 * Revision 1.11  1996/02/08  19:21:56  doug
 * *** empty log message ***
 *
 * Revision 1.10  1996/02/06  18:16:42  doug
 * *** empty log message ***
 *
 * Revision 1.9  1996/02/04  23:21:52  doug
 * *** empty log message ***
 *
 * Revision 1.8  1996/02/04  17:11:48  doug
 * *** empty log message ***
 *
 * Revision 1.7  1996/02/02  03:00:09  doug
 * getting closer
 *
 * Revision 1.6  1996/02/01  04:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.5  1996/01/31  03:06:53  doug
 * *** empty log message ***
 *
 * Revision 1.4  1996/01/30  01:50:19  doug
 * *** empty log message ***
 *
 * Revision 1.3  1996/01/29  00:08:38  doug
 * *** empty log message ***
 *
 * Revision 1.2  1996/01/27  00:09:24  doug
 * added grouped lists of parameters
 *
 * Revision 1.1  1996/01/21  20:12:39  doug
 * Initial revision
 *
 **********************************************************************/
