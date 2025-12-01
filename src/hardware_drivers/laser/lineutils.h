/**********************************************************************
 **                                                                  **
 **                          lineutils.h                             **
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

/* $Id: lineutils.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef LINE_UTILS_HEADER
#define LINE_UTILS_HEADER

/**********************************************************************
 **                                                                  **
 **                           prototypes                             **
 **                                                                  **
 **********************************************************************/

void printPoint(point2d_t *p, char *txt);
void printLineSeg(lineseg2d_t *l, char *txt);
void printLine(line2d_t *l, char *txt);
void convLineSeg2Line(lineseg2d_t *ls, line2d_t *l);
void convPoints2LineSeg(point2d_t *p1, point2d_t *p2, lineseg2d_t *l);
void convPoints2Line(point2d_t *p1, point2d_t *p2, line2d_t *l);
int inBoundsOfLineSeg(point2d_t *p, lineseg2d_t *ls);
int intersectTwoLineSegs(lineseg2d_t *ls1, lineseg2d_t *ls2, point2d_t *p);
int intersectTwoLines(line2d_t *l1, line2d_t *l2, point2d_t *p);
double cartesianDistance(point2d_t *p1, point2d_t *p2);
int simulateLaserScan(point2d_t *robotCenter, double robotHeading, 
		      lineseg2d_t map[], int nlines,  
		      double maxLaserRange, 
		      rawLaserScan_t *simLaserReadings, int numReadings);

#endif




/**********************************************************************
 * $Log: lineutils.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:40  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/08/12 18:45:51  saho
 * Initial revision
 *
 **********************************************************************/
