#ifndef cmdl_PARSER_H
#define cmdl_PARSER_H
/**********************************************************************
 **                                                                  **
 **  cmdl_parser.h                                                   **
 **                                                                  **
 **  prototype the parser functions                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_parser.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_parser.h,v $
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
* Revision 1.9  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.8  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.7  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.6  2004/02/20 14:39:35  doug
* cmdl interpreter runs a bit
*
* Revision 1.5  2003/11/07 20:29:27  doug
* added cmdli interpreter library
*
* Revision 1.4  2003/11/03 16:43:31  doug
* snapshot
*
* Revision 1.3  2003/10/24 21:03:39  doug
* finally compiles
*
* Revision 1.2  2003/10/23 19:14:44  doug
* closer to compiling a minimal set
*
* Revision 1.1  2003/10/22 22:14:37  doug
* starting to build the parser
*
**********************************************************************/

#include "cmdl_support.h"
//#include "cmdl_command.h"
#include "RobotType_ParmList.h"
#include "Block.h"
#include "Unit.h"
#include "Command.h"
#include "cmdl_parser.tab.h"

// The parser 
int sara_cmdl_parse();

/// Function to parse a single procedure definition
/// Returns true on success, false otherwise.
bool parseProcedureDef(const string &buffer);

// Set to enable user-level debug messages.
extern bool cmdl_user_debug;

// Set to enable YACC-level debug messages.
extern int sara_cmdl_debug;

// For the line number info
extern YYLTYPE yylloc;

namespace sara 
{
   // The current mission root record
   extern class cmdl_Root *cmdl_root;

   // the current loader object using the parser
   extern class cmdl_loader *the_cmdl_loader;
}

/*********************************************************************/
#endif
