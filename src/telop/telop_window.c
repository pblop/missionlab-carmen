/**********************************************************************
 **                                                                  **
 **                      telop_window.c                              **
 **                                                                  **
 **  Written by:  Khaled Ali                                         **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: telop_window.c,v 1.3 2009/01/24 02:40:11 zkira Exp $ */

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include <X11/Xthreads.h>
#include "UxXt.h"
#include "telop_hw_joystick.h"
#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/DrawingA.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <X11/Shell.h>


#include <X11/extensions/XI.h> 
#include <X11/extensions/XInput.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

/*  Authored by Khaled S. Ali and UIM/X */

#include <math.h>
#include "shared.h"
#include "adjust.h"
#include "telop.h"
#include "telop_joystick.h"

extern swidget  telop_interface;


GC  source_gc, back_gc, line_gc;


static	Widget	main_drawing_area;
static	Widget	joystick_drawing_area;
Widget*	g_joystick_drawing_area;
static	Widget	north_label;
static	Widget	south_label;
static	Widget	west_label;
static	Widget	east_label;
static	Widget	joystick_label;
static	swidget	UxParent;

// AERIAL
Widget  altitude_vel;
bool created_altitude_vel = false;
static  Widget  rowcol;

#define CONTEXT_MACRO_ACCESS 1
#include "telop_window.h"
#undef CONTEXT_MACRO_ACCESS

Widget	main_telop_window;
Widget	commit_button;
Widget	immediate_toggle;
Widget	delayed_toggle;
Widget	unitname_labelGadget;
Widget world_toggle;
Widget robot_toggle;
Widget joystick_toggle;
Widget mouse_toggle;
char * label;
void enable_mouse(void);
void disable_mouse(void);
void enable_joystick(void);
void disable_joystick(void);

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/
/*
static void activateCB_quit_button(Widget wgt, XtPointer cd, XtPointer cb)
{

#ifndef DESIGN_TIME

      jq_kill_js_driver();

      UxPopdownInterface(telop_interface);
      normal.teleaut_vector.direction.x = 0.0;
      normal.teleaut_vector.direction.y = 0.0;
      normal.teleaut_vector.direction.z = 0.0;
      normal.teleaut_vector.magnitude = 0.0;
      set_values(normal);
      gt_end_teleoperation(1);
	
#endif

}
*/

static void armCB_world_toggle( Widget wgt, XtPointer cd, XtPointer cb)
{
    Widget UxWidget = wgt;
    XtPointer UxClientData = cd;
    XtPointer UxCallbackArg = cb;

    commit(UxWidget, UxClientData, UxCallbackArg);

    telop_mode = 0;
    UxPutSet(world_toggle, "true");
    UxPutSet(robot_toggle, "false");
    label = ( "N" );
    XtVaSetValues(north_label, 
                  XtVaTypedArg, XmNlabelString, 
                  XmRString, label, 
                  strlen(label) + 1, 
                  NULL);
    label = ( "S" );
    XtVaSetValues(south_label, XtVaTypedArg, 
                  XmNlabelString, XmRString, 
                  label, strlen(label) + 1, 
                  NULL);
    label = ( "W" );
    XtVaSetValues(west_label, XtVaTypedArg,
                  XmNlabelString, XmRString,
                  label, strlen(label) + 1,
                  NULL);
    label = ( "E" );
    XtVaSetValues(east_label, XtVaTypedArg,
                  XmNlabelString, XmRString,
                  label, strlen(label) + 1,
                  NULL);
    joy_stop();
}

static  void    disarmCB_world_toggle(
    Widget wgt,
    XtPointer cd,
    XtPointer cb)
{
    UxPutSet(world_toggle, "true");
}

static  void    armCB_robot_toggle(
    Widget wgt,
    XtPointer cd,
    XtPointer cb)
{
    UxPutSet(world_toggle, "false");
    UxPutSet(robot_toggle, "true");
    
    telop_mode = 1;
    label = ( "F" );

    XtVaSetValues(
        north_label,
        XtVaTypedArg,
        XmNlabelString,
        XmRString,
        label,
        strlen(label) + 1,
        NULL);

    label = ( "B" );

    XtVaSetValues(
        south_label,
        XtVaTypedArg,
        XmNlabelString,
        XmRString,
        label,
        strlen(label) + 1,
        NULL);

    label = ( "L" );

    XtVaSetValues(
        west_label,
        XtVaTypedArg,
        XmNlabelString,
        XmRString,
        label,
        strlen(label) + 1,
        NULL);

    label = ( "R" );

    XtVaSetValues(
        east_label,
        XtVaTypedArg,
        XmNlabelString,
        XmRString,
        label,
        strlen(label) + 1,
        NULL);

    joy_stop();
}

static  void    disarmCB_robot_toggle(
    Widget wgt,
    XtPointer cd, 
    XtPointer cb)
{
    UxPutSet(robot_toggle, "true");
}


/* Callbacks for joystick_toggle */
static  void    armCB_joystick_toggle(
    Widget wgt,
    XtPointer cd,
    XtPointer cb)
{
    UxPutSet(joystick_toggle, "true");
    UxPutSet(mouse_toggle, "false");
    
    /* turn off physical mouse input */
    disable_mouse();
    joy_stop();
		
    /* turn on physical joystick input */
    enable_joystick();
}

static  void    disarmCB_joystick_toggle(
    Widget wgt,
    XtPointer cd,
    XtPointer cb)
{
    UxPutSet(joystick_toggle, "true");
}


/* END callbacks for joystick_toggle */

/* Callbacks for mouse_toggle */
static  void    armCB_mouse_toggle(
    Widget wgt,  
    XtPointer cd,   
    XtPointer cb)
{
    UxPutSet(joystick_toggle, "false");
    UxPutSet(mouse_toggle, "true");
    
    /* turn off physical joystick input */
    disable_joystick();
    joy_stop();
		
    /* turn on physical mouse input */
    enable_mouse();
}

static  void    disarmCB_mouse_toggle(
    Widget wgt,
    XtPointer cd,
    XtPointer cb)
{ 
    UxPutSet(mouse_toggle, "true");
}


/* END callbacks for mouse_toggle */

static	void	armCB_immediate_toggle(
    Widget wgt, 
    XtPointer cd, 
    XtPointer cb)
{
    Widget                  UxWidget = wgt;
    XtPointer               UxClientData = cd;
    XtPointer               UxCallbackArg = cb;

    commit(UxWidget, UxClientData, UxCallbackArg);
    
    adjust_mode = 0;
    UxPutSet(immediate_toggle, "true");
    UxPutSet(delayed_toggle, "false");
    UxPutSensitive(commit_button, "false");
    
}

static void disarmCB_immediate_toggle(Widget wgt, XtPointer cd,  XtPointer cb)
{
    UxPutSet(immediate_toggle, "true");
}

static void armCB_delayed_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    UxPutSet(immediate_toggle, "false");
    UxPutSet(delayed_toggle, "true");
    UxPutSensitive(commit_button, "true");
    adjust_mode = 1;
    newest = normal;
}

static void disarmCB_delayed_toggle(Widget wgt, XtPointer cd, XtPointer cb)
{
    UxPutSet(delayed_toggle, "true");
}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget _Uxbuild_main_telop_window(void)
{
    Widget _UxParent;
    Widget frame1, frame2, frame3, frame_area1, frame_area2, frame_area3;

    // Creation of main_telop_window
    _UxParent = UxParent;

    if ( _UxParent == NULL )
    {
        _UxParent = UxTopLevel;
    }

    main_telop_window = XtVaCreatePopupShell(
        "main_telop_window",
        applicationShellWidgetClass,
        _UxParent,
        XmNwidth, 570,
        XmNheight, 260,
        XmNx, 171,
        XmNy, 601,
        XmNtitle, "Telop",
        XmNtransient, TRUE,
        XmNmwmDecorations, 0,
        NULL );


    // Creation of main_drawing_area
    main_drawing_area = XtVaCreateManagedWidget(
        "main_drawing_area",
        xmDrawingAreaWidgetClass,
        main_telop_window,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNwidth, 570,
        XmNheight, 260,
        XmNx, 195,
        XmNy, -5,
        XmNunitType, XmPIXELS,
        NULL );


    // Creation of joystick_drawing_area
    joystick_drawing_area = XtVaCreateManagedWidget(
        "joystick_drawing_area",
        xmDrawingAreaWidgetClass,
        main_drawing_area,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNwidth, 181,
        XmNheight, 181,
        XmNx, 380,
        XmNy, 30,
        NULL );

    XtAddCallback(
        joystick_drawing_area,
        XmNexposeCallback,
        (XtCallbackProc) joy_redisplay,
        (XtPointer) 0x0 );
    XtAddCallback(
        joystick_drawing_area, XmNinputCallback,
        (XtCallbackProc) joy_input,
        (XtPointer) NULL );
    g_joystick_drawing_area=&joystick_drawing_area;

    // Creation of hardware_select_frame
    frame1 = XtVaCreateManagedWidget(
        "frame1",
        xmFrameWidgetClass,
        main_drawing_area,
        XmNshadowType, XmSHADOW_ETCHED_IN,
        XmNwidth, 250,
        XmNheight, 70,
        XmNx, 5,
        XmNy, 10,
        NULL );

    // Creation of label for frame
    XtVaCreateManagedWidget(
        "Select Hardware",
        xmLabelGadgetClass,
        frame1,
        XmNchildType,
        XmFRAME_TITLE_CHILD,
        XmNchildVerticalAlignment,
        XmALIGNMENT_CENTER,
        NULL);

    // Creation of drawing for frame
    frame_area1 = XtVaCreateManagedWidget(
        "frame_area1",
        xmDrawingAreaWidgetClass,
        frame1,
        XmNtopAttachment,    XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_FORM,
        XmNrightAttachment,  XmATTACH_FORM,
        NULL );

    // Creation of joystick_toggle
    // Gives the ability to enable/disable joystick input
    joystick_toggle = XtVaCreateManagedWidget(
        "joystick_toggle",
        xmToggleButtonGadgetClass,
        frame_area1,
        XmNx, 10,
        XmNy, 10,
        XmNwidth, 180,
        XmNheight, 20,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNhighlightThickness, 0,
        XmNindicatorType, XmONE_OF_MANY,
        RES_CONVERT( XmNlabelString, "Joystick" ),
        XmNmarginTop, 0,
        //XmNset, TRUE,
        XmNset, false,
        XmNsensitive, true,
        NULL );

    XtAddCallback(
        joystick_toggle,
        XmNarmCallback,
        (XtCallbackProc)armCB_joystick_toggle,
        (XtPointer) NULL );

    XtAddCallback(
        joystick_toggle,
        XmNdisarmCallback,
        (XtCallbackProc)disarmCB_joystick_toggle,
        (XtPointer) NULL );

    //  END JOYSTICK_TOGGLE

    // Creation of mouse_toggle
    // Gives the ability to enable/disable joystick input
    mouse_toggle = XtVaCreateManagedWidget(
        "mouse_toggle",
        xmToggleButtonGadgetClass,
        frame_area1,
        XmNx, 10,
        XmNy, 30,
        XmNwidth, 180,
        XmNheight, 20,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNhighlightThickness, 0,
        XmNindicatorType, XmONE_OF_MANY,
        RES_CONVERT( XmNlabelString, "Mouse" ),
        XmNmarginTop, 0,
        //XmNset, FALSE,
        XmNset, true,
        XmNsensitive, true,
        NULL );

    XtAddCallback(
        mouse_toggle,
        XmNarmCallback,
        (XtCallbackProc)armCB_mouse_toggle,
        (XtPointer)NULL );

    XtAddCallback(
        mouse_toggle,
        XmNdisarmCallback,
        (XtCallbackProc)disarmCB_mouse_toggle,
        (XtPointer)NULL );

    // END MOUSE_TOGGLE

    // Creation of coordinate_select_frame
    frame2 = XtVaCreateManagedWidget(
        "frame2",
        xmFrameWidgetClass,
        main_drawing_area,
        XmNshadowType, XmSHADOW_ETCHED_IN,
        XmNwidth, 250,
        XmNheight, 70,
        XmNx, 5,
        XmNy, 90,
        NULL );

    // Creation of label for frame
    XtVaCreateManagedWidget(
        "Select Coordinate",
        xmLabelGadgetClass,
        frame2,
        XmNchildType,
        XmFRAME_TITLE_CHILD,
        XmNchildVerticalAlignment,
        XmALIGNMENT_CENTER,
        NULL);

    // Creation of drawing for frame
    frame_area2 = XtVaCreateManagedWidget(
        "frame_area2",
        xmDrawingAreaWidgetClass,
        frame2,
        XmNtopAttachment,    XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_FORM,
        XmNrightAttachment,  XmATTACH_FORM,
        NULL );
    
    // Creation of world_toggle
    world_toggle = XtVaCreateManagedWidget(
        "world_toggle",
        xmToggleButtonGadgetClass,
        frame_area2,
        XmNx, 10,
        XmNy, 10,
        XmNwidth, 150,
        XmNheight, 20,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNhighlightThickness, 0,
        XmNindicatorType, XmONE_OF_MANY,
        RES_CONVERT( XmNlabelString, "World Coord. " ),
        //XmNset, FALSE,
        XmNset, true,
        XmNsensitive, true,
        NULL );
    XtAddCallback(
        world_toggle,
        XmNarmCallback,
        (XtCallbackProc)armCB_world_toggle,
        (XtPointer) NULL );
    XtAddCallback(
        world_toggle,
        XmNdisarmCallback,
        (XtCallbackProc)disarmCB_world_toggle,
        (XtPointer) NULL );

    // Creation of robot_toggle
    robot_toggle = XtVaCreateManagedWidget(
        "robot_toggle",
        xmToggleButtonGadgetClass,
        frame_area2,
        XmNx, 10,
        XmNy, 30,
        XmNwidth, 150,
        XmNheight, 20,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNhighlightThickness, 0,
        XmNindicatorType, XmONE_OF_MANY,
        RES_CONVERT( XmNlabelString, "Robot Coord." ),
        XmNmarginTop, 0,
        XmNhighlightThickness, 0,
        //XmNset, TRUE,
        XmNset, false,
        XmNsensitive, true,
        NULL );
    XtAddCallback(
        robot_toggle,
        XmNarmCallback,
        (XtCallbackProc)armCB_robot_toggle,
        (XtPointer) NULL );
    XtAddCallback(
        robot_toggle,
        XmNdisarmCallback,
        (XtCallbackProc)disarmCB_robot_toggle,
        (XtPointer) NULL );
    
    // Creation of Delay_select_frame
    frame3 = XtVaCreateManagedWidget(
        "frame3",
        xmFrameWidgetClass,
        main_drawing_area,
        XmNshadowType, XmSHADOW_ETCHED_IN,
        XmNwidth, 250,
        XmNheight, 70,
        XmNx, 5,
        XmNy, 170,
        NULL );
    
    // Creation of label for frame
    XtVaCreateManagedWidget(
        "Select Delay",
        xmLabelGadgetClass,
        frame3,
        XmNchildType,
        XmFRAME_TITLE_CHILD,
        XmNchildVerticalAlignment,
        XmALIGNMENT_CENTER,
        NULL);
    
    // Creation of drawing for frame
    frame_area3 = XtVaCreateManagedWidget(
        "frame_area3",
        xmDrawingAreaWidgetClass,
        frame3,
        XmNtopAttachment,    XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment,   XmATTACH_FORM,
        XmNrightAttachment,  XmATTACH_FORM,
        NULL );

    // Creation of immediate_toggle
    immediate_toggle = XtVaCreateManagedWidget(
        "immediate_toggle",
        xmToggleButtonGadgetClass,
        frame_area3,
        XmNx, 10,
        XmNy, 10,
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
        immediate_toggle,
        XmNarmCallback,
        (XtCallbackProc) armCB_immediate_toggle,
        (XtPointer) NULL );
    
    XtAddCallback(
        immediate_toggle,
        XmNdisarmCallback,
        (XtCallbackProc) disarmCB_immediate_toggle,
        (XtPointer) NULL );


    // Creation of delayed_toggle
    delayed_toggle = XtVaCreateManagedWidget(
        "delayed_toggle",
        xmToggleButtonGadgetClass,
        frame_area3,
        XmNx, 10,
        XmNy, 30,
        XmNwidth, 150,
        XmNheight, 20,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNindicatorType, XmONE_OF_MANY,
        RES_CONVERT( XmNlabelString, "Delayed Effect" ),
        XmNmarginTop, 0,
        XmNhighlightThickness, 0,
        XmNset, FALSE,
        XmNsensitive, TRUE,
        NULL );

    XtAddCallback(
        delayed_toggle,
        XmNarmCallback,
        (XtCallbackProc) armCB_delayed_toggle,
        (XtPointer) NULL );

    XtAddCallback(
        delayed_toggle,
        XmNdisarmCallback,
        (XtCallbackProc) disarmCB_delayed_toggle,
        (XtPointer) NULL );

    // Creation of commit_button
    commit_button = XtVaCreateManagedWidget(
        "commit_button",
        xmPushButtonGadgetClass,
        frame_area3,
        XmNx, 160,
        XmNy, 30,
        XmNwidth, 75,
        XmNheight, 20,
        RES_CONVERT( XmNlabelString, "Commit" ),
        XmNsensitive, FALSE,
        NULL );

    XtAddCallback(
        commit_button,
        XmNactivateCallback,
        (XtCallbackProc)commit,
        (XtPointer) NULL );
    
    // Creation of north_label
    north_label = XtVaCreateManagedWidget(
        "north_label",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 465,
        XmNy, 10,
        XmNwidth, 20,
        XmNheight, 20,
        RES_CONVERT( XmNlabelString, "N" ),
        NULL );
    
    // Creation of south_label
    south_label = XtVaCreateManagedWidget(
        "south_label",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 465,
        XmNy, 213,
        XmNwidth, 20,
        XmNheight, 20,
        RES_CONVERT( XmNlabelString, "S" ),
        NULL );
    
    // Creation of west_label
    west_label = XtVaCreateManagedWidget(
        "west_label",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 360,
        XmNy, 110,
        XmNwidth, 20,
        XmNheight, 20,
        RES_CONVERT( XmNlabelString, "W" ),
        NULL );
    

    // Creation of east_label
    east_label = XtVaCreateManagedWidget(
        "east_label",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 563,
        XmNy, 110,
        XmNwidth, 20,
        XmNheight, 20,
        RES_CONVERT( XmNlabelString, "E" ),
        NULL );
    
    // Creation of unitname_labelGadget
    unitname_labelGadget = XtVaCreateManagedWidget(
        "unitname_labelGadget",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 10,
        XmNy, 10,
        XmNwidth, 240,
        XmNheight, 30,
        RES_CONVERT( XmNlabelString, "" ),
        NULL );

    // Creation of joystick_label
    joystick_label = XtVaCreateManagedWidget(
        "joystick_label",
        xmLabelGadgetClass,
        main_drawing_area,
        XmNx, 400,
        XmNy, 230,
        XmNwidth, 140,
        XmNheight, 30,
        RES_CONVERT( XmNlabelString, "Joystick" ),
        NULL );

    // AERIAL
    rowcol = XtVaCreateWidget("rowcol", xmRowColumnWidgetClass,
                              main_drawing_area,
                              XmNorientation,
                              XmHORIZONTAL,
                              XmNx, 260,
                              XmNy, 70,
                              XmNwidth, 30,
                              XmNheight, 150,
                              NULL);
    
    altitude_vel = XtVaCreateManagedWidget ("altitude_vel",
                                            xmScaleWidgetClass, rowcol,
                                            XtVaTypedArg, XmNtitleString, XmRString, "Alt", 13,
                                            XmNmaximum, 200,
                                            XmNminimum, 0,
                                            XmNvalue , 100,
                                            XmNdecimalPoints, 2,
                                            XmNshowValue, True,
                                            NULL);
    created_altitude_vel = true;
    
    XtAddCallback(altitude_vel,
                  XmNvalueChangedCallback,
                  (XtCallbackProc)changed_avel_val,
                  NULL);
    
    XtManageChild(rowcol);
      
    return ( main_telop_window );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_main_telop_window( swidget _UxUxParent )
{
	Widget  rtrn;

	UxParent = _UxUxParent;

	{
		if(XtToolkitThreadInitialize())
		{
			printf("Xt Threading Supported\n");
		}
		else
		{
			printf("Xt Threading not supported\n");
			/*gray out joystick option */
		}
		source_gc = XCreateGC( UxDisplay, DefaultRootWindow( UxDisplay ),
		                       0, NULL );
		XSetForeground( UxDisplay, source_gc, BlackPixel( UxDisplay, UxScreen ) );
		XSetBackground( UxDisplay, source_gc, WhitePixel( UxDisplay, UxScreen ) );
		
		back_gc = XCreateGC( UxDisplay, DefaultRootWindow( UxDisplay ),
		                       0, NULL );
		XSetForeground( UxDisplay, back_gc, WhitePixel( UxDisplay, UxScreen ) );
		XSetBackground( UxDisplay, back_gc, WhitePixel( UxDisplay, UxScreen ) );
		
		/* line_gc = XCreateGC( UxDisplay, DefaultRootWindow( UxDisplay ),
		                     0, NULL );
		XSetForeground( UxDisplay, line_gc, BlackPixel( UxDisplay, UxScreen ) );
		XSetBackground( UxDisplay, line_gc, WhitePixel( UxDisplay, UxScreen ) );
		XSetFunction( UxDisplay, line_gc, GXxor);
		*/
		line_gc = source_gc;
		rtrn = _Uxbuild_main_telop_window();

		/* This next call enables joystick dragging.
		   We disabled it because it caused too many messages to be sent.
		*/
		/* XtAddEventHandler(UxGetWidget(joystick_drawing_area), Button1MotionMask, FALSE, joy_movement, 0); */
		return(rtrn);
	}
}

/**********************************************************************
 * $Log: telop_window.c,v $
 * Revision 1.3  2009/01/24 02:40:11  zkira
 * Aerial telop
 *
 * Revision 1.2  2008/10/27 21:21:50  zkira
 * Added 3D telop interface
 *
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/07/07 00:06:28  endo
 * telop default mode changed.
 *
 * Revision 1.2  2006/05/02 04:20:20  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/02 21:19:31  zkira
 * Added joystick support and changed telop GUI
 *
 * Revision 1.2  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.1  2000/02/29 21:57:43  saho
 * Initial revision
 *
 *
 *
 **********************************************************************/

