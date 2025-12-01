/**********************************************************************
 **                                                                  **
 **                       parse_support.cc                           **       
 **                                                                  **
 **  support functions for the cdl parser                            ** 
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: parse_support.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdlib.h>
#include <assert.h>
#include <iostream>

#include "prototypes.h"
#include "list.hpp"
#include "cdl_defs.h"
#include "cdl_gram.tab.h"

/********************************************************************
*								    *
*		         SemanticError			            *
*								    *
********************************************************************/

/* SemanticError will print the error message to stderr */

void
SemanticError(const char *Message)
{
   cerr << "ERROR: " << filename << ", line " << lineno << '\n';
   cerr << Message << '\n';

   had_error = true;
}

/********************************************************************
*								    *
*		         SyntaxError			            *
*								    *
********************************************************************/

/* SyntaxError will print the error message to stderr */

void
SyntaxError(const char *Message)
{
   SemanticError(Message);

   cerr << linebuf << '\n';

   for (int i = 0; i < tokenpos - tokenlen; i++)
      cerr << ' ';

   for (int i = 0; i < tokenlen; i++)
      cerr << '^';

   cerr << '\n';
}

/**********************************************************************
*                                                                     *
**********************************************************************/

bool 
isanonymous(const char *name)
{
   return( name[0] == '$' );
}

/**********************************************************************
*                                                                     *
**********************************************************************/

char *
AnonymousName(void)
{
   static unsigned seq = 0;
   char buf[80];

   sprintf(buf, "$AN_%d", seq++);

   return strdup(buf);
}

/**********************************************************************
*                                                                     *
**********************************************************************/

char *
Anonymous2External(const char *anonymous_name)
{
   if( anonymous_name[0] != '$' )
      return strdup(anonymous_name);

   return strdup(&anonymous_name[1]);
}

/**********************************************************************
*                                                                     *
**********************************************************************/

Symbol *
find_index(Symbol * parm, Symbol * index)
{
   Symbol *p;
   void *cur;

   if ((cur = parm->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->list_index == index)
	    return p;
      }
      while ((cur = parm->parameter_list.next(&p, cur)) != NULL);
   }

   return NULL;
}

/*********************************************************************
*                                                                     *
*********************************************************************/

Symbol *
find_name(Symbol * rec, char *name)
{
   Symbol *p;
   void *cur;

   if ((cur = rec->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (strcmp(p->name, name) == 0)
	    return p;
      }
      while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
   }

   return NULL;
}

/*********************************************************************
*                                                                     *
*********************************************************************/

// Add this user to the "users" list for each of the agents in the children list
void
UseChildren(Symbol *user)
{
   Symbol *p;
   void *cur;

   // Check each child to see if it is an agent.
   if ((cur = user->children.first(&p)) != NULL)
   {
      do
      {
	 p->users.append(user);
      }while ((cur = user->children.next(&p, cur)) != NULL);
   }
}


/*********************************************************************
*                                                                     *
*********************************************************************/

// Add this user to the "users" list for each of the children in the parmlist
void
AddUser(SymbolList * parmlist, Symbol * agent)
{
   Symbol *p;
   void *cur;

   // Check each child to see if it is an agent.
   if ((cur = parmlist->first(&p)) != NULL)
   {
      do
      {
	 assert(p);

	 if (p->symbol_type == PARM_HEADER)
	 {
	    // Is list
	    Symbol *lp;
	    void *cur_lp;

	    if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	    {
	       do
	       {
		  Symbol *gen = lp->input_generator;

		  if (gen && gen->is_agent())
		  {
		     gen->users.append(agent);
		  }
	       }
	       while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	    }
	 }
	 else
	 {
	    Symbol *gen = p->input_generator;

	    if (gen && gen->is_agent())
	    {
	       gen->users.append(agent);
	    }
	 }
      }
      while ((cur = parmlist->next(&p, cur)) != NULL);
   }
}

/*********************************************************************
*                                                                     *
*********************************************************************/

// Add any parameters mentioned in the defining record to the list
// These will be inputs that have not been assigned a value.
void
fill_parm_list(Symbol *defining_rec, SymbolList **list)
{
   assert(list);

   Symbol *p;
   void *cur;

   // Check each parm in the defining rec
   if ((cur = defining_rec->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 // Now check if the record is in the list of parms
	 bool exists = false;
	 if( *list != NULL )
	 {
	   Symbol *parm;
	   void *pos;
           if ((pos = (*list)->first(&parm)) != NULL)
           {
              do
              {
		 if( strcmp(p->name,parm->name) == 0 )
		 {
		    exists = true;
		    break;
		 }
              }while ((pos = (*list)->next(&parm, pos)) != NULL);
	    }
	 }
	 if( !exists )
	 {
	    if( *list == NULL )
	       *list = new SymbolList;

            (*list)->append(p->dup(false,p->name));
	 }
      }while ((cur = defining_rec->parameter_list.next(&p, cur)) != NULL);
   }
}


/**********************************************************************
 * $Log: parse_support.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.28  2003/04/06 12:58:48  endo
 * gcc 3.1.1
 *
 * Revision 1.27  2000/04/13 22:05:52  endo
 * Checked in for Doug.
 * This patch extends MissionLab to allow the user to import read-only
 * library code.  When the user attempts to change something which is
 * read-only, cfgedit pops up a dialog box and asks if it should import
 * the object so it can be edited.  If OK'd, it imports the object.
 *
 * This fixes the problem with FSA's (and other assemblages) not being
 * editable when they are loaded from the library.
 *
 * Revision 1.26  1996/10/04  21:10:59  doug
 * changes to get to version 1.0c
 *
 * Revision 1.26  1996/10/01 13:01:32  doug
 * got library writes working
 *
 * Revision 1.25  1996/05/05  21:45:27  doug
 * fixing compile warnings
 *
 * Revision 1.24  1995/10/30  21:38:25  doug
 * *** empty log message ***
 *
 * Revision 1.23  1995/10/06  21:35:33  doug
 * once more on fixing the list parm
 *
 * Revision 1.22  1995/10/06  21:28:04  doug
 * fill_parm_list wasn't checking list parameter correctly for null
 *
 * Revision 1.21  1995/09/19  15:31:53  doug
 * change so all parms defined in the def record exist in any instances
 *
 * Revision 1.20  1995/09/15  22:01:29  doug
 * fixed so handles multiline initializers
 *
 * Revision 1.19  1995/06/29  14:48:02  jmc
 * Added RCS log string.
 **********************************************************************/
