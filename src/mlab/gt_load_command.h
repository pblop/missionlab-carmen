/**********************************************************************
 **                                                                  **
 **                        gt_load_command.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_load_command.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include "gt_command.h"

extern char *command_filename_directory;

int gt_load_commands(char *filename);  /* returns 0 for success, nonzero otherwise */



/**********************************************************************
 * $Log: gt_load_command.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.5  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.4  1995/04/12  21:13:55  jmc
 * Added a declaration for command_filename_directory.
 *
 * Revision 1.3  1995/04/03  20:21:41  jmc
 * Added copyright notice.
 *
 * Revision 1.2  1994/07/12  19:25:21  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
