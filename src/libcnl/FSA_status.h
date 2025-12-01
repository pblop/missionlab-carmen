#ifndef FSA_STATUS_H
#define FSA_STATUS_H
/****************************************************************************
*                                                                           *
*                            FSA_status.h                                   *
*                                                                           *
*   Written by: Yoichiro Endo                                               *
*                                                                           *
*   Defines the global variables that take care of the FSA status; such as  *
*   what number of FSA (wheel or camera?) currently the robot is in, or     *
*   what number of state currently or previously it is in, and so on.       *
*                                                                           *
*   Copyright 1999, Georgia Tech Research Corporation                       *
*   Atlanta, Georgia  30332-0415                                            *
*   ALL RIGHTS RESERVED, See file COPYRIGHT for details.                    *
*                                                                           *
****************************************************************************/

/* $Id: FSA_status.h,v 1.1.1.1 2006/07/12 13:37:58 endo Exp $ */

#define FSA_MAX 100                 // How many FSAs (from motor, camera, to anything else)
                                    // the robot can have. 
extern int current_state[FSA_MAX];  // Records current state w.r.t. the FSA number.
extern int previous_state[FSA_MAX]; // Records revious state w.r.t. the FSA number.

// This variable contains infomation on whether the robot is in a new state
// or still in the previous state.
// For each FSA, if the robot is still in the same state as it was in
// the previous time cycle, it makes "in_new_state" to be TRUE.
// However, technically, the first same state is the true new state
// because it loads the new state during the transition from the old state.
extern bool in_new_state[FSA_MAX]; 

#endif
/**********************************************************************
 * $Log: FSA_status.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:58  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:02  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.12  2000/04/16 15:51:31  endo
 * alerted_once deleted.
 *
 * Revision 1.11  2000/02/18 02:37:38  endo
 * in_new_state and alerted_once added.
 *
 * Revision 1.10  1999/12/18 09:50:15  endo
 * This file defines the global variables that take care of
 * the FSA status; such as what number of FSA (wheel or camera?)
 * currently the robot is in, or what number of state currently
 * or previously it is in, and so on.
 *
 *
 **********************************************************************/
