/**********************************************************************
 **                                                                  **
 **  CMDLcommandAck.cc                                               **
 **                                                                  **
 **  the structure defining CMDLcommandAck messages                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLcommandAck.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLcommandAck.cc,v $
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
// Revision 1.4  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.3  2004/05/11 19:34:50  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2004/04/30 17:36:01  doug
// works for UPenn
//
// Revision 1.1  2004/04/27 16:21:45  doug
// support an operator console
//
///////////////////////////////////////////////////////////////////////

#include "CMDLcommandAck.h"
#include "MsgTypes.h"

namespace sara
{

// ******************************************************************
// Constructor
// Need to set our type field in the record.
CMDLcommandAck::CMDLcommandAck() :
   commMsg(MsgTypes::CMDLCOMMANDACK),
   response(INVALID)
{ /* empty */ }

// ******************************************************************/
// Write the msg to a stream buffer in network byte order.
// Returns true on success, false if the operation fails
bool 
CMDLcommandAck::to_stream(stringstream *str) const
{
   // Write our message type as the first data
   MsgType_to_stream(str, msgType);

   // Write the data
   out_string(str, opcon);
   out_uword(str, opconID);
   out_string(str, name);
   uint rsp = response;
   out_uint(str, rsp);


   return true;
}

// *******************************************************************
/// Build the msg from data in a stream buffer in network byte order
/// Returns the object on success, NULL if the operation fails
commMsg *
CMDLcommandAck::from_stream(stringstream *str)
{
   // Create an empty msg record
   CMDLcommandAck *msg = new CMDLcommandAck();

   // read the data
   in_string(str, msg->opcon);
   in_uword(str, msg->opconID);
   in_string(str, msg->name);
   uint rsp;
   in_uint(str, rsp);
   msg->response = (responseValues)rsp;

   return msg;
}

// *******************************************************************
/// Create a copy of the msg
commMsg *
CMDLcommandAck::clone() const
{
   // Create an empty msg record
   CMDLcommandAck *msg = new CMDLcommandAck();
   msg->cloneBase( this );

   // copy the data over
   msg->opcon = opcon;
   msg->opconID = opconID;
   msg->name = name;
   msg->response = response;

   return msg;
}

// *******************************************************************
// Stream out contents of the record for debugging.
ostream &operator << (ostream & out, const CMDLcommandAck &rec)
{
   out << "CMDLcommandAck: opcon=" << rec.opcon << " opconID=" << rec.opconID << " name=" << rec.name << " response=" << rec.response;

   return out;
}

// *******************************************************************
}
