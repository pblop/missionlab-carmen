/**********************************************************************
 **                                                                  **
 **                            circle.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: circle.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <math.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/Text.h>


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
#include "fsa.hpp"
#include "transition.hpp"
#include "circle.hpp"
#include "toolbar.h"
#include "callbacks.h"
#include "DrawArrow.h"
#include "../cdl/codegens.h"  // Get def of fsa_start_state_name
#include "ConstructPrintName.h"


extern Widget drawing_area;

//********************************************************************
// Constants
//********************************************************************
const int circle::ARROW_WIDTH_ = 5;
const double circle::SLOP_DISTANCE_TO_STATE_ = 10;

//********************************************************************

circle::~circle()
{        
   // No longer a valid object
   remove_obj(this);

   // Forget the screen_object.
   parm_inst_->this_screen_object = NULL;

   XtDestroyWidget(name_widget_);
}

//********************************************************************

void circle::DrawCircle(int x, int y, GC thisGC)
{        
    // Draw the circle itself
    if(is_selected(parm_inst_))
    {
        DrawTheCircle(x, y, gCfgEditGCs.select, true);
    }
    else if (highlighted_)
    {
        DrawTheCircle(x, y, gCfgEditGCs.highlight, true);
    }
    else
    {
        DrawTheCircle(x, y, thisGC, false);
    }

    if(name_widget_)
    {
        if(!managed_)
        {
            XtManageChild(name_widget_);
            managed_ = true;
        }

        // Get current values
        Position cur_x, cur_y;
        XtVaGetValues(
            name_widget_,
            XmNx, &cur_x,
            XmNy, &cur_y,
            NULL);

        Position new_x = x_ - width/2;
        Position new_y = y_ - height/2;
          
        if( cur_x != new_x || cur_y != new_y )
        {
            // set height and width of form in the center of the circle.
            XtVaSetValues(
                name_widget_,
                XmNx, new_x,
                XmNy, new_y,
                NULL);
        }
    }
}

//********************************************************************

void 
circle::lift()
{
   lifted_ = true;

   // Lift the output connections from this state
   transition *p;
   void *cur;
   if ((cur = out_links.first(&p)) != NULL)
   {
      do
      {
	 p->erase();
	 p->lift();
      } while ((cur = out_links.next(&p, cur)) != NULL);
   }

   // lift any input connections to this state
   if ((cur = in_links.first(&p)) != NULL)
   {
      do
      {
	 p->erase();
	 p->lift();
      } while ((cur = in_links.next(&p, cur)) != NULL);
   }
}

//********************************************************************

void 
circle::unlift()
{
   lifted_ = false;

   // Lift the output connections from this state
   transition *p;
   void *cur;
   if ((cur = out_links.first(&p)) != NULL)
   {
      do
      {
	 p->unlift();
	 p->draw();
      } while ((cur = out_links.next(&p, cur)) != NULL);
   }

   // lift any input connections to this state
   if ((cur = in_links.first(&p)) != NULL)
   {
      do
      {
	 p->unlift();
	 p->draw();
      } while ((cur = in_links.next(&p, cur)) != NULL);
   }
}

//********************************************************************

void circle::draw(void)
{
   if( !lifted_ )  // Skip ones that are being dragged
   {
      XtManageChild(name_widget_);
      managed_ = true;

      DrawCircle(x_, y_, gCfgEditGCs.state);

#if 0
      // Draw the output connections from this state
      transition *p;
      void *cur;
      if (cur = out_links.first(&p))
      {
         do
         {
	    // Draw only those links going to nodes that aren't lifted.
	    if( !p->dest()->is_lifted() )
	       p->draw();
         } while (cur = out_links.next(&p, cur));
      }

      // Needed to redraw the inputs so that after a state was moved
      // they would reappear.  However, don't want to continually redraw
      // them every expose event since that would do each one twice per event.
      // so, I'll try this.  It will screw up if you try to lift a transition
      // without lifting a state.

      // Draw any input connections to this state that are lifted.
      if (cur = in_links.first(&p))
      {
         do
         {
	    if( p->is_lifted() )
	    {
	       p->unlift();
	       p->draw();
	    }
         } while (cur = in_links.next(&p, cur));
      }
#endif
   }
}

//********************************************************************

void 
circle::erase()
{
    XtUnmanageChild(name_widget_);
    managed_ = false;

    DrawTheCircle(x_, y_, gCfgEditGCs.erase, is_selected(parm_inst_));

    // Erase the output connections from this state
    transition *p;
    void *cur;
    if ((cur = out_links.first(&p)) != NULL)
    {
        do
        {
            p->erase();
        } while ((cur = out_links.next(&p, cur)) != NULL);
    }

    // Erase the input connections to this state
    if ((cur = in_links.first(&p)) != NULL)
    {
        do
        {
            p->erase();
        } while ((cur = in_links.next(&p, cur)) != NULL);
    }
}

//********************************************************************

circle::circle(Symbol *fsa_rec, Symbol *parm_inst, bool leave_up)
{
   assert(parm_inst);
   Symbol *index_record = parm_inst->list_index;
   assert(index_record);
   assert(index_record->symbol_type == INDEX_NAME);

   parm_inst_ = parm_inst;
   index_ = index_record;
   agent_ = parm_inst->input_generator;
   fsa_rec_ = fsa_rec;
   x_ = index_record->location.x;
   y_ = index_record->location.y;
   lifted_ = false;
   managed_ = false;
   highlighted_ = false;

   is_start_ = strcmp(index_->name,fsa_start_state_name) == 0;

   // Remember the screen_object.
   parm_inst_->this_screen_object = (void *) this;

   char *state_name;
   if( is_start_ )
   {
      state_name = strdup("Start");
   }
   else if( agent_ )
   {
      state_name = ConstructPrintName(agent_);
   }
   else
   {
      state_name = strdup("????");
   }

   //*********************** create the state name *********************

   String translations =
   "Shift <Btn1Down>:   da_mouse_cb(1shift_click)\n\
    <Btn1Down>:         da_mouse_cb(1down)\n\
    Shift <Btn2Down>:   da_mouse_cb(2shift_click)\n\
    <Btn2Down>:         da_mouse_cb(2down)\n\
    <Btn1Up>:           da_mouse_cb(up)\n\
    <Btn1Motion>:       da_mouse_cb(btnmove)\n\
    <Btn3Down>:   	da_mouse_cb(3down)";

   // Create a dummy widget, so that we can measure the size of the name_widget_
   // (a widget showing the name of the state.)
   //XmString tmp_str = XmStringCreateLtoR(state_name, XmSTRING_DEFAULT_CHARSET);
   XmString tmp_str = XmStringCreateLtoR(
       state_name,
       (char *)(gFONTTAG_CLEAN_BOLD_12.c_str()));
   Widget tmp_widget = XtVaCreateManagedWidget(
       "",
       xmLabelWidgetClass, drawing_area,
       XmNlabelString, tmp_str,
       NULL);

   // Getting the size of the dummy widget. (widget, height)
   XtVaGetValues(
       tmp_widget,
       XmNwidth, &width,
       XmNheight, &height,
       NULL);

   // Deleting the dummy widget.
   XmStringFree(tmp_str);
   XtDestroyWidget(tmp_widget);

   // Creating the real name_widget
   XmString str = XmStringCreateLtoR(
       state_name,
       (char *)(gFONTTAG_CLEAN_MEDIUM_12.c_str()));
   name_widget_ = XtVaCreateManagedWidget(
       "", xmLabelWidgetClass,
       drawing_area,
       XmNx, x_-width/2,
       XmNy, y_-height/2,
       XmNalignment, XmALIGNMENT_BEGINNING,
       XmNlabelString, str,
       XmNbackground, gCfgEditPixels.stateBg,
       NULL);
   XmStringFree(str);
   free(state_name);
   state_name = NULL;

   // Save these augmentations as part of the default list
   XtOverrideTranslations(name_widget_, XtParseTranslationTable(translations));

   // Get the default translations so we can restore.
   XtVaGetValues(
       name_widget_,
       XmNtranslations, &default_translations,
       NULL);


   if(leave_up)
   {
      managed_ = true;
   }
   else
   {
      XtUnmanageChild(name_widget_);
   }
   /*
   // get height and width of form so can center it.
   XtVaGetValues(name_widget_,
		XmNwidth, &width,
		XmNheight, &height,
		NULL);
    */
   // pick a good radius
   double big = (double)width * (double)width + (double)height * (double)height;
   assert(big > 1);
   r_ = (int)(sqrt(big) / 2.0 + 4);

   if(leave_up)
   {
      draw();
   }

   // add it to the list of objects on the screen
   add_obj(this);
}

//********************************************************************

// Move the glyph to the new location.
void
circle::set_xy(Position x, Position y)
{
   if( x != x_ || y != y_ )
   {
      // Move the glyph
      if( name_widget_ )
      {
         XtVaSetValues(
             name_widget_,
             XmNx, x - width/2,
             XmNy, y - height/2,
             NULL);
      }
   
      // Move our shadow parms
      x_ = x;
      y_ = y;

      // Update the definition record
      assert(index_);
      index_->location.x = x_;
      index_->location.y = y_;

      // Unstick any transitions impinging on this state.
      transition *p;
      void *cur;
      if ((cur = in_links.first(&p)) != NULL)
      {
         do
         {
	    p->set_default_xy();
         } while ((cur = in_links.next(&p, cur)) != NULL);
      }
      if ((cur = out_links.first(&p)) != NULL)
      {
         do
         {
	    p->set_default_xy();
         } while ((cur = out_links.next(&p, cur)) != NULL);
      }
   }
}

//********************************************************************

void
circle::update()
{
   assert(index_);

   // Read the name back in case changed
   if( index_->name )
      free(index_->name);
   index_->name = XmTextGetString(name_widget_);

   // update location of the state
   Position x,y;
   XtVaGetValues(name_widget_,
		XmNx, 200,
		XmNy, 200,
		NULL);

   // Determine the center of the glyph.
   x_ = x + width/2,
   y_ = y + height/2,

   index_->location.x = x_;
   index_->location.y = y_;

   // update the output connections from this state
   transition *p;
   void *cur;
   if ((cur = out_links.first(&p)) != NULL)
   {
      do
      {
         p->update();
      } while ((cur = out_links.next(&p, cur)) != NULL);
   }

   // Read the values of constants back, in case changed
   for (int i = 0; i < num_slots; i++)
   {
      // Get the new value
      if( slot_info[i].is_const && slot_info[i].input_widget )
      {
         char *val = XmTextGetString(slot_info[i].input_widget);
         if( val )
         {
            delete *slot_info[i].value;
            *slot_info[i].value = val;
         }
      }
   }
}

//********************************************************************

Symbol *
circle::create_agent()
{        
   // Make an empty group symbol to hang under the state
   Symbol *group = new Symbol(GROUP_NAME);
   group->record_class = RC_USER;
   group->name = AnonymousName();
   group->location.x = 10;
   group->location.y = 10;

   // Connect the input generator
   parm_inst_->input_generator = group;
   group->users.append(fsa_rec_);

   // Attach the agent to the circle.
//   attach_agent(group);

   return group;
}


/*-----------------------------------------------------------------------*/
// called to redraw a state as normal
void
circle::redraw_unselected()
{
    DrawTheCircle(x_, y_, gCfgEditGCs.erase, true);
    DrawTheCircle(x_, y_, gCfgEditGCs.state, false);
}

/*-----------------------------------------------------------------------*/
// called to redraw a state as selected
void
circle::redraw_selected()
{
    DrawTheCircle(x_, y_, gCfgEditGCs.select, true);
}

//********************************************************************
// Delete this state from the FSA.
void
circle::delete_tree()
{
   erase();
   config->this_page()->this_fsa()->delete_state(this);
}

//********************************************************************

void 
circle::draw_or_erase_input_links(bool erase)
{
   // draw or erase the input connections to this state

   transition *p;
   void *cur;
   if ((cur = in_links.first(&p)) != NULL)
   {
      do
      {
	 if( erase )
	    p->erase();
	 else
	    p->draw();
      } while ((cur = in_links.next(&p, cur)) != NULL);
   }
}

//********************************************************************

bool
circle::cursor_hit(Position x, Position y) const
{        
   double dist = sqrt((double)(sqr(x_ - x) + sqr(y_ - y)));

   return (dist <= (SLOP_DISTANCE_TO_STATE_+r_));
}

//********************************************************************

void
circle::DrawTheCircle(int x, int y, GC thisGC, bool fill)
{        
    int dX;

    // Draw the circle itself
    if(fill)
    {
        // Fill it with the specified color.
        XFillArc(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            thisGC,
            x-r_,
            y-r_,
            r_*2,
            r_*2,
            0,
            360*64);
    }
    else
    {
        // Fill it with the background color.
        XFillArc(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            gCfgEditGCs.erase,
            x-r_,
            y-r_,
            r_*2,
            r_*2,
            0,
            360*64);
    }

    XDrawArc(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        thisGC,
        x-r_,
        y-r_,
        r_*2,
        r_*2,
        0,
        360*64);

    if(!is_start_)
    {
        dX = ARROW_WIDTH_;

        DrawArrow(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            thisGC,
            x+dX,
            y-r_,
            x-dX,
            y-r_,
            2*dX);

        DrawArrow(
            XtDisplay(drawing_area),
            XtWindow(drawing_area),
            thisGC,
            x-dX,
            y+r_,
            x+dX,
            y+r_,
            2*dX);
    }
}

//********************************************************************
void 
circle::xor_outline(Position x, Position y)
{
    XDrawArc(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        x-r_,
        y-r_,
        r_*2,
        r_*2,
        0,
        360*64);
}

//********************************************************************
void
circle::handle_events()
{
   // restore the default translations.
   XtVaSetValues(name_widget_,
           XmNtranslations, default_translations,
           NULL);
}
   
//********************************************************************
void
circle::bypass_events()
{
   // Set the translations to pass mouse events to the state machine. 
   XtVaSetValues(name_widget_,
           XmNtranslations, mouse_translations,
           NULL);
}

/*-----------------------------------------------------------------------*/

// make a copy of the current state
screen_object *
circle::dup(bool entire_tree)
{
   circle *new_state = NULL;

   return new_state;
}

/*-----------------------------------------------------------------------*/

// verify can make a copy of the current circle
bool
circle::verify_can_copy() const
{
   // Refuse to copy the start state
   if ( is_start_ )
   {
      warn_user("Can not copy the START state");
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// verify can delete the current circle
bool
circle::verify_can_delete() const
{
   // Refuse to delete the start state
   if ( is_start_ )
   {
      warn_user("Can not delete the START state");
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// verify can link the current circle
bool
circle::verify_can_link() const
{
   // Refuse to link the start state
   if ( is_start_ )
   {
      warn_user("Can not LINK the START state");
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// delete this state (leaving the underlying symbol)
void
circle::unhook_leaving_visible_children()
{
   config->this_page()->this_fsa()->unhook_state(this);
}

/*-----------------------------------------------------------------------*/

// Replace the current subtree
void
circle::set_subtree(Symbol *rec)
{
   // If we got an anonymous group, just replace ours.
   if( rec->symbol_type == GROUP_NAME && 
       (rec->name == NULL || rec->name[0]=='$') )
   {
      if( parm_inst_->input_generator )
         parm_inst_->input_generator->delete_tree();
      parm_inst_->input_generator = rec;
   }
   else
   {
      // Need to insert this record into a group
      // So, if we don't have one, then make a group.

      Symbol *grp = parm_inst_->input_generator;
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
         grp->users.append(fsa_rec_);
   
         parm_inst_->input_generator = grp;
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
circle::import_object()
{
   // Import it.
   Symbol *rec = get_subtree();

   if( rec )
   {
      rec->import_symbol();
   }
   else
   {
      warn_user("Internal Error: No state to import!");
   }

   // Mark drawing as changed
   config->made_change();
}

///////////////////////////////////////////////////////////////////////
// $Log: circle.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.3  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.2  2005/02/07 22:25:25  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.45  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.44  2000/04/13 21:37:26  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.43  1999/12/16 21:47:14  endo
// rh-6.0 porting.
//
// Revision 1.42  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.42  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.41  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.41  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.40  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.39  1996/03/09  01:09:23  doug
// *** empty log message ***
//
// Revision 1.38  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.37  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.36  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.35  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.34  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.33  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.32  1996/02/17  17:05:53  doug
// *** empty log message ***
//
// Revision 1.31  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.30  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.29  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.28  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.27  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.26  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.25  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.24  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.23  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.22  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.21  1995/11/04  18:57:35  doug
// *** empty log message ***
//
// Revision 1.20  1995/10/31  19:22:01  doug
// handle text field changes in loose focus callback
//
// Revision 1.19  1995/07/06  20:38:14  doug
// *** empty log message ***
//
// Revision 1.18  1995/06/29  17:31:26  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
