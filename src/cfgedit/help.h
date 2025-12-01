/**********************************************************************
 **                                                                  **
 **                              help.h                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: help.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef HELP_H
#define HELP_H

// help page indices
typedef enum help_pages {

HELP_PAGE_copyright,
HELP_PAGE_new_design,
HELP_PAGE_save_as,
HELP_PAGE_exit,
HELP_PAGE_discard,
HELP_PAGE_start_over,
HELP_PAGE_select_file,
HELP_PAGE_overwrite_file,
HELP_PAGE_overwrite_workspace,
HELP_PAGE_unbind_arch,
HELP_PAGE_unbind_robot,
HELP_PAGE_bind_robot,
HELP_PAGE_choose_actuator_for_obp,
HELP_PAGE_choose_sensor_for_ibp,
HELP_PAGE_choose_robot_for_rbp,
HELP_PAGE_select_agent,
HELP_PAGE_exit_fsa,
HELP_PAGE_state_name,
HELP_PAGE_transition_name,
HELP_PAGE_split_nodes,

HELP_PAGE_index,
NUM_HELP_PAGES
} help_pages;



//------------------------------------------------------
void help_callback(Widget parent, void *ppage, void *cbs);
void index_callback(Widget parent, void *p, void *cbs);


void init_help(void);

#endif  


/**********************************************************************
 * $Log: help.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:33  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.14  2002/01/12 23:23:16  endo
 * Mission Expert functionality added.
 *
 * Revision 1.13  2000/03/14 00:02:39  endo
 * The "Start Over" button was added to CfgEdit.
 *
 * Revision 1.12  1997/02/14 16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.11  1995/11/04  23:46:50  doug
 * *** empty log message ***
 *
 * Revision 1.10  1995/06/29  18:26:16  jmc
 * Added copyright message.
 *
 * Revision 1.9  1995/06/05  23:03:16  doug
 * *** empty log message ***
 *
 * Revision 1.8  1995/05/03  14:57:41  doug
 * *** empty log message ***
 *
 * Revision 1.7  1995/03/07  14:53:49  doug
 * *** empty log message ***
 *
 * Revision 1.6  1995/02/28  14:52:08  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/02/09  21:32:43  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/02/08  20:34:01  doug
 * names working
 *
 * Revision 1.2  1995/02/07  21:49:33  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/01/26  00:02:31  doug
 * Initial revision
 *
 * Revision 1.3  1994/12/09  19:07:20  doug
 * *** empty log message ***
 *
 * Revision 1.3  1994/12/09  19:07:20  doug
 * *** empty log message ***
 *
 * Revision 1.2  1994/12/06  17:08:49  doug
 * *** empty log message ***
 *
 * Revision 1.2  1994/12/06  17:08:49  doug
 * *** empty log message ***
 *
 * Revision 1.1  1994/10/26  15:57:50  doug
 * Initial revision
 *
 **********************************************************************/
