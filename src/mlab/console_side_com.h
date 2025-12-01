/**********************************************************************
 **                                                                  **
 **                          console_side_com.h                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: console_side_com.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef CONSOLE_SIDE_COM_H
#define CONSOLE_SIDE_COM_H

void new_command( int robot_id, gt_Command* cmd );
void sim_put_state( int robot_id, char* key, char* value );
void link_robots( IPCommunicator* comm );
void gt_init_communication( char* ipt_home );
void gt_communication_loop();
void send_robots_pause_state( int val );
void send_robot_pause_state( int robot_id, int val );
void send_robots_debug_state( int val );
void send_robot_debug_state( int robot_id, int val );
void send_robots_report_state( int val );
void send_robot_report_state( int robot_id, int val );
void gt_close_communication();
void send_robots_suicide_msg();
void send_robot_suicide_msg( int robot_id );
void clear_communications();
void send_robot_step_one_cycle( int robot_id, int val );
void send_robots_step_one_cycle( int val );
void send_robots_proceed_mission(); 
void send_drawing_vector_field_msg( int iID );
void send_done_drawing_vector_field_msg( int iID );
void send_robot_report_sensor_readings( int iID );
void send_robots_feedback_state(int val);
void send_robot_feedback_state(int robot_id, int val);

#endif



/**********************************************************************
 * $Log: console_side_com.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.16  2002/07/02 20:39:27  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.15  2000/08/15 20:53:53  endo
 * send_robots_proceed_mission added.
 *
 * Revision 1.14  2000/06/13 16:52:08  endo
 * link_robots() added.
 *
 * Revision 1.13  1999/12/18 09:40:55  endo
 * send_robots_report_state, send_robot_report_state
 * added.
 *
 * Revision 1.12  1999/12/16 22:48:11  mjcramer
 * RH6 port
 *
 * Revision 1.9  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.8  1996/03/04  22:51:58  doug
 * *** empty log message ***
 *
 * Revision 1.7  1995/04/03  18:06:08  jmc
 * Added copyright notice.
 *
 * Revision 1.6  1994/11/03  19:34:13  doug
 * Added single step cycle command
 *
 * Revision 1.5  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.4  1994/10/11  20:13:34  doug
 * added tcxclose call
 *
 * Revision 1.3  1994/10/11  17:10:48  doug
 * added seq field to gt_command record
 *
 * Revision 1.2  1994/10/04  22:21:42  doug
 * Getting execute to work
 *
 * Revision 1.1  1994/10/03  21:14:10  doug
 * Initial revision
 *
 **********************************************************************/
