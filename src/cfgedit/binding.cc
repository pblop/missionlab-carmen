/**********************************************************************
 **                                                                  **
 **                             binding.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: binding.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <unistd.h>

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


#include <memory.h>
#include <malloc.h>

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
#include "toolbar.h"
#include "verify.h"
#include "cfg_apply.h"

static bool restart_binding = false;


/*-----------------------------------------------------------------------*/

// Called to unbind a actuator or sensor binding
// Returns false to abort the scanning

// bp is the binding point and bp->bound_to is the sensor or actuator record.
// Delete bp->bound_to
static bool
unbind_if_bp(Symbol *bp, Symbol *page_def, Symbol *robot,
	     SymbolList * page_stack, char **message, bool *restart)
{
   if( bp == NULL ||
       bp->symbol_type != BP_NAME  ||
       bp->bound_to == NULL )
   {
      return true;
   }

   Symbol *rec = bp->bound_to;

   if( rec->symbol_type != ROBOT_NAME && 
      (rec->symbol_type != AGENT_NAME || (rec->construction != CS_IBP &&
					  rec->construction != CS_OBP)))
   {
      char buf[256];
      sprintf(buf,"Internal Error: Corrupt hardware record %s",
		rec->description);
      *message = strdup(buf);
      return false;
   }

   // Copy the children list back to the binding point so we don't lose
   // any unconnected agents
   bp->children = rec->children;

   // Copy parameter values down (if they exist in the bp) so don't lose them
   Symbol *bp_p;
   void *bp_cur;
   if ((bp_cur = bp->parameter_list.first(&bp_p)) != NULL)
   {
      do
      {
	 bool moved_it = false;
         Symbol *hdw_p;
         void *hdw_cur;
         if ((hdw_cur = rec->parameter_list.first(&hdw_p)) != NULL)
         {
            do
            {
               if( strcmp(bp_p->name,hdw_p->name) == 0 )
               {
		  // Move the parm down to the bp record
		  if(!bp->parameter_list.replace(bp_p, hdw_p))
		  {
                     char buf[256];
                     sprintf(buf,"Unable to move values down from hardware record %s",
		              rec->description);
                     *message = strdup( buf );
                     return false;
		  }

		  moved_it = true;
		  break;
	       }
            } while ((hdw_cur = rec->parameter_list.next(&hdw_p, hdw_cur)) != NULL);
	 }

	 if( !moved_it )
	 {
            char buf[256];
            sprintf(buf,"Inconsistent Library: Parameter %s exists in the binding point %s but not in the hardware record %s!",
	              bp_p->name, 
		      rec->defining_rec->name,
		      rec->name);
            *message = strdup( buf );
            return false;
         }
      } while ((bp_cur = bp->parameter_list.next(&bp_p, bp_cur)) != NULL);
   }

   // unbind the data record
   bp->bound_to = NULL;

   // Restart, since mucked with the tree.
   *restart = true;
   return false;
}

/*-----------------------------------------------------------------------*/

//NOTE: Need to unbind all I/O points connected to a robot when unbind
//      the robot.

// bp is the binding point and bp->bound_to is the hardware record to delete.
// Returns true if successful
static bool
unbind_bp(Symbol * rec)
{
   if( rec == NULL || 
       rec->symbol_type != BP_NAME  ||
       rec->bound_to == NULL ||
      (rec->bound_to->symbol_type != ROBOT_NAME && 
         (rec->bound_to->symbol_type != AGENT_NAME || 
			(rec->bound_to->construction != CS_IBP &&
		        rec->bound_to->construction != CS_OBP))))
   {
      warn_user("The selected object is not a hardware record\n");
      return false;
   }

   char *message;
   bool restart = false;
   if( unbind_if_bp(rec, NULL, NULL, NULL, &message, &restart) )
   {
      warn_user(message);
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

// Called to unbind the architecture
// Returns true if successful

static bool
unbind_arch()
{
   // Change the scope list
   UseArch(NULL);

   // Reset the editor stuff
   config->unbind_arch();

   // TRICK: Instead of modifying the in-memory configuration,
   //        write it to a file and reload.

   // Write the configuration to a temporary file
   char filename[256];
   sprintf(filename,"/tmp/cfgedit%d.cdl",getpid());
   int good = save_workspace(config->root(), filename, false, false);

   if (!good)
   {
      warn_userf("Error writing temporary file '%s'!", filename);
      return false;
   }

   // Delete the existing configuration
   config->new_design();

   // Reload the config
   if (config->load(filename, false))
   {
      warn_userf("Error reading temporary file '%s'!", filename);
      return false;
   }  

   // delete the temp
   unlink(filename);

   // Mark drawing as changed
   config->made_change();

   return true;
}

/*-----------------------------------------------------------------------*/

// Called to bind the architecture
// Returns true if successful

static bool
bind_arch(void)
{
   config->create_arch_lists();

   // Define the selection dialog structure
   dialog_rec dialog =
   {(DIALOG_BLDR) XmCreateSelectionDialog,
    {
       {XmNdialogTitle, "Select Target Architecture"},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {XmNlistItems, config->arch_names()},
       {XmNlistItemCount, config->num_arches()},
       {XmNmustMatch, true},
       {NULL, 0}
    },
    {
       {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_select_file},
       {NULL, NULL, NULL}
    }
   };

   char *arch_name;
   int ok = wait_SelectionBox(NULL, &dialog, &arch_name);
   if (!ok)
   {
      // User selected abort 
      return false;
   }

   // Reset the editor stuff
   if( !config->bind_arch(arch_name) )
   {
      warn_userf("Unable to bind to architecture: %s", arch_name);
      return false;
   }

   // TRICK: Instead of modifying the in-memory configuration,
   //        write it to a file and reload.

   // Write the configuration to a temporary file
   char filename[256];
   sprintf(filename,"/tmp/cfgedit%d.cdl",getpid());
   int good = save_workspace(config->root(), filename, false, false);

   if (!good)
   {
      warn_userf("Unable to bind to architecture: %s\nError writing temporary file '%s'!", arch_name,filename);
      return false;
   }

   // Delete the existing configuration
   config->new_design();

   // Reload the config
   if (config->load(filename, false))
   {
      warn_userf("Unable to bind to architecture: %s\nError reading temporary file '%s'!", arch_name,filename);
      return false;
   }  

   // delete the temp
   unlink(filename);

   // Mark drawing as changed
   config->made_change();

   return true;
}


/*-----------------------------------------------------------------------*/

//NOTE: When bind a robot, need to check that all enclosed I/O points 
//      correctly match the capabilities of the robot.

// Called when binding IBP & OBP records
// Returns true if successful
static bool
bind_if_bp(Symbol * rec, Symbol *page_def, Symbol *robot,
	   SymbolList * page_stack, char **message, bool *restart)
{
   if (rec == NULL || 
       rec->symbol_type != BP_NAME || 
       rec->bound_to ||
       rec->defining_rec == NULL || 
       rec->defining_rec->symbol_type != BP_CLASS)
   {
      return true;
   }

   SymbolList *recs;
   CS_NAMES construct = rec->defining_rec->construction;

   if (construct == CS_OBP )
   {
      if( robot == NULL )
      {
	 *message = strdup("Internal Error: binding actuators without a robot");
	 return false;
      }
      assert(robot->defining_rec);
      recs = GetRecords(&robot->defining_rec->table, 
			f_symbol_type, c_equal, (void *)ACTUATOR_NAME,
			f_binds_to, c_equal, (void *)rec->defining_rec);
   }
   else if( construct == CS_IBP )
   {
      if( robot == NULL )
      {
	 *message = strdup("Internal Error: binding sensors without a robot");
	 return false;
      }
      assert(robot->defining_rec);
      recs = GetRecords(&robot->defining_rec->table, 
			f_symbol_type, c_equal, (void *)SENSOR_NAME,
			f_binds_to, c_equal, (void *)rec->defining_rec);
   }
   else if( construct == CS_RBP )
   {
      recs = GetRecords(f_symbol_type, c_equal, (void *)ROBOT_CLASS,
			f_binds_to, c_equal, (void *)rec->defining_rec);
   }
   else
   {
      char buf[256];
      sprintf(buf, "Internal Error: unknown binding point contruction: %d",
		  construct);

      *message = strdup( buf );
      return false;
   }

   // Warn if didn't find any matches
   int len = recs->len();

   if (len == 0)
   {
      if (construct == CS_OBP)
	 *message = "Error: There are no matching actuators for this binding point";
      else if (construct == CS_IBP)
	 *message = "Error: There are no matching sensors for this binding point";
      else 
	 *message = "Error: There are no matching robots for this binding point";

      return false;
   }

   Symbol *hdw_def;
   // Now see if there is an obvious binding (only one choice)
   if (len == 1)
   {
      recs->first(&hdw_def);
   }
   // No, so ask the user
   else
   {
      char buf[256];
      sprintf(buf, "Choose %s to bind with %s", 
		construct == CS_OBP ? "actuator" : 
		construct == CS_IBP ? "sensor" : 
		"robot",
		rec->name);

      // Create an array of strings with the choices
      XmString *str = (XmString *) XtMalloc(len * sizeof(XmString));
      assert(str);

      Symbol *p;
      void *cur;           
      int i=0;
      if ((cur = recs->first(&p)) != NULL)
      {        
         do  
         {  
	    str[i++] = XmStringCreateLocalized(p->name);
         } while ((cur = recs->next(&p, cur)) != NULL);
      }              

      // Define the dialog structure
      dialog_rec dialog =
      {(DIALOG_BLDR) XmCreateSelectionDialog,
       {
	  {XmNlistLabelString, buf},
	  {XmNlistItemCount, len},
	  {XmNlistItems, str},
	  {XmNmustMatch, true},
	  {XmNokLabelString, "Ok"},
	  {XmNcancelLabelString, "Abort"},
	  {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
	  {NULL, 0}
       },
       {
	  {XmNhelpCallback, (XtCallbackProc) help_callback,
	   construct == CS_OBP ? 
			(XtPointer) HELP_PAGE_choose_actuator_for_obp : 
	   construct == CS_IBP ?  
			(XtPointer) HELP_PAGE_choose_sensor_for_ibp :
	   		(XtPointer) HELP_PAGE_choose_robot_for_rbp},
	  {NULL, NULL, NULL}
       }
      };

      if( page_stack )
         config->goto_page(page_stack, rec);

      char *selection;
      int ok = wait_SelectionBox(NULL, &dialog, &selection);

      for (i = 0; i < len; i++)
	 XmStringFree(str[i]);
      XtFree((char *) str);

      if (!ok)
      {
	 // User selected abort 
         return false;
      }

      // get the record for the one selected
      if( construct == CS_RBP )
         hdw_def = LookupName(selection);
      else // Sensor or Actuator, so look in robot symbol table
         hdw_def = LookupName(&robot->defining_rec->table,selection);

      if( hdw_def == NULL )
      {
         char buf[256];
         sprintf(buf,"Internal Error: no record matching selection: %s",
		  selection);
         *message = strdup( buf );
         return false;
      }
   }

   // Actually bind the thing
   Symbol *hdw = hdw_def->inst_of();

   // double link it so we can unbind
   rec->bound_to = hdw;
   hdw->bound_to = rec;

   // Copy the location over so we don't jump the glyph
   hdw->location = rec->location;

   // Move any children of the base record up to the hardware record 
   hdw->children = rec->children;
   rec->children.clear();

   // Copy parameter values up so don't lose them
   Symbol *bp_p;
   void *bp_cur;
   if ((bp_cur = rec->parameter_list.first(&bp_p)) != NULL)
   {
      do
      {
	 bool moved_it = false;
         Symbol *hdw_p;
         void *hdw_cur;
         if ((hdw_cur = hdw->parameter_list.first(&hdw_p))!=NULL)
         {
            do
            {
               if( strcmp(bp_p->name,hdw_p->name) == 0 )
               {
		  // Move the bp parm up to the hardware record
		  if(!hdw->parameter_list.replace(hdw_p, bp_p))
		  {
                     char buf[256];
                     sprintf(buf,"Unable to move values up from binding point %s",
		              rec->description);
                     *message = strdup( buf );
                     return false;
		  }

		  // move the old hdw parm down to the bp record
		  if(!rec->parameter_list.replace(bp_p, hdw_p))
		  {
                     char buf[256];
                     sprintf(buf,"Unable to move empty parm back down to binding point from hardware record %s",
		              rec->description);
                     *message = strdup( buf );
                     return false;
		  }
		  moved_it = true;
		  break;
	       }
            } while ((hdw_cur = hdw->parameter_list.next(&hdw_p, hdw_cur)) != NULL);
	 }

	 if( !moved_it )
	 {
            char buf[256];
            sprintf(buf,"Inconsistent Library: Parameter %s exists in the binding point %s but not in the hardware record %s!",
	              bp_p->name, 
		      rec->defining_rec->name,
		      hdw_def->name);
            *message = strdup( buf );
            return false;
         }
      } while ((bp_cur = rec->parameter_list.next(&bp_p, bp_cur)) != NULL);
   }

   // Restart, since mucked with the tree.
   *restart = true;
   return false;
}

/*-----------------------------------------------------------------------*/

static Symbol *robot_candidate = NULL;

// Check if the robot_candidate is consistent with this record
// Returns true if successful
static bool
check_robot(Symbol * rec, Symbol *page_def, Symbol *robot,
	SymbolList * page_stack, char **message, bool *restart)
{
   assert(robot_candidate);

   if (rec == NULL || 
       rec->symbol_type != BP_NAME || 
       rec->bound_to ||
       rec->defining_rec == NULL || 
       rec->defining_rec->symbol_type != BP_CLASS)
   {
      return true;
   }

   CS_NAMES construct = rec->defining_rec->construction;
   if( construct == CS_RBP )
   {
      // Weird error: just hit a robot binding point
      *message = "Internal Error: Found robot binding point in check_robot";
      return false;
   }

   SymbolList *recs;

   if (construct == CS_OBP )
   {
      recs = GetRecords(&robot_candidate->table, 
			f_symbol_type, c_equal, (void *)ACTUATOR_NAME,
			f_binds_to, c_equal, (void *)rec->defining_rec);
   }
   else if( construct == CS_IBP )
   {
      recs = GetRecords(&robot_candidate->table, 
			f_symbol_type, c_equal, (void *)SENSOR_NAME,
			f_binds_to, c_equal, (void *)rec->defining_rec);
   }

   // if didn't find any matches, then it is not consistent
   if( recs->len() == 0 )
   {
      *message = NULL;
      return false;
   }

   // Is consistent
   return true;
}

/*-----------------------------------------------------------------------*/

// Try to insert any needed robots
// Returns true if successful
static bool
insert_robot(Symbol * rec, Symbol *page_def, Symbol *robot,
           SymbolList * page_stack, char **message, bool *restart)
{
   if (rec == NULL || 
       rec->symbol_type != BP_NAME || 
       rec->bound_to ||
       rec->defining_rec == NULL || 
       rec->defining_rec->symbol_type != BP_CLASS)
   {
      return true;
   }

   CS_NAMES construct = rec->defining_rec->construction;

   if( (construct == CS_OBP || construct == CS_IBP) && robot == NULL)
   {
      // Get the list of all robots 
      SymbolList *recs = GetRecords(f_symbol_type, c_equal,(void *)ROBOT_CLASS);

      // Prune it to just the list of robots that are consistent with the 
      // input and output binding points in this subtree.
      void   *cur;
      Symbol *r;
      SymbolList consistent_robots;
      if( (cur = recs->first(&r)) != NULL )
      {
	 do
	 {
	    // Set the global to this proposed robot binding point
	    robot_candidate = r;

            // Check if works
            if( cfg_apply(page_def, check_robot) )
	    {
	       // Yes, remember it.
	       consistent_robots.append(r);
	    }
         } while( (cur = recs->next(&r,cur)) != NULL);
      }
      // Clear the global.
      robot_candidate = NULL;

      // Warn if didn't find any matches
      int len = consistent_robots.len();

      if (len == 0)
      {
	 *message = "Unable to find a robot binding point which is consistent with this subtree.\nYou will either need to add a new class of robots to the library or choose\ndifferent Input/Output binding points to remove the conflict.";
         return false;
      }

      Symbol *robot_rec;
      // Now see if there is an obvious binding (only one choice)
      if (len == 1)
      {
         consistent_robots.first(&robot_rec);
      }
      // No, so ask the user
      else
      {
         char buf[256];
         sprintf(buf,"Choose one of these robots for this subtree.");

         // Create an array of strings with the choices
         XmString *str = (XmString *) XtMalloc(len * sizeof(XmString));
         assert(str);

         Symbol *p;
         void *cur;           
         int i=0;
         if ((cur = consistent_robots.first(&p)) != NULL)
         {        
            do  
            {  
	       str[i++] = XmStringCreateLocalized(p->name);
            } while ((cur = consistent_robots.next(&p, cur)) != NULL);
         }              
   
         // Define the dialog structure
         dialog_rec dialog =
         {(DIALOG_BLDR) XmCreateSelectionDialog,
          {
	     {XmNlistLabelString, buf},
	     {XmNlistItemCount, len},
	     {XmNlistItems, str},
	     {XmNmustMatch, true},
	     {XmNokLabelString, "Ok"},
	     {XmNcancelLabelString, "Abort"},
	     {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
	     {NULL, 0}
          },
          {
	     {XmNhelpCallback, (XtCallbackProc) help_callback,
	   		           (XtPointer) HELP_PAGE_choose_robot_for_rbp},
	     {NULL, NULL, NULL}
          }
         };

         if( page_stack )
            config->goto_page(page_stack, rec);

         char *selection;
         int ok = wait_SelectionBox(NULL, &dialog, &selection);

         for (i = 0; i < len; i++)
	    XmStringFree(str[i]);
         XtFree((char *) str);

         if (!ok)
         {
	    // User selected abort 
            return false;
         }

         // get the record for the one selected
         robot_rec = LookupName(selection);
   
         if( robot_rec == NULL )
         {
            char buf[256];
            sprintf(buf,"Internal Error: no record matching selection: %s",
		     selection);
            *message = strdup( buf );
            return false;
         }
      }

      // Actually insert the robot record by creating it as a new umbrella 
      // object that encloses this page
//fprintf(stderr,"Inserting an instance of the %s robot\n",robot_rec->name);

      // Make the RBP record
      assert(robot_rec->binds_to);
      Symbol *rbp = robot_rec->binds_to->inst_of();

      // Make the robot record
      Symbol *robot = robot_rec->inst_of();

      // Bind them
      rbp->bound_to = robot;
      robot->bound_to = rbp;
    
      // The children list are what this page had
      robot->children = page_def->children;

      // The configuration node now just has the rbp as a child
      page_def->children.clear();
      page_def->children.append(rbp);

      // Now modify the robot's new children so they point to it
      // as their user instead of the old page def.
      Symbol *p;
      if ((cur = robot->children.first(&p)) != NULL)
      {
         do
         {
            p->users.remove(page_def);
            p->users.append(robot);
         } while ((cur = robot->children.next(&p, cur)) != NULL);
      }  

      // Probably should restart the scan when add a record
      *restart = true;
      return false;
   }


   return true;
}

/*-----------------------------------------------------------------------*/

// Called when binding IBP & OBP records
// Returns true if successful
static bool
bind_bp(Symbol * rec)
{
   if (rec == NULL || rec->symbol_type != BP_NAME || 
       rec->defining_rec == NULL || rec->defining_rec->symbol_type != BP_CLASS)
   {
      warn_user("The selected object is not a binding point\n");
      return false;
   }

   char *message = NULL;
   bool rtn;
   do
   {
      restart_binding = false;
      rtn = bind_if_bp(rec, NULL, NULL, NULL, &message, &restart_binding);
   } while( restart_binding );

   if( !rtn )
   {
      if(message != NULL)
      	warn_user(message);
      return false;
   }

   return true;
}

/*-----------------------------------------------------------------------*/

/* called when binding button is clicked in glyph */
void
bind_cb(void)
{
   // Make sure the configuration is consistent before we bind.
   if( !verify(false, false) )
      return;

   bool changed = false;

   if( !config->arch_is_bound() )
   {
      if( !bind_arch() )
	 return;
      changed = true;
   }

   if( selected )
   {
      // If there is a selection, only bind it
      if( bind_bp(selected) )
         changed = true;
   }
   else
   {
      // Add any missing robot binding points
      if( !cfg_apply(config->root(), insert_robot) )
	 return;

      // Bind the configuration.
      if( cfg_apply(config->root(), bind_if_bp) )
         return;

      warn_user("Configuration bound successfully");
      changed = true;
   }

   if( changed )
   {
      // Regenerate the drawing to get the changes
      config->regen_page();

      // Mark drawing as changed
      config->made_change();

      // Reset the placement menu to enable the allowed components
      reset_placement_list();
   }
}

/*-----------------------------------------------------------------------*/

/* called when unbind button is clicked in hardware record glyph */
void
unbind_cb(void)
{
   if( !config->arch_is_bound() )
      return;

   // Don't check or warn for an empty configuration
   bool is_empty = config->root()->children.len() == 0;
   if( !is_empty )
   {
      // Make sure the configuration is consistent before we unbind.
      if( !verify(false, false) )
         return;

      // Define the unbind_arch dialog structure
      dialog_rec dialog =
         {(DIALOG_BLDR) XmCreateQuestionDialog,
	     {
	        {XmNmessageString, "Caution: Unbinding will lose any architecture specific information!"},
	        {XmNokLabelString, "Unbind"},
	        {XmNcancelLabelString, "Cancel"},
	        {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
	        {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
	        {NULL, 0}
	     },
	     {
	        {XmNhelpCallback, (XtCallbackProc) help_callback,
	         (XtPointer) HELP_PAGE_unbind_arch},
	        {NULL, NULL, NULL}}
         };

      if (!wait_dialog_box(NULL, &dialog, NULL))
         return;

   } // End if not empty configuration

   // got the OK to unbind
   bool changed = false;

   if( selected )
   {
      // If there are selections, only unbind those

      // Want to deal with the binding point.
      // the selected list stacks the hardware record instead.
      Symbol *rec = selected;
      if( rec && rec->bound_to )
	 rec = rec->bound_to;

      if( unbind_bp(rec) )
         changed = true;
   }
   else
   {
      // Need to unbind any hardware records.
      bool rtn;
      do
      {
         restart_binding = false;
         rtn = cfg_apply(config->root(), unbind_if_bp);
      } while( restart_binding );

      if( !rtn )
         return;

      if( unbind_arch() )
	 warn_user("Configuration unbound successfully");

      changed = true;
   }

   if( changed )
   {
      // Regenerate the drawing to get the changes
      config->regen_page();

      // Mark drawing as changed
      config->made_change();

      // Reset the placement menu to enable the allowed components
      reset_placement_list();
   }
}


///////////////////////////////////////////////////////////////////////
// $Log: binding.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
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
// Revision 1.39  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.38  2000/02/10 04:42:01  endo
// Checked in for Douglas C. MacKenzie.
// He got rid of warning signs upon compilation.
//
// Revision 1.37  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.36  1996/12/06  16:15:23  doug
// respelled loose as lose
//
// Revision 1.35  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.35  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.34  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.33  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.32  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.31  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.30  1996/02/18  23:31:19  doug
// binding is working!!!!
//
// Revision 1.29  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.28  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.26  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.25  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.24  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.23  1996/01/17  18:47:01  doug
// *** empty log message ***
//
// Revision 1.22  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.21  1995/11/21  23:10:08  doug
// *** empty log message ***
//
// Revision 1.20  1995/11/08  16:50:02  doug
// *** empty log message ***
//
// Revision 1.19  1995/10/09  21:58:06  doug
// *** empty log message ***
//
// Revision 1.18  1995/09/26  21:42:41  doug
// make so doesn't jump when bind
//
// Revision 1.17  1995/09/26  17:58:06  doug
// make so doesn't jump when bind
//
// Revision 1.16  1995/06/29  16:22:53  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
