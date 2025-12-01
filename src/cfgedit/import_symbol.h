/**********************************************************************
 **                                                                  **
 **                     import_symbol.h                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 2000 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: import_symbol.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef IMPORT_SYMBOL_H
#define IMPORT_SYMBOL_H

// called to create a duplicate of a read-only symbol in user space.
// Returns the imported symbol on success, NULL on error.
void import_symbol(screen_object *s);

#endif


/**********************************************************************
 * $Log: import_symbol.h,v $
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
 * Revision 1.1  2000/04/13 21:45:37  endo
 * Initial revision
 *
 **********************************************************************/
