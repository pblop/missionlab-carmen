/**********************************************************************
 **                                                                  **
 **                               display.h                          **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: display.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/**********************************************************************
 **                                                                  **
 **                             display.h                            **
 **                                                                  **
 **********************************************************************/

#include "new_long_term_memory.h"

void set_position(float robot_x, float robot_y, float robot_theta,
	     float j1_theta, float j2_theta, float j3_theta,
	     float j4_theta, float j5_theta,
	     float finger_x, float finger_y, float finger_z);
				   
void overlay_copy(float robot_x, float robot_y, float robot_theta,
	     float j1_theta, float j2_theta, float j3_theta,
	     float j4_theta, float j5_theta);
				   
void            exit_display(void);
void            init_display(int for_overlay,int debug,int solid,int color);
void            set_goal(float x, float y, float z);
void            move_goal(float x, float y, float z);
int             set_obstacle(float x_scale,float y_scale,float z_scale,
	                  float x, float y, float z);
void move_obstacle(int num,float x_scale,float y_scale,float z_scale,
	                  float x, float y, float z);
void add_regions_to_display(void);
void add_via_points_to_display(int draw_path);
void user_display(void);




///////////////////////////////////////////////////////////////////////
// $Log: display.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:23  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.1  2000/03/22 04:39:51  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

