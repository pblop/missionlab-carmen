/**********************************************************************
 **                                                                  **
 **                            dock.c                                **
 **                                                                  **
 **  Written by: Ronald C. Arkin and RJonathan Diaz                  **
 **                                                                  **
 **  Copyright 2000 - 2003, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: dock.c,v 1.1.1.1 2006/07/12 13:37:55 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "gt_std.h"

#define EPSILON 0.00001
#define TANGENTIAL_MAGNITUDE 1.0
#define TEST_LENGTH 32

struct d_vector { 	
  double magnitude;
  double direction_xy;	/* 0 is North, 90 is East (stored in RADIANS)*/
};

double d_find_length_2d(double x1, double y1, double x2, double y2)
{
  double dx,dy;
  
  dx = fabs(x1 - x2);
  dy = fabs(y1 - y2);
  return(sqrt((double) ((dx*dx)+(dy*dy))));
}
double d_compute_angle_2d(double x1, double y1, double x2, 
			  double y2, double x3, double y3)
{
  double angle,a, b, c;
  
  /*  computes angle from 2d points in plane of triangle using
      law of cosines, x2 is fulcrum */
  
  c = d_find_length_2d(x1,y1,x2,y2);
  b = d_find_length_2d(x2,y2,x3,y3);
  a = d_find_length_2d(x1,y1,x3,y3);
  angle = acos(((b*b)+(c*c)-(a*a))/(2*b*c));

  /*
  if (angle == 0) {
    if (d_find_length_2d(x1,y1,x2,y2) < d_find_length_2d(x1,y1,x3,y3))
	angle = M_PI;
  }
  */

  return(angle);
}

/* add v2 to v1 and store in v1 */
void add_vector2(struct d_vector *v1, struct d_vector *v2) 
{
  double x1,y1,x2,y2;
  double d1,d2;
  double r1,r2,th1,th2;
  
  r1 = v1->magnitude;
  r2 = v2->magnitude;
  th1 = v1->direction_xy;
  th2 = v2->direction_xy;

  if (r1 == 0) {
    v1->direction_xy = v2->direction_xy;
    v1->magnitude = v2->magnitude;
    return;
  }
  if (r2 == 0) return;

  if (th1 >= M_PI*2) {
    th1 -= M_PI*2;
  }
  if (th2 >= M_PI*2) {
    th2 -= M_PI*2;	
  }
  if (th1 < 0.0) {
    th1 += M_PI*2;
  }
  if (th2 < 0.0) {
    th2 += M_PI*2;
  }

  x1 = r1*cos(th1);
  y1 = r1*sin(th1);
  x2 = r2*cos(th2);
  y2 = r2*sin(th2);

  /* compute result and store in v1 */
  d1 = x1 + x2;
  d2 = y1 + y2;
  
  v1->magnitude = sqrt((d1*d1)+(d2*d2));
  if (v1->magnitude != 0) {
    if ((fabs(d1) < EPSILON) && (fabs(d2) < EPSILON))
      v1->direction_xy = 0;   
    else
      v1->direction_xy = atan2(d2,d1);   
  }
  return;
}

/*-----------------------------------------------------------------------*/

void move_goal_vector(double i, double j, double x, 
		      double y, double *vxy)
{
  /* i,j is robot position */
  double angle;

  if (j == y && i == x) angle = 0.0;
  else if (i == x) {
    if (j<y)
      angle = M_PI/2.0;
    else	
      angle = 3.0*M_PI/2.0;
  }
  else if (j == y) {
    if (i<x)
	angle = 0.0;
      else	
	angle = M_PI;
  }
  else {
    angle = d_compute_angle_2d(x+TEST_LENGTH+1,y,x,y,i,j);
    if ((i<x) &&(j<y))
      angle = M_PI - angle;
    if ((i<x) && (j >y))
      angle = M_PI + angle;
    if ((i>x) && (j<y))
      angle = M_PI - angle;
    if ((i>x) && (j >y))
      angle = M_PI + angle;
  }
  *vxy = angle;
    
  return;
}

/*-------------------------------------------------------------*/

Vector 
compute_dock_force_on_robot(Vector robotpos, Vector dockpos,
			    double dockxy, double controlled_radius,
			    double cone_angle, double ballistic_gain, double controlled_gain)
{
  Vector retval;
  struct d_vector total_vec,vec;
  double tan_vector_xy;
  double rx,ry;
  double len_r_dock,ang_rob_dock_test,nx,ny,ndx,ndy,dist_up_test;
  double test_dx,test_dy;
  double goal_xy;
  /* Is point influenced */

  retval.x = 0.0;
  retval.y = 0.0;
  retval.z = 0.0;
  
  rx = robotpos.x;
  ry = robotpos.y;
  
  /*****/
  len_r_dock = d_find_length_2d(rx,ry,dockpos.x,dockpos.y);
  if (len_r_dock > 0.05) { /* point not at dock */
    move_goal_vector(rx,ry,dockpos.x,dockpos.y,&goal_xy);
       
    if (len_r_dock > controlled_radius) {
      /* In  BALLISTIC ZONE */
      total_vec.magnitude = ballistic_gain;
      total_vec.direction_xy = goal_xy;
      
    }
    else {   /* IN CONTROLLED ZONE */
      test_dx = TEST_LENGTH * cos(dockxy);
      test_dy = TEST_LENGTH * sin(dockxy);
      ang_rob_dock_test = d_compute_angle_2d(rx,ry,dockpos.x,dockpos.y,
					     dockpos.x+test_dx,dockpos.y+test_dy);
            
      if (fabs (ang_rob_dock_test - M_PI/2.0)< EPSILON)
	ang_rob_dock_test = M_PI/2.0;
      
      if (cos(ang_rob_dock_test) != 0) {
	dist_up_test = len_r_dock/cos(ang_rob_dock_test);
      }
      else {
	dist_up_test = 0.0; /* I suppose this is not supposed to happen ??-jdiaz*/
      }
      nx = dockpos.x + (dist_up_test * cos(dockxy));
      ny = dockpos.y + (dist_up_test * sin(dockxy));
      
      if (ang_rob_dock_test < (M_PI/2.0)) {
	ndx = nx - rx;
	ndy = ny - ry;
      }
      else {
	ndx = rx - nx;
	ndy = ry - ny;
      }
      /* resultant vector is from dx,dy,dz tp nx,ny,nz */
      /* now convert to spherical coordinates */
      /* CRC tables - pg. 369 */
      
      if (!((fabs(ndx) <EPSILON) && (fabs(ndy) <EPSILON))) {
	tan_vector_xy = atan2(ndy,ndx);
      }
      else {
	tan_vector_xy = dockxy;
      }
      
      if (ang_rob_dock_test < cone_angle) {  /* ATTRACTIVE CONE AREA */
	/* TANGENT COMPONENT */
	total_vec.magnitude = 
	 (ang_rob_dock_test/cone_angle)*TANGENTIAL_MAGNITUDE * controlled_gain; 
	total_vec.direction_xy = tan_vector_xy ;  
	/* MOVE TO GOAL COMPONENT */
	vec.magnitude =  controlled_gain;  
	vec.direction_xy = goal_xy ;  
	/* ADD TWO COMPONENTS */
	add_vector2(&total_vec,&vec);
	
      } /* end attractive zone */
      else {   /* COERCIVE ZONE */
	
	/* TANGENT COMPONENT */
	total_vec.magnitude = TANGENTIAL_MAGNITUDE * controlled_gain;  
	total_vec.direction_xy = tan_vector_xy ;  
	/* MOVE TO GOAL COMPONENT */
	vec.magnitude = 
		(1.0 -((controlled_radius - len_r_dock)/controlled_radius)) * controlled_gain;  
	vec.direction_xy = goal_xy ;  
	/* ADD TWO COMPONENTS */
	add_vector2(&total_vec,&vec);
      }  /* end coercive zone */
      
    }
  }
  else {
    /* point at dock */
    total_vec.magnitude = 0.0;
  }
  /******/
  if (total_vec.magnitude == 0.0) {
    retval.x = 0.0;
    retval.y = 0.0;
  }
  else {
    retval.x = cos(total_vec.direction_xy) * total_vec.magnitude;
    retval.y = sin(total_vec.direction_xy) * total_vec.magnitude;
  }

  return retval;
}

/**********************************************************************
 * $Log: dock.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 13:20:20  endo
 * gcc 3.1.1
 *
 **********************************************************************/
