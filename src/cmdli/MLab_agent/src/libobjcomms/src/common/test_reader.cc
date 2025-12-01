

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

   const char *p = rmsg->data.data();
   uint sz = rmsg->data.size();

   int num = *((int *)p) - base;

   cerr << "+";
}

int main(int, char **)
{
   rawMsg dummy;

   // create the comm object
   objcomms *comobj = new objcomms();

   // turn on all debug
   comobj->setDebug(-1);
   
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
   objcomms::SubscriptionHandle sh = comobj->subscribeTo(MsgTypes::RAWMSG, msgCallback, 0);

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

   // dump the stats
   cerr << "Number of bytes sent:                 " << comobj->bytesTrn << endl;
   cerr << "Number of bytes received:             " << comobj->bytesRcv << endl;
   cerr << "Number of packets sent:               " << comobj->packetsTrn << endl;
   cerr << "Number of packets received:           " << comobj->packetsRcv << endl;
   cerr << "Number of messages sent:              " << comobj->messagesTrn << endl;
   cerr << "Number of messages received:          " << comobj->messagesRcv << endl;
   cerr << "Number of outbound packets discarded: " << comobj->packetsTrnDiscard << endl;
   cerr << "Number of inbound packets discarded:  " << comobj->packetsRcvDiscard << endl;
   cerr << "Number of inbound packets corrupted:  " << comobj->packetsCorrupted << endl;

   for(int i=0; i<(int)sentmessages.size(); i++)
   {
      if( sentmessages[i].size() != 1 || 
	  sentmessages[i].data()[0] != '.' )
      {
         cerr << "did not receive message " << i << " containing " << sentmessages[i].size() << " bytes." << endl;
      }
   }
}
}
