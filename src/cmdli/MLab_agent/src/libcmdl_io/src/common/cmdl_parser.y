%{
/**********************************************************************
 **  cmdl_parser.y                                                   **
 **                                                                  **
 **  parser for cmdl (command description language)                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **                                                                  **
 **  Based on: gt_command_yac.y                                      **
 **     Written by:  Jonathan M. Cameron                             **
 **     Copyright 1995, 1996, 1997 Georgia Tech Research Corporation **
 **     Atlanta, Georgia  30332-0415                                 **
 **     ALL RIGHTS RESERVED, See file COPYRIGHT for details.         **
 **********************************************************************/

/* $Id: cmdl_parser.y,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
 * $Log: cmdl_parser.y,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/01 00:19:15  endo
 * CMDLi from MARS 2020 migrated into AO-FNC repository.
 *
 * Revision 1.1.1.1  2006/06/29 20:42:07  endo
 * cmdli local repository.
 *
 * Revision 1.26  2004/11/12 21:56:59  doug
 * renamed loader so doesn't conflict with missionlab
 *
 * Revision 1.25  2004/11/04 21:36:39  doug
 * moved test.cc to util
 *
 * Revision 1.24  2004/10/29 22:38:36  doug
 * working on waittimeout
 *
 * Revision 1.23  2004/10/25 22:58:16  doug
 * working on mars2020 integrated demo
 *
 * Revision 1.22  2004/10/22 21:40:29  doug
 * added goto and command exception support
 *
 * Revision 1.21  2004/07/30 13:41:32  doug
 * support sync command
 *
 * Revision 1.20  2004/05/11 19:34:49  doug
 * massive changes to integrate with USC and GaTech
 *
 * Revision 1.19  2004/04/13 17:56:29  doug
 * handle just a main section without subprocedures
 *
 * Revision 1.18  2004/03/19 22:48:21  doug
 * working on exceptions.
 *
 * Revision 1.17  2004/03/18 22:11:17  doug
 * coded call and return
 *
 * Revision 1.16  2004/03/17 22:17:17  doug
 * working on procedure calls
 *
 * Revision 1.15  2004/03/15 21:41:06  doug
 * coded ActionCommand and Call
 *
 * Revision 1.14  2004/03/12 17:03:52  doug
 * working on procedures
 *
 * Revision 1.13  2004/03/09 21:36:08  doug
 * promote Blocks to mission segments and procedures
 *
 * Revision 1.12  2004/03/08 14:52:34  doug
 * cross compiles on visual C++
 *
 * Revision 1.11  2004/03/01 23:37:51  doug
 * working on real overlay
 *
 * Revision 1.10  2004/02/28 16:35:42  doug
 * getting cmdli to work in sara
 *
 * Revision 1.9  2004/02/20 14:39:35  doug
 * cmdl interpreter runs a bit
 *
 * Revision 1.8  2003/11/10 22:48:30  doug
 * working on cmdli
 *
 * Revision 1.7  2003/11/07 22:54:51  doug
 * working on cmdl_io
 *
 * Revision 1.6  2003/11/07 20:29:27  doug
 * added cmdli interpreter library
 *
 * Revision 1.5  2003/11/03 16:43:31  doug
 * snapshot
 *
 * Revision 1.4  2003/10/27 17:45:37  doug
 * added a test harness
 *
 * Revision 1.3  2003/10/24 21:03:39  doug
 * finally compiles
 *
 * Revision 1.2  2003/10/23 19:14:44  doug
 * closer to compiling a minimal set
 *
 * Revision 1.1.1.1  2003/10/22 22:08:18  doug
 * initial import
 *
 **********************************************************************/

/* rename the yylloc variable so can include two scanners */
#ifndef yylloc
#define yylloc sara_cmdl_lloc
#endif

#include "mic.h"
#include "cmdl_scanner.h"
#include "cmdl_support.h"
#include "cmdl_loader.h"
#include "RobotType_ParmList.h"
#include "RobotType_Info.h"
#include "RobotType_Info.h"
#include "Block.h"
#include "Unit.h"
#include "Command.h"
#include "ActionCommand.h"
#include "CallCommand.h"
#include "GotoCommand.h"
#include "SyncCommand.h"
#include "ReturnCommand.h"
#include "cmdl_Root.h"
#include "forward_reference_check.h"

using namespace sara;		// make all sara names visible in this file

// Turn on verbose parse error messages
#define YYERROR_VERBOSE 1

/**********************************************************************/
// Global variables used by the parser


namespace sara 
{
// The root of the cmdl info
cmdl_Root *cmdl_root;

// the current loader object using the parser
cmdl_loader *the_cmdl_loader;
}

// Turns on user-level debug messages
bool cmdl_user_debug = false;

// The active blocks
Block *mainBlock = NULL;
Block *procBlock = NULL;

// forward references pending for the current blocks
typedef vector< forward_reference_check > forward_refs_T;
forward_refs_T mainRefs;
forward_refs_T procRefs;

// an empty string to pass around
static string emptyString;

/***************************************************************/
%}

%union {
   bool       		boolean;
   double     		number;
   string    		*String;
   sara::PairOfStrings 	*Assign;
   sara::RobotType_ParmList 	*robot_parameters;
   sara::Unit 		*unit;
   sara::Command 		*command;
   sara::Block 		*block;
   sara::BlockList 		*blocks;
   sara::keyValuePair         *options;
   sara::keyValueMap          *optionsList;
   sara::cmdl_transition::Transitions_T   *exceptionsList;
   sara::cmdl_transition      *exception;
}

%token MISSION_NAME SCENARIO MISSION_AREA LOAD_MAP SP
%token UNIT_TOKEN NEW_ROBOT CALL_TOKEN GOTO_TOKEN RETURN_TOKEN IF_TOKEN SYNC_TOKEN
%token START_COMMANDS IMMEDIATE_COMMAND
%token TELEOPERATE_TOKEN SWEEPTO_TOKEN OCCUPY_TOKEN
%token FOLLOW_TOKEN QUIT_TOKEN SET_TOKEN PRINT_TOKEN
%token CONSOLE_DB_TOKEN ROBOT_INFO_TOKEN CREATE_OBSTACLES OBSTACLE_TOKEN
%token FORMATION TECHNIQUE AND UNTIL MOUSE FREEZE TIMEOUT
%token PHASE_LINE_TOKEN ON_COMPLETION ACKNOWLEDGE MESSAGE SPEED
%token SHOW_TRAILS_TOKEN SHOW_REP_OBST_TOKEN SHOW_MOV_VECT_TOKEN
%token SEED_TOKEN OBSTACLE_COVERAGE_TOKEN ZOOM_FACTOR_TOKEN
%token SCALE_ROBOTS_TOKEN ROBOT_LENGTH_TOKEN SHOW_COLORMAP_TOKEN
%token MIN_OBSTACLE_RADIUS_TOKEN MAX_OBSTACLE_RADIUS_TOKEN CYCLE_DURATION_TOKEN
%token DEBUG_ROBOTS_TOKEN DEBUG_SIMULATOR_TOKEN DEBUG_SCHEDULER_TOKEN
%token DEFPROCEDURE ENDPROCEDURE DEFAULTSYNCTIMEOUT

%token <boolean> BOOLVAL
%token <String> STRING DATEVAL TIME LABEL 
%token <number> NUMBER
%token <formation> FORMATION_TYPE
%token <technique> TECHNIQUE_TYPE
%type <robot_parameters> robot_parameter_list
%type <Assign> robot_parameter 
%type <unit> unit_list unit robot_list
%type <options> opt_clause
%type <optionsList> opt_clause_list opt_clauses
%type <exceptionsList> opt_exception_list 
%type <exception> anException
%type <String> maybe_label stringed_value

%%

/* NOTE: "statements" are executed immediately and are never entered into a
         command list.  "commands" are entered into a command list (which may
         or may not be executed immediately). */


file:      info_part commands_part
/*         | IMMEDIATE_COMMAND composite_immediate_command */
         ;

info_part:
           info_statement
         | info_part info_statement
         ;

info_statement:
           mission_name_statement
         | robot_definition_statement
         | unit_description_statement
         | mission_option_statement
/*
         | scenario_statement
	 | mission_area_statement
	 | load_map_statement
         | starting_point_statement
         | set_parameter_statement
         | populate_statement
         | obstacle_statement
         | print_statement
*/
         ;

mission_name_statement:
           MISSION_NAME STRING 					{ 
                   the_cmdl_loader->setMissionName( *$2 );
                }
         ;

mission_option_statement:
           SET_TOKEN STRING '=' stringed_value
                { 
                   cmdl_root->getOptions()->set_string(*$2, *$4);
                }
         ;

stringed_value	: STRING 				{ 
		$$ = $1;
		}
         | BOOLVAL
                {
                stringstream ss;
                ss << $1;
		$$ = new string( ss.str() );
                }
         | STRING '=' NUMBER
                {
                stringstream ss;
                ss << $3;
		$$ = new string( ss.str() );
                }
         ;

	   /* NEW_ROBOT name_of_robot executable [color [host[(parms)]]] */

robot_definition_statement:
           NEW_ROBOT STRING STRING
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT STRING STRING STRING
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
		   ri->color = *($4);
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT STRING STRING STRING STRING
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
		   ri->color = *($4);
		   ri->host = *($5);
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT STRING STRING '(' robot_parameter_list ')'
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
		   ri->parms = $5;
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT STRING STRING STRING '(' robot_parameter_list ')'
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
		   ri->color = *($4);
		   ri->parms = $6;
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT STRING STRING STRING STRING '(' robot_parameter_list ')'
                {
                   if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of robot type '%s'",$2->c_str() );
                      YYABORT;
                   }

                   RobotType_Info *ri = new RobotType_Info();
		   ri->name = *($2);
		   ri->executable = *($3);
		   ri->color = *($4);
		   ri->host = *($5);
		   ri->parms = $7;
                   if( !cmdl_root->defineRobotType( ri ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of robot type '%s' was undetected until the insertion!", $2->c_str() );
 		      delete ri;
                      YYABORT;
                   }
                }
         | NEW_ROBOT error
                {
                   sara_cmdl_syntax_error(@1.first_line,"Unable to parse the new-robot command.\nUsage: NEW-ROBOT robot-name robot-executable [host] [(robot parameter list)]);");
                   YYABORT;
                }
         | NEW_ROBOT STRING error
                {
                   sara_cmdl_error(@1.first_line,"Unable to parse the new-robot executable name.  Try putting it in quotes.\nUsage: NEW-ROBOT robot-name robot-executable [host] [(robot parameter list)]);");
                   YYABORT;
                }
         ;

robot_parameter_list	: robot_parameter 			{
		$$ = new RobotType_ParmList();
		$$->define($1->first,$1->second);
                delete $1;
                }
         | robot_parameter_list ',' robot_parameter
                {
                   $$ = $1;
		   $$->define($3->first,$3->second);
                   delete $3;
                }
         ;

robot_parameter	: STRING '=' STRING 				{ 
		$$ = new PairOfStrings(*$1, *$3);
		}
         | STRING '=' BOOLVAL
                {
                stringstream ss;
                ss << $3;
		$$ = new PairOfStrings(*$1, ss.str());
                }
         | STRING '=' NUMBER
                {
                stringstream ss;
                ss << $3;
		$$ = new PairOfStrings(*$1, ss.str());
                }
         ;

unit_description_statement:
           UNIT_TOKEN '<' STRING '>' robot_list
                { 
                   if( cmdl_root->unitExists( *$3 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of unit '%s'",$3->c_str() );
                      YYABORT;
                   }

                   Unit *theUnit = $5;
		   theUnit->setName( *$3 );
                   if( !cmdl_root->defineUnit( theUnit ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of unit '%s' was undetected until the insertion!", $3->c_str() );
 		      delete theUnit;
                      YYABORT;
                   }
                   
                }
         | UNIT_TOKEN '<' STRING '>' unit_list
                {
                   if( cmdl_root->unitExists( *$3 ) )
                   {
                      sara_cmdl_error(@1.first_line,"Duplicate definition of unit '%s'",$3->c_str() );
                      YYABORT;
                   }

                   Unit *theUnit = $5;
		   theUnit->setName( *$3 );
                   if( !cmdl_root->defineUnit( theUnit ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of unit '%s' was undetected until the insertion!", $3->c_str() );
 		      delete theUnit;
                      YYABORT;
                   }
                   
                }
         ;

unit_list:
           '(' unit ')'
                { 
                   $$ = new Unit;
                   $$->addMember( $2 );
                }
         | unit_list '(' unit ')'
                {
                   $$ = $1;
                   $$->addMember( $3 );
                }
         ;


unit:
           robot_list
                { 
                   $$ = $1;
                }
         | '<' STRING '>' robot_list
                {
                   if( cmdl_root->unitExists( *$2 ) )
                   {
                      sara_cmdl_error(@2.first_line,"Duplicate definition of unit '%s'",$2->c_str() );
                      YYABORT;
                   }

                   Unit *theUnit = $4;
		   theUnit->setName( *$2 );
                   if( !cmdl_root->defineUnit( theUnit ) )
                   {
                      sara_cmdl_error(@1.first_line,"Internal Error: Duplicate definition of unit '%s' was undetected until the insertion!", $2->c_str() );
 		      delete theUnit;
                      YYABORT;
                   }
                   $$ = theUnit;
                }
         | '<' STRING '>' unit_list
                {
                   if( cmdl_root->unitExists( *$2 ) )
                   {
                      sara_cmdl_error(@2.first_line,"Duplicate definition of unit '%s'",$2->c_str() );
                      YYABORT;
                   }

                   Unit *theUnit = $4;
		   theUnit->setName( *$2 );
// NEED TO ATTACH THE SUB UNITS????? theUnit->members = *$4;
                   if( !cmdl_root->defineUnit( theUnit ) )
                   {
                      sara_cmdl_error(@2.first_line,"Internal Error: Duplicate definition of unit '%s' was undetected until the insertion!", $2->c_str() );
 		      delete theUnit;
                      YYABORT;
                   }
                   $$ = theUnit;
                }
         ;


robot_list:
           STRING
                { 
                   $$ = new Unit;

                   // is it a unit?
                   if( cmdl_root->unitExists( *$1 ) )
                   {
                      Unit *unit;
                      if( (unit = cmdl_root->getUnit( *$1 ) ) == NULL )
                      {
                         sara_cmdl_error(@1.first_line,"Didn't find reference to unit '%s' after found it the first time!",$1->c_str() );
                         YYABORT;
                      }

                      $$->addMember(unit);
                   }
                   else if( cmdl_root->robotTypeExists( *$1 ) )
                   {
                      RobotType_Info *ri;
                      if( (ri = cmdl_root->getRobotType( *$1 ) ) == NULL )
                      {
                         sara_cmdl_error(@1.first_line,"Didn't find reference to robot type '%s' after found it the first time!",$1->c_str() );
                         YYABORT;
                      }

//                      Unit *unit = new Unit;
//                      unit->setPrimitive( ri );
//                      $$->addMember(unit);
                      $$->setPrimitive( ri );
                   }
                   else
                   {
                      sara_cmdl_error(@1.first_line,"Reference to undefined unit or robot type '%s'",$1->c_str() );
                      YYABORT;
                   }
                }
         | robot_list STRING
                {
                   $$ = $1;
  
                   // If we made a primitive, we now have to cleanup and add a unit
                   if( $$->isPrimitive() )
                   {
                      Unit *unit = new Unit;
                      unit->addMember($$);
                      $$ = unit;
                   }

                   // is it a unit?
                   if( cmdl_root->unitExists( *$2 ) )
                   {
                      Unit *unit;
                      if( (unit = cmdl_root->getUnit( *$2 ) ) == NULL )
                      {
                         sara_cmdl_error(@2.first_line,"Didn't find reference to unit '%s' after found it the first time!",$2->c_str() );
                         YYABORT;
                      }

                      $$->addMember(unit);
                   }
                   else if( cmdl_root->robotTypeExists( *$2 ) )
                   {
                      RobotType_Info *ri;
                      if( (ri = cmdl_root->getRobotType( *$2 ) ) == NULL )
                      {
                         sara_cmdl_error(@2.first_line,"Didn't find reference to robot type '%s' after found it the first time!",$2->c_str() );
                         YYABORT;
                      }

                      Unit *unit = new Unit;
                      unit->setPrimitive( ri );
                      $$->addMember(unit);
                   }
                   else
                   {
                      sara_cmdl_error(@2.first_line,"Reference to undefined unit or robot type '%s'",$2->c_str() );
                      YYABORT;
                   }
                }
         ;

/*
set_parameter_statement:
           SET_TOKEN SEED_TOKEN NUMBER              { seed = nint($3);           }
         | SET_TOKEN SHOW_TRAILS_TOKEN              { set_show_trails(TRUE);     }
         | SET_TOKEN SHOW_TRAILS_TOKEN BOOLVAL      { set_show_trails($3);       }
         | SET_TOKEN SHOW_REP_OBST_TOKEN            { set_show_repelling_obstacles(TRUE); }
         | SET_TOKEN SHOW_REP_OBST_TOKEN BOOLVAL    { set_show_repelling_obstacles($3);   }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN            { set_show_movement_vectors(TRUE);    }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN BOOLVAL    { set_show_movement_vectors($3);      }
         | SET_TOKEN SHOW_COLORMAP_TOKEN            { set_show_colormap(TRUE);   }
         | SET_TOKEN SHOW_COLORMAP_TOKEN BOOLVAL    { set_show_colormap($3);     }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN             { set_debug_robots(TRUE);    }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN BOOLVAL     { set_debug_robots($3);      }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN          { set_debug_simulator(TRUE); }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN BOOLVAL  { set_debug_simulator($3);   }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN          { set_debug_scheduler(TRUE); }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN BOOLVAL  { set_debug_scheduler($3);   }
         | SET_TOKEN SCALE_ROBOTS_TOKEN             { set_scale_robots(TRUE);    }
         | SET_TOKEN SCALE_ROBOTS_TOKEN BOOLVAL     { set_scale_robots($3);      }
         | SET_TOKEN ROBOT_LENGTH_TOKEN NUMBER
                {
                   if (!set_robot_length((double)$3)) {
                      sara_cmdl_error(@1.first_line,"Error in SET ROBOT-LENGTH %1.8g statement", $3);
                      YYABORT;
                      }
                }
         | SET_TOKEN OBSTACLE_COVERAGE_TOKEN NUMBER
                {
                   if (!set_obstacle_coverage((double)$3)) {
                      sara_cmdl_error(@1.first_line,"Error in SET OBSTACLE-COVERAGE %1.8g", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN MIN_OBSTACLE_RADIUS_TOKEN NUMBER
                {
		   if (!set_min_obstacle_radius((double)$3)) {
                      sara_cmdl_error(@1.first_line,"Error in SET MIN-OBSTACLE-RADIUS %1.8g statement", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN MAX_OBSTACLE_RADIUS_TOKEN NUMBER 
                {
		   if (!set_max_obstacle_radius((double)$3)) {
                      sara_cmdl_error(@1.first_line,"Error in SET MAX-OBSTACLE-RADIUS %1.8g statement", $3);
                      YYABORT;
		      }
                }
         | SET_TOKEN ZOOM_FACTOR_TOKEN NUMBER
                {
		   if (!set_zoom_factor((double)$3/100.0)) {
		      sara_cmdl_error(@1.first_line,"Error in SET ZOOM-FACTOR %1.8g statement", $3);
		      YYABORT;
		      }
		}
         | SET_TOKEN CYCLE_DURATION_TOKEN NUMBER
                {
		   if (!set_cycle_duration((double)$3)) {
                      sara_cmdl_error(@1.first_line,"Error in SET CYCLE-DURATION %1.8g statement", $3);
                      YYABORT;
		      }
                }
         ;


populate_statement:
           CREATE_OBSTACLES
                {
                   create_obstacles(TRUE);
                }
         ;

obstacle_statement:
           OBSTACLE_TOKEN NUMBER NUMBER NUMBER
                {
                   if (gt_add_obstacle($2+origin_x, $3+origin_y, $4) !=  GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Unable to add Obstacle (x=%1.8g, y=%1.8g, r=%1.8g).", 
                                  $2, $3, $4);
                      YYABORT;
                      }
                }
         ;

print_statement:
           PRINT_TOKEN CONSOLE_DB_TOKEN             { gt_print_db(); }
         | PRINT_TOKEN ROBOT_INFO_TOKEN             { gt_print_robot_info(); }
         ;

*/

commands_part:
           START_COMMANDS {mainBlock = new Block();} block_defs block	
	   {
	      // check any pending forward references
	      for(uint i=0; i<mainRefs.size(); ++i)
	      {
		 mainRefs[i].checkIt();
	      }
	      mainRefs.clear();

              // copy over the main block
              the_cmdl_loader->addMainBlock( mainBlock );
	      mainBlock = NULL;
	   }
         | START_COMMANDS {mainBlock = new Block();} block	
	   {
	      // check any pending forward references
	      for(uint i=0; i<mainRefs.size(); ++i)
	      {
		 mainRefs[i].checkIt();
	      }
	      mainRefs.clear();

              // copy over the main block
              the_cmdl_loader->addMainBlock( mainBlock );
	      mainBlock = NULL;
	   }
         ;

block_defs:	def_procedure
		| block_defs def_procedure
	  ;

def_procedure:	DEFPROCEDURE {procBlock = new Block();} STRING block ENDPROCEDURE
		{
	           // check any pending forward references
	           for(uint i=0; i<procRefs.size(); ++i)
	           {
		      procRefs[i].checkIt();
	           }
	           procRefs.clear();

                   // label the block with the proc name.
                   procBlock->set_label(*$3);

		   // add it to the mission
                   the_cmdl_loader->addBlock(procBlock);

		   // done with the block
		   procBlock = NULL;
		}
	  ;

block:
           block_option_statement
         | command
         | block command
         | block block_option_statement
         ;

block_option_statement:
           SET_TOKEN STRING '=' stringed_value
                { 
		   if( procBlock )
                      procBlock->getOptions()->set_string(*$2, *$4);
		   else
                      mainBlock->getOptions()->set_string(*$2, *$4);
                }
         ;

command:       maybe_label UNIT_TOKEN STRING STRING opt_clause_list opt_exception_list
		{
                   // get the unit
                   Unit *unit;
                   if( (unit = cmdl_root->getUnit( *$3 ) ) == NULL )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined unit '%s'.",$3->c_str() );
                   }

                   // if this command is for us, check the action is defined
                   string *action = $4;
		   if( unit->isMember( cmdl_root->get_ourName()) && !the_cmdl_loader->isActionName( *action ) )
                   {
                      sara_cmdl_error(@4.first_line,"Reference to undefined action '%s'.",action->c_str() );
		   }

		   // add the command
                   Command *cmd = new ActionCommand(*$1, unit, *action, $5, *$6);
		   if( procBlock )
                      procBlock->addCommand( cmd );
		   else
                      mainBlock->addCommand( cmd );
		}
           | maybe_label UNIT_TOKEN STRING CALL_TOKEN STRING 
		{
                   // subroutine call

                   // get the unit
                   Unit *unit;
                   if( (unit = cmdl_root->getUnit( *$3 ) ) == NULL )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined unit '%s'.",$3->c_str() );
                   }

                   // check the subroutine is defined
                   string *sub = $5;
                   if( !the_cmdl_loader->isBlockName( *sub ) )
                   {
                      sara_cmdl_error(@5.first_line,"Reference to undefined procedure '%s'.",sub->c_str() );
                   }

		   // add it
                   Command *cmd = new CallCommand(*$1, unit, *sub);
		   if( procBlock )
                      procBlock->addCommand( cmd );
		   else
                      mainBlock->addCommand( cmd );
		}
           | maybe_label UNIT_TOKEN STRING SYNC_TOKEN
		{
                   // force a syncronization

                   // get the unit
                   Unit *unit;
                   if( (unit = cmdl_root->getUnit( *$3 ) ) == NULL )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined unit '%s'.",$3->c_str() );
                   }

		   // add it
                   Command *cmd = new SyncCommand(*$1, unit);
		   if( procBlock )
                      procBlock->addCommand( cmd );
		   else
                      mainBlock->addCommand( cmd );
		}
           | maybe_label UNIT_TOKEN STRING RETURN_TOKEN 
		{
                   // return from subroutine call

                   // get the unit
                   Unit *unit;
                   if( (unit = cmdl_root->getUnit( *$3 ) ) == NULL )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined unit '%s'.",$3->c_str() );
                   }

		   // add it
                   Command *cmd = new ReturnCommand(*$1, unit);
		   if( procBlock )
                      procBlock->addCommand( cmd );
		   else
                      mainBlock->addCommand( cmd );
		}
           | IF_TOKEN STRING opt_clause_list CALL_TOKEN STRING
		{
                   // call a subroutine 

                   // check the trigger is defined
                   string *trig = $2;
                   if( !the_cmdl_loader->isTriggerName( *trig ) )
                   {
                      sara_cmdl_error(@2.first_line,"Reference to undefined trigger '%s'.",trig->c_str() );
                   }

                   // check the subroutine is defined
                   string *sub = $5;
                   if( !the_cmdl_loader->isBlockName( *sub ) )
                   {
                      sara_cmdl_error(@5.first_line,"Reference to undefined procedure '%s'.",sub->c_str() );
                   }

                   // create it
                   cmdl_transition *trans = new cmdl_transition(true, *trig, $3, *sub);

		   if( procBlock )
                      procBlock->addException( trans );
		   else
                      mainBlock->addException( trans );
		}
           | maybe_label UNIT_TOKEN STRING GOTO_TOKEN STRING 
		{
                   // jump to a label

                   // get the unit
                   Unit *unit;
                   if( (unit = cmdl_root->getUnit( *$3 ) ) == NULL )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined unit '%s'.",$3->c_str() );
                   }

                   // check the destination is defined
                   string *dest = $5;
		   bool valid;
		   if( procBlock )
                      valid = procBlock->isValidCommandLabel( *dest );
		   else
                      valid = mainBlock->isValidCommandLabel( *dest );
                   if( !valid )
                   {
		      // probably forward reference
		      stringstream buf;
		      buf << "Reference to undefined command label '" << *dest << "'";
		      if( procBlock )
                         procRefs.push_back( forward_reference_check(@5.first_line, buf.str(), procBlock, *dest ));
		      else
                         mainRefs.push_back( forward_reference_check(@5.first_line, buf.str(), mainBlock, *dest ));
		   }

		   // add it
                   Command *cmd = new GotoCommand(*$1, unit, *dest);
		   if( procBlock )
                      procBlock->addCommand( cmd );
		   else
                      mainBlock->addCommand( cmd );
		}
         ;

/*
composite_immediate_command:
           immediate_command 
         | composite_immediate_command AND immediate_command
         ;

immediate_command:
         | set_parameter_statement
         | populate_statement
         | obstacle_statement
         | print_statement
         | unit_description_statement
         ;

teleoperate_command:
           UNIT_TOKEN STRING TELEOPERATE_TOKEN STRING {gt_start_command();} opt_teleoperate_clause_list
                {
                   if (gt_add_command_to_step($2, TELEOPERATE, $4) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding command: UNIT %s TELEOPERATE %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

sweepto_command:
           UNIT_TOKEN STRING SWEEPTO_TOKEN STRING {gt_start_command();} opt_sweepto_clause_list
                {
                   if (gt_add_command_to_step($2, SWEEPTO, $4) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding command: UNIT %s SWEEPTO %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

follow_command:
           UNIT_TOKEN STRING FOLLOW_TOKEN STRING {gt_start_command();} opt_follow_clause_list
                {
                   if (gt_add_command_to_step($2, FOLLOW, $4) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding command: UNIT %s FOLLOW %s",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

occupy_command:
           UNIT_TOKEN STRING OCCUPY_TOKEN STRING {gt_start_command();} opt_occupy_clause_list
                {
                   if (gt_add_command_to_step($2, OCCUPY, $4) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding command: UNIT %s OCCUPY %s ...",$2,$4);
                      YYABORT;
                      }
                   free($2);
                   free($4);
                }
         ;

quit_command:
           QUIT_TOKEN
                { 
                   if (gt_add_command_to_step(NULL, QUIT, NULL) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding QUIT command");
                      YYABORT;
                      }
                }
         ;


set_parameter_command:
           SET_TOKEN SHOW_TRAILS_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_TRAILS, SET, (char *)TRUE)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET SHOW-TRAILS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_TRAILS_TOKEN BOOLVAL
                { 
                   if (gt_add_command_to_step((char *)SHOW_TRAILS,SET,(char *)$3)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET SHOW-TRAILS %s command",
                                  ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }
 

         | SET_TOKEN SHOW_REP_OBST_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_REPELLING_OBSTACLES,
                                              SET, (char *)TRUE) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET HIGHLIGHT-REPELLING-OBSTACLES\
TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_REP_OBST_TOKEN BOOLVAL
                { 
                   if (gt_add_command_to_step((char *)SHOW_REPELLING_OBSTACLES,
                                              SET,(char *)$3) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET HIGHLIGHT-REPELLING-OBSTACLES\
%s command", ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }

         | SET_TOKEN SHOW_MOV_VECT_TOKEN
                { 
                   if (gt_add_command_to_step((char *)SHOW_MOVEMENT_VECTORS,
                                              SET, (char *)TRUE) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET SHOW-MOVEMENT-VECTORS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN SHOW_MOV_VECT_TOKEN BOOLVAL
                { 
                   if (gt_add_command_to_step((char *)SHOW_MOVEMENT_VECTORS,
                                              SET,(char *)$3) != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET SHOW-MOVEMENT-VECTORS %s command",
                                  ($3 ? "ON" : "OFF") );
                      YYABORT;
                      }
                }

         | SET_TOKEN DEBUG_ROBOTS_TOKEN 
                { 
                   if (gt_add_command_to_step((char *)DEBUG_ROBOTS,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG-ROBOTS TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_ROBOTS_TOKEN BOOLVAL
                { 
                   if (gt_add_command_to_step((char *)DEBUG_ROBOTS,SET,(char *)$3) 
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG-ROBOTS %s command",
                                  ($3 ? "ON" : "OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SIMULATOR,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG_SIMULATOR TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SIMULATOR_TOKEN BOOLVAL  
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SIMULATOR,SET,(char *)$3)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG-SIMULATOR %s command",
                                  ($3 ? "ON":"OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SCHEDULER,SET,(char *)TRUE)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG_SCHEDULER TRUE command");
                      YYABORT;
                      }
                }
         | SET_TOKEN DEBUG_SCHEDULER_TOKEN BOOLVAL  
                { 
                   if (gt_add_command_to_step((char *)DEBUG_SCHEDULER,SET,(char *)$3)
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET DEBUG-SCHEDULER %s command",
                                  ($3 ? "ON":"OFF"));
                      YYABORT;
                      }
                }
         | SET_TOKEN CYCLE_DURATION_TOKEN NUMBER
                {
                   if ($3 < MIN_CYCLE_DURATION) {
                      sara_cmdl_error(@1.first_line,"Cycle duration value, %1.8g, is less than\n\
 the minimum value of %1.8g seconds", $3, MIN_CYCLE_DURATION);
                      YYABORT;
                      }
                   if ($3 > MAX_CYCLE_DURATION) {
                      sara_cmdl_error(@1.first_line,"Cycle duration value, %1.8g, is greater than\n\
 the maximum value of %1.8g seconds", $3, MAX_CYCLE_DURATION);
                      YYABORT;
                      }
                   if (gt_add_command_to_step((char *)CYCLE_DURATION,SET,(char *)&($3))
                       != GT_SUCCESS) {
                      sara_cmdl_error(@1.first_line,"Error adding SET CYCLE-DURATION %1.8g command",$3);
                      YYABORT;
                      }
                }
         ;


print_command: 
           PRINT_TOKEN CONSOLE_DB_TOKEN
                {
                   if (gt_add_command_to_step((char *)CONSOLE_DB, PRINT, NULL)
                       != GT_SUCCESS) {
                      sara_cmdl_error("Error adding PRINT CONSOLE-DATABASE command");
                      YYABORT;
                      }
                }
         | PRINT_TOKEN ROBOT_INFO_TOKEN
                {
                   if (gt_add_command_to_step((char *)ROBOT_INFO, PRINT, NULL)
                       != GT_SUCCESS) {
                      sara_cmdl_error("Error adding PRINT ROBOT-INFO command");
                      YYABORT;
                      }
                }
         ;

*/

opt_clause_list:  opt_clauses {
	      $$ = $1;
	   }
	 | {
	      $$ = new keyValueMap;
	 }
         ;

opt_clauses:  opt_clause {
	      $$ = new keyValueMap;
	      (*$$)[$1->first] = $1->second;
	   }
         | opt_clauses opt_clause {
	      $$ = $1;
              if( $$->find( $2->first ) != $$->end() )
              {
                 sara_cmdl_error(@2.first_line,"Duplicate definition of option '%s'.", $2->first.c_str() );
              }
	      else
	      {
	         (*$$)[$2->first] = $2->second;
	      }
	   }
         ;

opt_clause:  STRING '=' STRING {
	      $$ = new keyValuePair(*$1, *$3);
	   }
           | STRING '=' NUMBER {
              stringstream buf;
              buf << std::setprecision(FloatPrecision) << $3;
	      $$ = new keyValuePair(*$1, buf.str());
	   }
           | STRING '=' '[' STRING ']' {

              // If we have an environment file, look up the environmental object
	      if( cmdl_root->currentEnvironment() )
	      {
                 env_object *obj;
                 if( (obj = cmdl_root->getObject( *$4 ) ) == NULL )
                 {
                    sara_cmdl_error(@4.first_line,"Reference to undefined environmental object '%s'.",$4->c_str() );
                    YYABORT;
                 }
              
                 if( !obj->theShape )
                 {
                    sara_cmdl_error(@4.first_line,"Referenced environmental object '%s' has no defined shape!",$4->c_str() );
                    YYABORT;
                 }

                 Vector loc = obj->theShape->center();
                 stringstream buf;
                 buf << loc;

	         $$ = new keyValuePair(*$1, buf.str());
	      }
	      else
	      {
		 string val = "[" + *$4 + "]";
	         $$ = new keyValuePair(*$1, val);
	      }
	   }
           | STRING '=' '<' NUMBER ',' NUMBER '>' {

              // A raw UTM coordinate
              Vector loc($4, $6);
              stringstream buf;
              buf << loc;
	      $$ = new keyValuePair(*$1, buf.str());
	   }
           | STRING '=' '<' NUMBER ',' NUMBER ',' NUMBER '>' {

              // A raw UTM coordinate with altitude
              Vector loc($4, $6, $8);
              stringstream buf;
              buf << loc;
	      $$ = new keyValuePair(*$1, buf.str());
	   }

/*
           formation_clause
         | technique_clause
         | speed_clause
         | phase_line_clause
         | on_completion_clause
*/


         ;

opt_exception_list:  	opt_exception_list anException {
	      $$ = $1;
	      $$->push_back( $2 );
	   }
	 		| {
              $$ = new cmdl_transition::Transitions_T;
	   }
         		;

anException:  	'{' IF_TOKEN STRING opt_clause_list CALL_TOKEN STRING '}'
		{
                   // call an exception handler

                   // check the trigger is defined
                   string *trig = $3;
                   if( !the_cmdl_loader->isTriggerName( *trig ) )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined trigger '%s'.",trig->c_str() );
                   }

                   // check the subroutine is defined
                   string *sub = $6;
                   if( !the_cmdl_loader->isBlockName( *sub ) )
                   {
                      sara_cmdl_error(@6.first_line,"Reference to undefined procedure '%s'.",sub->c_str() );
                   }

                   // create it
                   $$ = new cmdl_transition(true, *trig, $4, *sub);
		}
            	| '{' IF_TOKEN STRING opt_clause_list GOTO_TOKEN STRING '}'
		{
                   // goto an exception handler

                   // check the trigger is defined
                   string *trig = $3;
                   if( !the_cmdl_loader->isTriggerName( *trig ) )
                   {
                      sara_cmdl_error(@3.first_line,"Reference to undefined trigger '%s'.",trig->c_str() );
                   }

                   // check the destination is defined
                   string *dest = $6;
		   bool valid;
		   if( procBlock )
                      valid = procBlock->isValidCommandLabel( *dest );
		   else
                      valid = mainBlock->isValidCommandLabel( *dest );
                   if( !valid )
                   {
		      // probably forward reference
		      stringstream buf;
		      buf << "Reference to undefined command label '" << *dest << "'";
		      if( procBlock )
                         procRefs.push_back( forward_reference_check(@6.first_line, buf.str(), procBlock, *dest ));
		      else
                         mainRefs.push_back( forward_reference_check(@6.first_line, buf.str(), mainBlock, *dest ));
                   }

                   // create it
                   $$ = new cmdl_transition(false, *trig, $4, *dest);
		}
         ;

/*

opt_teleoperate_clause_list:
           // empty 
         | opt_teleoperate_clause_list opt_teleoperate_clause
         ;

opt_teleoperate_clause:
           formation_clause
         | speed_clause
         | on_completion_clause
         ;


opt_sweepto_clause_list:
           // empty 
         | opt_sweepto_clause_list opt_sweepto_clause
         ;

opt_sweepto_clause:
           formation_clause
         | technique_clause
         | speed_clause
         | phase_line_clause
         | on_completion_clause
         ;


opt_follow_clause_list:
           // empty 
         | opt_follow_clause_list opt_follow_clause
         ;

opt_follow_clause:
           formation_clause 
         | technique_clause 
         | speed_clause
         | phase_line_clause
         | on_completion_clause
         ;

opt_occupy_clause_list:
           // empty 
         | opt_occupy_clause_list opt_occupy_clause
         ;

opt_occupy_clause:
           formation_clause
         | until_clause
         ;


formation_clause:  
           FORMATION '=' FORMATION_TYPE { gt_specify_formation($3); }
         | FORMATION FORMATION_TYPE     { gt_specify_formation($2); }
         | FORMATION_TYPE               { gt_specify_formation($1); }
         ;

technique_clause:
           TECHNIQUE '=' TECHNIQUE_TYPE { gt_specify_technique($3); }
         | TECHNIQUE TECHNIQUE_TYPE     { gt_specify_technique($2); }
         | TECHNIQUE_TYPE               { gt_specify_technique($1); }
         ;


speed_clause:
           SPEED NUMBER       { gt_specify_speed($2); }
         | SPEED '=' NUMBER   { gt_specify_speed($3); }
         ;

phase_line_clause:
           phase_line_clause_start
         | phase_line_clause_start ACKNOWLEDGE STRING
                { 
                   gt_specify_phase_line_msg($3);
                   free($3);
                }
         | phase_line_clause_start ACKNOWLEDGE STRING WAIT
                { 
                   gt_specify_phase_line_msg($3);
                   gt_specify_phase_line_wait(TRUE);
                   free($3);
                }
         ;

phase_line_clause_start:
           PHASE_LINE_TOKEN STRING
                {
                   gt_specify_phase_line($2, NULL, NULL);
                   free($2);
                }
         | PHASE_LINE_TOKEN STRING DATEVAL TIME
                {
                   gt_specify_phase_line($2, $3,   $4);
                   free($2);
                   free($3);
                   free($4);
                }
         | PHASE_LINE_TOKEN STRING TIME DATEVAL
                {
                   gt_specify_phase_line($2, $4,   $3);
                   free($2); 
                   free($3);
                   free($4);
                }
         | PHASE_LINE_TOKEN STRING TIME
                {
                   gt_specify_phase_line($2, NULL, $3);
                   free($2);
                   free($3);
                }
         ;

on_completion_clause:
           ON_COMPLETION MESSAGE STRING
                { 
                   gt_specify_completion($3, FALSE);
                   free($3);
                } 
         | ON_COMPLETION MESSAGE STRING FREEZE
                { 
                   gt_specify_completion($3, TRUE);
                   free($3);
                }
         ;

until_clause:
           UNTIL TIME
                { 
                   gt_specify_until(NULL, $2);
                   free($2);
                }
         | UNTIL TIME DATEVAL
                { 
                   gt_specify_until($3, $2);
                   free($2);
                   free($3);
                }
         | UNTIL DATEVAL TIME
                { 
                   gt_specify_until($2, $3);
                   free($2);
                   free($3);
                }
         | UNTIL NUMBER
                {
                   char time_str[20];
                   sprintf(time_str, "%d", nint($2));
                   gt_specify_until(NULL, time_str);
                }
         | UNTIL TIMEOUT TIME
                {
                   if ($3[0] == '+') {
                      $3[0] = '-';
                      gt_specify_until(NULL, $3);
                      free($3);
                      }
                   else {
                      char wait_time[20];
                      sprintf(wait_time, "-%s", $3);
                      gt_specify_until(NULL, wait_time);
                      free($3);
                      }
                }
         | UNTIL TIMEOUT NUMBER
                {
                   char wait_time[20];
                   sprintf(wait_time, "-%d", nint($3));
                   gt_specify_until(NULL, wait_time);
                }
         ;

*/

maybe_label:    LABEL {
		   $$ = $1;
		}
	|
		{
		   $$ = &emptyString;
		}
	;

%%


/**********************************************************************/
