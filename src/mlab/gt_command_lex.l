%{
/**********************************************************************
 **                                                                  **
 **                         gt_command_lex.l                         **
 **                                                                  **
 **     Lexical scanner for command description language (cmdl)      **
 **                                                                  **
 **       See the MissionLab user's manual for details.              **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_command_lex.l,v 1.2 2008/07/16 21:16:09 endo Exp $  */

extern "C" {
#include <stdio.h>
#include <string.h>
}

#define yylval cmdl_lval

/***************************************************************/
/* Get rid of "defined but not used" warnings from gcc */
#define YY_NO_UNPUT
  
/***************************************************************/

/* redefine the input to allow input from strings OR files */

char cmdl_command_buff[1024];
int cmdl_command_buff_pos; /* Next character to read */
int cmdl_reading_string;   /* TRUE means reading from a string,
			      FALSE means reading froma  file */

#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
{ \
    int i = 0; \
    if (cmdl_reading_string) { \
       result = 0;\
       while (cmdl_command_buff[cmdl_command_buff_pos]) {\
	   buf[i++] = cmdl_command_buff[cmdl_command_buff_pos++];\
	   result++;\
	   } \
       buf[result] = '\0'; \
       }\
    else { \
       if ( (result = read( fileno(yyin), (char *) buf, max_size )) < 0 ) \
	  YY_FATAL_ERROR( "read() in flex scanner failed" ); \
       } \
}

#include "console.h"
#include "gt_command.h"
#include "gt_console_db.h"
#include "gt_command_yac.h"


static int lineno = 1;

static int reading_robot_name = FALSE;

char cmdl_filename[256];

void cmdl_error(char *s);

%}

comment "--".*

id [a-zA-Z][a-zA-Z0-9_\-]*[\*]?

filename [/a-zA-Z0-9\.][/a-zA-Z0-9\-\._]*[~]?

number -?(([0-9]+)|([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))(([eE][-+]?[0-9]+)?)

qstring \"[^\"\n]*[\"\n]

date [0-9]?[0-9]-[0-9][0-9]-[0-9][0-9]

time ([0-9]?[0-9]:[0-9][0-9])|([0-9]?[0-9]:[0-9][0-9]:[0-9][0-9])

rtime (\+[0-9]?[0-9]:[0-9][0-9])|(\+[0-9]?[0-9]:[0-9][0-9]:[0-9][0-9])|(\+[0-9]+)

step_number [0-9][a-zA-Z0-9]*\.

mission_name MISSION[\t ]+NAME

start_commands COMMAND[\t ]+LIST:

imd_command IMMEDIATE[\t ]+COMMAND:

telop ((TELEOPERATE)|(TELOP))

acknowledge ((ACKNOWLEDGE)|(ACK))

message ((MESSAGE)|(MSG))

console_db ((CONSOLE-DATABASE)|(CONSOLE-DB))

show_trails ((SHOW-TRAILS)|(SHOW-ROBOT-TRAILS))

show_rep_obst ((HIGHLIGHT-REPELLING-OBSTACLES)|(SHOW-REPELLING-OBSTACLES))

use_military_style ((USE-MILITARY-STYLE)|(SHOW-MILITARY-STYLE))

%%

     /* TOKENS */

{mission_name}   { return MISSION_NAME;      }
SCENARIO         { return SCENARIO;          }
OVERLAY          { return OVERLAY;           }
MISSION-AREA     { return MISSION_AREA;      }
LOAD-MAP         { return LOAD_MAP;          }
SP               { return SP;                }
UNIT             { return UNIT_TOKEN;        }
HEIGHTMAP        { return HEIGHTMAP;         }

NEW-ROBOT        { reading_robot_name = TRUE;
                   return NEW_ROBOT;         }

CREATE-OBSTACLES { return CREATE_OBSTACLES;  }
OBSTACLE         { return OBSTACLE_TOKEN;    }

{start_commands} { return START_COMMANDS;    }
{imd_command}    { return IMMEDIATE_COMMAND; }

START            { return START_TOKEN;       }
MOVETO         	 { return MOVETO_TOKEN;      }
{telop}          { return TELEOPERATE_TOKEN; }
SWEEPTO          { return SWEEPTO_TOKEN;     }
OCCUPY         	 { return OCCUPY_TOKEN;      }
FOLLOW         	 { return FOLLOW_TOKEN;      }
STOP           	 { return STOP_TOKEN;        }
QUIT             { return QUIT_TOKEN;        }
SET              { return SET_TOKEN;         }

PRINT            { return PRINT_TOKEN;       }
{console_db}     { return CONSOLE_DB_TOKEN;  }
ROBOT-INFO       { return ROBOT_INFO_TOKEN;  }

AND            	 { return AND;               }
	       	                           
FORMATION      	 { return FORMATION;         }
TECHNIQUE      	 { return TECHNIQUE;         }
SPEED            { return SPEED;             }
PHASE-LINE     	 { return PHASE_LINE_TOKEN;  }
UNTIL      	 { return UNTIL;             }
TIMEOUT          { return TIMEOUT;           }
ON-COMPLETION    { return ON_COMPLETION;     }
{acknowledge}    { return ACKNOWLEDGE;       }
{message}        { return MESSAGE;           }
WAIT             { return WAIT;              }

(SEED|SYSTEM-SEED)    { return SEED_TOKEN;                }
{show_trails}         { return SHOW_TRAILS_TOKEN;         }
{show_rep_obst}       { return SHOW_REP_OBST_TOKEN;       }
{use_military_style}  { return USE_MILITARY_STYLE_TOKEN;  }
SHOW-MOVEMENT-VECTORS { return SHOW_MOV_VECT_TOKEN;       }
SHOW-COLORMAP         { return SHOW_COLORMAP_TOKEN;       }
OBSTACLE-COVERAGE     { return OBSTACLE_COVERAGE_TOKEN;   }
ZOOM-FACTOR           { return ZOOM_FACTOR_TOKEN;         }
SCALE-ROBOTS          { return SCALE_ROBOTS_TOKEN;        }
ROBOT-LENGTH          { return ROBOT_LENGTH_TOKEN;        }
MIN-OBSTACLE-RADIUS   { return MIN_OBSTACLE_RADIUS_TOKEN; }
MAX-OBSTACLE-RADIUS   { return MAX_OBSTACLE_RADIUS_TOKEN; }
CYCLE-DURATION        { return CYCLE_DURATION_TOKEN;      }
DEBUG-ROBOTS          { return DEBUG_ROBOTS_TOKEN;        }
DEBUG-SIMULATOR       { return DEBUG_SIMULATOR_TOKEN;     }
DEBUG-SCHEDULER       { return DEBUG_SCHEDULER_TOKEN;     }
INJECTTRACK           { return INJECTTRACK_TOKEN;         }
RED                   { return RED_TOKEN;                 }
YELLOW                { return YELLOW_TOKEN;              }

(ON|TRUE|YES)    { yylval.boolean = TRUE;
                   return BOOLEAN;           }
(OFF|FALSE|NO)   { yylval.boolean = FALSE;
                   return BOOLEAN;           }


     /* FORMATIONS */

NO-FORMATION   { yylval.formation = NO_FORMATION;
                 return FORMATION_TYPE;
               }
LINE           { yylval.formation = LINE;
                 return FORMATION_TYPE;
               }
LINE-LEFT      { yylval.formation = LINE_LEFT;
                 return FORMATION_TYPE;
               }
LINE-RIGHT     { yylval.formation = LINE_RIGHT;
                 return FORMATION_TYPE;
               }
WEDGE          { yylval.formation = WEDGE;
                 return FORMATION_TYPE;
               }
WEDGE-LEFT     { yylval.formation = WEDGE_LEFT;
                 return FORMATION_TYPE;
               }
WEDGE-RIGHT    { yylval.formation = WEDGE_RIGHT;
                 return FORMATION_TYPE;
               }
DIAMOND        { yylval.formation = DIAMOND;
                 return FORMATION_TYPE;
               }
COLUMN         { yylval.formation = COLUMN;
                 return FORMATION_TYPE;
               }
SWEEP          { yylval.formation = SWEEP;
                 return FORMATION_TYPE;
               }


     /* TECHNIQUES */

TRAVELING            { yylval.technique = TRAVELING;
                       return TECHNIQUE_TYPE;
                     }
TRAVELING-OVERWATCH  { yylval.technique = TRAVELING_OVERWATCH;
                       return TECHNIQUE_TYPE;
                     }
BOUNDING-OVERWATCH   { yylval.technique = BOUNDING_OVERWATCH;
                       return TECHNIQUE_TYPE;
                     }


     /* TIME */

{date}         { yylval.strval = strdup(yytext);
                 return DATE;
	       }

{time}         { yylval.strval = strdup(yytext);
                 return TIME;
	       }

{rtime}        { yylval.strval = strdup(yytext);
                 return TIME;
	       }


     /* STEP NUMBER */

{step_number}  { yylval.strval = strdup(yytext);
                 return STEP_NUMBER;
	       }



     /* NUMBERS */

{number}       { float tempnum;
                 if (sscanf(yytext, "%f", &tempnum) == 1) {
		    yylval.fval = tempnum;
                    return NUMBER;
                    }
                 else {
		    cmdl_error("Illegal number syntax");
		    yylval.fval = 0.0;
		    return NUMBER;
		    }
               }



     /* ROBOT */

ROBOT          { 
                  yylval.robot = ROBOT;
		  return ROBOT_TYPE;
               }


     /* IDENTIFIERS */

{id}           {
                  gt_Robot type;
		  /* This kluge is necessary to test for previously existing
		     robot names only.  Testing all the time leads to spurious
		     matches such as preventing different "robots" from using
		     the same executable.  The variable reading_robot_name
		     must be TRUE immediately after reading NEW-ROBOT for ONE
		     token only.  Same applies to QUOTED STRINGS below. */
                  if (reading_robot_name) {
		     if ((type = gt_robot_type(yytext)) == GT_FAILURE) {
			yylval.strval = strdup(yytext);
			reading_robot_name = FALSE;
			return IDENTIFIER;
			}
		     else {
			yylval.robot = type;
			reading_robot_name = FALSE;
			return ROBOT_TYPE;
			}
		     }
		  else {
		     yylval.strval = strdup(yytext);
		     return IDENTIFIER;
		     }
               }



     /* FILENAMES */

{filename}     { 
                  yylval.strval = strdup(yytext);
		  return FILENAME;
               }



     /* QUOTED STRINGS */

{qstring}      {
                  gt_Robot type;
		  yylval.strval = strdup(yytext+1);  /* skip open quote */
		  if (yylval.strval[yyleng-2] != '"')
		     cmdl_error("Unterminated character string");
		  else /* remove close quote */
		     yylval.strval[yyleng-2] = '\0';
		  /* See comment in IDENTIFIERS above */
		  if (reading_robot_name) {
		     if ((type = gt_robot_type(yylval.strval)) == GT_FAILURE) {
			reading_robot_name = FALSE;
			return QSTRING;
			}
		     else {
			free(yylval.strval);
			yylval.robot = type;
			reading_robot_name = FALSE;
			return ROBOT_TYPE;
			}
		     }
		  else {
		     return QSTRING;
		     }
               }



     /* SPECIAL CHARACTERS */

\:            { return ':'; }

\(            { return '('; }

\)            { return ')'; }

\<            { return '<'; }

\>            { return '>'; }

\=            { return '='; }

\,            { return ','; }



     /* WHITE SPACE AND COMMENTS */

[\t ]+          ;  /* ignore whitespace */

{comment}       ;  /* ignore comments */

\n              {  lineno++; }

.               {  /* default */
                   printf("\nWarning, invalid character <%s> ignored in line %d\n",
			  yytext, lineno);
                }

%%

void cmdl_error(char *s)
{
   char msg[256];
   if (cmdl_filename[0] != '\0')
      sprintf(msg, "Syntax error in command description file\n\
   '%s' at line %d:\n   %s at '%s'.", cmdl_filename, lineno, s, yytext);
   else
      sprintf(msg, "Syntax error in user command:\n %s at '%s'.", s, yytext);
   warn_user(msg);
}


void cmdl_errorf(const char *fmt, ...)
{
   va_list args;
   char msg[512];
   char err[256];

   va_start(args, fmt);
   vsprintf(err, fmt, args);
   va_end(args);
   if (cmdl_filename[0] != '\0')
      sprintf(msg, "Error in command description file\n\
   '%s' at line %d:\n   %s.", cmdl_filename, lineno, err);
   else
      sprintf(msg, "Error in user command:\n %s.", err);
   warn_user(msg);
}


int cmdl_wrap()
{
   return TRUE;
}



void cmdl_file_reset()
{
   lineno = 1;

   YY_NEW_FILE;

   cmdl_command_buff[0] = '\0';
   cmdl_command_buff_pos = 0;
   cmdl_reading_string = FALSE;
}



/**********************************************************************
 * $Log: gt_command_lex.l,v $
 * Revision 1.2  2008/07/16 21:16:09  endo
 * Military style is now a toggle option.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2005/04/08 01:56:03  pulam
 * addition of cnp behaviors
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.51  1997/02/14  15:49:30  zchen
 * *** empty log message ***
 *
 * Revision 1.50  1996/10/09  17:07:19  doug
 * New version of lex allows getting rid of kludge
 *
 * Revision 1.49  1996/08/22  15:09:48  doug
 * added linux compatibility code
 *
 * Revision 1.48  1995/06/16 14:59:49  jmc
 * Added function cmdl_never_called() to eliminate compiler
 * complaints (with -Wall).
 *
 * Revision 1.47  1995/06/14  17:13:24  jmc
 * Added token for SHOW-COLORMAP.
 *
 * Revision 1.46  1995/06/12  16:18:02  jmc
 * Added MISSION-AREA and LOAD-MAP statements.
 *
 * Revision 1.45  1995/05/06  00:42:44  jmc
 * Changed the tokens for METERS_PER_PIXEL to ZOOM_FACTOR.
 *
 * Revision 1.44  1995/05/04  19:54:30  jmc
 * Various fixes to accomodate the different style of map scaling.
 * Renamed MAGNIFY-ROBOTS tokens to SCALE-ROBOTS.  Renamed
 * ROBOT-MAGNIFICATION to ROBOT-LENGTH.
 *
 * Revision 1.43  1995/04/26  18:56:03  jmc
 * Added token for TELEOPERATION behavior.
 *
 * Revision 1.42  1995/04/25  22:16:39  jmc
 * Modified some of the user warnings to put the filename on a
 * separate line since they can be very long.
 *
 * Revision 1.41  1995/04/24  16:43:07  jmc
 * Added YES and NO as synonyms for the BOOLEANs True and False.
 *
 * Revision 1.40  1995/04/20  17:52:48  jmc
 * Corrected spelling.
 *
 * Revision 1.39  1995/04/20  16:58:44  jmc
 * Modified error messages to put filenames on fresh lines
 * since some filenames are really long.
 *
 * Revision 1.38  1995/04/14  17:26:45  jmc
 * Changed POPULATE-OBSTACLES to CREATE-OBSTACLES.
 *
 * Revision 1.37  1995/04/03  19:29:32  jmc
 * Added copyright notice.
 *
 * Revision 1.36  1995/02/15  19:51:47  jmc
 * Fixes to the regular expressions for "id" and "filename".  Added
 * tokens for SWEEPTO behavior.   Added tokens/scanning for new
 * formation types (LINE_LEFT, etc).
 *
 * Revision 1.35  1995/02/14  22:10:33  jmc
 * Adjusted syntax of id to let it end with an asterisk (to indicate
 * that the measure label shouldn't be drawn).
 *
 * Revision 1.34  1995/02/14  16:35:03  jmc
 * Fixed small error in filename syntax that kept filenames with
 * dashes in them from being scanned properly.
 *
 * Revision 1.33  1995/02/02  22:51:35  jmc
 * Added token for SET CYCLE-DURATION statement.  Improved the
 * cmdl_error functions so that they print different messages
 * depending on whether the error is in the user command or a file.
 *
 * Revision 1.32  1995/02/02  16:40:18  jmc
 * Added token for SET SHOW-MOVEMENT-VECTORS grammar.
 *
 * Revision 1.31  1995/01/26  23:33:55  jmc
 * To accommodate the new version of flex, had to remove most of the
 * "define yy* cmdl_*" defines since that is now handled as a flex
 * option.  Added the cmdl_wrap function since it is not provided by
 * default now.  Restored the "#define yylval cmdl_lval" define since
 * it appears necessary despite the flex prefix option.
 *
 * Revision 1.30  1995/01/26  21:26:50  jmc
 * Narrowed some error messages to avoid cropping problems on some
 * systems.
 *
 * Revision 1.29  1994/11/15  15:52:02  jmc
 * Changed NO_FORMATION to NO-FORMATION.
 *
 * Revision 1.28  1994/11/14  21:23:19  jmc
 * Added recognition of the NO_FORMATION formation syntax.
 *
 * Revision 1.27  1994/11/10  23:30:58  jmc
 * Added tokens for set "magnify robots" and "robot magnification"
 * statements.
 *
 * Revision 1.26  1994/11/07  17:50:12  jmc
 * Added a token for OBSTACLE.
 *
 * Revision 1.25  1994/11/04  15:25:46  jmc
 * Added token for SET DEBUG-SCHEDULER.
 *
 * Revision 1.24  1994/11/03  18:41:13  jmc
 * Added tokens for SET HIGHLIGHT-REPELLING-OBSTACLES.  Added
 * alternate token for SHOW-TRAILS (SHOW-ROBOT-TRAILS).
 *
 * Revision 1.23  1994/11/02  17:06:24  jmc
 * Added SEED token as well as system-seed for the system random
 * seed.
 *
 * Revision 1.22  1994/10/28  18:06:07  jmc
 * Added tokens for TIMEOUT to support the new OCCUPY UNTIL syntax.
 *
 * Revision 1.21  1994/10/27  21:40:08  jmc
 * Added tokens for several SET commands for SYSTEM-SEED,
 * OBSTACLE-COVERAGE, METERS-PER-PIXEL, MIN-OBSTACLE-RADIUS, and
 * MAX-OBSTACLE-RADIUS as well as for the POPULATE-OBSTACLES command.
 *
 * Revision 1.20  1994/10/25  14:35:02  jmc
 * Changed the way robots are handled.  A central repository of robot
 * information is now kept and the the "robot type" is simply an
 * index into that array (gt_robot_info).  Added flag
 * reading_robot_name to help disambiguate names/strings from
 * possible robot names.  Added hacks to id and qstring to check
 * strings for robot names as appropriate.  Removed all the old robot
 * names except ROBOT.  Added tokens for defining new robots via
 * NEW-ROBOT commands.  Added tokens for PRINT commands.  Worked on
 * making sure strings allocated via strdup get reclaimed eventually.
 * Added tokens for < and > for change in syntax to units
 * necessitated by the new robot syntax.
 *
 * Revision 1.19  1994/09/02  20:13:29  jmc
 * Fixed a minor #include ordering problem.
 *
 * Revision 1.18  1994/08/29  22:51:01  jmc
 * Added keyword for WAIT (for phase-line acknowledge clause).
 *
 * Revision 1.17  1994/08/29  17:52:20  jmc
 * Added tokens for parsing SET and QUIT commands.
 *
 * Revision 1.16  1994/08/26  16:01:20  jmc
 * Added syntax for setting show-trails, debug-simulator, and
 * debug-robots parameter values.
 *
 * Revision 1.15  1994/08/12  15:33:52  jmc
 * Allow speed tokens.  Also added '=' so that syntax like SPEED=3.4
 * can be recognized.
 *
 * Revision 1.14  1994/08/11  20:24:40  jmc
 * Changed UNTIL-KEY to UNTIL.  Added keywords for ON-COMPLETION,
 * ACKNOWLEDGE, and MESSAGE.
 *
 * Revision 1.13  1994/08/05  16:59:16  jmc
 * Modified {date} and {time} expressions to make leading zeros
 * optional.
 *
 * Revision 1.12  1994/08/04  21:06:22  jmc
 * Tweaked the {number} and {rtime} definitions to improve the
 * robustness of lexing relative time specs.
 *
 * Revision 1.11  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.10  1994/08/03  16:54:06  jmc
 * Minor hack to warn_user error message format.
 *
 * Revision 1.9  1994/08/02  20:05:48  jmc
 * Converted cmdl_error2 to cmdl_errorf with a format and arguments
 * like printf to simplify its use.
 *
 * Revision 1.8  1994/07/26  21:02:38  jmc
 * Added a second error routine cmdl_error2 for non-syntax errors.
 *
 * Revision 1.7  1994/07/26  18:49:39  jmc
 * Added cmdl_filename to keep a copy of the filename for better
 * syntax errors.  Modified cmdl_error/yyerror to use cmdl_filename.
 * Changed error message to use error dialog box.
 *
 * Revision 1.6  1994/07/22  17:15:26  jmc
 * Various modifications to allow manual entry of commands to be
 * executed immediately.  Added several cmdl_* variables for getting
 * commands from strings.  Redefined YY_INPUT to allow input to come
 * from a file or a string.  Added IMMEDIATE_COMMAND token.
 *
 * Revision 1.5  1994/07/20  17:28:06  jmc
 * Changed HUMMER to HMMVW and added synonym for HUMMER.
 *
 * Revision 1.4  1994/07/12  19:14:38  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.3  1994/07/12  18:56:56  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:07:54  jmc
 * Added RCS automatic identification strings
 *********************************************************************/
