

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
vector<string> sentmessages;

int base = 0;

// the comm object
objcomms *comobj = NULL;

void msgCallback(commMsg *msg, void *)
{
   rawMsg *rmsg = dynamic_cast<rawMsg *>(msg);
   if( rmsg == NULL )
   {
      cerr << "Unable to upcast commMsg to rawMsg!" << endl;
      return;
   }

   byte *p = (byte *)rmsg->data.data();
   uint sz = rmsg->data.size();
   int num = *((int *)p);
   p += sizeof(int);

   if( sz != num + 2*sizeof(int) )
   {
      cerr << "bad packet size: received size=" << sz << " their stated size =" << num + 2*sizeof(int)  << endl;
   }

   uint accum = num;

   for(int i=0; i<num; i++)
   {
      accum += *p++;
   }

   // get the hash code
   uint their_accum = *((int *)p);

   if( accum == their_accum )
   {
      cerr << "+";

      // echo it back
//cerr << "echoing to address: " << msg->senderAddr.str() << endl;
      if( !comobj->sendMsg( msg->senderAddr, msg ) )
      {
         cerr << "Error broadcasting msg!" << endl;
         exit(1);
      }
   }
   else
   {
      cerr << "-";
   }
}

int main(int, char **)
{
   // create the comm object
   comobj = new objcomms();

   // Turn on all debug
//   comobj->setDebug(objcomms::DEBUG_TRNS | objcomms::DEBUG_RECV | objcomms::DEBUG_RECV_DETAILS);

   // open it
   uint inst = 99999;
   if( !comobj->open(1,"127.0.0.255", 50346, "", &inst) )
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

   int oldcnt = 0;
   int newcnt = 0;
   do
   {
      sleep(6000);
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

   for( int i=0; i<(int)sentmessages.size(); i++ )
   {
      if( sentmessages[i].size() != 1 || 
          sentmessages[i].data()[0] != '.' )
      {
         cerr << "did not receive message " << i << " containing " << sentmessages[i].size() << " bytes." << endl;
      }
   }
}
}
