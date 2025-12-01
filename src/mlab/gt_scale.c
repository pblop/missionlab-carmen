/**********************************************************************
 **                                                                  **
 **                            gt_scale.c                            **
 **                                                                  **
 **   Functions and variables related to the scale of the drawing.   **
 **   (Consolidated from gt_world.*, console.*, and draw.*,          **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               ** 
 **               Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_scale.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <math.h>
#include <malloc.h>

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "gt_sim.h"
#include "console.h"
#include "draw.h"
#include "gt_map.h"

#include "gt_scale.h"



/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)


/**********************************************************************
 **                                                                  **
 **                         global variables                         **
 **                                                                  **
 **********************************************************************/

double meters_per_pixel = 1.0;
static double nominal_meters_per_pixel = 1.0;

static const double MIN_ZOOM_FACTOR = 0.1;
static const double MAX_ZOOM_FACTOR = 4.0;
static const double ZOOM_FACTOR_EPS = 0.001;
static const double ZOOM_FACTOR_INCREMENT = sqrt(2.0);
double zoom_factor = 1.0;


int scale_robots = TRUE;

static double MIN_ROBOT_LENGTH_METERS =  0.0;
static double MAX_ROBOT_LENGTH_METERS = 10.0;

static const double DEFAULT_ROBOT_LENGTH_PIXELS = 12.0;
static const double MIN_ROBOT_LENGTH_PIXELS =   1.0;
static const double MAX_ROBOT_LENGTH_PIXELS = 100.0;

const double MIN_VISBLE_ROBOT_TO_DISPLAY_RATIO = 0.02;

double robot_length_meters  = 3.0;
double robot_length_pixels  = DEFAULT_ROBOT_LENGTH_PIXELS;


/**********************************************************************
 **                                                                  **
 **                         local variables                          **
 **                                                                  **
 **********************************************************************/

static Widget scale_panel;
static Widget scale_panel_shell;
static Widget zoom_factor_scale;
static Widget robot_length_scale;
static Widget scale_robots_button;

static double saved_zoom_factor;
static double saved_robot_length;

/**********************************************************************
 **                                                                  **
 **                    local and global functions                    **
 **                                                                  **
 **********************************************************************/




/**********************************************************************
 **     update_robot_length_scale      **
 ****************************************/

static void update_robot_length_scale()
{
   XmString title;

   /* Update the robot length scale units */
   if (scale_robots) {
      title = XSTRING("Robot length (meters)");
      XtVaSetValues(robot_length_scale,
		    XmNtitleString, title,
		    XmNminimum, nint(MIN_ROBOT_LENGTH_METERS * 100.0),
		    XmNmaximum, nint(MAX_ROBOT_LENGTH_METERS * 100.0),
		    XmNvalue, nint(robot_length_meters * 100.0),
		    NULL);
      XmScaleSetValue(robot_length_scale, nint(robot_length_meters*100.0));
      }
   else {
      title = XSTRING("Robot length (pixels)");
      XtVaSetValues(robot_length_scale,
		    XmNminimum, nint(MIN_ROBOT_LENGTH_PIXELS * 100.0),
		    XmNmaximum, nint(MAX_ROBOT_LENGTH_PIXELS * 100.0),
		    XmNvalue, nint(robot_length_pixels * 100.0),
		    XmNtitleString, title,
		    NULL);
      XmScaleSetValue(robot_length_scale, nint(robot_length_pixels*100.0));
      }

   XmStringFree(title);
}


/**********************************************************************
 **         set_scale_robots          **
 ****************************************/

void set_scale_robots(int flag)
{
   erase_robots();
   scale_robots = flag;
   XtVaSetValues(scale_robots_button, XmNvalue, flag, NULL);

   update_robot_length_scale();
}


/**********************************************************************
 **        toggle_scale_robots        **
 ****************************************/

static void toggle_scale_robots()
{

   erase_robots();
   if (XmToggleButtonGetState(scale_robots_button)) {
      XmToggleButtonSetState(scale_robots_button, True, True);
      }
   else {
      XmToggleButtonSetState(scale_robots_button, False, True);
      }

   scale_robots = XmToggleButtonGetState(scale_robots_button);

   update_robot_length_scale();
}



/**********************************************************************
 **          set_robot_length          **
 ****************************************/
int set_robot_length(double len)
{
    double minValue, maxValue;

    if (scale_robots)
    {
        minValue = MIN_ROBOT_LENGTH_METERS;
        maxValue = MAX_ROBOT_LENGTH_METERS;
    }
    else
    {
        minValue = MIN_ROBOT_LENGTH_PIXELS;
        maxValue = MAX_ROBOT_LENGTH_PIXELS;
    }

    // Check against min/max values

    /* Quick Hack - Remove the check for minimum robot size and
       clamp the minimum display size to 1 pixel */
       /*   if (len < minValue)
    {
        warn_userf(
            "Robot length value, %1.8g, is less than\nthe minimum value of %1.8g",
            len,
            minValue);

        return false;
    }
    else */ if (len > maxValue)
    {
        warn_userf(
            "Robot length value, %1.8g, is greater than\nthe maximum value of %1.8g",
            len,
            maxValue);

        return false;
    }

    // set the value
    if (scale_robots)
    {
        robot_length_meters = len;
        robot_length_pixels = len / meters_per_pixel;
	// Nadeem - Just clamp the minimun pixel size to 1
	if (robot_length_pixels < 1) robot_length_pixels = 1;
    }
    else
    {
        robot_length_pixels = len;
        robot_length_meters = len * meters_per_pixel;
    }

    erase_robots();
    update_robot_length_scale();

    // If the robot is not easily visible, circle it.
    if ((MIN_VISBLE_ROBOT_TO_DISPLAY_RATIO*mission_area_width_meters) > 
        robot_length_meters)
    {
        set_circle_robots(true);
    }

    return true;
}


/**********************************************************************
 **       recompute_robot_length       **
 ****************************************/

void recompute_robot_length()
{
   double len;

   /* Get the smallest mission area dimension */
   len = mission_area_width_meters;
   if (mission_area_height_meters < len)
      len = mission_area_height_meters;
   
   /* The minimum robot length should equivalent to
      about 2 pixels across (zoomed in) */
   MIN_ROBOT_LENGTH_METERS = 0.5 * meters_per_pixel;

   /* The maximum robot length should be equivalent to
      about 1/5 of the minimum mission dimension */
   MAX_ROBOT_LENGTH_METERS = len / 5.0;

   /* Set the robot_length */
   if (scale_robots) {
      robot_length_meters = DEFAULT_ROBOT_LENGTH_PIXELS * meters_per_pixel;
      if (robot_length_meters < MIN_ROBOT_LENGTH_METERS)
	 robot_length_meters = MIN_ROBOT_LENGTH_METERS;
      else if (robot_length_meters > MAX_ROBOT_LENGTH_METERS)
	 robot_length_meters = MAX_ROBOT_LENGTH_METERS;
      }
   else
      robot_length_pixels = DEFAULT_ROBOT_LENGTH_PIXELS;

   update_robot_length_scale();
}



/**********************************************************************
 **                                                                  **
 **                 functions for scaling the world                  **
 **                                                                  **
 **********************************************************************/


/*-----------------------------------------------------------------------*/
/* scale_apply_cb: called to allow modification of the scaling parameters */
static void scale_apply_cb(Widget w, Widget base)
{
   int i;

   /* see if the world needs to get rescaled */
   XmScaleGetValue(zoom_factor_scale, &i);
   double new_zoom = (double)i / 100.0;
   if ( new_zoom != saved_zoom_factor ) {
      set_zoom_factor(new_zoom);
      saved_zoom_factor = new_zoom;
      }

   /* save the current robot length */
   XmScaleGetValue(robot_length_scale, &i);
   double new_rl =  (double)i / 100.0;
   if ( new_rl != saved_robot_length) {
      set_robot_length(new_rl);
      saved_robot_length = new_rl;
      }
}


/*-----------------------------------------------------------------------*/
/* scale_ok_cb: called to allow modification of the world config */
static void scale_ok_cb(Widget w, Widget base)
{
   XtPopdown(scale_panel_shell);

   scale_apply_cb(w, base);
}



/*-----------------------------------------------------------------------*/
/* scale_cancel_cb: called to hide the world scale panel */
static void scale_cancel_cb(Widget w, Widget base)
{
   XtPopdown(scale_panel_shell);
}


/*-----------------------------------------------------------------------*/
/* create_scale_panel: called to allow modification of the world config */
void gt_create_scale_panel(Widget parent)
{
   Widget       form1, form2, widget;

   /* create the popup */
   scale_panel_shell = XtVaCreatePopupShell(
      "World Scale", xmDialogShellWidgetClass, parent,
      XmNdeleteResponse, XmDESTROY,
      NULL);

   scale_panel = XtVaCreateWidget(
      "gt-scale-panel", xmPanedWindowWidgetClass, scale_panel_shell,
      XmNsashHeight,        1,
      XmNsashWidth,         1,
      NULL);

   form1 = XtVaCreateWidget(
      "scale-form1", xmFormWidgetClass, scale_panel,
      NULL);

   zoom_factor_scale = XtVaCreateManagedWidget(
      "zoom-factor-scale", xmScaleWidgetClass, form1,
      RES_CONVERT(XmNtitleString, "Zoom Factor (% of nominal size)"),
      XmNminimum,               nint(MIN_ZOOM_FACTOR * 100.0),
      XmNmaximum,               nint(MAX_ZOOM_FACTOR * 100.0),
      XmNvalue,                 nint(zoom_factor * 100.0),
      XmNshowValue,             True,
      XmNorientation,           XmHORIZONTAL,
      XmNtopAttachment,         XmATTACH_FORM,
      XmNleftAttachment,        XmATTACH_FORM,
      XmNrightAttachment,       XmATTACH_FORM,
      NULL);

   scale_robots_button = XtVaCreateManagedWidget(
      "scale-robots-toggle", xmToggleButtonWidgetClass, form1,
      RES_CONVERT( XmNlabelString, "Scale Robots " ),
      XmNset,                   (Boolean) scale_robots,
      XmNvalue,                 scale_robots,
      XmNindicatorType,         XmN_OF_MANY,
      XmNtopAttachment,         XmATTACH_WIDGET,
      XmNtopWidget,             zoom_factor_scale,
      XmNtopOffset,             5,
      XmNbottomAttachment,      XmATTACH_FORM,
      XmNbottomOffset,          5,
      XmNleftAttachment,        XmATTACH_FORM,
      XmNindicatorSize,         12,
      NULL);
   XtAddCallback( scale_robots_button, XmNvalueChangedCallback,
                (XtCallbackProc) toggle_scale_robots, NULL);

   robot_length_scale = XtVaCreateManagedWidget(
      "robot-length-scale", xmScaleWidgetClass, form1,
      RES_CONVERT(XmNtitleString, "Robot length (meters)"),
      XmNminimum,               nint((robot_length_meters / 10.0)*100.0),
      XmNmaximum,               nint((robot_length_meters * 10.0)*100.0),
      XmNvalue,                 nint(robot_length_meters * 100.0),
      XmNdecimalPoints,         2,
      XmNshowValue,             True,
      XmNorientation,           XmHORIZONTAL,
      XmNtopAttachment,         XmATTACH_WIDGET,
      XmNtopWidget,             zoom_factor_scale,
      XmNbottomAttachment,      XmATTACH_FORM,
      XmNleftAttachment,        XmATTACH_WIDGET,
      XmNleftWidget,            scale_robots_button,
      XmNleftOffset,            5,
      XmNrightAttachment,       XmATTACH_FORM,
      NULL);

   XtManageChild(form1);


   form2 = XtVaCreateWidget(
      "scale-form2", xmFormWidgetClass, scale_panel,
      XmNfractionBase,  7,
      XmNskipAdjust,    True,
      NULL);

   widget = XtVaCreateManagedWidget(
      "  Ok  ",   xmPushButtonGadgetClass,      form2,
      XmNtopAttachment,         XmATTACH_FORM,
      XmNbottomAttachment,      XmATTACH_FORM,
      XmNleftAttachment,        XmATTACH_POSITION,
      XmNleftPosition,          1,
      XmNrightAttachment,       XmATTACH_POSITION,
      XmNrightPosition,         2,
      XmNshowAsDefault,         True,
      XmNdefaultButtonShadowThickness,1,
      NULL);
   XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) scale_ok_cb, NULL);

   widget = XtVaCreateManagedWidget(
      " Apply ", xmPushButtonGadgetClass,       form2,
      XmNtopAttachment,         XmATTACH_FORM,
      XmNbottomAttachment,      XmATTACH_FORM,
      XmNleftAttachment,        XmATTACH_POSITION,
      XmNleftPosition,          3,
      XmNrightAttachment,       XmATTACH_POSITION,
      XmNrightPosition,         4,
      XmNshowAsDefault,         False,
      XmNdefaultButtonShadowThickness,1,
      NULL);
   XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) scale_apply_cb, NULL);

   widget = XtVaCreateManagedWidget(
      " Cancel ", xmPushButtonGadgetClass,      form2,
      XmNtopAttachment,         XmATTACH_FORM,
      XmNbottomAttachment,      XmATTACH_FORM,
      XmNleftAttachment,        XmATTACH_POSITION,
      XmNleftPosition,          5,
      XmNrightAttachment,       XmATTACH_POSITION,
      XmNrightPosition,         6,
      XmNshowAsDefault,         False,
      XmNdefaultButtonShadowThickness,1,
      NULL);
   XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) scale_cancel_cb, NULL);

   XtManageChild(form2);
}


/**********************************************************************
 **    gt_popup_scale_panel    **
 ****************************************/

void gt_popup_scale_panel()
{
   /* Update the meter_per_pixel slider values */
   XmScaleSetValue(zoom_factor_scale, nint(zoom_factor*100.0));
   saved_zoom_factor = nint(zoom_factor * 100.0) / 100.0;

   /* Update the scale_robots button and robot_length slider */
   XtVaSetValues(scale_robots_button, XmNvalue, scale_robots, NULL);
   update_robot_length_scale();
   if (scale_robots) {
      saved_robot_length = nint(robot_length_meters * 100.0) / 100.0;
      }
   else {
      saved_robot_length = nint(robot_length_pixels * 100.0) / 100.0;
      }

   /* pop it up */
   XtManageChild(scale_panel);
   XtPopup(scale_panel_shell, XtGrabNone);
}




/**********************************************************************
 **          set_zoom_factor           **
 ****************************************/

int set_zoom_factor(double new_zoom)
{
   /* Check the range of the zoom factor */
   if (new_zoom < MIN_ZOOM_FACTOR - ZOOM_FACTOR_EPS) {
      warn_userf("Error: Can't zoom out any further, the zoom factor (%1.1f%%)\n\
       must be greater than or equal to than %1.1f%%",
		 new_zoom*100.0, MIN_ZOOM_FACTOR*100.0);
      return FALSE;
      }
   else if (new_zoom > MAX_ZOOM_FACTOR + ZOOM_FACTOR_EPS) {
      warn_userf("Error: Can't zoom in any further, the zoom factor (%1.1f%%)\n\
       must be less than or equal to than %1.1f%%",
		 new_zoom*100.0, MAX_ZOOM_FACTOR*100.0);
      return FALSE;
      }
   zoom_factor = new_zoom;

   /* update meters_per_pixel */
   meters_per_pixel = nominal_meters_per_pixel / zoom_factor;

   int wid = (int)(mission_area_width_meters / meters_per_pixel);
   int hi = (int)(mission_area_height_meters / meters_per_pixel);  

   XtVaSetValues(drawing_area, XmNwidth, wid, XmNheight, hi, NULL);
     
   // create a properly sized pixmap
     XFreePixmap(XtDisplay(drawing_area), drawing_area_pixmap);
      drawing_area_pixmap = XCreatePixmap(XtDisplay(drawing_area),
				       RootWindowOfScreen(XtScreen(drawing_area)), 
				       wid,

				       hi,
				       DefaultDepthOfScreen(XtScreen(drawing_area)));
       
   
   /* Update the displays */
   XmScaleSetValue(zoom_factor_scale, nint(zoom_factor*100.0));
   draw_scale_ruler();
   if (gMapInfo->photo_loaded) {
      if (!rescale_photo_pixmap(gMapInfo, drawing_area)) {
	 warn_userf("Error rescaling photo pixmap for zoom factor = %d%%",
		    nint(zoom_factor*100.0));
	 return FALSE;
	 }
   }
   
 
   
   clear_map();
   draw_world();

   return TRUE;
}



/**********************************************************************
 **              zoom_in               **
 ****************************************/

void zoom_in()
{
   /* Zoom in means things get bigger! */

   set_zoom_factor(zoom_factor * ZOOM_FACTOR_INCREMENT);
}


/**********************************************************************
 **              zoom_out              **
 ****************************************/

void zoom_out()
{
   /* Zoom out means things get smaller! */

   set_zoom_factor(zoom_factor / ZOOM_FACTOR_INCREMENT);
}


/**********************************************************************
 **        set_meters_per_pixel        **
 ****************************************/

void set_meters_per_pixel(double mpp)
{

   meters_per_pixel = nominal_meters_per_pixel = mpp;
}



/**********************************************************************
 * $Log: gt_scale.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/08/24 22:22:34  endo
 * Draw_Neutral ghosting problem fixed.
 *
 * Revision 1.3  2007/08/10 04:41:06  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.2  2006/10/20 17:23:36  nadeem
 * Removed the check for minimum robot size. Clamped the minimum display size to 1 pixel.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/07/11 10:07:50  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.3  2005/03/23 07:36:56  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.11  1998/11/03  02:38:23  endo
 * MIN_ROBOT_LENGTH_METERS = 2.0 * meters_per_pixel was changed to
 * MIN_ROBOT_LENGTH_METERS = 0.5 * meters_per_pixel, so that
 * the minimum robot length can be 0.3 meters (1.2 meters, previoisly).
 *
 * Revision 1.10  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.9  1995/07/05  20:46:06  jmc
 * Removed select color specification so it will use the values
 * specified in the fallback resources.
 *
 * Revision 1.8  1995/06/30  18:12:26  jmc
 * Modified scale panels to allow bottom part to handle
 * resizes better.
 *
 * Revision 1.7  1995/06/13  19:48:36  jmc
 * Added code to resize photo when zoom factor is changed.
 *
 * Revision 1.6  1995/05/08  14:38:10  jmc
 * Removed obsolete comment.
 *
 * Revision 1.5  1995/05/06  22:02:26  jmc
 * Numerous changes to incorporate the idea of a zoom factor (so that
 * meters_per_pixel = nominal_meters_per_pixel / zoom_factor).
 * Converted a lot of meters_per_pixel stuff to zoom_factor (eg,
 * mpp_scale, etc).  Allow zoom factor to go from 10% to 400% (100%
 * being nominal).  Changed zoom_in and zoom_out functions to change
 * the zoom_factor, not meters_per_pixel directly.  Added
 * set_zoom_factor function (which recomputes meters_per_pixel).
 *
 * Revision 1.4  1995/05/05  17:30:10  jmc
 * Improved comments and pruned the RCS log section.
 *
 * Revision 1.3  1995/05/05  16:33:43  jmc
 * Removed some obsolete NOT_NEEDED code.
 *
 * Revision 1.2  1995/05/05  16:13:29  jmc
 * Tweaked a comment.
 *
 * Revision 1.1  1995/05/05  14:12:21  jmc
 * Initial revision
 **********************************************************************/
