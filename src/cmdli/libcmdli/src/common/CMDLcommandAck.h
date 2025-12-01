#ifndef CMDLcommandAck_H
#define CMDLcommandAck_H
/**********************************************************************
 **                                                                  **
 **  CMDLcommandAck.h                                                **
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

/* $Id: CMDLcommandAck.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLcommandAck.h,v $
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

#include "mic.h"
#include "commMsg.h"

namespace sara
{
// *******************************************************************
// commandAck messages broadcast between the CMDL interpreters
class CMDLcommandAck: public commMsg
{
public:
   // Constructors
   CMDLcommandAck();

   // Destructor
   virtual ~CMDLcommandAck() {};

   // -------------------- start of data ------------------------
   /// It takes two pieces of info to uniquely identify a command
   /// The opcon (name of the operator console sending the command)
   /// and the opconID (command sequence number from the opcon)
   string opcon;

   /// The user's taskID
   uword opconID;

   /// Their name
   string name;

   /// Their response to the command
   typedef enum {
      INVALID = 0,     // not a valid response
      OK,              // command succeeded
      FAILED           // the command failed because it was invalid
   } responseValues;
   responseValues response;

   // -------------------- end of data ------------------------
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   bool to_stream(stringstream *str) const;

   /// Build the msg from data in a stream buffer in network byte order
   /// Returns the object on success, NULL if the operation fails
   static commMsg *from_stream(stringstream *str);

   /// Create a copy of the msg
   commMsg *clone() const;

   // Stream out contents of the record for debugging.
   friend ostream &operator << (ostream & out, const CMDLcommandAck &obj);
};

// *********************************************************************
}
#endif

