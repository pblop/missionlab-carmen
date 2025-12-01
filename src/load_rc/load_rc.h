#ifndef LOAD_RC_H
#define LOAD_RC_H

/**********************************************************************
 **                                                                  **
 **                            load_rc.h                             **
 **                                                                  **
 **  Prototype include file for rc parser                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: load_rc.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include "list.hpp"
#include "symbol_table.hpp"


typedef GTList<char *> rc_chain;
extern symbol_table<rc_chain> *_rc_table;


// Returns 0 if success
//         1 if parse error, but some of the table is loaded
//         2 if unable to open file
int
load_rc(const char *filename, symbol_table<rc_chain> *table,
        bool no_path_search, bool verbose, bool debug);



// Returns 0 if false
//         1 if true
//        -1 if not specified
//        -2 if value is not "true" or "false"
int
check_bool_rc(const symbol_table<rc_chain> &table, const char *name);


// Returns 1 if value is found
//         0 if value not found
//        -1 if keyword not specified
int
check_value_rc(const symbol_table<rc_chain> &table, 
                const char *key,
                const char *value);

// Returns 
//        false if not specified
//        true if value is loaded
bool
check_dbl_rc(const symbol_table<rc_chain> &table, const char *name, double *d);


/**********************************************************************
 * $Log: load_rc.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.10  2002/01/13 01:29:29  endo
 * list -> GTList
 *
 * Revision 1.9  1996/04/06  23:54:45  doug
 * *** empty log message ***
 *
 * Revision 1.8  1996/03/06  23:40:22  doug
 * *** empty log message ***
 *
 * Revision 1.7  1996/03/04  22:52:36  doug
 * *** empty log message ***
 *
 * Revision 1.6  1995/11/19  21:49:15  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/11/19  21:38:02  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/11/19  21:13:30  doug
 * added function to read booleans from table
 *
 * Revision 1.3  1995/08/24  19:32:51  doug
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/24  16:02:44  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/08/24  15:45:24  doug
 * Initial revision
 *
 * Revision 1.1  1995/08/24  15:45:24  doug
 * Initial revision
 *
 * Revision 1.6  1995/06/29  14:38:04  jmc
 * Added RCS log string.
 **********************************************************************/

#endif
