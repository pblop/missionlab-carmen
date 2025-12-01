/**********************************************************************
 **                                                                  **
 **                               map_bldr.c                          **
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

/* $Id: map_bldr.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/* #define DEBUG */

#define STEP_DEBUG 0
#define DEBUG_MAP 0



/*******************************************************************************

				MAP BUILDER

	Independent Process Used to Construct Long-term-memory

*******************************************************************************/


/*-----------------------------------------------------------*/

/* preliminaries                   */

/*-----------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "std.h"
#include "clipstruct.h"
#include "new_long_term_memory.h"
#include "cart_lib.h"
#include "nav.h"
#include "map_display.h"
#include "map_bldr.h"

#ifdef TRANSPUTER
#include "map_data.h"
#endif

// ENDO - gcc-3.4
extern struct stack_element *check_merge(
    struct stack_element *start,
    struct stack_element *stack_header,
    int *merged);

// ENDO - gcc-3.4
extern struct stack_element *terrain_patch(
    struct stack_element *convex_list,
    struct stack_element *temp_list);

/* Global Data Structures */
static int      stack_count = 0;
static int      vertex_count = 0;
static char     msg[80];

/* RRB */
void list_obstacles(void){
  struct obstacle_list_element *obs = long_term_memory.top_of_obstacles;
  struct vertex_list_element *head, *cur;
  int obsctr = 0;

  while(obs){
    head = obs->grown_obstacle->next;
    cur = head;
    printf("obs %d\n", obsctr++);
    do{
      printf(" %5.2f %5.2f \n", cur->x, cur->y);
      cur = cur->next;
    } while (cur != head);

    obs = obs->next;
  } // while obs
}



/*---------------------------------------------------------------------------


				add_one_obstacle

	subroutine to add one obstacle to the map
	Vertices must be ordered in CLOCKWISE direction!
	Attaches them to border list and returns new border

----------------------------------------------------------------------------*/

static struct obstacle_list_element *
add_one_obstacle(struct stack_element * border_list,
		 int number_vertices, Point vertices[], double length)
{
   int i;
   struct obstacle_list_element *obstacle_list_head, *next_obstacle;
   struct vertex_list_element *next_vertex;

   /* initialize obstacle list */
   obstacle_list_head = malloc_obstacle_list_element();
   obstacle_list_head->next = NULL;
   obstacle_list_head->actual_obstacle = malloc_vertex();
   obstacle_list_head->actual_obstacle->features = NULL;

   /* input obstacles */
   next_obstacle = obstacle_list_head;
   next_obstacle->status = 0;	/* initially unknown */

   next_vertex = next_obstacle->actual_obstacle;

   for (i = 0; i < number_vertices; i++)
   {
      /* must be strictly positive */
      if (vertices[i].x < 0 || vertices[i].y < 0)
      {
	 sprintf(msg, "map_builder:add_one_obstacle - vertices must be positive\n");
	 puts(msg);
	 exit(1);
      }

      next_vertex->x = vertices[i].x;
      next_vertex->y = vertices[i].y;
      next_vertex->corresponding = NULL;
      next_vertex->passable = NULL;
      next_vertex->looked_at = 0;

      /* do all but last one */
      if (i < number_vertices - 1)
      {
	 next_vertex->next = malloc_vertex();
	 next_vertex->next->features = NULL;
	 next_vertex->next->previous = next_vertex;
	 next_vertex = next_vertex->next;
      }
   }

   /* tie off obstacle (complete circular list) */
   next_obstacle->actual_obstacle->previous = next_vertex;
   next_vertex->next = next_obstacle->actual_obstacle;

   /* grow obstacles */
   grow_obstacles(obstacle_list_head, length);

   merge_overlap_obstacles_border(obstacle_list_head, border_list);

   merge_obstacle_to_obstacle(obstacle_list_head);

   /* connect obstacles to border */
   attach_obstacles(obstacle_list_head, border_list);

   /* return obstacle */
   return (obstacle_list_head);
}



/*
 * ------------------------------------------------------------------------
 * 
 * FIND_CONCAVE
 * 
 * finds most concave angle in a given region. Returns a pointer to the vertex
 * after the pivot of the angle Sets anchors of the angle in pt1 and pt2
 * 
 * concave_angle_mode:  1=most, 2=least, 3=first
 * ------------------------------------------------------------------------
 */

static struct vertex_list_element *
find_concave(struct vertex_list_element * start,
	     struct vertex_list_element * pt1,
	     struct vertex_list_element * pt2,
	     int concave_angle_mode)
{
   struct vertex_list_element *current, *steepest = NULL;
   double          angle, maxangle, minangle;

   switch (concave_angle_mode)
   {
      case 1:
	 current = start;
	 maxangle = PATH_PLAN_INFINITY;
	 while (current->next != start)
	 {			/* not at the beginning */
	    if (!correctside(current, current->next, current->next->next))
	    {
	       /* concave angle found */
	       angle = compute_angle(current, current->next, current->next->next);
	       if (angle < maxangle)
	       {		/* most concave so far */
		  steepest = current->next;
		  maxangle = angle;
	       }
	    }
	    current = current->next;	/* get next vertex */
	 }
	 /* last case (start angle) */
	 if (!correctside(current, current->next, current->next->next))
	 {
	    /* concave angle found */
	    angle = compute_angle(current, current->next, current->next->next);
	    if (angle < maxangle)
	    {			/* most concave so far */
	       steepest = current->next;
	       maxangle = angle;
	    }
	 }
	 break;

      case 2:
	 current = start;
	 minangle = -1.0;

	 do
	 {			/* not at the beginning */
	    if (!correctside(current, current->next, current->next->next))
	    {
	       /* concave angle found */
	       angle = compute_angle(current, current->next, current->next->next);
	       if (angle > minangle)
	       {		/* most concave so far */
		  steepest = current->next;
		  minangle = angle;
	       }
	    }
	    current = current->next;	/* get next vertex */
	 } while (current != start);
	 break;

      case 3:
	 current = start;
	 do
	 {			/* not at the beginning */
	    if (!correctside(current, current->next, current->next->next))
	    {
	       /* concave angle found */
	       steepest = current->next;
	       break;
	    }
	    current = current->next;	/* get next vertex */
	 } while (current != start);
	 break;
   }				/* end switch */

   if (steepest != NULL)
   {
      /* return anchors of most concave line segment           */
      /* set anchor 1 */
      pt1->x = steepest->previous->x;
      pt1->y = steepest->previous->y;
      pt1->passable = steepest->previous->passable;
      pt1->previous = steepest->previous->previous;
      pt1->corresponding = steepest->previous->corresponding;
      pt1->next = steepest;

      /* set anchor 2 */
      pt2->x = steepest->x;
      pt2->y = steepest->y;
      pt2->passable = steepest->passable;
      pt2->previous = steepest->previous;
      pt2->corresponding = steepest->corresponding;
      pt2->next = steepest->next;

      return (steepest->next);
   }
   else
   {
      return NULL;
   }
}


/*---------------------------------------------------------------------
			
			make_regions_convex

        function that breaks up a region into convex regions
	Returns pointer to newly created convex region stack

	convex_select_mode:  1=leftmost, 2=rightmost, 3=opposite
-----------------------------------------------------------------------*/

static struct stack_element *
make_regions_convex(struct stack_element * region_stack_top, int convex_select_mode, int concave_angle_mode, int display)
{
   int             is_convex;
   int             loop_counter;
   int             loop_counter2;
   struct vertex_list_element *region_head;
   struct vertex_list_element *temp;
   struct vertex_list_element anchor1, anchor2;	/* angle anchors */
   struct vertex_list_element *next_region_vertex;
   struct vertex_list_element *new1_region_head, *new2_region_head;

   /* used for splitting regions */
   struct vertex_list_element *hold = NULL, *head1, *head2;
   struct vertex_list_element *next1_to_add, *next2_to_add;
   struct stack_element *convex_stack_top, *stack_node, *temp_stack;

   /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
   /* initialize return stack      */
   convex_stack_top = NULL;
   region_head = region_stack_top->region;

#ifdef DEBUG
   sprintf(msg, "Partitioning into convex regions\n");
   puts(msg);
#endif

   loop_counter = 0;
   while (TRUE)
   {
      remove_redundant_vertices(region_stack_top);

      if (loop_counter++ > 1000)
      {
	 sprintf(msg, "probable infinite loop -1- in map_builder:make_regions_convex\n");
	 puts(msg);
	 abort();
      }

      /* Check to see if region is convex  */
      is_convex = slop_convex(region_head->next);

#ifndef TRANSPUTER
      if (display > 1)
      {
	 clear_map_display();
	 display_one_region(region_head->next);
      }
#endif

      /* if not convex , find most concave angle */
      if (!is_convex)
      {
	 next_region_vertex = find_concave(region_head->next, &anchor1, &anchor2, concave_angle_mode);

	 if (next_region_vertex == NULL)
	    is_convex = TRUE;
      }

      /* if not convex , find most concave angle */
      if (!is_convex)
      {

	 /* create headers for new region lists */
	 new1_region_head = malloc_vertex();
	 new2_region_head = malloc_vertex();
	 new1_region_head->features = new2_region_head->features = NULL;
	 next_region_vertex = anchor1.previous;

	 /*
	  * find first vertex that is on correctside of anchor and does not
	  * intersect with any of the lines
	  */
	 switch (convex_select_mode)
	 {
	    case 1:		/* leftmost */
	       while (TRUE)
	       {
		  if (two_in_region(next_region_vertex))
		     next_region_vertex = next_region_vertex->previous;
		  if (inclusive_correctside(&anchor2, anchor2.next, next_region_vertex) ||
		      inclusive_correctside(&anchor1, &anchor2, next_region_vertex))
		  {

		     /*
		      * does candidate intersect with ANY of the lines in the
		      * region
		      */
		     if (checknointersect(&anchor2, next_region_vertex, region_head->next))
		     {
			hold = next_region_vertex;
			break;
		     }
		  }
		  next_region_vertex = next_region_vertex->previous;
	       }

	       /* candidate found */
	       /* adjacent to anchor - backup -special case */
	       if (hold == anchor2.next)
	       {
		  hold = hold->next;
		  while (TRUE)
		  {
		     if (two_in_region(hold))
			hold = hold->next;
		     if (checknointersect(&anchor2, hold, region_head->next))
			break;
		     hold = hold->next;
		  }
	       }
	       break;

	    case 2:		/* rightmost */
	       hold = anchor2.next;
	       hold = hold->next;
	       while (TRUE)
	       {
		  if (two_in_region(hold))
		     hold = hold->next;
		  if (checknointersect(&anchor2, hold, region_head->next) &&
		      (inclusive_correctside(&anchor1, &anchor2, hold) ||
		       inclusive_correctside(&anchor2, anchor2.next, hold)))
		     break;
		  hold = hold->next;
	       }
	       break;

	    case 3:		/* opposite */
	       hold = NULL;
	       while (TRUE)
	       {
		  if (next_region_vertex == anchor2.next ||
		      next_region_vertex == anchor2.next->previous)
		  {
		     break;
		  }

		  if (two_in_region(next_region_vertex))
		     next_region_vertex = next_region_vertex->previous;

		  if (inclusive_correctside(&anchor2, anchor2.next, next_region_vertex))
		  {
		     /*
		      * does candidate intersect with ANY of the lines in the
		      * region
		      */
		     if (checknointersect(&anchor2, next_region_vertex, region_head->next))
		     {
			hold = next_region_vertex;
			break;
		     }
		  }
		  next_region_vertex = next_region_vertex->previous;
	       }

	       /* candidate found */
	       /* adjacent to anchor - backup -special case */
	       if ((hold == anchor2.next) || (hold == anchor2.next->previous) || (hold == NULL))
	       {
		  hold = anchor2.next->next;
		  loop_counter2 = 0;

		  while (TRUE)
		  {
		     if (loop_counter2++ > 200)
		     {
		       printf("map_builder:make_regions_convex - No single vertices exist in region\n");
		       printf("anchor: %5.2f %5.2f\n", anchor2.x, anchor2.y);
		       abort();
		     }

		     if (two_in_region(hold))
		     {
			hold = hold->next;
			continue;
		     }

		     if (checknointersect(&anchor2, hold, region_head->next))
		     {
			/* good one! */
			break;
		     }

		     hold = hold->next;

		  }
	       }
	       break;
	 }			/* end switch */

	 /* add most concave vertex to both new regions */
	 head1 = next1_to_add = add_vertex(new1_region_head, &anchor2);
	 head2 = next2_to_add = add_vertex(new2_region_head, &anchor2);

	 /* complete first region        */
	 temp = hold;
	 while (temp->next != anchor2.next)
	 {
	    next1_to_add = add_vertex(next1_to_add, temp);
	    temp = temp->next;

	 }

	 /* complete second region       */
	 temp = anchor2.next;
	 while (temp->next != hold->next)
	 {
	    next2_to_add = add_vertex(next2_to_add, temp);
	    temp = temp->next;
	 }
	 next2_to_add = add_vertex(next2_to_add, hold);

	 /* complete circular lists       */
	 next1_to_add->next = head1;
	 head1->previous = next1_to_add;
	 head1->passable = head2->previous;
	 next2_to_add->next = head2;
	 head2->previous = next2_to_add;
	 head2->previous->passable = head1;

	 /* correct all references to old region */
	 patch_region(head1);
	 patch_region(head2);

	 region_stack_top = region_stack_top->next;	/* pop region_stack */

	 /* push on two regions  */

	 stack_node = malloc_stack_element();
	 stack_node->region = head1;
	 stack_node->next = region_stack_top;
	 temp_stack = stack_node;
	 region_stack_top = stack_node = malloc_stack_element();
	 stack_node->region = head2;
	 stack_node->next = temp_stack;
	 region_head = region_stack_top->region;

	 /* tie the two regions together */
      }
      else
      {				/* region is convex */
	 /* push convex region on */
	 stack_node = malloc_stack_element();
	 stack_node->next = convex_stack_top;
	 stack_node->region = region_head->next;
	 convex_stack_top = stack_node;

	 region_stack_top = region_stack_top->next;	/* pop region stack */

	 if (region_stack_top == NULL)	/* all regions checked? */
	    break;
	 region_head = region_stack_top->region;
      }

      /* RRB
      if(stack_node){
	display_single_region(stack_node->region);
	if(stack_node->next) display_single_region(stack_node->next->region);
      }
      */
   }

   return (convex_stack_top);
}



/*---------------------------------------------------------------------------

	sub to add an obstacle

	Accepts a list of vertices corresponding to a wall
	constructs a region, then passes it to check convex

	VERTICES MUST BE ENTERED CLOCKWISE

---------------------------------------------------------------------------*/

void 
load_obstacle(int num, Point vertices[], double grow_length)
{
   /* stack variables & functions			 */
   struct stack_element *temp_stack;

   /* get pointer to current region stack */
   temp_stack = long_term_memory.top_of_convex_region_stack;

   /* add obstacle to the region stack */
   add_one_obstacle(temp_stack, num, vertices, grow_length);

   long_term_memory.top_of_convex_region_stack = temp_stack;
}


/*---------------------------------------------------------------------------

        Takes a file name of a map and loads it.
	map file format:
	   WALLS numvertices    - number of vertex pairs in map
	   x y
	   . .
	   . .
	   x y

	   OBSTACLES numvertices    - number of vertex pairs in 1st obstacle
	   x y
	   . .
	   . .
	   x y

            .
	    .
	    .


	MAP VERTICES MUST BE ENTERED COUNTER CLOCKWISE (as shows on screen)
	OBSTACLE VERTICES MUST BE ENTERED CLOCKWISE (as shows on screen)

---------------------------------------------------------------------------*/

void 
load_map(char *map_file, int display, 
	 double robot_radius, double robot_safety_margin)
{
  int i;

  /* vertex variables & functions			 */
  struct vertex_list_element *region_head;
  struct vertex_list_element *next;
  struct vertex_list_element *border;
  struct vertex_list_element *next_vertex;
  struct vertex_list_element temp;

  /* stack variables & functions			 */
  struct obstacle_list_element *obstacle_list_head;
  struct obstacle_list_element *next_obstacle;

  struct stack_element *region_stack_top;
  struct stack_element *border_list;
  int done;
  int number_vertices;
  int res;
  char keywordString[100];
   
#ifdef TRANSPUTER
  int             pos;

#else
  FILE           *fp = NULL;

#endif

  /* This function is in this file (map_bldr.c) */
  initialize_long_term_memory();

  /* RRB - note that this section disconnects these structures from the linked list created 
     in initialize_long_term_memory. */

  /* set up region stack */
  region_head              = malloc_vertex();
  region_head->next        = NULL;
  region_head->previous    = NULL;
  region_head->features    = NULL;

  region_stack_top         = malloc_stack_element();
  region_stack_top->region = region_head;
  region_stack_top->next   = NULL;

  border                   = region_head;

#ifndef TRANSPUTER
  if (!(fp = fopen(map_file, "r")))
    {
      perror("load_map(): unable to open map file ");
      exit(1);
    }
#endif


  /*******************************/
  /* read total # WALL  vertices */
  /*******************************/

#ifdef TRANSPUTER
  pos = 0;
  number_vertices = MAP_DATA[pos++];
#else
  res = fscanf(fp, "%s %d", keywordString, &number_vertices);
  if( (res != 2) || (strcmp(keywordString, "WALLS") != 0))
    {
      perror("map_bldr.c: load_map() -- Expecting Keyword WALLS followed by # of vertices.\n");
      exit(1);
    }
  if(DEBUG_MAP) printf("Map has %d WALL vertices\n", number_vertices);
#endif

  /* mark walls solid by default */
  temp.corresponding = NULL;
  temp.passable      = NULL;
  temp.looked_at     = 0;

  /* start building list */
  next = region_head;

  if (display > 1) printf("map wall vertices:\n");

  /* Loop through all wall vertices */
  for (i = 0; i < number_vertices; i++)
    {

#ifdef TRANSPUTER
      temp.x = MAP_DATA[pos++];
      temp.y = MAP_DATA[pos++];
#else
      fscanf(fp, "%lf %lf ", &(temp.x), &(temp.y));
#endif

      /* must be strictly non-negative */
      if ((temp.x < 0) || (temp.y < 0))
	{
	  printf("map_builder:load_map() - vertices must have non-negative coordinates\n");
	  exit(1);
	}
      if (display > 1)
	{
	  printf("  %2d.\t%5.2f\t%5.2f\n", i+1, temp.x, temp.y);
	}
      next = add_vertex(next, &temp); /* add_vertex() is in cart_lib.c */
    } /* for i up to number_vertices */


  /* Tie off linked list into a ring */
  next->next = region_head->next;
  region_head->previous = next;
  region_head->next->previous = region_head->previous;

  /* Note that the original structure "region_head" never gets packed with data,
   * but is not included in the ring.*/

#ifdef DEBUG
  if (verify_links(region_stack_top))
    {
      printf("map_builder:load_map() - verify_links 1 failed\n");
    }
#endif

#ifndef TRANSPUTER
  if (display)
    {
      /* set the scale for the window display */
      set_display_scale(border);

      display_grown(border, NULL); 

      if ((display == 2) && STEP_DEBUG)
	{
	  puts("Initial border: press 1 and enter to continue\n");
	  scanf("%d ", &i);
	}
    }
#endif

  /* This modifies the structures pointed to by region_stack_top */
  grow_region(region_stack_top, robot_radius + robot_safety_margin);

#ifdef DEBUG
  if (verify_links(region_stack_top))
    {
      puts("map_builder:load_map - verify_links 2 failed\n");
    }
#endif

#ifndef TRANSPUTER
  if (display)
    {
      display_grown(border, region_stack_top->region);

      if ((display == 2) && STEP_DEBUG)
	{
	  puts("Grown border: press 1 and enter to continue\n");
	  scanf("%d", &i);
	}

    }
#endif




  /**************************/
  /* now load the obstacles */
  /**************************/
  done = FALSE;
  do
    { /* while !done */

#ifdef TRANSPUTER
      number_vertices = MAP_DATA[pos++];
#else
      number_vertices = 0;
      res = fscanf(fp, "%s %d ", keywordString, &number_vertices);
      if((res != 2) || (strcmp(keywordString, "OBSTACLES") != 0))
	{
	  perror("map_bldr.c:load_map() -- Expecting Keyword OBSTACLES followed by # vertices\n");
	  exit(1);
	}
      if( DEBUG_MAP) printf("Map has %d OBSTACLE vertices\n", number_vertices);
#endif

      if (number_vertices <= 0)
	{
	  obstacle_list_head = NULL;
	  done = TRUE;
	  continue;
	}

      /* get pointer to current region stack */
      border_list = region_stack_top;

      /* initialize obstacle list */
      obstacle_list_head                            = malloc_obstacle_list_element();
      obstacle_list_head->next                      = NULL;
      obstacle_list_head->actual_obstacle           = malloc_vertex();
      obstacle_list_head->actual_obstacle->features = NULL;

      /* input obstacles */
      next_obstacle = obstacle_list_head;
      next_obstacle->status = 0;          /* initially unknown */

      next_vertex = next_obstacle->actual_obstacle;

      for (i = 0; i < number_vertices; i++)
	{

#ifdef TRANSPUTER
	  temp.x = MAP_DATA[pos++];
	  temp.y = MAP_DATA[pos++];
#else
	  fscanf(fp, "%lf %lf ", &temp.x, &temp.y);
#endif

	  /* must be strictly non-negative */
	  if (temp.x < 0 || temp.y < 0)
	    {
	      printf("map_builder: load_map() - obstacle vertices must be non-negative\n");
	      exit(1);
	    }

	  if (display > 1)
	    {
	      printf("  %2d.\t%5.2f\t%5.2f\n", i+1, temp.x, temp.y);
	    }

	  next_vertex->x             = temp.x;
	  next_vertex->y             = temp.y;
	  next_vertex->corresponding = NULL;
	  next_vertex->passable      = NULL;
	  next_vertex->looked_at     = 0;

	  /* do all but last one */
	  if (i < number_vertices - 1)
	    {
	      next_vertex->next           = malloc_vertex();
	      next_vertex->next->features = NULL;
	      next_vertex->next->previous = next_vertex;
	      next_vertex                 = next_vertex->next;
	    }
	} /* for i = 0 to number_vertices */

      /* tie off obstacle (complete circular list) */
      next_obstacle->actual_obstacle->previous = next_vertex;
      next_vertex->next = next_obstacle->actual_obstacle;

      /* grow obstacles */
      grow_obstacles(obstacle_list_head, robot_radius + robot_safety_margin);

      merge_overlap_obstacles_border(obstacle_list_head, border_list);

      merge_obstacle_to_obstacle(obstacle_list_head);

      /* connect obstacles to border */
      attach_obstacles(obstacle_list_head, border_list);

      /* RRB
      step_display(border_list); 
      */      
    } while (!done);

#ifndef TRANSPUTER
  /* close the input file */
  fclose(fp);
#endif

  long_term_memory.top_of_convex_region_stack = region_stack_top;
  long_term_memory.top_of_obstacles = obstacle_list_head;

  list_obstacles();

}

/*---------------------------------------------------------------------------

	map_builder

        builds the map created by load_map and load_obstacle

convex_select_mode:  1=leftmost, 2=rightmost, 3=opposite
concave_angle_mode:  1=most, 2=least, 3=first
---------------------------------------------------------------------------*/

//struct terrain_identifier *THE_TERRAIN = NULL;

void 
map_builder(int display, int convex_select_mode, int concave_angle_mode)
{
  /* stack variables & functions			 */
  struct stack_element *temp_stack;
  struct stack_element *convex_region_stack, *current_convex_region;
  int merged;

  //struct stack_element *check_merge(), *terrain_patch();
  //struct terrain_identifier *malloc_terrain_identifier();
  //struct error_facts        * malloc_error_facts();


#ifndef TRANSPUTER
  int             jnk;

#endif

#if 0
  /* DCM: fake out the terrrain code since it doesn't work */
  THE_TERRAIN = malloc_terrain_identifier();
  THE_TERRAIN->traversal_factor = 1.0;
  THE_TERRAIN->transition_zone  = FALSE;
  THE_TERRAIN->error_data       = malloc_error_facts();
#endif
   /* get pointer to current region stack */
  temp_stack = long_term_memory.top_of_convex_region_stack;

  /* make the regions convex */
  convex_region_stack = make_regions_convex(temp_stack, convex_select_mode, concave_angle_mode, display);

  /*  step_display(convex_region_stack); */

#ifndef TRANSPUTER
  if (display)
    {
      display_regions(convex_region_stack); 

      if ((display == 2) && STEP_DEBUG)
	{
	  puts("convex regions: press 1 and enter to continue\n");
	  scanf("%d", &jnk);
	}

    }
#endif

  patch_for_obstacles(convex_region_stack);

   /* attempt to merge regions */
  current_convex_region = convex_region_stack;
  while (current_convex_region != NULL)
    {
      do
	{
	  convex_region_stack =
	    check_merge(current_convex_region, convex_region_stack, &merged);

	  if (merged)
	    current_convex_region = convex_region_stack;
	} while (merged);

      /* move to next region */
      current_convex_region = current_convex_region->next;
    }

#ifndef TRANSPUTER
  if (display)
    {
      display_regions(convex_region_stack); 

      if ((display == 2) && STEP_DEBUG)
	{
	  int             jnk;
	  puts("merged regions: press 1 and enter to continue\n");
	  scanf("%d", &jnk);
	}

    }
#endif

  convex_region_stack = terrain_patch(convex_region_stack, NULL);
  //convex_region_stack = terrain_patch(convex_region_stack, THE_TERRAIN);

   /* update ltm pointer */
  long_term_memory.top_of_convex_region_stack = convex_region_stack;
}


/*------------------------------------------------------------------------------

			INITIALIZE_LONG_TERM_MEMORY

------------------------------------------------------------------------------*/

void
initialize_long_term_memory(void)
{
   int             i;

   /* Set up Vertex pointers for freelist   */
   for (i = 0; i < MAX_VERTICES - 1; i++)
   {
      long_term_memory.vle[i].next = &(long_term_memory.vle[i + 1]);
      long_term_memory.vle[i].previous = NULL;
      long_term_memory.vle[i].passable = NULL;
      long_term_memory.vle[i].looked_at = 0;
   }
   long_term_memory.vle[MAX_VERTICES - 1].next = NULL;
   long_term_memory.next_vertex = &(long_term_memory.vle[0]);


   /* Set up stack pointer for freelist     */
   for (i = 0; i < MAX_STACK_ELEMENTS - 1; i++)
     {
       long_term_memory.se[i].next = &(long_term_memory.se[i + 1]);
     }
   long_term_memory.se[MAX_STACK_ELEMENTS - 1].next = NULL;
   long_term_memory.next_stack_element = &(long_term_memory.se[0]);


   /* set up obstacle pointers for freelist */
   for (i = 0; i < MAX_OBSTACLES - 1; i++)
      {
	long_term_memory.obs[i].next = &(long_term_memory.obs[i + 1]);
      }
   long_term_memory.obs[MAX_OBSTACLES - 1].next = NULL;
   long_term_memory.next_obstacle = &(long_term_memory.obs[0]);


   /* vertex features, terrain_ids, and error_facts allocated when needed */
   long_term_memory.next_error_facts    = 0;
   long_term_memory.next_terrain_id     = 0;
   long_term_memory.next_vertex_feature = 0;


}


/*----------------------------------------------------------------------------

                              PSEUDO- MALLOC functions

-----------------------------------------------------------------------------*/

/*---------------------------- MALLOC_ERROR_FACTS ---------------------------*/

struct error_facts * malloc_error_facts(void)
{
   if (long_term_memory.next_error_facts == MAX_ERROR_FACTS)
   {
      puts("\n NO MORE STORAGE FOR ERROR FACTS - PLEASE EXPAND");
      exit(1);
   }
   long_term_memory.next_error_facts++;
   return (&(long_term_memory.err_facts[long_term_memory.next_error_facts - 1]));
}

/*---------------------------- MALLOC_TERRAIN_IDENTIFIER --------------------*/

struct terrain_identifier
               *
malloc_terrain_identifier()
{
   if (long_term_memory.next_terrain_id == MAX_TERRAIN_IDS)
   {
      puts("\n NO MORE STORAGE FOR TERRAIN IDENTIFIERS - PLEASE EXPAND");
      exit(1);
   }
   long_term_memory.next_terrain_id++;
   //   printf("malloc terrain: %d \n", (int)(&(long_term_memory.terrain[long_term_memory.next_terrain_id - 1])));
   return (&(long_term_memory.terrain[long_term_memory.next_terrain_id - 1]));
}

/*---------------------------- MALLOC_VERTEX_FEATURES -----------------------*/

struct vertex_features
               *
malloc_vertex_features()
{

   if (long_term_memory.next_vertex_feature == MAX_VERTEX_FEATURES)
   {
      puts("\n NO MORE STORAGE FOR VERTEX_FEATURES - PLEASE EXPAND");
      exit(1);
   }
   long_term_memory.next_vertex_feature++;
   return (&(long_term_memory.ver_feat[long_term_memory.next_vertex_feature - 1]));
}

/*---------------------------- MALLOC_VERTEX --------------------------------*/

struct vertex_list_element *
malloc_vertex(void)
{
   struct vertex_list_element *temp;

   vertex_count++;
   if ((long_term_memory.next_vertex)->next == NULL)
   {
     printf("\nmap_bldr.c: malloc_vertex():\n");
     printf(" -- No more storage for vertex elements\n");
     printf(" -- Please increase MAX_VERTICES in new_long_term_memory.h\n");
     exit(1);
   }

   temp = long_term_memory.next_vertex;
   long_term_memory.next_vertex = (long_term_memory.next_vertex)->next;

   return (temp);
}

/*---------------------------- MALLOC_STACK_ELEMENT -------------------------*/

struct stack_element *
malloc_stack_element(void)
{
   struct stack_element *temp;
   static int num_allocated = 0;

   stack_count++;
   num_allocated++;
   if (long_term_memory.next_stack_element == NULL)
   {
     printf("\nmap_bldr.c: malloc_stack_element():\n");
     printf(" -- No more storage for stack elements after %d\n", num_allocated);
     printf(" -- Please increase MAX_STACK_ELEMENTS in new_long_term_memory.h\n");
     exit(1);
   }

   temp = long_term_memory.next_stack_element;
   long_term_memory.next_stack_element =
      (long_term_memory.next_stack_element)->next;

   return (temp);
}

/*------------------- MALLOC_OBSTACLE_LIST_ELEMENT --------------------------*/

struct obstacle_list_element *
malloc_obstacle_list_element(void)
{
   struct obstacle_list_element *temp;

   if (long_term_memory.next_obstacle == NULL)
   {
     printf("\n map_bldr.c: malloc_obstacle_list_element():\n");
     printf(" -- No more storage for obstacles\n");
     printf(" -- please increase MAX_OBSTACLES in new_long_term_memory.h\n");
     exit(1);
   }
   temp = long_term_memory.next_obstacle;
   temp->grown_obstacle = NULL;
   temp->actual_obstacle = NULL;
   long_term_memory.next_obstacle = (long_term_memory.next_obstacle)->next;
   return (temp);
}



/*-----------------------------------------------------------------------------

			PSEUDO Free Functions

-----------------------------------------------------------------------------*/

/*------------------------------- FREE_VERTEX -------------------------------*/

/* This had better be called only for the most recently assigned vertex! */
void
free_vertex(struct vertex_list_element * address)
{
   vertex_count--;		/* push it on */
   address->next = long_term_memory.next_vertex;
   long_term_memory.next_vertex = address;
}

/*-------------------- FREE_STACK_ELEMENT ---------------------------------*/

void
free_stack_element(struct stack_element * address)
{
   stack_count--;
   address->next = long_term_memory.next_stack_element;
   long_term_memory.next_stack_element = address;
}

/*-------------------------- FREE_OBSTACLE ---------------------------------*/

void
free_obstacle(struct obstacle_list_element *address)
{
   address->next = long_term_memory.next_obstacle;
   long_term_memory.next_obstacle = address;
}



///////////////////////////////////////////////////////////////////////
// $Log: map_bldr.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.2  2006/05/14 05:57:38  endo
// gcc-3.4 upgrade
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:40:03  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

