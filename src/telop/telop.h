/**********************************************************************
 **                                                                  **
 **                         telop.h                                  **
 **                                                                  **
 **  Written by:  Khaled S.  Ali                                     **
 **                                                                  ** 
 **  All the functions that mlab needs to know about to create and   **
 **  start the Telop interface.                                      **
 **                                                                  ** 
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: telop.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef TELOP_H
#define TELOP_H

#include <X11/Intrinsic.h>

#include "shared.h"

extern int telop_num_robots;
extern int *telop_robot;
extern bool bTelopStart;
extern char *telop_unit_name;

void  gt_create_telop_interface(Widget top_level, XtAppContext app, char * personality_filename);

void  gt_popup_telop_interface(char *unit_name, int num_robots, int *robot_id);

void gt_popup_telop_personality(char *unit_name, int num_robots, int *robot_id);

void gt_end_teleoperation(int window);

#endif  /* TELOP_H */

/**********************************************************************
 * $Log: telop.h,v $
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
 * Revision 1.14  2003/04/02 21:28:21  zkira
 * Added bTelopStart for telop interface
 *
 * Revision 1.13  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.12  2001/12/22 16:14:29  endo
 * RH 7.1 porting.
 *
 * Revision 1.11  2000/06/13 17:03:45  endo
 * extern void  gt_popup_telop_personality();
 *
 * Revision 1.10  2000/02/29 22:08:05  saho
 * Updated file header for consistency with other files.
 *
 * Revision 1.9  1995/07/06 17:23:41  kali
 * *** empty log message ***
 *
 * Revision 1.8  1995/07/06  15:43:45  kali
 * *** empty log message ***
 *
 * Revision 1.7  1995/06/27  15:55:41  kali
 * added prototype for gt_popup_telop_personality
 *
 * Revision 1.6  1995/06/05  19:05:04  kali
 * included shared.h and added externs for num_sliders and sliders[]
 *
 * Revision 1.5  1995/05/08  19:53:40  jmc
 * Tweaked the comments.
 *
 * Revision 1.4  1995/05/05  20:06:25  kali
 * removed ascii style function prototypes so that UIMX would accept it
 *
 * Revision 1.3  1995/05/03  21:54:42  jmc
 * Added function prototype for gt_end_teleoperation().
 *
 * Revision 1.2  1995/05/03  19:09:43  jmc
 * Added arguments to interface functions.  Declared unit-related
 * global variables.  Added RCS strings.
 **********************************************************************/
