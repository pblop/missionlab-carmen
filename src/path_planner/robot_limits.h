/**********************************************************************
 **                                                                  **
 **                               robot_limits.h                          **
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

/* $Id: robot_limits.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef ROBOT_LIMITS_H
#define ROBOT_LIMITS_H
/**********************************************************************
 **                                                                  **
 **                            robot_limits.h                        **
 **                                                                  **
 **********************************************************************/

#ifndef STD_H
#include "std.h"
#endif

#define NUM_LINKS 6
#define NUM_JOINTS 7

/*
 * link0 is the robot, verticle from the floor to the arm mount point.
 * link1 is the base, verticle from the mount point to the shoulder rotation pin
 * link2 is the upper arm, it is from the shoulder to the elbow
 * link3 is next, from the elbow to the wrist 
 * link4 is the length of the hand
 * link5 is the length of the fingers
 */

/* all values are in feet */
#define BASE_CG_HEIGHT          FEET_FROM_INCHES(15.0)

#define LINK0_LENGTH            FEET_FROM_INCHES(24.0)
#define LINK0_RADIUS            FEET_FROM_INCHES(12.0)

#define LINK1_LENGTH            FEET_FROM_INCHES(10.0)
#define LINK1_RADIUS            FEET_FROM_INCHES( 4.0)

#define LINK2_LENGTH            FEET_FROM_INCHES(10.0)
#define LINK2_RADIUS            FEET_FROM_INCHES( 3.0)

#define LINK3_LENGTH            FEET_FROM_INCHES(10.0)
#define LINK3_RADIUS            FEET_FROM_INCHES( 2.0)

#define LINK4_LENGTH            FEET_FROM_INCHES( 2.0)
#define LINK4_RADIUS            FEET_FROM_INCHES( 2.0)

/* without straw */
/* #define LINK5_LENGTH            FEET_FROM_INCHES( 4.0) */
/* with straw */
#define LINK5_LENGTH            FEET_FROM_INCHES( 14.75 )
#define LINK5_RADIUS            FEET_FROM_INCHES( 0.5)

/* all angle values are in degrees */
/* speeds are in degres per second, or feet per second */
/* joint 0 is movement of wheels, at the begining of link 0 */
/* speed is in feet per second */
#define JOINT0_MIN_ANGLE        -PATH_PLAN_INFINITY   
#define JOINT0_MAX_ANGLE         PATH_PLAN_INFINITY
#define JOINT0_MAX_SPEED         4.0

/* joint 1 is steer angle of wheels, at the begining of link 0 */
/* angle is positive clockwise looking down from above */
#define JOINT1_MIN_ANGLE        -PATH_PLAN_INFINITY   
#define JOINT1_MAX_ANGLE         PATH_PLAN_INFINITY
#define JOINT1_MAX_SPEED         75.0

/* joint 2 is waist, at the begining of link 1 (horizontal rotation) */
/* angle is positive counter clockwise looking down from above */
/* 0 is towards home position on the robot */
#define JOINT2_MIN_ANGLE        -174.0	/* -175 is hard limit */
#define JOINT2_MAX_ANGLE         174.0	/* +175 is hard limit */
#define JOINT2_MAX_SPEED         100.0

/* joint 3 is shoulder, at the begining of link 2 (verticle rotation) */
/* angle is positive clockwise looking from robot 90 degree position with
   arm homed.  0 is with arm verticle */
#define JOINT3_MIN_ANGLE        -19.0	/* -20 is hard limit */
#define JOINT3_MAX_ANGLE         89.0	/*  90 is hard limit */
#define JOINT3_MAX_SPEED         62.0

/* joint 4 is elbow, at the begining of link 3 (verticle rotation) */
/* angle is positive clockwise looking from robot 90 degree position with
   arm homed.  0 is with arm verticle */
#define JOINT4_MIN_ANGLE           0.0
#define JOINT4_MAX_ANGLE         125.0
#define JOINT4_MAX_SPEED         100.0

/* joint 5 is wrist_pitch, at the begining of link 4 (verticle rotation) */
/* angle is positive clockwise looking from robot 90 degree position with
   arm homed.  0 is with arm verticle */
#define JOINT5_MIN_ANGLE        -110.0
#define JOINT5_MAX_ANGLE         110.0
#define JOINT5_MAX_SPEED         180.0

/* joint 6 is wrist_roll, at the begining of link 5 (rotates about the arm) */
/* angle is positive clockwise looking down from above with arm verticle. */
/* 0 is homed position */
#define JOINT6_MIN_ANGLE        -180.0
#define JOINT6_MAX_ANGLE         180.0
#define JOINT6_MAX_SPEED         360.0

/***************************************************************************/
#endif



///////////////////////////////////////////////////////////////////////
// $Log: robot_limits.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:24  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:51  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:00  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:24  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.2  2003/04/06 10:16:11  endo
// Checked in for Robert R. Burridge. Various bugs fixed.
//
// Revision 1.1  2000/03/22 04:40:15  saho
// Initial revision
//
///////////////////////////////////////////////////////////////////////

