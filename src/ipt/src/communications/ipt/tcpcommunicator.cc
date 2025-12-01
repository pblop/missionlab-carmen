//////////////////////////////////////////////////////////////////////////////
//
//                                 tcpcommunicator.cc
//
// This file implements the subclass of IPCommunicator which should be used
// when TCP/IP connections are the only kind of connections expected and 
// allowed, i.e., especially for UNIX modules.
//
// Classes defined for export:
//    TCPCommunicator
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

#ifdef VXWORKS
#include <selectLib.h>
#include <sockLib.h>
#include <taskLib.h>
#else
#include <sys/time.h>
#endif

#include "./libc.h"

#include <ipt/tcpcommunicator.h>
#include <ipt/primcomm.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/internal_messages.h>
#include <ipt/event.h>
#include <ipt/connection.h>
#include <ipt/fdconnection.h>
#include <ipt/forconnection.h>
#include <ipt/time.h>
#include <ipt/list.h>
#include <ipt/formconfig.h>
#include <ipt/format.h>
#include <ipt/callbacks.h>
#include <ipt/translator.h>
#include <ipt/route.h>
#include <ipt/hash.h>

declareConnectionCallback(TCPCommunicator);
implementConnectionCallback(TCPCommunicator);
declareHandlerCallback(TCPCommunicator);
implementHandlerCallback(TCPCommunicator);
declareTimerCallback(TCPCommunicator);
implementTimerCallback(TCPCommunicator);

/* initialize a TCPcommunicator to listen for incoming messages at port "port"
   If "port" is 0, a free port will be generated */
void TCPCommunicator::initialize(int port)
{
    protect_against_signals();

    _server = 0;
    _connection_mask = new fd_set;
    _translators = IPList::Create();
    
    FD_ZERO(_connection_mask);

    register_message(IPTPeerDisconnectingMsgNum, IPT_PEER_DISCONNECTING_MSG,
                     IPT_PEER_DISCONNECTING_FORM);
    register_message(IPTRoutingTableMsgNum, IPT_ROUTING_TABLE_MSG,
                     IPT_ROUTING_TABLE_FORM);
    register_message(IPTConnectionRoutingMsgNum, IPT_CONNECTION_ROUTING_MSG,
                     IPT_CONNECTION_ROUTING_FORM);
    register_message(IPTReportConnectionsMsgNum, IPT_REPORT_CONNECTIONS_MSG,
                     IPT_REPORT_CONNECTIONS_FORM);
    register_message(IPTReregisterMessagesMsgNum, IPT_REREGISTER_MESSAGES_MSG,
                     IPT_REREGISTER_MESSAGES_FORM);
    
    RegisterHandler(LookupMessage(IPTPeerDisconnectingMsgNum),
                    new HandlerCallback(TCPCommunicator)
                    (this, &TCPCommunicator::peer_disconnecting_hand));
    RegisterHandler(LookupMessage(IPTConnectionRoutingMsgNum),
                    new HandlerCallback(TCPCommunicator)
                    (this, &TCPCommunicator::connection_routing_hand));
    RegisterHandler(LookupMessage(IPTTranslationMsgNum),
                    new HandlerCallback(TCPCommunicator)
                    (this, &TCPCommunicator::translation_hand));

    _routers = new IPRouterTable();
    TCPRouter* router = new TCPRouter(this, port);
    _routers->add_router(router);
    _port = router->port();
}

/* Create a TCPCommunicator with a name that will connect to a IPT server
   on machine "host_name" */
TCPCommunicator::TCPCommunicator(const char* mod_name, const char* host_name,
                                 void (*setup)(TCPCommunicator*))
    : IPCommunicator(mod_name, host_name)
{
    initialize(0);

    if (setup)
        (*setup)(this);

    /***** implement using best route to server ******/
    _server = MakeConnection(SERVER_NAME, ServerHostName(), SERVER_PORT);

    server_register();

    _server->
        AddDisconnectCallback(new ConnectionCallback(TCPCommunicator)
                              (this,
                               &TCPCommunicator::server_disconnecting_hand));
}

void TCPCommunicator::server_register()
{
    IPMessage* msg = WaitForMsg(_server, IPTVersionInfoMsgNum);
    
    IPTVersionInfoStruct* info = (IPTVersionInfoStruct*) msg->FormattedData();
    if (info->major_version != IPT_MAJOR_VERSION) {
        printf("IPT Error:  Major version %d does not match server major version %d\n", IPT_MAJOR_VERSION, info->major_version);
        exit(-1);
    }

    if (info->minor_version != IPT_MINOR_VERSION)  {
        printf("IPT Warning: Minor version number %d does not match server minor version %d\n", IPT_MINOR_VERSION, info->minor_version);
    }

    if (info->domain_name) {
        int len = strlen(info->domain_name);
        if (len) {
            char* name = new char[++len];
            bcopy(info->domain_name, name, len);
            SetDomainName(name);
        }
    }

    msg->DeleteFormatted(info);
    delete msg;
}

/* Create a TCPCommunicator which will listen for incoming messages on port
   "port".  This creation function is used when creating the IPServer 
   subclass of TCPCommunicator */
TCPCommunicator::TCPCommunicator(int port)
    : IPCommunicator()
{
    initialize(port);
}

void TCPCommunicator::send_routes()
{
    IPTRoutingTableStruct rt;
    rt.num_routes = _routers->num_routers();
    rt.routes = new IPTRoutingStruct[rt.num_routes];
    int i;
    for (i=0;i<rt.num_routes;i++) {
        IPRouting* routing = _routers->router(i)->routing();
        rt.routes[i].type = (char*) routing->type();
        rt.routes[i].sub_type = (char*) routing->sub_type();
        rt.routes[i].comparison = routing->comparison();
        rt.routes[i].parameters = (char*) routing->parameters();
    }

    SendMessage(_server, LookupMessage(IPTRoutingTableMsgNum),
                &rt);

    // ENDO - MEM LEAK
    //delete rt.routes;
    delete [] rt.routes;
    rt.routes = NULL; // Added by ENDO
}

void TCPCommunicator::add_router(IPRouter* router, int where,
                                 const char* other)
{
    IPRouter* other_router;
    if (where == IPT_ROUTE_AFTER)
        other_router = _routers->lookup_router(other);
    else
        other_router = NULL;

    _routers->add_router(router, where, other_router);
}

/* Make a TCP/IP connection to a module named "name" on machine "host"
   at port "port" given that your name is "this_name".  "this_name" seems
   redundant, but there are situations (say in multi IPT server systems)
   where you want to embellish your own name before making the connection .
   This routine assumes that the only kinds of connection being made are
   TCPConnections.  Sub-classes that introduce new connection types
   must override this member function and put in a new version that
   checks connection types */
int TCPCommunicator::register_connection(IPConnection* res,
                                         const char* this_name)
{
    /* set up this name, if necessary */
    int allocated = 0;
    if (!this_name) {
        if (strchr(res->Name(), ':')) {
            allocated = 1;
            this_name = new char[strlen(DomainName())+strlen(ModuleName())+2];
            sprintf((char*) this_name, "%s:%s", DomainName(), ModuleName());
        } else
            this_name = ModuleName();
    }

    /* Send a connection message to the receiving module */
    IPTModuleConnectingStruct mc;
    mc.byte_order = BYTE_ORDER;
    mc.alignment = ALIGN;
    mc.name = (char*) this_name;
    mc.host = (char*) ThisHost();
    int result;
    if (SendMessage(res, IPTModuleConnectingMsgNum, &mc) < 0) {
        printf("Error sending initialization to %s, closing it\n");
        DeactivateConnection(res);
        result = 0;
    } else {
        int ha = HandlersActive();
        if (ha)
            DisableHandlers();
        IPMessage* msg = ReceiveMessage(res, IPT_MODULE_CONNECTING_MSG);
        if (msg) {
            int byte_order = (int) msg->Data()[0];
            if (byte_order == BYTE_ORDER_UNDEFINED)
                result = 0;
            else {
                res->SetByteOrder(byte_order);
                res->SetAlignment((int) msg->Data()[1]);
                result = 1;
            }
            delete msg;
        } else
            result = 0;
        if (ha)
            EnableHandlers();
    }
    if (allocated)
        delete (char*) this_name;

    return result;
}

IPConnection* TCPCommunicator::MakeConnection(const char* name,
                                              const char* host, int port,
                                              const char* this_name,
                                              int fail)
{
    /* make sure we aren't already connected */
    IPConnection* res = LookupConnection(name);
    if (res && res->Active()) {
        return res;
    }

    int sd = connectToModule(name, host, port, fail);
    if (sd < 0) 
        return res;

    /* either create or activate a connection */
    if (!res) {
        ForConnection* cont = new ForConnection(name, this);
        cont->Body(new TCPConnection(name, host, this, sd));
        res = cont;
        printf("Made connection %s (%s)\n", name, host);
        AddConnection(res);
    } else {
        res->Activate(sd, host);
        printf("Reactivated connection %s (%s)\n", name, host);
        ActivateConnection(res);
    }

    if (!register_connection(res, this_name))
        return NULL;

    return res;
}

/* Adds a connection and updates the internal connection mask */
void TCPCommunicator::AddConnection(IPConnection* conn)
{
    IPCommunicator::AddConnection(conn);

    if (conn->Active() && conn->FD() != -1) 
        ActivateConnection(conn);

    if (!_server || conn == _server)
        return;

    const char* colon_ptr = strchr(conn->Name(), ':');
    if (colon_ptr) {
        int len = colon_ptr - conn->Name();
        char* domain_name = new char[len+1];
        bcopy(conn->Name(), domain_name, len);
        domain_name[len] = '\0';
        IPTranslator* translator = lookup_translator(domain_name);
        conn->set_id_translator(translator);
        delete domain_name;
    }
}

/* Updates the connection mask with the new connection file descriptor */
void TCPCommunicator::ActivateConnection(IPConnection* conn)
{
    if (conn->FD() != -1)
        FD_SET(conn->FD(), _connection_mask);
}

/* Updates the connection mask by removing the connection file descriptor
   and deactivates the connection */
void TCPCommunicator::DeactivateConnection(IPConnection* conn)
{
    if (conn->Active()) {
        FD_CLR(conn->FD(), _connection_mask);

        IPCommunicator::DeactivateConnection(conn);
    }
}

/* Close the connection and free memory */
TCPCommunicator::~TCPCommunicator()
{
    delete _connection_mask;
    for (IPTranslator* t = (IPTranslator*) _translators->First();
         t; t = (IPTranslator*) _translators->Next())
        delete t;
    IPList::Delete(_translators);
    delete _routers;
}
    
/* The method used for waiting for input for a TCP/IP communicator.  We
   use a select on the connection mask to check for pending input.  Then
   we go through the active connections to see if their file descriptors
   match the active file descriptors gotten from select.  If so, we
   add a DataEvent to the event queue to flag that there is data pending
   from that connection */
int TCPCommunicator::WaitForInput(double timeout)
{
    int stat;
    fd_set readMask = *_connection_mask;
    struct timeval* tp;
    struct timeval tv;
    int res = 0;

    if (Finished())
        return 0;

    /* format the timeout */
    if (timeout == IPT_BLOCK) {
        tp = 0;
    } else {
        IPTime t(timeout);
        tv.tv_sec = t.Sec();
        tv.tv_usec = t.MSec()*1000;
        tp = &tv;
    }

    StartBlocking();
    /* check for incoming data */
    do {
        stat = select(FD_SETSIZE, &readMask, NULL, NULL, tp);
    } while (stat < 0 && errno == EINTR);
    EndBlocking();
  
    if (stat <= 0) 
        return 0;

    /* monitor administrative connections */
    IPConnection* conn = (IPConnection*) AdministrativeConnections()->First();
    for (;conn;conn = (IPConnection*) AdministrativeConnections()->Next())
        if (conn->FD() != -1 && FD_ISSET(conn->FD(), &readMask)) {
            AddEvent(new AdministrativeEvent(conn));
            res = 1;
        }

    /* flag incoming data from individual connections */
    conn = (IPConnection*) Connections()->First();
    for (;conn;conn = (IPConnection*) Connections()->Next())
        if (conn->Active() && conn->FD() != -1 &&
            FD_ISSET(conn->FD(), &readMask)) {
            res = 1;
            if (!conn->DataAvailable())
                DeactivateConnection(conn);
            else {
                AddEvent(new DataEvent(conn));
            }
        }

    return res;
}

IPConnection* TCPCommunicator::add_connection(const char* type,
                                              IPConnection* temp)
{
    IPRouter* router = _routers->lookup_router(type);
    if (!router) {
        printf("IPT(TCP): No router %s, failing connection for %s\n",
               type, temp->Name());
        return NULL;
    }

    /* find out the name and host of the connecting module */
    IPMessage* msg = temp->Receive();
    if (!msg) {
        printf("Warning: error setting up connection\n");
        return NULL;
    }
    temp->SetByteOrder((int) msg->Data()[0]);
    temp->SetAlignment((int) msg->Data()[1]);

    /* Make or activate the real connection instance */
    IPTModuleConnectingStruct* mc =
        (IPTModuleConnectingStruct*) msg->FormattedData();
    ForConnection *res = (ForConnection*) LookupConnection(mc->name);
    if (res && res->Body() && res->Active()) {
        if (res->ByteOrder() == BYTE_ORDER_UNDEFINED) {
            if (!resolve_collision(res, temp))
                return NULL;
        } else {
            fprintf(stderr,
                    "Module %s (%s) trying to connect more than once\n",
                    mc->name, mc->host);
            temp->Deactivate();
            return NULL;
        }
    }
    if (res && res->Body() && !strcmp(res->Body()->Type(), type)) {
        printf("IPT(TCP): Activating connection from %s on %s (%d)\n",
               mc->name, mc->host, temp->FD());

        res->activate(mc->host, temp);
        ActivateConnection(temp);
    } else {
        printf("IPT(TCP): Adding connection from %s on %s (%d)\n",
               mc->name, mc->host, temp->FD());
        IPConnection* body = router->copy_connection(mc->name, mc->host, temp);
        res = (ForConnection*) DeclareConnection(mc->name);
        res->Body(body);
        ActivateConnection(res);
    }

    res->SetByteOrder(temp->ByteOrder());
    res->SetAlignment(temp->Alignment());

    /* set up this name, if necessary */
    char* this_name;
    int allocated = 0;
    if (strchr(res->Name(), ':')) {
        allocated = 1;
        this_name = new char[strlen(DomainName())+strlen(ModuleName())+2];
        sprintf((char*) this_name, "%s:%s", DomainName(), ModuleName());
    } else
        this_name = (char*) ModuleName();

    /* Send a connection message to the receiving module */
    IPTModuleConnectingStruct mmc;
    mmc.byte_order = BYTE_ORDER;
    mmc.alignment = ALIGN;
    mmc.name = (char*) this_name;
    mmc.host = (char*) ThisHost();
    SendMessage(res, IPTModuleConnectingMsgNum, &mmc);
    if (allocated)
        delete this_name;

    /* do the connection administrata */
    connect_notify(res);

    /* get all available messages from the module */
    while (res->DataAvailable())
        res->GetMessage();

    /* clean up the message */
    delete msg;

    return res;
}

/* declare that a connection to a module named "name" will be made.  Adds
   an empty forwarded connection to the connection list, if it has not already
   been done */
IPConnection* TCPCommunicator::DeclareConnection(const char* name)
{
    IPConnection* res = LookupConnection(name);
    if (res)
        return res;

    res = new ForConnection(name, this);
    AddConnection(res);

    return res;
}

/* initiate a connection to module named "name".  If required is IPT_REQUIRED
   block until the module connect.  If "required" is IPT_OPTIONAL, then return
   an unactivated connection if the IPServer knows nothing about the module
   named "name", otherwise return the connection.  If "required" is
   IPT_NONBLOCKING and the module is not already connected the routine will
   always return an unactivated connection, but it will register a standing
   request with the server for a connection to "name" and when a module named
   "name" connects, this module will connect to that one. This routine should
   only return NULL on an error */
IPConnection* TCPCommunicator::Connect(const char* name,
                                       int required)
{
    /* strip off domain name if not really an inter-domain request */
    if (DomainName()) {
        const char* colon_ptr = strchr(name, ':');
        if (colon_ptr) {
            int len = colon_ptr - name;
            if (len == (int) strlen(DomainName()) &&
                !bcmp((char*) name, (char*) DomainName(), len))
                name = colon_ptr+1;
        }
    }
            
    /* make sure we aren't already connected and active */
    IPConnection* res = LookupConnection(name);
    if (res && res->Active()) {
        return res;
    }

    if (!_server)
        return NULL;

    printf("IPT(TCP): Making connection to %s (%d)\n", name, required);

    /* Query server for connection information */
    IPTRequestConnectionStruct rc;
    rc.name = (char*) name;
    rc.required = required;

    while (1) {
        if (required == IPT_NONBLOCKING) {
            SendMessage(_server, IPTRequestConnectionMsgNum, &rc);
            res = DeclareConnection(name);
        } else {
            IPMessage* msg = Query(_server, IPTRequestConnectionMsgNum, &rc,
                                   IPTConnectionRoutingMsgNum);
            if (!msg) {
                printf("IPT Error getting connection to %s\n", name);
                if (!_server->Active()) {
                    printf("IPT: Server inactive...");
                    fflush(stdout);
                    while (!_server->Active()) {
                        printf(".");
                        fflush(stdout);
                        Idle(1.0);
                    }
                }
                res = NULL;
            } else {
                res = make_connection(msg);
                delete msg;
            }
        }

        if (required != IPT_REQUIRED || (res && res->Active())) {
            return res;
        }
#ifdef VXWORKS
        taskDelay(20);
#else
        usleep(300000);
#endif
        printf("IPT(TCP): Trying connection to %s again\n", name);
    }
}

IPConnection* TCPCommunicator::make_connection(IPMessage* msg)
{
    IPTConnectionRoutingStruct* cr =
        (IPTConnectionRoutingStruct*) msg->FormattedData();

    if (!cr->type || !strlen(cr->type))
        return DeclareConnection(cr->name);

    IPRouter* router = _routers->lookup_router(cr->type);
    if (!router) {
        printf("IPT(TCP): Unknown router %s, failing %s\n",
               cr->type, cr->name);
        return DeclareConnection(cr->name);
    }

    ForConnection* res = (ForConnection*) LookupConnection(cr->name);
    if (!res || !res->Body() || strcmp(res->Body()->Type(), cr->type)) {
        IPConnection* body =
            router->route_connection(cr->name, cr->sub_type, cr->parameters);
        if (!res)
            res = new ForConnection(cr->name, this);
        res->Body(body);
        AddConnection(res);
    } else {
        router->activate_connection(res->Body(), cr->sub_type, cr->parameters);
        if (res->FD() != -1)
            ActivateConnection(res);
    }
     
    if (res && res->Active()) {
        if (!register_connection(res))
            return NULL;

        connect_notify(res);
    }

    ProcessEvents();

    return res;
}    

/* Queries the server for the message ID of the message with name "name" */
int TCPCommunicator::get_message_id(const char* name)
{
    IPMessage* msg = Query(_server, IPTRegisterMessageMsgNum, &name,
                           IPTMessageRegisteredMsgNum);
    if (msg) {
        int id = *(int*) msg->FormattedData();
        delete msg;

        return id;
    }

    return -1;
}

/* register "n" mesages with names in the array "names" with the IPServer,
   getting the message ID for each one */
int* TCPCommunicator::register_messages(int n, char** names)
{
    IPTRegisterMessageSetStruct rms;
    rms.n = n;
    rms.names = names;

    IPMessage* msg = Query(_server, IPTRegisterMessageSetMsgNum,  &rms,
                           IPTMessageSetRegisteredMsgNum);
    if (!msg)
        return NULL;

    IPTMessageSetRegisteredStruct* msr =
        (IPTMessageSetRegisteredStruct*) msg->FormattedData();

    int* res = new int[n];
    bcopy((char*) msr->ids, (char*) res, n*sizeof(int));

    msg->DeleteFormatted(msr);
    delete msg;

    return res;
}

void TCPCommunicator::LogSend(IPConnection* conn, IPMessage* msg)
{
    if (conn == _server)
        return;
    if (!_server) {
        IPCommunicator::LogSend(conn, msg);
        return;
    }
    if (!_server->Active())
        return;
    
    IPTLoggingStruct log;
    log.operation = 0;
    if (msg->Type()) 
        log.msg_name = msg->Type()->Name();
    else
        log.msg_name = NULL;
    log.msg_id = msg->ID();
    log.instance = msg->Instance();
    log.origin = ModuleName();
    if (conn) 
        log.destination = conn->Name();
    else
        log.destination = NULL;
    log.size = msg->SizeData();
    log.extra = NULL;

    SendMessage(_server, IPT_LOGGING_MSG, (void*) &log);
}

void TCPCommunicator::LogReceive(IPConnection* conn, IPMessage* msg,
                                 const char* extra)
{
    if (conn == _server)
        return;
    if (!_server) {
        IPCommunicator::LogReceive(conn, msg, extra);
        return;
    }
    if (!_server->Active())
        return;
    
    IPTLoggingStruct log;
    log.operation = 1;
    if (msg->Type()) 
        log.msg_name = msg->Type()->Name();
    else
        log.msg_name = NULL;
    log.msg_id = msg->ID();
    log.instance = msg->Instance();
    log.origin = ModuleName();
    if (conn) 
        log.destination = conn->Name();
    else
        log.destination = NULL;
    log.size = msg->SizeData();
    log.extra = extra;

    SendMessage(_server, IPT_LOGGING_MSG, (void*) &log);
}
        
IPConnection* TCPCommunicator::
AddAdministrativeConnection(int fd,
                            IPConnectionCallback* callback,
                            IPConnectionCallback* close_cb)
{
    IPConnection* conn = IPCommunicator::AddAdministrativeConnection(fd,
                                                                     callback,
                                                                     close_cb);
    
    FD_SET(fd, _connection_mask);

    return conn;
}

void TCPCommunicator::RemoveAdministrativeConnection(int fd)
{
    IPCommunicator::RemoveAdministrativeConnection(fd);

    FD_CLR(fd, _connection_mask);
}
        
void TCPCommunicator::close_socket_hand(IPConnection* conn)
{
    if (conn->FD() != -1)
        close_socket(conn->FD());
}
    
int TCPCommunicator::add_translator(int id, IPConnection* conn)
{
    IPTranslator* translator_table = conn->translator();

    if (!_server)
        return IPCommunicator::add_translator(id, conn);

    IPTRequestTranslationStruct rt;
    rt.domain_name = (char*) translator_table->domain_name();
    rt.conn = (char*) conn->Name();
    rt.id = id;
    SendMessage(_server, LookupMessage(IPTRequestTranslationMsgNum), &rt);

    translator_table->make_translation(id, -1);

    return -1;
}

void TCPCommunicator::translation_hand(IPMessage* msg)
{
    IPTTranslationStruct* trans = (IPTTranslationStruct*) msg->FormattedData();
    IPConnection* conn = LookupConnection(trans->conn);
    IPTranslator* translator_table = conn->translator();

    if (!translator_table) {
        printf("IPT(TCP): Cannot translate for connection %s without translator\n", 
               trans->conn);
        return;
    }

    if (trans->translated_id == -1) {
        printf("IPT(TCP): Cannot translate message %d for domain %s, assuming the same\n",
               trans->id, translator_table->domain_name());
    } else
        translator_table->make_translation(trans->id, trans->translated_id);
}

IPTranslator* TCPCommunicator::lookup_translator(const char* domain_name)
{
    IPTranslator* t;
    for (t = (IPTranslator*) _translators->First();
         t; t = (IPTranslator*) _translators->Next()) 
        if (!strcmp(domain_name, t->domain_name())) {
            _translators->Append((char*) t);
            return t;
        }

    t = new IPTranslator(domain_name);
    _translators->Append((char*) t);

    return t;
}

void TCPCommunicator::peer_disconnecting_hand(IPMessage* msg)
{
    if (!_translators->Length())
        return;

    char* dead_domain = *(char**) msg->FormattedData();
    for (IPTranslator* t = (IPTranslator*) _translators->First(); t;
         t = (IPTranslator*) _translators->Next()) 
        if (!strcmp(dead_domain, t->domain_name())) {
            t->clear_translations();
            for (IPConnection* conn = (IPConnection*) Connections()->First();
                 conn; conn = (IPConnection*) Connections()->Next()) 
                if (conn->translator() == t)
                    DeactivateConnection(conn);
            return;
        }
}

void TCPCommunicator::connection_routing_hand(IPMessage* msg)
{
    make_connection(msg);
}

void TCPCommunicator::server_disconnecting_hand(IPConnection*)
{
    AddTimer(0.5,
             new TimerCallback(TCPCommunicator)(this,
                                                &TCPCommunicator::try_server));
}

static int count_types(char*, char* elem, void* data)
{
    IPMessageType* type = (IPMessageType*) elem;
    IPTReregisterMessagesStruct* rm = (IPTReregisterMessagesStruct*) data;
    
    if (type->ID() > IPTLastBuiltInMsgNum)
        rm->num_messages++;

    return 1;
}

static int pack_types(char*, char* elem, void* data)
{
    IPMessageType* type = (IPMessageType*) elem;
    IPTReregisterMessagesStruct* rm = (IPTReregisterMessagesStruct*) data;
    
    if (type->ID() > IPTLastBuiltInMsgNum) {
        rm->messages[rm->num_messages].msg_id = type->ID();
        rm->messages[rm->num_messages].msg_name = type->Name();
        rm->num_messages++;
    }

    return 1;
}

void TCPCommunicator::try_server(IPTimer* timer)
{
    _server = MakeConnection(SERVER_NAME, ServerHostName(), SERVER_PORT,
                             NULL, 1);

    if (!_server || !_server->Active())
        return;

    server_register();
    send_routes();

    /* remove the timer */
    RemoveTimer(timer);

    /* report the messages */
    IPTReregisterMessagesStruct rm;
    rm.num_messages = 0;
    MessageTable()->Iterate(count_types, (void*) &rm);
    rm.messages = new IPTMessageIDStruct[rm.num_messages];
    rm.num_messages = 0;
    MessageTable()->Iterate(pack_types, (void*) &rm);
    SendMessage(_server, IPT_REREGISTER_MESSAGES_MSG, (void*) &rm);
    delete rm.messages;
    
    /* report the connections */
    IPTReportConnectionsStruct rc;
    IPList* connections = Connections();
    IPConnection* conn;

    rc.num_conns = 0;
    for (conn = (IPConnection*) connections->First(); conn;
         conn = (IPConnection*) connections->Next()) 
        if (conn != _server && conn->Active())
            rc.num_conns++;

    int i = 0;;
    rc.conn_names = (const char**) new char*[rc.num_conns];
    for (conn = (IPConnection*) connections->First(); conn;
         conn = (IPConnection*) connections->Next()) 
        if (conn != _server && conn->Active())
            rc.conn_names[i++] = conn->Name();

    SendMessage(_server, IPT_REPORT_CONNECTIONS_MSG, (void*) &rc);

    delete (char**) rc.conn_names;
}

int TCPCommunicator::resolve_collision(ForConnection* existing,
                                       IPConnection* incoming)
{
    if (strcmp(existing->Name(), ModuleName()) <= 0) {
        IPTModuleConnectingStruct mc;
        mc.byte_order = (char) BYTE_ORDER_UNDEFINED;
        mc.alignment = (char) ALIGN_UNDEFINED;
        mc.name = (char*) ModuleName();
        mc.host = (char*) ThisHost();
        SendMessage(incoming, IPTModuleConnectingMsgNum, &mc);
        incoming->Deactivate();
        return 0;
    }

    DeactivateConnection(existing);
    existing->Body(NULL);
    return 1;
}
    
