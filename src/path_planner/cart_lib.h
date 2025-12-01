/**********************************************************************
 **                                                                  **
 **                               cart_lib.h                          **
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

/* $Id: cart_lib.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef CART_LIB_H
#define CART_LIB_H

/*******************************************************************************

			cart_lib.h

		NAVIGATOR - CARTOGRAPHER functions

*******************************************************************************/

#include "new_long_term_memory.h"

/* eventually, these should read the overlay. */
extern double g_StartX;
extern double g_StartY;


int            verify_links(struct stack_element * map);
void            edit_model_data(void);
void            print_model_data(void);
void            free_path(struct path_node * path);
int             two_in_region(struct vertex_list_element * vertex);
double          round_float(double x);
void            round_region(struct stack_element * stack);
void            round_border(struct vertex_list_element * border);
void            round_obstacle(struct obstacle_list_element * obstacle);
int             same_region(struct vertex_list_element * v1, struct vertex_list_element * v2);
struct vertex_list_element *grow(struct vertex_list_element * p2,double dist);
int             concave(struct vertex_list_element * vertex);
struct vertex_list_element *
add_vertex(
	   struct vertex_list_element * location,	/* pointer to end of
							 * list */
	   struct vertex_list_element * vertex);	/* pointer to vertex	  */
double
find_length(struct vertex_list_element * pt1,
	    struct vertex_list_element * pt2);
void            print_list(struct vertex_list_element * pointer);
void            back_print_list(struct vertex_list_element * pointer);
double
compute_angle(struct vertex_list_element * pt1,
	      struct vertex_list_element * pivot,
	      struct vertex_list_element * pt3);
int
inclusive_correctside(struct vertex_list_element * pt1,
		      struct vertex_list_element * pt2,
		      struct vertex_list_element * pt3);
int
correctside(struct vertex_list_element * pt1,
	    struct vertex_list_element * pt2,
	    struct vertex_list_element * pt3);
void
perp(
     struct vertex_list_element * p1,	/* points on line */
     struct vertex_list_element * p2,	/* points on line */
     struct vertex_list_element * p3perp);	/* output for perpendicular */
int
checknointersect(struct vertex_list_element * pt1,
		 struct vertex_list_element * pt2,
		 struct vertex_list_element * region_ptr);
int
intersect(struct vertex_list_element * l1p1,
	  struct vertex_list_element * l1p2,
	  struct vertex_list_element * l2p1,
	  struct vertex_list_element * l2p2);
int
true_lintersect(struct vertex_list_element * l1p1,
		struct vertex_list_element * l1p2,
		struct vertex_list_element * l2p1,
		struct vertex_list_element * l2p2);
int
lintersect(struct vertex_list_element * l1p1,
	   struct vertex_list_element * l1p2,
	   struct vertex_list_element * l2p1,
	   struct vertex_list_element * l2p2);
double
same(struct vertex_list_element * p1,
     struct vertex_list_element * p2,
     struct vertex_list_element * q1,
     struct vertex_list_element * q2);
int
colinear(struct vertex_list_element * l1p1,
	 struct vertex_list_element * l1p2,
	 struct vertex_list_element * l2p1,
	 struct vertex_list_element * l2p2);

int
obstacle_checknointersect(struct vertex_list_element * pt1,
			  struct vertex_list_element * pt2,
			  struct vertex_list_element * region_ptr,
			  struct vertex_list_element ** return_vertex);
struct vertex_list_element *copy_region(struct vertex_list_element * region);
int
line_segment_collides(struct path_node * p1,
		      struct path_node * p2,
		      struct stack_element * region_list);
int
plus_line_segment_collides(struct path_node * p1,
			   struct path_node * p2,
			   struct stack_element * region_list);

double path_length(struct path_node * path);
double path_cost(struct path_node * path);

void print_obstacles(struct obstacle_list_element * obstacle_list);

void grow_obstacles(struct obstacle_list_element * list_head, double length);

void
attach_obstacles(struct obstacle_list_element * obstacle_list,
		 struct stack_element * border_list);
double
find_closest_clear(struct vertex_list_element * vertex_list,
		   struct vertex_list_element * border_vertex_list,
		   struct obstacle_list_element * obstacle_list,
		   struct vertex_list_element ** ret_obs,
		   struct vertex_list_element ** ret_bor);

struct vertex_list_element *
merge_obstacle(
	       struct vertex_list_element * border_list,
	       struct vertex_list_element * obs,
	       struct vertex_list_element * bor);
void            patch_for_obstacles(struct stack_element * convex_list);

void            patch_region(struct vertex_list_element * head);
int obstacle_checknointersect2(struct vertex_list_element * pt1,
			   struct vertex_list_element * pt2,
			   struct vertex_list_element * region_ptr);
void            fixup_overlap_border(struct stack_element * stack);
void
cross_vertex(struct vertex_list_element * vertex,
	     struct vertex_list_element * line);
int
line_equation(struct vertex_list_element * p1,
	      struct vertex_list_element * p2,
	      double *slope,
	      double *intercept);

void            fixup_overlap_obstacle(struct obstacle_list_element * obstacle);

void            grow_region(struct stack_element * stack_top, double length);
void
merge_overlap_obstacles_border(struct obstacle_list_element * obstacles,
			       struct stack_element * border_stack);
void            merge_obstacle_to_obstacle(struct obstacle_list_element * obstacles);
void
improve_path(struct path_node * path,
	     struct stack_element * convex_region_list,
	     double safety_margin);
void improve_path2(struct path_node     *path,
		   struct stack_element *convex_region_list);
void
improve_path3(struct path_node * path,
	      struct stack_element * convex_region_list);
void
relax_path(struct path_node * path_start,
	   double safety_margin,
	   struct stack_element * region_list,
	   double relax_increment_size);

struct path_node * a_star(struct stack_element *map,
			  struct vertex_list_element *start,
			  struct vertex_list_element *goal);

struct path_node * a_star3(struct stack_element *map,
			   struct vertex_list_element *start,
	                   struct vertex_list_element * goal,
	                   double safety_margin);

void            clear_display(void);
int             slop_convex(struct vertex_list_element * list);
void
merge_with_border(struct vertex_list_element * obstacle_collide,
		  struct vertex_list_element * border_collide);

int
inside_meadow(struct vertex_list_element * pt,
	      struct vertex_list_element * meadow_list);
void
add_to_open(struct vertex_list_element * vertex,
	    struct open_node * open_list,
	    struct vertex_list_element * goal,
	    struct stack_element * map);
void
add_goal_to_open(struct open_node * open_list,
		 struct vertex_list_element * goal,
		 struct stack_element * map);
int
plus2_line_segment(struct path_node * p1,
		   struct path_node * p2,
		   struct stack_element * region_list);
void
straighten_the_path(struct path_node * path,
		    struct path_node * goal_path_point,
		    struct stack_element * convex_region_list);
void
                identify_region(struct stack_element * region_list);

void
edit_region(struct stack_element * region_list,
	    int region_number);
void            print_vertex_data(struct vertex_list_element * vertex);
void            add_vertex_features(struct vertex_list_element * vertex);
void            print_vertex_features(struct vertex_list_element * vertex);
void            review_freespace_data(struct stack_element * current, int region_number);
void            edit_terrain_data(struct stack_element * current);
void            print_terrain_data(struct stack_element * current);
void 
add_to_open3(struct vertex_list_element * vertex,
	     struct open_node * open_list,
	     struct vertex_list_element * goal,
	     struct stack_element * map,
	     double safety_margin);
void 
straighten_the_path3(struct path_node * path,
		     struct path_node * goal_path_point,
		     struct stack_element * convex_region_list);
int 
plus5_line_segment(struct path_node * p1,
		   struct path_node * p2,
		   struct stack_element * region_list);

void remove_redundant_vertices(struct stack_element * regions);
struct vertex_list_element *
find_meadow( struct vertex_list_element *pt,struct stack_element *meadow_stack);


#endif

/**********************************************************************
 * $Log: cart_lib.h,v $
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
