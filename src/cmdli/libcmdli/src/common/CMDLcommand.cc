/**********************************************************************
 **                                                                  **
 **  CMDLcommand.cc                                                  **
 **                                                                  **
 **  the structure defining CMDLcommand messages                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLcommand.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLcommand.cc,v $
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
// Revision 1.6  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.5  2004/10/29 22:38:38  doug
// working on waittimeout
//
// Revision 1.4  2004/05/11 19:34:50  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.3  2004/04/30 17:36:01  doug
// works for UPenn
//
// Revision 1.2  2004/04/20 19:01:46  doug
// coded
//
// Revision 1.1  2004/04/16 15:33:46  doug
// snap
//
///////////////////////////////////////////////////////////////////////

#include "CMDLcommand.h"
#include "MsgTypes.h"

namespace sara
{
// ******************************************************************
// Constructor
// Need to set our type field in the record.
CMDLcommand::CMDLcommand() :
   commMsg(MsgTypes::CMDLCOMMAND),
   action(CMD_INVALID)
{ /* empty */ }

// ******************************************************************/
// Write the msg to a stream buffer in network byte order.
// Returns true on success, false if the operation fails
bool 
CMDLcommand::to_stream(stringstream *str) const
{
   // Write our message type as the first data
   MsgType_to_stream(str, msgType);

   // Write the data
   out_string(str, opcon);
   out_uword(str, opconID);
   uint act = action;
   out_uint(str, act);
   out_string(str, mission);
   out_uint(str, version);
   out_string(str, block);
   out_uint(str, command);
   out_uint(str,gotAcks.size() );
   for(stringSet::const_iterator it=gotAcks.begin(); it!=gotAcks.end(); ++it)
   {
      out_string(str, *it);
   }
   out_string(str, data);

   /// The list of robots that we have already received acks from.
   /// They should not send an ack to this message.
   strings gotAcks;
   return true;
}

// *******************************************************************
/// Build the msg from data in a stream buffer in network byte order
/// Returns the object on success, NULL if the operation fails
commMsg *
CMDLcommand::from_stream(stringstream *str)
{
   // Create an empty msg record
   CMDLcommand *msg = new CMDLcommand();

   // read the data
   in_string(str, msg->opcon);
   in_uword(str, msg->opconID);
   uint act;
   in_uint(str, act);
   msg->action = (actionValues)act;
   in_string(str, msg->mission);
   in_uint(str, msg->version);
   in_string(str, msg->block);
   in_uint(str, msg->command);
   uint num;
   in_uint(str, num);
   for(uint i=0; i<num; ++i)
   {
      string val;
      in_string(str, val);
      msg->gotAcks.insert(val);
   }
   in_string(str, msg->data);

   return msg;
}

// *******************************************************************
/// Create a copy of the msg
commMsg *
CMDLcommand::clone() const
{
   // Create an empty msg record
   CMDLcommand *msg = new CMDLcommand();
   msg->cloneBase( this );

   // copy the data over
   msg->opcon = opcon;
   msg->opconID = opconID;
   msg->action = action;
   msg->mission = mission;
   msg->version = version;
   msg->block = block;
   msg->command = command;
   for(stringSet::const_iterator it=gotAcks.begin(); it!=gotAcks.end(); ++it)
   {
      msg->gotAcks.insert( *it );
   }
   msg->data = data;

   return msg;
}

// *******************************************************************
/// convert the command value to a string
string 
CMDLcommand::actionToString(const actionValues action)
{
   string out;

   switch( action )
   {
      case CMD_INVALID:
         out = "INVALID";
         break;

      case CMD_DOWNLOAD:
         out = "DOWNLOAD";
         break;

      case CMD_START:
         out = "START";
         break;

      case CMD_STARTAT:
         out = "STARTAT";
         break;

      case CMD_STOP:
         out = "STOP";
         break;

      case CMD_ESTOP:
         out = "ESTOP";
         break;

      case CMD_PAUSE:
         out = "PAUSE";
         break;

      case CMD_RESUME:
         out = "RESUME";
         break;

      case CMD_RESUMEAT:
         out = "RESUMEAT";
         break;

      case CMD_IGNOREROBOT:
         out = "IGNOREROBOT";
         break;
   }

   return out;
}

// *******************************************************************
// Stream out contents of the record for debugging.
ostream &operator << (ostream & out, const CMDLcommand &rec)
{
   out << "CMDLcommand from " << rec.opcon << "(" << rec.opconID << "): " << CMDLcommand::actionToString(rec.action) << " '" << rec.mission << "' " << rec.version << " Blk: '" << rec.block << "' Cmd: " << rec.command;
   for(stringSet::const_iterator it=rec.gotAcks.begin(); it!=rec.gotAcks.end(); ++it)
   {
      out << " GotAck:" << *it;
   }
   out << " data=" << rec.data;

   return out;
}

// *******************************************************************
}
