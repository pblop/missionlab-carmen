#ifndef GotoCommand_H
#define GotoCommand_H
/**********************************************************************
 **                                                                  **
 **  GotoCommand.h                                                   **
 **                                                                  **
 **  Jump to a labeled command                                       **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: GotoCommand.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: GotoCommand.h,v $
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
// Revision 1.1  2004/10/22 21:40:29  doug
// added goto and command exception support
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "Command.h"

// ********************************************************************
namespace sara 
{

class GotoCommand : public Command
{
   public:
      /// Constructors
      /// pass the empty string "" if no label
      GotoCommand(const string &label, class Unit *unit, const string &destLabel);

      /// Destructor
      virtual ~GotoCommand();

      /// get the destination label 
      const string &getDestination() {return destLabel;}

      /// Stream out in a form suitable for the loader
      virtual string print();

   private:
      /// The destination label for this Goto
      string destLabel;
};

}
/***************************************************************************/
#endif
