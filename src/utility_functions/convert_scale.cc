/**********************************************************************
 **                                                                  **
 **                        convert_scale.cc                          **
 **                                                                  **
 **  Written by: Nadeem Ahmed Syed                                   **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

#include <stdio.h>
#include "convert_scale.h"

// PPM. From OpenMap Planet.java defaultPixelsPerMeter
int pixelsPerMeter =  3272; 

// EARTH_RADIUS. From Planet.java wgs84_earthEquatorialRadiusMeters 
float planetRadius = 6378137.0; 

// EARTH_PIX_RADIUS
float planetPixelRadius = planetRadius * pixelsPerMeter; 

// Width of the overlay image in meters
double map_width = 800; 

// Height of the overlay image in metere
double  map_height = 800; 

// Width of the overlay image in pixels
double map_width_pixels = 800; 

// default height of the overlay image in pixels
double  map_height_pixels = 800; 

// modified pixel_scale to do computations in meters.
float map_scale = pixelsPerMeter;

// Scale used by openmap to generate the overlay map.
float map_scale_pixels = 4000000; 

float scaled_radius = planetPixelRadius / map_scale; 

// center of the overlay map
double g_ref_longitude = -117; 
double g_ref_latitude = 32;   


/* Bunch of simple math functions below to help with the computations */

double my_asinh(double x)
{
  return log(x + sqrt(x*x +1));
}

double my_sinh(double x)
{
  return (exp(x) - exp(-x))/2.0;
}

double degToRad(double deg) 
{
  return (deg * (M_PI/180.0));
}

double radToDeg(double rad) 
{
  return (rad * (180.0/M_PI));
}

/* end of local functions */


/* This method take the target location in longitude and latitude format 
   and returns the equivalent X, Y position in meters. 
   The return values are stored in the last 2 parameters
*/
void geographic2global(
    double target_lat, 
    double target_lon,
    double *ptarget_X,
    double *ptarget_Y)
{

  double wx = map_width/2;
  double hy = map_height/2;

  double phi = degToRad(target_lat);
  double lambda = degToRad(target_lon);

  double asinh_of_tanRefLat = my_asinh(tan(degToRad(g_ref_latitude)));

  scaled_radius = planetPixelRadius / map_scale;

  *ptarget_X = ((scaled_radius *  (lambda - degToRad(g_ref_longitude))) + wx) /** cos(degToRad(g_ref_latitude))*/;
  *ptarget_Y = (scaled_radius * (my_asinh(tan(phi)) - asinh_of_tanRefLat)) + hy;
}

/* This method take the target location in X and Y in terms of meters 
   and returns the equivalent longitude, latitude position. The return 
   values are stored in the last 2 parameters.
*/
void global2geographic(
    double target_x,
    double target_y, 
    double *ptarget_lat,
    double *ptarget_lon)
{

  double wx = map_width/2;
  double hy = map_height/2;

  double asinh_of_tanRefLat = my_asinh(tan(degToRad(g_ref_latitude)));
  scaled_radius = planetPixelRadius / map_scale;

  float wc = asinh_of_tanRefLat * scaled_radius;

  double my_x, my_y;
  my_x = target_x;
  my_y = target_y;

  target_x -= wx;
  target_y -= hy;

  /* Doing this seems to screw up something. So comment it out 
  target_x = target_x * acos(degToRad(g_ref_latitude));
  */

  *ptarget_lat = radToDeg(atan(my_sinh((target_y + wc) / scaled_radius)));
  *ptarget_lon = radToDeg( target_x / scaled_radius + degToRad(g_ref_longitude));
}


/* This method take the target location in longitude and latitude format 
   and returns the equivalent X, Y position in pixels. The return values 
   are storedin the last 2 parameters
*/
void geographic2pixel(
    double target_lat, 
    double target_lon,
    double *ptarget_X,
    double *ptarget_Y)
{

  double wx = map_width_pixels/2;
  double hy = map_height_pixels/2;

  double phi = degToRad(target_lat);
  double lambda = degToRad(target_lon);

  double asinh_of_tanRefLat = my_asinh(tan(degToRad(g_ref_latitude)));

  scaled_radius = planetPixelRadius / map_scale_pixels;

  *ptarget_X = (scaled_radius *  (lambda - degToRad(g_ref_longitude))) + wx;
  *ptarget_Y = (scaled_radius * (my_asinh(tan(phi)) - asinh_of_tanRefLat)) + hy;
}

/* This method take the target location in X and Y pixel position and returns 
   the equivalent longitude, latitude position. The return values are stored
   in the last 2 parameters.
*/
void pixel2geographic(
    double target_x,
    double target_y, 
    double *ptarget_lat,
    double *ptarget_lon)
{

  double wx = map_width_pixels/2;
  double hy = map_height_pixels/2;

  double asinh_of_tanRefLat = my_asinh(tan(degToRad(g_ref_latitude)));

  scaled_radius = planetPixelRadius / map_scale_pixels;

  float wc = asinh_of_tanRefLat * scaled_radius;

  target_x -= wx;
  target_y -= hy;

  *ptarget_lat = radToDeg(atan(my_sinh((target_y + wc) / scaled_radius)));
  *ptarget_lon = radToDeg( target_x / scaled_radius + degToRad(g_ref_longitude));

  target_x += wx;
  target_y += hy;
}

/**********************************************************************
 * $Log: convert_scale.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.6  2007/03/12 06:04:06  nadeem
 * Modified code to remove the overloading and hence notational abuse of MISSION-AREA statement in the .ovl file
 *
 * Revision 1.5  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.4  2007/03/10 03:14:00  nadeem
 * fixed a bug.
 *
 * Revision 1.3  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.2  2006/09/26 18:35:04  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 **********************************************************************/
