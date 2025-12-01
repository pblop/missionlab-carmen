/**********************************************************************
 **                                                                  **
 **  ActionCommand.cc - A parameter list                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: ActionCommand.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: ActionCommand.cc,v $
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
// Revision 1.5  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.4  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
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
#include "ActionCommand.h"
#include "Unit.h"
#include "env_object.h"

namespace sara 
{

// ***************************************************************************
// Constructor
ActionCommand::ActionCommand(const string &theLabel, Unit *theUnit, const string &theAction, keyValueMap *theOptions, cmdl_transition::Transitions_T &ourTransitions) :
   Command(theLabel, theUnit),
   transitions(ourTransitions),
   action(theAction),
   options(theOptions)
{ /* empty */ }

// **************************************************************************
// Destructor
ActionCommand::~ActionCommand()
{
}

//**************************************************************************
// Stream out in a form suitable for the loader
string
ActionCommand::print()
{
   stringstream out;

   // print the command
   out << action << " ";
   if( options && !options->empty() )
   {
      bool needComma = false;
      for(keyValueMap::iterator it=options->begin(); it!=options->end(); ++it)
      {
         if( needComma )
            out << ", " << endl;
         else
            needComma = true;

         out << it->first << "=" << it->second;
      }
   }

   return out.str();
}

// **************************************************************************
}
