#ifndef rawMsg_H
#define rawMsg_H
/**********************************************************************
 **                                                                  **
 **  rawMsg.h                                                        **
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

/* $Id: rawMsg.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: rawMsg.h,v $
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
// Revision 1.6  2004/11/12 21:45:07  doug
// fixed NPE when uncompress fails
//
// Revision 1.5  2004/05/11 19:34:34  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2003/02/13 16:17:00  doug
// waitFor seems to work
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

#include "mic.h"
#include "commMsg.h"

namespace sara
{
// *******************************************************************
// Base class for user rawunications messages
class rawMsg : public commMsg
{
public:
   /// constructors
   rawMsg();

   /// destructor
   virtual ~rawMsg() {};

   // ------------------------- start of data ------------------------
   /// The data
   string data;
   // -------------------------- end of data -------------------------

   /// Write the msg to a stream buffer in network byte order.
   /// Returns true on success, false if the operation fails
   bool to_stream(stringstream *str) const;

   // Create a copy of the msg 
   commMsg *clone() const;

   /// Build the msg from data in a stream buffer in network byte order
   /// Returns the object on success, NULL if the operation fails
   static commMsg *from_stream(stringstream *str);
};

/*********************************************************************/
}
#endif
