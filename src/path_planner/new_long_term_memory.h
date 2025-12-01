/**********************************************************************
 **                                                                  **
 **                  new_long_term_memory.h                          **
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

/* $Id: new_long_term_memory.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef LT_MEMORY_H
#define LT_MEMORY_H
/*---------------------------------------------------------------------------

			NEW LONG TERM MEMORY.H
			include increased stack elements
		Header file for processes accessing LTM

----------------------------------------------------------------------------*/

#include <stdio.h>

#include "clipstruct.h"
#include "vertex.h"

#define DEFAULT_FILE_NAME ".dat"

#define MAX_VERTICES 12000	/* Limit to number of vertices in LTM */
#define MAX_STACK_ELEMENTS 700	/* Maximum number of Convex regions */
#define MAX_VERTEX_FEATURES 350	/* Maximum Number of vertices that can have */
#define MAX_OBSTACLES 100 	/* Max # of obstacles */

#define MAX_TERRAIN_IDS 40	/* Maximum different terrain types */
#define MAX_ERROR_FACTS 40	/* Same Number of terrain error data */
#define MAX_3D_POINTS 500

/* All allocation of LTM is handled by pseudo-malloc functions */

/* LONG TERM MEMORY DEFINITION */

struct long_term_memory_t
{
   struct stack_element         se[MAX_STACK_ELEMENTS]; /* Storage for stack elements */
   struct vertex_list_element   vle[MAX_VERTICES];	/* Storage for Vertices */
   struct obstacle_list_element obs[MAX_OBSTACLES];	/* Storage for obstacles */
   struct stack_element         *next_stack_element;    /* Storage management pointers */
   struct vertex_list_element   *next_vertex;
   struct obstacle_list_element *next_obstacle;
   struct vertex_features ver_feat[MAX_VERTEX_FEATURES];/* Storage for vertex features */
   int                    next_vertex_feature;
   struct terrain_identifier terrain[MAX_TERRAIN_IDS];	/* Storage for terrain IDs */
   int                       next_terrain_id;
   struct error_facts err_facts[MAX_ERROR_FACTS];	/* Storage for terrain error data */
   int                next_error_facts;
   struct stack_element *top_of_convex_region_stack;	/* Pointer to top of convex region stack */
   struct obstacle_list_element *top_of_obstacles;       /* pointer to top of obstacle stack */

   /* new additions */
   struct point_3d p_3d[MAX_3D_POINTS];
   int             next_model_3d;
   int             next_point_3d;

};

extern struct long_term_memory_t long_term_memory;

/*****************************************************/
void            initialize_long_term_memory(void);
void            save_long_term_memory(void);
void            load_long_term_memory(void);

#endif

/**********************************************************************
 * $Log: new_long_term_memory.h,v $
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
