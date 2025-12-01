/**********************************************************************
 **                                                                  **
 **                          toolbar.cc                              **
 **                                                                  **
 **  Code to support the toolbar buttons                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: toolbar.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

#include <Xm/Xm.h>
#include <Xm/MessageB.h>
#include <sys/time.h>      
#include <sys/timeb.h>    

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
#include "so_movement.h"
#include "ConstructPrintName.h"
#include "EventLogging.h"
#include "callbacks.h"
#include "library_manager.h"
#include "import_symbol.h"
#include "PuDataType.h"
#include "TypeInfo.hpp"
#include "HaveParameters.h"
#include "renumber_robots.h"
#include "mission_expert.h"

#define SUCCESS 1
#define FAILURE 0

using std::string;

extern void internal_place_obj(Position x, Position y);
extern screen_object *find_object(Position x, Position y);
extern void gSavePickedOverlayName(string overlayName);

SYSTEM_MODE system_mode = EDIT_MODE;

Symbol *selected;		// The symbol selected on this page

// Black board operation theory:
//   when a cut or copy operation occurs the symbols are connected to the
//   blackboard (and potentially removed from the drawing) and the
//   fill_type is not LINK_FILL.  
//   When a paste operation occurs, copies of the entire tree for each
//   blackboard of the symbols are inserted into the drawing.  
//   This supports multiple paste operations.
//   If objects were cut, care is taken to delete them if they are not pasted.
//  
//   Link operations also connect the symbols to the blackboard but set
//   fill_type to LINK_FILL.  When a paste operation occurs, the symbols are 
//   linked to the current page.

static Symbol *blackboard;		// The src symbol for the cut/copied obj
static Symbol *blackboard_subtree;	// The subtree for the blackboard symbol
static bool    blackboard_is_state;     // Is the bb a state?

typedef enum FILL_TYPE
{
   LINK_FILL, CUT_FILL, COPY_FILL
};
static FILL_TYPE fill_type;	// how was blackboard filled?

int paste_offset = 100;		// Where to plop new pasted objects

GTList < screen_object * >objs;	// List of screen_objects on cur page

//char *gWaypointOverlayName = NULL;

// Function to write current state of mission to CDL file for usability study
void
save_cdl_for_replay(const char* event)
{
    const rc_chain *files;
    int pid, sec, msec;
    char buf[2048];
    char *dir;
    static int replay_file_num = 0; // Starting reference # for replay filenames

    // Event logging has to be ON.
    if (!gEventLogging->eventLoggingIsON()) return;

    files = rc_table.get("ReplayDir");
    pid = getpid();

    if (files && files->first(&dir))
    {
	gEventLogging->getTime(sec, msec);
	sprintf(buf,"%s/cdl_replay_%d_%3.3d_%d.%3.3d_%s.cdl", dir, pid, replay_file_num, sec, msec, event);
	save_workspace(config->root(), buf, false, false);

	gEventLogging->log("Saving CDL for replay %s", buf);

	replay_file_num++;
    }
}

// Manage the selection list
//---------------------------------------------------------------------------

static void
redraw(screen_object * s)
{
   if (is_selected(s))
      s->redraw_selected();
   else
      s->redraw_unselected();
}

//---------------------------------------------------------------------------
bool
are_selections()
{
   return selected != NULL;
}

//---------------------------------------------------------------------------
//DCM 12/6/96: Added checks that s and selected are not NULL to fix a core dump.
bool
is_selected(screen_object * s)
{
   return (s != NULL) && 
	  (selected != NULL) &&
	  (selected == s->get_src_sym());
}

//---------------------------------------------------------------------------
bool
is_selected(Symbol * rec)
{
   return selected == rec;
}

//---------------------------------------------------------------------------
// Mark the object as selected, if possible.
// returns true if object is selected, false otherwise.
bool
select(screen_object * s)
{
   Symbol *rec = s->get_src_sym();

   // Check if it is read only.
   if((!gDisableImportSymbol) && (rec->record_class != RC_USER))
   {
       // Yes, try to import it.
       import_symbol(s);
       
       // Redraw the page.  We mucked it up pretty good.
       config->regen_page();

       // Return false so the object won't get selected, since the 
       // mouse button is no longer down.
       return false;
   }

   // Remember which object is selected.
   selected = rec;

   // Redraw this one object to show it in the selected color.
   redraw(s);

   return true;
}

//---------------------------------------------------------------------------
void
unselect()
{
   Symbol *rec = selected;

   selected = NULL;

   if (rec && rec->this_screen_object)
      redraw((screen_object *) rec->this_screen_object);
}

//---------------------------------------------------------------------------
void
forget_selections()
{
   selected = NULL;
}

//---------------------------------------------------------------------------
void
objects_handle_events()
{
   // tell each object to handle its own mouse events
   screen_object *s;
   void *cur;

   if ((cur = objs.first(&s)) != NULL)
   {
      do
      {
	 s->handle_events();
      }
      while ((cur = objs.next(&s, cur)) != NULL);
   }
}

//---------------------------------------------------------------------------
void
objects_bypass_events()
{
   // tell each object to pass mouse events through to the state machine
   screen_object *s;
   void *cur;

   if ((cur = objs.first(&s)) != NULL)
   {
      do
      {
	 s->bypass_events();
      }
      while ((cur = objs.next(&s, cur)) != NULL);
   }
}

/*-----------------------------------------------------------------------*/

// Called to delete anything still in the blackboard
void
dump_blackboard()
{
   if (blackboard)
   {
      // Delete the subtree if we did a cut
      if (fill_type == CUT_FILL)
	 blackboard->delete_tree();

      blackboard = NULL;
      blackboard_subtree = NULL;
      blackboard_is_state = false;
   }
   paste_offset = 100;
}

/*-----------------------------------------------------------------------*/

// Called to prompt and maybe delete the selected symbols
static void
cut_selected()
{
    // Adding new stuff deletes the old
    dump_blackboard();
    fill_type = CUT_FILL;

    if (selected)
    {
        screen_object *s = (screen_object *) selected->this_screen_object;

        if (s)
        {
            bool did_delete = false;

            // Make sure it is not read only.
            Symbol *rec = s->get_src_sym();

            if(rec->record_class != RC_USER)
            {
                warn_userf("This object is Read-Only");
                return;
            }

            if (s->verify_can_delete())	// will put up error messages
            {
                {
                    if( s->is_state() )
                    {
                        Symbol *p = s->get_src_sym();
                        if( p && p->list_index && p->list_index->description )
                        {
                            gEventLogging->log("Cut %s", p->list_index->description);
                        }
                        else
                        {
                            gEventLogging->log("Cut State???");
                        }
                    }  
                    else
                    {  
                        Symbol *p = s->get_subtree();
                        if( p && p->description )
                        {
                            gEventLogging->log("Cut %s", p->description);
                        }
                        else
                        {
                            gEventLogging->log("Cut Trans???");
                        }
                    }  

                }

                // Erase the object
                s->erase();

                // Delete from the symbol tree
                s->unhook_leaving_visible_children();

                // No longer on the screen
                if( !objs.remove(s) )
                    warn_user("Internal Error: Unable to remove cut objects from screen");

                // Move the object to the blackboard
                blackboard = s->get_src_sym();
                blackboard_subtree = s->get_subtree();
                blackboard_is_state = s->is_state();

                // Mark drawing as changed
                config->made_change();

                if (gMExp->isEnabled())
                {
                    gMExp->madeChange();
                }

                did_delete = true;
            }

            // Remove it from the list of selected objects
            selected = NULL;

            if (!did_delete)
                s->redraw_unselected();
        }
        else
        {
            warn_userf("Internal Error: Unable to access selected object%s",
                       selected->description);
            selected = NULL;
        }
    }
    else
    {
        warn_user("Nothing selected to cut!");
    }
}

/*-----------------------------------------------------------------------*/
// Called to delete the selected symbols
static void delete_selected(void)
{
    screen_object *s = NULL;
    Symbol *rec = NULL;
    bool did_delete = false;

    if (!selected)
    {
        warn_user("Nothing selected to delete!");
        return;
    }

    s = (screen_object *) selected->this_screen_object;

    if (s == NULL)
    {
        warn_userf("Internal Error: Unable to access selected object%s",
                   selected->description);
        selected = NULL;
    }

    // Make sure it is not read only.
    rec = s->get_src_sym();

    if(rec->record_class != RC_USER)
    {
        warn_userf("This object is Read-Only");
        return;
    }

    if (s->verify_can_delete())	// will put up error messages
    {
        {
            if( s->is_state() )
            {
                Symbol *p = s->get_src_sym();
                if( p && p->list_index && p->list_index->description )
                {
                    gEventLogging->log("Delete %s", p->list_index->description);
                }
                else
                {
                    gEventLogging->log("Delete State???");
                }
            }  
            else
            {  
                Symbol *p = s->get_subtree();
                if( p && p->description )
                {
                    gEventLogging->log("Delete %s", p->description);
                }
                else
                {
                    gEventLogging->log("Delete Trans???");
                }
            }  

        }

        // Erase the object
        s->erase();

        // Delete from the symbol tree
        s->unhook_leaving_visible_children();

        // No longer on the screen
        if(!objs.remove(s))
        {
            warn_user("Internal Error: Unable to remove objects from screen");
        }

        // Mark drawing as changed
        config->made_change();

        if (gMExp->isEnabled())
        {
            gMExp->madeChange();
        }

        did_delete = true;
    }

    // Remove it from the list of selected objects
    selected = NULL;

    if (!did_delete) s->redraw_unselected();
}

/*-----------------------------------------------------------------------*/

// make copies of the selected objects in the blackboard
static void
copy_selected()
{
   // Adding new stuff deletes the old
   dump_blackboard();
   fill_type = COPY_FILL;

   bool did_one = false;

   if (selected)
   {
      screen_object *s = (screen_object *) selected->this_screen_object;

      if (s)
      {
	 if (s->verify_can_copy())	// will put up error messages
	 {
	     Symbol *p = s->get_subtree();
	     
	     if( p && p->name ) gEventLogging->log("Copy %s", p->name);
	     else gEventLogging->log("Copy ????");

	     // Link the object to the blackboard
	     blackboard = s->get_src_sym();
	     blackboard_subtree = s->get_subtree();
	     blackboard_is_state = s->is_state();
	 }

	 // Erase the selected version
	 s->erase();

	 // Remove it from the list of selected objects.
	 selected = NULL;

	 // redraw 
	 s->redraw_unselected();

	 did_one = true;
      }
      else
      {
	 warn_userf("Internal Error: Unable to get object pointer for %s",
		    selected->description);
      }
   }
   else
   {
      warn_user("Nothing selected to copy!");
   }

   // Regen to pick up the changes
   if (did_one)
      config->regen_page();
}

/*-----------------------------------------------------------------------*/

// Add links to the selected objects to the blackboard
static void
link_selected()
{
   // Adding new stuff deletes the old
   dump_blackboard();
   fill_type = LINK_FILL;

   if (selected)
   {
      screen_object *s = (screen_object *) selected->this_screen_object;

      if (s)
      {
	 // Make sure it is not read only.
	 Symbol *rec = s->get_src_sym();

	 if(rec->record_class != RC_USER)
	 {
	     warn_userf("This object is Read-Only");
	     return;
	 }

	 if (s->verify_can_link())	// will put up error messages

	 {
	    {
	       Symbol *p = s->get_subtree();
	       if( p && p->name ) gEventLogging->log("Link %s", p->name);
	       else gEventLogging->log("Link ????");
	    }

	    // Link the object to the blackboard
	    blackboard = s->get_src_sym();
	    blackboard_subtree = s->get_subtree();
            blackboard_is_state = s->is_state();
	 }

	 // Erase the selected version
	 s->erase();

	 // Remove it from the list of selected objects.
	 selected = NULL;

	 // redraw 
	 s->redraw_unselected();
      }
      else
      {
	 warn_userf("Internal Error: Unable to get object pointer for %s",
		    selected->description);
      }
   }
   else
   {
      warn_user("Nothing selected to link!");
   }
}

/*-----------------------------------------------------------------------*/

// Paste the blackboard back in
bool
can_paste(bool show_warnings)
{
   if (blackboard == NULL)
   {
      if (show_warnings)
	 warn_user("Internal Error: Empty blackboard");
      return false;
   }

   // Make sure that states & transitions only go in FSA's and glyphs don't
   bool goes_in_fsa = blackboard->symbol_type == INPUT_NAME;

   // Make sure that states and transitions are only put in fsa's
   if (goes_in_fsa && !config->this_page()->is_fsa())
   {
      if (show_warnings)
	 warn_user("States and Transitions can only be pasted into FSA operators");
      return false;
   }

   // Make sure that glyphs are not put in fsa's
   if (!goes_in_fsa && config->this_page()->is_fsa())
   {
      if (show_warnings)
	 warn_user("Only States and Transitions can be pasted into FSA operators");
      return false;
   }

   // Make sure that aren't pasting a linked glyph into one of its home pages.
   if (fill_type == LINK_FILL && show_warnings)
   {
      if (blackboard)
      {
	 // Now, for each symbol in the blackboard, make sure it doesn't
	 // already exist on this page
	 screen_object *obj;
	 void *pos;

	 if ((pos = objs.first(&obj)) != NULL)
	 {
	    do
	    {
	       if (blackboard == obj->get_src_sym())
	       {
		  warn_user("Linked object already exists on this page\nUse multiple output connections instead");
		  return false;
	       }
	    }
	    while ((pos = objs.next(&obj, pos)) != NULL);
	 }
      }
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// Paste the blackboard back in
static void
paste()
{
    if (blackboard == NULL)
    {
        warn_user("Nothing in buffer to paste!");
        return;
    }

    bool replace_mode = false;

    if (can_paste(true))
    {
        // If there is a selection, then check if it is the same type
        // as what is in the buffer
        if (selected)
        {
            screen_object *s = (screen_object *)selected->this_screen_object;

            bool sel_is_state = s && s->is_state();

            if (selected->symbol_type != blackboard->symbol_type ||
                sel_is_state != blackboard_is_state )
            {
                if (blackboard->symbol_type == INPUT_NAME)
                {
                    if ( blackboard_is_state )
                    {
                        warn_user("Can only paste states into other states or onto the background");
                        unselect();
                        return;
                    }
                    else
                    {
                        warn_user("Can only paste triggers into other triggers");
                        unselect();
                        return;
                    }
                }
                else
                {
                    warn_user("Can only paste glyphs into other glyphs or onto the background");
                    unselect();
                    return;
                }
            }

            if (blackboard->symbol_type != INPUT_NAME)
            {
                warn_user("Can only paste glyphs onto the background");
                unselect();
                return;
            }

            replace_mode = true;
        }
        else if (blackboard->symbol_type == INPUT_NAME && !blackboard_is_state )
        {
            warn_user("Can only paste triggers into other triggers");
            return;
        }

        Symbol *rec;

        // Get the correct symbol to work with.
        if (replace_mode)
        {
            rec = blackboard_subtree;
        }
        else
        {
            rec = blackboard;
        }

        // Do we really want to add a COPY of the object?
        if (fill_type != LINK_FILL)
        {
            // If is an instance of a robot binding point, dup the binding
            // point and will get a new bound agent too.
            if (rec->bound_to && rec->symbol_type == ROBOT_NAME)
            {
                // Really should duplicate the defining binding point
                rec = rec->bound_to;
            }

            // duplicate the tree for the symbol
            rec = rec->dup_tree(false);
        }

        if (replace_mode)
        {
            screen_object *s = (screen_object *) selected->this_screen_object;

            if (s == NULL)
            {
                warn_user("Internal Error: Selected object doesn't exist??");
                unselect();
                return;
            }
            s->set_subtree(rec);

            {
                Symbol *p = s->get_subtree();
                if( p && p->name ) gEventLogging->log("Replace %s", p->name);
                else gEventLogging->log("Replace ????");
            }

        }
        else
        {
            // Move it back to the upper left corner
            if (blackboard_is_state && rec->list_index)
            {
                rec->list_index->location.x = paste_offset;
                rec->list_index->location.y = paste_offset;
            }
            else
            {
                rec->location.x = paste_offset;
                rec->location.y = paste_offset;
            }
            paste_offset += 20;

            if (rec->symbol_type == RULE_NAME)
            {
                warn_user("Can only paste triggers into other triggers");
                unselect();
                return;
            }

            // If is a state, need to create it specially
            if ( blackboard_is_state )
            {
                config->this_page()->this_fsa()->add_state(rec);
            }
            else
            {
                // Put it unconnected into the current page.
                connect_object_to_page(rec);
            }

            {
                if( rec && rec->name ) gEventLogging->log("Paste %s", rec->name);
                else gEventLogging->log("Paste ????");
            }
        }

        // Mark drawing as changed
        config->made_change();

        if (gMExp->isEnabled())
        {
            gMExp->madeChange();
        }

        // Regen to pick up the changes
        config->regen_page();
    }
}


void modifyScreenObjectFieldByName(screen_object *s, char *mod_field, char *newval)
{
  Symbol *rec = s->get_subtree();
  /* Find mod_field by name */
  Symbol *p;
  void *cur;
  if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
      do 
		{
		  if( p->name[0] == '%' && strcasecmp(p->name, mod_field)==0)
			{
			  p->input_generator->name = strdup(newval);
			  break;
			}
		  
		} while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }
}

int modifyScreenObjectStringParameterByName(screen_object *s, char *name, char *new_val)
{
  Symbol *rec = s->get_subtree();
  Symbol *p;
  char *str;
  void *cur;

  if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
      do 
	{
	  if( p->name[0] == '%' && strcasecmp(p->name, name)==0)
	    {
	      str = strdup(new_val);

	      char *qstr;
	      qstr = (char *)malloc(strlen(str) + 3);
	      sprintf(qstr, "\"%s\"", str); // Adding quotes around it.
	      free(str);
	      p->input_generator->name = qstr; 
	      
	      return SUCCESS;
	    }
	  
	} while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }
  return FAILURE;
}

/* Function preserved for backward compatibilty */
void modifyGoalLocation(screen_object *s, double x, double y)
{
  char goal_loc[512];

  sprintf(goal_loc, "%.2f, %.2f", x, y);
  modifyScreenObjectFieldByName(s, "%Goal_Location", goal_loc);
}

/*-----------------------------------------------------------------------*/
// This is a function which will create lists of parameter names and its
// values of state "*s", and also count how many lists were created. It
// will return SUCCESS or FAILURE depending on the status.
/*-----------------------------------------------------------------------*/
int listStateParmNamesAndValues(screen_object *s, char ***names, char ***values, int *num_found)
{
  Symbol *rec = s->get_subtree();
  Symbol *parm;
  void *cur;

  int nfnd = 0, nalc = 0;
  char **rtn_names = NULL;
  char **rtn_values = NULL;

  // Go through the list to find parameters which start
  // with "%".
  if ((cur = rec->parameter_list.first(&parm)) != NULL)
  {
    do {
      if( parm->name[0] == '%')
      {
	char *name = parm->name;
	char *value = parm->input_generator->name;
	nfnd++;

	// Now the name and value, put them in the list.
	if (nfnd > nalc)
	{
	  int n;
	  char **cn, **cv;
	  n = (nalc == 0)? 1: nalc * 2;

	  if (!(cn = (char **) realloc(rtn_names, sizeof(char*) * n)))
	    return FAILURE;
	  if (!(cv = (char **) realloc(rtn_values, sizeof(char*) * n)))
	    return FAILURE;

	  rtn_names = cn;
	  rtn_values = cv;
	  nalc = n;
	}
	
	rtn_names[nfnd - 1] = name;
	rtn_values[nfnd - 1] = value;
      }
    } while ((cur = rec->parameter_list.next(&parm, cur)) != NULL);
  }
  *names = rtn_names;
  *values = rtn_values;
  *num_found = nfnd;
  return SUCCESS;
}

/*-----------------------------------------------------------------------*/
// This funtion changes the values of the parameter "*name" in the state
// "*t" to be the input "*value".
/*-----------------------------------------------------------------------*/
void updateTriggerFieldManually(screen_object *t, char *name, char *value)
{
  Symbol *rec = t->get_subtree();
  Symbol *parm;
  void *cur;

  if ((cur = rec->parameter_list.first(&parm)) != NULL)
  {
    do {
      if( parm->name[0] == '%' && strcasecmp(parm->name, name)==0)
	parm->input_generator->name = strdup(value);
    } while ((cur = rec->parameter_list.next(&parm, cur)) != NULL);
  }
}

/*-----------------------------------------------------------------------*/
// This function changes the parameter values of a trigger "*t" if it has
// common parameter with this state "*s".
/*-----------------------------------------------------------------------*/
void updateTriggerFieldAutomatically(screen_object *t, screen_object *s)
{
  if(t)
  {
    Symbol *sub = t->get_subtree();
    
    // Make sure the condition meets.
    if( t->has_subtree() > 0 &&           // The trigger has a subtree.
	config->this_page()->is_fsa() &&  // This is FSA.
	HaveParameters(sub) &&            // The trigger has a parameter. 
	(user_privileges & MODIFY_PRIV) ) // The user can modify.
    {
      int count;
      char **names, **values;

      // List the parameter names and values in this state.
      if (listStateParmNamesAndValues(s, &names, &values, &count) == FAILURE)
	return;

      // Change the parameter values of this trigger.
      if ((names != NULL)&&(values != NULL))
      {
	for (int i = 0; i < count; i++)
	  updateTriggerFieldManually(t, names[i], values[i]);
      }
    }
  }
}

void addAtGoalTransition(screen_object *p, screen_object *q, double x, double y)
{
  screen_object *t = config->this_page()->this_fsa()->add_transition(p, q);
  Symbol *rep = config->find_agent_by_name("AtGoal");
  t->set_subtree(rep);
  
  modifyGoalLocation(t, x, y);
}

screen_object *addTransitionByName(screen_object *p, screen_object *q, char *sname)
{
  screen_object *t = config->this_page()->this_fsa()->add_transition(p, q);
  Symbol *rep = config->find_agent_by_name(sname);
  t->set_subtree(rep);
  return t;
}


screen_object *placeStateByName(char *sname)
{
  screen_object *s = config->this_page()->this_fsa()->add_state();
  if (s) place_object(s);
  else {
    set_mode(NULL, EDIT_MODE);
    return NULL;
  }

  int HSPACE = 450;
  int HSTART = 120;
  int HEND   = 1020;
  int VSPACE = 300;
  int VSTART = 300;

  int num_obs = config->this_page()->this_fsa()->getNumberOfStatesInFSA() - 2;

  /* Even rows run left to right */
  if (((num_obs/ 3) % 2) == 0)
    {
      internal_place_obj((Position)((HSPACE * (num_obs % 3)) + HSTART), 
			 (Position)((VSPACE * (num_obs / 3)) + VSTART));
    }
  /* Odd rows place right to left */
  else
    {
      internal_place_obj((Position)(HEND - (HSPACE * (num_obs % 3))),
			 (Position)((VSPACE * (num_obs/ 3)) + VSTART));
    }
  Symbol *rep = config->find_agent_by_name(sname);
  s->set_subtree(rep);
  
  return s;
}

screen_object *addGotoState(double x, double y)
{
  
  screen_object *s;
  if (!(s = placeStateByName("GoTo"))) return NULL;
  
  modifyGoalLocation(s, x, y);  
  save_cdl_for_replay("placed_waypoint");

  return s;
}

char *rmspaces(char *s)
{
  char *p = s;
  while (*p != '\0') {
    if (*p == ' ') *p = '_';
    if (*p == '\n') *p = '\0';
    p++;
  }
  return s;
}


/* Planner goto design */
void modifyGotoLocationParams(screen_object *s, double goal_loc_x, double goal_loc_y,
							  double move_to_location_gain)
{
  char buf[1024];
  sprintf(buf, "%.2f,%.2f", goal_loc_x, goal_loc_y);
  modifyScreenObjectFieldByName(s, "%Goal_Location", buf);
  
  sprintf(buf, "%.2f", move_to_location_gain);
  modifyScreenObjectFieldByName(s, "%move_to_location_gain", buf);
}
void modifyGotoObstacleParams(screen_object *s, double gain, 
							  double sphere, double safety) 
{
  char buf[1024];

  sprintf(buf, "%.2f", gain);
  modifyScreenObjectFieldByName(s, "%avoid_obstacle_gain", buf);

  sprintf(buf, "%.2f", sphere);
  modifyScreenObjectFieldByName(s, "%avoid_obstacle_sphere", buf);

  sprintf(buf, "%.2f", safety);
  modifyScreenObjectFieldByName(s, "%avoid_obstacle_safety_margin", buf);
}


screen_object *addGotoStateComplex(double goal_x, double goal_y, double goal_gain,
								   double obs_gain, double obs_sphere, double obs_safety)
{
  
  screen_object *s;
  if (!(s = placeStateByName("GoTo"))) return NULL;
  
  modifyGotoLocationParams(s, goal_x, goal_y, goal_gain);
  modifyGotoObstacleParams(s, obs_gain, obs_sphere, obs_safety);

  return s;
}
void addAtGoalTransitionComplex(screen_object *p, screen_object *q, double x, 
								double y, double tolerance)
{
  char buf[1024];
  screen_object *t = config->this_page()->this_fsa()->add_transition(p, q);
  Symbol *rep = config->find_agent_by_name("AtGoal");
  t->set_subtree(rep);
  
  modifyGoalLocation(t, x, y);
  sprintf(buf, "%.2f", tolerance);
  modifyScreenObjectFieldByName(t, "%Goal_Tolerance", buf);
}

void planner_goto_design(void) 
{
    FILE* fp;
    char buf[1024];
    char fname[512];
    double x, y, xx, yy;
    double lgain, ogain, osphere, osafety, gtolerance;
    double last_tolerance;
    screen_object *p = NULL, *q = NULL;
    screen_object *s;
    screen_object *xy=NULL;
    //time_t tm = time(NULL);
   
    sprintf(fname, "points.txt"); 
    sprintf(buf, "path_planner marc3f.map %s", fname);
    system(buf);

    /* Add a localization node Localize so that the start 
       position is no longer taken from the overlay */
    if( (xy = placeStateByName("Localize")) == NULL)
        fprintf(stderr,"ERROR: Cannot place state Localize!\n");

    if ((fp = fopen(fname, "r"))) {
        while (fgets(buf, 1023, fp)) {
            sscanf(buf, "%lf %lf %lf %lf %lf %lf %lf", &x, &y, &lgain, &ogain, 
                   &osphere, &osafety, &gtolerance);
            if (!(q = addGotoStateComplex(x, y, lgain, ogain, osphere, osafety))) break;
            if (p != NULL) 
                addAtGoalTransitionComplex(p, q, xx, yy, last_tolerance);
            else 
            {
		  
                /* Fill the fields of Localize; now we know the start point */
                sprintf(buf, "%.2f,%.2f", x, y);
                modifyScreenObjectFieldByName(xy, "%New_Location", buf);
		  
                sprintf(buf, "%.2f", 0.0);
                modifyScreenObjectFieldByName(xy, "%New_Heading", buf);
		  
                if ((s = find_object(50, 50)))  /* Handle the first state */
                {
			  
                    addTransitionByName(s, xy, "Immediate");
                    addTransitionByName(xy, q, "Immediate");
                }
            }
            p = q;
            xx = x;
            yy = y;
            last_tolerance = gtolerance;
        }
        fclose(fp);
        if (((s = placeStateByName("Stop")) != NULL) && q != NULL) {
            addAtGoalTransitionComplex(q, s, xx, yy, last_tolerance);
        }
    }
    /* Regenerate the drawing to get the changes */
    config->regen_page();
  
    /* Mark drawing as changed */
    config->made_change();

    if (gMExp->isEnabled())
    {
        gMExp->madeChange();
    }
  
    set_mode(NULL, EDIT_MODE);
  
    //unlink(fname); 
}

void waypoint_design(void)
{ 
    FILE* fp;
    string ovlNameStr;
    char fname[512];
    char buf[1024], ovlname[1024];
    double x, y, xx, yy;
    screen_object *p = NULL, *q = NULL;
    time_t tm = time(NULL);
    int status;
  
    /*
    if (gWaypointOverlayName)
    {
        free(gWaypointOverlayName);
        gWaypointOverlayName = NULL;
    }
    */

    sprintf(fname, "waypoints_%s.txt", rmspaces(ctime(&tm))); //file name
  
    if(gEventLogging->eventLoggingIsON())
    {
        sprintf(buf, "mlab -w %s -e %s -n", fname, gEventLogging->getLogfileName());
    }
    else 
    {
        sprintf(buf, "mlab -w %s -n", fname);
    }

    gEventLogging->start("system call %s", buf);
    gEventLogging->pause();

    // Execute mlab using system call
    system(buf);

    gEventLogging->resume(true);
    gEventLogging->end("system call mlab");

    fp = fopen(fname, "r");

    if (fp)
    {
        while (fscanf(fp, "%s", buf) != EOF)
        {
            if (!strcmp(buf, "waypoint"))
            {
                fscanf(fp,"%lf", &x);
                fscanf(fp,"%lf", &y);
	    
                if (!(q = addGotoState(x, y))) break;
                if (p != NULL) addAtGoalTransition(p, q, xx, yy);

                p = q;
                xx = x;
                yy = y;
            }
            else if (!strcmp(buf, "overlay"))
            {
                status = fscanf(fp,"%s", ovlname);

                if (status != EOF)
                {
                    //gWaypointOverlayName = strdup(ovlname);
                    ovlNameStr = ovlname;
                    gSavePickedOverlayName(ovlNameStr);
                }
	    
            }
        }

        fclose(fp);
        unlink(fname); 
    }
    else
    {
        fprintf(stderr, "Error(cfgedit): waypoint_design(). Waypoint file cannot be opended.\n");
    }
  
    /* Regenerate the drawing to get the changes */
    config->regen_page();
  
    /* Mark drawing as changed */
    config->made_change();

    if (gMExp->isEnabled())
    {
        gMExp->madeChange();
    }

    set_mode(NULL, EDIT_MODE);
}
/*-----------------------------------------------------------------------*/
// This function returns an array of char which contains value for
// the parm_name in the state_name of sreeen_obj sub-FSA. Note that,
// the returned charactors contains '"' in the beginning and at the end.
// num_found return how many times it fond the state_name.
/*-----------------------------------------------------------------------*/
char **listThisStateInSubFSA(screen_object *screen_obj, char *state_name, char *parm_name, int *num_found)
{
  int nfnd = 0, nalc = 0;
  char **retval = NULL;

  // Looking for <state_name> in the sub-FSA.
  if (screen_obj->has_subtree()) {
    Symbol *kid = screen_obj->get_subtree();
    Symbol *node1;
    void *cur1;
    if ((cur1 = kid->children.first(&node1)) != NULL){
    do {
	
    Symbol *node2;
    void *cur2;
    if ((cur2 = node1->children.first(&node2)) != NULL) {
    do {
    
    if (node2->is_fsa()) { // We are now in sub-FSA
    Symbol *node3;
    void *cur3;
    if ((cur3 = node2->parameter_list.first(&node3)) != NULL) {
    do {

    Symbol *node4;
    void *cur4;
    if ((cur4 = node3->parameter_list.first(&node4)) != NULL) {
    do {

    Symbol *index = node4->input_generator;
    if (index) {

    Symbol *node5;
    void *cur5;
    if ((cur5 = index->children.first(&node5)) != NULL) {
    do {

    // This should be a state. Go further down if it
    // is the one we are looking for (i.e., state_name).
    if (strcmp(node5->name, state_name) == 0) {

    Symbol *data_type = PuDataType(node5, parm_name);
    Symbol *node6;
    void *cur6;
    if ((cur6 = index->parameter_list.first(&node6)) != NULL) {
    do {

    if (!strcmp(node6->name, parm_name)) {
    char *str = data_type->TypeRecord->PrintValue(node6,",");
    nfnd++;

    if (nfnd > nalc) {
    int n;
    char **t;
    n = (nalc == 0)? 1: nalc * 2;

    if (!(t = (char **) realloc(retval, sizeof(char*) * n))) return NULL;
    retval = t;
    nalc = n;
    } // Corresponds to "if (nfnd > nalc)"
    retval[nfnd - 1] = str; // Add this string to the array,

    } // Corresponds to "if (!strcmp(node6->name, parm_name))"
    } while ((cur6 = index->parameter_list.next(&node6, cur6)) != NULL);
    } // Corresponds to "if ((cur6 = index->parameter_list.first(&node6)) != NULL)"
    } // Corresponds to "if (strcmp(node5->name, state_name) == 0)"
    } while ((cur5 = index->children.next(&node5, cur5)) != NULL);
    } // Corresponds to "if ((cur5 = index->children.first(&node5)) != NULL)"
    } // Corresponds to "if (node4->input_generator)"
    } while ((cur4 = node3->parameter_list.next(&node4, cur4)) != NULL);
    } // Corresponds to "if ((cur4 = node3->... ))"
    } while ((cur3 = node2->parameter_list.next(&node3, cur3)) != NULL);
    } // Corresponds to "if ((cur3 = node2->... ))"
    } // Corresponds to "if ((cur2 = node1->... ))"
    } while ((cur2 = node1->children.next(&node2, cur2)) != NULL);
    } // Corresponds to  "if ((cur = kid->... )"
    } while ((cur1 = kid->children.next(&node1, cur1)) != NULL);
    } // Corresponds to "if ((cur = kid->children.first(&node1)) != NULL)"
  } // Corresponds to "(screen_obj->has_subtree())"
  *num_found = nfnd;
  return retval;
}

/*-----------------------------------------------------------------------*/
// This function performs "cut" on the main state, and create a new same state
// so that we can get rid of the triggers coming out it easily.
/*-----------------------------------------------------------------------*/
screen_object *cutTaskExitedTransition(screen_object *task_star)
{
  Position x_pos, y_pos;
  task_star->get_xy(&x_pos, &y_pos);
  char *task_star_name = ObjectName(task_star->get_subtree());

  screen_object *new_task_star = config->this_page()->this_fsa()->add_state();
  if (new_task_star)
    {
      place_object(new_task_star);
    }
  else
    {
      set_mode(NULL, EDIT_MODE);
      return task_star;
    }

  selected = task_star->get_src_sym();
  cut_selected();

  internal_place_obj(x_pos, y_pos);
  Symbol *rep = config->find_agent_by_name(task_star_name);
  new_task_star->set_subtree(rep);

  return new_task_star;

}
/*-----------------------------------------------------------------------*/
char *queryScreenObjectParameter(screen_object *s, char *name)
{
  Symbol *rec = s->get_subtree();
  Symbol *p;
  void *cur;
  if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
      do 
	{
	  if( p->name[0] == '%' && strcasecmp(p->name, name)==0)
	    {
	      return p->input_generator->name;
	    }
	  
	} while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }
  return NULL;
}

/*-----------------------------------------------------------------------*/
// This funtion creates "TaskExited" triggers coming out from the
// <task_star> state by going down to the sub-FSA, and looks for
// "ExitTask" state and its notify_message string. The notify_message
// string will be used as the value the "TaskExited" parameter.
void TaskExited_design(screen_object *task_star, Symbol *cur)
{

  // Position of the Task*
  Position x_pos, y_pos;
  task_star->get_xy(&x_pos, &y_pos);
  
  int HSPACE = 300;
  int VSPACE = 300;
  int dx = 0;

  // This prevents "Stop" state from overlapping at the same position
  // as the prevous one since the "Stop" won't be deleted by
  // cutTaskExitedTransition().
  int offset = 5*(rand() % 20); 

  char *state_name = "ExitTask";
  char *parm_name = "%notify_message";
  char *trgger_name = "TaskExited";
  int count; // It counts how many "taskExited" triggers have to be created.
  char ** names = listThisStateInSubFSA(task_star, state_name, parm_name, &count);

  if (names != NULL)
    {
      for (int i = 0; i < count; i++)
	{
	  screen_object *stp = config->this_page()->this_fsa()->add_state();
	  if (stp)
	    place_object(stp);
	  else
	    {
	      set_mode(NULL, EDIT_MODE);
	      return;
	    }

	  // Managing the x position to place the "Stop" state.
	  dx = abs((i)*(HSPACE/(count - i))) + offset;

	  internal_place_obj(x_pos + dx, y_pos + VSPACE);
	  Symbol *rep = config->find_agent_by_name("Stop");
	  stp->set_subtree(rep);

	  // Let's construct the "TaskExited" trigger to the "Stop" state.
	  screen_object *t = addTransitionByName(task_star, stp, trgger_name);
	  modifyScreenObjectStringParameterByName(t, parm_name, names[i]);
      	}
    }
}


/*-----------------------------------------------------------------------*/

void
cancel_cb()
{
   if( system_mode == START_CONNECT_TRANSITION )
   {
       gEventLogging->cancel("AddTransition");
   }
   else if( system_mode == MAKING_CONNECTION )
   {
       gEventLogging->cancel("AddConnection");
   }

   set_mode(NULL, EDIT_MODE);
}

/*-----------------------------------------------------------------------*/

// called when one of the toolbar buttons is clicked to change the system mode
void
set_mode(Widget w, SYSTEM_MODE new_mode)
{
    screen_object *s;
    Symbol *p;
    bool confirmed;
    extern int num_transitions;
    SYSTEM_MODE old_mode = system_mode;

    system_mode = new_mode;

    switch (system_mode) {

    default:
        warn_userf("Error: Unknown system mode %d in set_mode\n", system_mode);

        // reset to edit_mode
        system_mode = EDIT_MODE;
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), arrow_cursor);
        break;

    case EDIT_MODE:
        gEventLogging->log("set_mode EDIT_MODE");
        if(old_mode == CONNECTING_TRANSITION || old_mode == MAKING_CONNECTION)
        {
            end_rubber_band();
        }
        else if( old_mode == PLACEMENT )
        {
            abort_place_obj();
        }

        // reset the cursor to the arrow picture
        XDefineCursor(XtDisplay(top_level),XtWindow(top_level), arrow_cursor);
        objects_handle_events();
        remove_grabs();

        break;

    case CUT:
        gEventLogging->log("set_mode CUT");
        cut_selected();
        renumber_robots();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("cut_object");   
        break;

    case COPY:
        gEventLogging->log("set_mode COPY");
        copy_selected();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("copied_object");   
        break;

    case DELETE:
        gEventLogging->log("set_mode DELETE");
        delete_selected();
        renumber_robots();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("delete_object");   
        break;

    case DUPLICATE:
        gEventLogging->log("set_mode DUPLICATE");

        // Make sure this level is not read-only.
        if (config->is_read_only_page())
        {
            warn_userf("This level is Read-Only");
            set_mode(NULL, EDIT_MODE);
            break;
        }
        copy_selected();
        paste();
        renumber_robots();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("duplicated_object");    
        break;

    case LINK:
        gEventLogging->log("set_mode LINK");
        link_selected();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("linked_object");  
        break;

    case MOVE_MODE:
        gEventLogging->log("set_mode MOVE_MODE");
        break;

    case PASTE:
        gEventLogging->log("set_mode PASTE");

        // Make sure this level is not read-only.
        if (config->is_read_only_page())
        {
            warn_userf("This level is Read-Only");
            set_mode(NULL, EDIT_MODE);
            break;
        }
        paste();
        renumber_robots();
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("pasted_object");  
        break;

    case START_CONNECT_TRANSITION:
        gEventLogging->log("set_mode START_CONNECT_TRANSITION");
        gEventLogging->start("AddTransition Trans%d", num_transitions + 1);

        // set the cursor to the transition picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level),trans_cursor);
        objects_bypass_events();
        break;

    case PLACE_STATE:
        gEventLogging->log("set_mode PLACE_STATE");
        s = config->this_page()->this_fsa()->add_state();
        if (s)
        {
            place_object(s);

            p = s->get_src_sym();

            if( p && p->list_index && p->list_index->description )
            {
                gEventLogging->start("PlaceState \"%s\"", p->list_index->description);
            }
            else
            {
                gEventLogging->start("PlaceState \"Unknown\"");
            }
        }
        else
        {
            set_mode(NULL, EDIT_MODE);
        }
        break;

    case WAYPOINT_DESIGN:
        gEventLogging->log("set_mode WAYPOINT_DESIGN");
        gEventLogging->start("Placing Waypoints");
        waypoint_design();
        gEventLogging->end("Placing Waypoints");
        break;

    case PLANNER_DESIGN:
        gEventLogging->log("set_mode PLANNER_DESIGN");
        planner_goto_design();

        set_mode(NULL, EDIT_MODE);
        break;

    case MOVE_UP:
        gEventLogging->log("set_mode MOVE_UP");
        move_up_cb();
        if (old_mode == MOVE_UP)
            set_mode(NULL, EDIT_MODE);
        else
            set_mode(NULL, old_mode);
        break;

    case NEW_PRIMITIVE:
        gEventLogging->log("set_mode NEW_PRIMITIVE");
        gEventLogging->start("NewPrimitive");

        s = select_behavior(PG_PRIMS, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewPrimitive");
        save_cdl_for_replay("new_primitive");    
        break;

    case NEW_AGENT:
        gEventLogging->log("set_mode NEW_AGENT");
        gEventLogging->start("NewAgent");

        s = select_behavior(PG_AGENTS, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewAgent");
        save_cdl_for_replay("new_agent");   
        break;

    case NEW_TRIGGER:
        gEventLogging->log("set_mode NEW_TRIGGER");
        gEventLogging->start("NewTrigger");

        s = select_behavior(PG_TRIGS, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewTrigger");
        save_cdl_for_replay("new_trigger"); 
        break;

    case IMPORT_LIB_FNC:
        gEventLogging->log("set_mode IMPORT_LIB_FNC");
        gEventLogging->start("ImportLibFnc");

        s = select_behavior(PG_AGENTS, true);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("ImportLibFnc");
        save_cdl_for_replay("imported_library_function");
        break;

    case ADD_TO_LIB_FNC:
        gEventLogging->log("set_mode ADD_TO_LIB_FNC");
        if(!selected) warn_user("Nothing selected to add to library!");
        else make_library_component_from_selected();

        set_mode(NULL, EDIT_MODE);
        break;

    case DELETE_FROM_LIB_FNC:
        gEventLogging->log("set_mode DELETE_FROM_LIB_FNC");
        remove_library_component();

        set_mode(NULL, EDIT_MODE);
        break;

    case GROUP_COMPONENT:
        gEventLogging->log("set_mode GROUP_COMPONENT");
        gEventLogging->start("GroupComponent");

        // Get changes and then keep update from overwriting our changes
        config->this_page()->update();
        config->this_page()->skip_update();
       
        config->this_page()->group(selected);

        // Regenerate the drawing to get the changes
        config->regen_page();

        // Mark drawing as changed
        config->made_change();

        gEventLogging->end("GroupComponent");
        set_mode(NULL, EDIT_MODE);
        save_cdl_for_replay("grouped_objects");  
        break;

    case NEW_OPERATOR:
        gEventLogging->log("set_mode NEW_OPERATOR");
        gEventLogging->start("NewOp");

        s = select_behavior(PG_OPS, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewOp");
        save_cdl_for_replay("new_operator");  
        break;

    case NEW_RBP:
        gEventLogging->log("set_mode NEW_RBP");
        gEventLogging->start("NewRBP");

        s = select_behavior(PG_RBP, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewRBP");
        save_cdl_for_replay("new_rbp"); 
        break;

    case NEW_IBP:
        gEventLogging->log("set_mode NEW_IBP");
        gEventLogging->start("NewIBP");

        s = select_behavior(PG_IBP, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewIBP");
        save_cdl_for_replay("new_ibp");  
        break;

    case NEW_OBP:
        gEventLogging->log("set_mode NEW_OBP");
        gEventLogging->start("NewOBP");

        s = select_behavior(PG_OBP, false);

        if (s) place_object(s);
        else set_mode(NULL, EDIT_MODE);

        gEventLogging->end("NewOBP");
        save_cdl_for_replay("new_obp"); 
        break;

    case MAKING_CONNECTION:
        gEventLogging->log("set_mode MAKING_CONNECTION");
        gEventLogging->start("AddConnection");

        // Make sure this level is not read-only.
        if (config->is_read_only_page())
        {
            warn_userf("This level is Read-Only");
            gEventLogging->cancel("AddConnection");
            set_mode(NULL, EDIT_MODE);
            break;
        }
        // set the cursor to the connection picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), connect_cursor);
        objects_bypass_events();
        break;

    case CONNECTING_TRANSITION:
        gEventLogging->log("set_mode CONNECTING_TRANSITION");
        // set the cursor to the transition connection picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), trans_cursor);
        objects_bypass_events();

        gEventLogging->status("FirstState");
        break;

    case PLACEMENT:
        gEventLogging->log("set_mode PLACEMENT");
        // set the cursor to the hand picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), hand_cursor);
        objects_bypass_events();
        // set the cursor to the pick constant picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), pick_const_cursor);
        // set the cursor to the conversion picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), cvt_input_cursor);
        objects_handle_events();
        break;

    case DRAGGING:
        gEventLogging->log("set_mode DRAGGING");
        // set the cursor to the hand picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), hand_cursor);
        objects_bypass_events();
        // set the cursor to the pick constant picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), pick_const_cursor);
        // set the cursor to the conversion picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), cvt_input_cursor);
        objects_handle_events();
        break;

    case PUSHUP:
        gEventLogging->log("set_mode PUSHUP");
        // set the cursor to the pick constant picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), pick_const_cursor);
        // set the cursor to the conversion picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), cvt_input_cursor);
        objects_handle_events();
        break;

    case CVT_INPUT:
        gEventLogging->log("set_mode CVT_INPUT");
        // set the cursor to the conversion picture
        XDefineCursor(XtDisplay(top_level), XtWindow(top_level), cvt_input_cursor);
        objects_handle_events();
        break;

    case EXPAND_SUBTREES:
        gEventLogging->log("set_mode EXPAND_SUBTREES");
        ExpandSubtrees(config->root());
        set_mode(NULL, EDIT_MODE);
        break;

    case START_OVER:
        gEventLogging->log("set_mode START_OVER");
        config->start_over();
        set_mode(NULL, EDIT_MODE);
        break;

    case MEXP_ADD_MISSION_TO_CBRLIB:
        gEventLogging->log("set_mode MEXP_ADD_MISSION_TO_CBRLIB");
        gMExp->saveSelectionToCBRLibrary(config->root());

        set_mode(NULL, EDIT_MODE);
        break;

    case MEXP_REPLAY_MISSION:
        gEventLogging->log("set_mode MEXP_REPLAY_MISSION");
        gMExp->runMissionExpert(MEXP_RUNMODE_REPLAY);
        set_mode(NULL, EDIT_MODE);
        break;

    case MEXP_VIEW_MAP_MISSION:
        gEventLogging->log("set_mode MEXP_VIEW_MAP_MISSION");
        gMExp->runMissionExpert(MEXP_RUNMODE_VIEW);
        set_mode(NULL, EDIT_MODE);
        break;

    case MEXP_NEW_MISSION:
        gEventLogging->log("set_mode MEXP_NEW_MISSION");
        confirmed = FALSE;

        if (gMExp->isDisabled())
        {
            warn_userf("Mission Expert not supported.");
            set_mode(NULL, EDIT_MODE);
            break;
        }

        if (config->needs_save() == TRUE)
        {
            // Check to see if the user wants to save the workspace.
            confirmed = gPopWin->discardConfirm(w);

            if (!confirmed)
            {
                set_mode(NULL, EDIT_MODE);
                break;
            }
        }

        // Clear the current workspace and start new one.
        config->new_design();

        // Run Mission Expert.
        gMExp->runMissionExpert(MEXP_RUNMODE_NEW);
        set_mode(NULL, EDIT_MODE);
        break;
    }

    reset_placement_list();
}

///////////////////////////////////////////////////////////////////////
// $Log: toolbar.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2007/09/07 23:10:04  endo
// The overlay name is now remembered when the coordinates are picked from an overlay.
//
// Revision 1.2  2006/07/26 18:07:47  endo
// ACDLPlus class added.
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
// Revision 1.52  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.51  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.50  2002/05/06 17:59:14  endo
// "Delete" functionality added in the edit menu.
//
// Revision 1.49  2002/01/31 10:44:20  endo
// Parameters chaged due to the change of max_vel and base_vel.
//
// Revision 1.48  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.47  2001/02/01 20:06:55  endo
// Modification to display the robot ID and name in the
// "Individual Robot" glyph was added.
//
// Revision 1.46  2000/06/25 03:04:14  endo
// updateTriggerFieldAutomatically() and related functions added.
//
// Revision 1.45  2000/04/25 07:25:27  endo
// TaskExited_design and its related functions added.
//
// Revision 1.44  2000/04/13 21:49:27  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.43  2000/04/02 05:19:11  endo
// marc3f.dat -> marc3f.map
//
// Revision 1.42  2000/03/31 17:03:01  endo
// giNumWayPoints replaced with getNumberOfStatesInFSA.
//
// Revision 1.41  2000/03/30 03:37:35  endo
// marc.ovl --> marc3f.ovl
//
// Revision 1.40  2000/03/30 01:33:04  endo
// setxyTheta changed to Localize.
//
// Revision 1.39  2000/03/23 21:09:01  endo
// *** empty log message ***
//
// Revision 1.38  2000/03/23 20:56:03  endo
// set_mode(NULL, EDIT_MODE); added to the
// case PLANNER_DESIGN: .
//
// Revision 1.37  2000/03/23 20:10:50  endo
// Made cfgedit to turn off or on path_planner based on
// the flag specified in .cfgeditrc.
//
// Revision 1.36  2000/03/22 06:27:51  endo
// mapper --> path_planner
//
// Revision 1.35  2000/03/22 03:59:08  saho
// Added code for integrating cfgedit and the path_planner.
// Some functions that Johnatan wrote for the waypoints were generalized
// to apply not only to GoTo states.
//
// Revision 1.34  2000/03/20 20:36:57  endo
// reset_waypoint_positioning() added.
//
// Revision 1.33  2000/03/15 19:01:44  endo
// Checked in for sapan. Fixed a line to make sure it doesn't
// ask eventlog_filename when the EventLogging is not on.
//
// Revision 1.32  2000/03/15 17:05:35  sapan
// Added logging calls for waypoints
//
// Revision 1.31  2000/03/13 23:59:03  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.30  2000/02/15 22:28:49  sapan
// *** empty log message ***
//
// Revision 1.28  2000/02/02 23:41:53  jdiaz
// waypoints support
//
// Revision 1.27  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.26  1996/12/06  16:18:42  doug
// Added checks to is_selected to make sure that s and selected are
// not null to stop a core dump problem.
// Probably just passes the buck farther up the tree, but this was
// a real problem.
//
// Revision 1.25  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.25  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.24  1996/06/02 16:25:59  doug
// added ability to group nodes
//
// Revision 1.23  1996/05/07  19:53:20  doug
// fixing compile warnings
//
// Revision 1.22  1996/03/12  17:45:14  doug
// *** empty log message ***
//
// Revision 1.21  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.20  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.19  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.18  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.17  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.16  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.15  1996/02/20  22:52:24  doug
// adding EditParms
//
// Revision 1.14  1996/02/20  20:00:22  doug
// *** empty log message ***
//
// Revision 1.13  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.12  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.9  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.8  1996/02/04  17:11:48  doug
// *** empty log message ***
//
// Revision 1.7  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.6  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.5  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.4  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.3  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.2  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.1  1996/01/21  20:12:39  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
