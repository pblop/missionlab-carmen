#ifndef RC_PARSER_H
#define RC_PARSER_H
/**********************************************************************
 **                                                                  **
 **  rc_paser.h                                                      **
 **                                                                  **
 **  externs for the resource parser                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rc_parser.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rc_parser.h,v $
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
// Revision 1.3  2004/11/12 21:59:51  doug
// renamed yacc/bision stuff so doesn't conflict with missionlab
//
// Revision 1.2  2004/05/11 19:34:31  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  1997/12/01 15:38:07  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "resources.h"

extern int sara_rc_debug;
extern bool sara_rc_had_error;
extern const string *sara_rc_filename; 
extern sara::res_T *sara_rc_table;

// Main entry point
int sara_rc_parse(void);

// rc_SyntaxError prints an error message to stderr
void sara_rc_SyntaxError(const char *Message);

/*********************************************************************/
#endif
