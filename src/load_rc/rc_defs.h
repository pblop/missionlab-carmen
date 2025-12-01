#ifndef RC_DEFS_H
#define RC_DEFS_H
/**********************************************************************
 **                                                                  **
 **                            rc_defs.h                             **
 **                                                                  **
 **  Definition include file for the RC parser.                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: rc_defs.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include "load_rc.h"

void rc_SyntaxError(const char *Message);

extern FILE *rc_in;
extern int rc_debug;
int rc_lex(void);
int rc_parse(void);

extern int rc_lineno;
extern char rc_linebuf[];
extern int rc_tokenpos;
extern int rc_tokenlen;
extern const char *_rc_filename;
extern bool rc_had_error;

/**********************************************************************
 * $Log: rc_defs.h,v $
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
 * Revision 1.5  1996/03/06  23:40:22  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/08/24  19:32:51  doug
 * *** empty log message ***
 *
 * Revision 1.3  1995/08/24  15:50:13  doug
 * *** empty log message ***
 *
 * Revision 1.2  1995/08/24  15:45:24  doug
 * *** empty log message ***
 *
 * Revision 1.1  1995/08/24  15:14:14  doug
 * Initial revision
 *
 **********************************************************************/
#endif
