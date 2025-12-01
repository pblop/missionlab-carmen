#ifndef comm_stream_H
#define comm_stream_H
/**********************************************************************
 **                                                                  **
 **  comm_stream.h                                                   **
 **                                                                  **
 **  base file which users subclass to get object comms              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: comm_stream.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: comm_stream.h,v $
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
// Revision 1.3  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.2  2003/01/23 22:39:12  doug
// making progress
//
// Revision 1.1  2003/01/22 22:09:54  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "objthreads.h"

namespace sara
{
typedef enum commstate {DISCONNECTED, CONNECTED} commstate;

/**********************************************************************/
class comm_stream 
{
public:
   /// Create a communications object 
   comm_stream();

   /// Destroy the communications object
   ~comm_stream();

   /// open the communications port 
   /// Returns true on success, false on failure.
   /// max_instances_per_host : The maximum number of copies of the 
   ///                          program running on any one host.
   ///                          (NOTE: Keep as small as practical)
   /// IP_address:              Broadcast address (e.g., 192.168.0.255)
   /// port:                    Base port.  Several ports may be used,
   ///                          [port ... port+max_instances_per_host-1]
   /// basename:                A unique network name for the process.
   ///                          If more than one instance, will append
   ///                          the instance number to make unique.
   ///                          If empty, will use hostname.
   /// instance_number:         If not NULL, the instance number of 
   ///                          this process on this host is returned.
   bool open(const uint   max_instances_per_host,
             const string IP_address,
             const uint   port,
	     const string &basename,
             uint *instance_number = NULL) = 0;

   /// Get our unique network name
   string getName() const = 0;

   /// Send the message to the destination
   bool sendMsg(const string &dest, commMsg message) = 0;

   /// Broadcast the message to all listeners
   bool broadcastMsg(commMsg message) = 0;

   /// Send the message to the list of destinations
   bool multicastMsg(const strings &dests, commMsg message) = 0;

   /// Register a marshalling & unmarshalling handler for a message type
   /// returns: true on success
   ///          false if another handler already registered
   bool attachHandler(commMsg::MsgType msgType, message_handler_t handler) = 0;

   /// Subscribe to get copies of messages meeting the criteria
   /// Returns: a subscription handle used to cancel the subscription
   SubscriptionHandle subscribeTo(commMsg::MsgType msgType, message_callback_t callback) = 0;

private:
   /// the object actually doing the work.
   class comm_stream_implementation *impl;

   /// Disable copy constructors
   comm_stream(const comm_stream&);
   comm_stream& operator= (const comm_stream&);
};

/*********************************************************************/
}
#endif
