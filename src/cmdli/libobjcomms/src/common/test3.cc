

#include "mic.h"
#include "objcomms.h"
#include "commMsg.h"
namespace sara
{

bool test(const string &a, const string &b)
{
   return a.size() == b.size() && strcmp(a.c_str(), b.c_str()) == 0;
}

int main(int, char **)
{
   // Test the string marshalleling

   int cnt = 1;
   for(int loop = 0; loop<30; loop++)
   {
      cnt *= 2;
      string input;
      for(int c=0; c < cnt-1; c++)
      {
         input += "a";
      }

      stringstream st;

      commMsg::out_string(&st, input);

      st << "BBBB";

      string output;
      commMsg::in_string(&st, output);

      if( test(input,output) )
      {
         cerr << cnt-1 << " worked" << endl;
      }
      else
      {
         cerr << cnt-1 << " failed **********" << endl;
      }
      
   }

}
}
