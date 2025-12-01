/**********************************************************************
 **                                                                  **
 **                               map_bldr.h                          **
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

/* $Id: map_bldr.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef MAP_BUILDER_H
#define MAP_BUILDER_H

/***********************************************************/
/*                map_builder.h                            */
/***********************************************************/

void load_map(char *filename,int display,double robot_radius, double robot_safety_margin);
void load_obstacle(int num,Point vertices[],double grow_length);
void map_builder(int display,int convex_select_mode,int concave_angle_mode);

struct stack_element *malloc_stack_element(void);
struct vertex_list_element * malloc_vertex(void);
struct obstacle_list_element *malloc_obstacle_list_element(void);
void free_vertex(struct vertex_list_element * address);
#endif

/**********************************************************************
 * $Log: map_bldr.h,v $
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
