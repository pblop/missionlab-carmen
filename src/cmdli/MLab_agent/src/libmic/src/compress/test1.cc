
namespace sara
{

#include "mic.h"
#include "compress.h"
#include "compress_zlib.h"


int main(int, char **)
{
   compress *comp = new compress_zlib(9);


//   string input;

//   char *data = "abcddddddddddddddefgggggggggggggggggggggggghijklmnopqrstuvwxyz";
   for(int i=0; i<20; i++)
   {
//   input = &data[i];
//   cerr << "The input data: '" << input << "'" << endl;

   stringstream ss;

   for(int j=0; j< 65536+i; j++)
   {
      int k = rand();
      ss.write((char *)&k, sizeof(k));
   }

//   ss << input << ends;
   stringstream *compressed = comp->compressStream(&ss);




   stringstream *expanded = comp->expandStream(compressed);

   string orig = ss.str();
   string back = expanded->str();
   int orig_len = orig.size();
   int back_len = back.size();
   if( orig_len != back_len || memcmp(orig.data(),back.data(), orig_len) != 0 )
   {
      cerr << "The uncompressed data doesn't match the original!" << endl;
   }
   else
   {
      cerr << "The uncompressed matches!" << endl;
   }

      cerr << endl;
   }

}
}
