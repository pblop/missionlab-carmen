/**********************************************************************
 **                                                                  **
 **  encryption_evp.cc                                               **
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

/* $Id: encryption_evp.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: encryption_evp.cc,v $
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
// Revision 1.7  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.6  2003/06/17 20:21:40  doug
// back out the last change
//
// Revision 1.5  2003/06/17 20:15:34  doug
// I am not sure it is needed, but rewind the incoming streams to make sure get everything.
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

#include <openssl/evp.h>
#include "mic.h"
#include "encryption.h"
#include "encryption_evp.h"

namespace sara
{
// ********************************************************************
/// Constructor
encryption_evp::encryption_evp(const string &key) :
   encryption(key)
{
   /* empty */
}
	
// ********************************************************************
/// encryption the data in the stream "in" and write it to the output stream.
stringstream *
encryption_evp::encryptStream(stringstream *in)
{
   return encryptStream( in->str() );   
}

// ********************************************************************
/// encryption the data in the string "in" and write it to the output stream.
stringstream *
encryption_evp::encryptStream(const string &input)
{
   // Allocate a buffer for evp to write into
   static const uint BUFSIZE = 2048;
   byte buf[BUFSIZE];

   // Create the stream we will return
   stringstream *out = new stringstream();

   // init the evp encryption library.
   EVP_CIPHER_CTX *ctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX evp;
   EVP_CIPHER_CTX_init(&evp);
   ctx = &evp;
#else
   ctx = EVP_CIPHER_CTX_new();
#endif

   unsigned char iv[] = {1,2,3,4,5,6,7,8};

   if( EVP_EncryptInit(ctx, EVP_bf_cbc(), (byte *)key.data(), iv) != 1 )
   {
      ERROR("Error initing EVP encryption library");
      return out;
   }

   byte *p = (byte *)input.data();
   int bytesLeft = input.size();
   int outlen; 
   while( bytesLeft > 0 )
   {
      int thisPass = min((int)BUFSIZE - 64, bytesLeft);

      // encryption one output block's worth.
      if( EVP_EncryptUpdate(ctx, buf, &outlen, p, thisPass) != 1 )
      {
         ERROR("Error encryptioning data");
         return out;
      }

      // Copy the output to our output stream
      if( outlen > 0 )
      {
         out->write((char *)buf, outlen);
      }

      // Update our counter
      bytesLeft -= thisPass;
      p += thisPass;
   }

   // Flush any pending data
   if( EVP_EncryptFinal(ctx, buf, &outlen) != 1 )
   {
      ERROR("Error finishing encrypting data");
      return out;
   }

   // Copy the output to our output stream
   if( outlen > 0 )
   {
      out->write((char *)buf, outlen);
   }

#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX_cleanup(ctx);
#else
   EVP_CIPHER_CTX_free(ctx);
#endif

//cerr << "Generated " << out->str().size() << " bytes of encrypted data from " << in->str().size() << " raw bytes" << endl;

   return out;
}

// ********************************************************************
/// Decryption the data in the stream "in" and write it to the output stream.
stringstream *
encryption_evp::decryptStream(stringstream *in)
{ 
   // Allocate a buffer for evp to write into
   static const uint BUFSIZE = 2048;
   byte buf[BUFSIZE];

   // Create the stream we will return
   stringstream *out = new stringstream();

   // init the evp encryptionion library.
   EVP_CIPHER_CTX *ctx;
#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX evp;
   EVP_CIPHER_CTX_init(&evp);
   ctx = &evp;
#else
   ctx = EVP_CIPHER_CTX_new();
#endif

   unsigned char iv[] = {1,2,3,4,5,6,7,8};

   if( EVP_DecryptInit(ctx, EVP_bf_cbc(), (byte *)key.data(), iv) != 1 )
   {
      ERROR("Error initing EVP decryption library");
      return out;
   }

   string input( in->str() );
   byte *p = (byte *)input.data();
   int bytesLeft = input.size();
   int outlen; 
   while( bytesLeft > 0 )
   {
      int thisPass = min((int)BUFSIZE - 64, bytesLeft);

      // decrypt one output block's worth.
      if(EVP_DecryptUpdate(ctx, buf, &outlen, p, thisPass) != 1)
      {
         ERROR("Error decrypting data");
         break;
      }

      // Copy the output to our output stream
      if( outlen > 0 )
      {
         out->write((char *)buf, outlen);
      }

      // Update our counter
      bytesLeft -= thisPass;
      p += thisPass;
   }

   // Flush any pending data
   if(EVP_DecryptFinal(ctx, buf, &outlen) != 1)
   {
      ERROR("Error finishing decrypting data");
   }

   // Copy the output to our output stream
   if( outlen > 0 )
   {
      out->write((char *)buf, outlen);
   }

#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX_cleanup(ctx);
#else
   EVP_CIPHER_CTX_free(ctx);
#endif

//cerr << "Generated " << out->str().size() << " bytes of raw data from " << in->str().size() << " encrypted bytes" << endl;

   return out;
}

// ********************************************************************
/// encrypt the data in "in" and write it to "out"
/// The buffer size must be a multiple of the current block size
/// returns: 0 on success
///         -1 if size is not a multiple of the cypher block size
///         -2 if error encrypting data
int 
encryption_evp::encryptBuffer(byte *in, byte *out, const int size)
{
   // init the evp encryption library.
    EVP_CIPHER_CTX *ctx;
 #if OPENSSL_VERSION_NUMBER < 0x10100000
    EVP_CIPHER_CTX evp;
    EVP_CIPHER_CTX_init(&evp);
    ctx = &evp;
 #else
    ctx = EVP_CIPHER_CTX_new();
 #endif

   unsigned char iv[] = {1,2,3,4,5,6,7,8};

   if( EVP_EncryptInit(ctx, EVP_bf_cbc(), (byte *)key.data(), iv) != 1)
   {
      ERROR("Error initing EVP encryption library");
      return -2;
   }

   // get the block size 
   int blockSize = EVP_CIPHER_CTX_block_size(ctx);

   if( size % blockSize != 0 )
   {
ERROR("size=%d, blockSize=%d\n", size, blockSize);
      return -1;
   }

   // turn off padding
   EVP_CIPHER_CTX_set_padding(ctx, 0);

   byte *p = in;
   int bytesLeft = size;
   int bytesOut = 0;
   int outlen; 
   while( bytesLeft > 0 )
   {
      int thisPass = min(size - bytesOut, bytesLeft);

      // encrypt one output block's worth.
      if( EVP_EncryptUpdate(ctx, &out[bytesOut], &outlen, p, thisPass) != 1 )
      {
         ERROR("Error encryptioning data");
         return -2;
      }

      // Update our counter
      bytesLeft -= thisPass;
      bytesOut += outlen;
      p += thisPass;
   }

   // Flush any pending data
   if( EVP_EncryptFinal(ctx, &out[bytesOut], &outlen) != 1 )
   {
      ERROR("Error finishing encrypting data");
      return -2;
   }

   bytesOut += outlen;

   if( bytesOut != size )
   {
ERROR("Wrong size of encrypted data: bytesOut=%d, size=%d\n", bytesOut, size);
      return -2;
   }

#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX_cleanup(ctx);
#else
   EVP_CIPHER_CTX_free(ctx);
#endif

   return 0;
}

// ********************************************************************
/// encrypt the data in "in" and write it to "out"
/// The buffer size must be a multiple of the current block size
/// returns: 0 on success
///         -1 if size is not a multiple of the cypher block size
///         -2 if error encrypting data
int 
encryption_evp::decryptBuffer(byte *in, byte *out, const int size)
{ 
   // init the evp encryptionion library.
    EVP_CIPHER_CTX *ctx;
 #if OPENSSL_VERSION_NUMBER < 0x10100000
    EVP_CIPHER_CTX evp;
    EVP_CIPHER_CTX_init(&evp);
    ctx = &evp;
 #else
    ctx = EVP_CIPHER_CTX_new();
 #endif

   unsigned char iv[] = {1,2,3,4,5,6,7,8};

   if( EVP_DecryptInit(ctx, EVP_bf_cbc(), (byte *)key.data(), iv) != 1)
   {
      ERROR("Error initing EVP decryption library");
      return -2;
   }

   // get the block size 
   int blockSize = EVP_CIPHER_CTX_block_size(ctx);

   if( size % blockSize != 0 )
   {
ERROR("size=%d, blockSize=%d\n", size, blockSize);
      return -1;
   }

   // turn off padding
   EVP_CIPHER_CTX_set_padding(ctx, 0);

   byte *p = in;
   int bytesLeft = size;
   int bytesOut = 0;
   int outlen; 
   while( bytesLeft > 0 )
   {
      int thisPass = min(size - bytesOut, bytesLeft);

      // decrypt one output block's worth.
      if(EVP_DecryptUpdate(ctx, &out[bytesOut], &outlen, p, thisPass) != 1)
      {
         ERROR("Error decrypting data");
         return -2;
      }

      // Update our counter
      bytesLeft -= thisPass;
      bytesOut += outlen;
      p += thisPass;
   }

   // Flush any pending data
   if(EVP_DecryptFinal(ctx, &out[bytesOut], &outlen) != 1)
   {
      ERROR("Error finishing decrypting data");
      return -2;
   }

   // Update our counter
   bytesOut += outlen;

   if( bytesOut != size )
   {
ERROR("Wrong size of encrypted data: bytesOut=%d, size=%d\n", bytesOut, size);
      return -2;
   }

#if OPENSSL_VERSION_NUMBER < 0x10100000
   EVP_CIPHER_CTX_cleanup(ctx);
#else
   EVP_CIPHER_CTX_free(ctx);
#endif

   return 0;
}

// ********************************************************************
}
