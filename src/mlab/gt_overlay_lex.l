%{
/**********************************************************************
 **                                                                  **
 **                         gt_overlay_lex.l                         **
 **                                                                  **
 **                                                                  **
 **      Lexical scanner for overlay description language (odl)      **
 **                                                                  **
 **      See the MissionLab user's manual for details.               **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_overlay_lex.l,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <string.h>
}

#include "console.h"
#include "gt_measure.h"

#define yylval odl_lval
#include "gt_overlay_yac.h"

/***************************************************************/
/* Get rid of "defined but not used" warnings from gcc */
#define YY_NO_UNPUT 

/***************************************************************/

static int lineno = 1;

void odl_error(char *s);

char odl_filename[256];

%}


comment "--".*

id [a-zA-Z][a-zA-Z0-9_\-]*[\*]?

qstring \"[^\"\n]*[\"\n]

number [+-]?(([0-9]+)|([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))(([eE][-+]?[0-9]+)?)

control_measures CONTROL[\t ]+MEASURES:

ldlc ((LDLC)|("LD/LC"))

filename [a-zA-Z0-9_\-\.]*

%%

     /* TOKENS */

SCENARIO            { return SCENARIO;         }
SITE                { return SITE;             }
ORIGIN              { return ORIGIN;           }
GLOBAL_REF_POINT    { return GLOBAL_REF_POINT; }
HEADING             { return HEADING;          }
MISSION-AREA        { return MISSION_AREA;     }
LOAD-MAP            { return LOAD_MAP;         }
SET                 { return SET_TOKEN;        }
ZOOM-FACTOR         { return ZOOM_FACTOR_TOKEN;}
SCALE		    { return SCALE_TOKEN;      }
{control_measures}  { return CONTROL_MEASURES; }

AA                  { return AA_TOKEN;         }
ATK                 { return ATK_TOKEN;        } 
AXIS                { return AXIS_TOKEN;       }
BOUNDARY            { return BOUNDARY_TOKEN;   }
ATTRACTOR           { return ATTRACTOR_TOKEN;  }
OBJECT              { return OBJECT_TOKEN;  }
BASKET              { return BASKET_TOKEN;  }
BP                  { return BP_TOKEN;         }
GAP                 { return GAP_TOKEN;        }
HEIGHTMAP           { return HEIGHTMAP;        }
{ldlc}              { return LDLC_TOKEN;       }
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
      yylval.style = STYLE_MOVABLE;
      return OBJ_STYLE;        
   }

FIXED               { 
      yylval.style = STYLE_FIXED;
      return OBJ_STYLE;        
   }

CONTAINER           { 
      yylval.style = STYLE_CONTAINER;
      return OBJ_STYLE;        
   }


     /* QUOTED STRINGS */

{qstring}      { yylval.strval = strdup(yytext+1);  /* skip open quote */
                 if (yylval.strval[yyleng-2] != '"')
		    odl_error("Unterminated character string");
                 else /* remove close quote */
		    yylval.strval[yyleng-2] = '\0';
                 return QSTRING;
               }


     /* IDENTIFIERS */

{id}           { yylval.strval = strdup(yytext);
                 return IDENTIFIER;
               }


     /* NUMBERS */

{number}       { float tempnum;
                 if (sscanf(yytext, "%f", &tempnum) == 1) {
		    yylval.fval = tempnum;
                    return NUMBER;
                    }
                 else {
		    odl_error("Illegal number syntax");
		    yylval.fval = 0.0;
		    return NUMBER;
		    }
               }


     /* FILENAME */

{filename}           {
                 if ((yytext[0] == '-') && (yytext[1] == '-'))
                 {
                     /* It's a comment. Igore */
                 }
                 else
                 {
                     yylval.strval = strdup(yytext);
                     return FILENAME;
                 }
               }


     /* WHITE SPACE AND COMMENTS */

[\t ]+          ;  /* ignore whitespace */

{comment}       ;  /* ignore comments */

\n              {  lineno++; }

.               {  /* default */
                   printf("\nWarning, invalid character <%s> ignored in line %d\n",
			  yytext, lineno);
                }


%%

void odl_error(char *s)
{
   char msg[256];
   sprintf(msg, "Syntax error in overlay file\n   '%s'\n\
   at line %d:   %s at '%s'.", odl_filename, lineno, s, yytext);
   warn_user(msg);
}



void odl_errorf(const char *fmt, ...)
{
   va_list args;
   char msg[512];
   char err[256];

   va_start(args, fmt);
   vsprintf(err, fmt, args);
   va_end(args);
   sprintf(msg, "Error in overlay file\n   '%s' at line %d:\n    %s.", 
	   odl_filename, lineno, err);
   warn_user(msg);
}



int odl_wrap()
{
   return TRUE;
}



void odl_file_reset()
{
   lineno = 1;

   YY_NEW_FILE;
}



/**********************************************************************
 * $Log: gt_overlay_lex.l,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.4  2007/03/12 00:06:33  nadeem
 * Fixed code so that the mission area is expressed in meters internally for simulations.
 *
 * Revision 1.3  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
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
 * Revision 1.36  2000/07/07 18:18:32  endo
 * WAYPOINT-FILE added.
 *
 * Revision 1.35  2000/04/25 09:20:34  jdiaz
 * added HEADING
 *
 * Revision 1.34  2000/02/07 05:15:16  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.33  1999/08/31 15:44:29  endo
 * *** empty log message ***
 *
 * Revision 1.32  1999/08/30 21:34:52  jdiaz
 * added tokens for utm conversion
 *
 * Revision 1.31  1997/02/14 15:49:30  zchen
 * *** empty log message ***
 *
 * Revision 1.30  1996/10/09  17:07:19  doug
 * New version of lex allows getting rid of kludge
 *
 * Revision 1.29  1996/08/22  15:09:48  doug
 * added linux compatibility code
 *
 * Revision 1.28  1996/02/08 19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.27  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.26  1995/10/03  15:41:15  doug
 * added the attractor keyword
 *
 * Revision 1.25  1995/06/16  14:59:31  jmc
 * Added function odl_never_called() to eliminate compiler complaints
 * (with -Wall).
 *
 * Revision 1.24  1995/06/12  15:59:53  jmc
 * Added LOAD-MAP token.
 *
 * Revision 1.23  1995/05/06  00:42:34  jmc
 * Changed the tokens for METERS_PER_PIXEL to ZOOM_FACTOR.
 *
 * Revision 1.22  1995/05/02  21:20:59  jmc
 * Added tokens for SET METERS-PER-PIXEL statement.
 *
 * Revision 1.21  1995/05/02  19:43:08  jmc
 * Changed WORK-AREA to MISSION-AREA.
 *
 * Revision 1.20  1995/05/02  18:55:36  jmc
 * Added token for WORK-AREA statement.
 *
 * Revision 1.19  1995/04/20  18:17:09  jmc
 * Corrected spelling.
 *
 * Revision 1.18  1995/04/20  16:58:53  jmc
 * Modified error messages to put filenames on fresh lines
 * since some filenames are really long.
 *
 * Revision 1.17  1995/04/05  21:15:08  jmc
 * Added syntax for starting point grammar.
 *
 * Revision 1.16  1995/04/03  20:26:57  jmc
 * Added copyright notice.
 *
 * Revision 1.15  1995/02/15  19:49:31  jmc
 * Small modification to improve the regular expression for "id".
 *
 * Revision 1.14  1995/02/14  22:10:50  jmc
 * Adjusted syntax of id to let it end with an asterisk (to indicate
 * that the measure label shouldn't be drawn).
 *
 * Revision 1.13  1995/01/26  23:34:22  jmc
 * To accommodate the new version of flex, had to remove most of the
 * "define yy* odl_*" defines since that is now handled as a flex
 * option.  Added the odl_wrap function since it is not provided by
 * default now.  Restored the "#define yylval odl_lval" define since
 * it appears necessary despite the flex prefix option.
 *
 * Revision 1.12  1995/01/26  21:26:37  jmc
 * Narrowed some error messages to avoid cropping problems on some
 * systems.
 *
 * Revision 1.11  1994/11/07  15:32:03  jmc
 * Added a token for OBSTACLE.
 *
 * Revision 1.10  1994/09/03  13:52:07  jmc
 * Fixed minor problem with ordering of #includes.
 *
 * Revision 1.9  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.8  1994/08/03  16:55:53  jmc
 * Added odl_errorf function for invoking the error dialog with a
 * printf style format.  Minor hack to format of error messages.
 *
 * Revision 1.7  1994/07/26  18:48:57  jmc
 * Tweaked odl_error/yyerror message.
 *
 * Revision 1.6  1994/07/26  18:32:21  jmc
 * Added odl_filename to keep a copy of the filename for better
 * syntax errors.  Modified odl_error/yyerror to use odl_filename.
 * Changed error message to use error dialog box.
 *
 * Revision 1.5  1994/07/20  17:28:06  jmc
 * Tweaked syntax for LD/LC.
 *
 * Revision 1.4  1994/07/20  17:16:20  jmc
 * Added "LD/LC" synonym for line of departure/line of contact control measure.
 *
 * Revision 1.3  1994/07/12  19:22:19  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  17:27:34  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
