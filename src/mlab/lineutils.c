/**********************************************************************
 **                                                                  **
 **                            lineutils.c                           **
 **                                                                  **
 **  Written by: Jonathan Diaz                                       **    
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: lineutils.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "lineutils.h"
#include "3d_vis_wid.h"

/**********************************************************************
 **                                                                  **
 **                            constants                             **
 **                                                                  **
 **********************************************************************/
#define EPS (1e-6)
#define SUCCESS 1 
#define FAILURE 0
#define TRUE 1
#define FALSE 0


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

// Macro sets s to be the smaller of a and b, and sets l to the larger.
#define SORT_TWO(a, b, s, l) if (a < b) {s = a; l = b;} else {s = b; l = a;}
 
// Macro determines if var and val are approximately equal to within epsilon
#define APPROX(var, val) ((var) >= ((val) - EPS) && (var) <= ((val) + EPS))

extern ObstacleHandler *Obs_Handler;
extern TerrainMap *Terrain;

/****************************************
 **          NEW_SIM_OBSTACLE          **
 ****************************************/
/* function returns a sim_obstacle in new storage with center
   set to the location of point p.
*/
sim_obstacle_t* newSimObstacle( point2d_t* p, double height)
{
    sim_obstacle_t* retval;
  
    if ( !( retval = (sim_obstacle_t*) malloc( sizeof( sim_obstacle_t ) ) ) )
    {
        return NULL;
    }
    retval->center[ 0 ] = p->x;
    retval->center[ 1 ] = p->y;
    retval->center[ 2 ] = height;
    retval->next = NULL;

    return retval;
}
/****************************************
 **            PRINT_LINE_REC          **
 ****************************************/
void printLineRec( char* txt, lineRec2d_t* l )
{
    lineRec2d_t* tmp;
  
    printf( "%s{\n", txt );
    for ( tmp = l; tmp != NULL; tmp = tmp->next )
    {
        printf( "Line: (%f, %f) (%f, %f)\n", tmp->ls.endpt[0].x,
                tmp->ls.endpt[0].y, tmp->ls.endpt[1].x, 
                tmp->ls.endpt[1].y);
    }
    printf( "}\n" );
}

/****************************************
 **            CONV_LINESEG            **
 ****************************************/
/* function converts a line segment into a line. The input parameter
   is s. The record, pointed to by l, will be filled with the
   real line equation for the segment.
*/
void convLineseg( lineseg2d_t* s, line2d_t* l )
{
    double dy, dx, m;

    // The change in x and y
    dy = s->endpt[ 0 ].y - s->endpt[ 1 ].y;
    dx = s->endpt[ 0 ].x - s->endpt[ 1 ].x;

    // Test for a vertical line, i.e. dx is approximately zero
    if ( APPROX( dx, 0 ) )
    {
        // Set isdef to false and store the x-intercept in c
        l->isdef = FALSE;
        l->c = s->endpt[ 0 ].x;
        return;
    }

    // Set the slope and y-intercept
    // y = m*x + b
    // b = -m*x + y
    m = dy / dx;
    l->m = m;
    l->b = -1 * m * s->endpt[ 1 ].x + s->endpt[ 1 ].y; 
    l->isdef = TRUE;
}

/****************************************
 **           CONV_TWO_POINTS          **
 ****************************************/
/* function converts two points into a line. The input parameters
   are p1 and p2. The record, pointed to by l, will be filled with the
   real line equation for the segment.
*/
void convTwoPoints( point2d_t* p1, point2d_t* p2, line2d_t* l )
{
    double dy, dx, m;

    // The change in x and y
    dy = p1->y - p2->y;
    dx = p1->x - p2->x;

    // Test for a vertical line, i.e. dx is approximately zero
    if ( APPROX( dx, 0 ) )
    {
        // Set isdef to false and store the x-intercept in c
        l->isdef = FALSE;
        l->c = p1->x;
        return;
    }

    // Set the slope and y-intercept
    // y = m*x + b
    // b = -m*x + y
    m = dy / dx;
    l->m = m;
    l->b = -1 * m * p2->x + p2->y; 
    l->isdef = TRUE;
}
/****************************************
 **            NEW_LINE_REC            **
 ****************************************/
/* function returns a line record in new storage from
   the line segment given in pts.
*/
lineRec2d_t* newLineRec( double* pts, double height, int highpt)
{
    lineRec2d_t* retval;

    if ( !( retval = (lineRec2d_t*) malloc( sizeof( lineRec2d_t ) ) ) )
    {
        return NULL;
    }
  
    // Set the line segment's endpoints
    retval->ls.endpt[ 0 ].x = *pts++;
    retval->ls.endpt[ 0 ].y = *pts++;
    retval->ls.endpt[ 1 ].x = *pts++;
    retval->ls.endpt[ 1 ].y = *pts++;
    retval->height = height;
    retval->trueheight = (double)highpt + height;

    // Set the line information
    convLineseg( &( retval->ls ), &( retval->ln ) );
    retval->next = NULL;
  
    return retval;
}

/****************************************
 **            SET_LINE_REC            **
 ****************************************/
/* function fills a line record with the line segment and real line information */
void setLineRec( double x1, double y1, double x2, double y2, lineRec2d_t* retval )
{
    retval->ls.endpt[ 0 ].x = x1;
    retval->ls.endpt[ 0 ].y = y1;
    retval->ls.endpt[ 1 ].x = x2;
    retval->ls.endpt[ 1 ].y = y2;
    convLineseg( &( retval->ls ), &( retval->ln ) );
    retval->next = NULL;
}

/****************************************
 **        INTERSECT_TWO_LINES         **
 ****************************************/
/* function takes two lines and determines their intersection if
   it exists. On success, the intersection point is stored in space
   pointed to by p. If there is no intersection FAILURE is returned.
*/
int intersectTwoLines( line2d_t* l1, line2d_t* l2, point2d_t* p )
{
    double x, y;
  
    if ( !l1->isdef && !l2->isdef )
    {
        return FAILURE; // Two vertical lines do not intersect
    }
    if (l1->isdef && l2->isdef && APPROX(l1->m, l2->m)) 
    {
        return FAILURE;     // Two parallel lines do not intersect
    }
    if (!l1->isdef)
    {
        x = l1->c;              // Solve for x, which lies on the vertical line l1
        y = l2->m * x + l2->b;  // Solve for y, by inserting x into the line equation for l2
    }
    else if ( !l2->isdef )
    {
        x = l2->c;              // Solve for x, which lies on the vertical line l2
        y = l1->m * x + l1->b;  // Solve for y, by inserting x into the line equation for l1
    }
    else
    {
        // Solve for x by setting line equations equal
        // m1*x + b1 = m2*x + b2
        // m1*x - m2*x = b2 - b1
        // (m1 - m2)*x = b2 - b1
        // x = (b2 - b1)/(m1 - m2)
        x = (l2->b - l1->b) / (l1->m - l2->m);
        // Solve for y, by inserting x into the line equation for l1
        y = l1->m * x + l1->b;
    }

    // Set the point of intersection to p.
    p->x = x;
    p->y = y;
  
    return SUCCESS;
}

/****************************************
 **     IN_BOUNDS_OF_LINE_SEGMENT      **
 ****************************************/
/* function determines if a point on a line
   is within the extent of a segment of that line.
*/
int inBoundsOfLineSegment( point2d_t* p, lineRec2d_t* l )
{
    double sx, sy, lx, ly;

    if ( !l->ln.isdef )
    { 
        // Vertical line bound only in y
        SORT_TWO( l->ls.endpt[0].y, l->ls.endpt[1].y, sy, ly );
        if ( !( p->y >= sy && p->y <= ly ) )
        {
            return FAILURE;
        }
    }
    else if ( APPROX( l->ln.m, 0 ) )
    {
        // Horizontal line bound only in x
        SORT_TWO( l->ls.endpt[ 0 ].x, l->ls.endpt[ 1 ].x, sx, lx );
        if ( !( p->x >= sx && p->x <= lx ) )
        {
            return FAILURE;
        }
    } 
    else
    {
        // Normal line bound in x and in y
        SORT_TWO( l->ls.endpt[ 0 ].x, l->ls.endpt[ 1 ].x, sx, lx );
        SORT_TWO( l->ls.endpt[ 0 ].y, l->ls.endpt[ 1 ].y, sy, ly );
        if ( !( p->x >= sx && p->x <= lx && p->y >= sy && p->y <= ly ) )
        {
            return FAILURE;
        }
    }
    return SUCCESS;
}

/****************************************
 **       INTERSECT_TWO_LINESEGS       **
 ****************************************/
/* function determines if two line segments intersect. If there is an intersection
   SUCCESS is returned and the intersection point is stored in p. Otherwise there
   is no intersection and FAILURE is returned
*/
int intersectTwoLinesegs( lineRec2d_t* r, lineRec2d_t* s, point2d_t* p )
{
    // First intersect the two infinite lines
    if ( !intersectTwoLines( &( r->ln ), &( s->ln ), p ) )
    {
        return FAILURE;
    }

    // the two infinite lines intersected, now check if the point
    // falls within the bounds of the line segment s
    if ( !inBoundsOfLineSegment( p, s ) )
    {
        return FAILURE;
    }
  
    // and then check to see that it falls within the bounds of r
    if ( !inBoundsOfLineSegment( p, r ) )
    {
        return FAILURE;
    }

    return SUCCESS;
}

/****************************************
 **         CARTESIAN_DISTANCE         **
 ****************************************/
/* function returns the cartesian distance between two points in 2-space */
double cartesianDistance( point2d_t* r, point2d_t* s )
{
    return sqrt( ( r->x - s->x ) * ( r->x - s->x ) + ( r->y - s->y ) * ( r->y - s->y ) );
}

/****************************************
 **    SIMULATE_SONAR_LINE_DETECTION   **
 ****************************************/
/* Function calculates intersections for each sonar against the series of line segments stored in lines. 
   Each sonar is a ray originating at the center of the robot and heading in directions 
   distributed regularly about the unit circle. The ray has length given by sonar range and 
   line segment intersection calculations are performed.
   The list of obstacles is returned in new storage.
*/
sim_obstacle_t* simulateSonarLineDetection( point2d_t* robotCenter, double robotHeading, 
					   lineRec2d_t* lines, int numSonar, double maxSonarRange, 
					   int* numObsDetected )
{
    point2d_t inters, best;
    lineRec2d_t ray, *tmp;
    double x, y, bdist, dist;
    int i, count = 0;
    sim_obstacle_t *retval = NULL, *tail;
    double height;

    for ( i = 0; i < numSonar; i++ )
    {
        // Calculate a new endpoint spaced about a circle
        x = robotCenter->x + maxSonarRange * cos( ( robotHeading + 360.0 / numSonar * i ) * M_PI / 180 );
        y = robotCenter->y + maxSonarRange * sin( ( robotHeading + 360.0 / numSonar * i ) * M_PI / 180 );
    
        // Fill the line segment ray with the robot's center and (x,y) as endpoints.
        setLineRec( robotCenter->x, robotCenter->y, x, y, &ray );
  
        // Init tmp and invalidate bdist
        // Find the closest intersection on the line segment ray.
        tmp = lines;
        bdist = maxSonarRange; 
        while ( tmp != NULL )
        {
	  // printf("line!: %f %f    %f %f\n", tmp->ls.endpt[ 0 ].x, tmp->ls.endpt[ 0 ].y, tmp->ls.endpt[ 1 ].x, tmp->ls.endpt[ 1 ].x);
            if ( intersectTwoLinesegs( &ray, tmp, &inters ) )
            {
                // We have an intersection, determine if it is the closest
                dist = cartesianDistance( &inters, robotCenter );
                if ( dist < bdist )
                {
                    bdist = dist;
                    best.x = inters.x;
                    best.y = inters.y;
		    height = tmp->trueheight;
                } 
            }
            tmp = tmp->next;
        }
        if ( bdist < maxSonarRange )
        {
            // There was an intersection, it is stored in best
            count++;
            if ( retval == NULL )
            {
                retval = tail = newSimObstacle( &best, height );
            }
            else
            {
                tail->next = newSimObstacle( &best, height );
                tail = tail->next;
            }
        }
    }

    *numObsDetected = count;
    return retval;
}

/****************************************
 **  CALCULATE_RAY_LINE_INTERSECTIONS  **
 ****************************************/
sim_obstacle_t* calculateRayLineIntersections( point2d_t* origin, double heading, 
					      lineRec2d_t* lines, int* numObsDetected )
{
    point2d_t inters;
    lineRec2d_t ray, *tmp;
    double x, y;
    int count = 0;
    sim_obstacle_t *retval = NULL, *tail;

    // Calculate a new endpoint a large distance away to approximate an infinite ray
    x = origin->x + 1e9 * cos( heading * M_PI / 180 );
    y = origin->y + 1e9 * sin( heading * M_PI / 180 );
  
    // Fill the line segment ray with the robot's center and (x,y) as endpoints.
    setLineRec( origin->x, origin->y, x, y, &ray );

    // Find all intersections on the line segment ray.
    tmp = lines;
    while ( tmp != NULL )
    {
        if ( intersectTwoLinesegs( &ray, tmp, &inters ) )
        {
            // We have an intersection, add it to the list
            count++;
            if ( retval == NULL )
            {
                retval = tail = newSimObstacle( &inters );
            }
            else
            {
                tail->next = newSimObstacle( &inters );
                tail = tail->next;
            }
        }
        tmp = tmp->next;
    }
  
    *numObsDetected = count;
    return retval;
}

/*******************************************
 **  CALCULATELineSegment3DINTERSECTIONS  **
 *******************************************/
sim_obstacle_t* haveLOSwalls( double x1, double y1, double z1,
			      double x2, double y2, double z2,
			      lineRec2d_t* lines, int* numObsDetected )
{
  
  point2d_t inters;
  lineRec2d_t xy, *tmp;
  int count = 0;
  sim_obstacle_t *retval = NULL, *tail;
  
  setLineRec(x1, y1, x2, y2, &xy);  
  
  double xy_dist = sqrt(pow(x2 - x1, 2) + pow(y2-y1, 2));
  double z_slope = (z2 - z1) / xy_dist;
  
  // Find all intersections on the line segment ray.
  tmp = lines;
  while ( tmp != NULL )
    {
      if ( intersectTwoLinesegs( &xy, tmp, &inters ) )
        {
	  
	  double d = sqrt(pow(inters.x - x1, 2) + pow(inters.y -y1, 2));
	  double z_height = z1 + d * z_slope;
	  
	  if (z_height <= tmp->trueheight)
	    {
	      // We have an intersection, add it to the list
	      count++;
	      if ( retval == NULL )
		{
		  retval = tail = newSimObstacle( &inters );
		}
	      else
		{
		  tail->next = newSimObstacle( &inters );
		  tail = tail->next;
		}
	    }
	}
      
      tmp = tmp->next;
    }
  
  *numObsDetected = count;
  return retval;
}

/**********************************************************************
 * $Log: lineutils.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/02/08 19:40:43  pulam
 * Added handling of zones (no-fly, weather)
 * Fixed up some of the icarus objectives to add turn limiting
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/06/08 14:50:21  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
