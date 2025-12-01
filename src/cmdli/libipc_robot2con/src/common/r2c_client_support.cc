/**********************************************************************
 **                                                                  **
 **  r2c_client_support.cc                                           **
 **                                                                  **
 **  support functions for the console rpc services (client side)    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: r2c_client_support.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: r2c_client_support.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:22:47  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.20  2004/05/11 19:34:36  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.19  2003/04/11 18:46:57  doug
// added export capabilities
//
// Revision 1.18  1999/03/11 22:24:40  doug
// working
//
// Revision 1.17  1998/05/13 22:14:00  doug
// win95 code didn't work on unix, so ifdef
//
// Revision 1.16  1998/05/13 22:04:58  doug
// win95 changes
//
// Revision 1.15  1998/02/23 15:53:37  doug
// added list_prims
//
// Revision 1.14  1998/02/18 15:06:22  doug
// *** empty log message ***
//
// Revision 1.13  1998/02/18 14:56:01  doug
// *** empty log message ***
//
// Revision 1.12  1998/02/18 14:10:07  doug
// added get_fsa
//
// Revision 1.11  1998/02/18 10:31:28  doug
// changed string .data() to .c_str() to get rid of trailing junk
//
// Revision 1.10  1998/02/13 13:38:33  doug
// WIN32 changes
//
// Revision 1.9  1998/02/10 14:44:58  doug
// added win32 init/exit code for RPC DLL
//
// Revision 1.8  1998/02/10 09:12:14  doug
// moved data defs to libipc_support
//
// Revision 1.7  1997/12/31 23:04:49  doug
// *** empty log message ***
//
// Revision 1.6  1997/12/31 03:04:05  doug
// *** empty log message ***
//
// Revision 1.5  1997/12/31 03:00:54  doug
// fixing for win32
//
// Revision 1.4  1997/12/17 15:57:53  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/17 15:50:09  doug
// rewrote
//
// Revision 1.2  1997/12/13 23:20:56  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/11 15:55:48  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#ifdef WIN32
   #include "winsock2.h"
#endif
#ifdef linux
   #include <netdb.h>
   #include <sys/socket.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>
#endif
#include "mic.h"
#include <ctype.h>
#include "r2c_defs.h"
#include "r2c_client_support.h"

using namespace sara;

/*********************************************************************/
// Free our rpc client data record
void
client_exit_r2c(CLIENT *clnt)
{
   if( clnt )
      clnt_destroy(clnt);

#ifdef WIN32
   // Shut down the RPC DLL.
   RPCExit();

	// Shutdown the sockets subsystem.
	WSACleanup();
#endif
}

/*********************************************************************/
// Initialize a client connection to the server.
//    host is the name the server is running on.
//    tries is the number of attempts to make.
//    timeout_secs is the number of seconds to set the timeout to.
//    offset is an offset added to the program number to handle multiple
//    robots per processor.
// Returns the CLIENT handle or NULL if unable to connect
CLIENT * 
client_start_r2c(const string &host, 
                 const int tries_requested,
		 const int timeout_secs,
		 const uint offset)
{
#ifdef WIN32

	// Startup the sockets subsystem
	WORD versionRequested = MAKEWORD(2,0);
	WSADATA wsaData;
	if( WSAStartup(versionRequested, &wsaData) != 0 )
	{
		ERROR("Unable to find matching sockets DLL");
		return NULL;
	}

	// We are asking for sockets version 2, if only get 1.1, then let if fly anyway.
#if 0
	if( wsaData.wVersion != versionRequested )
	{
		ERROR("Unable to find correct version of sockets");
		WSACleanup();
		return false;
	}
#endif

   INFORM("Using winsock version %d.%d", wsaData.wVersion & 0xff, wsaData.wVersion >> 8);

	// for windows check first, whether the portmapper is running or
	// not. If the portmapper is not running, we will have a timeout
	// of aproximately 60 seconds without having any possibility to
	// change the timeout for UPD connections.
	// For this reason, we first try to connect to the sunrpc/tcp
	// port to check the existance of the portmapper.

   hostent *hostnm;
	sockaddr_in myaddr;
	char hostname[256];
	int rtn;
	if( (rtn = gethostname(&hostname[0], 256)) != 0 )
	{
		if( rtn == SOCKET_ERROR )
		{
			int error = WSAGetLastError();
			switch(error)
			{
				case WSAEFAULT:
		       	ERROR("gethostname failed. WSAEFAULT");
			   	break;
				case WSANOTINITIALISED:
		       	ERROR("gethostname failed. WSANOTINITIALISED");
			   	break;
				case WSAENETDOWN:
		       	ERROR("gethostname failed. WSAENETDOWN");
			   	break;
				case WSAEINPROGRESS:
		       	ERROR("gethostname failed. WSAEINPROGRESS");
			   	break;
	
				default:
		       	ERROR("gethostname failed. Unknown error");
			   	break;
			}
		}
		else
		{
			ERROR("gethostname failed. Errorno = %s", strerror( errno ));
		}
			
	   return NULL;
	}

   hostnm = gethostbyname(hostname);
   if( hostnm == (struct hostent *) 0 ) 
	{
      ERROR("gethostbyname failed");
      return NULL;
   }
   myaddr.sin_addr.s_addr= *((u_long FAR *) hostnm->h_addr_list[0]);
   myaddr.sin_family = PF_INET;
   myaddr.sin_port = ntohs(111);

	struct servent *se = getservbyname("sunrpc","tcp");
	if (se)
	{
		myaddr.sin_port = se->s_port;
	}

	int fdsock;
   if( (fdsock = socket( AF_INET, SOCK_STREAM, 0 )) < 0 ) 
	{
      ERROR("Unable to open socket to port mapper");
      return NULL;
   }

   // Finally do the connect
   if( connect( fdsock, (struct sockaddr *)&myaddr, sizeof(myaddr) ) != 0 ) 
	{
		ERROR("You must start the port mapper before running this program!");
		closesocket(fdsock);
      return NULL;
    }
    closesocket(fdsock);

    // Init the RPC DLL
    RPCDATA rpcdata;
    RPCInit(0x09, (LPRPCDATA) &rpcdata);
#endif

   // create the RPC transport handle
   CLIENT *clnt = NULL;

   // Make sure we try at least once.
   int tries = max(1, tries_requested);

	INFORM("Will try %d times to connect to robot", tries);

#ifdef WIN32
	// Check if we have a raw IP address.  gethostbyname hangs win95 
	// if you give it an unknown raw IP address.
	bool symbolic_name = false;
	for(uint i=0; i<host.size(); ++i)
	{
		if( isalpha(host[i]) )
		{
			symbolic_name = true;
			break;
		}
	}

	struct hostent *hp = NULL;
	if( symbolic_name )
	{
		// convert the host name to the ip address.
		if( (hp = gethostbyname(host.c_str())) == NULL )
		{
			ERROR("Unknown host machine for robot '%s'", host.c_str());
			return NULL;
		}
	}
	else
	{
		// Handle a raw IP address.
		ulong raw_addr = inet_addr(host.c_str());
		if( raw_addr == INADDR_NONE )
		{
			ERROR("Badly formed host address for robot: '%s'",host.c_str());
			return NULL;
		}

      if( (hp = gethostbyaddr( (char *)&raw_addr, 4, AF_INET)) == NULL )
		{
			ERROR("Unknown host machine for robot '%s'", host.c_str());
			return NULL;
		}
	}
#endif

   int pass;
   for(pass=0; clnt == NULL && pass<tries; pass++)
   {
#ifdef WIN32
		// Define the network address of the robot.
		struct sockaddr_in robotaddr;
		memset(&robotaddr, 0, sizeof(struct sockaddr_in));
		robotaddr.sin_family = AF_INET;
		robotaddr.sin_addr.s_addr = ((struct in_addr *)(hp->h_addr))->s_addr;

		struct timeval timeout;
		timeout.tv_sec = 1;	// seconds
		timeout.tv_usec = 0;	// microseconds
		uint OUTPORT = 0;	// Don't care what output port it uses.

      clnt = (CLIENT *)clntudp_create(&robotaddr, R2Cprogram + offset, R2Cversion, 
					timeout, &OUTPORT);
#else
cerr << "client: R2Cprogram=" << R2Cprogram << " offset=" << offset << " program number=" << R2Cprogram + offset << endl;
      clnt = (CLIENT *)clnt_create(const_cast<char *>(host.c_str()), 
		R2Cprogram + offset, R2Cversion, "udp");
#endif
   }

   if( !clnt )
   {
      ERROR("Unable to attach to robot control program Host:'%s' Program: %lu Version: %ld Port: udp   tries=%d",
		host.c_str(), R2Cprogram + offset, R2Cversion, pass);
   }

   /* set the timeout value */
   struct timeval Timeout;
   Timeout.tv_sec = timeout_secs;
   Timeout.tv_usec = 0;
   if( clnt )
      clnt_control(clnt, CLSET_TIMEOUT, (char *)&Timeout);

//   client_handle = clnt;
   return clnt;
}

/*********************************************************************/
