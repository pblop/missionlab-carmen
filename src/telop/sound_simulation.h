/**********************************************************************
 **                                                                  **
 **                      sound_simulation.h                          **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_simulation.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef _SOUND_SIMULATION_
#define _SOUND_SIMULATION_

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#include "shared.h"

void  gt_create_sound_simulation_interface(Widget top_level, XtAppContext app);
void  gt_popup_sound_simulation_interface(void);
void  gt_end_sound_simulation(int window);

extern XtAppContext gSoundSimulationAppContext;  
extern Widget gSoundSimulationTopLevel;
extern Display *gSoundSimulationDisplay;
extern int gSoundSimulationScreen;

#endif /* _SOUND_SIMULATION_ */

/**********************************************************************
 * $Log: sound_simulation.h,v $
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
 * Revision 1.3  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.2  2001/12/22 16:14:29  endo
 * RH 7.1 porting.
 *
 * Revision 1.1  2000/02/29 21:29:06  saho
 * Initial revision
 *
 *
 **********************************************************************/
