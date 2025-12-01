/**********************************************************************
 **                                                                  **
 **                        configuration.cc                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: configuration.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <string>

#include <Xm/Xm.h>
#include <Xm/Separator.h>
#include <Xm/LabelG.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushBG.h>
#include <Xm/MessageB.h>
#include <Xm/ScrollBar.h>


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
#include "renumber_robots.h"
#include "no_make_needed.h"
#include "EventLogging.h"
#include "callbacks.h"
#include "mission_expert.h"
#include "assistantDialog.h"

const char *empty_name = "UNTITLED";

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string configuration::EMPTY_STRING_ = "";

/*-----------------------------------------------------------------------*/
void configuration::update_menu_bar(void)
{
    char *name = "??????";

    if (at_top())
    {
        name = configname;
    }
    else
    {
        // Check if there is a page name override.
        Symbol *pg = config->parent_page();

        if(pg && pg->symbol_type == INDEX_NAME)
        {
            name = pg->name;
        }

        // No override, so use the normal name
        else
        {
            Symbol *sym = cur_page->this_agent();

            if (sym)
            {
                name = sym->name;
            }
        }
    }

    // Update the menu bar
    XmString str = XmStringCreateLocalized(name);

    XtVaSetValues(
        cur_page_label,
        XmNlabelString, str,
        NULL);

    XmStringFree(str);
}

//-----------------------------------------------------------------------
// This function repositions the vertical and horizontal slider bars to
// their original positions.
//-----------------------------------------------------------------------
void configuration::reset_slider_bars(void)
{
    Widget vBar, hBar;
    int sliderSize, increment, pageIncrement;

    XtVaGetValues(
        scrolled_window,
        XmNverticalScrollBar, &vBar,
        XmNhorizontalScrollBar, &hBar,
        NULL);
    
    XtVaGetValues(
        vBar,
        XmNsliderSize, &sliderSize,
        XmNincrement, &increment,
        XmNpageIncrement, &pageIncrement,
        NULL );

    XmScrollBarSetValues(
        vBar,
        0,
        sliderSize,
        increment,
        pageIncrement,
        true);

    XtVaGetValues(
        hBar,
        XmNsliderSize, &sliderSize,
        XmNincrement, &increment,
        XmNpageIncrement, &pageIncrement,
        NULL );

    XmScrollBarSetValues(
        hBar,
        0,
        sliderSize,
        increment,
        pageIncrement,
        true);
}

/*-----------------------------------------------------------------------*/
// Delete the current configuration
void
configuration::delete_configuration(void)
{
   // Remove the current page.
   if (cur_page)
   {
      delete cur_page;
      cur_page = NULL;
   }

   // Free the config name
   if (configname)
   {
      free(configname);
      configname = NULL;
   }

   // Free the file name
   if (filename)
   {
      free(filename);
      filename = NULL;
   }
   change_filename(NULL);

   // No robots active
   robot_ = NULL;

   // Architecture is unbound
   arch_ = NULL;

   // don't need write
   needs_write = false;

   // Delete the configuration tree
   root_->delete_tree();
   root_ = NULL;

   // Revert back to the free architecture
   UseArch(NULL);

   // Reset the editor stuff
   XmString str = XmStringCreateLocalized("free");
   XtVaSetValues(cur_arch_label,
                 XmNlabelString, str,
                 NULL);
   XmStringFree(str);  

   reset_binding_list();
}

/*-----------------------------------------------------------------------*/
void
configuration::make_empty_config(void)
{
   // Make a record for the configuration
   Symbol *p = new Symbol(CONFIGURATION_NAME);

   p->defining_rec = NULL;
   p->record_class = RC_USER;
   p->name = strdup("");
   p->location.x = 10;
   p->location.y = 10;

   cur_page = new Page(p);
   if (configname)
      free(configname);
   configname = strdup(p->name);

   root_ = p;
   filename = NULL;
   robot_ = NULL;

   update_menu_bar();
}

/*-----------------------------------------------------------------------*/

// Delete the current configuration and create an empty design.
void configuration::new_design(void)
{
   delete_configuration();

   traversal_links.clear();

   // Make an empty configuration
   make_empty_config();

   reset_placement_list();
}

/*-----------------------------------------------------------------------*/

// called to clear an old configuration from the screen
configuration::~configuration()
{
   delete_configuration();
}

/*-----------------------------------------------------------------------*/

// write a new configuration up on the screen
configuration::configuration(void)
{
   configname = NULL;
   robot_ = NULL;

   // Make a record for the configuration
   make_empty_config();

   for (int i = 0; i < NUM_GROUPS; i++)
   {
      num_behaviors_[i] = 0;
      behavior_names_[i] = NULL;
   }
   num_arches_ = 0;
}

/*-----------------------------------------------------------------------*/

// Add a new toplevel page into the traversal stack.
void
configuration::add_new_top_page(Symbol * new_page_rec)
{
   // add the new top page to the end of the stack.
   traversal_links.append(new_page_rec);

   // Will be a new page name for our parent.
   update_menu_bar();
}

/*-----------------------------------------------------------------------*/

/* move down to the expanded view of the selected agent */
void
configuration::move_down(Symbol * rec, bool open_sub_page, Symbol *prestuff)
{
   // selections don't persist across pages
   forget_selections();

   if( open_sub_page )
   {
      Symbol *cur = cur_page->this_agent();

      // Save current location so can back up
      traversal_links.insert(cur);

      // If prestuff data, then add that to the stack so it will display
      // instead of the current page name.
      if( prestuff )
      {
         assert( prestuff->symbol_type == INDEX_NAME );
         traversal_links.insert(prestuff);
      }

      // If moving into a robot, then remember it
      if (rec->is_robot() || rec->is_robot_bp())
      {
         robot_ = rec;
         gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ROBOT);
      }

      // Move down to the new page
      delete cur_page;
      cur_page = NULL;
      cur_page = new Page(rec);
      
      update_menu_bar();
      reset_placement_list();

      if (cur->is_robot())
      {    
          gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ASSEMBLAGE);
      }
      else if (cur->is_fsa())
      {
          gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_FSA_EMPTY); 
      }
      else if (cur->symbol_type == GROUP_NAME)
      {
          gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ROBOT);
      }
      else
      {
          gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
      }
   }
   else // Show the implementation of this agent
   {
      if( !rec->defining_rec || !rec->defining_rec->name )
      {
         warn_user("Internal error: No defining record");
         return;
      }

      char filename[2048];
      sprintf(filename, "%s.cnl", rec->defining_rec->name);
      edit_file(filename);
   }
}

/*-----------------------------------------------------------------------*/

// Called when the user wishes to check what the parent page is.
Symbol *
configuration::parent_page()
{
   Symbol *parent = NULL;
   parent_page_loc_ = traversal_links.first(&parent);

   return parent;
}

/*-----------------------------------------------------------------------*/

// Called when the user wishes to check what the parent page is.
Symbol *
configuration::next_parent_page()
{
   Symbol *parent = NULL;
   parent_page_loc_ = traversal_links.next(&parent, parent_page_loc_);

   return parent;
}

/*-----------------------------------------------------------------------*/

// Called when the user wishes to move up the tree
void
configuration::move_up()
{
   if (at_top())
   {
      warn_userf("Already at top level");
      gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
      return;
   }

   gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
   // selections don't persist across pages
   forget_selections();

   // If leaving the robot, then unmark it
   if (cur_page->this_agent()->is_robot())
      robot_ = NULL;

   if( traversal_links.isempty() )
   {
      warn_userf("Error: already at top level");
      gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
      return;
   }

   // Get the symbol defining our parent page from the stack
   Symbol *cur = traversal_links.get();

   // If it is prestuff data, then skip over it
   if( cur->symbol_type == INDEX_NAME )
   {
      if( traversal_links.isempty() )
      {
         warn_userf("Error: already at top level");
         gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
         return;
      }

      cur = traversal_links.get();
   }

   // Erase the existing page
   delete cur_page;
   cur_page = NULL;

   // Create the new page
   cur_page = new Page(cur);

   update_menu_bar();

   reset_placement_list();
}

/*-----------------------------------------------------------------------*/

// Called when split creates a new node, to make sure the old one
// is replaced if it is in the traversal list
void
configuration::replace_traversal_page(Symbol *old_rec, Symbol *new_rec)
{
   traversal_links.replace(old_rec,new_rec);
   if( old_rec == cur_page->this_agent() )
      regen_page(new_rec);
}

//-----------------------------------------------------------------------
// called to load a new configuration
bool
configuration::load(const char *new_filename, const bool reset_filename)
{
   gEventLogging->start("load a new configuration %s", new_filename);

   // --------------- Everything here is non-destructive ------------------

   // Scan across for just the filename without directories or extensions
   const char *start = strrchr(new_filename, '/');

   if (start == NULL)
      start = new_filename;
   else
      start++;			// Bump past the slash we just found.

   char *fileonly = strdup(start);

   char *end = strchr(fileonly, '.');

   if (end != NULL)
      *end = '\0';

   // load_cdl wants a NULL terminated list of filenames to load
   const char *file_list[2];

   file_list[0] = new_filename;
   file_list[1] = NULL;

   //------------- OK, commit. --------------------
   // delete the current configuration (if there is one) 
   new_design();

   // Load the new one
   bool errors;
   Symbol *cfg = load_cdl(file_list, errors, false, debug_load_cdl);

   // If unable to load anything, leave them with an empty config
   if (cfg == NULL)
   {
      warn_user("Error: Unable to load the configuration!");
      gEventLogging->cancel("Loadfile");
      return true;
   }

   if( errors )
   {
      // If some errors, ask for confirmation
      // Define the exit_design dialog structure
      dialog_rec dialog =
      {  (DIALOG_BLDR)XmCreateQuestionDialog,
         {
            {XmNmessageString, "Unable to completely load the configuration: Continue with partial load?"},
            {XmNokLabelString,    "Continue"},
            {XmNcancelLabelString,"Cancel"},
            {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
            {XmNdialogStyle,     XmDIALOG_FULL_APPLICATION_MODAL},
            {NULL, 0}
         },
         { {NULL, NULL, NULL} }
      };

      if( !wait_dialog_box(NULL, &dialog,NULL) )
      {
	 gEventLogging->cancel("Loadfile");
         return true;
      }
   }

   // if got a group as the top agent, copy its child list over and kill it.
   if (cfg->symbol_type == GROUP_NAME)
   {
      root_->children = cfg->children;

      // Stop delete from removing the name
      cfg->name = NULL;
      delete cfg;

      // Need to delete the user lists for each of the children, since
      // they point at the group record, which we just deleted
      // and nothing really uses the top level agents
      Symbol *p;
      void *cur;

      if ((cur = root_->children.first(&p)) != NULL)
      {
	 do
	 {
	    p->users.clear();
	 }
	 while ((cur = root_->children.next(&p, cur)) != NULL);
      }
   }
   else
   {
      root_->children.append(cfg);
   }
    
   // Update the architecture binding, if changed
   if( choosen_architecture != arch_ )
   {
      // Update the menu bar
      XmString str = XmStringCreateLocalized(choosen_architecture->name);

      XtVaSetValues(cur_arch_label,
      		    XmNlabelString, str,
   		    NULL);
      XmStringFree(str);

      arch_ = (Symbol *)choosen_architecture;
      reset_binding_list();
   }

   if( reset_filename )
   {
      // Save the configuration name for display on the menu bar
      if (configname)
         free(configname);
      configname = fileonly;

      if (filename)
         free(filename);
      filename = strdup(new_filename);
   }

   // Mark doesn't need save
   did_save();

   // Renumber the robots to get consistent names
   renumber_robots();

   // Check if the executables are current
   if( no_make_needed() )
   {
      have_fresh_make = true;
      XtSetSensitive(run_button, true);
   }

   create_name_lists();

   reset_placement_list();

   // Draw the new page
   regen_page();

   update_menu_bar();
   reset_slider_bars();

   gEventLogging->end("load a new configuration %s", new_filename);

   return false;
}

//-----------------------------------------------------------------------
// Create the behavior names
//-----------------------------------------------------------------------
void configuration::create_name_lists()
{
    // Build the list of restrictions
    char *names[33];
    int index = 0;
    int num_names = 0;

    while( architecture_table[index].name )
    {
        if(name_visibility & architecture_table[index].bit)
        {
            names[num_names++] = architecture_table[index].name;
        }

        index ++;
    }
    names[num_names] = NULL;

    AllNames(list_of_names, names);

    // delete the list if it exists
    for (int i = 0; i < NUM_GROUPS; i++)
    {
        if (behavior_names_[i])
        {
            delete behavior_names_[i];
        }
        behavior_names_[i] = NULL;

        // Count number of entries
        num_behaviors_[i] = list_of_names[i].len();

        // Allocate space for the names
        if(num_behaviors_[i])
        {
            behavior_names_[i] = (XmString *)XtMalloc(num_behaviors_[i]*sizeof(XmString));
        }

        // Create the strings
        void *cur;
        char *str;
        int j = 0;
        if( (cur = list_of_names[i].first(&str)) != NULL )
        {
            do
            {
                behavior_names_[i][j++] = XmStringCreateLocalized( str );
            } while( (cur = list_of_names[i].next(&str,cur)) != NULL );
        }
    }
}

//-----------------------------------------------------------------------
// Get the list of all the names for specified kind.
//-----------------------------------------------------------------------
vector<config_bahavior_name_t> configuration::get_name_list_(int kind)
{
    config_bahavior_name_t behavior_name;
    vector<config_bahavior_name_t> name_list;
    string wholestring;
    char *names[33];
    char *str = NULL;
    int index, length, num_names = 0;
    void *cur = NULL;

    index = 0;

    while(architecture_table[index].name)
    {
        if( name_visibility & architecture_table[index].bit )
        {
            names[num_names++] = architecture_table[index].name;
        }

        index++;
    }

    names[num_names] = NULL;

    AllNames(list_of_names, names);

    if((cur = list_of_names[kind].first(&str)) != NULL)
    {
        do
        {
            // Get the whole string (name + description)
            wholestring = str;
            length = wholestring.size();

            // Rererieve the name and description fields, separated by "|".
            index = wholestring.find("|");
            behavior_name.name = wholestring.substr(0, index);
            behavior_name.description = wholestring.substr(index+2, length-index-2);

            // Trim the trailing whitespace of the name fileld.
            index = behavior_name.name.find(" ");
            behavior_name.name = behavior_name.name.substr(0, index);

            // Add it to the list.
            name_list.push_back(behavior_name);

        } while((cur = list_of_names[kind].next(&str,cur)) != NULL);
    }

    return name_list;
}

//-----------------------------------------------------------------------
// return the desired symbol based on its position in the name list
Symbol *
configuration::behavior_by_pos(int kind, int pos) const
{
    // Create the strings
    void *cur;
    char *str;
    int i = 0;
    if( (cur = list_of_names[kind].first(&str)) != NULL )
    {
        do
        {
            // Are we done?
            if( i == pos )
            {
                char *selection = strdup(str);

                // Strip off any description
                strtok(selection, ": ");
       
                Symbol *cur = config->find_agent_by_name(selection);

                if (cur == NULL)
                {
                    warn_userf(
                        "Internal error - behavior not found in library: %s",
                        selection);

                    return NULL;
                }

                // Done, return the symbol
                return cur;
            }

            // Count it
            i++;

        } while((cur = list_of_names[kind].next(&str,cur)) != NULL);
    }

    warn_user("Internal Error: pos is out of range in behavior_by_pos");

    return NULL;
}

//-----------------------------------------------------------------------
// return the desired symbol based on its position in the name list
int
configuration::pos_by_name(int kind, char *name) const
{
#if 0
   // Search the strings
   void *cur;
   char *str;
   int i = 0;
   if( cur = list_of_names[kind].first(&str) )
   {
      do
      {
	 char *selection = strdup(str);

         // Strip off any description
         strtok(selection, ": ");
       
	 // Are we done?
	 if( i == pos )
	 {
	    Symbol *cur = config->find_agent_by_name(selection);
	    if (cur == NULL)
	    {
               warn_userf("Internal error - behavior not found in library: %s",
			selection);
	       return NULL;
            }

            // Done, return the symbol
	    return cur;
	 }

	 // Count it
	 i++;

      } while( cur = list_of_names[kind].next(&str,cur) );
   }

   warn_user("Internal Error: pos is out of range in behavior_by_pos");
   return NULL;
#endif
   return 0;
}

//-----------------------------------------------------------------------
// called to write the current configuration to a file
bool
configuration::save(const char *filename)
{
   // Mark doesn't need save
   did_save();

   // Remember the new name
   configname = (char *)filename;

   // good save
   return true;
}

//-----------------------------------------------------------------------
// called to change the configuration name
void
configuration::rename(const char *name)
{
   if( configname )
      free(configname);

   // Remember the new name
   configname = strdup(name);
}

//-----------------------------------------------------------------------
// Create the architecture names
void
  configuration::create_arch_lists()
{
   // Create the behavior lists
   GTList < char *>arch_names;

   ArchNames(&arch_names);

   /* create a SORTED list of the names as Xmstrings */
   num_arches_ = arch_names.len();

   // Allocate space for the names
   arch_names_ = new XmString[num_arches_];

   for (int i = 0; i < num_arches_; i++)
      arch_names_[i] = XmStringCreateLocalized(arch_names.get());
}

/*-----------------------------------------------------------------------*/

// Bind to the chosen architecture
// Return true if successful
bool
configuration::bind_arch(char *archname)
{
   Symbol *arch = LookupName(archname);

   if (arch == NULL)
   {
      warn_userf("Internal Error: Didn't find record for architecture: %s",
		 archname);
      return false;
   }

   // Update the menu bar
   XmString str = XmStringCreateLocalized(archname);

   XtVaSetValues(cur_arch_label,
		 XmNlabelString, str,
		 NULL);
   XmStringFree(str);

   // Do the work
   UseArch(arch);
   arch_ = arch;

   reset_binding_list();

   // May add buttons on glyphs
   regen_page();

   return true;
}

/*-----------------------------------------------------------------------*/

// Unbind the architecture
// Return true if successful
bool
configuration::unbind_arch()
{
   // Update the menu bar
   XmString str = XmStringCreateLocalized("free");

   XtVaSetValues(cur_arch_label,
		 XmNlabelString, str,
		 NULL);
   XmStringFree(str);

   reset_binding_list();

   // May remove buttons on glyphs
   regen_page();

   return true;
}

/*-----------------------------------------------------------------------*/

// Delete and regenerate the current page to get any changes
void
configuration::regen_page(Symbol *new_page_def)
{
   Symbol *cur;
   if( new_page_def )
      cur = new_page_def;
   else
      cur = cur_page->this_agent();

   delete cur_page;
   cur_page = NULL;

   cur_page = new Page(cur);

   update_menu_bar();
}

/*-----------------------------------------------------------------------*/

void
  configuration::made_change()
{
   needs_write = true;
   XtSetSensitive(cur_file_label, true);

   // Also need to clear the ability to run the executable
   have_fresh_make = false;
   XtSetSensitive(run_button, false);
}

/*-----------------------------------------------------------------------*/

void
  configuration::did_save()
{
   needs_write = false;
   XtSetSensitive(cur_file_label, false);
}

/*-----------------------------------------------------------------------*/

// load the page stack as the current page and then position so rec is
// in the center of the window.
// NOTE: the stack of pages is ordered with the top level on the bottom.
void 
configuration::goto_page(SymbolList *page_stack, Symbol *rec)
{
   // Move up to the top of the config
   while ( !at_top() )
      traversal_links.get();

   // load the stack of records while not destroying the page_stack
   Symbol *p;
   void *cur;
   if ((cur = page_stack->first(&p)) != NULL)
   {
      do
      {
         // insert the page record
         traversal_links.append(p);

         // If moving into a robot, then remember it
         if (p->is_robot() || (rec != NULL && rec->is_robot_bp()))
            robot_ = p;
      }
      while ((cur = page_stack->next(&p, cur)) != NULL);
   }

   // TRICK::: we stacked the desired page, so move up will delete the page
   // that is up and draw the real page we want.
   move_up();

   // Center up on the record
}

/*-----------------------------------------------------------------------*/

Symbol *configuration::find_agent_by_name(char *name) const
{
    Symbol *rtn = LookupName(name);

    if(rtn == NULL)
    {
        rtn = LookupName(&free_arch->table, name);
    }

    return rtn;
}

/*-----------------------------------------------------------------------*/
// This routine will make cfgdit to quit the current configuration
// and start new one.
void configuration::start_over(void)
{

    // First, make sure the user wants to really start over.
    //if (config->needs_save())
    if (needs_save())
    {
        // Define the exit_design dialog structure
        dialog_rec start_over_dialog =
            {(DIALOG_BLDR) XmCreateQuestionDialog,
             {
                 {XmNmessageString, "Discard workspace and start over?"},
                 {XmNokLabelString, "Yes"},
                 {XmNcancelLabelString, "Cancel"},
                 {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                 {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                 {NULL, 0}
             },
             {
                 {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer)HELP_PAGE_start_over},
                 {NULL, NULL, NULL}}
            };

        if (!wait_dialog_box(NULL, &start_over_dialog, NULL))
        {
            return;
        }
    }
   
    // Mark drawing as clean
    did_save();

    /*
    if (gWaypointOverlayName != NULL)
    {
        free(gWaypointOverlayName);
        gWaypointOverlayName = NULL;
    }
    */

    // Start new configuration
    if (gMExp)
    {
        gMExp->initialize();
    }

    gPopWin->runInitialDialog();
    gEventLogging->log("RestartConfig");
}

void configuration::quit(void)
{
    save_cdl_for_replay("quit");

    gEventLogging->close();

    if (gMExp != NULL)
    {
        delete gMExp;
        gMExp = NULL;
    }

    if (gEventLogging != NULL)
    {
        delete gEventLogging;
    }

    delete this;
    exit(0);
}

// This page (level) is read-only
bool configuration::is_read_only_page(void)
{
    Symbol *p;
    bool is_read_only = false;

    p = this_page()->this_agent();

    if (p) is_read_only = (p->record_class != RC_USER);

    return is_read_only;
}

//-----------------------------------------------------------------------
// This function finds the specified state in FSA.
//-----------------------------------------------------------------------
string configuration::findStateInFSA_(
    string fsaName,
    string stateName,
    SymbolList *pageStack,
    Symbol *node,
    Symbol **location)
{
    Symbol *p = NULL, *ruleList = NULL, *rule = NULL;
    Symbol *memberList = NULL, *member = NULL, *mem = NULL;
    Symbol *startState = NULL, *dest = NULL;
    symbol_table <Symbol> states;
    void *cur;
    char *ruleSrcName = NULL, *ruleDestName = NULL, *memName = NULL;
    string description, nodeName, realStateName;
    bool used = false;
    const bool DEBUG = false;

    nodeName = node->name;

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
                    ruleList = p;
                }
                else if (p->data_type == member_type)
                {
                    memberList = p;
                }
            }
        }
        while ((cur = node->parameter_list.next(&p, cur)) != NULL);
    }

    // Make sure found the states
    if (memberList != NULL)
    {
        // Create the states for the FSA

        if ((cur = memberList->parameter_list.first(&member)) != NULL)
        {
            do
            {
                if (DEBUG)
                {
                    fprintf(
                        stderr,
                        "member->list_index->description = %s\n",
                        member->list_index->description);
                    fprintf(
                        stderr,
                        "member->list_index->name = %s\n",
                        member->list_index->name);
                }

                description = member->list_index->description;
                realStateName = member->list_index->name;

                if ((nodeName == fsaName) && (description == stateName))
                {
                    return realStateName;
                }

                if (member->input_generator == NULL ||
                    member->input_generator->children.len() == 0)
                {
                    if (member->input_generator)
                    {
                        // Move into the state
                        pageStack->insert(member->list_index);
                        pageStack->insert(member->input_generator);
                    }

                    return EMPTY_STRING_;
                }

                if ( strcasecmp(member->list_index->name,"start") == 0)
                {
                    startState = member;
                }

                realStateName = findStateInPage_(
                    fsaName,
                    stateName,
                    pageStack,
                    member->input_generator,
                    member->list_index,
                    location);

                if (realStateName != EMPTY_STRING_)
                {
                    return realStateName;
                }

                // put the states into the symbol table for easy access.
                states.put(member->list_index->name, member);
            }
            while ((cur = memberList->parameter_list.next(&member, cur)) != NULL);
        }
    }

    // Check the transitions.
    if (ruleList != NULL)
    {
        // Step through each rule

        if ((cur = ruleList->parameter_list.first(&rule)) != NULL)
        {
            do
            {
                used = false;
                ruleSrcName = rule->list_index->name;
                ruleDestName = rule->input_generator->list_index->name;

                // Check it against each state to find the one sourcing it.
                mem = (Symbol *)states.first();

                while (mem)
                {
                    memName = mem->list_index->name;

                    if (strcmp(ruleSrcName, memName) == 0)
                    {
                        used = true;
                        dest = (Symbol *)states.get(ruleDestName);

                        realStateName = findStateInPage_(
                            fsaName,
                            stateName,
                            pageStack,
                            rule->input_generator->input_generator,
                            NULL,
                            location);
                        
                        if (realStateName != EMPTY_STRING_)
                        {
                            return realStateName;
                        }

                        break;
                    }

                    mem = (Symbol *)states.next();
                }
            }
            while ((cur = ruleList->parameter_list.next(&rule, cur)) != NULL);
        }
    }

    return EMPTY_STRING_;
}

//-----------------------------------------------------------------------
// This function finds the specified state in node.
//-----------------------------------------------------------------------
string configuration::findStateInNode_(
    string fsaName,
    string stateName,
    SymbolList *pageStack,
    Symbol *node,
    Symbol **location)
{
    Symbol *p = NULL, *ig = NULL, *lp = NULL;
    string realStateName;
    void *cur = NULL, *curLp = NULL;
    bool isGrouper = false;

    if (node == NULL)
    {
        return EMPTY_STRING_;
    }
    
    if (node->marked)
    {
        return EMPTY_STRING_;
    }

    if (node->symbol_type == BP_NAME)
    {
        if(node->is_bound_bp())
        {
            realStateName = findStateInNode_(
                fsaName,
                stateName,
                pageStack,
                node->bound_to,
                location);

            return realStateName;
        }
    }
    // If this is a grouping agent, then we won't show detail in the glyph
    isGrouper = node->is_grouping_op();

    // Mark this node as checked
    node->marked = true;

    // Check its direct children
    if (!isGrouper)
    {
        if ((cur = node->parameter_list.first(&p)) != NULL)
        {
            do
            {
                if (p->symbol_type == PARM_HEADER)
                {
                    if ((curLp = p->parameter_list.first(&lp)) != NULL)
                    {
                        do
                        {
                            ig = lp->input_generator;

                            if(ig != (Symbol *)UP)
                            {
                                if (ig->is_agent())
                                {
                                    realStateName = findStateInNode_(
                                        fsaName,
                                        stateName,
                                        pageStack,
                                        ig,
                                        location);

                                    if (realStateName != EMPTY_STRING_)
                                    {
                                        return realStateName;
                                    }
                                }
                            }
                        }
                        while ((curLp = p->parameter_list.next(&lp, curLp)) != NULL);
                    }
                }
                else
                {
                    ig = p->input_generator;

                    if (ig != (Symbol *)UP)
                    {
                        if (ig->is_agent())
                        {
                            realStateName = findStateInNode_(
                                fsaName,
                                stateName,
                                pageStack,
                                ig,
                                location);

                            if (realStateName != EMPTY_STRING_)
                            {
                                return realStateName;
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
        realStateName = findStateInPage_(
            fsaName,
            stateName,
            pageStack,
            node,
            NULL,
            location);

        return realStateName;
    }

    return EMPTY_STRING_;
}

//-----------------------------------------------------------------------
// This function finds the specified state in page.
//-----------------------------------------------------------------------
string configuration::findStateInPage_(
    string fsaName,
    string stateName,
    SymbolList *pageStack,
    Symbol *pageDef,
    Symbol *index,
    Symbol **location)
{
    Symbol *p = NULL;
    string realStateName;
    void *cur = NULL;

    // Make sure things are as expected
    assert(pageDef);
    assert(pageDef->is_grouping_op());

    // The FSA states stuff the index also
    if (index != NULL)
    {
        pageStack->insert(index);
    }

    // push this page
    pageStack->insert(pageDef);

    if (pageDef->is_fsa())
    {
        realStateName = findStateInFSA_(
            fsaName,
            stateName,
            pageStack,
            pageDef,
            location);

        if (realStateName != EMPTY_STRING_)
        {
            return realStateName;
        }
    }
    else
    {
        pageDef->clear_marks();

        // check the tree for each of the children.
        if ((cur = pageDef->children.first(&p)) != NULL)
        {
            do
            {
                // Check the source node
                realStateName = findStateInNode_(
                    fsaName,
                    stateName,
                    pageStack,
                    p,
                    location);

                if (realStateName != EMPTY_STRING_)
                {
                    return realStateName;
                }

            }
            while ((cur = pageDef->children.next(&p, cur)) != NULL);
        }
    }

    // pop this page
    pageStack->get();

    // pop the index
    if (index != NULL)
    {
        pageStack->get();
    }

    return EMPTY_STRING_;
}

//-----------------------------------------------------------------------
// This function hightlights the specified state.
//-----------------------------------------------------------------------
string configuration::highlightState(string fsaName, string stateName)
{
    circle *state = NULL;
    Symbol *agent = NULL, *location = NULL;
    SymbolList *pageStack = NULL;
    string thisFSAName, stateIndex = EMPTY_STRING_;
    bool found = false;

    // Make sure we get any changes from the currently displayed page
    this_page()->update();

    // Get the root.
    agent = root();

    if (agent->children.len() == 0)
    {
        // The configuration is empty.
        return stateIndex;
    }
   
    // Clear all the marked nodes
    agent->clear_marks();

    pageStack = new SymbolList;

    stateIndex = findStateInPage_(
        fsaName,
        stateName,
        pageStack,
        agent,
        NULL,
        &location);
        
    if (stateIndex != EMPTY_STRING_)
    {
        if (this_page()->is_fsa())
        {
            thisFSAName = this_page()->this_agent()->name;

            if (thisFSAName == fsaName)
            {
                state = this_page()->this_fsa()->getState(stateIndex);

                if (state != NULL)
                {
                    found = true;
                }
            }
        }

        if (!found)
        {
            goto_page(pageStack, NULL);

            state = this_page()->this_fsa()->getState(stateIndex);
        }

        if (state != NULL)
        {
            this_page()->this_fsa()->clearHighlightStates();
            state->highlightCircle(true);
            this_page()->this_fsa()->draw();
        }
    }

    delete pageStack;
    pageStack = NULL;

    return stateIndex;
}

//-----------------------------------------------------------------------
// This function clears hightlighted states.
//-----------------------------------------------------------------------
void configuration::clearHighlightStates(void)
{
    if ((this_page()->this_fsa()) != NULL)
    {
        this_page()->this_fsa()->clearHighlightStates();
        this_page()->this_fsa()->draw();
    }
}

//-----------------------------------------------------------------------
// This function checks to see if the current page is the FSA page.
//-----------------------------------------------------------------------
bool configuration::currentPageIsFSA(void)
{
    return (this_page()->is_fsa());
}


///////////////////////////////////////////////////////////////////////
// $Log: configuration.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2007/09/07 23:10:03  endo
// The overlay name is now remembered when the coordinates are picked from an overlay.
//
// Revision 1.2  2007/08/10 15:14:59  endo
// CfgEdit can now save the CBR library via its GUI.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.3  2006/01/10 06:07:39  endo
// AO-FNC Type-I check-in.
//
// Revision 1.2  2005/02/07 22:25:25  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.62  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.61  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.60  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.59  2000/03/31 17:08:46  endo
// reset_waypoint_positioning() deleted since giNumWayPoints
// was replaced with getNumberOfStatesInFSA.
//
// Revision 1.58  2000/03/20 20:39:50  endo
// reset_waypoint_positioning() added in
// the start over function.
//
// Revision 1.57  2000/03/15 17:04:52  sapan
// Added logging calls
//
// Revision 1.56  2000/03/14 00:00:08  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.55  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.54  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.54  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.53  1996/06/04 20:12:56  doug
// *** empty log message ***
//
// Revision 1.52  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.51  1996/05/02  22:58:26  doug
// *** empty log message ***
//
// Revision 1.50  1996/03/06  23:39:17  doug
// *** empty log message ***
//
// Revision 1.49  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.48  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.47  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.46  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.45  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.44  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.43  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.42  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.41  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.40  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.39  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.38  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.37  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
// Revision 1.36  1996/01/19  20:54:42  doug
// *** empty log message ***
//
// Revision 1.35  1996/01/17  18:47:01  doug
// *** empty log message ***
//
// Revision 1.34  1995/12/14  21:53:29  doug
// *** empty log message ***
//
// Revision 1.33  1995/12/05  17:25:29  doug
// *** empty log message ***
//
// Revision 1.32  1995/12/01  21:17:53  doug
// new should now work
//
// Revision 1.31  1995/11/30  23:31:07  doug
// *** empty log message ***
//
// Revision 1.30  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.29  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.28  1995/11/07  14:30:36  doug
// *** empty log message ***
//
// Revision 1.27  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.26  1995/10/09  21:58:06  doug
// *** empty log message ***
//
// Revision 1.26  1995/10/09  21:58:06  doug
// *** empty log message ***
//
// Revision 1.25  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.24  1995/09/15  22:02:20  doug
// added support for the command list in the executive
//
// Revision 1.23  1995/09/07  14:22:26  doug
// works
//
// Revision 1.22  1995/09/01  16:14:56  doug
// Able to run double wander config
//
// Revision 1.21  1995/08/17  22:33:11  doug
// fixed name of config at top level
//
// Revision 1.20  1995/07/06  17:40:00  doug
// When load a configuration that is a group and decide to
// delete the group header, need to also clear the user
// lists in the children since they point to the group record just deleted
//
// Revision 1.19  1995/06/29  17:33:59  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
