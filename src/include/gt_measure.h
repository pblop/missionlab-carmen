/**********************************************************************
 **                                                                  **
 **                            gt_measure.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: gt_measure.h,v 1.1.1.1 2008/07/14 16:44:20 endo Exp $ */


#ifndef GT_MEASURE_H
#define GT_MEASURE_H

#include "gt_simulation.h"

typedef enum {
   NO_MEASURE,
   BOUNDARY,
   LDLC,
   ASSEMBLY_AREA,
   PASSAGE_POINT,
   GAP,
   ATTACK_POSITION,
   AXIS,
   PHASE_LINE,
   BATTLE_POSITION,
   STARTING_POINT,
   OBJECTIVE,
   WALL,
   DOOR,
   ROOM,
   HALLWAY
} gt_Measure_type;

#ifdef GT_MEASURE_C
char *gt_measure_type_name[] = {
   "No_Measure", "Boundary", "LDLC", "AA", "PP", 
   "Gap", "ATK", "Axis", "PL", "BP", "SP", "OBJ", "Wall",
   "Door", "Room", "Hallway"
};
#else
extern char *gt_measure_type_name[];
#endif

#define GT_NUM_MEASURE_TYPES 16


typedef enum {
   NO_MEASURE_DATA,
   POINT,
   POLYLINE,
   REGION
} gt_Measure_data_type;


typedef struct {
   unsigned num_points;
   gt_Point point[1];
} gt_Multipoint;               /* allocate big enough for the array of points */

typedef struct {
        gt_Measure_type type;
   gt_Measure_data_type data_type;
                    int draw_label;
                  float width;
                  double height;
          gt_Multipoint data;
} gt_Measure;


void gt_center_of_measure(gt_Measure *measure, gt_Point *point);

gt_Measure *gt_new_point_measure(gt_Measure_type type, 
				 float x, float y, float width);

gt_Measure *gt_new_multipoint_measure(gt_Measure_type type,
                                      gt_Measure_data_type data_type,
                                      float width,
                                      gt_Point_list *points,
                                      unsigned *size,
                                      double height = 1);


void gt_convert_latlon_to_utm(double lat, double lon, double *utmx, 
			      double *utmy); 


#endif  /* GT_MEASURE_H */



/**********************************************************************
 * $Log: gt_measure.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:20  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:48  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2005/03/23 07:36:53  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 22:59:46  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.10  2000/02/07 05:20:54  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.9  1999/08/30 21:26:30  jdiaz
 * add function declaration  for gt_convert_latlon_to_utm
 *
 * Revision 1.8  1995/03/31 22:07:13  jmc
 * Added a copyright notice.
 *
 * Revision 1.7  1995/02/14  22:09:03  jmc
 * Added draw_label field to gt_Measure to allow user to specify
 * whether or not the label for the measure should be drawn when the
 * measure is drawn.
 *
 * Revision 1.6  1994/10/25  15:20:32  jmc
 * Changed NO_DATA to NO_MEASURE_DATA because this was apparently
 * #define'd somewhere, although I could not track down exactly
 * #where.
 *
 * Revision 1.5  1994/08/04  18:35:57  jmc
 * Added width to gt_Measure (for Axis and Gap).  Added width to the
 * argument lists for gt_new_point_measure and
 * gt_new_multipoint_measure.
 *
 * Revision 1.4  1994/07/19  17:01:30  jmc
 * Modified to allow GT_MEASURE_C to determine whether the global strings will be
 * declared or defined here.  Moved string definition back into this file from
 * gt_measure.c.
 *
 * Revision 1.3  1994/07/19  16:32:38  jmc
 * Moved definition of char *gt_measure_type_name[] from the here to gt_measure.c
 * to avoid duplication in object modules.
 *
 * Revision 1.2  1994/07/14  13:20:08  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
