#ifndef GT_STD_H
#define GT_STD_H

/*****************************************************************************

	gt_std.h

	Description:
		function prototypes for standard c library functions
		(Renamed from previous doug.h, based on std.h)


        Written by: Douglas C. MacKenzie

        Copyright 1995, Georgia Tech Research Corporation 
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details. 

******************************************************************************/
/* $Id: gt_std.h,v 1.2 2009/03/02 18:48:37 zkira Exp $ */

#include <cmath>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using std::max;

/**********************************************************************
 **                                                                  **
 **                         template functions                       **
 **                                                                  **
 **********************************************************************/
#ifndef GTSQR
#define GTSQR(x) ( (x) * (x) )
#endif

/*
// template<class T> T sqr(T x) { return x * x; };
#ifndef sqr
#define sqr(x) ( (x) * (x) )
#endif

// template<class T> T max(T a, T b) { return a>b?a:b; };
#ifndef max
#define max(a,b) ( (a) > (b) ? (a) : (b) )
#endif

// template<class T> T min(T a, T b) { return a<b?a:b; };
#ifndef min
#define min(a,b) ( (a) < (b) ? (a) : (b) )
#endif
*/

inline double vlength( double x, double y )
{
    return sqrt( x*x + y*y );
}

inline double vlength( double x, double y, double z )
{
    return sqrt( x*x + y*y + z*z );
}

/**********************************************************************
 **                                                                  **
 **                         define constants                         **
 **                                                                  **
 **********************************************************************/

#define GT_SUCCESS (0)
#define GT_FAILURE (-1)

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

// higher value, for float only
#ifndef GT_INFINITY
#define GT_INFINITY 1000000000000.0
#endif

// old value, for integer use
#ifndef INT_INFINITY
#define INT_INFINITY 10000000
#endif

#ifndef PI
#define     PI    	3.14159265358979323846
#define     PI_OVER_2   1.57079632679489661923
#define     PI_OVER_4	0.78539816339744830962
#define     PI2 	(2*PI)
#endif

#ifndef EPS_ZERO
#define EPS_ZERO (0.00001)
#endif


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define INCHES_FROM_FEET(val) ((val)*12.0)
#define FEET_FROM_INCHES(val) ((val)/12.0)
#define INCHES_TO_FEET(val) FEET_FROM_INCHES(val)
#define FEET_TO_INCHES(val) INCHES_FROM_FEET(val)

#define FEET_FROM_METERS(val) ((val)*3.28084)
#define METERS_FROM_FEET(val) ((val)/3.28084)

#define DEGREES_FROM_RADIANS(val) ((val)*180.0/PI)
#define RADIANS_FROM_DEGREES(val) ((val)*PI/180.0)
#define RADIANS_TO_DEGREES(val) DEGREES_FROM_RADIANS(val)
#define DEGREES_TO_RADIANS(val) RADIANS_FROM_DEGREES(val)

#define msqrt(a) (a<EPS_ZERO?0:sqrt(a))
#define macos(a) (a>(1-EPS_ZERO)?0:acos(a))
#define masin(a) (asin(a))

// get cos of angle between unit vectors (in radians)
#define vector_cos(x1,y1,x2,y2) ((x1)*(x2)+(y1)*(y2))

/* compute the first intersection of a ray and a circle 
     q = base of ray
     v = unit vector along ray
     c = center of circle
     r = radius of circle
     d = distance along ray to intersection point : INFINITY if no intersection
*/
/* -- this is not currently used anywhere, but if it were it should really be an inline function.
      The only reason this is only being commented out is so that if this really is needed
      in the future, it can be used as a basis to make in inline function.
#define line_circle_intersection(q,v,c,r,d) \
   { Vector q_c,two; double t,f,pos=0,neg=0; \
     minus_2d(q,c,q_c); times_2d(v,2,two); \
     f = dot_2d(two,q_c); \
     t = sqr(f) - 4* (dot_2d(q_c,q_c) - sqr(r)); \
     if( t < 0 ) \
     { \
        d=GT_INFINITY; \
     } \
     else \
     { \
	pos = (-1*f + msqrt(t))/2; \
	neg=(-1*f - msqrt(t))/2;  \
        if( pos>=0 && pos<=neg ) \
        { \
           d=pos;\
        } \
        else if(neg>=0 && neg<=pos) \
        { \
           d=neg;\
        } \
        else \
        { \
          d=GT_INFINITY; \
        } \
     }\
   }
*/

#define add_2d(a,b,r) {(r).x=(a).x + (b).x; (r).y=(a).y + (b).y;}
#define plus_2d(a,b) {(a).x += (b).x; (a).y += (b).y;}
#define minus_2d(a,b,r) {(r).x=(a).x - (b).x; (r).y=(a).y - (b).y;}
#define times_2d(a,b,r) {(r).x=(a).x * (b); (r).y=(a).y * (b);}
#define mult_2d(a,b) {(a).x *= (b); (a).y *= (b);}
#define div_2d(a,b) {(a).x /= (b); (a).y /= (b);}
#define divide_2d(a,b,r) {(r).x=(a).x / (b); (r).y=(a).y / (b);}
#define minus_3d(a,b,r) {(r).x=(a).x - (b).x; (r).y=(a).y - (b).y; (r).z=(a).z - (b).z;}
#define add_3d(a,b,r) {(r).x=(a).x + (b).x; (r).y=(a).y +(b).y; (r).z=(a).z + (b).z;}
#define plus_3d(a,b) {(a).x += (b).x; (a).y += (b).y; (a).z += (b).z;}
#define times_3d(a,b,r) {(r).x=(a).x * (b); (r).y=(a).y * (b); (r).z=(a).z * (b);}
#define mult_3d(a,b) {(a).x *= (b); (a).y *= (b); (a).z *= (b);}
#define div_3d(a,b) {(a).x /= (b); (a).y /= (b); (a).z /= (b);}
#define divide_3d(a,b,r) {(r).x=(a).x / (b); (r).y=(a).y / (b); (r).z=(a).z / (b);}
#define scale_2d(a,b) {(a).x *= (b); (a).y *= (b);}
#define scale_3d(a,b) {(a).x *= (b); (a).y *= (b); (a).z *= (b);}


// rotate 3d vector by angle in degrees, ccw around axis
#define rotate_x(v,t) {Vector in=(v); \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).y = in.y*c - in.z*s;\
		       (v).z = in.y*s + in.z*c;}

#define rotate_y(v,t) {Vector in=(v); \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).x =      in.x*c + in.z*s;\
		       (v).z = -1 * in.x*s + in.z*c;}

#define rotate_z(v,t) {Vector in=v; \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).x = in.x*c - in.y*s;\
		       (v).y = in.x*s + in.y*c;}

// calculate 3d cross product of 2 Vectors
// cross product is ABsin(theta)
#define cross_3d(a,b,r) {(r).x = (a).y*(b).z - (a).z*(b).y;\
			 (r).y = (a).z*(b).x - (a).x*(b).z;\
			 (r).z = (a).x*(b).y - (a).y*(b).x;}

// calculate dot product of two 3d Vectors
// dot product is ABcos(theta)
// Also: dot product gives parallel component of resultant vector
#define dot_3d(a,b) ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)
#define dot_2d(a,b) ((a).x*(b).x + (a).y*(b).y)

// get length of vector
#define len_2d(v) (msqrt(GTSQR((v).x) + GTSQR((v).y)))
#define len_3d(v) (msqrt(GTSQR((v).x) + GTSQR((v).y)+ GTSQR((v).z)))

// make v a unit vector
#define unit_3d(v) {double xx_len=len_3d(v);\
		    (v).x = (v).x/xx_len;\
		    (v).y = (v).y/xx_len;\
		    (v).z = (v).z/xx_len;}
#define unit_2d(v) {double xx_len=len_2d(v);\
                    if( xx_len > EPS_ZERO )\
		       {(v).x = (v).x/xx_len; (v).y = (v).y/xx_len;}\
		    else\
		       {(v).x = 0.0; (v).y = 0.0;}\
		   }

// get length of vector
#define vector_length(x,y) vlength((x),(y))

// bound degrees to -180..+180
#define deg_range_pm_180(t) {(t)=fmod((double)(t),360); if((t)>180) (t) -= 360; if((t)<=-180) (t) += 360;}

// bound degrees to [0.. 360)
#define deg_range_pm_360(t) {(t)=fmod((double)(t),360); if((t)<0) (t) += 360;}

#ifndef bound
// bound variable to low..high range
#define bound(v,low,high) max( min((v),(high)) ,(low))
#endif

#ifndef sign
#define sign(x) ((x)<0 ? -1 : 1)
#endif

#ifndef interpolate
#define interpolate(low_val,low_rtn,big_val,big_rtn,val) \
     ((low_rtn)+((val)-(low_val))*(double)((big_rtn)-(low_rtn))/(double)((big_val)-(low_val)))

#endif

/**********************************************************************
 **                                                                  **
 **                              types                               **
 **                                                                  **
 **********************************************************************/


typedef struct
{
    double x;
    double y;
    double z;
} Point;

typedef Point Vector;
#define ZERO_VECTOR {0.0, 0.0, 0.0}
#define VECTOR_CLEAR(v) {(v).x=0.0; (v).y=0.0; (v).z=0.0;}

// Geographic Coordinates
typedef struct GeoCoord
{
    double latitude;
    double longitude;
};

typedef struct
{
    double x;
    double delta_x;
    double y;
    double delta_y;
} Ray;

inline double AngleOfVector( Vector suVector )
{
    double dHeading = atan2( suVector.y, suVector.x );
    dHeading = DEGREES_FROM_RADIANS( dHeading );
    deg_range_pm_360( dHeading );

    return dHeading;
}

typedef struct GTRobotPosID_t
{
    int id;
    double x;
    double y;
    double z;
    double heading;
};

// ENDO - gcc 3.4: Moved from gt_message_types.h
typedef struct gt_Point 
{
    float x;
    float y;
    float z;
} gt_Point;

// ENDO - gcc 3.4: Moved from gt_std_types.h
struct obs_reading
{
    Vector center;
    double r;
};

struct raw_sonar_reading
{
    double val;
};

// ENDO - gcc 3.4: Moved from gt_simulation.h
struct gt_Point_array
{
    int num_points;
    gt_Point *point;
};

struct gt_Point_list
{
    float  x;
    float  y;
    gt_Point_list* next;
};

struct robot_position
{
    Vector v;
    double heading; // yaw
};

struct gps_position
{
    double latitude;
    double longitude;
    double direction;
    double pingtime;
    int pingloss;
};

struct type_pry {
    int status;  // 0: successful
                 // 1: serial port not open
                 // 2: reading corrupted or no reading
    double shaft_heading;
    double pitch;          // 0 = level, 90 = front up
    double roll;           // 0 = level, 90 = banked right
    double yaw;            // 0 = east, 90 = north
};
/******************************* end gt_std.h ********************************/
#endif   // GT_STD_H


/**********************************************************************
 * $Log: gt_std.h,v $
 * Revision 1.2  2009/03/02 18:48:37  zkira
 * Added full 3D pose info (roll/pitch/yaw)
 *
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2006/12/05 01:50:44  endo
 * GeoCoord added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/06/08 22:33:55  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.3  2006/05/14 06:23:36  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2006/02/19 17:56:41  endo
 * Experiment related modifications
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.25  2003/04/06 11:54:35  endo
 * gcc 3.1.1
 *
 * Revision 1.24  2002/10/25 01:37:39  kaess
 * - "robot-too-cautious" bug fixed: robot should be in
 * cautious mode iff inside safety margin, which is the dase
 * when COOP returns an infinit vector - infinity needed to be changed
 *
 * Revision 1.23  2002/07/02 20:36:58  blee
 * added scale_2d, scale_3d, and deg_range_pm_360 macros and
 * AngleOfVector()
 *
 * Revision 1.22  2001/12/22 16:03:33  endo
 * RH 7.1 porting.
 *
 * Revision 1.21  2000/11/03 20:47:43  blee
 * Commented out line_circle_intersection and fixed potential
 * sources of macro expansion errors.
 *
 * Revision 1.20  1999/12/16 22:59:16  mjcramer
 * rh6 port
 *
 * Revision 1.19  1999/06/29 02:58:31  mjcramer
 * Added 3D functions
 *
 * Revision 1.18  1996/05/15  19:20:30  doug
 * fixed problem with min,max,sqr templates by making them macros
 *
 * Revision 1.17  1996/03/03  23:51:41  doug
 * *** empty log message ***
 *
 * Revision 1.16  1995/10/18  14:01:14  doug
 * *** empty log message ***
 *
 * Revision 1.15  1995/10/11  22:04:13  doug
 * *** empty log message ***
 *
 * Revision 1.14  1995/04/14  18:18:48  jmc
 * Renamed THIS file from doug.h to gt_std.h.
 *
 * Revision 1.13  1995/03/31  22:01:33  jmc
 * Added a copyright notice.
 *
 * Revision 1.12  1994/10/26  19:08:12  doug
 * add scalar division div_2d
 *
 * Revision 1.11  1994/10/19  14:24:24  doug
 * *** empty log message ***
 *
 * Revision 1.10  1994/10/17  14:27:55  doug
 * removing protypes now that headerfiles have them
 *
 * Revision 1.9  1994/10/17  14:24:38  doug
 * *** empty log message ***
 *
 * Revision 1.8  1994/10/05  16:32:23  doug
 * fixing include of math.h
 *
 * Revision 1.7  1994/10/05  16:04:14  doug
 * removed sin/cos prototypes
 *
 * Revision 1.6  1994/08/18  20:04:56  doug
 * moved gt_std.h doug.h
 *
 * Revision 1.5  1994/08/18  19:29:27  doug
 * *** empty log message ***
 *
 * Revision 1.4  1994/08/12  17:14:54  doug
 * remove popen prototype
 *
 * Revision 1.3  1994/08/12  17:13:13  doug
 * remove fopen prototype
 *
 * Revision 1.2  1994/07/14  13:15:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
