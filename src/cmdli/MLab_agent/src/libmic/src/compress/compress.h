#ifndef compress_H
#define compress_H
/**********************************************************************
 **                                                                  **
 **  compress.h                                                      **
 **                                                                  **
 **  Base class for implementations to Compress/expand data streams  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: compress.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: compress.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:30:29  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.2  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.1  2003/01/27 17:38:12  doug
// added compress library to compress stringstreams
//
///////////////////////////////////////////////////////////////////////

#include <sstream>
#include "mic.h"

namespace sara
{

// *******************************************************************
// Base class for buffer compression
class compress
{
public:
   /// Constructor
   /// compressionLevel: 0=none, 1=fast, 9=best, -1=requests best tradeoff
   compress(int compressionLevel = -1);
	
   /// Compress the data in the stream "in" and write it to the output stream.
   virtual stringstream *compressStream(stringstream *in);

   /// Uncompress the data in the stream "in" and write it to the output stream.
   virtual stringstream *expandStream(stringstream *in);

   /// The requested compression level: 0=none, 1=fast, 9=best, -1=requests best tradeoff
   int compressionLevel;
};

/*********************************************************************/
}
#endif
