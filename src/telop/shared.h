/**********************************************************************
 **                                                                  **
 **                         shared.h                                 **
 **                                                                  **
 **  Written by:  Khaled S.  Ali                                     **
 **                                                                  ** 
 **  Some shared types and variables.                                ** 
 **                                                                  ** 
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: shared.h,v 1.2 2008/10/27 21:47:26 zkira Exp $ */



#ifndef SHARED_H
#define SHARED_H

typedef struct
 {
   double  x, y, z;
 }  point;

struct GTVector
 {
  point  direction;
  double  magnitude;
 };

struct values_type
{
    /* struct vector  teleaut_vector; */
    struct GTVector  teleaut_vector;
};


struct sound_values_type
 {
     /* struct vector  sound_vector; */
     struct GTVector  sound_vector;
 };


/* These are used by the telop joystick */
extern struct values_type  original;
extern struct values_type  normal;
extern struct values_type  newest;

/* These are used by the sound simulation joystick */
extern struct sound_values_type  sound_original;
extern struct sound_values_type  sound_normal;
extern struct sound_values_type  sound_newest;



struct param_type
 {
  char  key[100]; 
  double  base; 
  int  inc; 
 };

struct personality
 {
  char  title[30];
  int  num_params;
  struct param_type  params[10];
 };

typedef struct personality  personality_type;

struct motivational_vector
 {
  char  title[30];
  int  num_params;
  struct param_type  params[10];
 };


typedef struct motivational_vector  motivational_vector_type;  

#endif


/**********************************************************************
 * $Log: shared.h,v $
 * Revision 1.2  2008/10/27 21:47:26  zkira
 * Added z component for 3D telop
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
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.7  2002/01/13 01:48:10  endo
 * vector -> GTVector
 *
 * Revision 1.6  2000/02/29 22:17:34  saho
 * Added struct sound_values_type .
 *
 * Revision 1.5  1995/08/09 19:13:03  kali
 * personality sliders now adjust any number of parameters
 *
 * Revision 1.4  1995/06/28  19:27:30  jmc
 * Added RCS id and log strings.
 **********************************************************************/
