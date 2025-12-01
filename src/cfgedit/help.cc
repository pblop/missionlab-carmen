/**********************************************************************
 **                                                                  **
 **                             help.cc                              **
 **                                                                  **
 **                                                                  **
 **             creates and manages a help system with index         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: help.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/SelectioB.h>
#include <stdio.h>

#include "design.h"
#include "popups.h"
#include "gt_std_types.h"
#include "help.h"

char* help_text[ NUM_HELP_PAGES ] = {
#include "help_text/copyright.txt"
,
#include "help_text/new_design.txt"
,
#include "help_text/save_as.txt"
,
#include "help_text/exit.txt"
,
#include "help_text/discard.txt"
,
#include "help_text/start_over.txt"
,
#include "help_text/select_file.txt"
,
#include "help_text/overwrite_file.txt"
,
#include "help_text/overwrite_workspace.txt"
,
#include "help_text/unbind_arch.txt"
,
#include "help_text/unbind_robot.txt"
,
#include "help_text/bind_robot.txt"
,
#include "help_text/choose_actuator_for_obp.txt"
,
#include "help_text/choose_sensor_for_ibp.txt"
,
#include "help_text/choose_robot_for_rbp.txt"
,
#include "help_text/select_agent.txt"
,
#include "help_text/exit_fsa.txt"
,
#include "help_text/state_name.txt"
,
#include "help_text/transition_name.txt"
,
#include "help_text/split_nodes.txt"
,


#include "help_text/index.txt"
};


char* index_text[ NUM_HELP_PAGES ];

XmString help_strings[  NUM_HELP_PAGES ];
XmString index_strings[ NUM_HELP_PAGES ];

static Widget help_dialog  = NULL;
static Widget index_dialog = NULL;


// This callback is used to kill ourselves and set the dialog pointer
// to NULL so it can't be referenced again by help_callback().
// This function is called from the Done button in the help dialog.
// It is also our XmNdestroyCallback, so reset our dialog_ptr to NULL.
static void help_done()
{
    if ( !help_dialog ) // prevent unnecessarily destroying twice
    {
        return;
    }
    XtDestroyWidget( help_dialog ); // this might call ourselves..
    help_dialog = NULL;
}

static void index_done()
{
    if ( !index_dialog ) // prevent unnecessarily destroying twice
    {
        return;
    }
    XtDestroyWidget( index_dialog );
    index_dialog = NULL;
}


void index_pick_callback( Widget parent, void* p, void* vcbs )
{
    XmSelectionBoxCallbackStruct* cbs = (XmSelectionBoxCallbackStruct*) vcbs;

    if ( cbs->reason == XmCR_OK )
    {
        char* val;
        XmStringGetLtoR( cbs->value, XmSTRING_DEFAULT_CHARSET, &val );

        // now find the page number for the topic
        for ( int i = 0; i < ( NUM_HELP_PAGES - 1 ); i++ )
        {
            if( strcmp( index_text[ i ], val ) == 0 )
            {
                index_done();
                help_callback( parent, (void*) i, NULL);
                return;
            }
        }
        // NOTE:: No match, so let it fall into no match case
    }

    warn_user( "Select one of the help topic" );
    index_done();
}

void index_callback( Widget parent, void* p, void* cbs )
{
    // If help dialog is up, remove it 
    if ( help_dialog )
    {
        help_done();
    }

    if ( index_dialog )
    {
        // user pressed Help button in MesageDialog again.  We're
        // already up, so just make sure we're visible and return.
        XtPopup( XtParent( index_dialog ), XtGrabNone );
        XMapRaised( XtDisplay( index_dialog ), XtWindow( XtParent( index_dialog ) ) );
        return;
    }
    else
    {
        // We're not up, so create new index Dialog
        Arg args[6];

        // Action area button labels.
        XmString show  = XmStringCreateLocalized( "Show" );
        XmString done  = XmStringCreateLocalized( "Exit" );
        XmString label = XmStringCreateLocalized( "Index of help pages" );

        XtSetArg( args[ 0 ], XmNokLabelString, show );
        XtSetArg( args[ 1 ], XmNcancelLabelString, done );
        XtSetArg( args[ 2 ], XmNmustMatch, True );
        XtSetArg( args[ 3 ], XmNlistLabelString, label );
        XtSetArg( args[ 4 ], XmNlistItems, index_strings );
        XtSetArg( args[ 5 ], XmNlistItemCount, NUM_HELP_PAGES - 1 );

        index_dialog = XmCreateSelectionDialog( top_level, "index", args, 6 );

        XmStringFree( show  );
        XmStringFree( done  );
        XmStringFree( label );

        XtAddCallback( index_dialog, XmNcancelCallback, (XtCallbackProc) index_done, 0 );
        XtAddCallback( index_dialog, XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_index );

        XtAddCallback( index_dialog, XmNokCallback, (XtCallbackProc) index_pick_callback, 0 );


        // If our parent dies, we must reset "dialog" to NULL!
        XtAddCallback( index_dialog, XmNdestroyCallback, (XtCallbackProc) index_done, 0 );

        XtUnmanageChild( XmSelectionBoxGetChild( index_dialog, XmDIALOG_APPLY_BUTTON ) );
    }

    // display the dialog
    XtManageChild( index_dialog );
    XtPopup( XtParent( index_dialog ), XtGrabNone );
}

void help_callback( Widget parent, void* ppage, void* cbs )
{
    int page = (int) ppage;

    if ( help_dialog )
    {
        // Load the correct page 
        XtVaSetValues( help_dialog, XmNmessageString, help_strings[ page ], NULL );

        // user pressed Help button in MesageDialog again.  We're
        // already up, so just make sure we're visible and return.
        XtPopup(XtParent(help_dialog), XtGrabNone);
        XMapRaised(XtDisplay(help_dialog), XtWindow(XtParent(help_dialog)));

        // If on index page, set "Index" to insensitive.
        XtSetSensitive(XmMessageBoxGetChild(help_dialog,XmDIALOG_HELP_BUTTON),
                       page != HELP_PAGE_index);

        return;
    }
    else
    {
        // We're not up, so create new Help Dialog
        Arg args[ 4 ];

        // Action area button labels.
        XmString done  = XmStringCreateLocalized( "Done"  );
        XmString index = XmStringCreateLocalized( "Index" );

        XtSetArg( args[ 0 ], XmNcancelLabelString, done );
        XtSetArg( args[ 1 ], XmNhelpLabelString,   index );
        help_dialog = XmCreateInformationDialog( top_level, "help", args, 2 );

        // pass help_done() the address of "dialog" so it can reset
        XtAddCallback( help_dialog, XmNcancelCallback, (XtCallbackProc) help_done, &help_dialog );

        // if index, popup the index dialog
        XtAddCallback( help_dialog, XmNhelpCallback, (XtCallbackProc) index_callback, 0 );

        // If our parent dies, we must reset "dialog" to NULL!
        XtAddCallback( help_dialog, XmNdestroyCallback, (XtCallbackProc) help_done, &help_dialog );

        XmStringFree( done  );  // once dialog is created, these
        XmStringFree( index );  // strings are no longer needed.
        XtUnmanageChild( XmMessageBoxGetChild(help_dialog, XmDIALOG_OK_BUTTON ) );
    }

    // Load the correct page 
    XtVaSetValues( help_dialog, XmNmessageString, help_strings[ page ], NULL );

    // Help Index too outdated
    XtSetSensitive( XmMessageBoxGetChild( help_dialog, XmDIALOG_HELP_BUTTON ), false );

    // display the dialog
    XtManageChild( help_dialog );
    XtPopup( XtParent( help_dialog ), XtGrabNone );
}

void init_help()
{
    // Load the help pages as strings
    for( int i = 0; i < NUM_HELP_PAGES; i++ )
    {
        help_strings[ i ] = XmStringCreateLtoR( help_text[ i ], XmSTRING_DEFAULT_CHARSET );
    }

    // Extract the title strings for the index
    for( int i = 0; i < ( NUM_HELP_PAGES - 1 ); i++ )
    {
        char buf[ 256 ];
        sscanf( help_text[ i ], "%*s %[^\n]", buf );
        index_text[ i ] = strdup( buf );
        index_strings[ i ] = XmStringCreateLtoR( buf, XmSTRING_DEFAULT_CHARSET );
    }
}



///////////////////////////////////////////////////////////////////////
// $Log: help.cc,v $
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
// Revision 1.17  2002/07/02 20:25:44  blee
// Removed USE macros that were causing compiler warnings.
//
// Revision 1.16  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.15  2000/10/16 19:33:40  endo
// Modified due to the compiler upgrade.
//
// Revision 1.14  2000/03/14 00:02:05  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.13  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.12  1996/05/08  15:16:39  doug
// fixing compile warnings
//
// Revision 1.11  1995/11/04  23:46:50  doug
// *** empty log message ***
//
// Revision 1.10  1995/06/29  18:03:17  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
