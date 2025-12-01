/**********************************************************************
 **                                                                  **
 **                            cdl_defs.h                            **
 **                                                                  **
 **  Definition include file for CDL.                                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cdl_defs.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern int lineno;
extern int tokenpos;
extern int tokenlen;

extern const char *filename;
extern int had_error;
extern char linebuf[];
extern bool is_lhs;

int yylex(void);
int yyparse(void);



/**********************************************************************
 * $Log: cdl_defs.h,v $
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
 * Revision 1.8  1995/06/29  14:20:42  jmc
 * Added RCS log string.
 *
 * Revision 1.8  1995/06/29  14:20:42  jmc
 * Added RCS log string.
 **********************************************************************/
