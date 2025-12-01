#ifndef RC_SCANNER_H
#define RC_SCANNER_H
/**********************************************************************
 **                                                                  **
 **  rc_scanner.h                                                    **
 **                                                                  **
 **  define externs from rc_scanner.l                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rc_scanner.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rc_scanner.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:34:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/11/12 21:59:51  doug
// renamed yacc/bision stuff so doesn't conflict with missionlab
//
// Revision 1.1  1997/12/01 15:38:07  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>

int sara_rc_lex(void);
extern int sara_rc_lineno;
extern FILE *sara_rc_in;

/**********************************************************************/
#endif
