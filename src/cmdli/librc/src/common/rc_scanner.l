%{
/**********************************************************************
 **                                                                  **
 **  rc_scanner.l                                                    **
 **                                                                  **
 **  flex scanner for resource files                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rc_scanner.l,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rc_scanner.l,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:34:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.7  2004/11/12 21:59:51  doug
// renamed yacc/bision stuff so doesn't conflict with missionlab
//
// Revision 1.6  2004/05/11 19:34:31  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.5  2002/10/10 20:22:10  doug
// Added save capabilities to the resource library
//
// Revision 1.4  1998/02/09 13:28:57  doug
// Added win32 changes
//
// Revision 1.3  1997/12/10 12:14:51  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/01 15:38:07  doug
// works
//
// Revision 1.1  1997/12/01 12:29:54  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <string.h>
#include <ctype.h>

#include "rc_parser.tab.h"
#include "rc_parser.h"
#include "resources.h"
#include "rc_scanner.h"

/***************************************************************/    
 
#define YY_NO_UNPUT

/********************************************************************/

%}

/* Tell flex to keep track of line numbers */
%option yylineno   
%option noyywrap
%option never-interactive

ALPHA 				[a-zA-Z]
NUMERIC	 			[0-9]
PUNC	 			[_/.\-+]

/* declare states */
%x ST_COMMENT

/********************************************************************/

/* the patterns */
%%

"=" 					return '=';
":" 					return SEP;
"," 					return SEP;

{ALPHA}("_"|{ALPHA}|{NUMERIC})*		{ 
   sara_rc_lval.str = strdup(sara_rc_text);
   return NAME;
   }

"\""[^\"]*"\""				{ 
   sara_rc_lval.str = strdup(&sara_rc_text[1]);
   sara_rc_lval.str[strlen(sara_rc_lval.str) -1] = '\0';
   return STRING;
   }

({PUNC}|{ALPHA}|{NUMERIC})*		{ 
   sara_rc_lval.str = strdup(sara_rc_text);
   return STRING;
   }

[ \t\r]+        			{ /* eat up whitespace */ }

"#"[^\n]*                              { /* handle comments */ }  

"/*"                            BEGIN(ST_COMMENT); /* handle C style comments */
<ST_COMMENT>[^*]*         /* eat anything that's not a '*' */
<ST_COMMENT>"*"+[^*/]*    /* eat up '*'s not followed by '/'s */
<ST_COMMENT>"*"+"/"        BEGIN(INITIAL); /* found the end of the comment */

"//"[^\n]*"\n"             /* handle C++ comments */

[ \t\r\n]+                                      /* eat up whitespace */



\n        				{ /* end of lines */ }

.					{ 
   char buf[256];
   if (isprint(sara_rc_text[0]))
      sprintf(buf, "Illegal character (%c)", sara_rc_text[0]);
   else
      sprintf(buf, "Illegal character (\\%d)", sara_rc_text[0]);

   sara_rc_SyntaxError(buf);
   }

%%

