/**********************************************************************
 **                                                                  **
 **                            PickAgent.cc                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: PickAgent.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


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

#include "load_cdl.h"
#include "globals.h"
#include "configuration.hpp"
#include "screen_object.hpp"
#include "fsa.hpp"
#include "transition.hpp"
#include "circle.hpp"
#include "popups.h"
#include "EventLogging.h"
#include "ConstructPrintName.h"
#include "PickAgent.h"
#include "toolbar.h"
#include "mission_expert.h"

struct ok_cb_struct 
{
   int            kind;
   screen_object *s;
   Widget         shell;
   Widget         list;
};

static bool done;

/*-----------------------------------------------------------------------*/
static void
Ok_cb(Widget w, ok_cb_struct *data)
{
    //USE(w);

    if(data == NULL)
    {
        warn_user("Internal Error: NULL data pointer in Ok_cb\n");
    }
    else if(data->s == NULL)
    {
        warn_user("Internal Error: NULL object pointer in Ok_cb\n");
    }
    else
    {
        int *pos;
        int pos_cnt;
        if(XmListGetSelectedPos(data->list, &pos, &pos_cnt))
        {
            if (pos_cnt != 1)
            {
                warn_userf("Internal Error: invalid number of selections in Ok_cb: %d",
                           pos_cnt);
            }
            else
            {
                // Notice that the position returned is 1 based.
                Symbol *cur = config->behavior_by_pos(data->kind, pos[0]-1);
                if (cur == NULL)
                {
                    warn_user("Internal Error: Agent not found in library in Ok_cb");
                }
                else
                {

                    // Checking to see if this original screen_object has
                    // "*" at the end of the name. If it is, let's delete the state,
                    // and create a new one, in order to get rid of the "TaskExited"
                    // triggers coming out of it.
                    char name_str[4096];
                    sprintf(name_str, "%s", ObjectName(data->s->get_subtree()));
                    if (name_str[strlen(name_str)-1] == '*')
                        data->s = cutTaskExitedTransition(data->s);		   

                    // Setting subtree according to the new screen_object.
                    data->s->set_subtree(cur);

                    // Now, checking to see if the new screen_object has
                    // "*" at the end of the name. If it is, it create the
                    // "TaskExited" triggers.
                    sprintf(name_str, "%s", ObjectName(data->s->get_subtree()));
                    if (name_str[strlen(name_str)-1] == '*')
                        TaskExited_design(data->s, cur);
      
                    // Regenerate the drawing to get the changes
                    config->regen_page();
   
                    // Mark drawing as changed
                    config->made_change();

                    if (gMExp->isEnabled())
                    {
                        gMExp->madeChange();
                    }

                    gEventLogging->endModify("Agent \"%s\"", cur->name);
                }
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
Cancel_cb(Widget w, Widget shell)
{
    //USE(w);

    XtDestroyWidget(shell);
    done = true;
    gEventLogging->cancel("PickAgent");
}

/*-----------------------------------------------------------------------*/

// When return is pressed, respond by getting the default button and 
// activating it.
static void
rtn_cb(Widget w, Widget form, XmAnyCallbackStruct *cbs)
{
    //USE(w);

    Widget dflt = NULL;
    XtVaGetValues(form, XmNdefaultButton, &dflt, NULL);
    if(dflt)
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

void
PickAgent(screen_object *s)
{
    if(s == NULL)
    {
        warn_user("NULL object to PickAgent");
        return;
    }

    if(s->is_state())
    {
        circle *c = (circle *)s;
        if(c->is_start())
        {
            warn_user("The start state can not be modified");
            gEventLogging->cancel("PickAgent");
            return;
        }

        Symbol *p = s->get_src_sym();
        if(p && p->list_index && p->list_index->description)
        {
            gEventLogging->startModify("Agent %s \"%s\"", 
                                       p->list_index->description, 
                                       ObjectName(s->get_subtree()));
        }
        else
        {
            gEventLogging->startModify("Agent State??? \"%s\"",
                                       ObjectName(s->get_subtree()));
        }
    }
    else
    {
        Symbol *p = s->get_subtree();
        if(p && p->description)
        {
            gEventLogging->startModify("Agent %s \"%s\"", 
                                       p->description, 
                                       ObjectName(s->get_subtree()));
        }
        else
        {
            gEventLogging->startModify("Agent Trans??? \"%s\"",
                                       ObjectName(s->get_subtree()));
        }
    }

    int kind = PG_TRIGS;
    if(s->is_state())
        kind = PG_AGENTS;

    char *title = "Select New Task";

    Widget dialog = XtVaCreatePopupShell("PickAgent",
                                         xmDialogShellWidgetClass, main_window, 
                                         XmNtitle, title,
                                         XmNx, 100,
                                         XmNy, 100,
                                         NULL);

    Widget pane = XtVaCreateWidget("pane", xmPanedWindowWidgetClass, dialog,
                                   XmNsashWidth, 1,
                                   XmNsashHeight, 1,
                                   NULL);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    int n = config->num_behaviors(kind);
    if(n < 1)
    {
        warn_user("Internal Error: No choices for this item\n");
        gEventLogging->cancel("PickAgent");
        return;
    }

    XmString *names = config->behavior_names(kind);

    Arg args[16];
    int argcount = 0;

    XtSetArg(args[argcount], XmNselectionPolicy, XmBROWSE_SELECT); argcount++;
    XtSetArg(args[argcount], XmNleftAttachment, XmATTACH_FORM); argcount++;
    XtSetArg(args[argcount], XmNrightAttachment, XmATTACH_FORM); argcount++;
    XtSetArg(args[argcount], XmNtopAttachment, XmATTACH_FORM); argcount++;
    XtSetArg(args[argcount], XmNvisibleItemCount, min(20,n)); argcount++;
    XtSetArg(args[argcount], XmNlistSizePolicy, XmRESIZE_IF_POSSIBLE);argcount++;
    XtSetArg(args[argcount], XmNitemCount, n); argcount++;
    XtSetArg(args[argcount], XmNitems, (XtArgVal)names); argcount++;

    // Create agent list 
    Widget list = XmCreateScrolledList(pane, "List", args, argcount);


    // Select the current value.
    int pos = 1;
// int pos = config->pos_by_name(kind, s->print_name());

    XmListSelectPos(list, pos, false);

    XtManageChild(list);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Create the action area
    Widget form2 = XtVaCreateWidget("", xmFormWidgetClass, pane, 
                                    XmNfractionBase, 5,
                                    NULL);

    // Add a callback to the list window that handles return key.
    XtAddCallback(
        list,
        XmNdefaultActionCallback,
        (XtCallbackProc)rtn_cb,
        (XtPointer)form2);
   
    // Create the OK button.
    Widget ok = XtVaCreateManagedWidget(
        "Ok", xmPushButtonWidgetClass, form2,
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
    data->kind = kind;
    data->shell = dialog;
    data->list = list;
    data->s = s;
    XtAddCallback(
        ok,
        XmNactivateCallback,
        (XtCallbackProc)Ok_cb,
        (XtPointer)data);

    // Create the Cancel button.
    Widget cancel = XtVaCreateManagedWidget(
        "Cancel", 
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

    XtAddCallback(
        cancel,
        XmNactivateCallback,
        (XtCallbackProc)Cancel_cb,
        dialog);

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
    while(!done)
    {
        XtAppProcessEvent(app, XtIMAll);
    }

    return;
}


///////////////////////////////////////////////////////////////////////
// $Log: PickAgent.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.3  2006/07/10 06:09:43  endo
// Clean-up for MissionLab 7.0 release.
//
// Revision 1.2  2006/01/30 02:47:27  endo
// AO-FNC CBR-CNP Type-I check-in.
//
// Revision 1.1.1.1  2005/02/06 22:59:31  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.15  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.14  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.13  2000/04/25 07:22:41  endo
// TaskExited_design added.
//
// Revision 1.12  2000/04/13 21:35:27  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.11  1998/12/04 21:33:06  liston
// Removed the two lines which caused the scrolled list of
// behaviors to have zero area.
//
// Revision 1.10  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.9  1996/05/07  19:53:20  doug
// fixing compile warnings
//
// Revision 1.8  1996/03/12  17:45:14  doug
// *** empty log message ***
//
// Revision 1.7  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.6  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.5  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.1  1996/02/21  15:46:22  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
