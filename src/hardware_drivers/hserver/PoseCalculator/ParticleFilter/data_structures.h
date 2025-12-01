/**********************************************************************
 **                                                                  **
 **                     data_structures.h                            **
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

/* $Id: data_structures.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef DATA_STRUCTURES
#define DATA_STRUCTURES

#include <sys/time.h>
#include <unistd.h>

#define NSCANS 361  /* Number of Laser reading per scan*/
#define MAX_VALID_READING 6000


#define EPS (1e-6)
#define SUCCESS 1
#define FAILURE 0
#define TRUE 1
#define FALSE 0

typedef unsigned char BOOL;


/**************************************************************************/
/*   GEOMETRIC STRUCTURES                                                 */
/**************************************************************************/

typedef struct point2d
{
  /* A point in two dimensions */
  double x;
  double y; 
} point2d_t;


typedef struct lineseg2d
{
  /* A line segment in two dimensions*/
  double x1, y1;
  double x2, y2;
} lineseg2d_t;



typedef struct line2d
{
  /* A line in two dimensions. */ 
  /* We use a polar representation of the line.  */
  double r;     /* perpendicular distance from the origin to the line */
  double theta;   /* angle between the line and the positive coordinate axis */
  double x, y; /* coordinates of perdictular intersection point */
  double Vx, Vy; /* Vector along the line */
} line2d_t;





/**************************************************************************/
/*   SENSOR RELATED STRUCTURES                                            */
/**************************************************************************/


typedef struct rawLaserScan
{
  /* Raw laser scan data */
  unsigned int dist[NSCANS];
  float x, y, t;
  struct timeval tstamp;
} rawLaserScan_t;

typedef struct laserScan
{
  /*  Laser scan data in Carthesian Coordinates */
  double x[NSCANS];
  double y[NSCANS];
} laserScan_t;

typedef struct 
{
  double x[NSCANS];
  double y[NSCANS];
  double rho[NSCANS];
  double phi[NSCANS];
  
  double winningX[NSCANS];
  double winningY[NSCANS];
  double winningRho[NSCANS];
  double winningPhi[NSCANS];
  
  int num; //number of entries <= NSCANS
  int winningNum;
} tanfitRec_t;




/**************************************************************************/
/*   ROBOT LOCATION                                                       */
/**************************************************************************/

typedef struct pose
{
  /*  Robot pose */
  double x;
  double y;
  double theta;
} pose_t;


/* MAP */

#define MAX_MAP_WALLS 150
typedef struct 
{
  lineseg2d_t walls[MAX_MAP_WALLS];
  int nwallsInMap;
} map_t;


#endif

/**********************************************************************
 * $Log: data_structures.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/05 21:35:54  endo
 * PoseCalculator from MARA 2020 migrated
 *
 * Revision 1.1  2004/09/10 20:02:53  endo
 * New PoseCalc integrated.
 *
 * Revision 1.1  2000/08/12 18:45:51  saho
 * Initial revision
 *
 **********************************************************************/
