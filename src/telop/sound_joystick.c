/**********************************************************************
 **                                                                  **
 **                      sound_joystick.c                            **
 **                                                                  **
 **  Written by:  Alexander Stoytchev                                **
 **                                                                  **
 **  Functions similar to telop to implement sound source simulation **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sound_joystick.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */


#include <math.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/X.h>
#include <X11/Xutil.h>
/* #include <X11/Xos.h> */
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>
#include "UxXt.h"

#ifdef UIMX
#include <UxLib.h>
#include "UxLib.h"
#endif

#include "shared.h"
#include "adjust.h"
#include "sound_joystick.h"

#define RADIANS_TO_DEGREES 57.295646
#define SOUND_JOY_WIDTH  180
#define SOUND_JOY_HEIGHT  180
#define SOUND_GAIN_MAX  2.0

float    sound_getdistance();
double   sound_getmagnitude();
point    sound_getdirection();

extern GC  	sound_source_gc, sound_back_gc, sound_line_gc;
extern Display  *UxDisplay;

/* double          SOUND_GAIN_MAX; */
int  		sound_first_time = 1;
int gSoundX, gSoundY;

/* compute distance between 2 points */
float sound_getdistance(int x1, int y1, int x2, int y2)
 {
  float  a, b, c;
  a = fabs( (double) x2 - x1 );
  b = fabs( (double) y2 - y1 );
  c = sqrt( a*a + b*b );
  return( c );
 }


/* compute magnitude from distance joystick is depressed, total amount that
   it can be depressed, and maximum magnitude that is wanted */
double sound_getmagnitude(float distance, int total, float max)
 {
  float  ratio;
  ratio = distance / total;
  if ( ratio > 1.0 ) ratio = 1.0;
  return( ratio * max );
 }


/* compute direction (in radians) that joystick is depressed in 
   0 is North, increasing to the East */
point sound_getdirection(int ox, int oy, int px, int py)
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
void sound_redisplay(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
 {
  XmDrawingAreaCallbackStruct  *call_data = (XmDrawingAreaCallbackStruct *) UxCallbackArg;

  if (call_data->event->xexpose.count == 0)
   {
    XFillArc( UxDisplay, XtWindow(w), sound_back_gc, 0, 0, SOUND_JOY_WIDTH, SOUND_JOY_HEIGHT,
              0, 23040 );
    XDrawArc( UxDisplay, XtWindow(w), sound_source_gc, 0, 0, SOUND_JOY_WIDTH, SOUND_JOY_HEIGHT,
              0, 23040 );
    XFillArc( UxDisplay, XtWindow(w), sound_source_gc,
              SOUND_JOY_WIDTH/2 - 2, SOUND_JOY_HEIGHT/2 - 2, 5, 5, 0, 23040 );
   }
 }


/* clear the joystick */
void  sound_clear(Widget w)
 {
  XFillArc( UxDisplay, XtWindow(w), sound_back_gc, 0, 0, SOUND_JOY_WIDTH, SOUND_JOY_HEIGHT,
            0, 23040 );
  XDrawArc( UxDisplay, XtWindow(w), sound_source_gc, 0, 0, SOUND_JOY_WIDTH, SOUND_JOY_HEIGHT,
            0, 23040 );
  XFillArc( UxDisplay, XtWindow(w), sound_source_gc,
            SOUND_JOY_WIDTH/2 - 2, SOUND_JOY_HEIGHT/2 - 2, 5, 5, 0, 23040 );
 }



/* --------------------------------------------------------------------------
   Any mouse button presses or releases to the joystick goes to this callback.
   The left mouse button causes a line to be drawn to the pointer position.
   This injects a vector of magnitude and direction corresponding to the
   position of the joystick.
   The middle mouse button causes the joystick to be cleared. 
   This sets the magnitude of the vector from the joystick to zero.
----------------------------------------------------------------------------*/
void sound_input(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg)
 {
  XmDrawingAreaCallbackStruct  *call_data = (XmDrawingAreaCallbackStruct *) UxCallbackArg;
  double  magnitude;
  point  direction;

  switch (call_data->event->type)
   {
    case ButtonPress:
      switch (call_data->event->xbutton.button)
       {
        case Button1:
          /* giving a new magnitude and direction */
          if (!sound_first_time)
	  {
            sound_clear(w);
	  }
          else
	  {
            sound_first_time = 0;
	  }

          gSoundX = call_data->event->xbutton.x;
          gSoundY = call_data->event->xbutton.y;

          XDrawLine(
	      UxDisplay,
	      XtWindow(w),
	      sound_line_gc,
	      SOUND_JOY_WIDTH/2,
	      SOUND_JOY_HEIGHT/2,
	      gSoundX,
	      gSoundY);

          magnitude = sound_getmagnitude(
	      sound_getdistance(SOUND_JOY_WIDTH/2, SOUND_JOY_HEIGHT/2, gSoundX, gSoundY),
	      SOUND_JOY_WIDTH/2,
	      SOUND_GAIN_MAX);

          direction = sound_getdirection(SOUND_JOY_WIDTH/2, SOUND_JOY_HEIGHT/2, gSoundX, gSoundY);

          if (sound_adjust_mode)
			{
            sound_newest.sound_vector.magnitude = magnitude;
            sound_newest.sound_vector.direction = direction;
           }
          else
           {
            sound_normal.sound_vector.magnitude = magnitude;
            sound_normal.sound_vector.direction = direction;
           } 
          break;
        case Button2:
          /* return joystick to normal position */
          if (!sound_first_time)
            sound_clear(w);
          sound_first_time = 1;
          magnitude = 0.0;
		  direction.x = 0.0;
          direction.y = 0.0;
          if (sound_adjust_mode)
           {
            sound_newest.sound_vector.magnitude = magnitude;
            sound_newest.sound_vector.direction = direction;
           }
          else
           {
            sound_normal.sound_vector.magnitude = magnitude;
            sound_normal.sound_vector.direction = direction;
           }   
          break;
        default:
          break;
       }
    default:
      break;
   }
  
  if (!sound_adjust_mode)
    sound_set_values(sound_normal);

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
void sound_movement(Widget w, caddr_t client_data, XEvent call_data)
 {
  double  magnitude, direction;
  int  i;

  if ((call_data->type == MotionNotify) &&
      (call_data->xmotion.state == Button1Mask))
   {
    XDrawLine
	(XtDisplay(w),
	 XtWindow(w),
	 sound_line_gc,
	 SOUND_JOY_WIDTH/2,
	 SOUND_JOY_HEIGHT/2,
	 gSoundX,
	 gSoundY);

    gSoundX = call_data->xmotion.x;
    gSoundY = call_data->xmotion.y;

    XDrawLine(
	XtDisplay(w),
	XtWindow(w),
	sound_line_gc, 
	SOUND_JOY_WIDTH/2,
	SOUND_JOY_HEIGHT/2,
	gSoundX,
	gSoundY);

    magnitude = sound_getmagnitude(
	sound_getdistance(SOUND_JOY_WIDTH/2, SOUND_JOY_HEIGHT/2, gSoundX, gSoundY),
	SOUND_JOY_WIDTH/2,
	SOUND_GAIN_MAX);

    direction = sound_getdirection(SOUND_JOY_WIDTH/2, SOUND_JOY_HEIGHT/2, gSoundX, gSoundY);
   }
  if (sound_adjust_mode)
   {
    sound_newest.sound_vector.magnitude = magnitude;
    sound_newest.sound_vector.direction = direction;
   }
  else
   {
    sound_normal.sound_vector.magnitude = magnitude;
    sound_normal.sound_vector.direction = direction;
   }
  if (!sound_adjust_mode)
   {
    for ( i = 0; i < vehicles.num_vehicles; i++ )
     {
      DAMN_COMM_send_message(teleaut_turn_behavior[i], "VectorMsg", 
			     &sound_normal.sound_vector);
      DAMN_COMM_send_message(teleaut_speed_behavior[i], "VectorMsg", 
			     &sound_normal.sound_vector);
     }
   }

 }
#endif



/**********************************************************************
 * $Log: sound_joystick.c,v $
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
 * Revision 1.1  2000/02/29 21:35:23  saho
 * Initial revision
 *
 *
 **********************************************************************/
