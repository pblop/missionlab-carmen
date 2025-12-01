/**********************************************************************
 **                                                                  **
 **  rawMsg.cc                                                       **
 **                                                                  **
 **  Handle raw data                                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: rawMsg.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rawMsg.cc,v $
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
// Revision 1.9  2004/11/12 21:45:07  doug
// fixed NPE when uncompress fails
//
// Revision 1.8  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.7  2003/02/26 14:55:48  doug
// controller now runs with objcomms
//
// Revision 1.6  2003/02/17 23:00:13  doug
// assigns task lead
//
// Revision 1.5  2003/02/13 16:17:00  doug
// waitFor seems to work
//
// Revision 1.4  2003/02/04 18:23:07  doug
// switched to sending senderID in every packet header
//
// Revision 1.3  2003/02/03 22:36:02  doug
// It seems to work finally!
//
// Revision 1.2  2003/01/31 22:36:48  doug
// working a bit
//
// Revision 1.1  2003/01/31 19:30:13  doug
// test1 builds and links!
//
///////////////////////////////////////////////////////////////////////

#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
// ********************************************************************
/// Constructor
rawMsg::rawMsg() : 
   commMsg(MsgTypes::RAWMSG)
{ /* empty */ }

// *********************************************************************
// Write the msg data to a stream buffer in network byte order.
// Returns true on success, false if the operation fails.
bool 
rawMsg::to_stream(stringstream *ost) const
{
   // Write our message type as the first data
   MsgType_to_stream(ost, msgType);

   // Write the data
   out_string(ost, data);

   return true;
}

// *********************************************************************
// Loads the msg header from a stream buffer presented in 
// network byte order.
// Returns true on success, false if the operation fails.
commMsg *
rawMsg::from_stream(stringstream *ost)
{
   rawMsg *rtn = new rawMsg();

   in_string(ost, rtn->data);
/*
   int offset = ost->tellg();

   // Read our data
   // This builds a string from the stream data starting at the current offset
   // and continuing to the end of the data (npos).
   rtn->data = string(ost->str(), offset, string::npos);
*/
   return rtn;
}

// ********************************************************************
// Create a copy of the msg 
commMsg *
rawMsg::clone() const
{
   rawMsg *rtn = new rawMsg();
   rtn->cloneBase( this );

   rtn->data = data;

   return rtn;
}

// ********************************************************************
}
