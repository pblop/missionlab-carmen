#include <stdio.h>
#include <stdlib.h>

#include <ipt/tcpcommunicator.h>
#include <ipt/internal_messages.h>
#include <ipt/primcomm.h>
#include <ipt/fdconnection.h>
#include <ipt/formconfig.h>
#include <ipt/message.h>

class ServerInterface : public TCPCommunicator {
  public:
    ServerInterface();

    int open_server(const char* host_name);
    void dump_messages(const char* msg_file);
    void dump_connection_net(const char* conn_file);
    void flush_connections();
    void flush();
    void close_server();

  private:
    IPConnection* _server;
};

ServerInterface::ServerInterface() : TCPCommunicator(0)
{
    _server = 0;

    register_message(IPTServerDumpMsgNum, IPT_SERVER_DUMP_MSG,
                     IPT_SERVER_DUMP_FORM);
    register_message(IPTServerFlushMsgNum, IPT_SERVER_FLUSH_MSG,
                     IPT_SERVER_FLUSH_FORM);
    register_message(IPTServerDumpNetMsgNum, IPT_SERVER_DUMP_NET_MSG,
                     IPT_SERVER_DUMP_NET_FORM);
    register_message(IPTServerFlushNetMsgNum, IPT_SERVER_FLUSH_NET_MSG,
                     IPT_SERVER_FLUSH_NET_FORM);
}

int ServerInterface::open_server(const char* host_name)
{
    int sd;
    if (!connectAtPort(host_name, SERVER_PORT, sd))
        return 0;

    if (_server) {
        close_server();
    }

    _server = new TCPConnection(SERVER_NAME, host_name, this, sd);
    IPTModuleConnectingStruct mc;
    mc.byte_order = BYTE_ORDER;
    mc.alignment = ALIGN;
    mc.name = "IPTServerInterface";
    mc.host = (char*) ThisHost();

    if (SendMessage(_server, IPTModuleConnectingMsgNum, &mc) >= 0) {
        return 1;
    }

    return 0;
}

void ServerInterface::dump_messages(const char* msg_file)
{
    SendMessage(_server, IPTServerDumpMsgNum, &msg_file);
}

void ServerInterface::dump_connection_net(const char* conn_file)
{
    SendMessage(_server, IPTServerDumpNetMsgNum, &conn_file);
}

void ServerInterface::flush()
{
    SendMessage(_server, IPTServerFlushMsgNum, NULL);
}

void ServerInterface::flush_connections()
{
    SendMessage(_server, IPTServerFlushNetMsgNum, NULL);
}

void ServerInterface::close_server()
{
    _server->Deactivate();
    delete _server;
    _server = NULL;
}

main()
{
    ServerInterface* intf = new ServerInterface();
    const char* host_name = getenv("IPTHOST");
    if (!host_name)
        host_name = intf->ThisHost();

    int option;
    while (1) {
        printf("    0) Quit\n");
        printf("    1) Dump messages\n");
        printf("    2) Flush pending\n");
        printf("    3) Dump connection net\n");
        printf("    4) Flush connection net\n");
        printf("     ---> ");
        scanf("%d", &option);

        if (option < 0 || option > 4) {
            printf("Invalid option %d\n", option);
            continue;
        }

        if (option == 0)
            break;

        if (!intf->open_server(host_name)) {
            printf("No server on machine (%s)\n", host_name);
            continue;
        }

        if (option == 1) 
            intf->dump_messages("messages.out");
        else if (option == 2)
            intf->flush();
        else if (option == 3)
            intf->dump_connection_net("stdout");
        else if (option == 4)
            intf->flush_connections();

        intf->close_server();
    }

    delete intf;
}
