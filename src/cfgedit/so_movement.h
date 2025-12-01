/**********************************************************************
 **                                                                  **
 **                           so_movement.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: so_movement.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef SO_MOVEMENT_H
#define SO_MOVEMENT_H

void connect_object_to_page(Symbol *p);
void place_object(screen_object *s);
void button_down_on_object(screen_object *s, Position x, Position y);
void button_down_on_background(Position x, Position y);
void button_up(Position x, Position y);
void mouse_movement_with_button_down(Position x, Position y);
void mouse_movement_with_button_up(Position x, Position y);
void start_rubber_band(screen_object *s, Position x, Position y);
void end_rubber_band(void);
void remove_grabs(void);
void abort_place_obj(void);




// Callback routine for mouse input
void da_mouse_cb(Widget w, XButtonEvent * event, String * args, int *num_args);

// Called on expose events to redraw the screen
void refresh_screen(Widget w,
               XtPointer no_client_data,
	       XmDrawingAreaCallbackStruct *callback_data);

// Contains the name of the trigger the user is about to create.
extern char *this_trigger;

#endif


/**********************************************************************
 * $Log: so_movement.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:34  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.6  2000/01/22 20:18:44  endo
 * extern char *this_trigger; added.
 * This modification will allow cfgedit FSA
 * to choose the default trigger based on
 * the state the trigger is originated
 * from. For example, the default trigger
 * of GoTo state will be AtGoal.
 *
 * Revision 1.5  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.4  1996/02/29  01:48:49  doug
 * *** empty log message ***
 *
 * Revision 1.3  1996/02/06  18:16:42  doug
 * *** empty log message ***
 *
 * Revision 1.2  1996/02/02  03:00:09  doug
 * getting closer
 *
 * Revision 1.1  1996/01/30  01:50:19  doug
 * Initial revision
 *
 **********************************************************************/
