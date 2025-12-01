///////////////////////////////////////////////////////////////////////////////
//
//                                 server.cc
//
// This file declares the server class.  This class can be used to implement
// the central "operator" for IPT.  The server has two main purposes,
// first, to provide a consistent mapping between message name (strings) and
// message IDs (numbers).  Second, to arrange the connections between client
// modules.  The servers are set up so that several of them can be connected
// together.  Each server manages its own "domain" and each domain can have
// a name.  
// 
// Classes defined for export:
//    IPServer
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

#include "./libc.h"

#include <ipt/server.h>
#include <ipt/internal_messages.h>
#include <ipt/callbacks.h>
#include <ipt/list.h>
#include <ipt/message.h>
#include <ipt/connection.h>
#include <ipt/messagetype.h>
#include <ipt/hash.h>
#include <ipt/primcomm.h>
#include <ipt/formconfig.h>
#include <ipt/pending.h>
#include <ipt/conn_net.h>
#include <ipt/route.h>
#include <ipt/fakeconnection.h>
#include <ipt/filter.h>

declareConnectionCallback(IPServer);
implementConnectionCallback(IPServer);

declareHandlerCallback(IPServer);
implementHandlerCallback(IPServer);

struct PeerDescriptor {
    PeerDescriptor(const char* name, const char* host);
    ~PeerDescriptor() { delete name; delete host; delete translations; }

    char* name;
    char* host;
    IPConnection* conn;
    IPHashTable* translations;
};

/* When a connection request is pending, we create one of these and add it to 
   the pending connection request list */
class ConnectionRequest {
  public:
    ConnectionRequest(IPConnection*, int, const char* name);
    virtual ~ConnectionRequest();

    const char* Target() const { return _target; }
    IPConnection* Connection() const { return _connection; }
    int Instance() const { return _instance; }

    virtual void ArrangeConnection(IPServer*, IPConnection*);

  private:
    char *_target;                /* Name of module we want to connect to */
    IPConnection* _connection;    /* Connetion to requesting module */
    int _instance;                /* instance number of the request message */
};

/* When a remote connection request is pending, i.e., a request for connection
   to a module which is in another domain, use one of these on the connection 
   request list */
class RemoteConnectionRequest : public ConnectionRequest { 
  public:
    RemoteConnectionRequest(IPConnection*, int, const char* name);

    virtual void ArrangeConnection(IPServer*, IPConnection*);
};

class TableRequest : public IndirectRequest {
  public:
    TableRequest(IPServer* server, char* full_name, IPRoutingTable* req_table,
                 IPConnection* initiator, int instance,
                 IPConnection* receptor, ConnectionNet* net,
                 IPMessage* msg = NULL)
        : IndirectRequest(initiator, instance, receptor, msg)
            {
                _server = server; _full_name = full_name; _req_table=req_table;
                _conn_net = net;
            }
    virtual ~TableRequest() { delete _full_name; }
    
    virtual void service(IPMessage*);
    virtual void clear(IPConnection*);

  private:
    IPServer* _server;
    char* _full_name;
    IPRoutingTable* _req_table;
    ConnectionNet* _conn_net;
};

static char* copy_string(const char*);

class ReconnectRequest : public PendingRequest {
  public:
    ReconnectRequest(ConnectionNet* net, IPConnection* initiator,
                     const char* name1, const char* name2)
        : PendingRequest(initiator) 
            { _net = net;
              _name1 = copy_string(name1); _name2 = copy_string(name2); }
    virtual ~ReconnectRequest() { delete _name1; delete _name2; }
    virtual void service(IPMessage*);
    virtual void clear(IPConnection*) {}

  private:
    ConnectionNet* _net;
    char* _name1;
    char* _name2;
};

class ConfirmRequest : public IndirectRequest {
  public:
    ConfirmRequest(IPServer* server, const char* full_name, IPRouting* route,
                   IPConnection* initiator, int instance,
                   IPConnection* receptor, IPMessage* msg = NULL)
        : IndirectRequest(initiator, instance, receptor, msg)
            {
                _server = server; _full_name = copy_string(full_name);
                _route = route;
            }
    virtual ~ConfirmRequest() { delete _full_name; }
    
    virtual void service(IPMessage*);
    virtual void clear(IPConnection*);

  private:
    IPServer* _server;
    char* _full_name;
    IPRouting* _route;
};

class InitFilter : public IPFilter {
  public:
    virtual int Check(IPMessage* msg, IPConnection*) {
        return msg->Type() &&
            ((msg->Type()->ID() == IPTReportConnectionsMsgNum) ||
             (msg->Type()->ID() == IPTReregisterMessagesMsgNum));
    }
};

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

/* Take a module name and a domain name and allocate and return the 
   resulting full name */
static char* make_full_name(const char* name, const char* domain)
{
    char* res = new char[strlen(domain)+strlen(name)+2];
    sprintf(res, "%s:%s", domain, name);

    return res;
}

PeerDescriptor::PeerDescriptor(const char* n, const char* h)
{
    name = copy_string(n);
    host = copy_string(h);
    conn = NULL;
    translations = new IPHashTable(300, int_hash, int_eq);
}

/* Make a connection request from client connected by "conn" which made
   the request with a query with instance "instance" for a module named 
   "target" */
ConnectionRequest::ConnectionRequest(IPConnection* conn, int instance,
                                     const char* target)
{
    _instance = instance;
    _target = copy_string(target);

    _connection = conn;
}

/* Delete a connection request */
ConnectionRequest::~ConnectionRequest()
{
    delete _target;
}

/* Fulfill a connection request to the server "server" with connectin "conn" */
void ConnectionRequest::ArrangeConnection(IPServer* server, IPConnection* conn)
{
    server->arrange_connection(Connection(), conn, Instance());
}

/* Create a remote connection request for module "target", i.e., a connection 
   that comes one-hopped through another domain server.  The server is 
   connected by connection "conn" and the query instance is "instance".  The
   request is on behalf of a module named "req_name" on "req_host" */
RemoteConnectionRequest::RemoteConnectionRequest(IPConnection* conn,
                                                 int instance,
                                                 const char* target)
    : ConnectionRequest(conn, instance, target)
{
}

/* Have "server" arrange a remote connection to the module connected by
   "conn" */
void RemoteConnectionRequest::ArrangeConnection(IPServer* server,
                                                IPConnection* conn)
{
    server->arrange_remote_connection(Connection(), conn, Instance());
}

/* Create an IPT server with name "name", domain name "name",  an a priori 
   message file named "message_file", an array of domain specifiers named 
   "domains" and a log file "log_file". If "message_file" is NULL, there is no
   a priori message file. If "log_file" is NULL, then all logging will be done
   to standard out.  If "domains" is NULL then there are no other peer 
   domains.  */
IPServer::IPServer(const char* name, const char* message_file,
                   const char* domain_name, IPDomainSpec* peer_domains,
                   const char* log_file)
    : TCPCommunicator(SERVER_PORT)
{
    printf("\t\t\tIPT Server\n");
    printf("\t\t\tVersion %d.%d.%d\n", IPT_MAJOR_VERSION, IPT_MINOR_VERSION,
           IPT_SUB_VERSION);
    printf("\t\tRunning on: %s\n", ThisHost());
    if (domain_name) {
        printf("\t\tDomain Name: %s\n", domain_name);
    }
    if (message_file)
        printf("\t\tMessage File: %s\n", message_file);
    if (log_file) {
        printf("\t\tLog File: %s\n", log_file);
        _log_fp = fopen(log_file, "w");
        if (!_log_fp) {
            printf("Cannot open log file %s\n", log_file);
            exit(-1);
        } 
    } else
        _log_fp = stdout;

    ModuleName(name);

    AddConnectCallback(new ConnectionCallback(IPServer)(this,
                                                        &IPServer::
                                                        mod_connecting));
    AddDisconnectCallback(new ConnectionCallback(IPServer)(this,
                                                           &IPServer::
                                                           mod_disconnecting));

    register_message(IPTRequestDomainNameMsgNum, IPT_REQUEST_DOMAIN_NAME_MSG,
                     IPT_REQUEST_DOMAIN_NAME_FORM);
    register_message(IPTDomainNameMsgNum, IPT_DOMAIN_NAME_MSG,
                     IPT_DOMAIN_NAME_FORM);
    register_message(IPTServerDumpMsgNum, IPT_SERVER_DUMP_MSG,
                     IPT_SERVER_DUMP_FORM);
    register_message(IPTServerFlushMsgNum, IPT_SERVER_FLUSH_MSG,
                     IPT_SERVER_FLUSH_FORM);
    register_message(IPTConfirmRemoteConnectionMsgNum,
                     IPT_CONFIRM_REMOTE_CONNECTION_MSG,
                     IPT_CONFIRM_REMOTE_CONNECTION_FORM);
    register_message(IPTRemoteConnectionConfirmedMsgNum,
                     IPT_REMOTE_CONNECTION_CONFIRMED_MSG,
                     IPT_REMOTE_CONNECTION_CONFIRMED_FORM);
    register_message(IPTServerDumpNetMsgNum, IPT_SERVER_DUMP_NET_MSG,
                     IPT_SERVER_DUMP_NET_FORM);
    register_message(IPTClearRemoteConnectionMsgNum,
                     IPT_CLEAR_REMOTE_CONNECTION_MSG,
                     IPT_CLEAR_REMOTE_CONNECTION_FORM);
    register_message(IPTServerFlushNetMsgNum, IPT_SERVER_FLUSH_NET_MSG,
                     IPT_SERVER_FLUSH_NET_FORM);
    register_message(IPTPeerConnectingMsgNum, IPT_PEER_CONNECTING_MSG,
                     IPT_PEER_CONNECTING_FORM);
    register_message(IPTRoutingTableRequestMsgNum,
                     IPT_ROUTING_TABLE_REQUEST_MSG,
                     IPT_ROUTING_TABLE_REQUEST_FORM);
    register_message(IPTRequestedRoutingTableMsgNum,
                     IPT_REQUESTED_ROUTING_TABLE_MSG,
                     IPT_REQUESTED_ROUTING_TABLE_FORM);
    register_message(IPTReportRemoteConnectionMsgNum,
                     IPT_REPORT_REMOTE_CONNECTION_MSG,
                     IPT_REPORT_REMOTE_CONNECTION_FORM);

    RegisterHandler(LookupMessage(IPTRequestDomainNameMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  req_domain_name_hand));
    RegisterHandler(LookupMessage(IPTLoggingMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  logging_hand));
    RegisterHandler(LookupMessage(IPTDeletionActionMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::deletion_hand));
    RegisterHandler(LookupMessage(IPTServerDumpMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  server_dump_hand));
    RegisterHandler(LookupMessage(IPTServerFlushMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  server_flush_hand));
    RegisterHandler(LookupMessage(IPTConfirmRemoteConnectionMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  remote_confirm_hand));
    RegisterHandler(LookupMessage(IPTServerDumpNetMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  server_dump_net_hand));
    RegisterHandler(LookupMessage(IPTClearRemoteConnectionMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  remote_clear_hand));
    RegisterHandler(LookupMessage(IPTServerFlushNetMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  server_flush_net_hand));
    RegisterHandler(LookupMessage(IPTRequestTranslationMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  request_translation_hand));
    RegisterHandler(LookupMessage(IPTPeerConnectingMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  peer_connecting_hand));
    RegisterHandler(LookupMessage(IPTRoutingTableMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  routing_table_hand));
    RegisterHandler(LookupMessage(IPTRoutingTableRequestMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  routing_table_req_hand));
    RegisterHandler(LookupMessage(IPTReportRemoteConnectionMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  remote_report_hand));
    
    _pending_register = new PendingCallback();
    RegisterHandler(LookupMessage(IPTMessageRegisteredMsgNum),
                    _pending_register);

    _pending_tables = new PendingCallback();
    RegisterHandler(LookupMessage(IPTRequestedRoutingTableMsgNum),
                    _pending_tables);

    _pending_confirms = new PendingCallback();
    RegisterHandler(LookupMessage(IPTRemoteConnectionConfirmedMsgNum),
                    _pending_confirms);

    _request_list = IPList::Create();
    _next_message_num = LOWEST_MESSAGE_NUMBER;
    _conn_net = new ConnectionNet();
    _peer_table = new IPHashTable(32, str_hash, str_eq);
    _routing_tables = new IPHashTable(32, str_hash, str_eq);

    FILE* fp;
    if (message_file && (fp = fopen(message_file, "r"))) 
        read_message_file(fp);
    if (message_file && !fp)
        printf("Message file %s not read\n", message_file);

    SetDomainName(copy_string(domain_name));

    int peer_count = 0;
    if (peer_domains && peer_domains->name && peer_domains->host) {
        if (!domain_name) 
            printf("***Peers are irrelevant without a defined domain name***\n");
        else {
            for (;peer_domains[peer_count].name;peer_count++) {
                printf("\t\tPeer %s on %s\n", peer_domains[peer_count].name,
                       peer_domains[peer_count].host);
                add_peer(peer_domains[peer_count].name,
                         peer_domains[peer_count].host);
            }
        }
    }

    /* Try and handle recovery messages before other initialization messages */
    IPMessage* msg;
    InitFilter f;
    while ((msg = ReceiveMessage(&f, 1.0))) {
        if (msg->Type()->ID() == IPTReportConnectionsMsgNum)
            report_connections_hand(msg);
        else if (msg->Type()->ID() == IPTReregisterMessagesMsgNum)
            reregister_messages_hand(msg);
        delete msg;
    }

    RegisterHandler(LookupMessage(IPTRequestConnectionMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  req_connection_hand));
    RegisterHandler(LookupMessage(IPTRegisterMessageMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  register_message_hand));
    RegisterHandler(LookupMessage(IPTRegisterMessageSetMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  register_message_set_hand));
    RegisterHandler(LookupMessage(IPTReportConnectionsMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  report_connections_hand));
    
    RegisterHandler(LookupMessage(IPTReregisterMessagesMsgNum),
                    new HandlerCallback(IPServer)(this,
                                                  &IPServer::
                                                  reregister_messages_hand));
    
}

static int delete_peers(char*, char* data)
{
    delete (PeerDescriptor*) data;

    return 1;
}

static int delete_routes(char*, char* data)
{
    if (!data)
        return 1;

    delete (IPRoutingTable*) data;

    return 1;
}


/* Delete an IPT server */
IPServer::~IPServer()
{
    disable_disconnect_handlers();
    
    IPList::Delete(_request_list);
    delete _conn_net;
    _peer_table->Iterate(delete_peers);
    delete _peer_table;
    _routing_tables->Iterate(delete_routes);
    delete _routing_tables;
}

/* Read in the a priori message file.  Each line should be either blank, 
   start with a # and have a comment, or contain a single message name.  
   Message names defined this way cannot contain spaces */
void IPServer::read_message_file(FILE* fp)
{
    char buffer[500];
    int id;

    while (1) {
        if (!fgets(buffer, 499, fp))
            return;

        if (buffer[0] == '#')
            continue;

        char* id_string = strrchr(buffer, ' ');
        if (!id_string)
            continue;
        *id_string++ = '\0';
        char* message_name = buffer;
        if (sscanf(id_string, "%d", &id) != 1)
            continue;

        register_message(id, message_name, NULL);
        if (id >= _next_message_num)
            _next_message_num = id+1;
    }
}

/* Used to check if a connection passed in as "param" matches the connection
   request passed in as "item".  Used as an iterator over IPServer's 
   _request_list */
static int check_request(const char* param, const char* item)
{
    ConnectionRequest* req = (ConnectionRequest*) item;
    IPConnection* conn = (IPConnection*) param;

    if (!strcmp(conn->Name(), req->Target())) {
        req->ArrangeConnection((IPServer*) conn->Communicator(), conn);
        delete req;
        return 1;
    }

    return 0;
}

/* Called whenever a module connects to the server.  Sends the version number
   back to the module, and then checks to see if the connection satisfies any
   pending connection requests */
void IPServer::mod_connecting(IPConnection* conn)
{
    IPTVersionInfoStruct v;
    v.major_version = IPT_MAJOR_VERSION;
    v.minor_version = IPT_MINOR_VERSION;
        
    if (DomainName())
        v.domain_name = (char*) DomainName();
    else
        v.domain_name = "";

    SendMessage(conn, IPTVersionInfoMsgNum, &v);

}

static int dead_request(const char* param, const char* item)
{
    ConnectionRequest* req = (ConnectionRequest*) item;
    IPConnection* conn = (IPConnection*) param;

    if (conn == req->Connection()) {
        delete req;
        return 1;
    }

    return 0;
}

void IPServer::mod_disconnecting(IPConnection* conn)
{
    _pending_register->clear(conn);
    _pending_tables->clear(conn);
    _pending_confirms->clear(conn);
    remove_routing(conn->Name());
    IPList* domains = _conn_net->domains(conn->Name());

    for (char* domain_name = domains->First(); domain_name;
         domain_name = domains->Next())
        clear_remote_connection(domain_name, conn->Name());
    _conn_net->disconnect(conn->Name());
    _request_list->RemoveAll(dead_request, (char*) conn);
}

int IPServer::duplicate_request(IPConnection* conn, const char* name)
{
    ConnectionRequest* req = (ConnectionRequest*) _request_list->First();
    for (;req;req = (ConnectionRequest*) _request_list->Next()) 
        if (conn == req->Connection() && !strcmp(name, req->Target()))
            return 1;

    return 0;
}

void IPServer::remove_request(IPConnection* conn, const char* name)
{
    ConnectionRequest* req = (ConnectionRequest*) _request_list->First();
    for (;req;req = (ConnectionRequest*) _request_list->Next()) 
        if (conn == req->Connection() && !strcmp(name, req->Target())) {
            _request_list->Remove((char*) req);
            return;
        }
}

/* Handler for the connection request message.  */
void IPServer::req_connection_hand(IPMessage* msg)
{
    IPTRequestConnectionStruct* rc =
        (IPTRequestConnectionStruct* ) msg->FormattedData();

    IPRoutingTable* req_table = lookup_routing(msg->Connection()->Name());
    if (!req_table) {
        printf("IPT(Server): %s cannot request a connection until it registers a routing table\n",
               msg->Connection()->Name());
        fail_connection_request(rc->name, msg);
        return;
    }

    printf("IPT(Server): Module %s (%s) requesting connection to %s (%d)\n",
           msg->Connection()->Name(), msg->Connection()->Host(),
           rc->name, rc->required);

    if (_conn_net->connected(rc->name, msg->Connection()->Name())) {
        printf("IPT(Server): Already connected\n");
        fail_connection_request(rc->name, msg);
        return;
    }

    IPRoutingTable* table = lookup_routing(rc->name);
    if (table) {
        routing_reply(msg, rc->name, match_routings(req_table, table));
        return;
    }

    /* Check if this is an inter-domain connectino request */
    char* colon_ptr = (char*) strchr(rc->name, ':');
    char* domain_name = NULL;
    if (colon_ptr) {   /* yes, strip off the domain name and module name */
        domain_name = (char*) rc->name;
        rc->name = colon_ptr+1;
        *colon_ptr = '\0';
    }

    if (!colon_ptr || (DomainName() && !strcmp(domain_name, DomainName()))) {
        if (rc->required == IPT_REQUIRED) {
            remove_request(msg->Connection(), rc->name);
            ConnectionRequest* cr =
                new ConnectionRequest(msg->Connection(), msg->Instance(),
                                      rc->name);
            _request_list->Append((char*) cr);
        } else if (rc->required == IPT_NONBLOCKING) {
            if (!duplicate_request(msg->Connection(), rc->name)) {
                ConnectionRequest* cr =
                    new ConnectionRequest(msg->Connection(), msg->Instance(),
                                          rc->name);
                _request_list->Append((char*) cr);
            }
        } else {
            printf("IPT(Server): No module %s on optional\n", rc->name);
            fail_connection_request(rc->name, msg);
        }
    } else {  // if a inter-domain-request

        /* first, make sure we are connected to that domain server */
        IPConnection* dom_server = connect_to_domain(domain_name);
        char* full_name = make_full_name(rc->name, domain_name);
        if (!dom_server || !dom_server->Active()) {
            printf("IPT(Server): Module %s is invalid domain, failing connection request\n", domain_name);
            fail_connection_request(full_name, msg);
            delete full_name;
        } else {   // yes
            IPTRoutingTableRequestStruct rtr;
            rtr.name = (char*) rc->name;
            rtr.required = rc->required;
            _pending_tables->query(dom_server,
                                   LookupMessage(IPTRoutingTableRequestMsgNum),
                                   &rtr,
                                   new TableRequest(this, full_name,
                                                    req_table,
                                                    msg->Connection(),
                                                    msg->Instance(),
                                                    dom_server, _conn_net));
        }
    }

    // make sure we delete the right thing 
    if (domain_name)
        rc->name = domain_name;
        
    msg->DeleteFormatted(rc);
}

/* Come up with the right message ID for message with name "name" and return
   it.   */
int IPServer::register_a_message(const char* name)
{
    IPMessageType* type = LookupMessage(name);
    int id;
    if (type) 
        id = type->ID();
    else {
        id = register_global_message(name);
        type = register_message(id, name, NULL);
    }

    return id;
}
    
/* Handler for a message registration message */
void IPServer::register_message_hand(IPMessage* msg)
{
    char** name = (char**) msg->FormattedData();
    int id = register_a_message(*name);

    Reply(msg, IPTMessageRegisteredMsgNum, &id);

    msg->DeleteFormatted(name);
}

/* Handler to register a set of messages */
void IPServer::register_message_set_hand(IPMessage* msg)
{
    IPTRegisterMessageSetStruct* rms =
        (IPTRegisterMessageSetStruct*) msg->FormattedData();
    int i;
    int* ids = new int[rms->n];
    for (i=0;i<rms->n;i++) 
        ids[i] = register_a_message(rms->names[i]);

    IPTMessageSetRegisteredStruct msr;
    msr.n = rms->n;
    msr.ids = ids;
    Reply(msg, IPTMessageSetRegisteredMsgNum, &msr);
    delete ids;
    msg->DeleteFormatted(rms);
}

/* Register an interdomain message with name "name" */
int IPServer::register_global_message(const char*)
{
    return _next_message_num++;
}
    
/* Handles a request for the domain name */
void IPServer::req_domain_name_hand(IPMessage* msg)
{
    char* dns;
    if (DomainName()) {
        dns = (char*) DomainName();
        Reply(msg, IPTDomainNameMsgNum, (void*) &dns);
    } else {
        dns = "";
        Reply(msg, IPTDomainNameMsgNum, (void*) &dns);
    }
}

void IPServer::logging_hand(IPMessage* msg)
{
    IPTLoggingStruct* msg_log = (IPTLoggingStruct*) msg->FormattedData();

    if (msg_log->operation == 0) {
        fprintf(_log_fp, "%s: Sent %s(ID %d, instance %d size %d) to %s\n",
               msg_log->origin, msg_log->msg_name, msg_log->msg_id,
               msg_log->instance, msg_log->size, msg_log->destination);
    } else {
        fprintf(_log_fp, "%s: Received %s(ID %d, instance %d size %d) from %s",
               msg_log->origin, msg_log->msg_name, msg_log->msg_id,
               msg_log->instance, msg_log->size, msg_log->destination);
        if (strlen(msg_log->extra))
            fprintf(_log_fp, " (%s)\n", msg_log->extra);
        else
            fprintf(_log_fp, "\n");
    }
    fflush(_log_fp);

    msg->DeleteFormatted((void*) msg_log);
}

class DeleteCallback : public IPConnectionCallback {
  public:
    DeleteCallback(const char* action);
    virtual ~DeleteCallback();

    virtual void Execute(IPConnection*);

  private:
    char* _action;
};

DeleteCallback::DeleteCallback(const char* action)
{
    int len = strlen(action)+1;
    _action = new char[len];
    bcopy(action, _action, len);
}

DeleteCallback::~DeleteCallback()
{
    delete _action;
}

void DeleteCallback::Execute(IPConnection*)
{
    system(_action);
}
    

void IPServer::deletion_hand(IPMessage* msg)
{
    IPTDeletionActionStruct* da =
        (IPTDeletionActionStruct*) msg->FormattedData();
    IPConnection* conn = LookupConnection(da->module);

    if (!conn) {
        printf("IPT(Server): Can't add deletion action to nonexistent %s\n",
               da->module);
        return;
    }

    IPConnectionCallback* cb =
        (IPConnectionCallback*) conn->DisconnectCallbacks()->First();
    for (;cb;cb = (IPConnectionCallback*) conn->DisconnectCallbacks()->Next()){
        conn->RemoveDisconnectCallback(cb);
        delete cb;
    }
    conn->AddDisconnectCallback(new DeleteCallback(da->action));
}

/* Connections to a domain named "domain_name".  If we are the domain master
   we must just passively wait for the connection to just happen, otherwise
   we petition the domain master for the machine of that domain server and
   connect to it */
IPConnection* IPServer::connect_to_domain(const char* domain_name)
{
    if (!DomainName()) {
        printf("IPT(Server): Need domain name to connect to other domain %s\n",
               domain_name);
        return NULL;
    }

    char* full_name = make_full_name(ModuleName(), domain_name);

    IPConnection* res = LookupConnection(full_name);
    if (res && res->Active()) {
        delete full_name;
        return res;
    }

    PeerDescriptor* desc = lookup_peer(domain_name);
    if (!desc) {
        printf("IPT(Server): Cannot connect to unknown domain %s\n",
               domain_name);
        delete full_name;
        return NULL;
    }

    int add_callback = (res == NULL);
    char* my_full_name = make_full_name(ModuleName(), DomainName());
    res = MakeConnection(full_name,
                         desc->host, SERVER_PORT, my_full_name, 1);
    delete full_name;
    delete my_full_name;

    if (!res || !res->Active()) {
        printf("IPT(Server): Cannot connect to peer %s on %s\n", domain_name,
               desc->host);
        return res;
    }

    IPMessage* msg = WaitForMsg(res, IPTVersionInfoMsgNum);

    IPTVersionInfoStruct* info = (IPTVersionInfoStruct*) msg->FormattedData();
    if (info->major_version != IPT_MAJOR_VERSION) {
        printf("IPT Error:  Major version %d does not match peer major version %d\n", IPT_MAJOR_VERSION, info->major_version);
        exit(-1);
    }
    
    if (info->minor_version != IPT_MINOR_VERSION)  {
        printf("IPT Warning: Minor version number %d does not match peer minor version %d\n", IPT_MINOR_VERSION, info->minor_version);
    }

    if (strcmp(info->domain_name, desc->name)) {
        printf("IPT(Server): Peer on %s is named %s not %s, renaming\n",
               desc->host, info->domain_name, desc->name);
        rename_peer(desc->name, info->domain_name);
        desc->conn = res;
        res = NULL;
    } else
        desc->conn = res;

    msg->DeleteFormatted(info);
    delete msg;

    const char* my_name = DomainName();
    SendMessage(res, IPT_PEER_CONNECTING_MSG, &my_name);

    if (res && add_callback) 
        res->AddDisconnectCallback(new ConnectionCallback(IPServer)
                                   (this, &IPServer::peer_disconnecting));
                          
    return res; 
}

/* Wait for a connection named "conn_name" */
IPConnection* IPServer::wait_for_connection(const char* conn_name)
{
    printf("IPT(Server): Waiting for module %s\n", conn_name);
    IPConnection* res = LookupConnection(conn_name);

    if (res)
        return res;

    printf("Waiting for domain server %s to connect\n", conn_name);

    while (!(res = LookupConnection(conn_name))) {
        Idle();
    }

    return res;
}

void IPServer::server_dump_hand(IPMessage* msg)
{
    void* raw_data = msg->FormattedData();
    char* filename = *(char**) raw_data;
    FILE* fp;

    if (*filename == '\0' || !strcmp(filename, "stdout")) {
        fp = stdout;
        filename = "stdout";
    } else {
        fp = fopen(filename, "w");
        if (!fp) {
            printf("IPT(Server): Error opening message dump file %s\n",
                   filename);
            filename = "stdout";
            fp = stdout;
        }
    }
    printf("(IPT(Server): Dumping messages to %s\n", filename);
    msg->DeleteFormatted(raw_data);

    int i;
    printf("\tDumping from %d to %d\n", LOWEST_MESSAGE_NUMBER,
           _next_message_num);
    IPMessageType* type;
    for (i=LOWEST_MESSAGE_NUMBER;i<_next_message_num;i++) {
        type = LookupMessage(i);
        if (type)
            fprintf(fp, "%s %d\n", type->Name(), type->ID());
    }

    if (fp != stdout)
        fclose(fp);
}

void IPServer::server_flush_hand(IPMessage*)
{
    _pending_register->clear_all();
    _pending_tables->clear_all();
    _pending_confirms->clear_all();
}
        
void IPServer::remote_confirm_hand(IPMessage* msg)
{
    IPTConfirmRemoteConnectionStruct* crc =
        (IPTConfirmRemoteConnectionStruct*) msg->FormattedData();
    int res;
    if (!lookup_routing(crc->target))
        res = -1;
    else {
        res = !_conn_net->connected(crc->initiator, crc->target);
        if (res)
            _conn_net->connect(crc->initiator, crc->target);
    }

    Reply(msg, LookupMessage(IPTRemoteConnectionConfirmedMsgNum), &res);
}
        
void IPServer::server_dump_net_hand(IPMessage* msg)
{
    void* raw_data = msg->FormattedData();
    char* filename = *(char**) raw_data;
    FILE* fp;

    if (*filename == '\0' || !strcmp(filename, "stdout")) {
        fp = stdout;
        filename = "stdout";
    } else {
        fp = fopen(filename, "w");
        if (!fp) {
            printf("IPT(Server): Error opening message dump file %s\n",
                   filename);
            filename = "stdout";
            fp = stdout;
        }
    }
    printf("Dumping connection network to %s\n", filename);
    msg->DeleteFormatted(raw_data);

    _conn_net->dump(fp);

    if (fp != stdout)
        fclose(fp);
}

void IPServer::server_flush_net_hand(IPMessage*)
{
    IPConnection* conn = (IPConnection*) Connections()->First();
    for (;conn;conn = (IPConnection*) Connections()->Next()) {
        if (_conn_net->connected(conn->Name(), NULL)) {
            _conn_net->disconnect(conn->Name());
            DeactivateConnection(conn);
        }
    }
}

void IPServer::clear_remote_connection(const char* domain_name,
                                       const char* mod_name)
{
    IPConnection* dom_server = connect_to_domain(domain_name);
    if (!dom_server || !dom_server->Active())
        return;
    
    char* full_name = make_full_name(mod_name, DomainName());

    SendMessage(dom_server,
                LookupMessage(IPTClearRemoteConnectionMsgNum), &full_name);
    delete full_name;
    return;
}

void IPServer::remote_clear_hand(IPMessage* msg)
{
    char** disconnecting = (char**) msg->FormattedData();

    _conn_net->disconnect(*disconnecting);
    remove_routing(*disconnecting);
    printf("IPT(Server): Remote %s disconnecting\n", *disconnecting);
}
        
    
PeerDescriptor* IPServer::add_peer(const char* name, const char* host)
{
    PeerDescriptor* desc = (PeerDescriptor*) _peer_table->Find(name);
    if (!desc) {
        desc = new PeerDescriptor(name, host);

        _peer_table->Insert(desc->name, strlen(desc->name)+1, (char*) desc);
    } else {
        delete desc->host;
        desc->host = copy_string(host);
    }

    return desc;
}

void IPServer::remove_peer(const char* name)
{
    PeerDescriptor* desc = lookup_peer(name);
    if (name) {
        delete desc->translations;
        desc->translations = new IPHashTable(300, int_hash, int_eq);
    }
}

PeerDescriptor* IPServer::lookup_peer(const char* name)
{
    return (PeerDescriptor*) _peer_table->Find(name);
}

void IPServer::rename_peer(const char* old_name, const char* new_name)
{
    PeerDescriptor* desc = (PeerDescriptor*) _peer_table->Remove(old_name);
    delete desc->name;
    desc->name = copy_string(new_name);
    _peer_table->Insert(new_name, strlen(new_name)+1, (char*) desc);
}

void IPServer::request_translation_hand(IPMessage* msg)
{
    IPTRequestTranslationStruct* rt =
        (IPTRequestTranslationStruct*) msg->FormattedData();
    PeerDescriptor* desc = lookup_peer(rt->domain_name);

    IPTTranslationStruct repl;
    repl.conn = rt->conn;
    repl.id = rt->id;
    if (!desc) {
        repl.translated_id = -1;
        printf("IPT(Server): No peer %s for translation request %d\n",
               rt->domain_name, rt->id);
        Reply(msg, LookupMessage(IPTTranslationMsgNum), (void*) &repl);
        return;
    }

    repl.translated_id = (long) desc->translations->Find((char*) &rt->id);
    if (repl.translated_id) {
        Reply(msg, LookupMessage(IPTTranslationMsgNum), (void*) &repl);
        return;
    }

    if (!desc->conn || !desc->conn->Active()) {
        desc->conn = connect_to_domain(desc->name);
        if (!desc->conn || !desc->conn->Active()) {
            printf("IPT(Server):Peer %s inactive for translation request %d\n",
                   rt->domain_name, rt->id);
            repl.translated_id = -1;
            Reply(msg, LookupMessage(IPTTranslationMsgNum), (void*) &repl);
            return;
        }
    }

    IPMessageType* type = LookupMessage(rt->id);
    if (!type) {
        printf("IPT(Server): No message for translation request %d\n", rt->id);
        repl.translated_id = -1;
        Reply(msg, LookupMessage(IPTTranslationMsgNum), (void*) &repl);
        return;
    }
    const char* msg_name = type->Name();
    _pending_register->query(desc->conn,
                             LookupMessage(IPTRegisterMessageMsgNum),
                             &msg_name,
                             new RegisterRequest(msg->Connection(),
                                                 msg->Instance(), desc->conn,
                                                 rt->conn, rt->id));
}
        
void IPServer::peer_connecting_hand(IPMessage* msg)
{
    char* domain_name = *(char**) msg->FormattedData();
    PeerDescriptor* desc = add_peer(domain_name, msg->Connection()->Host());
    printf("IPT(Server): Peer %s connecting from machine %s\n", domain_name,
           msg->Connection()->Host());
    if (!desc->conn) {
        desc->conn = msg->Connection();
        desc->conn->AddDisconnectCallback(new ConnectionCallback(IPServer)
                                          (this,
                                           &IPServer::peer_disconnecting));
    }
}
        
void IPServer::peer_disconnecting(IPConnection* conn)
{
    const char* colon_ptr = strchr(conn->Name(), ':');
    if (!colon_ptr)
        return;
    int len = colon_ptr - conn->Name();
    char* domain_name = new char[len+1];
    bcopy(conn->Name(), domain_name, len);
    domain_name[len] = '\0';

    printf("IPT(Server): Peer %s on machine %s disconnecting\n", domain_name,
           conn->Host());

    remove_peer(domain_name);

    delete domain_name;
}

IPRoutingTable* IPServer::lookup_routing(const char* name)
{
    return (IPRoutingTable*) _routing_tables->Find(name);
}

void IPServer::remove_routing(const char* name)
{
    IPRoutingTable* table = (IPRoutingTable*) _routing_tables->Remove(name);
    if (!table)
        delete table;
}

void IPServer::add_routing(IPRoutingTable* table)
{
    _routing_tables->Insert(table->connection()->Name(),
                            strlen(table->connection()->Name())+1,
                            (char*) table);
}

IPRoutingTable* IPServer::add_routing(const char* name,
                                      IPTRequestedRoutingTableStruct* rrt)
{
    IPConnection* conn = LookupConnection(name);
    if (!conn) 
        conn = new FakeConnection(name, this, -1);
    
    IPRoutingTable* table = new IPRoutingTable(conn, rrt->num_routes, 1);
    
    int i;
    for (i=0;i<rrt->num_routes;i++) 
        table->table()[i].set_routing(rrt->routes[i].type,
                                      rrt->routes[i].sub_type,
                                      rrt->routes[i].comparison,
                                      rrt->routes[i].parameters);
    add_routing(table);

    return table;
}

IPConnection* IPServer::lookup_connection(const char* name)
{
    IPRoutingTable* table = lookup_routing(name);
    if (table)
        return table->connection();
    else
        return NULL;
}

void IPServer::routing_table_hand(IPMessage* msg) 
{
    IPTRoutingTableStruct* rt = (IPTRoutingTableStruct*) msg->FormattedData();
    IPConnection* conn = msg->Connection();
    IPRoutingTable* table = lookup_routing(conn->Name());
    if (table) {
        printf("IPT(Server): replacing route tables for %s\n", conn->Name());
        remove_routing(conn->Name());
    }

    table = new IPRoutingTable(conn, rt->num_routes);
    int i;
    for (i=0;i<rt->num_routes;i++) 
        table->table()[i].set_routing(rt->routes[i].type,
                                      rt->routes[i].sub_type,
                                      rt->routes[i].comparison,
                                      rt->routes[i].parameters);
    add_routing(table);
    
    _request_list->RemoveAll(check_request, (char*) conn);
}

void IPServer::routing_table_req_hand(IPMessage* msg)
{
    IPTRoutingTableRequestStruct* rtr =
        (IPTRoutingTableRequestStruct*) msg->FormattedData();
    IPTRequestedRoutingTableStruct rrt;

    IPRoutingTable* table = lookup_routing(rtr->name);
    if (!table) {
        if (rtr->required) {
            ConnectionRequest* cr =
                new RemoteConnectionRequest(msg->Connection(), msg->Instance(),
                                            rtr->name);
            _request_list->Append((char*) cr);
            return;
        } else
            bzero((char*) &rrt, sizeof(IPTRequestedRoutingTableStruct));
    } else {
        rrt.num_routes = table->num_routes();
        rrt.routes = new IPTRoutingStruct[rrt.num_routes];
        int i;
        for (i=0;i<rrt.num_routes;i++) {
            rrt.routes[i].type = (char*) table->route(i)->type();
            rrt.routes[i].sub_type = (char*) table->route(i)->sub_type();
            rrt.routes[i].comparison = table->route(i)->comparison();
            rrt.routes[i].parameters = (char*) table->route(i)->parameters();
        }
    }

    Reply(msg, LookupMessage(IPTRequestedRoutingTableMsgNum), &rrt);

    if (rrt.num_routes)
        delete rrt.routes;
}

void IPServer::remote_report_hand(IPMessage* msg)
{
    IPTReportRemoteConnectionStruct* rrc =
        (IPTReportRemoteConnectionStruct*) msg->FormattedData();
    if (lookup_routing(rrc->target) &&
        !_conn_net->connected(rrc->initiator, rrc->target))
        _conn_net->connect(rrc->initiator, rrc->target);
}

void IPServer::report_connections_hand(IPMessage* msg)
{
    IPTReportConnectionsStruct* rc =
        (IPTReportConnectionsStruct*) msg->FormattedData();

    int i;
    IPConnection* conn = msg->Connection();
    IPConnection* other;
    const char* colon;
    const char* name;
    for (i=0;i<rc->num_conns;i++) {
        name = rc->conn_names[i];
        colon = strchr(name, ':');
        if (colon) {
            IPTConfirmRemoteConnectionStruct crc;
            int len = colon - (char*) name;
            char* domain_name = new char[len+1];
            bcopy(name, domain_name, len);
            domain_name[len] = '\0';
            IPConnection* dom = connect_to_domain(domain_name);

            if (!dom)
                continue;

            crc.initiator = make_full_name(msg->Connection()->Name(),
                                           DomainName());
            crc.target = (char*) colon+1;
            _pending_confirms->query(dom, 
                             LookupMessage(IPTConfirmRemoteConnectionMsgNum),
                             &crc,
                             new ReconnectRequest(_conn_net, dom,
                                                  msg->Connection()->Name(),
                                                  rc->conn_names[i]));
            delete crc.initiator;
            delete domain_name;
        } else {
            other = LookupConnection(name);
            if (other && other->Active() &&
                !_conn_net->connected(conn->Name(), name))  {
                printf("IPT(Server): Reconnecting %s to %s\n", conn->Name(),
                       name);
                _conn_net->connect(conn->Name(), name);
            }
        }
    }
}

void IPServer::reregister_messages_hand(IPMessage* msg)
{
    IPTReregisterMessagesStruct* rm =
        (IPTReregisterMessagesStruct*) msg->FormattedData();
    int i;
    IPMessageType* type;
    const char* name;
    int id;
    for (i=0;i<rm->num_messages;i++) {
        name = rm->messages[i].msg_name;
        id = rm->messages[i].msg_id;
        type = LookupMessage(name);
        if (!type) 
            register_message(id, name, NULL);
        else if (type->ID() != id) {
            printf("IPT(Server): Message named %s was registered as %d, now registered as %d by %s\n",
                   name, type->ID(), id, msg->Connection()->Name());
            printf("IPT(Server): Fatal inconsistency, restart system from scratch\n");
            exit(-1);
        }
    }
}
        
void IPServer::fail_connection_request(const char* name, IPMessage* msg)
{
    IPTConnectionRoutingStruct cr;

    cr.name = (char*) name;
    cr.type = NULL;
    cr.sub_type = NULL;
    cr.parameters = NULL;

    printf("IPT(Server): Routing failed between %s and %s\n",
           msg->Connection()->Name(), name);

    Reply(msg, LookupMessage(IPTConnectionRoutingMsgNum), &cr);
}

IPRouting* IPServer::match_routings(IPRoutingTable* initiator,
                                    IPRoutingTable* other)
{
    int res = strcmp(initiator->connection()->Name(),
                     other->connection()->Name());
    if (!res)
        return NULL;
    int swapped = 0;
    if (res < 0) {
        IPRoutingTable* temp = other;
        other = initiator;
        initiator = temp;
        swapped = 1;
    }

    int min_error = 100000;
    int best_i = -1;
    int best_j = -1;
    int i,j;

    for (i=0;i<initiator->num_routes(); i++) 
        for (j=0;j<other->num_routes(); j++) 
            if (initiator->route(i)->matches(other->route(j))) {
                int error = i+j;
                if (error < min_error) {
                    min_error = error;
                    best_i = i;
                    best_j = j;
                }
            }

    if (best_i == -1 || best_j == -1)
        return NULL;

    if (swapped)
        return initiator->route(best_i);
    else
        return other->route(best_j);
}

int IPServer::routing_reply(IPMessage* msg, const char* name,
                            IPRouting* route, int force_no_domain)
{
    char* colon = (char*) strchr(name, ':');
    int res = 1;
    if (!force_no_domain && colon) {
        int len = colon - (char*) name;
        char* domain_name = new char[len+1];
        bcopy(name, domain_name, len);
        domain_name[len] = '\0';
        IPConnection* dom = connect_to_domain(domain_name);
        if (dom) {
            char* full_name = make_full_name(msg->Connection()->Name(),
                                             DomainName());

            if (strcmp(DomainName(), domain_name) < 0) {
                IPTConfirmRemoteConnectionStruct crc;
                crc.initiator = full_name;
                crc.target = colon+1;

                _pending_confirms->query(dom,
                             LookupMessage(IPTConfirmRemoteConnectionMsgNum),
                                         &crc,
                                         new ConfirmRequest(this, name,
                                                            route,
                                                            msg->Connection(),
                                                            msg->Instance(),
                                                            dom));
                                                     
                return -1;
            } else {
                IPTReportRemoteConnectionStruct rrc;
                rrc.initiator = full_name;
                rrc.target = colon+1;

                SendMessage(dom,
                            LookupMessage(IPTReportRemoteConnectionMsgNum),
                            &rrc);
            }
            delete full_name;
        }

        delete domain_name;
    }

    if (res) {
        IPTConnectionRoutingStruct cr;

        cr.name = (char*) name;
        cr.type = (char*) route->type();
        cr.sub_type = (char*) route->sub_type();
        cr.parameters = (char*) route->parameters();

        Reply(msg, LookupMessage(IPTConnectionRoutingMsgNum), &cr);
        printf("IPT(Server): route established between %s and %s (%s,%s,%s)\n",
               msg->Connection()->Name(), name,
               cr.type, cr.sub_type, cr.parameters);

        _conn_net->connect(msg->Connection()->Name(), name);
    }

    return res;
}
    
/* Arrange a connection between an initiator and a receiver in which the
   initiator asked for it with a message with instance "instance" */
void IPServer::arrange_connection(IPConnection* initiator,
                                  IPConnection* receptor, int instance)
{
    IPRoutingTable* req_table = lookup_routing(initiator->Name());
    IPRoutingTable* table = lookup_routing(receptor->Name());
    IPTConnectionRoutingStruct cr;
    bzero((char*) &cr, sizeof(IPTConnectionRoutingStruct));

    IPRouting* route;
    if (table && req_table && (route = match_routings(req_table, table))) {
        cr.name = (char*) receptor->Name();
        cr.type = (char*) route->type();
        cr.sub_type = (char*) route->sub_type();
        cr.parameters = (char*) route->parameters();
        printf("IPT(Server): Route established between %s and %s (%s,%s,%s)\n",
               initiator->Name(), receptor->Name(),
               cr.type, cr.sub_type, cr.parameters);
        _conn_net->connect(initiator->Name(), receptor->Name());
    }

    IPMessage out(LookupMessage(IPTConnectionRoutingMsgNum),
                  instance, &cr);
    SendMessage(initiator, &out);
}

/* Arrange a remote connection on behalf of a module named "name" on machine
   "host" whose server, "server" has asked for to module connected by 
   "receptor".  If "direct" is 1, then respond with IPTConnectionInfoMsgNum
   because we are resonding directly.  If "direct" is 0 then this is a delayed
   response and respond with IPTDelayedConnectionInfoMsgNum */
void IPServer::arrange_remote_connection(IPConnection* server,
                                         IPConnection* receptor, int instance)
{
    IPTRequestedRoutingTableStruct rrt;
    IPRoutingTable* table = lookup_routing(receptor->Name());
    if (!table) {
        rrt.num_routes = 0;
    } else {
        rrt.num_routes = table->num_routes();
        rrt.routes = new IPTRoutingStruct[rrt.num_routes];
        int i;
        for (i=0;i<rrt.num_routes;i++) {
            rrt.routes[i].type = (char*) table->route(i)->type();
            rrt.routes[i].sub_type = (char*) table->route(i)->sub_type();
            rrt.routes[i].comparison = table->route(i)->comparison();
            rrt.routes[i].parameters = (char*) table->route(i)->parameters();
        }
    }

    IPMessage out(LookupMessage(IPTRequestedRoutingTableMsgNum),
                  instance, &rrt);
    SendMessage(server, &out);

    if (rrt.num_routes)
        delete rrt.routes;
}

void TableRequest::service(IPMessage* msg)
{
    IPTRequestedRoutingTableStruct* rrt =
        (IPTRequestedRoutingTableStruct*) msg->FormattedData();
    IPMessage out(_server->LookupMessage(IPTRequestConnectionMsgNum),
                  instance(), 0, NULL);
    out.Connection(initiator());

    if (_conn_net->connected(initiator()->Name(), _full_name)) {
        printf("IPT(Server): Already connected\n");
        _server->fail_connection_request(_full_name, &out);
    } else if (rrt->num_routes == 0)  {
        printf("IPT(Server): No routes\n");
        _server->fail_connection_request(_full_name, &out);
    } else {
        IPRoutingTable* table = _server->add_routing(_full_name, rrt);
        _server->routing_reply(&out, _full_name,
                               _server->match_routings(_req_table, table));
    }
}

void TableRequest::clear(IPConnection* conn)
{
    if  (conn != initiator()) {
        IPMessage out(_server->LookupMessage(IPTRequestConnectionMsgNum),
                      instance(), 0, NULL);
        out.Connection(initiator());
        printf("IPT(server): Cleared\n");
        _server->fail_connection_request(_full_name, &out);
    }
}

void ReconnectRequest::service(IPMessage* msg)
{
    int answer = *(int*) msg->FormattedData();
    if (answer != -1 && !_net->connected(_name1, _name2))  {
        printf("IPT(Server): Reconnecting %s and %s\n", _name1, _name2);
        _net->connect(_name1, _name2);
    }
}

void ConfirmRequest::service(IPMessage* msg)
{
    int answer = *(int*) msg->FormattedData();
    IPMessage out(_server->LookupMessage(IPTRequestConnectionMsgNum),
                  instance(), 0, NULL);
    out.Connection(initiator());
    if (answer < 1) {
        printf("IPT(server): Ignore the following error\n");
        _server->fail_connection_request(_full_name, &out);
    } else {
        _server->routing_reply(&out, _full_name, _route, 1);
    }
}

void ConfirmRequest::clear(IPConnection* conn)
{
    if  (conn != initiator()) {
        IPMessage out(_server->LookupMessage(IPTRequestConnectionMsgNum),
                      instance(), 0, NULL);
        out.Connection(initiator());
        printf("IPT(Server): Module disconnect\n ");
        _server->fail_connection_request(_full_name, &out);
    }
}
