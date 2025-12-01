#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ipt/ipt.h>
#include <ipt/connection.h>
#include <ipt/filter.h>
#include <ipt/message.h>
#include <ipt/callbacks.h>
#include <ipt/timer.h>

static IPMessageSpec message_specs[] = {
    { "TestMsg", "{ int, string }" },
    { "FooType", "{ int, FooYou }" },
    { "BarType", "{ float, FooYou }" }, 
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

static void disconn_handler(IPConnection* conn, void*)
{
    IPCommunicator* ipt = conn->Communicator();

/*
    while (!conn->Active()) {
        printf("Restablishing connection\n");
        ipt->Connect(argv[2], IPT_OPTIONAL);
        if (conn->Active()) {
            printf("Reconnected to %s (%s)\n",
                   conn->Name(), conn->Host());
            break;
        } else
            ipt->Sleep(1.0);
    }
*/
    ipt->Connect(conn->Name());
}

static void conn_handler(IPConnection* conn, void*)
{
    printf("Connection %x connecting\n", conn);
}

class TimerTest : public IPTimerCallback
{
  public:
    TimerTest(IPCommunicator* comm, const char* msg)
        { _comm = comm; _msg = msg; }

    virtual void Execute(IPTimer* timer) {
        printf("Timer %x: %s\n", this, _msg);
        _comm->AddOneShot(timer->interval() + 1.0, this);
    }

  private:
    const char* _msg;
    IPCommunicator* _comm;
};

main(int argc, char** argv)
{
    IPCommunicator* ipt =  IPCommunicator::Instance(argv[1]);
    ipt->RegisterNamedFormatter("FooYou", "{ float, int, string }");
    ipt->RegisterMessages(message_specs);
    IPMessageType* t1 = ipt->LookupMessage("TestMsg");
    ipt->AddConnectCallback(conn_handler);

    if (argc < 3) {
        ipt->PigeonHole(t1);
        ipt->RegisterHandler(t1, ph_handler);
        TimerTest* t = new TimerTest(ipt, "Hello world");
        ipt->AddOneShot(1.0, t);

        printf("Start loop\n");
        while (1) {
            ipt->Sleep(1.0);
            printf(".\n");
        }
    } else {
        printf("Active %s to %s\n", argv[1], argv[2]);
        IPConnection* conn = ipt->Connect(argv[2]);
        printf("Connected to %s (%s %x)\n",
               conn->Name(), conn->Host(), conn);

        conn->AddDisconnectCallback(disconn_handler);

        int i;
        char buffer[100];
        TestType t;
        t.str = buffer;
        for (i=0;i<20;i++) {
            sprintf(buffer, "Hello world %d", i);

            t.n = i;
            ipt->SendMessage(conn, t1, &t);
            printf("Sent %d\n", i);

            ipt->Sleep(1.0);
        }
    }

    delete ipt;
    IPTclear_list_cache();

    return 0;
}
