/*---------------------------------------------------------------------------
 
        adjust.h
 
        Functions for handling the movement of slider-bars.
        Currently, there are only two slider-bars, but there
        will be more later.  At least one for each behavior.
        Also for handling the reset button and the commit button.
 
        Author: Khaled S. Ali
 
        Copyright 1995, Georgia Tech Research Corporation
        Atlanta, Georgia  30332-0415
        ALL RIGHTS RESERVED, See file COPYRIGHT for details.

---------------------------------------------------------------------------*/

/* $Id: adjust.c,v 1.2 2008/10/27 21:21:50 zkira Exp $ */


#include <math.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Scale.h>

#ifdef UIMX
#include <UxLib.h>
#include "UxLib.h"
#include "UxScale.h"
#endif

#include "adjust.h"
#include "shared.h"
#include "telop_window.h"
#include "gt_com.h"
#include "telop.h"

/*-------------------------------------------------------------------------
	Global Variables
-------------------------------------------------------------------------*/

/*
 // The following 4 variables are used for implementing the Reset Parameters
   button and the Delayed mode.
*/
struct values_type  original;
struct values_type  normal;
struct values_type  newest;

struct sound_values_type  sound_original;
struct sound_values_type  sound_normal;
struct sound_values_type  sound_newest;


int  adjust_mode = 0;
int  sound_adjust_mode = 0;
//int  telop_mode = 1;
int  telop_mode = 0;


/*****************************  TELOP  **********************************/

/*-------------------------------------------------------------------------

	set_value puts the values of the joystick and slider_bars in the
	databases for the robots.

-------------------------------------------------------------------------*/
void set_values(values_type values)
{
   int i;
   char  value[100];

   if (telop_num_robots <= 0)
   {
      /* No unit specified, so broadcast */
      sprintf(value, "%f", values.teleaut_vector.direction.x);
      gt_update(BROADCAST_ROBOTID, "joystick_x", value);
  
      sprintf(value, "%f", values.teleaut_vector.direction.y);
      gt_update(BROADCAST_ROBOTID, "joystick_y", value);

      sprintf(value, "%f", values.teleaut_vector.magnitude);
      gt_update(BROADCAST_ROBOTID, "joystick_magnitude", value);
      
      sprintf(value, "%d", telop_mode);
      gt_update(BROADCAST_ROBOTID, "telop_mode", value);    

      // AERIAL
      sprintf(value, "%f", values.teleaut_vector.direction.z);
      gt_update(BROADCAST_ROBOTID, "joystick_z", value);
   }
   else 
   {
      for (i=0; i<telop_num_robots; i++)
      {
        /* Send the messages to each robot in the unit */
        sprintf(value, "%f", values.teleaut_vector.direction.x);
        gt_update(telop_robot[i], "joystick_x", value);
        
        sprintf(value, "%f", values.teleaut_vector.direction.y);
        gt_update(telop_robot[i], "joystick_y", value);
        
        sprintf(value, "%f", values.teleaut_vector.magnitude);
        gt_update(telop_robot[i], "joystick_magnitude", value);
        
        // AERIAL
        sprintf(value, "%f", values.teleaut_vector.direction.z);
        gt_update(telop_robot[i], "joystick_z", value);
        
        sprintf(value, "%d", telop_mode);
        gt_update(telop_robot[i], "telop_mode", value);  	 
      }
   }
}


/*--------------------------------------------------------------------------
	reset sets the slider-bars back to their original values
--------------------------------------------------------------------------*/

void reset(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    set_values(original); 
}


/*--------------------------------------------------------------------------
	commit sends all the values of the joystick and slider-bars
	to the arbiters and teleautonomy behaviors.
--------------------------------------------------------------------------*/

void commit(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    set_values(newest); 
}







//************************************************************************
//                             SOUND                                    
//************************************************************************

//-----------------------------------------------------------------------
// set_value puts the values of the sound_joystick and slider_bars in the
// databases for the robots.
//-----------------------------------------------------------------------

void sound_set_values(sound_values_type values)
{
    char  value[100];

    sprintf(value, "%f", values.sound_vector.direction.x);
    gt_update(BROADCAST_ROBOTID, "sound_direction_x", value);
   
    sprintf(value, "%f", values.sound_vector.direction.y);
    gt_update(BROADCAST_ROBOTID, "sound_direction_y", value);
   
    sprintf(value, "%f", 5.0*values.sound_vector.magnitude);
    gt_update(BROADCAST_ROBOTID, "sound_volume", value);
}


/*--------------------------------------------------------------------------
	reset sets the slider-bars back to their original values
--------------------------------------------------------------------------*/
void sound_reset(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg)
{
  sound_set_values(sound_original); 
}


/*--------------------------------------------------------------------------
	commit sends all the values of the sound_joystick and slider-bars
	to the arbiters and sound behaviors.
--------------------------------------------------------------------------*/
void sound_commit(Widget UxWidget, XtPointer UxClientData, XtPointer UxCallbackArg)
{
    sound_set_values(sound_newest); 
}



/*****************************  SOUND  **********************************/









/*-------------------------------------------------------------------------
  
        Function for adjusting the value controlled by a personality slider 
	bar
  
-------------------------------------------------------------------------*/
void  adjust_personality(Widget w, XtPointer client_data, XmScaleCallbackStruct *call_data)
{
  personality_type * slider = (personality_type *) client_data;
  int  value = (int) call_data->value;
  float  max_value, min_value, new_value;
  char  valuestr[100]; 
  int  i, j;
  int  number_parameters;

  number_parameters = slider->num_params;
  
  for (j=0; j<number_parameters; j++)
   {
    max_value = slider->params[j].base * 2;
    min_value = 0.0;
    if ( slider->params[j].inc )
      new_value = min_value + (max_value - min_value) * ( (float) value / 100);
    else
      new_value = min_value + (max_value - min_value) * 
		   ( (float) (100 - value) / 100 ); 
    
    if (telop_num_robots <= 0)
     {
      /* No unit specified, so broadcast */
      sprintf(valuestr, "%f", new_value);
      gt_update(BROADCAST_ROBOTID, slider->params[j].key, valuestr);
     }
    else
     {
      for (i=0; i<telop_num_robots; i++)
       {
	/* Send the messages to each robot in the unit */
	sprintf(valuestr, "%f", new_value);
	gt_update(telop_robot[i], slider->params[j].key, valuestr);
       }
     }
   }
 }

void  adjust_motivational_vector(Widget w, XtPointer client_data, XmScaleCallbackStruct *call_data)
{
  motivational_vector_type * slider = (motivational_vector_type *) client_data;
  int  value = (int) call_data->value;
  float  max_value, min_value, new_value;
  char  valuestr[100]; 
  int  i, j;
  int  number_parameters;

  /* printf(" adjust_motivational_vector() in adjust.c \n"); */

  number_parameters = slider->num_params;
  
  for (j=0; j<number_parameters; j++)
   {
    max_value = slider->params[j].base * 2;
    min_value = 0.0;
    if ( slider->params[j].inc )
      new_value = min_value + (max_value - min_value) * ( (float) value / 100);
    else
      new_value = min_value + (max_value - min_value) * 
		   ( (float) (100 - value) / 100 ); 
    
    if (telop_num_robots <= 0)
     {
      /* No unit specified, so broadcast */
      sprintf(valuestr, "%f", new_value);
      gt_update(BROADCAST_ROBOTID, slider->params[j].key, valuestr);
     }
    else
     {
      for (i=0; i<telop_num_robots; i++)
       {
	/* Send the messages to each robot in the unit */
	sprintf(valuestr, "%f", new_value);
	gt_update(telop_robot[i], slider->params[j].key, valuestr);
       }
     }
   }
 }
/**********************************************************************
 * $Log: adjust.c,v $
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
 * Revision 1.2  2006/07/07 00:06:28  endo
 * telop default mode changed.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:21  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.12  2003/04/02 21:33:57  zkira
 * Added telop_mode broadcast
 *
 * Revision 1.11  2002/07/18 17:02:58  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.10  2000/03/24 18:58:08  endo
 * Debug disabled.
 *
 * Revision 1.9  2000/02/29 21:09:29  saho
 * Added functionality for sound simulation and motivational vectors.
 *
 * Revision 1.8  1995/08/09 18:41:05  kali
 * personality sliders now adjust any number of parameters
 *
 * Revision 1.7  1995/06/23  14:16:41  kali
 * changed parameter sent to the callback adjust_personality.
 *
 * Revision 1.6  1995/06/20  18:37:40  kali
 * fixed call to gt_update
 *
 * Revision 1.5  1995/06/14  20:27:46  kali
 * Working on adjust_personality function.
 *
 * Revision 1.4  1995/06/05  19:30:45  kali
 * added a shell for the function adjust_personality
 *
 * Revision 1.3  1995/05/03  21:59:50  jmc
 * Improved test for num_robots in set_values.
 *
 * Revision 1.2  1995/05/03  19:14:07  jmc
 * Added code to set_values to send the telop messages to the robots
 * in the unit, if it exists.  If not, it reverts to broadcasting to
 * all robots, as it used to do.   Added RCS strings.  Included
 * telop.h for declaration of unit-related globals.
 **********************************************************************/
