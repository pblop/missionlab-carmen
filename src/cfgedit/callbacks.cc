/**********************************************************************
 **                                                                  **
 **                             callbacks.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: callbacks.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <X11/keysym.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/LabelG.h>


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
#include "callbacks.h"
#include "EventLogging.h"
#include "mission_expert.h"

/*-----------------------------------------------------------------------*/

Widget
PostDialog(Widget parent, int dialog_type, char *msg)
{
   Widget dialog;
   XmString text;

   dialog = XmCreateMessageDialog(parent, "dialog", NULL, 0);
   text = XmStringCreateLocalized(msg);
   XtVaSetValues(dialog,
		 XmNdialogType, dialog_type,
		 XmNmessageString, text,
		 NULL);
   XmStringFree(text);
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   XtSetSensitive(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON), false);
   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc) XtDestroyWidget, NULL);
   XtManageChild(dialog);

   return dialog;
}

/*-----------------------------------------------------------------------*/

#if 0
/* called when zoom button is clicked in glyph */
void
move_down_cb(Widget w, move_down_cbs * data, XmPushButtonCallbackStruct * cbs)
{
   switch (data->style)
    {
    case STYLE_CIRCLE:
       {
	  circle *c = (circle *) data->ptr;
	  Symbol *s = c->agent();

	  // If nothing there, create an empty group
	  if (s == NULL)
	     s = c->create_agent();

	  assert(s);

	  // Move into it.
	  config->move_down(s, true, c->index());
	  break;
       }

    case STYLE_TRIGGER:
       {
	  transition *t = (transition *) data->ptr;
	  Symbol *s = (Symbol *) t->the_agent();

	  // If nothing there, create an empty group
	  if (s == NULL)
	     s = t->create_agent();

	  assert(s);

	  // Move into it.
	  config->move_down(s, true);
	  break;
       }

    case STYLE_GLYPH:
       {
          Page *pg = config->this_page();

	  // Need to use the parent of the pushbutton.
	  glyph *g = pg->widget2obj(XtParent(w));

	  if (g == NULL)
	  {
	     warn_user("Didn't find glyph to match widget in move_down_cb");
	     return;
	  }
	  config->move_down(g->get_src_sym(), g->has_sub_tree());
	  break;
       }

    case STYLE_COMMAND_LIST:
       {
	  // Want to edit the commands from a text editor.
	  char **c = (char **) data->ptr;

	  edit_string(c);

	  // Mark drawing as changed
	  config->made_change();
	  break;
       }
    }
}
#endif

/*-----------------------------------------------------------------------*/
/* called when zoom-out button is clicked in command bar */
void
move_up_cb()
{
   config->move_up();
}

/*-----------------------------------------------------------------------*/

void
update_desc(Widget w, char **value)
{
   assert(value);

   // Read the field back 
   char *new_value = XmTextGetString(w);

   // If changed
   if (*value == NULL || strcmp(*value, new_value))
   {
      // Get rid of the old one
      if (*value)
         free(*value);

      // Prune blanks from left end.
      char *left = new_value;
      while( *left && isspace(*left) )
	 left++;

      // Prune blanks from right end
      int len = strlen(left);
      if( len > 0 )
      {
         char *right = &left[len - 1];
         while( right >= left && isspace(*right) )
	    *right-- = '\0';
      }

      // Duplicate the remaining substring and free the big one.
      *value = strdup(left);
      free(new_value);

      // Mark drawing as changed
      config->made_change();
   }
   else
   {
      // Same as we already have, so dump it.
      free(new_value);
   }
}

/*-----------------------------------------------------------------------*/

void
tf_activated(Widget w, tf_activated_cbs * cbs)
{
    assert(cbs->value);
    bool need_regen = false;

    // Read the field back in case changed
    char *value = XmTextGetString(cbs->widget);

    if (*cbs->value == NULL || strcmp(*cbs->value, value))
    {
        if (cbs->value)
        {
            if (*cbs->value)
            {
                free(*cbs->value);
            }

            *cbs->value = value;
        }
        else
        {
            warn_user("Error: NULL Value pointer, unable to save new value!");
        }

        // Mark drawing as changed
        config->made_change();

        if (gMExp->isEnabled())
        {
            gMExp->madeChange();
        }
    }
    else
    {
        free(value);
    }

    if (need_regen)
    {
        // Regenerate the drawing to get the change
        config->regen_page();
    }
}

/*-----------------------------------------------------------------------*/
// Called when the user clicks in a text field
void
tf_gaining_focus(Widget w, int input_num)
{
   // start cursor on left edge
   XmTextFieldSetInsertionPosition(w, 0);

   // make sure the text show starts at the left edge
   XmTextFieldShowPosition(w, 0);

//DCM 12-4-99: Add support for converting a constant input into a link.
   if( system_mode == CVT_INPUT )
	{
	   // Then the user wants to convert this constant input into a link.
	   if (input_num < 0 )
	   {
	      warn_userf("Only constant inputs can be converted to input links");
         set_mode(NULL, EDIT_MODE);
	      return;
	   }

	   // First, need to find the glyph record
      screen_object *g = config->this_page()->widget2obj(XtParent((XtParent(w))));
      if (g == NULL)
      {
         warn_user("Internal Error: Didn't find glyph in tf_gaining_focus!");
         set_mode(NULL, EDIT_MODE);
         return;
      }

      // Do the conversion.
      g->convert_constant_to_link(input_num);

		// Leave the convert mode
      set_mode(NULL, EDIT_MODE);

      // Mark drawing as changed
      config->made_change();
   
      // Regenerate the drawing to get the change
      config->regen_page();
   }
   else
   {
      // turn the cursor on when entering
      XtVaSetValues(w, XmNcursorPositionVisible, true, NULL);
   }
}

/*-----------------------------------------------------------------------*/
// Called when the user leaves a text field
void
tf_loosing_focus(Widget w, int /*input_num*/)
{
   // start cursor on left edge
   XmTextFieldSetInsertionPosition(w, 0);

   // make sure the text show starts at the left edge
   XmTextFieldShowPosition(w, 0);

   // turn the cursor off when leaving
   XtVaSetValues(w, XmNcursorPositionVisible, false, NULL);
}

/*-----------------------------------------------------------------------*/

static inline void
string_check_adding(XmTextVerifyCallbackStruct * cbs,
		    bool first_must_be_alpha,
		    bool convert_blanks_to_underscores,
		    bool must_be_alpha_numeric,
		    bool convert_bars_to_blanks)
{
   bool deleted_chars = false;

   // If requested, ensure that the first character is A-Z,a-z
   if (first_must_be_alpha && cbs->startPos == 0)
   {
      if (!isalpha(cbs->text->ptr[0]))
      {
	 // delete the first character of the insertion text by shifting.
	 for (int i = 0; i < cbs->text->length - 1; i++)
	    cbs->text->ptr[i] = cbs->text->ptr[i + 1];
	 cbs->text->length -= 1;

	 if (cbs->text->length == 0)
	 {
	    cbs->doit = false;
	    return;
	 }
	 else
	 {
	    deleted_chars = true;
	 }
      }
   }

   for (int pos = 0; pos < cbs->text->length; pos++)
   {
      bool delete_it = false;

      // If requested, convert bars to blanks
      if (convert_bars_to_blanks && (cbs->text->ptr[pos] == '|'))
      {
	 cbs->text->ptr[pos] = ' ';
      }

      // If requested, convert blanks to underscores
      if (convert_blanks_to_underscores && isspace(cbs->text->ptr[pos]))
      {
	 cbs->text->ptr[pos] = '_';
      }

      // If requested, ensure that the first character is A-Z,a-z,_
      if (must_be_alpha_numeric &&
	  !isalnum(cbs->text->ptr[pos]) &&
	  cbs->text->ptr[pos] != '_')
      {
	 delete_it = true;
      }

      if (delete_it)
      {
	 // delete the character by shifting.
	 for (int i = pos; i < cbs->text->length - 1; i++)
	    cbs->text->ptr[i] = cbs->text->ptr[i + 1];
	 cbs->text->length -= 1;

	 if (cbs->text->length == 0)
	 {
	    cbs->doit = false;
	    return;
	 }
	 else
	 {
	    // Since we deleted this char, need to recheck this spot
	    pos--;
	    deleted_chars = true;
	 }
      }
   }

   // Deleted some chars and there are some left, so recheck the remainder
   // since the first position test is position dependent.
   if (deleted_chars && cbs->startPos == 0)
   {
      // Recurse to verify that the new string is legal.
      string_check_adding(cbs,
			  first_must_be_alpha,
			  convert_blanks_to_underscores,
			  must_be_alpha_numeric,
			  convert_bars_to_blanks);
   }
}

/*-----------------------------------------------------------------------*/

void
tf_verify(Widget w, tf_types data_type, XmTextVerifyCallbackStruct * cbs)
{
   // ignore backspaces
   if (cbs->text->ptr == NULL)
      return;

   switch (data_type)
    {
    case TF_AGENT_NAME:
       string_check_adding(cbs, true, true, true, true);
       break;

    case TF_STATE_NAME:
       string_check_adding(cbs, true, true, true, true);
       break;

    case TF_INITIALIZER:
       break;

    case TF_INDEX_NAME:
       string_check_adding(cbs, true, true, true, true);
       break;

    case TF_DESC:
       string_check_adding(cbs, false, false, false, true);
       break;
    }
}

/*-----------------------------------------------------------------------*/

void
add_new_group(void)
{
   Symbol *cur = new Symbol(GROUP_NAME);
   place_object( new glyph(cur, false, &cur->location) );
}

/*-----------------------------------------------------------------------*/

screen_object *
select_behavior(int kind, const bool import)
{
   config->create_name_lists();

   // Define the dialog structure
   dialog_rec dialog =
   {(DIALOG_BLDR) XmCreateSelectionDialog,
       {
	  {XmNlistLabelString, "Select object for placement"},
	  {XmNlistItems, config->behavior_names(kind)},
	  {XmNlistItemCount, config->num_behaviors(kind)},
	  {XmNmustMatch, true},
	  {XmNokLabelString, "Ok"},
	  {XmNcancelLabelString, "Cancel"},
	  {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
	  {NULL, 0}
       },
       {
	  {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_select_agent},
	  {NULL, NULL, NULL}
       }
   };

   char *selection;
   int ok = wait_SelectionBox(NULL, &dialog, &selection);

   // User selected abort 
   if (!ok)
      return NULL;

   // Strip off any description
   strtok(selection, ": ");

   Symbol *cur = config->find_agent_by_name(selection);
   if (cur == NULL)
   {
      warn_userf("Internal error - behavior not found in library: %s", selection);
      return NULL;
   }

   glyph *g = NULL;
   if (cur->is_agent())
   {
      bool detail = !cur->is_fsa();

      // return a duplicate of the tree for the symbol
      g = new glyph(cur->dup_tree(import), detail, &cur->location);
   }
   else
   {
      bool detail = !cur->is_fsa();
      g = add_new_prim(cur, detail);
   }

   return g;
}

/*-----------------------------------------------------------------------*/

Symbol *
pick_behavior_for_delete(int kind)
{
   config->create_name_lists();

   // Define the dialog structure
   dialog_rec dialog =
   {(DIALOG_BLDR) XmCreateSelectionDialog,
       {
	  {XmNlistLabelString, "Select component to remove"},
	  {XmNlistItems, config->behavior_names(kind)},
	  {XmNlistItemCount, config->num_behaviors(kind)},
	  {XmNmustMatch, true},
	  {XmNokLabelString, "Ok"},
	  {XmNcancelLabelString, "Cancel"},
	  {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
	  {NULL, 0}
       },
       {
	  {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_select_agent},
	  {NULL, NULL, NULL}
       }
   };

   char *selection;
   int ok = wait_SelectionBox(NULL, &dialog, &selection);

   // User selected abort 
   if (!ok)
      return NULL;

   // Strip off any description
   strtok(selection, ": ");

   Symbol *cur = config->find_agent_by_name(selection);
   if (cur == NULL)
   {
      warn_userf("Internal error - behavior not found in library: %s", selection);
      return NULL;
   }

   return cur;
}

/*-----------------------------------------------------------------------*/

/* get clicks on arrow buttons to connect up links */
void
button_click(Widget w, int input_num)
{
   static screen_object *start = NULL;

   gEventLogging->log("Glyph Button [%s] pressed", XtName(w));

   screen_object *g = config->this_page()->widget2obj(XtParent((XtParent(w))));
   if (g == NULL)
   {
      warn_user("Internal Error: Didn't find glyph in button_click!");
      return;
   }

   // Make sure it is not read only.
   Symbol *rec = g->get_src_sym();
   
   if((gDisableImportSymbol) && (rec->record_class != RC_USER))
   {
       warn_userf("This object is Read-Only");
       set_mode(NULL, EDIT_MODE);
       return;
   }

   switch(system_mode)
   {     
      case EDIT_MODE:
         if( input_num < 0 )
         {
            start = g;
            set_mode(NULL, MAKING_CONNECTION);

            Position x, y;

            // Find out where the mouse is. 
            Window root, child;
            int    root_x, root_y;               // position in root window
            int    win_x, win_y;                 // position in w's frame
            unsigned int keys_buttons;           //status of mouse buttons
            if( XQueryPointer(XtDisplay(drawing_area),XtWindow(drawing_area),
                         &root, &child, &root_x, &root_y, &win_x, &win_y,
                         &keys_buttons))
            {
               // Pointer is on the screen where the window is.
               x = win_x;
               y = win_y;
            }        
            else        
            {
               // Pointer is off the screen.
               x = 100;
               y = 100;
            }

	    start_rubber_band(g, x, y);
         }
         break;
   
      case MAKING_CONNECTION:
         if( input_num >= 0 )
         {
            Symbol *tail = start->get_src_sym()->real_source();
            Symbol *head = g->get_src_sym();

	    if (start == g )
	    {
	       warn_userf("An output can not be connected to its own input");
               set_mode(NULL, EDIT_MODE);
	       return;
	    }

            if (tail)
            {
	       slot_record slot_info = g->get_slot_info(input_num);

	       if (tail->defining_rec && slot_info.data_type)
	       {
	          Symbol *tail_type = tail->defining_rec->data_type;
	          Symbol *head_type = slot_info.data_type;

	          if (head_type && tail_type && head_type != tail_type &&
		       head->symbol_type != BP_NAME && tail->symbol_type != BP_NAME)
	          {
		     warn_userf("Port types must match: Output = %s, input = %s",
			         tail_type->name, head_type->name);
                     set_mode(NULL, EDIT_MODE);
		     return;
	          }
	       }
            }

            // Erase rubberband.
            set_mode(NULL, EDIT_MODE);

            if (start->add_connection(g, input_num))
	       config->regen_page();
         }
         break;

      case CVT_INPUT:
         if( input_num >= 0 )
         {
            slot_record slt = g->get_slot_info(input_num);
	    bool bad = slt.list_header || 
		   (slt.input_generator_ && *slt.input_generator_);

	    if( !bad )
	    {
               g->convert_link_to_constant(input_num);
   
               // Mark drawing as changed
               config->made_change();
   
               // Regenerate the drawing to get the change
               config->regen_page();
            }
	    else
	    {
	       warn_user("Can only convert unconnected inputs!");
	    }
         }  
	 else
	 {
	    warn_user("Can only convert inputs!");
	 }

         // Reset mode.
         set_mode(NULL, EDIT_MODE);

         break;

      default:
         break;
    }
}

/*-----------------------------------------------------------------------*/

void
push_parm(Symbol * p, Symbol * parm)
{
   // Make a copy of the parm and change it into a pushup reference
   Symbol *new_parm = parm->dup();

   new_parm->input_generator = parm->input_generator->dup(false, parm->input_generator->name);

   if (new_parm->symbol_type != PU_INPUT_NAME)
   {
      // Then first level, so change name and type
      free(new_parm->name);
      new_parm->name = strdupcat(strdup("%"), parm->name);
      new_parm->symbol_type = PU_INPUT_NAME;
   }
   else
   {
      free(new_parm->name);
      new_parm->name = strdup(parm->name);
   }

   // Check if the parm already exists in the parent
   Symbol *rec;
   void *cur;

   if ((cur = p->parameter_list.first(&rec)) != NULL)
   {
      do
      {
         if (strcmp(rec->name, new_parm->name) == 0)
         {
            // Is a dup, so forget it.
            delete new_parm;

            return;
         }
      }
      while ((cur = p->parameter_list.next(&rec, cur)) != NULL);
   }

   // Add the new parm in the parent's record
   p->parameter_list.append(new_parm);
}

/*-----------------------------------------------------------------------*/

static int
process_pu_dests(Symbol * agent, Symbol * parm)
{
   bool found_it = false;

   // Add the pushed up parm to each of our users
   Symbol *p;
   void *cur;

   if ((cur = agent->users.first(&p)) != NULL)
   {
      do
      {
	 // If the user is our binding point, or a group 
	 // then move on up one more level.
	 if ( //p->symbol_type == GROUP_NAME ||
	     (p->symbol_type == BP_NAME && p->bound_to == agent))
	 {
	    found_it = process_pu_dests(p, parm);
	 }
	 else if (p->symbol_type == COORD_NAME &&
		  p->defining_rec->operator_style == FSA_STYLE)
	 {
	    // Then want to hang the new parm off of the index record
	    // So, find the parm record for the member list
	    void *cur_parm;
	    Symbol *member_list = NULL;

	    if ((cur_parm = p->parameter_list.first(&member_list)) != NULL)
	    {
	       do
	       {
		  if (member_list->symbol_type == PARM_HEADER &&
		      member_list->data_type == member_type)
		  {
		     found_it = true;
		     break;
		  }
	       }
	       while ((cur_parm = p->parameter_list.next(&member_list, cur_parm)) != NULL);
	    }
	    if (!found_it)
	    {
	       warn_user("Error: Didn't find the member list!");
	       continue;
	    }

	    // Now look for the state connected to this agent
	    void *cur_state;
	    Symbol *state = NULL;

	    found_it = false;

	    if ((cur_state = member_list->parameter_list.first(&state)) != NULL)
	    {
	       do
	       {
		  if (state->input_generator == agent)
		  {
		     found_it = true;

		     // Push it up
		     push_parm(state->list_index, parm);

		     break;
		  }
	       }
	       while ((cur_state = member_list->parameter_list.next(&state, cur_state)) != NULL);
	    }
	    if (!found_it)
	    {
	       warn_user("Error: Didn't find matching state!");
	       continue;
	    }
	 }
	 else
	 {
	    // Push it up
	    push_parm(p, parm);

	    found_it = true;
	 }
      }
      while ((cur = agent->users.next(&p, cur)) != NULL);
   }
   return found_it;
}

/*-----------------------------------------------------------------------*/

/* called to pushup a constant input */
void
pushup_constant_cb(Widget w, const_click * data)
{
   if( system_mode != PUSHUP )
      return;

   bool found_user = false;

   if (data->style == STYLE_GLYPH)
   {
      // Get a pointer to the symbol we are mucking with
      Symbol *agent = data->g->get_src_sym();

      assert(agent != NULL);

      // Get a pointer to the parm we are pushing up
      Symbol *parm = data->g->input_parm(data->slot);

      assert(parm != NULL);

      found_user = process_pu_dests(agent, parm);
      if (found_user)
      {
	 // fixup this parm
	 if( parm->input_generator )
	 {
	    // Make the constant into a standard pushed up parm
	    parm->input_generator->symbol_type = PU_INITIALIZER;
	    parm->input_generator->data_type = NULL;
	    parm->input_generator->name = NULL;
	 }
	 else
	 {
	    warn_user("Internal Error: Missing input generator in pushup parm");
	 }
      }
   }

   if (found_user)
   {
      // Regenerate the drawing to get the changes
      config->regen_page();

      // Mark drawing as changed
      config->made_change();

      // Reset the placement menu to enable the allowed components
      reset_placement_list();
   }
   else
   {
      warn_user("Error: There is no parent to push the parm to!");
   }

   set_mode(NULL, EDIT_MODE);
   push_up_input = false;
}

/*-----------------------------------------------------------------------*/

/* called to pushup an FSA component parameter into the FSA record */
void
pushup_FSA_component_parm(Symbol *fsa, Symbol *parm, 
	bool change_name, Symbol *new_type, char *new_name)
{
   if( fsa->defining_rec->operator_style != FSA_STYLE)
   {
      warn_user("Error: Didn't find the FSA parent record in pushup_FSA_component_parm!");
      return;
   }

   // fixup this parm record
   if( parm->input_generator == NULL )
   {
      warn_user("Error: Missing input generator in pushup_FSA_component_parm");
      return;
   }

   if( change_name )
   {
      char *old_name = parm->name;
      parm->name = new_name;

      // Push up the renamed parm
      push_parm(fsa, parm);

      // Restore the name
      parm->name = old_name;

      // Set the new name for this parameter
      parm->input_generator->name = new_name;

      // Set the new type for this parameter
      parm->input_generator->data_type = new_type;
   }
   else
   {
      // Push up the parm
      push_parm(fsa, parm);

      // Zero out the value, since just pushed it up
      parm->input_generator->name = NULL;
      parm->input_generator->data_type = NULL;
   }

   // Make the constant into a pushed up parm
   parm->input_generator->symbol_type = PU_INITIALIZER;
}

/*-----------------------------------------------------------------------*/

// called to unhook a connection
void
kill_connection_cb(Widget w, int input_num)
{
   gEventLogging->log("Glyph Button [%s] pressed", XtName(w));

   screen_object *s = config->this_page()->widget2obj(XtParent(XtParent(w)));

   if (s == NULL)
   {
      warn_user("Internal Error: Didn't find screen object in kill_connection_cb!");
      return;
   }

   // Make sure it is not read only.
   Symbol *rec = s->get_src_sym();
   
   if((gDisableImportSymbol) && (rec->record_class != RC_USER))
   {
       warn_userf("This object is Read-Only");
       set_mode(NULL, EDIT_MODE);
       return;
   }

   // Unhook the connection
   s->remove_input_connection(input_num);

   // Regenerate the drawing to convert the X back to a button
   config->regen_page();

   // Mark drawing as changed
   config->made_change();
}

/*-----------------------------------------------------------------------*/

// Called to add a new slot to a list
void
add_slot_cb(Widget w, int slot)
{
   gEventLogging->log("Glyph Button [%s] pressed", XtName(w));

   screen_object *s = config->this_page()->widget2obj(XtParent((XtParent(w))));

   if (s == NULL)
   {
      warn_user("Internal Error: Didn't find object in add_slot_cb!");
      return;
   }

   // Make sure it is not read only.
   Symbol *rec = s->get_src_sym();
   
   if((gDisableImportSymbol) && (rec->record_class != RC_USER))
   {
       warn_userf("This object is Read-Only");
       set_mode(NULL, EDIT_MODE);
       return;
   }

   if( !s->add_input_slot(slot) )
   {
      warn_user("Internal Error: Unable to add new slot to record!");
      return;
   }

   // Regenerate the drawing to show the new slot
   config->regen_page();

   // Mark drawing as changed
   config->made_change();
}

/*-----------------------------------------------------------------------*/

// Called when one of the name visibility toggles changes on the configure menu
void
configure_nv_toggle_cb(Widget w, unsigned long architecture, 
	XmToggleButtonCallbackStruct *cbs)
{
   if( cbs->reason == XmCR_VALUE_CHANGED )
   {
      // build a bit mask for the new value of the bit.
      unsigned long bit = cbs->set ? architecture : 0;

      // Make the value of the global this new value.
      name_visibility = (name_visibility & (~architecture)) | bit;

      // Rebuild the name lists
      config->create_name_lists();
   }
}

///////////////////////////////////////////////////////////////////////
// $Log: callbacks.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2007/09/18 22:36:10  endo
// Mission time windows in ICARUS objectives can be now updated based on the environment message.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.68  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.67  2002/01/31 10:44:20  endo
// Parameters chaged due to the change of max_vel and base_vel.
//
// Revision 1.66  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.65  2000/04/13 21:37:13  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.64  2000/01/20 03:16:05  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.63  1998/11/12  19:47:34  endo
// "Abort" button reanamed --> "Cancel"
//
// Revision 1.62  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.61  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.63  1996/10/03 21:46:40  doug
// nested FSA's are working
//
// Revision 1.62  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.61  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.60  1996/06/02 16:25:59  doug
// removed group_cb
//
// Revision 1.59  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.58  1996/05/02  22:58:26  doug
// *** empty log message ***
//
// Revision 1.57  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.56  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.55  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.54  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.53  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.52  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.51  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.50  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.49  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.48  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.47  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.46  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.45  1996/01/20  23:40:47  doug
// Split nodes code was completely rewritten to present multiple choices
//
// Revision 1.44  1996/01/19  20:54:42  doug
// *** empty log message ***
//
// Revision 1.43  1996/01/10  19:34:39  doug
// *** empty log message ***
//
// Revision 1.42  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.41  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.40  1995/11/12  22:39:46  doug
// *** empty log message ***
//
// Revision 1.39  1995/11/08  16:50:02  doug
// *** empty log message ***
//
// Revision 1.38  1995/11/07  14:30:36  doug
// *** empty log message ***
//
// Revision 1.37  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.36  1995/11/04  18:57:35  doug
// *** empty log message ***
//
// Revision 1.35  1995/10/31  19:22:01  doug
// handle text field changes in loose focus callback
//
// Revision 1.34  1995/10/30  23:06:43  doug
// *** empty log message ***
//
// Revision 1.33  1995/10/27  20:28:50  doug
// *** empty log message ***
//
// Revision 1.32  1995/10/26  14:59:55  doug
// type checking is working
//
// Revision 1.31  1995/09/26  21:42:27  doug
// working on group_cb
//
// Revision 1.31  1995/09/26  21:42:27  doug
// working on group_cb
//
// Revision 1.30  1995/09/15  22:02:20  doug
// added support for the command list in the executive
//
// Revision 1.29  1995/09/07  14:22:26  doug
// works
//
// Revision 1.28  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.27  1995/06/29  17:07:34  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
