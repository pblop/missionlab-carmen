%{
/*
     el_lex.l : lex patterns for reading an event log file

     By: Douglas C. MacKenzie

     Copyright 1995, Georgia Tech Research Corporation
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: el_lex.l,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "el_gram.tab.h"


/***************************************************************/

/* Get rid of "defined but not used" warnings from gcc */
#define YY_NO_UNPUT

/********************************************************************
*                                                                   *
*                        el_wrap                                    *
*                                                                   *
********************************************************************/

/* el_wrap will always return true, indicating that no other file follows */

#define YY_SKIP_YYWRAP
int
el_wrap(void)
{
   return 1;  /* Done */
}

/********************************************************************/

%}

ALPHA 				[$a-zA-Z]
NUMERIC	 			[0-9]

/********************************************************************/

/* %option yylineno */

/********************************************************************/
/* the patterns */
%%

start			 		{ 
   return(START); 
   }

end			 		{ 
   return(END); 
   }

status			 		{ 
   return(STATUS); 
   }

event			 		{ 
   return(EVENT); 
   }

cancel			 		{ 
   return(CANCEL); 
   }

StartModify		 		{ 
   return(STARTMODIFY); 
   }

EndModify		 		{ 
   return(ENDMODIFY); 
   }

{NUMERIC}+"."{NUMERIC}+":" 		{ 
   // A time stamp
   el_lval.time = atof(yytext);

   // Remember the last time we have seen so far so can close
   // any open events at the end of the file.
   last_time = el_lval.time;

   return(TIME); 
   }

{ALPHA}("_"|{ALPHA}|{NUMERIC})*		{ 
   el_lval.name = strdup(el_text);
   return NAME;
   }

"\"".*"\""				{ 
   el_lval.string = strdup(&el_text[1]);
   el_lval.string[strlen(el_lval.string) -1] = '\0';
   return STRING;
   }

[\t \r\n]+				{ /* eat up whitespace */ }

%%


/**********************************************************************
 * $Log: el_lex.l,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.6  1996/10/08  17:57:41  doug
 * flex upgrade allows getting rid of kludge
 *
 * Revision 1.5  1996/09/03  16:06:13  doug
 * fixed for linux
 *
 * Revision 1.4  1996/04/18 00:20:37  doug
 * *** empty log message ***
 *
 * Revision 1.3  1996/04/13  21:54:42  doug
 * *** empty log message ***
 *
 * Revision 1.2  1996/03/06  23:40:03  doug
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/05  22:24:51  doug
 * Initial revision
 *
 *
 **********************************************************************/
