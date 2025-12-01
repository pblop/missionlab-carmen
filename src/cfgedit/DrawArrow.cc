/**********************************************************************
 **                                                                  **
 **                          arrow_head.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: DrawArrow.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <assert.h>
#include <math.h>

#include "load_cdl.h"
#include "design.h"
#include "globals.h"

typedef struct         
{    
   double x; 
   double y;
} Vector;

#ifndef PI
#define     PI          3.14159265358979323846
#endif
#define RADIANS_FROM_DEGREES(val) ((val)*PI/180.0)
#define DEGREES_TO_RADIANS(val) RADIANS_FROM_DEGREES(val)

#define rotate_z(v,t) {Vector in=v; \
                       double c=cos(DEGREES_TO_RADIANS(t));\
                       double s=sin(DEGREES_TO_RADIANS(t));\
                       v.x = in.x*c - in.y*s;\
                       v.y = in.x*s + in.y*c;}

#define mult_2d(a,b) {a.x *= (b); a.y *= (b);}
#define EPS_ZERO (0.00001)
#define msqrt(a) (a<EPS_ZERO?0:sqrt(a))
#define len_2d(v) (msqrt(sqr(v.x) + sqr(v.y)))
#define unit_2d(v) {double xx_len=len_2d(v);\
       	v.x = v.x/xx_len;\
	v.y = v.y/xx_len;}


//************************************************************
// draw a line from the tail to the point with arrow head on the point

void
DrawArrow(Display *dsp, Window w, GC gc, 
	int tail_x, int tail_y, int point_x, int point_y,
      	int ah_length)
{
   const double one_over_cos_30 = 1.1547005;

   // Make a vector from the tail to the point
   Vector v;
   v.x = point_x - tail_x;
   v.y = point_y - tail_y;

   // Shorten it up by the length of the arrow head, so comes to a point
   double len = len_2d(v);
   mult_2d(v, (len - ah_length) / len);

   // Draw the line
   XDrawLine(dsp, w, gc, tail_x, tail_y, 
			 (int)(tail_x + v.x + 0.5), (int)(tail_y + v.y + 0.5));

   // Make a vector from the point to the tail
   Vector r;
   r.x = tail_x - point_x;
   r.y = tail_y - point_y;

   // Make it the desired length
   unit_2d(r);
   mult_2d(r, ah_length * one_over_cos_30);

   XPoint points[3];
   points[0].x = point_x;
   points[0].y = point_y;

   // Load the right one
   rotate_z(r,-30);
   points[1].x = (int)(point_x + r.x+0.5);
   points[1].y = (int)(point_y + r.y+0.5);

   // Load the left one
   rotate_z(r,60);
   points[2].x = (int)(point_x + r.x+0.5);
   points[2].y = (int)(point_y + r.y+0.5);

   // Draw the arrow head
   XFillPolygon(dsp, w, gc, points, 3, Convex, CoordModeOrigin);
}

///////////////////////////////////////////////////////////////////////
// $Log: DrawArrow.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:31  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.1  1996/01/31  03:06:53  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
