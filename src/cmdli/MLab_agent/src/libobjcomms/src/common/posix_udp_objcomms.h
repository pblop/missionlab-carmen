#ifndef posix_udp_objCOMMS_H
   #define posix_udp_objCOMMS_H
/**********************************************************************
 **                                                                  **
 **  posix_udp_objcomms.h                                            **
 **                                                                  **
 **  implement the objcomms for posix systems using udp              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: posix_udp_objcomms.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_udp_objcomms.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/02 17:01:33  endo
// Compiler error fixed.
//
// Revision 1.1  2006/07/01 00:31:55  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.41  2004/11/06 14:13:06  doug
// Change not being able to unicast to a non fatal error so that multicast-only uses can operate multiple instances on a single host
//
// Revision 1.40  2004/11/06 01:11:56  doug
// snap
//
// Revision 1.39  2004/09/21 13:52:05  doug
// added support for robotname@ipaddress:port to addresslist.  This allows easily having a single resource file to run multiple robots on a single computer for testing
//
// Revision 1.38  2004/08/12 16:28:53  doug
// change open interface to add a parameter to append instance number or not
//
// Revision 1.37  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.36  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.35  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.34  2004/04/12 21:20:21  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.33  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.32  2004/03/31 14:15:06  doug
// standardize the objcomms interface methods
//
// Revision 1.31  2004/03/26 21:25:08  doug
// finally compiles under win32
//
// Revision 1.30  2004/03/26 00:18:58  doug
// working on win32 port
//
// Revision 1.29  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.28  2004/03/01 00:47:10  doug
// cmdli runs in sara
//
// Revision 1.27  2003/07/12 01:59:44  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.26  2003/07/08 20:49:05  doug
// Rework to get controller to exit cleanly on terminate
//
// Revision 1.25  2003/07/03 16:32:23  doug
// Fixed a nasty race condition where we would read the header on a UDP packet and the packet would get overwritten before we went back and read the body, leaving the header and the body not matching.
//
// Revision 1.24  2003/04/28 22:00:38  doug
// switched to a pipe to wake up the select, but still is blocking
//
// Revision 1.23  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.22  2003/04/25 22:13:11  doug
// fixed handling of dropped tcp connections and changed to use non-padding encryptBuffer calls on the packet header.
//
// Revision 1.21  2003/04/24 20:56:51  doug
// still coding on tcp support
//
// Revision 1.20  2003/04/21 17:43:27  doug
// working on adding tcp support to objcomms to handle large data streams point to point
//
// Revision 1.19  2003/04/17 20:33:05  doug
// Adding support for tcp connections
//
// Revision 1.18  2003/04/11 18:46:55  doug
// added export capabilities
//
// Revision 1.17  2003/02/19 22:15:15  doug
// Change waitfor to use the new TimeOfDay
//
// Revision 1.16  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.15  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.14  2003/02/06 20:29:30  doug
// trying to get the packet loss down
//
// Revision 1.13  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.12  2003/01/31 22:36:48  doug
// working a bit
//
// Revision 1.11  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.10  2003/01/31 02:08:22  doug
// snapshot
//
// Revision 1.9  2003/01/31 00:47:26  doug
// compiles!
//
// Revision 1.8  2003/01/29 22:19:56  doug
// snapshot
//
// Revision 1.7  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.6  2003/01/27 23:35:35  doug
// snapshot
//
// Revision 1.5  2003/01/24 22:40:03  doug
// getting closer
//
// Revision 1.4  2003/01/22 22:09:54  doug
// snapshot
//
// Revision 1.3  2003/01/22 16:16:02  doug
// snapshot
//
// Revision 1.2  2003/01/20 22:08:36  doug
// the framework builds
//
// Revision 1.1.1.1  2003/01/20 20:43:41  doug
// initial import
//
///////////////////////////////////////////////////////////////////////

#include <deque>
#include <list>
#include "mic.h"
#include "objcomms.h"
#include "objcomms_implementation.h"
#include "objthreads.h"
#include "mutex.h"

#include "PacketHeader.h"

namespace sara
{
// *********************************************************************
class posix_udp_objcomms : public objcomms_implementation
{
public:
   // Open the communications port (UDP)
   posix_udp_objcomms(objcomms *base);

   // close and delete the communications port
   ~posix_udp_objcomms();

   /// is the object already open?
   bool isOpen() const {return opened;}

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
	     const string &procName,
	     const bool    appendInstance,
             const uint    maxInstancesPerHost,
             uint         *instanceNumber);

   /// load a set of addresses for the team members
   /// Loading these values Will cause multicasts to use multiple unitcast packets!
   /// The address format is ip:port, for example: "192.168.1.3:4321"
   bool loadMemberAddresses(const strings &addresses);

   /// Wait for the transmit queue to drain out, or the timeout
   void waitForTransmitter(const TimeOfDay &timeout);

   /// Get our unique network name
   string getName() const;

   /// Get our unique network address
   ComAddr getAddr() const;

   /// Send the message reliabily to the destination
   bool sendMsg(const ComAddr &dest, const commMsg &message);

   /// Broadcast the message to all listeners
   bool broadcastMsg(const commMsg &message);

   /// Register a marshalling & unmarshalling handler for a message type
   /// returns: true on success
   ///          false if another handler already registered
   bool attachHandler(commMsg::MsgType_T msgType, objcomms::messageBuilder_T builder);

   /// Subscribe to get copies of messages meeting the criteria
   /// Returns: a subscription handle used to cancel the subscription
   objcomms::SubscriptionHandle subscribeTo(commMsg::MsgType_T msgType, objcomms::message_callback_t callback, objcomms::USERDATA userData);

   /// Cancel a previous subscription
   bool cancelSubscription(objcomms::SubscriptionHandle handle);

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

   /// Begin queueing messages for us of the specified type
   /// msgType is the type of message to wait for
   /// Returns a queueing handle on success, NULL on failure.
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   objcomms::QueueingHandle beginQueueing(commMsg::MsgType_T msgType);

   /// Suspend the thread until either a message arrives or the timeout expires
   /// handle was returned from a call to beginQueueing
   /// timeout is the maximum time to wait
   /// returns: The message, if one arrived
   ///          NULL if the timeout expired or the handle is invalid
   commMsg *waitFor(objcomms::QueueingHandle handle, const TimeOfDay &timeout);

   /// Stop queueing messages for us 
   /// Returns true on success, false if the handle is invalid
   /// NOTE: endQueueing must be called after a successful call to 
   /// beginQueueing to stop the queueing process and free consumed resources
   bool endQueueing(objcomms::QueueingHandle handle);

private:
   /// Internal wrapper function around setsockopt to hide ifdefs
   /// returns true on success, false on failure and errno is set.
#ifdef WIN32
   typedef int socklen_t;
#endif
   bool setSocketOption(int s, int level, int optname, const void *optval,socklen_t optlen) const;
/*
   /// Allocate and connect the broadcast socket using UDP
   /// Returns true on success, false on failure.
   bool open(const string &interfaceName,
             const uint    interfaceIndex,
             const string &multicastAddress,
             const uint    port,
	     const string &baseName,
             const uint    maxInstancesPerHost,
             uint         *instanceNumber);
*/
   /// prepare the message, potentually compressing and/or encrypting it
   /// return a pointer to a new stringstream containing the output.
   /// The user is responsible for deleting the returned stringstream.
   stringstream * prepareMsg(stringstream *in, bool &isCompressed);

   /// Pointer to our base record.  Used to access the Run function
   objcomms *parent;

   /// output queue and protection used by the transmit thread
   mutex     outputData_guard;
   condition outputData_changed;
   /// Describe messages on the output queue
   struct outputMsgInfo
   {
      stringstream *dataStream;

      // if isCompressed, then dataStream was compressed
      bool isCompressed;

      // if isDirected, then dest holds the destination address.
      bool isDirected;
      ComAddr dest;
   };
   typedef deque<outputMsgInfo *> outputMsgQueue;
   outputMsgQueue  outputData;
   /// Maximum number of messages we allow to be queued
   static const uint outputData_MAXSIZE = 100;

   /// input queue and protection used by the receiver thread
   mutex     inputData_guard;
   condition inputData_changed;

   /// Describe messages on the input queue
   class inputMsgInfo
   {
      public:
         // constructor
         inputMsgInfo(): 
	    dataStream(NULL) 
         {};

         // destructor
         ~inputMsgInfo() 
         {
            if (dataStream) 
	       delete dataStream;
         }

         stringstream *dataStream;

         // if isCompressed, then dataStream is compressed
         bool isCompressed;
         string senderID;
         ComAddr senderAddr;
   };
   typedef deque<inputMsgInfo *> inputMsgQueue;
   inputMsgQueue  inputData;
   /// Maximum number of messages we allow to be queued
   static const uint inputData_MAXSIZE = 100;

/*
   /// Track open tcp connections
   struct openConnection_T
   {
      int     socket;
      ComAddr dest;
   };
   typedef map<ComAddr, openConnection_T, less<ComAddr> > ConnectionContainer;
   ConnectionContainer openConnections;
*/

   /// Define the port number that the communications will utilize.
   /// If num_instances_per_host > 1, then uses multiple ports starting
   /// with this one.
   uint base_port;

   /// Define the broadcast IP address.
   string BROADCAST_IP;

#if defined(linux)
#define SOCKET int
#endif

   // We open three sockets for receiving communications on the designated port:
   //    udpsock  -  sends all packets
   //                if udpsock_isbound, then receives standard unicast UDP packets
   //    mulsock  -  receives multicast packets
   //    tcpsock  -  accepts tcp connections on the port
   SOCKET udpsock;
   bool   udpsock_isBound;
   SOCKET mulsock;
   SOCKET tcpsock;

   /// the transmit/receiver paired socket file descriptors
   SOCKET pipe_input;
   SOCKET pipe_output;

   /// loaded in open for its local use and also used later in transmit thread.
   sockaddr_in multicast_addr;

   /// Track open tcp connections
   class conInfo
   {
   public:
      conInfo() 
      {
         addr_len = sizeof(addr);
      }
      SOCKET sd;
      struct sockaddr_in addr;
      int addr_len;
      ComAddr dest;
   };
   typedef map<ComAddr, conInfo *, less<ComAddr> > conInfoContainer;
   conInfoContainer cons;
   mutex cons_mutex;

   /// The number of instances per host we allow
   uint num_instances_per_host;

   /// Our instance number on this host
   uint instance;

   /// The name we operate under on the network.
   string ourNetworkName;

   /// our network address
   ComAddr ourNetworkAddress;

   /// define a map to hold the addresses of the message handlers, indexed
   /// by the message type (MsgType).
   typedef map<commMsg::MsgType_T, objcomms::messageBuilder_T, less<commMsg::MsgType_T> > messageBuilders_T;
   messageBuilders_T messageBuilders;

   /// transmitter thread
   class transmitter: public objthreads 
   {
   public:
      transmitter(posix_udp_objcomms *parent);
      void run(int);
      void shutdown();

   private:
      posix_udp_objcomms *parent;
      bool exiting;
   };

   /// receiver thread
   class receiver : public objthreads 
   {
   public:
      /// constructor
      receiver(posix_udp_objcomms *parent);

      /// Handle a newly arrived packet 
      /// sd - the socket descriptor to read the packet from
      /// isStream - true if transport queue messages (tcp), 
      ///            false if next message replaces this one (udp)
      // ENDO - FC5
      //bool receiver::handle_packet(int sd, const bool isStream);
      bool handle_packet(int sd, const bool isStream);

      /// Main thread for the receiver
      void run(int);
      void shutdown();

   private:
      posix_udp_objcomms *parent;
      bool exiting;

      // the encryption object
      class encryption *encryptor;

      // Define a type for our receiver packet container
      struct Packet
      {
         // constructor
         Packet() : dataStream(NULL) {};

         // destructor
         ~Packet() {delete dataStream; dataStream=NULL;};

         // The header on the packet
         PacketHeader header;

         // The data
         stringstream *dataStream;
      };

      // Save packets we are assembling into a message
      struct StorageEntry
      {
         // The sender's sequence number for this message
         byte msgNumber;

         // The total number of packets in this message
         byte totalPackets;

         // The number we have received so far
         byte receivedPackets;

         // The packets
         vector<Packet *> packets;
      };

      // Define our packet storage area, where we save packets while 
      // assembling the complete message.
      typedef map<string, StorageEntry *, less<string> > Storage_T;

      // A buffer to hold the packets pending re-assembly
      Storage_T storage;
   };

   /// thread to disperse incoming messages
   class disperser : public objthreads 
   {
   public:
      disperser(posix_udp_objcomms *parent);
      void run(int);
      void shutdown();

   private:
      posix_udp_objcomms *parent;
      bool exiting;
   };
   /// subscriber list and protection used by the disperser thread
   mutex     subscriberList_guard;
   typedef pair<objcomms::message_callback_t, objcomms::USERDATA> SubEntry;
   typedef list<SubEntry> SubscriberList_T;
   typedef map<commMsg::MsgType_T, SubscriberList_T> Subscribers_T;
   Subscribers_T  subscribers;


   /// wait list and protection used by the disperser thread
   class waitListEntry
   {
   public:
      // waitList_guard is used to protect this condition
      condition signaled;

      // The type of messages requested
      commMsg::MsgType_T msgType;

      // waiting process is responsible for freeing the commMsg
      deque<commMsg *> queue;
   };
   // Used to guard modifications to waitList and signals/waits on signaled
   mutex waitList_guard;
   typedef list<waitListEntry *> waitList_T;
   typedef map<commMsg::MsgType_T, waitList_T> Waiters_T;
   Waiters_T  waitList;



   /// Remember our instances of the threads
   transmitter *trans;
   receiver    *rec;
   disperser *dsp;

   /// A compressor object, loaded if compression is supported
   class compress *compressor;

   /// Should we compress our outbound data?
   bool compressData;

   /// should we encrypt the data?
   bool encryptData;

   /// The encryption key
   string encryptionKey;

   /// Set when we are done
   bool exiting;

   /// if true, multicasts are sent as a series of unicasts instead.
   bool multicastIsUnicast;

   /// if false, we don't try to receive packets on the multicast interface
   bool receive_multicast;

   /// the addresses of all the team members, only used if multicastIsUnicast is true
   class AddrEntry
   {
      public:
	 AddrEntry(const string &n, const ComAddr &a) : name(n), address(a) {};
	 AddrEntry(const ComAddr &a) : address(a) {};
      string  name;
      ComAddr address;
   };
   typedef vector< AddrEntry > addrList;
   addrList teamAddrs;

   bool opened;
};

/*********************************************************************/
}
#endif
