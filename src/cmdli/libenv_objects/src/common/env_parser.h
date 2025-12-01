#ifndef ENV_PARSER_H
#define ENV_PARSER_H
/**********************************************************************
 **                                                                  **
 **  env_paser.h                                                     **
 **                                                                  **
 **  externs for the environment parser                              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: env_parser.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: env_parser.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:21:03  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.4  2004/05/11 19:34:39  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.3  2002/09/20 13:05:19  doug
// *** empty log message ***
//
// Revision 1.2  1999/03/10 00:22:28  doug
// moved extra to here from env_object
//
// Revision 1.1  1998/06/16 14:47:25  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "Vector.h"
#include "env_parser.tab.h"
#include "environment.h"

                                                                                                                                                                  
extern int env_debug;
extern bool env_had_error;
extern const std::string *_env_filename; 
extern bool env_verbose;

extern class sara::environment *theEnv;

// Main entry point
int env_parse(void);

// rc_SyntaxError prints an error message to stderr
void env_SyntaxError(const char *Message);

/*********************************************************************/
#endif
