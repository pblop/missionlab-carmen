/**********************************************************************
 **                                                                  **
 **                              page.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: page.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

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

/*-----------------------------------------------------------------------*/

void 
Page::erase()
{
    XFillRectangle(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.erase,
        0, 
        0,
        DRAWING_WIDTH,
        DRAWING_HEIGHT);
}

/*-----------------------------------------------------------------------*/

// called to clear an old page from the screen
Page::~Page()
{
    assert(this);

    // save any changes
    if( !skip_update_ )
        update();

    skip_update_ = false;

    // Delete the objects
    while( !objs.isempty() )
    {
        screen_object *p = objs.get();
        delete( p );
    };

    // Erase the screen
    erase();
}

/*-----------------------------------------------------------------------*/

// draw the node and recurse on its source parm nodes
void
Page::Create_tree(Symbol * new_agent, Location *loc)
{
   if( new_agent == NULL )
      return;

   if( new_agent->marked )
      return;

   if( new_agent->symbol_type == BP_NAME && new_agent->is_bound_bp() )
   {
      // If this is a binding point and it is bound, then show the
      // device driver instead of the binding point
      Create_tree(new_agent->bound_to, &new_agent->bound_to->location);
      return;
   }

   // If this is a grouping agent, then we won't show detail in the glyph
   bool is_grouper = new_agent->is_grouping_op();

   // Create a glyph for the record
   glyph *node = new glyph(new_agent, !is_grouper, loc);

   // Position and display the new record
   node->set_xy(loc->x, loc->y);
   node->draw();
   node->draw_links();

   // Mark this node as generated
   new_agent->marked = true;

   // Create its direct children
//DCM 12-4-99:   if( !is_grouper )
   {
	 Symbol *p;
	 void *cur;

	 if ((cur = new_agent->parameter_list.first(&p)) != NULL)
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
//DCM 12-4-99: Check that don't instantiate a group under a group.
               if (lp->input_generator &&
                   lp->input_generator->is_agent() &&
                   (!is_grouper || !lp->input_generator->is_grouping_op()) )
			{
			   assert(lp->list_index);

			   Create_tree(lp->input_generator,
				       &lp->list_index->location);
			}
		     }
		     while ((cur_lp = p->parameter_list.next(&lp, cur_lp)) != NULL);
		  }
	       }
	       else
	       {
//DCM 12-4-99: Check that don't instantiate a group under a group.
             if (p->input_generator &&
                 p->input_generator->is_agent() &&
                 (!is_grouper || !p->input_generator->is_grouping_op()) )
		  {
		     Create_tree(p->input_generator,
				 &p->location);
		  }
	       }
	    }
	    while ((cur = new_agent->parameter_list.next(&p, cur)) != NULL);
	 }
   }
}

/*-----------------------------------------------------------------------*/

// write a new page up on the screen
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

// NOTE The constructor calls:  this makes sure the lists are created first
Page::Page(Symbol * new_agent)
{
   // Make sure things are as expected
   assert(this);
   assert(new_agent);
   assert(new_agent->is_grouping_op());


   skip_update_ = false;
   _moving_glyph = false;
   fsa_node_ = NULL;

   if( new_agent->is_fsa() )
   {
      // FSA are handled as a special case because we want to show the states.
      fsa_node_ = new fsa(new_agent);

      // Draw it
      fsa_node_->draw();
   }
   else
   {
      // Reset the marks.
      // It is likely that one input tree can be attached to multiple consumers
      // on the same page.  In that case, we only show it once.
      new_agent->clear_marks();

      // Create the tree for each of the children.
      // There are no parms in grouping nodes
      Symbol *p;
      void *cur;
      if ((cur = new_agent->children.first(&p)) != NULL)
      {
         do
         {
            Create_tree(p,&p->location);
         } while ((cur = new_agent->children.next(&p, cur)) != NULL);
      }

      // draw the connecting links
      draw_links();
   }

   agent = new_agent;
}

//-----------------------------------------------------------------------

// called to draw all the links 
void
Page::draw_links()
{
   assert(this);

   screen_object *p;
   void *cur;

   if ((cur = objs.first(&p)) != NULL)
   {
      do
      {
	 p->draw_links();
      }
      while ((cur = objs.next(&p, cur)) != NULL);
   }
}

//-----------------------------------------------------------------------

// called to refresh the screen
void
Page::refresh()
{
   assert(this);

   if (fsa_node_)
   {
      fsa_node_->draw();
   }
   else
   {
      draw_links();
   }
}

//-----------------------------------------------------------------------

// called to lookup a glyph from a widget
screen_object *
Page::widget2obj(Widget w)
{
   assert(this);

   screen_object *p;
   void *cur;

   if ((cur = objs.first(&p)) != NULL)
   {
      do
      {
	 if (p->object_widget() == w)
	    return p;
      }
      while ((cur = objs.next(&p, cur)) != NULL);
   }

   return NULL;
}

//-----------------------------------------------------------------------

// called to lookup a glyph from a symbol
screen_object *
Page::symbol2obj(Symbol *rec)
{
   assert(this);

   screen_object *p;
   void *cur;

   if ((cur = objs.first(&p)) != NULL)
   {
      do
      {
	 if (p->get_src_sym() == rec)
	    return p;
      }
      while ((cur = objs.next(&p, cur)) != NULL);
   }
   return NULL;
}

//-----------------------------------------------------------------------

// called to refresh the screen
void
Page::update()
{
#if 0
   if (fsa_node_)
   {
      fsa_node_->update();
   }
   else
   {
      // update the objs
      screen_object *p;
      void *cur;

      if (cur = objs.first(&p))
      {
	 do
	 {
	    p->update();
	 }
	 while (cur = objs.next(&p, cur));
      }
   }
#endif
}

//-----------------------------------------------------------------------
#if 0
// called to move the waiting glyph into the page.
void
Page::include_waiting()
{
   glyph *g = config->get_waiting();

   // Add the waiting glyph into the list of objs on the page
   objs.append(g);

   // Get the record defining the new glyph
   Symbol *p = g->get_src_sym();

   // If is an instance of a binding point, we want to deal with the
   // binding point itself.
   if( p->is_hdw() )
   {  
      p = p->bound_to;
   }

   // if this is the first agent on the page, then it defines it.
   // DCM: I don't think this ever happens.
   if (agent == NULL)
   {
      agent = p;
   }
   else
   {
      // Add the new symbol as a child of the parent symbol defining this page.
      agent->children.append( p );

      // Add the parent symbol as a user of this new symbol 
      p->users.append( agent );
   }

   config->clear_waiting();
}

//-----------------------------------------------------------------------

// called to move the waiting state into the page.
void
Page::include_waiting_state()
{
   // Add the waiting state into the table of states in the fsa
   fsa_node_->include_state(waiting_state_);

   // Get the agent record hanging under the new state
   Symbol *p = waiting_state_->agent();

   // Add the fsa symbol as a user of the group hanging under the state 
   if( p != NULL )
   {
//      p->users.append( waiting_state_->index() );
      p->users.append( fsa_node_->fsa_node() );
   }

   // Mark that are done
   waiting_state_ = NULL;
}
#endif

//-----------------------------------------------------------------------

// Called to form a group around the symbol "node".
// There may be many users of "node" and they will be modified to
// no point to the new group, which will be the only user of "node".
void
Page::group(Symbol *node)
{
   // Since a lot of nodes may be pointing at this record
   // I will allocate a new record for "node" and make this
   // one the group.  It saves a bunch of work.

   // Make a copy of the node symbol 
   Symbol *new_node = new Symbol;
   *new_node = *node;

   Symbol *group = node;
   group->symbol_type = GROUP_NAME;
   group->record_class = RC_USER;
   group->name = strdupcat(group->name,"_group");
   group->defining_rec = NULL;

   // The selected node defines the group
   group->children.append(new_node);

   // users of the node now use the group instead
   // so the group is the only user of the node
   new_node->users.clear();
   new_node->users.append(group);
}

//-----------------------------------------------------------------------

// called to check if a symbol is included on the current page
bool 
Page::is_on_this_page(Symbol *sym)
{
   assert(this);
   assert(sym);

   // If the symbol defines this page, then yes.
   if( sym == agent )
      return true;

   // Else, check each of the objs to see if they match.
   screen_object *p;
   void *cur;
   if ((cur = objs.first(&p)) != NULL)
   {
      do
      {
	 if( p->get_src_sym() == sym )
	    return true;
      } while ((cur = objs.next(&p, cur)) != NULL);
   }
   return false;
}


///////////////////////////////////////////////////////////////////////
// $Log: page.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.41  2000/04/13 21:47:54  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.40  2000/01/20 03:24:59  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.39  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.38  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.38  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.37  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.36  1996/02/08  19:21:56  doug
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
// Revision 1.31  1996/01/20  23:40:47  doug
// Split nodes code was completely rewritten to present multiple choices
//
// Revision 1.30  1996/01/19  20:54:42  doug
// *** empty log message ***
//
// Revision 1.29  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.28  1995/10/30  23:06:43  doug
// *** empty log message ***
//
// Revision 1.27  1995/10/27  20:28:50  doug
// *** empty log message ***
//
// Revision 1.26  1995/10/06  21:45:38  doug
// *** empty log message ***
//
// Revision 1.25  1995/09/26  21:43:49  doug
// working on the group function
//
// Revision 1.24  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.23  1995/06/29  18:06:49  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
