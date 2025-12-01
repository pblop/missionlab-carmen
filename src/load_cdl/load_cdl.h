#ifndef LOAD_CDL_H
#define LOAD_CDL_H

/**********************************************************************
 **                                                                  **
 **                            load_cdl.h                            **
 **                                                                  **
 **  Prototype include file for CDL parser                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: load_cdl.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include "symtab.h"
#include "prototypes.h"
#include "cdl_gram.tab.h"
#include "apply.h"
#include "ExpandSubtrees.h"

bool init_cdl(bool debug=false);
Symbol *load_cdl(const char **names, bool &errors, bool is_library=false, bool debug=false);

void set_user_class();



/**********************************************************************
 * $Log: load_cdl.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:49  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:07  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.9  1996/02/26  05:02:34  doug
 * *** empty log message ***
 *
 * Revision 1.9  1996/02/26  05:02:34  doug
 * *** empty log message ***
 *
 * Revision 1.8  1996/01/28  23:13:00  doug
 * *** empty log message ***
 *
 * Revision 1.7  1995/10/09  20:06:46  doug
 * added an errors parameter to the load_cdl call so can load files
 * with partial errors
 *
 * Revision 1.6  1995/06/29  14:38:04  jmc
 * Added RCS log string.
 **********************************************************************/

#endif
