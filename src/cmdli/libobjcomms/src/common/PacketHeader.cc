/**********************************************************************
 **                                                                  **
 **  PacketHeader.cc                                                 **
 **                                                                  **
 **  Read and write a packet header from a byte array                **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: PacketHeader.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: PacketHeader.cc,v $
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
// Revision 1.1  2004/11/08 19:36:51  doug
// redid header read/write so packs fields, instead of just copying over the header, which is 4 bytes per field, even for a char.
//
///////////////////////////////////////////////////////////////////////

#include "commMsg.h"
#include "PacketHeader.h"

namespace sara
{
// ********************************************************************
/// Helper function to write the packetheader
/// All data must already be in network byte order!
bool 
writePacketHeader(char *buffer, PacketHeader *header)
{
   char *p = buffer;

   // senderID
   memcpy(p, header->senderID, PacketHeader::MAXIDSIZE);
   p += PacketHeader::MAXIDSIZE;

   *(ulong *)p = header->IPaddress;
   p += 4;
   *(ushort *)p = header->port;
   p += 2;

   /// The number of data bytes following this header in this message, in network byte order!
   *(ulong *)p = header->dataSize;
   p += 4;
   
   /// A sequence number assigned to the message containing this packet
   *(byte *)p = header->msgNumber;
   p += 1;

   /// The index of this packet in the message, 0-255
   *(byte *)p = header->packetIndex;
   p += 1;

   /// The total number of packets in the message, 1-255
   *(byte *)p = header->totalPackets;
   p += 1;

   /// Is the data compressed? (Only valid on the header for packet 0)
   *(byte *)p = header->isCompressed ? 1 : 0;
   p += 1;

   return true;
}

// ********************************************************************
/// Helper function to read the packetheader
bool 
readPacketHeader(char *buffer, PacketHeader *header)
{
   char *p = buffer;

   // senderID
   memcpy(header->senderID, p, PacketHeader::MAXIDSIZE);
   p += PacketHeader::MAXIDSIZE;

   header->IPaddress = *(ulong *)p;
   p += 4;
   header->port = *(ushort *)p;
   p += 2;

   /// The number of data bytes following this header in this message, in network byte order!
   header->dataSize = *(ulong *)p;
   p += 4;
   
   /// A sequence number assigned to the message containing this packet
   header->msgNumber = *(byte *)p;
   p += 1;

   /// The index of this packet in the message, 0-255
   header->packetIndex = *(byte *)p;
   p += 1;

   /// The total number of packets in the message, 1-255
   header->totalPackets = *(byte *)p;
   p += 1;

   /// Is the data compressed? (Only valid on the header for packet 0)
   header->isCompressed = *(byte *)p != 0;
   p += 1;

   return true;
}

// ********************************************************************
} // end namespace sara
