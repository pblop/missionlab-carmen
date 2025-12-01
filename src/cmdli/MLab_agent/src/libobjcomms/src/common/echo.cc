

#include "mic.h"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{
objcomms *comobj = NULL;

void msgCallback(commMsg *msg)
{
   rawMsg *rmsg = dynamic_cast<rawMsg *>(msg);
   if( rmsg == NULL )
   {
      cerr << "Unable to upcast commMsg to rawMsg!" << endl;
      return;
   }

/*
   cerr << "received message " << *((int *)(rmsg->data.data())) << " from ";
   const char *p = msg->senderID.data();
   uint sz = msg->senderID.size();
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
   cerr << " containing " << rmsg->data.size() << " bytes" << endl;
*/

   // echo it back.
   if( !comobj->broadcastMsg( msg ) )
   {
      cerr << "Error broadcasting msg!" << endl;
   }
}

int main(int, char **)
{
   // create the comm object
   comobj = new objcomms();

   // open it
   uint inst = 99999;
   if( !comobj->open(1,"192.168.1.255", 50346, "", &inst) )
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
   objcomms::SubscriptionHandle sh = comobj->subscribeTo(MsgTypes::RAWMSG, msgCallback);

   while(1)
   {
      sleep(20);

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
   cerr << endl << endl;

   }

//   delete comobj;
}
}
