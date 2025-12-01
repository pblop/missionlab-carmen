/**********************************************************************
 **                                                                  **
 **                      status_window.c                             **
 **                                                                  **
 **  Written by:  Jung-Hoon Hwang and Zsolt Kira                     **
 **  Copyright 2003, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: status_window.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#define _RENTRANT
#include <stdio.h>
#include <stdarg.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/MenuShell.h>
#include "UxXt.h"
#include <Xm/LabelG.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushBG.h>
#include <Xm/DrawnB.h>
#include <Xm/DrawingA.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <X11/Shell.h>

/*******************************************************************************
       Includes, Defines, and Global variables from the Declarations Editor:
*******************************************************************************/

/*  Authored by Khaled S. Ali and UIM/X */

#include <math.h>
#include <time.h>
#include "gt_sim.h"
#include "estop.xbm"
#include "deadman.xbm"
#include "shared.h"
#include "adjust.h"
#include "telop.h"
#include "telop_joystick.h"

extern GC  source_gc, back_gc, line_gc;
GC  red_gc, blue_gc, gray_gc;

static	Widget	status_drawing_area;
static	Widget	status_egoview_area;
static  Widget	status_targetview_area;
static	Widget	status_batteryview_area;
static	Widget	status_commview_area;
static	Widget	status_waypt_skip_button;
static	Widget	status_close_button;

static	swidget	UxParent;

Widget deadman_button_pb;
Widget estop_button_pb;

#define CONTEXT_MACRO_ACCESS 1
#include "status_window.h"
#undef CONTEXT_MACRO_ACCESS

Widget  main_status_window;
extern char * label;

#define UPDATE_DRAWABLE 5  // must be larger than 1
#define NUMOFLASER      360
#define NUMOFSONAR      20
#define INDEXOFLASER    4
#define INDEXOFSONAR    5
#define d2r             (3.14159265/180.0)
#define r2d             (180.0/3.14159265)


bool    bStart;
bool    bGUISTART = FALSE;
int     iTemp;
float   fCommHistory[40][2]; // 0~1
float   fLaser[NUMOFLASER];
int     iLaser[NUMOFLASER][2];
float   fSonar[NUMOFSONAR];
int     iSonar[NUMOFSONAR][2];
int     iStatus[10];
float   f_latitude, f_longitude, f_direction;
double  pingloss_threshold = 0, pingdelay_threshold = 0;
float   fBattery;
Vector  vObjective;


typedef struct cstrings{
    char cStr[80];
    int  iLength;
};

cstrings str1;

int iCount=0;


/*******************************************************************************
       The following are drawstring.  Draw a string on widget.
*******************************************************************************/
void XTextOut(Widget w, bool bRedraw, int x, int y, const char* ctext, ...)
{
    va_list argptr;
    char str[100];
    va_start(argptr, ctext);
    vsprintf(str,ctext, argptr);

    sprintf(str1.cStr, str); // temporary for check
    str1.iLength = strlen(str1.cStr);
    if(bRedraw)
    {
        XFillRectangle( UxDisplay, XtWindow(w), gray_gc, x-1, y-10, 1+6*str1.iLength, 11  );
    }
    XDrawString( UxDisplay, XtWindow(w), source_gc, x, y, str1.cStr, str1.iLength  );
}

void XTextOut(Widget w, int iLength, bool bRedraw, int x, int y, const char* ctext, ...)
{
    va_list argptr;
    char str[100];
    va_start(argptr, ctext);
    vsprintf(str,ctext, argptr);

    sprintf(str1.cStr, str); // temporary for check
    str1.iLength = strlen(str1.cStr)-iLength;
    if(bRedraw)
    {
        XFillRectangle( UxDisplay, XtWindow(w), gray_gc, x-1, y-10, 1+6*str1.iLength, 11  );
    }
    XDrawString( UxDisplay, XtWindow(w), source_gc, x, y, str1.cStr, str1.iLength  );
}

/*******************************************************************************
       The following are callback functions.
*******************************************************************************/

// Skip Waypoint button activated
static void activateCB_status_waypt_skip_button(Widget wgt, XtPointer cd, XtPointer cb)
{
    gt_update (BROADCAST_ROBOTID, "skip", "1");
}

// Close Telop button activated
static void activateCB_status_close_button(Widget wgt, XtPointer cd, XtPointer cb)
{
    gt_end_teleoperation(1);
}

/* initialize GC: red, blue */
void initialize_graphics_context(Widget w)
{

    Pixel fg,bg;
    XGCValues gcv;
    Display* dpy = XtDisplay( w );
    int scr = DefaultScreen( dpy );
    Colormap cmap = DefaultColormap( dpy, scr );

    XColor color, ignore;

    XtVaGetValues(
        w,
        XmNforeground, &fg,
        XmNbackground, &bg,
        NULL);

    XAllocNamedColor( dpy, cmap, "blue", &color, &ignore );
    gcv.foreground = color.pixel;
    gcv.background = bg;
    blue_gc = XtGetGC( w, GCForeground | GCBackground, &gcv );
    XSetFunction( XtDisplay( w ), blue_gc, GXcopy );

    XAllocNamedColor( dpy, cmap, "red", &color, &ignore );
    gcv.foreground = color.pixel;
    gcv.background = bg;
    red_gc = XtGetGC( w, GCForeground | GCBackground, &gcv );
    XSetFunction( XtDisplay( w ), red_gc, GXcopy );

    XAllocNamedColor( dpy, cmap, "lightgray", &color, &ignore );
    gcv.foreground = color.pixel;
    gcv.background = bg;
    gray_gc = XtGetGC( w, GCForeground | GCBackground, &gcv );
    XSetFunction( XtDisplay( w ), gray_gc, GXcopy );

    f_latitude = 0.0;
    f_longitude = 0.0;
}


// Arm deadman button (via joystick press)
void status_arm_deadman() {
    Pixel top_shadow, bottom_shadow;

    // Update the GUI.... switch top and bottom shadow colors to make it appear as if it is pressed
    XtVaGetValues( deadman_button_pb,
                   XmNtopShadowColor, &top_shadow,
                   XmNbottomShadowColor, &bottom_shadow,
                   NULL);

    XtVaSetValues( deadman_button_pb,
                   XmNtopShadowColor, bottom_shadow,
                   XmNbottomShadowColor, top_shadow,
                   NULL );
}

void status_disarm_deadman() {
    Pixel top_shadow, bottom_shadow;

    // Update the GUI.... switch top and bottom shadow colors to make it appear as if it is pressed
    XtVaGetValues( deadman_button_pb,
                   XmNtopShadowColor, &top_shadow,
                   XmNbottomShadowColor, &bottom_shadow,
                   NULL);

    XtVaSetValues( deadman_button_pb,
                   XmNtopShadowColor, top_shadow,
                   XmNbottomShadowColor, bottom_shadow,
                   NULL );
}

/* EStop button */
static void activateCB_estop_button(Widget wgt, XtPointer cd, XtPointer cb)
{
    // Pause execution
    gt_toggle_pause_execution();
}

/* Update egocentric display */
void ego_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    // draw the white area (clear background)
    if( bStart || (UxClientData!=0x0))
    {
        XFillRectangle( UxDisplay, XtWindow(w), back_gc, 1, 1, 358, 358  );
        XDrawRectangle( UxDisplay, XtWindow(w), source_gc, 0, 0, 360, 360 );

        XDrawRectangle( UxDisplay, XtWindow(w), source_gc, 175, 170, 10, 15 );
    }

    // Display laser data
    int i,j,k[2];
    float fDegree, x1,y1,x2,y2;
    if(iStatus[INDEXOFLASER])
    {
        fLaser[NUMOFLASER-1]=(float)(iTemp%30)/50+0.3;
				x1 = 180.0+180.0*fLaser[NUMOFLASER-1]*cos(d2r*(NUMOFLASER-1));
				y1 = 180.0-180.0*fLaser[NUMOFLASER-1]*sin(d2r*(NUMOFLASER-1));
				for(i=0;i<NUMOFLASER; i++)
				{
            fDegree = d2r*i;
						if ( i==0 )
						{
                k[0]=iLaser[0][0];
								k[1]=iLaser[0][1];
						}
						j=i+1;
						if (j>=NUMOFLASER )
            {
                XDrawLine( UxDisplay, XtWindow(w), back_gc, iLaser[i][0], iLaser[i][1], k[0], k[1] );
            }
            else
            {
                XDrawLine( UxDisplay, XtWindow(w), back_gc, iLaser[i][0], iLaser[i][1], iLaser[j][0], iLaser[j][1] );
            }

            fLaser[i]=(float)(iTemp%30)/50+0.3;
            x2=180.0+180.0*fLaser[i]*cos(fDegree);
            y2=180.0-180.0*fLaser[i]*sin(fDegree);

            XDrawLine( UxDisplay, XtWindow(w), blue_gc, (int)x1, (int)y1, (int)x2, (int)y2 );
            x1=x2;
            y1=y2;
            iLaser[i][0]=(int) x2;
            iLaser[i][1]=(int) y2;
       }
    }
    else if(iStatus[INDEXOFSONAR])
    {
        fSonar[NUMOFSONAR-1]=(float)(iTemp%30)/50+0.3;
        x1 = 180.0+180.0*fSonar[NUMOFSONAR-1]*cos(d2r*18*(NUMOFSONAR-1));
        y1 = 180.0-180.0*fSonar[NUMOFSONAR-1]*sin(d2r*18*(NUMOFSONAR-1));
        for(i=0;i<NUMOFSONAR; i++)
        {
            fDegree = d2r*i*18;
            if ( i==0 )
            {
                k[0]=iSonar[0][0];
                k[1]=iSonar[0][1];
            }
            j=i+1;
            if (j>=NUMOFSONAR )
            {
                XDrawLine( UxDisplay, XtWindow(w), back_gc, iSonar[i][0], iSonar[i][1], k[0], k[1] );
            }
            else
            {
                XDrawLine( UxDisplay, XtWindow(w), back_gc, iSonar[i][0], iSonar[i][1], iSonar[j][0], iSonar[j][1] );
            }
            fSonar[i]=(float)(iTemp%30)/50+0.3;
            x2=180.0+180.0*fSonar[i]*cos(fDegree);
            y2=180.0-180.0*fSonar[i]*sin(fDegree);
    
            XDrawLine( UxDisplay, XtWindow(w), red_gc, (int)x1, (int)y1, (int)x2, (int)y2 );
            x1=x2;
            y1=y2;
            iSonar[i][0]=(int) x2;
            iSonar[i][1]=(int) y2;
        }
    }
}


/* Redraw the target area in the case of an expose event */
void target_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    time_t timer;
    float fDistance, fDirection;

    // To modify the position of objective, modify vObjective !
    if (vObjective.x < 0 || vObjective.y < 0)
    {
        XTextOut(w, true, 100, 25, "N/A           ");
    }
    else
    {

        fDistance = sqrt((vObjective.x-f_latitude)*(vObjective.x-f_latitude)+(vObjective.y-f_longitude)*(vObjective.y-f_longitude));

        fDirection = d2r*f_direction - atan2((vObjective.y-f_longitude),(vObjective.x-f_latitude));
        fDirection = atan2(sin(fDirection), cos(fDirection));

        if (fDirection >= 0)
        {
            XTextOut(w, true, 100, 25, "(%.2f, %.2f) %.2f m, R%6.2f degree", vObjective.x, vObjective.y, fDistance, r2d*fDirection);
        }
        else
        {
            XTextOut(w, true, 100, 25, "(%.2f, %.2f) %.2f m, L%6.2f degree", vObjective.x, vObjective.y, fDistance, -1.0*r2d*fDirection);
        }
    }

    // Draw the data in the drawing area
    timer=time(NULL);
    XTextOut(w, 1, true, 10, 55, "GPS: %.2f %.2f  DTG: %s", f_latitude, f_longitude, asctime(localtime(&timer)));
    XTextOut(w, 1, true, 10, 55, "Target: %.2f %.2f  DTG: %s", f_latitude, f_longitude, asctime(localtime(&timer)));
}

/* Redraw the area in the case of an expose event */
void battery_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    int illl;          // in % range

    illl = 10*(iTemp%10); // temporary for check
    XFillRectangle( UxDisplay, XtWindow(w), back_gc, 120, 5, 240, 25  );
    XDrawRectangle( UxDisplay, XtWindow(w), source_gc, 120, 5, 240, 25  );

    if (fBattery > 0)
    {
        XFillRectangle( UxDisplay, XtWindow(w), source_gc, 120, 5, (int)(4.8*(float)fBattery), 25  );
        XTextOut(w, true, 230, 22, "%.2lf %s", fBattery,"V"); // temporary for check
    } else {
        XTextOut(w, true, 230, 22, "N/A");
    }
}

/* Redraw the area in the case of an expose event */
void comm_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    int i;

    XFillRectangle( UxDisplay, XtWindow(w), back_gc, 100, 5, 260, 80 );
    XDrawRectangle( UxDisplay, XtWindow(w), source_gc, 100, 5, 260, 80 );

    XDrawLine( UxDisplay, XtWindow(w), blue_gc,
               101, 85-(int)(30*log10(1.0+pingdelay_threshold)),
	             101+260, 85-(int)(30*log10(1.0+pingdelay_threshold)) );

    XDrawLine( UxDisplay, XtWindow(w), red_gc,
               101, 85-(int)(30*log10(1.0+pingloss_threshold)),
	             101+260, 85-(int)(30*log10(1.0+pingloss_threshold)) );

    for (i=0;i<39;i++)
    {
        fCommHistory[i][0]=fCommHistory[i+1][0];
        fCommHistory[i][1]=fCommHistory[i+1][1];
    }

    for (i=0;i<39;i++)
    {
        XDrawLine( UxDisplay, XtWindow(w), blue_gc, 101+(int)(i*260)/40, 85-(int)(30*log10(1.0+fCommHistory[i][0])), 101+(int)((i+1)*260)/40, 85-(int)(30*log10(1.0+fCommHistory[i+1][0])) );
        XDrawLine( UxDisplay, XtWindow(w), red_gc, 101+(int)(i*260)/40, 85-(int)(70.0/100.0*fCommHistory[i][1]), 101+(int)((i+1)*260)/40, 85-(int)(70.0/100.0*fCommHistory[i+1][1]) );
    }

    XTextOut(w, true, 10, 50, "Delay%5.2f%s",(fCommHistory[39][0]),"ms"); // temporary for check
    XDrawLine( UxDisplay, XtWindow(w), blue_gc, 80, 45, 90, 45);
    XTextOut(w, true, 10, 70, "Loss %3d%s",(int)(fCommHistory[39][1]),"%%"); // temporary for check
    XDrawLine( UxDisplay, XtWindow(w), red_gc, 80, 65, 90, 65);
}


/*******************************************************************************
        Update each drawable by force.
*******************************************************************************/

// zk for updating of the gps label (can be used for others too)
// hoon mod. ( the reason of putting the update_rate is the bug of X-window : bad drawable w/o reason !!! )
void UpdateAllDrawables()
{
    char latitude[15];
    char longitude[15];
    char direction[15];
    char pingtime[15];
    char pingloss[15];
    char spingdelay_threshold[15];
    char spingloss_threshold[15];
    char battery_level[15];

    char *sObjective;
    float f_goal_latitude, f_goal_longitude;

    int sret;
    int robotID = find_first_robot_rec();

    if ((gt_inquire (robotID, "latitude") !=NULL) && (gt_inquire(robotID, "longitude") !=NULL))
    {
        strcpy (latitude, gt_inquire (robotID, "latitude"));
        strcpy (longitude, gt_inquire (robotID, "longitude"));
        strcpy (direction, gt_inquire (robotID, "direction"));
        strcpy (pingtime, gt_inquire (robotID, "pingtime"));
        strcpy (pingloss, gt_inquire (robotID, "pingloss"));
        strcpy (battery_level, gt_inquire (robotID, "battery_level"));
        strcpy (spingloss_threshold, gt_inquire (robotID, "pingloss_threshold"));
        strcpy (spingdelay_threshold, gt_inquire (robotID, "pingdelay_threshold"));

        f_latitude = (float) atof(latitude);
        f_longitude = (float) atof(longitude);
        f_direction = (float) atof(direction);
        fCommHistory[39][0] = (float) atof(pingtime);
        fCommHistory[39][1] = (float) atof(pingloss);
        fBattery = (float) 0.1 * atof(battery_level);
        sscanf(spingloss_threshold, "%lf", &pingloss_threshold);
        sscanf(spingdelay_threshold, "%lf", &pingdelay_threshold);
    }
    else
    {
        strcpy (latitude, "N/A      ");
        strcpy (longitude, "N/A     ");
    }

    if ( (sObjective = gt_inquire(robotID, "to_goal")) )
    {
        sret = sscanf(sObjective, "%f %f", &f_goal_latitude, &f_goal_longitude);

        if ( sret != 2)
        {
            vObjective.x = -1;
      	    vObjective.y = -1;
        }
        else
        {
            vObjective.x = f_goal_latitude;
            vObjective.y = f_goal_longitude;
        }
    }
    else
    {
        vObjective.x = -1;
        vObjective.y = -1;
    }

    iCount++;
    if(iCount>=UPDATE_DRAWABLE)
    {
        iCount = 0;
        if ( main_status_window )
        {
            target_redisplay(status_targetview_area, (XtPointer) 0x0, (XtPointer) 0x0);
            battery_redisplay(status_batteryview_area, (XtPointer) 0x0, (XtPointer) 0x0);
            comm_redisplay(status_commview_area, (XtPointer) 0x0, (XtPointer) 0x0);
            bStart = FALSE;
        }
    }
}

void UpdateTargetDisplay()
{
   if ( bTelopStart )
        UpdateAllDrawables();

}

/*******************************************************************************
       The 'build_' function creates all the widgets
       using the resource values specified in the Property Editor.
*******************************************************************************/

static Widget	_Uxbuild_main_status_window()
{
    int x_offset = 0;
    int y_offset = 0;

    Widget		_UxParent;
    Widget      text_label;
    Pixel fg,bg;
    XGCValues gcv;
    Pixmap button_pixmap;

    /* Creation of main_status_window */
    _UxParent = UxParent;
    if ( _UxParent == NULL )
    {
        _UxParent = UxTopLevel;
    }

    x_offset =  XWidthOfScreen(XtScreen(_UxParent)) - 1280;

    main_status_window = XtVaCreatePopupShell(
        "main_status_window",
        applicationShellWidgetClass,
        _UxParent,
        XmNwidth, 380,
        XmNheight, 765,
        XmNx, 117 + x_offset,
        XmNy, 601 + y_offset,
        XmNtitle, "Status",
        XmNtransient, TRUE,
        XmNmwmDecorations, 0, /* This line makes unframed window */
	      NULL );

    /* Creation of status_drawing_area */
    status_drawing_area = XtVaCreateManagedWidget(
        "status_drawing_area",
				xmDrawingAreaWidgetClass,
				main_status_window,
				XmNresizePolicy, XmRESIZE_NONE,
				XmNwidth, 380,
				XmNheight, 765,
				XmNx, 1195,
				XmNy, -5,
				XmNunitType, XmPIXELS,
				NULL );

    initialize_graphics_context(status_drawing_area);

    XtVaGetValues(
        status_drawing_area,
				XmNforeground, &fg,
				XmNbackground, &bg,
				NULL);

    gcv.foreground = bg;

    /* drawing the Deadman button */
    button_pixmap = XCreatePixmapFromBitmapData(
        XtDisplay (status_drawing_area),
				RootWindowOfScreen(XtScreen(status_drawing_area)),
				deadman_bits,deadman_width,deadman_height,
				fg,bg,
				DefaultDepthOfScreen(XtScreen(status_drawing_area)));

    /* Creation of Deadman button */
    deadman_button_pb = XtVaCreateManagedWidget(
				"button_pb",
				xmPushButtonGadgetClass,//xmDrawnButtonWidgetClass,
				status_drawing_area,
				XmNlabelType, XmPIXMAP,
				XmNlabelPixmap, button_pixmap,
				XmNx, 10,
				XmNy, 600,
				XmNshadowType, XmSHADOW_OUT,
				XmNpushButtonEnabled, true,
				NULL );

    /* drawig the E-Stop button */
    button_pixmap = XCreatePixmapFromBitmapData(
        XtDisplay (status_drawing_area),
				RootWindowOfScreen(XtScreen(status_drawing_area)),
				estop_bits,estop_width,estop_height,
				fg,bg,
				DefaultDepthOfScreen(XtScreen(status_drawing_area)));

    XDrawArc( XtDisplay(status_drawing_area), button_pixmap, red_gc, 0, 0, 99, 99, 0, 23040 );
    XFillArc( XtDisplay(status_drawing_area), button_pixmap, red_gc, 10, 10, 79, 79, 0, 23040 );

    sprintf(str1.cStr,"E-Stop");
    str1.iLength=strlen(str1.cStr);
    XDrawString( XtDisplay (status_drawing_area), button_pixmap, back_gc, 35, 55, str1.cStr, str1.iLength  );

    /* Creation of E-Stop button */
    estop_button_pb = XtVaCreateManagedWidget(
				"estop_button_pb",
				xmToggleButtonGadgetClass,//xmDrawnButtonWidgetClass,
				status_drawing_area,
				XmNlabelType, XmPIXMAP,
				XmNlabelPixmap, button_pixmap,
				XmNx, 150,
				XmNy, 600,
				XmNshadowType, XmSHADOW_OUT,
				XmNpushButtonEnabled, true,
				NULL );

    XtAddCallback(
				estop_button_pb,
				XmNvalueChangedCallback,
				(XtCallbackProc) activateCB_estop_button,
				(XtPointer) NULL );

    /* Creation of status_egoview_area */
    status_egoview_area = XtVaCreateManagedWidget(
				"status_egoview_area",
				xmDrawingAreaWidgetClass,
				status_drawing_area,
				XmNresizePolicy, XmRESIZE_NONE,
				XmNwidth, 370,
				XmNheight, 370,
				XmNx, 5,
				XmNy, 5,
				XmNunitType, XmPIXELS,
				NULL );

    XtAddCallback(
				status_egoview_area,
				XmNexposeCallback,
				(XtCallbackProc) ego_redisplay,
				(XtPointer) 0x1 );

    /* Creation of status_targetview_area */
    status_targetview_area = XtVaCreateManagedWidget(
				"status_targetview_area",
				xmDrawingAreaWidgetClass,
				status_drawing_area,
				XmNresizePolicy, XmRESIZE_NONE,
				XmNwidth, 370,
				XmNheight, 60,
				XmNx, 5,
				XmNy, 380,
				NULL );

    XtAddCallback(
				status_targetview_area,
				XmNexposeCallback,
				(XtCallbackProc) target_redisplay,
				(XtPointer) 0x0 );

    text_label = XtVaCreateManagedWidget(
				"text_label",
				xmLabelGadgetClass,
				status_targetview_area,
				XmNx, 10,
				XmNy, 10,
				XmNwidth, 360,
				XmNheight, 20,
				XmNalignment, XmALIGNMENT_BEGINNING,
				RES_CONVERT( XmNlabelString, "Objective" ),
				NULL );

    /* Creation of status_batteryview_area */
    status_batteryview_area = XtVaCreateManagedWidget(
				"status_batteryview_area",
				xmDrawingAreaWidgetClass,
				status_drawing_area,
				XmNresizePolicy, XmRESIZE_NONE,
				XmNwidth, 370,
				XmNheight, 35,
				XmNx, 5,
				XmNy, 445,
				XmNunitType, XmPIXELS,
				NULL );

    XtAddCallback(
				status_batteryview_area,
				XmNexposeCallback,
				(XtCallbackProc) battery_redisplay,
				(XtPointer) 0x0 );

    text_label = XtVaCreateManagedWidget(
				"text_label",
				xmLabelGadgetClass,
				status_batteryview_area,
				XmNx, 10,
				XmNy, 10,
				XmNwidth, 360,
				XmNheight, 20,
				XmNalignment, XmALIGNMENT_BEGINNING,
				RES_CONVERT( XmNlabelString, "Battery : " ),
				NULL );
		
    /* Creation of status_commview_area */
    status_commview_area = XtVaCreateManagedWidget(
				"status_commview_area",
				xmDrawingAreaWidgetClass,
				status_drawing_area,
				XmNresizePolicy, XmRESIZE_NONE,
				XmNwidth, 370,
				XmNheight, 110,
				XmNx, 5,
				XmNy, 485,
				XmNunitType, XmPIXELS,
				NULL );
		
    XtAddCallback(
				status_commview_area,
				XmNexposeCallback,
				(XtCallbackProc) comm_redisplay,
				(XtPointer) 0x0 );
		
    text_label = XtVaCreateManagedWidget(
				"text_label",
				xmLabelGadgetClass,
				status_commview_area,
				XmNx, 10,
				XmNy, 10,
				XmNwidth, 360,
				XmNheight, 20,
				XmNalignment, XmALIGNMENT_BEGINNING,
				RES_CONVERT( XmNlabelString, "Comm. " ),
				NULL );

		/* Creation of waypt_skip_button */
		status_waypt_skip_button = XtVaCreateManagedWidget(
				"status_waypt_skip_button",
				xmPushButtonGadgetClass,
				status_drawing_area,
				XmNx, 150,
				XmNy, 722,
				XmNwidth, 130,
				XmNheight, 30,
				RES_CONVERT( XmNlabelString, "Skip Waypoint" ),
				XmNsensitive, TRUE,
				NULL );
		
    XtAddCallback(
				status_waypt_skip_button,
				XmNactivateCallback,
				(XtCallbackProc) activateCB_status_waypt_skip_button,
				(XtPointer) NULL );
		
		status_close_button = XtVaCreateManagedWidget(
				"status_close_button",
				xmPushButtonGadgetClass,
				status_drawing_area,
				XmNx, 10,
				XmNy, 722,
				XmNwidth, 100,
				XmNheight, 30,
				RES_CONVERT( XmNlabelString, " Close Telop " ),
				XmNsensitive, TRUE,
				NULL );

    XtAddCallback(
				status_close_button,
				XmNactivateCallback,
				(XtCallbackProc) activateCB_status_close_button,
				(XtPointer) NULL );

    return ( main_status_window );
}


/*******************************************************************************
       The following is the 'Interface function' which is the
       external entry point for creating this interface.
       This function should be called from your application or from
       a callback function.
*******************************************************************************/

Widget	create_main_status_window( swidget _UxUxParent )
{
		Widget  rtrn;
    int i;

    bStart = TRUE;
    iTemp = 0;
    for (i=0;i<40;i++)
    {
        fCommHistory[i][0] = 0.0;
        fCommHistory[i][1] = 0.0;
    }
    for (i=0;i<10;i++)
    {
        iStatus[i] = 0;
    }
    for (i=0;i<NUMOFLASER;i++)
    {
        fLaser[i] = 0.5;
        iLaser[i][0] = 0;
        iLaser[i][1] = 0;
    }
    for (i=0;i<NUMOFSONAR;i++)
    {
        fSonar[i] = 0.0;
        iSonar[i][0] = 0;
        iSonar[i][1] = 0;
    }
    vObjective.x = -1.0;
    vObjective.y = -1.0;


    UxParent = _UxUxParent;

    {
        source_gc = XCreateGC( UxDisplay, DefaultRootWindow( UxDisplay ), 0, NULL );
        XSetForeground( UxDisplay, source_gc, BlackPixel( UxDisplay, UxScreen ) );
        XSetBackground( UxDisplay, source_gc, WhitePixel( UxDisplay, UxScreen ) );

        back_gc = XCreateGC( UxDisplay, DefaultRootWindow( UxDisplay ), 0, NULL );
        XSetForeground( UxDisplay, back_gc, WhitePixel( UxDisplay, UxScreen ) );
        XSetBackground( UxDisplay, back_gc, WhitePixel( UxDisplay, UxScreen ) );


        line_gc = source_gc;
        rtrn = _Uxbuild_main_status_window();
        bGUISTART = TRUE;

        return(rtrn);
    }
}

void status_report_sensors( int robot_id, int cnt, SensorReading* reading)
{

#define SENSOR_SICKLMS200      0
#define SENSOR_SONAR           1
#define SENSOR_IR              2
#define SENSOR_XYT             3
#define SENSOR_PANTILT_CAMERA  4
#define SENSOR_COLOR_OBJECTS   5
#define SENSOR_ARM             6
#define SENSOR_VIDEO           7
#define SENSOR_CAMERA_FOCUS    8

    float meters_per_pixel = 1.0/20.0; // zzk , jh

    Widget drawing_area = status_egoview_area;  // zk change this!, jh

    static int initialized = 0;
    static int lastCnt;
    static float lastHeading;
    static gt_Point lastLoc;

    static int num_laser_points = 0;
    static int num_laser_danger_points = 0;
    static int num_laser_linear_points = 0;
    static XPoint* laser_danger_points = NULL;
    static XPoint* laser_points = NULL;
    static XPoint* laser_linear_points = NULL;

    static int num_color_points = 0;
    static XPoint color_points[ 32 ];
    int color_point_colors[ 32 ];

    static int num_sonar_points = 0;
    static XPoint sonar_points[ 32 ];
    static XArc sonar_arcs[ 32 ];
    int x2,y2,d;
    static int erase_sonar_points = 0,erase_sonar_arcs = 0;

    int i,x,y;
    double angle;
    Vector v;

    static GC whiteGC;
    static GC colorGC[ 7 ];
    /*
    char color_names[ 7 ][ 20 ] = {
        "midnight blue", // laser
        "black",         // sonar, arm
        "orange",        // ir
        "red","green","yellow", // color objects
        "purple",        // pan/tilt camera, camera_focus
    };
    */
    // If teloperation window is not started, quit
    if ( !bTelopStart )
    {
				return;
    }

    if ( !initialized )
    {
        int laser_count = 0;

        initialized = 1;

        // get the number of laser readings
        for ( i = 0; i < cnt; i++ )
        {
            if ( reading[ i ].type == SENSOR_SICKLMS200 )
            {
                laser_count++;
            }
        }
        // allocate space for laser reading data. These are never deallocated
        // since they stay around for the entire life of the program.
        if ( laser_count > 0 )
        {
            laser_danger_points = new XPoint[ laser_count ];
            laser_points        = new XPoint[ laser_count ];
            laser_linear_points = new XPoint[ laser_count ];
        }

        lastCnt = cnt;
        lastLoc.x = 0; // zzk cur->loc.x;
        lastLoc.y = 0; // zzk cur->loc.y;
        lastHeading = 0; // zzk cur->heading;
        for ( i = 0; i < 7; i++ )
        {
            colorGC[ i ] = source_gc; // zk wch_lookup_color( color_names[ i ] );
        }
        whiteGC = back_gc; // zk wch_lookup_color( "white" );
    }

    if ( erase_obstacles )
    {
        if ( num_laser_points )
        {
            if ( laser_normal_mode )
            {
                if ( laser_connected_mode )
                {
                    XDrawLines( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                                whiteGC, laser_points, num_laser_points, CoordModeOrigin );
                    XDrawLines( XtDisplay( drawing_area ), drawing_area_pixmap,
                                whiteGC, laser_points, num_laser_points, CoordModeOrigin );
                }
                else
                {
                    XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                                 whiteGC, laser_points, num_laser_points, CoordModeOrigin );
                    XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                                 whiteGC, laser_danger_points, num_laser_danger_points,
                                 CoordModeOrigin );

                    XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                                 whiteGC, laser_points, num_laser_points, CoordModeOrigin );
                    XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                                 whiteGC, laser_danger_points, num_laser_danger_points,
                                 CoordModeOrigin );
                }
            }
        }
        if ( num_laser_linear_points )
        {
            if ( laser_linear_mode )
            {
                XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                             whiteGC, laser_linear_points, num_laser_linear_points,
                             CoordModeOrigin );
                XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                             whiteGC, laser_linear_points, num_laser_linear_points,
                             CoordModeOrigin );
            }
        }
        if ( num_color_points )
        {
            for ( i = 0; i < num_color_points; i++ )
            {
                XFillArc( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                          whiteGC, color_points[ i ].x, color_points[ i ].y,
                          10, 10, 0, 360 * 64 );
                XFillArc( XtDisplay( drawing_area ), drawing_area_pixmap,
                          whiteGC, color_points[ i ].x, color_points[ i ].y,
                          10, 10, 0, 360 * 64 );
            }
        }
        if ( num_sonar_points)
        {
            if ( erase_sonar_arcs )
            {
                XDrawArcs( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                           whiteGC, sonar_arcs, num_sonar_points );
                XDrawArcs( XtDisplay( drawing_area ), drawing_area_pixmap,
                           whiteGC, sonar_arcs, num_sonar_points );
                erase_sonar_arcs = 0;
            }
            if ( erase_sonar_points )
            {
                for ( i = 0; i < num_sonar_points; i++ )
                {
                    XFillArc( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                              whiteGC, sonar_points[ i ].x, sonar_points[ i ].y,
                              5, 5, 0, 360 * 64 );
                    XFillArc( XtDisplay( drawing_area ), drawing_area_pixmap,
                              whiteGC, sonar_points[ i ].x, sonar_points[ i ].y,
                              5, 5, 0, 360 * 64 );
                }
                erase_sonar_points = 0;
            }
        }
    }

    num_laser_points = 0;
    num_laser_danger_points = 0;
    num_laser_linear_points = 0;
    num_color_points = 0;
    num_sonar_points = 0;

    for ( i = 0; i < cnt; i++ )
    {
        v.x = reading[ i ].reading;
        v.y = 0;
        angle = reading[ i ].angle + 90;  // zzk cur->heading + reading[ i ].angle;
        deg_range_pm_360( angle );
        rotate_z( v, angle );
        x = 180 + (int) ( v.x / meters_per_pixel );     // zzk 180-> jh
        y = 180 - (int) ( v.y / meters_per_pixel );     // zzk 180-> jh
        switch ( reading[ i ].type )
        {
        case SENSOR_SICKLMS200:
            if ( laser_connected_mode )
            {
                laser_points[ num_laser_points ].x = x;
                laser_points[ num_laser_points ].y = y;
                num_laser_points++;
            }
            else
            {
                if ( reading[ i ].color )
                {
                    laser_danger_points[ num_laser_danger_points ].x = x;
                    laser_danger_points[ num_laser_danger_points ].y = y;
                    num_laser_danger_points++;
                }
                else
                {
                    laser_points[ num_laser_points ].x = x;
                    laser_points[ num_laser_points ].y = y;
                    num_laser_points++;
                }
            }
            if ( laser_linear_mode )
            {
                laser_linear_points[ num_laser_linear_points ].x =
                    180 + num_laser_linear_points;             // 180-> jh
                laser_linear_points[ num_laser_linear_points ].y =
                    180 -                                      // 180-> jh
                    (int)( ( reading[ num_laser_linear_points ].reading + 1 ) / meters_per_pixel );
                num_laser_linear_points++;
            }
            break;
        case SENSOR_COLOR_OBJECTS:
            color_point_colors[ num_color_points ] = reading[ i ].color + 3;
            color_points[ num_color_points ].x = x;
            color_points[ num_color_points ].y = y;
            num_color_points++;
            break;
        case SENSOR_SONAR:
            sonar_points[ num_sonar_points ].x = x;
            sonar_points[ num_sonar_points ].y = y;
            if (sonar_arc_mode) {
                d = (int) ( reading[ i ].reading / meters_per_pixel );
                x2 = 180;  // zzk (int) ( cur->loc.x / meters_per_pixel );       , 180-> jh
                y2 = 180;  // zzk drawing_area_height_pixels - (int) ( cur->loc.y / meters_per_pixel );, 180-> jh
                sonar_arcs[ num_sonar_points ].width = (int) ( d * 2 );
                sonar_arcs[ num_sonar_points ].height = (int) ( d * 2 );
                sonar_arcs[ num_sonar_points ].angle1 = (short) ( ( angle - 15.0 ) * 64.0 );
                sonar_arcs[ num_sonar_points ].angle2 = 30 * 64;
                sonar_arcs[ num_sonar_points ].x = x2 - d;
                sonar_arcs[ num_sonar_points ].y = y2 - d;
            } else {
                sonar_points[ num_sonar_points ].x = x;
                sonar_points[ num_sonar_points ].y = y;
            }
            num_sonar_points++;
            break;
        }
    }

    if ( num_laser_points )
    {
	    if ( laser_normal_mode )
			{
            if ( laser_connected_mode )
            {
                XDrawLines( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                            colorGC[ 0 ], laser_points, num_laser_points,CoordModeOrigin );
                XDrawLines( XtDisplay( drawing_area ), drawing_area_pixmap,
                            colorGC[ 0 ], laser_points, num_laser_points,CoordModeOrigin );
            } else {
                XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                             colorGC[ 0 ], laser_points, num_laser_points,
                             CoordModeOrigin );
                XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                             colorGC[3], laser_danger_points, num_laser_danger_points,
                             CoordModeOrigin);

                XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                             colorGC[ 0 ], laser_points, num_laser_points,
                             CoordModeOrigin );
                XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                             colorGC[3], laser_danger_points, num_laser_danger_points,
                             CoordModeOrigin);
            }
	    }
    }
    if ( num_laser_linear_points )
    {
        if ( laser_linear_mode )
        {
            XDrawPoints( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                         colorGC[ 0 ], laser_linear_points, num_laser_linear_points,
                         CoordModeOrigin );
            XDrawPoints( XtDisplay( drawing_area ), drawing_area_pixmap,
                         colorGC[ 0 ], laser_linear_points, num_laser_linear_points,
                         CoordModeOrigin );
        }
    }

    if ( num_color_points )
    {
        for ( i = 0; i < num_color_points; i++ )
        {
            XFillArc( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                      colorGC[ color_point_colors[ i ] ], color_points[ i ].x, color_points[ i ].y,
                      10, 10, 0, 360 * 64 );
            XFillArc( XtDisplay( drawing_area ), drawing_area_pixmap,
                      colorGC[ color_point_colors[ i ] ], color_points[ i ].x, color_points[ i ].y,
                      10, 10, 0, 360 * 64 );
        }
    }

    if ( num_sonar_points )
    {
        if ( sonar_arc_mode )
        {
            XDrawArcs( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                       colorGC[ 1 ], sonar_arcs, num_sonar_points );
            XDrawArcs( XtDisplay( drawing_area ), drawing_area_pixmap,
                       colorGC[ 1 ], sonar_arcs, num_sonar_points );
            erase_sonar_arcs = 1;
        }
        if ( sonar_point_mode )
        {
            for ( i = 0; i < num_sonar_points; i++ )
            {
                XFillArc( XtDisplay( drawing_area ), XtWindow( drawing_area ),
                          colorGC[ 1 ], sonar_points[ i ].x, sonar_points[ i ].y,
                          5, 5, 0, 360 * 64 );
                XFillArc( XtDisplay( drawing_area ), drawing_area_pixmap,
                          colorGC[ 1 ], sonar_points[ i ].x, sonar_points[ i ].y,
                          5, 5, 0, 360 * 64 );
                erase_sonar_points = 1;
            }
        }
    }
}

/**********************************************************************
 * $Log: status_window.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 06:39:58  endo
 * gcc-3.4 upgrade.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/02 21:14:57  zkira
 * Initial revision
 *
 **********************************************************************/

