/**********************************************************************
 **                                                                  **
 **                            gt_map.h                              **
 **                                                                  **
 **         Functions to deal with maps/photographic underlays.      **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_map.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_MAP_H
#define GT_MAP_H

extern "C" {
#include <X11/X.h>
#include <X11/Intrinsic.h>
}

typedef struct {

   int     photo_loaded;
   char   *photo_filename;

   Pixmap  photo_pixmap;
   int     photo_pixmap_width;
   int     photo_pixmap_height;

   double  photo_origin_x;       /* meters (easting of lower left corner)  */
   double  photo_origin_y;       /* meters (northing of lower left corner) */
   double  photo_width_meters;   /* meters (east-west)   */
   double  photo_height_meters;  /* meters (north-south) */

   int     photo_max_gray;
   int     photo_width_cells;    /* num columns of photo data */
   int     photo_height_cells;   /* num rows of photo data */
   unsigned char *photo_data;

} Map_info;


Map_info *create_map();

void delete_map(Map_info *map);


int load_map(Map_info *map,
	     const char *map_info_filename,
	     Widget window);

int rescale_photo_pixmap(Map_info *map,
			 Widget window);

#endif  /* GT_MAP_H */


/**********************************************************************
 * $Log: gt_map.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.5  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.4  1995/06/13  19:17:58  jmc
 * Added function prototype for rescale_photo_pixmap().
 *
 * Revision 1.3  1995/06/12  23:36:02  jmc
 * Added function prototype fro delete_map().
 *
 * Revision 1.2  1995/06/12  15:59:03  jmc
 * Renamed load_map_photo() to load_map() since the photo is only one
 * part of the map.
 *
 * Revision 1.1  1995/06/09  21:28:38  jmc
 * Initial revision
 **********************************************************************/
