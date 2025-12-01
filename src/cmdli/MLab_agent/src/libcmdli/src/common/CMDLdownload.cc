/**********************************************************************
 **                                                                  **
 **  CMDLdownload.cc                                                  **
 **                                                                  **
 **  the structure defining CMDLdownload messages                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLdownload.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLdownload.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:14:58  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.3  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.2  2004/06/11 16:10:02  doug
// move to sara namespace
//
// Revision 1.1  2004/04/27 16:21:45  doug
// support an operator console
//
///////////////////////////////////////////////////////////////////////

#include "CMDLdownload.h"
#include "MsgTypes.h"

namespace sara
{
// ******************************************************************
// Constructor
// Need to set our type field in the record.
CMDLdownload::CMDLdownload() :
   commMsg(MsgTypes::CMDLDOWNLOAD)
{ /* empty */ }

// ******************************************************************/
// Write the msg to a stream buffer in network byte order.
// Returns true on success, false if the operation fails
bool 
CMDLdownload::to_stream(stringstream *str) const
{
   // Write our message type as the first data
   MsgType_to_stream(str, msgType);

   // Write the data
   out_string(str, mission);
   out_uint(str, version);
   out_string(str, cmdlCode);

   return true;
}

// *******************************************************************
/// Build the msg from data in a stream buffer in network byte order
/// Returns the object on success, NULL if the operation fails
commMsg *
CMDLdownload::from_stream(stringstream *str)
{
   // Create an empty msg record
   CMDLdownload *msg = new CMDLdownload();

   // read the data
   in_string(str, msg->mission);
   in_uint(str, msg->version);
   in_string(str, msg->cmdlCode);

   return msg;
}

// *******************************************************************
/// Create a copy of the msg
commMsg *
CMDLdownload::clone() const
{
   // Create an empty msg record
   CMDLdownload *msg = new CMDLdownload();
   msg->cloneBase( this );

   // copy the data over
   msg->mission = mission;
   msg->version = version;
   msg->cmdlCode = cmdlCode;

   return msg;
}

// *******************************************************************
// Stream out contents of the record for debugging.
ostream &operator << (ostream & out, const CMDLdownload &rec)
{
   out << "CMDLdownload:  '" << rec.mission << "' " << rec.version << " cmdlCode:" << endl << rec.cmdlCode << endl;

   return out;
}

// *******************************************************************
}
