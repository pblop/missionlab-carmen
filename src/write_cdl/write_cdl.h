/**********************************************************************
 **                                                                  **
 **                             write_cdl.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: write_cdl.h,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#ifndef CDL_CODEGEN_H
#define CDL_CODEGEN_H

#include <string>

#include "symbol.hpp"

bool 
save_workspace(Symbol * agent, const char *filename, bool debug, bool backup_files);

bool 
write_library(SymbolList * agent, const char *filename, bool debug, 
	bool backup_files, const char *source_file);

bool     
prune_list(SymbolList *symlist, Symbol * agent, RECORD_CLASS desired_class, 
	const bool is_root);

bool save_abstracted_workspace(Symbol * agent, const char *filename);

typedef enum acdl2cdl_status_t {
    ACDL2CDL_STATUS_FAILURE,
    ACDL2CDL_STATUS_SUCCESS,
    ACDL2CDL_STATUS_EMPTYMISSION
};

char *acdl2cdl(
    const char *acdlCode,
    const string defaultRobotName,
    int *status);

#endif


/**********************************************************************
 * $Log: write_cdl.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2006/08/02 21:39:24  endo
 * acdlp2cdl() implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.11  2003/04/06 09:10:03  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.10  1999/12/16 22:53:02  endo
 * *** empty log message ***
 *
 * Revision 1.1  1996/09/25 19:12:07  doug
 * Initial revision
 *
 * Revision 1.11  1995/11/21  23:10:08  doug
 * *** empty log message ***
 *
 * Revision 1.10  1995/06/29  17:28:03  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
