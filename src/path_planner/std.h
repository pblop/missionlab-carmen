/**********************************************************************
 **                                                                  **
 **                               std.h                              **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: std.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef STD_H
#define STD_H

/*****************************************************************************

	std.h

	Description:
		function prototypes for standard c library functions

	Modification History:
		Date		Author		Remarks

******************************************************************************/

/* $Id: std.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/**********************************************************************
 **                                                                  **
 **                         define constants                         **
 **                                                                  **
 **********************************************************************/

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef TRUE
#define TRUE (!FALSE)
#endif

#define GT_SUCCESS (0)
#define GT_FAILURE (-1)

#ifndef OK
#define OK            (0)
#endif

#ifndef ERROR
#define ERROR         (-1)
#endif

/* Error severity codes */
#ifndef ABORT
#define ABORT (1)
#endif

#ifndef WARN
#define WARN (2)
#endif

#ifndef INFO
#define INFO (3)
#endif

#ifndef PATH_PLAN_INFINITY
#define PATH_PLAN_INFINITY 10000000
#endif

#ifndef PI
#define     PI    	3.14159265358979323846
#define     PI_OVER_2   1.57079632679489661923
#define     PI_OVER_4	0.78539816339744830962
#define     PI2 	(2.0*PI)
#endif

#ifndef EPS_ZERO
#define EPS_ZERO (0.00001)
#endif

#ifndef PLANNER_PARAM_FILE
#define PLANNER_PARAM_FILE "plannerParams.dat"
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

/* get cos of angle between unit vectors (in radians) */
#define vector_cos(x1,y1,x2,y2) ((x1)*(x2)+(y1)*(y2))

/* compute the first intersection of a ray and a circle 
     q = base of ray
     v = unit vector along ray
     c = center of circle
     r = radius of circle
     d = distance along ray to intersection point : PATH_PLAN_INFINITY if no intersection
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
        d=PATH_PLAN_INFINITY; \
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
          d=PATH_PLAN_INFINITY; \
        } \
     }\
   }
*/

#define minus_2d(a,b,r) {(r).x=(a).x - (b).x; (r).y=(a).y - (b).y;}
#define add_2d(a,b,r) {(r).x=(a).x + (b).x; (r).y=(a).y + (b).y;}
#define plus_2d(a,b) {(a).x += (b).x; (a).y += (b).y;}
#define times_2d(a,b,r) {(r).x=(a).x * (b); (r).y=(a).y * (b);}
#define mult_2d(a,b) {(a).x *= (b); (a).y *= (b);}
#define div_2d(a,b) {(a).x /= (b); (a).y /= (b);}
#define divide_2d(a,b,r) {(r).x=(a).x / (b); (r).y=(a).y / (b);}


/* rotate 3d vector by angle in degrees, ccw around axis */
#define rotate_x(v,t) {Vector in=v; \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).y = in.y*c - in.z*s;\
		       (v).z = in.y*s + in.z*c;}

#define rotate_y(v,t) {Vector in=v; \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).x =      in.x*c + in.z*s;\
		       (v).z = -1 * in.x*s + in.z*c;}

#define rotate_z(v,t) {Vector in=v; \
		       double c=cos(DEGREES_TO_RADIANS(t));\
		       double s=sin(DEGREES_TO_RADIANS(t));\
		       (v).x = in.x*c - in.y*s;\
		       (v).y = in.x*s + in.y*c;}

/* calculate 3d cross product of 2 Vectors */
/* cross product is ABsin(theta) */
#define cross_3d(a,b,r) {(r).x = (a).y*(b).z - (a).z*(b).y;\
			 (r).y = (a).z*(b).x - (a).x*(b).z;\
			 (r).z = (a).x*(b).y - (a).y*(b).x;}

/* calculate dot product of two 3d Vectors */
/* dot product is ABcos(theta) */
/* Also: dot product gives parallel component of resultant vector */
#define dot_3d(a,b) ((a).x*(b).x + (a).y*(b).y + (a).z*(b).z)
#define dot_2d(a,b) ((a).x*(b).x + (a).y*(b).y)

/* get length of vector */
#define len_2d(v) (msqrt(sqr((v).x) + sqr((v).y)))
#define len_3d(v) (msqrt(sqr((v).x) + sqr((v).y)+ sqr((v).z)))

/* make v a unit vector */
#define unit_3d(v) {double xx_len=len_3d(v);\
		    (v).x = (v).x/xx_len;\
		    (v).y = (v).y/xx_len;\
		    (v).z = (v).z/xx_len;}
#define unit_2d(v) {double xx_len=len_2d(v);\
		    (v).x = (v).x/xx_len;\
		    (v).y = (v).y/xx_len;}

/* get length of vector */
#define vector_length(x,y) (msqrt((double)(x)*(double)(x) + (double)(y)*(double)(y)))

/* bound degrees to -180..+180 */
#define deg_range_pm_180(t) {while((t)>180) (t) -= 360; while((t)<=-180) (t) += 360;}


#ifndef min
#define min(x,y) ((x) < (y) ? (x) : (y) )
#endif

#ifndef max
#define max(x,y) ((x) > (y) ? (x) : (y) )
#endif

#ifndef bound
/* bound variable to low..high range */
#define bound(v,low,high) max( min((v),(high)) ,(low))
#endif

#ifndef sign
#define sign(x) ((x)<0 ? -1 : 1)
#endif

#ifndef sqr
#define sqr(x) ((x)*(x))
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

typedef struct
{
    double x;
    double delta_x;
    double y;
    double delta_y;
} Ray;

/******************************* end doug.h ********************************/
#endif


/**********************************************************************
 * $Log: std.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:51  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:00  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 05:57:38  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 23:00:23  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 10:16:11  endo
 * Checked in for Robert R. Burridge. Various bugs fixed.
 *
 * Revision 1.2  2000/11/03 20:45:38  blee
 * Changed the sentry, commented out line_circle_intersection,
 * and fixed potential sources of macro expansion errors.
 *
 * Revision 1.1  2000/03/22 04:40:18  saho
 * Initial revision
 *
 * Revision 1.6  1995/05/12  18:08:18  doug
 * removing True and False defs
 *
 * Revision 1.5  1995/03/14  18:45:20  doug
 * copied the mlab version back here
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
