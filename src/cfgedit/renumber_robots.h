/**********************************************************************
 **                                                                  **
 **                     renumber_robots.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: renumber_robots.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef RENUMBER_ROBOTS_H
#define RENUMBER_ROBOTS_H


extern int num_robots;	// Robots are named "cfgRobotxxx" where 
                        // xxx is a 1 based integer.
			// and cfg is the name of the configuration

void renumber_robots(void);
char *RobotName(const int n);

#endif


/**********************************************************************
 * $Log: renumber_robots.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:34  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.2  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.1  1996/02/28  03:57:00  doug
 * Initial revision
 *
 **********************************************************************/
