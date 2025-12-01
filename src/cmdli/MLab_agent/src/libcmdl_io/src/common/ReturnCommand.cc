/**********************************************************************
 **                                                                  **
 **  ReturnCommand.cc - Return from a subroutine                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: ReturnCommand.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: ReturnCommand.cc,v $
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
// Revision 1.4  2004/10/22 21:40:29  doug
// added goto and command exception support
//
// Revision 1.3  2004/05/11 19:34:49  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2004/03/18 22:11:17  doug
// coded call and return
//
// Revision 1.1  2004/03/17 21:53:33  doug
// working on subroutine calls
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include "Unit.h"
#include "ReturnCommand.h"

namespace sara 
{
// ***************************************************************************
// Constructor
ReturnCommand::ReturnCommand(const string &theLabel, Unit *theUnit) :
   Command(theLabel, theUnit)
{ /* empty */ }

// **************************************************************************
// Destructor
ReturnCommand::~ReturnCommand()
{
}

//**************************************************************************
// Stream out in a form suitable for the loader
string
ReturnCommand::print()
{
   stringstream out;

   // print the call
   out << "Return";

   return out.str();
}

// **************************************************************************
}
