/**********************************************************************
 **                                                                  **
 **                           map_display.c                          **
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

/* $Id: map_display.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/*-------------------------------------------------------------------------

				map_display.c

-------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "std.h"
#include "window.h"
#include "clipstruct.h"
#include "cart_lib.h"
#include "map_display.h"
#include "short_term_memory.h"


#define WINDOW_WIDTH	600
#define WINDOW_HEIGHT	600

static WIN_STRUCT *window = 0;

char *CTEXT[NUM_COLORS]= {"Black", "White", "Red", "Green", "Blue", "Yellow", "Cyan"};
unsigned long CNUM[NUM_COLORS];


/* RRB -- check out the current regions */
void step_display(struct stack_element *convex_region_stack)
{   
  struct stack_element       *the_stack_top = convex_region_stack;
  int                         stack_counter = 0;
  struct vertex_list_element *the_region;
  struct vertex_list_element *the_ptr;
  struct vertex_list_element *line2;
  int vle_ctr = 0;
  int             draw_color;
  char garb[120];

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

  while(the_stack_top){
    printf("\tstack element %d\n",stack_counter++);
    the_region = the_stack_top->region;
    vle_ctr = 0;
    the_ptr = the_region->next;

    window_clear(window); 
    window_flush(window); 

    line2 = the_ptr->next;
    do{
      printf(" %02d:\t %5.2f\t%5.2f\t%s\n", 
	     vle_ctr++, the_ptr->x, the_ptr->y, 
	     (the_ptr->passable?"PASS":"WALL"));
	
      if (the_ptr->passable != NULL) draw_color = COLOR_PASSABLE;
      else                           draw_color = COLOR_WALL;
      drawline(the_ptr->x, the_ptr->y, line2->x, line2->y, draw_color);

      the_ptr = line2;
      line2 = the_ptr->next;
    } while(the_ptr != the_region->next);

    window_flush(window); 
    printf("press a key ");
    scanf("%s ", garb);

    the_stack_top = the_stack_top->next;
  } // while the_stack_top
} // RRB

void display_single_region(struct vertex_list_element *vertex){
  struct vertex_list_element *the_ptr;
  struct vertex_list_element *line2;
  int vle_ctr = 0;
  int             draw_color;
  char garb[120];

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

    vle_ctr = 0;
    the_ptr = vertex->next;
  
    window_clear(window); 
    window_flush(window); 

    line2 = the_ptr->next;
    do{
      printf(" %02d:\t %5.2f\t%5.2f\t%s\n", 
	     vle_ctr++, the_ptr->x, the_ptr->y, 
	     (the_ptr->passable?"PASS":"WALL"));
	
      if (the_ptr->passable != NULL) draw_color = COLOR_PASSABLE;
      else                           draw_color = COLOR_WALL;
      drawline(the_ptr->x, the_ptr->y, line2->x, line2->y, draw_color);

      the_ptr = line2;
      line2 = the_ptr->next;
    } while(the_ptr != vertex->next);

    window_flush(window); 
    printf("press a key ");
    scanf("%s ", garb);
}





/*-----------------------------------------------------------------------------

				open_map_display

		opens the map window

------------------------------------------------------------------------------*/

void
open_map_display(void)
{
   /* open the display window */
   window = window_open("map display", WINDOW_WIDTH, WINDOW_HEIGHT, NUM_COLORS, TRUE, CTEXT, CNUM, (WIN_COLORS *)0);
}


/*-----------------------------------------------------------------------------

				close_map_display

		closes the map window

------------------------------------------------------------------------------*/

void
close_map_display(void)
{
   if (window != 0)
   {
      /* close the display window */
      window_close(window);
      window = 0;
   }
}

/*-----------------------------------------------------------------------------

				clear_map_display

		clears the map window

------------------------------------------------------------------------------*/

void
clear_map_display(void)
{
   window_clear(window);
}

/*-----------------------------------------------------------------------------

				draw_line

		draw a line in the display, scaled appropriatly

------------------------------------------------------------------------------*/

void
drawline(double x1, double y1, double x2, double y2, int draw_color)
{
 /* The original code was flipping the x coordinates for evey line drawn.
    This was extremely confusing especially when using the mouse to
    select start and end points for a path. Fixed by saho (june 26, 1999). 
 */
   window_drawline(window,x1,y1,x2,y2,draw_color);
}


/*-----------------------------------------------------------------------------

				draw_circle

		draw a circle in the display, scaled appropriatly

RRB added this
------------------------------------------------------------------------------*/

void
drawcircle(double x, double y, double r, int draw_color)
{
   window_drawcircle(window,x,y,r,draw_color);
}


/*----------------------------------------------------------------------------

				MAX_MIN_XY

		obtains maximum and minimum x and y values for a region

Returns 0 if there was a problem, else returns 1
Packs the max and min variables.

-----------------------------------------------------------------------------*/

static int
max_min_xy(struct vertex_list_element * pointer, double *max_x, double *max_y, double *min_x, double *min_y)

{
   struct vertex_list_element *start;

   *max_x = 0;
   *max_y = 0;
   *min_x = PATH_PLAN_INFINITY;
   *min_y = PATH_PLAN_INFINITY;

   /* If there are zero or one points, just return with maximal size. */
   if(!pointer) return(0);
   if(pointer->next == pointer) return(0);

   start = pointer;
   while (pointer->next != start)
   {
      if (pointer->x > *max_x)	 *max_x = pointer->x;
      if (pointer->y > *max_y)	 *max_y = pointer->y;
      if (pointer->x < *min_x)	 *min_x = pointer->x;
      if (pointer->y < *min_y)	 *min_y = pointer->y;
      if(!pointer->next) return(0);
      pointer = pointer->next;
   } 
   return(1);
}


/*-----------------------------------------------------------------------------

				display_regions

		Displays graphically the all regions in a stack

This is the one that gets called.
------------------------------------------------------------------------------*/

void
display_regions(struct stack_element *current_convex_region)
{

   struct vertex_list_element *line1, *line2, *start;
   int             draw_color;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   window_clear(window); 

   /* RRB -- hack hack... we want a little circle at the start point, but don't have the overlay. */
   drawcircle(g_StartX, g_StartY, 0.2, BLACK);
   


   while (current_convex_region != NULL)
   {				/* display all regions */

      line1 = current_convex_region->region->next;
      start = line1;
      line2 = line1->next;

      do
	{
	  if (line1->passable != NULL) draw_color = COLOR_PASSABLE;
	  else                         draw_color = COLOR_WALL;
	  
	  /* draw_color = COLOR_WALL;  */
	  if((DRAW_PASSABLES) || !line1->passable)
	    drawline(line1->x, line1->y, line2->x, line2->y, draw_color);

	  line1 = line2;
	  line2 = line2->next;
	} while (line1 != start);
      current_convex_region = current_convex_region->next;	/* get next region */
   } /* while current_convex_region != NULL */

   window_flush(window);
}


/*-----------------------------------------------------------------------------

				display_passages

		Displays graphically all region passages in a stack

------------------------------------------------------------------------------*/

void
display_passages(struct stack_element * current_convex_region)
{
   double          max_x, max_y, min_x, min_y;
   struct vertex_list_element *line1, *line2, *start;
   int             draw_color;
   struct stack_element *start_region;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   start_region = current_convex_region;	/* get first region */

   while (current_convex_region != NULL)
   {
      max_min_xy(current_convex_region->region->next, &max_x, &max_y, &min_x, &min_y);
      current_convex_region = current_convex_region->next;
   }

   current_convex_region = start_region;	/* restore to top */
   window_scale(window, min_x, max_x, min_y, max_y);

   while (current_convex_region != NULL)
   {				/* display all regions */
      /* display first line in a region */
      line1 = start = current_convex_region->region->next;
      line2 = line1->next;
      if (line1->passable != NULL)
      {
	 draw_color = COLOR_WALL;
	 drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
      }
      /* display remaining lines in the region */
      line1 = line2;
      line2 = line2->next;
      while (line1 != start)
      {
	 if (line1->passable != NULL)
	 {
	    draw_color = COLOR_WALL;
	    drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
	 }
	 line1 = line2;
	 line2 = line2->next;
      }
      current_convex_region = current_convex_region->next;	/* get next region */
   }
   window_flush(window);
}


/*--------------------------------------------------------------------------

				DISPLAY_GROWN

		Displays border and grown border

If border is NULL and grown isn't, there could be a problem with scale
of window.

--------------------------------------------------------------------------*/

void
display_grown(struct vertex_list_element * border,
	      struct vertex_list_element * grown)
{
  struct vertex_list_element *line1, *line2, *start;
  int             draw_color;

  if(border != NULL)
    {
      set_display_scale(border);

      /* display lines in the region */
      start = border->next;
      line1 = border->next;
      line2 = line1->next;

      do
	{
	  if (line1->passable != NULL) draw_color = COLOR_PASSABLE;
	  else                         draw_color = COLOR_WALL;

	  drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
	  line1 = line2;
	  line2 = line2->next;
	}      while(line1 != start);
    } /* if border != NULL */

  if (grown != NULL)
    {
      start = grown->next;
      line1 = grown->next;
      line2 = line1->next;

      do
	{
	  if (line1->passable != NULL) draw_color = COLOR_PASSABLE;
	  else                         draw_color = COLOR_GROWN;

	  drawline(line1->x, line1->y, line2->x, line2->y, draw_color);

	  if (line1->corresponding != NULL)
	    {
	      draw_color = COLOR_OBSTACLE;
	      drawline(line1->x, line1->y,
		       line1->corresponding->x, line1->corresponding->y, draw_color);
	    }
	  line1 = line2;
	  line2 = line2->next;
	}      while(line1 != start);
    } /* if grown != NULL */

  window_flush(window);
}


/*----------------------------------------------------------------------------

			DISPLAY_OBSTACLES

	Adds obstacles to current display

-----------------------------------------------------------------------------*/

void
display_obstacles(struct obstacle_list_element * obstacles)
{
   struct vertex_list_element *cv;
   struct obstacle_list_element *current;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   /* add obstacles to display */
   current = obstacles;
   while (current != NULL)
   {
      if (current->grown_obstacle != NULL)
      {
	 cv = current->grown_obstacle->next;
	 do
	 {
	    drawline(cv->x, cv->y, cv->next->x, cv->next->y, COLOR_OBSTACLE);

	    /* for corresponding */
	    if (cv->corresponding != NULL)
	    {
	       drawline(cv->x, cv->y, cv->corresponding->x,
			cv->corresponding->y, COLOR_OBSTACLE);
	    }
	    /* end corresponding */
	    cv = cv->next;
	 } while (cv != current->grown_obstacle->next);
      }
      current = current->next;
   }

   /* original obstacle */
   current = obstacles;
   while (current != NULL)
   {
      cv = current->actual_obstacle->next;
      do
      {
	 drawline(cv->x, cv->y, cv->next->x, cv->next->y, COLOR_OBSTACLE);
	 cv = cv->next;
      } while (cv != current->actual_obstacle->next);
      current = current->next;
   }

   window_flush(window);
}


/*---------------------------------------------------------------------------

			DISPLAY_OBSTACLES2

		Displays only obstacles with status 1

---------------------------------------------------------------------------*/


void
display_obstacles2(struct vertex_list_element * border,
		   struct obstacle_list_element * obstacles)
{
   double          max_x, max_y, min_x, min_y;
   struct vertex_list_element *line1, *line2, *start, *cv;
   struct obstacle_list_element *current;
   int             draw_color;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   max_min_xy(border->next, &max_x, &max_y, &min_x, &min_y);
   window_scale(window, min_x, max_x, min_y, max_y);

   /* display first line in a region */
   line1 = start = border->next;
   line2 = line1->next;
   draw_color = COLOR_WALL;
   drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
   /* display remaining lines in the region */
   line1 = line2;
   line2 = line2->next;
   while (line1 != start)
   {
      drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
      line1 = line2;
      line2 = line2->next;
   }

   /* obstacles */
   current = obstacles;
   while (current != NULL)
   {
      if (current->status == 1)
      {
	 cv = current->grown_obstacle->next;
	 do
	 {
	    draw_color = COLOR_OBSTACLE;

	    drawline(cv->x, cv->y, cv->next->x, cv->next->y, draw_color);
	    cv = cv->next;
	 } while (cv != current->grown_obstacle->next);
      }
      current = current->next;
   }
   current = obstacles;
   while (current != NULL)
   {
      if (current->status == 1)
      {
	 cv = current->actual_obstacle->next;
	 do
	 {
	    draw_color = COLOR_WALL;
	    drawline(cv->x, cv->y, cv->next->x, cv->next->y, draw_color);
	    cv = cv->next;
	 } while (cv != current->actual_obstacle->next);
      }
      current = current->next;
   }
   window_flush(window);
}


/*-----------------------------------------------------------------------------

				DISPLAY_PATH

		Displays graphically all the segments in the path

------------------------------------------------------------------------------*/

void
display_path(struct path_node * path)
{
   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   /* draw path */
   while (path->next_point != NULL)
   {
      drawline(path->x, path->y, path->next_point->x,
	       path->next_point->y, COLOR_PATH);
      path = path->next_point;
   }

   window_flush(window);
}


/*----------------------------------------------------------------------------

			DISPLAY_PATHONLY

	Displays path, border and obstacles, but no convex region lines

-----------------------------------------------------------------------------*/
void
display_pathonly(struct stack_element * current_convex_region,
		 struct path_node * path)

{
   int             draw_color;
   double          max_x, max_y, min_x, min_y;
   struct vertex_list_element *line1, *line2, *start;
   struct stack_element *start_region;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   printf("\n ** PATH LENGTH = %f  PATH COST = %f",
	  path_length(path), path_cost(path));

   start_region = current_convex_region;	/* get first region */

   while (current_convex_region != NULL)
   {
      max_min_xy(current_convex_region->region->next, &max_x, &max_y, &min_x, &min_y);
      current_convex_region = current_convex_region->next;
   }
   current_convex_region = start_region;	/* restore to top */
   window_scale(window, min_x, max_x, min_y, max_y);


   while (current_convex_region != NULL)
   {				/* display all regions */
      /* display first line in a region */
      line1 = start = current_convex_region->region->next;
      line2 = line1->next;
      draw_color = COLOR_WALL;
      if (line1->passable == NULL)
	 drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
      /* display remaining lines in the region */
      line1 = line2;
      line2 = line2->next;
      draw_color = COLOR_WALL;
      while (line1 != start)
      {
	 if (line1->passable == NULL)
	    drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
	 line1 = line2;
	 line2 = line2->next;
      }
      current_convex_region = current_convex_region->next;	/* get next region */
   }

   /* path stuff */
   draw_color = COLOR_PATH;
   while (path->next_point != NULL)
   {
      drawline(path->x, path->y, path->next_point->x,
	       path->next_point->y, draw_color);
      path = path->next_point;
   }
   window_flush(window);
}

/*-----------------------------------------------------------------------------

				DISPLAY_ID_REGION

		Displays graphically one region in the stack

------------------------------------------------------------------------------*/

void
display_id_region(struct stack_element * current_convex_region,
		  int region_number)
{
   double          max_x, max_y, min_x, min_y;
   struct vertex_list_element *line1, *line2, *start;
   int             draw_color, i;
   struct stack_element *start_region;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }


   start_region = current_convex_region;	/* get first region */

   while (current_convex_region != NULL)
   {
      max_min_xy(current_convex_region->region->next, &max_x, &max_y, &min_x, &min_y);
      current_convex_region = current_convex_region->next;
   }
   current_convex_region = start_region;	/* restore to top */
   window_scale(window, min_x, max_x, min_y, max_y);

   i = 1;
   while (current_convex_region != NULL)
   {				/* display all regions */
      /* display first line in a region */
      if (i == region_number)
      {
	 line1 = start = current_convex_region->region->next;
	 line2 = line1->next;
	 if (line1->passable == NULL)
	    draw_color = COLOR_WALL;
	 else
	    draw_color = COLOR_PASSABLE;
	 drawline(line1->x, line1->y, line2->x, line2->y, draw_color);

	 /* display remaining lines in the region */
	 line1 = line2;
	 line2 = line2->next;
	 while (line1 != start)
	 {
	    if (line1->passable == NULL)
	       draw_color = COLOR_WALL;
	    else
	       draw_color = COLOR_PASSABLE;
	    drawline(line1->x, line1->y, line2->x, line2->y, draw_color);
	    line1 = line2;
	    line2 = line2->next;
	 }
      }
      i++;
      current_convex_region = current_convex_region->next;	/* get next region */
   }
   window_flush(window);
}

/*--------------------------------------------------------------------------

				display_one_region

		Displays one region

--------------------------------------------------------------------------*/

void
display_one_region(struct vertex_list_element * region)
{
   struct vertex_list_element *line1, *line2;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   line1 = region->next;
   line2 = line1->next;

   do
   {
      drawline(line1->x, line1->y, line2->x, line2->y, COLOR_OBSTACLE);
      line1 = line2;
      line2 = line2->next;

   } while (line1 != region->next);

   window_flush(window);
}


/*--------------------------------------------------------------------------

				set_display_scale

		uses border to set the window scale

--------------------------------------------------------------------------*/

void
set_display_scale(struct vertex_list_element * border)
{
   double          max_x, max_y, min_x, min_y;

   /* if window isn't open, then just open it: eases debug */
   if (window == 0)
   {
      open_map_display();
   }

   if(!max_min_xy(border->next, &max_x, &max_y, &min_x, &min_y))
     {
       printf("map_display:set_display_scale() -- ill-formed ring of vertices.\n");
       exit(0);
     }

   window_scale(window, min_x, max_x, min_y, max_y);
}




/*----------------------------------------------------------------------------

		pick_xy

pick an x,y point with the cursor
if free_space then must be within a freespace region

returns	  0 if left button pressed (x,y selected)
        <>0 if escape pressed (abort)
-----------------------------------------------------------------------------*/

int
pick_xy(int free_space, double *map_x, double *map_y)
{
   char            msg[80];
   int             xr, yr, x, y;
   unsigned int    kbstat; // ENDO - gcc-3.4
   int             print_location;
   int             loc_x = 0, loc_y = 0;
   unsigned long   event_mask;
   XEvent          event;
   Window          rw, cw;
   struct vertex_list_element map_point;

#define TEXT_X                    0
#define TEXT_Y                  (window->width-16)

   /* select which events we want to see */
   event_mask = ExposureMask | ButtonPressMask;
   XSelectInput(window->the_display, window->window, event_mask);

   printf("Move mouse and press left button to select point or other button to quit\n");

   display_regions(long_term_memory.top_of_convex_region_stack);

   while (1)
   {
      print_location = FALSE;

      XNextEvent(window->the_display, &event);

      switch (event.type)
      {
	    /* Previously obsured window or part of window exposed */
	 case Expose:
	   display_regions(long_term_memory.top_of_convex_region_stack);
	    break;

	 case ButtonPress:
	    XQueryPointer(
            window->the_display,
            event.xmotion.window,
            &rw,
            &cw,
            &xr,
            &yr,
            &x,
            &y,
            &kbstat);

	    switch (event.xbutton.button)
	    {
	       case Button1:
		  /*
		   * left button pressed, convert x,y pixel to map
		   * coordinates
		   */
		  *map_x = inv_scale_x(x);
		  *map_y = inv_scale_y(y);

		  if (free_space)
		  {
		     /* check if X,Y location is in free space */
		     map_point.x = *map_x;
		     map_point.y = *map_y;
		     map_point.next = NULL;
		     map_point.passable = NULL;
		     if (find_meadow(&map_point, long_term_memory.top_of_convex_region_stack) == NULL)
		     {
			printf("\nLocation not in free space\n");
			printf("Move mouse and press left button to select point or other button to quit\n");
		     }
		     else
			return 0;
		  }
		  else
		     return 0;
		  break;

	       case Button2:
	       case Button3:
		  return TRUE;
		  break;
	    }

	    if (print_location)
	    {
	       /* then print the location of the point */
	       sprintf(msg, "x=%3d y=%3d", loc_x, loc_y);
	       XDrawImageString(window->the_display, window->window,
				window->text_gc, TEXT_X, TEXT_Y, msg, 11);
	    }
      }
   }





}


int mouse_read()
{
  int             xr, yr, x, y;
  unsigned int    kbstat; // ENDO - gcc-3.4
  unsigned long   event_mask;
  XEvent          event;
  Window          rw, cw;

  /* select which events we want to see */
  event_mask = ExposureMask | ButtonPressMask;
  XSelectInput(window->the_display, window->window, event_mask);

  while (1)
    {

      XNextEvent(window->the_display, &event);

      switch (event.type)
	{
	  /* Previously obsured window or part of window exposed */
	case Expose:
	  /*display_regions(long_term_memory.top_of_convex_region_stack); */
	  break;

	case ButtonPress:
	  XQueryPointer(
          window->the_display,
          event.xmotion.window,
          &rw,
          &cw,
          &xr,
          &yr,
          &x,
          &y,
          &kbstat);

	  switch (event.xbutton.button)
	    {
	    case Button1:
	      return FALSE;
	      break;
		  
	    case Button2:
	    case Button3:
	      return TRUE;
	      break;
	    }
	}
    }
}



///////////////////////////////////////////////////////////////////////
// $Log: map_display.c,v $
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
// Revision 1.1  2000/03/22 04:40:04  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

