/* ********************************************************************
 **                                                                  **
 **  posix_udp_objcomms.cc                                           **
 **                                                                  **
 **  posix implementation of the objcomms class                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 ******************************************************************* */

/* $Id: posix_udp_objcomms.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: posix_udp_objcomms.cc,v $
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
// Revision 1.78  2004/11/13 00:40:54  doug
// fixed a byte order problem with port in the senderaddress in the header
//
// Revision 1.77  2004/11/12 21:45:07  doug
// fixed NPE when uncompress fails
//
// Revision 1.76  2004/11/08 19:36:51  doug
// redid header read/write so packs fields, instead of just copying over the header, which is 4 bytes per field, even for a char.
//
// Revision 1.75  2004/11/06 14:13:06  doug
// Change not being able to unicast to a non fatal error so that multicast-only uses can operate multiple instances on a single host
//
// Revision 1.74  2004/09/21 13:52:05  doug
// added support for robotname@ipaddress:port to addresslist.  This allows easily having a single resource file to run multiple robots on a single computer for testing
//
// Revision 1.73  2004/08/31 20:17:39  doug
// fixing debug print
//
// Revision 1.72  2004/08/31 20:08:26  doug
// cleanup debug messages
//
// Revision 1.71  2004/08/20 18:11:59  doug
// fix debug message
//
// Revision 1.70  2004/08/12 16:28:53  doug
// change open interface to add a parameter to append instance number or not
//
// Revision 1.69  2004/08/12 16:09:54  doug
// correct selection of multicast port, and allow passing port as part of multicast address using colon syntax in open method.
//
// Revision 1.68  2004/07/30 13:42:40  doug
// fix selection of interface
//
// Revision 1.67  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.66  2004/04/30 17:35:59  doug
// works for UPenn
//
// Revision 1.65  2004/04/27 16:19:32  doug
// added support for multicast as unicast
//
// Revision 1.64  2004/04/14 17:40:15  doug
// fixed endian issues
//
// Revision 1.63  2004/04/13 19:44:52  doug
// change to passing a cloned message for subscriber callbacks
//
// Revision 1.62  2004/04/12 21:20:21  doug
// udp unicast, udp multicast, and tcp streams all work on linux and win32!
//
// Revision 1.61  2004/04/07 12:57:46  doug
// fix typo
//
// Revision 1.60  2004/04/06 15:51:08  doug
// working on test_reader.exe on win32
//
// Revision 1.59  2004/04/05 22:19:22  doug
// multicasting works on linux.  Trying to build test_reader.exe on win32
//
// Revision 1.58  2004/03/31 14:15:06  doug
// standardize the objcomms interface methods
//
// Revision 1.57  2004/03/26 21:42:30  doug
// no encryption for win32
//
// Revision 1.56  2004/03/26 21:25:08  doug
// finally compiles under win32
//
// Revision 1.55  2004/03/26 00:18:58  doug
// working on win32 port
//
// Revision 1.54  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.53  2004/03/08 14:52:17  doug
// cross compiles on visual C++
//
// Revision 1.52  2004/03/01 00:47:10  doug
// cmdli runs in sara
//
// Revision 1.51  2003/07/12 01:59:44  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.50  2003/07/10 17:41:30  doug
// Robot deaths finally work!
//
// Revision 1.49  2003/07/10 16:48:53  doug
// test harness for commMsg in and out functions
//
// Revision 1.48  2003/07/10 13:57:22  doug
// cleanup debug message
//
// Revision 1.47  2003/07/09 22:19:16  doug
// The robot now exits cleanly
//
// Revision 1.46  2003/07/08 21:15:59  doug
// cleanup debug messages
//
// Revision 1.45  2003/07/08 21:09:29  doug
// Cleanup error messages
//
// Revision 1.44  2003/07/08 20:49:05  doug
// Rework to get controller to exit cleanly on terminate
//
// Revision 1.43  2003/07/03 16:32:23  doug
// Fixed a nasty race condition where we would read the header on a UDP packet and the packet would get overwritten before we went back and read the body, leaving the header and the body not matching.
//
// Revision 1.42  2003/06/18 21:46:59  doug
// debugging the debug prints
//
// Revision 1.41  2003/06/17 20:42:36  doug
// adding debug to track down broadcast problem
//
// Revision 1.40  2003/06/16 21:44:10  doug
// need to clear isDirected when broadcast
//
// Revision 1.39  2003/04/29 21:25:10  doug
// Hey tcp works!
//
// Revision 1.38  2003/04/28 22:00:38  doug
// switched to a pipe to wake up the select, but still is blocking
//
// Revision 1.37  2003/04/28 21:44:15  doug
// tried using out of band data to break the select call, but doesn't work.  Will next try creating a pipe between the reader and writer threads to break the select.
//
// Revision 1.36  2003/04/25 22:13:11  doug
// fixed handling of dropped tcp connections and changed to use non-padding encryptBuffer calls on the packet header.
//
// Revision 1.35  2003/04/24 20:56:51  doug
// still coding on tcp support
//
// Revision 1.34  2003/04/23 20:57:13  doug
// still working on tcp support
//
// Revision 1.33  2003/04/22 23:51:46  doug
// fixed compile errors
//
// Revision 1.32  2003/04/21 17:43:27  doug
// working on adding tcp support to objcomms to handle large data streams point to point
//
// Revision 1.31  2003/04/17 20:33:05  doug
// Adding support for tcp connections
//
// Revision 1.30  2003/04/11 18:46:55  doug
// added export capabilities
//
// Revision 1.29  2003/03/21 15:55:33  doug
// moved libccl_code to the sara namespace
//
// Revision 1.28  2003/03/16 03:24:01  doug
// moved libccl_code to the sara namespace
//
// Revision 1.27  2003/02/28 23:50:48  doug
// working on timing
//
// Revision 1.26  2003/02/28 22:07:28  doug
// turn off some debug prints
//
// Revision 1.25  2003/02/28 14:33:56  doug
// use new TimeOfDay call
//
// Revision 1.24  2003/02/27 22:43:01  doug
// seems to work
//
// Revision 1.23  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.22  2003/02/19 22:15:15  doug
// Change waitfor to use the new TimeOfDay
//
// Revision 1.21  2003/02/17 23:22:32  doug
// Allow sent messages to also show up as received
//
// Revision 1.20  2003/02/17 23:00:13  doug
// assigns task lead
//
// Revision 1.19  2003/02/14 21:47:13  doug
// links again
//
// Revision 1.18  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.17  2003/02/10 22:39:54  doug
// tested out well
//
// Revision 1.16  2003/02/06 20:29:30  doug
// trying to get the packet loss down
//
// Revision 1.15  2003/02/04 21:51:34  doug
// It sends and receives packets now
//
// Revision 1.14  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.13  2003/02/03 22:36:02  doug
// It seems to work finally!
//
// Revision 1.12  2003/01/31 22:36:48  doug
// working a bit
//
// Revision 1.11  2003/01/31 19:30:13  doug
// test1 builds and links!
//
// Revision 1.10  2003/01/31 15:02:24  doug
// It compiles again
//
// Revision 1.9  2003/01/31 02:08:22  doug
// snapshot
//
// Revision 1.8  2003/01/31 00:47:26  doug
// compiles!
//
// Revision 1.7  2003/01/29 22:19:56  doug
// snapshot
//
// Revision 1.6  2003/01/28 22:01:10  doug
// snapshot
//
// Revision 1.5  2003/01/27 23:35:35  doug
// snapshot
//
// Revision 1.4  2003/01/24 22:40:03  doug
// getting closer
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

#define SUPPORT_COMPRESSION
//#if !defined(WIN32)
//#define SUPPORT_ENCRYPTION
//#endif

// *********************************************************************

#include <errno.h>
#ifdef WIN32
   #include "winsock2.h"
   #define EADDRINUSE WSAEADDRINUSE
   #include <io.h>
   #include <ws2tcpip.h>
   #include <iphlpapi.h>
#endif
#ifdef linux
   #include <unistd.h>
   #include <fcntl.h>
   #include <netdb.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
   #include <net/if.h>
   #include <sys/ioctl.h>
   #include <errno.h>

   #define closesocket(s)   close(s)
   #define SOCKET_ERROR  (-1)
#endif

#include "mic.h"
#include <ctype.h>
#if defined(SUPPORT_COMPRESSION)
   #include "compress_zlib.h"
#endif
#if defined(SUPPORT_ENCRYPTION)
   #include "encryption_evp.h"
#endif
#include "posix_udp_objcomms.h"

namespace sara
{
// buffers for data coming in the wire
static const int BUFSIZE = PacketHeader::MAXDATASIZE + PacketHeader::HEADERSIZE + 16;

// *********************************************************************
posix_udp_objcomms::posix_udp_objcomms(objcomms *base) :
parent(base),
udpsock(-1),
udpsock_isBound(false),
mulsock(-1),
tcpsock(-1),
num_instances_per_host(1),
compressData(false),
encryptData(false),
encryptionKey(""),
exiting(false),
multicastIsUnicast(false),
receive_multicast(true),
opened(false)
{
#if defined(WIN32)
   WORD wVersionRequested;
   WSADATA wsaData;
   int err;
 
   wVersionRequested = MAKEWORD( 2, 2 );
 
   err = WSAStartup( wVersionRequested, &wsaData );
   if ( err != 0 ) 
   {
       /* Tell the user that we could not find a usable */
       /* WinSock DLL.                                  */
       ERROR("Unable to load version 2.2 of the winsock library!");
   }
 
   /* Confirm that the WinSock DLL supports 2.2.*/
   /* Note that if the DLL supports versions greater    */
   /* than 2.2 in addition to 2.2, it will still return */
   /* 2.2 in wVersion since that is the version we      */
   /* requested.                                        */
 
   if ( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 ) 
   {
       /* Tell the user that we could not find a usable */
       /* WinSock DLL.                                  */
       WSACleanup( );
       ERROR("Unable to load version 2.2 of the winsock library!");
   }
 
   /* The WinSock DLL is acceptable. Proceed. */
#endif
}

// *********************************************************************
/// Internal wrapper function around setsockopt to hide ifdefs
/// returns true on success, false on failure and errno is set.
bool 
posix_udp_objcomms::setSocketOption(int s, int level, int optname, const void *optval, socklen_t optlen) const
{
#if defined(linux)
   return setsockopt(s, level, optname, optval, optlen) != SOCKET_ERROR;
#elif defined(WIN32)
   return setsockopt(s, level, optname, (const char *)&optval, optlen) != SOCKET_ERROR;
#else
#error "Unsupported architecture in posix_udp_objcomms::setSocketOption";
#endif
};

// *********************************************************************
/// load a set of addresses for the team members
/// Loading these values Will cause multicasts to use multiple unitcast packets!
/// The address format is ip:port, for example: "192.168.1.3:4321"
bool 
posix_udp_objcomms::loadMemberAddresses(const strings &addresses)
{
   multicastIsUnicast = true;

   for(uint i=0; i<addresses.size(); ++i)
   {
      string::size_type pos = addresses[i].find('@');
      if( pos != addresses[i].npos )
      {
         teamAddrs.push_back( AddrEntry( addresses[i].substr(0, pos), 
		                         ComAddr( addresses[i].substr(pos+1))));
      }
      else
      {
	 WARN("Address entry is missing '@' sign.  Format: name@ip:port");
         teamAddrs.push_back( AddrEntry( ComAddr( addresses[i])));
      }
   }

INFORM("posix_udp_objcomms::loadMemberAddresses - setting multicast as unicast:");
for(uint i=0; i<teamAddrs.size(); ++i)
{
   PRINT("   %s = %s\n", teamAddrs[i].name.c_str(), teamAddrs[i].address.str().c_str());
}

   return true;
}

// *********************************************************************
/// open the communications port 
/// Allocate and connect the broadcast socket using UDP
///
/// multicastAddress:        Multicast address (e.g., 224.0.0.0:PORT to 224.0.0.255:PORT)
///                          where PORT is the port number 0-65535 to use for multcast.
///                          If PORT is not specified, then port (next parm) will be used.
/// port:                    Base unicast port.  Several ports may be used,
///                          [port ... port+maxInstancesPerHost-1]
/// procName:                A unique network name for the process.
///                          If empty, will use hostname.
/// appendInstance:          If true and more than one instance, will append
///                          the instance number to the name to make it unique.
/// maxInstancesPerHost:     The maximum number of copies of the 
///                          program running on any one host.
///                          (NOTE: Keep as small as practical)
/// instanceNumber:          If not NULL, the instance number of 
///                          this process on this host is returned.
/// Returns true on success, false on failure.
bool 
posix_udp_objcomms::open(
             const string &multicastAddress,
             const uint    port,
	     const string &procName,
	     const bool    appendInstance,
             const uint    maxInstancesPerHost,
             uint         *instanceNumber)
{
   // get a local copy of the port
   uint    our_port = port;
   ComAddr our_addr;
   bool    our_addr_valid = false;

   // A list of addresses, one for each interface, that we listen on
   // only loaded if doing real multicasting
   typedef vector< ComAddr > addrs;
   addrs interfaces;
   uint theInf = 0;

   if( multicastIsUnicast )
   {
      num_instances_per_host = 1;

      // see if they specified an address for us in the teamAddrs
      // not worth looking if they didn't give us a name.
      if( !procName.empty() )
      {
         for(uint i=0; i<teamAddrs.size(); ++i)
         {
            if( !cmp_nocase(teamAddrs[i].name, procName) )
            {
               // we found us.  Use the address they specified.
               our_port = teamAddrs[i].address.getPort();
	       our_addr = teamAddrs[i].address;
	       our_addr_valid = true;
               break;
            }
         }
      }
   }
   else
   {
      // Remember how many to try on each host.
      num_instances_per_host = maxInstancesPerHost;
   }

   // remember our port in the member data
   base_port = our_port;

   if( parent->debug & objcomms::DEBUG_GEN )
   {
      INFORM("posix_udp_objcomms::open - supporting %d instances per host", num_instances_per_host);
   }

   ComAddr multicast_address;

   // Remember the broadcast IP address.
   BROADCAST_IP = multicastAddress;

   // make sure it is valid
   if( !multicast_address.load( multicastAddress ) )
   {
      if( multicastIsUnicast )
      {
	 // just fail quietly if they already set us to unicast
         receive_multicast = false;
      }
      else
      {
#if defined(linux)
         ERROR("Unable to parse multicast address '%s': address invalid", BROADCAST_IP.c_str());
#elif defined(WIN32)
         int err = WSAGetLastError();
         ERROR("Received error %d trying to parse multicast address '%s': address invalid", err, BROADCAST_IP.c_str());
#else
#error "Unsupport Architecture in posix_udp_objcomms::open"
#endif
         return false;
      }
   }

   // if they didn't specify a port, use the base port
   if( multicast_address.getPort() == 0 )
   {
      multicast_address.setPort(port);
   }

   // We open three sockets for receiving communications on the designated port:
   //    udpsock  -  sends all packets
   //                if udpsock_isbound, then receives standard unicast UDP packets
   //    mulsock  -  receives multicast packets
   //                Note: we do not open mulsock if receive_multicast is false
   //    tcpsock  -  accepts tcp connections on the port
   // ***************************************************************************
   // Set up a socket for standard UDP communications.
   if( (udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
   {
      ERROR("Unable to open broadcast socket: open failed");
      return false;
   }

   // fill in the address for binding our unicast port
   sockaddr_in interface_addr;
   memset((char *)&interface_addr, sizeof(interface_addr), 0);
   interface_addr.sin_family = AF_INET;

   // if they told us what address to use, then use it.
   if( our_addr_valid )
   {
      // get the defining internet address for the first interface 
      interface_addr.sin_addr = our_addr.build_in_addr();
   }
   // otherwise, pick one based on the available network interfaces
   else
   {
      const int maxInterfaces(16);
#if defined(linux)
      // Allocate space to hold the info about the NICs
      uint buflen = sizeof(struct ifreq) * maxInterfaces;
      char buf[ buflen ];
   
      // create the info record
      struct ifconf ifc;
      ifc.ifc_len = buflen;
      ifc.ifc_buf = buf;

      // get the list of interface records
      if( ioctl(udpsock, SIOCGIFCONF, &ifc) < 0 )
      {
         ERROR_with_perror("Failed to load interface records");
         return false;
      }

      // walk the list of interfaces
      uint numRecs = ifc.ifc_len / sizeof(struct ifreq);
      struct ifreq *irec = (struct ifreq *)buf;
      for(uint index = 0; index < numRecs; index++)
      {
         // Don't add the loopback if we are doing real multicast.
         if( !teamAddrs.empty() || strcasecmp(irec->ifr_name, "lo") != 0 )
         {
            if( irec->ifr_addr.sa_family == AF_INET )
            {
	       interfaces.push_back( ComAddr( ((struct sockaddr_in *)&irec->ifr_addr)->sin_addr) );
   
	       if( parent->debug & objcomms::DEBUG_GEN )
	       {
	          string straddr( inet_ntoa(((struct sockaddr_in *)&irec->ifr_addr)->sin_addr) );
	          INFORM("Adding interface %s with address address %s and flags %d", irec->ifr_name, straddr.c_str(), irec->ifr_flags);
	       }
            }
         }
   
         // move to the next record
         irec ++;
      }


#elif defined(WIN32)

      // Allocate space to hold the info about the NICs
      IP_ADAPTER_INFO AdapterInfo[maxInterfaces];
      DWORD dwBufLen = sizeof(AdapterInfo);  

      // Get a list of interface records
      DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
      if( dwStatus != ERROR_SUCCESS)
      {
         ERROR("posix_udp_objcomms::open - received error %d from GetAdapatersInfo");
         return false;
      }

      // Walk the list of interfaces
      PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo; 
      do 
      {
         // It returns a list of addresses for each interface,
         // but we are just going to take the first one.
         interfaces.push_back( ComAddr( pAdapterInfo->IpAddressList.IpAddress.String ) );
   
         if( parent->debug & objcomms::DEBUG_GEN )
         {
	    INFORM("Adding interface with address %s", pAdapterInfo->IpAddressList.IpAddress.String);
         }

         // move to next record
         pAdapterInfo = pAdapterInfo->Next;
      } while(pAdapterInfo);                    // Terminate if last adapter
   
#else
#error "Undefined architecture"
#endif

      uint numInfs = interfaces.size();
      if( numInfs == 0 )
      {
         // oops, add the loopback.
         interfaces.push_back( ComAddr("127.0.0.1") );
//      ERROR("No UP NIC interfaces identified!");
//      return false;
      }

      if( !teamAddrs.empty() )
      {
         // Use the interface they specified for us.
         bool foundIt = false;
         for(uint a=0; !foundIt && a<teamAddrs.size(); ++a)
         {
            for(uint i=0; i<numInfs; i++)
            {
               if( interfaces[i].addrsEqual(teamAddrs[a].address) )
               {
                  // we found it.  Use this one.
                  foundIt = true;
                  theInf = i;
               }
            }
         }
   
         if( !foundIt )
         {
            ERROR("Unable to find a network interface that matches any of the node addresses!");
            return false;
         }
      }

      // get the defining internet address for the first interface 
      interface_addr.sin_addr = interfaces[theInf].build_in_addr();
   }


   // Bind the socket to the port, looping through ports if multiple instances are supported
   for( uint tries=0; tries<num_instances_per_host; tries++, our_port++ )
   {
      interface_addr.sin_port = htons(our_port);

      // Try to bind the socket to the broadcast address/port.
      // loops if port is already assigned and tries the next higher.
      int rtn = bind(udpsock, (sockaddr *)&interface_addr, sizeof(interface_addr) );

      // Port is already in use.  Try next higher.
      if( rtn < 0 && errno == EADDRINUSE )
         continue;

      // A real error.
      if( rtn < 0 )
      {
#if defined(WIN32)
         int err = WSAGetLastError();
         ERROR("Received error %d trying to bind udp socket IP:ports <%s:%d-%d>",
                           err, inet_ntoa(interface_addr.sin_addr), base_port, our_port);
#else
         ERROR_with_perror("Unable to bind udp socket IP:ports <%s:%d-%d>",
                           inet_ntoa(interface_addr.sin_addr), base_port, our_port);
#endif
         break;
      }

      // Got one.
      break;
   }

   udpsock_isBound = true;
   if( our_port >= base_port + num_instances_per_host )
   {
      if( num_instances_per_host > 1 )
      {
         ERROR("Too many instances in use trying to bind unicast udp socket.\nTried ports <%d-%d>.\nRestart with more instances supported per host.", 
               base_port, 
               base_port + num_instances_per_host - 1);
      }
      else
      {
         ERROR("Unable to bind unicast udp socket to port %d.  Is an instance already running?", base_port);
      }

      WARN("Unicast incoming messages will not be supported");

      // just pick a random port
      interface_addr.sin_port = 0;
      int rtn = bind(udpsock, (sockaddr *)&interface_addr, sizeof(interface_addr) );
      if( rtn < 0 )
      {
#if defined(WIN32)
         int err = WSAGetLastError();
         ERROR("Received error %d trying to bind random port to udp socket for IP <%s>",
                           err, inet_ntoa(interface_addr.sin_addr));
#else
         ERROR_with_perror("Unable to bind random port to udp socket for IP <%s>",
                           inet_ntoa(interface_addr.sin_addr));
#endif
	 our_port = 0;
         udpsock_isBound = false;
      }
      else
      {
	 struct sockaddr_in sa;
	 socklen_t len = sizeof( struct sockaddr_in );

	 // find out the port that we were assigned.
         if( getsockname(udpsock, (struct sockaddr *)&sa, &len) < 0 )
	 {
#if defined(WIN32)
            int err = WSAGetLastError();
            ERROR("Received error %d trying to get random port number for udp socket for IP <%s>",
                           err, inet_ntoa(interface_addr.sin_addr));
#else
            ERROR_with_perror("Error trying to get random port number for udp socket for IP <%s>",
                           inet_ntoa(interface_addr.sin_addr));
#endif
	    our_port = 0;
            udpsock_isBound = false;
	 }
	 else if( sa.sin_family != AF_INET )
         {
            ERROR("Unexpected sa_family %d returned by getsockname for random udp socket for IP <%s>", sa.sin_family, inet_ntoa(interface_addr.sin_addr));
	    our_port = 0;
            udpsock_isBound = false;
	 }
	 else
	 {
            interface_addr.sin_port = sa.sin_port;
	    our_port = ntohs( interface_addr.sin_port );
            WARN("Bound unicast to random port %d", our_port);
	 }
      }

      // Remember our instance number
      instance = 0;
   }
   else
   {
      // Remember our instance number
      instance = our_port - port;

      if( parent->debug & objcomms::DEBUG_GEN )
      {
         INFORM("udpsock listening on %s:%d", inet_ntoa(interface_addr.sin_addr), ntohs(interface_addr.sin_port));
      }
   }

INFORM("posix_udp_objcomms-open:: udpsock listening on %s:%d", inet_ntoa(interface_addr.sin_addr), ntohs(interface_addr.sin_port));

   ourNetworkAddress = ComAddr(interface_addr);

   if( parent->debug & objcomms::DEBUG_GEN && num_instances_per_host > 1 )
   {
      INFORM("Supporting %d instance%s per host", num_instances_per_host, num_instances_per_host > 1 ? "s" : "");
   }

   // Return the port offset, if they want it.
   if( instanceNumber != NULL )
   {
      *instanceNumber = instance;
   }

   // Set the flag to allow multiple binds to the same port number.
   // Note: all sockets overlapping the address must set the flag.
   int value = 1;
   if( setsockopt(udpsock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) == SOCKET_ERROR)
   {
      WARN_with_perror("Unable to add address reuse permissions to socket. Multiple robots per processor will not be supported.");
   }


   // ****************************************************************************
   // Set up a socket for multicast UDP communications.

   if( receive_multicast )
   {
      // open the socket
      if( (mulsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
      {
         if( !multicastIsUnicast )
	 {
            ERROR("Unable to open multicast socket: open failed");
            return false;
	 }
	 else
	 {
            receive_multicast = false;
	 }
      }
   }

   if( receive_multicast )
   {
      // Set the flag to allow multiple binds to the same port number.
      // Note: all sockets overlapping the address must set the flag.
      value = 1;
      if( setsockopt(mulsock, SOL_SOCKET, SO_REUSEADDR, (const char *)&value, sizeof(value)) == SOCKET_ERROR)
      {
         WARN_with_perror("Unable to add address reuse permissions to socket. Multiple robots per processor will not be supported.");
      }
   
      // load the multicast group address for the bind operation
      memset((char *)&multicast_addr, 0, sizeof(multicast_addr));
      multicast_addr.sin_family = AF_INET;
      // get the correct port: either from the address parm or the baseport
      multicast_addr.sin_port = multicast_address.port;
      multicast_addr.sin_addr.s_addr = INADDR_ANY;
   
      // bind the multicast socket to our muliticast group and port 
      if( bind(mulsock,(struct sockaddr *)&multicast_addr, sizeof(multicast_addr)) == SOCKET_ERROR )
      {
         if( !multicastIsUnicast )
	 {
#if defined(WIN32)
            int err = WSAGetLastError();
            ERROR("Received error %d trying to bind UDP multicast socket to %s:%d", err, inet_ntoa(multicast_addr.sin_addr), our_port);
#else
            ERROR_with_perror("Unable to bind UDP multicast socket to %s:%d", inet_ntoa(multicast_addr.sin_addr), our_port);
#endif
            return false;
	 }
	 else
	 {
            receive_multicast = false;
	 }
      }
   }

   struct ip_mreq mreq;
   if( receive_multicast )
   {
      // point the multicast address back to the real IP address, since it is used by the transmitter
      multicast_addr.sin_addr = multicast_address.build_in_addr();

      // join the multicast group
      mreq.imr_multiaddr = multicast_addr.sin_addr;
      mreq.imr_interface = interface_addr.sin_addr;
      if( setsockopt(mulsock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (const char *)&mreq, sizeof(mreq)) == SOCKET_ERROR)
      {
         if( !multicastIsUnicast )
	 {
#if defined(WIN32)
            int err = WSAGetLastError();
            string addr( inet_ntoa(mreq.imr_multiaddr) );
            string intr( inet_ntoa(mreq.imr_interface) );
            ERROR("Received error %d trying to join multicast group %s:%d on interface %s",
	          err, addr.c_str(), our_port, intr.c_str());
#else
            string addr( inet_ntoa(mreq.imr_multiaddr) );
            string intr( inet_ntoa(mreq.imr_interface) );
            ERROR_with_perror("Unable to join multicast group %s:%d on interface %s",
	          addr.c_str(), our_port, intr.c_str());
#endif
            return false;
	 }
	 else
	 {
            receive_multicast = false;
	 }
      }
   }
   
   if( receive_multicast )
   {
      // set our TTL to 16 to be safe
      uint ttl = 16;
      if( setsockopt(mulsock, IPPROTO_IP, IP_MULTICAST_TTL, (const char *)&ttl, sizeof(ttl)) == SOCKET_ERROR)
      {
         if( !multicastIsUnicast )
	 {
#if defined(WIN32)
            int err = WSAGetLastError();
            ERROR("Received error %d trying to set multicast TTL to %d", err, ttl);
#else
            ERROR_with_perror("Received error trying to set multicast TTL to %d", ttl);
#endif
            return false;
	 }
	 else
	 {
            receive_multicast = false;
	 }
      }
   }

   if( receive_multicast )
   {
      if( parent->debug & objcomms::DEBUG_GEN )
      {
         string addr( inet_ntoa(mreq.imr_multiaddr) );
         string intr( inet_ntoa(mreq.imr_interface) );
	 uint port(  ntohs(multicast_addr.sin_port) );
         INFORM("joined multicast group %s:%d on interface %s", addr.c_str(), port, intr.c_str());
      }
{
string addr( inet_ntoa(mreq.imr_multiaddr) );
string intr( inet_ntoa(mreq.imr_interface) );
uint port(  ntohs(multicast_addr.sin_port) );
INFORM("posix_udp_objcomms::open - joined multicast group %s:%d on interface %s", addr.c_str(), port, intr.c_str());
}
   }

   if( multicastIsUnicast )
   {
      if( parent->debug & objcomms::DEBUG_GEN )
      {
         INFORM("posix_udp_objcomms::open - Setting outbound multicast as unicast");
      }
   }

   if( receive_multicast )
   {
      if( parent->debug & objcomms::DEBUG_GEN )
      {
         INFORM("posix_udp_objcomms::open - Will receive inbound multicast");
      }
   }

   // *************************************************************************
   // Open a socket for TCP communications.
   if( (tcpsock = socket( PF_INET, SOCK_STREAM, 0)) < 0 )
   {
      ERROR("Unable to open tcp socket: open failed");
      return false;
   }

   // bind the socket to our port using the specified interface
   interface_addr.sin_port = htons(our_port);
   if( bind(tcpsock,(struct sockaddr *)&interface_addr, sizeof(struct sockaddr)) < 0)
   {
      ERROR_with_perror("Unable to bind tcp socket");
      WARN("Unicast incoming TCP connections will not be supported");
      close(tcpsock);
      tcpsock = -1;
   }
   else
   {
      if( parent->debug & objcomms::DEBUG_GEN )
      {
         INFORM("posix_udp_objcomms::open - tcpsock listening on %s:%d", inet_ntoa( interface_addr.sin_addr ), ntohs(interface_addr.sin_port));
      }

      // Start accepting tcp connection requests 
      if( listen(tcpsock, 64) < 0 )
      {
         ERROR_with_perror("Unable to listen on tcp socket");
         return false;
      }
   }

   // *************************************************************************
   // Pick a (hopefully) unique name for us
   stringstream st;
   bool hadHint = false;

   // If they gave us a hint, use it.
   if( procName.size() > 0 )
   {
      st << procName;
      hadHint = true;
   }
   else
   {
      // Nope, build a default name
      // 1st, check for an environment variable.
      char *name = getenv("ROBOTNAME");

      // 2nd, if no env var, use the machine name
      if( name == NULL )
         name = getenv("HOST");

      // 3rd, if HOST wasn't set, use the hostname call.
      if( name == NULL )
      {
         char our_name[80];
         if( gethostname(our_name, 80) != 0 )
         {
            // If that failed, give up and just use 'robot'.
            strcpy(our_name,"robot");
         }
         st << our_name;
      }
      else
      {
         // Copy the name in.
         st << name;
      }
   }

   // append our instance number (ones based), unless num_instances is 1
   // or they specified a name and told us to just use it.
   if( num_instances_per_host > 1 && appendInstance )
   {
      st << (instance + 1);
   }

   // remember our name (clipped to the max length)
   string tmp = st.str();
   if( tmp.size() > PacketHeader::MAXIDSIZE )
   {
      // take the right most MAXIDSIZE characters
      ourNetworkName = string(tmp, tmp.size() - PacketHeader::MAXIDSIZE, string::npos); 
   }
   else
   {
      ourNetworkName = tmp;
   }
INFORM("posix_udp_objcomms::open - using network name of '%s'", ourNetworkName.c_str());

   // *************************************************************************
   // open receiver/sender paired sockets for the tranmit thread to use to wake up the receiver thread
   int listener;
   int connector;
   int acceptor;

   if( (listener = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
   {
      ERROR("Unable to open internal tcp receiver socket on loopback interface: open failed");
      return false;
   }

   struct sockaddr_in listen_addr;
   memset(&listen_addr, 0, sizeof(listen_addr));
   listen_addr.sin_family = AF_INET;
   listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
   listen_addr.sin_port = 0;   /* kernel choses port.  */
   if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr)) == -1)
   {
      ERROR("Unable to open internal tcp receiver socket on loopback interface: bind failed");
      return false;
   }
   if (listen(listener, 1) == -1)
   {
      ERROR("Unable to open internal tcp receiver socket on loopback interface: listen failed");
      return false;
   }

   // Now the sender
   if( (connector = socket(PF_INET, SOCK_STREAM, 0)) < 0 )
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: open failed");
      return false;
   }

   // find out the port number that the receiver is using.
   struct sockaddr_in connect_addr;
#if defined(linux)
   socklen_t size;
#elif defined(WIN32)
   int size;
#endif
   size = sizeof (connect_addr);
   if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: getsockname failed");
      return false;
   }
   if (size != sizeof (connect_addr))
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: getsockname returned wrong size");
      return false;
   }

   if (connect(connector, (struct sockaddr *)&connect_addr, sizeof(connect_addr)) == -1)
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: connect failed");
      return false;
   }

   size = sizeof (listen_addr);
   acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
   if (acceptor == -1)
   {
      ERROR("Unable to open tcp transmit socket on loopback interface: accept failed");
      return false;
   }
   if (size != sizeof(listen_addr))
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: accept returned wrong size");
      return false;
   }

   // check that it is working.
   if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: unexpected failure after setup!");
      return false;
   }
   if (size != sizeof (connect_addr)
       || listen_addr.sin_family != connect_addr.sin_family
       || listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
       || listen_addr.sin_port != connect_addr.sin_port) 
   {
      ERROR("Unable to open internal tcp transmit socket on loopback interface: unexpected address difference after setup!");
      return false;
   }

   // It is working.  Now clean up.
   close(listener);
   pipe_output = connector;
   pipe_input = acceptor;

/*
   // set the pipe to non-blocking reads
   int val = fcntl(pipe_output, F_GETFL, 0);
   fcntl(pipe_output, F_SETFL, val | O_NONBLOCK);
*/

   // Fire up the transmit and receive threads
   trans = new transmitter(this);
   trans->start_thread();

   rec = new receiver(this);
   rec->start_thread();

   dsp = new disperser(this);
   dsp->start_thread();

   opened = true;

   return true;
}

/*********************************************************************/
/// Wait for the transmit queue to drain out, or the timeout
void 
posix_udp_objcomms::waitForTransmitter(const TimeOfDay &timeout)
{
   const TimeOfDay endTime = timeout + TimeOfDay::now();

   // Get a lock on the queue
   outputData_guard.lock();

   // Wait for the queue to go to zero
   while( TimeOfDay::now() < endTime && outputData.size() > 0 )
   {
      TimeOfDay left = endTime - TimeOfDay::now();
      outputData_changed.wait(outputData_guard, left);
   }

   // Free our lock on the queue
   outputData_guard.unlock();
}

// ********************************************************************
// Get our network name
string 
posix_udp_objcomms::getName() const
{
   return ourNetworkName;
}

// ********************************************************************
// Get our network address
ComAddr 
posix_udp_objcomms::getAddr() const
{
   return ourNetworkAddress;
}

// *********************************************************************
// Close the port
posix_udp_objcomms::~posix_udp_objcomms()
{
   // Close down the threads
   trans->shutdown();
   rec->shutdown();
   dsp->shutdown();

   // wait for the threads to finish (up to 1/10 sec each)
   trans->wait_for_completion(100);
   rec->wait_for_completion(100);
   dsp->wait_for_completion(100);

   if( udpsock > -1 )
   {
      closesocket(udpsock);
      udpsock = -1;
   }
   if( mulsock > -1 )
   {
      closesocket(mulsock);
      mulsock = -1;
   }
   if( tcpsock > -1 )
   {
      closesocket(tcpsock);
      tcpsock = -1;
   }

   if( pipe_output > -1 )
   {
      closesocket(pipe_output);
      pipe_output = -1;
   }
   if( pipe_input > -1 )
   {
      closesocket(pipe_input);
      pipe_input = -1;
   }

   // Release any waiters
   exiting = true;
   Waiters_T::iterator mtIT;
   for(mtIT = waitList.begin(); mtIT != waitList.end(); ++mtIT)
   {
      waitList_T::iterator it;
      for(it = mtIT->second.begin(); it != mtIT->second.end(); ++it)
      {
         // Signal the waiters to wake up and die
         (*it)->signaled.signal();

         // delete any pending messages
         (*it)->queue.erase((*it)->queue.begin(), (*it)->queue.end());
      }
   }

   // So we don't get into trouble later
   parent = NULL;
}

// ******************************************************************
/// prepare the message, potentually compressing and/or encrypting it
/// return a pointer to a new stringstream containing the output.
/// The user is responsible for deleting the returned stringstream.
stringstream * 
posix_udp_objcomms::prepareMsg(stringstream *in, bool &isCompressed)
{
   // init
   isCompressed = false;
   bool localIn = false;

   // Compress the data, if supported and requested
#if defined(SUPPORT_COMPRESSION)
   if( compressData )
   {
      compress *compressor = new compress_zlib();
      stringstream *str = compressor->compressStream( in );
      delete compressor;

      // did it help?
      if( str->str().size() < in->str().size() )
      {
         // Yes, send it compressed
         isCompressed = true;
         in = str;
         localIn = true;
      }
      else
      {
         // No, send it raw
         delete str;
      }
   }
#endif

   // Encrypt the data, if supported and requested
#if defined(SUPPORT_ENCRYPTION)
   if( encryptData )
   {
      // Create an encryption object in two steps to prevent init race conditions against the key.
      static encryption *encryptor = NULL;
      if( encryptor == NULL )
      {
         encryptor = new encryption_evp(encryptionKey);
      }

      // Encrypt the packet
      stringstream *str = encryptor->encryptStream(in);

      // Switch to the encrypted buffer
      if( localIn )
      {
         delete in;
      }
      in = str;
   }
#endif

   return new stringstream( in->str() );
}

// ******************************************************************
/// Send the message reliabily to the destination
/// Returns false if the port is not open.
bool 
posix_udp_objcomms::sendMsg(const ComAddr &dest, const commMsg &msg)
{
   // Duck out if socket not open.
   if( udpsock < 0 )
   {
      ERROR("socket is not open in send_broadcast");
      return false;
   }

   // update the counter
   parent->messagesTrn ++;

   // Create the streambuffer to hold the raw message data
   stringstream *ost = new stringstream();

   // Write the data to a stream buffer
   if( !msg.to_stream(ost) )
   {
      ERROR("Unable to convert message to stream");
      return false;
   }

   // Create the msgInfo record
   outputMsgInfo *info = new outputMsgInfo();
   info->isDirected = true;
   info->dest = dest;

   // compress and/or encrypt the data
   info->dataStream = prepareMsg(ost, info->isCompressed);
   delete ost;
   ost = NULL;

   // Get a lock on the queue
   outputData_guard.lock();

   // Wait for there to be room to add the message
   while( outputData.size() > outputData_MAXSIZE )
   {
      outputData_changed.wait(outputData_guard);
   }

   // Add our message to the end of the queue
   outputData.push_back(info);

   // Signal that we changed the queue
   outputData_changed.signal();

   // Free our lock on the queue
   outputData_guard.unlock();

   return true;
}

// ******************************************************************
/// Broadcast the message to all listeners
bool 
posix_udp_objcomms::broadcastMsg(const commMsg &msg)
{
   // Duck out if socket not open.
   if( udpsock < 0 )
   {
      ERROR("socket is not open in send_broadcast");
      return false;
   }

   // update the counter
   parent->messagesTrn ++;

   // Create the streambuffer to hold the raw message data
   stringstream *ost = new stringstream();

   // Write the data to a stream buffer
   if( !msg.to_stream(ost) )
   {
      ERROR("Unable to convert message to stream");
      return false;
   }

   // if debugging, dump it.
   if( parent->debug & objcomms::DEBUG_RAW )
   {
      INFORM("posix_udp_objcomms::broadcastMsg - sending %d byte message of type %d (internal format):", ost->str().size(), msg.msgType);

      // make a copy, seekg doesn't seem to work
      stringstream localstr(ost->str());

      int ch;
      int pos = 0;
      const int numPerLine(8);
      unsigned char chars[numPerLine];
      while( (ch=localstr.get()) != EOF )
      {
         PRINT(" 0x%2.2x", ch);
         chars[pos++] = ch;
         if( pos >= numPerLine )
         {
            PRINT("\t");
            for(int i=0; i<numPerLine; i++)
            {
               if( isprint(chars[i]) )
                  PRINT("%c", chars[i]);
               else
                  PRINT(".");
            }
            PRINT("\n");
            pos = 0;
         }
      }
   }

   // Create the msgInfo record
   outputMsgInfo *info = new outputMsgInfo();
   info->isDirected = false;

   // compress and/or encrypt the data
   info->dataStream = prepareMsg(ost, info->isCompressed);
   delete ost;
   ost = NULL;

   // Get a lock on the queue
   outputData_guard.lock();

   // Wait for there to be room to add the message
   while( outputData.size() > outputData_MAXSIZE )
   {
      outputData_changed.wait(outputData_guard);
   }

   // Add our message to the end of the queue
   outputData.push_back(info);

   // Signal that we changed the queue
   outputData_changed.signal();

   // Free our lock on the queue
   outputData_guard.unlock();

   return true;
}

// ******************************************************************
/// Register a marshalling & unmarshalling handler for a message type
/// returns: true on success
///          false if another handler already registered
bool 
posix_udp_objcomms::attachHandler(commMsg::MsgType_T msgType, objcomms::messageBuilder_T builder)
{
   messageBuilders[msgType] = builder;
   return true;
}

// ******************************************************************
/// Subscribe to get copies of messages meeting the criteria
/// Returns: a subscription handle used to cancel the subscription
objcomms::SubscriptionHandle 
posix_udp_objcomms::subscribeTo(commMsg::MsgType_T msgType, objcomms::message_callback_t callback, objcomms::USERDATA userData)
{
   // Get a lock on the subscriber list
   subscriberList_guard.lock();

   // Add their callback into the list.
   subscribers[msgType].push_back(SubEntry(callback,userData));

   // Free our lock on the subscriber list
   subscriberList_guard.unlock();

   // Use the address of the callback as their handle
   return(objcomms::SubscriptionHandle)callback;
}

// ******************************************************************
/// Cancel a previous subscription
bool 
posix_udp_objcomms::cancelSubscription(objcomms::SubscriptionHandle handle)
{
   return true;
}

// ******************************************************************
/// enable message compression
/// Returns true on success, false if compression is not supported.
bool 
posix_udp_objcomms::enableCompression()
{
#if defined(SUPPORT_COMPRESSION)
   compressData = true;
   return true;
#else
   return false;
#endif
}

// ******************************************************************
/// disable message compression
/// Returns true on success, false if compression was not enabled.
bool 
posix_udp_objcomms::disableCompression()
{
   bool rtn = compressData;
   compressData = false;
   return rtn;
}

// ******************************************************************
/// enable message encription
/// Returns true on success, false if encryption is not supported.
bool 
posix_udp_objcomms::enableEncryption(const string &encryptKey)
{
#if defined(SUPPORT_ENCRYPTION)
   encryptData = true;
   return true;
#else
   return false;
#endif
}

// ******************************************************************
/// disable message encription
/// Returns true on success, false if encryption was not enabled.
bool 
posix_udp_objcomms::disableEncryption()
{
   bool rtn = encryptData;
   encryptData = false;
   return rtn;
}

// ******************************************************************
/// change the key used for message encription
/// Returns true on success, false if the key is invalid
bool 
posix_udp_objcomms::changeEncryptionKey(const string &newKey)
{
   encryptionKey = newKey;
   return true;
}

// *********************************************************************
posix_udp_objcomms::disperser::disperser(posix_udp_objcomms *ourParent) :
parent(ourParent),
exiting(false)
{
   /* empty */
}

// ******************************************************************
void 
posix_udp_objcomms::disperser::shutdown()
{
   if( parent->parent->debug & objcomms::DEBUG_RECV )
   {
      INFORM("posix_udp_objcomms::disperser::shutdown - called");
   }

   // Tell the thread to quit
   exiting = true;

   // Wake up the thread if it is waiting on the signal
   parent->inputData_changed.signal();
}

// ******************************************************************
// Our job is to disperse the received messages to the interested listeners
void
posix_udp_objcomms::disperser::run(int)
{
   // The main run loop
   inputMsgInfo *info = NULL;
   commMsg *msg = NULL;
   while( !exiting )
   {
      // Get a lock on the queue
      parent->inputData_guard.lock();

      // Wait for a message to show up
      while( parent->inputData.size() == 0 )
      {
         parent->inputData_changed.wait(parent->inputData_guard);
      
         // Is this an exit wakeup?
         if( exiting )
         {
            parent->inputData_guard.unlock();
            break;
         }
      }
      
      // Are we exiting?
      if( exiting )
      {
         parent->inputData_guard.unlock();
         break;
      }

      // Remove the first one from the queue
      info = parent->inputData.front();
      parent->inputData.pop_front();

      // Signal that we changed the queue
      parent->inputData_changed.signal();

      // Free our lock on the queue
      parent->inputData_guard.unlock();

      // If the message is encrypted, decrypt it.
#if defined(SUPPORT_ENCRYPTION)
      if( parent->encryptData )
      {
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            INFORM("posix_udp_objcomms::disperser::run - decrypting message");
         }

         // Create an encryption object in two steps to prevent an initialization race on the key
         static encryption_evp *encryptor = NULL;
         if( encryptor == NULL )
         {
            encryptor = new encryption_evp(parent->encryptionKey);
         }

         // decrypt the data
         stringstream *str = encryptor->decryptStream(info->dataStream);
   
         // Switch to the decrypted buffer
         delete info->dataStream;
         info->dataStream = str;

	 // duck out if error decrypting
         if( !info->dataStream )
         {
            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - Discarding message - Unable to decrypt body");
            }

            delete info;
            info = NULL;
            continue;
         }
      }
#endif

      // If the message is compressed, uncompress it.
#if defined(SUPPORT_COMPRESSION)
      if( info->isCompressed )
      {
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            INFORM("posix_udp_objcomms::disperser::run - uncompressing message");
         }
         compress *compressor = new compress_zlib();
         stringstream *str = compressor->expandStream( info->dataStream );
         delete compressor;
   
         // Switch to the uncompressed buffer
         delete info->dataStream;
         info->dataStream = str;

	 // duck out if error expanding
         if( !info->dataStream )
         {
            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - Discarding message - Unable to expand compressed body");
            }

            delete info;
            info = NULL;
            continue;
         }
      }
#endif

      // get the message type
      commMsg::MsgType_T msgtype = commMsg::MsgType_from_stream(info->dataStream);

      // if debugging, dump it.
      if( parent->parent->debug & objcomms::DEBUG_RAW )
      {
         INFORM("posix_udp_objcomms::disperser::run - received message type %d (internal format):", msgtype);

         // make a copy, seekg doesn't seem to work
         stringstream localstr(info->dataStream->str());

         int size = 0;
         int ch;
         int pos = 0;
         const int numPerLine(8);
         unsigned char chars[numPerLine];
         while( (ch=localstr.get()) != EOF )
         {
            size++;
            PRINT(" 0x%2.2x", ch);
            chars[pos++] = ch;
            if( pos >= numPerLine )
            {
               PRINT("\t");
               for(int i=0; i<numPerLine; i++)
               {
                  if( isprint(chars[i]) )
                     PRINT("%c", chars[i]);
                  else
                     PRINT(".");
               }
               PRINT("\n");
               pos = 0;
            }
         }
         PRINT("\n");
         PRINT("containing %d bytes\n", size);
      }

      // Can we rebuild it?
      messageBuilders_T::iterator mit = parent->messageBuilders.find(msgtype);
      if( mit == parent->messageBuilders.end() )
      {
         // Nope, unknown message type.
         if( parent->parent->debug & objcomms::DEBUG_RECV )
         {
            INFORM("posix_udp_objcomms::disperser::run - Discarding unknown message. type= %d", msgtype);
         }

         delete info;
         info = NULL;
         continue;
      }

      msg = (*mit->second)( info->dataStream );
      if( msg == NULL )
      {
         // invalid message
         WARN("Discarding invalid message. type=%d", msgtype);
         delete info;
         info = NULL;
         delete msg;
         msg = NULL;
         continue;
      }

      // set the senderID
      msg->senderID = info->senderID;
      msg->senderAddr = info->senderAddr;

      // update the counter
      parent->parent->messagesRcv ++;

      // Get a lock on the subscriber list
      bool used = false;
      parent->subscriberList_guard.lock();

      // Does anyone want it?
      Subscribers_T::iterator it = parent->subscribers.find(msg->msgType);
      if( it != parent->subscribers.end() )
      {
         // Yup.  Walk the list
         SubscriberList_T::iterator sit;
         for( sit=it->second.begin(); sit != it->second.end(); sit++ )
         {
            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - passing message type= %d to subscriber", msgtype);
            }
            // Invoke their callback.
            (sit->first)( msg->clone(), sit->second);
            used = true;

            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - subscriber returned", msgtype);
            }
         }
      }

      // Free our lock on the subscriber list
      parent->subscriberList_guard.unlock();

      // Now deal with the waiters
      // Get the lock
      parent->waitList_guard.lock();

      // Any waiters?
      Waiters_T::iterator wit = parent->waitList.find(msg->msgType);
      if( wit != parent->waitList.end() )
      {
         // Yup.  Walk the list
         waitList_T::iterator sit;
         for( sit=wit->second.begin(); sit != wit->second.end(); sit++ )
         {
            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - Passing message type= %d to waiter", msgtype);
            }

            // Give them a copy of the message
            (*sit)->queue.push_back( msg->clone() );
            used = true;

            if( parent->parent->debug & objcomms::DEBUG_RECV )
            {
               INFORM("posix_udp_objcomms::disperser::run - Waiter returned", msgtype);
            }

            // If their queue was empty, signal them
            if( (*sit)->queue.size() == 1 )
            {
               (*sit)->signaled.signal();
            }
         }
      }

      // Free our lock on the waiter list
      parent->waitList_guard.unlock();

      // Warn about no subscribers or waiters.
      if( !used && parent->parent->debug & objcomms::DEBUG_RECV )
      {
         INFORM("posix_udp_objcomms::disperser::run - Discarding message of type %d because no subscribers or waiters", msgtype);
      }

      // done with it.
      delete msg;
      msg = NULL;
      delete info;
      info = NULL;
   }

   if( parent->parent->debug & objcomms::DEBUG_RECV )
   {
      INFORM("posix_udp_objcomms::disperser::run - disperser thread exiting");
   }
}

// *********************************************************************
posix_udp_objcomms::transmitter::transmitter(posix_udp_objcomms *ourParent) :
parent(ourParent),
exiting(false)
{ /* empty */ }

// ******************************************************************
void 
posix_udp_objcomms::transmitter::shutdown()
{
   if( parent->parent->debug & objcomms::DEBUG_TRNS )
   {
      INFORM("posix_udp_objcomms::transmitter::shutdown - called");
   }

   // Tell the thread to quit
   exiting = true;

   // Wake up the thread if it is waiting on the signal
   parent->outputData_changed.signal();
}

// ******************************************************************
void
posix_udp_objcomms::transmitter::run(int)
{
   if( parent->parent->debug & objcomms::DEBUG_TRNS )
   {
      INFORM("posix_udp_objcomms::transmitter::run - starting");
   }

   // Duck out if sockets not open.
   if( parent->udpsock < 0 )
   {
      ERROR("posix_udp_objcomms::transmitter::run - udpsock socket is not open in transmitter thread!");
      return;
   }
   if( !parent->multicastIsUnicast && parent->mulsock < 0 )
   {
      ERROR("posix_udp_objcomms::transmitter::run - mulsock socket is not open in transmitter thread!");
      return;
   }

   // A buffer to hold the raw outbound packetHeader
   PacketHeader header;
   memset(&header, 0, sizeof(PacketHeader));

   // set our network name in the outbound headers
   strncpy(header.senderID, parent->getName().c_str(),PacketHeader::MAXIDSIZE);
   struct sockaddr_in ourAddr = parent->getAddr().buildSockaddr_in();
   header.IPaddress = ourAddr.sin_addr.s_addr;
   header.port = ourAddr.sin_port;

   if( parent->parent->debug & objcomms::DEBUG_TRNS )
   {
      INFORM("posix_udp_objcomms::transmitter::run - Labeling packets with (in network byte order) port=%d", header.port);
   }

   // buffers for data sending out the wire
   static const int BUFSIZE = PacketHeader::MAXDATASIZE + PacketHeader::HEADERSIZE + 16;
   byte buf1[BUFSIZE];

#if defined(SUPPORT_ENCRYPTION)
   encryption *encryptor = new encryption_evp(parent->encryptionKey);
   byte buf2[BUFSIZE];
#endif

   // A sequence number for messages we send
   byte outputMsgNumber = 0;

   /// build us a local instance of the multicast address.
   sockaddr_in networkAddr;
   memcpy(&networkAddr, &parent->multicast_addr, sizeof(struct sockaddr_in));

   // The main run loop
   while( !exiting )
   {
      // Get a lock on the queue
      parent->outputData_guard.lock();

      // Wait for a message to show up
      while( parent->outputData.size() == 0 )
      {
         parent->outputData_changed.wait(parent->outputData_guard);
      
         // Is this an exit wakeup?
         if( exiting )
         {
            parent->outputData_guard.unlock();
            break;
         }
      }
      
      // Are we exiting?
      if( exiting )
      {
         parent->outputData_guard.unlock();
         break;
      }

      // Remove the first one from the queue
      outputMsgInfo *info = parent->outputData.front();
      parent->outputData.pop_front();

      // Signal that we changed the queue
      parent->outputData_changed.signal();

      // Free our lock on the queue
      parent->outputData_guard.unlock();

      // set the compressed flag in the header
      header.isCompressed = info->isCompressed;

      // get a pointer to the data
      string data = info->dataStream->str();
      uint size = data.size();

      if( info->isDirected )
      {
         // If it fits in a single packet, send it as reliable unicast UDP.
	 // If it is larger than that, use TCP.
         if( size <= PacketHeader::MAXDATASIZE )
	 {
            // *********************** Use UDP to send the message ******************
	    byte *pdata = (byte *)data.data();

	    // Setup the packet header
	    header.msgNumber = outputMsgNumber ++;
	    header.totalPackets = 1;
	    header.dataSize = htonl(size);
            header.packetIndex = 0;

            if( parent->parent->debug & objcomms::DEBUG_TRNS )
            {
               int pi = header.packetIndex;
               int tp = header.totalPackets;
               int ds = ntohl(header.dataSize);
               int ms = header.msgNumber;
               INFORM("posix_udp_objcomms::transmitter - sending packet with header: ");
               PRINT("   header.packetIndex = %d", pi);
               PRINT("   header.totalPackets = %d",tp);
               PRINT("   header.dataSize = %d",ds);
               PRINT("   header.msgNumber = %d\n",ms);
            }

            // Copy the data to the buffer
            byte *buf = &buf1[0];

	    // write the packetheader to the buffer
	    // can't just memcpy, since structure is not packed.
//            memcpy(buf, (byte *)&header, PacketHeader::HEADERSIZE);
            writePacketHeader((char *)buf, &header);

            memcpy(&buf[PacketHeader::HEADERSIZE], pdata, size);

            // Encrypt the header
#if defined(SUPPORT_ENCRYPTION)
            if( parent->encryptData )
            {
               int rtn = encryptor->encryptBuffer(buf, buf2, PacketHeader::HEADERSIZE);
               if( rtn == -1 )
               {
                  // bad buffer size!
                  ERROR("Message header size of %d bytes is not a multiple of the encryption block size!", PacketHeader::HEADERSIZE);
                  continue;
               }
               if( rtn < 0 )
               {
                  // encryption error
                  ERROR("Unable to encrypt message header!");
                  continue;
               }

               // copy the encrypted data back over
               memcpy(buf, buf2, PacketHeader::HEADERSIZE);
            }
#endif

            // Compute the size of the entire packet
            int packetSize = PacketHeader::HEADERSIZE + size;

            // if debugging, dump it.
            if( parent->parent->debug & objcomms::DEBUG_RAW )
            {
               INFORM("posix_udp_objcomms::transmitter::run - UDP unicast send of %d byte packet (network format):", packetSize);
               int pos = 0;
               const int numPerLine(8);
               unsigned char chars[numPerLine];
               for(int i=0; i<packetSize; i++)
               {
                  PRINT(" 0x%2.2x", buf[i]);
                  chars[pos++] = buf[i];
                  if( pos >= numPerLine )
                  {
                     PRINT("\t");
                     for(int i=0; i<numPerLine; i++)
                     {
                        if( isprint(chars[i]) )
                           PRINT("%c", chars[i]);
                        else
                           PRINT(".");
                     }
                     PRINT("\n");
                     pos = 0;
                  }
               }
               if( pos > 0 )
               {
                  for(int i=pos; i<numPerLine; i++)
		  {
                     PRINT("     ");
		  }

                  PRINT("\t");
                  for(int i=0; i<pos; i++)
                  {
                     if( isprint(chars[i]) )
                        PRINT("%c", chars[i]);
                     else
                        PRINT(".");
                  }
                  PRINT("\n");
               }
            }

	    // build the destination address
	    struct sockaddr_in dest = info->dest.buildSockaddr_in();

#if defined(linux)
            if( sendto(parent->udpsock, buf, packetSize, 0, (sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0 )
#elif defined(WIN32)
            if( sendto(parent->udpsock, (const char *)buf, packetSize, 0, (sockaddr *)&dest, sizeof(struct sockaddr_in)) < 0 )
#endif
            {
               ERROR_with_perror("Unable to UDP unicast message to %s",info->dest.str().c_str());
            }

            // Update the counters
            parent->parent->packetsTrn ++;
            parent->parent->bytesTrn += packetSize;
	 }
	 else
	 {
            // *********************** Use TCP to send the message ******************
	    // check if we already have an open tcp connection to the host
	    conInfo *con = NULL;
	    conInfoContainer::iterator it = parent->cons.find( info->dest );
	    if( it == parent->cons.end() )
	    {
	       // if not, open one (non-blocking) and set it to autoclose in AUTOCLOSE seconds
	       con = new conInfo();
	       con->sd = socket(PF_INET, SOCK_STREAM, 0);
	       if( con->sd < 0 )
	       {
#if defined(WIN32)
                  int err = WSAGetLastError();
                  ERROR("posix_udp_objcomms::transmitter::run - Received error %d while trying to open a tcp socket to %s", err, con->dest.str().c_str());
#else
                  ERROR_with_perror("posix_udp_objcomms::transmitter::run - Received error while trying to open a tcp socket to %s", con->dest.str().c_str());
#endif

		  // unable to create the socket!
		  parent->parent->packetsTrnDiscard++;
		  delete info->dataStream;
		  delete info;
		  continue;
	       }

	       // load the destination address
	       con->dest = info->dest;
	       con->addr = con->dest.buildSockaddr_in();
	       con->addr_len = sizeof(con->addr);
                                                                                
	       // open the connection
	       if( connect(con->sd, (struct sockaddr *)&con->addr, con->addr_len) < 0 )
	       {
#if defined(WIN32)
                  int err = WSAGetLastError();
                  ERROR("posix_udp_objcomms::transmitter::run - Received error %d while trying to connect a tcp stream to %s", err, con->dest.str().c_str());
#else
                  ERROR_with_perror("posix_udp_objcomms::transmitter::run - Received error while trying to connect a tcp stream to %s", con->dest.str().c_str());
#endif

		  // unable to connect the stream!
		  parent->parent->packetsTrnDiscard++;
		  delete info->dataStream;
		  delete info;
		  continue;
	       }

	       // Add it to our list of open connections
	       parent->cons_mutex.lock();
	       parent->cons[con->dest] = con;
	       parent->cons_mutex.unlock();

	       // wake up the receiver so it will notice the new receiver socket
	       char c = '\0';
	       if( send(parent->pipe_input, &c, 1, 0) < 0 )
	       {
		  perror("posix_udp_objcomms::transmitter::run - Error writing to transmit/receive pipe");
	       }

	       if( parent->parent->debug & objcomms::DEBUG_TRNS )
	       {
                  stringstream str;
                  str <<  con->dest.str();
		  INFORM("posix_udp_objcomms::transmitter::run - opened connection to %s", str.str().c_str() );
	       }
	    }
	    else
	    {
	       con = it->second;
	    }
        
	    // Setup the packet header
	    header.msgNumber = outputMsgNumber ++;
	    header.totalPackets = 1;
	    header.packetIndex = 0;
	    header.dataSize = htonl(data.size());

	    // copy the header to a buffer to get ready for writing
	    // can't just use the structure, since isn't packed
	    byte headbuf[PacketHeader::HEADERSIZE];
	    writePacketHeader((char *)headbuf, &header);

	    byte *buf = headbuf;
         
	    // Encrypt the header
#if defined(SUPPORT_ENCRYPTION)
	    if( parent->encryptData )
	    {
	       int rtn = encryptor->encryptBuffer(buf, buf2, PacketHeader::HEADERSIZE);
	       if( rtn == -1 )
	       {
		  // bad buffer size!
		  ERROR("Message header size of %d bytes is not a multiple of the encryption block size!", PacketHeader::HEADERSIZE);
		  continue;
	       }
	       if( rtn < 0 )
	       {
		  // encryption error
		  ERROR("Unable to encrypt message header!");
		  continue;
	       }

	       // Switch to the encrypted buffer
	       buf = buf2;
	    }
#endif
	    // send it
	    int cnt = send(con->sd, (char *)buf, PacketHeader::HEADERSIZE, 0);
	    if( cnt < 0 )
	    {
#if defined(WIN32)
	       int err = WSAGetLastError();
	       ERROR("posix_udp_objcomms::transmitter::run - Received error %d while trying to write the header to a tcp stream connected to %s", err, con->dest.str().c_str());
#else
	       ERROR_with_perror("posix_udp_objcomms::transmitter::run - Received error while trying to write the header to a tcp stream connected to %s", con->dest.str().c_str());
#endif
	       continue;
	    }
	    if( cnt != PacketHeader::HEADERSIZE )
	    {
	       ERROR("Only wrote %d bytes of %d byte header to tcp stream", cnt, PacketHeader::HEADERSIZE);
	       continue;
	    }

	    // send it
	    byte *pdata = (byte *)data.data();
	    uint size = data.size();
	    while( size > 0 )
	    {
	       int cnt = send(con->sd, (char *)pdata, size, 0);
	       if( cnt < 0 )
	       {
#if defined(WIN32)
		  int err = WSAGetLastError();
		  ERROR("posix_udp_objcomms::transmitter::run - Received error %d while trying to write data to a tcp stream connected to %s", err, con->dest.str().c_str());
#else
		  ERROR_with_perror("posix_udp_objcomms::transmitter::run - Received error while trying to write data to a tcp stream connected to %s", con->dest.str().c_str());
#endif
		  continue;
	       }

	       // step past what got wrote this pass
	       size -= cnt;
	       pdata += cnt;
	    }
	 }
      }
      else
      {
	 // ******************* Not directed.  So, multicast it. ********************
         byte *pdata = (byte *)data.data();
         uint size = data.size();

         // compute the number of packets required
         int totalPackets = (size + PacketHeader::MAXDATASIZE - 1) / PacketHeader::MAXDATASIZE;
         if( totalPackets > 255 )
         {
            ERROR("Message of %d bytes is to large to transmit!", size);
            continue;
         }

         // Setup the packet header
         header.msgNumber = outputMsgNumber ++;
         header.totalPackets = totalPackets;
         header.dataSize = htonl(size);

         // For each packet
         uint pos = 0;
         int packetNumber = 0;
         while( pos < size )
         {
            // Build the next packet
            header.packetIndex = packetNumber ++;
            int dataSize = min((int)PacketHeader::MAXDATASIZE, (int)(size - pos));

            if( parent->parent->debug & objcomms::DEBUG_TRNS )
            {
               int pi = header.packetIndex;
               int tp = header.totalPackets;
               int ds = ntohl(header.dataSize);
               int ms = header.msgNumber;
               INFORM("posix_udp_objcomms::transmitter - multicasting packet with header: ");
               PRINT("   header.packetIndex = %d",pi);
               PRINT("   header.totalPackets = %d",tp);
               PRINT("   header.dataSize = %d",ds);
               PRINT("   header.msgNumber = %d",ms);
            }

            // Copy the data to a buffer
            byte *buf = &buf1[0];
	    // can't just memcpy since the structure is not packed.
            writePacketHeader((char *)buf, &header);
//            memcpy(buf, (byte *)&header, PacketHeader::HEADERSIZE);
            memcpy(&buf[PacketHeader::HEADERSIZE], &pdata[pos], dataSize);

            // Encrypt the header
#if defined(SUPPORT_ENCRYPTION)
            if( parent->encryptData )
            {
               int rtn = encryptor->encryptBuffer(buf, buf2, PacketHeader::HEADERSIZE);
               if( rtn == -1 )
               {
                  // bad buffer size!
                  ERROR("Message header size of %d bytes is not a multiple of the encryption block size!", PacketHeader::HEADERSIZE);
                  continue;
               }
               if( rtn < 0 )
               {
                  // encryption error
                  ERROR("Unable to encrypt message header!");
                  continue;
               }

               // copy the encrypted data back over
               memcpy(buf, buf2, PacketHeader::HEADERSIZE);
            }
#endif

            // step past this block in the user data
            pos += dataSize;

            // Compute the size of the entire packet
            int packetSize = PacketHeader::HEADERSIZE + dataSize;

            // if debugging, dump it.
            if( parent->parent->debug & objcomms::DEBUG_RAW )
            {
               INFORM("posix_udp_objcomms::transmitter::run - sending %d byte packet (network format):", packetSize);
               int pos = 0;
               const int numPerLine(8);
               unsigned char chars[numPerLine];
               for(int i=0; i<packetSize; i++)
               {
                  PRINT(" 0x%2.2x", buf[i]);
                  chars[pos++] = buf[i];
                  if( pos >= numPerLine )
                  {
                     PRINT("\t");
                     for(int i=0; i<numPerLine; i++)
                     {
                        if( isprint(chars[i]) )
                           PRINT("%c", chars[i]);
                        else
                           PRINT(".");
                     }
                     PRINT("\n");
                     pos = 0;
                  }
               }
               if( pos > 0 )
               {
                  for(int i=pos; i<numPerLine; i++)
		  {
                     PRINT("     ");
		  }

                  PRINT("\t");
                  for(int i=0; i<pos; i++)
                  {
                     if( isprint(chars[i]) )
                        PRINT("%c", chars[i]);
                     else
                        PRINT(".");
                  }
                  PRINT("\n");
               }
            }

            if( parent->multicastIsUnicast )
            {
               // loop through the list of robots and send a copy of the message to each one.
               for(uint msg=0; msg<parent->teamAddrs.size(); ++msg)
               {
                  if( msg > 0 )
                  {
                     // yield the cpu to let the last message get out
                     // otherwise, (I think) we overwrite pending packets
                     thread_yield();
                  }

                  if( parent->parent->debug & objcomms::DEBUG_TRNS )
                  {
                     stringstream str;
                     str <<  parent->teamAddrs[msg].address.str();
                     INFORM("UDP multicast as unicast sendto %s", str.str().c_str());
                  }

                  sockaddr_in sa = parent->teamAddrs[msg].address.buildSockaddr_in();
#if defined(linux)
                  if( sendto(parent->udpsock, buf, packetSize, 0, (sockaddr *)&sa, sizeof(networkAddr)) < 0 )
#elif defined(WIN32)
                  if( sendto(parent->udpsock, (const char *)buf, packetSize, 0, (sockaddr *)&sa, sizeof(networkAddr)) < 0 )
#else
#error "undefined architecture"
#endif
                  {
#if defined(linux)
                     ERROR_with_perror("Unable to send multicast as unicast message to %s", parent->teamAddrs[msg].address.str().c_str());
#elif defined(WIN32)
                     int err = WSAGetLastError();
                     ERROR("Received error %d trying to multicast message to %s", err, parent->teamAddrs[msg].address.str().c_str());
#else
#error "undefined architecture"
#endif
                  }
               }
            }
            else
            {
               // Loop through the ports, in case multiple robots are on the host.
               int port = parent->base_port;
               for( uint inst=0; inst<parent->num_instances_per_host; inst++, port++ )
               {
                  if( inst > 0 )
                  {
                     // yield the cpu to let the last message get out
                     // otherwise, (I think) we overwrite pending packets
                     thread_yield();
                  }

                  networkAddr.sin_port = htons(port);

                  if( parent->parent->debug & objcomms::DEBUG_TRNS )
                  {
                     INFORM("UDP multicast sendto %s:%d", inet_ntoa( networkAddr.sin_addr ), ntohs(networkAddr.sin_port));
                  }

	          // we use mulsock for multicast writes since it has special socket options
	          // set in open to increase the TTL, etc.
#if defined(linux)
                  if( sendto(parent->mulsock, buf, packetSize, 0, (sockaddr *)&networkAddr, sizeof(networkAddr)) < 0 )
#elif defined(WIN32)
                  if( sendto(parent->mulsock, (const char *)buf, packetSize, 0, (sockaddr *)&networkAddr, sizeof(networkAddr)) < 0 )
#else
#error "undefined architecture"
#endif
                  {
#if defined(linux)
	             string straddr( inet_ntoa(networkAddr.sin_addr) );
                     ERROR_with_perror("Unable to multicast message to %s:%d", straddr.c_str(), port);
#elif defined(WIN32)
                     int err = WSAGetLastError();
   	             string straddr( inet_ntoa(networkAddr.sin_addr) );
                     ERROR("Received error %d trying to multicast message to %s:%d", err, straddr.c_str(), port);
#else
#error "undefined architecture"
#endif
                  }
               }
            }

            // Update the counters
            parent->parent->packetsTrn ++;
            parent->parent->bytesTrn += packetSize;
         }
      }

      // done sending this message
      delete info->dataStream;
      delete info;
   }

   if( parent->parent->debug & objcomms::DEBUG_TRNS )
   {
      INFORM("posix_udp_objcomms::transmitter::run - Shutting down transmitter");
   }
}

// *********************************************************************
posix_udp_objcomms::receiver::receiver(posix_udp_objcomms *ourParent) :
parent(ourParent),
exiting(false)
#if defined(SUPPORT_ENCRYPTION)
,encryptor(NULL)
#endif
{
}

// ******************************************************************
void 
posix_udp_objcomms::receiver::shutdown()
{
   if( parent->parent->debug & objcomms::DEBUG_RECV )
   {
      INFORM("posix_udp_objcomms::receiver::shutdown - called");
   }

   // Tell the thread to quit
   exiting = true;

   // Wake up the thread if it is waiting on input data
   char ch = 'T';
   send(parent->pipe_input, &ch, 1, 0);
}

// *********************************************************************
/// Handle a newly arrived packet 
/// sd - the socket descriptor to read the packet from
/// addr - the sender's address to note in the message
/// isStream - true if transport queue messages (tcp), 
///            false if next message replaces this one (udp)
///            Used to prevent race conditions by reading the 
///            entire message in one chunk for UDP
bool 
posix_udp_objcomms::receiver::handle_packet(int sd, const bool isStream)
{
   if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
   {
      INFORM("posix_udp_objcomms::handle_packet - starting");
   }

   PacketHeader header;
   enum {BUFSIZE=2048};
   byte buf[BUFSIZE];
   stringstream *str = new stringstream;

   // if not a stream, read the packet in one read
   if( !isStream )
   {
      int cnt = recv(sd, (char *)buf, BUFSIZE, 0);
      if( cnt < PacketHeader::HEADERSIZE + 1 )
      {
         parent->parent->packetsRcvDiscard ++;
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            INFORM("posix_udp_objcomms::handle_packet - returning FALSE due to failure to read udp packet header. only read %d bytes instead of %d", cnt, (int)(PacketHeader::HEADERSIZE + 1));
         }
         return false;
      }
      // can't just memcpy since the structure is not packed.
      readPacketHeader((char *)buf, &header);
//      memcpy((byte *)&header, buf, PacketHeader::HEADERSIZE);
      cnt -= PacketHeader::HEADERSIZE;
      str->write((char *)&buf[PacketHeader::HEADERSIZE], cnt);
   }
   // is a stream, so read header and then blocks to get entire message
   else
   {
      // first read the header to find out the size of the message

      // can't just memcpy since the structure is not packed.
      //int cnt = recv(sd, (char *)&header, PacketHeader::HEADERSIZE, 0);
      int cnt = recv(sd, (char *)buf, PacketHeader::HEADERSIZE, 0);
      readPacketHeader((char *)buf, &header);

      if( cnt < PacketHeader::HEADERSIZE )
      {
         parent->parent->packetsRcvDiscard ++;
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            INFORM("posix_udp_objcomms::handle_packet - returning FALSE due to failure to read header from tcp stream");
         }
         return false;
      }
   }

   // decrypt the header, if necessary
#if defined(SUPPORT_ENCRYPTION)
   if( parent->encryptData )
   {
      if( encryptor == NULL )
      {
         // Create an encryption object
         encryptor = new encryption_evp(parent->encryptionKey);
      }

      // decrypt the header
      byte buf[PacketHeader::HEADERSIZE];

      // can't just memcpy since the structure is not packed.
      byte hdrbuf[PacketHeader::HEADERSIZE];
      writePacketHeader(hdrbuf, &header);


//      int rtn = encryptor->decryptBuffer((byte *)&header, buf, PacketHeader::HEADERSIZE);
      int rtn = encryptor->decryptBuffer(hdrbuf, buf, PacketHeader::HEADERSIZE);
      if( rtn == -1 )
      {
         // bad buffer size!
         ERROR("Message header size of %d bytes is not a multiple of the encryption block size!", PacketHeader::HEADERSIZE);
         return false;
      }
      if( rtn < 0 )
      {
         // encryption error
         ERROR("Unable to decrypt message header!");
         return false;
      }

      readPacketHeader(buf, &header);
//      memcpy(&header, buf, PacketHeader::HEADERSIZE);
   }
#endif

   // now read the data if it was a stream
   if( isStream )
   {
      int numleft = ntohl(header.dataSize);
      while( numleft > 0 )
      {
         int thispass = min((int)BUFSIZE, numleft);
         int cnt = recv(sd, (char *)buf, thispass, 0);
         if( cnt < 1 )
         {
            parent->parent->packetsRcvDiscard ++;

            if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
            {
               INFORM("posix_udp_objcomms::handle_packet - returning FALSE due to failure to read from tcp stream");
            }

            return false;
         }
         if( cnt > 0 )
         {
            numleft -= cnt;
            str->write((char *)buf, cnt);
         }
      }
   }

   // if debugging, dump the message.
   if( parent->parent->debug & objcomms::DEBUG_RAW )
   {
      INFORM("posix_udp_objcomms::receiver::handle_packet - received %d byte packet (network format):", str->str().size());

      // make a copy, seekg doesn't seem to work
      stringstream localstr(str->str());

INFORM("posix_udp_objcomms::receiver::handle_packet - (in network byte order) header.port=%d", header.port);

ComAddr ca( header.IPaddress, header.port);

      // build the senderName from a null terminated char buffer
      char idbuf[PacketHeader::MAXIDSIZE+1];
      strncpy(idbuf, header.senderID, PacketHeader::MAXIDSIZE);
      idbuf[PacketHeader::MAXIDSIZE] = '\0';
      string senderID = idbuf;

      PRINT("senderID='%s' IPaddress:port=%s dataSize=%ld msgNumber=%u packetIndex=%u totalPackets=%u isCompressed=%d\n", senderID.c_str(), ca.str().c_str(), ntohl(header.dataSize), header.msgNumber, header.packetIndex, header.totalPackets, header.isCompressed);

      const int numPerLine(8);
      unsigned char chars[numPerLine];
      int pos = 0;
      byte hdrbuf[PacketHeader::HEADERSIZE];
      writePacketHeader((char *)hdrbuf, &header);
//      byte *p = (byte *)&header;
      byte *p = hdrbuf;
      for(int i=0; i<PacketHeader::HEADERSIZE; i++)
      {
         chars[pos++] = *p;
         PRINT(" 0x%2.2x", *p++);
         if( pos >= numPerLine )
         {
            PRINT("\t");
            for(int i=0; i<numPerLine; i++)
            {
               if( isprint(chars[i]) )
                  PRINT("%c", chars[i]);
               else
                  PRINT(".");
            }
            PRINT("\n");
            pos = 0;
         }
      }

      int size = 0;
      int ch;
      while( (ch=localstr.get()) != EOF )
      {
         size++;
         PRINT(" 0x%2.2x", ch);
         chars[pos++] = ch;
         if( pos >= numPerLine )
         {
            PRINT("\t");
            for(int i=0; i<numPerLine; i++)
            {
               if( isprint(chars[i]) )
                  PRINT("%c", chars[i]);
               else
                  PRINT(".");
            }
            PRINT("\n");
            pos = 0;
         }
      }
      PRINT("\n");
   }

   // Update the counters
   parent->parent->packetsRcv ++;
   parent->parent->bytesRcv += PacketHeader::HEADERSIZE + ntohl(header.dataSize);

   // Holds the message
   inputMsgInfo *info = NULL;

   // build the senderName from a null terminated char buffer
   char idbuf[PacketHeader::MAXIDSIZE+1];
   strncpy(idbuf, header.senderID, PacketHeader::MAXIDSIZE);
   idbuf[PacketHeader::MAXIDSIZE] = '\0';
   string senderID = idbuf;

   // build the senderAddr from the header
   ComAddr senderAddr( header.IPaddress, header.port );

   // set our network name in the outbound headers
   strncpy(header.senderID, parent->getName().c_str(),PacketHeader::MAXIDSIZE);
   struct sockaddr_in ourAddr = parent->getAddr().buildSockaddr_in();
   header.IPaddress = ourAddr.sin_addr.s_addr;
   header.port = ourAddr.sin_port;


   // Is it invalid?
   if( header.totalPackets == 0 )
   {
      WARN("Dropping message: totalPackets=0");
   }
   // Is this a one-packet message?
   // If so, lets speed up its handling
   else if( header.totalPackets == 1 )
   {
      // Create the msgInfo record
      info = new inputMsgInfo();
      info->isCompressed = header.isCompressed;

      // attach the datastream
      info->dataStream = str;

      // copy over the senderID 
      info->senderID = senderID;
      info->senderAddr = senderAddr;
   }
   else
   {
      // Otherwise, try to find any other packets for the message in our storage.
      StorageEntry *entry = NULL;

      // If this is first packet in this message, prepare the entry.
      Storage_T::iterator it = storage.find( senderID );
      if( it == storage.end() )
      {
         // Create and init an entry.
         entry = new StorageEntry();
         entry->msgNumber = header.msgNumber;
         entry->totalPackets = header.totalPackets;
         entry->receivedPackets = 0;

         // make sure is long enough
         while( entry->packets.size() < entry->totalPackets )
         {
            entry->packets.push_back(NULL);
         }

         storage[senderID] = entry;
      }
      else
      {
         entry = it->second;

         // If existing entry is for a different message, 
         // discard that old data and re-init it.
         if( entry->msgNumber != header.msgNumber )
         {
            // Delete the old data
            for( uint i=0; i<entry->packets.size(); i++ )
            {
               if( entry->packets[i] != NULL )
               {
                  delete entry->packets[i];
                  entry->packets[i] = NULL;
               }
            }
            entry->msgNumber = header.msgNumber;
            entry->totalPackets = header.totalPackets;
            entry->receivedPackets = 0;

            // make sure is long enough
            while( entry->packets.size() < entry->totalPackets )
            {
               entry->packets.push_back(NULL);
            }
         }
      }

      // Make sure the header values are reasonable
      if( header.packetIndex > entry->packets.size() ||
          header.totalPackets != entry->packets.size() ||
          entry->packets[ header.packetIndex ] != NULL )
      {
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            int pi = header.packetIndex;
            int tp = header.totalPackets;
            INFORM("posix_udp_objcomms::handle_packet - returning FALSE due to corrupt header");
            PRINT("header.packetIndex = ",pi);
            PRINT("entry->packets.size = ",entry->packets.size());
            PRINT("header.totalPackets = ",tp);
            bool b = entry->packets[header.packetIndex] != NULL;
            PRINT("entry->packets[packetIndex]!=NULL  = ",b );
         }

         parent->parent->packetsCorrupted ++;
         return false;
      }

      // create the packet
      Packet *packet = new Packet();
      memcpy(&packet->header, &header, sizeof(PacketHeader));
      packet->dataStream = str;

      // attach it.
      entry->packets[ packet->header.packetIndex ] = packet;
      entry->receivedPackets ++;

      // If the message is complete, reassemble it.
      if( entry->receivedPackets == entry->totalPackets )
      {
         // Create the msgInfo record
         info = new inputMsgInfo();
         info->senderID = senderID;
         info->senderAddr = senderAddr;
         info->dataStream = new stringstream();

         // process the packets
         Packet *pck;
         for( int i=0; i< entry->totalPackets; i++ )
         {
            pck = entry->packets[i];
            if( pck == NULL )
            {
               ERROR("Internal Error: Missing packet %d while assembling message", i);
               delete info;

               if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
               {
                  INFORM("posix_udp_objcomms::handle_packet - returning FALSE due to missing packet %d of %d", i, entry->totalPackets);
               }

               return false;
            }

            // if packet 0, set the compressed flag
            if( i == 0 )
            {
               info->isCompressed = pck->header.isCompressed;
            }

            // append the data
            string str = pck->dataStream->str();
            info->dataStream->write( str.data(), str.size() );

            // delete the used packet data
            delete entry->packets[i];
            entry->packets[i] = NULL;
         }
      }
   }

   // if we built a message, send it out.
   if( info )
   {
      if( parent->parent->debug & objcomms::DEBUG_RECV )
      {
         INFORM("posix_udp_objcoms::receiver::handle_packet - queuing received message");
      }

      // Get a lock on the queue
      parent->inputData_guard.lock();

      // If there is no room in the message queue, then drop the message
      if( parent->inputData.size() > inputData_MAXSIZE )
      {
         // Update the counter
         parent->parent->packetsRcvDiscard ++;

         WARN("Dropping message: input queue is full");
         delete info;
      }
      else
      {
         // Add it to the end of the queue
         parent->inputData.push_back( info );

         // Signal that we changed the queue
         parent->inputData_changed.signal();
      }

      info = NULL;

      // Free our lock on the queue
      parent->inputData_guard.unlock();
   }

   if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
   {
      INFORM("posix_udp_objcomms::handle_packet - returning success");
   }

   // done with this packet
   return true;
}

// *********************************************************************
/// Main thread for the receiver
void 
posix_udp_objcomms::receiver::run(int)
{
   if( parent->parent->debug & objcomms::DEBUG_RECV )
   {
      INFORM("posix_udp_objcomms::receiver::run - starting");
   }
/*
   // Duck out if socket not open.
   if( parent->udpsock < 0 )
   {
      ERROR("posix_udp_objcomms::receiver::run - udpsock socket is not open in receiver thread");
      return;
   }
*/
   // used to wait for input in select
   fd_set rfds;

   // used to hold closed sockets needing removal
   typedef vector<ComAddr> closedCons_T;
   closedCons_T closedCons;
   closedCons_T::iterator ccit;

   // an empty address used for arriving broadcast packets
   ComAddr noAddress;

   // The main run loop
   while( !exiting )
   {
      if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
      {
         INFORM("posix_udp_objcomms::receiver::run - listening on:");
      }

      // Enter all our open sockets into the descriptor set
      FD_ZERO(&rfds);
      SOCKET highest = 0;

      if( parent->udpsock_isBound )
      {
         FD_SET(parent->udpsock, &rfds);
         if( parent->udpsock > highest )
            highest = parent->udpsock;
      }

      if( parent->receive_multicast )
      {
         FD_SET(parent->mulsock, &rfds);
         if( parent->mulsock > highest )
            highest = parent->mulsock;
      }

      if( parent->tcpsock != -1 )
      {
         FD_SET(parent->tcpsock, &rfds);
         if( parent->tcpsock > highest )
            highest = parent->tcpsock;
      }

      // transmit/receiver pipe
      FD_SET(parent->pipe_output, &rfds);
      if( parent->pipe_output > highest )
         highest = parent->pipe_output;

      parent->cons_mutex.lock();
      conInfoContainer::iterator it;
      for( it=parent->cons.begin(); it!=parent->cons.end(); ++it )
      {
         FD_SET(it->second->sd, &rfds);
         if( it->second->sd > highest )
            highest = it->second->sd;

         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            PRINT("\tTCP connection to %s", it->second->dest.str().c_str());
         }
      }
      parent->cons_mutex.unlock();

      // wait for something to arrive
      int rtn = select(highest+1, &rfds, NULL, NULL, NULL);

      // Are we exiting?
      if( exiting )
         break;      

      if( rtn < 0 )
      {
         perror("Error from select");
         exit(3);
      }
      if( rtn > 0 )
      {
         if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
         {
            INFORM("receive got something ");
         }

         // check our udp socket
         if( parent->udpsock_isBound && FD_ISSET(parent->udpsock, &rfds) )
         {
            // Got one: handle it
            if( !handle_packet(parent->udpsock, false) )
            {
               if( parent->parent->debug & objcomms::DEBUG_RECV )
               {
                  INFORM("posix_udp_objcoms::receiver::run - corrupt packet reported by handle_packet");
               }
            }
            if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
            {
               INFORM("posix_udp_objcoms::receiver::run - Received broadcast packet");
            }
         }

         // check our multicast socket
         if( parent->receive_multicast )
         {
            if( FD_ISSET(parent->mulsock, &rfds) )
            {
               // Got one: handle it
               if( !handle_packet(parent->mulsock, false) )
               {
                  if( parent->parent->debug & objcomms::DEBUG_RECV )
                  {
                     INFORM("posix_udp_objcoms::receiver::run - corrupt packet reported by handle_packet");
                  }
               }
               if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
               {
                  INFORM("posix_udp_objcoms::receiver::run - Received broadcast packet");
               }
            }
         }

         // check our pipe socket and discard any waiting data
         if( FD_ISSET(parent->pipe_output, &rfds) )
         {
            if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
            {
               INFORM("posix_udp_objcoms::receiver::run - wakeup");
            }
            char buf[16];
            while( read(parent->pipe_output, buf, 16) > 0 );
         }

         // check our main tcp socket
         if( parent->tcpsock != -1 )
         {
            if( FD_ISSET(parent->tcpsock, &rfds) )
            {
               // go ahead and accept the connection, adding it to our list of open ones
               conInfo *newcon = new conInfo();
               newcon->sd = accept(parent->tcpsock, (sockaddr*)&newcon->addr, (socklen_t *)&newcon->addr_len);
               newcon->dest.load( &newcon->addr, newcon->addr_len );
               parent->cons_mutex.lock();
               parent->cons[newcon->dest] = newcon;
               parent->cons_mutex.unlock();

               if( parent->parent->debug & objcomms::DEBUG_RECV )
               {
                  INFORM("posix_udp_objcomms::receiver::run - opening connection %s", newcon->dest.str().c_str());
               }
            }

            // check our open tcp connections
            parent->cons_mutex.lock();
            for( it=parent->cons.begin(); it!=parent->cons.end(); ++it )
            {
               // is there something here?
               if( FD_ISSET(it->second->sd, &rfds) )
               {
                  if( parent->parent->debug & objcomms::DEBUG_RECV_DETAILS )
                  {
                     INFORM("posix_udp_objcoms::receiver::run - received tcp packet");
                  }
                  if( parent->parent->debug & objcomms::DEBUG_RECV )
                  {
                     PRINT(">");
                  }

                  // Got one: handle it
                  if( !handle_packet(it->second->sd, true) )
                  {
                     if( parent->parent->debug & objcomms::DEBUG_RECV )
                     {
                        INFORM("posix_udp_objcomms::receiver::run - EOF reading packet: The other end has closed connection %s", it->first.str().c_str());
                     }

                     // eof: the client has closed the connection
                     closedCons.push_back( it->first );
                     continue;
                  }
               }
            } 
            // if we marked any for deletion, remove them now.
            if( closedCons.size() > 0 )
            {
               for(ccit=closedCons.begin(); ccit!=closedCons.end(); ++ccit)
               {
                  if( parent->parent->debug & objcomms::DEBUG_RECV )
                  {
                     INFORM("posix_udp_objcomms::receiver::run - closing connection %s", ccit->str().c_str());
                  }
   
                  it = parent->cons.find( *ccit );
                  if( it != parent->cons.end() )
                  {
                     close( it->second->sd );
                     parent->cons.erase( it );
                  }
                  else
                  {
                     ERROR("posix_udp_objcomms::receiver::run - Unable to close tcp connection! %s", ccit->str().c_str() );
                  }
               }
               // clean out our deletion list.
               closedCons.erase(closedCons.begin(), closedCons.end());
            }
   
            parent->cons_mutex.unlock();
         }
      }
   }

   if( parent->parent->debug & objcomms::DEBUG_RECV )
   {
      INFORM("posix_udp_objcomms::receiver::run - Receiver thread exiting");
   }
}

// **********************************************************************
/// Begin queueing messages for us of the specified type
/// msgType is the type of message to wait for
/// Returns a queueing handle on success, NULL on failure.
/// NOTE: endQueueing must be called after a successful call to 
/// beginQueueing to stop the queueing process and free consumed resources
objcomms::QueueingHandle
posix_udp_objcomms::beginQueueing(commMsg::MsgType_T msgType)
{
   waitListEntry *entry = new waitListEntry();

   // remember the msgType
   entry->msgType = msgType;

   // Get the lock
   waitList_guard.lock();

   // put us on the wait list
   waitList[msgType].push_back( entry );

   // done with the lock
   waitList_guard.unlock();

   return entry;
}

// **********************************************************************
/// Suspend the thread until either a message arrives or the timeout expires
/// handle was returned from a call to beginQueueing
/// timeout is the maximum time to wait.
/// returns: The message, if one arrived
///          NULL if the timeout expired or the handle is invalid
commMsg *
posix_udp_objcomms::waitFor(objcomms::QueueingHandle handle, const TimeOfDay &timeout)
{
   // duck out if process is terminating
   if( exiting )
   {
      return NULL;
   }

   // Can't do anything without a handle
   if( handle == NULL )
   {
      ERROR("Internal Error: posix_udp_objcomms::waitFor called with NULL handle!");
      return NULL;
   }

   waitListEntry *entry = (waitListEntry *)handle;
   commMsg *rtn = NULL;

   // Get the lock
   waitList_guard.lock();

   // if there are no messages waiting, then sleep on the condition
   if( entry->queue.empty() )
   {
      // now wait for a message
      entry->signaled.wait(waitList_guard, timeout);

      // duck out if process is terminating
      if( exiting )
      {
         return NULL;
      }
   }

   // Do we have a message to return?
   if( !entry->queue.empty() )
   {
      rtn = entry->queue.front();
      entry->queue.pop_front();
   }

   // done with the lock
   waitList_guard.unlock();

   return rtn;
}

// **********************************************************************
/// Stop queueing messages for us 
/// Returns true on success, false if the handle is invalid
/// NOTE: endQueueing must be called after a successful call to 
/// beginQueueing to stop the queueing process and free consumed resources
bool 
posix_udp_objcomms::endQueueing(objcomms::QueueingHandle handle)
{
   // Can't do anything without a handle
   if( handle == NULL )
   {
      ERROR("Internal Error: posix_udp_objcomms::endQueueing called with NULL handle!");
      return false;
   }

   waitListEntry *entry = (waitListEntry *)handle;
   bool rtn = true;

   // Get the lock
   waitList_guard.lock();

   waitList_T::iterator theEnd = waitList[entry->msgType].end();
   waitList_T::iterator it = find(waitList[entry->msgType].begin(), theEnd, entry );
   if( it == theEnd )
   {
      ERROR("Internal Error: Unable to remove waitListEntry!");
      rtn = false;
   }
   else
   {
      waitList[entry->msgType].erase( it );
   }

   // done with the lock
   waitList_guard.unlock();

   // Get rid of the entry
   delete entry;

   // Did we get one?
   return rtn;
}

// **********************************************************************
}
