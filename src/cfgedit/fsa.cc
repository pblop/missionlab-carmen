/**********************************************************************
 **                                                                  **
 **                             fsa.cc                               **
 **                                                                  **
 **                                                                  **
 **                    Used to create/edit fsa's                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: fsa.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

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
#include "toolbar.h"
#include "callbacks.h"
#include "so_movement.h"
#include "../cdl/codegens.h"

const int FSA_WIDTH        = 800;
const int FSA_HEIGHT       = 800;
const int FSA_WINDOW_WIDTH = 500;
const int FSA_WINDOW_HEIGHT= 400;
const int SLOP_DISTANCE_TO_STATE = 30;
const int MAX_DISTANCE_TO_LINE  = 5;

//***************************************************************************


void
fsa::erase()
{
    XFillRectangle(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.erase,
        0,
        0,
        FSA_WIDTH,
        FSA_HEIGHT);
}

//*************************************************************************
void fsa::draw(bool erase)
{
//   erase();

   circle *cur = (circle *)states.first();
   while( cur )
   {
      if( erase )
         cur->erase();
      else
         cur->draw();
      cur = (circle *)states.next();
   }
}

//*************************************************************************
circle *fsa::getState(string stateName)
{
    string thisName;
    circle *state = NULL;

    state = (circle *)states.first();

    while(state != NULL)
    {
        if (state->name() != NULL)
        {
            thisName = state->name();

            if (thisName == stateName)
            {
                return state;
            }
        }

        state = (circle *)states.next();
    }

    return NULL;
}

//*************************************************************************

fsa::~fsa()
{
   circle *cur = (circle *)states.first();
   while( cur )
   {
      delete cur;
      cur = (circle *)states.next();
   }
}

/****************************************************************************/

int num_transitions = 0;

screen_object *
fsa::add_transition(screen_object *src, screen_object *des)
{
   circle *source_state = (circle *)src->base_rec();
   circle *dest_state = (circle *)des->base_rec();

   // If this is the first transition, then create the rule_list parm
   if( rule_list_ == NULL )
   {
      // Find the name of the rules in the definition record
      Symbol *p;
      void *cur;
      Symbol *rule_list = NULL;

      if ((cur = src_symbol_->defining_rec->parameter_list.first(&p)) != NULL)
      {
         do
         {
	    // Remember the list of rules
	    if (p->data_type == expression_type)
	    {
	       rule_list = p;
	       break;
	    }
         } while ((cur = src_symbol_->defining_rec->parameter_list.next(&p, cur)) != NULL);
      }

      if( rule_list == NULL )
      {
	 cerr << "Error: Didn't find rule list parameter\n";
	 cerr << "Definitions for FSA style coordination operators\n";
	 cerr << "must include a list parameter of type "
	      << expression_type->name << '\n';
      }
      else
      {
	 // Dup the record.
         rule_list_ = new Symbol(*rule_list);

	 // Add it as an input parm to the fsa
	 src_symbol_->parameter_list.append(rule_list_);
      }
   }

   if( rule_list_ )
   {
      // Build a data record for this instance of the parameter reference
      // And attach the index record
      Symbol *inst = new Symbol;
      inst->name = strdup(rule_list_->name);
      inst->symbol_type = INPUT_NAME;
      inst->name = NULL;
      inst->list_index = source_state->index();

      // Add it to the list in the header record
      rule_list_->parameter_list.append(inst);
      assert(inst->list_index);

      // Make a default rule symbol
      Symbol *rule = new Symbol(RULE_NAME);
      rule->name = AnonymousName();
      rule->list_index = dest_state->index();
      assert(rule->list_index);

      Symbol *dup = DefineName(rule);
      assert(dup == NULL);

      inst->input_generator = rule;

      // Make an empty group symbol to hang under the state
      Symbol *group = new Symbol(GROUP_NAME);
      group->record_class = RC_USER;
      group->name = AnonymousName(); 
      group->location.x = 10;
      group->location.y = 10;
         
      group->description = strdup("Trans000");
      sprintf(&group->description[5],"%d", ++num_transitions);

      // Connect the input generator
      rule->input_generator = group;
      group->users.append(src_symbol_);

      // If the trigger which the user is about to create has already
      // default name (specified in so_movement.cc) and it is
      // in the list, create that trigger. If there is no defalut
      // name specified, name it "Immediate". If "Immediate" is not
      // in the list, then, probably, the architecture is not
      // AuRA.urban. So, use the old default "FirstTime".

      if (!this_trigger)
           this_trigger = "Immediate";

      Symbol *ft_def;
      Symbol *ft;

      ft_def = LookupName(this_trigger);
      if( ft_def )
      {
	 ft = config->find_agent_by_name(this_trigger);

         // Connect it up
         group->children.append(ft);
         ft->users.append(group);
      }
      else
      {
          ft_def = LookupName("Immediate");
          if( ft_def )
          {
              ft = ft_def->dup_tree(false);
         
             // Connect it up
             group->children.append(ft);
             ft->users.append(group);
          }
          else
	  {
             ft_def = LookupName("FirstTime");
             if( ft_def )
             {
                ft = ft_def->dup_tree(false);
         
                // Connect it up
                group->children.append(ft);
                ft->users.append(group);
             }
	  }
      }

      // Make the transition
      transition *tmp = new transition(source_state,dest_state,inst);
      tmp -> set_subtree(ft); // Set the subtree of this trigger.

      // Draw the link
      tmp->draw();

      return tmp;
   }

   return NULL;
}

int fsa::getNumberOfStatesInFSA(void) {
  void *cur;
  int cnt = 0;
  screen_object *s;

  if( (cur = objs.first(&s)) ) 
    {
      do
	{
	  Symbol *rec = s->get_src_sym();
	  if( rec->symbol_type == INPUT_NAME && 
	      rec->input_generator && 
	      rec->input_generator->symbol_type != RULE_NAME)
	    {
	      cnt++;
	    }
	} while( (cur = objs.next(&s,cur)) );
   }
  return cnt;
}
//*************************************************************************

circle * fsa::add_state(Symbol *old_inst)
{
   // If this is the first state, then need to create the member_list_ parm
   if( member_list_ == NULL )
   {
      // Find the name of the members
      Symbol *p;
      void *cur;
      Symbol *member_list = NULL;

      if ((cur = src_symbol_->parameter_list.first(&p)) != NULL)
      {
         do
         {
	    // Remember the list of rules
	    if (p->data_type == member_type)
	    {
	       member_list = p;
	       break;
	    }
         } while ((cur=src_symbol_->parameter_list.next(&p, cur)) != NULL);
      }

      // Convert it to a list
      member_list->symbol_type = PARM_HEADER;
      member_list_ = member_list;
   }

   // Define the new state
   Symbol *state = new Symbol(INDEX_NAME);
   state->record_class = RC_USER;
   state->location.x = 50;
   state->location.y = 50;

   // Count the number of existing states to determine the new index value
   screen_object *s;
   void *cur;
   int cnt = 0;
   if( (cur = objs.first(&s)) ) 
   {
      do
      {
	 Symbol *rec = s->get_src_sym();
         if( rec->symbol_type == INPUT_NAME && 
	     rec->input_generator && 
	     rec->input_generator->symbol_type != RULE_NAME)
	 {
	    cnt++;
	 }
      } while( (cur = objs.next(&s,cur)) );
   }

   state->index_value = cnt;
   if( state->index_value == 0 )
   {
      state->name = strdup(fsa_start_state_name);
      state->description = strdup(fsa_start_state_name);
   }
   else if( old_inst && 
	    old_inst->list_index && 
	    old_inst->list_index->description &&
	    strncmp(old_inst->list_index->description, "State", 5) )
   {
      state->name = AnonymousName(); 
      state->description = strdup(old_inst->list_index->description);
   }
   else
   {
      state->name = AnonymousName(); 
      state->description = strdup("State000");
      assert(state->index_value >= 0 && state->index_value < 1000);
      sprintf(&state->description[5],"%d",state->index_value);
   }

   if( old_inst && old_inst->list_index )
   {
      state->location.x = old_inst->list_index->location.x;
      state->location.y = old_inst->list_index->location.y;
   }

   // Add it to the rule list table in the header record
   member_list_->table.put(state->name,state);

   // Build a data record for this instance of the parameter reference
   // And attach the index record
   Symbol *inst = new Symbol;
   inst->name = strdup(member_list_->name);
   inst->symbol_type = INPUT_NAME;
   inst->record_class = RC_USER;
   inst->list_index = state;

   if( old_inst && old_inst->input_generator )
   {
      inst->input_generator = old_inst->input_generator;
      inst->input_generator->users.append(src_symbol_);
   }
   else
   {
      // Make an empty group symbol to hang under the state
      Symbol *group = new Symbol(GROUP_NAME);
      group->record_class = RC_USER;
      group->name = AnonymousName(); 
      group->location.x = 10;
      group->location.y = 10;
      
      // Connect the input generator
      inst->input_generator = group;
      group->users.append(src_symbol_);

      // Put an idle agent under the state
      Symbol *noop_def = LookupName("Stop");
      if( noop_def )
      {
         Symbol *noop = noop_def->dup_tree(false);
         
         // Connect it up
         group->children.append(noop);
         noop->users.append(group);
      }
   }

   // create the new circle.
   circle *c = new circle(src_symbol_, inst, false);

   states.put(c->name(), c);

   // Add it to the member list in the header record
   member_list_->parameter_list.append(inst);
   assert(inst->list_index);

   // Leave it up for now.
   c->lift();

   return c;
}

//*************************************************************************

fsa::fsa(Symbol *node)
{
   adding_transition_ = false;
   trans_start_ = NULL;

   src_symbol_ = node;

   // Find the rules
   Symbol *p;
   void *cur;
   rule_list_ = NULL;
   member_list_ = NULL;

   if ((cur = node->parameter_list.first(&p)) != NULL)
   {
      do
      {
	 if (p->symbol_type == PARM_HEADER)
	 {
	    // Is a list

	    // Remember the list of rules
	    if (p->data_type == expression_type)
		 {
		    if( rule_list_ )
			 {
             warn_userf("Internal Error: found multiple lists of transitions for the FSA");
			 }
	       rule_list_ = p;
		 }
	    else if (p->data_type == member_type)
		 {
		    if( member_list_ )
			 {
             warn_userf("Internal Error: found multiple lists of states for the FSA");
			 }
	       member_list_ = p;
		 }
	 }
      } while ((cur = node->parameter_list.next(&p, cur)) != NULL);
   }

   // Make sure found the states
   if (member_list_ != NULL)
   {
      // Create the states for the FSA
      Symbol *member;

      if((cur = member_list_->parameter_list.first(&member)) != NULL)
      {
         do
         {
	    circle *c = new circle(node, member, true);

            // put the states into the symbol table for easy access.
	    states.put(member->list_index->name, c);
         } while ((cur = member_list_->parameter_list.next(&member, cur)) != NULL);
      }
   }
	else
	{
      warn_userf("Internal Error: Didn't find list of states for the FSA");
	}

   // Make sure found the rules
   if (rule_list_ != NULL)
   {
      // Step through each rule
      Symbol *rule;
      if( (cur = rule_list_->parameter_list.first(&rule)) != NULL )
      {
         do
	 {
            bool used = false;
	    char *rule_src_name = rule->list_index->name;
	    char *rule_dest_name = rule->input_generator->list_index->name;

            // Check it against each state to find the one sourcing it.
            circle *cir = (circle *)states.first();
            while (cir)
            {
	       char *cir_name = cir->name();
	       if (strcmp(rule_src_name, cir_name) == 0)
	       {
		  used = true;
	          circle *dest = (circle *)states.get(rule_dest_name);
		  if( dest == NULL )
		  {
		     cerr << "Error: Didn't find destination " << rule_dest_name
			  << " for FSA link leaving state " << rule_src_name 
			  << "\n";
		  }
		  else
		  {
	             transition *t = new transition(cir,dest, rule);
		     t->draw();
		  }

		  break;
	       }

	       cir = (circle *)states.next();
	    }

	    if( !used )
	    {
	       warn_userf("Internal Error: Didn't find source state '%s' for FSA link to state '%s'", rule_src_name, rule_dest_name);
	    }
	 } while ((cur = rule_list_->parameter_list.next(&rule, cur)) != NULL);
      }
   }
	else
	{
      warn_userf("Internal Error: Didn't find list of transitions for the FSA");
	}

   dragging_ = false;
}

//*************************************************************************

void
fsa::update()
{
   circle *cur = (circle *)states.first();
   while( cur )
   {
      cur->update();

      cur = (circle *)states.next();
   }
}

#if 0
/****************************************************************************/
/* find closest line and return the distance to it in pixels                */
/****************************************************************************/

int
closest_line (int x, int y, int *line)
{
   int i;
   int min_dist;
   int dist;
   int min_num;
   double Ux, Uy, Vx, Vy;
   double length;
   double h;

   min_num = -1;
   min_dist = GT_INFINITY;

   for (i = 0; i < num_points; i++)
     {
	/* create a unit vector along the line */
	Ux = points[i + 1].x - points[i].x;
	Uy = points[i + 1].y - points[i].y;
	length = sqrt(sqr(Ux) + sqr(Uy));
	Ux /= length;
	Uy /= length;

	/* create a vector to the cursor location */
	Vx = x - points[i].x;
	Vy = y - points[i].y;

	/* calculate the dot product, to see if is along the line segment */
	/* recall: dot product gives parallel component of resultant vector */
	h = Ux * Vx + Uy * Vy;

	if (h < 0 || h > length)
	  {
	     /* The perpendicular from the point to the line does
	    not intersect the line between the two end points */
	     continue;
	  }

	// now calculate the distance via the cross product */
	//recall cross product gives perpendicular component of resultant vector
	dist = abs ((int)(Ux * Vy - Uy * Vx + 0.5));

	if (dist < min_dist)
	  {
	     min_dist = dist;
	     min_num = i;
	  }
     }

   if (min_dist > MAX_DISTANCE_TO_LINE)
     {
	return false;
     }

   *line = min_num;
   return true;
}

//---------------------------------------------------------------------------
static void
place_transition(Widget w,
            caddr_t client_data,
            XmSelectionBoxCallbackStruct *fcb)
{
   /* unexpose the file selection dialog */
   XtUnmanageChild(w);
   
   char *name; 
   XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET , &name);

   /* check name */
   if( name == NULL )
      return;
   int len = strlen(name);
   if( len <= 0)
      return; 

   /* see if is a duplicate */
   if( find_state_by_name(name) != NULL )
   {
       warn_userf("Error: State name %s already exists!", name);
       return;
   }

   T_states *cur = (T_states *)calloc(sizeof(T_states),1);
   cur->x = CIRCLE_RADIUS + 15;
   cur->y = CIRCLE_RADIUS + 15;
   cur->name = name;
   cur->lifted = true;

   // Add to chain
   cur->next = state_list;
   state_list = cur;
   num_states++;

   fsa_changed = true;

   start_dragging_new_state(cur);
}

#endif

//*************************************************************************

circle * fsa::dup_state(Symbol *old_state)
{
   // Copy the new state
   Symbol *state = old_state->dup();
   state->index_value = member_list_->index_value;
   state->name = strdup("State000");
   assert(state->index_value >= 0 && state->index_value < 1000);
   sprintf(&state->name[5],"%d",state->index_value);
   state->location.x = 7;
   state->location.y = 9;
   member_list_->index_value++;
   DefineName(state);

   // Add it to the rule list table in the header record
   member_list_->table.put(state->name,state);

   // Build a data record for this instance of the parameter reference
   // And attach the index record
   Symbol *inst = new Symbol;
   *inst = *(member_list_);
   inst->symbol_type = INPUT_NAME;
   inst->list_index = state;
   inst->record_class = RC_USER;

   // create the new circle.
   circle *c = new circle(src_symbol_, inst, false);

   states.put(c->name(), c);

   // Add it to the member list in the header record
   member_list_->parameter_list.append(inst);
   assert(inst->list_index);

   return c;
}

//*************************************************************************

// Returns true if success
bool
fsa::unhook_transition(transition *trans)
{
   if (rule_list_ == NULL)
      return false;

   // Step through each state
   circle *cir = (circle *)states.first();
   while (cir)
   {
      transition *t;  
      void *cur;
               
      // Check each link leaving the circle
      if ((cur = cir->out_links.first(&t)) != NULL)
      {              
         do          
         {
	    // Is it the one we are looking for?
            if (t == trans)
	    {
               // Remove it from the output list
               if( !cir->out_links.remove(t) )
		  warn_user("Unable to remove output link for transition");

               if( !t->dest()->in_links.remove(t) )
		  warn_user("Unable to remove input link for transition");

               if( !rule_list_->parameter_list.remove(t->inst()) )
		  warn_user("Unable to remove transition from rule list");

               return true;
	    }
         } while ((cur = cir->out_links.next(&t, cur)) != NULL);
      }

      cir = (circle *)states.next();
   }

   return false;
}

//*************************************************************************

// Returns true if success
bool
fsa::delete_transition(transition *trans)
{
   // Unhook it
   if( !unhook_transition(trans) )
      return false;

   // Kill it
   delete trans;

   // Return success
   return true;
}

//*************************************************************************

// Returns true if success
bool
fsa::unhook_state(circle *the_circle)
{
   // Remove all output transitions from the selected state.
   transition *t;  
   void *cur;
   bool looping;
   do
   {
      looping = false;
      if ((cur = the_circle->out_links.first(&t)) != NULL)
      {              
         do          
         {
	    // Kill it
            the_circle->out_links.remove(t);
            t->dest()->in_links.remove(t);
            rule_list_->parameter_list.remove(t->inst());
	    delete t;

	    // Just mucked with the link lists, so need to rescan.
	    looping = true;
	    break;
         } while ((cur = the_circle->out_links.next(&t, cur)) != NULL);
      }
   } while(looping);

   // Remove all input transitions to the selected state.
   circle *cir = (circle *)states.first();
   while (cir)
   {
      if( cir != the_circle )
      {
         bool looping;
         do
         {
            looping = false;
            transition *t;  
            void *cur;
            if ((cur = cir->out_links.first(&t)) != NULL)
            {              
               do          
               {
		  if( t->dest() == the_circle )
		  {
	             // Kill it
                     cir->out_links.remove(t);
                     the_circle->in_links.remove(t);
                     if( !rule_list_->parameter_list.remove(t->inst()) )
		     {
                        cerr << "Error: Didn't find transition to delete in rule_list\n";
		     }
	             delete t;
         
	             // Just mucked with the link lists, so need to rescan.
	             looping = true;
	             break;
		  }
               } while ((cur = cir->out_links.next(&t, cur)) != NULL);
            }
         } while(looping);
      }
      cir = (circle *)states.next();
   }

   // Remove the state
   if( !member_list_->parameter_list.remove(the_circle->get_src_sym()) )
   {
      cerr << "Error: Didn't find member to delete in member_list\n";
   }

   // remove the state from the symbol table so update doesn't check it.
   states.remove(the_circle->get_src_sym()->list_index->name);

   // Success
   return true;
}

//*************************************************************************

// Returns true if success
bool
fsa::delete_state(circle *the_circle)
{
   if( !unhook_state(the_circle) )
      return false;

   // Delete the glyph.
   delete the_circle;

   // Success
   return true;
}

//*************************************************************************

void
fsa::include_state(circle *p)
{
   states.put(p->name(), p); 
}

//*************************************************************************

bool 
fsa::ok_to_add_outlink(class screen_object *s)
{
   const circle *cir = states.first();
   while (cir)
   {
      if( cir == (circle *)s )
      {
	 if( cir->is_start() && cir->out_links.len() > 0 )
	    return false;
	 return true;
      }

      cir = states.next();
   }

   warn_userf("Error: State not found in ok_to_add_outlink!");
   return false;
}

//*************************************************************************

bool 
fsa::is_start_state(class screen_object *s)
{
   const circle *cir = states.first();
   while (cir)
   {
      if( cir == (circle *)s )
      {
	 if( cir->is_start() )
	    return true;
	 return false;
      }

      cir = states.next();
   }

   warn_userf("Error: State not found in ok_to_add_inlink!");
   return false;
}

//-----------------------------------------------------------------------
// This function turns off all the highlights of states.
//-----------------------------------------------------------------------
void fsa::clearHighlightStates(void)
{
    circle *state = NULL;

    state = (circle *)states.first();

    while(state != NULL)
    {
        if (state->isHighlighted())
        {
            state->highlightCircle(false);
        }

        state = (circle *)states.next();
    }
}

///////////////////////////////////////////////////////////////////////
// $Log: fsa.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 22:25:25  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.54  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.53  2000/04/22 18:58:11  endo
// Checked in for Doug.
// This patch fixes three more problems keeping cfgedit from duplicating
// large FSAs.  These bugs added many extra links to the states and triggers,
// causing the duplication process to use exponential amounts of memory.
// I have tested the patched cfgedit by duplicating FSAs with 100 waypoints
// with no noticable slowdown.
//
// Revision 1.52  2000/04/13 21:43:41  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.51  2000/03/31 17:06:26  endo
// fsa::getNumberOfStatesInFSA(void) added.
//
// Revision 1.50  2000/01/22 20:13:15  endo
// This modification will allow cfgedit FSA
// to choose the default trigger based on
// the state the trigger is originated
// from. For example, the default trigger
// of GoTo state will be AtGoal.
//
// Revision 1.49  1999/09/03 21:15:44  endo
//  Made "Immediate" to be a default trigger.
//
// Revision 1.48  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.47  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.47  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.46  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.45  1996/03/13  01:55:23  doug
// *** empty log message ***
//
// Revision 1.44  1996/03/12  17:45:14  doug
// *** empty log message ***
//
// Revision 1.43  1996/03/09  01:09:23  doug
// *** empty log message ***
//
// Revision 1.42  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.41  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.40  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.39  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.38  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.37  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.36  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.35  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.34  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.33  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.32  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.31  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.30  1995/11/12  22:39:46  doug
// *** empty log message ***
//
// Revision 1.29  1995/07/06  20:38:14  doug
// *** empty log message ***
//
// Revision 1.28  1995/06/29  17:51:32  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
