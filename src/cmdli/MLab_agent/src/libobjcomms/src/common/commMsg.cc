/**********************************************************************
 **                                                                  **
 **  commMsg.cc                                                      **
 **                                                                  **
 **  Abstract base class for communications messages                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: commMsg.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: commMsg.cc,v $
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
// Revision 1.17  2004/11/12 21:45:07  doug
// fixed NPE when uncompress fails
//
// Revision 1.16  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.15  2004/04/15 19:12:48  doug
// building DLL using new flags
//
// Revision 1.14  2004/04/14 17:40:15  doug
// fixed endian issues
//
// Revision 1.13  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.12  2003/07/10 16:48:33  doug
// Fixed a nasty bug where in_int and in_uint always returned zero
//
// Revision 1.11  2003/06/17 20:42:36  doug
// adding debug to track down broadcast problem
//
// Revision 1.10  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.9  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.8  2003/02/17 23:00:13  doug
// assigns task lead
//
// Revision 1.7  2003/02/12 20:14:09  doug
// snapshot
//
// Revision 1.6  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.5  2003/01/31 22:36:48  doug
// working a bit
//
// Revision 1.4  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.3  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.2  2003/01/24 22:40:03  doug
// getting closer
//
// Revision 1.1  2003/01/22 22:09:54  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#if defined(WIN32)
#include <winsock2.h>
#define uint16_t u_short
#define uint32_t u_long
#define int32_t long
#endif

#include "commMsg.h"
#include "objcomms.h"

namespace sara
{
// ********************************************************************
/// Constructor
commMsg::commMsg(MsgType_T t) : 
   msgType(t)
{ /* empty */ }

// ********************************************************************
/// Constructor
commMsg::commMsg(const commMsg &msg) :
   msgType(msg.msgType),
   senderID(msg.senderID),
   senderAddr(msg.senderAddr)
{ /* empty */ }

// ********************************************************************
/// Helper functions to read and write the MsgType
bool 
commMsg::MsgType_to_stream(stringstream *ost, MsgType_T mt)
{
   // Write out the message type 
   out_uword(ost, (uword)mt);
   return true;
}

// ********************************************************************
commMsg::MsgType_T 
commMsg::MsgType_from_stream(stringstream *str)
{
   // Read in the message type
   uword mt;
   in_uword(str, mt);
   return mt;
}

// ********************************************************************
/// Copy the information from the source base record into this record
/// to complete a clone operation
void
commMsg::cloneBase(const commMsg *src)
{
   senderID = src->senderID;
   senderAddr = src->senderAddr;
}

// *********************************************************************
bool 
commMsg::out_byte(stringstream *st, const byte val)
{
   st->write((char *)&val, 1);
   return true;
}

// **********************************************************************
bool 
commMsg::in_byte(stringstream *st, byte &val)
{
   st->read((char *)&val, 1);
   return true;
}

// **********************************************************************
bool 
commMsg::out_ubyte(stringstream *st, const ubyte val)
{
   st->write((char *)&val, 1);
   return true;
}

// **********************************************************************
bool 
commMsg::in_ubyte(stringstream *st, ubyte &val)
{
   st->read((char *)&val, 1);
   return true;
}

// **********************************************************************
bool 
commMsg::out_word(stringstream *st, const word val)
{
   uint16_t out = htons((uint16_t)val);
   st->write((char *)&out, 2);
   return true;
}

// **********************************************************************
bool 
commMsg::in_word(stringstream *st, word &val)
{
   char buf[2];
   st->read(buf, 2);
   val = (word)ntohs(*((uint16_t *)buf));
   return true;
}

// **********************************************************************
bool 
commMsg::out_uword(stringstream *st, const uword val)
{
   uint16_t out = htons((uint16_t)val);
   st->write((char *)&out, 2);
   return true;
}

// **********************************************************************
bool 
commMsg::in_uword(stringstream *st, uword &val)
{
   char buf[2];
   st->read(buf, 2);
   val = (uword)ntohs(*((uint16_t *)buf));
   return true;
}

// **********************************************************************
bool 
commMsg::out_short(stringstream *st, const short val)
{
   uint16_t out = htons((uint16_t)val);
   st->write((char *)&out, 2);
   return true;
}

// **********************************************************************
bool 
commMsg::in_short(stringstream *st, short &val)
{
   char buf[2];
   st->read(buf, 2);
   val = (short)ntohs(*((uint16_t *)buf));
   return true;
}

// **********************************************************************
bool 
commMsg::out_ulong(stringstream *st, const ulong val)
{
   uint32_t out = htonl((uint32_t)val);
   st->write((char *)&out, 4);
   return true;
}

// **********************************************************************
bool 
commMsg::in_ulong(stringstream *st, ulong &val)
{
   char buf[4];
   st->read(buf, 4);
   val = (ulong)ntohl(*((uint32_t *)buf));
   return true;
}

// **********************************************************************
bool 
commMsg::out_long(stringstream *st, const long val)
{
   uint32_t out = htonl((uint32_t)val);
   st->write((char *)&out, 4);
   return true;
}

// **********************************************************************
bool 
commMsg::in_long(stringstream *st, long &val)
{
   char buf[4];
   st->read(buf, 4);
   val = (int32_t)ntohl(*((uint32_t *)buf));
   return true;
}

// **********************************************************************
bool 
commMsg::out_uint(stringstream *st, const uint val)
{
   return out_ulong(st, val);
}

// **********************************************************************
bool 
commMsg::in_uint(stringstream *st, uint &val)
{
   ulong t;
   bool rtn = in_ulong(st, t);
   val = t;
   return rtn;
}

// **********************************************************************
bool 
commMsg::out_int(stringstream *st, const int val)
{
   return out_long(st, val);
}

// **********************************************************************
bool 
commMsg::in_int(stringstream *st, int &val)
{
   long t;
   bool rtn =  in_long(st, t);
   val = t;
   return rtn;
}

// **********************************************************************
bool 
commMsg::out_string(stringstream *st, const string &str)
{
   // The two most significant bits specify number of length bytes used
   // 00 = 1
   // 01 = 2
   // 10 = 4
  
   int length = str.size();

   // Write the length out using the minimum size 
   if( length < (1<<6) )
   {
      byte val = (byte)length;
      st->write((char *)&val, 1);
   }
   else if( length < (1<<14) )
   {
      uint16_t val = (uint16_t)length;

      // set the bits
      val |= (1 << 14);
      uint16_t out = htons(val);
      st->write((char *)&out, 2);
   }
   else if( length < (1<<30) )
   {
      uint32_t val = (uint32_t)length;

      // set the bits
      val |= (2 << 30);
      uint32_t out = htonl(val);
      st->write((char *)&out, 4);
   }
   else
   {
      // Gees, how big is it?  I give up.
      return false;
   }

   st->write(str.data(), length);
   return true;
}

// *********************************************************************
bool 
commMsg::in_string(stringstream *st, string &str)
{
   byte buf[4];

   // NOTE: In network byte order (which is what the raw data is)
   // the most significant byte is first.  So, we read the first
   // byte to discover how many more bytes to read to get the length.
   st->read((char *)&buf[0], 1);
   byte bits = (buf[0] >> 6) & 0x03;

   // if it is zero, we are done.
   int length;
   if( bits == 0 )
   {
      length = buf[0];
   }
   else if( bits == 1 )
   {
      st->read((char *)&buf[1], 1);
      buf[0] &= 0x3f;
      length = (int)ntohs(*((uint16_t *)buf));
   }
   else if( bits == 2 )
   {
      st->read((char *)&buf[1], 3);
      buf[0] &= 0x3f;
      length = (int)ntohl(*((uint32_t *)buf));
   }
   else
   {
      // 3 isn't used yet.
      return false;
   }

   char *c = new char[length];
   st->read(c, length);
   str = string(c, length);
   delete [] c;

   return true;
}

// **********************************************************************
// NOT PORTABLE - Only supports IEEE single and double precision   **
//                floating point on big and little endian machines.**
bool 
commMsg::in_float(stringstream *st, float &val)
{
   return in_long(st, *(long *)&val);
}

// **********************************************************************
// NOT PORTABLE - Only supports IEEE single and double precision   **
//                floating point on big and little endian machines.**
bool 
commMsg::out_float(stringstream *st, const float val)
{
   return out_long(st, *(long *)&val);
}

// *********************************************************************
// NOT PORTABLE - Only supports IEEE single and double precision   **
//                floating point on big and little endian machines.**
bool 
commMsg::in_double(stringstream *st, double &val)
{
   bool rtn = true;
   long *lp = (long *)&val;

   if( objcomms::MIC_BYTE_ORDER == objcomms::MIC_BIG_ENDIAN )
   {
      rtn &= in_long(st, *lp);
      rtn &= in_long(st, *(lp+1));
   }
   else
   {
      rtn &= in_long(st, *(lp+1));
      rtn &= in_long(st, *lp);
   }

   return rtn;
}

// *********************************************************************
// NOT PORTABLE - Only supports IEEE single and double precision   **
//                floating point on big and little endian machines.**
bool 
commMsg::out_double(stringstream *st, double val)
{
   bool rtn = true;
   long *lp = (long *)&val;

   if( objcomms::MIC_BYTE_ORDER == objcomms::MIC_BIG_ENDIAN )
   {
      rtn &= out_long(st, *lp);
      rtn &= out_long(st, *(lp+1));
   }
   else
   {
      rtn &= out_long(st, *(lp+1));
      rtn &= out_long(st, *lp);
   }

   return rtn;
}

// *********************************************************************
bool 
commMsg::in_bool(stringstream *st, bool &val)
{
   byte ch;
   bool rtn = in_byte(st, ch);
   if( ch == 0 )
   {
      val = false;
   }
   else if( ch == 1 )
   {
      val = true;
   }
   else
   {
      // Yuck, we got garbage.
      val = false;
      rtn = false;
   }

   return rtn;
}

// **********************************************************************
bool 
commMsg::out_bool(stringstream *st, const bool val)
{
   byte ch = val ? 1 : 0;
   return out_byte(st, ch);
}

// *********************************************************************
}
