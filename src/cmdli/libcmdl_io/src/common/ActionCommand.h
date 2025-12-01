#ifndef ActionCommand_H
#define ActionCommand_H
/**********************************************************************
 **                                                                  **
 **  ActionCommand.h                                                 **
 **                                                                  **
 **  Keep track of all the information involved with a single robot  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: ActionCommand.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: ActionCommand.h,v $
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
// Revision 1.6  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.5  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2004/03/17 21:53:33  doug
// working on subroutine calls
//
// Revision 1.3  2004/03/15 21:41:06  doug
// coded ActionCommand and Call
//
// Revision 1.2  2004/03/12 20:36:58  doug
// snap
//
// Revision 1.1  2004/03/11 19:43:42  doug
// splitting off from Command
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "RobotType_Info.h"
#include "cmdl_transition.h"
#include "Command.h"

namespace sara 
{
// ********************************************************************

class ActionCommand : public Command
{
   public:
      /// Constructors
      /// the object takes over ownership of the options object
      /// the label should be the empty string "" if the action is not labeled
      ActionCommand(const string &label, class Unit *unit, const string &action, keyValueMap *options, cmdl_transition::Transitions_T &transitions);

      /// Destructor
      virtual ~ActionCommand();

      // Maintain the action specified for this ActionCommand
      const string &getAction() {return action;}

      // Maintain the environmental object specified for this ActionCommand
      const keyValueMap *getOptions() const {return options;}

      /// The list of transitions leaving this ActionCommand
      cmdl_transition::Transitions_T transitions;

      /// Stream out in a form suitable for the loader
      virtual string print();

   private:
      /// The action tied to this ActionCommand
      string action;

      /// The options specified for the action
      keyValueMap *options;
};

/***************************************************************************/
}
#endif
