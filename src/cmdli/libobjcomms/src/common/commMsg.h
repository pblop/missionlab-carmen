#ifndef commMsg_H
#define commMsg_H
/**********************************************************************
 **                                                                  **
 **  commMsg.h                                                       **
 **                                                                  **
 **  Base class for user communications messages                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: commMsg.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: commMsg.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:54  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.13  2004/11/12 21:45:07  doug
// fixed NPE when uncompress fails
//
// Revision 1.12  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.10  2003/07/10 16:48:53  doug
// test harness for commMsg in and out functions
//
// Revision 1.9  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.8  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.7  2003/02/17 23:00:13  doug
// assigns task lead
//
// Revision 1.6  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.5  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.4  2003/01/31 22:36:48  doug
// working a bit
//
// Revision 1.3  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.2  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.1  2003/01/22 22:09:54  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#if defined(linux)
#include <netinet/in.h>
#endif
#include "mic.h"
#include "ComAddr.h"

namespace sara
{
// *******************************************************************
// Base class for user communications messages
class commMsg
{
public:
   /// Define the MsgType handle
   typedef uword MsgType_T;
   static const MsgType_T INVALID_MSG_TYPE = 0;

   /// Helper functions to read and write the MsgType
   static bool MsgType_to_stream(stringstream *ost, MsgType_T MsgType);
   static MsgType_T MsgType_from_stream(stringstream *ost);

   // ------------------------- start of data ------------------------
   // Holds the type of this message to assist in low cost RTT checking.
   MsgType_T msgType;

   // The name of the sender.
   string senderID;

   // The address of the sender
   ComAddr senderAddr;
   // -------------------------- end of data -------------------------
 
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   virtual bool to_stream(stringstream *ost) const = 0;

   /// Create a copy of the msg 
   virtual commMsg *clone() const = 0;

   /// Copy the information from the source base record into this record
   /// to complete a clone operation
   void cloneBase(const commMsg *src);

   // Helper functions to stream in and out basic types.
   static bool out_byte(stringstream *st, const byte val);
   static bool in_byte(stringstream *st, byte &val);
   static bool out_ubyte(stringstream *st, const ubyte val);
   static bool in_ubyte(stringstream *st, ubyte &val);
   static bool out_word(stringstream *st, const word val);
   static bool in_word(stringstream *st, word &val);
   static bool out_uword(stringstream *st, const uword val);
   static bool in_uword(stringstream *st, uword &val);
   static bool out_short(stringstream *st, const short val);
   static bool in_short(stringstream *st, short &val);
   static bool out_int(stringstream *st, const int val);
   static bool in_int(stringstream *st, int &val);
   static bool out_uint(stringstream *st, const uint val);
   static bool in_uint(stringstream *st, uint &val);
   static bool out_long(stringstream *st, const long val);
   static bool in_long(stringstream *st, long &val);
   static bool out_ulong(stringstream *st, const ulong val);
   static bool in_ulong(stringstream *st, ulong &val);
   static bool out_string(stringstream *st, const string &val);
   static bool in_string(stringstream *st, string &val);
   static bool out_float(stringstream *st, const float val);
   static bool in_float(stringstream *st, float &val);
   static bool out_double(stringstream *st, const double val);
   static bool in_double(stringstream *st, double &val);
   static bool out_bool(stringstream *st, const bool val);
   static bool in_bool(stringstream *st, bool &val);

   // Constructor
   commMsg(MsgType_T t = INVALID_MSG_TYPE);
   commMsg(const commMsg &msg);

   // Destructor
   virtual ~commMsg() {};
};

/*********************************************************************/
}
#endif
