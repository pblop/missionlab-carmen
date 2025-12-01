#ifndef OBJcomms_H
#define OBJcomms_H
/**********************************************************************
 **                                                                  **
 **  objcomms.h                                                      **
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

/* $Id: objcomms.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objcomms.h,v $
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
// Revision 1.30  2004/08/12 16:28:53  doug
// change open interface to add a parameter to append instance number or not
//
// Revision 1.29  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.28  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.27  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.26  2004/04/14 17:40:15  doug
// fixed endian issues
//
// Revision 1.25  2004/04/12 21:20:21  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.24  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.23  2004/03/31 14:15:06  doug
// standardize the objcomms interface methods
//
// Revision 1.22  2004/03/01 00:47:10  doug
// cmdli runs in sara
//
// Revision 1.21  2003/07/12 01:59:44  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.20  2003/07/09 22:19:16  doug
// The robot now exits cleanly
//
// Revision 1.19  2003/06/17 20:42:36  doug
// adding debug to track down broadcast problem
//
// Revision 1.18  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.17  2003/04/11 18:46:55  doug
// added export capabilities
//
// Revision 1.16  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.15  2003/02/19 22:15:15  doug
// Change waitfor to use the new TimeOfDay
//
// Revision 1.14  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.13  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.12  2003/02/10 22:39:54  doug
// tested out well
//
// Revision 1.11  2003/02/06 20:29:30  doug
// trying to get the packet loss down
//
// Revision 1.10  2003/02/04 21:51:34  doug
// It sends and receives packets now
//
// Revision 1.9  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.8  2003/01/31 15:02:24  doug
// It compiles again
//
// Revision 1.7  2003/01/31 02:08:22  doug
// snapshot
//
// Revision 1.6  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.5  2003/01/24 22:40:03  doug
// getting closer
//
// Revision 1.4  2003/01/23 22:39:12  doug
// making progress
//
// Revision 1.3  2003/01/22 22:09:54  doug
// snapshot
//
// Revision 1.2  2003/01/20 22:03:05  doug
// snapshot
//
// Revision 1.1.1.1  2003/01/20 20:43:41  doug
// initial import
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "TimeOfDay.h"
#include "commMsg.h"
#include "ComAddr.h"
//#include "Receipt.h"

namespace sara
{
/**********************************************************************/
class objcomms 
{
public:
   /// Create a communications object 
   objcomms();

   /// Destroy the communications object
   /// Closes the communications stream and invalidates any handles
   ~objcomms();

   /// Set the debug flags with the current values
   /// OR together the various bit flags to enable debug
   typedef enum {DEBUG_NONE=0, DEBUG_GEN=1<<0, DEBUG_TRNS=1<<1, DEBUG_RECV=1<<2, DEBUG_RECV_DETAILS=1<<3, DEBUG_RAW=1<<4} FLAGVALUES;
   FLAGVALUES debug;
   void setDebug(const uint flags) {debug = (FLAGVALUES)flags;}

   /// load a set of addresses for the team members
   /// Loading these values Will cause multicasts to use multiple unitcast packets!
   /// The address format is ip:port, for example: "192.168.1.3:4321"
   bool loadMemberAddresses(const strings &addresses);

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
   bool open(const string &multicastAddress,
             const uint    port,
	     const string  procName = "",
	     const bool    appendInstance = false,
             const uint    maxInstancesPerHost = 1,
             uint         *instanceNumber = NULL);

   /// Wait for the transmit queue to drain out, or the timeout
   void waitForTransmitter(const TimeOfDay &timeout);

   /// Get our unique network name
   string getName() const;

   /// Send the message to all listeners
   bool broadcastMsg(const commMsg &message);

   /// Reliably send the message to the destination
   /// Returns a Receipt object which can be used to check the status of the send
//   Receipt reliablySendMsg(const ComAddr &dest, const commMsg &message);

   /// Send the message to the destination
   bool sendMsg(const ComAddr &dest, const commMsg &message);

   /// Define the interface for message builders that build 
   /// messages from a stringstream
   /// Returns the message object or NULL if 'msgData' is not valid.
   typedef commMsg *(*messageBuilder_T)(stringstream *str);

   /// Register a message builder for a message type
   /// returns: true on success
   ///          false if another builder already registered
   bool attachHandler(commMsg::MsgType_T msgType, messageBuilder_T builder);

   /// Define the interface for message callbacks
   /// Note: The callback must copy the message if it wants it.
   /// It will be deleted after it returns
   typedef void *USERDATA;
   typedef void (*message_callback_t)(commMsg *msg, USERDATA userData);

   /// Define the subscription handle.
   typedef int SubscriptionHandle;

   /// Subscribe to get copies of messages meeting the criteria
   /// Returns: a subscription handle used to cancel the subscription
   SubscriptionHandle subscribeTo(commMsg::MsgType_T msgType, message_callback_t callback, USERDATA userData);

   /// Cancel a previous subscription
   bool cancelSubscription(SubscriptionHandle handle);

   /// Begin queueing messages for us of the specified type
   /// msgType is the type of message to wait for
   /// Returns a queueing handle on success, NULL on failure.
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   typedef void *QueueingHandle;
   QueueingHandle beginQueueing(commMsg::MsgType_T msgType);

   /// Suspend the thread until either a message arrives or the timeout expires
   /// handle was returned from a call to beginQueueing
   /// timeout is the maximum time to wait.
   /// returns: The message, if one arrived
   ///          NULL if the timeout expired or the handle is invalid
   commMsg *waitFor(QueueingHandle handle, const TimeOfDay &timeout);

   /// Stop queueing messages for us 
   /// Returns true on success, false if the handle is invalid
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   bool endQueueing(QueueingHandle handle);

   /// enable message compression
   /// Returns true on success, false if compression is not supported.
   bool enableCompression();

   /// disable message compression
   /// Returns true on success, false if compression was not enabled.
   bool disableCompression();

   /// enable message encription
   /// Returns true on success, false if encryption is not supported.
   bool enableEncryption(const string &encryptKey);

   /// disable message encription
   /// Returns true on success, false if encryption was not enabled.
   bool disableEncryption();

   /// change the key used for message encription
   /// Returns true on success, false if the key is invalid
   bool changeEncryptionKey(const string &encryptKey);

   /// debug function to print the com link stats
   friend ostream &operator << (ostream & out, const objcomms &obj);

   /// Number of bytes sent/received over the wire
   ulong bytesTrn;
   ulong bytesRcv;

   /// Number of packets sent/received
   ulong packetsTrn;
   ulong packetsRcv;

   /// Number of messages sent/received
   ulong messagesTrn;
   ulong messagesRcv;

   /// Number of packets discarded due to transmit/receive buffer overruns
   ulong packetsTrnDiscard;
   ulong packetsRcvDiscard;

   /// Number of packets received corrupted
   ulong packetsCorrupted;

   /// for testing endian'ness
   static ulong MIC_BYTE_ORDER;
   static ulong MIC_BIG_ENDIAN;
   static ulong MIC_LITTLE_ENDIAN;

private:

   /// Disable copy constructors
   objcomms(const objcomms&);
   objcomms& operator= (const objcomms&);

   /// The comm_stream object that actually does the work.
   static class objcomms_implementation *cs;
};

/*********************************************************************/
}
#endif
