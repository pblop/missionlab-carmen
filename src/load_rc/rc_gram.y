/*
     rc_gram.y ----  yacc grammer for rc files

     By: Douglas C. MacKenzie

     Copyright 1995, Georgia Tech Research Corporation
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: rc_gram.y,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


%{
#include <assert.h>
#include <string>
#include "rc_defs.h"
#include "string_utils.h"

using std::string;

void rc_error(char *s);

bool rc_had_error = false;

/********************************************************************
*								    *
*		         rc_SyntaxError			            *
*								    *
********************************************************************/

/* rc_SyntaxError will print the error message to stderr */

void
rc_SyntaxError(const char *Message)
{
   cerr << "ERROR: " << _rc_filename << ", line " << rc_lineno << '\n';
   cerr << Message << '\n';
   cerr << rc_linebuf << '\n';

   for (int i = 0; i < rc_tokenpos - rc_tokenlen; i++)
      cerr << ' ';

   for (int i = 0; i < rc_tokenlen; i++)
      cerr << '^';

   cerr << '\n';

   rc_had_error = true;
}

%}

%union {
   char     *string;
   int       number;
   rc_chain *val_chain;
}

/* Define the tokens that lex will return */
%token SEP  
%token ASSIGN 

%token <string> NAME 
%token <string> STRING 
%type <val_chain> RHS

/************************** Start of CDL Grammer *****************************/
%%

Start		: Start Rule
                | Rule
		| error 	{ rc_SyntaxError("Ignoring definition"); }
		;

Rule		: NAME ASSIGN RHS {
      _rc_table->put($1,$3);
   }
                | NAME ASSIGN {
      rc_SyntaxError("Missing right hand side");
   }
                | NAME {
      rc_SyntaxError("Extra name");
   }
		;

RHS		: NAME         {
      string data = $1;
      data = replaceStringInString(data, "$MLAB_HOME", MLAB_HOME);
      $$ = new rc_chain(strdup(data.c_str()));
      //$$ = new rc_chain($1);
   }
		| STRING       {
      string data = $1;
      data = replaceStringInString(data, "$MLAB_HOME", MLAB_HOME);
      $$ = new rc_chain(strdup(data.c_str()));
      //$$ = new rc_chain($1);
   }
		| RHS SEP NAME {
      string data = $3;
      data = replaceStringInString(data, "$MLAB_HOME", MLAB_HOME);
      $$->append(strdup(data.c_str()));
      //$$->append($3);
   }
		| RHS SEP STRING {
      string data = $3;
      data = replaceStringInString(data, "$MLAB_HOME", MLAB_HOME);
      $$->append(strdup(data.c_str()));
      //$$->append($3);
   }
		| error        {
      rc_SyntaxError("Incorrect right hand side"); 
   }
		;


%%
/*=========================== End of CDL Grammer ============================*/

/************************************************************************/
/* rc_error is called by yacc when it finds an error. */

void rc_error(char *s)
{
   rc_SyntaxError(s);
}




/**********************************************************************
 * $Log: rc_gram.y,v $
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
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.6  1996/05/06  03:05:51  doug
 * fixing compiler warnings
 *
 * Revision 1.5  1996/03/06  23:40:22  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/08/24  19:32:51  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/08/24  16:02:44  doug
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/24  15:45:24  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/08/24  15:14:14  doug
 * Initial revision
 *
 **********************************************************************/
