/**********************************************************************
 **                                                                  **
 **                      sound_joystick.h                            **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Functions similar to telop to implement sound source simulation **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_joystick.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef SOUND_JOYSTICK_H
#define SOUND_JOYSTICK_H

#include <X11/Intrinsic.h>

extern double  TELOP_GAIN_MAX;

void sound_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg);
void sound_input(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg);
void sound_movement(Widget w, caddr_t client_data, XEvent call_data);

#endif




/**********************************************************************
 * $Log: sound_joystick.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.1  2000/02/29 21:35:38  saho
 * Initial revision
 *
 *
 **********************************************************************/
