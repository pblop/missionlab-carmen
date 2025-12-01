/**********************************************************************
 **                                                                  **
 **                        gt_command_panel.h                        **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_command_panel.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $  */

#ifndef GT_COMMAND_PANEL_H
#define GT_COMMAND_PANEL_H

#include <X11/Intrinsic.h>
#include <string>


#include "gt_command.h"

using std::string;

enum gt_Step_status
{
   NO_STEP,
   EXECUTING_STEP,
   FAILED_STEP,
   ABORTED_STEP
};


void gt_create_command_panel( Widget parent );

void EnableCommandPanelPauseButton( bool bEnable );

void gt_popup_command_panel();

void gt_display_step( gt_Step* this_cmd_list,
                      gt_Step_status step_status,
                      gt_Step* next_cmd_list );


void set_command_panel_filename( char* filename );

void update_pause_button(void);
void toggle_pause_execution_button(void);

void update_feedback_button(void);

const string FEEDBACK_BUTTON_LABEL_STOP = "Cut-Off Feedback";
const string FEEDBACK_BUTTON_LABEL_NONSTOP = "Receive Feedback";

#endif

/**********************************************************************
 * $Log: gt_command_panel.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.16  2002/07/02 20:40:40  blee
 * declared EnableCommandPanelPauseButton()
 *
 * Revision 1.15  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.14  1996/02/29  01:53:18  doug
 * ..
 *
 * Revision 1.13  1995/05/18  21:15:35  jmc
 * Added an update_pause_button function prototype.
 *
 * Revision 1.12  1995/04/26  12:42:22  jmc
 * Added function prototype for set_command_panel_filename.
 * Removed global declarations of command_filename_field and
 * command_filename_field_touched, since these are both taken
 * care of by the new function.
 *
 * Revision 1.11  1995/04/03  19:32:00  jmc
 * Added copyright notice.
 *
 * Revision 1.10  1994/11/12  18:54:06  jmc
 * Removed obsolete NEXT_STEP from gt_Step_status.
 *
 * Revision 1.9  1994/11/12  18:11:15  jmc
 * Changed the function prototype of gt_display_step to include the
 * next step.
 *
 * Revision 1.8  1994/11/03  15:10:29  jmc
 * Added FAILED_STEP to gt_Step_status.
 *
 * Revision 1.7  1994/10/31  16:51:29  jmc
 * Added ABORTED_STEP to types of step status.
 *
 * Revision 1.6  1994/08/25  21:53:48  jmc
 * Added prototypes for command_filename* since they have to be
 * globally visible to set them in console.c.
 *
 * Revision 1.5  1994/07/28  22:08:15  jmc
 * Added gt_Step_status type and added a step_status to the function
 * prototype for gt_display_step.
 *
 * Revision 1.4  1994/07/12  19:14:48  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.3  1994/07/12  18:59:28  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:11:49  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
