

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
vector<string> sentmessages;

int base = 0;

//int port = 50346;
int port = 12345;

void msgCallback(commMsg *msg, void *)
{
   rawMsg *rmsg = dynamic_cast<rawMsg *>(msg);
   if( rmsg == NULL )
   {
      cerr << "Unable to upcast commMsg to rawMsg!" << endl;
      return;
   }

   stringstream st(rmsg->data);



      // define the values
      byte b;
      ubyte ub;
      word w;
      uword uw;
      short s;
      int in;
      uint uin;
      long l;
      ulong ul;
      string str;
      float f;
      double d;
      bool bol;

      // read them in 
      commMsg::in_byte(&st, b);
      commMsg::in_ubyte(&st, ub);
      commMsg::in_word(&st, w);
      commMsg::in_uword(&st, uw);
      commMsg::in_short(&st, s);
      commMsg::in_int(&st, in);
      commMsg::in_uint(&st, uin);
      commMsg::in_long(&st, l);
      commMsg::in_ulong(&st, ul);
      commMsg::in_string(&st, str);
      commMsg::in_float(&st, f);
      commMsg::in_double(&st, d);
      commMsg::in_bool(&st, bol);

cerr << endl;
int b_sub = b;
uint ub_sub = ub;
cerr << "byte =    " << b_sub << endl;
cerr << "ubyte=    " << ub_sub << endl;
cerr << "word=     " << w << endl;
cerr << "uword=    " << uw << endl;
cerr << "short=    " << s << endl;
cerr << "int=      " << in << endl;
cerr << "uint=     " << uin << endl;
cerr << "long=     " << l << endl;
cerr << "ulong=    " << ul << endl;
cerr << "string=  '" << str << "'" << endl;
cerr << "float=    " << f << endl;
cerr << "double=   " << d << endl;
cerr << "bool=     " << bol << endl;

   cerr << "+";
}

int main(int, char **)
{
   rawMsg dummy;

   // create the comm object
   objcomms *comobj = new objcomms();

   // turn on all debug
//   comobj->setDebug(-1);
   
   // open it
   if( !comobj->open("234.5.6.7", port) )
   {
      cerr << "Error opening comobj!" << endl;
      exit(1);
   }

   // Start with compression off
   comobj->disableCompression();
   
   // Turn compression on
//   comobj->enableCompression();

   // start with encription off
   comobj->disableEncryption();

   // turn encryption on
//   comobj->enableEncryption("This Is A Weak Key!");

   // Register the packets we will use
   comobj->attachHandler(MsgTypes::RAWMSG, rawMsg::from_stream);

   // Register our callback function
   comobj->subscribeTo(MsgTypes::RAWMSG, msgCallback, 0);

   int oldcnt = 0;
   int newcnt = 0;
   do
   {
#if defined(linux)
      sleep(60);
#elif defined(WIN32)
      Sleep(60*1000);
#else
#error "undefined architecture!"
#endif
      oldcnt = newcnt;
      newcnt = comobj->messagesRcv;
      cerr << '.';
   } while( oldcnt != newcnt );
   cerr << endl;
}
}
