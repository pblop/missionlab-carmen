/*
     el_gram.y ----  yacc grammer for event log files

     By: Douglas C. MacKenzie

     Copyright 1995, Georgia Tech Research Corporation
     Atlanta, Georgia  30332-0415
     ALL RIGHTS RESERVED, See file COPYRIGHT for details.
*/

/* $Id: el_gram.y,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


%{
//#include <iostream.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <iostream>

#include "event_list.hpp"
#include "utilities.h"
#include "defs.h"

using std::cerr;

void el_error(char *s);
bool had_error = false;

/********************************************************************
*								    *
*		         el_SyntaxError			            *
*								    *
********************************************************************/

/* el_SyntaxError will print the error message to stderr */

void
el_SyntaxError(const char *Message)
{
   // cerr << "Syntax Error in Line " << yylineno << ": " << Message << '\n';
   cerr << "Syntax Error: " << Message << '\n';
   had_error = true;
}


/********************************************************************
*								    *
*		         semantic_error			            *
*								    *
********************************************************************/

/* semantic_error will print the error message to stderr */

void
semantic_error(const char *fmt, ...)
{
   va_list args;
   char msg[2048];

   va_start(args, fmt);
   vsprintf(msg, fmt, args);
   va_end(args);

   cerr << "Error: " << msg << '\n';
   had_error = true;
}

void
CancelEvent(double time, char *name, char *name2=NULL)
{
   // Need to find the start of this event so we can close it.
   // Also, this closes all open events down to this one's start

   event_list<events *>  subs;
   while(1)
   {
      if( open_stack.isempty() )
      {
         semantic_error("Unable to find start for event %s canceled at %.f",
		name, time);
	 break;
      }

      events *p = open_stack.pop();

      // If it is our start, then we are done.
      if( p->is_start_for(name) || (name2 && p->is_start_for(name2)))
      {
	    // Cancel this event and connect up the subevents.
	    p->cancel(time, subs);

            // Add us to the time line
	    time_line.append(p);
	    break;
      }
      else
      {
	    // Cancel the event 
	    p->cancel(time);

	    // The event is one of our subs
            subs.append(p);
      }
   }
}
void
EndEvents(double time, char *name, char *name2=NULL)
{
   // Need to find the start of this event so we can close it.
   // Also, this closes all open events down to this one's start

   event_list<events *>  subs;
   while(1)
   {
      if( open_stack.isempty() )
      {
         semantic_error("Unable to find start for event %s ending at %.f",
		name, time);
	 break;
      }

      events *p = open_stack.pop();

      // If it is our start, then we are done.
      if( p->is_start_for(name) || (name2 && p->is_start_for(name2)))
      {
	    // Finish this event and connect up the subevents.
	    p->finish(time, subs);

            // Add us to the time line
	    time_line.append(p);
	    break;
      }
      else
      {
	    // The event is done 
	    p->finish(time);

	    // The event is one of our subs
            subs.append(p);
      }
   }
}

%}

%union {
   char     *string;
   double    time;
   char     *name;
}

/* Define the tokens that lex will return */
%token <name>   NAME 
%token <string> STRING 
%token <time>   TIME

%token START 
%token END 
%token STATUS 
%token EVENT 
%token CANCEL 
%token STARTMODIFY 
%token ENDMODIFY 

/********************** Start of EventLog Grammer *****************************/
%%

Start		: Start Event
                | Event
		;

Event		: TIME START NAME {
      open_stack.push(new events($1, START, $3));
   }

		| TIME START NAME STRING {
      open_stack.push(new events($1, START, $3, $4));
   }

		| TIME START NAME NAME {
      open_stack.push(new events($1, START, $3, $4));
   }

		| TIME END NAME {
      EndEvents($1, $3);
   }

		| TIME CANCEL NAME {
      // Can't tell if the cancel is for a state or transition modify
      // So peek.
      events *p;
      char *name = $3;
      if( open_stack.first(&p) )
      {
         // HACK: Fixup event logs by converting "PickAgent" to "PickTrigger"
         //       if it was on a transition.
	 if( strcmp(p->event_name(),"PickTrigger") == 0 )
	 {
	    name = "PickTrigger";
	 }
      }

      CancelEvent($1, name);
   }

                | TIME STATUS NAME STRING {
      open_stack.push(new events($1, STATUS, $3, $4));
   }

                | TIME STATUS NAME {
      open_stack.push(new events($1, STATUS, $3));
   }

                | TIME EVENT NAME {
      // Hack to fixup event log
      if( strcmp($3,"StartMake") == 0 )
      {
         open_stack.push(new events($1, START, "Make"));
      }
      else if( strcmp($3,"EndMake") == 0 )
      {
         EndEvents($1, "Make");
      }
      else
      {
         open_stack.push(new events($1, EVENT, $3));
      }
   }

                | TIME EVENT NAME STRING {
      open_stack.push(new events($1, EVENT, $3, $4));
   }

                | TIME EVENT NAME NAME {
      open_stack.push(new events($1, EVENT, $3, $4));
   }

                | TIME STARTMODIFY NAME NAME STRING {
      /* Are we modifying an agent? */
      if( strcmp($3,"Agent") == 0 )
      {
	 if( $4[0] == 'S' )
	 {
            open_stack.push(new events($1, START, "PickAgent", $5));
// cout << "Pushing PickAgent\n";
	 }
	 else
	 {
            open_stack.push(new events($1, START, "PickTrigger", $5));
// cout << "Pushing PickTrigger\n";
	 }
      }
      else
      {
	 if( $4[0] == 'S' )
	 {
            open_stack.push(new events($1, START, "ModAgentParms", $5));
// cout << "Pushing ModAgentParms\n";
	 }
	 else
	 {
            open_stack.push(new events($1, START, "ModTriggerParms", $5));
// cout << "Pushing ModTriggerParms\n";
	 }
      }
   }

                | TIME ENDMODIFY NAME STRING {
      /* Was it modifying an agent? */
      if( strcmp($3,"Agent") == 0 )
      {
         EndEvents($1, "PickAgent", "PickTrigger");
      }
      else
      {
         EndEvents($1, "ModAgentParms", "ModTriggerParms");
      }
   }

		| error 	{ el_SyntaxError("Ignoring event"); }
		;

%%
/*====================== End of EventLog Grammer ============================*/

/************************************************************************/
/* el_error is called by yacc when it finds an error. */

void el_error(char *s)
{
   el_SyntaxError(s);
}

/**********************************************************************
 * $Log: el_gram.y,v $
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
 * Revision 1.5  2003/04/06 11:39:04  endo
 * gcc 3.1.1
 *
 * Revision 1.4  1996/09/03  16:06:13  doug
 * fixed for linux
 *
 * Revision 1.3  1996/06/01  21:54:20  doug
 * *** empty log message ***
 *
 * Revision 1.2  1996/04/13  21:54:42  doug
 * *** empty log message ***
 *
 * Revision 1.1  1996/03/05  22:24:51  doug
 * Initial revision
 *
 **********************************************************************/
