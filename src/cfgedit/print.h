/**********************************************************************
 **                                                                  **
 **                             print.h                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: print.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef PRINT_H
#define PRINT_H

#define WINDOW_ONLY 1
#define FULL_DESIGN 2

int print(Display *display, Window target_window, int type, char *filename);

#endif



/**********************************************************************
 * $Log: print.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:33  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.2  1995/06/29  18:11:57  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
