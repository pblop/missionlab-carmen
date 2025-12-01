/**********************************************************************
 **                                                                  **
 **                        gt_command_panel.c                        **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_command_panel.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $  */

#include <stdio.h>   
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/ArrowBG.h>
#include <Xm/DialogS.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>

#include "gt_command_panel.h"
#include "gt_load_command.h"
#include "gt_command.h"
#include "gt_sim.h"
#include "mission_design.h"
#include "console.h"
#include "console_side_com.h"

/**********************************************************************
 **                                                                  **
 **                            constants                             **
 **                                                                  **
 **********************************************************************/

#define DISPLAY_MAX_LEN 100



/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define	RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1



/**********************************************************************
 **                                                                  **
 **                         local variables                          **
 **                                                                  **
 **********************************************************************/

static Widget command_panel_shell;
static Widget command_panel;

static Widget user_command_field;
static int user_command_field_touched;

static Widget step_status_label;
static Widget step_display_area;
static Widget next_step_status_label;
static Widget next_step_display_area;

static Widget pause_button;
extern Widget console_pause_button;
extern Widget waypt_pause_button;
extern Widget estop_button_pb;
static Widget single_step_robot_button;
extern Widget console_feedback_button;

static Widget command_filename_field;
static    int command_filename_field_touched;


/**********************************************************************
 **                                                                  **
 **                     declare local functions                      **
 **                                                                  **
 **********************************************************************/

static void check_command_filename_field();
static void check_user_command_field();
static void strcat_maybe_wrap( char* buff, char* str, char* indent, int max_width );
static void display_command( char* buff, gt_Step* step, gt_Command* cmd );
static void hide_command_panel();

static void abort_command_execution();
static void backup_step_list();
static void execute_step_list();
static void execute_user_command();
static void forward_step_list();
static void load_command_file();
static void rewind_step_list();
static void single_step_command();
static void single_step_robot_command();


/**********************************************************************
 **                                                                  **
 **                    implement global functions                    **
 **                                                                  **
 **********************************************************************/



/**********************************************************************/

void gt_create_command_panel( Widget parent )
{
    Widget form1, form2, form3, form4;
    Widget label, button, b1, b2, b3, b4, b5, b7;

    command_panel_shell = XtVaCreatePopupShell(
        "Command Interface", xmDialogShellWidgetClass, parent,
        XmNdeleteResponse, XmDESTROY,
        NULL );

    // create the new widget
    command_panel = XtVaCreateWidget(
        "gt-command-panel", xmPanedWindowWidgetClass, command_panel_shell,
        XmNsashHeight, 3,
        XmNsashWidth,  3,
        NULL );

    // create the first form for user command entry
    form1 = XtVaCreateWidget(
        "command-form1", xmFormWidgetClass, command_panel,
        XmNmarginHeight,      4,
        XmNmarginWidth,       4,
        XmNhorizontalSpacing, 5, 
        NULL );

    label = XtVaCreateManagedWidget(
        "Command:", xmLabelGadgetClass, form1,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );

    button = XtVaCreateManagedWidget(
        " Execute ", xmPushButtonGadgetClass, form1,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNrightAttachment,   XmATTACH_FORM,
        NULL );
    XtAddCallback( button, XmNactivateCallback,
                   (XtCallbackProc) execute_user_command, NULL );

    user_command_field = XtVaCreateManagedWidget(
        "user-command", xmTextFieldWidgetClass, form1,
        XmNcolumns,           60,
        XmNmaxLength,         256,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        label,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       button,
        NULL );
    XtAddCallback( user_command_field, XmNactivateCallback,
                   (XtCallbackProc) execute_user_command, NULL );
    XtAddEventHandler( user_command_field, KeyPressMask | ButtonPressMask, false,
                       (XtEventHandler) check_user_command_field, NULL );
    XmTextFieldSetString( user_command_field,
                          " <Type in a command here>");
    user_command_field_touched = false;

    XtManageChild( form1 );


    // create the second form for loading and executing commands from files
    form2 = XtVaCreateWidget(
        "command-form2", xmFormWidgetClass, command_panel,
        XmNmarginHeight,      4,
        XmNmarginWidth,       4,
        XmNhorizontalSpacing, 5, 
        NULL );

    label = XtVaCreateManagedWidget(
        "File:", xmLabelGadgetClass, form2,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         12,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );

    button = XtVaCreateManagedWidget(
        " Load File ", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_FORM,
        NULL );
    XtAddCallback( button, XmNactivateCallback, 
                   (XtCallbackProc) load_command_file, NULL );
   
    command_filename_field = XtVaCreateManagedWidget(
        "command-filename", xmTextFieldWidgetClass, form2, 
        XmNcolumns,           63,
        XmNmaxLength,         256,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        label,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       button,
        NULL );
    XtAddCallback( command_filename_field, XmNactivateCallback, 
                   (XtCallbackProc) load_command_file, NULL );
    XtAddEventHandler( command_filename_field, KeyPressMask | ButtonPressMask, false,
                       (XtEventHandler) check_command_filename_field, NULL );
    XmTextFieldSetString( command_filename_field,
                          " <Type in a filename from which to load commands here>");
    command_filename_field_touched = false;

    b1 = XtVaCreateManagedWidget(
        "rewind-commands", xmPushButtonGadgetClass, form2,
        RES_CONVERT( XmNlabelString, " Rewind Commands " ),
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );
    XtAddCallback( b1, XmNactivateCallback,
                   (XtCallbackProc) rewind_step_list, NULL );

    b2 = XtVaCreateManagedWidget(
        "backward", xmArrowButtonGadgetClass, form2, 
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        b1,
        XmNarrowDirection,    XmARROW_LEFT,
        NULL );
    XtAddCallback( b2, XmNactivateCallback,
                   (XtCallbackProc) backup_step_list, NULL );

    b3 = XtVaCreateManagedWidget(
        "forward", xmArrowButtonGadgetClass, form2, 
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        b2,
        XmNarrowDirection,    XmARROW_RIGHT,
        NULL );
    XtAddCallback( b3, XmNactivateCallback,
                   (XtCallbackProc) forward_step_list, NULL );

    b7 = XtVaCreateManagedWidget(
        " ABORT ", xmPushButtonGadgetClass, form2,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_FORM,
        NULL );
    XtAddCallback( b7, XmNactivateCallback,
                   (XtCallbackProc) abort_command_execution, NULL );

    pause_button = XtVaCreateManagedWidget(
        "pause", xmPushButtonGadgetClass, form2,
        RES_CONVERT( XmNlabelString, " Pause  " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       b7,
        NULL );
    XtAddCallback(pause_button, XmNactivateCallback,
                  (XtCallbackProc) toggle_pause_execution_button, NULL );

    single_step_robot_button = XtVaCreateManagedWidget(
        "single-step-robot", xmPushButtonGadgetClass, form2,
        RES_CONVERT( XmNlabelString, " Single Step Robots " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       pause_button,
        NULL );
    XtAddCallback( single_step_robot_button, XmNactivateCallback,
                   (XtCallbackProc) single_step_robot_command, NULL );

    b5 = XtVaCreateManagedWidget(
        "single-step", xmPushButtonGadgetClass, form2,
        RES_CONVERT( XmNlabelString, " Single Step " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       single_step_robot_button,
        NULL );
    XtAddCallback( b5, XmNactivateCallback,
                   (XtCallbackProc) single_step_command, NULL );

    b4 = XtVaCreateManagedWidget(
        "execute-commands", xmPushButtonGadgetClass, form2,
        RES_CONVERT( XmNlabelString, " Execute Commands " ),
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         command_filename_field,
        XmNtopOffset,         8,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        b3,
        XmNleftOffset,        15,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       b5,
        NULL );
    XtAddCallback( b4, XmNactivateCallback,
                   (XtCallbackProc) execute_step_list, NULL );

    XtManageChild( form2 );


    // create the third form, for the step display area
    form3 = XtVaCreateWidget(
        "command-form3", xmFormWidgetClass, command_panel,
        XmNmarginHeight,      4,
        XmNmarginWidth,       4,
        NULL );

    step_status_label = XtVaCreateManagedWidget(
        "step-status-label", xmLabelGadgetClass, form3,
        RES_CONVERT( XmNlabelString, " " ),
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         0,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );

    {
        Arg wargs[14];
        int n = 0;
        XtSetArg( wargs[ n ], XmNcolumns,           DISPLAY_MAX_LEN);        n++;
        XtSetArg( wargs[ n ], XmNrows,              4);                      n++;
        XtSetArg( wargs[ n ], XmNmaxLength,         1024);                   n++;
        XtSetArg( wargs[ n ], XmNeditMode,          XmMULTI_LINE_EDIT);      n++;
        XtSetArg( wargs[ n ], XmNeditable,          false);                  n++;
        XtSetArg( wargs[ n ], XmNautoShowCursorPosition,  false);            n++;
        XtSetArg( wargs[ n ], XmNcursorPositionVisible,  false);             n++;
        XtSetArg( wargs[ n ], XmNtopAttachment,     XmATTACH_WIDGET);        n++;
        XtSetArg( wargs[ n ], XmNtopWidget,         step_status_label);      n++;
        XtSetArg( wargs[ n ], XmNleftAttachment,    XmATTACH_FORM);          n++;
        XtSetArg( wargs[ n ], XmNrightAttachment,   XmATTACH_FORM);          n++;
        XtSetArg( wargs[ n ], XmNvalue,
                 " <Steps will be displayed here as they are being executed>"); n++;
        XtSetArg( wargs[ n ], XmNscrollHorizontal,  false); n++;

        step_display_area = XmCreateScrolledText( form3, "current-step", wargs, n );
        XtManageChild( step_display_area );
    }

    next_step_status_label = XtVaCreateManagedWidget(
        "next-step-status-label", xmLabelGadgetClass, form3,
        RES_CONVERT( XmNlabelString, "Next Step:" ),
        XmNtopAttachment,     XmATTACH_WIDGET,
        XmNtopWidget,         step_display_area,
        XmNtopOffset,         5,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL );

    {
        Arg wargs[14];
        int n = 0;
        XtSetArg( wargs[ n ], XmNcolumns,           DISPLAY_MAX_LEN);        n++;
        XtSetArg( wargs[ n ], XmNrows,              2);                      n++;
        XtSetArg( wargs[ n ], XmNmaxLength,         1024);                   n++;
        XtSetArg( wargs[ n ], XmNeditMode,          XmMULTI_LINE_EDIT);      n++;
        XtSetArg( wargs[ n ], XmNeditable,          false);                  n++;
        XtSetArg( wargs[ n ], XmNautoShowCursorPosition,  false);            n++;
        XtSetArg( wargs[ n ], XmNcursorPositionVisible,  false);             n++;
        XtSetArg( wargs[ n ], XmNtopAttachment,     XmATTACH_WIDGET);        n++;
        XtSetArg( wargs[ n ], XmNtopWidget,         next_step_status_label); n++;
        XtSetArg( wargs[ n ], XmNleftAttachment,    XmATTACH_FORM);          n++;
        XtSetArg( wargs[ n ], XmNrightAttachment,   XmATTACH_FORM);          n++;
        XtSetArg( wargs[ n ], XmNvalue,
                 " <The next step to be executed will be displayed here>"); n++;
        XtSetArg( wargs[ n ], XmNscrollHorizontal,  false ); n++;

        next_step_display_area = XmCreateScrolledText( form3, "next-step", wargs, n );
        XtManageChild( next_step_display_area );
    }

    XtManageChild( form3 );


    // create the fourth form, for exiting
    form4 = XtVaCreateWidget(
        "command-form4", xmFormWidgetClass, command_panel,
        XmNskipAdjust,        True,
        XmNmarginHeight,      4,
        XmNmarginWidth,       4,
        NULL );

    button = XtVaCreateManagedWidget(
        "Close", xmPushButtonGadgetClass, form4,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_POSITION,
        XmNrightAttachment,   XmATTACH_POSITION,
        XmNleftPosition,      40,
        XmNrightPosition,     60,
        NULL );
    XtAddCallback( button, XmNactivateCallback,
                   (XtCallbackProc) hide_command_panel, NULL );

    XtManageChild( form4 );
}


/**********************************************************************/

void EnableCommandPanelPauseButton( bool bEnable )
{
    XtVaSetValues( pause_button, XmNsensitive, bEnable, NULL );
}

/**********************************************************************/

void gt_display_step( gt_Step* this_cmd_list,
                      gt_Step_status step_status,
                      gt_Step* next_cmd_list )
{
    char buff[ 1024 ];
    char line[ 256 ];

    // Deal with the current step
    if ( this_cmd_list == NULL )
    {

        // no current step
        XtVaSetValues( step_status_label, 
                       RES_CONVERT(XmNlabelString, " "),
                       NULL );
        XmTextSetString( step_display_area, "<No step executing>" );
    }
    else
    {
        // print this step name
        sprintf( buff, "%3s ", this_cmd_list->name );

        // print through the command list
        while ( this_cmd_list )
        {
            display_command( buff, this_cmd_list, this_cmd_list->command );

            if ( this_cmd_list->next && this_cmd_list->next->command )
            {
                strcat_maybe_wrap( buff, " AND", "   ", DISPLAY_MAX_LEN );
                sprintf( line, "\n    " );
                strcat( buff, line );
            }

            this_cmd_list = this_cmd_list->next;
        }

        // update the step status label
        switch ( step_status )
        {
        case NO_STEP:
            XtVaSetValues( step_status_label, RES_CONVERT( XmNlabelString, " " ), NULL );
            break;

        case EXECUTING_STEP:
            XtVaSetValues( step_status_label, RES_CONVERT( XmNlabelString, "Executing Step:" ), NULL );
            break;

        case FAILED_STEP:
            XtVaSetValues( step_status_label, RES_CONVERT( XmNlabelString, "Failed Step:" ), NULL );
            break;

        case ABORTED_STEP:
            XtVaSetValues( step_status_label, RES_CONVERT( XmNlabelString, "Aborted Step:" ), NULL );
            break;

        default:
            fprintf( stderr, "gt_display_step: Erroneous status (%d)\n", step_status );
        }

        // now display this in the command display window
        XmTextSetString( step_display_area, buff );
    }

   
    // Deal with the next step
    if ( next_cmd_list == NULL )
    {
        // no next step
        XmTextSetString( next_step_display_area, "<No next step>" );
        return;
    }
    else
    {
        // print this step name
        sprintf( buff, "%3s ", next_cmd_list->name );

        // print through the command list
        while ( next_cmd_list )
        {
            display_command( buff, next_cmd_list, next_cmd_list->command );

            if ( next_cmd_list->next && next_cmd_list->next->command )
            {
                strcat_maybe_wrap( buff, " AND", "   ", DISPLAY_MAX_LEN );
                sprintf( line, "\n    " );
                strcat( buff, line );
            }

            next_cmd_list = next_cmd_list->next;
        }

        // now display this in the command display window
        XmTextSetString( next_step_display_area, buff );
    }
}



/**********************************************************************/

void gt_popup_command_panel()
{
    XtManageChild( command_panel );
    XtPopup( command_panel_shell, XtGrabNone );
}



/**********************************************************************/

void set_command_panel_filename( char* filename )
{
    XmTextFieldSetString( command_filename_field, filename );
    XmTextFieldSetInsertionPosition( command_filename_field, strlen( filename ) );
    command_filename_field_touched = true;

    if ( gUseMlabMissionDesign )
    {
        gMMD->saveOverlayName( filename );
    }
}


/**********************************************************************/

void update_pause_button()
{
    if ( paused ) 
    {
        XtVaSetValues( pause_button, RES_CONVERT( XmNlabelString, " Resume " ), NULL );
        XtVaSetValues( estop_button_pb, RES_CONVERT( XmNset, "true" ), NULL );

        XmString str = XmStringCreateLocalized( "Resume" );
        XtVaSetValues( console_pause_button, XmNlabelString, str, NULL );
        XmStringFree( str );
    }
    else 
    {
        XtVaSetValues( pause_button, RES_CONVERT( XmNlabelString, " Pause  " ), NULL );
        XtVaSetValues( estop_button_pb, RES_CONVERT( XmNset, "false" ), NULL );

        XmString str = XmStringCreateLocalized("Pause");
        XtVaSetValues( console_pause_button, XmNlabelString, str, NULL );
        XmStringFree( str );
    }
}

/**********************************************************************/
void update_feedback_button()
{
    XmString str;

    str = gStopRobotFeedback?
        XmStringCreateLocalized((char *)(FEEDBACK_BUTTON_LABEL_NONSTOP.c_str())) :
        XmStringCreateLocalized((char *)(FEEDBACK_BUTTON_LABEL_STOP.c_str()));

    XtVaSetValues(
        console_feedback_button,
        XmNlabelString, str,
        NULL );

    XmStringFree( str );
}

/**********************************************************************
 **                                                                  **
 **                    implement local functions                     **
 **                                                                  **
 **********************************************************************/

/**********************************************************************/

static void abort_command_execution()
{
    gt_abort_command_execution();
}

/**********************************************************************/

static void backup_step_list()
{
    gt_backup_step_list();
}

/**********************************************************************/

static void check_command_filename_field()
{
    if ( !command_filename_field_touched )
    {
        XmTextFieldSetString( command_filename_field, "" );
        command_filename_field_touched = true;
    }
}

/**********************************************************************/

static void check_user_command_field()
{
    if ( !user_command_field_touched )
    {
        XmTextFieldSetString( user_command_field, "" );
        user_command_field_touched = true;
    }
}

/**********************************************************************/

static void display_command( char* buff, gt_Step* step, gt_Command* cmd )
{
    char line[ 256 ];

    if ( cmd == NULL )
    {
        fprintf( stderr, "display_command: Error, tried to print a NULL command\n" );
        return;
    }

    // print each command type appropriately
    switch ( cmd->behavior )
    {
    case START:
        sprintf( line, "UNIT %s START %s", step->unit, step->location );
        strcat( buff, line );
        break;

    case MOVETO:
    case TELEOPERATE:
    case SWEEPTO:   // ??? Will probably need a separate case eventually
        sprintf( line,"UNIT %s %s %s FORMATION %s %s", 
                 step->unit, gt_behavior_name[ cmd->behavior ], step->location,
                 gt_formation_name[ cmd->formation ],
                 gt_technique_name[ cmd->technique ]);
        strcat( buff, line );
     
        if ( cmd->speed != 0.0 )
        {
            sprintf( line," SPEED=%1.4g", cmd->speed );
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        if ( cmd->phase_line_given )
        {
            sprintf( line, " PHASE-LINE %s", step->phase_line_name );
            if ( cmd->time )
            {
                tm* time;
                char time_str[ 100 ];
                time = localtime( (time_t*) &( cmd->time ) );
                strftime( time_str, 100, " %m-%d-%y %T", time );
                strcat( line, time_str );
            }
            if ( cmd->phase_line_ack_msg )
            {
                char ack_str[ 256 ];
                sprintf( ack_str, " ACK \"%s\"", cmd->phase_line_ack_msg );
                strcat( line, ack_str );
            }
            if ( cmd->wait_at_phase_line )
            {
                strcat( line, " WAIT" );
            }
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        if ( cmd->completion_given )
        {
            sprintf( line, " ON-COMPLETION MSG \"%s\"", cmd->completion_msg );
            if ( cmd->freeze_on_completion )
            {
                strcat( line, " FREEZE" );
            }
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        break;

    case FOLLOW:
        sprintf( line,"UNIT %s FOLLOW %s FORMATION %s %s", step->unit, step->location, 
                 gt_formation_name[ cmd->formation ],
                 gt_technique_name[ cmd->technique ] );
        strcat( buff, line );
     
        if ( cmd->speed != 0.0 )
        {
            sprintf( line, " SPEED=%1.4g", cmd->speed );
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        if ( cmd->phase_line_given )
        {
            sprintf( line," PHASE-LINE %s", step->phase_line_name );
            if ( cmd->time )
            {
                tm* time;
                char time_str[ 100 ];
                time = localtime( (time_t*) &( cmd->time ) );
                strftime( time_str, 100, " %m-%d-%y %T", time );
                strcat( line, time_str );
            }
            if ( cmd->phase_line_ack_msg )
            {
                char ack_str[ 256 ];
                sprintf( ack_str, " ACK \"%s\"", cmd->phase_line_ack_msg );
                strcat( line, ack_str );
            }
            if ( cmd->wait_at_phase_line )
            {
                strcat( line, " WAIT" );
            }
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        if ( cmd->completion_given )
        {
            sprintf( line, " ON-COMPLETION MSG \"%s\"", cmd->completion_msg );
            if ( cmd->freeze_on_completion )
            {
                strcat( line, " FREEZE" );
            }
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        break;

    case OCCUPY:
        sprintf( line, "UNIT %s OCCUPY %s FORMATION %s",
                 step->unit, step->location, gt_formation_name[ cmd->formation ] );
        strcat( buff, line );
      
        if ( cmd->until_timeout_given )
        {
            if ( cmd->time > 0 )
            {
                // occupy until some set final time
                tm* time;
                char time_str[ 100 ];
                time = localtime( (time_t*) &( cmd->time ) );
                strftime( time_str, 100, "%m-%d-%y %T", time );
                sprintf( line," UNTIL %s", time_str );
            }
            else
            {
                // occupy until a timeout occurs
                int time, hours, minutes, seconds;
                time = abs( cmd->time );
                hours = time / 3600;
                minutes = ( time - 3600 * hours) / 60;
                seconds = time % 60;
                sprintf( line, " UNTIL TIMEOUT %02d:%02d:%02d", hours, minutes, seconds );
            }
            strcat_maybe_wrap( buff, line, "   ", DISPLAY_MAX_LEN );
        }

        break;

    case STOP:
        sprintf( line,"UNIT %s STOP", step->unit );
        strcat( buff, line );
        break;

    case QUIT:
        sprintf( line, "QUIT" );
        strcat( buff, line );
        break;

    case SET:
        switch ( gt_parameter_type[ step->parameter_to_set ] )
        {
        case BOOLEAN_PARAMETER:
            sprintf( line, "SET %s %s", gt_parameter_name[ step->parameter_to_set ],
                     step->parameter_value.i ? "ON" : "OFF" );
            break;

        case NUMBER_PARAMETER:
            sprintf( line, "SET %s %1.8g", gt_parameter_name[ step->parameter_to_set ],
                     step->parameter_value.d );
            break;
	  
        default:
            warn_userf( "Error in SET %s command, unknown type=%d",
                        gt_parameter_name[ step->parameter_to_set ], 
                        gt_parameter_type[ step->parameter_to_set ] );
            return;
        }
        strcat( buff, line );
        break;

    case PRINT:
        sprintf( line, "PRINT %s", gt_print_name[ step->print_what ] );
        strcat( buff, line );
        break;

    default:
        fprintf( stderr, "display_command: Error, unrecognized " );
        fprintf( stderr, "command behavior = %d\n", cmd->behavior );
        break;
    }
}

/**********************************************************************/

static void execute_step_list()
{
    if ( executing_step_list )
    {
        if ( paused )
        {
            toggle_pause_execution_button();
        }
        else
        {
            warn_user( "Already executing command list!" );
        }
    }
    else
    {
        gt_execute_step_list();
    }
}

/**********************************************************************/

static void execute_user_command()
{
    char* cmd;

    if ( user_command_field_touched )
    {
        cmd = XmTextFieldGetString( user_command_field );
        if ( ( cmd == NULL ) || ( strlen( cmd ) == 0 ) )
        {
            warn_user( "Unable to execute command: No command entered yet!" );
        }
        else
        {
            gt_execute_user_command( cmd );
        }
        XtFree( cmd );
    }
    else
    {
        warn_user("Unable to execute command: No command entered yet!");
    }
}

/**********************************************************************/

static void forward_step_list()
{
    gt_forward_step_list();
}

/**********************************************************************/

static void hide_command_panel()
{
    XtPopdown( command_panel_shell );
}

/**********************************************************************/

static void load_command_file()
{
    char msg[ 256 ];
    char* filename;
    if ( command_filename_field_touched )
    {
        if ( executing_step_list )
        {
            warn_user( "Executing commands now!  Wait for\n"
                       "completion or ABORT before loading files." );
            return;
        }

        // make sure we have a filename to read from
        filename = XmTextFieldGetString( command_filename_field );
        if ( ( filename == NULL ) || ( strlen( filename ) == 0 ) )
        {
            warn_user( "Commands NOT loaded successfully: No filename entered yet!" );
            if ( filename ) 
            {
                XtFree( filename );
            }
            return;
        }

        // load the commands
        if ( gt_load_commands( filename ) != 0 )
        {
            sprintf( msg, "Commands NOT loaded successfully from\n file: %s!",
                     filename );
            warn_user( msg );
        }
        set_file_open_dialog_filename( filename );
        XtFree( filename );
        gt_rewind_step_list();
    }
    else
    {
        warn_user( "Unable to load file: No filename entered yet!\n" );
    }
}

/**********************************************************************/

static void rewind_step_list()
{
    gt_rewind_step_list();
}




/**********************************************************************/

static void single_step_command()
{
    gt_execute_next_step();
}


/**********************************************************************/

static void single_step_robot_command()
{
    if ( paused )
    {
        send_robots_step_one_cycle( 1 );
    }
    else
    {
        if ( executing_step_list )
        {
            warn_user( "Warning: Press [Pause] first.  This button single steps\n"
                       "         the robots through one sensing-action cycle." );
        }
        else
        {
            warn_user( "Warning: You must be executing commands before this button\n"
                       "         can be used.  Then press [Pause].  Then this \n"
                       "         button single steps the robots through one \n"
                       "         sensing-action cycle." );
        }
    }
}

/**********************************************************************/

static void strcat_maybe_wrap( char* buff, char* str, char* indent, int max_width )
{
    // copy the string from 'str' into 'buff', wrapping first if necessary

    // complain if we don't have a buffer or string
    if ( buff == NULL )
    {
        fprintf( stderr, "Error in strcat_maybe_wrap: no buff!\n" );
        return;
    }
    if ( str == NULL )
    {
        fprintf( stderr, "Error in strcat_maybe_wrap: no str!\n" );
        return;
    }

    // figure out how long the last line of 'buff' is
    int line_len;
    int len = strlen( buff );
    for ( int i = len - 1; i >= 0; i-- )
    {
        if ( ( buff[ i ] == '\n' ) || ( i == 0 ) )
        {
            line_len = len - i;
            break;
        }
    }

    // wrap, if necessary
    if ( ( max_width - line_len ) < (int)(strlen( str )) )
    {
        strcat( buff, "\n" );
        if ( indent )
        {
            strcat( buff, indent );
        }
    }

    // copy in the string
    strcat( buff, str );
}

/**********************************************************************/
void toggle_pause_execution_button()
{
    gt_toggle_pause_execution();
    update_pause_button();
}

/**********************************************************************
 * $Log: gt_command_panel.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.3  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/11/08 17:14:41  endo
 * Memory leak problem solved.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.65  2003/04/02 22:10:56  zkira
 * Connected pause button to telop emergency stop button
 *
 * Revision 1.64  2002/07/02 20:40:17  blee
 * added EnableCommandPanelPauseButton()
 *
 * Revision 1.63  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.62  2001/12/22 16:20:28  endo
 * RH 7.1 porting.
 *
 * Revision 1.61  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.60  1996/02/29  01:53:18  doug
 * *** empty log message ***
 *
 * Revision 1.59  1995/06/30  18:53:18  jmc
 * Modified command panel to allow bottom part to handle resizes
 * better.
 *
 * Revision 1.58  1995/05/18  21:17:40  jmc
 * Added the function update_pause_button() and modified existing
 * toggle_pause function to use it.  Renamed toggle_pause_execution()
 * to toggle_pause_execution_button() to facilitate later removal of
 * gt_ prefixes.
 *
 * Revision 1.57  1995/04/26  18:55:11  jmc
 * Added handling for the TELEOPERATE behavior.
 *
 * Revision 1.56  1995/04/26  13:54:18  jmc
 * Added a call to set_file_open_dialog_filename() when new files are
 * loaded.
 *
 * Revision 1.55  1995/04/26  12:44:51  jmc
 * Added function set_command_panel_filename.  Changed global variables
 * command_filename_field and command_filename_field_touched to local
 * variables, since these are both taken care of in this module by the new
 * function.
 *
 * Revision 1.54  1995/04/25  21:54:53  jmc
 * Modified some of the user warnings to put the filename on a
 * separate line since they can be very long.
 *
 * Revision 1.53  1995/04/20  17:56:13  jmc
 * Corrected spelling.
 *
 * Revision 1.52  1995/04/19  20:38:11  jmc
 * Improved warning messages for pressing [Single Step Robots] button
 * in the wrong context.
 *
 * Revision 1.51  1995/04/03  19:31:03  jmc
 * Added copyright notice.
 *
 * Revision 1.50  1995/02/15  19:26:02  jmc
 * Modifications to display_step to handle the new SWEEPTO command.
 *
 * Revision 1.49  1995/02/13  20:43:07  jmc
 * Spelling fix.
 *
 * Revision 1.48  1995/02/13  18:54:08  jmc
 * Modified display_step to convert parameter references to allow
 * parameters to be either boolean or numbers (float point).
 *
 * Revision 1.47  1994/11/21  17:56:32  jmc
 * Moved checks on execute_step_list to here (from gt_command.c).
 * Extended functionality slightly.  Now pressing [Execute Commands]
 * while paused (and executing step list) unpauses.  Made
 * Pause/Resume usable while single-stepping.
 *
 * Revision 1.46  1994/11/15  21:49:44  jmc
 * Fixed a minor bug in display_step.
 *
 * Revision 1.45  1994/11/14  17:48:01  jmc
 * Added some bullet-proofing on several buttons (Pause,
 * Single-Step-Robot, etc).
 *
 * Revision 1.44  1994/11/14  16:09:16  jmc
 * Trivial typo correction.
 *
 * Revision 1.43  1994/11/14  15:29:21  jmc
 * Added a "Next Step" window.  Modified display_step to show both
 * the current step and the next step (in their separate windows).
 *
 * Revision 1.42  1994/11/11  22:58:39  jmc
 * Changed abort_command_execution to kill the robots if not
 * executing the step list (instead of complaining) so that it should
 * do the right thing when single stepping.
 *
 * Revision 1.41  1994/11/11  22:03:13  jmc
 * Tweaked name of single-step-robots button.
 *
 * Revision 1.40  1994/11/11  18:21:58  jmc
 * Minor hack to step display label.
 *
 * Revision 1.39  1994/11/11  16:59:33  jmc
 * Cleaned up minor problems (unused variables, etc) to get rid of
 * make -Wall problems.
 *
 * Revision 1.38  1994/11/03  23:17:16  jmc
 * Rearranged the command execution buttons and fiddled with their
 * labels to make room for a new button.  Added a [Single Step Robot
 * cycle] button.  Added single_step_robot_command function to be
 * invoked by the new button (when paused only).
 *
 * Revision 1.37  1994/11/03  15:11:00  jmc
 * Added printing of FAILED_STEP to gt_display_step.
 *
 * Revision 1.36  1994/11/02  00:34:04  jmc
 * Improved the resource names of the form widgets for the command
 * panel.
 *
 * Revision 1.35  1994/11/02  00:08:54  jmc
 * Fixed a typo in the gt_create_command_panel.
 *
 * Revision 1.34  1994/10/31  17:18:42  jmc
 * Added print ABORTED_STEP step status to gt_display_step.
 *
 * Revision 1.33  1994/10/28  20:08:46  jmc
 * Modifications necessary because until_given was changed to
 * until_timeout_given in gt_Command.
 *
 * Revision 1.32  1994/10/28  18:32:37  jmc
 * Modified display_command to handle the new syntax for the OCCUPY
 * UNTIL command.
 *
 * Revision 1.31  1994/10/27  16:29:00  jmc
 * Minor tweaks to some warning messages.
 *
 * Revision 1.30  1994/10/25  15:48:49  jmc
 * Added PRINT command to display_command.  Also the order of SET and
 * QUIT commands.
 *
 * Revision 1.29  1994/10/18  22:42:24  jmc
 * Minor tweak to error message.
 *
 * Revision 1.28  1994/10/18  22:35:48  jmc
 * Added warning (if not executing_step_list) from the function
 * abort_command_execution.  Disallow invoking load_command_file when
 * things are executing.  Added warning to that effect.
 *
 * Revision 1.27  1994/09/03  14:53:21  jmc
 * Removed initial position of command interface from
 * gt_create_command_panel and moved it to the fallback_resources in
 * console.c.
 *
 * Revision 1.26  1994/08/29  22:57:44  jmc
 * Tweaked display_command.
 *
 * Revision 1.25  1994/08/29  22:54:52  jmc
 * Added strcat_maybe_wrap function and used it to improve the
 * display of steps using the display_step and display_command
 * functions.
 *
 * Revision 1.24  1994/08/29  17:50:18  jmc
 * Modified display_command to display SET and QUIT commands.  Also
 * modified it to pass the step as part of the command since I had to
 * put more command-like things into the step structure and I didn't
 * want to add any more arguments to pass in those extras.
 *
 * Revision 1.23  1994/08/25  21:55:03  jmc
 * Make command_filename* variables globally visible so they can be
 * changed in console.c when startup files are loaded.
 *
 * Revision 1.22  1994/08/23  15:34:11  jmc
 * Implemented the pause button.
 *
 * Revision 1.21  1994/08/18  20:57:25  jmc
 * Worked on display_command function to improve printing of phase
 * line info.
 *
 * Revision 1.20  1994/08/17  22:57:33  doug
 * fixing rpc code
 *
 * Revision 1.19  1994/08/16  20:57:10  jmc
 * Minor modifications because of change in definition of gt_Step.
 *
 * Revision 1.18  1994/08/16  15:59:31  jmc
 * Changed command_list to step_list.
 *
 * Revision 1.17  1994/08/12  15:16:58  jmc
 * Added printing speed (if present).  Widened the display area a
 * little to make room to print the speed.
 *
 * Revision 1.16  1994/08/05  17:22:26  jmc
 * Modified display_command to use the new format for DATE TIME for
 * the phase line spec.
 *
 * Revision 1.15  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.14  1994/08/03  14:02:18  jmc
 * Modified display_step to handle a NULL step.  (Display "No Step:"
 * and clear the step display area.)
 *
 * Revision 1.13  1994/07/28  22:12:17  jmc
 * Added functions, widgets, and buttons for command list movement
 * and execution buttons: [<] [>] [Pause/Resume].  Added a step
 * status label above step display area.  Modified display_step to
 * also display the appropriate status using that label.
 *
 * Revision 1.12  1994/07/26  21:00:21  jmc
 * Changed default display message in user_command_field.
 *
 * Revision 1.11  1994/07/25  21:32:03  jmc
 * Tweaked an error message.
 * Hardcoded default screen position for command_panel.
 *
 * Revision 1.10  1994/07/22  21:52:47  jmc
 * Changed most error messages to use the error dialog box.
 *
 * Revision 1.9  1994/07/20  14:47:46  jmc
 * Added location to display of START command.
 *
 * Revision 1.8  1994/07/20  14:39:45  jmc
 * Changed "destination" to "location".
 *
 * Revision 1.7  1994/07/19  21:26:08  jmc
 * Removed unused variables for clean compiling.
 *
 * Revision 1.6  1994/07/19  15:58:59  jmc
 * Removed setting background to grey80; should do this in the .Xdefaults file.
 *
 * Revision 1.5  1994/07/12  19:14:43  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.4  1994/07/12  19:01:37  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.3  1994/07/12  17:12:50  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
