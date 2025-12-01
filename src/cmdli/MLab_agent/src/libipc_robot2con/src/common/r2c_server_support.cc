/**********************************************************************
 **                                                                  **
 **  r2c_server_support.cc                                           **
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

/* $Id: r2c_server_support.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: r2c_server_support.cc,v $
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
// Revision 1.12  2004/05/11 19:34:36  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2003/07/09 22:19:17  doug
// The robot now exits cleanly
//
// Revision 1.10  2003/07/08 20:48:04  doug
// Added termination support using an extra abort pipe
//
// Revision 1.9  2003/04/11 18:46:57  doug
// added export capabilities
//
// Revision 1.8  1999/03/11 22:24:40  doug
// working
//
// Revision 1.7  1998/02/13 16:56:28  doug
// WIN32 changes
//
// Revision 1.6  1998/02/13 13:38:33  doug
// WIN32 changes
//
// Revision 1.5  1998/02/10 15:48:03  doug
// *** empty log message ***
//
// Revision 1.4  1998/02/10 14:41:18  doug
// rewrote win32 version
//
// Revision 1.3  1998/02/05 10:28:25  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/13 23:20:56  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/11 15:55:48  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#ifdef linux
   #include <sys/errno.h>
   #include <gssrpc/rpc.h>
   #include <gssrpc/pmap_clnt.h> /* for pmap_unset */
   #include <unistd.h>
   #include <fcntl.h>
 
   extern "C" int getdtablesize(void);
   static int tsize;

   /// the transmit/receiver pipe file descriptors to allow canceling the wait
   int tpsock = -1;
   int rpsock = -1;
#endif
#ifdef WIN32
   bool gotR2Cmsg;
#endif
#include "r2c_defs.h"
#include "r2c_server_support.h"

bool continue_rpc;
bool rpc_verbose;

using namespace sara;

/*********************************************************************/
// Close down the rpc services 
void
server_exit_r2c(const uint offset)
{
   continue_rpc = false;
#ifdef WIN32
   // Stop the dispatch server from running.

   // DCM: I wonder if this should be RPCCancelAsyncCall with the handle
   //      returned by RPCAsyncSvcRun()?
   RPCCancelBlockingCall();
#else
   // wake up the receiver so it will notice we are exiting
   char c = 'T';
   write(tpsock, &c, 1);
#endif

   svc_unregister(R2Cprogram + offset, R2Cversion);

#ifdef WIN32
   // Shut down the RPC DLL.
   RPCExit();  
#endif
}


/*********************************************************************/
// Fire up the rpc services and get ready to listen for messages
// Returns true if successful.
bool 
server_start_r2c(const uint offset, const bool verbose)
{
   rpc_verbose = verbose;

#ifdef WIN32
    // Init the RPC DLL 
    RPCDATA rpcdata;
    RPCInit(0x09, (LPRPCDATA) &rpcdata);
#endif
   register SVCXPRT *transp;

   (void) pmap_unset(R2Cprogram + offset, R2Cversion);

   transp = svcudp_create(RPC_ANYSOCK);
   if (transp == NULL) 
   {
      ERROR("cannot create udp service in start_rpc.");
      return false;
   }

   if( rpc_verbose )
      cerr << "server: R2Cprogram=" << R2Cprogram << " offset=" << offset << " program number=" << R2Cprogram + offset << endl;

   if (!svc_register(transp, R2Cprogram + offset, R2Cversion, r2cprogram_1, IPPROTO_UDP)) 
   {
      ERROR("unable to register (R2Cprogram + offset, R2Cversion, udp).");
      return false;
   }

#ifdef linux
   /* Get max num of file descriptors */
   tsize = getdtablesize();

   // open a pipe to use in canceling a select call
   int filedes[2];
   if( pipe(filedes) < 0 )
   {
      perror("Unable to create the transmit/receiver pipe!");
      exit(5);
   }
   rpsock = filedes[0];  // reader
   tpsock = filedes[1];  // writer
                                                                                
   // set the pipe to non-blocking reads
   int val = fcntl(rpsock, F_GETFL, 0);
   fcntl(rpsock, F_SETFL, val | O_NONBLOCK);

#else
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
server_r2c_loop(bool blocking, int delay_secs)
{
   if( !continue_rpc )
      return false;

#ifdef WIN32
   // Check if a message arrived once per second for the desired number 
   // of seconds.
   uint loop = 0;
   gotR2Cmsg = false;
   while( !gotR2Cmsg && (blocking || (++loop < delay_secs) ) )
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

   // Make a copy of the RPC file descriptor
   fd_set readfds = svc_fdset;

   // add our abort pipe
   FD_SET(rpsock, &readfds);

   // If there are any messages waiting, process them.
   // If we are blocking, wait forever for a message.
   if( rpc_verbose )
      cerr << endl << "calling select. blocking=" << blocking << endl;

   switch( select(tsize, &readfds,  NULL, NULL, 
		blocking ? (timeval *)NULL : &timeout) )
   {
      case -1:
         if( rpc_verbose )
         {
            cerr << endl << "select returned error" << endl;
            cerr << "-";
         }

	 // Error
	 ERROR_with_perror("rpc_server_support:rpc_loop - select failed");
	 continue_rpc = false;
	 break;

      case 0:
         if( rpc_verbose )
         {
            cerr << endl << "select returned nothing" << endl;
            cerr << "0";
         }

	 // Nothing to do
	 break;

      default:
         if( rpc_verbose )
         {
            cerr << endl << "select returned packet" << endl;
            cerr << "+";
         }

         // check our abort pipe 
         if( FD_ISSET(rpsock, &readfds) )
         {
            if( !continue_rpc )
               return false;

            // hmm, spurious abort.
            char buf[16];
            while( read(rpsock, buf, 16) > 0 );
         }
                                                                                
	 if( rpc_verbose )
	    INFORM("Got a RPC message");

         svc_getreqset(&readfds);
   }
#endif

   return continue_rpc;
}

/*********************************************************************/
