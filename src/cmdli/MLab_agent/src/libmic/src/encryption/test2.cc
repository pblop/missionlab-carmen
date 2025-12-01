
namespace sara
{

//#include "mic.h"
#include "encryption.h"
#include "encryption_evp.h"


int main(int, char **)
{
   int a;

   encryption *comp = new encryption_evp("This is a bad key");


   for(int i=0; i<20; i++)
   {
      byte *buf1 = new byte[sizeof(int) * 65536];
      byte *buf2 = new byte[sizeof(int) * 65536];
      byte *buf3 = new byte[sizeof(int) * 65536];

   for(int j=0; j< 65530 + i; j++)
   {
      int k = rand();
      memcpy(&buf1[j*sizeof(int)], &k, sizeof(k));
   }
   int sizein = sizeof(int) * 65530;
   
   int sizecompressed = comp->encryptBuffer(buf1, sizein, buf2, sizeof(int) * 65536);

   int sizeexpanded = comp->decryptBuffer(buf2, sizecompressed, buf3, sizeof(int) * 65536);

   if( sizein != sizeexpanded || memcmp(buf1,buf3, sizeexpanded) != 0 )
   {
      cerr << "The unencrypted data doesn't match the original!" << endl;
   }
   else
   {
      cerr << "The unencrypted matches!" << endl;
   }

      cerr << endl;
   }
}
}
