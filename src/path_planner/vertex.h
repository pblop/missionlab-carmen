/**********************************************************************
 **                                                                  **
 **                               vertex.h                           **
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

/* $Id: vertex.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/*----------------------------------------------------------------------------

			vertex.h

	Data structures for cartographer and planner

----------------------------------------------------------------------------*/

/* definition for a 2D vertex	 */
struct vertex_t
{
   double x;
   double y;
};
typedef struct vertex_t VERTEX;


/* two way linked list          */

struct vertex_list_element
{
   double          x;		/* x coordinate */
   double          y;		/* y coordinate */
   struct vertex_list_element *previous;	/* Pointer to previous vertex            */
   struct vertex_list_element *next;	        /* Pointer to next vertex in list        */
   struct vertex_list_element *passable;	/* pointer to adjacent free
						 * space region - NULL if impassable     */
   struct vertex_list_element *corresponding;	/* pointer to associated vertex 
						 * (e.g. grown vertex -> original vertex */
   struct vertex_features     *features;        /* Pointer to feature list for a
					         * vertex, NULL if no features           */
   int looked_at;	                    /* whether the vertex has been explored  KSA */
};

/*---------------------------------------------------------------------------*/

/* Features for a vertex - Expand as necessary 	 	     */

struct vertex_features
{
   /* Pop - up vertex */
   double          up_x;
   double          up_y;
   double          up_z;

   double          length;	/* length of side line to top */

   /* next vertex after up vertex */
   double          side_x;
   double          side_y;
   double          side_z;
};

/*----------------------------------------------------------------------------*/

/* Definition for a stack element - used for region lists 	              */

struct stack_element
{
   struct vertex_list_element *region;	     /* Points to linked list of vertices */
   struct stack_element       *next;	     /* Pointer to next stack element     */
   struct terrain_identifier  *terrain_type; /* Points to terrain characteristics */
};



/*---------------------------------------------------------------------------
	
			Model-3d

		Stub for now
---------------------------------------------------------------------------*/

struct model_3d
{
   struct point_3d *point_list;
};

struct point_3d
{
   double          x;
   double          y;
   double          z;
   struct point_3d *next_point;
};

/*---------------------------------------------------------------------------*/

/* definition for obstacle list - contains pointer to original obstacle */

struct obstacle_list_element
{
   struct vertex_list_element *grown_obstacle;	/* List of vertices for 
						   grown obstacle */
   struct vertex_list_element *actual_obstacle;	/* List of vertices for
						 * original obstacle */
   int             status;	/*  Used when growing obstacle - denotes
				 *  location relative to world map
				 * -1 = outside border
				 *  0 = unknown (inital state)
				 *  1 = inside border
				 *  2 = merged into border
				 *  3 = merged with earlier inside obstacle
				 */

   struct obstacle_list_element *next;	/* Used to form obstacle lists */
};

/*----------------------------------------------------------------------------*/

/* definition for open node for A* search list */

struct open_node
{
   double          x;	         /* adit x */
   double          y;	         /* adit y */
   double          g;            /* non-heuristic distance */
   double          f;	         /* total cost */
   struct vertex_list_element *region;  /* first vertex of segment */
   struct open_node           *parent;  /* pointer to node above in search tree */
   struct terrain_identifier *terrain;  /* Indicates terrain type - used in
					 * cost function */
   struct open_node *next_onode; /* pointer to next open node for expansion */
};

/*---------------------------------------------------------------------------*/

/* definition for closed node for A* search list */

struct closed_node
{
   double          x1;		    /* x of vertex 1 (not adit)           */
   double          y1;		    /* y of vertex 1                      */
   double          x2;		    /* x of vertex 2 (not adit)           */
   double          y2;		    /* y of vertex 2                      */
   struct closed_node *next_cnode;  /* Points to next node on closed list */
};

/*----------------------------------------------------------------------------*/

/* definition for path node */
/* Produced by navigator after A* search completed */

struct path_node
{
   double          x;		/* x coordinate */
   double          y;		/* y coordinate */
   double          z;		/* z coordinate */
   struct vertex_list_element *region1;	/* region bordering path point */
   struct vertex_list_element *region2;	/* second region */
   struct terrain_identifier *terrain_type;	/* terrain required to cross
						 * to get here */
   struct path_node *next_point;/* Next point on path */
};

/*----------------------------------------------------------------------------*/

/* Data identifying terrain characteristics */

struct terrain_identifier
{
   struct error_facts *error_data;	/* pointer to statistical error data
					 * structure */
   double          traversal_factor;	/* Relative cost to travel on - 1 is
					 * optimal (must be >= 1) */
   int             transition_zone;	/* TRUE if transition zone - FALSE
					 * otherwise */
};

/*---------------------------------------------------------------------------*/

/* Statistical data characterizing terrain traversability */

struct error_facts
{
   /* Translational Errors */
   double          average_trans_error;	/* Average translation error */
   double          stan_dev_trans;	/* Standard deviation of
					 * translationaal error */
   double          base_trans_error;	/* Base value associated with any
					 * translation - start up */
   /* Rotational Errors */
   double          average_rot_error;	/* Average translation error */
   double          stan_dev_rot;/* Standard deviation of translationaal error */
   double          base_rot_error;	/* Base value associated with any
					 * translation - start up */
};

/*---------------------------------------------------------------------------*/

/* Used for relaxing Path during improve_path process in navigator */

struct relax_path_node
{				/* for transition zone relaxations */
   struct path_node *point1;	/* first point in transition zone */
   struct path_node *point2;	/* second point in transition zone */
   struct path_node *check_point1;	/* point to check with point 1 for
					 * collision */
   struct path_node *check_point2;	/* point to check woth point 2 for
					 * collision */
   double          delta_x;	/* increment to move (relax) in x directon */
   double          delta_y;	/* increment to move (relax) in y direction */
   /* old values for point 1 and point 2 follow */
   double          hold1_x;
   double          hold1_y;
   double          hold2_x;
   double          hold2_y;
   /* essentially side limit switch values */
   double          minus_limit_x;	/* limit for sliding with minus */
   double          minus_limit_y;	/* limit for sliding with minus */
   double          plus_limit_x;/* limit for sliding with plus */
   double          plus_limit_y;/* limit for sliding with plus */
   struct relax_path_node *next;/* pointer to next node */
};

/**********************************************************************
 * $Log: vertex.h,v $
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
 **********************************************************************/
