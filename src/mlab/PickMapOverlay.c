/**********************************************************************
 **                                                                  **
 **                       PickMapOverlay.cc                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 1996, 1997, Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: PickMapOverlay.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Form.h>
#include <Xm/SelectioB.h>
#include <Xm/PushBG.h>
#include <Xm/PushB.h>
#include <Xm/List.h>

#include "utilities.h"
#include "console.h"
#include "PickMapOverlay.h"
#include "cfgedit_common.h"

struct ok_cb_struct 
{
   Widget         shell;
   Widget         list;
};

static bool done;
static int  position;

extern XtAppContext app;
extern symbol_table<rc_chain> rc_table;

/*-----------------------------------------------------------------------*/

static void
Ok_cb(Widget w, ok_cb_struct *data)
{
   position = -1;
   if( data == NULL )
   {
      warn_user("Internal Error: NULL data pointer in Ok_cb\n");
   }
   else
   {
      int *pos;
      int pos_cnt;
      if( XmListGetSelectedPos(data->list, &pos, &pos_cnt) )
      {
         if (pos_cnt != 1)
         {
            warn_userf("Internal Error: invalid number of selections in Ok_cb: %d",
		   pos_cnt);
         }
	 else
	 {
            // Notice that the position returned is 1 based, convert to 0 based.
            position = pos[0] - 1;
	 }
      }
      else
      {
         warn_user("Internal Error: no item selected in Ok_cb");
      }
   }

   // Kill us
   XtDestroyWidget(data->shell);

   done = true;
}


/*-----------------------------------------------------------------------*/

static void
cancel_cb(Widget w, Widget shell)
{
   XtDestroyWidget(shell);
   position = -1;
   done = true;
}

/*-----------------------------------------------------------------------*/

// When return is pressed, respond by getting the default button and 
// activating it.
static void
rtn_cb(Widget w, Widget form, XmAnyCallbackStruct *cbs)
{
   Widget dflt = NULL;
   XtVaGetValues(form, XmNdefaultButton, &dflt, NULL);
   if( dflt )
   {
      // Make the button think it was pushed by the user.
      // We borrow the event field from our callback struct.
      XtCallActionProc(dflt, "ArmAndActivate", cbs->event, NULL, 0);
   }
   else
   {
      warn_user("Internal Error: Unable to get default button in rtn_cb\n");
   }
}

/*-----------------------------------------------------------------------*/

char *
PickMapOverlay()
{
   char **names;

   // Default is to look in this directory
   char *dir = ".";

   // If there is a resource variable set, look there instead.
   const rc_chain *dirs = rc_table.get(OVERLAY_FILES);

   char *str;
   if( dirs && dirs->first(&str) )
      dir = str;

   // Get the list of overlay files available.
   int cnt = build_dir_list(dir, ".ovl", &names);
   if( cnt == 0 )
      return strdup("marc.ovl");

   // Build the list of Xstrings
   XmString *strings = (XmString *)XtMalloc(cnt * sizeof(XmString));

   // Create the strings
   for(int i=0; i<cnt; i++)
      strings[i] = XmStringCreateLocalized( names[i] );

   char *title = "Select the desired environment";

   Widget dialog = XtVaCreatePopupShell("PickMapOverlay",
	xmDialogShellWidgetClass, main_window, 
	XmNtitle, title,
	XmNx, 200,
	XmNy, 200,
	NULL);

   Widget pane = XtVaCreateWidget("pane", xmPanedWindowWidgetClass, dialog,
	XmNsashWidth, 1,
	XmNsashHeight, 1,
	NULL);

   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   Arg args[16];
   int argcount = 0;
   int vis_cnt = cnt;
   if( vis_cnt > 12 )
      vis_cnt = 12;

   XtSetArg(args[argcount], XmNselectionPolicy, XmBROWSE_SELECT); argcount++;
   XtSetArg(args[argcount], XmNleftAttachment, XmATTACH_FORM); argcount++;
   XtSetArg(args[argcount], XmNrightAttachment, XmATTACH_FORM); argcount++;
   XtSetArg(args[argcount], XmNtopAttachment, XmATTACH_FORM); argcount++;
   XtSetArg(args[argcount], XmNvisibleItemCount, vis_cnt); argcount++;
   XtSetArg(args[argcount], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);argcount++;
   XtSetArg(args[argcount], XmNitemCount, cnt); argcount++;
   XtSetArg(args[argcount], XmNitems, (int)strings); argcount++;

   // Create agent list 
   Widget list = XmCreateScrolledList(pane, "", args, argcount);

   // Free the strings
   for(int i=0; i<cnt; i++)
      XmStringFree(strings[i]);

   // Select the current value.
   int pos = 1;
   XmListSelectPos(list, pos, false);

   XtManageChild(list);

   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   // Create the action area
   Widget form2 = XtVaCreateWidget("", xmFormWidgetClass, pane, 
        XmNfractionBase, 5,
	NULL);

   // Add a callback to the list window that handles return key.
   XtAddCallback(list, XmNdefaultActionCallback,(XtCallbackProc)rtn_cb,(XtPointer)form2);
   
   // Create the OK button.
   Widget ok = XtVaCreateManagedWidget("Ok", xmPushButtonWidgetClass, form2,
	XmNtopAttachment,	XmATTACH_FORM,
	XmNbottomAttachment,	XmATTACH_FORM,
	XmNleftAttachment,	XmATTACH_POSITION,
	XmNleftPosition,	1,
	XmNrightAttachment,	XmATTACH_POSITION,
	XmNrightPosition,	2,
	XmNshowAsDefault,	True,
	XmNdefaultButtonShadowThickness, 1,
	NULL);

   ok_cb_struct *data = new ok_cb_struct;
   data->shell = dialog;
   data->list = list;
   XtAddCallback(ok, XmNactivateCallback,(XtCallbackProc)Ok_cb,(XtPointer)data);

   // Create the Cancel button.
   Widget cancel = XtVaCreateManagedWidget("Cancel", 
	xmPushButtonGadgetClass, form2,
	XmNtopAttachment,	XmATTACH_FORM,
	XmNbottomAttachment,	XmATTACH_FORM,
	XmNleftAttachment,	XmATTACH_POSITION,
	XmNleftPosition,	3,
	XmNrightAttachment,	XmATTACH_POSITION,
	XmNrightPosition,	4,
	XmNshowAsDefault,	False,
	XmNdefaultButtonShadowThickness, 1,
	NULL);
   XtAddCallback(cancel, XmNactivateCallback, (XtCallbackProc)cancel_cb,dialog);
   XtManageChild(form2);

   // Fix the height of the action area
   Dimension h;
   XtVaGetValues(cancel, XmNheight, &h, NULL);
   XtVaSetValues(form2, 
	XmNdefaultButton, ok, 
	NULL);

   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

   XtManageChild(pane);

   XtPopup(dialog, XtGrabExclusive);

   /* Wait for the answer */
   done = false;
   while( !done )
   {
      XtAppProcessEvent(app, XtIMAll);
   }

   if( position == -1 )
      return NULL;

   char buf[2048];
   sprintf(buf,"%s/%s",dir,names[position]);
   return strdup(buf);
}


///////////////////////////////////////////////////////////////////////
// $Log: PickMapOverlay.c,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:08  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.6  1999/10/04 05:12:12  bbb
// Previously running mlab without a .cfgeditrc and then pulling
// down from File to Refresh would core dump the program. It was a
// simple problem of not checking for a null pointer.
//
// Revision 1.5  1998/12/04 21:42:12  liston
// Removed the two lines which caused a scrolled list to have
// zero area.
//
// Revision 1.4  1997/02/12 05:42:50  zchen
// *** empty log message ***
//
// Revision 1.3  1996/05/08  15:42:54  doug
// fixing warnings
//
// Revision 1.2  1996/03/01  00:47:09  doug
// *** empty log message ***
//
// Revision 1.1  1996/02/29  15:29:39  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
