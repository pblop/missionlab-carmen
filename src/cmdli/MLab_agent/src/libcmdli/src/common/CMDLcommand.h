#ifndef CMDLcommand_H
#define CMDLcommand_H
/**********************************************************************
 **                                                                  **
 **  CMDLcommand.h                                                   **
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

/* $Id: CMDLcommand.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLcommand.h,v $
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
// Revision 1.7  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.6  2004/10/29 22:38:38  doug
// working on waittimeout
//
// Revision 1.5  2004/05/11 19:34:50  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2004/04/30 17:36:01  doug
// works for UPenn
//
// Revision 1.3  2004/04/20 19:01:46  doug
// coded
//
// Revision 1.2  2004/04/20 18:46:30  doug
// working on commands
//
// Revision 1.1  2004/04/16 15:33:46  doug
// snap
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "commMsg.h"

namespace sara
{
// *******************************************************************
// command messages broadcast between the CMDL interpreters
class CMDLcommand: public commMsg
{
public:
   // Constructors
   CMDLcommand();

   // Destructor
   virtual ~CMDLcommand() {};

   // -------------------- start of data ------------------------
   // It takes two pieces of info to uniquely identify a command
   // The opcon (name of the operator console sending the command)
   // and the opconID (command sequence number from the opcon)
   string opcon;

   // The user's taskID
   uword opconID;

   /// The commanded action 
   typedef enum {
      CMD_INVALID = 0,     // not a valid command message
      CMD_START,           // Start executing the mission specified below.
      CMD_STARTAT,         // Start executing the mission specified below.
      CMD_STOP,            // Stop executing the mission and loiter
      CMD_ESTOP,           // Emergency stop of all robot movement
      CMD_PAUSE,           // Pause execution of the mission
      CMD_RESUME,          // Resume execution of the paused mission
      CMD_RESUMEAT,        // Resume execution of the paused mission, 
                           // but at the block/command specified below.
      CMD_DOWNLOAD,        // download the new mission
      CMD_IGNOREROBOT      // start ignoring the specified robot
   } actionValues;
   actionValues action;

   /// The identifier for the mission 
   string mission;

   /// The version number of the mission
   uint version;

   /// The block within the mission to STARTAT or RESUMEAT
   string block;

   /// The index of the active command within the active block
   uint command;

   /// The list of robots that we have already received acks from.
   /// They should not send an ack to this message.
   stringSet gotAcks;

   /// The CMDL mission data
   string data;

   // -------------------- end of data ------------------------
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   bool to_stream(stringstream *str) const;

   /// Build the msg from data in a stream buffer in network byte order
   /// Returns the object on success, NULL if the operation fails
   static commMsg *from_stream(stringstream *str);

   /// Create a copy of the msg
   commMsg *clone() const;

   /// convert the command value to a string
   static string actionToString(const actionValues action);

   // Stream out contents of the record for debugging.
   friend ostream &operator << (ostream & out, const CMDLcommand &obj);
};

// *********************************************************************
}
#endif

