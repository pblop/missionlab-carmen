/**********************************************************************
 **                                                                  **
 **                        gt_create_world.c                         **
 **                                                                  **
 **                                                                  **
 **  Written by: Douglas MacKenzie                                   **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_create_world.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <math.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#include <vector>

#include "gt_sim.h"
#include "console.h"
#include "gt_create_world.h"
#include "gt_simulation.h"
#include "obstacle.hpp"
#include "robot.hpp"
#include "3d_vis_wid.h"

using std::vector;

double Distance( double x1, double y1, double x2, double y2 )
{
    return vector_length( x1 - x2, y1 - y2 );
}

// Return a random number between 0 and iMax
// inclusive. If iMax < 0, then 0 is returned.
int RandomInt( int iMax )
{
   int iRandNum = 0;

   if ( iMax >= 0 )   // make sure iMax is non-negative
   {
       iRandNum = (int) ((double) iMax * (double) rand() / (double) RAND_MAX);
   }

   return iRandNum;
}

// Find a number between min and max
// inclusive. If iMax < iMin, then iMin is returned.
int RandomIntRange(int iMin, int iMax)
{
    int iRandNum = iMin;

    if ( iMax >= iMin )
    {
        iRandNum = iMin + (int) ( (double) ( iMax - iMin ) * rand() / RAND_MAX );
    }

    return iRandNum;
}

// Get random decimal value to given
// precision
double RandomDoubleRange( double dMin, double dMax )
{
    // get a random number between 0 and range
    double dRandNum = ( (double) rand() / (double) RAND_MAX ) * (dMax - dMin);

    return ( dMin + dRandNum );
}

bool Overlap( double x1, double y1, double r1, double x2, double y2, double r2 )
{
    return ( Distance( x1, y1, x2, y2 ) < ( r1 + r2 ) );
}

void GetRobots( vector<robot*>& clRobots )
{
    clRobots.clear();

    shape* pclShape;
    void* pvPos;
    if ( pvPos = object_list.first( &pclShape ) )
    {
        do
        {
            if( pclShape->is_robot() )
            {
                clRobots.push_back( (robot*) pclShape );
            }
        } while( pvPos = object_list.next( &pclShape, pvPos ) );
    }
}

void fill_region( int iSeed, double dObsDensity, 
                  double dStartX, double dStartY, 
                  double dWidth, double dHeight, 
                  double dMinObsRad, double dMaxObsRad, 
                  obs_t*& psuObs, int& iNumObs )
{
    bool   bOverlap;
    double dVolume;
    int    i;

    double dStopX = dStartX + dWidth;
    double dStopY = dStartY + dHeight;

    srand( iSeed );

    // get the volume in the region so far
    dVolume = 0;
    for ( i = 0; i < iNumObs; i++ )
    {
        if ( ( psuObs[ i ].center.x >= dStartX ) && 
             ( psuObs[ i ].center.x <= dStopX  ) && 
             ( psuObs[ i ].center.y >= dStartY ) && 
             ( psuObs[ i ].center.y <= dStopY  ) )
        {
            dVolume += PI * GTSQR( psuObs[ i ].radius );
        }
    }

    vector<robot*> clRobots;
    GetRobots( clRobots );

    while ( dVolume < ( dObsDensity * ( dWidth * dHeight ) ) )
    {
        int iLoops = 0;
        double dX;
        double dY;
        double dRadius;
	double dHeight;


        do // loop until find one that doesn't overlap
        {
            dX = RandomDoubleRange( dStartX + 1.0, dStopX - 2.0 );
            dY = RandomDoubleRange( dStartY + 1.0, dStopY - 2.0 );
            dRadius = RandomDoubleRange( dMinObsRad, dMaxObsRad );
	    dHeight = (rand()%10) + 1;


            // check for overlap
            bOverlap = false;
            for ( i = 0; ( i < iNumObs ) && !bOverlap; i++ )
            {
                bOverlap = Overlap( psuObs[ i ].center.x, psuObs[ i ].center.y, 
                                    psuObs[ i ].radius, dX, dY, dRadius );

                int j;
                for ( j = 0; ( j < (int)(clRobots.size()) ) && !bOverlap; j++ )
                {
                    /*
                    bOverlap = Overlap(
                        clRobots[ j ]->x_,
                        clRobots[ j ]->y_, 
                        clRobots[ j ]->r_,
                        dX,
                        dY,
                        dRadius );
                    */
                    bOverlap = Overlap(
                        clRobots[ j ]->getX(),
                        clRobots[ j ]->getY(), 
                        clRobots[ j ]->getR(),
                        dX,
                        dY,
                        dRadius );
                }
            }

            iLoops++;

        } while ( bOverlap && ( iLoops < 1000 ) );

        if ( iLoops >= 1000 )
        {
            warn_user("Error: Obstacle density is too high in CREATE_WORLD.\n");
            break;
        }

	if ( create_obstacle( dX, dY, dRadius, dHeight, 
			      &psuObs, &iNumObs ) == GT_SUCCESS )
        {
            dVolume += PI * GTSQR( dRadius );
        }
    }
}



/**********************************************************************
 **          create_obstacle           **
 ****************************************/
int create_obstacle( double x, double y, double radius, double height,
                     obs_t** rtn_obs, int* rtn_num_obs )
{
    obs_t* obs;
    int num_obs;

    // first, reallocate a new one-larger list of obstacles
    if ( *rtn_obs == NULL ) 
    {
        obs = (obs_t*) malloc( sizeof( obs_t ) );
        if ( obs == NULL )
        {
            fprintf( stderr, "Error: Ran out of memory in CREATE_OBSTACLE (malloc failed).\n" );
            return GT_FAILURE;
        }
        num_obs = 0;
    }
    else 
    {
        obs = (obs_t*) realloc( *rtn_obs, ( *rtn_num_obs + 1 ) * sizeof( obs_t ) );
        if ( obs == NULL )
        {
            fprintf( stderr, "Error: Ran out of memory in CREATE_OBSTACLE (realloc failed).\n" );
            return GT_FAILURE;
        }
        num_obs = *rtn_num_obs;
    }

    // fill the data in the new obstacle
    obs[num_obs].center.x = x;
    obs[num_obs].center.y = y;
    obs[num_obs].radius = radius;
    obs[num_obs].has_halo = -1; // No
    obs[num_obs].impacted = -1;	// No
    obs[num_obs].height = height;

    obstacle *p = new obstacle( x, y, radius, height, "black" );
    obs[ num_obs ].pobjShape = p;
    p->draw();
    num_obs++;

    // Add to list of all perceiveable features in the environment
    object_list.append( p );

    *rtn_obs = obs;
    *rtn_num_obs = num_obs;

    if (gt_show_3d_vis)
    {
        initialize_3d_vis();
    }

    return GT_SUCCESS;
}




/**********************************************************************
 * $Log: gt_create_world.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.5  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.4  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.3  2005/08/22 22:10:36  endo
 * For Pax River demo.
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.24  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.23  2002/04/04 18:41:20  blee
 * re-wrote Distance() and Overlap(). Added GetRobots(). Changed
 * create_world() to fill_region() and re-wrote it. Initialized
 * obs[ num_obs ].pobjShape in create_obstacle().
 *
 * Revision 1.22  2001/12/22 16:20:28  endo
 * RH 7.1 porting.
 *
 * Revision 1.21  2000/07/07 17:25:10  blee
 * Changed RandomInt, RandomIntRange, and RandomDoubleRange
 * so that numbers returned are more uniformly random.
 *
 * Revision 1.20  1999/09/27 08:24:49  bbb
 * The RandomInt function caused a coredump once when I was setting
 * up the obstacles. (It did a simple divide by zero error). I fixed
 * it so it can't do that any more. However, in looking at it I realize
 * that the RandomInt function doesn't do what it claims. The comments
 * say it returns a number between 0 and the max passed in (inclusive),
 * but in reality whenever the number 0 comes up, it returns 1.
 *
 * I think someone should chuck RandomInt and rewrite it---it's a simple
 * function but extremely poorly implemented currently.
 *
 * Revision 1.19  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.18  1996/02/20  16:47:27  doug
 * made detect_objects work on the dennings
 *
 * Revision 1.17  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.16  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.15  1995/04/03  20:18:25  jmc
 * Added copyright notice.
 *
 * Revision 1.14  1994/11/08  16:01:00  doug
 * init impacted field in obs array
 *
 * Revision 1.13  1994/11/07  17:22:12  jmc
 * Added the create_obstacle function to create individual obstacles
 * at specific locations.
 *
 * Revision 1.12  1994/11/04  20:31:10  jmc
 * Tweaked an error message in create_world.
 *
 * Revision 1.11  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.10  1994/10/31  16:35:09  jmc
 * Changed warn_userf message for out-of-memory conditions to
 * fprintf(stderr,...) commands since it is unlikely that warn_userf
 * would work with no memory.
 *
 * Revision 1.9  1994/10/26  23:18:15  doug
 * added slider bars and converted to meters
 *
 * Revision 1.8  1994/10/18  20:55:29  doug
 * fixing prototypes
 *
 * Revision 1.7  1994/09/02  20:02:24  jmc
 * Fixed a minor #include ordering problem.
 *
 * Revision 1.6  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.5  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.4  1994/07/26  16:05:42  jmc
 * Changed explicit "extern void warn_user" to include "gt_sim.h".
 *
 * Revision 1.3  1994/07/26  15:52:06  jmc
 * Converted error messages to use error dialog box.
 *
 * Revision 1.2  1994/07/12  19:18:42  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
