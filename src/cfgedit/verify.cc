/**********************************************************************
 **                                                                  **
 **                          verify.cc                               **
 **                                                                  **
 **                                                                  **
 **  Code generator targeted for the Schema architecure using CNL.   **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: verify.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <libc.h>

#include <Xm/Xm.h>

#include "buildbitmaps.h"
#include "buildmenu.h"
#include "list.hpp"
#include "load_cdl.h"
#include "popups.h"
#include "design.h"
#include "help.h"
#include "write_cdl.h"
#include "configuration.hpp"     
#include "globals.h"
#include "screen_object.hpp" 
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp" 
#include "fsa.hpp"
#include "transition.hpp" 
#include "circle.hpp"
#include "binding.h"
#include "utilities.h" 
#include "reporterror.h"
#include "toolbar.h"
#include "edit.h" 
#include "callbacks.h"

inline static char *
nameof(Symbol *n)
{
   if( n->description != NULL )
      return n->description;
   if( n->name[0] != '$' )
      return n->name;
   return "";
}


static bool
verify_page(const bool debug, SymbolList * page_stack, Symbol * page_def,
	    Symbol * consumer, char **message, Symbol ** location, 
	    Symbol *type, bool check_if_bound, Symbol * index);

      
//************************************************************************************
// Mark the state and recurse on all states reachable from this state.
void
sweep_state( symbol_table < Symbol > *states, Symbol *rule_list, Symbol *state)
{
   // Is this one marked?
   if( state->marked )
      return;

   // no, so mark it.
   state->marked = true;

   // Now, recurse on all states reachable from here.
   Symbol *rule;
   void *cur;

   if ((cur = rule_list->parameter_list.first(&rule)) != NULL)
   {
      do
      {
         // Does the rule leave our state?
         if( strcmp(rule->list_index->name, state->list_index->name) == 0 )
         {
            // Yes, so recurse on the destination.
	    char *rule_dest_name = rule->input_generator->list_index->name;

	    // Check it against each state to find the right one.
	    Symbol *mem = (Symbol *) states->first();

	    while (mem)
	    {
	       if (strcmp(rule_dest_name, mem->list_index->name) == 0)
	       {
                  sweep_state(states, rule_list, mem);
	       }

	       mem = (Symbol *) states->next();
	    }
	 }
      } while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
   }
}

/*-----------------------------------------------------------------------*/

// check this fsa
bool
verify_fsa(const bool debug, SymbolList * page_stack, Symbol * node,
	   Symbol * consumer, char **message, Symbol ** location, 
	   Symbol *type, bool check_if_bound)
{
   Symbol *our_type = node && node->defining_rec && node->defining_rec->data_type ? node->defining_rec->data_type : (Symbol *)NULL;

   if( type && our_type && strcmp(type->name,our_type->name) != 0)
   {
      char buf[1024];
      sprintf(buf, "The type of the FSA output (%s) does not match user's input type (%s)",
		our_type->name, type->name);
      *message = strdup(buf);
      return false;
   }

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

   // Remember the start state.
   Symbol *start_state = NULL;

   // Make sure found the states
   if (member_list != NULL)
   {
      // Create the states for the FSA
      Symbol *member;

      if ((cur = member_list->parameter_list.first(&member)) != NULL)
      {
	 do
	 {
	    if (member->input_generator == NULL ||
		member->input_generator->children.len() == 0)
	    {
	       char buf[1024];

	       sprintf(buf, "State \"%s\" has an empty definition", 
			nameof(member->list_index) );
	       *message = strdup(buf);

	       if (member->input_generator)
	       {
		  // Move into the state
		  page_stack->insert(member->list_index);
		  page_stack->insert(member->input_generator);
	       }

	       return false;
	    }

            if( strcasecmp(member->list_index->name,"start") == 0)
            {
               if( start_state != NULL )
               {
                  // Error, can only have one start state.
	          *message = strdup("Two states are named \"start\"");
	          return false;
               }
               start_state = member;
            }

	    if (!verify_page(debug, page_stack, member->input_generator, node, message, location, p->data_type, check_if_bound, member->list_index))
	       return false;

	    // put the states into the symbol table for easy access.
	    states.put(member->list_index->name, member);
	 }
	 while ((cur = member_list->parameter_list.next(&member, cur)) != NULL);
      }
   }

   if( start_state == NULL )
   {
      // Error, can only have one start state.
      *message = strdup("One state must be named \"start\"");
      return false;
   }

   // Check the transitions.
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

		  if (dest == NULL)
		  {
		     char buf[1024];

		     sprintf(buf,"The transition from state \"%s\" to \"%s\" is corrrupt",
			     nameof(rule->list_index), 
			     nameof(rule->input_generator->list_index));
		     *message = strdup(buf);
		     return false;
		  }
		  else
		  {
		     if (rule->input_generator == NULL ||
			 rule->input_generator->input_generator == NULL ||
			 rule->input_generator->input_generator->children.len() == 0)
		     {
			char buf[1024];

			sprintf(buf, "The transition from state \"%s\" to \"%s\" has an empty definition",
			     nameof(rule->list_index), 
			     nameof(rule->input_generator->list_index));
			*message = strdup(buf);
			return false;
		     }

		     if (!verify_page(debug, page_stack, rule->input_generator->input_generator, node, message, location, boolean_type, check_if_bound, NULL))
		     {
			return false;
		     }
		  }

		  break;
	       }

	       mem = (Symbol *) states.next();
	    }

	    if (!used)
	    {
	       char buf[1024];

	       sprintf(buf, "The transition from state \"%s\" to \"%s\" is corrrupt",
			nameof(rule->list_index), 
			nameof(rule->input_generator->list_index));
	       *message = strdup(buf);
	       return false;
	    }
	 }
	 while ((cur = rule_list->parameter_list.next(&rule, cur)) != NULL);
      }
   }

   // Check for unconnected states.
   if (rule_list != NULL && member_list != NULL)
   {
      // Starting with the start state, do a depth first sweep through the directed transition graph and mark each state reachable

      // First unmark each state.
      Symbol *mem = (Symbol *) states.first();
      while (mem)
      {
         mem->marked = false;
	 mem = (Symbol *) states.next();
      }
      
      // Starting with the start state, begin the sweep.
      sweep_state(&states, rule_list, start_state);

      // Finally, make sure all the states got marked.
      mem = (Symbol *) states.first();
      while (mem)
      {
         if( !mem->marked )
         {
	    *message = strdup("One or more of the states are unreachable");
	    return false;
         }
	 mem = (Symbol *) states.next();
      }
      
      // Starting with the start state, begin the sweep.
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// check this page
bool
verify_node(const bool debug, SymbolList * page_stack, Symbol * node,
	    Symbol * consumer, char **message, Symbol ** location, 
	    Symbol *type, bool check_if_bound)
{
   if (node == NULL)
   {
      char buf[1024];

      sprintf(buf, "The input link to node \"%s\" is unconnected", 
	nameof(consumer));
      *message = strdup(buf);
      return false;
   }

   if (node->marked)
      return true;

   if (node->symbol_type == AGENT_NAME && node->bound_to &&
       (node->defining_rec == NULL ||
	(node->defining_rec->symbol_type != SENSOR_NAME &&
	 node->defining_rec->symbol_type != ACTUATOR_NAME &&
	 node->symbol_type != ROBOT_NAME)))
   {
      char buf[1024];

      sprintf(buf, "Corrupt hardware record \"%s\"\nDelete the record and binding point and recreate.", nameof(node));
      *message = strdup(buf);
      return false;
   }

   if (node->symbol_type == BP_NAME)
   {
      if( node->is_bound_bp() )
      {
         // If this is a binding point and it is bound, then check the
         // device driver instead of the binding point
         return verify_node(debug, page_stack, node->bound_to, consumer, 
		message, location, type, check_if_bound);
      }
      else if( check_if_bound )
      {
         char buf[1024];

         sprintf(buf, "Binding point \"%s\" is not connected to a hardware device.",
		nameof(node));
         *message = strdup(buf);
         return false;
      }
   }

   if (debug)
      cerr << "Checking node \"" << nameof(node) << "\"\n";

   Symbol *our_type = NULL;
   if( node && 
       node->defining_rec && 
       node->defining_rec->data_type )
   {
      our_type = node->defining_rec->data_type;
   }

   if( type && our_type && our_type != void_type &&
       strcmp(type->name,our_type->name) != 0 )
   {
      char buf[1024];
      sprintf(buf, "The output type of \"%s\" (%s) does not match the input type of \"%s\" (%s)",
	nameof(node), our_type->name, nameof(consumer), type->name);
      *message = strdup(buf);
      return false;
   }

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
		     Symbol *ig = lp->input_generator;
		     if (ig == NULL)
		     {
			char buf[1024];

			sprintf(buf, "Unconnected input %s in \"%s\"",
				lp->name, nameof(node) );
			*message = strdup(buf);
			return false;
		     }
	             if( ig != (Symbol *) UP )
	             {
		        if (ig->is_agent())
		        {
			   if (lp->list_index == NULL)
			   {
			      char buf[1024];
   
			      sprintf(buf, "Missing index record in \"%s\"", 
					nameof(node) );
			      *message = strdup(buf);
			      return false;
			   }
   
			   if (ig->symbol_type == AGENT_NAME && ig->bound_to)
			   {
			      char buf[1024];
   
			      sprintf(buf, "Corrupt hardware record \"%s\"\nDelete the record and binding point and recreate.", nameof(ig) );
			      *message = strdup(buf);
			      return false;
			   }

			   // Check the source node
			   if (!verify_node(debug, page_stack, ig, node, 
					message, location, p->data_type, check_if_bound))
			      return false;
			}
			else if( ig->symbol_type == INITIALIZER )
		        {
			   if (ig->name == NULL || ig->name[0] == '\0')
			   {
			      char buf[1024];
   
			      sprintf(buf, "Constant %s has no value in \"%s\"", 
					lp->name, nameof(node) );
			      *message = strdup(buf);
			      return false;
			   }
			}
		     }
		  }
		  while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
	       }
	    }
	    else
	    {
	       Symbol *ig = p->input_generator;
	       if (ig == NULL)
	       {
		  char buf[1024];

		  sprintf(buf, "Unconnected input %s in \"%s\"",
			  p->name, nameof(node) );
		  *message = strdup(buf);
		  return false;
	       }
	       if( ig != (Symbol *) UP )
	       {
	          if (ig->symbol_type == AGENT_NAME &&
		      ig->bound_to)
	          {
		     char buf[1024];
   
		     sprintf(buf, "Corrupt hardware record \"%s\"\nDelete the record and binding point and recreate.", nameof(ig) );
		     *message = strdup(buf);
		     return false;
	          }
	          if (ig->is_agent())
	          {
		     // Check the source node
		     if (!verify_node(debug, page_stack, ig, node, message, location, p->data_type, check_if_bound))
		     {
		        return false;
		     }
	          }
		  else if( ig->symbol_type == INITIALIZER )
		  {
		     if (ig->name == NULL || ig->name[0] == '\0')
		     {
		        char buf[1024];
   
		        sprintf(buf, "Constant %s has no value in \"%s\"", 
					p->name, nameof(node) );
		        *message = strdup(buf);
		        return false;
		     }
		  }
               }
	    }
	 }
	 while ((cur = node->parameter_list.next(&p, cur)) != NULL);
      }
   }
   else
   {
      // This is a grouping node, so process it as a new page.
      return verify_page(debug, page_stack, node, node, message, 
		location, type, check_if_bound, NULL);
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
static bool
verify_page(const bool debug, SymbolList * page_stack, Symbol * page_def,
      Symbol * consumer, char **message, Symbol ** location, Symbol *type,
      bool check_if_bound, Symbol * index)
{
   bool top_page = page_stack->len() == 0;

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

   if (page_def->is_fsa())
   {
      // FSA are handled as a special case because we want to check the states.
      if (!verify_fsa(debug, page_stack, page_def, consumer, message, location, 		type, check_if_bound))
      {
	 return false;
      }
   }
   else
   {
      page_def->clear_marks();

      if (page_def->children.len() == 0)
      {
	 char buf[1024];

	 sprintf(buf,"The implementation of \"%s\" is empty!",nameof(page_def));
	 *message = strdup(buf);
	 return false;
      }

      // check that there are the correct number of children
      Symbol *p;
      void *cur;
      int num_gens = 0;
      if ((cur = page_def->children.first(&p)) != NULL)
      {
	 do
	 {
            // count this node as a generator if it has an output
            Symbol *real_src = p->real_source();
            if (real_src && 
                real_src->defining_rec &&
                real_src->defining_rec->data_type &&
                real_src->defining_rec->data_type != void_type)
            {
	       num_gens++;
	    }
	 }
	 while ((cur = page_def->children.next(&p, cur)) != NULL);
      }

      if (consumer == NULL && num_gens > 0 &&
	  (check_if_bound || !top_page) )
      {
	 char buf[1024];

	 sprintf(buf, "There is an unconnected output (missing output binding point?).");
	 *message = strdup(buf);
	 return false;
      }

      if (num_gens > 1)
      {
	 char buf[1024];

	 sprintf(buf, "There is more than one defining agent");
	 *message = strdup(buf);
	 return false;
      }

      // check the tree for each of the children.
      if ((cur = page_def->children.first(&p)) != NULL)
      {
	 do
	 {
	    // Check the source node
	    if (!verify_node(debug, page_stack, p, page_def, message, 
		location, page_def->data_type, check_if_bound))
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
verify(bool check_if_bound, bool quiet)
{
   char *message;
   SymbolList *page_stack = new SymbolList;

   bool debug = false;

   // Make sure we get any changes from the currently displayed page
   config->this_page()->update();

   // Make sure the architecture is bound
   if (check_if_bound && !config->arch_is_bound())
   {
      if( !quiet )
         warn_user("The configuration must be bound to an architecture before compiling");
      delete page_stack;
      return false;
   }

   Symbol *agent = config->root();

   if (agent->children.len() == 0)
   {
      if( !quiet )
         warn_user("The configuration is empty");
      delete page_stack;
      return false;
   }

   // Clear all the marked nodes
   agent->clear_marks();

   Symbol *location = NULL;
   if (!verify_page(debug, page_stack, agent, NULL, &message, 
		&location, void_type, check_if_bound, NULL))
   {
      if( !quiet )
      {
         config->goto_page(page_stack, location);
         warn_user(message);
      }
      delete page_stack;
      return false;
   }

   delete page_stack;

   return true;
}

///////////////////////////////////////////////////////////////////////
// $Log: verify.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.22  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.21  2000/03/22 18:05:10  endo
// Check in for Doug. According to Doug,
// "I have added code to cfgedit to check that the fsa is fully connected.
// Begining with the start state, it uses a depth first sweep to mark states
// reachable via transitions from the start state.  If any states are
// not reached during this sweep it flags it as an error and won't compile
// the configuration.  This will catch the case where users forget to add
// the initial transition to a group of waypoint states.
//
// Revision 1.20  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.19  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.19  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.18  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.17  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.16  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.15  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.14  1996/02/18  23:31:19  doug
// binding is working!!!!
//
// Revision 1.13  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.12  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.11  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.10  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.9  1995/12/05  17:25:29  doug
// *** empty log message ***
//
// Revision 1.8  1995/11/30  23:31:07  doug
// *** empty log message ***
//
// Revision 1.7  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.6  1995/11/27  16:48:58  doug
// allow multiple void objects (and robots) on the same page
//
// Revision 1.5  1995/11/22  12:11:22  doug
// check for bad input sensors
//
// Revision 1.4  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.3  1995/11/20  22:44:59  doug
// *** empty log message ***
//
///////////////////////////////////////////////////////////////////////
