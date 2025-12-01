%{
/**********************************************************************
 **                                                                  **
 **  env_scanner.l                                                   **
 **                                                                  **
 **  flex scanner for resource files                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **********************************************************************/

/* $Id: env_scanner.l,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: env_scanner.l,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:21:03  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.8  2004/05/11 19:34:39  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.7  2003/11/03 16:41:56  doug
// working on libcmdl
//
// Revision 1.6  2002/11/20 22:17:28  doug
// can assign loiter areas in 2d
//
// Revision 1.5  2002/11/13 17:15:50  doug
// objectives work!
//
// Revision 1.4  2002/10/02 15:51:48  doug
// moving to cvs
//
// Revision 1.3  2002/09/18 17:27:29  doug
// *** empty log message ***
//
// Revision 1.2  2002/06/13 15:19:51  doug
// *** empty log message ***
//
// Revision 1.1  1998/06/16 14:52:15  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <ctype.h>

#include "env_scanner.h"
#include "env_parser.h"

using namespace std;
using namespace sara;		// make all sara names visible in this file
/***************************************************************/    
 
#define YY_NO_UNPUT

/********************************************************************/

%}

/* Tell flex to keep track of line numbers */
%option yylineno   
%option noyywrap
%option never-interactive
%option case-insensitive

ALPHA 				[a-zA-Z]
NUMERIC	 			[0-9]

id [a-zA-Z][a-zA-Z0-9_\-]*[\*]?

number [+-]?(([0-9]+)|([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))(([eE][-+]?[0-9]+)?)

/********************************************************************/
                                                                                                         
/* declare states */
%x ST_COMMENT

/********************************************************************/

/* the patterns */
%%

"=" 		return '=';
":" 		return ':';
"," 		return ',';
";" 		return ';';
"<" 		return '<';
">" 		return '>';
disk 		return DISK;
polygon		return POLYGON;
at 		return AT;
radius 		return RADIUS;
color 		return COLOR;
photo 		return PHOTO;
MetersPerPixel 	return METERSPERPIXEL;
metersperpixel 	return METERSPERPIXEL;


SCENARIO            { return SCENARIO;         }
SITE                { return SITE;             }
ORIGIN              { return ORIGIN;           }
HEADING             { return HEADING;          }
MISSION-AREA        { return MISSION_AREA;     }
LOAD-MAP            { return LOAD_MAP;         }
SET                 { return SET_TOKEN;        }
CONTROL[\t ]+MEASURES+[:] { return CONTROL_MEASURES; }
AA                  { return AA_TOKEN;         }
assembly-area       { return AA_TOKEN;         }
LA                  { return LA_TOKEN;         }
loiter-area         { return LA_TOKEN;         }
ATK                 { return ATK_TOKEN;        }
AXIS                { return AXIS_TOKEN;       }
BOUNDARY            { return BOUNDARY_TOKEN;   }
ATTRACTOR           { return ATTRACTOR_TOKEN;  }
OBJECT              { return OBJECT_TOKEN;  }
BASKET              { return BASKET_TOKEN;  }
BP                  { return BP_TOKEN;         }
GAP                 { return GAP_TOKEN;        }
((LDLC)|("LD/LC"))  { return LDLC_TOKEN;       }
OBJ                 { return OBJ_TOKEN;        }
OBSTACLE            { return OBSTACLE_TOKEN;   }
PL                  { return PL_TOKEN;         }
PP                  { return PP_TOKEN;         }
SP                  { return SP_TOKEN;         }
LAT                 { return LAT_TOKEN;        }
LON                 { return LON_TOKEN;        }
UTM                 { return UTM_TOKEN;        }
WALL                { return WALL_TOKEN;       }
ROOM                { return ROOM_TOKEN;       }
DOOR                { return DOOR_TOKEN;       }
HALLWAY             { return HALLWAY_TOKEN;    }
CENTERLINE          { return CENTERLINE_TOKEN; }
WIDTH               { return WIDTH_TOKEN;      }
EXTENTS             { return EXTENTS_TOKEN;    }
WAYPOINT-FILE       { return WAYPOINT_FILE_TOKEN; }
MOVABLE             { 
      env_lval.style = STYLE_MOVABLE;
      return OBJ_STYLE;        
   }

FIXED               { 
      env_lval.style = STYLE_FIXED;
      return OBJ_STYLE;        
   }

CONTAINER           { 
      env_lval.style = STYLE_CONTAINER;
      return OBJ_STYLE;        
   }


     /* QUOTED STRINGS */

\"[^\"\n]*[\"\n] { 
                 if (yytext[yyleng-1] != '"')
		    env_SyntaxError("Unterminated character string");
                 env_lval.str = new string(yytext+1, yyleng-2);  /* skip quotes */
                 return STRING;
               }


     /* IDENTIFIERS */

{id}           { env_lval.str = new string(yytext);
                 return STRING;
               }


     /* NUMBERS */

{number}       {
   char *rtn;
   env_lval.number = strtod(yytext, &rtn);
   if( rtn == yytext || *rtn != '\0')
   {
      env_SyntaxError("Illegal number syntax");
      env_lval.number = 0.0;
   }

   return NUMBER;
   }

	/* Eat whitespace and comments */
[ \t\r]+        { /* eat up whitespace */ }

"#"[^\n]*       { /* handle one line comments */ }  
"//"[^\n]*      { /* handle one line comments */ }  
"--"[^\n]*      { /* handle one line comments */ }  
"/*"            BEGIN(ST_COMMENT); /* handle C style comments */
<ST_COMMENT>[^*]*         /* eat anything that's not a '*' */
<ST_COMMENT>"*"+[^*/]*    /* eat up '*'s not followed by '/'s */
<ST_COMMENT>"*"+"/"        BEGIN(INITIAL); /* found the end of the comment */


\n        	{ /* end of lines */ }

.					{ 
   char buf[256];
   if (isprint(env_text[0]))
      sprintf(buf, "Illegal character (%c)", env_text[0]);
   else
      sprintf(buf, "Illegal character (\\%d)", env_text[0]);

   env_SyntaxError(buf);
   }

%%

