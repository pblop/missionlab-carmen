/**********************************************************************
 **                                                                  **
 **                           apply.h                                **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: apply.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef APPLY_H
#define APPLY_H

// Returns false if it wants to quit.
//  Then: leave restart false to error out.
//        set restart true to restart the application at the root.   
typedef bool apply_fnc(Symbol *p, Symbol *page_def, Symbol *robot,   
         SymbolList * page_stack, char **message, bool *restart);    
	  
bool apply(Symbol *start, apply_fnc *fnc, 
	Symbol **location, char **msg, SymbolList **page_stack);

#endif


/**********************************************************************
 * $Log: apply.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:49  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:06  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  1996/10/04  21:10:30  doug
 * Initial revision
 *
 * Revision 1.1  1996/10/04  21:10:30  doug
 * Initial revision
 *
 * Revision 1.3  1996/02/18  23:31:19  doug
 * binding is working!!!!
 *
 * Revision 1.2  1996/02/18  00:03:13  doug
 * binding is working better
 *
 * Revision 1.1  1996/02/06  18:17:26  doug
 * Initial revision
 *
 *
 **********************************************************************/
