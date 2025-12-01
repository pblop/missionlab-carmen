/**********************************************************************
 **                                                                  **
 **                            Pushupparms.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: PushupParms.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


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
#include <Xm/MessageB.h>
#include <Xm/ToggleB.h>

#include "load_cdl.h"
#include "popups.h"
#include "configuration.hpp"
#include "globals.h"
#include "PuDataType.h"
#include "ConstructPrintName.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp"
#include "fsa.hpp"
#include "transition.hpp"
#include "callbacks.h"
#include "help.h"
#include "PushupParms.h"

#include "TypeInfo.hpp"

struct toggle_info {
   int  our_value;
   int *value;
};

static bool done;
static bool cancel;

/*-----------------------------------------------------------------------*/
static void
Ok_cb(Widget w, Widget shell)
{
    //USE(w);

    // Kill us
    XtDestroyWidget(shell);
	 
    done = true;
}

/*-----------------------------------------------------------------------*/

static void
cancel_cb(Widget w, Widget shell)
{
    //USE(w);

    // Kill us
    XtDestroyWidget(shell);

    done = true;
    cancel = true;
}

/*-----------------------------------------------------------------------*/

static void
toggled(Widget w, toggle_info *info)
{
    //USE(w);

    // Set the current value to our position
    *info->value = info->our_value;
}


/*-----------------------------------------------------------------------*/

static char *users_new_name;
static bool  canceled;

static void
done_entering_name(Widget w,
		bool change,
		XmFileSelectionBoxCallbackStruct * fcb)
{
   done = true;

   /* unexpose the file selection dialog */
   XtUnmanageChild(w);

   if( change )
   {
      XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET, &users_new_name);
      canceled = false;
   }
   else
   {
      canceled = true;
   }
}


/*-----------------------------------------------------------------------*/

void
PushupParms(fsa *fsa_rec, Symbol *rec)
{
    if( rec == NULL )
    {
        warn_user("Internal Error: Null record to PushupParms"); 
        return;
    }

    Widget dialog = XtVaCreatePopupShell("PushupParms",
                                         xmDialogShellWidgetClass, main_window, 
                                         XmNtitle, "Push up parameters",
                                         XmNx, 100,  
                                         XmNy, 100,
                                         NULL);

    Widget pane = XtVaCreateWidget("pane", xmPanedWindowWidgetClass, dialog,
                                   XmNsashWidth, 1,
                                   XmNsashHeight, 1,
                                   NULL);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Create control area
    Widget control = XtVaCreateWidget("",
                                      xmRowColumnWidgetClass, pane,
                                      XmNleftAttachment,	XmATTACH_FORM,
                                      XmNrightAttachment,	XmATTACH_FORM,
                                      XmNtopAttachment,	XmATTACH_FORM,
                                      NULL);

    // Create name
    char buf[2048];
    char *n = ObjectName(rec);
    sprintf(buf,"Select a %s parameter to push up", n);
    free(n);
    XmString str = XmStringCreateLocalized(buf);
    Widget w = XtVaCreateManagedWidget("", xmLabelGadgetClass, control,
                                       XmNlabelString, str,
                                       NULL);
    XmStringFree(str);

    // Count the parameters
    Symbol *p;
    void *cur;
    int num_values = 0;
    if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
        do 
        {
            if( p->name[0] == '%' )
            {
                num_values ++;
            }
        } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }

    // Allocate space for them
    // ENDO - gcc 3.4
    //char  **values = new (char *)[num_values];
    //Symbol **ptrs = new (Symbol *)[num_values];
    char  **values = new char *[num_values]; 
    Symbol **ptrs = new Symbol *[num_values];

    // Fill the array
    int pos = 0;
    if ((cur = rec->parameter_list.first(&p)) != NULL)
    {
        do 
        {
            if( p->name[0] == '%' )
            {
                char buf[2048];

                Symbol *data_type = PuDataType(rec, p->name);

                if( data_type && data_type->TypeRecord )
                {
                    sprintf(buf, "%s \t(%s)", &p->name[1], 
                            data_type->TypeRecord->PrintValue(p,","));
                    values[pos] = strdup(buf);
                    ptrs[pos] = p;
                }
                else
                {
                    sprintf(buf, "%s \t(%s)", &p->name[1], "???");
                    values[pos] = strdup(buf);
                    ptrs[pos] = p;
                }
                pos++;
            }
        } while ((cur = rec->parameter_list.next(&p, cur)) != NULL);
    }

    // Make a radio box with each parameter listed
    char *label = "Select one of the following to push up";
    int *current_selection = new int;
    *current_selection = -1;
    w = XtVaCreateWidget("", xmRowColumnWidgetClass, control,
                         XtVaTypedArg, XmNtitleString, XmRString, label, strlen(label)+1,
                         XmNpacking,	XmPACK_COLUMN,
                         XmNnumColumns,	1,
                         XmNradioBehavior, true,
                         XmNradioAlwaysOne, true,
                         XmNuserData, (int)current_selection,
                         NULL);

    for(int i=0; i<num_values; i++)
    {
        Widget btn = XtVaCreateManagedWidget(values[i],
                                             xmToggleButtonWidgetClass, w, 
                                             NULL);

        // Make a record to handle the toggling actions
        toggle_info *info = new toggle_info;
        info->our_value = i;
        info->value = current_selection;
        XtAddCallback(btn, XmNvalueChangedCallback, 
                      (XtCallbackProc)toggled, (XtPointer)info);
    }
    XtManageChild(w);
    XtManageChild(control);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Create the action area
    Widget form2 = XtVaCreateWidget("", xmFormWidgetClass, pane, 
                                    XmNfractionBase, 5,
                                    NULL);

    // Create the OK button.
    Widget ok_btn = XtVaCreateManagedWidget("Ok", xmPushButtonGadgetClass, form2,
                                            XmNtopAttachment,	XmATTACH_FORM,
                                            XmNbottomAttachment,	XmATTACH_FORM,
                                            XmNleftAttachment,	XmATTACH_POSITION,
                                            XmNleftPosition,	1,
                                            XmNrightAttachment,	XmATTACH_POSITION,
                                            XmNrightPosition,	2,
                                            XmNshowAsDefault,	True,
                                            XmNdefaultButtonShadowThickness, 1,
                                            NULL);
    XtAddCallback(ok_btn, XmNactivateCallback, (XtCallbackProc)Ok_cb, dialog);

    // Create the Cancel button.
    Widget cancel_btn = XtVaCreateManagedWidget("Cancel", 
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
    XtAddCallback(cancel_btn, XmNactivateCallback, (XtCallbackProc)cancel_cb,dialog);
    XtManageChild(form2);

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    XtManageChild(pane);
    XtPopup(dialog, XtGrabExclusive);
   
    /* Wait for the answer */    
    done = false;
    cancel = false;
    while( !done )
    {
        XtAppProcessEvent(app, XtIMAll);
    }

    if( cancel )
    {
        warn_user("User canceled operation"); 
        return;
    }

    if( *current_selection < 0 )
    {
        warn_user("No parameter selected, operation canceled"); 
        return;
    }

    // Check other states and transitions to see if there are other 
    // parameters with the same name and value.
    bool there_are_others = false;
    Symbol *this_parm = ptrs[*current_selection];
    if( this_parm->input_generator )
    {
        char *parm_name = this_parm->name;
        char *parm_value = this_parm->input_generator->name;

        // Check states
        Symbol *states = fsa_rec->members();
        Symbol *state;
        if ((cur = states->parameter_list.first(&state)) != NULL)
        {
            do 
            {
                Symbol *index = state->input_generator;
                if( index )
                {
                    Symbol *parm;
                    void *cur_parm;
                    if ((cur_parm = index->parameter_list.first(&parm)) != NULL)
                    {
                        do 
                        {
                            if( parm != this_parm )
                            {
                                if( strcmp(parm_name, parm->name) == 0 &&
                                    parm->input_generator != NULL &&
                                    parm->input_generator->name != NULL &&
                                    strcmp(parm_value, parm->input_generator->name) == 0 )
                                {
                                    there_are_others = true;
                                    break;
                                }
                            }
                        } while ((cur_parm = index->parameter_list.next(&parm, cur_parm)) != NULL);
                    }
      
                    if( there_are_others )
                    {
                        break;
                    }
                }
            } while ((cur = states->parameter_list.next(&state, cur)) != NULL);
        }

        // Check transitions, if didn't find one yet
        if( !there_are_others )
        {
            Symbol *rules = fsa_rec->rules();
            Symbol *trans;
            if ((cur = rules->parameter_list.first(&trans)) != NULL)
            {
                do 
                {
                    Symbol *index = trans->input_generator;
                    if( index )
                        index = index->input_generator;
   
                    if( index )
                    {
                        Symbol *parm;
                        void *cur_parm;
                        if ((cur_parm = index->parameter_list.first(&parm)) != NULL)
                        {
                            do 
                            {
                                if( parm != this_parm )
                                {
                                    if( strcmp(parm_name, parm->name) == 0 &&
                                        parm->input_generator != NULL &&
                                        parm->input_generator->name != NULL &&
                                        strcmp(parm_value, parm->input_generator->name) == 0 )
                                    {
                                        there_are_others = true;
                                        break;
                                    }
                                }
                            } while ((cur_parm = index->parameter_list.next(&parm, cur_parm)) != NULL);
                        }
         
                        if( there_are_others )
                        {
                            break;
                        }
                    }
                } while ((cur = rules->parameter_list.next(&trans, cur)) != NULL);
            }
        }
    }

    bool do_all = false;
    if( there_are_others )
    {
        char buf[2048];
        sprintf(buf,"Push up ALL %s parameters or just this one?",
                values[*current_selection]);

        // Define the exit_design dialog structure
        dialog_rec dialog =
            {(DIALOG_BLDR) XmCreateQuestionDialog,
             {
                 {XmNmessageString, buf},
                 {XmNokLabelString, "All"},
                 {XmNcancelLabelString, "This"},
                 {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                 {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                 {NULL, 0}
             },
             {
                 {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_workspace},
                 {NULL, NULL, NULL}}
            };

        do_all = wait_dialog_box(NULL, &dialog, NULL);
    }


    // Define the new_design dialog structure
    dialog_rec name_dialog =
        {(DIALOG_BLDR) XmCreatePromptDialog,
         {
             {XmNselectionLabelString, "Enter a new name and press `Rename' if you wish to rename the pushed up parameter"},
             {XmNokLabelString, "Rename"},
             {XmNcancelLabelString, "Original"},
             {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
             {NULL, 0}
         },
         {
             {XmNokCallback,(XtCallbackProc)done_entering_name,(XtPointer)true},
             {XmNcancelCallback,(XtCallbackProc)done_entering_name,(XtPointer)false},
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

    // Canceled means to leave the name unchanged
    bool use_new_name = false;
    Symbol *data_type;
    char *new_name = NULL;
    if( !canceled && users_new_name && users_new_name[0] != '\0')
    {
        // Construct the new name
        use_new_name = true;

        char buf[2048];
        buf[0] = '%';
        strcpy(&buf[1],users_new_name);
        new_name = strdup(buf);

        // Find the data type
        data_type= PuDataType(fsa_rec->fsa_node(),ptrs[*current_selection]->name);
    }


    if( do_all )
    {
        // Push all parameters with the same name and value up.
        Symbol *this_parm = ptrs[*current_selection];
        if( this_parm->input_generator )
        {
            char *parm_name = this_parm->name;
            char *parm_value = this_parm->input_generator->name;

            // Check states
            Symbol *states = fsa_rec->members();
            Symbol *state;
            if ((cur = states->parameter_list.first(&state)) != NULL)
            {
                do 
                {
                    Symbol *index = state->input_generator;
                    if( index )
                    {
                        Symbol *parm;
                        void *cur_parm;
                        if ((cur_parm = index->parameter_list.first(&parm)) != NULL)
                        {
                            do 
                            {
                                if( strcmp(parm_name, parm->name) == 0 &&
                                    parm->input_generator != NULL &&
                                    parm->input_generator->name != NULL &&
                                    strcmp(parm_value, parm->input_generator->name) == 0 )
                                {
                                    pushup_FSA_component_parm(fsa_rec->fsa_node(), parm, use_new_name, data_type, new_name);
                                }
                            } while ((cur_parm = index->parameter_list.next(&parm, cur_parm)) != NULL);
                        }
                    }
                } while ((cur = states->parameter_list.next(&state, cur)) != NULL);
            }
   
            // Check transitions
            Symbol *rules = fsa_rec->rules();
            Symbol *trans;
            if ((cur = rules->parameter_list.first(&trans)) != NULL)
            {
                do 
                {
                    Symbol *index = trans->input_generator;
                    if( index )
                        index = index->input_generator;
   
                    if( index )
                    {
                        Symbol *parm;
                        void *cur_parm;
                        if ((cur_parm = index->parameter_list.first(&parm)) != NULL)
                        {
                            do 
                            {
                                if( strcmp(parm_name, parm->name) == 0 &&
                                    parm->input_generator != NULL &&
                                    parm->input_generator->name != NULL &&
                                    strcmp(parm_value, parm->input_generator->name) == 0 )
                                {
                                    pushup_FSA_component_parm(fsa_rec->fsa_node(), parm, use_new_name, data_type, new_name);
                                }
                            } while ((cur_parm = index->parameter_list.next(&parm, cur_parm)) != NULL);
                        }
                    }
                } while ((cur = rules->parameter_list.next(&trans, cur)) != NULL);
            }
        }
    }
    else
    {
        pushup_FSA_component_parm(fsa_rec->fsa_node(), ptrs[*current_selection], use_new_name, data_type, new_name);
    }

    // Regenerate the drawing to get the changes
    config->regen_page();
   
    // Mark drawing as changed
    config->made_change();
}


///////////////////////////////////////////////////////////////////////
// $Log: PushupParms.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.2  2006/12/05 01:42:43  endo
// cdl parser now accepts {& a b}.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.4  2006/05/15 02:11:12  endo
// gcc-3.4 upgrade
//
// Revision 1.3  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.2  2006/01/30 02:47:27  endo
// AO-FNC CBR-CNP Type-I check-in.
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  1997/02/14  16:53:35  zchen
// Initial revision
//
// Revision 1.3  1996/10/03 21:46:40  doug
// nested FSA's are working
//
// Revision 1.2  1996/10/02 21:45:20  doug
// working on pushup in states and transitions
//
// Revision 1.1  1996/10/01 18:35:00  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
