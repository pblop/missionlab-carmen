%{
/**********************************************************************
 **                                                                  **
 **                         gt_overlay_yac.y                         **
 **                                                                  **
 **                                                                  **
 **        Parser for the overlay description language (odl)         **
 **                                                                  **
 **        See the MissionLab user's manual for details.             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  ** 
 **********************************************************************/

/* $Id: gt_overlay_yac.y,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
}

#include "gt_world.h"
#include "gt_scale.h"
#include "gt_measure.h"
#include "console.h"
#include "gt_console_db.h"
#include "3d_vis_wid.h"
#include "convert_scale.h"

extern int odl_lex();
extern void odl_error(char *);
extern void odl_errorf(const char *, ...);
int odl_parse();
double utmx, utmy;
int map_scale_factor = -1;
%}

%union {
           float  fval;
      OBJ_STYLES  style;
   gt_Point_list *coords;
            char *strval;
  double utm_coord[2];
}

%token SCENARIO SITE ORIGIN GLOBAL_REF_POINT MISSION_AREA LOAD_MAP CONTROL_MEASURES
%token SET_TOKEN ZOOM_FACTOR_TOKEN ATTRACTOR_TOKEN BASKET_TOKEN
%token AA_TOKEN ATK_TOKEN AXIS_TOKEN BOUNDARY_TOKEN BP_TOKEN GAP_TOKEN
%token LDLC_TOKEN OBJ_TOKEN OBSTACLE_TOKEN PL_TOKEN PP_TOKEN SP_TOKEN
%token UTM_TOKEN LAT_TOKEN LON_TOKEN WALL_TOKEN ROOM_TOKEN DOOR_TOKEN 
%token HALLWAY_TOKEN CENTERLINE_TOKEN WIDTH_TOKEN EXTENTS_TOKEN HEADING
%token WAYPOINT_FILE_TOKEN SCALE_TOKEN
%token HEIGHTMAP
%token <strval> QSTRING IDENTIFIER FILENAME
%token <fval> NUMBER OBJECT_TOKEN
%token <style> OBJ_STYLE

%type <strval> name
%type <coords> coordinate_pair coordinate_list
%type <utm_coord> utm_coordinate

%%

file:      info_part control_measure_part
         ;


info_part:
           info_statement
	 | info_part info_statement;  

info_statement:
           scenario_statement
         | site_statement
	 | origin_statement
	 | global_ref_pt_statement
         | heading_statement
	 | load_map_statement
         | heightmap_statement
	 | mission_area_statement
         | set_zoom_factor_statement
	 | scale_statement
         ;

scenario_statement:
           SCENARIO name
                {
		   if(gt_add_scenario_label($2) != GT_SUCCESS) {
		      odl_errorf("Unable to add SCENARIO %s", $2);
		      YYABORT;
		      }
		}
	 ;

site_statement:
           SITE name
                {
		   if (gt_add_site_name($2) != GT_SUCCESS) {
		      odl_errorf("Unable to add SITE %s", $2);
		      YYABORT;
		      }
		}
	 ;

origin_statement:
           ORIGIN NUMBER NUMBER
                {
		   origin_x = $2;
		   origin_y = $3;
		}
         | ORIGIN utm_coordinate
                {
		  origin_x = $2[0];
		  origin_y = $2[1];
		}
	   ;

global_ref_pt_statement:
	   GLOBAL_REF_POINT NUMBER NUMBER
               {
		 g_ref_latitude = $2;
		 g_ref_longitude = $3;
	       }
           ;

heading_statement:	   
	   HEADING NUMBER {origin_heading = $2;}
           ;

load_map_statement:
           LOAD_MAP name
                {
		   if (!load_map(gMapInfo, $2, drawing_area)) {
		      odl_errorf("Error: Unable to LOAD-MAP %s", $2);
		      YYABORT;
		      }
		}
         ;

heightmap_statement:
           HEIGHTMAP FILENAME NUMBER NUMBER NUMBER NUMBER
{ printf("Loading a height map: %s %d %d %d %d\n", $2, (int)$3, (int)$4, (int)$5, (int)$6);
  if (!load_heightmap($2, (int)$3, (int)$4, (int)$5, (int)$6)) {
  odl_errorf("Error: Unable to load a height map %s", $2);
  YYABORT;
}
};

scale_statement:
	SCALE_TOKEN NUMBER
{
	printf("Setting scale to: %d\n", (int)$2);
	map_scale_factor = (int)$2;
	set_meters_per_pixel(map_scale_factor / pixelsPerMeter);
	set_zoom_factor(1.0);
};



mission_area_statement:
           MISSION_AREA NUMBER NUMBER
                {
		   if ($2 < 0.0) {
		      odl_errorf("Mission-Area width must be positive (%1.8g)", $2);
		      YYABORT;
		      }
		   else if ($3 < 0.0) {
		      odl_errorf("Mission-Area height must be positive (%1.8g)", $3);
		      YYABORT;
		      }
		   else {
		      // The two lines below added by Nadeem to 
		      // use the same MISSION-AREA for lat/lon computations
		      map_width = $2; 
		      map_height = $3;
		      set_mission_area_size($2, $3);
		      }
		}
	 ;

set_zoom_factor_statement:
           SET_TOKEN ZOOM_FACTOR_TOKEN NUMBER
                {
		   if (!set_zoom_factor((double)$3/100.0)) {
		      odl_errorf("Error setting zoom factor (%1.8g)", $3);
		      YYABORT;
		      }
		}
         ;


control_measure_part:
           CONTROL_MEASURES control_measure_list
	 ;

control_measure_list:
           control_measure_statement
         | control_measure_list control_measure_statement
	 ;


control_measure_statement:
	   assembly_area_statement
	 | attack_position_statement
	 | axis_statement
	 | battle_position_statement
         | boundary_statement
	 | gap_statement
	 | ldlc_statement
	 | objective_statement
	 | obstacle_statement
	 | passage_point_statement
	 | attractor_statement
	 | object_statement
	 | basket_statement
	 | phase_line_statement
	 | starting_point_statement
         | wall_statement
	 | room_statement
         | door_statement
         | hallway_statement
	 | waypoint_file_statement
	 ;



assembly_area_statement:
           AA_TOKEN name coordinate_list
                { 
		   if(gt_add_assembly_area($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Assembly Area %s", $2);
		      YYABORT;
		      }
		}
         | AA_TOKEN name coordinate_list NUMBER
                { 
		   if(gt_add_assembly_area($2, $3, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Assembly Area %s", $2);
		      YYABORT;
		      }
		}
         ;

attack_position_statement:
           ATK_TOKEN name coordinate_list
                {
		   if(gt_add_attack_position($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Attack Point %s", $2);
		      YYABORT;
		      }
		}
         | ATK_TOKEN name coordinate_list NUMBER
                { 
		   if(gt_add_attack_position($2, $3, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Attack Point %s", $2);
		      YYABORT;
		      }
		}
	 ;

axis_statement:
           AXIS_TOKEN name coordinate_list
                {
		   if (gt_add_axis($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Axis %s", $2);
		      YYABORT;
		      }
		}
         | AXIS_TOKEN name coordinate_list NUMBER
                {
		   if (gt_add_axis($2, $3, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Axis %s", $2);
		      YYABORT;
		      }
		}
	 ;

battle_position_statement:
           BP_TOKEN name coordinate_list
                {
		   if (gt_add_battle_position($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Battle Position %s", $2);
		      YYABORT;
		      }
		}
         | BP_TOKEN name coordinate_list NUMBER
                {
		   if (gt_add_battle_position($2, $3, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Battle Position %s", $2);
		      YYABORT;
		      }
		}
	 ;

boundary_statement:
           BOUNDARY_TOKEN name coordinate_list
		{
		   if (gt_add_boundary($2, $3) != GT_SUCCESS) {
		      odl_errorf("Unable to add Boundary %s", $2);
		      YYABORT;
		      }
		}
	 ;

gap_statement:
           GAP_TOKEN name coordinate_pair coordinate_pair
                { 
		   $3->next = $4;
		   if (gt_add_gap($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Gap %s", $2);
		      YYABORT;
		      }
		}
         | GAP_TOKEN name coordinate_pair coordinate_pair NUMBER
                {
		   $3->next = $4;
		   if (gt_add_gap($2, $3, $5) != GT_SUCCESS) {
		      odl_errorf("Unable to add Gap %s", $2);
		      YYABORT;
		      }
		}
	 ;

ldlc_statement:
           LDLC_TOKEN name coordinate_list
                {
		   if (gt_add_ldlc($2, $3) != GT_SUCCESS) {
		      odl_errorf("Unable to add LD/LC %s", $2);
		      YYABORT;
		      }
		}
	 ;

objective_statement:
           OBJ_TOKEN name coordinate_list
                {
		   if (gt_add_objective($2, $3, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Objective %s", $2);
		      YYABORT;
		      }
		}
         | OBJ_TOKEN name coordinate_list NUMBER
                {
		   if (gt_add_objective($2, $3, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Objective %s", $2);
		      YYABORT;
		      }
		}
         ;

obstacle_statement:
           OBSTACLE_TOKEN coordinate_pair NUMBER
                {
		   if (gt_add_obstacle($2->x, $2->y, $3) !=  GT_SUCCESS) {
		      odl_errorf("Unable to add Obstacle (x=%1.9g, y=%1.9g, r=%1.9g)", 
				 $2->x, $2->y, $3);
		      YYABORT;
		      }
		}
            | OBSTACLE_TOKEN coordinate_pair NUMBER NUMBER 
               {
		 if (gt_add_obstacle($2->x, $2->y, $3, $4) !=  GT_SUCCESS) {
		   odl_errorf("Unable to add Obstacle (x=%1.9g, y=%1.9g, r=%1.9g, height = %1.9g)", 
			      $2->x, $2->y, $3, $4);
		   YYABORT;
		 }
               }
;

passage_point_statement:
           PP_TOKEN name coordinate_pair
                {  
		   if(gt_add_passage_point($2, $3->x, $3->y, -1.0) != GT_SUCCESS) {
		      odl_errorf("Unable to add Passage Point %s", $2);
		      YYABORT;
		      }
		}
         | PP_TOKEN name coordinate_pair NUMBER
                { 
		   if(gt_add_passage_point($2, $3->x, $3->y, $4) != GT_SUCCESS) {
		      odl_errorf("Unable to add Passage Point %s", $2);
		      YYABORT;
		      }
		}
	 ;

basket_statement:
           BASKET_TOKEN coordinate_pair
                {  
		   if(gt_add_basket($2->x, $2->y, -1) != GT_SUCCESS) {
		      odl_errorf("Unable to add Basket");
		      YYABORT;
		      }
		}
         | BASKET_TOKEN coordinate_pair NUMBER
                {  
		   if(gt_add_basket($2->x, $2->y, $3) != GT_SUCCESS) {
		      odl_errorf("Unable to add Basket");
		      YYABORT;
		      }
		}
	 ;


attractor_statement:
           ATTRACTOR_TOKEN coordinate_pair
                {  
		   if(gt_add_attractor($2->x, $2->y, -1) != GT_SUCCESS) {
		      odl_errorf("Unable to add Attractor");
		      YYABORT;
		      }
		}
         | ATTRACTOR_TOKEN coordinate_pair NUMBER
                {  
		   if(gt_add_attractor($2->x, $2->y, $3) != GT_SUCCESS) {
		      odl_errorf("Unable to add Attractor");
		      YYABORT;
		      }
		}
	 ;

object_statement: OBJECT_TOKEN coordinate_pair NUMBER OBJ_STYLE IDENTIFIER {  
      if(gt_add_object($2->x, $2->y, $3, $5, $4) != GT_SUCCESS) 
      {
         odl_errorf("Unable to create object");
	 YYABORT;
      }
   }
		| OBJECT_TOKEN coordinate_pair NUMBER OBJ_STYLE QSTRING {  
      if(gt_add_object($2->x, $2->y, $3, $5, $4) != GT_SUCCESS) 
      {
         odl_errorf("Unable to create object");
	 YYABORT;
      }
   }
	 ;


phase_line_statement:
           PL_TOKEN name coordinate_list
                {
		   if (gt_add_phase_line($2, $3) != GT_SUCCESS) {
		      odl_errorf("Unable to add Phase Line %s", $2);
		      YYABORT;
		      }
		}
	 ;


starting_point_statement:
           SP_TOKEN name coordinate_pair
                { 
		   if (gt_add_starting_point($2, $3->x, $3->y) != GT_SUCCESS) {
		      odl_errorf("Unable to add Starting Point %s", $2);
		      YYABORT;
		      }
		}
         ;


wall_statement:
	   WALL_TOKEN name coordinate_list
              {
		if (gt_add_wall($2, $3) != GT_SUCCESS) {
		  odl_errorf("Unable to add Wall %s", $2);
		  YYABORT;
		}
	      }
           |
	   WALL_TOKEN name coordinate_list NUMBER
              {
		if (gt_add_wall($2, $3, $4) != GT_SUCCESS) {
		  odl_errorf("Unable to add Wall %s", $2);
		  YYABORT;
		}
	      }
           ;
room_statement:
           ROOM_TOKEN name coordinate_list
              {
		if (gt_add_room($2, $3) != GT_SUCCESS) {
		  odl_errorf("Unable to add Room %s", $2);
		  YYABORT;
		}
	      }
           ;
door_statement:
           DOOR_TOKEN name coordinate_list
              {
		if (gt_add_door($2, $3) != GT_SUCCESS) {
		  odl_errorf("Unable to add Door %s", $2);
		  YYABORT;
		}
	      }
           ;

hallway_statement:
           HALLWAY_TOKEN name CENTERLINE_TOKEN coordinate_list WIDTH_TOKEN NUMBER EXTENTS_TOKEN coordinate_list 
              {
		if (gt_add_hallway($2, $4, $6, $8) != GT_SUCCESS) {
		  odl_errorf("Unable to add Hallway %s", $2);
		  YYABORT;
		}
	      }
           ;

waypoint_file_statement:
           WAYPOINT_FILE_TOKEN name 
              {
		if (gt_load_waypoint_file($2) != GT_SUCCESS)
		{
		  /* Keep parsing even if the file is not loaded. */
		}
	      }
           ;

coordinate_list:
           coordinate_pair
                {  $$ = $1; }
         | coordinate_list coordinate_pair
                {
		   gt_Point_list *pt = $1;
		   while (pt->next)
		      pt = pt->next;
		   pt->next = $2;
		   $$ = $1;
		}
	 ;


coordinate_pair:
		NUMBER NUMBER
   { 
     if (($$ = (gt_Point_list *)malloc(sizeof(gt_Point_list))) == NULL) {
       fprintf(stderr, "Ran out of memory creating coordinate pair (%f %f)",
	       $1, $2);
       odl_errorf("Ran out of memory creating coordinate pair (%f %f)",
		  $1, $2);
       YYABORT;
     }
     $$->x = $1 + origin_x;
     $$->y = $2 + origin_y;
     $$->next = 0;
   }
   | utm_coordinate {
     if (($$ = (gt_Point_list *)malloc(sizeof(gt_Point_list))) == NULL) {
       fprintf(stderr, "Ran out of memory creating coordinate pair");
       odl_errorf("Ran out of memory creating coordinate pair");
       YYABORT;
     }
     
     $$->x = $1[0] - origin_x;
     $$->y = $1[1] - origin_y;
     $$->next = 0;
   }
   ;

utm_coordinate: 
   UTM_TOKEN LAT_TOKEN NUMBER LON_TOKEN NUMBER {
     gt_convert_latlon_to_utm($3, $5, &utmx, &utmy);
     $$[0] = utmx;
     $$[1] = utmy;
   }
   | UTM_TOKEN LON_TOKEN NUMBER LAT_TOKEN NUMBER {
     gt_convert_latlon_to_utm($5, $3, &utmx, &utmy);
     $$[0] = utmx;
     $$[1] = utmy;
   }
   ;

name:      IDENTIFIER
         | QSTRING
         ;

%%


/**********************************************************************
 * $Log: gt_overlay_yac.y,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.7  2007/08/10 04:41:06  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.6  2007/03/12 06:04:07  nadeem
 * Modified code to remove the overloading and hence notational abuse of MISSION-AREA statement in the .ovl file
 *
 * Revision 1.5  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.4  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.3  2006/09/26 18:33:50  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.2  2006/09/20 18:34:48  nadeem
 * Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/07/11 10:07:49  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.5  2005/07/23 05:58:20  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/05/18 21:01:55  endo
 * Conflicts by previous mod resolved.
 *
 * Revision 1.3  2005/04/22 23:01:45  endo
 * Heightmap now accepts '*.*' as its name.
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.36  2000/07/07 18:16:52  endo
 * waypoint_file_statement added.
 *
 * Revision 1.35  2000/04/25 09:43:22  jdiaz
 * *** empty log message ***
 *
 * Revision 1.34  2000/02/07 05:14:26  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.33  1999/08/31 15:12:39  endo
 * *** empty log message ***
 *
 * Revision 1.32  1999/08/30 21:33:53  jdiaz
 * add utm_coordinate rules for UTM translations
 *
 * Revision 1.31  1997/02/14 15:49:30  zchen
 * *** empty log message ***
 *
 * Revision 1.30  1996/02/16  00:06:44  doug
 * *** empty log message ***
 *
 * Revision 1.29  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.28  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.27  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.26  1995/10/03  15:48:58  doug
 * attractor support added to control diameter
 *
 * Revision 1.25  1995/10/03  15:44:00  doug
 * added attractor statement
 *
 * Revision 1.24  1995/06/12  16:17:39  jmc
 * Added load_map_statement.
 *
 * Revision 1.23  1995/05/08  17:47:33  jmc
 * Changed old reference to mpp to zoom_factor.
 *
 * Revision 1.22  1995/05/06  00:43:28  jmc
 * Changed the grammar for SET METERS-PER-PIXEL to SET ZOOM-FACTOR.
 *
 * Revision 1.21  1995/05/04  22:05:11  jmc
 * Added include for gt_scale.h since all the scale related functions
 * moved to it.
 *
 * Revision 1.20  1995/05/02  21:21:16  jmc
 * Added grammar for SET METERS-PER-PIXEL statement.
 *
 * Revision 1.19  1995/05/02  19:49:49  jmc
 * Changed WORK-AREA to MISSION-AREA.
 *
 * Revision 1.18  1995/05/02  18:56:06  jmc
 * Added grammar for WORK-AREA statement.
 *
 * Revision 1.17  1995/04/20  18:18:31  jmc
 * Corrected spelling.
 *
 * Revision 1.16  1995/04/19  22:20:18  jmc
 * Added a fprintf(stderr, ...) message for running out of memory
 * since it may not be possible to pop up the warn_user interface at
 * that point.
 *
 * Revision 1.15  1995/04/14  21:25:52  jmc
 * Removed the defaults for the diameter/width of some control
 * measures (put those defaults in gt_console_db.c).  The gt_add_*
 * functions in gt_console_db.c know to use the default values by the
 * negative values specified in this file.
 *
 * Revision 1.14  1995/04/05  21:14:47  jmc
 * Added grammar for starting point.
 *
 * Revision 1.13  1995/04/05  20:36:15  jmc
 * Converted OBSTACLE rule to coordinate_pair (from NUMBER NUMBER) to
 * make it consistent with all the other rules.
 *
 * Revision 1.12  1995/04/04  20:54:03  jmc
 * Cosmetic changes: Reindented the statement rules to narrow them.
 * Alphabetized the statement rules.
 *
 * Revision 1.11  1995/04/03  20:28:05  jmc
 * Added copyright notice.
 *
 * Revision 1.10  1994/11/07  20:11:33  jmc
 * Small mods to make sure that everything uses the origin correctly.
 *
 * Revision 1.9  1994/11/07  19:56:13  jmc
 * Changed origin_x and origin_y to use globals now defined in
 * gt_world.c.
 *
 * Revision 1.8  1994/11/07  17:23:20  jmc
 * Implemented the grammar to add obstacles (OBSTACLE x y r).
 *
 * Revision 1.7  1994/10/25  20:23:23  jmc
 * Added syntax to optionally allow specifying the diameter of
 * passage points, attack positions, battle positions, and
 * objectives.
 *
 * Revision 1.6  1994/08/18  19:28:12  doug
 * Converted SUCCESS/FAILURE to GT_SUCCESS/GT_FAILURE
 *
 * Revision 1.5  1994/08/04  18:37:10  jmc
 * Added grammar for optional width on axis_statement and
 * gap_statement.
 *
 * Revision 1.4  1994/08/03  16:57:35  jmc
 * Added error trapping for errors in add_* functions calls using the
 * odl_error function and YYABORT.
 *
 * Revision 1.3  1994/07/12  19:22:27  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:34:45  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
