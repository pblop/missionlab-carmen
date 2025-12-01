#ifndef lowlevelcoms_H
   #define lowlevelcoms_H
/**********************************************************************
 **                                                                  **
 **  lowlevelcoms.h                                                  **
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

/* $Id: lowlevelcoms.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: lowlevelcoms.h,v $
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
// Revision 1.2  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2004/03/24 22:15:37  doug
// working on win32 port
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
class lowlevelcoms : public objcomms_implementation
{
public:
   // Open the communications port (UDP)
   lowlevelcoms(objcomms *base);

   // close and delete the communications port
   ~lowlevelcoms();

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
             uint *instance_number);

   /// Wait for the transmit queue to drain out, or the timeout
   void waitForTransmitter(const TimeOfDay &timeout);

   /// Get our unique network name
   string getName() const;

   /// Send the message reliabily to the destination
   bool sendMsg(const ComAddr &dest, commMsg *message);

   /// Broadcast the message to all listeners
   bool broadcastMsg(const commMsg *message);

   /// Send the message to the list of destinations
   bool multicastMsg(const strings &dests, commMsg *message);

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
   struct inputMsgInfo
   {
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

   /// The UDP broadcast socket file descriptor.  Init to not open.
   int udsock;

   /// The TCP socket file descriptor.  Init to not open.
   int tdsock;

   /// the transmit/receiver pipe file descriptors
   int tpsock;
   int rpsock;

   /// Track open tcp connections
   class conInfo
   {
   public:
      conInfo() 
      {
         addr_len = sizeof(addr);
      }
      int sd;
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

   /// The broadcast address for local network broadcasting.
   sockaddr_in broadcast_addr;

   /// The name we operate under on the network.
   string ourNetworkName;

   /// define a map to hold the addresses of the message handlers, indexed
   /// by the message type (MsgType).
   typedef map<commMsg::MsgType_T, objcomms::messageBuilder_T, less<commMsg::MsgType_T> > messageBuilders_T;
   messageBuilders_T messageBuilders;

   /// transmitter thread
   class transmitter: public objthreads 
   {
   public:
      transmitter(lowlevelcoms *parent);
      void run(int);
      void shutdown();

   private:
      lowlevelcoms *parent;
      bool exiting;
   };

   /// receiver thread
   class receiver : public objthreads 
   {
   public:
      /// constructor
      receiver(lowlevelcoms *parent);

      /// Handle a newly arrived packet 
      /// sd - the socket descriptor to read the packet from
      /// addr - the sender's address to note in the message
      /// isStream - true if transport queue messages (tcp), 
      ///            false if next message replaces this one (udp)
      bool receiver::handle_packet(int sd, const ComAddr &addr, const bool isStream);

      /// Main thread for the receiver
      void run(int);
      void shutdown();

   private:
      lowlevelcoms *parent;
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
      disperser(lowlevelcoms *parent);
      void run(int);
      void shutdown();

   private:
      lowlevelcoms *parent;
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
};

/*********************************************************************/
}
#endif
