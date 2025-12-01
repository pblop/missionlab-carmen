/*--------------------------------------------------------------------------
 
        telop_joystick.c
 
        Functions to implement the joystick.
 
        Author: Khaled S. Ali

        Copyright 1995, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.

--------------------------------------------------------------------------*/

/* $Id: telop_joystick.c,v 1.3 2009/01/24 02:40:11 zkira Exp $ */

#include <pthread.h>

#define _RENTRANT

#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/extensions/XInput.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include "UxXt.h"
#include <Xm/Scale.h>
#include "telop_hw_joystick.h"
#ifdef UIMX
#include <UxLib.h>
#include "UxLib.h"
#endif

#include "status_window.h"
#include "telop_aerial.h"
#include "shared.h"
#include "adjust.h"
#include "telop_joystick.h"

#define RADIANS_TO_DEGREES 57.295646
#define JOY_WIDTH  180
#define JOY_HEIGHT  180
#define TELOP_GAIN_MAX  2.0

float    getdistance();
double   getmagnitude();
point    getdirection();

extern Widget* g_joystick_drawing_area;

extern GC  	source_gc, back_gc, line_gc;
extern Display  *UxDisplay;

int  		first_time = 1;
int  		x, y;
double          alt_vel;

static pthread_mutex_t virtjoy_mutex = PTHREAD_MUTEX_INITIALIZER;

//static int mouse_enabled = 0; // indicate mouse input allowed
static int mouse_enabled = 1; // indicate mouse input allowed

void enable_mouse(void)
{
	mouse_enabled = 1;
}

void disable_mouse(void)
{
	mouse_enabled = 0;
}

/*Indicate whether ANYTHING (joystick or mouse) is enabled.
  Note that this does not have to be consistent with the other two enables.  ie. global_enable can be = 0 with
  mouse_enable = 1.  This is to remember last state.  When global_enable is set to 1, we know what to do.  */
static int global_enable = 1;

//static int joystick_enabled = 1; // indicate joystickinput allowed
static int joystick_enabled = 0; // indicate joystickinput allowed
static int deadman_armed = 0;  // indicate whether the deadman switch is armed or not

void enable_telop() {
  global_enable = 1;
  if (mouse_enabled)
    enable_mouse();
  else if (joystick_enabled)
    enable_joystick();
}

void disable_telop() {
  global_enable = 0;
  joy_stop();
}

void enable_joystick(void)
{
        // Enable the joystick if the deadman switch is armed
        if (deadman_armed)
          joystick_enabled = 1;
}

void disable_joystick(void)
{
        // Disable and stop the joystick
        joystick_enabled = 0;
        joy_stop();
}


void joy_hw_input(int axis[6]);

/* compute distance between 2 points */
float getdistance(int x1, int y1, int x2, int y2)
{
    float  a, b, c;
    a = fabs( (double) x2 - x1 );
    b = fabs( (double) y2 - y1 );
    c = sqrt( a*a + b*b );
    return( c );
}

void arm_deadman() {
  deadman_armed = 1;

  // If the mouse is enabled, don't enable the joystick too, even if the deaman switch is armed.
  if (!mouse_enabled)
    enable_joystick();

  // Update GUI to show deadman switch is armed
  status_arm_deadman();
}

void disarm_deadman() {
  deadman_armed = 0;

  if (joystick_enabled)
    disable_joystick();

  // Update GUI to show deadman switch is disarmed
  status_disarm_deadman();
}


/* compute magnitude from distance joystick is depressed, total amount that
   it can be depressed, and maximum magnitude that is wanted */
double getmagnitude(float distance, int total, float max)
 {
  float  ratio;
  ratio = distance / total;
  if ( ratio > 1.0 ) ratio = 1.0;
  return( ratio * max );
 }


/* compute direction (in radians) that joystick is depressed in 
   0 is North, increasing to the East */
point getdirection(int ox, int oy, int px, int py)
{
  point  dir; 

  dir.x = px - ox;
  dir.y = oy - py;
/*  if ( x==0.0 && y==0.0 )
    angle = 0.0;
  else
    angle = atan2(fabs((double)y),fabs((double)x));
  if ( x >= 0.0 )
    if ( y >= 0.0 )
      dir = (double) 90 - RADIANS_TO_DEGREES * angle;
    else
      dir = (double) 90 + RADIANS_TO_DEGREES * angle;
  else
    if ( y < 0.0 )
      dir = (double) 270 - RADIANS_TO_DEGREES * angle;
    else
      dir = (double) 270 + RADIANS_TO_DEGREES * angle;
  dir = fabs(dir) / RADIANS_TO_DEGREES;
*/
  return( dir );
}


/* Redraw the joystick in the case of an expose event */
void joy_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
 {
  XmDrawingAreaCallbackStruct  *call_data = (XmDrawingAreaCallbackStruct *) UxCallbackArg;

  pthread_mutex_lock(&virtjoy_mutex);
  XtAppLock(UxAppContext);
  XLockDisplay(UxDisplay);

  if (call_data->event->xexpose.count == 0)
   {
    XFillArc( UxDisplay, XtWindow(w), back_gc, 0, 0, JOY_WIDTH, JOY_HEIGHT,
              0, 23040 );
    XDrawArc( UxDisplay, XtWindow(w), source_gc, 0, 0, JOY_WIDTH, JOY_HEIGHT,
              0, 23040 );
    XFillArc( UxDisplay, XtWindow(w), source_gc,
              JOY_WIDTH/2 - 2, JOY_HEIGHT/2 - 2, 5, 5, 0, 23040 );
   }

  /* lazy initialization of a callback */
   if(!jq_get_axis_callback())jq_register_axis_callback(&joy_hw_input);

  XUnlockDisplay(UxDisplay);
  XtAppUnlock(UxAppContext);
  pthread_mutex_unlock(&virtjoy_mutex);

 }


/* clear the joystick */
void  clear(Widget w)
 {

  pthread_mutex_lock(&virtjoy_mutex );
  XtAppLock( UxAppContext );
  XLockDisplay( UxDisplay );


  XFillArc( UxDisplay, XtWindow(w), back_gc, 0, 0, JOY_WIDTH, JOY_HEIGHT,
            0, 23040 );
  XDrawArc( UxDisplay, XtWindow(w), source_gc, 0, 0, JOY_WIDTH, JOY_HEIGHT,
            0, 23040 );
  XFillArc( UxDisplay, XtWindow(w), source_gc,
            JOY_WIDTH/2 - 2, JOY_HEIGHT/2 - 2, 5, 5, 0, 23040 );

  XUnlockDisplay(UxDisplay);
  XtAppUnlock(UxAppContext);
  pthread_mutex_unlock(&virtjoy_mutex);

 }



/* --------------------------------------------------------------------------
   Any mouse button presses or releases to the joystick goes to this callback.
   The left mouse button causes a line to be drawn to the pointer position.
   This injects a vector of magnitude and direction corresponding to the
   position of the joystick.
   The middle mouse button causes the joystick to be cleared. 
   This sets the magnitude of the vector from the joystick to zero.
----------------------------------------------------------------------------*/
void joy_input(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
 {
  XmDrawingAreaCallbackStruct  *call_data = (XmDrawingAreaCallbackStruct *) UxCallbackArg;
  double  magnitude;
  point  direction;

  /*we don't want to display what the mouse is doing if disabled*/
  if(!mouse_enabled || !global_enable) return;

  switch (call_data->event->type)
   {
    case ButtonPress:
      switch (call_data->event->xbutton.button)
       {
        case Button1:
          /* giving a new magnitude and direction */
          if (!first_time)
            clear(w);
          else
            first_time = 0;
          x = call_data->event->xbutton.x;
          y = call_data->event->xbutton.y;

	  pthread_mutex_lock(&virtjoy_mutex);
	  XtAppLock(UxAppContext);
	  XLockDisplay(UxDisplay);

          XDrawLine(UxDisplay, XtWindow(w), line_gc, JOY_WIDTH/2,
                    JOY_HEIGHT/2, x, y);

	  XUnlockDisplay(UxDisplay);
	  XtAppUnlock(UxAppContext);
	  pthread_mutex_unlock(&virtjoy_mutex);

          magnitude = getmagnitude(getdistance(JOY_WIDTH/2, JOY_HEIGHT/2, x, y),
                         JOY_WIDTH/2, TELOP_GAIN_MAX);
          direction = getdirection(JOY_WIDTH/2, JOY_HEIGHT/2, x, y);

          // AERIAL
          direction.z = alt_vel;

          if (adjust_mode)
           {
            newest.teleaut_vector.magnitude = magnitude;
            newest.teleaut_vector.direction = direction;
           }
          else
           {
            normal.teleaut_vector.magnitude = magnitude;
            normal.teleaut_vector.direction = direction;
           } 
          break;
        case Button2:
          /* return joystick to normal position */
          if (!first_time)
	  {

            clear(w);
	  }
          first_time = 1;
          magnitude = 0.0;
	  direction.x = 0.0;
          direction.y = 0.0;
          direction.z = 0.0;
          if (adjust_mode)
           {
            newest.teleaut_vector.magnitude = magnitude;
            newest.teleaut_vector.direction = direction;
           }
          else
           {
            normal.teleaut_vector.magnitude = magnitude;
            normal.teleaut_vector.direction = direction;
           }   
          break;
        default:
          break;
       }
    default:
      break;
   }


  if (!adjust_mode)
    set_values(normal);

 }


void joy_hw_input(int axis[6])
{
	  double  magnitude;
	  point  direction;
	  long lxtemp;
	  long lytemp;
      long lxytemp;
	  int  ixtemp;
	  static int  first_time=1;
	  int  iytemp;
	  Widget * w;
	  //float sqroot;
	  long XMAX,YMAX,YMIN,XMIN;


	  /*don't do anything if the joystick isn't enabled*/
	  if(!joystick_enabled || !global_enable) return;


	  w=g_joystick_drawing_area;

	  XMAX=jq_get_xmax();
	  YMAX=jq_get_ymax();
	  XMIN=jq_get_xmin();
	  YMIN=jq_get_ymin();


	  /*Scale the axis data to fit joystick readings*/
	  lxtemp = axis[0];
	  lytemp = axis[1];

      lxytemp = (long)(sqrt(lxtemp*lxtemp+lytemp*lytemp));
      if(lxytemp !=0)
      {
          lxtemp=lxtemp*abs(lxtemp)/lxytemp;
          lytemp=lytemp*abs(lytemp)/lxytemp;
      }

      ixtemp = ( ((JOY_HEIGHT*(lxtemp-XMIN)) / (XMAX-XMIN)));
      iytemp = ( ((JOY_WIDTH* (lytemp-YMIN)) / (YMAX-YMIN)));

	  if(ixtemp<0) ixtemp = 1;
	  if(ixtemp>JOY_WIDTH) ixtemp = JOY_WIDTH-1;
	  if(iytemp<0) iytemp = 1;
	  if(iytemp>JOY_HEIGHT) iytemp = JOY_HEIGHT-1;

          lxtemp = x = ixtemp;
          lytemp = y = iytemp;

	  if(x<0) x = 1;
	  if(x>JOY_WIDTH) x = JOY_WIDTH-1;
	  if(y<0) y = 1;
	  if(y>JOY_HEIGHT) x = JOY_HEIGHT-1;

	  lxtemp-=JOY_WIDTH/2;
	  lytemp-=JOY_WIDTH/2;

	  if(((lxtemp*lxtemp)+(lytemp*lytemp)) > ((JOY_WIDTH/2)*(JOY_WIDTH/2)))
	  {

		/*Return if our coordinate is out of the circle bounds*/
		return;
	  }



	  /* Do the drawing stuff now*/
          if (!first_time)
	  {

	    clear(*w);
	  }
          else
          {

		first_time = 0;
	  }

	  pthread_mutex_lock(&virtjoy_mutex);
	  XtAppLock(UxAppContext);
	  XLockDisplay(UxDisplay);


          XDrawLine(UxDisplay, XtWindow(*w), line_gc, JOY_WIDTH/2,
                    JOY_HEIGHT/2, x, y);

	  XUnlockDisplay(UxDisplay);
	  XtAppUnlock(UxAppContext);
 	  pthread_mutex_unlock(&virtjoy_mutex);

          magnitude = getmagnitude(getdistance(JOY_WIDTH/2, JOY_HEIGHT/2, x, y),
                         JOY_WIDTH/2, TELOP_GAIN_MAX);
          direction = getdirection(JOY_WIDTH/2, JOY_HEIGHT/2, x, y);

          if (adjust_mode)
           {
            newest.teleaut_vector.magnitude = magnitude;
            newest.teleaut_vector.direction = direction;
           }
          else
           {
            normal.teleaut_vector.magnitude = magnitude;
            normal.teleaut_vector.direction = direction;
           } 

          if (!adjust_mode)
   	  {
    		set_values(normal);
   	  }

 }

/* --------------------------------------------------------------------------
   This is a callback for altitude velocity slider
----------------------------------------------------------------------------*/

void set_altitude_velocity(int current_value) {
  alt_vel = ((double)current_value - 100)/ 100.0;
  printf("Altitude velocity: %lf\n", alt_vel);

  if (adjust_mode)
  {
    newest.teleaut_vector.direction.z = alt_vel;
  }
  else
  {
    normal.teleaut_vector.direction.z = alt_vel;
  }
  
  if (!adjust_mode){
    set_values(normal);
  }
}

void increment_avel_val(int increment_amount) {
  int current_value = -1;

  if(!joystick_enabled || !global_enable) return;

  XmScaleGetValue(altitude_vel, &current_value);
  current_value += increment_amount;
  if (current_value > 200)
    current_value = 200;
  if (current_value < 0)
    current_value = 0;

  XmScaleSetValue(altitude_vel, current_value);
  
  set_altitude_velocity(current_value);
}

void changed_avel_val(_WidgetRec *scalewid, void * client_data, void *call_data)
{
  XmScaleCallbackStruct *cbs = (XmScaleCallbackStruct *)call_data;

  if(!joystick_enabled || !global_enable) return;

  set_altitude_velocity(cbs->value);
}

/* --------------------------------------------------------------------------
   This sets the magnitude of the vector to zero.
----------------------------------------------------------------------------*/
void joy_stop(void)
 {
  double  magnitude;
  point  direction;

  magnitude = 0.0;
  direction.x = 0.0;
  direction.y = 0.0;
  direction.z = 0.0;
  if (adjust_mode)
  {
       newest.teleaut_vector.magnitude = magnitude;
       newest.teleaut_vector.direction = direction;
  }
  else
  {
       normal.teleaut_vector.magnitude = magnitude;
       normal.teleaut_vector.direction = direction;
  }
  if (!adjust_mode)
    set_values(normal);

  if (created_altitude_vel)
    XmScaleSetValue(altitude_vel, 100);

  clear ( *g_joystick_drawing_area);
 }


/*---------------------------------------------------------------------------
   This is an event handler that takes care of the behavior when the 
   joystick is dragged.  That is, when the left mouse button is down
   and the pointer is moved.  It draws a line that follows the pointer
   around, and the vector from the joystick is changed accordingly as
   the pointer is moved.
---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------
	Not currently used because the number of messages generated
	slowed everything down significantly.
---------------------------------------------------------------------*/
#ifdef DUMMY

void joy_movement(Widget w, caddr_t client_data, XEvent call_data)
 {
  double  magnitude, direction;
  int  i;

  if ((call_data->type == MotionNotify) &&
      (call_data->xmotion.state == Button1Mask))
   {
    XDrawLine(XtDisplay(w), XtWindow(w), line_gc, JOY_WIDTH/2, JOY_HEIGHT/2,
              x, y);
    x = call_data->xmotion.x;
    y = call_data->xmotion.y;
    XDrawLine(XtDisplay(w), XtWindow(w), line_gc, JOY_WIDTH/2, JOY_HEIGHT/2,
              x, y);
    magnitude = getmagnitude(getdistance(JOY_WIDTH/2, JOY_HEIGHT/2, x, y),
                   JOY_WIDTH/2, TELOP_GAIN_MAX);
    direction = getdirection(JOY_WIDTH/2, JOY_HEIGHT/2, x, y);
   }
  if (adjust_mode)
   {
    newest.teleaut_vector.magnitude = magnitude;
    newest.teleaut_vector.direction = direction;
   }
  else
   {
    normal.teleaut_vector.magnitude = magnitude;
    normal.teleaut_vector.direction = direction;
   }
  if (!adjust_mode)
   {
    for ( i = 0; i < vehicles.num_vehicles; i++ )
     {
      DAMN_COMM_send_message(teleaut_turn_behavior[i], "VectorMsg", 
			     &normal.teleaut_vector);
      DAMN_COMM_send_message(teleaut_speed_behavior[i], "VectorMsg", 
			     &normal.teleaut_vector);
     }
   }

 }
#endif



/**********************************************************************
 * $Log: telop_joystick.c,v $
 * Revision 1.3  2009/01/24 02:40:11  zkira
 * Aerial telop
 *
 * Revision 1.2  2008/10/27 21:21:49  zkira
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
 * Revision 1.3  2003/04/02 21:22:17  zkira
 * Changed joystick enable/disable for GUI, added locks
 *
 * Revision 1.2  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.1  2000/02/29 23:55:16  saho
 * Initial revision
 *
 * Revision 1.4  1995/06/28 19:31:42  jmc
 * Added RCS id and log strings.
 **********************************************************************/
