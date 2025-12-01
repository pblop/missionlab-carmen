/**********************************************************************
 **                                                                  **
 **                               nav.c                              **
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

/* $Id: nav.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/*******************************************************************************

				NAVIGATOR
			
*******************************************************************************/

/*--------------------------------------------------------------------------*/

/* preliminaries                   */

/*------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "std.h"
#include "clipstruct.h"
#include "new_long_term_memory.h"
#include "short_term_memory.h"
#include "map_display.h"
#include "cart_lib.h"
#include "nav.h"




/* Global Data Structures */

int             straighten_path = 1;  /* 1 is straighten, 0 otherwise */
int             search_method;

   /* external global variables		                 */
   extern int      search_method;


/*---------------------------------------------------------------------------

				navigator

			Accepts user data then produces path

---------------------------------------------------------------------------*/
struct path_node *
navigator(double start_x, double start_y, double start_z,
	  double goal_x,  double goal_y,  double goal_z,
	  int display, double relax_increment_size,
	  double safety_margin)
{
   double length;
   double cur_dist;
   double delta_z;
   struct path_node *cur_point;
   struct vertex_list_element p1, p2;
   int improve_method = 0;

   /* RRB */

   /*   struct path_node *temp_path;
	int temp_num = 0;
	char garb[120];
   */

   /* stack variables & functions			 */
   struct stack_element *convex_region_stack;
   struct vertex_list_element start, goal;
   struct path_node *path;

   /* Main Planning Loop */

   convex_region_stack = long_term_memory.top_of_convex_region_stack;

   /* setup start X,Y location */
   start.x        = start_x;
   start.y        = start_y;
   start.next     = NULL;
   start.passable = NULL;

   if (find_meadow(&start, convex_region_stack) == NULL)
   {
      printf("\nstarting X=%5.2f Y=%5.2f not in free space\n",start.x, start.y);
      return NULL;
   }

   /* x and y given in global coordinates */
   short_term_memory.robot.global_base_x = start.x;
   short_term_memory.robot.global_base_y = start.y;

   /* setup goal X,Y location */
   goal.x        = goal_x;
   goal.y        = goal_y;
   goal.next     = NULL;
   goal.passable = NULL;

   if (find_meadow(&goal, convex_region_stack) == NULL)
   {
     printf("\ngoal X=%5.2f Y=%5.2f not in free space\n", goal.x, goal.y);
     return NULL;
   }

   search_method = 3;

   if (search_method == 1)
      path = a_star(convex_region_stack, &start, &goal);
   else if (search_method == 3)
      path = a_star3(convex_region_stack, &start, &goal, safety_margin);

#if 0
   /******** RRB*/
   temp_path = path;
   temp_num = 0;
   while(temp_path != NULL){
     printf("%d - %5.2f %5.2f\n", temp_num++, temp_path->x, temp_path->y);
     temp_path = temp_path->next_point;
   }
   /*
   printf("press a key ");
   scanf("%s ",garb);
   */
   /*************/
#endif

/* Removed to prevent incorrect paths when U turns are necessary */

   //   search_method = 1;

   improve_method = 1;
   if (straighten_path)
    {
      switch(improve_method){
      case 1:
	improve_path(path, convex_region_stack, safety_margin);
	break;
      case 2:
        improve_path2(path, convex_region_stack);
	break;
      case 3:
        improve_path3(path, convex_region_stack);
	break;
      default:
	printf("nav.c: navigator -- need a valid improve_method index\n");
	exit(0);
      }

     relax_path(path, safety_margin, convex_region_stack, relax_increment_size);

    }

/**/

   /* Use linear interpolation to set the Z values of the via points */
   length = path_length(path);
   if(length == 0)
   {
      path->z = start_z;
   }
   else
   {
      cur_point = path;
      cur_dist = 0;
      delta_z = (goal_z - start_z);
      while(cur_point != NULL)
      {
         cur_point->z = start_z + (delta_z * cur_dist / length);
         if(cur_point->next_point != NULL)
         {
            p1.x = cur_point->x;
            p1.y = cur_point->y;
            p2.x = cur_point->next_point->x;
            p2.y = cur_point->next_point->y;
            cur_dist += find_length(&p1, &p2);
         }
         cur_point = cur_point->next_point;
      }
   }

   short_term_memory.path = path;

   if(display)
   {
      display_path(path);
   }

   /* return pointer to via-point list, starting at 1st via-point. */
   /* skip start location, since are already there */
   if(path == NULL) return NULL;

   return path->next_point;
}





///////////////////////////////////////////////////////////////////////
// $Log: nav.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/10 21:51:16  pulam
// Fixed pathplanner
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:40:10  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

