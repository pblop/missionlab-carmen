#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ipt/ipt.h>
#include <ipt/connection.h>
#include <ipt/filter.h>
#include <ipt/message.h>

static IPMessageSpec message_specs[] = {
    { "TestMsg", "{ int, string }" },
    { NULL, NULL },
};

struct TestType {
    int n;
    char* str;
};

static void ph_handler(IPCommunicator*, IPMessage* msg, void*)
{
    if (!msg)
        return;

    msg->Print(1);
/*    char** data = (char**) msg->FormattedData();
    printf("Received :%s:\n", *data); 
    msg->DeleteFormatted(data); */
}

main(int argc, char** argv)
{
    IPCommunicator* ipt =  IPCommunicator::Instance(argv[1]);
    ipt->RegisterMessages(message_specs);
    IPMessageType* t1 = ipt->LookupMessage("TestMsg");

    if (argc < 3) {
        ipt->PigeonHole(t1);
        ipt->RegisterHandler(t1, ph_handler);

        while (1) {
            ipt->Sleep(1.0);
            sleep(5);
        }
    } else {
        printf("Active %s to %s\n", argv[1], argv[2]);
        IPConnection* conn;
        while (1) {
            conn = ipt->Connect(argv[2], IPT_OPTIONAL);
            if (conn->Active())
                break;
            printf("Out of luck on optional connection\n");
            ipt->Idle(1.0);
        }

        printf("Connected to %s (%s)\n",
               conn->Name(), conn->Host());

        int i;
        char buffer[100];
        TestType t;
        t.str = buffer;
        for (i=0;;i++) {
            sprintf(buffer, "Hello world %d", i);
            if (!conn->Active()) {
                printf("Restablishing connection\n");
                ipt->Connect(argv[2]);
                printf("Reconnected to %s (%s)\n",
                       conn->Name(), conn->Host());
            }

            t.n = i;
            ipt->SendMessage(conn, t1, &t);
            printf("Sent %d\n", i);

            ipt->Sleep(1.0);
        }
    }
}

