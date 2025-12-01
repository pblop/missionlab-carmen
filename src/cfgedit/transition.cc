/**********************************************************************
 **                                                                  **
 **                          transition.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: transition.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <Xm/Xm.h>
#include <Xm/Label.h>

#include <assert.h>
#include <math.h>

#include "gt_std.h"
#include "load_cdl.h"
#include "configuration.hpp"     
#include "globals.h"
#include "screen_object.hpp" 
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp" 
#include "fsa.hpp"
#include "transition.hpp" 
#include "circle.hpp"
#include "buildbitmaps.h"
#include "prototypes.h"
#include "callbacks.h"
#include "toolbar.h"
#include "DrawArrow.h"
#include "ConstructPrintName.h"
#include "popups.h"
#include "design.h"

//********************************************************************
// Constants
//********************************************************************
const int transition::BUBBLE_RADIUS_ = 5;
const int transition::ARROW_LENGTH_ = 10;

//********************************************************************

transition::~transition()
{
   // Forget the screen_object
   if( inst_ )
      inst_->this_screen_object = NULL;

   // remove it from the list of objects on the screen
   remove_obj(this);

   if( rule_widget_ )
   {
      // Destroy the widget
      XtDestroyWidget(rule_widget_);
   }
}


//********************************************************************

transition::transition(circle *src, circle *dest, Symbol *inst)
{
   assert(src);
   assert(dest);
   assert(inst);
   assert(inst->input_generator);
   assert(inst->input_generator->input_generator);

   // Set position caches to invalid values so will update first time.
   x_ = -1;
   y_ = -1;
   last_src_x_ = -1;
   last_src_y_ = -1;
   last_des_x_ = -1;
   last_des_y_ = -1;


   src_ = src;
   dest_ = dest;
   inst_ = inst;
   managed_ = false;
   moved_ = false;
   lifted_ = false;

   if( !inst_->input_generator->input_generator->location.is_default() )
   {
      x_ = inst_->input_generator->input_generator->location.x;
      y_ = inst_->input_generator->input_generator->location.y;
      moved_ = true;
   }

   // Remember the inputs and outputs  
   src->out_links.append(this);
   dest->in_links.append(this);

   //*********************** create the base glyph *********************
 
   char *trans_name = ConstructPrintName(inst_->input_generator->input_generator);
   bool is_sel =  is_selected(inst_);

   String translations =
   "Shift <Btn1Down>:   da_mouse_cb(1shift_click)\n\
    <Btn1Down>:         da_mouse_cb(1down)\n\
    Shift <Btn2Down>:   da_mouse_cb(2shift_click)\n\
    <Btn2Down>:         da_mouse_cb(2down)\n\
    <Btn1Up>:           da_mouse_cb(up)\n\
    <Btn1Motion>:       da_mouse_cb(btnmove)\n\
    <Btn3Down>:   	da_mouse_cb(3down)";

   XmString str = XmStringCreateLtoR(
       trans_name,
       (char *)(gFONTTAG_CLEAN_MEDIUM_12.c_str()));
   rule_widget_ = XtVaCreateManagedWidget(
       "", xmLabelWidgetClass,
       drawing_area,
       XmNalignment,   XmALIGNMENT_BEGINNING,
       XmNlabelString, str,
       XmNbackground, is_sel ? gCfgEditPixels.select : gCfgEditPixels.transitionBg,
       NULL);
   XmStringFree(str);
   free(trans_name);
   trans_name = NULL;

   // Save these augmentations as part of the default list
   XtOverrideTranslations(rule_widget_, XtParseTranslationTable(translations));

   // Get the default translations so we can restore.
   XtVaGetValues(rule_widget_,
           XmNtranslations, &default_translations,
           NULL);
   
   // get height and width of form so can center it.
   XtVaGetValues(rule_widget_,
           XmNwidth, &width_,
           XmNheight, &height_,
           NULL);

   XtUnmanageChild(rule_widget_);

   // Remember the screen_object.
   if( inst_ )
      inst_->this_screen_object = (void *)this;

   // add it to the list of objects on the screen
   add_obj(this);
}


//********************************************************************

void
transition::update()
{
   if( inst_ && 
       inst_->input_generator && 
       inst_->input_generator->input_generator &&
       moved_ )
   {
      // Get current values
      XtVaGetValues(rule_widget_,
        	XmNx, &x_,
        	XmNy, &y_,
        	NULL);

      x_ = (int)(x_ + width_/2);
      y_ = (int)(y_ + height_/2);
          
      inst_->input_generator->input_generator->location.x = x_;
      inst_->input_generator->input_generator->location.y = y_;
   }
}

//********************************************************************

void transition::compute_locations()
{
   // If nothing has moved, there is nothing to do.
   if( last_src_x_ == src_->x() &&
       last_src_y_ == src_->y() &&
       last_des_x_ == dest_->x() &&
       last_des_y_ == dest_->y() )
   {
      return;
   }

   // Recompute p1, p2, and x_,y_

   last_src_x_ = src_->x();
   last_src_y_ = src_->y();
   last_des_x_ = dest_->x();
   last_des_y_ = dest_->y();

   // If we have default values for x_,y_ then compute new ones
   // If the user has placed it at a specific point, leave it there.
   if( !moved_ )
   {
      // This is a bit tricky.
      // Have the centers of two circles and want to compute the point 1/2 way
      // between their edges.
      // So, need to subtract off the radius of each circle from the ends.

      // Make a vector from center of circle 1 to center of circle 2
      Vector v = {last_des_x_ - last_src_x_, last_des_y_ - last_src_y_};

      // Make a unit vector from center of circle 1 towards circle 2
      Vector u = v;
      unit_2d(u);

      // Compute the distance between the edges of the circles
      double len = len_2d(v) - dest_->r() - src_->r();	  

      // Now make a vector to the midpoint between the edges of the circles
      // Also, offset the point 5 pixels towards the source since the
      // arrow head will be on the dest end.  It just looks better.
      Vector mid = u;
      mult_2d(mid, src_->r() + len/2 - 5);

      // convert it to screen coordinates
      mid.x += last_src_x_;
      mid.y += last_src_y_;

      // check if there is a return link
      transition *t;
      void *cur;
      bool is_return_link = false;
      if ((cur = dest_->out_links.first(&t)) != NULL)
      {
         do
         {
            if (t->dest() == src_)
            {
               is_return_link = true;
               break;
            }
         } while ((cur=dest_->out_links.next(&t, cur)) != NULL);
      }

      // if there is a return link, then we want to offset the trigger glyphs
      if( is_return_link )
      {
	 // Using the right-hand rule, offset the trigger by 2/3 the height
         Vector offset = u;

         // mult_2d(offset, (int)((double)height_ * 2.0/3.0 + 0.5));
         mult_2d(offset, 35);
         rotate_z(offset,90);

         // add on the offset
	 mid.x += offset.x;
	 mid.y += offset.y;
      }

      // Remember the location
      x_ = (int)(mid.x + 0.5);
      y_ = (int)(mid.y + 0.5);
   }

   // Now compute p1,p2  These are the contact points of the src and des circles

   // This is a bit tricky.
   // Have the centers of two circles and the mid point of the glyph
   // Want to draw two lines to connect them
   // So, need to subtract off the radius of each circle from the ends.


   // Make a unit vector from center of circle 1 towards the mid point
   Vector v;
   v.x = (int)x_ - last_src_x_;
   v.y = (int)y_ - last_src_y_;
   unit_2d(v);

   // Now, figure out the point of contact on circle 1 (p1)
   mult_2d(v, src_->r());
   last_p1_x_ = (int)(v.x + 0.5) + last_src_x_;
   last_p1_y_ = (int)(v.y + 0.5) + last_src_y_;


   // Make a unit vector from center of circle 2 towards the midpoint
   v.x = (int)x_ - last_des_x_;
   v.y = (int)y_ - last_des_y_;
   unit_2d(v);

   // Now figure out the point of contract on circle 2 (p2)
   mult_2d(v, dest_->r());
   last_p2_x_ = (int)(v.x + 0.5) + last_des_x_;
   last_p2_y_ = (int)(v.y + 0.5) + last_des_y_;

   // Now compute where the bubble on the output node goes (at p1).

   // Make a unit vector from center of circle 1 to p1
   v.x = (int)last_p1_x_ - last_src_x_;
   v.y = (int)last_p1_y_ - last_src_y_;

   last_bubble_x_ = last_p1_x_ - BUBBLE_RADIUS_;
   last_bubble_y_ = last_p1_y_ - BUBBLE_RADIUS_;

   double theta = RADIANS_TO_DEGREES(atan2(-1 * v.y, v.x));
      
   last_bubble_t1_ = (int)((theta - 90) * 64);
   last_bubble_t2_ = 180 * 64; 
}


//********************************************************************

void transition::draw()
{
    if(!lifted_)
    {
        assert( rule_widget_ );

        GC ourGC;
        if(is_selected(inst_))
        {
            ourGC = gCfgEditGCs.select;
        }
        else
        {
            ourGC = gCfgEditGCs.transition;
        }

        compute_locations();

        // Draw the bubble on the output state
        XFillArc(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            ourGC, 
            last_bubble_x_,
            last_bubble_y_,
            BUBBLE_RADIUS_*2,
            BUBBLE_RADIUS_*2,
            last_bubble_t1_,
            last_bubble_t2_);

        // Draw the source connection without an arrow head
        XDrawLine(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            ourGC, 
            last_p1_x_,
            last_p1_y_,
            x_,
            y_);

        // draw a line from the mid point to the dest circle with arrow head
        DrawArrow(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            ourGC,
            x_,
            y_,
            last_p2_x_,
            last_p2_y_,
            //ARROW_HEAD_LENGTH);
            ARROW_LENGTH_);

        // Update the position of the glyph

        // Get current values
        Position cur_x, cur_y;
        XtVaGetValues(
            rule_widget_,
            XmNx, &cur_x,
            XmNy, &cur_y,
            NULL);

        Position new_x = (int)(x_ - width_/2);
        Position new_y = (int)(y_ - height_/2);
          
        if( cur_x != new_x || cur_y != new_y )
        {
            // Set X,Y locations of glyph 
            XtVaSetValues(
                rule_widget_,
                XmNx, new_x,
                XmNy, new_y,
                NULL);
        }

        // If isn't managed yet, manage it.
        if( !managed_ )
        {
            XtManageChild(rule_widget_);
            managed_ = true;
        }
    }
}


//********************************************************************

void transition::xor_outline(Position x, Position y)
{
    x_ = x;
    y_ = y;
    compute_locations();

    // Draw the source connection 
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR, 
        last_p1_x_,
        last_p1_y_,
        x_,
        y_);

    // draw the dest connection
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR, 
        x_,
        y_,
        last_p2_x_,
        last_p2_y_);
}

//********************************************************************

void transition::erase()
{
   assert( rule_widget_ );

   if( managed_ )
   {
      XtUnmanageChild(rule_widget_);
      managed_ = false;
   }

   // Draw the bubble on the output state
   XFillArc(
       XtDisplay(drawing_area),
       XtWindow(drawing_area),
       gCfgEditGCs.erase, 
       last_bubble_x_,
       last_bubble_y_,
       BUBBLE_RADIUS_*2,
       BUBBLE_RADIUS_*2,
       last_bubble_t1_,
       last_bubble_t2_);

   // Draw the source connection without an arrow head
   XDrawLine(
       XtDisplay(drawing_area),
       XtWindow(drawing_area),
       gCfgEditGCs.erase, 
       last_p1_x_,
       last_p1_y_,
       x_,
       y_);

   // draw a line from the mid point to the dest circle with arrow head
   DrawArrow(
       XtDisplay(drawing_area),
       XtWindow(drawing_area),
       gCfgEditGCs.erase, 
       x_,
       y_,
       last_p2_x_,
       last_p2_y_,
       ARROW_LENGTH_);
}

//********************************************************************

Symbol *
transition::create_agent()
{        
   // Make an empty group symbol to hang under the trigger
   Symbol *group = new Symbol(GROUP_NAME);
   group->record_class = RC_USER;
   group->name = AnonymousName();
   group->location.x = 10;
   group->location.y = 10;

   // Connect the input generator
   inst_->input_generator->input_generator = group;

   return group;
}

//********************************************************************

bool
transition::cursor_hit(Position x, Position y) const
{
   int cur_x = (int)(x_ - width_/2);
   int cur_y = (int)(y_ - height_/2);

   return ( !lifted_ &&
            x > cur_x && x < cur_x + width_ &&
            y > cur_y && y < cur_y + height_);
}


//********************************************************************
// Delete this transition from the FSA.
void
transition::delete_tree()
{
   erase();
   config->this_page()->this_fsa()->delete_transition(this);
}

//********************************************************************
void
transition::handle_events()
{
   // restore the default translations.
   XtVaSetValues(rule_widget_,
           XmNtranslations, default_translations,
           NULL);
}
   
//********************************************************************
void
transition::bypass_events()
{
   // Set the translations to pass mouse events to the state machine. 
   XtVaSetValues(rule_widget_,
           XmNtranslations, mouse_translations,
           NULL);
}

/*-----------------------------------------------------------------------*/

// make a copy of the current glyph
screen_object *
transition::dup(bool entire_tree)
{
    return NULL;
}

/*-----------------------------------------------------------------------*/

// delete this transition (leaving the underlying symbol)
void
transition::unhook_leaving_visible_children()
{
   config->this_page()->this_fsa()->unhook_transition(this);
}

/*-----------------------------------------------------------------------*/

void 
transition::redraw_unselected()
{
    XtVaSetValues(
        rule_widget_,
        XmNbackground, gCfgEditPixels.transitionBg,
        NULL);
 
    // Draw the bubble on the output state
    XFillArc(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.transition, 
        last_bubble_x_,
        last_bubble_y_,
        BUBBLE_RADIUS_*2,
        BUBBLE_RADIUS_*2,
        last_bubble_t1_,
        last_bubble_t2_);

    // Draw the source connection without an arrow head
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.transition,
        last_p1_x_,
        last_p1_y_,
        x_,
        y_);

    // draw a line from the mid point to the dest circle with arrow head
    DrawArrow(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.transition,
        x_,
        y_,
        last_p2_x_,
        last_p2_y_,
        ARROW_LENGTH_);
}

/*-----------------------------------------------------------------------*/

void 
transition::redraw_selected()
{
    XtVaSetValues(
        rule_widget_,
        XmNbackground, gCfgEditPixels.select, 
        NULL);
 
    // Draw the bubble on the output state
    XFillArc(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.select, 
        last_bubble_x_,
        last_bubble_y_,
        BUBBLE_RADIUS_*2,
        BUBBLE_RADIUS_*2,
        last_bubble_t1_,
        last_bubble_t2_);

    // Draw the source connection without an arrow head
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.select, 
        last_p1_x_,
        last_p1_y_,
        x_,
        y_);

    // draw a line from the mid point to the dest circle with arrow head
    DrawArrow(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.select, 
        x_,
        y_,
        last_p2_x_,
        last_p2_y_,
        ARROW_LENGTH_);
}

/*-----------------------------------------------------------------------*/

void 
transition::set_xy(Position x, Position y)
{
   x_ = x;
   y_ = y;

   if(inst_ && inst_->input_generator&& inst_->input_generator->input_generator)
   {
      inst_->input_generator->input_generator->location.x = x;
      inst_->input_generator->input_generator->location.y = y;
   }
}

/*-----------------------------------------------------------------------*/

// When you move a transition, it sticks there.
// This allows you to clear the sticky flag and let it float again.
void 
transition::set_default_xy()
{
   if(inst_ && inst_->input_generator&& inst_->input_generator->input_generator)
   {
      inst_->input_generator->input_generator->location.set_default();
      x_ = inst_->input_generator->input_generator->location.x;
      y_ = inst_->input_generator->input_generator->location.y;
      moved_ = false;
   }
}

/*-----------------------------------------------------------------------*/

Symbol * 
transition::get_subtree() const
{
   if( inst_ && inst_->input_generator )
      return inst_->input_generator->input_generator;
   else
      return NULL;
}

/*-----------------------------------------------------------------------*/

bool
transition::can_change_agent() const
{
   return true;
}

/*-----------------------------------------------------------------------*/

void 
transition::set_subtree(Symbol *rec)
{
   if( inst_ == NULL ||
       inst_->input_generator == NULL )
   {
      warn_user("Internal Error: Unable to set a new transition trigger");
   }

   // If we got an anonymous group, just replace ours.
   if( rec->symbol_type == GROUP_NAME && 
       (rec->name == NULL || rec->name[0]=='$') )
   {
      if( inst_->input_generator->input_generator )
      {
	 // Preserve the label screen location
	 rec->location = inst_->input_generator->input_generator->location;

         // Get rid of the old one
         inst_->input_generator->input_generator->delete_tree();
      }
      inst_->input_generator->input_generator = rec;
   }
   else
   {
      // Need to insert this record into a group
      // So, if we don't have one, then make a group.

      Symbol *grp = inst_->input_generator->input_generator;
      if( grp == NULL || grp->symbol_type != GROUP_NAME )
      {
         if( grp )
         {
	    grp->delete_tree();
         }

         // Make an empty group symbol to hang under the state
         grp = new Symbol(GROUP_NAME);
         grp->record_class = RC_USER;
         grp->name = AnonymousName();
         grp->users.append( src_->fsa_rec() );

         inst_->input_generator->input_generator = grp;
      }
      else
      {
	 // If we already have one, then clean it out
         while( !grp->children.isempty() )
         {
	    Symbol *rec = grp->children.get();
	    rec->delete_tree();
         }
         while( !grp->parameter_list.isempty() )
         {
	    grp->parameter_list.get();
         }
      }

      // Connect this new input generator
      grp->children.append(rec);
      rec->users.append(grp);

      // Add any parms that are defined as pushed up in the new group
      rec->pushup_parms(grp); 
   }
}

//********************************************************************
// This massively mucks with the screen objects.
// A full page regen is necessary.
void
transition::import_object()
{
   // Import it.
	Symbol *rec = get_subtree();

	if( rec )
	{
	   rec->import_symbol();
	}
	else
	{
      warn_user("Internal Error: No transition to import!");
	}

   // Mark drawing as changed
   config->made_change();
}

///////////////////////////////////////////////////////////////////////
// $Log: transition.cc,v $
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
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.34  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.33  2000/04/13 21:49:56  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.32  1998/12/02 17:50:22  liston
// Removed the lines that remove out_links and in_links from their
// attached circles in ~transition(), because the circles have
// already been deleted by the time we get here.
//
// Revision 1.31  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.30  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.29  1996/04/11  15:11:19  doug
// *** empty log message ***
//
// Revision 1.28  1996/03/09  01:09:23  doug
// *** empty log message ***
//
// Revision 1.27  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.26  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.25  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.24  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.23  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.22  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.21  1996/02/17  17:05:53  doug
// *** empty log message ***
//
// Revision 1.20  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.19  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.18  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.17  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.16  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.15  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.14  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.13  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.12  1995/10/26  14:59:55  doug
// type checking is working
//
// Revision 1.11  1995/07/06  20:38:14  doug
// *** empty log message ***
//
// Revision 1.10  1995/06/29  18:16:34  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
