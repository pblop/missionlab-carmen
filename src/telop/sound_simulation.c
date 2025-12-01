/**********************************************************************
 **                                                                  **
 **                      sound_simulation.c                          **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_simulation.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <malloc.h>

#include "UxXt.h"
#include "gt_com.h"
#include "shared.h"
#include "sound_simulation.h"
#include "sound_simulation_window.h"

XtAppContext gSoundSimulationAppContext;  
Widget gSoundSimulationTopLevel;
Display *gSoundSimulationDisplay = NULL;
int gSoundSimulationScreen;

int  sound_up = 0;

Widget sound_simulation_interface;

void  gt_create_sound_simulation_interface(Widget top_level, XtAppContext app)
{

    gSoundSimulationTopLevel = top_level;
    gSoundSimulationAppContext = app;  
    gSoundSimulationDisplay = XtDisplay(gSoundSimulationTopLevel);
    gSoundSimulationScreen = XDefaultScreen(gSoundSimulationDisplay);

    sound_simulation_interface = create_sound_simulation_window(NULL);
}



void gt_popup_sound_simulation_interface(void)
{

    //Pop up the interface
    XtPopup(sound_simulation_interface, XtGrabNone);
    sound_up = 1;
}




void gt_end_sound_simulation(int  window)
{
    if(window == 1) sound_up=0;
}

/**********************************************************************
 * $Log: sound_simulation.c,v $
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
 * Revision 1.1  2000/02/29 21:28:44  saho
 * Initial revision
 *
 *
 **********************************************************************/

