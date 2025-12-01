
/*******************************************************************************
	main_window.c

       Associated Header file: main_window.h
*******************************************************************************/

#include <stdio.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"

#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/DrawingA.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

/*  Authored by Khaled S. Ali and UIM/X and hacked together to support drawing
 of the compass by Jonathan F. Diaz*/

#include <math.h>

Widget          compass_interface;
GC  source_compass_gc, back_compass_gc, line_compass_gc;


static	Widget	main_compass_drawing_area;
static	Widget	compass_drawing_area;
static	Widget	compass_quit_button;
static	Widget	north_compass_label;
static	Widget	south_compass_label;
static	Widget	west_compass_label;
static	Widget	east_compass_label;

static	swidget	UxCompassParent;
XtAppContext    UxCompassAppContext;  
Widget          UxCompassTopLevel;
Display         *UxCompassDisplay;
int             UxCompassScreen, compassExposed = 0;

Widget	main_compass_window;

#define COMPASS_WIDTH  180
#define COMPASS_HEIGHT  180

void gt_end_compass(void)
{
  compassExposed = 0;
  return;
}
void  redisplayCompass(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
{
  XmDrawingAreaCallbackStruct  *call_data = (XmDrawingAreaCallbackStruct *) UxCallbackArg;
  
  if (call_data->event->xexpose.count == 0) {
    XFillArc( UxCompassDisplay, XtWindow(w), back_compass_gc, 0, 0, COMPASS_WIDTH, COMPASS_HEIGHT,
	      0, 23040 );
    XDrawArc( UxCompassDisplay, XtWindow(w), source_compass_gc, 0, 0, COMPASS_WIDTH, COMPASS_HEIGHT,
              0, 23040 );
    XFillArc( UxCompassDisplay, XtWindow(w), source_compass_gc,
              COMPASS_WIDTH/2 - 2, COMPASS_HEIGHT/2 - 2, 5, 5, 0, 23040 );
  }
}
void clearCompass(Widget w)
{
  XFillArc( UxCompassDisplay, XtWindow(w), back_compass_gc, 0, 0, COMPASS_WIDTH, COMPASS_HEIGHT,
	    0, 23040 );
  XDrawArc( UxCompassDisplay, XtWindow(w), source_compass_gc, 0, 0, COMPASS_WIDTH, COMPASS_HEIGHT,
	    0, 23040 );
  XFillArc( UxCompassDisplay, XtWindow(w), source_compass_gc,
	    COMPASS_WIDTH/2 - 2, COMPASS_HEIGHT/2 - 2, 5, 5, 0, 23040 );
}
void gt_draw_compass_heading(int heading)
{
  double ang, x, y, midx, midy, len;

  if (!compassExposed) return;
  clearCompass(compass_drawing_area);
  heading = 360 - heading;  /*convert from azimuth to angle*/
  heading += 90;
  if (heading < 0) heading += 360;
  if (heading > 360) heading -= 360;
  ang = heading * 0.017453293; /*convert to radians*/
  midx = COMPASS_WIDTH/2;
  midy = COMPASS_HEIGHT/2;
  len = midx * 0.85;
  x = cos(ang) * len + midx;
  y = midy - sin(ang) * len;
  XDrawLine(UxCompassDisplay, XtWindow(compass_drawing_area), 
	    source_compass_gc, midx, midy, x, y);
}
/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

static	void	activateCB_quit_button_compass(Widget wgt, XtPointer cd, XtPointer cb)
{
	UxPopdownInterface(compass_interface);
	gt_end_compass();
}
/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_main_compass_window()
{
	Widget		_UxParent;
	XmFontList labelsFontList = UxConvertFontList( "-*-*-bold-*-*-*-18-*-*-*-*-*-*-*" );
	XmFontList textsFontList = UxConvertFontList( "-*-*-bold-*-*-*-16-*-*-*-*-*-*-*" );

	/* Creation of main_telop_window */
	_UxParent = UxCompassParent;
	if ( _UxParent == NULL )
	{
		_UxParent = UxCompassTopLevel;
	}

	main_compass_window = XtVaCreatePopupShell( "main_compass_window",
			applicationShellWidgetClass,
			_UxParent,
			XmNwidth, 490,
			XmNheight, 260,
			XmNx, 71,
			XmNy, 295,
			XmNtitle, "Compass",
			XmNtransient, TRUE,
			XmNbuttonFontList, textsFontList,
			XmNlabelFontList, textsFontList,
			XmNtextFontList, textsFontList,
			NULL );


	/* Creation of main_drawing_area */
	main_compass_drawing_area = XtVaCreateManagedWidget( "main_compass_drawing_area",
			xmDrawingAreaWidgetClass,
			main_compass_window,
			XmNresizePolicy, XmRESIZE_NONE,
			XmNwidth, 490,
			XmNheight, 260,
			XmNx, 195,
			XmNy, -5,
			XmNunitType, XmPIXELS,
			NULL );


	/* Creation of joystick_drawing_area */
	compass_drawing_area = XtVaCreateManagedWidget( "compass_drawing_area",
			xmDrawingAreaWidgetClass,
			main_compass_drawing_area,
			XmNresizePolicy, XmRESIZE_NONE,
			XmNwidth, 181,
			XmNheight, 181,
			XmNx, 280,
			XmNy, 30,
			NULL );
	XtAddCallback( compass_drawing_area, XmNexposeCallback,
		(XtCallbackProc) redisplayCompass,
		(XtPointer) 0x0 );
	

	/* Creation of quit_button */
	compass_quit_button = XtVaCreateManagedWidget( "compass_quit_button",
			xmPushButtonGadgetClass,
			main_compass_drawing_area,
			XmNx, 30,
			XmNy, 190,
			XmNwidth, 180,
			XmNheight, 30,
			XmNfontList, textsFontList,
			RES_CONVERT( XmNlabelString, "End Compass" ),
			XmNsensitive, TRUE,
			NULL );
	XtAddCallback( compass_quit_button, XmNactivateCallback,
		(XtCallbackProc) activateCB_quit_button_compass,
		(XtPointer) NULL );

	/* Creation of north_label */
	north_compass_label = XtVaCreateManagedWidget( "north_label",
			xmLabelGadgetClass,
			main_compass_drawing_area,
			XmNx, 360,
			XmNy, 10,
			XmNwidth, 20,
			XmNheight, 20,
			XmNfontList, labelsFontList,
			RES_CONVERT( XmNlabelString, "N" ),
			NULL );


	/* Creation of south_label */
	south_compass_label = XtVaCreateManagedWidget( "south_label",
			xmLabelGadgetClass,
			main_compass_drawing_area,
			XmNx, 360,
			XmNy, 213,
			XmNwidth, 20,
			XmNheight, 20,
			XmNfontList, labelsFontList,
			RES_CONVERT( XmNlabelString, "S" ),
			NULL );


	/* Creation of west_label */
	west_compass_label = XtVaCreateManagedWidget( "west_label",
			xmLabelGadgetClass,
			main_compass_drawing_area,
			XmNx, 257,
			XmNy, 110,
			XmNwidth, 20,
			XmNheight, 20,
			XmNfontList, labelsFontList,
			RES_CONVERT( XmNlabelString, "W" ),
			NULL );


	/* Creation of east_label */
	east_compass_label = XtVaCreateManagedWidget( "east_label",
			xmLabelGadgetClass,
			main_compass_drawing_area,
			XmNx, 460,
			XmNy, 110,
			XmNwidth, 20,
			XmNheight, 20,
			XmNfontList, labelsFontList,
			RES_CONVERT( XmNlabelString, "E" ),
			NULL );

	return ( main_compass_window );
}

/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_main_compass_window( swidget _UxUxParent )
{
  Widget                  rtrn;
  
  UxCompassParent = _UxUxParent;
  {
    source_compass_gc = XCreateGC( UxCompassDisplay, DefaultRootWindow( UxCompassDisplay ),
			   0, NULL );
    XSetForeground( UxCompassDisplay, source_compass_gc, 
		    BlackPixel( UxCompassDisplay, UxCompassScreen ) );
    XSetBackground( UxCompassDisplay, source_compass_gc, 
		    WhitePixel( UxCompassDisplay, UxCompassScreen ) );
    
    back_compass_gc = XCreateGC( UxCompassDisplay, DefaultRootWindow( UxCompassDisplay ),
			 0, NULL );
    XSetForeground( UxCompassDisplay, back_compass_gc, 
		    WhitePixel( UxCompassDisplay, UxCompassScreen ) );
    XSetBackground( UxCompassDisplay, back_compass_gc, 
		    WhitePixel( UxCompassDisplay, UxCompassScreen ) );
    
    line_compass_gc = source_compass_gc;
    rtrn = _Uxbuild_main_compass_window();
    
    return(rtrn);
  }
}
void  gt_create_compass_interface(Widget top_level, XtAppContext app)
{
  UxCompassTopLevel = top_level;
  UxCompassAppContext = app;
  UxCompassDisplay = XtDisplay(UxCompassTopLevel);
  UxCompassScreen = XDefaultScreen(UxCompassDisplay);

  compass_interface = create_main_compass_window(NULL);
}



void  gt_popup_compass_interface(void) 
{
  /*UxPutLabelString(unitname_labelGadget, "Compass Viewer");*/
  compassExposed = 1;
  UxPopupInterface(compass_interface, no_grab);
}
/*******************************************************************************
       END OF FILE
*******************************************************************************/

