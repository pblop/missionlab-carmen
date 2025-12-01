/**********************************************************************
 **                                                                  **
 **  cmdli.cc                                                        **
 **                                                                  **
 **  The cmdl interpreter                                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: cmdli.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: cmdli.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:45  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:14:58  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.30  2004/05/11 19:34:50  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.29  2004/04/30 17:36:01  doug
* works for UPenn
*
* Revision 1.28  2004/04/27 18:36:07  doug
* fixed build problems with win32
*
* Revision 1.27  2004/04/27 16:19:34  doug
* added support for multicast as unicast
*
* Revision 1.26  2004/04/20 20:10:23  doug
* cleanup debug code
*
* Revision 1.25  2004/04/20 20:08:24  doug
* cleanup debug code
*
* Revision 1.24  2004/04/20 19:43:58  doug
* fix comment
*
* Revision 1.23  2004/04/13 22:29:43  doug
* switched to testing cmdli functions used by upenn
*
* Revision 1.22  2004/04/13 17:57:33  doug
* switch to using internal communications and not passing messages in and out
*
* Revision 1.21  2004/03/22 18:42:16  doug
* coded block transitions
*
* Revision 1.20  2004/03/19 22:48:22  doug
* working on exceptions.
*
* Revision 1.19  2004/03/19 15:30:05  doug
* call and return work
*
* Revision 1.18  2004/03/18 22:11:33  doug
* coded call and return
*
* Revision 1.17  2004/03/17 22:17:11  doug
* working on procedure calls
*
* Revision 1.16  2004/03/15 22:33:34  doug
* working on cutting over to ActionCommand
*
* Revision 1.15  2004/03/12 20:36:59  doug
* snap
*
* Revision 1.14  2004/03/09 22:01:04  doug
* promote Blocks to mission segments and procedures
*
* Revision 1.13  2004/03/08 14:52:35  doug
* cross compiles on visual C++
*
* Revision 1.12  2004/03/01 23:37:53  doug
* working on real overlay
*
* Revision 1.11  2004/03/01 00:47:16  doug
* cmdli runs in sara
*
* Revision 1.10  2004/02/28 21:59:17  doug
* preload dies
*
* Revision 1.9  2004/02/28 16:35:43  doug
* getting cmdli to work in sara
*
* Revision 1.8  2004/02/20 20:54:48  doug
* barriers work
*
* Revision 1.7  2004/02/20 14:39:36  doug
* cmdl interpreter runs a bit
*
* Revision 1.6  2004/01/15 00:24:10  doug
* working on cmdl
*
* Revision 1.5  2004/01/09 18:46:04  doug
* back from Florida
*
* Revision 1.4  2003/11/13 22:29:33  doug
* adding transitions
*
* Revision 1.3  2003/11/11 22:04:13  doug
* getting it to execute
*
* Revision 1.2  2003/11/10 22:48:33  doug
* working on cmdli
*
* Revision 1.1.1.1  2003/11/07 20:27:33  doug
* initial import
*
**********************************************************************/

#include "cmdli.h"
#include "cmdli_implementation.h"

namespace sara
{
// *********************************************************************
/// Constructor for the cmdl interpreter object.
class cmdli *
cmdli::createInstance()
{
   return new cmdli_implementation();
}

// ********************************************************************
}
