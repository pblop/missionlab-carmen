

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
vector<string> sentmessages;

int base = 0;

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

//cerr << "received message " << num << " containing " << sz << " bytes ";
   if( num < 0 || num >= (int)sentmessages.size() ||
       sz != sentmessages[num].size() ||
       memcmp(p, sentmessages[num].data(), sz) != 0 )
   {
   cerr << "received message " << num << " containing " << sz << " bytes that -- DOES NOT -- match!" << endl;
   }
   else
   {
       // got it
       sentmessages[num] = ".";
   }

/*
   for(uint ch=0; ch<sz; ch++)
   {
      if( isprint(p[ch]) )
      {
         cerr << (char)p[ch];
      }
      else
      {
         char hexbuf[256];
         sprintf(hexbuf, " 0x%2.2x ", (int)p[ch]);
         cerr << hexbuf;
      }
   }
*/
}

int main(int, char **)
{
   // create the comm object
   objcomms *comobj = new objcomms();

   // open it
   uint inst = 99999;
   if( !comobj->open("192.168.1.255", 50346, "", 1, &inst) )
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

   // Register our callback function
   objcomms::SubscriptionHandle sh = comobj->subscribeTo(MsgTypes::RAWMSG, msgCallback, 0);

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

      sentmessages.push_back( st.str() );

      msg->data = st.str();

/*
   stringstream *ss = new stringstream();
   for(int j=0; j< 65536 + i; j++)
   {
      int k = rand();
      ss.write((char *)&k, sizeof(k));
   }
 */

/*
   for(int i=0; i< 5536; i++)
   {
      char i = 'A';
      ss.write(&i, 1);
   }
*/

//      cerr << "broadcasting message '" << msg->data << "'" << endl;
      if( !comobj->broadcastMsg( *msg ) )
      {
         cerr << "Error broadcasting msg!" << endl;
         exit(1);
      }

///// here for testing!!!!
//      usleep(10);
   }

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
