#ifndef R2C_SERVER_SUPPORT_H
#define R2C_SERVER_SUPPORT_H
/**********************************************************************
 **                                                                  **
 **  r2c_server_support                                              **
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

/* $Id: r2c_server_support.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: r2c_server_support.h,v $
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
// Revision 1.4  2003/07/09 22:19:17  doug
// The robot now exits cleanly
//
// Revision 1.3  1999/03/11 22:24:40  doug
// working
//
// Revision 1.2  1997/12/13 23:20:56  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/11 15:55:48  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

// Close down the rpc services
void server_exit_r2c(const uint offset);

// Fire up the rpc services and get ready to listen for messages
// Returns true if successful.
bool server_start_r2c(const uint offset, const bool verbose);

// Call repeatedly to process rpc messages
// If blocking is true, then will wait delay_secs for a message.
// Returns true if OK, 
//         false when exit is indicated
bool server_r2c_loop(bool blocking, int delay_secs = 60);

// Set to false to exit the server
extern bool continue_r2c;

// Set to enable verbose mode in the RPC server.
extern bool r2c_verbose;

/*********************************************************************/
#endif
