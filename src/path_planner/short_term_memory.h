/**********************************************************************
 **                                                                  **
 **                               short_term_memory.h                          **
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

/* $Id: short_term_memory.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/*-----------------------------------------------------------------------------

			Short Term Memory.H

	Assumes include files already present in long term memory.h
		(Newvertex.h, stdio.h, rms files, etc.)

------------------------------------------------------------------------------*/

#define FREE_SLOT 0
#define RESIDENT 1
#define NOT_RECLAIMED 2
#define MAX_VERTICES_IN_SLOT 20
#define MAX_SHORT_TERM_MEMORY_SLOTS 50
#define MAX_NUMBER_OF_PATH_POINTS 75

/*---------------------------------------------------------------------------*/

/* Struct for robot data - e.g. position */
struct mobile_robot
{
   double          current_theta;	/* actual orientation */
   double          global_base_x;	/* x position relative to global map
					 * (not directly from getxy) */
   double          global_base_y;	/* y position relative to global map
					 * (not directly from getxy) */
   double          ultra_sensor_0_angle;	/* orientation of sensor 0
						 * relative to global map */
};

/*---------------------------------------------------------------------------*/

/* Data structure for holding instantiated meadows */

struct slot_element
{
   struct vertex_list_element region[MAX_VERTICES_IN_SLOT];	/* unfortunately must be
								 * worst case */

   /* include other copyable and changeable data here */

   struct stack_element *ltm_id;/* address of meadow in long_term_memory */
   int             resident;	/* 1 resident, 0 free, 2 not needed but not
				 * reclaimed */
   struct slot_element *next_slot;	/* Pointer to next slot (for lists) */
};

/*---------------------------------------------------------------------------*/

/* Holds Routes (paths) developed by navigator */

struct route_list
{
   struct path_node path_point[MAX_NUMBER_OF_PATH_POINTS];	/* list of points on a
								 * path */
   struct path_node *current_leg_start;	/* which leg of path is being
					 * currently executed */
   int             route_deposited;	/* TRUE when navigator puts route in,
					 * FALSE otherwise */
   int             route_completed;	/* TRUE when pilot completes route,
					 * FALSE otherwise */
};

/*---------------------------------------------------------------------------*/

/* SHORT TERM MEMORY DECLARATION */
struct short_term_memory_t
{
   struct slot_element slot[MAX_SHORT_TERM_MEMORY_SLOTS];	/* instantiated meadow
								 * list */
   struct slot_element *first_slot_used;	/* points to first meadow */
   struct slot_element *free_slot_list;	/* used for allocation of slots */
   struct route_list route;	/* path from navigator */
   struct path_node    *path;	/* dcm: path from navigator */

   /* bounding rectangle based on meadows (global coordinates) */
   double          lower_left_corner_x;
   double          lower_left_corner_y;
   double          upper_right_corner_x;
   double          upper_right_corner_y;

   struct mobile_robot robot;	/* robot frame of reference data */
};

extern struct short_term_memory_t short_term_memory;



///////////////////////////////////////////////////////////////////////
// $Log: short_term_memory.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:40:17  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

