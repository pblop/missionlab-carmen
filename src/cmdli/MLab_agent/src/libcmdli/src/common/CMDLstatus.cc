/**********************************************************************
 **                                                                  **
 **  CMDLstatus.cc                                                   **
 **                                                                  **
 **  Used by user injecting a task                                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLstatus.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLstatus.cc,v $
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
// Revision 1.8  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.7  2004/11/06 01:11:59  doug
// snap
//
// Revision 1.6  2004/07/30 13:42:15  doug
// handles comm timeouts and resyncs
//
// Revision 1.5  2004/05/11 19:34:50  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2004/04/13 22:29:43  doug
// switched to testing cmdli functions used by upenn
//
// Revision 1.3  2004/04/13 17:57:33  doug
// switch to using internal communications and not passing messages in and out
//
// Revision 1.2  2004/03/01 00:47:16  doug
// cmdli runs in sara
//
// Revision 1.1  2004/02/28 16:38:55  doug
// getting cmdli to work in sara
//
///////////////////////////////////////////////////////////////////////

#include "CMDLstatus.h"
#include "MsgTypes.h"

namespace sara
{
// ******************************************************************
// Constructor
// Need to set our type field in the record.
CMDLstatus::CMDLstatus() :
   commMsg(MsgTypes::CMDLSTATUS),
   status(MSGINVALID)
{ /* empty */ }

// ******************************************************************/
// Write the msg to a stream buffer in network byte order.
// Returns true on success, false if the operation fails
bool 
CMDLstatus::to_stream(stringstream *str) const
{
   // Write our message type as the first data
   MsgType_to_stream(str, msgType);

   // Write the data
   out_string(str, name);
   out_string(str, mission);
   out_uint(str, version);
   out_string(str, activeBlock);
   out_uint(str, activeCommand);
   int stat = status;
   out_int(str, stat);

   // write the number of strings to follow, and then the strings
   uint num = blocklist.size();
   out_uint(str, num);
   for(uint i=0; i<num; i++)
   {
      out_string(str, blocklist[i]);
   }

   // write the number of strings to follow, and then the strings
   num = ignorelist.size();
   out_uint(str, num);
   for(uint i=0; i<num; i++)
   {
      out_string(str, ignorelist[i]);
   }

   return true;
}

// *******************************************************************
/// Build the msg from data in a stream buffer in network byte order
/// Returns the object on success, NULL if the operation fails
commMsg *
CMDLstatus::from_stream(stringstream *str)
{
   // Create an empty msg record
   CMDLstatus *msg = new CMDLstatus();

   // read the data
   in_string(str, msg->name);
   in_string(str, msg->mission);
   in_uint(str, msg->version);
   in_string(str, msg->activeBlock);
   in_uint(str, msg->activeCommand);
   int stat;
   in_int(str, stat);
   msg->status = (statusValues)stat;

   // read the number of strings to follow, and then the strings
   uint num;
   in_uint(str, num);
   for(uint i=0; i<num; i++)
   {
      string val;
      in_string(str, val);
      msg->blocklist.push_back(val);
   }

   // read the number of strings to follow, and then the strings
   in_uint(str, num);
   for(uint i=0; i<num; i++)
   {
      string val;
      in_string(str, val);
      msg->ignorelist.push_back(val);
   }

   return msg;
}

// *******************************************************************
/// Create a copy of the msg
commMsg *
CMDLstatus::clone() const
{
   // Create an empty msg record
   CMDLstatus *msg = new CMDLstatus();
   msg->cloneBase( this );

   // copy the data over
   msg->name = name;
   msg->mission = mission;
   msg->version = version;
   msg->activeBlock = activeBlock;
   msg->activeCommand = activeCommand;
   msg->status = status;

   // copy the strings 
   for(uint i=0; i<blocklist.size(); i++)
   {
      msg->blocklist.push_back(blocklist[i]);
   }

   // copy the strings 
   for(uint i=0; i<ignorelist.size(); i++)
   {
      msg->ignorelist.push_back(ignorelist[i]);
   }

   return msg;
}

// *******************************************************************
/// convert the status value to a string
string 
CMDLstatus::statusToString(const statusValues status)
{
   string out;

   switch( status )
   {
      case MSGINVALID:
         out = "MSGINVALID";
         break;

      case CMDEXECUTING:
         out = "CMDEXECUTING";
         break;

      case CMDFAILED:
         out = "CMDFAILED";
         break;

      case CMDDONE:
         out = "CMDDONE";
         break;

      case ATBARRIER:
         out = "ATBARRIER";
         break;

      case PASTBARRIER:
         out = "PASTBARRIER";
         break;

      case MISSIONDONE:
         out = "MISSIONDONE";
         break;

      case PAUSED:
         out = "PAUSED";
         break;

      case STOPPED:
         out = "STOPPED";
         break;
   }

   return out;
}

// *******************************************************************
// Stream out contents of the record for debugging.
ostream &operator << (ostream & out, const CMDLstatus &rec)
{
   out << "'" << rec.name << "' " << rec.mission << ":" << rec.version << " Blk: '" << rec.activeBlock << "' Cmd: " << rec.activeCommand << " " << CMDLstatus::statusToString(rec.status);

   // dump the strings 
   if( !rec.blocklist.empty() )
   {
      out << "Waiting for: " << rec.blocklist[0];
      for(uint i=1; i<rec.blocklist.size(); i++)
      {
         out << ", " << rec.blocklist[i];
      }
   }

   // dump the strings 
   if( !rec.ignorelist.empty() )
   {
      out << "Ignoring: " << rec.ignorelist[0];
      for(uint i=1; i<rec.ignorelist.size(); i++)
      {
         out << ", " << rec.ignorelist[i];
      }
   }

   return out;
}

// *******************************************************************
}
