/**********************************************************************
 **                                                                  **
 **                      sound_simulation_window.c                   **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_simulation_window.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */


#ifndef	_SOUND_SIMULATION_WINDOW_INCLUDED
#define	_SOUND_SIMULATION_WINDOW_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/DrawingA.h>
#include <X11/Shell.h>

extern Widget	main_telop_window;
extern Widget	commit_button;
extern Widget	immediate_toggle;
extern Widget	delayed_toggle;
extern Widget	unitname_labelGadget;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_sound_simulation_window( swidget _UxUxParent );

#endif	/* _MAIN_WINDOW_INCLUDED */


/**********************************************************************
 * $Log: sound_simulation_window.h,v $
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
 * Revision 1.1  2000/02/29 21:31:58  saho
 * Initial revision
 *
 *
 **********************************************************************/
