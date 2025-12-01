#ifndef ComAddr_H
#define ComAddr_H
/**********************************************************************
 **                                                                  **
 **  ComAddr.h                                                       **
 **                                                                  **
 **  Represent the network address for an instance                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: ComAddr.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: ComAddr.h,v $
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
// Revision 1.1  2006/07/01 00:31:54  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.13  2004/08/12 16:09:10  doug
// add new methods
//
// Revision 1.12  2004/07/30 13:42:40  doug
// fix selection of interface
//
// Revision 1.11  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.10  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.9  2004/04/12 21:20:20  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.8  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.7  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.6  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.5  2003/04/24 20:56:51  doug
// still coding on tcp support
//
// Revision 1.4  2003/04/23 20:57:13  doug
// still working on tcp support
//
// Revision 1.3  2003/04/22 23:51:46  doug
// fixed compile errors
//
// Revision 1.2  2003/04/21 17:43:27  doug
// working on adding tcp support to objcomms to handle large data streams point to point
//
// Revision 1.1  2003/04/11 18:49:27  doug
// adding tcp capabilities
//
///////////////////////////////////////////////////////////////////////

#if defined(linux)
#include <netinet/in.h>
#elif defined(WIN32)
#include <WinSock2.h>
#endif
#include "mic.h"

namespace sara
{
// *******************************************************************
// Represent the network address for an instance
class ComAddr
{
public:
   /// Constructors
   ComAddr();
   ComAddr(const ComAddr &old);

   /// The address is specified in dotted quad notation (e.g., "192.168.1.1")
   /// The port number is in normal, local host byte order.
   ComAddr(const string &addr, uint port);

   /// Create from a sockaddr_in struct
   ComAddr(const sockaddr_in &address);

   /// Create from raw data in network byte order
   ComAddr(uint IPaddr, uword port);

   /// The port number defaults to 0
   ComAddr(const string &addr);
   ComAddr(const in_addr &address);

   /// Destructor
   ~ComAddr() {};

   /// assignment operator
   // ENDO - FC5
   //ComAddr &ComAddr::operator= (const ComAddr &old);
   ComAddr& operator= (const ComAddr &old);

   /// Return as a string
   string str() const;

   /// Load from an address record
   /// returns true on success, false on invalid address.
   bool load(struct sockaddr_in *addr, int addr_len);

   /// Load from a string (either a name or standard dotted notation)
   /// returns true on success, false on name not found
   bool load(const string &address, const int port);

   /// Load from a string (either a name or standard dotted notation)
   /// followed by a colon and the port number.  Ex: "192.168.0.0:5321"
   /// returns true on success, false on name not found
   bool load(const string &address);

   /// build and return the address as a sockaddr_in
   struct sockaddr_in buildSockaddr_in() const;

   /// build and return the address as an in_addr
   struct in_addr build_in_addr() const {return addr;}

   /// Is the address in left numerically less than the one in right?
   friend bool operator<(const ComAddr &left, const ComAddr &right);

   /// Are the addresses equal?
   friend bool operator==(const ComAddr &left, const ComAddr &right);

   /// Are the addresses equal, if we ignore the port?
   bool addrsEqual(const ComAddr &other) const; 

   /// Get the port number, as a normal integer
   uint getPort() const; 

   /// Set the port number, as a normal integer
   void setPort(uint port); 

public:
   /// The IP address as binary data, in network byte order!
   in_addr addr;

   /// the port number, in network byte order!
   uword port;

};

/*********************************************************************/
}
#endif
