#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#ifdef VXWORKS
#include <sockLib.h>
#else
#include <sys/socket.h>
#endif

#include "./libc.h"

#include <ipt/internal_messages.h>
#include <ipt/route.h>
#include <ipt/primcomm.h>
#include <ipt/list.h>
#include <ipt/hash.h>
#include <ipt/connection.h>
#include <ipt/ipt.h>
#include <ipt/fdconnection.h>
#include <ipt/callbacks.h>

declareConnectionCallback(TCPRouter);
implementConnectionCallback(TCPRouter);

/* Allocate and copy the string "src" and return it */
static char* copy_string(const char* src)
{
    if (!src)
        return NULL;
    
    int len = strlen(src)+1;
    char* res = new char[len];
    bcopy(src, res, len);

    return res;
}

IPRouting::IPRouting()
{
    _type = _sub_type = _parameters = NULL;
    _comparison = -1;
}

void IPRouting::set_routing(const char* type, const char* sub_type,
                            int comparison, const char* parameters)
{
    if (_type)
        delete _type;
    if (_sub_type)
        delete _sub_type;
    if (_parameters)
        delete _parameters;
    if (type)
        _type = copy_string(type);
    else
        _type = NULL;
    if (sub_type)
        _sub_type = copy_string(sub_type);
    else
        _sub_type = NULL;
    _comparison = comparison;

    if (parameters)
        _parameters = copy_string(parameters);
    else
        _parameters = NULL;
}

void IPRouting::set_sub_type(const char* sub_type)
{
    if (_sub_type)
        delete _sub_type;
    if (sub_type)
        _sub_type = copy_string(sub_type);
    else
        _sub_type = NULL;
}

void IPRouting::set_parameters(const char* parameters)
{
    if (_parameters)
        delete _parameters;
    if (parameters)
        _parameters = copy_string(parameters);
    else
        _parameters = NULL;
}

int IPRouting::matches(const IPRouting* other) const
{
    if (strcmp(_type, other->_type))
        return 0;

    switch(_comparison) {
      case IPT_SAME:
        return (strcmp(_sub_type, other->_sub_type) == 0);
      case IPT_DIFFERENT:
        return (strcmp(_sub_type, other->_sub_type) != 0);
      case IPT_HOSTNAME:
        return hosts_equal(_sub_type, other->_sub_type);
      default:
        return 1;
    }
}

IPRouting::~IPRouting()
{
    if (_type)
        delete _type;
    if (_sub_type)
        delete _sub_type;
    if (_parameters)
        delete _parameters;
}

IPRoutingTable::IPRoutingTable(IPConnection* conn, int num_routes,
                               int free_conn)
{
    _num_routes = num_routes;
    _connection = conn;
    _table = new IPRouting[num_routes];
    _free_conn = free_conn;
}

IPRoutingTable::~IPRoutingTable()
{
    delete _table;
    if (_free_conn)
        delete _connection;
}
    
IPRouter::IPRouter(IPCommunicator* comm, const char* type, int comparison)
{
    _communicator = comm;
    _routing = new IPRouting();
    _routing->set_routing(type, NULL, comparison, NULL);
}

IPRouter::~IPRouter()
{
    delete _routing;
}

IPConnection* IPRouter::route_connection(const char* name,
                                         const char* sub_type,
                                         const char* parameters)
{
    IPConnection* res = make_connection(name, sub_type, parameters);
    if (!res)
        return NULL;

    return res;
}

IPRouterTable::IPRouterTable()
{
    _routers = IPList::Create();
    _table = new IPHashTable(10, str_hash, str_eq);
}

IPRouterTable::~IPRouterTable()
{
    IPRouter* cur;
    IPRouter* doomed;
    for (cur = (IPRouter*) _routers->First(); cur; ) {
        doomed = cur;
        cur = (IPRouter*) _routers->Next();
        delete doomed;
    }
    
    IPList::Delete(_routers);
    delete _table;
}

int IPRouterTable::num_routers() const
{
    return _routers->Length();
}
     
IPRouter* IPRouterTable::router(int index) const
{
    if (index < 0 || index > num_routers())
        return NULL;
    IPRouter* res;
    for (res = (IPRouter*) _routers->First(); res && index--;
         res = (IPRouter*) _routers->Next());

    return res;
}

IPRouter* IPRouterTable::lookup_router(const char* name) const
{
    return (IPRouter*) _table->Find(name);
}

void IPRouterTable::add_router(IPRouter* router, int where, IPRouter* other)
{
    _table->Insert(router->type(), strlen(router->type())+1, (char*) router);

    switch (where) {
      case IPT_ROUTE_HIGHEST:
        _routers->Prepend((char*) router);
        break;
      case IPT_ROUTE_LOWEST:
        _routers->Append((char*) router);
        break;
      case IPT_ROUTE_AFTER:
        _routers->InsertAfter((char*) router, (char*) other);
        break;
    }

    return;
}

TCPRouter::TCPRouter(IPCommunicator* comm, int port)
    : IPRouter(comm, "TCP", IPT_NONE)
{
    int listen_fd;
    if (!listenAtPort(port, listen_fd)) {
        printf("IPT:  Failure to create module listen socket.\n");
        exit(-1);
    }

    char* buffer = new char[strlen(comm->ThisHost()) + 10];
    sprintf(buffer, "%s %d", comm->ThisHost(), port);

    routing()->set_parameters(buffer);

    // ENDO - MEM LEAK
    //delete buffer;
    delete [] buffer;
    buffer = NULL; // Added by ENDO

    IPConnectionCallback* close_callback =
        new ConnectionCallback(TCPRouter)
            (this, &TCPRouter::close_socket_hand);
    IPConnectionCallback* listen_callback =
        new ConnectionCallback(TCPRouter)
            (this, &TCPRouter::accept_new_connection_hand);
    comm->AddAdministrativeConnection(listen_fd,
                                      listen_callback, close_callback);
    _port = port;
}

void TCPRouter::accept_new_connection_hand(IPConnection* conn)
{
    int sd = accept_socket(conn->FD());

    /* create an empty TCPConnection instance */
    /* avoid static constructures due to bizarre MIPS R4000 bug */
    IPConnection* temp = new TCPConnection("NoName", "NoHost", communicator(), sd);
    communicator()->add_connection(type(), temp);
    delete temp;
}

void TCPRouter::close_socket_hand(IPConnection* conn)
{
    if (conn->FD() != -1)
        close_socket(conn->FD());
}
    
IPConnection* TCPRouter::make_connection(const char* name,
                                         const char*, const char* parameters)
{
    char* host = new char[strlen(parameters)];
    int port;

    if (sscanf(parameters, "%s %d", host, &port) != 2) {
        printf("IPT(TCPRouter): Error reading parameters %s\n", parameters);
        delete host;
        return NULL;
    }

    int sd = connectToModule(name, host, port);
    IPConnection* res;
    if (sd < 0) 
        res = NULL;
    else
        res = new TCPConnection(name, host, communicator(), sd);
    delete host;

    return res;
}

void TCPRouter::activate_connection(IPConnection* conn,
                                    const char*,
                                    const char* parameters)
{
    char* host = new char[strlen(parameters)];
    int port;

    if (sscanf(parameters, "%s %d", host, &port) != 2) {
        printf("IPT(TCPRouter): Error reading parameters %s\n", parameters);
        delete host;
        return;
    }

    int sd = connectToModule(conn->Name(), host, port, 1);
    if (sd > 0) 
        conn->Activate(sd, host);

    delete host;
}
    
IPConnection* TCPRouter::copy_connection(const char* name, const char* host,
                                         IPConnection* conn)
{
    if (conn->FD() == -1 || strcmp(conn->Type(), "TCP"))
        return NULL;

    TCPConnection* res =
        new TCPConnection(name, host, communicator(), conn->FD());

    res->SetData((FDConnection*) conn);
    res->SetByteOrder(conn->ByteOrder());
    res->SetAlignment(conn->Alignment());

    return res;
}
