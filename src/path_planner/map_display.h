/**********************************************************************
 **                                                                  **
 **                               map_display.h                          **
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

/* $Id: map_display.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef MAP_DISPLAY_H
#define MAP_DISPLAY_H

/***********************************************************/
/* map_display.h                            */
/***********************************************************/

#include "new_long_term_memory.h"

#define BLACK  (CNUM[0])
#define WHITE  (CNUM[1])
#define RED    (CNUM[2])
#define GREEN  (CNUM[3])
#define BLUE   (CNUM[4])
#define YELLOW (CNUM[5])
#define CYAN   (CNUM[6])

#define NUM_COLORS 7

extern unsigned long CNUM[NUM_COLORS];

#define COLOR_FOREGRND WHITE
#define COLOR_BACKGRND BLACK
#define COLOR_WALL     RED
#define COLOR_GROWN    GREEN
#define COLOR_PASSABLE BLUE 
/* #define COLOR_PASSABLE WHITE */
#define COLOR_PATH     BLACK
#define COLOR_OBSTACLE BLACK

#define DRAW_PASSABLES 0
void step_display(struct stack_element *convex_region_stack);
void display_single_region(struct vertex_list_element *vertex);



void  open_map_display(void);
void  close_map_display(void);
void  clear_map_display(void);
void  display_regions(struct stack_element * current_convex_region);
void  display_passages(struct stack_element * current_convex_region);
void  display_grown(struct vertex_list_element * border, struct vertex_list_element * grown);
void  display_obstacles(struct obstacle_list_element * obstacles);
void  display_obstacles2(struct vertex_list_element * border, struct obstacle_list_element * obstacles);
void display_path(struct path_node * path);
void display_pathonly(struct stack_element * current_convex_region, struct path_node * path);
void display_id_region(struct stack_element * current_convex_region, int region_number);
void display_one_region(struct vertex_list_element * region);
void  set_display_scale(struct vertex_list_element * border);
int pick_xy(int free_space,double *map_x,double *map_y);
void drawline(double x1, double y1, double x2, double y2, int draw_color);
void drawcircle(double x, double y, double r, int draw_color);
int mouse_read(); /* added by SAHO */
#endif

/**********************************************************************
 * $Log: map_display.h,v $
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
