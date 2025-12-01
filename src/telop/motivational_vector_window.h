/**********************************************************************
 **                                                                  **
 **                  motivational_vector_window.h                    **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: motivational_vector_window.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef	_MOTIVATIONAL_VECTOR_WINDOW_INCLUDED
#define	_MOTIVATIONAL_VECTOR_WINDOW_INCLUDED


#include <stdio.h>
#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>

#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <X11/Shell.h>

//---------------------------------------------------------------------
//       Declarations of globals.
//---------------------------------------------------------------------

Widget	create_motivational_vector_window(void);

extern XtAppContext gMotivationalVectorAppContext;  
extern Widget gMotivationalVectorTopLevel;
extern Widget *gMotivationalVector_sliderWidget;
extern Display *gMotivationalVectorDisplay;
extern int gMotivationalVectorScreen;

#endif	/* _MOTIVATIONAL_VECTOR_WINDOW_INCLUDED */


/**********************************************************************
 * $Log: motivational_vector_window.h,v $
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
 * Revision 1.2  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.1  2000/02/29 21:22:13  saho
 * Initial revision
 *
 *
 **********************************************************************/
