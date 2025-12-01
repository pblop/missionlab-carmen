/**********************************************************************
 **                                                                  **
 **                      sound_simulation_window.c                   **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_simulation_window.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/DrawingA.h>
#include <X11/Xlib.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/


#include <math.h>
#include "shared.h"
#include "adjust.h"
#include "sound_joystick.h"
#include "sound_simulation.h"

extern swidget  sound_simulation_interface;


GC  sound_source_gc, sound_back_gc, sound_line_gc;


static	Widget	sound_simulation_drawing_area;
static	Widget	sound_source_drawing_area;
static	Widget	sound_quit_button;
static	Widget	sound_north_label;
static	Widget	sound_south_label;
static	Widget	sound_west_label;
static	Widget	sound_east_label;
static	Widget	sound_source_label;
static	swidget	UxParent;

#define CONTEXT_MACRO_ACCESS 1
#include "sound_simulation_window.h"
#undef CONTEXT_MACRO_ACCESS

Widget	sound_simulation_window;
Widget	sound_commit_button;
Widget	sound_immediate_toggle;
Widget	sound_delayed_toggle;
Widget	sound_unitname_labelGadget;

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static void sound_activateCB_quit_button(Widget wgt, XtPointer cd, XtPointer cb)
{
        XtPopdown(sound_simulation_interface);
	sound_normal.sound_vector.direction.x = 0.0;
	sound_normal.sound_vector.direction.y = 0.0;
	sound_normal.sound_vector.magnitude = 0.0;
	sound_set_values(sound_normal);
	gt_end_sound_simulation(1);
}

static void sound_armCB_immediate_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    commit(wgt, cd, cb);
	
    sound_adjust_mode = 0;
    UxPutSet(immediate_toggle, "true");
    UxPutSet(delayed_toggle, "false");
    UxPutSensitive(commit_button, "false");
}

static void sound_disarmCB_immediate_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    UxPutSet(immediate_toggle, "true");
}

static void sound_armCB_delayed_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    UxPutSet(immediate_toggle, "false");
    UxPutSet(delayed_toggle, "true");
    UxPutSensitive(commit_button, "true");
    sound_adjust_mode = 1;
	
    sound_newest = sound_normal;
}

static void sound_disarmCB_delayed_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    UxPutSet(delayed_toggle, "true");
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget _Uxbuild_sound_simulation_window(void)
{
	Widget _UxParent;

	// Creation of sound_simulation_window
	_UxParent = UxParent;

	if ( _UxParent == NULL )
	{
	    _UxParent = gSoundSimulationTopLevel;
	}

	sound_simulation_window = XtVaCreatePopupShell(
	    "sound_simulation_window",
	    applicationShellWidgetClass,
	    _UxParent,
	    XmNwidth, 490,
	    XmNheight, 260,
	    XmNx, 71,
	    XmNy, 601,
	    XmNtitle, "Sound Simulation",
	    XmNtransient, TRUE,
	    NULL );


	/* Creation of sound_simulation_drawing_area */
	sound_simulation_drawing_area = XtVaCreateManagedWidget(
	    "sound_simulation_drawing_area",
	    xmDrawingAreaWidgetClass,
	    sound_simulation_window,
	    XmNresizePolicy, XmRESIZE_NONE,
	    XmNwidth, 490,
	    XmNheight, 260,
	    XmNx, 195,
	    XmNy, -5,
	    XmNunitType, XmPIXELS,
	    NULL );


	/* Creation of sound_source_drawing_area */
	sound_source_drawing_area = XtVaCreateManagedWidget(
	    "sound_source_drawing_area",
	    xmDrawingAreaWidgetClass,
	    sound_simulation_drawing_area,
	    XmNresizePolicy, XmRESIZE_NONE,
	    XmNwidth, 181,
	    XmNheight, 181,
	    XmNx, 280,
	    XmNy, 30,
	    NULL );

	XtAddCallback(
	    sound_source_drawing_area,
	    XmNexposeCallback,
	    (XtCallbackProc) sound_redisplay,
	    (XtPointer) 0x0 );

	XtAddCallback(
	    sound_source_drawing_area,
	    XmNinputCallback,
	    (XtCallbackProc) sound_input,
	    (XtPointer) NULL );


	/* Creation of quit_button */
	sound_quit_button = XtVaCreateManagedWidget(
	    "quit_button",
	    xmPushButtonGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 30,
	    XmNy, 190,
	    XmNwidth, 180,
	    XmNheight, 30,
	    RES_CONVERT( XmNlabelString, "End SoundSim" ),
	    XmNsensitive, TRUE,
	    NULL );

	XtAddCallback(
	    sound_quit_button,
	    XmNactivateCallback,
	    (XtCallbackProc) sound_activateCB_quit_button,
	    (XtPointer) NULL );


	/* Creation of commit_button */
	sound_commit_button = XtVaCreateManagedWidget(
	    "commit_button",
	    xmPushButtonGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 160,
	    XmNy, 100,
	    XmNwidth, 80,
	    XmNheight, 30,
	    RES_CONVERT( XmNlabelString, "Commit" ),
	    XmNsensitive, FALSE,
	    NULL );

	XtAddCallback(
	    sound_commit_button, XmNactivateCallback,
	    (XtCallbackProc) sound_commit,
	    (XtPointer) NULL );


	/* Creation of immediate_toggle */
	sound_immediate_toggle = XtVaCreateManagedWidget(
	    "immediate_toggle",
	    xmToggleButtonGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 10,
	    XmNy, 80,
	    XmNwidth, 230,
	    XmNheight, 20,
	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNhighlightThickness, 0,
	    XmNindicatorType, XmONE_OF_MANY,
	    RES_CONVERT( XmNlabelString, "Immediate Effect" ),
	    XmNset, TRUE,
	    XmNsensitive, TRUE,
	    NULL );

	XtAddCallback(
	    sound_immediate_toggle,
	    XmNarmCallback,
	    (XtCallbackProc) sound_armCB_immediate_toggle,
	    (XtPointer) NULL );

	XtAddCallback(
	    sound_immediate_toggle,
	    XmNdisarmCallback,
	    (XtCallbackProc) sound_disarmCB_immediate_toggle,
	    (XtPointer) NULL );

	/* Creation of delayed_toggle */
	sound_delayed_toggle = XtVaCreateManagedWidget(
	    "delayed_toggle",
	    xmToggleButtonGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 10,
	    XmNy, 100,
	    XmNwidth, 150,
	    XmNheight, 30,
	    XmNalignment, XmALIGNMENT_BEGINNING,
	    XmNindicatorType, XmONE_OF_MANY,
	    RES_CONVERT( XmNlabelString, "Delayed Effect" ),
	    XmNmarginTop, 0,
	    XmNhighlightThickness, 0,
	    XmNset, FALSE,
	    XmNsensitive, TRUE,
	    NULL );

	XtAddCallback(
	    sound_delayed_toggle,
	    XmNarmCallback,
	    (XtCallbackProc) sound_armCB_delayed_toggle,
	    (XtPointer) NULL );

	XtAddCallback(
	    sound_delayed_toggle,
	    XmNdisarmCallback,
	    (XtCallbackProc) sound_disarmCB_delayed_toggle,
	    (XtPointer) NULL );


	/* Creation of north_label */
	sound_north_label = XtVaCreateManagedWidget(
	    "north_label",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 360,
	    XmNy, 10,
	    XmNwidth, 20,
	    XmNheight, 20,
	    RES_CONVERT( XmNlabelString, "N" ),
	    NULL );


	/* Creation of south_label */
	sound_south_label = XtVaCreateManagedWidget(
	    "south_label",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 360,
	    XmNy, 213,
	    XmNwidth, 20,
	    XmNheight, 20,
	    RES_CONVERT( XmNlabelString, "S" ),
	    NULL );


	/* Creation of west_label */
	sound_west_label = XtVaCreateManagedWidget(
	    "west_label",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 257,
	    XmNy, 110,
	    XmNwidth, 20,
	    XmNheight, 20,
	    RES_CONVERT( XmNlabelString, "W" ),
	    NULL );


	/* Creation of east_label */
	sound_east_label = XtVaCreateManagedWidget(
	    "east_label",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 460,
	    XmNy, 110,
	    XmNwidth, 20,
	    XmNheight, 20,
	    RES_CONVERT( XmNlabelString, "E" ),
	    NULL );


	/* Creation of unitname_labelGadget */
	sound_unitname_labelGadget = XtVaCreateManagedWidget(
	    "unitname_labelGadget",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 10,
	    XmNy, 10,
	    XmNwidth, 240,
	    XmNheight, 30,
	    RES_CONVERT( XmNlabelString, "" ),
	    NULL );


	/* Creation of sound_source_label */
	sound_source_label = XtVaCreateManagedWidget(
	    "sound_source_label",
	    xmLabelGadgetClass,
	    sound_simulation_drawing_area,
	    XmNx, 300,
	    XmNy, 230,
	    XmNwidth, 140,
	    XmNheight, 30,
	    RES_CONVERT( XmNlabelString, "Sound_Source" ),
	    NULL );

	return ( sound_simulation_window );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_sound_simulation_window( swidget _UxUxParent )
{
    Widget                  rtrn;

    UxParent = _UxUxParent;

	sound_source_gc = XCreateGC(
	    gSoundSimulationDisplay,
	    DefaultRootWindow( gSoundSimulationDisplay ),
	    0,
	    NULL );

	XSetForeground(
	    gSoundSimulationDisplay,
	    sound_source_gc,
	    BlackPixel( gSoundSimulationDisplay, gSoundSimulationScreen ));

	XSetBackground(
	    gSoundSimulationDisplay,
	    sound_source_gc,
	    WhitePixel( gSoundSimulationDisplay, gSoundSimulationScreen ));
		
	sound_back_gc = XCreateGC(
	    gSoundSimulationDisplay,
	    DefaultRootWindow( gSoundSimulationDisplay ),
	    0,
	    NULL );

	XSetForeground(
	    gSoundSimulationDisplay,
	    sound_back_gc,
	    WhitePixel( gSoundSimulationDisplay, gSoundSimulationScreen ));

	XSetBackground(
	    gSoundSimulationDisplay,
	    sound_back_gc,
	    WhitePixel( gSoundSimulationDisplay, gSoundSimulationScreen ) );
		
	sound_line_gc = sound_source_gc;
	rtrn = _Uxbuild_sound_simulation_window();

	return(rtrn);
}



/**********************************************************************
 * $Log: sound_simulation_window.c,v $
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
 * Revision 1.2  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.1  2000/02/29 21:30:55  saho
 * Initial revision
 *
 *
 **********************************************************************/
