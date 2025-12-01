/**********************************************************************
 **                                                                  **
 **  objcomms.cc                                                     **
 **                                                                  **
 **  Provides an object-oriented interface to communications streams **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: objcomms.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objcomms.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:31:55  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.27  2004/08/12 16:28:53  doug
// change open interface to add a parameter to append instance number or not
//
// Revision 1.26  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.25  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.24  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.23  2004/04/14 17:40:15  doug
// fixed endian issues
//
// Revision 1.22  2004/04/12 21:20:21  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.21  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.20  2004/03/31 14:15:06  doug
// standardize the objcomms interface methods
//
// Revision 1.19  2004/03/01 00:47:10  doug
// cmdli runs in sara
//
// Revision 1.18  2003/07/12 01:59:44  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.17  2003/07/08 20:49:05  doug
// Rework to get controller to exit cleanly on terminate
//
// Revision 1.16  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.15  2003/04/11 18:46:55  doug
// added export capabilities
//
// Revision 1.14  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.13  2003/02/19 22:15:15  doug
// Change waitfor to use the new TimeOfDay
//
// Revision 1.12  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.11  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.10  2003/02/10 22:39:54  doug
// tested out well
//
// Revision 1.9  2003/02/06 20:29:30  doug
// trying to get the packet loss down
//
// Revision 1.8  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.7  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.6  2003/01/24 22:40:03  doug
// getting closer
//
// Revision 1.5  2003/01/23 22:39:12  doug
// making progress
//
// Revision 1.4  2003/01/22 22:09:54  doug
// snapshot
//
// Revision 1.3  2003/01/20 22:08:36  doug
// the framework builds
//
// Revision 1.2  2003/01/20 22:03:05  doug
// snapshot
//
// Revision 1.1.1.1  2003/01/20 20:43:41  doug
// initial import
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <stdlib.h>
#include <string.h>
#include "commMsg.h"
#include "objcomms_implementation.h"
#include "objcomms.h"

namespace sara
{
// ********************************************************************

/* From /usr/include/endian.h
   Definitions for byte order, according to significance of bytes,
   from low addresses to high addresses.  The value is what you get by
   putting '4' in the most significant byte, '3' in the second most
   significant byte, '2' in the second least significant byte, and '1'
   in the least significant byte, and then writing down one digit for
   each byte, starting with the byte at the lowest address at the left,
   and proceeding to the byte with the highest address at the right.  */

ulong objcomms::MIC_BYTE_ORDER;
ulong objcomms::MIC_BIG_ENDIAN = 0x04030201;
ulong objcomms::MIC_LITTLE_ENDIAN = 0x01020304;


objcomms_implementation *objcomms::cs = NULL;

// *********************************************************************
/// Create a communications object 
objcomms::objcomms() :
//   debug(DEBUG_NONE),
   debug(DEBUG_GEN),
   bytesTrn(0),
   bytesRcv(0),
   packetsTrn(0),
   packetsRcv(0),
   messagesTrn(0),
   messagesRcv(0),
   packetsTrnDiscard(0),
   packetsRcvDiscard(0),
   packetsCorrupted(0)
{
   if( !cs )
   {
      cs = objcomms_implementation::create(this);
   }

   byte *p = (byte *)&MIC_BYTE_ORDER;
   p[0] = 0x04;
   p[1] = 0x03;
   p[2] = 0x02;
   p[3] = 0x01;

//fprintf(stderr,"testLong = %lx\n", testLong);
//fprintf(stderr,"MIC_BIG_ENDIAN = %lx\n", MIC_BIG_ENDIAN);
//fprintf(stderr,"MIC_LITTLE_ENDIAN = %lx\n", MIC_LITTLE_ENDIAN);
}

// *********************************************************************
/// Destroy the communications object
/// Closes the communications stream and invalidates any handles
objcomms::~objcomms()
{
   if( cs )
   {
      delete cs;
      cs = NULL;
   }
}

// *********************************************************************
/// load a set of addresses for the team members
/// Loading these values Will cause multicasts to use multiple unitcast packets!
/// The address format is ip:port, for example: "192.168.1.3:4321"
bool 
objcomms::loadMemberAddresses(const strings &addresses)
{
   if( cs )
   {
      return cs->loadMemberAddresses(addresses);
   }

   return false;
}

// *********************************************************************
/// open the communications port 
/// Returns true on success, false on failure.
/// multicastAddress:        Multicast address (e.g., 224.0.0.0:PORT to 224.0.0.255:PORT)
///                          where PORT is the port number 0-65535 to use for multcast.
///                          If PORT is not specified, then port (next parm) will be used.
/// port:                    Base unicast port.  Several ports may be used,
///                          [port ... port+maxInstancesPerHost-1]
/// procName:                A unique network name for the process.
///                          If empty, will use hostname.
/// appendInstance:          If true and more than one instance, will append
///                          the instance number to the name to make it unique.
/// max_instances_per_host : The maximum number of copies of the 
///                          program running on any one host.
///                          (NOTE: Keep as small as practical)
/// instance_number:         If not NULL, the instance number of 
///                          this process on this host is returned.
bool 
objcomms::open(const string &multicastAddress,
             const uint    port,
	     const string  procName,
	     const bool    appendInstance,
             const uint    maxInstancesPerHost,
             uint         *instanceNumber)
{
   bool rtn = false;
   if( cs )
   {
      if( cs->isOpen() )
      {
         cerr << "objcomms::open - skipping open since already open" << endl;
         rtn = true;
      }
      else
      {
         rtn = cs->open(multicastAddress, port, procName, appendInstance, maxInstancesPerHost, instanceNumber);
      }
   }
   return rtn;
}

// *********************************************************************
/// Get our unique network name
string 
objcomms::getName() const
{
   string rtn;
   if( cs )
   {
      rtn = cs->getName();
   }
   return rtn;
}

// *********************************************************************
/// Wait for the transmit queue to drain out, or the timeout
void 
objcomms::waitForTransmitter(const TimeOfDay &timeout)
{
   if( cs )
   {
      cs->waitForTransmitter(timeout);
   }
}

// *********************************************************************
/// Send the message reliabily to the destination
bool 
objcomms::sendMsg(const ComAddr &dest, const commMsg &message)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->sendMsg(dest, message);
   }
   return rtn;
}

// *********************************************************************
/// Broadcast the message to all listeners
bool 
objcomms::broadcastMsg(const commMsg &message)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->broadcastMsg(message);
   }
   return rtn;
}

// *********************************************************************
/// Register a marshalling & unmarshalling handler for a message type
/// returns: true on success
///          false if another handler already registered
bool 
objcomms::attachHandler(commMsg::MsgType_T msgType, messageBuilder_T builder)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->attachHandler(msgType, builder);
   }
   return rtn;
}

// *********************************************************************
/// Subscribe to get copies of messages meeting the criteria
/// Returns: a subscription handle used to cancel the subscription
objcomms::SubscriptionHandle 
objcomms::subscribeTo(commMsg::MsgType_T msgType, message_callback_t callback, USERDATA userData)
{
   objcomms::SubscriptionHandle rtn = -1;
   if( cs )
   {
      rtn = cs->subscribeTo(msgType, callback, userData);
   }
   return rtn;
}

// ********************************************************************
/// Cancel a previous subscription
bool 
objcomms::cancelSubscription(SubscriptionHandle handle)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->cancelSubscription(handle);
   }
   return rtn;
}

// ********************************************************************
/// enable message compression
/// Returns true on success, false if compression is not supported.
bool 
objcomms::enableCompression()
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->enableCompression();
   }
   return rtn;
}

// ********************************************************************
/// disable message compression
/// Returns true on success, false if compression was not enabled.
bool 
objcomms::disableCompression()
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->disableCompression();
   }
   return rtn;
}

// ********************************************************************
/// enable message encription
/// Returns true on success, false if encryption is not supported.
bool 
objcomms::enableEncryption(const string &encryptKey)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->enableEncryption(encryptKey);
   }
   return rtn;
}

// ********************************************************************
/// disable message encription
/// Returns true on success, false if encryption was not enabled.
bool 
objcomms::disableEncryption()
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->disableEncryption();
   }
   return rtn;
}

// ********************************************************************
/// change the key used for message encription
/// Returns true on success, false if the key is invalid
bool 
objcomms::changeEncryptionKey(const string &encryptKey)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->changeEncryptionKey(encryptKey);
   }
   return rtn;
}

// ********************************************************************
/// Begin queueing messages for us of the specified type
/// msgType is the type of message to wait for
/// Returns a queueing handle on success, NULL on failure.
/// NOTE: endQueueing must be called after a successful call to 
/// beginQueueing to stop the queueing process and free consumed resources
objcomms::QueueingHandle
objcomms::beginQueueing(commMsg::MsgType_T msgType)
{
   objcomms::QueueingHandle rtn = NULL;
   if( cs )
   {
      rtn = cs->beginQueueing(msgType);
   }
   return rtn;
}

// ********************************************************************
/// Suspend the thread until either a message arrives or the timeout expires
/// handle was returned from a call to beginQueueing
/// timeout is the maximum time to wait.
/// returns: The message, if one arrived
///          NULL if the timeout expired or the handle is invalid
commMsg *
objcomms::waitFor(objcomms::QueueingHandle handle, const TimeOfDay &timeout)
{
   commMsg *rtn = NULL;
   if( cs )
   {
      rtn = cs->waitFor(handle, timeout);
   }
   return rtn;
}

// ********************************************************************
/// Stop queueing messages for us 
/// Returns true on success, false if the handle is invalid
/// NOTE: endQueueing must be called after a successful call to 
/// beginQueueing to stop the queueing process and free consumed resources
bool 
objcomms::endQueueing(objcomms::QueueingHandle handle)
{
   bool rtn = false;
   if( cs )
   {
      rtn = cs->endQueueing(handle);
   }
   return rtn;
}

// ********************************************************************
/// debug function to print the com link stats
ostream &operator << (ostream & out, const objcomms &obj)
{
   // dump the stats
   out << "Bytes sent:                 " << obj.bytesTrn << endl;
   out << "Bytes received:             " << obj.bytesRcv << endl;
   out << "Packets sent:               " << obj.packetsTrn << endl;
   out << "Packets received:           " << obj.packetsRcv << endl;
   out << "Messages sent:              " << obj.messagesTrn << endl;
   out << "Messages received:          " << obj.messagesRcv << endl;
   out << "Outbound packets discarded: " << obj.packetsTrnDiscard << endl;
   out << "Inbound packets discarded:  " << obj.packetsRcvDiscard << endl;
   out << "Inbound packets corrupted:  " << obj.packetsCorrupted << endl;

   return out;
}

/****************************************************************************/
}
