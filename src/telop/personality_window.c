/**********************************************************************
 **                                                                  **
 **                     personality_window.c                         **
 **                                                                  **
 **  Written by:  Khaled S. Ali and UIM/X                            **
 **                                                                  ** 
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: personality_window.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

//---------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------
#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include <Xm/RowColumn.h>
#include <Xm/PushBG.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/PanedW.h>
#include <X11/Shell.h>
#include <vector>
#include <string>

#include "personality_window.h"
#include "telop_window.h"
#include "telop.h"
#include "adjust.h"

using std::vector;
using std::string;

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
const int PERSONALITYWINDOW_WINDOWWIDTH = 250;
const int PERSONALITYWINDOW_ROWHEIGHT = 75;
const int PERSONALITYWINDOW_ROWOFFSET = 10;
const int PERSONALITYWINDOW_BUTTONHEIGHT = 50;
const int PERSONALITYWINDOW_DEFAULT_SLIDERVALUE = 50;
const int PERSONALITYWINDOW_POPUP_POSX = 300;
const int PERSONALITYWINDOW_POPUP_POSY = 400;
const int PERSONALITYWINDOW_WINDOWHEIGHT_EXTRA = 20;
const string PERSONALITYWINDOW_TITLE = "Personality";

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------
Widget PersonalityWindow_popupWindow;

//---------------------------------------------------------------------
//       The following are callback functions.
//---------------------------------------------------------------------
static void activateCB_close_button(Widget wgt, XtPointer cd, XtPointer cb)
{
    XtPopdown(PersonalityWindow_popupWindow);
    gt_end_teleoperation(2);
}

//---------------------------------------------------------------------
// Creation of personality_window
//---------------------------------------------------------------------
Widget create_personality_window(Widget parent)
{
    Widget personality_pane;
    Widget personality_form;
    Widget *personality_slider = NULL;
    Widget personality_rowColumn;
    Widget close_button;
    Widget close_form;
    XmString sliderTitle;
    vector<Widget> stackedWidget;
    int windowHeight, sliderAreaHeight;
    int i, numStackedWidget;
    char name[256];
	
    // Calculate the window heights first
    sliderAreaHeight = gPersonalityWindow_numPersonalities*(PERSONALITYWINDOW_ROWHEIGHT+PERSONALITYWINDOW_ROWOFFSET);
    windowHeight = sliderAreaHeight+PERSONALITYWINDOW_BUTTONHEIGHT+PERSONALITYWINDOW_WINDOWHEIGHT_EXTRA;

    // Create the popup shell
    PersonalityWindow_popupWindow = XtVaCreatePopupShell(
        "personality_window",
        applicationShellWidgetClass,
        parent,
        XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
        XmNheight, windowHeight,
        XmNx, PERSONALITYWINDOW_POPUP_POSX,
        XmNy, PERSONALITYWINDOW_POPUP_POSY,
        XmNtitle, PERSONALITYWINDOW_TITLE.c_str(),
        XmNtransient, TRUE,
        NULL );

    // Create a PanedWindow to manage the stuff in the popup shell
    personality_pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, PersonalityWindow_popupWindow,
        XmNsashHeight, 1,
        XmNsashWidth, 1,
        NULL);

    stackedWidget.push_back(personality_pane);

    // Creation of personality_form
    personality_form = XtVaCreateWidget(
        "personality_form",
        xmFormWidgetClass,
        personality_pane,
        XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
        XmNheight, sliderAreaHeight,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNx, 0,
        XmNy, 0,
        XmNunitType, XmPIXELS,
        NULL );

    stackedWidget.push_back(personality_form);

    // Creation of personality_rowColumn
    personality_rowColumn = XtVaCreateWidget(
        "personality_rowColumn",
        xmRowColumnWidgetClass,
        personality_form,
        XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
        XmNheight, sliderAreaHeight,
        XmNentryAlignment, XmALIGNMENT_CENTER,
        XmNpacking, XmPACK_TIGHT,
        XmNtopAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL );

    stackedWidget.push_back(personality_rowColumn);

    // Create the sliderbars

    personality_slider = new Widget[gPersonalityWindow_numPersonalities];

    for (i=0; i<gPersonalityWindow_numPersonalities; i++)
    {
        sprintf(name, "personality%d", i);

        sliderTitle = XmStringCreateLtoR
            ((String)gPersonalityWindow_personalities[i].title,
             XmSTRING_DEFAULT_CHARSET);

        /*
        personality_slider[i] = XtVaCreateWidget( 
            name,
            xmScaleWidgetClass,
            personality_rowColumn,
            XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
            XmNheight, PERSONALITYWINDOW_ROWHEIGHT,
            XmNorientation, XmHORIZONTAL,
            XmNminimum, 0,
            XmNscaleMultiple, 1,
            XmNshowValue, true,
            XmNtitleString, XmStringCreateLocalized(gPersonalityWindow_personalities[i].title),
            XmNvalue, PERSONALITYWINDOW_DEFAULT_SLIDERVALUE,
            NULL );
        */

        personality_slider[i] = XtVaCreateWidget( 
            name,
            xmScaleWidgetClass,
            personality_rowColumn,
            XmNminimum, 0,
            XmNvalue, PERSONALITYWINDOW_DEFAULT_SLIDERVALUE,
            XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
            XmNheight, PERSONALITYWINDOW_ROWHEIGHT,
            XmNorientation, XmHORIZONTAL,
            XmNscaleMultiple, 1,
            XmNshowValue, true,
            XmNtitleString, sliderTitle,
            NULL );

        XmStringFree(sliderTitle);

        XtAddCallback(
            personality_slider[i],
            XmNvalueChangedCallback,
            (XtCallbackProc) adjust_personality,
            (XtPointer) &(gPersonalityWindow_personalities[i]) );
	
        stackedWidget.push_back(personality_slider[i]);
    }

    // Creation of the close button form
    close_form = XtVaCreateWidget(
        "close_form",
        xmFormWidgetClass,
        personality_pane,
        XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
        XmNheight, PERSONALITYWINDOW_BUTTONHEIGHT,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNx, 0,
        XmNunitType, XmPIXELS,
        NULL );

    stackedWidget.push_back(close_form);

    // Creation of close_button
    close_button = XtVaCreateWidget(
        "close_button",
        xmPushButtonGadgetClass, close_form,
        XmNwidth, PERSONALITYWINDOW_WINDOWWIDTH,
        XmNheight, PERSONALITYWINDOW_BUTTONHEIGHT,
        RES_CONVERT( XmNlabelString, "Close Window" ),
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        NULL );

    XtAddCallback(
        close_button,
        XmNactivateCallback,
        (XtCallbackProc) activateCB_close_button,
        (XtPointer) NULL );

    stackedWidget.push_back(close_button);

    // Manage all the widgets

    numStackedWidget = stackedWidget.size();


    for (i=0; i<numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }
	
    return PersonalityWindow_popupWindow;
}

/**********************************************************************
 * $Log: personality_window.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/07 04:21:20  endo
 * Personality Window debugged.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.13  2003/04/06 11:33:13  endo
 * For gcc 3.1.1.
 *
 * Revision 1.12  2002/07/18 17:02:58  endo
 * Fixed the problem of not displaying correctly with OpenMotif.
 *
 * Revision 1.11  2000/02/29 22:22:13  saho
 * Fixed a bug. The personality window can have many slider bars depending
 * on an input file. The window is rescaled in run time but the area
 * for the sliders was not. Fixed that.
 *
 *
 **********************************************************************/
