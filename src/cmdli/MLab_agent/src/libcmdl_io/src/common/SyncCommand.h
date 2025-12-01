#ifndef SyncCommand_H
#define SyncCommand_H
/**********************************************************************
 **                                                                  **
 **  SyncCommand.h                                                   **
 **                                                                  **
 **  Force a synchronization                                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2004.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: SyncCommand.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: SyncCommand.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:19:15  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.1  2004/07/30 13:41:43  doug
// support sync command
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "Command.h"

// ********************************************************************
namespace sara 
{

class SyncCommand : public Command
{
   public:
      /// Constructors
      /// the object takes over ownership of the options object
      /// pass the empty string "" if not labeled
      SyncCommand(const string &label, class Unit *unit);

      /// Destructor
      virtual ~SyncCommand();

      /// Stream out in a form suitable for the loader
      virtual string print();

   private:
};

}
/***************************************************************************/
#endif
