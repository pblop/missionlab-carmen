#ifndef cmdl_support_H
#define cmdl_support_H
/**********************************************************************
 **                                                                  **
 **  cmdl_support.h                                                  **
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

/* $Id: cmdl_support.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

/**********************************************************************
* $Log: cmdl_support.h,v $
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
* Revision 1.5  2004/11/12 21:56:59  doug
* renamed loader so doesn't conflict with missionlab
*
* Revision 1.4  2004/05/11 19:34:49  doug
* massive changes to integrate with USC and GaTech
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

namespace sara 
{
/**********************************************************************/
// define some convience types.
typedef pair<string,string> PairOfStrings;

/**********************************************************************/
// Report errors
void sara_cmdl_error(const char *message);
void sara_cmdl_error(int lineno, const char *fmt, ...);

// Report syntax errors  
void sara_cmdl_syntax_error(int lineno, const char *fmt, ...);

/*********************************************************************/
}
#endif
