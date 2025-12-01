/**********************************************************************
 **                                                                  **
 **                          VectorFieldDlg.c                        **
 **                                                                  **
 **  Written by:  Brian Lee                                          **
 **                                                                  **
 **  Copyright 2002 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: VectorFieldDlg.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $*/

#include <stdio.h>

#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/PushBG.h>
#include <Xm/Label.h>
#include <Xm/Scale.h>

#include "gt_sim.h"
#include "console.h"
#include "VectorFieldDlg.h"

const int g_iDISTANCE_SCALE   = 100;
const int g_iRESOLUTION_SCALE = 10;

// constructor
ClVectorFieldDlg::ClVectorFieldDlg( Widget widgetParent, ClVectorFieldDlg** ppobjThis, 
                                    ClVectorField* pobjVectorField )
{
    m_widgetParent = widgetParent;
    m_ppobjThis = ppobjThis;
    m_pobjVectorField = pobjVectorField;
    m_iRobotID = ( pobjVectorField != NULL ) ? pobjVectorField->GetRobotID() : -1;

    assert( m_pobjVectorField != NULL );
    m_pobjVectorField->SetStopDrawing( false );

    m_widgetDrawFieldBtn   = NULL;
    m_widgetStopDrawingBtn = NULL;
}

// destructor
ClVectorFieldDlg::~ClVectorFieldDlg()
{
    if ( m_pobjVectorField != NULL )
    {
        m_pobjVectorField->SetStopDrawing( true );
    }

    // make sure the shell is destroyed
    if ( m_widgetDlgShell != NULL )
    {
        XtPopdown( m_widgetDlgShell );
        m_widgetDlgShell = NULL;
    }

    // if we know what pointer is pointing to us, set it to NULL since we're going away
    if ( ( m_ppobjThis != NULL ) && ( *m_ppobjThis != NULL ) )
    {
        *m_ppobjThis = NULL;
    }
}

// this sets up and displays the dialog
bool ClVectorFieldDlg::Create()
{
    char szLabelTemp[ 100 ];
    XmString strText;

    // make sure we have the things we need
    if ( ( m_widgetParent == NULL ) || 
         ( m_pobjVectorField == NULL ) )
    {
        return false;
    }

    // create the popup
    m_widgetDlgShell = XtVaCreatePopupShell( 
        "Vector Field Options", xmDialogShellWidgetClass, m_widgetParent, 
        XmNdeleteResponse, XmDESTROY,
        NULL );

    // create a pane to manage the stuff in the object
    Widget widgetPane = XtVaCreateWidget( 
        "pane", xmPanedWindowWidgetClass, m_widgetDlgShell, 
        XmNsashWidth,  1, 
        XmNsashHeight, 1, 
        NULL );

    // create a form to be the control area
    Widget widgetControlForm = XtVaCreateWidget( 
        "control-area", xmFormWidgetClass, widgetPane, 
        XmNpacking, XmPACK_TIGHT, 
        XmNnumColumns, 1, 
        XmNorientation, XmHORIZONTAL, 
        NULL );

    // create the slider that sets the distance to the field edge
    strText = XmStringCreateLocalized( "Distance to Field Edge (meters)" );
    m_widgetDistanceSlider = XtVaCreateManagedWidget( 
        "distance-slider", xmScaleWidgetClass, widgetControlForm,
        XmNtitleString,           strText, 
        XmNminimum,               1,
        XmNmaximum,               100 * g_iDISTANCE_SCALE,
        XmNvalue,                 (int)( m_pobjVectorField->GetEdgeDistance() + 0.5 ) * g_iDISTANCE_SCALE,
        XmNshowValue,             True,
        XmNdecimalPoints,         2,
        XmNorientation,           XmHORIZONTAL,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL );
    XmStringFree( strText );

    // create the slider that sets the field resolution
    strText = XmStringCreateLocalized( "Field Resolution (meters)" );
    m_widgetResolutionSlider = XtVaCreateManagedWidget( 
        "resolution-slider", xmScaleWidgetClass, widgetControlForm,
        XmNtitleString,           strText, 
        XmNminimum,               1,
        XmNmaximum,               5 * g_iRESOLUTION_SCALE,
        XmNvalue,                 (int)( m_pobjVectorField->GetResolution() + 0.5 ) * g_iRESOLUTION_SCALE,
        XmNshowValue,             True,
        XmNdecimalPoints,         1,
        XmNorientation,           XmHORIZONTAL,
        XmNtopAttachment,         XmATTACH_WIDGET,
        XmNtopWidget,             m_widgetDistanceSlider,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL );
    XmStringFree( strText );

    // create the robot ID label
    sprintf( szLabelTemp, "Robot ID: %d", m_iRobotID );
    strText = XmStringCreateLocalized( szLabelTemp );
    m_widgetRobotIdLabel = XtVaCreateManagedWidget( 
        "robot-id", xmLabelWidgetClass, widgetControlForm, 
        XmNtopAttachment,   XmATTACH_WIDGET, 
        XmNtopWidget,       m_widgetResolutionSlider, 
        XmNleftAttachment,  XmATTACH_FORM, 
        XmNrecomputeSize,   False, 
        XmNlabelString,     strText, 
        NULL );
    XmStringFree( strText );

    // create the "next robot" button
    Widget widgetNextRobotBtn = XtVaCreateManagedWidget( 
        ">>", xmPushButtonGadgetClass, widgetControlForm,
        XmNtopAttachment,    XmATTACH_WIDGET, 
        XmNtopWidget,        m_widgetResolutionSlider, 
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_WIDGET,
        XmNleftWidget,       m_widgetRobotIdLabel, 
        XmNdefaultButtonShadowThickness, 1, 
        NULL );
    XtAddCallback( widgetNextRobotBtn, XmNactivateCallback, NextRobotCallback, this );

    XtManageChild( widgetControlForm );

    // create the action area form
    Widget widgetActionForm = XtVaCreateWidget( 
        "action-area", xmFormWidgetClass, widgetPane, 
        XmNfractionBase, 10, 
        NULL );

    // create the "Draw Field" button
    m_widgetDrawFieldBtn = XtVaCreateManagedWidget( 
        "Draw Field", xmPushButtonGadgetClass, widgetActionForm,
        XmNtopAttachment,    XmATTACH_FORM, 
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_POSITION,
        XmNleftPosition,     1, 
        XmNrightAttachment,  XmATTACH_POSITION,
        XmNrightPosition,    3, 
        XmNdefaultButtonShadowThickness, 1, 
        NULL );
    XtAddCallback( m_widgetDrawFieldBtn, XmNactivateCallback, DrawFieldCallback, this );

    // create the "Stop Drawing" button
    m_widgetStopDrawingBtn = XtVaCreateManagedWidget( 
        "Stop Drawing", xmPushButtonGadgetClass, widgetActionForm,
        XmNtopAttachment,    XmATTACH_FORM, 
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_POSITION,
        XmNleftPosition,     4, 
        XmNrightAttachment,  XmATTACH_POSITION,
        XmNrightPosition,    6, 
        XmNdefaultButtonShadowThickness, 1, 
        NULL );
    XtAddCallback( m_widgetStopDrawingBtn, XmNactivateCallback, StopDrawingCallback, this );

    // create the "Close" button
    Widget widgetCloseBtn = XtVaCreateManagedWidget( 
        "Close", xmPushButtonGadgetClass, widgetActionForm,
        XmNtopAttachment,    XmATTACH_FORM, 
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_POSITION,
        XmNleftPosition,     7, 
        XmNrightAttachment,  XmATTACH_POSITION,
        XmNrightPosition,    9, 
        XmNdefaultButtonShadowThickness, 1, 
        NULL );
    XtAddCallback( widgetCloseBtn, XmNactivateCallback, CloseCallback, this );
    XtAddCallback( widgetCloseBtn, XmNdestroyCallback,  CloseCallback, this );

    XtManageChild( widgetActionForm );
    XtManageChild( widgetPane );

    // display the dialog
    XtPopup( m_widgetDlgShell, XtGrabNone );

    return true;
}

// this should totally destroy the dialog
void ClVectorFieldDlg::Destroy()
{
    if ( m_pobjVectorField != NULL )
    {
        m_pobjVectorField->SetStopDrawing( true );
    }

    // if the shell exists, kill it
    if ( m_widgetDlgShell != NULL )
    {
        XtPopdown( m_widgetDlgShell );
        m_widgetDlgShell = NULL;
    }

    // only invoke the destructor if we have a pointer to ourselves 
    //  (thereby implying we were dynamically allocated)
    if ( ( m_ppobjThis != NULL ) && ( *m_ppobjThis != NULL ) )
    {
        delete this;
    }
}

// return the parent
Widget ClVectorFieldDlg::GetParent()
{
    return m_widgetParent;
}

// raise the window
void ClVectorFieldDlg::Raise()
{
    if ( m_widgetDlgShell != NULL )
    {
        XRaiseWindow( XtDisplay( m_widgetDlgShell ), XtWindow( m_widgetDlgShell ) );
    }
}

// this starts the process of drawing the vector field
void ClVectorFieldDlg::DrawVectorField()
{
    assert( m_pobjVectorField != NULL );

    // only draw if the console is paused
    if ( paused )
    {
        // get the distance to the field edge
        int iDistance;
        XmScaleGetValue( m_widgetDistanceSlider, &iDistance );
        double dDistance = (double) iDistance / (double) g_iDISTANCE_SCALE;

        // get the field resolution
        int iResolution;
        XmScaleGetValue( m_widgetResolutionSlider, &iResolution );
        double dResolution = (double) iResolution / (double) g_iRESOLUTION_SCALE;

        UpdateControlsForDrawing( true );

        // give the vector field the new attributes and draw it
        m_pobjVectorField->SetMissionArea( mission_area_width_meters, mission_area_height_meters );
        m_pobjVectorField->SetStopDrawing( false );
        m_pobjVectorField->DrawVectorField( m_iRobotID, dDistance, dResolution );

        UpdateControlsForDrawing( false );
    }
    else
    {
        warn_user( "The console must be paused to\ntake advantage of this feature." );
    }
}

// update the dialog to represent the next robot
void ClVectorFieldDlg::GetNextRobotID()
{
    m_iRobotID = ::GetNextRobotID( m_iRobotID );

    char szTemp[ 100 ];
    sprintf( szTemp, "Robot ID: %d", m_iRobotID );
    XmString strLabel = XmStringCreateLocalized( szTemp );

    Arg args[ 1 ];
    XtSetArg( args[ 0 ], XmNlabelString, strLabel );
    XtSetValues( m_widgetRobotIdLabel, args, 1 );

    XmStringFree( strLabel );
}

void ClVectorFieldDlg::SetStopDrawing( bool bStopDrawing )
{
    m_pobjVectorField->SetStopDrawing( bStopDrawing );
}

void ClVectorFieldDlg::UpdateControlsForDrawing( bool bDrawing )
{
    assert( m_widgetDrawFieldBtn   != NULL );
    assert( m_widgetStopDrawingBtn != NULL );

    XtVaSetValues( m_widgetDrawFieldBtn,   XmNsensitive, !bDrawing, NULL );
    XtVaSetValues( m_widgetStopDrawingBtn, XmNsensitive,  bDrawing, NULL );
}

// callback for the "next robot" button
void ClVectorFieldDlg::NextRobotCallback( Widget widgetNextBtn, 
                                          XtPointer client_data, XtPointer call_data )
{
    ClVectorFieldDlg* pobjCallingObj = (ClVectorFieldDlg*) client_data;

    if ( pobjCallingObj != NULL )
    {
        pobjCallingObj->GetNextRobotID();
    }
}

// callback for the "draw field" button
void ClVectorFieldDlg::DrawFieldCallback( Widget widgetDrawBtn, 
                                          XtPointer client_data, XtPointer call_data )
{
    ClVectorFieldDlg* pobjCallingObj = (ClVectorFieldDlg*) client_data;

    if ( pobjCallingObj != NULL )
    {
        pobjCallingObj->DrawVectorField();
    }
}

// callback for the "draw field" button
void ClVectorFieldDlg::StopDrawingCallback( Widget widgetDrawBtn, 
                                            XtPointer client_data, XtPointer call_data )
{
    ClVectorFieldDlg* pobjCallingObj = (ClVectorFieldDlg*) client_data;

    if ( pobjCallingObj != NULL )
    {
        pobjCallingObj->SetStopDrawing( true );
    }
}

// callback for the "close" callback
void ClVectorFieldDlg::CloseCallback( Widget widgetCloseBtn, 
                                      XtPointer client_data, XtPointer call_data )
{
    ClVectorFieldDlg* pobjCallingObj = (ClVectorFieldDlg*) client_data;

    if ( pobjCallingObj != NULL )
    {
        pobjCallingObj->Destroy();
    }
}

/**********************************************************************
 * $Log: VectorFieldDlg.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2002/07/02 20:47:42  blee
 * Initial revision
 *
 *
 **********************************************************************/
