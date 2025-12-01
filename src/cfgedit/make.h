/**********************************************************************
 **                                                                  **
 **                             make.h                               **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: make.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#ifndef MAKE_H
#define MAKE_H

typedef enum {
    SIMPLE_MAKE_WINDOW,
    DETAILED_MAKE_WINDOW,
    NUM_MAKE_WINDOWS
} MakeWindowType_t;

bool make(void);

#endif

/**********************************************************************
 * $Log: make.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
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
 * Revision 1.4  2002/01/12 23:23:16  endo
 * Mission Expert functionality added.
 *
 * Revision 1.3  1997/02/14 16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/18  23:03:26  doug
 * *** empty log message ***
 *
 **********************************************************************/
