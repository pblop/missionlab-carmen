#ifndef objcomms_implementation_H
#define objcomms_implementation_H
/**********************************************************************
 **                                                                  **
 **  objcomms_implementation.h                                       **
 **                                                                  **
 **  abstract base class for objcomms implementations                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: objcomms_implementation.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: objcomms_implementation.h,v $
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
// Revision 1.19  2004/08/12 16:28:53  doug
// change open interface to add a parameter to append instance number or not
//
// Revision 1.18  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.17  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.16  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.15  2004/04/12 21:20:21  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.14  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.13  2004/03/31 14:15:06  doug
// standardize the objcomms interface methods
//
// Revision 1.12  2004/03/01 00:47:10  doug
// cmdli runs in sara
//
// Revision 1.11  2003/07/12 01:59:44  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.10  2003/04/11 18:46:55  doug
// added export capabilities
//
// Revision 1.9  2003/02/19 22:15:15  doug
// Change waitfor to use the new TimeOfDay
//
// Revision 1.8  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.7  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.6  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.5  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.4  2003/01/24 22:40:03  doug
// getting closer
//
// Revision 1.3  2003/01/23 22:39:12  doug
// making progress
//
// Revision 1.1  2003/01/22 22:09:54  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "objcomms.h"

namespace sara
{
class objcomms_implementation 
{
   public:
    // Create the comm object
    static objcomms_implementation *create(objcomms *root);

    // No extra work to do here
    virtual ~objcomms_implementation() {};

   /// is the object already open?
   virtual bool isOpen() const = 0;

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
   virtual bool open(const string &multicastAddress,
             const uint    port,
	     const string &procName,
	     const bool    appendInstance,
             const uint    maxInstancesPerHost,
             uint         *instanceNumber) = 0;

   /// Wait for the transmit queue to drain out, or the timeout
   virtual void waitForTransmitter(const TimeOfDay &timeout) = 0;

   /// load a set of addresses for the team members
   /// Loading these values Will cause multicasts to use multiple unitcast packets!
   /// The address format is ip:port, for example: "192.168.1.3:4321"
   virtual bool loadMemberAddresses(const strings &addresses) = 0;

   /// Get our unique network name
   virtual string getName() const = 0;

   /// Send the message reliabily to the destination
   virtual bool sendMsg(const ComAddr &dest, const commMsg &message) = 0;

   /// Broadcast the message to all listeners
   virtual bool broadcastMsg(const commMsg &message) = 0;

   /// Register a marshalling & unmarshalling handler for a message type
   /// returns: true on success
   ///          false if another handler already registered
   virtual bool attachHandler(commMsg::MsgType_T msgType, objcomms::messageBuilder_T builder) = 0;

   /// Subscribe to get copies of messages meeting the criteria
   /// Returns: a subscription handle used to cancel the subscription
   virtual objcomms::SubscriptionHandle subscribeTo(commMsg::MsgType_T msgType, objcomms::message_callback_t callback, objcomms::USERDATA userData) = 0;

   /// Cancel a previous subscription
   virtual bool cancelSubscription(objcomms::SubscriptionHandle handle) = 0;

   /// enable message compression
   /// Returns true on success, false if compression is not supported.
   virtual bool enableCompression() = 0;

   /// disable message compression
   /// Returns true on success, false if compression was not enabled.
   virtual bool disableCompression() = 0;

   /// enable message encription
   /// Returns true on success, false if encryption is not supported.
   virtual bool enableEncryption(const string &encryptKey) = 0;

   /// disable message encription
   /// Returns true on success, false if encryption was not enabled.
   virtual bool disableEncryption() = 0;

   /// change the key used for message encription
   /// Returns true on success, false if the key is invalid
   virtual bool changeEncryptionKey(const string &encryptKey) = 0;

   /// Begin queueing messages for us of the specified type
   /// msgType is the type of message to wait for
   /// Returns a queueing handle on success, NULL on failure.
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   virtual objcomms::QueueingHandle beginQueueing(commMsg::MsgType_T msgType) = 0;

   /// Suspend the thread until either a message arrives or the timeout expires
   /// handle was returned from a call to beginQueueing
   /// timeout is the maximum time to wait.
   /// returns: The message, if one arrived
   ///          NULL if the timeout expired or the handle is invalid
   virtual commMsg *waitFor(objcomms::QueueingHandle handle, const TimeOfDay &timeout) = 0;

   /// Stop queueing messages for us 
   /// Returns true on success, false if the handle is invalid
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   virtual bool endQueueing(objcomms::QueueingHandle handle) = 0;

   private:
      friend class posix_udp_objcomms;
      friend class win32_objcomms;
      friend class djgpp_objcomms;
		   
      // Only allow the friends access to the constructor
      objcomms_implementation() {};
};

/*********************************************************************/
}
#endif
