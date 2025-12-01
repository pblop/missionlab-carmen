/**********************************************************************
 **                                                                  **
 **                        gt_create_world.h                         **
 **                                                                  **
 **  Written by: Douglas MacKenzie                                   **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_create_world.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef CREATE_WORLD_H
#define CREATE_WORLD_H

#include "gt_simulation.h"
#include "shape.hpp"

struct obs_t
{
    gt_Point center;
    float    radius;

    short    has_halo;	// robot_id that drew halo and will remove it
    float    halo_radius;
    float    height;
    float    trueheight;

    short    impacted;	// robot_id that drew impact and will remove it
    float    impact_radius;
    shape*   pobjShape;
};

double Distance( double x1, double y1, double x2, double y2 );

void fill_region( int iSeed, double dObsDensity, 
                  double dStartX, double dStartY, 
                  double dWidth, double dHeight, 
                  double dMinObsRad, double dMaxObsRad, 
                  obs_t*& psuObs, int& iNumObs );


int create_obstacle( double x, double y, double radius, double height,
		     obs_t** rtn_obs, int* rtn_num_obs );




#endif



/**********************************************************************
 * $Log: gt_create_world.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.11  2002/04/04 18:40:38  blee
 * Added pobjShape to obs_t and changed create_world() to fill_region().
 *
 * Revision 1.10  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.9  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.8  1995/04/03  20:19:15  jmc
 * Added copyright notice.
 *
 * Revision 1.7  1994/11/08  16:01:00  doug
 * added impacted fields in obs array
 *
 * Revision 1.6  1994/11/07  21:00:48  jmc
 * Added a function prototype for Distance (which was already
 * implemented in gt_create_world.c).
 *
 * Revision 1.5  1994/11/07  17:22:31  jmc
 * Added function prototype for create_obstacle.
 *
 * Revision 1.4  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.3  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.2  1994/07/12  19:20:28  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
