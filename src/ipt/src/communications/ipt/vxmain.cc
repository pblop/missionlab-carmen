#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <taskLib.h>

#include <ipt/ipt.h>
#include <ipt/message.h>
#include <ipt/filter.h>

extern "C" {

void IPTtest(char* name, char* other_name)
{
    IPCommunicator* ipt = IPCommunicator::Instance(name);
    IPMessageType* t1 = ipt->RegisterMessage("TestMsg", "string");
    IPConnection* other;

    if (other_name)
        other = ipt->Connect(other_name);
    else
        other = NULL;

    char buffer[100];
    int i;
    MsgFilter filter(t1, other);
    for (i=0;;i++) {
        if (other) {
            sprintf(buffer, "Hello World %i", i);
            printf("Sending :%s:\n", buffer);

            ipt->SendMessage(other, t1, strlen(buffer)+1, (unsigned char*) buffer);
        }
        
        IPMessage* msg = ipt->ReceiveMessage(&filter);

        if (msg) {
            printf("Received %d: %s\n", msg->SizeData(), msg->Data());
            delete msg;
        } else 
            printf("NULL message\n");

        taskDelay(60);
    }

    delete ipt;
}

}
