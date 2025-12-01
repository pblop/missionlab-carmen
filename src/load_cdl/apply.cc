/**********************************************************************
 **                                                                  **
 **                          apply.cc                               **
 **                                                                  **
 **                                                                  **
 **  Code generator targeted for the Schema architecure using CNL.   **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: apply.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <libc.h>

#include "load_cdl.h"

static bool
apply_page(apply_fnc *fnc, SymbolList * page_stack, Symbol * page_def,
	    Symbol * consumer, char **message, Symbol ** location, 
	    Symbol *type, Symbol *robot, Symbol * index, bool *restart);

/*-----------------------------------------------------------------------*/

// check this fsa
bool
apply_fsa(apply_fnc *fnc, SymbolList * page_stack, Symbol * node,
	   Symbol * consumer, char **message, Symbol ** location, 
	   Symbol *type, Symbol *robot, bool *restart)
{
   if( !(*fnc)(node, node, robot, page_stack, message, restart) )
      return false;

   // Find the rules
   Symbol *p;
   void *cur;
   Symbol *rule_list = NULL;
   Symbol *member_list = NULL;

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->symbol_type == PARM_HEADER)
	 {
	    // Is a list

	    // Remember the list of rules
	    if (p->data_type == expression_type)
	       rule_list = p;
	    else if (p->data_type == member_type)
	       member_list = p;
	 }
      }
      while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   symbol_table < Symbol > states;

   // Make sure found the states
   if (member_list != NULL)
   {
      // Create the states for the FSA
      Symbol *member;

      if ((cur = member_list->parameter_list.first(&member)) != NULL)
      {
	 do
	 {
	    if (member->input_generator  &&
	        !apply_page(fnc, page_stack, member->input_generator, node, message, location, p->data_type, robot, member->list_index, restart))
	    {
	       return false;
	    }

	    // put the states into the symbol table for easy access.
	    states.put(member->list_index->name, member);
	 }
	 while ((cur = member_list->parameter_list.next(&member, cur)) != NULL);
      }
   }

   // Make sure found the rules
   if (rule_list != NULL)
   {
      // Step through each rule
      Symbol *rule;

      if ((cur = rule_list->parameter_list.first(&rule)) != NULL)
      {
	 do
	 {
	    bool used = false;
	    char *rule_src_name = rule->list_index->name;
	    char *rule_dest_name = rule->input_generator->list_index->name;

	    // Check it against each state to find the one sourcing it.
	    Symbol *mem = (Symbol *) states.first();

	    while (mem)
	    {
	       char *mem_name = mem->list_index->name;

	       if (strcmp(rule_src_name, mem_name) == 0)
	       {
		  used = true;
		  Symbol *dest = (Symbol *) states.get(rule_dest_name);

		  if (dest && 
		      rule->input_generator && 
		      rule->input_generator->input_generator )
		  {
		     if (!apply_page(fnc, page_stack, rule->input_generator->input_generator, node, message, location, boolean_type, robot, NULL, restart))
		     {
			return false;
		     }
		  }

		  break;
	       }

	       mem = (Symbol *) states.next();
	    }
	 }
	 while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
      }
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// check this page
bool
apply_node(apply_fnc *fnc, SymbolList * page_stack, Symbol * node,
	    Symbol * consumer, char **message, Symbol ** location, 
	    Symbol *type, Symbol *robot, bool *restart)
{
   if (node == NULL || node->marked)
      return true;

   // If moving into a robot, then remember it
   if (node->is_robot() || node->is_robot_bp())
      robot = node; 

   if( !(*fnc)(node, consumer, robot, page_stack, message, restart) )
      return false;

   // If this is a grouping agent, then we won't show detail in the glyph
   bool is_grouper = node->is_grouping_op();

   // Mark this node as checked
   node->marked = true;

   // Check its direct children
   if (!is_grouper)
   {
      Symbol *p;
      void *cur;

      if ((cur = node->parameter_list.first(&p)) != NULL)
      {
	 do
	 {
	    if (p->symbol_type == PARM_HEADER)
	    {
	       Symbol *lp;
	       void *cur_lp;

	       if ((cur_lp = p->parameter_list.first(&lp)) != NULL)
	       {
		  do
		  {
		     if (lp->input_generator && lp->input_generator->is_agent())
		     {
			// Check the source node
			if (!apply_node(fnc, page_stack, lp->input_generator, 
				node, message, location, p->data_type, robot, restart))
			   return false;
		     }
		  }
		  while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }
	    }
	    else
	    {
	       if (p->input_generator && p->input_generator->is_agent())
	       {
		  // Check the source node
		  if (!apply_node(fnc,page_stack, p->input_generator, node, 
			message, location, p->data_type, robot, restart))
		  {
		     return false;
		  }
	       }

	    }
	 }
	 while ((cur = node->parameter_list.next(&p, cur)) != NULL);
      }
   }
   else
   {
      if (!apply_page(fnc,page_stack, node, NULL, message, 
		location, type, robot, NULL, restart) )
      {
	 return false;
      }
   }

   if (node->symbol_type == BP_NAME && node->is_bound_bp())
   {
      // If this is a binding point and it is bound, then check the
      // device driver too.
      if (!apply_node(fnc,page_stack, node->bound_to, consumer, message, 
		location, type, robot, restart))
      {
	 return false;
      }
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// check a full page 
/*
   The page is defined by a grouping operator.  
   These are the following:
   GROUP_NAME
   CONFIGURATION_NAME
   ROBOT_NAME

   Starting with one of these nodes (which does NOT display)
   The tree for each child (unattached) is expanded until anouther grouping 
   operator is hit (or the end of the tree).  When a grouping operator
   is hit, it is drawn in the compact form.

   The binding points are handled as a special case. 
   If a device is bound to the BP, it hides the binding point record
   (the binding point is NOT displayed).
 */
bool
apply_page(apply_fnc *fnc, SymbolList * page_stack, Symbol * page_def,
      Symbol * consumer, char **message, Symbol ** location, Symbol *type,
      Symbol *robot, Symbol * index, bool *restart)
{
   // Make sure things are as expected
   assert(page_def);
   assert(page_def->is_grouping_op());

   // The FSA states stuff the index also
   if (index)
   {
      page_stack->insert(index);
   }

   // push this page
   page_stack->insert(page_def);

   // If moving into a robot, then remember it
   if (page_def->is_robot() || page_def->is_robot_bp())
      robot = page_def; 

   if (page_def->is_fsa())
   {
      // FSA are handled as a special case because we want to check the states.
      if (!apply_fsa(fnc, page_stack, page_def, consumer, message, location, 		type, robot, restart))
      {
	 return false;
      }
   }
   else
   {
      // Reset the marks.
      // It is likely that one input tree can be attached to multiple consumers
      // on the same page.  In that case, we only check it once.
      page_def->clear_marks();

      if( !(*fnc)(page_def, page_def, robot, page_stack, message, restart) )
	 return false;

      // check the tree for each of the children.
      void *cur;
      Symbol *p;
      if ((cur = page_def->children.first(&p)) != NULL)
      {
	 do
	 {
	    // Check the source node
	    if (!apply_node(fnc, page_stack, p, page_def, message, 
		location, page_def->data_type, robot, restart))
	    {
	       return false;
	    }
	 }
	 while ((cur = page_def->children.next(&p, cur)) != NULL);
      }
   }

   // pop this page
   page_stack->get();

   // pop the index
   if (index)
   {
      page_stack->get();
   }

   return true;
}

//**************************************************************
// Returns true if success
bool
apply(Symbol *start, apply_fnc *fnc, 
	Symbol **location, char **msg, SymbolList **page_stack)
{
   *page_stack = new SymbolList;
   *location = NULL;
   *msg = NULL;

   if (start->children.len() == 0)
      return true;

   // Clear all the marked nodes
   start->clear_marks();

   bool restart;
   bool rtn;
   do
   {
      restart = false;
      rtn = apply_page(fnc, *page_stack, start, NULL, msg, location, void_type, 
		NULL, NULL, &restart);
   } while( restart );

   // Error out
   if( !rtn )
      return false;

   // good return
   delete *page_stack;
   *page_stack = NULL;

   return true;
}

///////////////////////////////////////////////////////////////////////
// $Log: apply.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
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
// Revision 1.1  2003/04/06 12:58:48  endo
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
