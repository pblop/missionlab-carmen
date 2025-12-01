/**********************************************************************
 **                                                                  **
 **                  motivational_vector_window.c                    **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: motivational_vector_window.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

//---------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include <Xm/Scale.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/PanedW.h>
#include <X11/Shell.h>
#include <vector>
#include <string>

#include "motivational_vector.h"
#include "motivational_vector_window.h"
#include "adjust.h"
#include "telop.h"

using std::vector;
using std::string;

//---------------------------------------------------------------------
// Macros
//---------------------------------------------------------------------
#define	RES_CONVERT( res_name, res_value) XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
const int MOTIVATIONALVECTOR_WINDOWWIDTH = 250;
const int MOTIVATIONALVECTOR_ROWHEIGHT = 50;
const int MOTIVATIONALVECTOR_ROWOFFSET = 10;
const int MOTIVATIONALVECTOR_BUTTONHEIGHT = 50;
const int MOTIVATIONALVECTOR_DEFAULT_SLIDERVALUE = 50;
const int MOTIVATIONALVECTOR_POPUP_POSX = 550;
const int MOTIVATIONALVECTOR_POPUP_POSY = 400;
const string MOTIVATIONALVECTOR_TITLE = "Motivational Vector";

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------
XtAppContext gMotivationalVectorAppContext;  
Widget gMotivationalVectorTopLevel;
Widget *gMotivationalVector_sliderWidget = NULL;
Display *gMotivationalVectorDisplay = NULL;
int gMotivationalVectorScreen;

Widget MotivationalVector_popupWindow;

//---------------------------------------------------------------------
//       The following are callback functions.
//---------------------------------------------------------------------
static void activateCB_mv_close_button(Widget wgt, XtPointer cd, XtPointer cb)
{
    XtPopdown(MotivationalVector_popupWindow);
    gt_end_teleoperation(2);
}

//---------------------------------------------------------------------
// Creation of motivational_vector_window
//---------------------------------------------------------------------
Widget create_motivational_vector_window(void)
{
    Widget motivational_vector_pane;
    Widget motivational_vector_form;
    Widget motivational_vector_rowColumn;
    Widget close_button;
    Widget close_form;
    vector<Widget> stackedWidget;
    int windowHeight, sliderAreaHeight;
    int i, numStackedWidget;
    char name[256];

    // Calculate the window heights first
    sliderAreaHeight = gMotivationalVector_numVectors*(MOTIVATIONALVECTOR_ROWHEIGHT+MOTIVATIONALVECTOR_ROWOFFSET);
    windowHeight = sliderAreaHeight+MOTIVATIONALVECTOR_BUTTONHEIGHT;
    
    // Create the popup shell
    MotivationalVector_popupWindow = XtVaCreatePopupShell(
	"motivational_vector_window",
	applicationShellWidgetClass,
	gMotivationalVectorTopLevel,
	XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	XmNheight, windowHeight,
	XmNx, MOTIVATIONALVECTOR_POPUP_POSX,
	XmNy, MOTIVATIONALVECTOR_POPUP_POSY,
	XmNtitle, MOTIVATIONALVECTOR_TITLE.c_str(),
	XmNtransient, true,
	NULL);

    // Create a PanedWindow to manage the stuff in the popup shell
    motivational_vector_pane = XtVaCreateWidget(
	"pane",
	xmPanedWindowWidgetClass, MotivationalVector_popupWindow,
	XmNsashHeight, 1,
	XmNsashWidth, 1,
	NULL);

    stackedWidget.push_back(motivational_vector_pane);

    // Creation of motivational_vector_form
    motivational_vector_form = XtVaCreateWidget(
	"motivational_vector_form",
	xmFormWidgetClass,
	motivational_vector_pane,
	XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	XmNheight, sliderAreaHeight,
	XmNresizePolicy, XmRESIZE_NONE,
	XmNx, 0,
	XmNy, 0,
	XmNunitType, XmPIXELS,
	NULL );

    stackedWidget.push_back(motivational_vector_form);

    // Creation of motivational_vector_rowColumn
    motivational_vector_rowColumn = XtVaCreateWidget(
	"motivational_vector_rowColumn",
	xmRowColumnWidgetClass,
	motivational_vector_form,
	XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	XmNheight, sliderAreaHeight,
	XmNentryAlignment, XmALIGNMENT_CENTER,
	XmNpacking, XmPACK_TIGHT,
	XmNtopAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL );

    stackedWidget.push_back(motivational_vector_rowColumn);

    // Creation of the close button form
    close_form = XtVaCreateWidget(
	"close_form",
	xmFormWidgetClass,
	motivational_vector_pane,
	XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	XmNheight, MOTIVATIONALVECTOR_BUTTONHEIGHT,
	XmNresizePolicy, XmRESIZE_NONE,
	XmNx, 0,
	XmNunitType, XmPIXELS,
	NULL );

    stackedWidget.push_back(close_form);

    // Creation of close_button
    close_button = XtVaCreateWidget(
	"close_button",
	xmPushButtonGadgetClass, close_form,
	XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	XmNheight, MOTIVATIONALVECTOR_BUTTONHEIGHT,
	RES_CONVERT( XmNlabelString, "Close Window" ),
	XmNtopAttachment, XmATTACH_FORM,
	XmNleftAttachment, XmATTACH_FORM,
	XmNrightAttachment, XmATTACH_FORM,
	XmNbottomAttachment, XmATTACH_FORM,
	NULL );

    XtAddCallback(
	close_button,
	XmNactivateCallback,
	(XtCallbackProc) activateCB_mv_close_button,
	(XtPointer) NULL );

    stackedWidget.push_back(close_button);

    gMotivationalVector_sliderWidget = new Widget[gMotivationalVector_numVectors];

    for (i=0; i<gMotivationalVector_numVectors; i++)
    {
	sprintf(name, "motivational_vector%d", i);
	gMotivationalVector_sliderWidget[i] = XtVaCreateWidget( 
	    name,
	    xmScaleWidgetClass,
	    motivational_vector_rowColumn,
	    XmNwidth, MOTIVATIONALVECTOR_WINDOWWIDTH,
	    XmNheight, MOTIVATIONALVECTOR_ROWHEIGHT, 
	    XmNorientation, XmHORIZONTAL, 
	    XmNminimum, 0,
	    XmNscaleMultiple, 1,
	    XmNshowValue, true,
	    XmNtitleString, XmStringCreateLocalized(gMotivationalVector_vectors[i].title),
	    XmNvalue, MOTIVATIONALVECTOR_DEFAULT_SLIDERVALUE,
	    NULL );

	XtAddCallback(
	    gMotivationalVector_sliderWidget[i],
	    XmNvalueChangedCallback,
	    (XtCallbackProc) adjust_motivational_vector,
	    (XtPointer) &(gMotivationalVector_vectors[i]) );

	stackedWidget.push_back(gMotivationalVector_sliderWidget[i]);
    }

    numStackedWidget = stackedWidget.size();

    // Manage all the widgets

    for (i=0; i<numStackedWidget; i++)
    {
	XtManageChild(stackedWidget.back());
	stackedWidget.pop_back();
    }
	
    return MotivationalVector_popupWindow;
}

/**********************************************************************
 * $Log: motivational_vector_window.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.4  2003/04/06 11:34:30  endo
 * gcc 3.1.1
 *
 * Revision 1.3  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.2  2000/03/24 18:15:11  saho
 * *** empty log message ***
 *
 * Revision 1.1  2000/02/29 21:23:03  saho
 * Initial revision
 *
 *
 **********************************************************************/
