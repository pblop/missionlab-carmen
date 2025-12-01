/**********************************************************************
 **                                                                  **
 **                              draw.h                              **
 **                                                                  **
 **           Drawing functions for the main drawing area.           **
 **           (Based on gt_world.h v1.27.)                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **               Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: draw.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef DRAW_H
#define DRAW_H

#include "gt_measure.h"
#include "gt_map.h"



// Drawing related declarations
static const int DRAW_TEXT_VERTICAL_OFFSET = 12;

void DrawPath( double px, double py, double ph, double x, double y,
               double h, GC gc, int erase );
void DrawHalo( double x, double y, double r );
void DrawImpact( double x, double y, double r );
void DrawLine( double x1, double y1, double x2, double y2 );
void DrawLine(double x1, double y1, double x2, double y2, GC thisGC);
void DrawFilledCircle( double x, double y, double r, GC thisGC );
void DrawText(double x, double y, char* text);
void DrawText(double x, double y, GC gc, char* text);
void DrawVector( Vector c, Vector v, bool bDrawArrowHead = false );
void DrawCircle(double x, double y, double r, GC thisGC);

void draw_polygon( gt_Measure* measure );
void draw_polyline( gt_Measure* measure );
void draw_label( char* name, gt_Measure* measure );

void draw_mission_name( char* name );

void draw_assembly_area( char* name, gt_Measure* measure );
void draw_attack_position( char* name, gt_Measure* measure );
void draw_axis( char* name, gt_Measure* measure );
void draw_battle_position( char* name, gt_Measure* measure );
void draw_boundary( char* name, gt_Measure* measure );
void draw_wall( char* name, gt_Measure* measure );
void draw_door( char* name, gt_Measure* measure );
void draw_room( char* name, gt_Measure* measure );
void draw_hallway( char* name, gt_Measure* measure );

void draw_gap( char* name, gt_Measure* measure );
void draw_LDLC( char* name, gt_Measure* measure );
void draw_objective( char* name, gt_Measure* measure );
void draw_passage_point( char* name, gt_Measure* measure );
void draw_phase_line( char* name, gt_Measure* measure );
void draw_starting_point( char* name, gt_Measure* measure );

void draw_world();

void draw_photo( Map_info* map );

extern int gt_show_trails;
extern int gt_circle_robots;

#endif  // DRAW_H


/**********************************************************************
 * $Log: draw.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.2  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2005/07/27 20:38:09  endo
 * 3D visualization improved.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.10  2002/07/02 20:48:00  blee
 * changed DrawVector() and added DrawArrowHead()
 *
 * Revision 1.9  2000/02/07 05:16:26  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.8  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.7  1996/03/04  22:51:58  doug
 * *** empty log message ***
 *
 * Revision 1.6  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.5  1995/06/09  21:41:02  jmc
 * Added draw_photo function declaration.
 *
 * Revision 1.4  1995/05/04  22:05:50  jmc
 * Added a declaration for gt_show_trails (moved here from
 * gt_world.h).
 *
 * Revision 1.3  1995/04/10  17:09:12  jmc
 * Fixed the RCS ID string.
 *
 * Revision 1.2  1995/04/10  17:08:24  jmc
 * Added function prototype for draw_world().
 *
 * Revision 1.1  1995/04/07  20:06:02  jmc
 * Initial revision
 **********************************************************************/
