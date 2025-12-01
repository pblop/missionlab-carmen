/******************************************************************************
 **                                                                          **
 **                                 cart_lib.c                               **
 **                                                                          **
 **                                                                          **
 **                                                                          **
 **                                                                          **
 **  Written by:                                                             **
 **                                                                          **
 **  Copyright 2000, Georgia Tech Research Corporation                       **
 **  Atlanta, Georgia  30332-0415                                            **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.                    **
 *****************************************************************************/

/* $Id: cart_lib.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

			cart_lib.c

		        CARTOGRAPHER functions

			Author: Ronald C. Arkin
	
			
******************************************************************************/


/*---------------------------------------------------------------------------*/
/*                                preliminaries                              */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include "std.h"
#include "clipstruct.h"
#include "new_long_term_memory.h"
#include "short_term_memory.h"
#include "map_bldr.h"
#include "cart_lib.h"

#include "map_display.h"

#define MERGED     1
#define CHOP_ANGLE 0
#define RELAX_LIMIT 0

struct closed_node3
{
   double          x1;		/* x of vertex 1 (not adit) */
   double          y1;		/* y of vertex 1            */
   double          x2;		/* x of vertex 2 (not adit) */
   double          y2;		/* y of vertex 2            */
   double          open_x;
   double          open_y;
   struct closed_node3 *next_cnode;	/* Points to next node on closed list */
};


int line_equation(struct vertex_list_element *p1, struct vertex_list_element* p2, 
		  double *slope, double *intercept);

//struct path_node *build_path();

extern int      straighten_path;





/*-----------------------------------------------------------------------------

				verify_links

Check the map to make sure no circular linked lists are broken.

returns 1 on success, 0 on failure.

----------------------------------------------------------------------------*/

int 
verify_links(struct stack_element *map)
{
  struct stack_element *current;
  struct vertex_list_element *vertex;

  if( map == NULL )
    {
      printf("\ncart_lib:verify_links - map is null\n");
      return(0);
    }

  current = map;
  while( current != NULL )
    {
      if( current->region == NULL )
	{
	  printf("\ncart_lib:verify_links - map is ill-formed: region pointer is NULL\n");
	  return(0);
	}

      /* check forward links */
      vertex = current->region->next;
      do
	{
	  if(vertex == NULL)
	    {
	      printf("\ncart_lib:verify_links - vertex forward link broken\n");
	      return(0);
	    }

	  vertex = vertex->next;

	} while (vertex != current->region->next);

      /* check backward links */
      vertex = current->region->next;
      do
	{
	  if( vertex == NULL )
	    {
	      printf("\ncart_lib:verify_links - vertex backward link broken\n");
	      return(0);
	    }

	  vertex = vertex->previous;

	} while (vertex != current->region->next);

      /* move to next region */
      current = current->next;
    } /* while current != NULL */

  return(1);
}

/*---------------------------------------------------------------------

		       inclusive_correctside

Determines if the point is on the correct side of a line from start to end.
Correct side means the point is on the left side (relative to the vector
start->end) of a line passing through start and end.

Inclusive includes the case of the point colinear with the line as correct.

------------------------------------------------------------------------*/

int
inclusive_correctside(struct vertex_list_element * start,
		      struct vertex_list_element * end,
		      struct vertex_list_element * point)

{
   double Ux, Uy, Vx, Vy;
   double dist;

   /* create a vector along the line */
   Ux = end->x - start->x;
   Uy = end->y - start->y;

   /* create a vector to the cursor location */
   Vx = point->x - start->x;
   Vy = point->y - start->y;

   /* calculate the perpendicular distance to the point from the line via
    * the cross product 
    */
   dist = Ux * Vy - Uy * Vx;

   if(dist >= 0) return TRUE;
   else          return FALSE;
}


/*-----------------------------------------------------------------------------

				correctside

Determines if the point is on the correct side of a line from start to end.
Correct side means the point is on the left side (relative to the vector
start->end) of a line passing through start and end.

-----------------------------------------------------------------------------*/

int
correctside(struct vertex_list_element * start,
	    struct vertex_list_element * end,
	    struct vertex_list_element * point)

{
   double Ux, Uy, Vx, Vy;
   double dist;

   /* create a vector along the line */
   Ux = end->x - start->x;
   Uy = end->y - start->y;

   /* create a vector to the cursor location */
   Vx = point->x - start->x;
   Vy = point->y - start->y;

   /* calculate the perpendicular distance to the point from the line via
    * the cross product 
    */
   dist = Ux * Vy - Uy * Vx;

   if(dist > 0)  return TRUE;
   else          return FALSE;
}

int
path_correctside(struct path_node * start,
		 struct path_node * end,
		 struct path_node * point)

{
   double Ux, Uy, Vx, Vy;
   double dist;

   /* create a vector along the line */
   Ux = end->x - start->x;
   Uy = end->y - start->y;

   /* create a vector to the cursor location */
   Vx = point->x - start->x;
   Vy = point->y - start->y;

   /* calculate the perpendicular distance to the point from the line via
    * the cross product 
    */
   dist = Ux * Vy - Uy * Vx;

   if(dist > 0)  return TRUE;
   else          return FALSE;
}


/*-----------------------------------------------------------------------------

				find_start_goal

		Return terrain identifier for an open node with a meadow

-----------------------------------------------------------------------------*/

struct terrain_identifier *
find_start_goal(struct vertex_list_element *point, struct stack_element *map)
{
   struct stack_element *meadow_ptr;
   struct vertex_list_element *meadow;

   meadow_ptr = map;
   meadow = meadow_ptr->region;

   while (!inside_meadow(point, meadow))
   {
      meadow_ptr = meadow_ptr->next;

      /* not in meadow */
      if (meadow_ptr == NULL)
      {
         return (NULL);
      }
      meadow = meadow_ptr->region;
   }
   //   printf("find_start_goal: %d\n",(int)(meadow_ptr->terrain_type));

   return (meadow_ptr->terrain_type);
}


/*----------------------------------------------------------------------------

				FREE_PATH

			Frees storage for path

----------------------------------------------------------------------------*/

void
free_path(struct path_node * path)
{
   struct path_node *temp;

   while (path != NULL)
   {
      temp = path->next_point;
      free(path);
      path = temp;
   }
}

/*----------------------------------------------------------------------------

				TWO_IN_REGION

		Accepts a vertex in a region and checks to see if another
		vertex with the same x and y is present in the same region.
		Returns TRUE if so, FALSE otherwise.

-----------------------------------------------------------------------------*/
int
two_in_region(struct vertex_list_element * vertex)
{
   struct vertex_list_element *current;

   current = vertex->next;
   while (current != vertex)
   {
      if( current->x == vertex->x && current->y == vertex->y )
	 return (TRUE);

      current = current->next;
   }
   return (FALSE);
}

/*-------------------------------------------------------------------------

		ROUND FLOAT, REGION, BORDER, and OBSTACLE

		Uses Round_float to remove discrete math errors
		Rounds to three decimal places

-------------------------------------------------------------------------*/


double
round_float(double x)
{
   long            y;

   y = (long)((x + 0.0005) * 1000.0);
   return (((double) y) / 1000.0);
}

void
round_region(struct stack_element * stack)
{
    //double          round_float();
   struct vertex_list_element *current;

   current = stack->region->next;
   do
   {
      current->x = round_float(current->x);
      current->y = round_float(current->y);
      current = current->next;
   } while (current != stack->region->next);
   current = stack->region->next;
   do
   {
      if ((current->x == current->next->x) &&
	  (current->y == current->next->y))
      {
	 current->next = current->next->next;
	 current->next->next->previous = current;
      }
      current = current->next;
   } while ((current->x != stack->region->next->x) ||
	    (current->y != stack->region->next->y));
   if (stack->region->next != current)
      stack->region->next = current;
   return;

}


void
round_border(struct vertex_list_element * border)
{
//   double          round_float();
   struct vertex_list_element *current;

   current = border->next;
   do
   {
      current->x = round_float(current->x);
      current->y = round_float(current->y);
      current = current->next;
   } while (current != border->next);
   return;
}


void
round_obstacle(struct obstacle_list_element * obstacle)
{
    //double          round_float();
   struct vertex_list_element *current;

   current = obstacle->grown_obstacle->next;
   do
   {
      current->x = round_float(current->x);
      current->y = round_float(current->y);
      current = current->next;
   } while (current != obstacle->grown_obstacle->next);
   return;
}

/*-----------------------------------------------------------------------------

				SAME_REGION

			accepts two vertices and compares
		to see if they are in the same region

-----------------------------------------------------------------------------*/

int
same_region(struct vertex_list_element * v1, struct vertex_list_element * v2)
{
   struct vertex_list_element *current;

   if( v2 == NULL )
      return FALSE;

   current = v2;
   while (TRUE)
   {
      if (v1 == current)
	 return (TRUE);		/* same vertex */

      if( current == NULL )
      {
	 puts("broken vertex loop");
	 return FALSE;
      }

      current = current->next;
      if (current == v2)
	 return (FALSE);	/* no vertex in common */
   }
}


/*---------------------------------------------------------------------

		       COMPUTE_ANGLE


       function to compute angle between three 2D cartesian points
       Angle value returned in DEGREES
       Uses law of cosines
       Only returns angles in range from 0 to 180 degrees.
       Depending on which way the boundary bends at pivot,
       the returned angle could be inside or outside the free space.

----------------------------------------------------------------------*/

double
compute_angle(struct vertex_list_element * pt1,
	      struct vertex_list_element * pivot,
	      struct vertex_list_element * pt3)
{
   double          a, b, c;	/* lengths between points */
   double          val;

   a = find_length(pt1, pt3);
   b = find_length(pt1, pivot);
   c = find_length(pivot, pt3);
   val = (sqr(b) + sqr(c) - sqr(a)) / (2.0 * b * c);

   /* clip val back in case of floating-point errors */
   if (val > 1.0) val = 1.0;
   else if (val < -1.0) val = -1.0;

   return DEGREES_FROM_RADIANS(acos(val));
}


/*-----------------------------------------------------------------------------

				grow

		Grow a vertex out by "dist" distance

------------------------------------------------------------------------------*/

struct vertex_list_element *
grow(struct vertex_list_element * p2, double dist)
{
   double          angle, theta, phi;
   struct vertex_list_element v2, *grown, *p1, *p3;

   grown = malloc_vertex();
   grown->features = NULL;
   p1 = p2->previous;
   p3 = p2->next;
   theta = compute_angle(p1, p2, p3);
   v2.x = p2->x + 5.0; /* to get a vector for absolute zero-angle reference 
			  (straight out in the X direction from p2  */
   v2.y = p2->y;
   if (!correctside(p1, p2, p3))
   {				/* concave */
      theta = 360.0 - theta;
   }
   /* Now theta is the freespace-side angle, concave or convex. */

   theta = theta / 2;

   phi = compute_angle(&v2, p2, p3);
   /* This shouldn't happen... */
   if ((phi == 0) && (p2->y == p3->y) && (p3->x < p2->x)) phi = 180.0; 
   if (correctside(&v2, p2, p3)) phi = 360.0 - phi;

   angle = theta + phi;
   if (sin(RADIANS_FROM_DEGREES(theta)) != 0.0)
      dist /= sin(RADIANS_FROM_DEGREES(theta)); /* ensures robot can't get 
						   within dist of walls. */
   grown->x = p2->x + dist * cos(RADIANS_FROM_DEGREES(angle));
   grown->y = p2->y + dist * sin(RADIANS_FROM_DEGREES(angle));
   return (grown);
}


/*----------------------------------------------------------------------------

				GROW_MIDDLE
			
		Used by chop concave angle - grows straight out by length

------------------------------------------------------------------------------*/

static struct vertex_list_element *
grow_middle(struct vertex_list_element * p2,double length)
{
   double          angle, theta, phi;
   struct vertex_list_element v2, *grown, *p1, *p3;

   grown = malloc_vertex();
   grown->features = NULL;
   p1 = p2->previous;
   p3 = p2->next;
   theta = compute_angle(p1, p2, p3);
   v2.x = p2->x + 5.0;
   v2.y = p2->y;
   if (!correctside(p1, p2, p3))
   {				/* concave */
      theta = 360.0 - theta;
   }
   theta = theta / 2;
   phi = compute_angle(&v2, p2, p3);
   if ((phi == 0) && (p2->y == p3->y) && (p3->x < p2->x))
      phi = 180.0;
   if (correctside(&v2, p2, p3))
      phi = 360.0 - phi;
   angle = theta + phi;
   grown->x = p2->x + length * cos(RADIANS_FROM_DEGREES(angle));
   grown->y = p2->y + length * sin(RADIANS_FROM_DEGREES(angle));
   return (grown);
}


/*-----------------------------------------------------------------------------


				CONCAVE

		Checks to see if a vertex is concave.
		Returns TRUE if it is, FALSE otherwise

-----------------------------------------------------------------------------*/

int
concave(struct vertex_list_element * vertex)
{
   if (!correctside(vertex->previous, vertex, vertex->next))
      return (TRUE);
   else
      return (FALSE);
}


/*-------------------------------------------------------------------

				ADD_VERTEX
	
	Returns pointer to vertex just added at end of list.
	Accepts location (end of list) where vertex is to be added
	and pointer to vertex to be added.
	Allocates storage for new element
	and adds it as last item in doubly linked list

------------------------------------------------------------------------*/

struct vertex_list_element *
add_vertex(struct vertex_list_element * location, /* pointer to end of list */
	   struct vertex_list_element * vertex)	  /* pointer to vertex */
{
   struct vertex_list_element *ptr;	/* holds address of new element */

   ptr = malloc_vertex();

   ptr->features = NULL;
   ptr->next     = NULL;

   /* copy in vertex data */
   ptr->x             = vertex->x;
   ptr->y             = vertex->y;
   ptr->passable      = vertex->passable;
   ptr->corresponding = vertex->corresponding;

   /* link into chain */
   location->next = ptr;
   ptr->previous = location;	/* complete back link */

   return (ptr);
}


/*-----------------------------------------------------------------*/
/*
 * FIND_LENGTH
 * 
 * function to find the length of a line segment by the pythagorean theorem
 * Returns length as double
 * 
 * -------------------------------------------------------------------
 */

double
find_length(struct vertex_list_element * pt1,
	    struct vertex_list_element * pt2)

{
   double          delta_x, delta_y;

   delta_x = pt1->x - pt2->x;
   delta_y = pt1->y - pt2->y;

   return (sqrt( sqr(delta_x) + sqr(delta_y)));
}


/*-------------------------------------------------------------------------

				PERP

		constructs a perpendicular to a line at point p2

----------------------------------------------------------------------------*/

void
perp(
     struct vertex_list_element * p1,	/* points on line */
     struct vertex_list_element * p2,	/* points on line */
     struct vertex_list_element * p3perp)	/* output for perpendicular */
{
   p3perp->x = p2->x - (p2->y - p1->y);
   p3perp->y = p2->y + (p2->x - p1->x);
   return;
}


/*----------------------------------------------------------------------------

				CHECKNOINTERSECT

		check to see if a region has any lines in it
		that intersect with a specified line
		Return TRUE if no intersection, FALSE if there is.

------------------------------------------------------------------------------*/

int
checknointersect(struct vertex_list_element * pt1,
		 struct vertex_list_element * pt2,
		 struct vertex_list_element * region_ptr)
{
   struct vertex_list_element *current;

   current = region_ptr;
   do
   {
      if (colinear(pt1, pt2, pt1, current) && 
	 (min(pt1->x, pt2->x) <= current->x) &&
         (max(pt1->x, pt2->x) >= current->x) && 
	 (min(pt1->y, pt2->y) <= current->y) &&
         (max(pt1->y, pt2->y) >= current->y) && 
	 pt1->next != current->next && 
	 pt2->next != current->next)
      {
         return (FALSE);
      }

      if (intersect(pt1, pt2, current, current->next))
      {
         return (FALSE);
      }

      current = current->next;
   }
   while (current != region_ptr);

   return (TRUE);
}


#if 0
void rob_intersect(struct vertex_list_element * l1p1,
		   struct vertex_list_element * l1p2,
		   struct vertex_list_element * l2p1,
		   struct vertex_list_element * l2p2)
{

  double dx1 = l1p2->x - l1p1->x;
  double dy1 = l1p2->y - l1p1->y;
  double dx2 = l2p2->x - l2p1->x;
  double dy2 = l2p2->y - l2p1->y;
  double dx11= l2p1->x - l1p1->x;
  double dy11= l2p1->y - l1p1->y;
  double dx21= l2p2->x - l1p1->x;
  double dy21= l2p2->y - l1p1->y;
  double dx12= l2p1->x - l1p2->x;
  double dy12= l2p1->y - l1p2->y;
  double dx22= l2p2->x - l1p2->x;
  double dy22= l2p2->y - l1p2->y;
  double cx1 = dx1*dy11 - dy1*dx11;
  double cx2 = dx1*dy21 - dy1*dx21;
  double cx3 = dx1*dy2  - dy1*dx2;

  if(cx1 != 0) {  /* start of line 2 is not colinear with line 1 */
    if(cx2 != 0){ /* end of line 2 is not colinear with line 2 */

    }
  }
}

#endif

/*-----------------------------------------------------------------------------

				INTERSECT

		determines if 2 lines intersect
		includes colinear case

------------------------------------------------------------------------------*/


int
intersect(struct vertex_list_element * l1p1,
	  struct vertex_list_element * l1p2,
	  struct vertex_list_element * l2p1,
	  struct vertex_list_element * l2p2)

/* points of line1 and line2  - order irrelevant */
{
   int             return_value;
   int             parallel;
   double          dx1, dx2;
   double          mxl1, mxl2, mnl1, mnl2;

   return_value = FALSE;
   parallel = FALSE;		/* assume non-parallel */
   dx1 = l1p2->x - l1p1->x;
   dx2 = l2p2->x - l2p1->x;
   if (dx1 == 0 && dx2 == 0)
   {				/* slopes not computable */
     /* Both vertical */
      if (true_lintersect(l1p1, l1p2, l2p1, l2p2))
      {				/* colinear! */
	 /* colinear special cases */
	 mxl1 = max(l1p1->y, l1p2->y);
	 mxl2 = max(l2p1->y, l2p2->y);
	 mnl1 = min(l1p1->y, l1p2->y);
	 mnl2 = min(l2p1->y, l2p2->y);
	 if ((mnl1 < mxl2 && mxl1 > mxl2) || (mxl1 > mnl2 && mnl1 < mnl2)
	 || (mxl1 <= mxl2 && mnl1 >= mnl2) || (mxl2 <= mxl1 && mnl2 >= mnl1))
	    return (TRUE);	/* colinear intersects */
	 else
	    return (FALSE);
      }
      else
	 return (FALSE);
   }
   else if ((dx1 == 0) || (dx2 == 0))	/* slopes not computable, but not identical */
      parallel = FALSE;		        /* therefore they are non-parallel */
   else
   {
      if (((l2p2->y - l2p1->y) / dx2) == ((l1p2->y - l1p1->y) / dx1))	/* slopes equal? */
	 parallel = TRUE;
   }
   if (parallel)
   {
      if (true_lintersect(l1p1, l1p2, l2p1, l2p2))
      {				/* colinear! */
	 mxl1 = max(l1p1->x, l1p2->x);
	 mxl2 = max(l2p1->x, l2p2->x);
	 mnl1 = min(l1p1->x, l1p2->x);
	 mnl2 = min(l2p1->x, l2p2->x);
	 if ((mnl1 < mxl2 && mxl1 > mxl2) || (mxl1 > mnl2 && mnl1 < mnl2)
	 || (mxl1 <= mxl2 && mnl1 >= mnl2) || (mxl2 <= mxl1 && mnl2 >= mnl1))
	    return_value = TRUE;/* colinear intersects */
      }
      else
	 return_value = lintersect(l1p1, l1p2, l2p1, l2p2);
   }
   else
      return_value = lintersect(l1p1, l1p2, l2p1, l2p2);
   return (return_value);
}


/*-----------------------------------------------------------------------------

			LINTERSECT, TRUE_LINTERSECT, and SAME
                      algorithm for true_lintersect and same from
                               ALGORITHMS by Sedgewick
                                       p. 313
    			    Does not handle colinear
------------------------------------------------------------------------------*/

int
true_lintersect(struct vertex_list_element * l1p1,
		struct vertex_list_element * l1p2,
		struct vertex_list_element * l2p1,
		struct vertex_list_element * l2p2)
{
   return ((same(l1p1, l1p2, l2p1, l2p2) <= 0)
	   && (same(l2p1, l2p2, l1p1, l1p2) <= 0));
}

/*----------------------------------------------------------------------------*/

int
lintersect(struct vertex_list_element * l1p1,
	   struct vertex_list_element * l1p2,
	   struct vertex_list_element * l2p1,
	   struct vertex_list_element * l2p2)
{
   /* <= changed to <  in this routine - not pure intersect */
   return ((same(l1p1, l1p2, l2p1, l2p2) < 0)
	   && (same(l2p1, l2p2, l1p1, l1p2) < 0));
}

/*----------------------------------------------------------------------------*/

double
same(struct vertex_list_element * p1,
     struct vertex_list_element * p2,
     struct vertex_list_element * q1,
     struct vertex_list_element * q2)
{
   double          dx, dy, dx1, dx2, dy1, dy2;

   dx = (p2->x) - (p1->x);
   dy = (p2->y) - (p1->y);
   dx1 = (q1->x) - (p1->x);
   dy1 = (q1->y) - (p1->y);
   dx2 = (q2->x) - (p2->x);
   dy2 = (q2->y) - (p2->y);
   return ((double) (dx * dy1 - dy * dx1) * (dx * dy2 - dy * dx2));
}

/*--------------------------------------------------------------------------

				COLINEAR

		Determines if two lines are colinear
		Returns TRUE if they are, FALSE otherwise

-----------------------------------------------------------------------------*/

int
colinear(
	 struct vertex_list_element * l1p1,
	 struct vertex_list_element * l1p2,
	 struct vertex_list_element * l2p1,
	 struct vertex_list_element * l2p2)
{
   double          dx1, dx2, dy1, dy2;

   dx1 = (l1p2->x) - (l1p1->x);
   dx2 = (l2p2->x) - (l2p1->x);
   dy1 = (l1p2->y) - (l1p1->y);
   dy2 = (l2p2->y) - (l2p1->y);
   if ((dx1 == 0) && (dx2 == 0))
   {				/* parallel special case */
      if (true_lintersect(l1p1, l1p2, l2p1, l2p2))	/* colinear! */
	 return (TRUE);
      else
	 return (FALSE);
   }
   if (((dx1 == 0) && (dx2 != 0)) || ((dx2 == 0) && (dx1 != 0)))	/* different slopes */
      return (FALSE);

   /* compute slope */
   if ((dy1 / dx1) == (dy2 / dx2))
   {
      if (true_lintersect(l1p1, l1p2, l2p1, l2p2))	/* colinear! */
	 return (TRUE);
   }
   return (FALSE);
}


/*-----------------------------------------------------------------------------

				CROSS_VERTEX2

	Similar to cross vertex, but returns pointer to an allocated vertex

------------------------------------------------------------------------------*/

struct vertex_list_element *
cross_vertex2(struct vertex_list_element *vertex,struct vertex_list_element *line)
{
   int             stat1, stat2;
   struct vertex_list_element *new1;
   double          a, b, c, d;

   new1 = malloc_vertex();
   new1->features = NULL;

   stat1 = line_equation(vertex, vertex->next, &a, &b);
   stat2 = line_equation(line, line->next, &c, &d);

   if (stat1 && stat2)
   {
      /* if horzontal lines (zero slope) */
      if( fabs(a) < 0.0001 && fabs(c) < 0.001 )
      {
         new1->x = vertex->x;
         new1->y = vertex->y;
      }
      else
      {
         new1->x = (d - b) / (a - c);
         new1->y = (c * b - a * d) / (c - a);
      }
   }
   else
   {
      if (!stat1)
      {
	 new1->x = vertex->x;
	 if ((line->y - line->next->y) == 0)
	    new1->y = line->y;
	 else
	    new1->y = c * new1->x + d;
      }
      else
      {
	 new1->x = line->x;
	 if ((vertex->y - vertex->next->y) == 0)
	    new1->y = vertex->y;
	 else
	    new1->y = a * new1->x + b;
      }
   }
   new1->corresponding = line->corresponding;

   return (new1);
}

/*----------------------------------------------------------------------------

			OBSTACLE_CHECKNOINTERSECT

		check to see if a DISJOINT region has any lines in it
		that intersect with a specified line
		Return TRUE if no intersection, FALSE if there is.

------------------------------------------------------------------------------*/

int
obstacle_checknointersect(struct vertex_list_element * pt1,
			  struct vertex_list_element * pt2,
			  struct vertex_list_element * region_ptr,
			  struct vertex_list_element ** return_vertex)
{
   //struct vertex_list_element *current, *hold, *new, *cross_vertex2();
   struct vertex_list_element *current, *hold, *new1;
   double          length;

   current = region_ptr;	/* get first vertex */
   hold = NULL;
   length = PATH_PLAN_INFINITY;
   if (colinear(pt1, pt2, pt1, current) && (min(pt1->x, pt2->x) < current->x) &&
       (max(pt1->x, pt2->x) > current->x) && (min(pt1->y, pt2->y) < current->y) &&
       (max(pt1->y, pt2->y) > current->y) &&
       pt1->next != current->next && pt2->next != current->next)
   {
      hold = current;
      length = find_length(pt1, current);
   }
   if (intersect(pt1, pt2, current, current->next))
   {
      new1 = cross_vertex2(pt1, current);
      hold = current;
      length = find_length(pt1, new1);
   }
   current = current->next;
   while (current != region_ptr)
   {				/* test remaining  vertices */
      if (colinear(pt1, pt2, pt1, current) && (min(pt1->x, pt2->x) < current->x) &&
	  (max(pt1->x, pt2->x) > current->x) && (min(pt1->y, pt2->y) < current->y) &&
	  (max(pt1->y, pt2->y) > current->y) &&
	  pt1->next != current->next && pt2->next != current->next)
      {
	 if (find_length(pt1, current) < length)
	 {
	    hold = current;
	    length = find_length(pt1, current);
	 }

      }
      if (intersect(pt1, pt2, current, current->next))
      {
	 new1 = cross_vertex2(pt1, current);
	 if (find_length(pt1, new1) < length)
	 {
	    hold = current;
	    length = find_length(pt1, new1);
	 }
      }
      current = current->next;
   }
   if (hold == NULL)
   {
      *return_vertex = NULL;
      return (TRUE);
   }
   else
   {
      *return_vertex = hold;
      return (FALSE);
   }
}


/*----------------------------------------------------------------------------

				COPY_REGION

	Function makes a copy of a region and returns pointer to it

-----------------------------------------------------------------------------*/

struct vertex_list_element *
copy_region(struct vertex_list_element * region)
{
   struct vertex_list_element *old, *head, *next;

   old = region;

   next = malloc_vertex();
   head = next;

   while (TRUE)
   {				/* process vertex */
      next->x = old->x;
      next->y = old->y;
      next->passable = old->passable;
      next->corresponding = old->corresponding;
      next->features = old->features;

      /* stop if are back to start of old list */
      old = old->next;
      if (old == region)
	 break;

      next->next = malloc_vertex();
      next->next->previous = next;
      next = next->next;
   }

   /* complete circular list */
   next->next = head;
   head->previous = next;

   return (head);
}


/*----------------------------------------------------------------------------

			LINE_SEGMENT_COLLIDES
		
		Boolean test to see if a given path segment collides
			with the obstacles or border
		Returns TRUE if a collision, FALSE otherwise

-----------------------------------------------------------------------------*/
int
line_segment_collides(struct path_node * p1,
		      struct path_node * p2,
		      struct stack_element * region_list)
{
   struct vertex_list_element v1, v2, *current;
   struct stack_element *now_region;

   v1.x = p1->x;
   v1.y = p1->y;
   v2.x = p2->x;
   v2.y = p2->y;

   now_region = region_list;
   while (now_region != NULL)
   {
      current = now_region->region->next;
      do
      {
	 if (current->passable == NULL)
	 {			/* not passable */
	    if (intersect(&v1, &v2, current, current->next))
	       return (TRUE);
	    if (colinear(&v1, &v2, &v1, current) &&
		(min(v1.x, v2.x) <= current->x) &&
		(max(v1.x, v2.x) >= current->x) &&
		(min(v1.y, v2.y) <= current->y) &&
		(max(v1.y, v2.y) >= current->y))
	       return (TRUE);
	 }
	 current = current->next;
      } while (current != now_region->region->next);
      now_region = now_region->next;
   }
   return (FALSE);
}

/* routine handles different regions as obstacles */
int
plus_line_segment_collides(struct path_node * p1,
			   struct path_node * p2,
			   struct stack_element * region_list)
{
   struct vertex_list_element v1, v2, *current;
   struct stack_element *now_region;
   struct terrain_identifier *terra;

   v1.x = p1->x;
   v1.y = p1->y;
   v2.x = p2->x;
   v2.y = p2->y;
   terra = p2->terrain_type;
   now_region = region_list;
   while (now_region != NULL)
   {
      current = now_region->region->next;
      do
      {
	 if ((current->passable == NULL) ||
	     (now_region->terrain_type->transition_zone))
	 {			/* not passable */
	    if (intersect(&v1, &v2, current, current->next))
	       return (TRUE);
	    if (colinear(&v1, &v2, &v1, current) &&
		(min(v1.x, v2.x) <= current->x) &&
		(max(v1.x, v2.x) >= current->x) &&
		(min(v1.y, v2.y) <= current->y) &&
		(max(v1.y, v2.y) >= current->y))
	       return (TRUE);
	 }
	 current = current->next;
      } while (current != now_region->region->next);
      now_region = now_region->next;
   }
   return (FALSE);
}


/*--------------------------------------------------------------------------

				PATH_LENGTH

		Computes total path_length of a given path

----------------------------------------------------------------------------*/

double
path_length(struct path_node * path)
{
   struct vertex_list_element p1, p2;
   struct path_node *current;
   double          total;

   current = path;

   if (current == NULL)
      return 0;

   total = 0;
   while (current->next_point != NULL)
   {
      p1.x = current->x;
      p1.y = current->y;
      p2.x = current->next_point->x;
      p2.y = current->next_point->y;
      total += find_length(&p1, &p2);
      current = current->next_point;
   }

   return total;
}


/*--------------------------------------------------------------------------

				PATH_COST

		Computes total cost including terrain of a given path

----------------------------------------------------------------------------*/

double
path_cost(struct path_node * path)
{
   struct vertex_list_element p1, p2;
   struct path_node *current;
   double          total;

   current = path;
   total = 0;
   if (current == NULL)
      return (total);
   while (current->next_point != NULL)
   {
      p1.x = current->x;
      p1.y = current->y;
      p2.x = current->next_point->x;
      p2.y = current->next_point->y;
      total = find_length(&p1, &p2) * current->terrain_type->traversal_factor;
      current = current->next_point;
   }
   return (total);
}

/*----------------------------------------------------------------------------

			remove_redundant_vertices

	removes duplicated verticies

-----------------------------------------------------------------------------*/

void remove_redundant_vertices(struct stack_element * regions)
{
   int flag;
   struct vertex_list_element *cur_vertex;
   struct stack_element *cur_region;

   /* remove redundant points */
   cur_region = regions;
   while( cur_region != NULL )
   {
      cur_vertex = cur_region->region->next;

      do
      {
         if( cur_vertex->x == cur_vertex->next->x &&
             cur_vertex->y == cur_vertex->next->y )
         {
	    /* delete the 2nd vertex node */

	    if (cur_vertex->next == cur_region->region->next)
	       flag = TRUE;

	    cur_vertex->next->next->previous = cur_vertex;
	    cur_vertex->next = cur_vertex->next->next;

	    if (flag)
	    {
	       /* fixup region pointer */
	       cur_region->region->next = cur_vertex->next;

	       /* go recheck this vertex to see if same as next one */
	       continue;
	    }
         }

	 /* move to next vertex */
         cur_vertex = cur_vertex->next;
      } while (cur_vertex != cur_region->region->next);

      /* move to next region */
      cur_region = cur_region->next;
   }
}



/*----------------------------------------------------------------------------

			ATTACH_OBSTACLES

	Attaches obstacles to border list, producing a single region

This seems to be only called with a single obstacle, never a list.
-----------------------------------------------------------------------------*/

void
attach_obstacles(struct obstacle_list_element * obstacle_list,
		 struct stack_element * border_list)
{
  struct obstacle_list_element *current_obstacle, *nearest_clear_obstacle;
  double minimum_distance, distance;
  struct vertex_list_element *bor = NULL, *obs = NULL, *temp_obs, *temp_bor;
  char msg[80];

  /*
   * get closest obstacle that is not blocked by another obstacle or by
   * border
   */

  while (TRUE)
    {
      minimum_distance = PATH_PLAN_INFINITY;
      nearest_clear_obstacle = NULL;
      current_obstacle = obstacle_list;

      /* while obstacles remaining */
      while (current_obstacle != NULL)
	{
	  if (current_obstacle->status == 1)
	    {
	      if (current_obstacle->actual_obstacle->looked_at != MERGED)
		{
		  distance = find_closest_clear(current_obstacle->grown_obstacle,
						border_list->region->next,
						obstacle_list, &temp_obs,
						&temp_bor);

		  if (distance < minimum_distance)
		    {		
		      /* new candidate */
		      nearest_clear_obstacle = current_obstacle;
		      minimum_distance = distance;
		      obs = temp_obs;
		      bor = temp_bor;
		    }
		}
	    }
	  current_obstacle = current_obstacle->next;
	}
      if (nearest_clear_obstacle == NULL)
	{				
	  /* all merged */
	  break;
	}

      border_list->region->next = merge_obstacle(border_list->region, obs, bor);

      /* set temp flag that have merged */
      nearest_clear_obstacle->actual_obstacle->looked_at = MERGED;
    } /* while TRUE */

  /* reset merged flags */
  current_obstacle = obstacle_list;
  while (current_obstacle != NULL)
    {
      if (current_obstacle->status == 1)
	{
	  if (current_obstacle->actual_obstacle->looked_at != MERGED)
	    {
	      sprintf(msg,"\n ERROR - Obstacle not merged !");
	      puts(msg);
	      abort();
	    }
	  current_obstacle->actual_obstacle->looked_at = 0;
	}
      current_obstacle = current_obstacle->next;
    }
}



struct anglecheck{
  double inangle;
  double outangle;
  struct vertex_list_element *vle;
  struct anglecheck *next;
};

/*----------------------------------------------------------------------------
			
			FIND_CLOSEST_CLEAR

	 	find closest unblocked vertex for 1 obstacle
	     checks all obstacles and border list for intersect
			Returns distance

The problem: This function returns the first closest link it can find, which may lead to a loop-the-loop
in the border, which leads to problems.  We need to make sure that it captures the correct "version" of
the point -- coming or going, if that point has more than one "version" (i.e., the point is one end of
a passable wall).
----------------------------------------------------------------------------*/

double
find_closest_clear(struct vertex_list_element * vertex_list,
		   struct vertex_list_element * border_vertex_list,
		   struct obstacle_list_element * obstacle_list,
		   struct vertex_list_element ** ret_obs,
		   struct vertex_list_element ** ret_bor)
{
  double          length, minimum_distance;
  struct obstacle_list_element *current_obstacle;
  struct vertex_list_element *current_vertex, *current_border;
  int             clear;

  current_vertex = vertex_list;
  minimum_distance = PATH_PLAN_INFINITY;
  current_border = border_vertex_list->next;
  do
    { /* loop for current_vertex through entire obstacle */
      do
	{ /* loop for current_border through entire border so far */
	  length = find_length(current_vertex, current_border);
	  if (length < minimum_distance)
	    { /* found a new closest link, so check it against the other obstacles */
	      current_obstacle = obstacle_list;
	      clear = TRUE;
	      while (current_obstacle != NULL)
		{
		  if (current_obstacle->status == 1)
		    {
		      if (vertex_list != current_obstacle->grown_obstacle)
			{
			  if (!obstacle_checknointersect2(current_vertex,
							  current_border, current_obstacle->grown_obstacle) ||
			      !checknointersect(current_vertex, current_border,
						border_vertex_list->next))
			    {
			      clear = FALSE;
			      break;
			    }
			}
		    }
		  current_obstacle = current_obstacle->next;
		}
	      /* if the new link didn't hit an obstacle, set it as 
		 the return value (until a better one is found).  */
	      if (clear) 
		{
		  minimum_distance = length;
		  *ret_bor = current_border;
		  *ret_obs = current_vertex;
		}
	    }
	  current_border = current_border->next;
	} while (current_border != border_vertex_list->next);
      current_vertex = current_vertex->next;
    } while (current_vertex != vertex_list);
  /* Now we have the closest link, but maybe not the right member of the border linked list. 
     We need to look for others in the same location and verify which one is appropriate. */
  { /* RRB */
    struct anglecheck *angleptr = NULL, *angleptrlast = NULL, *angleptrhead = NULL, *angleptrnew = NULL;
    struct vertex_list_element *vertptr = border_vertex_list->next;
    double newangle;
    int flip;
    do{ /* loop for vertex through border_vertex_list */
      /* If the vertex matches... */
      if((vertptr->x == (*ret_bor)->x) && (vertptr->y == (*ret_bor)->y)){
	angleptrnew = (struct anglecheck *)malloc(sizeof(struct anglecheck));
	angleptrnew->next = NULL;
	angleptrnew->vle = vertptr;
	if(!angleptrhead){ /* if this is the first one... */
	  angleptrhead = angleptrnew;
	  angleptrlast = angleptrnew;
	}
	else{
	  angleptrlast->next = angleptrnew;
	  angleptrlast = angleptrnew;
	}
	if(((vertptr->previous->y == vertptr->y) &&
	    (vertptr->previous->x == vertptr->x))   || 
	   ((vertptr->next->y == vertptr->y) &&
	    (vertptr->next->x == vertptr->x))){
	  printf("cart_lib:find_closest_near -- Two adjacent border points are co-located\n");
	  abort();
	}
	angleptrnew->inangle = atan2(vertptr->previous->y - vertptr->y,
				     vertptr->previous->x - vertptr->x); 
	angleptrnew->outangle = atan2(vertptr->next->y - vertptr->y,
				      vertptr->next->x - vertptr->x); 
	/*	printf("New anglecheck struct at %5.2f %5.2f\n", vertptr->x, vertptr->y);
		printf("In: %5.2f Out: %5.2f \n", angleptrnew->inangle, angleptrnew->outangle);
	*/
      }
      vertptr = vertptr->next;
    } while (vertptr != border_vertex_list->next);
    
    /* If there is only one angle, then there is no issue. */
    if(!angleptrhead->next){
      free(angleptrhead);
      return (minimum_distance);
    }

    /* Now we have a linked list of two or more candidates.
       Next step is to find the correct open angle that contains the new link.
    */
    angleptr = angleptrhead;
    newangle = atan2((*ret_obs)->y - (*ret_bor)->y, (*ret_obs)->x - (*ret_bor)->x);
    while(angleptr != NULL){
      if(angleptr->inangle < angleptr->outangle) flip = 1;
      else flip = 0;

      /* If the incoming angle is in the current free angle, this is the one. */
      if(((flip == 0) && (newangle < angleptr->inangle) && (newangle > angleptr->outangle)) ||
	 ((flip == 1) && ((newangle < angleptr->inangle) || (newangle > angleptr->outangle))))
	{
	  *ret_bor = angleptr->vle;
	  break;
	}
      angleptr = angleptr->next;
    }
    /* finally, we need to free the malloc'ed storage */
    angleptr = angleptrhead;
    while(angleptr){
      angleptrlast = angleptr;
      angleptr = angleptr->next;
      free(angleptrlast);
    }
  } /* RRB */
  return (minimum_distance);
}


/*----------------------------------------------------------------------------

				MERGE_OBSTACLE

			Merge an obstacle with the border list
			Requires duplication of some vertices
			and appropriate passable adjustment
		Passables must be patched later by patch_for_obstacles

-----------------------------------------------------------------------------*/

struct vertex_list_element *
merge_obstacle(
	       struct vertex_list_element * border_list,
	       struct vertex_list_element * obs,
	       struct vertex_list_element * bor)
{
  struct vertex_list_element *current_border, *temp, *temp_obs, *hold;


  /* RRB
  printf("merge_obs: obs: %5.2f %5.2f %s\t bor: %5.2f %5.2f %s\n",
	 obs->x, obs->y, (obs->passable?"PASS":"WALL"),
	 bor->x, bor->y, (bor->passable?"PASS":"WALL"));
  */
   /* find the border record */
  current_border = border_list->next;

  /* Why not just set current_border = bor? */
  while (current_border != bor)
    {
      current_border = current_border->next;
    }
  /* So now current_border = bor... */

  /* remember next pointer for later */
  hold = current_border->next;

  /* insert a new record for the obstacle info and copy the data in */
  current_border->next = malloc_vertex();
  current_border->next->features = NULL;
  temp = current_border->next;

  temp->x = obs->x;
  temp->y = obs->y;
  temp->passable = obs->passable;
  temp->corresponding = obs->corresponding;
  temp->previous = current_border;

  temp_obs = obs->next;
  while (obs != temp_obs)
    {
      temp->next = malloc_vertex();
      temp->next->previous = temp;
      temp = temp->next;
      temp->features = NULL;
      temp->x = temp_obs->x;
      temp->y = temp_obs->y;
      temp->passable = temp_obs->passable;
      temp->corresponding = temp_obs->corresponding;
      temp_obs = temp_obs->next;
    }

  /* duplicate first obstacle vertex */
  temp->next = malloc_vertex();
  temp->next->previous = temp;
  temp = temp->next;
  temp->features = NULL;
  temp->x = obs->x;
  temp->y = obs->y;
  temp->passable = current_border;
  temp->corresponding = obs->corresponding;

	 /* duplicate first border_vertex */
  temp->next = malloc_vertex();
  temp->next->previous = temp;
  temp = temp->next;
  temp->features = NULL;
  temp->x = current_border->x;
  temp->y = current_border->y;
  temp->passable = current_border->passable;
  temp->corresponding = current_border->corresponding;

  temp->next = hold;
  hold->previous = temp;
  /* RRB added to fix connectivity of graph: */
  current_border->passable = current_border->next;

  return border_list->next;
}


/*-----------------------------------------------------------------------------

			PATCH_FOR_OBSTACLES

	 checks to make sure that all passables in a stack
		refer to regions on that stack
	if they don't, finds the region they do point to and corrects passable

------------------------------------------------------------------------------*/

void
patch_for_obstacles(struct stack_element * convex_list)
{
   struct stack_element *hold_stack, *current_stack;
   struct vertex_list_element *current_vertex, *hold_vertex;
   int             done, found;

   current_stack = convex_list;
   do
   {				/* next region */
      current_vertex = current_stack->region->next;
      do
      {				/* next vertex */
	 if (current_vertex->passable != NULL)
	 {
	    found = FALSE;
	    hold_stack = convex_list;
	    do
	    {			/* next region */
	       hold_vertex = hold_stack->region->next;
	       do
	       {		/* next vertex */
		  if (current_vertex->passable == hold_vertex->next)
		  {
		     found = TRUE;	/* region is on list */
		     break;
		  }
		  hold_vertex = hold_vertex->next;
	       } while (hold_vertex != hold_stack->region->next);
	       if (found == TRUE)
		  break;
	       hold_stack = hold_stack->next;
	    } while (hold_stack != NULL);
	    if (!found)
	    {
	       hold_stack = convex_list;
	       done = FALSE;
	       do
	       {		/* next region */
		  hold_vertex = hold_stack->region->next;
		  do
		  {		/* next vertex */
		     if (hold_vertex->next->x == current_vertex->x &&
			 hold_vertex->next->y == current_vertex->y &&
			 hold_vertex->x == current_vertex->next->x &&
			 hold_vertex->y == current_vertex->next->y)
		     {
			current_vertex->passable = hold_vertex;
			done = TRUE;
			break;
		     }
		     hold_vertex = hold_vertex->next;
		  } while (hold_vertex != hold_stack->region->next);
		  if (done)
		     break;
		  hold_stack = hold_stack->next;
	       } while (hold_stack != NULL);
	    }
	 }
	 current_vertex = current_vertex->next;
      } while (current_vertex != current_stack->region->next);
      current_stack = current_stack->next;
   } while (current_stack != NULL);
   return;
}



/*----------------------------------------------------------------------------

			GROW_OBSTACLE_VERTEX

		Grows in the opposite direction of GROW

-----------------------------------------------------------------------------*/
static struct vertex_list_element *
grow_obstacle_vertex(struct vertex_list_element * p2,double length)
{
   double          angle, theta, phi;
   struct vertex_list_element v2, *grown, *p1, *p3;

   grown = malloc_vertex();
   grown->features = NULL;
   p1 = p2->previous;
   p3 = p2->next;
   theta = compute_angle(p1, p2, p3);
   v2.x = p2->x + 5.0;
   v2.y = p2->y;
   if (!correctside(p1, p2, p3))
   {				/* concave */
      theta = 360.0 - theta;
   }
   theta = theta / 2;
   phi = compute_angle(&v2, p2, p3);
   if ((phi == 0) && (p2->y == p3->y) && (p3->x < p2->x))
      phi = 180.0;
   if (correctside(&v2, p2, p3))
      phi = 360.0 - phi;
   theta = theta + 180.0;
   angle = theta + phi;
   if (sin(RADIANS_FROM_DEGREES(theta)) != 0)
      length = length / sin(RADIANS_FROM_DEGREES(theta));
   grown->x = p2->x + length * cos(RADIANS_FROM_DEGREES(angle));
   grown->y = p2->y + length * sin(RADIANS_FROM_DEGREES(angle));
   return (grown);
}


/*----------------------------------------------------------------------------

			OBSTACLE_CHECKNOINTERSECT2

		Same as obstacle_checknointersect but no grown vertex made
		and no return vertex given
		Returns TRUE if NO intersection, FALSE if there is

----------------------------------------------------------------------------*/

int
obstacle_checknointersect2(struct vertex_list_element * pt1,
			   struct vertex_list_element * pt2,
			   struct vertex_list_element * region_ptr)
{
   struct vertex_list_element *current;

   current = region_ptr;	/* get first vertex */
   if (colinear(pt1, pt2, pt1, current) &&
       (min(pt1->x, pt2->x) < current->x) &&
       (max(pt1->x, pt2->x) > current->x) &&
       (min(pt1->y, pt2->y) < current->y) &&
       (max(pt1->y, pt2->y) > current->y) &&
       pt1->next != current->next && pt2->next != current->next)
      return (FALSE);
   if (intersect(pt1, pt2, current, current->next))
      return (FALSE);
   current = current->next;
   while (current != region_ptr)
   {				/* test remaining  vertices */
      if (colinear(pt1, pt2, pt1, current) &&
	  (min(pt1->x, pt2->x) < current->x) &&
	  (max(pt1->x, pt2->x) > current->x) &&
	  (min(pt1->y, pt2->y) < current->y) &&
	  (max(pt1->y, pt2->y) > current->y) &&
	  pt1->next != current->next && pt2->next != current->next)
	 return (FALSE);

      if (intersect(pt1, pt2, current, current->next))
	 return (FALSE);
      current = current->next;
   }
   return (TRUE);
}

/*-------------------------------------------------------------------------

			CHOP_CONCAVE_ANGLE

		Routine to cut down a concave angle from one vertex to
		two, reducing forward projection.

is_obstacle is TRUE if vertex on obstacle and FALSE if on border
--------------------------------------------------------------------------*/

static struct vertex_list_element *
chop_concave_angle(struct vertex_list_element * vertex,
		   struct vertex_list_element * next_to_add,
		   int is_obstacle,double length)

{
   double          alpha, distance, r, angle, dx, dy;
   struct vertex_list_element *long_way, temp, temp2, *middle;
   struct vertex_list_element *newtemp, *check;
   int             flag1, flag2;

   if (is_obstacle)
      long_way = grow_obstacle_vertex(vertex,length);
   else				/* is border */
      long_way = grow(vertex,length);

   /* math computation */
   alpha = compute_angle(vertex->previous, vertex, vertex->next);
   distance = find_length(vertex, long_way);
   r = (distance - length) * tan(RADIANS_FROM_DEGREES(alpha / 2));
   temp.x = vertex->x + 5.0;
   temp.y = vertex->y;
   temp.passable = NULL;
   angle = compute_angle(&temp, vertex, long_way);
   if ((angle == 0) && (vertex->y == long_way->y) && (long_way->x < vertex->x))
      angle = 180.0;


   if (!is_obstacle)
   {				/* case on obstacle */
      if (!correctside(&temp, vertex, long_way))
	 angle = 180.0;
      else
	 angle = 180.0 - angle;
   }
   else
   {				/* case on border */
      if (!correctside(&temp, vertex, long_way))
	 angle = angle;
      else
	 angle = 360.0 - angle;
   }
   dx = r * sin(RADIANS_FROM_DEGREES(angle));
   dy = r * cos(RADIANS_FROM_DEGREES(angle));
   middle = grow_middle(vertex,length);
   temp.x = middle->x + dx;
   temp.y = middle->y - dy;
   temp2.x = middle->x - dx;
   temp2.y = middle->y + dy;
   temp2.passable = NULL;
   temp.corresponding = temp2.corresponding = vertex;
   check = vertex->next;
   flag2 = flag1 = FALSE;
   do
   {				/* check to see if clear */
      newtemp = grow(check,length);
      if (intersect(vertex, &temp, check, newtemp) && (!concave(check)))
	 flag1 = TRUE;
      if (intersect(vertex, &temp2, check, newtemp) && (!concave(check)))
	 flag2 = TRUE;
      check = check->next;
   } while (check != vertex);
   if (flag1)
   {				/* if blocked , use long vertex */
      temp.x = long_way->x;
      temp.y = long_way->y;
   }
   if (flag2)
   {				/* if blocked, use long vertex */
      temp2.x = long_way->x;
      temp2.y = long_way->y;
   }

   /* order of addition is different for obstacles */
   if (is_obstacle)
   {
      next_to_add = add_vertex(next_to_add, &temp2);
      next_to_add = add_vertex(next_to_add, &temp);
   }
   else
   {
      next_to_add = add_vertex(next_to_add, &temp);
      next_to_add = add_vertex(next_to_add, &temp2);
   }
   return (next_to_add);
}

/*----------------------------------------------------------------------------

				FIXUP_OVERLAP_BORDER

			Takes a rough region and removes twisted parts
			and redundant vertices

-----------------------------------------------------------------------------*/

void
fixup_overlap_border(struct stack_element * stack)
{
   struct vertex_list_element *head, *vertex, *line;
   int             flag;

   head = stack->region;
   /* get interior vertex */
   /* assume it is first for now */
   vertex = head->next;
   /* remove crossed regions */
   do
   {				/* for entire region */
      line = vertex->next->next;
      while (line->next != vertex)
      {
	 if (intersect(vertex, vertex->next, line, line->next))
	 {
	    cross_vertex(vertex, line);	/* not cross2! doctors list */
	 }
	 line = line->next;
      }
      vertex = vertex->next;
   } while (vertex != head->next);
   head->next = vertex;

   /* remove redundant points */
   vertex = head->next;
   flag = FALSE;
   do
   {				/* for entire region */
      if ((colinear(vertex, vertex->next, vertex->next, vertex->next->next) &&
	   (find_length(vertex, vertex->next) <= find_length(vertex, vertex->next->next))) ||
	  ((vertex->x == vertex->next->x) && (vertex->y == vertex->next->y)))
      {
	 if (vertex->next == head->next)
	    flag = TRUE;
	 vertex->next->next->previous = vertex;
	 vertex->next = vertex->next->next;
	 if (flag)
	 {
	    head->next = vertex->next;
	    break;
	 }
      }
      vertex = vertex->next;
   } while (vertex != head->next);
   return;
}

/*------------------------------------------------------------------------------
		
				CROSS VERTEX

		Doctors a crossed list - excises all the crossed vertices

------------------------------------------------------------------------------*/


void
cross_vertex(struct vertex_list_element * vertex,
	     struct vertex_list_element * line)
{
   int             stat1, stat2;
   struct vertex_list_element *new1;
   double          a, b, c, d;

   new1 = malloc_vertex();
   new1->features = NULL;
   stat1 = line_equation(vertex, vertex->next, &a, &b);
   stat2 = line_equation(line, line->next, &c, &d);


   if (stat1 && stat2)
   {				/* normal case */
      /* if horzontal lines (zero slope) */
      if( fabs(a) < 0.0001 && fabs(c) < 0.001 )
      {
         new1->x = vertex->x;
         new1->y = vertex->y;
      }
      else
      {
         new1->x = (d - b) / (a - c);
         new1->y = (c * b - a * d) / (c - a);
      }
   }
   else
   {				/* abnormal */
      if (!stat1)
      {
	 new1->x = vertex->x;
	 if ((line->y - line->next->y) == 0)
	    new1->y = line->y;
	 else
	    new1->y = c * new1->x + d;
      }
      else
      {
	 new1->x = line->x;
	 if ((vertex->y - vertex->next->y) == 0)
	    new1->y = vertex->y;
	 else
	    new1->y = a * new1->x + b;
      }
   }
   /* tie off list */
   new1->next = line->next;
   new1->corresponding = line->corresponding;
   line->next->previous = new1;
   vertex->next = new1;
   new1->previous = vertex;
}

/*--------------------------------------------------------------------------

				LINE EQUATION

		computes slope and intercept for a line from 2 points
			if dx = 0 returns FALSE, otherwise TRUE

--------------------------------------------------------------------------*/

int
line_equation(struct vertex_list_element * p1,
	      struct vertex_list_element * p2,
	      double *slope,
	      double *intercept)
{
   double          dx, dy;

   dx = p2->x - p1->x;
   if (dx == 0)
      return (FALSE);

   dy = p2->y - p1->y;

   *slope = dy / dx;
   *intercept = p2->y - ((*slope) * p2->x);

   return (TRUE);
}

/*-----------------------------------------------------------------------------

			FIXUP_OVERLAP_OBSTACLE

		Handle overlapping obstacles

---------------------------------------------------------------------------*/

void
fixup_overlap_obstacle(struct obstacle_list_element * obstacle)
{
   struct vertex_list_element *head, *vertex, *line;
   int             flag;

   head = obstacle->grown_obstacle;
   /* get interior vertex */
   /* assume it is first for now */
   vertex = head->next;

   do
   {
      line = vertex->next->next;
      while (line->next != vertex)
      {
	 if (intersect(vertex, vertex->next, line, line->next))
	 {
	    cross_vertex(vertex, line);
	 }
	 line = line->next;
      }
      vertex = vertex->next;
   } while (vertex != head->next);
   head->next = vertex;

   /* remove redundant points */
   vertex = head->next;
   flag = FALSE;
   do
   {
      if (colinear(vertex, vertex->next, vertex->next, vertex->next->next) &&
	  (find_length(vertex, vertex->next) <= find_length(vertex, vertex->next->next)))
      {
	 if (vertex->next == head->next)
	    flag = TRUE;
	 vertex->next->next->previous = vertex;
	 vertex->next = vertex->next->next;
	 if (flag)
	 {
	    head->next = vertex->next;
	    break;
	 }
      }
      vertex = vertex->next;
   } while (vertex != head->next);
   return;
}

/*----------------------------------------------------------------------------

			OBSTACLE_INTERSECT_BORDER

	Checks to see if obstacle intersects border
	Returns TRUE if it does otherwise FALSE
	Border_return holds vertex that starts intersecting border segment
	Vertex_return holds vertex that starts obstacle intersection
	Both border_return and Vertex_return are NULL if no intersection

----------------------------------------------------------------------------*/

int
obstacle_intersect_border(
    struct vertex_list_element *obstacle_region,
    struct vertex_list_element *border,
    struct vertex_list_element **return_vertex,
    struct vertex_list_element **border_return)
{
   struct vertex_list_element *border_collide, *current;

   current = border;
   do
   {				/* for whole border */
      if (!obstacle_checknointersect(current, current->next, obstacle_region, &border_collide))
      {
	 *border_return = current;
	 *return_vertex = border_collide;
	 return (TRUE);		/* obstacle intersects */
      }
      current = current->next;
   } while (current != border);
   *border_return = *return_vertex = NULL;
   return (FALSE);
}

/*----------------------------------------------------------------------------

			OBSTACLE_INSIDE_BORDER

		Boolean test to see if an obstacle that does not
		intersect border is inside border.
		Returns TRUE if inside, FALSE otherwise.
		Takes advantage of fact that a line drawn to infinity from one vertex
		must intersect border an even number of times if it is inside
		the region,  odd number if outside.  Infinite point is -10,-10, since
		no negative vertices are allowed.

------------------------------------------------------------------------------*/

int
obstacle_inside_border(struct vertex_list_element *obstacle, struct vertex_list_element *border)
{

   int             intersect_count;
   int             rtnval;
   struct vertex_list_element *pt1;
   struct vertex_list_element pt2;	/* not a pointer ! */
   struct vertex_list_element *current;

   intersect_count = 0;		/* initialize */
   pt1 = obstacle;
   pt2.x = -10.0;		/* point is at infinity since only positive
				 * vertices allowed */
   pt2.y = -10.0;

   current = border;		/* get first vertex */

   do
   {
   if (colinear(pt1, &pt2, pt1, current) &&
       (min(pt1->x, pt2.x) < current->x) &&
       (max(pt1->x, pt2.x) > current->x) &&
       (min(pt1->y, pt2.y) < current->y) &&
       (max(pt1->y, pt2.y) > current->y))
   {
      intersect_count++;
   }
   else
   {
      if (intersect(pt1, &pt2, current, current->next))
      {
	 intersect_count++;
      }
   }
   current = current->next;
   } while (current != border);



   if (intersect_count % 2 == 0)
      rtnval =  FALSE;		/* even denotes outside */
   else
      rtnval =  TRUE;		/* odd denotes inside */

   return rtnval;
}

/*-----------------------------------------------------------------------------

				MERGE_OVERLAP_OBSTACLES_BORDER

		Merge overlapping obstacles into border

------------------------------------------------------------------------------*/

/* dcm : completely rewrote this routine.  I wonder why */

void
merge_overlap_obstacles_border(struct obstacle_list_element * obstacles,
			       struct stack_element * border_stack)
{
   /*
    * all obstacle status = 0 initially (unknown) status = -1 outside border
    * status = 1 inside border status = 2 merged into border status = 3
    * merged with earlier inside obstacle
    */

   struct obstacle_list_element *current_obstacle;
   struct vertex_list_element *border_collide, *vertex_collide;
   struct stack_element *current_region;
   //int             obstacle_intersect_border(), obstacle_inside_border();
   int             done;

   current_obstacle = obstacles;
   done = FALSE;
   
   /* for all obstacles */
   while (!done && current_obstacle != NULL)
   {
      /* if not yet merged */
      if (current_obstacle->status != 2)
      {
         /* check each region */
         current_region = border_stack;
	 while (!done && current_region != NULL)
	 {
	    /* check if should be merged with this border */
	    if (obstacle_intersect_border(current_obstacle->grown_obstacle,
	         current_region->region->next, &vertex_collide, &border_collide))
	    {
	       merge_with_border(vertex_collide, border_collide);

	       current_obstacle->status = 2;	/* merged into border */
	       done = TRUE;
	    }
	    else
	    {
	       if (obstacle_inside_border(current_obstacle->grown_obstacle, current_region->region->next))
	       {
	          current_obstacle->status = 1;	/* inside */
	       }
	       else
	       {
		  /* do an or operation on inclusion */
		  if( current_obstacle->status != 1 )
	             current_obstacle->status = -1;	/* outside border */
	       }
	    }

            /* move to next region */
	    current_region = current_region->next;
	 }

      }

      /* move to next obstacle */
      current_obstacle = current_obstacle->next;
   }

   return;
}

/*-----------------------------------------------------------------------------

			MERGE_WITH_BORDER

		Merges a collided obstacle with the border
		Performs surgery on border list

----------------------------------------------------------------------------*/

void
merge_with_border(struct vertex_list_element * obstacle_collide,
		  struct vertex_list_element * border_collide)
{
   struct vertex_list_element *bc2, *next, *new1, *border_hit, *current,
                  *new2;
   //struct vertex_list_element *cross_vertex2(), *add_vertex();

   next = new1 = cross_vertex2(border_collide, obstacle_collide);
   new1->passable = NULL;
   current = obstacle_collide->next;

   new1->next = obstacle_collide->next;
   if (!obstacle_checknointersect(new1, obstacle_collide->next, border_collide, &bc2))
   {
      if (bc2 != border_collide)
      {
	 new1->next = obstacle_collide->next;
	 new1->next = new2 = cross_vertex2(new1, bc2);
	 new2->passable = NULL;
	 new2->next = bc2->next;
	 bc2->next->previous = new2;
	 border_collide->next = new1;
	 new1->previous = border_collide;
	 new2->previous = new1;
	 return;		/* early return in case of same line segment
				 * intersecting twice */
      }
   }

   /* add intervening vertices */
   while( obstacle_checknointersect(current, current->next, border_collide, 
	  &border_hit))
   {
      next    = add_vertex(next, current);
      current = current->next;
   }
   next = add_vertex(next, current);	/* trailing vertex */

   next->next = new2 = cross_vertex2(current, border_hit);/* last to add */

   new2->passable = NULL;
   new2->next = border_hit->next;
   border_hit->next->previous = new2;
   border_collide->next = new1;
   new1->previous = border_collide;
   new2->previous = next;

   round_border(border_collide);
}

/*---------------------------------------------------------------------------

			MERGE_OBSTACLE_TO_OBSTACLE

		Merges fully interior obstacles to other fully
		interior obstacles if they overlap.

-----------------------------------------------------------------------------*/

void
merge_obstacle_to_obstacle(struct obstacle_list_element * obstacles)
{
   struct obstacle_list_element *next_obs, *current_obs;
   struct vertex_list_element *new1, *new2, *border_collide, *border_hit;
   struct vertex_list_element *obstacle_collide;
   struct vertex_list_element *bc2, *next, *current;

   current_obs = obstacles;
   while (current_obs != NULL)
   {				/* for all obstacles */
      if (current_obs->status == 1)
      {				/* inside only */
	 next_obs = current_obs->next;
	 while (next_obs != NULL)
	 {
	    if (next_obs->status == 1)
	    {			/* inside also */
	       if (obstacle_intersect_border(current_obs->grown_obstacle, next_obs->grown_obstacle,
					&obstacle_collide, &border_collide))
	       {
		  next = new1 = cross_vertex2(border_collide, obstacle_collide);
		  new1->passable = NULL;
		  current = obstacle_collide->next;
		  new1->next = obstacle_collide->next;
		  if (!obstacle_checknointersect(new1, obstacle_collide->next,
						 border_collide, &bc2))
		  {
		     if (bc2 != border_collide)
		     {
			new1->next = obstacle_collide->next;
			new1->next = new2 = cross_vertex2(new1, bc2);
			new2->passable = NULL;
			new2->next = bc2->next;
			bc2->next->previous = new2;
			border_collide->next = new1;
			new1->previous = border_collide;
			new2->previous = new1;
			next_obs->status = 3;
			goto bypass;	/* special case - 2 points on same
					 * line */
		     }
		  }

		  do
		  {
		     if (!obstacle_checknointersect(current, current->next, border_collide, &border_hit))
			break;
		     next = add_vertex(next, current);
		     current = current->next;
		  } while (TRUE);
		  next = add_vertex(next, current);
		  next->next = new2 = cross_vertex2(current, border_hit);
		  new2->passable = NULL;
		  new2->next = border_hit->next;
		  border_hit->next->previous = new2;
		  border_collide->next = new1;
		  new1->previous = border_collide;
		  new2->previous = next;
		  current_obs->grown_obstacle = new1;
		  next_obs->status = 3;
		  round_obstacle(current_obs);	/* remove rough edges
						 * (precision) */
	       }
	    }
      bypass:next_obs = next_obs->next;
	 }
      }
      current_obs = current_obs->next;
   }
   return;
}

#if 0
/*--------------------------------------------------------------------------

				CONVEX

			determines if a region is convex
			Returns TRUE if it is convex, FALSE otherwise

-----------------------------------------------------------------------------*/

int convex(struct vertex_list_element *list)
{
   struct vertex_list_element *vertex1, *current;

   /* check each line to insure that all other points are convex to it */
   vertex1 = list;
   do
   {
      current = vertex1->next->next;
      do
      {
	 if (!inclusive_correctside(vertex1, vertex1->next, current))
	    return (FALSE);
	 current = current->next;
      } while (current != vertex1->next->next)

      vertex1 = vertex1->next;
   } while (vertex1 != list);

   return (TRUE);
}
#endif
/*--------------------------------------------------------------------------

				SLOP_CONVEX

			determines if a region is convex within bounds
			Returns TRUE if it is convex, FALSE otherwise

-----------------------------------------------------------------------------*/

int
slop_convex(struct vertex_list_element * list)
/* pointer to region in question */
{
   struct vertex_list_element ccurrent, *anchor1, *anchor2, canchor1, canchor2;
   //double          round_float();
   int             rtn_val;

   /* anchor first segment */
   anchor1 = list;
   anchor2 = list->next;
   /* check first vertex */

   canchor1.x = round_float(anchor1->x);
   canchor1.y = round_float(anchor1->y);
   canchor2.x = round_float(anchor2->x);
   canchor2.y = round_float(anchor2->y);
   ccurrent.x = round_float(anchor2->next->x);
   ccurrent.y = round_float(anchor2->next->y);

   rtn_val = inclusive_correctside(&canchor1, &canchor2, &ccurrent);
#if 0
   rtn_val = correctside(&canchor1, &canchor2, &ccurrent);
#endif

   if (!rtn_val )
   {
      return (FALSE);
   }
   /* check remaining vertices */

   /* Repeat for remaining segments of region */
   anchor1 = anchor2;
   anchor2 = anchor2->next;
   while (anchor1 != list)
   {
      canchor1.x = round_float(anchor1->x);
      canchor1.y = round_float(anchor1->y);
      canchor2.x = round_float(anchor2->x);
      canchor2.y = round_float(anchor2->y);
      ccurrent.x = round_float(anchor2->next->x);
      ccurrent.y = round_float(anchor2->next->y);

      rtn_val = inclusive_correctside(&canchor1, &canchor2, &ccurrent);
#if 0
      rtn_val = correctside(&canchor1, &canchor2, &ccurrent);
#endif
      if (!rtn_val )
      {
	 return (FALSE);
      }
      anchor1 = anchor2;
      anchor2 = anchor2->next;
   }
   return (TRUE);
}

/*----------------------------------------------------------------------------

			       FIND_MEADOW

	       Accepts a point (in vertex format) and checks to see which,
	       if any, of the regions on the convex region stack passed
		       it is in.
	       Returns pointer to the region the point is in
		       or NULL if not in any.

-----------------------------------------------------------------------------*/

struct vertex_list_element *
find_meadow( struct vertex_list_element *pt, struct stack_element *meadow_stack)
{
   struct stack_element *meadow_ptr;
   struct vertex_list_element *meadow;

   meadow = meadow_stack->region->next;
   meadow_ptr = meadow_stack;
   while (!inside_meadow(pt, meadow))
   {
      meadow_ptr = meadow_ptr->next;

      /* not in any meadow */
      if (meadow_ptr == NULL)
      {
	 return (NULL);
      }
      meadow = meadow_ptr->region;
   }
   return (meadow_ptr->region);
}

/*----------------------------------------------------------------------------

				INSIDE_MEADOW

		Boolean function returning TRUE if point passed (in vertex
		format) is in region passed
		Read note below if point is a vertex.

------------------------------------------------------------------------------*/
int
inside_meadow(struct vertex_list_element * pt,
	      struct vertex_list_element * meadow_list)
{
  struct vertex_list_element *vertex1, *vertex2;

  vertex1 = meadow_list;
  vertex2 = meadow_list->next;

  if (!inclusive_correctside(vertex1, vertex2, pt))
    {
      return (FALSE);
    }

  vertex1 = vertex2;
  vertex2 = vertex2->next;
  while (inclusive_correctside(vertex1, vertex2, pt))
    {
      vertex1 = vertex2;
      vertex2 = vertex2->next;
      if (vertex1 == meadow_list->next)
	{
	  return (TRUE);
	}
    }

  /* just in case point is a convex vertex   */
  /* returns only 1st region it is in (not all) */
  /* if it is two or more regions, should be passable */

  /* Don't think these are possible with inclusive_correctside...*/
  if ((vertex2->x == pt->x) && (vertex2->y == pt->y))
    return (TRUE);

  if ((vertex1->x == pt->x) && (vertex1->y == pt->y))
    return (TRUE);

  return (FALSE);
}


/*-----------------------------------------------------------------------------

				MERGE_REGIONS

		create a new region from two others
		Return pointer to new region

------------------------------------------------------------------------------*/

struct vertex_list_element *
merge_regions(struct vertex_list_element * region1)
{
   struct vertex_list_element *next_to_add, *new_region, *current;

   /* create header for new region */
   new_region = malloc_vertex();
   new_region->features = NULL;
   new_region->x = region1->passable->next->x;
   new_region->y = region1->passable->next->y;
   new_region->passable = region1->passable->next->passable;
   new_region->corresponding = region1->passable->next->corresponding;
   new_region->next = region1->passable->next->next;

   /* complete adding region2 */
   current = region1->passable->next->next;
   next_to_add = new_region;
   while ((current->x != region1->next->x) ||
	  (current->y != region1->next->y))
   {
      next_to_add = add_vertex(next_to_add, current);
      current = current->next;
   }

   /* complete adding region1 */
   current = region1->next;
   next_to_add = add_vertex(next_to_add, current);
   current = current->next;
   while (current != region1)
   {
      next_to_add = add_vertex(next_to_add, current);
      current = current->next;
   }

   /* tie off list */
   new_region->previous = next_to_add;
   next_to_add->next = new_region;

   return (new_region);
}

/*----------------------------------------------------------------------------

				REPAIR_STACK

		Function that repairs a convex region stack after a merge
		of two of its regions has occurred.
		Passes back pointer to convex_stack_top or NULL if error.
		This is a DESTRUCTIVE OPERATION on the old stack.

-----------------------------------------------------------------------------*/
struct stack_element *repair_stack(
    struct vertex_list_element *new_region,
    struct stack_element *old1_stack_element,
    struct vertex_list_element *old2_region_vertex,
    struct stack_element *stack_head)
{
   struct stack_element *new_stack_element, *preceding;
   struct stack_element *current_stack;
   struct vertex_list_element *vertex;
   char msg[80];

   /* build new stack element - to replace two old ones */
   new_stack_element = malloc_stack_element();
   new_stack_element->region = new_region;
   new_stack_element->next = old1_stack_element->next;

   /* delete first old region  and splice in new_region */
   preceding = NULL;
   current_stack = stack_head;
   while (current_stack != NULL)
   {				/* search fo old1 region on stack */
      if (old1_stack_element == current_stack)
      {
	 /* found stack element  - delete it */
	 if (preceding != NULL)
	    preceding->next = new_stack_element;
	 else
	    stack_head = new_stack_element;
	 break;
	 /* can free old stack_element & region here */
      }
      preceding = current_stack;
      current_stack = current_stack->next;
   }
   /* remove second region */
   current_stack = stack_head;
   preceding = NULL;
   while (current_stack != NULL)
   {				/* look for region 2 */
      vertex = old2_region_vertex;
      do
      {
	 if (current_stack->region == vertex)
	 {
	    /* found stack element - delete it */
	    if (preceding != NULL)
	       preceding->next = current_stack->next;
	    else
	       stack_head = current_stack->next;
	    /* can free old 2 stack_element & region here */
	    return (stack_head);/* normal return */
	 }
	 vertex = vertex->next;
      } while (vertex != old2_region_vertex);
      preceding = current_stack;
      current_stack = current_stack->next;
   }
   /* error has occurred */
   sprintf(msg,"\n error stack item not found");
   puts(msg);
   return (NULL);
}

/*-----------------------------------------------------------------------------

				CHECK_MERGE

		Accepts a convex region and a list of convex regions and
		merges any that can be merged
		Returns pointer to resulting stack of convex regions
		Sets flag merged to TRUE if a merge occurred
			DESTRUCTIVE TO OLD STACK

------------------------------------------------------------------------------*/

struct stack_element *check_merge(
    struct stack_element *start,
    struct stack_element *stack_header,
    int *merged)
{
    // ENDO - gcc-3.4
    //struct vertex_list_element *hold, *temp, *current, *merge_regions(),
    struct vertex_list_element *hold, *temp, *current,
                  *new_region;
    // ENDO - gcc-3.4
    //struct stack_element *repair_stack(), *stack_top;
    struct stack_element *stack_top;
    //int             slop_convex();

   current = start->region->next;	/* get first region */
   stack_top = stack_header;
   *merged = FALSE;		/* init flag */
   do
   {				/* create new region */
      if (current->passable != NULL)
      {
	 new_region = merge_regions(current);

	 if (slop_convex(new_region->next))
	 {			/* if new region convex */
	    *merged = TRUE;	/* set flag */

	    stack_top = repair_stack(new_region, start, current->passable, stack_header);	/* fix stack */

	    patch_region(new_region);	/* update pointers */
	    break;
	 }
	 else
	 {			/* free-up vertices from unmerged regions */
	    temp = new_region;
	    do
	    {
	       hold = temp->next;
	       free_vertex(temp);
	       temp = hold;
	    } while (new_region != temp);
	 }
      }
      else
      {
	 break;
      }
      current = current->next;	/* get next region */
   } while (current != start->region->next);

   return (stack_top);
}

/*--------------------------------------------------------------------------

				PATCH_REGION

		Routine to backpatch all pointers to a new region
		Uses the region itself (passable component) to find
		pointers referring to old region

---------------------------------------------------------------------------*/

void
patch_region(struct vertex_list_element * head)
{
   struct vertex_list_element *current;

   current = head;		/* get first vertex */
   if (current->passable != NULL)	/* only involves passable vertices */
      current->passable->passable = current;
   current = current->next;	/* get remaining vertices */
   while (current != head)
   {				/* only involves passable vertices */
      if (current->passable != NULL)
	 current->passable->passable = current;
      current = current->next;
   }
   return;
}

/*----------------------------------------------------------------------------

				FIND_MEADOW2

		Accepts a point (in vertex format) and
		returns second region if point is in two
		Returns pointer to the second region the point is in
			or NULL if not in two.

-----------------------------------------------------------------------------*/

struct vertex_list_element *find_meadow2(
    struct vertex_list_element *pt,	/* point to be checked */
    struct stack_element *meadow_stack)	/* stack of convex regions */
{
   struct stack_element *meadow_ptr;
   struct vertex_list_element *meadow;

   meadow = meadow_stack->region->next;
   meadow_ptr = meadow_stack;
   while (!inside_meadow(pt, meadow))
   {
      meadow_ptr = meadow_ptr->next;
      if (meadow_ptr == NULL)
      {
	 return (NULL);
      }
      meadow = meadow_ptr->region->next;
   }
   meadow_ptr = meadow_ptr->next;
   if (meadow_ptr == NULL)
      return (NULL);
   meadow = meadow_ptr->region->next;
   while (!inside_meadow(pt, meadow))
   {
      meadow_ptr = meadow_ptr->next;
      if (meadow_ptr == NULL)
	 return (NULL);
      meadow = meadow_ptr->region->next;
   }
   return (meadow_ptr->region->next);
}

/*---------------------------------------------------------------------------

				NOT_ON_CLOSED

	Boolean function - Returns TRUE if node is not on closed list

----------------------------------------------------------------------------*/

int not_on_closed(
    struct vertex_list_element *vertex,
    struct closed_node *closed)
{
   struct closed_node *current;

   current = closed;

   while (current != NULL)
   {				/* check closed list */
      if ((current->x1 == vertex->x && current->y1 == vertex->y &&
	   current->x2 == vertex->next->x && current->y2 == vertex->next->y)
	  || (current->x2 == vertex->x && current->y2 == vertex->y &&
	  current->x1 == vertex->next->x && current->y1 == vertex->next->y))
      {
	 return (FALSE);	/* it is on closed */
      }
      current = current->next_cnode;
   }
   return (TRUE);
}

/*-----------------------------------------------------------------------------

				FIND_ADIT

	Find bisector of free space passage and return x and y value in node

------------------------------------------------------------------------------*/

void
find_adit(struct open_node * node,
	  struct vertex_list_element * vertex)
{

   /* find adit by splitting difference of x and y */
  node->x = (vertex->x + vertex->next->x)*0.5;
  node->y = (vertex->y + vertex->next->y)*0.5;

}

/*--------------------------------------------------------------------------

			CORRECT_X and CORRECT_Y

		Handles corner adjustments for tautness

---------------------------------------------------------------------------*/

double correct_x(
    struct vertex_list_element *pt1,
    struct vertex_list_element *pt2,
    double safety)
{
   double          dx, length, theta;
   double          val;

   dx = pt2->x - pt1->x;
   length = find_length(pt1, pt2);
   val = dx / length;

   if (val > 1.0)
      val = 1.0;
   else if (val < -1.0)
      val = -1.0;

   theta = acos(val);


   return (safety * cos(theta));
}

/*-------------------------------------------------------------------------*/

double correct_y(
    struct vertex_list_element *pt1,
    struct vertex_list_element *pt2,
    double safety)
{
   double          dy, length, theta;

   dy = pt2->y - pt1->y;
   length = find_length(pt1, pt2);
   theta = asin(dy / length);
   return (safety * sin(theta));
}

/*------------------------------------------------------------------------

			REVERSE_PATH

		Return a reversed copy of the path
-------------------------------------------------------------------------*/
struct path_node *reverse_path(struct path_node *path_in)
{
   struct path_node *hold_path, *current, *path_out;
   struct terrain_identifier *hold_terrain;

   current = path_in;

   /* last first */
   path_out = (struct path_node *) malloc(sizeof(struct path_node));
   hold_path = path_out;

   path_out->next_point = NULL;
   path_out->x = current->x;
   path_out->y = current->y;
   if (current->region2 != NULL)
   {
      path_out->region1 = current->region2;
      path_out->region2 = current->region1;
   }
   else
   {
      path_out->region1 = current->region1;
      path_out->region2 = current->region2;
   }
   path_out->terrain_type = current->terrain_type;
   hold_terrain = path_out->terrain_type;
   current = current->next_point;

   while (current != NULL)
   {
      path_out = (struct path_node *) malloc(sizeof(struct path_node));
      path_out->next_point = hold_path;
      hold_path = path_out;
      path_out->x = current->x;
      path_out->y = current->y;
      if (current->region2 != NULL)
      {
	 path_out->region1 = current->region2;
	 path_out->region2 = current->region1;
      }
      else
      {
	 path_out->region1 = current->region1;
	 path_out->region2 = current->region2;
      }
      path_out->terrain_type = hold_terrain;
      hold_terrain = current->terrain_type;
      current = current->next_point;
   }
   return (path_out);
}

/*----------------------------------------------------------------------------

				H_COMPUTE

     Heuristic computation - as crow flies (straight line distance to goal)

-----------------------------------------------------------------------------*/

double h_compute(struct open_node *node, struct open_node *goal)
{
   double          dx, dy;

   dx = node->x - goal->x;
   dy = node->y - goal->y;
   /*   dx = max(node->x, goal->x) - min(node->x, goal->x);
	dy = max(node->y, goal->y) - min(node->y, goal->y); */
   return (sqrt((dx * dx) + (dy * dy)));
}

double
h_compute2(struct open_node *node, struct open_node *goal, struct open_node *openlist)
{
   double          dx, dy;

   dx = node->x - goal->x;
   dy = node->y - goal->y;

   /*   dx = max(node->x, goal->x) - min(node->x, goal->x);
	dy = max(node->y, goal->y) - min(node->y, goal->y);*/

   return (sqrt((dx * dx) + (dy * dy)) * (openlist->terrain->traversal_factor));
}


/*-----------------------------------------------------------------------------

				TERRAIN_FIND_SPECIAL

		Return terrain identifier for a  vertex

----------------------------------------------------------------------------*/

struct terrain_identifier *
terrain_find_special(struct vertex_list_element *node, struct stack_element *map)
{
   struct stack_element       *current;
   struct vertex_list_element *vertex;

   if (node == NULL) return (NULL);

   current = map;
   while (TRUE)
   {
      vertex = current->region->next;
      do
      {
	if (vertex == node){
	  //	  printf("terrain_find_special: %d \n",(int)(current->terrain_type));
	    return (current->terrain_type);
	}
	 vertex = vertex->next;
      } while (vertex != current->region->next);
      current = current->next;
   }				/* should always be found */
}

/*-----------------------------------------------------------------------------

				BUILD_PATH

	Build completed path in a form that is acceptable to pilot
			Reverses previous order

------------------------------------------------------------------------------*/

struct path_node *build_path(
    struct open_node *openlist,
    struct vertex_list_element *start,
    struct stack_element *map)
{
   struct open_node *current, *temp_onode;
   struct path_node *end, *pnode = NULL, *previous;

   current = openlist;
   previous = NULL;
   while (current != NULL)
   {
      pnode = (struct path_node *) malloc(sizeof(struct path_node));
      pnode->next_point = previous;
      pnode->x = current->x;
      pnode->y = current->y;
      pnode->region1 = current->region;
      pnode->region2 = current->region->passable;
      pnode->terrain_type = current->terrain;
      previous = pnode;
      current = current->parent;

   }
   /* free up open list */
   current = openlist;
   while (current != NULL)
   {
      temp_onode = current->next_onode;
      free(current);
      current = temp_onode;
   }

   /* correct for start */
   pnode->region1 = find_meadow(start, map);
   pnode->region2 = NULL;

   /* correct for end */
   end = pnode;
   while (end->next_point != NULL) end = end->next_point;
   end->region2 = NULL;

   return (pnode);
}

/*-------------------------------------------------------------------------

			A_STAR - A* search of free space

-------------------------------------------------------------------------*/

struct path_node *a_star(struct stack_element *map, 
			  struct vertex_list_element *start,
			  struct vertex_list_element *goal)
{
  struct closed_node *closed, *cnode, *temp_cnode;
  struct open_node *open;
  struct vertex_list_element *current, *region, temp_open;
  struct path_node *path, *start_path;
  //struct terrain_identifier *terrain_find_special();
  //double          h_compute();
  //int             same_region();
  char            msg[80];

  closed = NULL;
  /* put start on open */
  open = (struct open_node *) malloc(sizeof(struct open_node));
  open->x = start->x;
  open->y = start->y;
  open->g = 0;
  open->terrain = find_start_goal(start, map);
  if(open->terrain == NULL)
    {
      printf("cart_lib:a_star - start location %5.2f, %5.2f not within map\n",
	      start->x, start->y);
      abort();
    }

  open->f = h_compute(open, (struct open_node *)goal);	/* OK since g is initially 0 */
  region = find_meadow(start, map);
  open->region = region;
  open->next_onode = NULL;
  open->parent = NULL;
  if (region == find_meadow(goal, map))
    {				/* goal in same region as start */
      path = (struct path_node *) malloc(sizeof(struct path_node));
      start_path = path;
      path->x = start->x;
      path->y = start->y;
      path->region1 = region;
      path->terrain_type = terrain_find_special(region, map);
      path->region2 = NULL;
      path->next_point = (struct path_node *) malloc(sizeof(struct path_node));
      path = path->next_point;
      path->x = goal->x;
      path->y = goal->y;
      path->region1 = region;
      path->terrain_type = terrain_find_special(region, map);
      path->region2 = NULL;
      path->next_point = NULL;
      return (start_path);
    }
  current = region->next;

  do
    {				/* add all passable line segments to open */
      if (current->passable != NULL)
	add_to_open(current->passable, open, goal, map);
      current = current->next;
    } while (current != region->next);

  open = open->next_onode;	/* delete start from openlist */

  /* now all passable points are on open */

  while (TRUE)
    {				/* search */
      /* Put first open node on closed */
      cnode = (struct closed_node *) malloc(sizeof(struct closed_node));
      cnode->x1 = open->region->x;
      cnode->y1 = open->region->y;
      cnode->x2 = open->region->next->x;
      cnode->y2 = open->region->next->y;
      cnode->next_cnode = closed;
      closed = cnode;
      /* This is not possible -- pointers above would have seg-faulted. */
      if (open == NULL)
	{				/* search fails */
	  sprintf(msg,"\n No path possible ");
	  puts(msg);
	  return (NULL);
	}
      temp_open.x = open->x;
      temp_open.y = open->y;
      if ((open->x == goal->x) && (open->y == goal->y))
	{
	  /* path found */
	  /* free closed nodes */
	  cnode = closed;
	  while (cnode != NULL)
	    {
	      temp_cnode = cnode->next_cnode;
	      free(cnode);
	      cnode = temp_cnode;
	    }
	  /* open nodes freed in build_path */
	  return (build_path(open, start, map));
	}
      if (same_region(find_meadow(goal, map), open->region) ||
	  same_region(find_meadow(goal, map), open->region->passable))
	{

	  add_goal_to_open(open, goal, map);
	  open = open->next_onode;	/* delete node from openlist */
	}
      else
	{				/* must be in different meadow */
	  current = open->region->next;
	  do
	    {
	      if ((current->passable != NULL) &&
		  not_on_closed(current, closed))
		add_to_open(current->passable, open, goal, map);

	      current = current->next;
	    } while (current != open->region->next);

	  open = open->next_onode;	/* delete node from openlist */
	}
    } /* while TRUE */
}

/*----------------------------------------------------------------------------

				G_COMPUTE

		Compute non-heuristic portion of evaluation function
			Based on distance traveled so far

-----------------------------------------------------------------------------*/

double
g_compute(struct open_node *node, struct open_node *openlist)
{
   double          dx, dy, old_g;

   /* openlist first node contains old value */
   /* update it */
   old_g = openlist->g;
   dx = node->x - openlist->x;
   dy = node->y - openlist->y;
   /*   dx = max(node->x, openlist->x) - min(node->x, openlist->x);
	dy = max(node->y, openlist->y) - min(node->y, openlist->y);*/
   return (sqrt((dx * dx) + (dy * dy)) + (openlist->g));
}
double

g_compute2(struct open_node *node, struct open_node *openlist)
{
   double          dx, dy, old_g;

   /* openlist first node contains old value */
   /* update it */
   old_g = openlist->g;
   dx = node->x - openlist->x;
   dy = node->y - openlist->y;

   /*   dx = max(node->x, openlist->x) - min(node->x, openlist->x);
	dy = max(node->y, openlist->y) - min(node->y, openlist->y);*/
   return ((sqrt((dx * dx) + (dy * dy)) * openlist->terrain->traversal_factor)
	   + openlist->g);
}

void myPrintPath(struct path_node *myPath)
{
  struct path_node *path;

  path = myPath;

  printf("\nMyPath:\n===============\n");
  while(path)
    {
      printf("%g %g\n", path->x, path->y);
      path = path->next_point;
    }
}


/*-----------------------------------------------------------------------------

				IMPROVE_PATH

	Refinement of coarse path into straighter and/or tauter path
		Tautness value is passable in safety_margin

-----------------------------------------------------------------------------*/
void
improve_path(struct path_node * path,
	     struct stack_element * convex_region_list,
	     double safety_margin)
{
   double          length2, start_length, length1, startx,
                   starty, startx2, starty2, cx, cy;
   struct path_node *pnext, *hold_path, *previous, *previous2;
   struct open_node node;
   struct path_node *start_point, *goal_path_point, *real_beginning;
   struct terrain_identifier *terrain_type;
   int             ii;

   if(0)   myPrintPath(path);

   /* TAUTNESS PART */
   /* safety_margin less than zero bypasses */
   real_beginning = path;
   while (TRUE)
   {
      start_point = path;
      do
      {				/* get a terrain start and end */
	 terrain_type = path->terrain_type;
	 if (terrain_type == start_point->terrain_type)
	    path = path->next_point;
	 else
	 {
	    goal_path_point = path;	/* goal is last point with same
					 * terrain type */
	    break;
	 }
	 if (path->next_point == NULL)
	 {
	    /* done! do something special */
	    goal_path_point = NULL;
	    break;
	 }
      } while (TRUE);
      /* start and end segment now defined for specific terrain type */
      /* do the tighten up */

      /* FOR NOW, there is only one terrain type, so the next part will only happen once. */

      path = start_point;

      hold_path = path;
      while (path->next_point != goal_path_point)
	{ /* loop for path until goal */
	  if (safety_margin >= 0)
	    {			/* moves towards vertices */
	      pnext = path->next_point;
	      while (TRUE)
		{  /* loop for pnext */
		  if (pnext == goal_path_point)
		    break;

		  /* This is designed for A*, not A*-3.  */
		  find_adit(&node, pnext->region1);
		  printf("node: %5.2f %5.2f\n", node.x, node.y);
		  if ((node.x == pnext->x) && (node.y == pnext->y)) /* Doesn't happen in some of the bad cases. */
		    {
		      if (find_length(pnext->region1, pnext->region1->next) > (safety_margin * 2))
			{
			  cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
			  cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);

			  startx = pnext->x;
			  starty = pnext->y;
			  start_length = path_length(path);

			  pnext->x = pnext->region1->x + cx;
			  pnext->y = pnext->region1->y + cy;
			  length1 = path_length(path);
			  pnext->x = pnext->region1->next->x - cx;
			  pnext->y = pnext->region1->next->y - cy;
			  if ((length1 < path_length(path)) &&
			      (length1 < start_length))
			    {
			      pnext->x = pnext->region1->x + cx;
			      pnext->y = pnext->region1->y + cy;
			    }
			  else
			    {
			      if ((start_length < length1) &&
				  (start_length < path_length(path)))
				{
				  pnext->x = startx;
				  pnext->y = starty;
				}
			    }
			} /* if find_length > 2*safety     */
		    } /* if node xy == pnext xy  */
		  pnext = pnext->next_point;
		} /* While TRUE  -- loop for pnext */
	    } /* if safety_margin >= 0 */
	  path = path->next_point;
	} /* loop for path until goal_point */

      path = hold_path;



      /* STRAIGHTNESS PART */

      if(0)   myPrintPath(path);


      /* removes unnecessary course changes - straightens lines */
      /* remove redundant points */

      pnext = path;
      while (pnext->next_point != goal_path_point)
      {
	 if ((pnext->next_point->x == pnext->x) && (pnext->next_point->y == pnext->y))
	    pnext->next_point = pnext->next_point->next_point;
	 pnext = pnext->next_point;
      }



      if(0)   myPrintPath(path);


      /* start straightness */
      /*
       * if (straighten_path)
       * straighten_the_path(path,goal_path_point,convex_region_list);
       */
      /* end straightness */
      /* final slipping towards corners */

      hold_path = path;
      while (path->next_point != goal_path_point)
      {
	 if (path->next_point->next_point == goal_path_point)
	    break;
	 if (safety_margin >= 0)
	 {			/* moves towards vertices */
	    previous = path;
	    pnext = path->next_point;
	    while (TRUE)
	    {
	       if (pnext->next_point == goal_path_point)
		  break;
	       find_adit(&node, pnext->region1);
	       if ((node.x == pnext->x) && (node.y == pnext->y))
	       {
		  if (find_length(pnext->region1, pnext->region1->next) > (safety_margin * 2))
		  {
		     cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
		     cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);

		     startx = pnext->x;
		     starty = pnext->y;
		     start_length = path_length(path);

		     pnext->x = pnext->region1->x + cx;
		     pnext->y = pnext->region1->y + cy;

		     length1 = path_length(path);
		     if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
		     plus2_line_segment(previous, pnext, convex_region_list))
			length1 = PATH_PLAN_INFINITY;
		     pnext->x = pnext->region1->next->x - cx;
		     pnext->y = pnext->region1->next->y - cy;
		     length2 = path_length(path);
		     if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
		     plus2_line_segment(previous, pnext, convex_region_list))
			length2 = PATH_PLAN_INFINITY;
		     if ((length1 < length2) && (length1 < start_length))
		     {
			pnext->x = pnext->region1->x + cx;
			pnext->y = pnext->region1->y + cy;
		     }
		     else
		     {
			if ((start_length < length1) && (start_length < length2))
			{
			   pnext->x = startx;
			   pnext->y = starty;
			}
		     }
		  }
	       }
	       previous = pnext;
	       pnext = pnext->next_point;
	    }
	 }
	 path = path->next_point;
      }
      path = hold_path;



      /* remove redundant points  again */

      pnext = path;
      while (pnext->next_point != goal_path_point)
      {
	 if ((pnext->next_point->x == pnext->x) && (pnext->next_point->y == pnext->y))
	    pnext->next_point = pnext->next_point->next_point;
	 pnext = pnext->next_point;
      }

      /* special for multi-terrain */
      if (goal_path_point != NULL)
      {
	 if (goal_path_point->next_point != NULL)
	 {			/* must be transition zone */
	    if (goal_path_point->next_point->next_point != NULL)	/* end is not in
									 * transition zone */
	       path = goal_path_point->next_point;	/* skip straightening in
							 * t zone */
	    else
	       break;
	 }
	 else
	    break;
      }
      else
	 break;
   }
   /* slide vertex on transition zones */
   /* find first transition zone - if any */
   /* use  special cost function including terrain */
   if (safety_margin >= 0)
   {
      for (ii = 1; ii <= 2; ii++)
      {
	 path = real_beginning;
	 previous2 = previous = NULL;
	 pnext = path;
	 while (pnext != NULL)
	 {
	    if (pnext->terrain_type->transition_zone)
	    {			/* if transition zone */
	       find_adit(&node, pnext->region1);
	       if ((node.x == pnext->x) && (node.y == pnext->y))
	       {
		  if (pnext == path)
		  {
		     if (!plus2_line_segment(pnext, pnext->next_point->next_point,
					     convex_region_list))
		     {
			/* cut out point and skip */
			pnext->next_point = pnext->next_point->next_point;
			goto cutout;
		     }
		     /* start point is in transition zone */
		     if (pnext->next_point->next_point == NULL)
			break;	/* all done */
		     /* otherwise slide the single point only */
		     if (find_length(pnext->next_point->region1,
		     pnext->next_point->region1->next) > (safety_margin * 2))
		     {
			cx = correct_x(pnext->next_point->region1,
			   pnext->next_point->region1->next, safety_margin);
			cy = correct_y(pnext->next_point->region1,
			   pnext->next_point->region1->next, safety_margin);
			startx = pnext->next_point->x;
			starty = pnext->next_point->y;
			start_length = path_cost(path);

			pnext->next_point->x = pnext->next_point->region1->x + cx;
			pnext->next_point->y = pnext->next_point->region1->y + cy;

			length1 = path_cost(path);
			if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			    plus2_line_segment(pnext, pnext->next_point, convex_region_list))
			   length1 = PATH_PLAN_INFINITY;
			pnext->next_point->x = pnext->next_point->region1->next->x - cx;
			pnext->next_point->y = pnext->next_point->region1->next->y - cy;
			length2 = path_cost(path);
			if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			    plus2_line_segment(pnext, pnext->next_point, convex_region_list))
			   length2 = PATH_PLAN_INFINITY;
			if ((length1 < length2) && (length1 < start_length))
			{
			   pnext->next_point->x = pnext->next_point->region1->x + cx;
			   pnext->next_point->y = pnext->next_point->region1->y + cy;
			}
			else
			{
			   if ((start_length < length1) && (start_length < length2))
			   {
			      pnext->next_point->x = startx;
			      pnext->next_point->y = starty;
			   }
			}
		     }
	       cutout:previous2 = previous;
		     previous = pnext;
		     pnext = pnext->next_point;
		  }
		  /* end region if starts in transition zone */

		  else
		  {
		     if (pnext->next_point->next_point == NULL)
		     {
			/* if end point is in transition zone */
			if (!plus2_line_segment(previous, pnext->next_point, convex_region_list))
			{
			   /* cut out point and break */
			   previous->next_point = pnext->next_point;
			   break;
			}
			if (find_length(pnext->region1, pnext->region1->next) > (safety_margin * 2))
			{
			   cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
			   cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);
			   startx = pnext->x;
			   starty = pnext->y;
			   start_length = path_cost(path);
			   pnext->x = pnext->region1->x + cx;
			   pnext->y = pnext->region1->y + cy;
			   length1 = path_cost(path);
			   if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length1 = PATH_PLAN_INFINITY;
			   pnext->x = pnext->region1->next->x - cx;
			   pnext->y = pnext->region1->next->y - cy;
			   length2 = path_cost(path);
			   if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length2 = PATH_PLAN_INFINITY;
			   if ((length1 < length2) && (length1 < start_length))
			   {
			      pnext->x = pnext->region1->x + cx;
			      pnext->y = pnext->region1->y + cy;
			   }
			   else
			   {
			      if ((start_length < length1) && (start_length < length2))
			      {
				 pnext->x = startx;
				 pnext->y = starty;
			      }
			   }
			}
			break;
		     }

		     else
		     {
			/* slide both points */
			if (find_length(pnext->region1,
				pnext->region1->next) > (safety_margin * 2))
			{
			   cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
			   cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);

			   startx = pnext->x;
			   starty = pnext->y;
			   startx2 = pnext->next_point->x;
			   starty2 = pnext->next_point->y;
			   start_length = path_cost(path);
			   pnext->x = pnext->region1->x + cx;
			   pnext->y = pnext->region1->y + cy;
			   pnext->next_point->x = pnext->next_point->region1->x + cx;
			   pnext->next_point->y = pnext->next_point->region1->y + cy;
			   length1 = path_cost(path);
			   if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length1 = PATH_PLAN_INFINITY;
			   pnext->x = pnext->region1->next->x - cx;
			   pnext->y = pnext->region1->next->y - cy;
			   pnext->next_point->x = pnext->next_point->region1->next->x - cx;
			   pnext->next_point->y = pnext->next_point->region1->next->y - cy;
			   length2 = path_cost(path);
			   if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length2 = PATH_PLAN_INFINITY;
			   if ((length1 < length2) && (length1 < start_length))
			   {
			      pnext->x = pnext->region1->x + cx;
			      pnext->y = pnext->region1->y + cy;
			      pnext->next_point->x = pnext->next_point->region1->x + cx;
			      pnext->next_point->y = pnext->next_point->region1->y + cy;
			   }
			   else
			   {
			      if ((start_length < length1) && (start_length < length2))
			      {
				 pnext->x = startx;
				 pnext->y = starty;
				 pnext->next_point->x = startx2;
				 pnext->next_point->y = starty2;
			      }
			   }
			}
		     }
		     previous2 = pnext;
		     previous = pnext->next_point;
		     pnext = pnext->next_point->next_point;
		  }
	       }
	       else
	       {
		  previous2 = previous;
		  previous = pnext;
		  pnext = pnext->next_point;
	       }
	    }			/* if find_adit */
	    /* new */
	    else
	    {
	       previous2 = previous;
	       previous = pnext;
	       pnext = pnext->next_point;
	    }
	    /* end new */
	 }			/* end for */
      }				/* end do */
   }

   if (straighten_path)
   {
      path = real_beginning;
      /* special case for start in transition zone */
      if (path->terrain_type->transition_zone)
	 path = path->next_point;
      /* end special */

      while (TRUE)
      {
	 start_point = path;
	 do
	 {			/* get a terrain start and end */
	    terrain_type = path->terrain_type;
	    if (terrain_type == start_point->terrain_type)
	       path = path->next_point;
	    else
	    {
	       goal_path_point = path;	/* goal is last point with same
					 * terrain type */
	       break;
	    }
	    if (path->next_point == NULL)
	    {
	       /* done! do something special */
	       goal_path_point = NULL;
	       break;
	    }
	 } while (TRUE);
	 path = start_point;
	 straighten_the_path(path, goal_path_point, convex_region_list);
	 if (goal_path_point != NULL)
	 {
	    if (goal_path_point->next_point != NULL)
	    {			/* must be transition zone */
	       if (goal_path_point->next_point->next_point != NULL)	/* end is not in
									 * transition zone */
		  path = goal_path_point->next_point;	/* skip straightening in
							 * t zone */
	       else
		  break;
	    }
	    else
	       break;
	 }
	 else
	    break;
      }
   }
   if (safety_margin >= 0)
   {
      for (ii = 1; ii <= 2; ii++)
      {
	 path = real_beginning;
	 previous2 = previous = NULL;
	 pnext = path;
	 while (pnext != NULL)
	 {
	    if (pnext->terrain_type->transition_zone)
	    {			/* if transition zone */
	       if (pnext != path)
	       {
		  find_adit(&node, pnext->region1);
		  pnext->x = node.x;
		  pnext->y = node.y;
	       }
	       if (pnext->next_point->next_point != NULL)
	       {
		  find_adit(&node, pnext->next_point->region1);
		  pnext->next_point->x = node.x;
		  pnext->next_point->y = node.y;
		  /* set back next region side */
	       }
	       if (TRUE)
	       {
		  if (pnext == path)
		  {
		     if (!plus2_line_segment(pnext, pnext->next_point->next_point,
					     convex_region_list))
		     {
			/* cut out point and skip */
			pnext->next_point = pnext->next_point->next_point;
			goto cutout2;
		     }
		     /* start point is in transition zone */
		     if (pnext->next_point->next_point == NULL)
			break;	/* all done */
		     /* otherwise slide the single point only */
		     if (find_length(pnext->next_point->region1,
		     pnext->next_point->region1->next) > (safety_margin * 2))
		     {
			cx = correct_x(pnext->next_point->region1,
			   pnext->next_point->region1->next, safety_margin);
			cy = correct_y(pnext->next_point->region1,
			   pnext->next_point->region1->next, safety_margin);
			startx = pnext->next_point->x;
			starty = pnext->next_point->y;
			start_length = path_cost(path);

			pnext->next_point->x = pnext->next_point->region1->x + cx;
			pnext->next_point->y = pnext->next_point->region1->y + cy;

			length1 = path_cost(path);
			if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			    plus2_line_segment(pnext, pnext->next_point, convex_region_list))
			   length1 = PATH_PLAN_INFINITY;
			pnext->next_point->x = pnext->next_point->region1->next->x - cx;
			pnext->next_point->y = pnext->next_point->region1->next->y - cy;
			length2 = path_cost(path);
			if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			    plus2_line_segment(pnext, pnext->next_point, convex_region_list))
			   length2 = PATH_PLAN_INFINITY;
			if ((length1 < length2) && (length1 < start_length))
			{
			   pnext->next_point->x = pnext->next_point->region1->x + cx;
			   pnext->next_point->y = pnext->next_point->region1->y + cy;
			}
			else
			{
			   if ((start_length < length1) && (start_length < length2))
			   {
			      pnext->next_point->x = startx;
			      pnext->next_point->y = starty;
			   }
			}
		     }
	       cutout2:previous2 = previous;
		     previous = pnext;
		     pnext = pnext->next_point;
		  }
		  /* end region if starts in transition zone */

		  else
		  {
		     if (pnext->next_point->next_point == NULL)
		     {
			/* if end point is in transition zone */
			if (!plus2_line_segment(previous, pnext->next_point, convex_region_list))
			{
			   /* cut out point and break */
			   previous->next_point = pnext->next_point;
			   break;
			}
			if (find_length(pnext->region1, pnext->region1->next) > (safety_margin * 2))
			{
			   cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
			   cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);
			   startx = pnext->x;
			   starty = pnext->y;
			   start_length = path_cost(path);
			   pnext->x = pnext->region1->x + cx;
			   pnext->y = pnext->region1->y + cy;
			   length1 = path_cost(path);
			   if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length1 = PATH_PLAN_INFINITY;
			   pnext->x = pnext->region1->next->x - cx;
			   pnext->y = pnext->region1->next->y - cy;
			   length2 = path_cost(path);
			   if (plus2_line_segment(pnext, pnext->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length2 = PATH_PLAN_INFINITY;
			   if ((length1 < length2) && (length1 < start_length))
			   {
			      pnext->x = pnext->region1->x + cx;
			      pnext->y = pnext->region1->y + cy;
			   }
			   else
			   {
			      if ((start_length < length1) && (start_length < length2))
			      {
				 pnext->x = startx;
				 pnext->y = starty;
			      }
			   }
			}
			break;
		     }

		     else
		     {
			/* slide both points */
			if (find_length(pnext->region1,
				pnext->region1->next) > (safety_margin * 2))
			{
			   cx = correct_x(pnext->region1, pnext->region1->next, safety_margin);
			   cy = correct_y(pnext->region1, pnext->region1->next, safety_margin);

			   startx = pnext->x;
			   starty = pnext->y;
			   startx2 = pnext->next_point->x;
			   starty2 = pnext->next_point->y;
			   start_length = path_cost(path);
			   pnext->x = pnext->region1->x + cx;
			   pnext->y = pnext->region1->y + cy;
			   pnext->next_point->x = pnext->next_point->region1->x + cx;
			   pnext->next_point->y = pnext->next_point->region1->y + cy;
			   length1 = path_cost(path);
			   if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length1 = PATH_PLAN_INFINITY;
			   pnext->x = pnext->region1->next->x - cx;
			   pnext->y = pnext->region1->next->y - cy;
			   pnext->next_point->x = pnext->next_point->region1->next->x - cx;
			   pnext->next_point->y = pnext->next_point->region1->next->y - cy;
			   length2 = path_cost(path);
			   if (plus2_line_segment(pnext->next_point, pnext->next_point->next_point, convex_region_list) ||
			       plus2_line_segment(previous, pnext, convex_region_list))
			      length2 = PATH_PLAN_INFINITY;
			   if ((length1 < length2) && (length1 < start_length))
			   {
			      pnext->x = pnext->region1->x + cx;
			      pnext->y = pnext->region1->y + cy;
			      pnext->next_point->x = pnext->next_point->region1->x + cx;
			      pnext->next_point->y = pnext->next_point->region1->y + cy;
			   }
			   else
			   {
			      if ((start_length < length1) && (start_length < length2))
			      {
				 pnext->x = startx;
				 pnext->y = starty;
				 pnext->next_point->x = startx2;
				 pnext->next_point->y = starty2;
			      }
			   }
			}
		     }
		     previous2 = pnext;
		     previous = pnext->next_point;
		     pnext = pnext->next_point->next_point;
		  }
	       }
	       else
	       {
		  previous2 = previous;
		  previous = pnext;
		  pnext = pnext->next_point;
	       }
	    }			/* if find_adit */
	    /* new */
	    else
	    {
	       previous2 = previous;
	       previous = pnext;
	       pnext = pnext->next_point;
	    }
	    /* end new */
	 }			/* end for */
      }				/* end do */
   }

   return;
}

/*-----------------------------------------------------------------------------

				TERRAIN_FIND

		Return terrain identifier for a path segment

----------------------------------------------------------------------------*/
struct terrain_identifier *terrain_find(
    struct open_node *node,
    struct stack_element *map)
{
  //   extern struct terrain_identifier *THE_TERRAIN;
   struct stack_element *current;
   struct vertex_list_element *vertex;

   //   return THE_TERRAIN;

   current = map;

   while (TRUE)
   {
      vertex = current->region->next;
      do
      {
	 if (vertex == node->region)
	    return (current->terrain_type);
	 vertex = vertex->next;
      } while (vertex != current->region->next);
      current = current->next;
   }				/* should always be found */
}

/*-----------------------------------------------------------------------------

				ADD_TO_OPEN

	Adds a vertex to the open list for later expansion in A-star

------------------------------------------------------------------------------*/

void
add_to_open(struct vertex_list_element *vertex,
	    struct open_node           *open_list,
	    struct vertex_list_element *goal,
	    struct stack_element       *map)
{
   struct open_node *node, *previous, *current;
   //struct terrain_identifier *terrain_find();
   //double          g_compute2(), h_compute();
   double h;

   node = (struct open_node *) malloc(sizeof(struct open_node));
   find_adit(node, vertex);	/* set x and y of adit to midpoint of line */
   node->region = vertex;

   node->terrain = terrain_find(node, map);
   node->g = g_compute2(node, open_list);	/* sets g value, first node
						 * on open is expanding */
   h = h_compute(node, (struct open_node *)goal);                   /* heuristic evaluation */
   node->f = node->g + h;
   current = open_list->next_onode;
   previous = open_list;
   node->parent = previous;

   while (current != NULL)
   {
      if (node->f < current->f)
      {
	 node->next_onode = previous->next_onode;
	 previous->next_onode = node;
	 break;
      }
      previous = current;
      current = current->next_onode;
   }
   if (current == NULL)
   {				/* Insert at end */
      node->next_onode = NULL;
      previous->next_onode = node;
   }
}

/*-----------------------------------------------------------------------------

				ADD_GOAL_TO_OPEN

		Specialized to add goal node to open_list

-----------------------------------------------------------------------------*/

void add_goal_to_open(
    struct open_node * open_list,
    struct vertex_list_element * goal,
    struct stack_element * map)
{
   struct open_node *node, *previous, *current;
   // ENDO - gcc-3.4
   //double          g_compute2(), h_compute(), h;
   double h;
   char msg[80];

   node = (struct open_node *) malloc(sizeof(struct open_node));
   node->x = goal->x;
   node->y = goal->y;
   node->region = find_meadow(goal, map);
   node->terrain = find_start_goal(goal, map);
   if( node->terrain == NULL )
   {
      sprintf(msg,"goal location not within map\n");
      puts(msg);
      return;
   }
   node->g = g_compute2(node, open_list);	/* sets g value, first node
						 * on open is expanding */
   h = h_compute2(node, (struct open_node *)goal, open_list);
   node->f = node->g + h;
   current = open_list->next_onode;
   node->parent = previous = open_list;
   while (current != NULL)
   {
      if (node->f < current->f)
      {
	 node->next_onode = previous->next_onode;
	 previous->next_onode = node;
	 break;
      }
      previous = current;
      current = current->next_onode;
   }
   if (current == NULL)
   {				/* Insert at end */
      node->next_onode = NULL;
      previous->next_onode = node;
   }
}

/*--------------------------------------------------------------------------

		Plus2_line_segment

--------------------------------------------------------------------------*/
int
plus2_line_segment(struct path_node * p1,
		   struct path_node * p2,
		   struct stack_element * region_list)
{
   struct vertex_list_element v1, v2, *current;
   struct stack_element *now_region;
   struct terrain_identifier *terra;

   v1.x = p1->x;
   v1.y = p1->y;
   v2.x = p2->x;
   v2.y = p2->y;
   terra = p1->terrain_type;
   now_region = region_list;
   while (now_region != NULL)
   {
      current = now_region->region->next;
      do
      {
	 if ((current->passable == NULL) ||
	     now_region->terrain_type->transition_zone)
	 {
	    /* not passable */
	    if (intersect(&v1, &v2, current, current->next))
	       return (TRUE);
	    if (colinear(&v1, &v2, &v1, current) &&
		(min(v1.x, v2.x) <= current->x) &&
		(max(v1.x, v2.x) >= current->x) &&
		(min(v1.y, v2.y) <= current->y) &&
		(max(v1.y, v2.y) >= current->y))
	       return (TRUE);
	 }
	 current = current->next;
      } while (current != now_region->region->next);
      now_region = now_region->next;
   }
   return (FALSE);
}

/*-----------------------------------------------------------------------------

			Straighten_the_path

-----------------------------------------------------------------------------*/

void
straighten_the_path(struct path_node * path,
		    struct path_node * goal_path_point,
		    struct stack_element * convex_region_list)
{
   struct path_node *base, *test_end, *pnext;

   base = path;
   while (TRUE)
   {
      if (base == NULL)
	 break;
      if (goal_path_point == NULL)
	 test_end = NULL;
      else
	 test_end = goal_path_point->next_point;
      if (base->next_point == test_end)	/* special case - point before end
					 * deleted */
	 break;
      if (base->next_point->next_point == test_end)
	 break;
      pnext = base->next_point;
      while (TRUE)
      {
	 if (pnext->next_point == test_end)	/* last point */
	    break;
	 else
	 {
	    if (plus2_line_segment(base, pnext->next_point, convex_region_list))	/* don't drop */
	       pnext = pnext->next_point;
	    else
	    {			/* cut out point */
	       base->next_point = pnext->next_point;	/* no previous to doctor */
	       pnext = pnext->next_point;
	    }
	 }
      }
      base = base->next_point;
   }
   return;
}

/*--------------------------------------------------------------------------

				MODIFY_BORDER

		Accepts a region and inserts a new vertex into that region
		starting at the point passed to it and containing the
		values of add (except for corresponding and passable
		Returns new vertex.

---------------------------------------------------------------------------*/

struct vertex_list_element *modify_border(
    struct vertex_list_element *add, 
    struct vertex_list_element *vertex)
{
   struct vertex_list_element *new1;

   new1 = malloc_vertex();	/* create new vertex */
   new1->features = NULL;
   new1->x = add->x;
   new1->y = add->y;
   new1->passable = NULL;	/* automatically NULL */
   new1->corresponding = NULL;	/* ditto */
   /* weave into list */
   new1->previous = vertex;
   new1->next = vertex->next;
   vertex->next->previous = new1;
   vertex->next = new1;
   return (new1);		/* return region starting at new vertex */
}

/*-----------------------------------------------------------------------------

			CUT_SHORT_PERP

		Accepts three vertices
		Constructs a perpendicular to p1-chopper by
		changing the values of victim.
		Right angle results
		Changes the values in victim destructively

-----------------------------------------------------------------------------*/

void
cut_short_perp(struct vertex_list_element * p1,
	       struct vertex_list_element * victim,
	       struct vertex_list_element * chopper)
{
    //double          round_float();
   double          length, angle, world_angle, line_adj;
   struct vertex_list_element v_plus, y_plus;

   length = find_length(victim, chopper);
   angle = compute_angle(p1, victim, chopper);
   /* compute distance to move along line */
   line_adj = length * cos(RADIANS_FROM_DEGREES(angle));
   y_plus.x = victim->x;	/* used for world angle */
   y_plus.y = victim->y + 5.0;
   if ((p1->y - victim->y) == 0.0)
   {				/* special case - parallel to x-axis */
      if (inclusive_correctside(&y_plus, victim, chopper))
	 world_angle = 0.0;
      else
	 world_angle = 180.0;

   }
   else
   {				/* regular case */
      v_plus.x = victim->x + 5.0;
      v_plus.y = victim->y;
      world_angle = compute_angle(p1, victim, &v_plus);
   }
   if ((chopper->y - victim->y) == 0)
   {
      if (!correctside(victim, &v_plus, p1))
	 world_angle = 360 - world_angle;

   }
   else
   {
      if (!inclusive_correctside(victim, &v_plus, p1))	/* correct if necessary */
	 world_angle = 360 - world_angle;
   }
   /* make final corrections */
   victim->x = round_float(victim->x + (line_adj * cos(RADIANS_FROM_DEGREES(world_angle))));
   victim->y = round_float(victim->y + (line_adj * sin(RADIANS_FROM_DEGREES(world_angle))));
   return;

}

/*-----------------------------------------------------------------------------

			ADJUST_ZONE_TO_RECTANGLE

	Converts rhombus(? parallelopiped) into rectangle
	Destructively changes values in zone
	Alters adjacent bordering regions by adding vertices

------------------------------------------------------------------------------*/

void adjust_zone_to_rectangle(
    struct vertex_list_element *zone,
    struct vertex_list_element *first,
    struct vertex_list_element *second)
{
   int             angle;
   struct vertex_list_element *l1p1, *l1p2, *l2p1, *l2p2;
   struct vertex_list_element *new_point;

   /* label lines - careful differs from labelling elsewhere */
   l1p1 = zone;
   l1p2 = zone->next;
   l2p1 = zone->next->next;
   l2p2 = zone->previous;

   if ((angle = ((int) (compute_angle(l1p1, l1p2, l2p1) + 0.5))) > 90)
   {
      /* chop l2p1 */
      cut_short_perp(l2p2, l2p1, l1p2);
      new_point = modify_border(l2p1, second);
      new_point->passable = NULL;
   }
   else
   {
      if (angle < 90)
      {
	 /* chop l1p2 */
	 cut_short_perp(l1p1, l1p2, l2p1);
	 new_point = modify_border(l1p2, first);
	 first->passable = NULL;
	 first = new_point;
      }
   }
   /* if equal 90 is OK as is */

   if ((angle = ((int) (compute_angle(l2p1, l2p2, l1p1) + 0.5))) > 90)
   {
      /* chop l1p1 */
      cut_short_perp(l1p2, l1p1, l2p2);
      new_point = modify_border(l1p1, first);
      new_point->passable = NULL;
   }
   else
   {
      if (angle < 90)
      {
	 /* chop l2p2 */
	 cut_short_perp(l2p1, l2p2, l1p1);
	 new_point = modify_border(l2p2, second);
	 second->passable = NULL;
	 second = new_point;
      }
   }
   /* if equal 90 is OK as is */

   /* make necessary passable connections to bordering regions */
   zone->passable = first;
   first->passable = zone;
   zone->next->next->passable = second;
   second->passable = zone->next->next;
}

/*--------------------------------------------------------------------------

				DOCTOR_TERRAIN

			Takes a candidate vertex of a new region
	Compares it against all old regions for overlap
	If found merges it into that region, taking care of all
		necessary bookkeeping (passable etc)
	Creates a transition zone and pushes it on temp_list

	Returns modified stack

---------------------------------------------------------------------------*/
struct stack_element
               *
doctor_terrain(
    struct vertex_list_element *vertex,
    struct stack_element *temp_list,
    double          terrain_weight)
{
   struct stack_element *current, *zone;
   struct vertex_list_element *rary, *save = NULL, *check, *zone_hold, l1p1,
                   l1p2, l2p1, l2p2;
   struct vertex_list_element perp1;
   struct terrain_identifier *malloc_terrain_identifier();
   struct error_facts *malloc_error_facts();
   int             found;

   printf("doctor terrain\n");
   found = FALSE;		/* initialize */
   current = temp_list;		/* get first region to compare */

   /* set up first line segment */
   l1p1.x = vertex->corresponding->x;
   l1p1.y = vertex->corresponding->y;
   l1p2.x = vertex->next->corresponding->x;
   l1p2.y = vertex->next->corresponding->y;
   if ((l1p2.x == l1p1.x) && (l1p2.y == l1p1.y))	/* this is significant -
							 * do not remove */
      return (temp_list);

   while (current != NULL)
   {				/* for all regions on old region list */
      check = current->region->next;	/* get first vertex */
      do
      {				/* for all vertices */
	 if ((check->previous->passable == NULL) && ((check->previous->corresponding != NULL) ||
					    (check->corresponding != NULL)))
	 {			/* must be passable */
	    if (check->corresponding == NULL)
	    {			/* modify as necessary */
	       /* move ahead to first corresponding */
	       rary = check->next;
	       while (rary->corresponding == NULL)
		  rary = rary->next;
	       l2p1.x = rary->corresponding->x;
	       l2p1.y = rary->corresponding->y;
	    }
	    else
	    {			/* non-null to begin with */
	       l2p1.x = check->corresponding->x;
	       l2p1.y = check->corresponding->y;
	    }
	    /* same for other point */
	    if (check->previous->corresponding == NULL)
	    {
	       /* move back until a corresponding found */
	       rary = check->previous->previous;
	       while (rary->corresponding == NULL)
		  rary = rary->previous;
	       l2p2.x = rary->corresponding->x;
	       l2p2.y = rary->corresponding->y;
	    }
	    else
	    {
	       l2p2.x = check->previous->corresponding->x;
	       l2p2.y = check->previous->corresponding->y;
	    }

	    /* safeguard - probably vestigial */
	    if ((l2p2.x == l2p1.x) && (l2p2.y == l2p1.y))	/* this is significant -
								 * do not remove */
	       goto skip1;

	    /* principal test */
	    if (colinear(&l1p1, &l1p2, &l2p1, &l2p2) && intersect(&l1p1, &l1p2, &l2p1, &l2p2))
	    {
	       /* found a match */
	       perp(vertex->next, vertex, &perp1);
	       if (correctside(vertex, &perp1, check->previous))
	       {
		  if (!found)
		  {		/* if more than one on a line */
		     save = check;
		     found = TRUE;
		  }
		  else
		  {
		     if (find_length(vertex, check) < find_length(vertex, save))
			save = check;
		  }
	       }
	    }
	 }
   skip1:check = check->next;
      } while (check != current->region->next);
      current = current->next;
   }

   /* found a linear match */

   if (found)
   {
      check = save;

      /* create stack element transition zone */

      zone = malloc_stack_element();
      zone_hold = zone->region = malloc_vertex();
      zone_hold->x = vertex->next->x;
      zone_hold->y = vertex->next->y;
      zone_hold->passable = vertex;
      vertex->passable = zone_hold;
      zone_hold->corresponding = NULL;
      zone_hold->features = NULL;

      zone_hold->next = malloc_vertex();
      zone_hold->next->previous = zone_hold;
      zone_hold = zone_hold->next;
      zone_hold->x = vertex->x;
      zone_hold->y = vertex->y;
      zone_hold->passable = NULL;	/* transition zone borders not
					 * passable */
      zone_hold->corresponding = NULL;
      zone_hold->features = NULL;

      zone_hold->next = malloc_vertex();
      zone_hold->next->previous = zone_hold;
      zone_hold = zone_hold->next;
      zone_hold->x = check->x;
      zone_hold->y = check->y;
      zone_hold->passable = check->previous;
      check->previous->passable = zone_hold;
      zone_hold->corresponding = NULL;
      zone_hold->features = NULL;

      zone_hold->next = malloc_vertex();
      zone_hold->next->previous = zone_hold;
      zone_hold = zone_hold->next;
      zone_hold->x = check->previous->x;
      zone_hold->y = check->previous->y;
      zone_hold->passable = NULL;	/* transition zone borders not
					 * passable */
      zone_hold->corresponding = NULL;
      zone_hold->features = NULL;

      /* tie off list */
      zone_hold->next = zone->region;
      zone->region->previous = zone_hold;

      /* square off zone */

      adjust_zone_to_rectangle(zone->region, vertex, check->previous);

      /* fix up terrain data */
      zone->terrain_type = malloc_terrain_identifier();
      zone->terrain_type->transition_zone = TRUE;

      /*
       * setting of traversal factor for transition zone arbitrarily chosen
       * at sum of two bordering zone values
       */

      zone->terrain_type->traversal_factor =	/* sum of two bordering
						 * terrain */
	 terrain_weight + (terrain_find_special(check, temp_list))->traversal_factor;
      zone->terrain_type->error_data = malloc_error_facts();


      /* --------------->add in TRANSITION ZONE ERROR DATA HERE<-------- */

      /* push new stack element (transition zone) on temp_list */


      zone->next = temp_list;
      return (zone);
   }
   else
      /* no match found */
      return (temp_list);
}


/*-----------------------------------------------------------------------------

			TERRAIN_PATCH

		High-level routine for splicing in terrain regions
		Returns adjusted convex_region stack containing
		all regions and splices

-----------------------------------------------------------------------------*/

struct stack_element *terrain_patch(
    struct stack_element *convex_list,
    struct stack_element *temp_list)
{
   //struct stack_element *current, *doctor_terrain(), *hold;
   struct stack_element *current, *hold;
   struct vertex_list_element *vertex, *hold_vertex, *save_it = NULL, *save_it2 = NULL;
   struct terrain_identifier *terrain_holder;
   int             hold_flag, second_flag;
   double          terrain_weight;
   struct terrain_identifier *malloc_terrain_identifier();
   struct error_facts *malloc_error_facts();

   //   printf("terrain patch\n");
   /* trivial case  - nothing to check against */
   hold_flag = FALSE;
   second_flag = FALSE;

   terrain_weight = 1.0;

   /* then no regions in list */
   if (temp_list == NULL)
   {
      terrain_holder = malloc_terrain_identifier();
      terrain_holder->traversal_factor = terrain_weight;
      terrain_holder->transition_zone = FALSE;
      terrain_holder->error_data = malloc_error_facts();
      current = convex_list;
      while (current != NULL)
      {				/* this code is below also */
	 current->terrain_type = terrain_holder;
	 current = current->next;
      }
      return (convex_list);
   }
   /* start at the top of the first new region */
   current = convex_list;
   while (current != NULL)
   {				/* for each region */

      vertex = current->region->next;	/* get first vertex */
      hold_flag = FALSE;
      second_flag = FALSE;
      do
      {				/* for each vertex */
	 hold_vertex = vertex->next;	/* save next vertex */
	 if ((vertex->passable == NULL) && (vertex->corresponding != NULL) &&
	     (vertex->next->corresponding != NULL))
	 {			/* must be a border vertex */
	    temp_list = doctor_terrain(vertex, temp_list, terrain_weight);	/* do this side */
	    if (hold_flag)
	    {
	       second_flag = TRUE;
	       save_it2 = save_it;
	    }
	 }
	 if (hold_vertex->previous != vertex)
	 {			/* back up to clear vertex - case where added */
	    if (hold_vertex->previous->corresponding == NULL)
	    {
	       hold_vertex->previous->corresponding = vertex->corresponding;
	       vertex = hold_vertex->previous;
	       hold_flag = TRUE;
	       save_it = vertex;
	    }
	    else
	    {
	       vertex = hold_vertex->previous;
	       hold_flag = FALSE;
	    }
	 }
	 else
	 {			/* plain case */
	    vertex = hold_vertex;
	    if (hold_flag)
	       save_it->corresponding = NULL;
	    hold_flag = FALSE;
	 }
	 if (second_flag)
	 {
	    save_it2->corresponding = NULL;
	    second_flag = FALSE;
	 }
      } while (vertex != current->region->next);

      current = current->next;	/* get next region on stack */
   }
   /* push all new regions onto temp_list */
   terrain_holder = malloc_terrain_identifier();
   terrain_holder->traversal_factor = terrain_weight;
   terrain_holder->transition_zone = FALSE;
   terrain_holder->error_data = malloc_error_facts();
   /* enter error data here as necessary */
   current = convex_list;	/* get first region */
   while (current != NULL)
   {				/* this code is above also */
      current->terrain_type = terrain_holder;
      hold = current->next;
      current->next = temp_list;/* push it on */
      temp_list = current;	/* new stack top */
      current = hold;		/* get next to push */
   }
   return (temp_list);
}

/*---------------------------------------------------------------------------

				NOT_ON_CLOSED3

	Boolean function - Returns TRUE if node is not on closed list

----------------------------------------------------------------------------*/

int not_on_closed3(
    struct open_node *open,
    struct vertex_list_element *vertex,
    struct closed_node3 *closed)
{
   struct closed_node3 *current;

   current = closed;
   while (current != NULL)
   {				/* check closed list */
      if ((current->open_x == open->x) && (current->open_y == open->y))
      {
	 if (((current->x1 == vertex->x) && (current->y1 == vertex->y) &&
	      (current->x2 == vertex->next->x) && (current->y2 == vertex->next->y))
	     || ((current->x2 == vertex->x) && (current->y2 == vertex->y) &&
		 (current->x1 == vertex->next->x) && (current->y1 == vertex->next->y)))
	 {

	    return (FALSE);	/* it is on closed */
	 }
      }
      current = current->next_cnode;
   }
   return (TRUE);
}

/*-------------------------------------------------------------------------

			A_STAR3 - A* search of free space

-------------------------------------------------------------------------*/

struct path_node *
a_star3(struct stack_element * map,	/* free space convex regions */
	struct vertex_list_element * start,	/* endpoints of path */
	struct vertex_list_element * goal,	/* endpoints of path */
	double safety_margin)

{
  struct closed_node3 *closed, *cnode, *temp_cnode;
  struct open_node *open;
  struct vertex_list_element *current, *region, temp_open;
  struct path_node *path, *start_path;
  char msg[80];

  closed = NULL;

  /* put start on open */
  open = (struct open_node *) malloc(sizeof(struct open_node));
  open->x = start->x;
  open->y = start->y;
  open->g = 0;

  open->terrain = find_start_goal(start, map);
  if( open->terrain == NULL )
    {
      sprintf(msg,"cart_lib:a_star3 - start location %3.2f,%3.2f not within map\n",
	      start->x,start->y);
      puts(msg);
      abort();
    }
  open->f = h_compute(open, (struct open_node *)goal);	/* OK since g is initially 0 */

  region = find_meadow(start, map);
  open->region = region;

  open->parent = NULL;
  open->next_onode = NULL;

  if (region == find_meadow(goal, map))
    {				/* goal in same region as start */
      path = (struct path_node *) malloc(sizeof(struct path_node));
      start_path = path;
      path->x = start->x;
      path->y = start->y;
      path->region1 = region;
      path->terrain_type = terrain_find_special(region, map);
      path->region2 = NULL;
      path->next_point = (struct path_node *) malloc(sizeof(struct path_node));
      path = path->next_point;
      path->x = goal->x;
      path->y = goal->y;
      path->region1 = region;
      path->terrain_type = terrain_find_special(region, map);
      path->region2 = NULL;
      path->next_point = NULL;
      return (start_path);
    }
  current = region->next;

  do
    {				/* add all passable line segments to open */
      if (current->passable != NULL)
	add_to_open3(current->passable, open, goal, map, safety_margin);
      current = current->next;
    } while (current != region->next);

  open = open->next_onode;	/* delete start from openlist */

  /* now all passable points are on open */
  closed = NULL;
  while (TRUE)
    {				/* search */
      /* Put first open node on closed */
      if (open == NULL)
	{				/* search fails */
	  sprintf(msg,"\n No path possible ");
	  puts(msg);
	  return (NULL);
	}
      temp_open.x = open->x;
      temp_open.y = open->y;
      if ((open->x == goal->x) && (open->y == goal->y))
	{
	  /* path found */
	  /* free closed nodes */
	  cnode = closed;
	  while (cnode != NULL)
	    {
	      temp_cnode = cnode->next_cnode;
	      free(cnode);
	      cnode = temp_cnode;
	    }
	  /* open nodes freed in build_path */
	  return (build_path(open, start, map));
	} /* if point is goal */

      if (same_region(find_meadow(goal, map), open->region) ||
	  ((open->region->passable != NULL) && 
	   same_region(find_meadow(goal, map), open->region->passable)))
	{
	  add_goal_to_open(open, goal, map);
	  open = open->next_onode;	/* delete node from openlist */
	} /* if point borders goal region */

      else
	{				/* must be in different meadow */
	  current = open->region->next;
	  do
	    {
	      if ((current->passable != NULL)
		  && not_on_closed3(open, open->region, closed))
		add_to_open3(current->passable, open, goal, map, safety_margin);

	      current = current->next;
	    } while (current != open->region);
	  cnode = (struct closed_node3 *) malloc(sizeof(struct closed_node3));
	  cnode->x1 = open->region->x;
	  cnode->y1 = open->region->y;
	  cnode->x2 = open->region->next->x;
	  cnode->y2 = open->region->next->y;
	  cnode->open_x = open->x;
	  cnode->open_y = open->y;
	  cnode->next_cnode = closed;
	  closed = cnode;
	  open = open->next_onode;
	}
    }
}

/*-----------------------------------------------------------------------------

				IMPROVE_PATH3

	Refinement of coarse path into straighter and/or tauter path

-----------------------------------------------------------------------------*/
void
improve_path3(struct path_node     *path,
	      struct stack_element *convex_region_list)
{
  struct path_node *hold_path;
  struct path_node *start_point, *goal_path_point, *real_beginning;
  struct terrain_identifier *terrain_type;
  char msg[80];

  real_beginning = path;
  hold_path = path;

  /* We need the tautening code here... */




  /* special for multi-terrain */

  if (straighten_path)
    {
      path = real_beginning;
      /* special case for start in transition zone */
      if (path->terrain_type->transition_zone)
	{
	  path = path->next_point;
	  sprintf(msg,"\n START IN TRANS ZONE");
	  puts(msg);
	  /* end special */
	}
      while (TRUE) /* outer */
	{
	  start_point = path;
	  while (TRUE) /* inner */
	    {			/* get a terrain start and end */
	      terrain_type = path->terrain_type;
	      // printf("terrain: %d %d\n", (int)(terrain_type), (int)(start_point->terrain_type));
	      if (terrain_type == start_point->terrain_type)
		{
		  path = path->next_point;
		}
	      else
		{
		  goal_path_point = path;	/* goal is last point with same
						 * terrain type */
		  break;
		}
	      if (path->next_point == NULL)
		{
		  /* done! do something special */
		  goal_path_point = NULL;
		  break;
		}
	    } /* while TRUE -- inner */
	  path = start_point;

	  //RRB
	  {
	    struct path_node *mypath = path;
	    int pathcount = 0;
	    printf("before straighten\n");
	    while(mypath != NULL){
	      printf("%d - %5.2f %5.2f    ",pathcount++, mypath->x, mypath->y);

	      if(mypath->region1){
		printf("R1: %5.2f %5.2f --> %5.2f %5.2f ||  ", mypath->region1->x, mypath->region1->y, 
		       mypath->region1->next->x, mypath->region1->next->y);
	      }
	      else printf("R1: NULL???   ");
	      if(mypath->region2){
		printf("R2: %5.2f %5.2f --> %5.2f %5.2f\n", mypath->region2->x, mypath->region2->y, 
		       mypath->region2->next->x, mypath->region2->next->y);
	      }
	      else printf("R2: NULL\n");

	      mypath = mypath->next_point;
	    }
	  }

	  straighten_the_path3(path, goal_path_point, convex_region_list);
	  //printf("straighten3 %d \n", (int)(goal_path_point));
	  
	  { //RRB
	    struct path_node *mypath = path;
	    int pathcount = 0;
	    printf("after straighten\n");
	    while(mypath != NULL){
	      printf("%d - %5.2f %5.2f\n",pathcount++, mypath->x, mypath->y);
	      mypath = mypath->next_point;
	    }
	  } // RRB

	  if (goal_path_point != NULL)
	    {
	      if (goal_path_point->next_point != NULL)
		{			/* must be transition zone */
		  if (goal_path_point->next_point->next_point != NULL)	/* end is not in
									 * transition zone */
		    {
		    path = goal_path_point->next_point;	/* skip straightening in
							 * t zone */
		  printf("t zone?\n");
		    }
		  else break;
		}
	      else break;
	    }
	  else break;
	} /* while TRUE -- outer */
    }
  return;
}

/*-----------------------------------------------------------------------------

				ADD_TO_OPEN3

	Adds a vertex to the open list for later expansion in A-star

------------------------------------------------------------------------------*/

void
add_to_open3(struct vertex_list_element * vertex,
	     struct open_node * open_list,
	     struct vertex_list_element * goal,
	     struct stack_element * map,
	     double safety_margin)
{
    struct open_node *rnode, *lnode, *node, *previous, *current;
   struct vertex_list_element *p1, *p2;
   // ENDO - gcc-3.4
   //double          g_compute2(), h_compute(), h, round_float();
   //double          correct_x(), correct_y(), cx, cy;
   double h;
   double cx, cy;

   node = (struct open_node *) malloc(sizeof(struct open_node));
   lnode = (struct open_node *) malloc(sizeof(struct open_node));
   rnode = (struct open_node *) malloc(sizeof(struct open_node));

   find_adit(node, vertex);	/* set x and y of adit */
   node->x = round_float(node->x);
   node->y = round_float(node->y);
   lnode->region = rnode->region = node->region = vertex;

   lnode->terrain = rnode->terrain = node->terrain = terrain_find(node, map);
   node->g = g_compute2(node, open_list);	/* sets g value, first node
						 * on open is expanding */
   h = h_compute2(node, (struct open_node *)goal, open_list);	/* heuristic evaluation */
   node->f = node->g + h;
   current = open_list->next_onode;
   node->parent = previous = open_list;
   while (current != NULL)
   {
      if (node->f < current->f)
      {
	 node->next_onode = previous->next_onode;
	 previous->next_onode = node;
	 break;
      }
      previous = current;
      current = current->next_onode;
   }
   if (current == NULL)
   {				/* Insert at end */
      node->next_onode = NULL;
      previous->next_onode = node;
   }
   /* now add left and right */
   p1 = vertex;
   p2 = vertex->next;
   if (safety_margin >= 0)
   {
      if (find_length(p1, p2) > (safety_margin * 2))
      {
	 cx = correct_x(p1, p2, safety_margin);
	 cy = correct_y(p1, p2, safety_margin);
	 lnode->x = round_float(p1->x + cx);
	 lnode->y = round_float(p1->y + cy);
	 rnode->x = round_float(p2->x - cx);
	 rnode->y = round_float(p2->y - cy);

	 /* insert left */
	 lnode->g = g_compute2(lnode, open_list);	/* sets g value, first
							 * node on open is
							 * expanding */
	 h = h_compute2(lnode, (struct open_node *)goal, open_list);	/* heuristic evaluation */
	 lnode->f = lnode->g + h;
	 current = open_list->next_onode;
	 previous = open_list;
	 lnode->parent = node->parent;
	 while (current != NULL)
	 {
	    if (lnode->f < current->f)
	    {
	       lnode->next_onode = previous->next_onode;
	       previous->next_onode = lnode;
	       break;
	    }
	    previous = current;
	    current = current->next_onode;
	 }
	 if (current == NULL)
	 {			/* Insert at end */
	    lnode->next_onode = NULL;
	    previous->next_onode = lnode;
	 }
	 /* now right */

	 rnode->g = g_compute2(rnode, open_list);	/* sets g value, first
							 * node on open is
							 * expanding */
	 h = h_compute2(rnode, (struct open_node *)goal, open_list);	/* heuristic evaluation */
	 rnode->f = rnode->g + h;
	 current = open_list->next_onode;
	 previous = open_list;
	 rnode->parent = node->parent;
	 while (current != NULL)
	 {
	    if (rnode->f < current->f)
	    {
	       rnode->next_onode = previous->next_onode;
	       previous->next_onode = rnode;
	       break;
	    }
	    previous = current;
	    current = current->next_onode;
	 }
	 if (current == NULL)
	 {			/* Insert at end */
	    rnode->next_onode = NULL;
	    previous->next_onode = rnode;
	 }
      }
   }
}


int inside_obstacle(struct path_node * p1, struct path_node * p2);

/*---------------------------------------------------------------------------

	Straighten the path 3
	
	Straightener for A3 search and relax path

---------------------------------------------------------------------------*/
void
straighten_the_path3(struct path_node *path,
		     struct path_node *goal_path_point,
		     struct stack_element *convex_region_list)
{
  struct path_node *base, *test_end, *pnext;

  base = path;
  while (TRUE)
    { /* outer */
      if (base == NULL)
	break;
      if (goal_path_point == NULL)
	test_end = NULL;
      else
	test_end = goal_path_point->next_point;
      if (base->next_point == test_end)	/* special case - point before end
					 * deleted */
	break;
      /* if there are just two points, this is optimal already. */
      if (base->next_point->next_point == test_end)
	break;

      pnext = base->next_point;
      while (TRUE) 
	{ /* inner */  /* loop for pnext. */
	  if (pnext->next_point == test_end)	/* last point */
	    break;
	  else
	    {
	      if (plus5_line_segment(base, pnext->next_point, convex_region_list))
		{			/* don't drop */
		  /*  printf("don't drop pnext: %5.2f %5.2f next: %5.2f %5.2f  third: %5.2f %5.2f -- plus5\n",
		      base->x, base->y, pnext->x, pnext->y, pnext->next_point->x, pnext->next_point->y); */
		  pnext = pnext->next_point;
		}
	      else if(inside_obstacle(base, pnext->next_point)){
		/*		  printf("don't drop pnext: %5.2f %5.2f next: %5.2f %5.2f -- inside obstacle!\n",
				  base->x, base->y, pnext->x, pnext->y); */
		  pnext = pnext->next_point;
	      }
	      else
		{			/* cut out point */
		  /*		  printf("drop pnext: %5.2f %5.2f next: %5.2f %5.2f  third: %5.2f %5.2f\n",
				  base->x, base->y, pnext->x, pnext->y, pnext->next_point->x, pnext->next_point->y); */
		  base->next_point = pnext->next_point;	/* no previous to doctor */
		  pnext = pnext->next_point;
		}
	    }
	} /* while TRUE -- inner */
      base = base->next_point;
    } /* while TRUE -- outer */
  return;
}

/*----------------------------------------------------------------
	PLUS5_LINE_SEGMENT

	Supercedes Plus4 - only used in relax path
	Handles problem with forbidden zones

Actually, it is used in straighten_the_path3 also.

This function tests the current line between two points against
every wall in every region.

-------------------------------------------------------------------------*/

int
plus5_line_segment(struct path_node * p1,
		   struct path_node * p2,
		   struct stack_element * region_list)
{
  struct vertex_list_element v1, v2, *current;
  struct stack_element      *now_region;

  v1.x = p1->x;
  v1.y = p1->y;
  v2.x = p2->x;
  v2.y = p2->y;
  now_region = region_list;
  while (now_region != NULL)
    {  /* loop for now_region */
      current = now_region->region->next;
      //      printf("%5.2f %5.2f    %5.2f %5.2f --- \n", v1.x, v1.y, v2.x, v2.y);
      do
	{ /* loop for current */
	  //  printf("    %5.2f %5.2f    %5.2f %5.2f  ", current->x, current->y, current->next->x, current->next->y);
	  if (current->passable == NULL)
	    {
	      //printf("WALL  ");
	      /* not passable */
	      if (intersect(&v1, &v2, current, current->next))
		{
		  //printf("INTSCT  ");
		  if (!(colinear(&v1, &v2, current, current->next)))
		    {
		      //printf("NOT_COLINEAR\n");
		      return (TRUE);
		    }
		}
	      //printf("NO_INTSCT\n");
	    } /* !passable */
	  //else printf("PASS  \n");
	  current = current->next;
	} while (current != now_region->region->next);
      now_region = now_region->next;
    } /* while now_region not null */
  return (FALSE);
}


/* The basic idea here is to 
1. Build a list of region/vertex for the path
2. Look for a convex angle of two walls in list.
3. return: don't drop

 */
/* threshold is a tenth of a meter... */
#define DIST_THRESH 0.1

int
inside_obstacle(struct path_node * p1, struct path_node * p2)
{
  struct vertex_list_element v1;

  double dist, dx, dy, angle;

  dx = p2->x - p1->x;
  dy = p2->y - p1->y;
  dist = sqrt((dx * dx) + (dy * dy));
  if(dist < DIST_THRESH) return FALSE;

  angle = atan2(dy, dx);
  
  v1.x = p1->x + DIST_THRESH*cos(angle);
  v1.y = p1->y + DIST_THRESH*sin(angle);

  if (find_meadow(&v1, long_term_memory.top_of_convex_region_stack) == NULL) return TRUE;
  else{
    //    printf("inside obstacle: found a meadow for: %5.2f %5.2f -- %5.2f %5.2f\n", 

    return (FALSE);
  } 
}


/******************************************************************

				RELAX_PATH

		Performs relaxation on transition zones

******************************************************************/

void
relax_path(struct path_node * path_start,
	   double safety_margin,
	   struct stack_element * region_list,
	   double relax_increment_size)
{
   struct path_node *current, *previous, *path, *goal_path_point, *start_point;
   struct terrain_identifier *terrain_type;
   double          length, divisor, dx, dy, cost0, cost1, cost2,
                   relax_start;
   double          temp, cx, cy;
   struct relax_path_node *temp_node, *node, *relax_list, *end_relax_list = NULL;

   while (TRUE)
   {
      /* initialize relax_list */
      relax_list = NULL;
      previous = NULL;
      current = path_start;
      if (current->terrain_type->transition_zone)
      {
	 /* start is in trans zone */
	 relax_list = (struct relax_path_node *) malloc(sizeof(struct relax_path_node));
	 end_relax_list = relax_list;
	 end_relax_list->next = NULL;
	 end_relax_list->point1 = current->next_point;
	 end_relax_list->point2 = end_relax_list->check_point2 = NULL;
	 end_relax_list->check_point1 = current->next_point->next_point;
      }
      previous = current;
      current = current->next_point;
      while (current->next_point != NULL)
      {
	 if (current->terrain_type->transition_zone)
	 {			/* transition zone */
	    if (current->next_point->next_point == NULL)
	    {
	       /* end is in t zone */
	       if (relax_list != NULL)
	       {
		  end_relax_list->next = (struct relax_path_node *) malloc(sizeof(struct relax_path_node));
		  end_relax_list = end_relax_list->next;
	       }
	       else
		  relax_list = end_relax_list = (struct relax_path_node *) malloc(sizeof(struct relax_path_node));
	       end_relax_list->next = NULL;
	       end_relax_list->point1 = current;
	       end_relax_list->check_point1 = previous;
	       end_relax_list->point2 = end_relax_list->check_point2 = NULL;
	       break;
	    }
	    /* is regular two slide type - full zone */
	    if (relax_list != NULL)
	    {
	       end_relax_list->next = (struct relax_path_node *) malloc(sizeof(struct relax_path_node));
	       end_relax_list = end_relax_list->next;
	    }
	    else
	       relax_list = end_relax_list = (struct relax_path_node *) malloc(sizeof(struct relax_path_node));
	    end_relax_list->next = NULL;
	    end_relax_list->point1 = current;
	    end_relax_list->check_point1 = previous;
	    end_relax_list->point2 = current->next_point;
	    end_relax_list->check_point2 = current->next_point->next_point;
	    previous = current->next_point;
	    current = current->next_point->next_point;
	 }
	 else
	 {			/* move to next path node */
	    previous = current;
	    current = current->next_point;
	 }
      }
      /* initial nodes allocated now pass through and compute values */
      /* first must be done differently - if in zone */

      /*       printf("relax list: %d\n", (int)relax_list); */
      node = relax_list;	/* get first node */
      while (node != NULL)
      {				/* while nodes remain to do */
	 node->hold1_x = node->point1->x;
	 node->hold1_y = node->point1->y;
	 if (node->point2 != NULL)
	 {
	    node->hold2_x = node->point2->x;
	    node->hold2_y = node->point2->y;
	 }

	 if (find_length(node->point1->region1,
			 node->point1->region1->next) > (safety_margin * 2))
	 {
	    /* can be moved */
	    dx = node->point1->region1->x - node->point1->region1->next->x;
	    dy = node->point1->region1->y - node->point1->region1->next->y;
	    length = sqrt((dx * dx) + (dy * dy));
	    divisor = length / relax_increment_size;
	    node->delta_x = dx / divisor;
	    node->delta_y = dy / divisor;
	    cx = correct_x(node->point1->region1, node->point1->region1->next, safety_margin);
	    cy = correct_y(node->point1->region1, node->point1->region1->next, safety_margin);
	    if ((dx != 0) && (dy != 0))
	    {
	       node->plus_limit_x = node->point1->region1->x + cx;
	       node->plus_limit_y = node->point1->region1->y + cy;
	       node->minus_limit_x = node->point1->region1->next->x - cx;
	       node->minus_limit_y = node->point1->region1->next->y - cy;
	    }
	    else
	    {
	       node->plus_limit_x = node->point1->region1->x + cx;
	       node->plus_limit_y = node->point1->region1->y + cy;
	       node->minus_limit_x = node->point1->region1->next->next->x - cx;
	       node->minus_limit_y = node->point1->region1->next->next->y - cy;
	    }
	    if (node->plus_limit_x < node->minus_limit_x)
	    {			/* swap */
	       temp = node->plus_limit_x;
	       node->plus_limit_x = node->minus_limit_x;
	       node->minus_limit_x = temp;
	    }
	    if (node->plus_limit_y < node->minus_limit_y)
	    {			/* swap */
	       temp = node->plus_limit_y;
	       node->plus_limit_y = node->minus_limit_y;
	       node->minus_limit_y = temp;
	    }
	 }
	 else
	 {			/* cannot be moved for this safety margin */
	    node->delta_x = 0;
	    node->delta_y = 0;
	    node->minus_limit_x = node->minus_limit_y = PATH_PLAN_INFINITY;
	    node->plus_limit_x = node->plus_limit_y = -PATH_PLAN_INFINITY;
	 }
	 node = node->next;
      }
      /* all limits set */
      /* start relaxation */
      relax_start = path_cost(path_start);
      while (TRUE)
      {				/* while not relaxed */
	 node = relax_list;	/* start at beginning */
	 while (node != NULL)
	 {			/* while not at end of a pass */
	    if (node->delta_x != 0 || node->delta_y != 0)
	    {			/* node is capable of movement */
	       /* save old values */
	       cost0 = path_cost(path_start);
	       node->hold1_x = node->point1->x;	/* save old values of point 1 */
	       node->hold1_y = node->point1->y;
	       if (node->point2 != NULL)
	       {		/* save old values of point 2 if needed */
		  node->hold2_x = node->point2->x;
		  node->hold2_y = node->point2->y;
	       }
	       node->point1->x = node->hold1_x + node->delta_x;
	       node->point1->y = node->hold1_y + node->delta_y;
	       if (node->point2 != NULL)
	       {		/* adjust point 2 if needed */
		  node->point2->x = node->hold2_x + node->delta_x;
		  node->point2->y = node->hold2_y + node->delta_y;
	       }

	       if ((node->point1->x < node->minus_limit_x) ||
		   (node->point1->y < node->minus_limit_y) ||
		   (node->point1->x > node->plus_limit_x) ||
		   (node->point1->y > node->plus_limit_y))
		  cost1 = PATH_PLAN_INFINITY;	/* out of bounds */
	       else		/* in bounds */
		  cost1 = path_cost(path_start);
	       /* check for collisions - if so cost = PATH_PLAN_INFINITY */
	       if (cost1 != PATH_PLAN_INFINITY)
	       {		/* cut down computation */
		  if (plus5_line_segment(node->point1, node->check_point1, region_list))
		     cost1 = PATH_PLAN_INFINITY;
		  else
		  {
		     if (node->point2 != NULL)
		     {
			if (plus5_line_segment(node->point2, node->check_point2, region_list))
			   cost1 = PATH_PLAN_INFINITY;
		     }
		  }
	       }
	       /* start cost 2 */
	       node->point1->x = node->hold1_x - node->delta_x;
	       node->point1->y = node->hold1_y - node->delta_y;
	       if (node->point2 != NULL)
	       {		/* adjust point 2 if needed */
		  node->point2->x = node->hold2_x - node->delta_x;
		  node->point2->y = node->hold2_y - node->delta_y;
	       }

	       if ((node->point1->x < node->minus_limit_x) ||
		   (node->point1->y < node->minus_limit_y) ||
		   (node->point1->x > node->plus_limit_x) ||
		   (node->point1->y > node->plus_limit_y))
		  cost2 = PATH_PLAN_INFINITY;	/* out of bounds */
	       else		/* in bounds */
		  cost2 = path_cost(path_start);
	       /* check for collisions - if so cost = PATH_PLAN_INFINITY */
	       if (cost2 != PATH_PLAN_INFINITY)
	       {		/* cut down computation */
		  if (plus5_line_segment(node->point1, node->check_point1, region_list))
		     cost2 = PATH_PLAN_INFINITY;
		  else
		  {
		     if (node->point2 != NULL)
		     {
			if (plus5_line_segment(node->point2, node->check_point2, region_list))
			   cost2 = PATH_PLAN_INFINITY;
		     }
		  }
	       }
	       /* start cost comparisons */
	       if ((cost0 < cost1) && (cost0 < cost2))
	       {		/* no change is lowest */
		  node->point1->x = node->hold1_x;
		  node->point1->y = node->hold1_y;

		  if (node->point2 != NULL)
		  {
		     node->point2->x = node->hold2_x;
		     node->point2->y = node->hold2_y;
		  }
	       }
	       if ((cost1 < cost2) && (cost1 < cost0))
	       {		/* plus delta is lowest */
		  node->point1->x = node->hold1_x + node->delta_x;
		  node->point1->y = node->hold1_y + node->delta_y;
		  if (node->point2 != NULL)
		  {
		     node->point2->x = node->hold2_x + node->delta_x;
		     node->point2->y = node->hold2_y + node->delta_y;
		  }
	       }
	    }			/* end if delta_x = 0 */
	    node = node->next;
	 }			/* end do nodes */
	 cost0 = path_cost(path_start);
	 if (cost0 >= relax_start + RELAX_LIMIT)
	    break;
	 else
	    relax_start = cost0;
      }				/* end while TRUE  in relax loop */
      /* reset to old values - lower cost than last pass */
      node = relax_list;
      while (node != NULL)
      {				/* while not end */
	 node->point1->x = node->hold1_x;	/* move old values in */
	 node->point1->y = node->hold1_y;
	 if (node->point2 != NULL)
	 {			/* same for point 2 if there is one */
	    node->point2->x = node->hold2_x;
	    node->point2->y = node->hold2_y;
	 }
	 /* free up storage for relax_nodes here */
	 temp_node = node->next;
	 free(node);
	 node = temp_node;
      }				/* end while */

      /* all relaxed */
      relax_start = path_cost(path_start);	/* get relaxed cost */

      /* set up for attempted restraightening */
      if (!straighten_path)
	 return;
      else
      {
	 path = path_start;
	 /* special case for start in transition zone */
	 if (path->terrain_type->transition_zone)
	    path = path->next_point;
	 /* end special */

	 while (TRUE)
	 {
	    start_point = path;
	    do
	    {			/* get a terrain start and end */
	       terrain_type = path->terrain_type;
	       if (terrain_type == start_point->terrain_type)
		  path = path->next_point;
	       else
	       {
		  goal_path_point = path;	/* goal is last point with
						 * same terrain type */
		  break;
	       }
	       if (path->next_point == NULL)
	       {
		  /* done! do something special */
		  goal_path_point = NULL;
		  break;
	       }
	    } while (TRUE);
	    path = start_point;

	    straighten_the_path3(path, goal_path_point, region_list);
	    if (goal_path_point != NULL)
	    {
	       if (goal_path_point->next_point != NULL)
	       {		/* must be transition zone */
		  if (goal_path_point->next_point->next_point != NULL)	/* end is not in
									 * transition zone */
		     path = goal_path_point->next_point;	/* skip straightening in
								 * t zone */
		  else
		     break;
	       }
	       else
		  break;
	    }
	    else
	       break;
	 }
	 if (path_cost(path_start) >= relax_start)	/* straighten no effect */
	    break;		/* all done */
	 /* otherwise  loop - try again */
      }
   }				/* first while (TRUE) */
   return;
}

/*---------------------------------------------------------------------------

			SPECIAL_CROSS_VERTEX

			Handles intersecting grown vertices

----------------------------------------------------------------------------*/

struct vertex_list_element *special_cross_vertex(
    struct vertex_list_element *v1,
    struct vertex_list_element *v2,
    struct vertex_list_element *v3,
    struct vertex_list_element *v4,
    struct vertex_list_element *next)
{
   struct vertex_list_element *tnext;
   int             stat1, stat2;
   struct vertex_list_element *new1;
   double          a, b, c, d;

   v4->corresponding = v1;
   v2->corresponding = v3;
   tnext = add_vertex(next, v4);
   new1 = malloc_vertex();
   new1->features = NULL;
   stat1 = line_equation(v1, v2, &a, &b);
   stat2 = line_equation(v3, v4, &c, &d);
   if (stat1 && stat2)
   {
      new1->x = (d - b) / (a - c);
      new1->y = (c * b - a * d) / (c - a);
   }
   else
   {
      if (stat1)
      {
	 new1->x = v1->x;
	 new1->y = a * new1->x + b;
      }
      else
      {
	 new1->x = v2->x;
	 new1->y = c * new1->x + d;
      }
   }

   new1->corresponding = v1->next;
   tnext = add_vertex(tnext, new1);
   tnext = add_vertex(tnext, v2);
   return (tnext);
}


/*------------------------------------------------------------------------------

				GROW_REGION

	Grows the initial convex region out a la configuration space

NOTE: is destructive - destroys old stack element 

------------------------------------------------------------------------------*/

void
grow_region(struct stack_element * stack_top, double length)
{
  struct stack_element *old_stack;
  struct vertex_list_element *next, *region_head, *current, *temp;
  struct vertex_list_element *temp_hold, *first, *hold = NULL, *temp2, *check;
  int             hold_flag, first_flag;

  old_stack = stack_top;
  current   = old_stack->region->next;
  first     = current;

  region_head = malloc_vertex();
  region_head->features = NULL;
  region_head->previous = NULL;
  region_head->next     = NULL;
  next = region_head;

  do
    {				/* for all vertices */
      hold_flag = 0;
      first_flag = 0;
      temp_hold = NULL;
      temp = grow(current, length);
      temp->passable = NULL;
      temp->corresponding = current;
      check = current->next;
      do
	{				/* check each vertex for collision */
	  temp2 = grow(check, length);
	  temp2->passable = NULL;
	  temp2->corresponding = check;
	  if (intersect(current, temp, check, temp2))
	    {			/* special case */
	      /* set flag and save values */
	      first_flag++;
	      if (temp_hold != NULL)
		{
		  free_vertex(temp_hold);
		}
	      hold = check;
	      temp_hold = temp2;

	    } /* if they intersect... */
	  if (first_flag == 0)  /* if there have been no intersections */
	    free_vertex(temp2);	/* free up storage */
	  else
	    {
	      if (first_flag == hold_flag) /* if no new crossing */
		free_vertex(temp2); /* free up storage */
	    }
	  hold_flag = first_flag;
	  check = check->next;
	} while (check != current);	/* check next   */

      if (first_flag == 0)
	{				/* no crossings */
	  if (concave(current) && (compute_angle(current->previous, current, current->next) < CHOP_ANGLE))
	    next = chop_concave_angle(current, next, FALSE, length);
	  else
	    next = add_vertex(next, temp);
	}
      else
	{				/* crossings present */
	  next = special_cross_vertex(current, temp, hold, temp_hold, next);
	  current = hold;
	}
      current = current->next;
    } while (current != first);

  next->next = region_head->next;
  region_head->previous = next;
  region_head->next->previous = region_head->previous;
  stack_top->region = region_head;
  /* This appears to simply set adrift the previous linked list. */

  /* fixup overlapping regions */
  fixup_overlap_border(stack_top);
  round_region(stack_top);
  return;
}



/*----------------------------------------------------------------------------

			SPECIAL_OBSTACLE_VERTEX

		Handles special case for obstacle_vertex

-----------------------------------------------------------------------------*/

struct vertex_list_element * 
special_obstacle_vertex(struct vertex_list_element *v1,
			struct vertex_list_element *v2, 
			struct vertex_list_element *v3, 
			struct vertex_list_element *v4, 
			struct vertex_list_element *next)
{
   struct vertex_list_element *tnext;
   int             stat1, stat2;
   struct vertex_list_element *new1;
   double          a, b, c, d;

   v4->corresponding = v1;
   v2->corresponding = v3;
   tnext = add_vertex(next, v4);
   new1 = malloc_vertex();
   new1->features = NULL;
   stat1 = line_equation(v1, v2, &a, &b);
   stat2 = line_equation(v3, v4, &c, &d);
   if (stat1 && stat2)
   {
      new1->x = (d - b) / (a - c);
      new1->y = (c * b - a * d) / (c - a);
   }
   else
   {
      if (stat1)
      {
	 new1->x = v1->x;
	 new1->y = a * new1->x + b;
      }
      else
      {
	 new1->x = v2->x;
	 new1->y = c * new1->x + d;
      }
   }
   new1->corresponding = v1->next;
   tnext = add_vertex(tnext, new1);
   tnext = add_vertex(tnext, v2);
   return (tnext);
}
/*-----------------------------------------------------------------------------

				GROW_ONE_OBSTACLE

				As the name implies

------------------------------------------------------------------------------*/

/* is destructive - destroys old stack element */
static void
grow_one_obstacle(struct obstacle_list_element * obstacle,double length)
{
   struct obstacle_list_element *old_obstacle;
   struct vertex_list_element *next, *region_head, *current, *temp;
   struct vertex_list_element *first, *hold = NULL, *temp2, *check;
   int             first_flag, hold_flag;
   struct vertex_list_element *temp_hold;

   old_obstacle = obstacle;
   region_head = malloc_vertex();
   next = region_head;
   region_head->features = NULL;
   region_head->previous = NULL;
   region_head->next = NULL;
   current = obstacle->actual_obstacle->next;
   first = current;
   do
   {				/* for all obstacle vertices */
      hold_flag = first_flag = 0;
      temp_hold = NULL;
      temp = grow_obstacle_vertex(current,length);
      temp->passable = NULL;
      temp->corresponding = current;
      check = current->next;
      do
      {				/* check for collision with other obstacle
				 * vertices */
	 temp2 = grow_obstacle_vertex(check,length);
	 temp2->passable = NULL;
	 temp2->corresponding = check;
	 if (intersect(current, temp, check, temp2))
	 {
	    first_flag++;
	    if (temp_hold != NULL)
	    {
	       free_vertex(temp_hold);
	    }
	    hold = check;
	    temp_hold = temp2;
	 }
	 if (first_flag == 0)
	    free_vertex(temp2);	/* free up storage */
	 else
	 {
	    if (first_flag == hold_flag)
	       free_vertex(temp2);
	 }
	 hold_flag = first_flag;
	 check = check->next;
      } while (check != current);
      if (first_flag == 0)
      {
	 if (concave(current) &&
	     (compute_angle(current->previous, current, current->next) < CHOP_ANGLE))
	    next = chop_concave_angle(current, next, TRUE, length);
	 else
	    next = add_vertex(next, temp);
      }
      else
      {
	 next = special_obstacle_vertex(current, temp, hold, temp_hold, next);
	 current = hold;
      }
      current = current->next;
   } while (current != first);
   next->next = region_head->next;
   region_head->previous = next;
   region_head->next->previous = next;
   obstacle->grown_obstacle = region_head->next;
   fixup_overlap_obstacle(obstacle);
   round_obstacle(obstacle);

   /* remove any newly redundant points */
   current = region_head->next;
   do
   {
      if ((current->x == current->next->x) &&
	  (current->y == current->next->y))
      {
	 current->next = current->next->next;
	 current->next->previous = current;
      }
      else
	 current = current->next;
   } while (current != region_head->next);

   return;
}

/*----------------------------------------------------------------------------
			
			GROW OBSTACLES

-----------------------------------------------------------------------------*/

void
grow_obstacles(struct obstacle_list_element *list_head, double length)
{
   struct obstacle_list_element *current_obstacle;

   current_obstacle = list_head;
   while (current_obstacle != NULL)
   {
      grow_one_obstacle(current_obstacle, length);
      current_obstacle = current_obstacle->next;
   }
}

/*-----------------------------------------------------------------------------

				IMPROVE_PATH2

	Refinement of coarse path into straighter and/or tauter path

The basic idea here is that each point in the path is a bead on a 
frictionless wire. We apply tension at the ends, and the beads find equilibrium.

1) Identify the line that will be the "wire" for each bead.

2) Each bead has a location (x,y), and "fixed" flag.

3) Ends start fixed.

4) Movement vector for each bead is the sum of the cosines of the two angles
of incidence multiplied by delta.

5) If an adjacent bead is co-located, look beyond it until one is found 
that is not co-located.

6) If the resultant vector drives the bead into an obstacle, that
bead becomes "fixed".

7) If a bead remains at the same location within some tolerance, "fix" it.

8) When they're all "fixed", end.

-----------------------------------------------------------------------------*/

struct bead {
  double x1;
  double y1;
  double x2;
  double y2;
  int fixed;
  struct bead *next;
  struct bead *previous;

  double x_now;
  double y_now;
  double x_next;
  double y_next;

  double angle;
  double distsq;
};

int colocatedBB(struct bead *bead1, struct bead *bead2)
{
  if((bead1->x_now == bead2->x_now) && (bead1->y_now == bead2->y_now)) return 1;
  else return 0;
}

//#define PULL_SPEED 0.5
#define PULL_SPEED 0.25
/*#define RRB_DEBUG 1 */

void improve_path2(struct path_node     *path,
		   struct stack_element *convex_region_list)
{
  struct path_node  *oldpnode = NULL;
  struct bead *wirehead = NULL, *lastbead = NULL, *newbead = NULL;
  struct bead *pre = NULL, *post = NULL;
  int improving = 0;
  double dx, dy;
  int sanityCheck = 0;
  double TotalLength = 0.0;
  double NewLength = 0.0, OldNewLength = 1000000.0;
  double angle1, angle2, deltaprev, deltapost, pull;

  oldpnode = path;
  /* copy the path from path to newpathhead */

#ifdef RRB_DEBUG
  printf("current path:\n");
#endif
  while(oldpnode){
    newbead = (struct bead *) malloc(sizeof(struct bead));
    newbead->next = NULL;
    newbead->previous = NULL;
    newbead->angle = 0.0;
    newbead->distsq = 0.0;
    newbead->x1 = oldpnode->x;
    newbead->y1 = oldpnode->y;
    newbead->x_now = newbead->x1;
    newbead->y_now = newbead->y1;
    newbead->x_next = newbead->x1;
    newbead->y_next = newbead->y1;

    if(!wirehead){ /* First element in the path */
      wirehead = newbead;
      newbead->fixed = 1;
      newbead->x2 = newbead->x1;
      newbead->y2 = newbead->y1;
      newbead->angle = 0;
#ifdef RRB_DEBUG
      printf("  %5.2f %5.2f --     fixed: %d\n", newbead->x_now, newbead->y_now, newbead->fixed);
#endif

    }
    else if(oldpnode->next_point){ /* intermediate element */
      lastbead->next = newbead;
      newbead->previous = lastbead;
      newbead->fixed = 0;
      if((newbead->x1 == oldpnode->region1->x) && (newbead->y1 == oldpnode->region1->y)){
	newbead->x2 = oldpnode->region2->x;
	newbead->y2 = oldpnode->region2->y;
      }
      else if((newbead->x1 == oldpnode->region2->x) && (newbead->y1 == oldpnode->region2->y)){
	newbead->x2 = oldpnode->region1->x;
	newbead->y2 = oldpnode->region1->y;
      }
      else if((newbead->x1 == round_float(0.5*(oldpnode->region1->x + oldpnode->region2->x))) && 
	      (newbead->y1 == round_float(0.5*(oldpnode->region1->y + oldpnode->region2->y)))){
	newbead->x1 = oldpnode->region1->x;
	newbead->y1 = oldpnode->region1->y;
	newbead->x2 = oldpnode->region2->x;
	newbead->y2 = oldpnode->region2->y;
      }
      else{
	printf("cart_lib:improve_path2 -- trouble with path regions.\n");
	printf("newbead: p1: %5.2f %5.2f  p2: %5.2f %5.2f\n", newbead->x1, newbead->y1, newbead->x2, newbead->y2);
	printf("oldnode R1: %5.2f %5.2f  R2: %5.2f %5.2f\n", oldpnode->region1->x, oldpnode->region1->y,
	       oldpnode->region2->x, oldpnode->region2->y);
	exit(4);
      }
      dx = newbead->x2 - newbead->x1;
      dy = newbead->y2 - newbead->y1;
      newbead->angle = atan2(dy, dx);
      newbead->distsq = dx*dx + dy*dy;
      improving++;
    }
    else{  /* Last element  */
      lastbead->next = newbead;
      newbead->previous = lastbead;
      newbead->fixed = 1;
      newbead->x2 = newbead->x1;
      newbead->y2 = newbead->y1;
      newbead->angle = 0;
    }
    if(newbead->previous){
      dx = newbead->x_now - newbead->previous->x_now;
      dy = newbead->y_now - newbead->previous->y_now;
      TotalLength += sqrt(dx*dx + dy*dy);
#ifdef RRB_DEBUG
      printf("  %5.2f %5.2f -- length: %5.2f fixed? %d\n", newbead->x_now, newbead->y_now, sqrt(dx*dx + dy*dy), newbead->fixed);
#endif
    }
    lastbead = newbead;
    oldpnode = oldpnode->next_point;
  } /* loop for oldpnode */

#ifdef RRB_DEBUG
  printf("Total Length = %5.2f Improvable nodes: %d\n", TotalLength, improving);
#endif

  /* At this point, we've created a doubly linked list of beads
     representing all the path nodes, and identified the line segment
     upon which the bead can ride between (x1,y1) and (x2,y2). (beads start at (x1,y1).)
     The two endpoints have been marked as fixed.
     "improving" is set to the number of nodes still potentially moving.
  */

  
#ifdef RRB_DEBUG
  newbead = wirehead;
  while(newbead){
    printf("Bead: P1:    %5.2f %5.2f   P2: %5.2f %5.2f\n", newbead->x1, newbead->y1, newbead->x2, newbead->y2);
    printf("      PNOW:  %5.2f %5.2f  NXT: %5.2f %5.2f\n", newbead->x_now, newbead->y_now, newbead->x_next, newbead->y_next);
    printf("      angle: %5.2f distsq: %5.2f, fixed: %d\n", newbead->angle, newbead->distsq, newbead->fixed);
    printf("      PREV: %s  NEXT: %s\n", (newbead->previous?"PTR ":"NULL"), (newbead->next?"PTR ":"NULL"));
    newbead = newbead->next;
  }
#endif

  while(improving){
    sanityCheck++;
#ifdef RRB_DEBUG
    printf("loops: %d\n", sanityCheck);
#endif
    newbead = wirehead;
    while(newbead){ /* loop through all beads and adjust the position of those not fixed. */
      if(newbead->fixed){ /* don't modify the fixed beads */
#ifdef RRB_DEBUG
	printf("skipping bead at %5.2f %5.2f -- fixed\n", newbead->x_now, newbead->y_now);
#endif
	newbead = newbead->next;
	continue;
      }
#ifdef RRB_DEBUG
      printf("bead at %5.2f %5.2f can be moved\n", newbead->x_now, newbead->y_now);
#endif
      /* Check for co-location and establish non-co-located pre and post locations */
      pre = newbead->previous;
      while(colocatedBB(newbead, pre)) pre = pre->previous;
      post = newbead->next;
      while(colocatedBB(newbead, post)) post = post->next;

      /* Now we have non-co-located neighbors who will be exerting a pull. */
      angle1 = atan2(pre->y_now - newbead->y_now, pre->x_now - newbead->x_now);
      angle2 = atan2(post->y_now - newbead->y_now, post->x_now - newbead->x_now);
      
      deltaprev = newbead->angle - angle1;
      deltapost = angle2 - newbead->angle;

      pull = (cos(deltaprev) + cos(deltapost))*PULL_SPEED;
      newbead->x_next = newbead->x_now + pull*cos(newbead->angle);
      newbead->y_next = newbead->y_now + pull*sin(newbead->angle);


      /* check to see whether new point is valid */
      if(pull > 0.0){ /* if being pulled toward (x2, y2), check to see if we're going beyond it */
	dx = newbead->x_next - newbead->x1;
	dy = newbead->y_next - newbead->y1;
	if((dx*dx + dy*dy) > newbead->distsq){
	  newbead->x_next = newbead->x2;
	  newbead->y_next = newbead->y2;
#ifdef RRB_DEBUG
	  printf("bead at %5.2f %5.2f fixed due to p2 limit : %d\n", newbead->x_next, newbead->y_next, improving);
	  printf("pull: %5.2f dx: %5.2f dy: %5.2f distsq: %5.2f\n", pull, dx, dy, newbead->distsq);
#endif
	}
      } /* pull > thresh */
      else if(pull < 0.0){ /* if being pulled toward (x1, y1), check to see if we're going beyond it */
	dx = newbead->x_next - newbead->x2;
	dy = newbead->y_next - newbead->y2;
	if((dx*dx + dy*dy) > newbead->distsq){
	  newbead->x_next = newbead->x1;
	  newbead->y_next = newbead->y1;
#ifdef RRB_DEBUG
	  printf("bead at %5.2f %5.2f fixed due to p1 limit : %d\n", newbead->x_next, newbead->y_next, improving);
	  printf("pull: %5.2f dx: %5.2f dy: %5.2f distsq: %5.2f\n", pull, dx, dy, newbead->distsq);
#endif
	}
      } /* pull < -thresh */

      /* for now, we're not going to worry about checking the entire path before setting now to next. */
      newbead->x_now = newbead->x_next;
      newbead->y_now = newbead->y_next;

      newbead = newbead->next;
    } /* loop for newbead */
    if(sanityCheck > 500){
      printf("cart_lib:improve_path2 -- tightening not making progress. Oscillation? \n");
      abort();
    }
    
    newbead = wirehead->next;

    
#ifdef RRB_DEBUG
    printf("improved path: \n");
    printf("  %5.2f %5.2f\n", wirehead->x_now, wirehead->y_now);
#endif

    NewLength = 0.0;
    while(newbead){
      dx = newbead->x_now - newbead->previous->x_now;
      dy = newbead->y_now - newbead->previous->y_now;
      NewLength += sqrt(dx*dx + dy*dy);
#ifdef RRB_DEBUG
      printf("  %5.2f %5.2f -- length %5.2f\n", newbead->x_now, newbead->y_now, sqrt(dx*dx + dy*dy)); 
#endif
      newbead = newbead->next;
    }
#ifdef RRB_DEBUG
    printf("Total length: %5.2f\n", NewLength); 
#endif

  oldpnode = path;
  newbead = wirehead;
  while(newbead){
    lastbead = newbead;
    oldpnode->x = newbead->x_now;
    oldpnode->y = newbead->y_now;
    newbead = newbead->next;
    oldpnode = oldpnode->next_point;
  }
#ifdef RRB_DEBUG
  display_path(path);
#endif


#ifdef RRB_DEBUG
    printf("press a key ");
    scanf("%s ", garb);
#endif

    if(sanityCheck > 50) if(OldNewLength - NewLength < 0.001) improving = 0;
    OldNewLength = NewLength;
  } /* while still improving */

  /* copy new path into path structures. */
  oldpnode = path;
  newbead = wirehead;
  while(newbead){
    lastbead = newbead;
    oldpnode->x = newbead->x_now;
    oldpnode->y = newbead->y_now;
    newbead = newbead->next;
    oldpnode = oldpnode->next_point;
    free(lastbead);
  }
}









///////////////////////////////////////////////////////////////////////
// $Log: cart_lib.c,v $
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
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  2003/04/06 15:40:33  endo
// gcc 3.1.1
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:39:45  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////



