/*--------------------------------------------------------------------------

	telop_joystick.h

	Functions to implement the joystick.

	Author: Khaled S. Ali

	Copyright 1995, Georgia Tech Research Corporation
	Atlanta, Georgia  30332-0415
	ALL RIGHTS RESERVED, See file COPYRIGHT for details.

--------------------------------------------------------------------------*/

/* $Id: telop_joystick.h,v 1.3 2009/01/24 02:40:11 zkira Exp $ */


#ifndef TELOP_JOYSTICK_H
#define TELOP_JOYSTICK_H

#include <X11/Intrinsic.h>

extern double  TELOP_GAIN_MAX;

void joy_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg);
void joy_input(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg);
void joy_stop(void);
void joy_movement(Widget w, caddr_t client_data, XEvent call_data);
void changed_avel_val(Widget w, XtPointer client_data, XtPointer call_data);
void increment_avel_val(int increment_amount);

void enable_joystick();  // zk trw
void disable_joystick(); // zk trw

void enable_telop();  // zk trw
void disable_telop(); 

void arm_deadman();
void disarm_deadman();

#endif


/**********************************************************************
 * $Log: telop_joystick.h,v $
 * Revision 1.3  2009/01/24 02:40:11  zkira
 * Aerial telop
 *
 * Revision 1.2  2008/10/27 21:21:49  zkira
 * Added 3D telop interface
 *
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/02 21:24:02  zkira
 * Added telop enable/disable and deadman enable/disable
 *
 * Revision 1.2  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.1  2000/02/29 23:56:03  saho
 * Initial revision
 *
 * Revision 1.2  1995/06/28 19:26:18  jmc
 * Added RCS id and log strings.
 **********************************************************************/
