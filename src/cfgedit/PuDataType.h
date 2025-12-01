/**********************************************************************
 **                                                                  **
 **                             PuDataType.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: PuDataType.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef PU_DATATYPE_H
#define PU_DATATYPE_H

Symbol *PuDataType(Symbol *rec, char *name);
Symbol *PuDataType(
    Symbol *rec,
    char *parm_name,
    bool *is_advanced_parm,
    bool *is_pair,
    Symbol **pair);
bool PuIsAdvancedParmValue(Symbol *rec, char *parm_name);

#endif


/**********************************************************************
 * $Log: PuDataType.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.3  2006/12/05 01:42:43  endo
 * cdl parser now accepts {& a b}.
 *
 * Revision 1.2  2006/08/29 15:12:31  endo
 * Advanced parameter flag added.
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
 * Revision 1.2  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.1  1996/02/24  17:07:51  doug
 * Initial revision
 *
 **********************************************************************/
