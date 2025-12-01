

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
int base = 0;
uint port = 12345;

int main(int, char **)
{
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

//   ComAddr dest("192.168.1.16", port);

   for(int i=base; i<100; i++)
   {
      rawMsg *msg = new rawMsg();


      // load the values
      byte b = i;
      ubyte ub = i;
      word w = i;
      uword uw = i;
      short s = i;
      int in = i;
      uint uin = i;
      long l = i;
      ulong ul = i;

      stringstream tmp;
      tmp << i;
      string str = tmp.str();

      float f = i;
      double d = i;
      bool bol = (i % 2 == 1);

      // write them into a string buffer
      stringstream st;
      commMsg::out_byte(&st, b);
      commMsg::out_ubyte(&st, ub);
      commMsg::out_word(&st, w);
      commMsg::out_uword(&st, uw);
      commMsg::out_short(&st, s);
      commMsg::out_int(&st, in);
      commMsg::out_uint(&st, uin);
      commMsg::out_long(&st, l);
      commMsg::out_ulong(&st, ul);
      commMsg::out_string(&st, str);
      commMsg::out_float(&st, f);
      commMsg::out_double(&st, d);
      commMsg::out_bool(&st, bol);


      // load the message
      msg->data = st.str();

//      cerr << "broadcasting message '" << msg->data << "'" << endl;

      if( !comobj->broadcastMsg( *msg ) )
      {
         cerr << "Error broadcasting msg!" << endl;
         exit(1);
      }

/*
      msg->data = "unicast sent from doug";
      if( !comobj->sendMsg(dest, *msg) )
      {
         cerr << "Error unicasting msg!" << endl;
         exit(1);
      }
*/
      cerr << "o";
#if defined(linux)
      sleep(1);
#elif defined(WIN32)
      Sleep(1*1000);
#else
#error "undefined architecture!"
#endif
   }

   int oldcnt = 0;
   int newcnt = 0;
   do
   {
#if defined(linux)
      sleep(2);
#elif defined(WIN32)
      Sleep(2*1000);
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
