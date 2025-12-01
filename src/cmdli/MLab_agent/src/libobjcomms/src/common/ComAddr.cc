/**********************************************************************
 **                                                                  **
 **  ComAddr.cc                                                      **
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

/* $Id: ComAddr.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: ComAddr.cc,v $
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
// Revision 1.15  2004/11/13 00:40:54  doug
// fixed a byte order problem with port in the senderaddress in the header
//
// Revision 1.14  2004/08/12 16:09:10  doug
// add new methods
//
// Revision 1.13  2004/07/30 13:42:40  doug
// fix selection of interface
//
// Revision 1.12  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.10  2004/04/27 16:19:32  doug
// added support for multicast as unicast
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
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(WIN32)
#endif

#include "ComAddr.h"
#include "mic.h"

namespace sara
{
// *******************************************************************
// Constructor
ComAddr::ComAddr() :
port(0)
{
   memset(&addr,0,sizeof(struct in_addr));
}

// *******************************************************************
// Constructor
ComAddr::ComAddr(const ComAddr &old) :
port(old.port)
{
   memcpy(&addr,&old.addr,sizeof(struct in_addr));
}

// ********************************************************************
// Constructor
ComAddr::ComAddr(const string &dst_addr, uint dst_port)
{
   // load the address
#if defined(linux)
   if( inet_aton(dst_addr.c_str(), &addr) == 0 )
#elif defined(WIN32)
   if( (addr.s_addr = inet_addr(dst_addr.c_str())) == INADDR_NONE )
#else
#error "Unsupported architecture"
#endif
   {
      ERROR("ComAddr::ComAddr - Unable to load address '%s': address invalid", dst_addr.c_str() );
   }

   // convert to network byte order.
   port = htons(dst_port);
}

// ********************************************************************
/// The address is formatted at addr:port, example: "192.168.1.1:3413"
/// If no port is specified, the port number defaults to 0
ComAddr::ComAddr(const string &dst_addr)
{
   char *dup = strdup( dst_addr.c_str() );

   // find the colon
   char *colon = strchr(dup, ':');
   if( colon )
   {
      // skip past the colon and get the port number
      port = htons( atoi( colon + 1 ) );
   
      // mark the end of the ip address
      *colon = '\0';      
   }
   else
   {
      port = 0;
   }

   // load the address
#if defined(linux)
   if( inet_aton(dup, &addr) == 0 )
#elif defined(WIN32)
   if( (addr.s_addr = inet_addr(dup)) == INADDR_NONE )
#else
#error "Unsupported architecture"
#endif
   {
      ERROR("ComAddr::ComAddr - Unable to load address '%s': address invalid", dst_addr.c_str() );
   }

   free( dup );
}

// ********************************************************************
/// The port number defaults to 0
ComAddr::ComAddr(const in_addr &address) :
   port(0)
{
   addr.s_addr = address.s_addr;
};

// ********************************************************************
/// Create from a sockaddr_in struct
ComAddr::ComAddr(const sockaddr_in &address)
{
   addr.s_addr = address.sin_addr.s_addr;
   port = address.sin_port;
};

// ********************************************************************
/// Create from raw data
/// Both IPaddr and portNum must already be in network byte order
ComAddr::ComAddr(uint IPaddr, uword portNum)
{
   addr.s_addr = IPaddr;
   port = portNum;
}

// ********************************************************************
// assignment operator
ComAddr &
ComAddr::operator= (const ComAddr &old)
{
   port = old.port;
   memcpy(&addr,&old.addr,sizeof(struct in_addr));
   return *this;
}

// ********************************************************************
// Return as a string, with all values in normal, local host byte order 
string 
ComAddr::str() const
{
   stringstream str;
   str << inet_ntoa( addr ) << ":" << ntohs(port);
   return str.str();
}

// ********************************************************************  
// Load from an address record
// returns true on success, false on invalid address.
bool 
ComAddr::load(struct sockaddr_in *saddr, int addr_len)
{
   // copy the address over
   addr = saddr->sin_addr; 
   port = saddr->sin_port;

   return true;
}

// ********************************************************************
// Load from a string (either a name or standard dotted notation)
// returns true on success, false on name not found
bool 
ComAddr::load(const string &address, const int dest_port)
{
//cerr << "calling gethostbyname with name='" << address << "'" << endl;
   struct hostent *hptr = gethostbyname(address.c_str());
   if( hptr == NULL )
   {
      ERROR_with_perror("gethostbyname returned NULL!");
      return false;
   }
//cerr << "official host name: " << hptr->h_name << endl;
//char **pptr = hptr->h_addr_list;
//for(;*pptr != NULL; pptr++)
//   cerr << "returned addr: " << inet_ntoa( *(in_addr *)*pptr) << endl;

   // just take the first address
   addr = *(in_addr *)*hptr->h_addr_list;

   // convert to network byte order.
   port = htons(dest_port);

   return true;
}

// ********************************************************************
/// Load from a string (either a name or standard dotted notation)
/// followed by a colon and the port number.  Ex: "192.168.0.0:5321"
/// returns true on success, false on name not found
bool 
ComAddr::load(const string &address)
{
   char *dup = strdup( address.c_str() );

   // find the colon
   char *colon = strchr(dup, ':');
   if( colon )
   {
      // skip past the colon and get the port number
      port = htons( atoi( colon + 1 ) );
   
      // mark the end of the ip address
      *colon = '\0';      
   }
   else
   {
      port = 0;
   }

   // load the address
#if defined(linux)
   if( inet_aton(dup, &addr) == 0 )
#elif defined(WIN32)
   if( (addr.s_addr = inet_addr(dup)) == INADDR_NONE )
#else
#error "Unsupported architecture"
#endif
   {
      return false;
   }

   free( dup );
   return true;
}

// ********************************************************************
// build and return the address as a sockaddr_in
struct sockaddr_in 
ComAddr::buildSockaddr_in() const
{
   struct sockaddr_in sin_addr;
   memset(&sin_addr, 0, sizeof(struct sockaddr_in));
   sin_addr.sin_family = AF_INET;
   sin_addr.sin_port = port;
   sin_addr.sin_addr = addr;

   return sin_addr;
}

// ********************************************************************
/// Is the address in left numerically less than the one in right?
bool 
operator<(const ComAddr &left, const ComAddr &right)
{
   // check each byte, starting from the most significant byte (left)
   byte *pl = (byte *)&left.addr;
   byte *pr = (byte *)&right.addr;
   for( uint i=0; i<sizeof(struct in_addr); i++ )
   {
      // if they are not equal, return their relationship
      if( *pl < *pr )
         return true;
      else if( *pl > *pr )
         return false;

      // they are equal, so check the next significant byte
      pl++;
      pr++;
   }

   // all match down to the last byte, so the port number is the decider.
   if( left.port < right.port )
      return true;
   else
      return false;
}

// ********************************************************************
/// Are the addresses equal?
bool 
operator==(const ComAddr &left, const ComAddr &right)
{
   // check each byte, starting from the most significant byte (left)
   byte *pl = (byte *)&left.addr;
   byte *pr = (byte *)&right.addr;
   for( uint i=0; i<sizeof(struct in_addr); i++ )
   {
      // if they are not equal, return their relationship
      if( *pl != *pr )
         return false;

      // they are equal, so check the next significant byte
      pl++;
      pr++;
   }

   // all match down to the last byte, so the port number is the decider.
   if( left.port != right.port )
      return false;

   return true;
}

// ********************************************************************
/// Are the addresses equal, if we ignore the port?
bool 
ComAddr::addrsEqual(const ComAddr &other) const
{
   // check each byte, starting from the most significant byte (left)
   byte *pl = (byte *)&addr;
   byte *pr = (byte *)&other.addr;
   for( uint i=0; i<sizeof(struct in_addr); i++ )
   {
      // if they are not equal, return their relationship
      if( *pl != *pr )
         return false;

      // they are equal, so check the next significant byte
      pl++;
      pr++;
   }

   // all match down to the last byte
   return true;
}

// ********************************************************************
/// Get the port number, as a normal integer
uint 
ComAddr::getPort() const
{
   // convert to local host byte order.
   return ntohs(port);
}

// ********************************************************************
/// Set the port number, as a normal integer
void 
ComAddr::setPort(uint new_port)
{
   // convert to network byte order.
   port = htons( new_port );
}

// ********************************************************************
}
