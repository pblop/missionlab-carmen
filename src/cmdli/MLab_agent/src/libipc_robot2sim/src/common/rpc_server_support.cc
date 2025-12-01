/**********************************************************************
 **                                                                  **
 **  rpc_server_support.cc                                           **
 **                                                                  **
 **  support functions for the rpc services (server side)            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rpc_server_support.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rpc_server_support.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:24:20  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.12  2004/05/11 19:34:35  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2002/10/02 18:18:47  doug
// *** empty log message ***
//
// Revision 1.10  2000/01/12 21:42:42  doug
// getdtablesize now has a prototype, so remove ours
//
// Revision 1.9  1998/06/18 20:13:09  doug
// *** empty log message ***
//
// Revision 1.8  1998/02/13 16:55:43  doug
// WIN32 changes
//
// Revision 1.7  1998/02/11 11:29:35  doug
// fixed for WIN32
//
// Revision 1.6  1998/02/10 15:48:17  doug
// Added code for Win32
//
// Revision 1.5  1998/02/10 09:15:27  doug
// moved data types to libipc_support
//
// Revision 1.4  1997/12/11 12:00:24  doug
// working
//
// Revision 1.3  1997/12/10 15:52:03  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/08 14:58:58  doug
// *** empty log message ***
//
// Revision 1.1  1997/11/26 13:06:10  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#ifdef linux
   #include <sys/errno.h>
   #include <gssrpc/rpc.h>
   #include <gssrpc/pmap_clnt.h> /* for pmap_unset */

   static int tsize;
#endif
#ifdef WIN32
   bool gotRPCmsg;
#endif
#include "rpc_defs.h"
#include "rpc_server_support.h"

namespace sara
{
bool continue_rpc;
bool rpc_verbose;

/*********************************************************************/
// Close down the rpc services 
void
server_exit_rpc()
{
#ifdef WIN32
   // Stop the dispatch server from running.

   // DCM: I wonder if this should be RPCCancelAsyncCall with the handle
   //      returned by RPCAsyncSvcRun()?
   RPCCancelBlockingCall();
#endif

   svc_unregister(RPCprogram, RPCversion);
  
#ifdef WIN32
   // Shut down the RPC DLL.
   RPCExit();
#endif
}


/*********************************************************************/
// Fire up the rpc services and get ready to listen for messages
// Returns true if successful.
bool 
server_start_rpc()
{
#ifdef WIN32
    // Init the RPC DLL
    RPCDATA rpcdata;
    RPCInit(0x09, (LPRPCDATA) &rpcdata);
#endif

   register SVCXPRT *transp;

   (void) pmap_unset(RPCprogram, RPCversion);

   transp = svctcp_create(RPC_ANYSOCK, 0, 0);
   if (transp == NULL) 
   {
      ERROR("cannot create tcp service in start_rpc.");
      return false;
   }
   if (!svc_register(transp, RPCprogram, RPCversion, rpcprogram_1, IPPROTO_TCP))
   {
      ERROR("unable to register (RPCprogram, RPCversion, tcp).");
      return false;
   }

#ifdef linux
   /* Get max num of file descriptors */
   tsize = getdtablesize();
#endif

#ifdef WIN32
   // this function tells the RPC4WIN.DLL to begin listening for
   // incoming connections upon all registerred services.
   // It will then dispatch all calls as they are received.
   if( RPCAsyncSvcRun() == NULL )
   {
      FATAL_ERROR("RPCAsyncSvcRun failed!");
   }
#endif

   // Init the global
   continue_rpc = true;

   return true;
}

/*********************************************************************/
// Call repeatedly to process rpc messages
// If blocking is true, then will wait delay_secs for a message.
// Returns true if OK,
//         false when exit is indicated
bool 
server_rpc_loop(bool blocking, int delay_secs)
{
   bool continue_rpc = true;

#ifdef WIN32
   // Check if a message arrived once per second for the desired number 
   // of seconds.
   uint loop = 0;
   gotRPCmsg = false;
   while( !gotRPCmsg && (blocking || (++loop < delay_secs) ) )
   {
      Sleep( 1000 );	// 1 second

      // Run the Windows message handler in case no one else is doing it
      MSG msg; 
      while( PeekMessage(&msg, (HWND) NULL, 0, 0,PM_REMOVE))
      {
         TranslateMessage(&msg); 
			DispatchMessage(&msg); 
      }	
   }

#else

   // Set our timeout value
   timeval timeout;
   timeout.tv_sec = delay_secs;
   timeout.tv_usec = 0;

   // Check the RPC file descriptor
   fd_set readfds = svc_fdset;

   // If there are any messages waiting, process them.
   // If we are blocking, wait forever for a message.
   switch( select(tsize, &readfds,  NULL, NULL, 
		blocking ? (timeval *)NULL : &timeout) )
   {
      case -1:
	 // Error
	 ERROR_with_perror("rpc_server_support:rpc_loop - select failed");
	 continue_rpc = false;
	 break;

      case 0:
	 // Nothing to do
	 break;

      default:
	 if( rpc_verbose )
	    INFORM("Got a RPC message");
         svc_getreqset(&readfds);
   }
#endif

   return continue_rpc;
}

/*********************************************************************/
}
