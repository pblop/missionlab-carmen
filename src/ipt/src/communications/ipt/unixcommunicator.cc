//////////////////////////////////////////////////////////////////////////////
//
//                                 unixcommunicator.cc
//
// This file implements the subclass of TCPCommunicator which should be used
// when same machine Unix socket based connections should be allowed as well
// as TCP/IP connections.
//
// Classes defined for export:
//    UnixCommunicator
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>

#include <sys/socket.h>

#include "./libc.h"

#ifdef VXWORKS
#include <selectLib.h>
#include <sockLib.h>
#else
#include <sys/time.h>
#endif

#include <ipt/unixcommunicator.h>
#include <ipt/primcomm.h>
#include <ipt/forconnection.h>
#include <ipt/fdconnection.h>
#include <ipt/time.h>
#include <ipt/list.h>
#include <ipt/event.h>
#include <ipt/internal_messages.h>
#include <ipt/message.h>
#include <ipt/formconfig.h>
#include <ipt/callbacks.h>
#include <ipt/format.h>
#include <ipt/route.h>

class UnixRouter : public IPRouter {
  public:
    UnixRouter(IPCommunicator* comm, int port, const char*);

    virtual IPConnection* copy_connection(const char*, const char*,
                                          IPConnection*);

  protected:
    virtual IPConnection* make_connection(const char* name,
                                          const char* sub_type,
                                          const char* parameters);
    virtual void activate_connection(IPConnection* conn, const char* sub_type,
                                     const char* parameters);

  private:
    void accept_new_connection_hand(IPConnection* conn);
    void close_socket_hand(IPConnection* conn);

  private:
    const char* _user_id;
};

declareConnectionCallback(UnixRouter);
implementConnectionCallback(UnixRouter);

UnixRouter::UnixRouter(IPCommunicator* comm, int port, const char* user_id)
    : IPRouter(comm, "Unix", IPT_HOSTNAME)
{
    int unix_fd;
    if (!listenAtSocket(port, unix_fd, user_id)) {
        printf("IPT(UNIX): Failure to create UNIX listen sockent.\n");
        printf("   Here's the first thing you should try:\n");
        printf("     cd /tmp\n");
        printf("     rm .ipt*\n");
        printf("   This is a problem with not cleaning up after\n");
        printf("   UNIX sockets (only done by ~IPCommunicator or\n");
        printf("   IPTdeinitialize()");
        printf("   If all the files are removed, and this still\n");
        printf("   doesn't do it, either you have not set the variable\n");
        printf("   USER in your system or your user ID might be too long.\n");
        printf("   Try setting the env. variable IPTUSER to something\n");
        printf("   shorter, but still unique\n");
        exit(-1);
    }

    IPConnectionCallback* close_callback =
        new ConnectionCallback(UnixRouter)
            (this, &UnixRouter::close_socket_hand);
    IPConnectionCallback* listen_callback =
        new ConnectionCallback(UnixRouter)
            (this, &UnixRouter::accept_new_connection_hand);
    comm->AddAdministrativeConnection(unix_fd,
                                      listen_callback, close_callback);

    _user_id = user_id;

    char buffer[10];
    sprintf(buffer, "%d", port);
    routing()->set_parameters(buffer);
    routing()->set_sub_type(comm->ThisHost());
}    

void UnixRouter::accept_new_connection_hand(IPConnection* conn)
{
    int fd = conn->FD();
    
    /* accept the connection */
    int sd = accept(fd, NULL, NULL);

    /* create an empty TCPConnection instance */
    UnixConnection temp("NoName", "NoHost", communicator(), sd);

    communicator()->add_connection(type(), &temp);
}

void UnixRouter::close_socket_hand(IPConnection* conn)
{
    if (conn->FD() != -1)
        close_socket(conn->FD());
}
    
IPConnection* UnixRouter::make_connection(const char* name,
                                          const char*, const char* parameters)
{
    int port;

    if (sscanf(parameters, "%d", &port) != 1) {
        communicator()->printf("IPT(UnixRouter):Error reading parameters %s\n",
                               parameters);
        return NULL;
    }

    int sd = connectToModule(name, port, _user_id, 1);
    if (sd < 0) 
        return NULL;
    else
        return new UnixConnection(name, communicator()->ThisHost(),
                                  communicator(), sd);
}

void UnixRouter::activate_connection(IPConnection* conn, const char*,
                                     const char* parameters)
{
    int port;
    if (sscanf(parameters, "%d", &port) != 1) {
        communicator()->printf("IPT(UnixRouter):Error reading parameters %s\n",
                               parameters);
        return;
    }

    int sd = connectToModule(conn->Name(), port, _user_id, 1);
    if (sd > 0) 
        conn->Activate(sd, communicator()->ThisHost());
}
    
IPConnection* UnixRouter::copy_connection(const char* name,
                                          const char* host, IPConnection* conn)
{
    if (conn->FD() == -1 || strcmp(conn->Type(), "Unix"))
        return NULL;

    UnixConnection* res =
        new UnixConnection(name, host, communicator(), conn->FD());

    res->SetData((FDConnection*) conn);
    res->SetByteOrder(conn->ByteOrder());
    res->SetAlignment(conn->Alignment());

    return res;
}

static void setup_unix_communicator(TCPCommunicator* comm)
{
    comm->DisableHandlers();
}

/* create a Unix communicator */
UnixCommunicator::UnixCommunicator(const char* mod_name, const char* host_name)
    : TCPCommunicator(mod_name, host_name, setup_unix_communicator)
{
    initialize_unix();
    EnableHandlers(); // we have to keep things from connecting til this point
}

/* create a Unix communicator with a given port */
UnixCommunicator::UnixCommunicator(int port) : TCPCommunicator(port)
{
    initialize_unix();
}

/* close a Unix communicator, and clean up the port */
UnixCommunicator::~UnixCommunicator()
{
    close_unix_port(_port, _user_id);
}

/* initialize the Unix communicator by seting up the socket listener for
   Unix socket connections */
void UnixCommunicator::initialize_unix()
{
    _user_id = getenv("IPTUSER");
    if (!_user_id)
        _user_id = getenv("USER");
    if (!_user_id)
        _user_id = "";

    add_router(new UnixRouter(this, _port, _user_id), IPT_ROUTE_HIGHEST);
}
