/**********************************************************************
 **                                                                  **
 **                            gt_world.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_world.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef GT_WORLD_H
#define GT_WORLD_H

#include <X11/Intrinsic.h>

#include "gt_simulation.h"

void gt_randomize_seed( int update_display );

void clear_obstacles();
void create_obstacles( int force );

void FillRegion( double dStartX,    double dStartY, 
                 double dWidth,     double dHeight, 
                 double dMinObsRad, double dMaxObsRad, 
                 double dCoverage,  int iSeed );

void ClearRegion( double dStartX, double dStartY, 
                  double dWidth,  double dHeight );

void RemoveObsAt( double dX, double dY );

void gt_create_obstacle_panel( Widget w );
void gt_popup_obstacle_panel();

int gt_add_obstacle( double x, double y, double radius, double height = 1 );
int overlap_obstacle( double x, double y );

int set_obstacle_coverage( double coverage );
int set_min_obstacle_radius( double radius );
int set_max_obstacle_radius( double radius );

extern double origin_x, origin_y, origin_heading;  // meters
extern double g_ref_longitude, g_ref_latitude; 
extern float map_scale; 
extern double map_width, map_height; 

void recompute_obstacle_radius_range();

#endif  // GT_WORLD_H


/**********************************************************************
 * $Log: gt_world.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.3  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.2  2006/09/20 18:34:48  nadeem
 * Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/03/23 07:36:56  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.39  2002/04/04 18:42:54  blee
 * Added FillRegion(), ClearRegion(), RemoveObsAt().
 *
 * Revision 1.38  2000/04/25 08:56:12  jdiaz
 * added origin_heading variable
 *
 * Revision 1.37  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.36  1995/05/04  22:08:27  jmc
 * Moved all scale related stuff to gt_scale.*.
 *
 * Revision 1.35  1995/05/04  20:08:09  jmc
 * Made most of the set_* functions return a boolean indicating
 * whether thy executed properly.  Deleted several #defines since
 * these constants are now only needed in gt_world.h (because the
 * set_* functions use them).  Added function prototypes for
 * recompute_obstacle_radius_range() and recompute_robot_length().
 *
 * Revision 1.34  1995/05/02  21:31:55  jmc
 * Removed set_meters_per_pixel (moved to console.h).
 * Added update_mpp_scale function.
 *
 * Revision 1.33  1995/04/27  19:24:55  jmc
 * Changed function prototype for set_meters_per_pixel to return a
 * boolean indicating successful completion.  removed
 * MIN/MAX_METERS_PER_PIXEL and put them in console.h as
 * min_meters_per_pixel and max_meters_per_pixel.
 *
 * Revision 1.32  1995/04/19  21:00:58  jmc
 * Removed min/max_obs_r since these are not needed anywhere except
 * gt_world.c.
 *
 * Revision 1.31  1995/04/19  20:18:31  jmc
 * Moved declarations of min/max_obs_r from gt_sim.h to here since
 * they are defined in gt_world.c.
 *
 * Revision 1.30  1995/04/14  15:27:41  jmc
 * Added function prototopes for clear_obstacles and create_obstacles
 * (were formerly clear_world and populate_world in gt_sim.h).
 *
 * Revision 1.29  1995/04/14  14:26:33  jmc
 * Renamed configuration_panel functions to obstacle_panel functions.
 *
 * Revision 1.28  1995/04/10  16:32:19  jmc
 * Removed declarations for drawing functions and moved them to
 * draw.h.  Also cleaned up order of include for Intrinsic.h to
 * avoid redefinition warnings.
 *
 * Revision 1.27  1995/04/05  21:44:19  jmc
 * Added a function prototype for draw_starting_point.
 *
 * Revision 1.26  1995/04/03  20:36:50  jmc
 * Added copyright notice.
 *
 * Revision 1.25  1995/03/08  14:38:26  jmc
 * Removed GAP_NAME and PASSAGE_NAME since there are better ways now
 * to control whether the label is printed.
 *
 * Revision 1.24  1995/03/03  21:05:54  jmc
 * Added a function declaration for draw_mission_name.
 *
 * Revision 1.23  1995/02/28  16:37:01  doug
 * removed DrawCircle extern
 *
 * Revision 1.22  1995/02/14  22:06:40  jmc
 * Removed extern declaration for gt_write_names.  This variable was
 * deleted because a more flexible way of controlling printing of
 * names was implemented.
 *
 * Revision 1.21  1995/01/11  21:17:49  doug
 * added new show vector option
 *
 * Revision 1.20  1994/11/10  22:44:18  jmc
 * Added min/max robot_magnification and function prototypes for
 * setting "magnify_robots" and "user_robot_magnification".
 *
 * Revision 1.19  1994/11/09  21:05:03  doug
 * support fixed and scaled robot sizes
 *
 * Revision 1.18  1994/11/08  22:58:10  jmc
 * Added function prototypes for zoom_in and zoom_out.
 *
 * Revision 1.17  1994/11/08  16:01:38  doug
 * added drawimpact fnc
 *
 * Revision 1.16  1994/11/07  21:33:37  jmc
 * Added a function prototype for overlap_obstacle.
 *
 * Revision 1.15  1994/11/07  19:10:50  jmc
 * Added declarations for origin_x and origin_y.
 *
 * Revision 1.14  1994/11/07  17:24:23  jmc
 * Added a function prototype for gt_add_obstacle.
 *
 * Revision 1.13  1994/11/02  23:46:23  jmc
 * Added prototype for gt_randomize_seed.
 *
 * Revision 1.12  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.11  1994/10/31  17:22:48  jmc
 * Added function prototypes for the scale modification panel.
 *
 * Revision 1.10  1994/10/28  18:36:39  jmc
 * Changed MIN_METERS_PER_PIXEL to 1.0 (from 0.5) to avoid rescaling
 * requiring too much memory.
 *
 * Revision 1.9  1994/10/28  00:32:53  jmc
 * Added function prototypes for all the draw_* functions which are
 * based on the data in the measure.
 *
 * Revision 1.8  1994/10/27  18:58:23  jmc
 * Added constants for the legal values of the variables controlled
 * by sliders.
 *
 * Revision 1.7  1994/10/25  20:24:12  jmc
 * Added radius argument to DrawPolygon for the radius in the case
 * when a polyline isn't specified and a circle is drawn.
 *
 * Revision 1.6  1994/10/25  15:32:54  jmc
 * Minor readability hack.
 *
 * Revision 1.5  1994/07/18  20:46:56  jmc
 * Added include X11/Intrinsic for Widget declaration.
 *
 * Revision 1.4  1994/07/18  19:41:53  jmc
 * Renamed configuration_popup to configuration_panel and added
 * declaration for gt_popup_configuration_panel.
 *
 * Revision 1.3  1994/07/18  19:35:43  jmc
 * Moved gt_configuration_popup declaration here and renamed it to
 * gt_create_configuration_popup.
 *
 * Revision 1.2  1994/07/14  13:27:07  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
