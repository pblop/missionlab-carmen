#include <stdio.h>
#include <math.h>

#include <ipt/ipt.h>
#include <ipt/message.h>

static IPMessageSpec message_specs[] = {
    { "TestMsg", "string" },
    { "TestMsg2", "string" },
    { NULL, NULL },
};

static void handler(IPCommunicator*, IPMessage* msg, void*)
{
    if (!msg)
        return;

    char** data = (char**) msg->FormattedData();
    printf("Received :%s:\n", *data);
    msg->DeleteFormatted(data);
}

main(int argc, char** argv)
{
    IPCommunicator* ipt = IPCommunicator::Instance(argv[1]);
    ipt->RegisterMessages(message_specs);
    IPMessageType* t1 = ipt->LookupMessage("TestMsg");

    if (argc < 3) {
        ipt->DeclareSubscription("TestMsg");
        ipt->DeclareSubscription("TestMsg2");
        int i;
        char buffer[100];
        char* bufptr = buffer;
        for (i=0;;i++) {
            sprintf(buffer, "Hello world %d", i);
            IPMessage* out = new IPMessage(t1,
                                           ipt->generate_instance_num(),
                                           (void*) &bufptr);
            ipt->Publish(t1, out);
            ipt->Sleep(1.0);
        }
    } else {
        printf("Active %s to %s\n", argv[1], argv[2]);
        IPConnection* conn = ipt->Connect(argv[2]);

        ipt->RegisterHandler(t1, handler, (void*) argv[2]);
        ipt->Subscribe(argv[2], "TestMsg");
        ipt->Subscribe(argv[2], "TestMsg2");
        ipt->MainLoop();
    }
}
