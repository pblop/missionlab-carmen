#ifndef CMDLstatus_H
#define CMDLstatus_H
/**********************************************************************
 **                                                                  **
 **  CMDLstatus.h                                                    **
 **                                                                  **
 **  the structure defining CMDLstatus messages                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLstatus.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLstatus.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/11 10:30:15  endo
// Compiling error fixed.
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
// Revision 1.7  2004/11/05 16:28:25  doug
// rename INVALID to avoid a conflict with missionlab
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

#include "mic.h"
#include "commMsg.h"
#include "cmdl.h"
#include "TimeOfDay.h"

namespace sara
{
// *******************************************************************
// status messages broadcast between the CMDL interpreters
class CMDLstatus: public commMsg
{
public:
   // Constructors
   CMDLstatus();

   // Destructor
   virtual ~CMDLstatus() {};

   // -------------------- start of data ------------------------
   /// The sender's identifying name
   string name;

   /// The identifier for the active mission the sender is executing
   string mission;

   /// The version number of the mission
   uint version;

   /// The active block within the mission
   string activeBlock;

   /// The index of the active command within the active block
   uint activeCommand;

   /// The sender's current operational status for the active command
   typedef enum {
      MSGINVALID    = 0,     // not a valid status message
      CMDEXECUTING,          // cmd executing normally (i.e., still moving towards the waypoint)
      CMDFAILED,             // cmd failed and will not complete (i.e., waypoint is unreachable)
      CMDDONE,               // sender successfully finished the command (i.e., at the waypoint)
      ATBARRIER,             // cmd is complete and sender is waiting at synchronization barrier
      PASTBARRIER,           // Sender is through the barrier and waiting for teammates to notice
      MISSIONDONE,           // sender has completed the mission
      PAUSED,                // operator has paused the robot
      STOPPED                // operator has stopped the robot
   } statusValues;
   statusValues status;

   /// List of other nodes we are waiting for at a barrier
   typedef vector < string > blockList_T;
   blockList_T blocklist;

   /// List of other nodes we are ignoring
   typedef vector < string > ignoreList_T;
   ignoreList_T ignorelist;

   // -------------------- end of data ------------------------
   
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   bool to_stream(stringstream *str) const;

   /// Build the msg from data in a stream buffer in network byte order
   /// Returns the object on success, NULL if the operation fails
   static commMsg *from_stream(stringstream *str);

   /// Create a copy of the msg
   commMsg *clone() const;

   /// convert the status value to a string
   static string statusToString(const statusValues status);

   // Stream out contents of the record for debugging.
   friend ostream &operator << (ostream & out, const CMDLstatus &obj);
};

// *********************************************************************
}
#endif

