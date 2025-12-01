#include <stdio.h>

#include <ipt/ipt.h>

main(int argc, char** argv)
{
    IPConnection* other;
    char buffer[100];
    int i, size;
    IPCommunicator* comm;

    comm = iptCommunicatorInstance(argv[1], NULL);
    iptRegisterMessage(comm, "TestMsg", NULL);

    other = iptConnect(comm, argv[2], IPT_REQUIRED);

    for (i=0;;i++) {
        sprintf(buffer, "Hello World %i", i);
        printf("Sending :%s:\n", buffer);
        iptSendRawMsg(other, "TestMsg",
                        strlen(buffer)+1, (unsigned char*) buffer);
        size = iptReceiveRawMsg(NULL, other,
                                "TestMsg", 100,
                                (unsigned char*) buffer, IPT_BLOCK);

        printf("Received %d: %s\n", size, buffer);

        sleep(1);
    }
}
