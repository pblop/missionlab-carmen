/**********************************************************************
 **                                                                  **
 **  compress_zlib.cc                                                **
 **                                                                  **
 **  zlib implementation of Compress/expand data streams             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: compress_zlib.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: compress_zlib.cc,v $
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
// Revision 1.10  2004/11/12 21:47:46  doug
// fixed so can expand beyond a single buffer
//
// Revision 1.9  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.8  2004/04/14 17:55:07  doug
// correct to real baseclass
//
// Revision 1.7  2003/06/17 20:21:03  doug
// back out the last change
//
// Revision 1.6  2003/06/17 20:16:41  doug
// I am not sure it is needed, but rewind the incoming streams to make sure get everything.
//
// Revision 1.5  2003/02/26 14:54:16  doug
// controller now runs with objcomms
//
// Revision 1.4  2003/02/10 22:40:19  doug
// Clean up buffer management
//
// Revision 1.3  2003/02/04 18:23:34  doug
// removed debug print
//
// Revision 1.2  2003/01/27 23:24:27  doug
// added compression and encription libraries
//
// Revision 1.1  2003/01/27 17:38:12  doug
// added compress library to compress stringstreams
//
///////////////////////////////////////////////////////////////////////

#include <zlib.h>
#include "mic.h"
#include "compress.h"
#include "compress_zlib.h"

namespace sara
{
// ********************************************************************
/// Constructor
/// compressionLevel: 0=none, 1=fast, 9=best, 6=default
compress_zlib::compress_zlib(int level) :
   compress(level)
{
   /* empty */
}
	
// ********************************************************************
/// Compress the data in the stream "in" and write it to the output stream.
stringstream *
compress_zlib::compressStream(stringstream *in)
{
   // Allocate a buffer for zlib to write into
   static const uint BUFSIZE = 2048;
   byte buf[BUFSIZE];

   // Create the stream we will return
   stringstream *out = new stringstream();

   // init the zlib compression library.
   z_stream zstream;
   memset(&zstream, 0, sizeof(zstream));

   // Get ready to compress the input data
   string input( in->str() );
   zstream.next_in = (Bytef *)input.data();
   zstream.avail_in = input.size(); 

   // Init it.
   int rtn = deflateInit(&zstream, compressionLevel);
   if( rtn != Z_OK )
   {
      ERROR("Error initing deflate to zlib compression library: %s", 
		      rtn == Z_MEM_ERROR ? "Memory Error" :
		      rtn == Z_STREAM_ERROR ? "Invalid Compression Level Error" :
		      rtn == Z_VERSION_ERROR ? "Version Error" :
		      "Unknown Error");
   }

   bool done = false;
   while( !done )
   {
      // Compress one output block's worth.
      zstream.next_out = buf;
      zstream.avail_out = BUFSIZE; 

      rtn = deflate(&zstream, Z_FINISH);
      if( rtn != Z_OK && rtn != Z_STREAM_END )
      {
         ERROR("Error from deflate call to zlib compression library: %s", 
		      rtn == Z_STREAM_ERROR ? "Stream Error" :
		      rtn == Z_BUF_ERROR ? "Buffer Error" :
		      "Unknown Error");
      }

      // Copy the output to our output stream
      if( zstream.avail_out < BUFSIZE )
      {
         out->write((char *)buf, BUFSIZE - zstream.avail_out);
      }

      // Are we done?
      if( rtn == Z_STREAM_END )
      {
         done = true;
      }
   }

//cerr << "Generated " << zstream.total_out << " bytes of compressed data from " << zstream.total_in << " raw bytes" << endl;

   rtn = deflateEnd(&zstream);
   if( rtn != Z_OK )
   {
      ERROR("Error closing zlib compression library: %s  %s", 
		      rtn == Z_STREAM_ERROR ? "Stream Error" :
		      rtn == Z_DATA_ERROR ? "Data Error" :
		      "Unknown Error", zstream.msg);
   }

   return out;
}

// ********************************************************************
/// Uncompress the data in the stream "in" and write it to the output stream.
stringstream *
compress_zlib::expandStream(stringstream *in)
{ 
   // Allocate a buffer for zlib to write into
   static const uint BUFSIZE = 2048;
   byte buf[BUFSIZE];

   // Create the stream we will return
   stringstream *out = new stringstream();

   // init the zlib compression library.
   z_stream zstream;
   memset(&zstream, 0, sizeof(zstream));

   // get ready to expand the input data
   in->seekg(0, std::ios::end);
   int length = in->tellg();
   in->seekg(0);
   if(length == 0 )
   {
      cerr << "zero starting length!" << endl;
      return NULL;
   }
   byte *data = new byte[length];
   in->read((char *)data, length);
   zstream.next_in = (Bytef *)data;
   zstream.avail_in = length; 
   
   // Init it.
   int rtn = inflateInit(&zstream);
   if( rtn != Z_OK )
   {
      ERROR("Error initing inflate to zlib compression library: %s", 
		      rtn == Z_MEM_ERROR ? "Memory Error" :
		      rtn == Z_VERSION_ERROR ? "Version Error" :
		      "Unknown Error");
   }

   bool done = false;
   int limit = 0;
   while( !done && limit++ < 100000 )
   {
      // Expand one raw block's worth.
      zstream.next_out = buf;
      zstream.avail_out = BUFSIZE; 

      rtn = inflate(&zstream, Z_FINISH);
//      if( rtn != Z_OK && /*rtn != Z_BUF_ERROR &&*/ rtn != Z_STREAM_END )
      if( rtn != Z_OK && rtn != Z_BUF_ERROR && rtn != Z_STREAM_END )
      {
         ERROR("Error from inflate call to zlib compression library: %s", 
		      rtn == Z_STREAM_ERROR ? "Stream Error" :
//		      rtn == Z_BUF_ERROR ? "Buffer Error" :
		      "Unknown Error");
	 delete out;
	 out = NULL;
	 break;
      }

      // Copy the output to our output stream
      if( zstream.avail_out < BUFSIZE )
      {
         out->write((char *)buf, BUFSIZE - zstream.avail_out);
      }
      else if( rtn == Z_BUF_ERROR )
      {
	 // We didn't generate any output, and threw a buffer_error, so quit.
	 delete out;
	 out = NULL;
	 break;
      }

      // Are we done?
      if( rtn == Z_STREAM_END )
      {
         done = true;
      }
   }

   rtn = inflateEnd(&zstream);
   if( rtn != Z_OK )
   {
      ERROR("Error closing zlib compression library: %s  %s", 
		      rtn == Z_STREAM_ERROR ? "Stream Error" :
		      rtn == Z_DATA_ERROR ? "Data Error" :
		      "Unknown Error", zstream.msg);
   }

   // Clean up
   delete [] data;

   // done.
   return out;
}

// ********************************************************************
}
