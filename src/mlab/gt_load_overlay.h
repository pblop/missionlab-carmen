/**********************************************************************
 **                                                                  **
 **                         gt_load_overlay.h                        **
 **                                                                  **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron          (c) Copyright, 1994   **
 **********************************************************************/

/* $Id: gt_load_overlay.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

void interrupt_load_overlay(char *filename);
int gt_load_overlay(char *filename);  /* returns 0 for success, nonzero otherwise */

extern FILE *odl_in;
extern int odl_parse();
extern void odl_file_reset();
extern char odl_filename[];


/**********************************************************************
 * $Log: gt_load_overlay.h,v $
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
 * Revision 1.6  2002/10/31 21:36:37  ebeowulf
 * Added interrupt_load_overlay, to fix Start Place problem.
 *
 * Revision 1.5  2000/07/07 18:23:37  endo
 * variable declearation moved from gt_load_overlay.c .
 *
 * Revision 1.4  1997/02/12 05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.3  1995/04/03  20:24:13  jmc
 * Added copyright notice.
 *
 * Revision 1.2  1994/07/12  19:25:31  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
