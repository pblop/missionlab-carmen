/**********************************************************************
 **                                                                  **
 **                             ConstructPrintName.h                 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: ConstructPrintName.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef CONSTRUCTPRINTNAME_H
#define CONSTRUCTPRINTNAME_H

bool ParmExtremes(Symbol *rec, Symbol *parm, char **left, char **right);
char *ObjectName(Symbol *rec);
char *ConstructPrintName(Symbol *p);

#endif


/**********************************************************************
 * $Log: ConstructPrintName.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:31  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.2  1996/02/29  01:48:49  doug
 * *** empty log message ***
 *
 * Revision 1.1  1996/02/25  01:14:20  doug
 * Initial revision
 *
 **********************************************************************/
