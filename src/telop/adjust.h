/*---------------------------------------------------------------------------

	adjust.h

	Functions for handling the movement of slider-bars.
	Currently, there are only two slider-bars, but there
	will be more later.  At least one for each behavior.
	Also for handling the reset button and the commit button.

	Author: Khaled S. Ali

        Copyright 1995, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.

---------------------------------------------------------------------------*/

/* $Id: adjust.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef ADJUST_H
#define ADJUST_H

#include <Xm/Xm.h>

#include "shared.h"

extern int  adjust_mode;
extern int  sound_adjust_mode;
extern int  telop_mode;

/* Used by the Telop */
void  set_values(values_type values);
void  reset();
void commit(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg);

/* Used by the Sound Simulation routines */
void sound_set_values(sound_values_type values);
void sound_reset(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg);
void sound_commit(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg);

/* personality ane emotional routines */
void  adjust_personality(Widget w, XtPointer client_data, XmScaleCallbackStruct *call_data);
void  adjust_motivational_vector(Widget w, XtPointer client_data, XmScaleCallbackStruct *call_data);

#endif /* ADJUST_H */


/**********************************************************************
 * $Log: adjust.h,v $
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
 * Revision 1.6  2003/04/02 21:35:54  zkira
 * Added telop_mode
 *
 * Revision 1.5  2002/07/18 17:02:58  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.4  2000/03/01 00:04:37  saho
 * *** empty log message ***
 *
 *
 **********************************************************************/
