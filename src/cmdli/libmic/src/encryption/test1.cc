

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
   stringstream ss;

   for(int j=0; j< 65536 + i; j++)
   {
      int k = rand();
      ss.write((char *)&k, sizeof(k));
   }
   
/*
   for(int i=0; i< 5536; i++)
   {
      char i = 'A';
      ss.write(&i, 1);
   }
*/

//   ss << ends;

//   ss << input << ends;
   stringstream *compressed = comp->encryptStream(&ss);


//   cerr << "The encrypted data: '" << compressed->str() << "'" << endl;


   stringstream *expanded = comp->decryptStream(compressed);

   string orig = ss.str();
   string back = expanded->str();
   int orig_len = orig.size();
   int back_len = back.size();
   if( orig_len != back_len || memcmp(orig.data(),back.data(), orig_len) != 0 )
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
