/**********************************************************************
 **                                                                  **
 **                        convert_scale.h                           **
 **                                                                  **
 **  Written by: Nadeem Ahmed Syed                                   **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

#ifndef SCALE_CONVERT_H
#define SCALE_CONVERT_H

#include <math.h>

//#define PI 3.14159265

/* All the methods in this file (as of 09/19/06) assume that the following 
   variables are defined and accessible somewhere

   Currently, for the mlab executable these variables are defined in gt_world.c
   And for the robot executables, these are defined in robot_side_comm.c
*/

extern float map_scale; // modified pixel_scale to do computations in meters.
extern float map_scale_pixels; // Scale used by openmap to generate the overlay map.
extern double map_width;  // default width of the overlay image in meters
extern double map_height; // default height of the overlay image in meters
extern double map_width_pixels;  // default width of the overlay image in pixels
extern double map_height_pixels; // default height of the overlay image in pixels
extern double g_ref_longitude;  // center of the overlay map
extern double g_ref_latitude;   // center of the overlay map
extern int pixelsPerMeter;  //  PPM. From OpenMap Planet.java defaultPixelsPerMeter

/* Bunch of simple math functions below to help with the computations */
double my_asinh(double x);
double my_sinh(double x);
double degToRad(double deg); 
double radToDeg(double rad); 



/* This method take the target location in longitude and latitude format 
   and returns the equivalent X, Y position. The return values are stored
   in the last 2 parameters
*/
void geographic2global(
    double target_lat,
    double target_lon,
    double *ptarget_X,
    double *ptarget_Y);

/* This method take the target location in longitude and latitude format 
   and returns the equivalent X, Y position in pixels. The return values 
   are storedin the last 2 parameters
*/
void geographic2pixel(
    double target_lat, 
    double target_lon,
    double *ptarget_X,
    double *ptarget_Y);

/* This method take the target location in X and Y format and returns the 
   equivalent longitude, latitude position. The return values are stored
   in the last 2 parameters.
*/
void global2geographic(
    double target_x,
    double target_y, 
    double *ptarget_lat,
    double *ptarget_lon);


/* This method take the target location in X and Y pixel position and returns 
   the equivalent longitude, latitude position. The return values are stored
   in the last 2 parameters.
*/
void pixel2geographic(
    double target_x,
    double target_y, 
    double *ptarget_lat,
    double *ptarget_lon);


#endif // SCALE_CONVERT_H

/**********************************************************************
 * $Log: convert_scale.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/03/12 06:04:06  nadeem
 * Modified code to remove the overloading and hence notational abuse of MISSION-AREA statement in the .ovl file
 *
 * Revision 1.4  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.3  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.2  2006/09/26 18:35:04  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 **********************************************************************/
