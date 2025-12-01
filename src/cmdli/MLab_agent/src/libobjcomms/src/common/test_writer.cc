

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
vector<string> sentmessages;

int base = 0;
//uint port = 50346;
uint port = 12345;

int main(int, char **)
{
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

//   ComAddr dest("192.168.1.16", port);
   ComAddr dest("192.168.1.213", port);

   for(int i=base; i<5000; i++)
   {
      rawMsg *msg = new rawMsg();

      char buf[sizeof(int)];
      *((int *)buf) = i;
      stringstream st;
      st.write(buf,sizeof(int));

      int val;
//      int size = rand() % 1000;
      int size = i;
      for(int j=0; j<size; j++)
      {
         val = rand();
//	 st.write((char *)&val, sizeof(int));
	 st.write((char *)&val, 1);
//	 st << (char)((int)'A' + (j % 26));
      }

      msg->data = st.str();

//      cerr << "broadcasting message '" << msg->data << "'" << endl;

/*      if( !comobj->broadcastMsg( *msg ) )
      {
         cerr << "Error broadcasting msg!" << endl;
         exit(1);
      }
*/


      msg->data = "unicast sent from doug";
      if( !comobj->sendMsg(dest, *msg) )
      {
         cerr << "Error unicasting msg!" << endl;
         exit(1);
      }

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
