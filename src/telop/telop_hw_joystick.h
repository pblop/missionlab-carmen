/**********************************************************************
 **                                                                  **
 **                          telop_hw_joystick.h                     **
 **                                                                  **
 **  Written by: Michael Langford                                    **                                                    
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: telop_hw_joystick.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */


#ifndef TELOP_HW_JOYSTICK_H
#define TELOP_HW_JOYSTICK_H

void *jq_get_axis_callback(void);
int jq_register_axis_callback(void (*cb)(int axis[6]));
long jq_get_xmax(void);
long jq_get_xmin(void);
long jq_get_ymax(void);
long jq_get_ymin(void);
int jq_kill_js_driver(void);

#endif

/**********************************************************************
 * $Log: telop_hw_joystick.h,v $
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
 * Revision 1.1  2003/04/02 21:25:47  zkira
 * Initial revision
 *
 **********************************************************************/
