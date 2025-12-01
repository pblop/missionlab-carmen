#ifndef CMDLdownload_H
#define CMDLdownload_H
/**********************************************************************
 **                                                                  **
 **  CMDLdownload.h                                                   **
 **                                                                  **
 **  the structure defining CMDLdownload messages                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: CMDLdownload.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: CMDLdownload.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:14:58  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.3  2004/11/12 21:55:16  doug
// tweaks for final Ft. Benning
//
// Revision 1.2  2004/06/11 16:10:02  doug
// move to sara namespace
//
// Revision 1.1  2004/04/27 16:21:45  doug
// support an operator console
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include "commMsg.h"

namespace sara
{
// *******************************************************************
// download messages broadcast between the CMDL interpreters
class CMDLdownload: public commMsg
{
public:
   // Constructors
   CMDLdownload();

   // Destructor
   virtual ~CMDLdownload() {};

   // -------------------- start of data ------------------------
   /// The identifier for the mission 
   string mission;

   /// The version number of the mission
   uint version;

   /// The cmdl code for the mission
   string cmdlCode;

   // -------------------- end of data ------------------------
   // Write the msg to a stream buffer in network byte order.
   // Returns true on success, false if the operation fails
   bool to_stream(stringstream *str) const;

   /// Build the msg from data in a stream buffer in network byte order
   /// Returns the object on success, NULL if the operation fails
   static commMsg *from_stream(stringstream *str);

   /// Create a copy of the msg
   commMsg *clone() const;

   // Stream out contents of the record for debugging.
   friend ostream &operator << (ostream & out, const CMDLdownload &obj);
};

// *********************************************************************
}
#endif

