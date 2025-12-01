/**********************************************************************
 **                                                                  **
 **  rpc_client_support                                              **
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

/* $Id: rpc_client_support.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rpc_client_support.h,v $
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
// Revision 1.8  2004/05/11 19:34:35  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.7  1998/06/19 02:44:07  doug
// changed timeout to uint
//
// Revision 1.6  1998/06/18 20:13:09  doug
// *** empty log message ***
//
// Revision 1.5  1998/02/14 11:00:47  doug
// WIN32 changes
//
// Revision 1.4  1998/02/10 16:51:52  doug
// *** empty log message ***
//
// Revision 1.3  1998/02/10 09:15:27  doug
// moved data types to libipc_support
//
// Revision 1.2  1997/12/10 15:52:03  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/10 14:16:14  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include "rpc_defs.h"

namespace sara
{
// Free our rpc client data record
void client_exit_rpc(CLIENT *clnt);

// Initialize a client connection to the server.
//    host is the name the server is running on.
//    tries is the number of attempts to make.
//    timeout_msecs is the number of milliseconds to set the timeout to.
// Returns the CLIENT handle or NULL if unable to connect
CLIENT *
client_start_rpc(const string &host, const int tries, const uint timeout_msecs);

}
/*********************************************************************/
