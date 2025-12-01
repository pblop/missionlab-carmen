/**********************************************************************
 **                                                                  **
 **                            console.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Communication interface to robot-executables                    **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: console.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef CONSOLE_H
#define CONSOLE_H

extern bool consoleConnected,useConsole;
extern int consoleTextLine;
extern int console_fd;
extern char consoleStatus[100];
extern int console_total_recieve;
extern bool gDisableConsoleRobotControl;

extern int console_report_level;

extern bool ipt_server_found;
extern char *ipt_server_name;

extern bool hclientConnected;

void consoleDisconnect();
void consoleConnect();

#endif

/**********************************************************************
# $Log: console.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:25  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.3  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
