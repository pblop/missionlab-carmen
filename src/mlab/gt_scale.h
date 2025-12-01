/**********************************************************************
 **                                                                  **
 **                            gt_scale.h                            **
 **                                                                  **
 **           Functions related to the scale of the drawing.         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **               Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_scale.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_SCALE_H
#define GT_SCALE_H

/* Scale related declarations */

extern double meters_per_pixel;
void set_meters_per_pixel(double mpp);

extern double zoom_factor;
int set_zoom_factor(double new_zoom);
void zoom_in();
void zoom_out();

extern int scale_robots;
void set_scale_robots(int flag);

extern double robot_length_meters;
extern double robot_length_pixels;
int set_robot_length(double len);
void recompute_robot_length();


void gt_create_scale_panel(Widget w);
void gt_popup_scale_panel();

extern int map_scale_factor;


#endif  /* GT_SCALE_H */


/**********************************************************************
 * $Log: gt_scale.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/08/10 04:41:06  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.2  1995/05/06  21:45:19  jmc
 * Added declarations for zoom-related data and functions.
 * Reorganized.
 *
 * Revision 1.1  1995/05/05  14:12:13  jmc
 * Initial revision
 **********************************************************************/
