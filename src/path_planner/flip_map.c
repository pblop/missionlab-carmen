/**********************************************************************
 **                                                                  **
 **                               flip_map.c                          **
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

/* $Id: flip_map.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

				flip_map

	                 Used to rotate maps

******************************************************************************/


/*--------------------------------------------------------------------------*/

/* preliminaries                   */

/*------------------------------------------------------------------*/
#include <stdio.h>
#include "std.h"

#define MAX_POINTS             10000

struct
{
   double          x;
   double          y;
}               points[MAX_POINTS];


/*---------------------------------------------------------------------------

				Main program

	Accepts a list of vertices corresponding to a wall
	constructs a region, then passes it to check convex
	Prints out results, and gusdisplays.


---------------------------------------------------------------------------*/

int
main(int argc, char *argv[])
{
   int             i;
   int             num_points = 0;
   double          x;
   double          y;
   double          min_x = PATH_PLAN_INFINITY;
   double          min_y = PATH_PLAN_INFINITY;
   double          max_x = 0;
   double          max_y = 0;
   double          t;
   double          width;
   double          height;
   int             flip_x;
   int             flip_y;
   int             flip_xy;
   int             flip_t;
   int             convert;
   int             cnt;
   int             do_meters;

   if (argc < 2)
   {
      fprintf(stderr, "Filter to flip maps. Reads from stdin and writes to stdout\n");
      fprintf(stderr, " -x to flip x axis (left to right)\n");
      fprintf(stderr, " -y to flip y axis (top to bottom)\n");
      fprintf(stderr, " -xy to flip x<->y (rotate 90 degrees)\n");
      fprintf(stderr, " -t to flip the file top-to-bottom (clockwise<->ccw)\n");
      fprintf(stderr, " -c convert map to 'C' floating point array\n");
      fprintf(stderr, " -x to flip x axis (left to right)\n");
      fprintf(stderr, " -m to convert meters to feet\n");
   }

   flip_x = FALSE;
   flip_y = FALSE;
   flip_xy = FALSE;
   flip_t = FALSE;
   do_meters = FALSE;
   convert = FALSE;

   for (i = 1; i < argc; i++)
   {
      if (strcmp(argv[i], "-x") == 0 || strcmp(argv[i], "-X") == 0)
	 flip_x = TRUE;
      if (strcmp(argv[i], "-y") == 0 || strcmp(argv[i], "-Y") == 0)
	 flip_y = TRUE;
      if (strcmp(argv[i], "-xy") == 0 || strcmp(argv[i], "-XY") == 0)
	 flip_xy = TRUE;
      if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "-T") == 0)
	 flip_t = TRUE;
      if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "-C") == 0)
	 convert = TRUE;
      if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "-M") == 0)
	 do_meters = TRUE;
   }

   if (convert)
   {
      /* printf("float MAP_DATA[%d]={\n",num_points*2+1); */
      printf("float MAP_DATA[]={");
   }

   cnt = 0;
   while (1)
   {
      num_points = 0;
      scanf("%d", &num_points);
      if (num_points == 0)
	 break;

      for (i = 0; i < num_points; i++)
      {
	 scanf("%lf%lf", &x, &y);
	 points[i].x = x;
	 points[i].y = y;

	 if (cnt == 0)
	 {
	    if (x > max_x)
	       max_x = x;
	    if (x < min_x)
	       min_x = x;
	    if (y > max_y)
	       max_y = y;
	    if (y < min_y)
	       min_y = y;
	 }
      }

      if (cnt == 0)
      {
	 width = max_x - min_x + 1;
	 height = max_y - min_y + 1;
      }

      printf("\n%d%s\n", num_points, convert ? "," : "");
      for (i = 0; i < num_points; i++)
      {
	 if (flip_t)
	 {
	    x = points[num_points - i - 1].x;
	    y = points[num_points - i - 1].y;
	 }
	 else
	 {
	    x = points[i].x;
	    y = points[i].y;
	 }

         if(do_meters)
         {
            x *= 3.28084;
            y *= 3.28084;
         }

	 if (flip_x)
	    x = max_x - x + min_x;
	 if (flip_y)
	    y = max_y - y + min_y;
	 if (flip_xy)
	 {
	    t = x;
	    x = y;
	    y = t;
	 }

	 printf("%f%c\t%f%s", x, convert ? ',' : ' ', y, convert ? "," : "");
	 putchar('\n');
      }
      cnt++;
   }
   if (convert)
      printf("\n0 };\n");

   return 0;
}



///////////////////////////////////////////////////////////////////////
// $Log: flip_map.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:39:59  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

