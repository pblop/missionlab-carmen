/**********************************************************************
 **                                                                  **
 **                         import_symbol.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 2000 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: import_symbol.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MessageB.h>

#include "symbol.hpp"
#include "symtab.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "popups.h"
#include "help.h"
#include "import_symbol.h"
#include "EventLogging.h"
#include "toolbar.h"

//**********************************************************************
// called to create a duplicate of a read-only symbol in user space.
void
import_symbol(screen_object *s)
{
   char msg[256];

   if (gDisableImportSymbol)
   {
       warn_userf("This object is Read-Only");
       return;
   }

   sprintf(msg, "This object is a library read-only component.\nDo you want to import it so you can modify it?");

   // Define the new_design dialog structure
   dialog_rec import_ro_object_dialog =
   {(DIALOG_BLDR) XmCreateQuestionDialog,
   {
       {XmNmessageString, msg},
       {XmNokLabelString, "Import"},
       {XmNcancelLabelString, "Cancel"},
       {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {NULL, 0}
   },
   {
      {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_file},
      {NULL, NULL, NULL}}
   };

   gEventLogging->start("Do you want to import it so you can modify it?");

   if (wait_dialog_box(NULL, &import_ro_object_dialog, NULL))
   {
       gEventLogging->log("Do you want to import it so you can modify it? = Import");
       gEventLogging->end("Do you want to import it so you can modify it?");
   }
   else
   {
       gEventLogging->log("Do you want to import it so you can modify it? = Cancel");
       gEventLogging->end("Do you want to import it so you can modify it?");
       return;
   }
   // Got the OK to import it, so do it.
   s->import_object( );

   save_cdl_for_replay("object_imported");

   // Redraw the page.  We mucked it up pretty good.
   config->regen_page();
}

//**********************************************************************
