/**********************************************************************
 **                                                                  **
 **                    library_manager.cc                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: library_manager.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

#include <Xm/Xm.h>

#include <Xm/PushBG.h>
#include <Xm/ToggleBG.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

#include <stdarg.h>

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


static Widget library_manager = NULL;
static Symbol *selected_library = NULL;
static Symbol *marked_library = NULL;
static bool canceled = false;
static bool done = false;

/*-----------------------------------------------------------------------*/

static void 
lower_library_manager(Widget w, bool was_ok)
{
   if( library_manager != NULL )
      XtPopdown(library_manager);

   if( was_ok )
   {
      if( marked_library )
         selected_library = marked_library;
   }
   else
   {
      canceled = true;
   }

   done = true;
}

/*-----------------------------------------------------------------------*/

static void 
toggled(Widget w, int which, XmToggleButtonCallbackStruct *state)
{
   if( state->set )
   {
      Symbol *lib;
      void *pos;
      int cnt = 0;
      if( (pos = loaded_libraries.first(&lib)) != NULL )
      {
         do
         {
	    if( cnt == which )
	       break;
	    cnt ++;
         } while( (pos = loaded_libraries.next(&lib,pos)) != NULL );
      }
   
      if( cnt != which )
      {
         warn_user("Unable to find library record to display");
         return;
      }

      selected_library = lib;
cerr << "selected library" << selected_library->name << "\n";
   }
}

/*-----------------------------------------------------------------------*/

static void 
create_library_manager()
{
   Dimension win_x, win_y;
   XtVaGetValues(top_level, 
	XmNx, &win_x,
	XmNy, &win_y,
	NULL);

   // Create the base shell
   library_manager = XtVaCreatePopupShell(
      "Library Manager", topLevelShellWidgetClass, top_level,
      XmNdeleteResponse, 	XmUNMAP,
      XmNx,			win_x + 100,
      XmNy,			win_y + 100,
      NULL);

   // create a paned window to manage the stuff
   Widget pane = XtVaCreateWidget("pane", xmPanedWindowWidgetClass, library_manager,
      NULL);

   // create a form to hold the display area
   Widget aform = XtVaCreateWidget("form", xmFormWidgetClass, pane,
      NULL);

   XmString string = XmStringCreateLocalized("Select the library to be modified:");
   Widget label = XtVaCreateManagedWidget("", xmLabelGadgetClass, aform,
      XmNtopAttachment,				XmATTACH_FORM,
      XmNtopOffset,				8,
      XmNleftAttachment,			XmATTACH_FORM,
      XmNlabelString,				string,
      NULL);
   XmStringFree(string);

   Arg args[8];
   int arg=0;
   XtSetArg(args[arg], XmNtopAttachment, XmATTACH_WIDGET); arg++;
   XtSetArg(args[arg], XmNtopWidget, label); arg++;
   XtSetArg(args[arg], XmNleftAttachment, XmATTACH_FORM); arg++;
   XtSetArg(args[arg], XmNleftOffset, 8); arg++;
   XtSetArg(args[arg], XmNborderWidth, 0); arg++;

   Widget radio_box = XmCreateRadioBox(aform, "libButtons", args, arg);

   Symbol *lib;
   void *pos;
   int cnt = 0;
   if( (pos = loaded_libraries.first(&lib)) != NULL )
   {
      do
      {
         Widget button = XtVaCreateManagedWidget(lib->name, xmToggleButtonGadgetClass, radio_box,
            NULL);
         XtAddCallback(button, XmNvalueChangedCallback, (XtCallbackProc)toggled, (void *)cnt++);

      } while( (pos = loaded_libraries.next(&lib,pos)) != NULL );
   }

   XtManageChild(radio_box);
   XtManageChild(aform);


   // create a form to hold the buttons
   Widget form = XtVaCreateWidget("form", xmFormWidgetClass, pane,
      XmNfractionBase,		5,
      NULL);

   // Create the OK button to exit
   Widget ok = XtVaCreateManagedWidget("Ok", xmPushButtonGadgetClass, form,
      XmNtopAttachment,				XmATTACH_FORM,
      XmNbottomAttachment,			XmATTACH_FORM,
      XmNleftAttachment,			XmATTACH_POSITION,
      XmNleftPosition,				1,
      XmNrightAttachment,			XmATTACH_POSITION,
      XmNrightPosition,				2,
      XmNshowAsDefault,				True,
      XmNdefaultButtonShadowThickness,		1,
      NULL);
   XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)lower_library_manager, (void *)true);

   // Create the Cancel button to exit
   Widget cancel = XtVaCreateManagedWidget("Cancel", xmPushButtonGadgetClass, form,
      XmNtopAttachment,				XmATTACH_FORM,
      XmNbottomAttachment,			XmATTACH_FORM,
      XmNleftAttachment,			XmATTACH_POSITION,
      XmNleftPosition,				2,
      XmNrightAttachment,			XmATTACH_POSITION,
      XmNrightPosition,				3,
      XmNdefaultButtonShadowThickness,		1,
      NULL);
   XtAddCallback(cancel, XmNactivateCallback, (XtCallbackProc)lower_library_manager, (void *)false);

   // Manage the action area form
   XtManageChild(form);
   XtManageChild(pane);
}

/*-----------------------------------------------------------------------*/

void 
raise_library_manager()
{
   done = false;

   if( library_manager == NULL )
      create_library_manager();

   // Make sure it is visible
   XtPopup(library_manager,XtGrabNone);

   XtVaSetValues(library_manager, XmNiconic, False, NULL);
   marked_library = NULL;
}

/*-----------------------------------------------------------------------*/

static void 
write_arch_lib_file(Symbol *the_lib, Symbol *the_arch, const bool debug)
{
   // Write the file for the specified architectur and library.
   Symbol *library_rec;
   void *cur = the_lib->children.first(&library_rec);
   bool found_one = false;
   while( cur )
   {
      if( library_rec->arch == the_arch )
      {
         // Get a list of all the symbols defined from this source file
         SymbolList *librecs;
         librecs = GetLibraryRecords(library_rec->arch, library_rec->name);

         // Write them to a new file
         char *filename = library_rec->name;

cerr << "Writing library file: " << filename << "\n";

         bool success = write_library(librecs,filename,debug,backup_files,
		library_rec->name);

         if( !success )
         {
            warn_user("Unable to write library");
	    return;
         }

         found_one = true;
	 break;
      }

      cur = the_lib->children.next(&library_rec, cur);
   }

   if( !found_one )
   {
      warn_user("The library file for this architecture is empty!");
      return;
   }
}

/*-----------------------------------------------------------------------*/

static char *name;

static void
done_entering_name(Widget w,
		caddr_t client_data,
		XmFileSelectionBoxCallbackStruct * fcb)
{
   done = true;

   /* unexpose the file selection dialog */
   XtUnmanageChild(w);

   XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET, &name);
}


/*-----------------------------------------------------------------------*/

void 
make_library_component_from_selected()
{
   const bool debug = false;

   // Define the new_design dialog structure
   dialog_rec name_dialog =
   {(DIALOG_BLDR) XmCreatePromptDialog,
    {
       {XmNselectionLabelString, "Enter a name for the new component"},
       {XmNokLabelString, "Done"},
       {XmNcancelLabelString, "Cancel"},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {NULL, 0}
    },
    {
      {XmNokCallback,(XtCallbackProc)done_entering_name,(XtPointer)PASS_WIDGET},
      {XmNhelpCallback,(XtCallbackProc) help_callback,(XtPointer) HELP_PAGE_save_as},
      {NULL, NULL, NULL}
    }
   };

   done = false;
   open_dialog_box(NULL, &name_dialog, NULL);
   while( !done )
   {
      XtAppProcessEvent(app, XtIMAll);
   }

   if( canceled || name == NULL || name[0] == '\0')
   {
      return;
   }

   if( choosen_architecture != free_arch && !is_defined_in(name, "free") )
   {
      warn_user("You must first define the generic component before adding architecture specific versions!\n");
      return;
   }

   raise_library_manager();

   while( !done )
   {
      XtAppProcessEvent(app, XtIMAll);
   }

   if( canceled )
   {
      return;
   }

   assert(selected);
   selected->name = name;

   // Add the component into the library symbol table
   DefineNameInArch((Symbol *)choosen_architecture, selected); 

#if 0
   // Add the component into the library symbol table
   Symbol *rec = DefineName(selected);
//   Symbol *rec = DefineNameInArch((Symbol *)choosen_architecture, selected); 

   if( rec != NULL && rec != selected )
   {
      char buf[2048];
      sprintf(buf,"Replace the existing %s component?", selected->name);

      // Define the exit_design dialog structure
      dialog_rec dialog =
      {(DIALOG_BLDR) XmCreateQuestionDialog,
       {
          {XmNmessageString, buf},
          {XmNokLabelString, "Yes"},
          {XmNcancelLabelString, "Cancel"},
          {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
          {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
          {NULL, 0}
       },
       {
          {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_workspace},
          {NULL, NULL, NULL}}
      };

      if (!wait_dialog_box(NULL, &dialog, NULL))
         return;

      if( !RemoveName(rec, false) )
      {
         warn_user("Error removing existing record!");
         return;
      }

      if( DefineName(selected) )
      {
         warn_user("Error adding new record!");
         return;
      }
   }
#endif

   // Build the source name for the library file
   char buf[2048];
   if( choosen_architecture == free_arch )
   {
      // Due to an oversight, the generic files end in ".gen" and
      // the name of that architecture is "free".  AuRA and UGV match.
      sprintf(buf,"%s.gen",selected_library->name);
   }
   else
   {
      sprintf(buf,"%s.%s",selected_library->name, choosen_architecture->name);
   }

   // Mark the selected component's nodes as in the library "selected_library".
   selected->move_tree_into_library(buf, debug);  

   // OK, write the new library file
   write_arch_lib_file(selected_library, (Symbol *)choosen_architecture, debug);

   warn_userf("Successfully added component %s to the library", selected->name);
}

/*-----------------------------------------------------------------------*/

void 
remove_library_component(void)
{
   const bool debug = false;

   raise_library_manager();

   while( !done )
   {
      XtAppProcessEvent(app, XtIMAll);
   }

   if( canceled )
   {
      return;
   }

   Symbol *agent = pick_behavior_for_delete(PG_AGENTS);
   if( agent == NULL )
   {
      return;
   }

   if( !RemoveFromLibrary(agent, (Symbol *)choosen_architecture) )
   {
      warn_userf("Error removing component %s from the library", agent->name);
      return;
   }

   // Build the source name for the library file
   char buf[2048];
   if( choosen_architecture == free_arch )
   {
      // Due to an oversight, the generic files end in ".gen" and
      // the name of that architecture is "free".  AuRA and UGV match.
      sprintf(buf,"%s.gen",selected_library->name);
   }
   else
   {
      sprintf(buf,"%s.%s",selected_library->name, choosen_architecture->name);
   }

   // OK, write the new library file
   write_arch_lib_file(selected_library, (Symbol *)choosen_architecture, debug);

   warn_userf("Successfully removed component %s from the library",agent->name);
}

///////////////////////////////////////////////////////////////////////
// $Log: library_manager.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.10  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.9  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.8  1996/10/09  16:51:10  doug
// remove unused variable
//
// Revision 1.7  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.8  1996/10/01 17:57:54  doug
// add library component works
//
// Revision 1.7  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.6  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.5  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.4  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.3  1995/11/29  23:13:28  doug
// *** empty log message ***
//
// Revision 1.2  1995/11/27  23:31:53  doug
// *** empty log message ***
//
// Revision 1.1  1995/11/27  21:58:50  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
