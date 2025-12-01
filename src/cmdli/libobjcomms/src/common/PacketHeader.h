#ifndef PacketHeader_H
#define PacketHeader_H
/**********************************************************************
 **                                                                  **
 **  PacketHeader.h                                                  **
 **                                                                  **
 **  The data structure defining the raw packet header               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: PacketHeader.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: PacketHeader.h,v $
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
// Revision 1.11  2004/11/08 19:36:51  doug
// redid header read/write so packs fields, instead of just copying over the header, which is 4 bytes per field, even for a char.
//
// Revision 1.10  2004/08/20 21:45:39  doug
// working on bud_monitor
//
// Revision 1.9  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.7  2003/04/24 20:56:51  doug
// still coding on tcp support
//
// Revision 1.6  2003/04/23 20:57:13  doug
// still working on tcp support
//
// Revision 1.5  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.4  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.3  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.2  2003/01/31 00:47:26  doug
// compiles!
//
// Revision 1.1  2003/01/29 22:19:56  doug
// snapshot
//
///////////////////////////////////////////////////////////////////////

#if defined(__cplusplus) || defined(c_plusplus)
#include "mic.h"

namespace sara
{
#else
typedef unsigned char  byte;
typedef unsigned short uword;
typedef unsigned char  bool;

/// Define the #word# data type: signed, 16 bits
#endif
// *******************************************************************
// Base class for user communications messages
// The largest size packet a host is required to process is 576 bytes,
// and the largest possible UDP header is 60 bytes, so the maximum size
// of the Packet structure should be limited to 516 bytes.
// We restrict it to 512 to simplify encryption which uses 8 byte blocks

// DCM: I have switched to 1024 packets.
struct PacketHeader
{
   /// The size of the senderID in bytes
#if defined(__cplusplus) || defined(c_plusplus)
   enum {MAXIDSIZE = 8};
#else
#define MAXIDSIZE (8)
#endif

   /// The (hopefully) unique name we operate under on the network
   /// NOTE: IT IS NOT NULL TERMINATED!
   char senderID[MAXIDSIZE];

   /// The sender's IP:port address in network byte order!
   ulong IPaddress;
   uword port;
   
   /// The number of data bytes following this header in this message, in network byte order!
   ulong dataSize;

   /// A sequence number assigned to the message containing this packet
   byte msgNumber; 

   /// The index of this packet in the message, 0-255
   byte packetIndex; 

   /// The total number of packets in the message, 1-255
   byte totalPackets; 

   /// Is the data compressed? (Only valid on the header for packet 0)
   bool isCompressed;

   /// The size of the header is a multiple of 8 bytes so it can be encrypted
#if defined(__cplusplus) || defined(c_plusplus)
   enum {HEADERSIZE = 24};
#else
#define HEADERSIZE (24)
#endif


   /// The size of padding in bytes
#if defined(__cplusplus) || defined(c_plusplus)
   enum {PADSIZE = HEADERSIZE - (MAXIDSIZE + 4 + 2 + 4 + 1 + 1 + 1 + 1)};
#else
#define PADSIZE (HEADERSIZE - (MAXIDSIZE + 4 + 2 + 4 + 1 + 1 + 1 + 1))
#endif

// This doesn't seem to work.  I guess enums are unsigned.
//#if( PADSIZE < 0 )
//#error "NEGATIVE PADSIZE!! INCREASE HEADERSIZE IN PacketHeader.h"
//#elif( PADSIZE > 0 )
#if( PADSIZE > 0 )
   byte padding[PADSIZE];
#endif

   /// The maximum allowed size of the data
//   enum {MAXDATASIZE = 512 - HEADERSIZE};
#if defined(__cplusplus) || defined(c_plusplus)
   enum {MAXDATASIZE = 1024 - HEADERSIZE};
#else
#define MAXDATASIZE (1024 - HEADERSIZE)
#endif
};

/// Helper function to read the packetheader
bool readPacketHeader(char *buffer, PacketHeader *header);

/// Helper function to write the packetheader
/// All data must already be in network byte order!
bool writePacketHeader(char *buffer, PacketHeader *header);

#if defined(__cplusplus) || defined(c_plusplus)
/*********************************************************************/
}
#endif
#endif
