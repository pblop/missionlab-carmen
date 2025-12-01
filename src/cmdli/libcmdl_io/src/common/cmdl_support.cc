/**********************************************************************
 **                                                                  **
 **  cmdl_support.cc                                                 **
 **                                                                  **
 **  cmdl_support functions for the parser                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdl_support.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_support.cc,v $
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
* Revision 1.8  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.7  2004/10/25 22:58:16  doug
* working on mars2020 integrated demo
*
* Revision 1.6  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.5  2004/03/08 14:52:34  doug
* cross compiles on visual C++
*
* Revision 1.4  2004/01/09 18:46:03  doug
* back from Florida
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

#include "mic.h"
#include "cmdl_support.h"
#include "cmdl_loader.h"
#include "cmdl_parser.h"
#include <stdarg.h>

namespace sara 
{
/**********************************************************************/
void 
sara_cmdl_error(const char *message)
{
   PRINT(message);
   PRINT("\n");
   the_cmdl_loader->set_error();
}

/**********************************************************************/
// Report errors
void
sara_cmdl_error(int lineno, const char *fmt, ...)
{
   char buf[4096]; 

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"Error Near Line %d: ", lineno);
   vsprintf(&buf[strlen(buf)],fmt, args);
   va_end(args);
   
   sara_cmdl_error(buf);
}

/*********************************************************************/
// Report syntax errors
void
sara_cmdl_syntax_error(int lineno, const char *fmt, ...)
{
   char buf[4096]; 

   va_list args;
   va_start(args, fmt);
   sprintf(buf,"Syntax Error Near Line %d: ", lineno);
   vsprintf(&buf[strlen(buf)],fmt, args);
   va_end(args);
   
   sara_cmdl_error(buf);
}

/*********************************************************************/
}
