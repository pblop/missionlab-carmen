/************************************************************************/
%{

/**********************************************************************
 **                                                                  **
 **  env_parser.y                                                    **
 **                                                                  **
 **  bison parser for the environment files                          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **                                                                  **
 **********************************************************************/

/* $Id: env_parser.y,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: env_parser.y,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.24  2004/07/30 13:47:57  doug
* back from USC
*
* Revision 1.23  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.22  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.21  2004/03/01 23:37:48  doug
* working on real overlay
*
* Revision 1.20  2003/03/13 22:42:15  doug
* seems to work
*
* Revision 1.19  2002/11/20 22:17:28  doug
* can assign loiter areas in 2d
*
* Revision 1.18  2002/11/14 22:34:15  doug
* working on photo loading
*
* Revision 1.17  2002/11/14 15:25:09  doug
* snapshot
*
* Revision 1.16  2002/11/13 18:47:09  doug
* added boundary
*
* Revision 1.15  2002/11/13 18:18:39  doug
* added passagePoint
*
* Revision 1.14  2002/11/13 18:12:50  doug
* added battlePosition
*
* Revision 1.13  2002/11/13 17:52:09  doug
* added AttackPosition
*
* Revision 1.12  2002/11/13 17:40:44  doug
* assemblyArea added
*
* Revision 1.11  2002/11/13 17:15:50  doug
* objectives work!
*
* Revision 1.10  2002/11/08 22:03:48  doug
* 3d circles work
*
* Revision 1.9  2002/11/06 22:14:45  doug
* runs a bit
*
* Revision 1.8  2002/11/06 19:12:01  doug
* compiles!
*
* Revision 1.7  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.6  2002/10/23 19:39:25  doug
* stringstreams changed from gcc 2.9 to 3.0
*
* Revision 1.5  2002/10/02 15:51:48  doug
* moving to cvs
*
* Revision 1.4  2001/08/15 18:41:15  doug
* commented out code to load photo
*
* Revision 1.3  1999/03/10 00:22:28  doug
* moved extra to here from env_object
*
* Revision 1.2  1998/06/16 18:09:43  doug
* *** empty log message ***
*
* Revision 1.1  1998/06/16 14:46:40  doug
* Initial revision
*
**********************************************************************/

/************************************************************************/
 
#include <stdarg.h>
#include "mic.h"
#include "Vector.h" 
#include "env_scanner.h" 
#include "env_parser.h"
#include "disk.h" 
#include "environment.h" 
#include "polygon.h" 
#include "assemblyArea.h"
#include "loiterArea.h"
#include "attackPosition.h"
#include "axis.h"
#include "battlePosition.h"
#include "boundary.h"
#include "line.h"
#include "ldlc.h"
#include "passagePoint.h"
#include "gap.h"
#include "object.h"
#include "phaseLine.h"
#include "objective.h"
#include "circle.h"
#include "wall.h"
#include "tile.h"
#include "photo.h"
   
using namespace std;
using namespace sara;		// make all sara names visible in this file
void env_error(char *s);

bool env_had_error = false;
const string *_env_filename;
bool env_verbose = false;
class sara::environment *theEnv = NULL;

/************************************************************************/
// env_error is called by yacc when it finds an error.
void env_error(char *s)
{
   env_SyntaxError(s);
}

/********************************************************************/
// env_SyntaxError prints an error message
void env_SyntaxError(const char *msg)
{
   ERROR("Syntax error loading environment file %s, line %d: %s", 
		_env_filename->c_str(), env_lineno, msg);

   env_had_error = true;
}

/********************************************************************/
// env_SyntaxErrorf prints an error message with printf-style parameters
void env_SyntaxErrorf(const char *fmt, ...)
{
   va_list args;
   char err[512];

   va_start(args, fmt);
   vsprintf(err, fmt, args);
   va_end(args);
   env_SyntaxError(err);
}

// *******************************************************************
static const double DEFAULT_objective_DIA = 50;
static const double DEFAULT_assemblyArea_DIA = 50;
static const double DEFAULT_loiterArea_DIA = 500;
static const double DEFAULT_attackPosition_DIA = 50;
static const double DEFAULT_battlePosition_DIA = 50;
static const double DEFAULT_passagePoint_DIA = 50;

// *******************************************************************

%}

%union {
   std::string    *str;
   sara::strings  *strs;
   double          number;
   sara::Vector   *point;
   sara::Vector   *utmPoint;
   sara::Vectors  *points;
   OBJ_STYLES      style;
}

/* Define the tokens that lex will return */
%token DISK  
%token POLYGON  
%token AT 
%token RADIUS 
%token COLOR 
%token PHOTO  
%token METERSPERPIXEL  

%token <str> STRING 
%type <strs> Label
%token <number> NUMBER OBJECT_TOKEN


%token SCENARIO SITE ORIGIN MISSION_AREA LOAD_MAP CONTROL_MEASURES
%token SET_TOKEN ATTRACTOR_TOKEN BASKET_TOKEN
%token AA_TOKEN LA_TOKEN ATK_TOKEN AXIS_TOKEN BOUNDARY_TOKEN BP_TOKEN 
%token GAP_TOKEN
%token LDLC_TOKEN OBJ_TOKEN OBSTACLE_TOKEN PL_TOKEN PP_TOKEN SP_TOKEN
%token UTM_TOKEN LAT_TOKEN LON_TOKEN WALL_TOKEN ROOM_TOKEN DOOR_TOKEN 
%token HALLWAY_TOKEN CENTERLINE_TOKEN WIDTH_TOKEN EXTENTS_TOKEN HEADING
%token WAYPOINT_FILE_TOKEN
%token <style> OBJ_STYLE

%type <point>  coordinate
%type <points> coordinate_list
%type <utmPoint> utm_coordinate

/************************** Start of Grammer *****************************/
%%

Start:     info_part control_measure_part
	 | control_measure_part
         ;

info_part:
           info_statement
	 | info_part info_statement;  

info_statement:
           scenario_statement
         | site_statement
	 | origin_statement
         | heading_statement
	 | load_map_statement
	 | mission_area_statement
         ;

scenario_statement:
           SCENARIO STRING
                {
                   // Set it.
                   theEnv->scenario_name = *$2;
		}
	 ;

site_statement:
           SITE STRING
                {
                   // Set it.
                   theEnv->site_name = *$2;
		}
	 ;

origin_statement:
           ORIGIN NUMBER NUMBER
                {
		   theEnv->origin_x = $2;
		   theEnv->origin_y = $3;
		}
         | ORIGIN utm_coordinate
                {
		  theEnv->origin_x = $2->x;
		  theEnv->origin_y = $2->y;
		}
	   ;

heading_statement:	   
	   HEADING NUMBER 
             	{
		   theEnv->origin_heading = $2;
		}
           ;

load_map_statement:
           	LOAD_MAP STRING
           	{
		   theEnv->addObject( new photo(new tile(), *$2) );
		   if( env_verbose )
	   	      INFORM("\t\tPhoto %s", $2);
		}
		| PHOTO STRING AT NUMBER ',' NUMBER METERSPERPIXEL '=' NUMBER
		{
		   theEnv->addObject( new photo(new tile(), *$2, Vector($4,$6), $9) );
		   if( env_verbose )
	   	      INFORM("\t\tPhoto %s at <%.2f,%.2f>, MetersPerPixel= %.2f", $2, $4, $6, $9);
		}
         ;

mission_area_statement:
           MISSION_AREA NUMBER NUMBER
                {
		   if ($2 < 0.0) 
                   {
		      env_SyntaxErrorf("Mission-Area width must be positive (%1.8g)", $2);
		      YYABORT;
		   }
		   else if ($3 < 0.0) 
                   {
		      env_SyntaxErrorf("Mission-Area height must be positive (%1.8g)", $3);
		      YYABORT;
		   }
		   else 
                   {
		      theEnv->width = $2;
		      theEnv->height = $2;
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
	   objective_statement
	 | assembly_area_statement
	 | loiter_area_statement
	 | attack_position_statement
	 | battle_position_statement
	 | passage_point_statement
         | boundary_statement
         | wall_statement 
	 | gap_statement
/*
	 | axis_statement
	 | ldlc_statement
	 | phase_line_statement
	 | object_statement
	 | obstacle_statement
	 | attractor_statement
	 | basket_statement
	 | starting_point_statement
	 | room_statement
         | door_statement
         | hallway_statement
	 | waypoint_file_statement
*/
	 ;

objective_statement:
           OBJ_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new objective( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new objective( new circle((*$3)[0], DEFAULT_objective_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | OBJ_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new objective( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new objective( new circle((*$3)[0], DEFAULT_objective_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | OBJ_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "Objective has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new objective( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | OBJ_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "Objective has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new objective( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 ;

assembly_area_statement:
	   AA_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new assemblyArea( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new assemblyArea( new circle((*$3)[0], DEFAULT_assemblyArea_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | AA_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new assemblyArea( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new assemblyArea( new circle((*$3)[0], DEFAULT_assemblyArea_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | AA_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "assemblyArea has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new assemblyArea( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | AA_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "assemblyArea has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new assemblyArea( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 ;

loiter_area_statement:
	   LA_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new loiterArea( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new loiterArea( new circle((*$3)[0], DEFAULT_loiterArea_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | LA_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new loiterArea( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new loiterArea( new circle((*$3)[0], DEFAULT_loiterArea_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | LA_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "loiterArea has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new loiterArea( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | LA_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "loiterArea has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new loiterArea( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 ;

attack_position_statement:
	   ATK_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new attackPosition( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new attackPosition( new circle((*$3)[0], DEFAULT_attackPosition_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | ATK_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new attackPosition( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new attackPosition( new circle((*$3)[0], DEFAULT_attackPosition_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | ATK_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "attackPosition has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new attackPosition( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | ATK_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "attackPosition has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new attackPosition( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 ;

battle_position_statement:
	   BP_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new battlePosition( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new battlePosition( new circle((*$3)[0], DEFAULT_battlePosition_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | BP_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new battlePosition( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new battlePosition( new circle((*$3)[0], DEFAULT_battlePosition_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | BP_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "battlePosition has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new battlePosition( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | BP_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "battlePosition has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new battlePosition( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 ;

passage_point_statement:
	   PP_TOKEN STRING coordinate_list
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new passagePoint( new polygon(*$3), new strings(*$2)) );
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$3)[0], DEFAULT_passagePoint_DIA / 2.0), new strings(*$2)) );
                   }
		}
         | PP_TOKEN STRING coordinate_list STRING
                {
                   if( $3->size() >= 2 )
		      theEnv->addObject( new passagePoint( new polygon(*$3), new strings(*$2), *$4) );
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$3)[0], DEFAULT_passagePoint_DIA / 2.0), new strings(*$2), *$4) );
                   }
		}
         | PP_TOKEN STRING coordinate_list NUMBER
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "passagePoint has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$3)[0], $4 / 2.0), new strings(*$2)) );
                   }
		}
         | PP_TOKEN STRING coordinate_list NUMBER STRING
                { 
                   if( $3->size() != 1 )
                   {
                      stringstream out;
                      out << "passagePoint has " << $3->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$3)[0], $4 / 2.0), new strings(*$2), *$5) );
                   }
		}
	 | PP_TOKEN coordinate_list
                {
                   if( $2->size() >= 2 )
		      theEnv->addObject( new passagePoint( new polygon(*$2)) );
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$2)[0], DEFAULT_passagePoint_DIA / 2.0)) );
                   }
		}
         | PP_TOKEN coordinate_list STRING
                {
                   if( $2->size() >= 2 )
		      theEnv->addObject( new passagePoint( new polygon(*$2), *$3) );
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$2)[0], DEFAULT_passagePoint_DIA / 2.0), *$3) );
                   }
		}
         | PP_TOKEN coordinate_list NUMBER
                { 
                   if( $2->size() != 1 )
                   {
                      stringstream out;
                      out << "passagePoint has " << $2->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$2)[0], $3 / 2.0)) );
                   }
		}
         | PP_TOKEN coordinate_list NUMBER STRING
                { 
                   if( $2->size() != 1 )
                   {
                      stringstream out;
                      out << "passagePoint has " << $2->size() << " points and a diameter!" << endl;
                      ERROR(out.str().c_str());
                   }
                   else
                   {
		      theEnv->addObject( new passagePoint( new circle((*$2)[0], $3 / 2.0), *$4) );
                   }
		}
	 ;

boundary_statement:
           BOUNDARY_TOKEN Label coordinate_list
		{
		   if( $3->size() < 2 )
                   {
                      env_SyntaxError("Boundaries must contain at least two points");
                   }
                   else
                   {
		      theEnv->addObject( new boundary( new line(*$3), $2) );
                   }
		}
         | BOUNDARY_TOKEN Label coordinate_list STRING
		{
		   if( $3->size() < 2 )
                   {
                      env_SyntaxError("Boundaries must contain at least two points");
                   }
                   else
                   {
		      theEnv->addObject( new boundary( new line(*$3), $2, *$4));
                   }
		}
	 ;

wall_statement:
	   WALL_TOKEN Label coordinate_list
              	{
		   if( $3->size() < 2 )
                   {
                      env_SyntaxError("Walls must contain at least two points");
                   }
                   else
                   {
		      theEnv->addObject( new wall( new line(*$3), $2) );
                   }
	      	}
         | WALL_TOKEN Label coordinate_list STRING
		{
		   if( $3->size() < 2 )
                   {
                      env_SyntaxError("Walls must contain at least two points");
                   }
                   else
                   {
		      theEnv->addObject( new wall( new line(*$3), $2, *$4));
                   }
		}
           ;

gap_statement:
           GAP_TOKEN Label coordinate coordinate
                { 
		   theEnv->addObject( new gap( gap::makeGap(*$3, *$4), $2) );
		}
         | GAP_TOKEN Label coordinate coordinate STRING
                { 
		   theEnv->addObject( new gap( gap::makeGap(*$3, *$4), $2, *$5) );
		}
         | GAP_TOKEN Label coordinate coordinate NUMBER
                {
		   theEnv->addObject( new gap( gap::makeGap(*$3, *$4, $5), $2) );
		}
         | GAP_TOKEN Label coordinate coordinate NUMBER STRING
                {
		   theEnv->addObject( new gap( gap::makeGap(*$3, *$4, $5), $2, *$6) );
		}
	 ;

/*
ldlc_statement:
           LDLC_TOKEN STRING coordinate_list
                {
		   theEnv->addObject( new ldlc(*$2, *$3, "black") );
		}
	 ;

phase_line_statement:
           PL_TOKEN STRING coordinate_list
                {
		   theEnv->addObject( new phaseLine(*$2, *$3, "black") );
		}
	 ;

object_statement: OBJECT_TOKEN coordinate NUMBER OBJ_STYLE IDENTIFIER {  
		      theEnv->addObject( new object(*$2, $3, *$5, $4==STYLE_MOVABLE, $4==STYLE_CONTAINER) );
   		}
		| OBJECT_TOKEN coordinate NUMBER OBJ_STYLE STRING {  
		      theEnv->addObject( new object(*$2, $3, *$5, $4==STYLE_MOVABLE, $4==STYLE_CONTAINER));
   		}
	 	;

obstacle_statement:
           OBSTACLE_TOKEN coordinate NUMBER
                {
		   if (gt_add_obstacle($2->x, $2->y, $3) !=  GT_SUCCESS) {
		      env_SyntaxError("Unable to add Obstacle (x=%1.9g, y=%1.9g, r=%1.9g)", 
				 $2->x, $2->y, $3);
		      YYABORT;
		      }
		}
         ;

basket_statement:
           BASKET_TOKEN coordinate
                {  
		   if(gt_add_basket($2->x, $2->y, -1) != GT_SUCCESS) {
		      env_SyntaxError("Unable to add Basket");
		      YYABORT;
		      }
		}
         | BASKET_TOKEN coordinate NUMBER
                {  
		   if(gt_add_basket($2->x, $2->y, $3) != GT_SUCCESS) {
		      env_SyntaxError("Unable to add Basket");
		      YYABORT;
		      }
		}
	 ;


attractor_statement:
           ATTRACTOR_TOKEN coordinate
                {  
		   if(gt_add_attractor($2->x, $2->y, -1) != GT_SUCCESS) {
		      env_SyntaxError("Unable to add Attractor");
		      YYABORT;
		      }
		}
         | ATTRACTOR_TOKEN coordinate NUMBER
                {  
		   if(gt_add_attractor($2->x, $2->y, $3) != GT_SUCCESS) {
		      env_SyntaxError("Unable to add Attractor");
		      YYABORT;
		      }
		}
	 ;

starting_point_statement:
           SP_TOKEN STRING coordinate
                { 
		   if (gt_add_starting_point($2, $3->x, $3->y) != GT_SUCCESS) {
		      env_SyntaxError("Unable to add Starting Point %s", $2);
		      YYABORT;
		      }
		}
         ;

room_statement:
           ROOM_TOKEN STRING coordinate_list
              {
		if (gt_add_room($2, $3) != GT_SUCCESS) {
		  env_SyntaxError("Unable to add Room %s", $2);
		  YYABORT;
		}
	      }
           ;
door_statement:
           DOOR_TOKEN STRING coordinate_list
              {
		if (gt_add_door($2, $3) != GT_SUCCESS) {
		  env_SyntaxError("Unable to add Door %s", $2);
		  YYABORT;
		}
	      }
           ;

hallway_statement:
           HALLWAY_TOKEN STRING CENTERLINE_TOKEN coordinate_list WIDTH_TOKEN NUMBER EXTENTS_TOKEN coordinate_list 
              {
		if (gt_add_hallway($2, $4, $6, $8) != GT_SUCCESS) {
		  env_SyntaxError("Unable to add Hallway %s", $2);
		  YYABORT;
		}
	      }
           ;

waypoint_file_statement:
           WAYPOINT_FILE_TOKEN STRING 
              {
		if (gt_load_waypoint_file($2) != GT_SUCCESS)
		{
		  // Keep parsing even if the file is not loaded.
		}
	      }
           ;

*/
coordinate_list:
           coordinate
                {  
                   $$ = new Vectors();
                   $$->push_back(*$1); 
                }
         | coordinate_list coordinate
                {
                   $$ = $1;
                   $$->push_back(*$2); 
		}
	 ;


coordinate:
		NUMBER NUMBER
   { 
      $$ = new Vector($1 + theEnv->origin_x, $2 + theEnv->origin_y, 0);
   }
   | utm_coordinate {
      $$ = new Vector($1->x - theEnv->origin_x, $1->y - theEnv->origin_y, 0);
   }
   ;

utm_coordinate: 
   UTM_TOKEN LAT_TOKEN NUMBER LON_TOKEN NUMBER 
   {
     double utmx, utmy;
     environment::convert_latlon_to_utm($3, $5, utmx, utmy);
     $$ = new Vector(utmx, utmy, 0);
   }
   | UTM_TOKEN LON_TOKEN NUMBER LAT_TOKEN NUMBER {
     double utmx, utmy;
     environment::convert_latlon_to_utm($5, $3, utmx, utmy);
     $$ = new Vector(utmx, utmy, 0);
   }
   ;

Label:
		STRING
   { 
      $$ = new strings(*$1);
   }
   | /* empty */ 
   {
      $$ = new strings();
   }
   ;


%%
/***********************************************************************/
