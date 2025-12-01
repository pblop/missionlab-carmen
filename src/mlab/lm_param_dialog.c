#include <stdlib.h>
#include <stdio.h>
#include <Xm/DialogS.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/PushBG.h>
#include <Xm/Label.h>
#include <Xm/RowColumn.h>

#include "gt_sim.h"
#include "lm_param_dialog.h"
#include "console.h"

void InitializeHistory( SuLMParams asuParamHistory[] );
Widget GetTopShell( Widget w );
Widget CreateDialog( Widget parent, SuLMViewDialog* psuDialog );
void DrawingAreaCallback( Widget drawingArea, 
                          XtPointer client_data, XtPointer call_data );
void DoneCallback( Widget doneButton, 
                   XtPointer client_data, XtPointer call_data );
void NextCallback( Widget nextButton, 
                   XtPointer client_data, XtPointer call_data );
void DrawTheWindow( SuLMViewDialog* psuDialog );
void GetMaxAndMinParamValues( SuLMParams* psuMaxValues, SuLMParams* psuMinValues, 
                              const SuLMParams asuParamHistory[] );
void DrawDivisions( Widget drawingArea, Pixmap pixmap, GC gc );
void DrawGraphs( SuLMViewDialog* psuDialog, SuLMParams suMaxParams, 
                 SuLMParams suMinParams, Pixmap pixmap, GC gc );
int GetDistFromTop( float fMax, float fMin, float fValue, int iRange );
int GetDistFromTop( int iMax, int iMin, int iValue, int iRange );
void UpdateLabels( SuLMViewDialog* psuDialog,  SuLMParams suMaxParams, SuLMParams suMinParams );
void UpdateLabelSet( Widget aLabels[], float fMaxValue, float fCurrentValue, float fMinValue );
void UpdateLabel( Widget label, const char* szFormat, float fValue );

const int iTEXT_OFFSET = 0;   // offset for labels

const int iGOAL_LABEL_SET               = 0;
const int iOBSTACLE_LABEL_SET           = 1;
const int iOBSTACLE_SPHERE_LABEL_SET    = 2;
const int iWANDER_LABEL_SET             = 3;
const int iWANDER_PERSISTENCE_LABEL_SET = 4;

const int iTITLE_OFFSET         = 0;
const int iMAX_VALUE_OFFSET     = 1;
const int iCURRENT_VALUE_OFFSET = 2;
const int iMIN_VALUE_OFFSET     = 3;

int InitializeLMParamDialog( SuLMViewDialog* psuDialog )
{
    int iSuccess = FALSE;

    if ( psuDialog != NULL )
    {
        // set all values to zero initially
        InitializeHistory( psuDialog->asuParamHistory );

        // make sure the first index to be updated is the zero index
        psuDialog->iLastUpdatedIndex = LM_PARAM_HISTORY_SIZE - 1;

        // set the robot ID to default -1
        psuDialog->iRobotID = -1;

        iSuccess = TRUE;
    }

    return iSuccess;
}

int SetLMParamDialogRobotID( SuLMViewDialog* psuDialog, int iRobotID )
{
    int iSuccess = FALSE;

    if ( psuDialog != NULL )
    {
        psuDialog->iRobotID = iRobotID;
        iSuccess = TRUE;
    }

    return iSuccess;
}

const Widget GetLMParamDialog( const SuLMViewDialog* psuDialog )
{
    Widget dialog = NULL;

    if ( psuDialog != NULL )
    {
        dialog = psuDialog->dialog;
    }

    return dialog;
}

int GetLMParamRobotID( const SuLMViewDialog* psuDialog )
{
    int iRobotID = -1;

    if ( psuDialog != NULL )
    {
        iRobotID = psuDialog->iRobotID;
    }

    return iRobotID;
}

int GetLastUpdatedLMParamIndex( const SuLMViewDialog* psuDialog )
{
    int iIndex = -1;

    if ( psuDialog != NULL )
    {
        iIndex = psuDialog->iLastUpdatedIndex;
    }

    return iIndex;
}

int UpdateLMParamDialogValue( SuLMViewDialog* psuDialog, SuLMParams suValues )
{
    int iSuccess = FALSE;

    if ( psuDialog != NULL )
    {
        // increment the list updated index w/ wrap-around to zero
        psuDialog->iLastUpdatedIndex = 
          ( psuDialog->iLastUpdatedIndex + 1 ) % LM_PARAM_HISTORY_SIZE;

        psuDialog->asuParamHistory[ psuDialog->iLastUpdatedIndex ] = suValues;
        iSuccess = TRUE;
    }

    return iSuccess;
}

int PopupLMParamDialog( SuLMViewDialog* psuDialog, Widget parent )
{
    int iSuccess = FALSE;

    if ( psuDialog != NULL )
    {
        psuDialog->dialog = CreateDialog( parent, psuDialog );

        if ( psuDialog->dialog != NULL )
        {
            iSuccess = TRUE;
        }
    }

    return iSuccess;
}

Widget CreateDialog( Widget parent, SuLMViewDialog* psuDialog )
{
    const int iDRAWING_AREA_HEIGHT = 460;
    const int iDRAWING_AREA_WIDTH  = 300;

    const char* aszGRAPH_LABELS[] = { "Move to goal gain",
                                      "Avoid obstacle gain",
                                      "Avoid obstacle sphere",
                                      "Wander gain",
                                      "Wander persistence" };

    int i, j;
    Widget newDialog, pane, controlForm, drawingArea, tempWidget, 
           drawingAreaForm, labelForm, actionForm;

    newDialog = XtVaCreatePopupShell("Learning Momentum Parameters",
      xmDialogShellWidgetClass, GetTopShell( parent ), 
      XmNheight,           500, 
      XmNwidth,            500, 
      XmNallowShellResize, True, 
      XmNdeleteResponse,   XmDESTROY,
      NULL);

    // create a pane to manage the stuff in the object
    pane = XtVaCreateWidget( "pane", xmPanedWindowWidgetClass, newDialog, 
                             XmNsashWidth,  1, 
                             XmNsashHeight, 1, 
                             NULL );

    // create a form to be the control area
    controlForm = XtVaCreateWidget( "control-area", xmRowColumnWidgetClass, pane, 
                                    XmNpacking, XmPACK_TIGHT, 
                                    XmNnumColumns, 1, 
                                    XmNorientation, XmHORIZONTAL, 
                                    NULL );

    // create the labels
    labelForm = XtVaCreateWidget( "label-form", xmRowColumnWidgetClass, controlForm, 
                                  XmNtopAttachment,    XmATTACH_FORM, 
                                  XmNbottomAttachment, XmATTACH_FORM, 
                                  XmNpacking,          XmPACK_COLUMN, 
                                  XmNnumColumns,       1, 
                                  NULL );

    Widget graphLabelForm;
    for ( i = 0; i < LM_PARAM_GRAPH_COUNT; i++ )
    {
        char szTemp[ 30 ];
        sprintf( szTemp, "graph-label-form%d", i );
        graphLabelForm = XtVaCreateWidget( szTemp, xmRowColumnWidgetClass, labelForm, 
                                           XmNpacking,    XmPACK_TIGHT, 
                                           XmNnumColumns, 1, 
                                           NULL );

        for ( j = 0; j < LM_PARAM_LABELS_PER_GRAPH; j++ )
        {
            const char* szLabel = "";

            sprintf( szTemp, "graph-label%d-%d", i, j );
            if ( j == iTITLE_OFFSET )
            {
                szLabel = aszGRAPH_LABELS[ i ];
            }
            else if ( j == iMAX_VALUE_OFFSET )
            {
                szLabel = "Max Value: ";
            }
            else if ( j == iCURRENT_VALUE_OFFSET )
            {
                szLabel = "Value: ";
            }
            else if ( j == iMIN_VALUE_OFFSET )
            {
                szLabel = "Min Value: ";
            }

            XmString strText = XmStringCreateLocalized( (char*) szLabel );
            psuDialog->aaLabels[ i ][ j ] = XtVaCreateWidget( 
                szTemp, xmLabelWidgetClass, graphLabelForm, 
                XmNleftAttachment,  XmATTACH_FORM, 
                XmNrightAttachment, XmATTACH_FORM, 
                XmNrecomputeSize,   False, 
                XmNlabelString,     strText, 
                NULL );
            XmStringFree( strText );
            XtManageChild( psuDialog->aaLabels[ i ][ j ] );
        }

        XtManageChild( graphLabelForm );
    }

    XtManageChild( labelForm );

    drawingAreaForm = XtVaCreateWidget( "drawing-area-form", xmFormWidgetClass, 
                                        controlForm, NULL );

    // make the drawing area
    drawingArea = XtVaCreateWidget( "drawing-area", xmDrawingAreaWidgetClass, 
                                    drawingAreaForm,
                                    XmNwidth,            iDRAWING_AREA_WIDTH, 
                                    XmNheight,           iDRAWING_AREA_HEIGHT, 
                                    XmNresizePolicy,     XmRESIZE_NONE,
                                    XmNleftAttachment,	 XmATTACH_FORM,
                                    XmNtopAttachment,	 XmATTACH_FORM,
                                    XmNrightAttachment,	 XmATTACH_FORM,
                                    XmNbottomAttachment, XmATTACH_FORM,
                                    NULL );

    psuDialog->drawingArea = drawingArea;

    XtAddCallback( drawingArea, XmNexposeCallback, DrawingAreaCallback, psuDialog );
    XtAddCallback( drawingArea, XmNresizeCallback, DrawingAreaCallback, psuDialog );

    XtManageChild( drawingArea );
    XtManageChild( drawingAreaForm );

    XtManageChild( controlForm );

    // create a form to be the action area
    actionForm = XtVaCreateWidget( "action-area", xmFormWidgetClass, pane, 
                             XmNfractionBase, 5, 
                             NULL );

    tempWidget = XtVaCreateManagedWidget( ">>", xmPushButtonGadgetClass, actionForm,
                                        XmNtopAttachment,    XmATTACH_FORM, 
                                        XmNbottomAttachment, XmATTACH_FORM,
                                        XmNleftAttachment,   XmATTACH_POSITION,
                                        XmNleftPosition,     1, 
                                        XmNrightAttachment,  XmATTACH_POSITION,
                                        XmNrightPosition,    2, 
                                        XmNdefaultButtonShadowThickness, 1, 
                                        NULL );

    XtAddCallback( tempWidget, XmNactivateCallback, NextCallback, NULL );

    tempWidget = XtVaCreateManagedWidget( "Close", xmPushButtonGadgetClass, actionForm,
                                        XmNtopAttachment,    XmATTACH_FORM, 
                                        XmNbottomAttachment, XmATTACH_FORM,
                                        XmNleftAttachment,   XmATTACH_POSITION,
                                        XmNleftPosition,     3, 
                                        XmNrightAttachment,  XmATTACH_POSITION,
                                        XmNrightPosition,    4,
                                        XmNshowAsDefault,    True,
                                        XmNdefaultButtonShadowThickness, 1, 
                                        NULL );

    XtAddCallback( tempWidget, XmNactivateCallback, DoneCallback, NULL );

    XtManageChild( actionForm );
    XtManageChild( pane );

    XtPopup( newDialog, XtGrabNone );

    return newDialog;
}

void InitializeHistory( SuLMParams asuParamHistory[] )
{
    int i;

    // set all values to zero
    for ( i = 0; i < LM_PARAM_HISTORY_SIZE; i++ )
    {
        asuParamHistory[ i ].fGoalGain          = -1.0;
        asuParamHistory[ i ].fAvoidObstacleGain = -1.0;
        asuParamHistory[ i ].fObstacleSphere    = -1.0;
        asuParamHistory[ i ].fWanderGain        = -1.0;
        asuParamHistory[ i ].iWanderPersistence =   -1;
    }
}

Widget GetTopShell( Widget w )
{
    while ( w && !XtIsWMShell( w ) )
    {
        w = XtParent( w );
    }

    return w;
}

int UpdateLMParamDialogDisplay( SuLMViewDialog* psuDialog )
{
    if ( psuDialog != NULL )
    {
        DrawTheWindow( psuDialog );
    }

    return ( psuDialog != NULL );
}

void DrawTheWindow( SuLMViewDialog* psuDialog )
{
    Dimension width, height;
    SuLMParams suMaxParams, suMinParams;
    Widget drawingArea = psuDialog->drawingArea;
    XGCValues xgcv;
    GC gc = XtGetGC( drawingArea, GCForeground, &xgcv );
    
    // drawing variables
    Display* pDisplay = XtDisplay( drawingArea );
    Window window = XtWindow( drawingArea );
    Screen* pScreen = XtScreen( drawingArea );

    // get the max and min values of each parameter
    GetMaxAndMinParamValues( &suMaxParams, &suMinParams, 
                             psuDialog->asuParamHistory );

    // update the label text
    UpdateLabels( psuDialog, suMaxParams, suMinParams );

    // get the dimensions of the drawing area
    XtVaGetValues( drawingArea, XtNwidth, &width, XtNheight, &height, NULL );

    // create a pixmap buffer to draw on
    Pixmap pixmap = XCreatePixmap( pDisplay, RootWindowOfScreen( pScreen ), 
                                   width, height, 
                                   DefaultDepthOfScreen( pScreen ) );

    // white out the drawing area
    XSetForeground( pDisplay, gc, WhitePixelOfScreen( pScreen ) );
    XFillRectangle( pDisplay, pixmap, gc, iTEXT_OFFSET, 0, width, height );

    // divide up the graphing area
    DrawDivisions( drawingArea, pixmap, gc );

    // draw all of the graphs
    DrawGraphs( psuDialog, suMaxParams, suMinParams, pixmap, gc );

    // copy the buffer to the display 
    XCopyArea( pDisplay, pixmap, window, gc, 0, 0, width, height, 0, 0 );

    // free the pixmap
    XFreePixmap( pDisplay, pixmap );

    // free the GC
    XtReleaseGC( drawingArea, gc );
}

void UpdateLabels( SuLMViewDialog* psuDialog, 
                   SuLMParams suMaxParams, SuLMParams suMinParams )
{
    //char szTemp[ 40 ];
    SuLMParams* psuParams;

    if ( ( psuDialog != NULL ) && ( psuDialog->iRobotID >= 0 ) )
    {
        psuParams = &( psuDialog->asuParamHistory[ psuDialog->iLastUpdatedIndex ] );

        UpdateLabelSet( psuDialog->aaLabels[ iGOAL_LABEL_SET ], suMaxParams.fGoalGain, 
                        psuParams->fGoalGain, suMinParams.fGoalGain );
        UpdateLabelSet( psuDialog->aaLabels[ iOBSTACLE_LABEL_SET ], 
                        suMaxParams.fAvoidObstacleGain,  psuParams->fAvoidObstacleGain, 
                        suMinParams.fAvoidObstacleGain );
        UpdateLabelSet( psuDialog->aaLabels[ iOBSTACLE_SPHERE_LABEL_SET ], 
                        suMaxParams.fObstacleSphere,  psuParams->fObstacleSphere, 
                        suMinParams.fObstacleSphere );
        UpdateLabelSet( psuDialog->aaLabels[ iWANDER_LABEL_SET ], suMaxParams.fWanderGain, 
                        psuParams->fWanderGain, suMinParams.fWanderGain );
        UpdateLabelSet( psuDialog->aaLabels[ iWANDER_PERSISTENCE_LABEL_SET ], 
                        (float) suMaxParams.iWanderPersistence, 
                        (float) psuParams->iWanderPersistence, 
                        (float) suMinParams.iWanderPersistence );
    }
}

void UpdateLabelSet( Widget aLabels[], float fMaxValue, float fCurrentValue, float fMinValue )
{
    UpdateLabel( aLabels[ iMAX_VALUE_OFFSET     ], "Max value: %f", fMaxValue     );
    UpdateLabel( aLabels[ iCURRENT_VALUE_OFFSET ], "Value: %f",     fCurrentValue );
    UpdateLabel( aLabels[ iMIN_VALUE_OFFSET     ], "Min value: %f", fMinValue     );
}

void UpdateLabel( Widget label, const char* szFormat, float fValue )
{
    char szTemp[ 40 ];
    XmString strLabel;
    Arg args[ 1 ];

    sprintf( szTemp, szFormat, fValue );
    strLabel = XmStringCreateLocalized( szTemp );

    XtSetArg( args[ 0 ], XmNlabelString, strLabel );
    XtSetValues( label, args, 1 );
    XmStringFree( strLabel );
}

void DrawGraphs( SuLMViewDialog* psuDialog,  SuLMParams suMaxParams, 
                 SuLMParams suMinParams, Pixmap pixmap, GC gc )
{
    int i, iCurrent, iX, iY, iPrevX, iPrevY, iOffsetDelta;
    Dimension width, height;
    float fPixelDifference;
    SuLMParams suLastParams;

    Widget drawingArea = psuDialog->drawingArea;
    Display* pDisplay = XtDisplay( drawingArea );
    Screen* pScreen = XtScreen( drawingArea );

    Colormap colorMap = DefaultColormapOfScreen( pScreen );
    XColor color, unused;

    XtVaGetValues( drawingArea, XtNwidth, &width, XtNheight, &height, NULL );

    if ( !XAllocNamedColor( pDisplay, colorMap, "Blue", &color, &unused ) )
    {
        XtWarning( "Couldn't allocated 'blue' color." );
    }
    XSetForeground( pDisplay, gc, color.pixel );

    width -= iTEXT_OFFSET;
    fPixelDifference = (float) width / (float) LM_PARAM_HISTORY_SIZE;
    iOffsetDelta = height / LM_PARAM_GRAPH_COUNT;

    i = 1;
    iCurrent = ( psuDialog->iLastUpdatedIndex + 1 ) % LM_PARAM_HISTORY_SIZE;
    suLastParams = psuDialog->asuParamHistory[ iCurrent ];
    iCurrent = ( iCurrent + 1 ) % LM_PARAM_HISTORY_SIZE;

    iPrevX = iTEXT_OFFSET;

    do
    {
        int iOffset = 0;
        SuLMParams suCurrentParams = psuDialog->asuParamHistory[ iCurrent ];
        iX = iTEXT_OFFSET + (int) ( (float) ( i * width ) / (float) LM_PARAM_HISTORY_SIZE );

        // goal gain
        iPrevY = GetDistFromTop( suMaxParams.fGoalGain, suMinParams.fGoalGain, 
                                 suLastParams.fGoalGain, iOffsetDelta ) + iOffset;
        iY     = GetDistFromTop( suMaxParams.fGoalGain, suMinParams.fGoalGain, 
                                 suCurrentParams.fGoalGain, iOffsetDelta ) + iOffset;
        XDrawLine( pDisplay, pixmap, gc, iPrevX, iPrevY, iX, iY );
        iOffset += iOffsetDelta;

        // avoid obstacle gain
        iPrevY = GetDistFromTop( suMaxParams.fAvoidObstacleGain, suMinParams.fAvoidObstacleGain, 
                                 suLastParams.fAvoidObstacleGain, iOffsetDelta ) + iOffset;
        iY     = GetDistFromTop( suMaxParams.fAvoidObstacleGain, suMinParams.fAvoidObstacleGain, 
                                 suCurrentParams.fAvoidObstacleGain, iOffsetDelta ) + iOffset;
        XDrawLine( pDisplay, pixmap, gc, iPrevX, iPrevY, iX, iY );
        iOffset += iOffsetDelta;

        // avoid obstacle sphere
        iPrevY = GetDistFromTop( suMaxParams.fObstacleSphere, suMinParams.fObstacleSphere, 
                                 suLastParams.fObstacleSphere, iOffsetDelta ) + iOffset;
        iY     = GetDistFromTop( suMaxParams.fObstacleSphere, suMinParams.fObstacleSphere, 
                                 suCurrentParams.fObstacleSphere, iOffsetDelta ) + iOffset;
        XDrawLine( pDisplay, pixmap, gc, iPrevX, iPrevY, iX, iY );
        iOffset += iOffsetDelta;

        // wander gain
        iPrevY = GetDistFromTop( suMaxParams.fWanderGain, suMinParams.fWanderGain, 
                                 suLastParams.fWanderGain, iOffsetDelta ) + iOffset;
        iY     = GetDistFromTop( suMaxParams.fWanderGain, suMinParams.fWanderGain, 
                                 suCurrentParams.fWanderGain, iOffsetDelta ) + iOffset;
        XDrawLine( pDisplay, pixmap, gc, iPrevX, iPrevY, iX, iY );
        iOffset += iOffsetDelta;

        // wander persistence
        iPrevY = GetDistFromTop( suMaxParams.iWanderPersistence, suMinParams.iWanderPersistence, 
                                 suLastParams.iWanderPersistence, iOffsetDelta ) + iOffset;
        iY     = GetDistFromTop( suMaxParams.iWanderPersistence, suMinParams.iWanderPersistence, 
                                 suCurrentParams.iWanderPersistence, iOffsetDelta ) + iOffset;
        XDrawLine( pDisplay, pixmap, gc, iPrevX, iPrevY, iX, iY );
        iOffset += iOffsetDelta;

        // update variables for next loop
        suLastParams = suCurrentParams;
        iPrevX = iX;

        // go to the next element w/ wrap-around
        iCurrent = ( iCurrent + 1 ) % LM_PARAM_HISTORY_SIZE;
        i++;
    } while ( iCurrent != psuDialog->iLastUpdatedIndex );
}

int GetDistFromTop( float fMax, float fMin, float fValue, int iRange )
{
    float fDelta = fMax - fMin;
    int iDistance = 0;

    // if the current value is uninitialized, it should appear on the bottom
    if ( fValue == -1 )
    {
        iDistance = iRange;
    }
    // otherwise, if all values are the same, draw a flat line in the middle
    else if ( fDelta == 0 )
    {
        iDistance = iRange / 2;
    }
    else
    {
        iDistance = (int) ( ( ( fMax - fValue ) / fDelta ) * (float) iRange );
    }

    return iDistance;
}

int GetDistFromTop( int iMax, int iMin, int iValue, int iRange )
{
    return GetDistFromTop( (float) iMax, (float) iMin, (float) iValue, iRange );
}

void DrawDivisions( Widget drawingArea, Pixmap pixmap, GC gc )
{
    // drawing variables
    Display* pDisplay = XtDisplay( drawingArea );
    Screen* pScreen = XtScreen( drawingArea );    

    Dimension width, height;
    XtVaGetValues( drawingArea, XtNwidth, &width, XtNheight, &height, NULL );
    XSetForeground( pDisplay, gc, BlackPixelOfScreen( pScreen ) );

    int iLeft = iTEXT_OFFSET;
    int iRight = width - 1;

    for ( int i = 0; i < LM_PARAM_GRAPH_COUNT; i++ )
    {
        int iTop = ( height / LM_PARAM_GRAPH_COUNT ) * i;
        int iBottom = ( height / LM_PARAM_GRAPH_COUNT ) * ( i + 1 ) - 1;

        // left side
        XDrawLine( pDisplay, pixmap, gc, iLeft, iTop, iLeft, iBottom );
        // right side
        XDrawLine( pDisplay, pixmap, gc, iRight, iTop, iRight, iBottom );
        // top
        XDrawLine( pDisplay, pixmap, gc, iLeft, iTop, iRight, iTop );
        // bottom
        XDrawLine( pDisplay, pixmap, gc, iLeft, iBottom, iRight, iBottom );
    }
}

void GetMaxAndMinParamValues( SuLMParams* psuMaxValues, SuLMParams* psuMinValues, 
                              const SuLMParams asuParamHistory[] )
{
    psuMaxValues->fGoalGain = 0;
    psuMaxValues->fAvoidObstacleGain = 0;
    psuMaxValues->fObstacleSphere = 0;
    psuMaxValues->fWanderGain = 0;
    psuMaxValues->iWanderPersistence = 0;

    psuMinValues->fGoalGain = FLT_MAX;
    psuMinValues->fAvoidObstacleGain = FLT_MAX;
    psuMinValues->fObstacleSphere = FLT_MAX;
    psuMinValues->fWanderGain = FLT_MAX;
    psuMinValues->iWanderPersistence = INT_MAX;

    // look at all values in the array
    for ( int i = 0; i < LM_PARAM_HISTORY_SIZE; i++ )
    {
        // goal gain
        if ( psuMaxValues->fGoalGain < asuParamHistory[ i ].fGoalGain )
        {
            psuMaxValues->fGoalGain = asuParamHistory[ i ].fGoalGain;
        }
        if ( ( psuMinValues->fGoalGain > asuParamHistory[ i ].fGoalGain ) && 
             ( asuParamHistory[ i ].fGoalGain != -1.0 ) )
        {
            psuMinValues->fGoalGain = asuParamHistory[ i ].fGoalGain;
        }

        // obstacle gain
        if ( psuMaxValues->fAvoidObstacleGain < asuParamHistory[ i ].fAvoidObstacleGain )
        {
            psuMaxValues->fAvoidObstacleGain = asuParamHistory[ i ].fAvoidObstacleGain;
        }
        if ( ( psuMinValues->fAvoidObstacleGain > asuParamHistory[ i ].fAvoidObstacleGain ) && 
             ( asuParamHistory[ i ].fAvoidObstacleGain != -1.0 ) )
        {
            psuMinValues->fAvoidObstacleGain = asuParamHistory[ i ].fAvoidObstacleGain;
        }

        // obstacle sphere
        if ( psuMaxValues->fObstacleSphere < asuParamHistory[ i ].fObstacleSphere )
        {
            psuMaxValues->fObstacleSphere = asuParamHistory[ i ].fObstacleSphere;
        }
        if ( ( psuMinValues->fObstacleSphere > asuParamHistory[ i ].fObstacleSphere ) && 
             ( asuParamHistory[ i ].fObstacleSphere != -1.0 ) )
        {
            psuMinValues->fObstacleSphere = asuParamHistory[ i ].fObstacleSphere;
        }

        // wander gain
        if ( psuMaxValues->fWanderGain < asuParamHistory[ i ].fWanderGain )
        {
            psuMaxValues->fWanderGain = asuParamHistory[ i ].fWanderGain;
        }
        if ( ( psuMinValues->fWanderGain > asuParamHistory[ i ].fWanderGain ) && 
             ( asuParamHistory[ i ].fWanderGain != -1.0 ) )
        {
            psuMinValues->fWanderGain = asuParamHistory[ i ].fWanderGain;
        }

        // wander persistence
        if ( psuMaxValues->iWanderPersistence < asuParamHistory[ i ].iWanderPersistence )
        {
            psuMaxValues->iWanderPersistence = asuParamHistory[ i ].iWanderPersistence;
        }
        if ( ( psuMinValues->iWanderPersistence > asuParamHistory[ i ].iWanderPersistence ) && 
             ( asuParamHistory[ i ].iWanderPersistence != -1 ) )
        {
            psuMinValues->iWanderPersistence = asuParamHistory[ i ].iWanderPersistence;
        }
    }

    // if anything was uninitialized, set values to 0
    if ( psuMinValues->fGoalGain == FLT_MAX )
    {
        psuMinValues->fGoalGain = 0.0;
    }
    if ( psuMinValues->fAvoidObstacleGain == FLT_MAX )
    {
        psuMinValues->fAvoidObstacleGain = 0.0;
    }
    if ( psuMinValues->fObstacleSphere == FLT_MAX )
    {
        psuMinValues->fObstacleSphere = 0.0;
    }
    if ( psuMinValues->fWanderGain == FLT_MAX )
    {
        psuMinValues->fWanderGain = 0.0;
    }
    if ( psuMinValues->iWanderPersistence == INT_MAX )
    {
        psuMinValues->iWanderPersistence = 0;
    }
}

void DrawingAreaCallback( Widget drawingArea, 
                          XtPointer client_data, XtPointer call_data )
{
    DrawTheWindow( (SuLMViewDialog*) client_data );
}

void DoneCallback( Widget doneButton, 
                   XtPointer client_data, XtPointer call_data )
{
    Widget dialog = GetTopShell( doneButton );

    DeleteLMParamDialog( dialog );
    XtPopdown( dialog );
}

void NextCallback( Widget nextButton, 
                   XtPointer client_data, XtPointer call_data )
{
    GetNextRobotForLMParamDialog( GetTopShell( nextButton ) );
}
