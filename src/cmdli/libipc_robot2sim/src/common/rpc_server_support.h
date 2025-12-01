/**********************************************************************
 **                                                                  **
 **  rpc_server_support                                              **
 **                                                                  **
 **  prototypes for the server support functions                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: rpc_server_support.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rpc_server_support.h,v $
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
// Revision 1.5  2004/05/11 19:34:35  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  1998/02/10 15:48:17  doug
// Added code for Win32
//
// Revision 1.3  1997/12/11 12:00:24  doug
// working
//
// Revision 1.2  1997/12/10 14:16:14  doug
// *** empty log message ***
//
// Revision 1.1  1997/11/26 13:06:21  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

namespace sara
{
// Close down the rpc services
void server_exit_rpc();

// Fire up the rpc services and get ready to listen for messages
// Returns true if successful.
bool server_start_rpc();

// Block waiting for rpc messages
bool server_rpc_loop(bool blocking, int delay_secs);

// Set to false to exit the server
extern bool continue_rpc;

// Set to enable verbose mode in the RPC server.
extern bool rpc_verbose;
}

/*********************************************************************/
