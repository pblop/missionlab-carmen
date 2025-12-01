%{
/**********************************************************************
 **  cmdl_scanner.l                                                  **
 **                                                                  **
 **  Lexical scanner for cmdl (command description language)         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **                                                                  **
 **  Based on: gt_command_lex.l                                      **
 **     Written by:  Jonathan M. Cameron                             **
 **     Copyright 1995, 1996, 1997 Georgia Tech Research Corporation **
 **     Atlanta, Georgia  30332-0415                                 **
 **     ALL RIGHTS RESERVED, See file COPYRIGHT for details.         **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_scanner.l,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $  */

 /********************************************************************
 * $Log: cmdl_scanner.l,v $
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
 * Revision 1.19  2004/11/12 21:56:59  doug
 * renamed loader so doesn't conflict with missionlab
 *
 * Revision 1.18  2004/10/25 22:58:16  doug
 * working on mars2020 integrated demo
 *
 * Revision 1.17  2004/10/22 21:40:29  doug
 * added goto and command exception support
 *
 * Revision 1.16  2004/07/30 13:41:32  doug
 * support sync command
 *
 * Revision 1.15  2004/05/11 19:34:49  doug
 * massive changes to integrate with USC and GaTech
 *
 * Revision 1.14  2004/03/19 22:48:21  doug
 * working on exceptions.
 *
 * Revision 1.13  2004/03/18 22:11:17  doug
 * coded call and return
 *
 * Revision 1.12  2004/03/15 21:41:06  doug
 * coded ActionCommand and Call
 *
 * Revision 1.11  2004/03/11 19:43:42  doug
 * splitting off from Command
 *
 * Revision 1.10  2004/03/08 14:52:34  doug
 * cross compiles on visual C++
 *
 * Revision 1.9  2004/03/01 23:37:51  doug
 * working on real overlay
 *
 * Revision 1.8  2004/02/28 16:35:42  doug
 * getting cmdli to work in sara
 *
 * Revision 1.7  2003/11/10 22:48:30  doug
 * working on cmdli
 *
 * Revision 1.6  2003/11/07 22:54:51  doug
 * working on cmdl_io
 *
 * Revision 1.5  2003/11/07 20:29:27  doug
 * added cmdli interpreter library
 *
 * Revision 1.4  2003/11/03 16:43:31  doug
 * snapshot
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
 *********************************************************************/

/* rename the yylloc variable so can include two scanners */
#ifndef yylloc
#define yylloc sara_cmdl_lloc
#endif

#include "mic.h"
#include "cmdl_parser.h"

using namespace sara;		// make all sara names visible in this file

/********************************************************************
*                                                                   *
*                        yyinit                                     *
*                                                                   *
********************************************************************/

static YY_BUFFER_STATE inputbuf = NULL;

// yyinit will get ready to scan a buffer from memory
void
sara_cmdl_init(const char *buffer, const int length)
{
   inputbuf = yy_scan_bytes(buffer, length);
}

/********************************************************************
*                                                                   *
*                        yyexit                                     *
*                                                                   *
********************************************************************/

// yyexit will free the allocated internal buffer
void
sara_cmdl_exit()
{
   if( inputbuf )
      yy_delete_buffer(inputbuf);
   inputbuf = NULL;
}

%}

/********************************************************************/
/* Declare the flex options */
                                                                                
/* Keep track of line numbers */
%option yylineno
                                                                                
/* Get rid of a defined but not used warning. */
%option nounput
                                                                                
/* Keep lex from declaring a C style prototype for ccl_wrap */
%option noyywrap
                                                                                
/* No default acceptance rules */
%option nodefault
                                                                                
/* Enable debug mode */
%option debug
                                                                                
/* go ahead and be efficient */
%option never-interactive

/* we are ignoring case on keywords */
%option case-insensitive 
                                                                                
/**************************************************************/
/* Define some macros */

id [a-zA-Z][a-zA-Z0-9_\-]*[\*]?

filename [/a-zA-Z0-9\.][/a-zA-Z0-9\-\._]*[~]?

number -?(([0-9]+)|([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))(([eE][-+]?[0-9]+)?)

qstring \"[^\"\n]*[\"\n]

/********************************************************************/
                                                                                
/* declare states */
%x ST_COMMENT

/*************************************************************/
/* Define the regular expressions to extract tokens */
%%

MISSION[\t ]+NAME   	{ yylloc.first_line = sara_cmdl_lineno; return MISSION_NAME;  }
SCENARIO                { yylloc.first_line = sara_cmdl_lineno; return SCENARIO;      }
CALL             { yylloc.first_line = sara_cmdl_lineno; return CALL_TOKEN;        }
GOTO             { yylloc.first_line = sara_cmdl_lineno; return GOTO_TOKEN;        }
SYNCHRONIZE      { yylloc.first_line = sara_cmdl_lineno; return SYNC_TOKEN;        }
SYNC             { yylloc.first_line = sara_cmdl_lineno; return SYNC_TOKEN;        }
RETURN           { yylloc.first_line = sara_cmdl_lineno; return RETURN_TOKEN;      }
MISSION-AREA     { yylloc.first_line = sara_cmdl_lineno; return MISSION_AREA;      }
LOAD-MAP         { yylloc.first_line = sara_cmdl_lineno; return LOAD_MAP;          }
SP               { yylloc.first_line = sara_cmdl_lineno; return SP;                }
UNIT             { yylloc.first_line = sara_cmdl_lineno; return UNIT_TOKEN;        }
IF               { yylloc.first_line = sara_cmdl_lineno; return IF_TOKEN;          }

NEW-ROBOT        { yylloc.first_line = sara_cmdl_lineno; return NEW_ROBOT;         }

CREATE-OBSTACLES { yylloc.first_line = sara_cmdl_lineno; return CREATE_OBSTACLES;  }
OBSTACLE         { yylloc.first_line = sara_cmdl_lineno; return OBSTACLE_TOKEN;    }

COMMAND[\t ]+LIST: { yylloc.first_line = sara_cmdl_lineno; return START_COMMANDS;    }
IMMEDIATE[\t ]+COMMAND:  { yylloc.first_line = sara_cmdl_lineno; return IMMEDIATE_COMMAND; }

(TELEOPERATE)|(TELOP) {yylloc.first_line = sara_cmdl_lineno; return TELEOPERATE_TOKEN;}
SWEEPTO          { yylloc.first_line = sara_cmdl_lineno; return SWEEPTO_TOKEN;     }
OCCUPY         	 { yylloc.first_line = sara_cmdl_lineno; return OCCUPY_TOKEN;      }
FOLLOW         	 { yylloc.first_line = sara_cmdl_lineno; return FOLLOW_TOKEN;      }
QUIT             { yylloc.first_line = sara_cmdl_lineno; return QUIT_TOKEN;        }
SET              { yylloc.first_line = sara_cmdl_lineno; return SET_TOKEN;         }

PRINT            { yylloc.first_line = sara_cmdl_lineno; return PRINT_TOKEN;       }
(CONSOLE-DATABASE)|(CONSOLE-DB)  { yylloc.first_line = sara_cmdl_lineno; return CONSOLE_DB_TOKEN;  }
ROBOT-INFO       { yylloc.first_line = sara_cmdl_lineno; return ROBOT_INFO_TOKEN;  }

AND            	 { yylloc.first_line = sara_cmdl_lineno; return AND;               }
	       	                           
FORMATION      	 { yylloc.first_line = sara_cmdl_lineno; return FORMATION;         }
TECHNIQUE      	 { yylloc.first_line = sara_cmdl_lineno; return TECHNIQUE;         }
SPEED            { yylloc.first_line = sara_cmdl_lineno; return SPEED;             }
PHASE-LINE     	 { yylloc.first_line = sara_cmdl_lineno; return PHASE_LINE_TOKEN;  }
UNTIL      	 { yylloc.first_line = sara_cmdl_lineno; return UNTIL;             }
defaultSyncTimeout      { yylloc.first_line = sara_cmdl_lineno; return DEFAULTSYNCTIMEOUT;  }
ON-COMPLETION    { yylloc.first_line = sara_cmdl_lineno; return ON_COMPLETION;     }
(ACKNOWLEDGE)|(ACK) { yylloc.first_line = sara_cmdl_lineno; return ACKNOWLEDGE;    }
(MESSAGE)|(MSG)  { yylloc.first_line = sara_cmdl_lineno; return MESSAGE;           }

(SEED|SYSTEM-SEED)    { yylloc.first_line = sara_cmdl_lineno; return SEED_TOKEN;                }
(SHOW-TRAILS)|(SHOW-ROBOT-TRAILS) { yylloc.first_line = sara_cmdl_lineno; return SHOW_TRAILS_TOKEN;         }
(HIGHLIGHT-REPELLING-OBSTACLES)|(SHOW-REPELLING-OBSTACLES) { 
		yylloc.first_line = sara_cmdl_lineno; 
		return SHOW_REP_OBST_TOKEN;       
	}
SHOW-MOVEMENT-VECTORS { yylloc.first_line = sara_cmdl_lineno; return SHOW_MOV_VECT_TOKEN;       }
SHOW-COLORMAP         { yylloc.first_line = sara_cmdl_lineno; return SHOW_COLORMAP_TOKEN;       }
OBSTACLE-COVERAGE     { yylloc.first_line = sara_cmdl_lineno; return OBSTACLE_COVERAGE_TOKEN;   }
ZOOM-FACTOR           { yylloc.first_line = sara_cmdl_lineno; return ZOOM_FACTOR_TOKEN;         }
SCALE-ROBOTS          { yylloc.first_line = sara_cmdl_lineno; return SCALE_ROBOTS_TOKEN;        }
ROBOT-LENGTH          { yylloc.first_line = sara_cmdl_lineno; return ROBOT_LENGTH_TOKEN;        }
MIN-OBSTACLE-RADIUS   { yylloc.first_line = sara_cmdl_lineno; return MIN_OBSTACLE_RADIUS_TOKEN; }
MAX-OBSTACLE-RADIUS   { yylloc.first_line = sara_cmdl_lineno; return MAX_OBSTACLE_RADIUS_TOKEN; }
CYCLE-DURATION        { yylloc.first_line = sara_cmdl_lineno; return CYCLE_DURATION_TOKEN;      }
DEBUG-ROBOTS          { yylloc.first_line = sara_cmdl_lineno; return DEBUG_ROBOTS_TOKEN;        }
DEBUG-SIMULATOR       { yylloc.first_line = sara_cmdl_lineno; return DEBUG_SIMULATOR_TOKEN;     }

DEBUG-SCHEDULER       				{ 
		yylloc.first_line = sara_cmdl_lineno; 
		return DEBUG_SCHEDULER_TOKEN;     
		}

defprocedure	{ yylloc.first_line = sara_cmdl_lineno; return DEFPROCEDURE;  }
endprocedure	{ yylloc.first_line = sara_cmdl_lineno; return ENDPROCEDURE;  }

(ON|TRUE|YES)    { sara_cmdl_lval.boolean = true;
                   yylloc.first_line = sara_cmdl_lineno; return BOOLVAL;           }
(OFF|FALSE|NO)   { sara_cmdl_lval.boolean = false;
                   yylloc.first_line = sara_cmdl_lineno; return BOOLVAL;           }


     /* TIME */

[0-9]?[0-9]-[0-9][0-9]-[0-9][0-9] 	{ 
		sara_cmdl_lval.String = new string(sara_cmdl_text);
                yylloc.first_line = sara_cmdl_lineno; return DATEVAL;
		}

([0-9]?[0-9]:[0-9][0-9])|([0-9]?[0-9]:[0-9][0-9]:[0-9][0-9]) 	{
		sara_cmdl_lval.String = new string(sara_cmdl_text);
                yylloc.first_line = sara_cmdl_lineno; return TIME;
		}

(\+[0-9]?[0-9]:[0-9][0-9])|(\+[0-9]?[0-9]:[0-9][0-9]:[0-9][0-9])|(\+[0-9]+)  {
		sara_cmdl_lval.String = new string(sara_cmdl_text);
		yylloc.first_line = sara_cmdl_lineno; return TIME;
		}


     /* block labels */

[a-zA-Z0-9_][a-zA-Z0-9_]*\.	{
		sara_cmdl_lval.String = new string(sara_cmdl_text, strlen(sara_cmdl_text)-1);
		yylloc.first_line = sara_cmdl_lineno; 
                return LABEL;
		}


     /* NUMBERS */

{number}       { float tempnum;
                 if (sscanf(sara_cmdl_text, "%f", &tempnum) == 1) {
		    sara_cmdl_lval.number = tempnum;
                    yylloc.first_line = sara_cmdl_lineno; return NUMBER;
                    }
                 else {
		    sara_cmdl_error(sara_cmdl_lineno, "Illegal number syntax");
		    sara_cmdl_lval.number = 0.0;
		    yylloc.first_line = sara_cmdl_lineno; 
		    return NUMBER;
		    }
               }



     /* IDENTIFIERS */

{id}           {
		sara_cmdl_lval.String = new string(sara_cmdl_text);
		yylloc.first_line = sara_cmdl_lineno; 
		return STRING;
               }


     /* QUOTED STRINGS */

{qstring}      {
		if (sara_cmdl_text[yyleng-1] != '"')
		   sara_cmdl_error(sara_cmdl_lineno, "Unterminated character string");
		else 
                {
                   // prune leading and trailing quotes
		   sara_cmdl_lval.String = new string(sara_cmdl_text+1, strlen(sara_cmdl_text+1)-1);
           	}
		yylloc.first_line = sara_cmdl_lineno; 
		return STRING;
               }

     /* WHITE SPACE AND COMMENTS */

"/*"            BEGIN(ST_COMMENT); /* handle C style comments */
<ST_COMMENT>[^*\n]*         /* eat anything on this line that is not a '*' */
<ST_COMMENT>"*"+[^*/\n]*    /* eat a '*' on this line not followed by '/'s or '*'s */
<ST_COMMENT>"\n"            /* move to the next line */
<ST_COMMENT>"*"+"/"        BEGIN(INITIAL); /* found the end of the comment */
                                                                                
                                                                                
"//"[^\n]*"\n"  /* handle C++ style comments */
"--"[^\n]*"\n"  /* handle cmdl style comments */
                                                                                
[ \t\r\n]+      /* eat up whitespace */

.       {
        yylloc.first_line = sara_cmdl_lineno;
        return sara_cmdl_text[0];     /* pass it to the parser */
        }

%%

// *********************************************************************
