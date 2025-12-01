/**********************************************************************
 **                                                                  **
 **                           waypoint.h                             **
 **                                                                  **
 **  Written by:  Yoichiro Endo and Jonathan F. Diaz                 **
 **                                                                  **
 **  Copyright 2000 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: waypoint.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef WAYPOINT_H
#define WAYPOINT_H

extern char *waypoints_filename; 
extern char *ovl_filename;
extern point2d_t *waypts_ary;
extern int giNumWayPoints;
extern int giWayptArySize;

void add_waypoint(float x, float y);
void delete_waypoint(float x, float y);
void return_waypoints(void);

void ask_save_ovl(void);
void create_save_ovl_as_dialog(Widget parent);

#endif

/**********************************************************************
 * $Log: waypoint.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:11  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/07/07 18:27:34  endo
 * Initial revision
 *
 **********************************************************************/
