/**********************************************************************
 **                                                                  **
 **                            globals.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: globals.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef GLOBALS_H
#define GLOBALS_H

#include <string>

#include "load_rc.h"
#include "symtab.h"

using std::string;

/************************************************************************/

// The current configuration global
extern class configuration *config;
extern int debug_load_cdl;
extern bool debug_save;
extern bool special_rc_file;
extern const char *rc_filename;
extern bool backup_files;
extern bool hide_FSA_detail;
extern bool gAutomaticExecution;

extern XtAppContext app;

typedef struct CfgEditGCTypes_t
{
    GC black;
    GC XOR;
    GC erase;
    GC select;
    GC transition;
    GC glyph;
    GC state;
    GC highlight;
};
extern CfgEditGCTypes_t gCfgEditGCs;

typedef struct CfgEditPixelTypes_t
{
    Pixel select;
    Pixel insensitive;
    Pixel glyphBg;
    Pixel glyphFg;
    Pixel transitionBg;
    Pixel transitionFg;
    Pixel stateBg;
    Pixel stateFg;
    Pixel XORBg;
    Pixel XORFg;
    Pixel black;
    Pixel blue;
    Pixel red;
    Pixel yellow;
    Pixel green;
    Pixel smoke;
    Pixel plum;
    Pixel darkseagreen2;
    Pixel snow4;
    Pixel white;
    Pixel khaki;
    Pixel darkslateblue;
    Pixel gray95;
    Pixel gray90;
    Pixel gray85;
    Pixel gray80;
    Pixel gray75;
    Pixel gray70;
    Pixel gray65;
    Pixel gray60;
    Pixel gray55;
    Pixel gray50;
    Pixel gray45;
    Pixel gray40;
    Pixel gray35;
    Pixel gray30;
    Pixel gray25;
    Pixel gray20;
    Pixel gray15;
    Pixel gray10;
    Pixel gray5;
};
extern CfgEditPixelTypes_t gCfgEditPixels;

extern XColor black_color, white_color;          

extern XmFontList gCfgEditFontList;
extern Font gCfgEditDefaultFont;
extern string gFONTTAG_CLEAN_BOLD_14;
extern string gFONTTAG_CLEAN_BOLD_12;
extern string gFONTTAG_CLEAN_MEDIUM_12;

static const int INIT_COLS = 8;
static const int NAME_COLS = 24;
static const int COL_SEP = 5;

// Max number of robot files supported in one configuration
const int MAX_ROBOTS = 64;

// These globals are used in the run command
extern bool have_fresh_make;
extern char *robot_names[MAX_ROBOTS];

extern symbol_table<rc_chain> rc_table;

typedef enum EDIT_STATES {ES_NORMAL, ES_LINK_OBJECT, ES_MOVE_OBJECT};
extern EDIT_STATES edit_state;

extern SymbolList loaded_libraries;

extern Widget main_window;
extern Widget scrolled_window;

extern int gDisplayHeight;
extern int gDisplayWidth;

extern bool gDisableImportSymbol;



/****************** The privilege stuff ****************/
typedef unsigned char USER_PRIVILEGES;
extern USER_PRIVILEGES user_privileges;

// Need EXECUTE_PRIV in order to run a configuration 
#define EXECUTE_PRIV 	(1 << 0)

// Need MODIFY_PRIV in order to be able to modify parameters in a configuration
#define MODIFY_PRIV     (1 << 1)

// Need EDIT_PRIV in order to be able to create or modify configurations
// using componts existing in the libraries
#define EDIT_PRIV	(1 << 2)

// Need CREATE_PRIV in order to be able to create new components
#define  CREATE_PRIV 	(1 << 3)

// Need LIBRARY_PRIV in order to be able to modify library components
#define LIBRARY_PRIV 	(1 << 4)

// Need REALROBOTS_PRIV in order to be able to run real robots
#define REALROBOTS_PRIV (1 << 5)

// Need MEXP_MODIFY_CBRLIB_PRIV in order to be able to modify the library of 
// the CBRServer.
#define MEXP_MODIFY_CBRLIB_PRIV (1 << 6)

/*******************************************************/

template<class T> T sqr(T x) { return x * x; }
template<class T> T max(T a, T b) { return a>b?a:b; }
template<class T> T min(T a, T b) { return a<b?a:b; }

#endif


/**********************************************************************
 * $Log: globals.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.7  2007/07/17 21:53:51  endo
 * Meta data sorting function added.
 *
 * Revision 1.6  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 * Revision 1.5  2006/09/26 18:30:27  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.4  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.3  2006/09/18 18:35:11  alanwags
 * cim_send_message() implemented.
 *
 * Revision 1.2  2006/09/13 19:03:48  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/05/15 01:23:28  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.3  2005/08/09 19:12:44  endo
 * Things improved for the August demo.
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:33  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.33  2002/01/31 10:44:20  endo
 * Parameters chaged due to the change of max_vel and base_vel.
 *
 * Revision 1.32  2002/01/12 23:23:16  endo
 * Mission Expert functionality added.
 *
 * Revision 1.31  2001/09/14 01:28:20  ebeowulf
 * Added the display size to the list of global variables
 *
 * Revision 1.30  1999/12/18 10:10:34  endo
 * extern bool hide_FSA_detail added.
 *
 * Revision 1.29  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.28  1996/10/04  20:58:17  doug
 * changes to get to version 1.0c
 *
 * Revision 1.28  1996/10/01 13:00:11  doug
 * went to version 1.0c
 *
 * Revision 1.27  1996/03/06 23:39:17  doug
 * *** empty log message ***
 *
 * Revision 1.26  1996/02/27  05:01:12  doug
 * *** empty log message ***
 *
 * Revision 1.25  1996/02/25  01:13:14  doug
 * *** empty log message ***
 *
 * Revision 1.24  1996/02/22  00:55:17  doug
 * adding popups to fsa states/trans
 *
 * Revision 1.23  1996/02/20  22:52:24  doug
 * adding EditParms
 *
 * Revision 1.22  1996/02/18  00:03:13  doug
 * binding is working better
 *
 **********************************************************************/
