/**********************************************************************
 **                                                                  **
 **                      status_window.h                             **
 **                                                                  **
 **  Written by:  Jung-Hoon Hwang                                    **
 **                                                                  **
 **  Copyright 2003, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: status_window.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef	_STATUS_WINDOW_INCLUDED
#define	_STATUS_WINDOW_INCLUDED

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

struct SensorReading;

/*******************************************************************************
       Declarations of global functions.
*******************************************************************************/

Widget	create_main_status_window( swidget _UxUxParent );
void status_report_sensors( int robot_id, int cnt, SensorReading* reading);
void UpdateTargetDisplay();

void status_disarm_deadman();
void status_arm_deadman();

#endif	/* _MAIN_WINDOW_INCLUDED */


/**********************************************************************
 * $Log: status_window.h,v $
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
 * Revision 1.1  2003/04/02 21:16:26  zkira
 * Initial revision
 *
 **********************************************************************/
