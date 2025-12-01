

#include "mic.h"
#include "objcomms.h"
#include "objthreads.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
vector<string> sentmessages;

int base = 0;

class reader : public objthreads
{
   public:
   reader(ThreadData parm);

   void run(ThreadData parm);
};

reader::reader(ThreadData parm) :
   objthreads(parm)
{ /* empty */ }

void 
reader::run(ThreadData parm)
{
   objcomms *comobj = (objcomms *)parm;
   while(1)
   {
      commMsg *msg = comobj->waitFor(MsgTypes::RAWMSG, 5*1000);
      if( msg == NULL )
      {
         cerr << "Did not receive a rawMsg!" << endl;
      }
      else
      {
         rawMsg *rmsg = dynamic_cast<rawMsg *>(msg);
         if( rmsg == NULL )
         {
            cerr << "Unable to upcast commMsg to rawMsg!" << endl;
            return;
         }

         cerr << "Got Msg: '" << rmsg->data << "'" << endl;
      }

   }
}

int main(int, char **)
{
   // create the comm object
   objcomms *comobj = new objcomms();

   // open it
   uint inst = 99999;
   if( !comobj->open(2,"192.168.1.255", 50346, "", &inst) )
   {
      cerr << "Error opening comobj!" << endl;
      exit(1);
   }

   // Start with compression off
//   comobj->disableCompression();
   
   // Turn compression on
   comobj->enableCompression();

   // start with encription off
//   comobj->disableEncryption();

   // turn encryption on
   comobj->enableEncryption("This Is A Weak Key!");

   // Register the packets we will use
   comobj->attachHandler(MsgTypes::RAWMSG, rawMsg::from_stream);

   // create the reader and start it running
   reader *rdr = new reader((ThreadData)comobj);
   rdr->start_thread();

   // Send it some messages
   for(int i=base; i<5; i++)
   {
      rawMsg *msg = new rawMsg();

      stringstream st;
      st << "Message " << i;
      msg->data = st.str();

      if( !comobj->broadcastMsg( msg ) )
      {
         cerr << "Error broadcasting msg!" << endl;
         exit(1);
      }

      usleep(8*1000*1000);
   }

   // wait for the comms to quiet down
   int oldcnt = 0;
   int newcnt = 0;
   do
   {
      sleep(2);
      oldcnt = newcnt;
      newcnt = comobj->messagesRcv;
      cerr << '.';
   } while( oldcnt != newcnt );
   cerr << endl;
}
}
