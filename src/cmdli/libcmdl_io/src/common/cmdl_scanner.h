#ifndef cmdl_scanner_H
#define cmdl_scanner_H
/**********************************************************************
 **                                                                  **
 **  cmdl_scanner.h                                                  **
 **                                                                  **
 **  prototypes and externs from the scanner                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_scanner.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_scanner.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:15  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:19:15  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.2  2003/11/07 20:29:27  doug
* added cmdli interpreter library
*
* Revision 1.1.1.1  2003/10/22 22:08:18  doug
* initial import
*
**********************************************************************/

/* The lexxer function */
int sara_cmdl_lex();

/* The current line number */
extern int sara_cmdl_lineno;

/* Debug flag */
extern int sara_cmdl__flex_debug;

/* buffer management */
typedef struct yy_buffer_state *YY_BUFFER_STATE;
YY_BUFFER_STATE cmdl__scan_buffer(char *base, unsigned int size);
void cmdl__delete_buffer(YY_BUFFER_STATE buffer);

// cmdl_init will get ready to scan a buffer from memory
void sara_cmdl_init(const char *buffer, const int length);

// cmdl_exit will free the allocated internal buffer
void sara_cmdl_exit();

/*********************************************************************/
#endif
