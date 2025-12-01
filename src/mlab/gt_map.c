/**********************************************************************
 **                                                                  **
 **                            gt_map.c                              **
 **                                                                  **
 **         Functions to deal with maps/photographic underlays.      **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: gt_map.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>

#include <X11/Xlib.h>
}

#include "file_utils.h"
#include "string_utils.h"
#include "convert_scale.h"
#include "gt_load_command.h"
#include "console.h"
#include "gt_console_db.h"
#include "gt_scale.h"
#include "gt_std.h"

#include "gt_map.h"

extern int debug;


/**********************************************************************
 **                                                                  **
 **                         local functions                          **
 **                                                                  **
 **********************************************************************/

/**********************************************************************
 **           initialize_map           **
 ****************************************/

static void initialize_map(Map_info *map)
{
   map->photo_loaded        = FALSE;
   map->photo_filename      = NULL;

   map->photo_pixmap        = 0;
   map->photo_pixmap_width  = 0;
   map->photo_pixmap_height = 0;

   map->photo_origin_x      = 0.0;
   map->photo_origin_y      = 0.0;
   map->photo_width_meters  = 0.0;
   map->photo_height_meters = 0.0;

   map->photo_max_gray      = 0;
   map->photo_width_cells   = 0;
   map->photo_height_cells  = 0;
   map->photo_data          = NULL;
}


/**********************************************************************
 **           load_map_info            **
 ****************************************/

static int load_map_info(Map_info *map,
			 const char *map_info_filename)
{
   /* Read the data from the map info file.  The map info files have the 
      following format:

          # Lines that begin with pound symbols are comments.
	  # A comment line identifying the map area is suggested.
	  # Blank lines are ignored.

	  photo-filename
	  photo-origin-x photo-origin-y photo-width photo-height

      where the "photo-filename" must be by itself on a single line and the
      photo location information is in meters starting on the following line.
   */

   FILE *info_file;
   char line[256];

   /* Open the info file */
   if ((info_file = fopen(map_info_filename, "r")) == NULL) {
      warn_userf("Error! Unable to open map info file '%s'.",
		 map_info_filename);
      return FALSE;
      }

   int finished = FALSE;
   int read_filename = FALSE;

   while (!finished && (feof(info_file) == 0)) {

      /* read the next line */
      if (fgets(line, 256, info_file) == NULL) {
	 warn_userf("Error reading map information from file '%s'.",
		    map_info_filename);
	 fclose(info_file);
	 return FALSE;
	 }

      /* Strip any coments and white space */
      remove_comments(line, '#');
      trim_white_space(line);

      /* Parse the line */
      if (!NO_STRING(line)) {
	 if (!read_filename) {
	    map->photo_filename = COPY_STRING(line);
	    read_filename = TRUE;
	 }
	 else {
	    float x, y, width, height, scale;
	    //double max_lat, max_lon;

	    if (sscanf(line, "%f%f%f%f%f", &x, &y, &width, &height, &scale) == 5)
	      {
		// We have 5 values on the line. So we have a scale factor
		// This means we are using geographic coordinates. 
		// Let's set the parameters for that first
		map_width = 
		  (scale/pixelsPerMeter)*width /* * cos(degToRad(g_ref_latitude))*/ ;
		map_height = (scale / pixelsPerMeter) * height;
		map_scale = pixelsPerMeter; 

		// We will keep the original pixel related values around, 
		// just in case...
		map_width_pixels = width;;
		map_height_pixels = height;
		map_scale_pixels = scale;
		
		// Now update the mission area again
		set_mission_area_size(map_width, map_height);		
		
		// Finally do the default stuff
		map->photo_origin_x      = x;
		map->photo_origin_y      = y;
		map->photo_width_meters  = map_width;
		map->photo_height_meters = map_height;
		
		finished = TRUE;
	      }
	    else {
	      if (sscanf(line, "%f%f%f%f", &x, &y, &width, &height) != 4)
		{
		  warn_userf("Error: Problem reading location information from \n\
       map information from file '%s':\n\
            %s\n\
       Dimensions [x y width height] should be on one line!",
			     map_info_filename, line);
		  fclose(info_file);
		  return FALSE;
		}
	      map->photo_origin_x      = x;
	      map->photo_origin_y      = y;
	      map->photo_width_meters  = width;
	      map->photo_height_meters = height;
	      
	      finished = TRUE;
	    }
	 }
      }
   }
		
      fclose(info_file);

      /* Check to make sure we read it okay. */
      if (!finished)
	return FALSE;

   return TRUE;
}


/**********************************************************************
 **        read_pgm_file_header        **
 ****************************************/

static int read_pgm_file_header(FILE *file,
				int *binary,
				int *width,
				int *height,
				int *max_gray)
{
   char magic_number[3];

   /* Parse the magic number */
   if (fread(magic_number, 1, 2, file) != 2) {
      warn_userf("Error reading magic number from PGM file");
      return FALSE;
      }
   magic_number[2] = '\0';

   if (string_equal(magic_number, "P2"))
      *binary = FALSE;
   else if (string_equal(magic_number, "P5"))
      *binary = TRUE;
   else {
      warn_userf("Erroneous magic number (%s) from PGM file", magic_number);
      return FALSE;
      }

   int field = 1;
   int line_num = 0;
   char line[256];

   // ENDO - MEM LEAK
   memset(line, 0x0, sizeof(char)*256); // Added by ENDO

   while ((field <= 3) && (feof(file) == 0)) {

      /* read the next line */
      line_num++;
      if (fgets(line, 256, file) == NULL) {
	 warn_userf("Error reading line %d of header of PGM file",
		    line_num);
	 return FALSE;
	 }
      remove_comments(line, '#');
      trim_white_space(line);


      if (!NO_STRING(line)) {

	 int i;
	 int num[4];
	 int count = sscanf(line, "%d%d%d", &(num[1]), &(num[2]), &(num[3]));

	 /* Assign what we've read to the appropriate variables */
	 for (i=1; i<=count; i++) {

	    switch (field) {
	    case 1:
	       *width = num[i];
	       field++;
	       break;
	    case 2:
	       *height = num[i];
	       field++;
	       break;
	    case 3:
	       *max_gray = num[i];
	       field++;
	       break;
	    default:
	       break;
	       }
	    }
	 }
      }

   if (field < 3)
      return FALSE;
   
   return TRUE;
}


/**********************************************************************
 **           read_pgm_data            **
 ****************************************/

static int read_pgm_data(FILE *file,
			 Map_info *map,
			 int binary)
{
   long size = map->photo_width_cells * map->photo_height_cells;

   /* if the map already has a data array, delete it */
   if (map->photo_data != NULL)
      free(map->photo_data);

   /* create a new array for the data */
   if ((map->photo_data = (unsigned char *)malloc(size)) == NULL) {
      fprintf(stderr, "Error: Unable to create photo data array for\n");
      fprintf(stderr, "       PGM image data, (%d x %d)\n",
	      map->photo_width_cells, map->photo_height_cells);
      return FALSE;
      }

   /* read the data into the array */
   if (binary) {
      /* Read it in RAW format */
      int count = fread(map->photo_data, 1, size, file);
      if (count != size) {
	 if (feof(file) != 0) {
	    warn_userf("Error reading PGM binary data: Unexpected EOF");
	    return FALSE;
	    }
	 else {
	    warn_userf("Error reading PGM binary data");
	    return FALSE;
	    }
	 }
      }
   else {
      /* Read it in ASCII format (and fix max_gray) */
      double max_gray = map->photo_max_gray;
      int i, num;
      for (i=0; i<size; i++) {
	 if (fscanf(file, "%d", &num) != 1) {
	    warn_userf("Error reading PGM ASCII data (data %d)", i);
	    return FALSE;
	    }
	 map->photo_data[i] = nint((double)(255.0 * num / max_gray));
	 if ((feof(file) != 0) && (i < size)) {
	    warn_userf("Error reading PGM ASCII data (data %d):\n\
      Unexpected EOF!", i);
	    return FALSE;
	    }
	 }
      map->photo_max_gray = 255;
      }

   return TRUE;
}


/**********************************************************************
 **       generate_photo_pixmap        **
 ****************************************/

static int generate_photo_pixmap(Map_info *map,
				 Widget window)
{
   Display *display   = XtDisplay(window);
   Drawable drawable  = RootWindowOfScreen(XtScreen(window));
   unsigned int depth = DefaultDepthOfScreen(XtScreen(window));

   /* If there is any existing photo pixmap, free it */
   if (map->photo_pixmap) {
      XFreePixmap(display, map->photo_pixmap);
      map->photo_pixmap = 0;
      map->photo_loaded = FALSE;
      }



   /* Create the new pixmap */
   unsigned int width  = nint(map->photo_width_meters / meters_per_pixel);
   unsigned int height = nint(map->photo_height_meters / meters_per_pixel);
   map->photo_pixmap = XCreatePixmap(display, drawable, width, height, depth);
   {  /* Make sure the pixmap got created */
      Window root;
      int x, y;
      unsigned int w, h, bw, d;

      Status status = XGetGeometry(display, map->photo_pixmap,
				   &root, &x, &y, &w, &h, &bw, &d);

      if ((status == 0) || (w != width) || (h != height)) {
	 warn_userf("Error creating a %dx%d pixmap for photo\n\
      (from file %s)", width, height, map->photo_filename);
	 return FALSE;
	 }
   }
   map->photo_pixmap_width = width;
   map->photo_pixmap_height = height;
   /* Now convert the photo data and write it into the pixmap */
   {
      XGCValues  xgcv;
      GC gc;
      long row, col, i, x, y, intensity;
      double max_gray = MAX_GRAY;
      double max_gray_photo = map->photo_max_gray;
 
      double cell_width_pixels  =
	 (map->photo_width_meters / map->photo_width_cells) /
	 meters_per_pixel;
      int cwp = nint(cell_width_pixels+0.5);

    
      double cell_height_pixels = 
	 (map->photo_height_meters / map->photo_height_cells) /
	 meters_per_pixel;
      int chp = nint(cell_height_pixels+0.5);

      if (debug) {
	 fprintf(stderr, "Scaling map photo: Cell ");
	 fprintf(stderr, "width=%1.5g, height=%1.5g pixels (drawn %dx%d)\n",
		 cell_width_pixels, cell_height_pixels, cwp, chp);
	 }

      /* Let the user know this is going to be slow */
      set_busy_cursor();

     
      /* write the cells to the pixmap */
      i = 0;
      for (row=0; row < map->photo_height_cells; row++) {
	 
	 y = nint((double)(row * cell_height_pixels));

	 if (debug && (row % 10 == 0)) 
	    fprintf(stderr, "Drawing photo pixmap row %d\n", row);

	 for (col=0; col < map->photo_width_cells; col++) {

	    x = nint((double)(col * cell_width_pixels));

	    /* Draw the rectangle for this cell */
	    intensity = nint(max_gray * ((double)map->photo_data[i] /
					 max_gray_photo));
	    i++;
	    xgcv.foreground = gray[intensity];
	    gc = XtGetGC(window, GCForeground, &xgcv);
	    XFillRectangle(display, map->photo_pixmap, gc, x, y, cwp, chp);
	    }
	 }
      reset_cursor();
   }

   map->photo_loaded = TRUE;    /* in case the pixmap was recreated */
   return TRUE;
}



/**********************************************************************
 **                                                                  **
 **                         global functions                         **
 **                                                                  **
 **********************************************************************/


/**********************************************************************
 **             create_map             **
 ****************************************/

Map_info *create_map()
{
   Map_info *map = (Map_info *)malloc(sizeof(Map_info));

   if (map == NULL)
      return NULL;

   initialize_map(map);

   return map;
}



/**********************************************************************
 **             delete_map             **
 ****************************************/

void delete_map(Map_info *map)

{
   /* free any old data before initializing */
   if (map->photo_filename) {
      free(map->photo_filename);
      map->photo_filename = NULL;
      }
   if (map->photo_data) {
      free(map->photo_data);
      map->photo_data = NULL;
      }

   initialize_map(map);
}



/**********************************************************************
 **           load_map           **
 ****************************************/

int load_map(Map_info *map,
	     const char *map_info_filename,
	     Widget window)
{
   char full_filename[256];

   if (monochrome) {
      warn_user("Error: Cannot load map underlay on monochrome display!");
      return TRUE;  /* Don't error out, just complain */
      }

   /* load the map photo information */
   if (find_full_filename(map_info_filename,
			  command_filename_directory, full_filename) == NULL) {
   	if(find_full_filename(map_info_filename, "/usr/src/overlays/", full_filename) == NULL)
   	{
      warn_userf("Error! Unable to find map info file\n       '%s'.",
		 map_info_filename);
      return FALSE;
   	}
      }
   if (!load_map_info(map, full_filename)) {
      warn_userf("Error loading map photo info file '%s'",
		 full_filename);
      return FALSE;
      }
   
   if (debug) {
      fprintf(stderr, "\nLoading map photo:\n");
      fprintf(stderr, "   filename: %s\n", map->photo_filename);
      fprintf(stderr, "   origin x: %1.8g meters\n", map->photo_origin_x);
      fprintf(stderr, "   origin y: %1.8g meters\n", map->photo_origin_y);
      fprintf(stderr, "      width: %1.8g meters\n", map->photo_width_meters);
      fprintf(stderr, "     height: %1.8g meters\n", map->photo_height_meters);
      fprintf(stderr, "\n");
      }
   
   /* See what kind of file the photo is in */
   char ext[40];
   extract_extension(map->photo_filename, ext);

   /* read the file */
   if (string_equal(ext, "pgm")) {

      int binary;
      FILE *photo_file;

      /* open the file */
      if (find_full_filename(map->photo_filename,
			     command_filename_directory, full_filename) == NULL) {
    	   	if(find_full_filename(map->photo_filename, "/usr/src/overlays/", full_filename) == NULL)
    	   	{
	 warn_userf("Error! Unable to map photo data file\n       '%s'.",
		    full_filename);
	 return FALSE;
    	   	}
	 }
      if ((photo_file = fopen(full_filename, "r")) == NULL) {
	 warn_userf("Error! Unable to open map photo file\n       '%s'.",
		    full_filename);
	 return FALSE;
	 }

      /* read the header */
      {
	 int w, h, max_gray;
	 if (!read_pgm_file_header(photo_file, &binary, &w, &h, &max_gray)) {
	    warn_userf("Error reading header in PGM map photo file '%s'.",
		       map->photo_filename);
	    return FALSE;
	    }
	 map->photo_width_cells = w;
	 map->photo_height_cells = h;
	 map->photo_max_gray = max_gray;
      }

      /* Apparently it isn't necessary to skip any remaining white space */

      /* read the data */
      if (!read_pgm_data(photo_file, map, binary)) {
	 warn_userf("Error reading data in PGM map photo file '%s'.",
		    map->photo_filename);
	 return FALSE;
	 }

      /* close the file */
      fclose(photo_file);

      /* copy/convert the grayscale image into the pixmap */
      if (!generate_photo_pixmap(map, window)) {
	 warn_userf("Error generating photo pixmap for file '%s'",
		    map->photo_filename);
	 return FALSE;
	 }

      }
   else {
      warn_userf("Error!  Don't know how to read map photo file '%s'",
		 map->photo_filename);
      return FALSE;
      }

   if (map->photo_loaded) {
      clear_map();
      gt_draw_db();
      }

   if (debug)
      fprintf(stderr, "Finished loading map photo\n");

   return TRUE;
}



/**********************************************************************
 **        rescale_photo_pixmap        **
 ****************************************/

int rescale_photo_pixmap(Map_info *map,
			 Widget window)
{
   if (map->photo_loaded)
      return generate_photo_pixmap(map, window);
   else
      return TRUE;
}






/**********************************************************************
 * $Log: gt_map.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.4  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.3  2007/03/12 06:04:07  nadeem
 * Modified code to remove the overloading and hence notational abuse of MISSION-AREA statement in the .ovl file
 *
 * Revision 1.2  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/11/08 17:14:41  endo
 * Memory leak problem solved.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.14  1999/02/22  18:09:47  xu
 * made the underlay photo display working on linux
 *
 * Revision 1.14  1999/02/15  05:42:50  
 * *** empty log message ***
 *
 * Revision 1.13  1999/02/15  05:42:50  xu
 * made the underlay photo displaying work on linux
 * by changing a few variables from type int to type long,
 * and moving i++ from nint(max_gray * ((double)map->photo_data[i++] /
 *					 max_gray_photo));
 * out to a seperate step, since nint is actually a macro that invokes i twice.
 *
 * Revision 1.12  1996/02/20  16:47:27  doug
 * made detect_objects work on the dennings
 *
 * Revision 1.11  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.10  1995/07/07  16:40:34  jmc
 * Minor hack to some comments.
 *
 * Revision 1.9  1995/07/05  18:01:21  jmc
 * Added a call to gt_draw_db() to the bottom of load_map() primarily
 * to refresh the mission name on top of the map.
 *
 * Revision 1.8  1995/06/16  17:13:39  jmc
 * Improved delete_map().
 *
 * Revision 1.7  1995/06/16  15:00:25  jmc
 * Deleted unused variables to eliminate compiler complaints
 * (with -Wall).
 *
 * Revision 1.6  1995/06/14  21:38:19  jmc
 * Modified load_map() to find the files in the
 * command_filename_directory, if it exists.
 *
 * Revision 1.5  1995/06/13  19:52:00  jmc
 * Added function rescale_photo_pixmap() for external access to
 * generate_photo_pixmap().  Moved setting of map->photo_loaded into
 * generate_photo_pixmap() so that recreating the pixmap doesn't
 * screw up rescale_photo_pixmap().
 *
 * Revision 1.4  1995/06/13  18:30:55  jmc
 * Modified generate_photo_pixmap() to show the busy cursor while it
 * is doing the compute intensive part.
 *
 * Revision 1.3  1995/06/12  23:57:20  jmc
 * Added a debug message.
 *
 * Revision 1.2  1995/06/12  23:41:55  jmc
 * Changed map_photo to map (eg, load_map_photo() is now load_map())
 * since photos are only a part of a map.  Separated initialization
 * part of create_map() into initialize_map().  Implemented
 * delete_map().
 *
 * Revision 1.1  1995/06/09  21:28:43  jmc
 * Initial revision
 **********************************************************************/
