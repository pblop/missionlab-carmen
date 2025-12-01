/**********************************************************************
 **                                                                  **
 **                       so_movement.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: so_movement.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <Xm/Xm.h>

#include "load_cdl.h"
#include "design.h"
#include "screen_object.hpp"
#include "configuration.hpp"
#include "page.hpp"
#include "fsa.hpp"
#include "toolbar.h"
#include "globals.h"
#include "popups.h"
#include "so_movement.h"
#include "EditParms.h"
#include "PushupParms.h"
#include "PickAgent.h"
#include "EventLogging.h"
#include "ConstructPrintName.h"
#include "utilities.h"
#include "HaveParameters.h"
#include "import_symbol.h"
#define  USE_MATCHED_STATE_AND_TRIGGER
#include "cfgedit_common.h" // Mathced pairs of state and triggers are defined.

/********************************************************************

This code handles 5 events in a generic fashion using screen_objects
1. Button down on object
2. Button down on background
3. Button up
4. Mouse movement with button down
5. Mouse movement with button up

********************************************************************/

static Position cursor_x;
static Position cursor_y;
static Position offset_x;
static Position offset_y;

static screen_object *cur_object = NULL;
static screen_object *transition_start = NULL;

static Position rubber_band_fp_x;
static Position rubber_band_fp_y;

static int grabs_pending = 0;

// Contains the name of the trigger the user is about to create.
char *this_trigger = NULL;

//********************************************************************

void 
remove_grabs(void)
{        
   while( grabs_pending > 0 )
   {
      XtRemoveGrab(drawing_area);
      grabs_pending --;
   }
}

//********************************************************************

void 
start_rubber_band(screen_object *s, Position x, Position y)
{        
    s->get_output_xy(&rubber_band_fp_x, &rubber_band_fp_y);

    // Move to new spot
    cursor_x = x;
    cursor_y = y;

    // draw 
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        rubber_band_fp_x,
        rubber_band_fp_y,
        cursor_x,
        cursor_y);
}

//********************************************************************  

static void 
move_rubber_band(Position x, Position y)
{  
    // Erase from old position
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        rubber_band_fp_x,
        rubber_band_fp_y,
        cursor_x,
        cursor_y);

    // Move to new spot
    cursor_x = x;
    cursor_y = y;

    // redraw 
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        rubber_band_fp_x,
        rubber_band_fp_y,
        cursor_x,
        cursor_y);
}

//********************************************************************

void 
end_rubber_band(void)
{        
    // Erase from old position
    XDrawLine(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        gCfgEditGCs.XOR,
        rubber_band_fp_x,
        rubber_band_fp_y,
        cursor_x,
        cursor_y);
}

//********************************************************************

void 
internal_place_obj(Position x, Position y)
{        
   // Erase old copy
   cur_object->xor_outline(cursor_x + offset_x, cursor_y + offset_y);

   // Set the final location
   cur_object->set_xy(x + offset_x, y + offset_y);

   // replace object so gets redrawn
   cur_object->unlift();

   // Draw final version
   cur_object->draw();
   cur_object->draw_links();

   // Mark drawing as changed
   config->made_change();

   // Remove the cursor grab
   remove_grabs();

   // Nothing pending.
   cur_object = NULL;

   // Reset operating mode.
   set_mode(NULL, EDIT_MODE);

   // Back to normal 
   objects_handle_events();
}

//********************************************************************

void 
abort_place_obj(void)
{        
   if( cur_object )
   {
      // Erase old copy
      cur_object->xor_outline(cursor_x + offset_x, cursor_y + offset_y);

      delete cur_object;

      // Nothing pending.
      cur_object = NULL;
   }
}

//********************************************************************  

static void 
internal_move_obj(Position x, Position y)
{  
   // Erase from old position
   cur_object->xor_outline(cursor_x + offset_x, cursor_y + offset_y);

   // Move to new spot
   cursor_x = x;
   cursor_y = y;

   // redraw the outline
   cur_object->xor_outline(cursor_x + offset_x, cursor_y + offset_y);
}

//********************************************************************  

static void 
internal_start_move(screen_object *s, Position x, Position y)
{  
   gEventLogging->start("Move Object <%d %d>", x, y);
   cur_object = s;

   cursor_x = x;
   cursor_y = y;

   // Calculate an offset to draw the object at so it doesn't jump
   Position s_x, s_y;
   s->get_xy(&s_x, &s_y);

   offset_x = s_x - x;
   offset_y = s_y - y;

   // Lift so doesn't get redrawn
   s->lift();

   // So we get all mouse movement events
   objects_bypass_events();

   // Erase current version
   s->erase();
   s->erase_links();

   // Replace with an outline
   s->xor_outline(cursor_x + offset_x, cursor_y + offset_y);

   // grab the cursor
   XtAddGrab(drawing_area, true, false);
   // Also keep the cancel button active.
   AddSystemGrabs();

   grabs_pending++;
}

//********************************************************************  

static void
internal_start_connect_transiton(screen_object *s, Position x, Position y)
{
  // Name the default of this trigger to be the name
  // of a trigger that matches to the state it is
  // originated from.
  Symbol *sub = s->get_subtree();
  char *state_name = ObjectName(sub);
  int pos = 0;
  this_trigger = NULL;

  while( matched_state_trigger[pos].state )
  {
    if( strcasecmp(matched_state_trigger[pos].state, state_name)==0 )
      this_trigger = matched_state_trigger[pos].trigger;
    
    pos ++;
  }

  // Remember this as the source for the transition
  transition_start = s;
  start_rubber_band(s, x, y);
}

//********************************************************************

// Called to connect an object to the current page.

// NOTE: May need to add a containing object level which will cause
//       screen_objects to all be deleted and recreated!!!

void 
connect_object_to_page(Symbol *p)
{        
   // If is an instance of a binding point, we want to deal with the
   // binding point itself.
   if( p->is_hdw() )
   {
      p = p->bound_to;  
   }

      // Add the new symbol as a child of the parent symbol defining this page.
      config->this_page()->this_agent()->children.append( p );

      // Add the parent symbol as a user of this new symbol
      p->users.append( config->this_page()->this_agent() );

      // Check if there is now more than one object at the top level.
      if( config->at_top() && num_objs() > 1 )
      {
         // Create a new umbrella object that adds a new top level
         // with a single object.  This is necessary to allow copying complete 
         // configurations.

         // Make a new group node
         Symbol *group = new Symbol(GROUP_NAME);
         group->name = AnonymousName();
         group->record_class = RC_USER;
         group->defining_rec = NULL;
    
         // The children list are what the configuration node had
         Symbol *cfg = config->root();
         group->children = cfg->children;

         // The configuration node now just has the group as a child
         cfg->children.clear();
         cfg->children.append(group);

         // Now modify the group's new children so they point to the group
         // as their user instead of the configuration.
         Symbol *p;
         void *cur;
   
         if ((cur = group->children.first(&p)) != NULL)
         {
            do
            {
               p->users.remove(cfg);
               p->users.append(group);
            }
            while ((cur = group->children.next(&p, cur)) != NULL);
         }  
   
         // Now need to move into the new group so still see the existing items.
         config->move_down(group,true);
      }
}

//********************************************************************

// Called when an object is ready for placement.
// It should NOT have been drawn yet.
// States must have been added already with fsa::add_state
// This function will add glyphs to the page.

void 
place_object(screen_object *s)
{        
   Symbol *rec = s->get_src_sym();

   // If is a state, it has already been added, so skip it.
   if ( !s->is_state() )
	{
      // Put it unconnected into the current page.
      connect_object_to_page(rec);
	}

   // NOTE *** s is now invalid because connect_object_to_page may recreate it

   cur_object = (screen_object *)rec->this_screen_object;

   // lift object so doesn't get redrawn due to expose events
   cur_object->erase();
   cur_object->lift();

   // So we get all mouse movement events
   objects_bypass_events();

   // Find out where the mouse is.
   Window root, child;
   int    root_x, root_y;		// position in root window
   int    win_x, win_y;                 // position in w's frame
   unsigned int keys_buttons;		//status of mouse buttons
   if( XQueryPointer(XtDisplay(drawing_area),XtWindow(drawing_area), 
		&root, &child, &root_x, &root_y, &win_x, &win_y,
		&keys_buttons))
   {
      // Pointer is on the screen where the window is.
      cursor_x = win_x;
      cursor_y = win_y;
   }
   else
   {
      // Pointer is off the screen.
      cursor_x = 100;
      cursor_y = 100;
   }

   offset_x = 0;
   offset_y = 0;

   cur_object->xor_outline(cursor_x + offset_x, cursor_y + offset_y);

#if 0
   // grab the cursor
   XtAddGrab(drawing_area, true, false);
   // Also keep the cancel button active.
   AddSystemGrabs();
   grabs_pending++;
#endif

   set_mode(NULL, PLACEMENT);

}

//********************************************************************

void 
button1_down_on_object(screen_object *s, Position x, Position y)
{        
    Symbol *rec = s->get_src_sym();

    gEventLogging->log("Button1 pressed on the object %s [%s]", rec->name, rec->description);

    assert(rec);

    switch(system_mode) {

    default:
        warn_userf("Error: System mode %d not handled in button_down_on_object\n",
                   system_mode);

        // Revert back to edit mode
        set_mode(NULL, EDIT_MODE);
        break;

    case EDIT_MODE:
        if( !is_selected(s) )
        {
            // Clear any other selections
            unselect();
	 
            // Make this object selected, if possible.
            if( !select(s) ) break;
        }

        // Start moving the object
        internal_start_move(s, x, y);
        set_mode(NULL, DRAGGING);
        break;

    case MOVE_MODE:
        // Start moving the object
        internal_start_move(s, x, y);
        set_mode(NULL, DRAGGING);
        break;

    case START_CONNECT_TRANSITION:
        if( config->this_page()->this_fsa()->ok_to_add_outlink(s) )
        {
            internal_start_connect_transiton(s, x, y);
            set_mode(NULL, CONNECTING_TRANSITION);
        }
        else
        {
            warn_user("The Start state already has an output transition");
            set_mode(NULL, EDIT_MODE);
        }

        break;

    case PLACE_STATE:
    case NEW_PRIMITIVE:
    case NEW_RBP:
    case NEW_OBP:
    case NEW_IBP:
    case NEW_OPERATOR:
    case MAKING_CONNECTION:
    case PLACEMENT:
    case DRAGGING:
        break;


        // Handle pushing up a parameter from a state or trigger
    case PUSHUP:
        int style = s->has_subtree();
        if( style > 0 )
        {
            Symbol *sub = s->get_subtree();
            if( config->this_page()->is_fsa() )
            {
                if( !HaveParameters(sub) )
                {
                    set_mode(NULL, EDIT_MODE);
                    warn_user("This object has no parameters to push up!");
                    return;
                }
               
                if( (user_privileges & MODIFY_PRIV) == 0 )
                {
                    set_mode(NULL, EDIT_MODE);
                    warn_user("You have insufficient privilages to pushup parameters");
                    return;
                }

                PushupParms(config->this_page()->this_fsa(), sub);
            }
        }
        else
        {
            warn_user("This object is not modifiable");
        }

        set_mode(NULL, EDIT_MODE);
        break;
    }
}

//********************************************************************

void button2_down_on_object(screen_object *s, Position x, Position y)
{      
    unselect();

    Symbol *rec = s->get_src_sym();

    gEventLogging->log("Button2 pressed on the object %s [%s]", rec->name, rec->description);

    if( system_mode == EDIT_MODE )
    {            
        int style = s->has_subtree();
        if( style > 0 )
        {
            Symbol *sub = s->get_subtree();
            if( config->this_page()->is_fsa() )
            {
                if( !HaveParameters(sub) )
                {
                    warn_user("This object has no modifiable parameters");
                    return;
                }

                if( (user_privileges & MODIFY_PRIV) == 0 )
                {
                    warn_user("You have insufficient privilages to modify parameters");
                    return;
                }

                // Check if it is read only.
                if( sub->record_class != RC_USER )
                {
                    // Yes, try to import it.
                    import_symbol(s);

                    // Redraw the page.  We mucked it up pretty good.
                    config->regen_page();

                    // Return since the mouse button is no longer down.
                    return;
                }

                if( s->is_state() )
                {
                    Symbol *p = s->get_src_sym();
                    if( p && p->list_index && p->list_index->description )
                    {
                        gEventLogging->startModify(
                            "Parms %s \"%s\"",
                            p->list_index->description,
                            strip_crs(ConstructPrintName(s->get_subtree())));
                    }
                    else
                    {
                        gEventLogging->startModify(
                            "Parms State??? \"%s\"",
                            strip_crs(ConstructPrintName(s->get_subtree())));
                    }
                }
                else
                {
                    Symbol *p = s->get_subtree();
               
                    if( p && p->description )
                    {
                        gEventLogging->startModify(
                            "Parms %s \"%s\"",
                            p->description,
                            strip_crs(ConstructPrintName(s->get_subtree())));
                    }
                    else
                    {
                        gEventLogging->startModify(
                            "Parms Trans??? \"%s\"",
                            strip_crs(ConstructPrintName(s->get_subtree())));
                    }
                }

                EditParms(sub, false);

                gEventLogging->endModify(
                    "Parms \"%s\"",
                    strip_crs(ConstructPrintName(sub)));

                // Write CDL file showing change in configuration
                save_cdl_for_replay("modified_parameters");        
            }
            else
            {
                if(sub->record_class == RC_LIBRARY &&
                   (user_privileges & LIBRARY_PRIV) == 0 )
                {
                    warn_user("You have insufficient privilages to modify library components");
                }
                else
                {
                    config->move_down(sub, style == 1);
                }
            }
        }
        else
        {
            warn_user("This object is not modifiable");
        }
    }
}


//********************************************************************

void  button2_down_on_background(Position x, Position y)
{        
   gEventLogging->log("Button2 pressed on the background");
}

//********************************************************************

void button1_down_on_background(Position x, Position y)
{        
    gEventLogging->log("Button1 pressed on the background");

    if( system_mode == PLACEMENT )
    {
        screen_object *s = cur_object;

        if(s->is_state()) gEventLogging->end("PlaceState");
        else gEventLogging->end("PlaceGlyph");

        internal_place_obj(x,y);
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("placed_state");                

#if 0
        // If we just placed a state, try to pop up the parm selection box.
        if( s->is_state() )
        {
            int style = s->has_subtree();
            Symbol *sub = s->get_subtree();
            if( style > 0 &&
                config->this_page()->is_fsa() && 
                HaveParameters(sub) &&
                (user_privileges & MODIFY_PRIV) )
            {
                button2_down_on_object(s, x, y);
            }
        }
#endif
    }
    else
    {
        // Clears all selections
        unselect();
    }
}

//********************************************************************

void 
shift1_click_on_object(screen_object *s, Position /*x*/, Position /*y*/)
{        
   Symbol *rec = s->get_src_sym();

   gEventLogging->log("Shift-Button1 pressed on the object %s [%s]", rec->name, rec->description);

   if( !config->this_page()->is_fsa() || !s->has_subtree() )
   {
      warn_user("This object is not modifiable");
      return;
   }

   if( (user_privileges & EDIT_PRIV) == 0 )
   {
      warn_user("You have insufficient privilages to select new tasks");
      return;
   }

   if( !s->can_change_agent() )
   {
      warn_user("This object is not modifiable");
      return;
   }

   // Check if it is read only.
   if( s->get_subtree()->record_class != RC_USER )
   {
      // Yes, try to import it.
      import_symbol(s);

      // Redraw the page.  We mucked it up pretty good.
      config->regen_page();

      // Return.  Let them click again if they want to change it.
      return;
   }

   PickAgent(s);

   save_cdl_for_replay("changed_state");                   
}

//********************************************************************

void 
shift2_click_on_object(screen_object *s, Position x, Position y)
{        
   Symbol *rec = s->get_src_sym();

   gEventLogging->log("Shift-Button2 pressed on the object %s [%s]", rec->name, rec->description);

   if( system_mode == EDIT_MODE )
   {            
      int style = s->has_subtree();
      if( style > 0 )
      {
         Symbol *sub = s->get_subtree();
	 if( sub->record_class == RC_LIBRARY &&
	     (user_privileges & LIBRARY_PRIV) == 0 )
	 {
            warn_user("You have insufficient privilages to modify library components");
	 }
	 else
	 {
            config->move_down(sub, style == 1);
	 }
      }
      else
      {
         warn_user("This object is not modifiable");
      }
   }
}

//********************************************************************

void 
shift1_click_on_background(Position x, Position y)
{        
   gEventLogging->log("Shift-Button1 pressed on the background");
}

//********************************************************************

void 
shift2_click_on_background(Position x, Position y)
{        
   gEventLogging->log("Shift-Button2 pressed on the background");
}

//********************************************************************  

/* planner */
screen_object * 
find_object(Position x, Position y)
{  
   // Find a matching object, if one exists
   screen_object *s;
   void *cur;

   if ((cur = objs.first(&s)) != NULL)
   {
      do
      {
         if( s->cursor_hit(x, y) )
         {
	    return s;
         }
      } while ((cur = objs.next(&s, cur)) != NULL);
   }
   return NULL;
}

//********************************************************************  

void 
button_up(Position x, Position y)
{  
   if( system_mode == DRAGGING )
   {
      gEventLogging->end("Move Object <%d %d>", x, y);

      internal_place_obj(x,y);
      set_mode(NULL, EDIT_MODE);

      save_cdl_for_replay("moved_object");               
   }
   else if( system_mode == CONNECTING_TRANSITION )
   {
      // Find a matching object, if one exists
      screen_object *s = find_object(x, y);
      screen_object *t = NULL;

      if( transition_start && s && s != transition_start )
      {
	 if( config->this_page()->this_fsa()->is_start_state(s) )
	 {
            warn_user("The Start state is not allowed to have input transitions");
	 }
	 else
	 {
            // this is the destination for the transition
	    t = config->this_page()->this_fsa()->add_transition(transition_start, s);

	    // This will update the parameters for this trigger
	    // according to the "transion_start" state, if it is
	    // necessary.
	    updateTriggerFieldAutomatically(t, transition_start);
 
	    // Mark drawing as changed
	    config->made_change();
	 }
      }
 
      // Revert back to edit mode
      set_mode(NULL, EDIT_MODE);

      gEventLogging->end("AddTransition");

      save_cdl_for_replay("added_transition");                  

#if 0
      // If we just placed a transition, try to pop up the parm selection box.
      if( t )
      {
         int style = t->has_subtree();
         Symbol *sub = t->get_subtree();
	 if( style > 0 &&
	     config->this_page()->is_fsa() && 
	     HaveParameters(sub) &&
             (user_privileges & MODIFY_PRIV) )
         {
            button2_down_on_object(t, x, y);
         }
      }
#endif

   // Regenerate the drawing to get the changes
   config->regen_page();

   // Mark drawing as changed
   config->made_change();

   }
}

//********************************************************************  

void 
mouse_movement_with_button_down(Position x, Position y)
{  
   if( system_mode == CONNECTING_TRANSITION )
   {
      move_rubber_band(x,y);
   }
   else if( cur_object )
   {
      internal_move_obj(x,y);
   }
}

//********************************************************************  

void 
mouse_movement_with_button_up(Position x, Position y)
{  
   // If placing, then drag the outline
   if( system_mode == PLACEMENT )
   {
      internal_move_obj(x,y);
   }
   else if( system_mode == MAKING_CONNECTION )
   {
      move_rubber_band(x,y);
   }
}

/*-----------------------------------------------------------------------*/
// This handles events sent from the background drawable.
// It doesn't catch events sent to an object itself, such as
// clicks inside a glyph

void
da_mouse_cb(Widget w, XButtonEvent * event, String * args, int *num_args)
{
   assert(*num_args == 1);

   // May have gotten a click inside a glyph structure.
   int x = event->x;
   int y = event->y;

   Widget t = w;
   while(t != drawing_area)
   {
      Position dx, dy;
      
      XtVaGetValues(t,
      		XmNx, &dx, 
		XmNy, &dy, 
		NULL);
      x += dx;
      y += dy;

      t = XtParent(t);
   }

   if (args[0][0] == 'm')       // "move"
   {
      mouse_movement_with_button_up(x, y);
   }
   else if (args[0][0] == '1')  // "1down"
   {
      // Find a matching object, if one exists
      screen_object *s = find_object(x, y);

      if (args[0][1] == 'd')  // "1down"
      {
         if( s )
            button1_down_on_object(s, x, y);
         else
            button1_down_on_background(x, y);
      }
      else if (args[0][1] == 's')  // "1shift"
      {
         if( s )
            shift1_click_on_object(s, x, y);
         else
            shift1_click_on_background(x, y);
      }
   }        
   else if (args[0][0] == '2')  // "2down"
   {
      // Find a matching object, if one exists
      screen_object *s = find_object(x, y);

      if (args[0][1] == 'd')  // "1down"
      {
         if( s )
            button2_down_on_object(s, x, y);
         else
            button2_down_on_background(x, y);
      }
      else if (args[0][1] == 's')  // "1shift"
      {
         if( s )
            shift2_click_on_object(s, x, y);
         else
            shift2_click_on_background(x, y);
      }
   }        
   else if (args[0][0] == '3')  // "3down"
   {
      // Find a matching object, if one exists
      screen_object *s = find_object(x, y);

      // Treat the right button same as a shift click on the left.
      if( s )
         shift1_click_on_object(s, x, y);
      else
         shift1_click_on_background(x, y);
   }        
   else if (args[0][0] == 'u')  // "up on button 1"
   {
      button_up(x, y);
   }
   else if (args[0][0] == 'b')  // "btnmove"
   {
      mouse_movement_with_button_down(x, y);
   }        
   else                         // "btnmove"
   {
      cerr << "message not handled in da_mouse_cb: " << args[0] << '\n';
   }        
}  

/*-----------------------------------------------------------------------*/
// called by the expose event to refresh the screen
void
refresh_screen(Widget w,
	       XtPointer no_client_data,
	       XmDrawingAreaCallbackStruct * callback_data)
{
   if ((callback_data->event == NULL) ||
       (callback_data->event->xexpose.count == 0))
   {
      // Redraw the screen objects
      screen_object *s;
      void *cur;

      // Draw the objects
      if ((cur = objs.first(&s)) != NULL)
      {
         do
         {
            s->draw();
         } while ((cur = objs.next(&s, cur)) != NULL);
      }  

      // Draw the links connecting the objects
      if ((cur = objs.first(&s)) != NULL)
      {
         do
         {
            s->draw_links();
         } while ((cur = objs.next(&s, cur)) != NULL);
      }  
   }
}

///////////////////////////////////////////////////////////////////////
// $Log: so_movement.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2006/08/29 15:12:31  endo
// Advanced parameter flag added.
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
// Revision 1.35  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.34  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.33  2000/06/25 03:02:27  endo
// updateTriggerFieldAutomatically() added in button_up().
//
// Revision 1.32  2000/04/22 19:00:12  endo
// Checked in for Doug.
// This patch fixes three more problems keeping cfgedit from duplicating
// large FSAs.  These bugs added many extra links to the states and triggers,
// causing the duplication process to use exponential amounts of memory.
// I have tested the patched cfgedit by duplicating FSAs with 100 waypoints
// with no noticable slowdown.
//
// Revision 1.31  2000/04/13 21:49:08  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.30  2000/03/22 02:15:26  saho
// Made find_object(Position x, Position y) to be non-static.
//
// Revision 1.29  2000/02/13 20:36:05  sapan
// Added calls to record current configuration as CDL file.
//
// Revision 1.28  2000/02/03 00:01:11  jdiaz
// waypoints
//
// Revision 1.27  2000/02/02 23:50:56  endo
// *** empty log message ***
//
// Revision 1.25  2000/01/22 20:08:56  endo
// This modification will allow cfgedit FSA
// to choose the default trigger based on
// the state the trigger is originated
// from. For example, the default trigger
// of GoTo state will be AtGoal.
//
// Revision 1.24  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.23  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.23  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.22  1996/05/08  15:16:39  doug
// fixing compile warnings
//
// Revision 1.21  1996/03/13  01:55:23  doug
// *** empty log message ***
//
// Revision 1.20  1996/03/12  17:45:14  doug
// *** empty log message ***
//
// Revision 1.19  1996/03/08  00:46:56  doug
// *** empty log message ***
//
// Revision 1.18  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.17  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.16  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.15  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.14  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.13  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.12  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.9  1996/02/20  22:52:24  doug
// adding EditParms
//
// Revision 1.8  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.7  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.6  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.5  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.3  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.2  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.1  1996/01/30  01:50:19  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
