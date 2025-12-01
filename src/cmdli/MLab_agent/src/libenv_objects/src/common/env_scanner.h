#ifndef ENV_SCANNER_H
#define ENV_SCANNER_H
/**********************************************************************
 **                                                                  **
 **  env_scanner.h                                                   **
 **                                                                  **
 **  define externs from env_scanner.l                               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: env_scanner.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: env_scanner.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.2  2002/09/20 13:05:19  doug
* *** empty log message ***
*
* Revision 1.1  1998/06/16 18:10:06  doug
* Initial revision
*
**********************************************************************/

#include <stdio.h>

int env_lex(void);
extern int env_lineno;
extern const char *env_filename;
extern FILE *env_in;

typedef enum OBJ_STYLES {STYLE_FIXED, STYLE_MOVABLE, STYLE_CONTAINER};

/**********************************************************************/
#endif
