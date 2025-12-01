/**********************************************************************
 **                                                                  **
 **                             callbacks.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: callbacks.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <Xm/Xm.h>
#include "glyph.hpp"
#include "circle.hpp"

typedef enum tf_types {TF_AGENT_NAME, TF_STATE_NAME, TF_INITIALIZER, 
		       TF_INDEX_NAME, TF_DESC};

typedef enum STYLE_TYPES {STYLE_CIRCLE, STYLE_TRIGGER, STYLE_GLYPH, 
			  STYLE_COMMAND_LIST};

struct move_down_cbs {
   STYLE_TYPES style;
   XtPointer ptr;
};

struct const_click {
   STYLE_TYPES style;
   class glyph  *g;
   class circle *c;
   int    slot;
};

struct tf_activated_cbs {
   char    **value;
   Widget    widget;
   Symbol   *symbol;
   tf_types  type;
};


// Externs for callbacks that invoke class methods.
void move_up_cb();
void move_glyph_cb(Widget w, XButtonEvent *event, String *args, int *num_args);

void tf_activated(Widget w, tf_activated_cbs *cbs);
void tf_gaining_focus(Widget w, int input_num);
void tf_loosing_focus(Widget w, int input_num);
void tf_verify(Widget w, tf_types data_type, XmTextVerifyCallbackStruct *cbs);


screen_object * select_behavior(int kind,const bool import);
Symbol * pick_behavior_for_delete(int kind);

void button_click(Widget w, int input_num);

void init_show_state(int x, int y, int r);

/* called to pushup a constant input */
void pushup_constant_cb(Widget w, const_click * data);

void kill_connection_cb(Widget w, int slot);
void add_new_group(void);
void add_slot_cb(Widget w, int slot);

void update_desc(Widget w, char **value);

// typedef void (*fp)(int x, int y);

// Called when one of the name visibility toggles changes on the configure menu
void
configure_nv_toggle_cb(Widget w, unsigned long architecture, 
        XmToggleButtonCallbackStruct *cbs);

// Called to convert a parameter into a pushed up parm
void push_parm(Symbol *rec, Symbol * parm);

/* called to pushup an FSA component parameter into the FSA record */
void pushup_FSA_component_parm(Symbol *fsa, Symbol *parm,bool change_name, Symbol *data_type, char *new_name);

#endif


/**********************************************************************
 * $Log: callbacks.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:32  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.37  2000/01/20 03:18:54  endo
 * Code checked in for doug. A feature for
 * cfgedit to allow toggling constants to
 * input allows was added.
 *
 * Revision 1.36  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.35  1996/10/04  20:58:17  doug
 * changes to get to version 1.0c
 *
 * Revision 1.37  1996/10/03 21:46:40  doug
 * nested FSA's are working
 *
 * Revision 1.36  1996/10/02 21:45:20  doug
 * working on pushup in states and transitions
 *
 * Revision 1.35  1996/10/01 13:00:11  doug
 * went to version 1.0c
 *
 * Revision 1.34  1996/06/02 16:25:59  doug
 * removed group_cb
 *
 * Revision 1.33  1996/05/02  22:58:26  doug
 * *** empty log message ***
 *
 * Revision 1.32  1996/02/19  21:57:05  doug
 * library components and permissions now work
 *
 * Revision 1.31  1996/02/06  18:16:42  doug
 * *** empty log message ***
 *
 * Revision 1.30  1996/02/02  03:00:09  doug
 * getting closer
 *
 * Revision 1.29  1996/02/01  04:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.28  1996/01/31  03:06:53  doug
 * *** empty log message ***
 *
 * Revision 1.27  1996/01/30  01:50:19  doug
 * *** empty log message ***
 *
 * Revision 1.26  1996/01/29  00:08:38  doug
 * *** empty log message ***
 *
 * Revision 1.25  1996/01/27  00:09:24  doug
 * added grouped lists of parameters
 *
 * Revision 1.24  1995/11/08  16:50:02  doug
 * *** empty log message ***
 *
 * Revision 1.23  1995/11/04  23:46:50  doug
 * *** empty log message ***
 *
 * Revision 1.22  1995/11/04  18:57:35  doug
 * *** empty log message ***
 *
 * Revision 1.21  1995/10/31  19:22:01  doug
 * handle text field changes in loose focus callback
 *
 * Revision 1.20  1995/09/26  21:42:27  doug
 * working on group_cb
 *
 * Revision 1.19  1995/09/15  22:02:20  doug
 * added support for the command list in the executive
 *
 * Revision 1.18  1995/09/07  14:22:26  doug
 * works
 *
 * Revision 1.17  1995/06/29  17:06:13  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
