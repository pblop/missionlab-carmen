/**********************************************************************
 **                                                                  **
 **  GotoCommand.cc - Goto a subroutine                              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: GotoCommand.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: GotoCommand.cc,v $
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
// Revision 1.1  2004/10/22 21:40:29  doug
// added goto and command exception support
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "Unit.h"
#include "GotoCommand.h"

namespace sara 
{
// ***************************************************************************
// Constructor
GotoCommand::GotoCommand(const string &theLabel, Unit *theUnit, const string &theDestLabel) :
   Command(theLabel, theUnit),
   destLabel(theDestLabel)
{ /* empty */ }

// **************************************************************************
// Destructor
GotoCommand::~GotoCommand()
{
}

//**************************************************************************
// Stream out in a form suitable for the loader
string
GotoCommand::print()
{
   stringstream out;

   // print the Goto
   out << "Goto " << destLabel;

   return out.str();
}

// **************************************************************************
}
