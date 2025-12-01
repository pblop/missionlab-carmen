/**********************************************************************
 **                                                                  **
 **                            lineutils.h                           **
 **                                                                  **
 **  Written by: Jonathan Diaz                                       **    
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: lineutils.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef LINE_UTILS_H
#define LINE_UTILS_H

#include "gt_std.h"

typedef unsigned char BOOL;

typedef struct point2d
{
  /* A point in two dimensions */
  double x, y; 
} point2d_t;

typedef struct lineseq2d
{
  /* A line segment in two dimensions*/
  point2d_t endpt[2];
} lineseg2d_t;

typedef struct line2d
{
  /* A line in two dimensions */ 
  /* If the line is vertical, then c is the x-intercept
     and isdef is set to false. Otherwise m is the slope,
     b is the y-intercept, and isdef is set to true.
  */
  double m, b, c;
  BOOL isdef;
} line2d_t;

typedef struct lineRec2d
{
  /* Records a line segment and the corresponding real
     line equation on which it lies. A next pointer is included
     to ease in  maintaining a linked list structure. 
  */
  lineseg2d_t ls;
  line2d_t ln;
  struct lineRec2d *next;
  double height;
  double trueheight;
} lineRec2d_t;

typedef struct sim_obstacle
{
  double center[3];
  struct sim_obstacle *next;
} sim_obstacle_t;

// ENDO - gcc 3.4: Moved from gt_simulation.h
struct room_t
{
    lineRec2d_t* walls;
    room_t* next;
};

struct door_t
{
    lineRec2d_t* doorway;
    int marked;
    door_t* next;
};

struct hallwayRec_t
{
    Point start;
    Point end;
    double width;
    lineRec2d_t *extents;
    hallwayRec_t* next;
};

extern sim_obstacle_t *newSimObstacle(point2d_t *p, double height = 1);
extern lineRec2d_t *newLineRec(double *pts, double height = 1, int highpt = 0);
extern void setLineRec(double x1, double y1, double x2, double y2, lineRec2d_t *retval);
extern int intersectTwoLinesegs(lineRec2d_t *r, lineRec2d_t *s, point2d_t *p);
extern void printLineRec(char *txt, lineRec2d_t *l);
extern double cartesianDistance(point2d_t *r, point2d_t *s);
extern sim_obstacle_t *calculateRayLineIntersections(point2d_t *origin, double heading, 
						     lineRec2d_t *lines, int *numObsDetected);
extern sim_obstacle_t *simulateSonarLineDetection(point2d_t *robotCenter, double robotHeading, 
						  lineRec2d_t *lines, int numSonar, double maxSonarRange, int *numObsDetected);
extern sim_obstacle_t* haveLOSwalls( double x1, double y1, double z1,
				     double x2, double y2, double z2,
				     lineRec2d_t* lines, int* numObsDetected );

#endif

/**********************************************************************
 * $Log: lineutils.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/06/08 14:50:22  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
