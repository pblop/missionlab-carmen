#include <stdio.h>

#include <ipt/tcx.h>

TCX_REG_MSG_TYPE messageArray[] = {
    { "TestMsg", "string" },
};

main(int argc, char** argv)
{
    TCX_MODULE_PTR other;
    char buffer[100];
    int i, size;
    char* bufptr;

    tcxInitialize(argv[1], NULL);
    tcxRegisterMessages(messageArray, 1);

    for (i=0;;i++) {
        other = tcxConnectOptional(argv[2]);
        if (other && tcxTestActiveModule(other)) {
            bufptr = &buffer[0];
            sprintf(buffer, "Hello World %i", i);
            printf("Sending :%s:\n", buffer);
            tcxSendMsg(other, "TestMsg", &bufptr);
            tcxRecvMsg("TestMsg", NULL, &bufptr, NULL);
            printf("Received %s\n", bufptr);
            tcxFreeReply("TestMsg", &bufptr);
        }

        sleep(1);
    }
}
