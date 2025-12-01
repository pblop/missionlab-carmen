/**********************************************************************
 **                                                                  **
 **                     personality_window.h                         **
 **                                                                  **
 **  Written by:  Khaled S.  Ali                                     **
 **                                                                  ** 
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: personality_window.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef	_PERSONALITY_WINDOW_INCLUDED
#define	_PERSONALITY_WINDOW_INCLUDED


#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>

#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

#include "shared.h"

//---------------------------------------------------------------------
//       Declarations of globals
//---------------------------------------------------------------------

Widget create_personality_window(Widget parent);

extern int gPersonalityWindow_numPersonalities;
extern personality_type gPersonalityWindow_personalities[];

#endif	/* _PERSONALITY_WINDOW_INCLUDED */


/**********************************************************************
 * $Log: personality_window.h,v $
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
 * Revision 1.12  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.11  2000/02/29 22:23:43  saho
 * Updated the header.
 *
 *
 **********************************************************************/
