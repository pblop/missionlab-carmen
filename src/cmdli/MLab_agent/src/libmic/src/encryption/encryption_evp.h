#ifndef encryption_evp_H
#define encryption_evp_H
/**********************************************************************
 **                                                                  **
 **  encryption_evp.h                                                **
 **                                                                  **
 **  evp implementation of encrypt/decrypt data streams              **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2003.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: encryption_evp.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: encryption_evp.h,v $
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
// Revision 1.5  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  2003/04/25 22:11:52  doug
// changed encryptBuffer so does not pad and requires the buffer length to be a multiple of the blocksize
//
// Revision 1.3  2003/04/23 20:57:10  doug
// still working on tcp support
//
// Revision 1.2  2003/01/29 21:32:25  doug
// Added support for buffers
//
// Revision 1.1  2003/01/27 23:24:27  doug
// added compression and encription libraries
//
///////////////////////////////////////////////////////////////////////

#include <sstream>
#include "mic.h"
#include "encryption.h"

namespace sara
{
// *******************************************************************
// Base class for buffer encryptionion
class encryption_evp : public encryption
{
public:
   /// Constructor
   /// They suggest a key that is 16 bytes long, but it accepts variable length
   encryption_evp(const string &key);
	
   /// encryption the data in the stream "in" and write it to the output stream.
   stringstream *encryptStream(stringstream *in);

   /// encrypt the data in the string "in" and write it to the output stream.
   stringstream *encryptStream(const string &in);

   /// Decryption the data in the stream "in" and write it to the output stream.
   stringstream *decryptStream(stringstream *in);

   /// encrypt the data in "in" and write it to "out"
   /// The buffer size must be a multiple of the current block size
   /// returns: 0 on success
   ///         -1 if size is not a multiple of the cypher block size
   ///         -2 if error encrypting data
   int encryptBuffer(byte *in, byte *out, const int size);

   /// encrypt the data in "in" and write it to "out"
   /// The buffer size must be a multiple of the current block size
   /// returns: 0 on success
   ///         -1 if size is not a multiple of the cypher block size
   ///         -2 if error encrypting data
   int decryptBuffer(byte *in, byte *out, const int size);
};

/*********************************************************************/
}
#endif
