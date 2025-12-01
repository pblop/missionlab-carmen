/**********************************************************************
 **                                                                  **
 **                      ExpandSubtrees.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: ExpandSubtrees.cc,v 1.1.1.1 2008/07/14 16:44:21 endo Exp $ */


#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include "load_cdl.h"
#include "ExpandSubtrees.h"

/*-----------------------------------------------------------------------*/

// Determines if this node sets the values for any pushed up parms 

static bool
SetsPuParms(Symbol *node)
{
   // check for PU_INPUT_NAMEs with an INITIALIZER
   Symbol *p;
   void *cur;
   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if( p->symbol_type == PARM_HEADER )
	 {
            Symbol *lp;
            void *lp_cur;
            if ((lp_cur = p->parameter_list.first(&lp)) != NULL)
            {
               do
               {
	          if( lp && 
		      lp->symbol_type == PU_INPUT_NAME &&
		      lp->input_generator && 
		      (lp->input_generator->symbol_type == INITIALIZER || 
		       lp->input_generator->symbol_type == PU_INITIALIZER) )
	          {
		     return true;
	          }
               } while ((lp_cur = p->parameter_list.next(&lp, lp_cur)) != NULL);
	    }
	 }
	 else
	 {
	    if(p && 
	       p->symbol_type == PU_INPUT_NAME &&
	       p->input_generator && 
	       (p->input_generator->symbol_type == INITIALIZER ||
	        p->input_generator->symbol_type == PU_INITIALIZER) )
	    {
	       return true;
	    }
	 }
      } while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   return false;
}

/*-----------------------------------------------------------------------*/

// Called on each node to check if it should be expanded
// Returns false to abort the scanning
// Sets restart if it needs the scan restarted.

static bool
expand(Symbol *node, Symbol *page_def, Symbol *robot,
	     SymbolList * page_stack, char **message, bool *restart)
{
   // If we don't set any pushed up parms, then no need to dup our children.
   if( !SetsPuParms(node) )
      return true;

   bool made_change = false;

   // Check each of our direct children
   Symbol *child;
   void *cur;
   if ((cur = node->children.first(&child)) != NULL)
   {
      do
      {
         if( child->users.len() > 1 )
         {
	    // Make us a distinct copy of the node
	    Symbol *new_copy = child->dup_tree(true);

            // Connect it
	    if(!node->children.replace(child, new_copy))
	    {
               *message = strdup("Unable to replace old child");
               return false;
	    }

            // Fixup the users lists
	    child->users.remove(node);
	    new_copy->users.append(node);

	    made_change = true;
         }
      } while ((cur = node->children.next(&child, cur)) != NULL);
   }

   // Now check the input sources
   Symbol *p;
   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if( p->symbol_type == PARM_HEADER )
	 {
            Symbol *lp;
            void *lp_cur;
            if ((lp_cur = p->parameter_list.first(&lp)) != NULL)
            {
               do
               {
                  if( lp &&
		      lp->input_generator &&
		      lp->input_generator->is_agent() &&
		      lp->input_generator->users.len() > 1 )
                  {
	             // Make us a distinct copy of the node
	             Symbol *new_copy = lp->input_generator->dup_tree(true);
         
                     // Fixup the users lists
	             lp->input_generator->users.remove(node);
	             new_copy->users.append(node);
         
                     // Connect it
		     lp->input_generator = new_copy;
         
	             made_change = true;
                  }
               } while ((lp_cur = p->parameter_list.next(&lp, lp_cur)) != NULL);
	    }
	 }
	 else
	 {
            if( p &&
		p->input_generator &&
		p->input_generator->is_agent() &&
		p->input_generator->users.len() > 1 )
            {
	       // Make us a distinct copy of the node
	       Symbol *new_copy = p->input_generator->dup_tree(true);
         
               // Fixup the users lists
	       p->input_generator->users.remove(node);
	       new_copy->users.append(node);
         
               // Connect it
	       p->input_generator = new_copy;
         
	       made_change = true;
            }
	 }
      } while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   // It appears that restarting is not necessary since we never change
   // this node, only the children and input generators.
#if 0
   // If we made a change, we should restart since we are mucking with the tree
   if( made_change )
   {
      // need Restart, since mucked with the tree.
      *restart = true;
      return false;
   }
#endif

   return true;
}

/*-----------------------------------------------------------------------*/

/* called when unbind button is clicked in hardware record glyph */
void
ExpandSubtrees(Symbol *root)
{
   // Need to expand any linked subtrees
   Symbol *location;
   char *msg;
   SymbolList *page_stack;
   apply(root, expand, &location, &msg, &page_stack);
}


///////////////////////////////////////////////////////////////////////
// $Log: ExpandSubtrees.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:21  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:49  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:06  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.7  2003/04/06 12:58:48  endo
// gcc 3.1.1
//
// Revision 1.6  1996/10/04  21:10:59  doug
// changes to get to version 1.0c
//
// Revision 1.6  1996/10/01 13:01:32  doug
// got library writes working
//
// Revision 1.5  1996/06/17 22:58:49  doug
// *** empty log message ***
//
// Revision 1.4  1996/05/05  21:45:27  doug
// fixing compile warnings
//
// Revision 1.3  1996/03/19  23:11:04  doug
// needed to handle PU_INITIALIZERs in code where checks if sets pu parms
//
// Revision 1.2  1996/02/26  05:02:34  doug
// *** empty log message ***
//
// Revision 1.1  1996/02/25  19:00:53  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
