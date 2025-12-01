%{
/*
     rc_lex.l : lex patterns for reading a resource file

     By: Douglas C. MacKenzie

     Copyright 1995, Georgia Tech Research Corporation
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: rc_lex.l,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "rc_defs.h"
#include "rc_gram.tab.h"


//int rc_lineno = 1; i  //Old compilation error: rc_lineno redefinition
int rc_tokenpos = 0;
int rc_tokenlen = 0;

const int MAX_LINE_LEN = 512;
char rc_linebuf[MAX_LINE_LEN];

//******************************************************************
// Get rid of undefined errors from flex
#define YY_NO_UNPUT


/********************************************************************
*                                                                   *
*                        rc_wrap                                    *
*                                                                   *
********************************************************************/

/* rc_wrap will always return true, indicating that no other file follows */

#define YY_SKIP_YYWRAP 
int
rc_wrap(void)
{
   return 1;  /* Done */
}

/********************************************************************/

%}

ALPHA 				[a-zA-Z]
NUMERIC	 			[0-9]
PUNC	 			[_/.\-+]

/********************************************************************/

/* the patterns */
%%

":" 					{ 
   rc_tokenlen = yyleng;
   rc_tokenpos+=yyleng;
   return(SEP); 
   }

"," 					{ 
   rc_tokenlen = yyleng;
   rc_tokenpos+=yyleng;
   return(SEP); 
   }

"=" 					{ 
   rc_tokenlen = yyleng;
   rc_tokenpos+=yyleng;
   return(ASSIGN); 
   }

{ALPHA}("_"|"."|{ALPHA}|{NUMERIC}|"$")*	{ 
   rc_tokenlen = yyleng;
   rc_tokenpos+=yyleng;
   rc_lval.string = strdup(rc_text);
   return NAME;
   }

"\"".*"\""				{ 
   rc_tokenlen = yyleng;
   rc_tokenpos +=yyleng;

   rc_lval.string = strdup(&rc_text[1]);
   rc_lval.string[strlen(rc_lval.string) -1] = '\0';
   return STRING;
   }

({PUNC}|{ALPHA}|{NUMERIC}|"$")*		{ 
   rc_tokenlen = yyleng;
   rc_tokenpos+=yyleng;
   rc_lval.string = strdup(rc_text);
   return STRING;
   }

"\t"					{ 
   /* figure out where next tab stop is located */ 
   int tabstop = 8 * ((rc_tokenpos + 8) / 8);
   rc_tokenlen = tabstop - rc_tokenpos;
   rc_tokenpos = tabstop;
   }

[ \r]+        				{ 
   rc_tokenlen = rc_leng;
   rc_tokenpos+=rc_leng;
   /* eat up whitespace */ 
   }

"#"[^\n]*                              { 
   rc_tokenlen = rc_leng;
   rc_tokenpos+=rc_leng;
   /* handle comments */ 
   }  



\n.*        				{ 
   if( rc_leng >= MAX_LINE_LEN )
   {
      rc_SyntaxError("Length of line too long: Skipping buffering\n");
      memcpy(rc_linebuf, rc_text+1, MAX_LINE_LEN-1);
      rc_linebuf[MAX_LINE_LEN-1] = '\0';
   }
   else
   {
      strcpy(rc_linebuf, rc_text+1); 	// save the next line 
   }
   rc_lineno++; 			// count them 
   rc_tokenpos = 0; 		// Start of line
   rc_tokenlen = 0;

   yyless(1); 			// give back all but the \n to rescan
   
   // Remind lex we are at the begining of a line so the caret works 
   yy_set_bol(true);
   }

.					{ 
   rc_tokenlen = rc_leng;
   rc_tokenpos+=rc_leng;

   char buf[256];
   if (isprint(rc_text[0]))
      sprintf(buf, "Illegal character (%c)", rc_text[0]);
   else
      sprintf(buf, "Illegal character (\\%d)", rc_text[0]);

   rc_SyntaxError(buf);
   }

%%


/**********************************************************************
 * $Log: rc_lex.l,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/06/18 19:46:11  endo
 * .cfgeditrc now accepts MLAB_HOME
 *
 * Revision 1.1.1.1  2005/02/06 23:00:08  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.8  1999/09/03 20:15:00  endo
 * rule modified, so that it accepts "." inside
 * a name, such as "agents.AuRA.urban".
 *
 * Revision 1.7  1996/10/09 15:45:13  doug
 * *** empty log message ***
 *
 * Revision 1.6  1996/10/04  21:13:55  doug
 * changes to get to version 1.0c
 *
 * Revision 1.5  1995/10/30 21:35:17  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/10/11  22:05:16  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/08/24  19:32:51  doug
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/24  15:45:24  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/08/24  14:17:17  doug
 * Initial revision
 *
 **********************************************************************/
