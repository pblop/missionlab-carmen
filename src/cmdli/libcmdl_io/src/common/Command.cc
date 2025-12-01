/**********************************************************************
 **                                                                  **
 **  Command.cc - A parameter list                                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: Command.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: Command.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:19:14  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.10  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.9  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  2004/03/15 21:41:06  doug
// coded ActionCommand and Call
//
// Revision 1.7  2004/03/12 20:36:58  doug
// snap
//
// Revision 1.6  2004/03/09 21:36:08  doug
// promote Blocks to mission segments and procedures
//
// Revision 1.5  2004/03/01 23:37:51  doug
// working on real overlay
//
// Revision 1.4  2004/02/20 14:39:35  doug
// cmdl interpreter runs a bit
//
// Revision 1.3  2003/11/10 22:48:30  doug
// working on cmdli
//
// Revision 1.2  2003/11/07 20:29:27  doug
// added cmdli interpreter library
//
// Revision 1.1  2003/11/03 16:45:16  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "Unit.h"
#include "Command.h"

namespace sara 
{
// ***************************************************************************
// Constructor
Command::Command(const string &theLabel, Unit *theUnit) :
   label(theLabel),
   unit(theUnit)
{
   isLabeled = !theLabel.empty();

   // cache the list of robots used in this command
   if( !theUnit )
   {
      FATAL_ERROR("Command::Command - NULL unit parameter");
   }
   names = theUnit->getRobotNames();
}

// ***************************************************************************
// Destructor
Command::~Command()
{
}

// ***************************************************************************
}
