#ifndef R2C_CLIENT_SUPPORT_H
#define R2C_CLIENT_SUPPORT_H
/**********************************************************************
 **                                                                  **
 **  r2c_client_support                                              **
 **                                                                  **
 **  prototypes for the client support functions                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: r2c_client_support.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: r2c_client_support.h,v $
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
// Revision 1.11  1999/03/11 22:24:40  doug
// working
//
// Revision 1.10  1998/02/23 15:53:37  doug
// added list_prims
//
// Revision 1.9  1998/02/18 15:06:22  doug
// *** empty log message ***
//
// Revision 1.8  1998/02/18 14:10:07  doug
// added get_fsa
//
// Revision 1.7  1998/02/13 13:41:53  doug
// WIN32
//
// Revision 1.6  1998/02/10 09:12:14  doug
// moved data defs to libipc_support
//
// Revision 1.5  1997/12/31 23:04:49  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/31 03:00:54  doug
// fixing for win32
//
// Revision 1.3  1997/12/17 15:57:53  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/17 15:50:09  doug
// rewrote
//
// Revision 1.1  1997/12/11 15:55:48  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "r2c_defs.h"
#include "string"

// Free our rpc client data record
void client_exit_r2c(CLIENT *clnt);

// Initialize a client connection to the server.
//    host is the name the server is running on.
//    tries is the number of attempts to make.
//    timeout_secs is the number of seconds to set the timeout to. 
//    offset is an offset added to the program number to handle multiple
//    robots per processor. 
// Returns the CLIENT handle or NULL if unable to connect

CLIENT *
client_start_r2c(const string &host, const int tries, const int timeout_secs,
					  const uint offset);

/*********************************************************************/
#endif
