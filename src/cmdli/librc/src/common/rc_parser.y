/************************************************************************/
%{

/**********************************************************************
 **                                                                  **
 **  rc_parser.y                                                     **
 **                                                                  **
 **  bison parser for the resource files                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rc_parser.y,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rc_parser.y,v $
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
// Revision 1.5  1998/06/18 21:14:28  doug
// fixed error reporting
//
// Revision 1.4  1997/12/10 12:14:51  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/08 14:58:47  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/01 15:38:07  doug
// parser for the resource files
//
// Revision 1.1  1997/12/01 12:36:09  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

/************************************************************************/
 
#include "mic.h"
#include <string>
  
#include "rc_scanner.h" 
#include "rc_parser.h"

using namespace sara;
   
void sara_rc_error(char *s);

bool sara_rc_had_error = false;
const string *sara_rc_filename;
res_T  *sara_rc_table = NULL;

/************************************************************************/
// rc_error is called by yacc when it finds an error.
void sara_rc_error(char *s)
{
   sara_rc_SyntaxError(s);
}

/********************************************************************/
// rc_SyntaxError prints an error message
void sara_rc_SyntaxError(const char *msg)
{
   ERROR("Syntax error loading resource file %s, line %d: %s", 
		sara_rc_filename->c_str(), sara_rc_lineno, msg);

   sara_rc_had_error = true;
}

/********************************************************************/

%}

%union {
   char     *str;
   int       number;
   sara::strings  *val_chain;
}

/* Define the tokens that lex will return */
%token SEP  
%token ASSIGN 

%token <str> NAME 
%token <str> STRING 
%type <val_chain> RHS

/************************** Start of CDL Grammer *****************************/
%%

Start		: Rules
			| /* empty */
			;

Rules		: Rules Rule
         | Rule
			| error 	{ sara_rc_SyntaxError("Ignoring definition"); }
			;

Rule		: NAME '=' RHS {
      (*sara_rc_table)[$1] = *($3);
   }
                | NAME '=' {
      sara_rc_SyntaxError("Missing right hand side");
   }
                | NAME {
      sara_rc_SyntaxError("Extra name");
   }
		;

RHS		: NAME         {
      $$ = new strings;
      $$->push_back((string)$1);
   }
		| STRING       {
      $$ = new strings;
      $$->push_back((string)$1);
   }
		| RHS SEP NAME {
      $$->push_back((string)$3);
   }
		| RHS SEP STRING {
      $$->push_back((string)$3);
   }
		| error        {
      sara_rc_SyntaxError("Incorrect right hand side"); 
   }
		;


%%
/***********************************************************************/
