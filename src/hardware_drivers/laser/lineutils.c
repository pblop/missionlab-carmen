/**********************************************************************
 **                                                                  **
 **                          lineutils.c                             **
 **                                                                  **
 **  Functions for working with lines and line intersections.        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan Diaz  and                                 **
 **               Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: lineutils.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "data_structures.h"


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

/* Macro sets s to be the smaller of a and b, and sets l to the larger. */
#define SORT_TWO(a, b, s, l) if (a < b) {s = a; l = b;} else {s = b; l = a;} 
/* Macro determines if var and val are approximately equal to within epsilon */
#define APPROX(var, val) ((var) >= ((val) - EPS) && (var) <= ((val) + EPS))

#define MAX(a, b) (((a) > (b))? (a): (b))
#define MIN(a, b) (((a) < (b))? (a): (b))


/**********************************************************************
 **            PRINT_POINT                                           **
 **********************************************************************/
void printPoint(point2d_t *p, char *txt)
{
  printf("%s{", txt);
  printf("\tPoint: (%f, %f) ", p->x,p->y);
  printf("}\n");
}

/**********************************************************************
 **            PRINT_LINE_SEG                                        **
 **********************************************************************/
void printLineSeg(lineseg2d_t *l, char *txt)
{
  printf("%s{", txt);
  printf("\tLineSeg: (%f, %f) (%f, %f)", l->x1,l->y1,l->x2,l->y2);
  printf("}\n");
}


/**********************************************************************
 **            PRINT_LINE                                            **
 **********************************************************************/
void printLine(line2d_t *l, char *txt)
{
  printf("%s{", txt);
  printf("\tLine{\n");
  printf("\t\tr: %f theta: %f\n", l->r, 180/M_PI*l->theta);
  printf("\t\tVx: %f Vy: %f\n", l->Vx, l->Vy);
  printf("\t\t(%f, %f)", l->x, l->y);
  printf("}\n");
}

/**********************************************************************
 **        convLineSeg2Line                                          **
 **********************************************************************/
/* function converts a line segment into a line. */
void convLineSeg2Line(lineseg2d_t *ls, line2d_t *l)
{
  double dx, dy, wx, wy;
  double a1,b1, a2,b2;
  double x1,y1, x2,y2;
  double t, s;
  double det;

  dx = (ls->x2 - ls->x1);
  dy = (ls->y2 - ls->y1);

  l->Vx = dx;
  l->Vy = dy;
  
  
  // Find the intersection point
  a1 = ls->x1;            b1 = ls->y1;
  a2 = ls->x2;            b2 = ls->y2;
  
  x1 = 0;                y1 = 0;
  x2 = -dy;              y2 = dx;

  det= (a2-a1)*(y1-y2) - (b2-b1)*(x1-x2);
  t=   (x1-a1)*(y1-y2) - (y1-b1)*(x1-x2);
  s=   (a2-a1)*(y1-b1) - (b2-b1)*(x1-a1);
       
  t /= det;
     
  // This is the intersection point
  wx = a1 + t*(a2-a1);
  wy = b1 + t*(b2-b1);
  l->r = sqrt(wx*wx + wy*wy);
  l->theta = atan2(dx, -dy); 
  l->x = ls->x1;
  l->y = ls->y1;
}


/**********************************************************************
 **          convPoints2LineSeg                                      **
 **********************************************************************/
/* This function converts two points into a line segment. 
   The input parameters are p1 and p2. The record, pointed to by l, 
   will be filled with the line segment parameters.
*/
void convPoints2LineSeg(point2d_t *p1, point2d_t *p2, lineseg2d_t *l)
{
  l->x1= p1->x;
  l->y1= p1->y;
  l->x2= p2->x;
  l->y2= p2->y;
}


/**********************************************************************
 **          convPoints2Line                                         **
 **********************************************************************/
/* This function converts two points into a line. 
   The input parameters are p1 and p2. The record, pointed to by l, 
   will be filled with the line parameters.
*/
void convPoints2Line(point2d_t *p1, point2d_t *p2, line2d_t *l)
{
  double dx, dy, wx, wy;
  double a1,b1, a2,b2;
  double x1,y1, x2,y2;
  double t, s;
  double det;

  dx = (p2->x - p1->x);
  dy = (p2->y - p1->y);

  l->Vx = dx;
  l->Vy = dy;
  
  
  // Find the intersection point
  a1 = p1->x;            b1 = p1->y;
  a2 = p2->x;            b2 = p2->y;
  
  x1 = 0;                y1 = 0;
  x2 = -dy;              y2 = dx;

  det= (a2-a1)*(y1-y2) - (b2-b1)*(x1-x2);
  t=   (x1-a1)*(y1-y2) - (y1-b1)*(x1-x2);
  s=   (a2-a1)*(y1-b1) - (b2-b1)*(x1-a1);
       
  t /= det;
  
  // This is the intersection point
  wx = a1 + t*(a2-a1);
  wy = b1 + t*(b2-b1);
  l->r = sqrt(wx*wx + wy*wy);
  l->theta = atan2(dx, -dy); 
  l->x = p1->x;
  l->y = p1->y;
}

/**********************************************************************
 **     IN_BOUNDS_OF_LINE_SEGMENT      **
 ****************************************/
/* function determines if a point on a line
   is within the extent of a segment of that line.
*/
int inBoundsOfLineSeg(point2d_t *p, lineseg2d_t *ls)
{
  double a1,b1, a2,b2;

  a1 = ls->x1;    b1 = ls->y1;
  a2 = ls->x2;    b2 = ls->y2;
  
   // Quick rejection test. If the rectangle surrounding
   // the line segment does not include the point reject
   if( (MAX(a1,a2) >= p->x && p->x >= MIN(a1,a2)) &&
       (MAX(b1,b2) >= p->y && p->y >= MIN(b1,b2))    )

     return SUCCESS;

   return FAILURE;
}

/**********************************************************************
 **        INTERSECT_TWO_LINE_SEGS                                   **
 **********************************************************************/

/* This function determines if two line segments intersect. 
 * If there is an intersection  SUCCESS is returned and the intersection 
 * point is stored in p. 
 * Otherwise there is no intersection and FAILURE is returned
 *
 *  Let the first line segment be defined by the points (a1,b1) and (a2,b2).
 *  Let the second line segment be defined by the points (x1,y1) and (x2,y2).
 *  These points are used to represent two lines (in vector form) that pass
 *  through the two line segments:
 *    (a,b)= (a1,b1) +t*((a2-a1),(b2-b1))
 *    (x,y)= (x1,y1) +s*((x2-x1),(y2-y1))
 *
 *  The resulting system is with two unknowns (t and s). The solutions is
 *  found using the Crammer's formula. If t and s both lie in the interval
 *  [0,1] then the line segments intersect; otherwise they don't.
 */

int intersectTwoLineSegs(lineseg2d_t *ls1, lineseg2d_t *ls2, point2d_t *p)
{
   double a1,b1, a2,b2;
   double x1,y1, x2,y2;
   double t, s;
   double det;

   a1= ls1->x1;    b1= ls1->y1;
   a2= ls1->x2;    b2= ls1->y2;

   x1= ls2->x1;    y1=ls2->y1;
   x2= ls2->x2;    y2=ls2->y2;

   // Quick rejection test. If the rectangels surrounding
   // two line segments do not intersect then the line segments
   // cannot intersect. However, if the rectangles intersect
   // we have to make additional calculatins.
   if( (MAX(a1,a2) >= MIN(x1,x2)) && (MAX(x1,x2) >= MIN(a1,a2)) &&
       (MAX(b1,b2) >= MIN(y1,y2)) && (MAX(y1,y2) >= MIN(b1,b2))    )
     {
       det= (a2-a1)*(y1-y2) - (b2-b1)*(x1-x2);
       t=   (x1-a1)*(y1-y2) - (y1-b1)*(x1-x2);
       s=   (a2-a1)*(y1-b1) - (b2-b1)*(x1-a1);
       
       if(!det)  // parallel?
	 return FAILURE;
       
       else
	 {
	   t /= det;
	   s /= det;
	   if( ((t>=0)&&(t<=1)) && ((s>=0)&&(s<=1)))
	     {
	       // This is the intersection point
	       p->x = a1 + t*(a2-a1);
               p->y = b1 + t*(b2-b1);
	       return SUCCESS;
	     }
	 }
     }
   
   return FAILURE;  // the line segments do not intersect
}



/**********************************************************************
 **       INTERSECT_TWO_LINES                                        **
 **********************************************************************/

/* This function determines if two lines  intersect. 
 * If there is an intersection  SUCCESS is returned and the intersection 
 * point is stored in p. 
 * Otherwise there is no intersection and FAILURE is returned
 *
 *  Let the first line be defined by the line segment specified by the 
 *  points (a1,b1) and (a2,b2) which lie on the first line.
 *  Let the second line be defined by the line segment specified by the 
 *  points (x1,y1) and (x2,y2) which lie on the second line.
 *
 *  These points are used to represent two lines (in vector form) that pass
 *  through the two line segments:
 *    (a,b)= (a1,b1) +t*((a2-a1),(b2-b1))
 *    (x,y)= (x1,y1) +s*((x2-x1),(y2-y1))
 *
 *  The resulting system is with two unknowns (t and s). The solutions is
 *  found using the Crammer's formula. 
 */
int intersectTwoLines(line2d_t *l1, line2d_t *l2, point2d_t *p)
{
   float a1,b1, a2,b2;
   float x1,y1, x2,y2;
   float t;
   float det;


   a1= l1->x;    
   b1= l1->y;
   a2= l1->x + l1->Vx;    
   b2= l1->y + l1->Vy;    

   x1= l2->x;    
   y1= l2->y;
   x2= l2->x + l2->Vx;    
   y2= l2->y + l2->Vy;    


   if( (l1->theta == l2->theta) && ( l1->r == l2->r) )
     {
       return FAILURE;  // the lines overlap
     }
   else
     {
       det= (a2-a1)*(y1-y2) - (b2-b1)*(x1-x2);
       t=   (x1-a1)*(y1-y2) - (y1-b1)*(x1-x2);
       
       if(!det)  // parallel? (should not happen)
	 return FAILURE;
       
       else
	 {
	   t /= det;
	   
	   // This is the intersection point
	   p->x = a1 + t*(a2-a1);
	   p->y = b1 + t*(b2-b1);
	   return SUCCESS;
	 }
     }
}



/**********************************************************************
 **         CARTESIAN_DISTANCE                                       **
 **********************************************************************/
/* function returns the cartesian distance between two points in 2-space */
double cartesianDistance(point2d_t *p1, point2d_t *p2)
{
  return sqrt((p1->x - p2->x) * (p1->x - p2->x) + 
              (p1->y - p2->y) * (p1->y - p2->y));
}


/**********************************************************************
 **    SIMULATE_LASER_SCAN                                           **
 **********************************************************************/
/* Function calculates intersections for each laser ray  against a
   series of line segments.

   Laser rays originate at the center of the robot and head
   in directions distributed evenly along the unit semi-circle. 
*/
int simulateLaserScan(point2d_t *robotCenter, double robotHeading, 
		      lineseg2d_t map[], int nlines,  
		      double maxLaserRange, 
		      rawLaserScan_t *simLaserReadings, int numReadings)
{
  point2d_t inters, raypt;
  lineseg2d_t ray;
  double bdist, dist;
  int i, j;

  for (i = 0; i < numReadings; i++) {
    /* Calculate a new endpoint spaced about a semi-circle */
    raypt.x = robotCenter->x + maxLaserRange * cos(-M_PI/2.0 + robotHeading + (M_PI/numReadings*i));
    raypt.y = robotCenter->y + maxLaserRange * sin(-M_PI/2.0 + robotHeading + (M_PI/numReadings*i));
    
    /* Fill the line segment ray with the robot's center and (x,y) as endpoints. */
    convPoints2LineSeg(robotCenter, &raypt, &ray);
    
    /* Init tmp and invalidate bdist */
    /* Find the closest intersection on the line segment ray. */
  
    bdist = maxLaserRange; 
    for (j = 0; j < nlines; j++) {
      if (intersectTwoLineSegs(&ray, &map[j], &inters)) {
	/* We have an intersection, determine if it is the closest */
	dist = cartesianDistance(&inters, robotCenter);
	if (dist < bdist) {
	  bdist = dist;
	} 
      }
    }
    simLaserReadings->dist[i] = bdist;
  }
  
  return SUCCESS;
}


/**********************************************************************
 * $Log: lineutils.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/08/12 18:45:31  saho
 * Initial revision
 *
 **********************************************************************/
