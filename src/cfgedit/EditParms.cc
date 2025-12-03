/**********************************************************************
 **                                                                  **
 **                            edit_parms.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 1997, 1999 - 2003 Georgia Tech Research        **
 **  Corporation Atlanta, Georgia  30332-0415                        **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: EditParms.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

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
#include <Xm/LabelG.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Scale.h>
#include <Xm/ToggleB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Protocols.h>
#include <Xm/Frame.h>

#include "load_cdl.h"
#include "popups.h"
#include "configuration.hpp"
#include "globals.h"
#include "PuDataType.h"
#include "ConstructPrintName.h"
#include "EditParms.h"
#include "TypeInfo.hpp"
#include "mission_expert.h"

struct toggle_info
{
   int            bit;
   unsigned long *status;
};

struct parm_def
{
    Widget w;
    class TypeInfo *TypeRecord;
    //Symbol *ig;
    Symbol *p;
    bool isAdvanced;

    ~parm_def() {};
};

typedef enum EditParmsReturnStatus_t {
    EDITPARMS_RETURN_PENDING,
    EDITPARMS_RETURN_OK,
    EDITPARMS_RETURN_CANCEL,
    EDITPARMS_RETURN_ADVANCED,
    EDITPARMS_RETURN_BASIC
};

const int EDIT_PARMS_DIALOG_HEIGHT = 400;
const int EDIT_PARMS_DIALOG_SLIDER_WIDTH = 50;

GTList<parm_def *> dialog_def;
static bool done = true;
static Widget dialog = NULL;
static int dialogReturnStatus = EDITPARMS_RETURN_PENDING;

//-----------------------------------------------------------------------
// Move up a widget hierarchy until a shell widget is reached
//-----------------------------------------------------------------------
static Widget GetTopShell( Widget w )
{
    while ( w && !XtIsWMShell( w ) )
    {
        w = XtParent( w );
    }

    return w;
}

//-----------------------------------------------------------------------
static void Ok_cb(Widget w, Widget shell)
{
   while( !dialog_def.isempty() )
   {
      parm_def *pd = dialog_def.get();

      if (pd->w != NULL)
      {
          //pd->TypeRecord->UpdateFromDisplayWidget(pd->w, &pd->ig->name);
          //pd->TypeRecord->UpdateFromDisplayWidget(pd->w, &(pd->p->input_generator->name));
          pd->TypeRecord->UpdateFromDisplayWidget(pd->w, pd->p);
      }
   }

   // Regenerate the drawing to get the changes
   config->regen_page();
   
   // Mark drawing as changed
   config->made_change();

   if (gMExp->isEnabled())
   {
       gMExp->madeChange();
   }
   
   // Kill us
   XtDestroyWidget(shell);
	 
   done = true;
   dialogReturnStatus = EDITPARMS_RETURN_OK;
}

//-----------------------------------------------------------------------
static void close_cb(
    Widget w,
    XtPointer client_data,
    XtPointer callback_data)
{
    if (dialogReturnStatus == EDITPARMS_RETURN_PENDING)
    {
        dialogReturnStatus = EDITPARMS_RETURN_CANCEL;
    }

    // destroy the dialog box
    if (dialog != NULL)
    {
        XtDestroyWidget(dialog);
        dialog = NULL;
    }
    done = true;
}

/*-----------------------------------------------------------------------*/

static void cancel_cb(Widget w, Widget shell)
{
    // Dump the old record
    while( !dialog_def.isempty() )
    {
        delete dialog_def.get();
    }

    // Kill us
    XtDestroyWidget(shell);

    done = true;
    dialogReturnStatus = EDITPARMS_RETURN_CANCEL;
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Advanced" button is pressed.
//-----------------------------------------------------------------------
static void advanced_cb(
    Widget w,
    XtPointer client_data,
    XtPointer callback_data)
{
    Symbol *rec = (Symbol *)client_data;

    dialogReturnStatus = EDITPARMS_RETURN_ADVANCED;

    XtPopdown(dialog);

    EditParms(rec, true);
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Basic" button is pressed.
//-----------------------------------------------------------------------
static void basic_cb(
    Widget w,
    XtPointer client_data,
    XtPointer callback_data)
{
    Symbol *rec = (Symbol *)client_data;

    dialogReturnStatus = EDITPARMS_RETURN_BASIC;

    XtPopdown(dialog);

    EditParms(rec, false);
}

/*-----------------------------------------------------------------------*/
void EditParms(Symbol *rec, bool showAdvancedParm)
{
    XEvent event;
    Atom atomWmDeleteWindow;
    Widget pane, scroll, control, form2, ok, cancel, advanced;
    Widget widgetEventSource;
    Symbol *p = NULL, *p2 = NULL, *data_type = NULL;
    Symbol *pair = NULL;
    parm_def *pd = NULL;
    GTList<parm_def *> new_dialog_def;
    char buf[2048];
    char *n = NULL;
    //char *nvalue = NULL;
    void *cur = NULL, *cur2 = NULL;
    bool isAdvancedParm = false, isPair = false;

    if( rec == NULL )
    {
        warn_user("Internal Error: Null record to EditParms"); 
        return;
    }

    n = ObjectName(rec);
    sprintf(buf,"Set %s Parameters", n);
    free(n);

    dialog = XtVaCreatePopupShell(
        "EditParms",
        xmDialogShellWidgetClass, main_window, 
        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
        XmNtitle, buf,
        XmNx, 100,  
        XmNy, 100,
        NULL);

    pane = XtVaCreateWidget(
        "pane", xmPanedWindowWidgetClass, dialog,
        XmNsashWidth, 1,
        XmNsashHeight, 1,
        NULL);

    if (showAdvancedParm)
    {
        scroll = XtVaCreateManagedWidget (
            "scroll",
            xmScrolledWindowWidgetClass, pane,
            XmNwidth, TYPEINFO_DEFAULT_WIDTH + EDIT_PARMS_DIALOG_SLIDER_WIDTH,
            XmNheight, EDIT_PARMS_DIALOG_HEIGHT,
            XmNscrollBarDisplayPolicy, XmSTATIC,
            XmNscrollingPolicy, XmAUTOMATIC,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNtopAttachment, XmATTACH_FORM,
            NULL);
    }

    control = XtVaCreateWidget(
        "control",
        xmRowColumnWidgetClass, showAdvancedParm? scroll : pane,
        XmNpacking,	XmPACK_TIGHT,
        XmNnumColumns, 1,
        NULL);

    // Make a modification line for each parameter
    if (done)
    {
        if ((cur = rec->parameter_list.first(&p)) != NULL)
        {
            do 
            {
                if(p->name[0] == '%')
                {
                    data_type = PuDataType(rec, p->name, &isAdvancedParm, &isPair, &pair);

                    if( data_type && data_type->TypeRecord )
                    {
                        // Remember relevent stuff so the ok callback can update the tree.
                        pd = new parm_def;
                        //pd->ig = p->input_generator;
                        pd->TypeRecord = data_type->TypeRecord;
                        pd->p = p;
                        pd->isAdvanced = isAdvancedParm;

                        if (isPair)
                        {
                            // Do not handle this parameter because it is already
                            // displayed by its pair.
                            delete pd;
                            continue;
                        }
                        else if ((!showAdvancedParm) && (isAdvancedParm))
                        {
                            // Do not display the advanced parameter.
                            pd->w = NULL;
                        }
                        else
                        {
                            if (pair != NULL)
                            {
                                if ((cur2 = rec->parameter_list.first(&p2)) != NULL)
                                {
                                    do
                                    {
                                        if(p2->name[0] == '%')
                                        {
                                            if (strcmp(p2->name, pair->name) == 0)
                                            {
                                                p->pair = p2;
                                                break;
                                            }
                                        }

                                    } while ((cur = rec->parameter_list.next(&p2, cur2)) != NULL);
                                }
                            }

                            pd->w = data_type->TypeRecord->CreateDisplayWidget(control, p);
                        }

                        dialog_def.append(pd);
                    }
                }
            } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
        }
    }
    else
    {
        // Dialog is still open. Use the existing dialog_def.
        while(!dialog_def.isempty())
        {
            pd = dialog_def.get();
            p = pd->p->dup(false, pd->p->name);

            //nvalue = NULL;

            if (pd->w != NULL)
            {
                //pd->TypeRecord->UpdateFromDisplayWidget(pd->w, &nvalue);
                pd->TypeRecord->UpdateFromDisplayWidget(pd->w, p);
                //p->input_generator->name = nvalue;
            }

            if ((!showAdvancedParm) && pd->isAdvanced)
            {
                // Do nothing.
            }
            else
            {
                pd->w = pd->TypeRecord->CreateDisplayWidget(control, p);
            }

            new_dialog_def.append(pd);
        }

        dialog_def = new_dialog_def;
    }

    XtManageChild(control);

    if (showAdvancedParm)
    {
        XtManageChild(scroll);
    }

    // Create the action area
    form2 = XtVaCreateWidget(
        "", xmFormWidgetClass, pane, 
        XmNfractionBase, 5,
        NULL);

    // Create the OK button.
    ok = XtVaCreateManagedWidget(
        "Ok", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,	XmATTACH_FORM,
        XmNbottomAttachment,	XmATTACH_FORM,
        XmNleftAttachment,	XmATTACH_POSITION,
        XmNleftPosition,	0,
        XmNrightAttachment,	XmATTACH_POSITION,
        XmNrightPosition,	1,
        XmNshowAsDefault,	True,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(ok, XmNactivateCallback, (XtCallbackProc)Ok_cb, dialog);

    // Create the Cancel button.
    cancel = XtVaCreateManagedWidget(
        "Cancel", 
        xmPushButtonGadgetClass, form2,
        XmNtopAttachment,	XmATTACH_FORM,
        XmNbottomAttachment,	XmATTACH_FORM,
        XmNleftAttachment,	XmATTACH_POSITION,
        XmNleftPosition,	2,
        XmNrightAttachment,	XmATTACH_POSITION,
        XmNrightPosition,	3,
        XmNshowAsDefault,	False,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(cancel, XmNactivateCallback, (XtCallbackProc)cancel_cb,dialog);

    // Create the Cancel button.
    if (showAdvancedParm)
    {
        advanced = XtVaCreateManagedWidget(
            "Basic", 
            xmPushButtonGadgetClass, form2,
            XmNtopAttachment,	XmATTACH_FORM,
            XmNbottomAttachment,	XmATTACH_FORM,
            XmNleftAttachment,	XmATTACH_POSITION,
            XmNleftPosition,	4,
            XmNrightAttachment,	XmATTACH_POSITION,
            XmNrightPosition,	5,
            XmNshowAsDefault,	False,
            XmNdefaultButtonShadowThickness, 1,
            NULL);
        XtAddCallback(advanced, XmNactivateCallback, (XtCallbackProc)basic_cb, (void *)rec);
    }
    else
    {
        advanced = XtVaCreateManagedWidget(
            "Advanced", 
            xmPushButtonGadgetClass, form2,
            XmNtopAttachment,	XmATTACH_FORM,
            XmNbottomAttachment,	XmATTACH_FORM,
            XmNleftAttachment,	XmATTACH_POSITION,
            XmNleftPosition,	4,
            XmNrightAttachment,	XmATTACH_POSITION,
            XmNrightPosition,	5,
            XmNshowAsDefault,	False,
            XmNdefaultButtonShadowThickness, 1,
            NULL);
        XtAddCallback(advanced, XmNactivateCallback, (XtCallbackProc)advanced_cb, (void *)rec);
    }


    XtManageChild(form2);

#if 0
    // Fix the height of the action area
    Dimension h;
    XtVaGetValues(
        cancel, 
        XmNheight, &h, 
        NULL);
    XtVaSetValues(
        form2, 
        XmNpaneMaximum, h, 
        XmNpaneMinimum, h, 
        XmNdefaultButton, ok,
        NULL);
#endif

    XtManageChild(pane);
   
    // Wait for the answer
    done = false;
    dialogReturnStatus = EDITPARMS_RETURN_PENDING;

    // get the atomWmDeleteWindow message specifier and add a callback for it
    atomWmDeleteWindow = XmInternAtom(
        XtDisplay(dialog),
        "WM_DELETE_WINDOW",
        false);

    XmAddWMProtocolCallback(
        dialog,
        atomWmDeleteWindow, 
        close_cb,
        NULL);

    if (dialog != NULL)
    {
        XtPopup(dialog, XtGrabExclusive);

        // make sure pending events are handled first    
        XFlush(XtDisplay(dialog));
        XmUpdateDisplay(XtParent(dialog));

        // dialogReturnStatus is set to another value in callbacks to the "cancel"
        // button, the "OK" button, and the WM_DELETE_WINDOW message.
        // (in your WM_DELETE_WINDOW message handler, be sure to check 
        // and see if dialogReturnStatus has allready been set by something else
        // since the message gets handled after "OK" and "cancel" buttons
        // get clicked.)
        while (dialogReturnStatus == EDITPARMS_RETURN_PENDING)
        {
            // get the next event from the event queue
            XNextEvent(XtDisplay(dialog), &event);

            // get the widget the event originated from
            widgetEventSource = XtWindowToWidget(
                event.xany.display,
                event.xany.window);

            // get the shell the event originated in
            widgetEventSource = GetTopShell(widgetEventSource);

            // dispatch the event if: 1) the event originated from
            // a child of the dialog box, or 2) we got an Expose event.
            // We let through the Expose events unconditionally since 
            // we still want background windows to update themselves.
            if ((widgetEventSource == dialog) || 
                (event.type == Expose))
            {
                XtDispatchEvent(&event);
            }
        }
    }



}


///////////////////////////////////////////////////////////////////////
// $Log: EditParms.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.4  2006/12/05 01:42:43  endo
// cdl parser now accepts {& a b}.
//
// Revision 1.3  2006/08/29 15:12:31  endo
// Advanced parameter flag added.
//
// Revision 1.2  2006/08/08 17:29:53  lilia
// scroll window added to EditParms.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:31  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.20  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.19  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.18  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.17  2000/04/13 22:27:57  endo
// Checked in for Doug.
//
// Revision 1.16  1999/07/03 21:40:03  endo
// The point-and-click function moved to in
// src/load_cdl/TwoPoints.hpp.
//
// Revision 1.15  1999/06/02 23:38:32  xu
// Fixed a bug in the point-and-click function that crashes with dealing multiple parameters
// of a GoTo behavior.
//
// Revision 1.14  1999/06/02 01:53:11  xu
// Modified the function of middle-button-clicking on a "GoTo" behavior
// to bring up mlab to display an overlay, so the user can use mouse to
// point-and-click on the overlay and get the target coordinates easily.
//
// Revision 1.13  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.12  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.12  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.11  1996/05/08 15:16:39  doug
// fixing compile warnings
//
// Revision 1.10  1996/04/06  23:54:05  doug
// *** empty log message ***
//
// Revision 1.9  1996/03/08  00:46:56  doug
// *** empty log message ***
//
// Revision 1.8  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.7  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.6  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.5  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.4  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.3  1996/02/25  01:13:14  doug
// *** empty log message ***
//
// Revision 1.2  1996/02/20  22:52:24  doug
// adding EditParms
//
// Revision 1.1  1996/02/20  22:20:48  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
