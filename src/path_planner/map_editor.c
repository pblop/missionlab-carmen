/**********************************************************************
 **                                                                  **
 **                               map_editor.c                       **
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

/* $Id: map_editor.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

				map_edit

	                 Used to create/edit maps

******************************************************************************/


/*--------------------------------------------------------------------------*/

/* preliminaries                   */

/*------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>

#include "vfc_lib.h"
#include "std.h"
#include "clipstruct.h"
#include "new_long_term_memory.h"
#include "map_display.h"
#include "cart_lib.h"
#include "map_bldr.h"
#include "window.h"
#include "map_editor.h"


/* Global Data Structures */
extern WIN_STRUCT *window;


int             concave_angle_mode;	/* 0 if most concave,1 if least, 2 if
					 * first */

int             convex_select_mode;	/* 1 is opposite, 2 is rightmost */



#define WINDOW_WIDTH            512
#define WINDOW_HEIGHT           512
#define MAX_POINTS             1000
#define MAX_DISTANCE_TO_POINT     5
#define MAX_DISTANCE_TO_LINE      5
#define TEXT_X                    0
#define TEXT_Y                  (WINDOW_WIDTH-16)


/* array to hold vertex points */
static XPoint   points[MAX_POINTS];
static int      num_points = 0;

// ENDO - gcc-3.4
//static WIN_STRUCT *window = 0;
WIN_STRUCT *window = 0;

/*----------------------------------------------------------------------------

				open_map_display

		opens the map window

-----------------------------------------------------------------------------*/

static void open_display(void)
{
   WIN_COLORS      colors[NUM_COLORS];

   /* init the colormap */
   colors[BLACK].r = 0;
   colors[BLACK].g = 0;
   colors[BLACK].b = 0;

   colors[WHITE].r = 65535;
   colors[WHITE].g = 65535;
   colors[WHITE].b = 65535;

   colors[RED].r = 65535;
   colors[RED].g = 0;
   colors[RED].b = 0;

   colors[GREEN].r = 0;
   colors[GREEN].g = 65535;
   colors[GREEN].b = 0;

   colors[BLUE].r = 0;
   colors[BLUE].g = 0;
   colors[BLUE].b = 65535;

   colors[COLOR_PATH].r = 5000;
   colors[COLOR_PATH].g = 5000;
   colors[COLOR_PATH].b = 65535;

   colors[COLOR_GROWN].r = 0;
   colors[COLOR_GROWN].g = 65535;
   colors[COLOR_GROWN].b = 1000;

   colors[COLOR_OBSTACLE].r = 60000;
   colors[COLOR_OBSTACLE].g = 0;
   colors[COLOR_OBSTACLE].b = 60000;

   /* open the display window */
   window = window_open("map display", WINDOW_WIDTH, WINDOW_HEIGHT, NUM_COLORS, 0,(char **)0,(unsigned long *)0,colors);
}

/*----------------------------------------------------------------------------

				close_map_display

		closes the map window

-----------------------------------------------------------------------------*/

static void close_display(void)
{
   /* close the display window */
   window_close(window);
   window = 0;
}


/****************************************************************************/
/* find closest point and return the distance to it in pixels               */
/****************************************************************************/

int
closest_point(int x, int y, int *point)
{
   int             i;
   int             min_dist;
   int             dist;
   int             min_num;

   min_num = -1;
   min_dist = PATH_PLAN_INFINITY;

   for (i = 0; i < num_points; i++)
   {
       dist = (int)(sqrt((double) (sqr(points[i].x - x) + sqr(points[i].y - y))) + 0.5);

      if (dist < min_dist)
      {
	 min_dist = dist;
	 min_num = i;
      }
   }

   if (min_dist > MAX_DISTANCE_TO_POINT)
      return FALSE;

   *point = min_num;

   return TRUE;
}

/****************************************************************************/
/* find closest line and return the distance to it in pixels                */
/****************************************************************************/

int
closest_line(int x, int y, int *line)
{
   int             i;
   int             min_dist;
   int             dist;
   int             min_num;
   double          Ux, Uy, Vx, Vy;
   double          length;
   double          h;

   min_num = -1;
   min_dist = PATH_PLAN_INFINITY;

   for (i = 0; i < num_points; i++)
   {
      /* create a unit vector along the line */
      Ux = points[i + 1].x - points[i].x;
      Uy = points[i + 1].y - points[i].y;
      length = sqrt(sqr(Ux) + sqr(Uy));
      Ux /= length;
      Uy /= length;

      /* create a vector to the cursor location */
      Vx = x - points[i].x;
      Vy = y - points[i].y;

      /* calculate the dot product, to see if is along the line segment */
      /* recall: dot product gives parallel component of resultant vector */
      h = Ux * Vx + Uy * Vy;

      if (h < 0 || h > length)
      {
	 /* The perpendicular from the point to the line does
	    not intersect the line between the two end points */
	 continue;
      }

      /* now calculate the distance via the cross product */
      /*recall cross product gives perpendicular component of resultant vector*/
      dist = (int)(fabs(Ux * Vy - Uy * Vx + 0.5));

      if (dist < min_dist)
      {
	 min_dist = dist;
	 min_num = i;
      }
   }

   if (min_dist > MAX_DISTANCE_TO_LINE)
   {
      return FALSE;
   }

   *line = min_num;

   return TRUE;
}

/****************************************************************************/
/* handle window events                                                     */
/****************************************************************************/

static void
handle_events(WIN_STRUCT * window)
{
   char            msg[80];
   int             i;
   int             xr, yr, x, y;
   unsigned int    kbstat; // ENDO - gcc 3.4
   int             point;
   int             line;
   int             drag;
   int             print_location;
   int             loc_x = 0, loc_y = 0;
   XPoint          dragging[3];
   XEvent          event;
   Window          rw, cw;

   XSetForeground(window->the_display, window->xor_gc, RED);
   XSetForeground(window->the_display, window->gc, BLUE);
   XSetForeground(window->the_display, window->text_gc, BLACK);
   XSetBackground(window->the_display, window->text_gc, WHITE);

   drag = FALSE;
   while (1)
   {
      print_location = FALSE;

      XNextEvent(window->the_display, &event);

      switch (event.type)
      {
	    /* Previously obsured window or part of window exposed */
	 case Expose:
	    if (num_points > 0)
	    {
	       points[num_points].x = points[0].x;
	       points[num_points].y = points[0].y;
	       XDrawLines(window->the_display, window->window, window->gc,
			  points, num_points + 1, CoordModeOrigin);
	    }
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
		  if (closest_point(x, y, &point))
		  {
		     /* then drag the point */
		     drag = TRUE;

		     /* setup a 2 line array for rubberbanding */
		     if (point > 0)
		     {
			dragging[0].x = points[point - 1].x;
			dragging[0].y = points[point - 1].y;
		     }
		     else
		     {
			dragging[0].x = points[num_points - 1].x;
			dragging[0].y = points[num_points - 1].y;
		     }

		     dragging[1].x = x;
		     dragging[1].y = y;

		     if (point < num_points - 1)
		     {
			dragging[2].x = points[point + 1].x;
			dragging[2].y = points[point + 1].y;
		     }
		     else
		     {
			dragging[2].x = points[0].x;
			dragging[2].y = points[0].y;
		     }
		     print_location = TRUE;
		     loc_x = x;
		     loc_y = y;
		  }
		  else if (closest_line(x, y, &line))
		  {
		     /* then insert a new point */
		     for (i = num_points; i > line; i--)
		     {
			points[i + 1] = points[i];
		     }
		     num_points++;
		     point = line + 1;

		     /* set dragging */
		     drag = TRUE;

		     /* setup a 2 line array for rubberbanding */
		     dragging[0].x = points[line].x;
		     dragging[0].y = points[line].y;

		     dragging[1].x = x;
		     dragging[1].y = y;

		     dragging[2].x = points[line + 1].x;
		     dragging[2].y = points[line + 1].y;
		     print_location = TRUE;
		     loc_x = x;
		     loc_y = y;
		  }

		  if (drag)
		  {
		     XDrawLines(window->the_display, window->window, window->xor_gc,
				dragging, 3, CoordModeOrigin);
		  }
		  break;

	       case Button2:
		  if (closest_point(x, y, &point))
		  {
		     print_location = TRUE;
		     loc_x = points[point].x;
		     loc_y = points[point].y;
		  }
		  break;
	       case Button3:
		  return;

	    }
	    break;

	 case ButtonRelease:
	    XQueryPointer(window->the_display, event.xmotion.window,
			  &rw, &cw, &xr, &yr, &x, &y, &kbstat);

	    if (drag)
	    {
	       /* set the new points */
	       points[point].x = x;
	       points[point].y = y;
	       if (point == 0)
	       {
		  points[num_points].x = points[0].x;
		  points[num_points].y = points[0].y;
	       }

	       /* clear the screen */
	       XClearWindow(window->the_display, window->window);

	       /* redraw the polygon */
	       XDrawLines(window->the_display, window->window, window->gc,
			  points, num_points + 1, CoordModeOrigin);

	       drag = FALSE;

	       print_location = TRUE;
	       loc_x = x;
	       loc_y = y;
	    }
	    break;

	 case MotionNotify:
	    XQueryPointer(window->the_display, event.xmotion.window,
			  &rw, &cw, &xr, &yr, &x, &y, &kbstat);

	    if (drag)
	    {

	       /* delete the old one */
	       XDrawLines(window->the_display, window->window, window->xor_gc,
			  dragging, 3, CoordModeOrigin);

	       /* update the point */
	       dragging[1].x = x;
	       dragging[1].y = y;

	       /* draw the new one */
	       XDrawLines(window->the_display, window->window, window->xor_gc,
			  dragging, 3, CoordModeOrigin);

	       print_location = TRUE;
	       loc_x = x;
	       loc_y = y;
	    }
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


/*---------------------------------------------------------------------------

				Main program

	Accepts a list of vertices corresponding to a wall
	constructs a region, then passes it to check convex
	Prints out results, and gusdisplays.


---------------------------------------------------------------------------*/

void
map_editor(char *map_file)
{
   unsigned long   event_mask;
   int             i;
   char            input[80];
   char            new_file[80];
   int             done;
   FILE           *file;
   double          x, y;

   /* optionally load file */
   done = FALSE;
   while (!done)
   {
      do
      {
	 printf("\nload map '%s'? [y/n]: ", map_file);
	 scanf("%s", input);
      } while (input[0] != 'y' && input[0] != 'Y' &&
	       input[0] != 'n' && input[0] != 'N');

      if (input[0] != 'y' && input[0] != 'Y')
      {
	 printf("Enter new map filename: ");
	 scanf("%s", new_file);

	 if ((file = fopen(new_file, "r")) == NULL)
	 {
	    perror("error opening file");
	    do
	    {
	       printf("\ncreate map '%s'? [y/n]: ", new_file);
	       scanf("%s", input);
	    } while (input[0] != 'y' && input[0] != 'Y' &&
		     input[0] != 'n' && input[0] != 'N');

	    if (input[0] == 'y' || input[0] == 'Y')
	    {
	       /*
	        * init the polygon to a triangle, easier than starting from
	        * nothing
	        */
	       points[0].x = 75;
	       points[0].y = 50;

	       points[1].x = 25;
	       points[1].y = 100;

	       points[2].x = 125;
	       points[2].y = 100;

	       num_points = 3;

	       done = TRUE;
	       strcpy(map_file, input);
	    }
	 }
	 else
	 {

	    fscanf(file, "%d", &num_points);
	    for (i = 0; i < num_points; i++)
	    {
	       fscanf(file, "%lf%lf", &x, &y);
	       points[i].x = (int)x;
	       points[i].y = (int)y;
	    }
	    fclose(file);
	 }
	 done = TRUE;
      }
      else
      {
	 if ((file = fopen(map_file, "r")) == NULL)
	 {
	    perror("error opening file");
	    do
	    {
	       printf("\ncreate map '%s'? [y/n]: ", map_file);
	       scanf("%s", input);
	    } while (input[0] != 'y' && input[0] != 'Y' &&
		     input[0] != 'n' && input[0] != 'N');

	    if (input[0] == 'y' || input[0] == 'Y')
	    {
	       /*
	        * init the polygon to a triangle, easier than starting from
	        * nothing
	        */
	       points[0].x = 75;
	       points[0].y = 50;

	       points[1].x = 25;
	       points[1].y = 100;

	       points[2].x = 125;
	       points[2].y = 100;

	       num_points = 3;

	       done = TRUE;
	    }
	 }
	 else
	 {

	    fscanf(file, "%d", &num_points);
	    for (i = 0; i < num_points; i++)
	    {
	       fscanf(file, "%lf%lf", &x, &y);
	       points[i].x = (int)x;
	       points[i].y = (int)y;
	    }
	    fclose(file);
	    done = TRUE;
	 }
      }
   }

   open_display();

   /* select which events we want to see */
   event_mask = ExposureMask | ButtonPressMask | PointerMotionHintMask;
   event_mask |= ButtonReleaseMask | OwnerGrabButtonMask | ButtonMotionMask;

   XSelectInput(window->the_display, window->window, event_mask);

   handle_events(window);

   close_display();

   /* optionally save file */
   done = FALSE;
   while (!done)
   {
      do
      {
	 printf("\nsave changes? [y/n]: ");
	 scanf("%s", input);
      } while (input[0] != 'y' && input[0] != 'Y' &&
	       input[0] != 'n' && input[0] != 'N');

      if (input[0] == 'y' || input[0] == 'Y')
      {
	 if ((file = fopen(map_file, "w")) == NULL)
	 {
	    perror("error opening file");
	    printf("Changes lost");
	    return;
	 }

	 fprintf(file, "%d\n", num_points);
	 for (i = 0; i < num_points; i++)
	 {
	    x = points[i].x;
	    y = points[i].y;
	    fprintf(file, "%f %f\n", x, y);
	 }
	 fclose(file);
	 done = TRUE;
      }
      else
      {
	 done = TRUE;
      }
   }
}



///////////////////////////////////////////////////////////////////////
// $Log: map_editor.c,v $
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
// Revision 1.1  2000/03/22 04:40:06  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

