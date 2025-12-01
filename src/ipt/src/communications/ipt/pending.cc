///////////////////////////////////////////////////////////////////////////////
//
//                                 pending.cc
//
// This file implements the queued message classes used by the IPT server to
// avoid getting stuck inside message queries.
// This file also implements the class for maintaining the connection graph
//
// Classes defined for export:
//    PendingRequest, PendingCallback, IndirectRequest
//    ConnectionNet
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

#include <ipt/ipt.h>
#include <ipt/message.h>
#include <ipt/connection.h>
#include <ipt/list.h>
#include <ipt/internal_messages.h>
#include <ipt/pending.h>
#include <ipt/hash.h>
#include <ipt/conn_net.h>

PendingRequest::PendingRequest(IPConnection* tar, IPMessage* msg)
{
    _target = tar;
    if (msg)
        _message = new IPMessage(*msg);
    else
        _message = 0;
}

PendingRequest::~PendingRequest()
{
    if (_message)
        delete _message;
}

PendingCallback::PendingCallback()
{
    _pending = IPList::Create();
}

PendingCallback::~PendingCallback()
{
    clear_all();

    IPList::Delete(_pending);
}

static int check_pending(const char* param, const char* item)

{
    PendingRequest* req = (PendingRequest*) item;
    IPMessage* msg = (IPMessage*) param;

    if (msg->Connection() == req->target() &&
        msg->Instance() == req->message()->Instance()) {
        req->service(msg);
        delete req;
        return 1;
    }

    return 0;
}

void PendingCallback::Execute(IPMessage* msg)
{
    _pending->Remove(check_pending, (char*) msg);
}

void PendingCallback::pend(PendingRequest* req)
{
    _pending->Append((char*) req);
}

static int check_doomed(const char* param, const char* item)
{
    PendingRequest* req = (PendingRequest*) item;
    IPConnection* conn = (IPConnection*) param;

    if (req->related(conn)) {
        req->clear(conn);
        delete req;
        return 1;
    }

    return 0;
}

void PendingCallback::clear(IPConnection* doomed)
{
    _pending->RemoveAll(check_doomed, (char*) doomed);
}
    
void PendingCallback::clear_all()
{
    PendingRequest* cur = (PendingRequest*) _pending->First();
    PendingRequest* doomed = NULL;
    for (; cur; cur = (PendingRequest*) _pending->Next()) {
        if (doomed)
            delete doomed;
        cur->clear(NULL);
        doomed = cur;
    }
    if (doomed)
        delete doomed;
}

void PendingCallback::query(IPConnection* conn, IPMessageType* type,
                            void* data, PendingRequest* pr)
{
    IPCommunicator* comm = conn->Communicator();
    IPMessage out(type, comm->generate_instance_num(), data);

    comm->SendMessage(conn, &out);
    pr->set_message(&out);
    pend(pr);
}

void IndirectRequest::service(IPMessage* msg)
{
    IPCommunicator* comm = initiator()->Communicator();
    
    IPMessage out_msg(msg->Type(),
                      instance(), msg->SizeData(), msg->Data());
    comm->SendMessage(initiator(), &out_msg);
}

class ConnectionList : public IPHashTable
{
  public:
    ConnectionList(int size, int (*hash_func)(const char*),
                   int (*eq_func)(const char*, const char*));
    virtual ~ConnectionList();

    int interdomain_involved() const;
    void involve_domain(const char*);
    IPList* domains() const { return _domains; }
    void uninvolve();

  private:
    IPList* _domains;
};

ConnectionList::ConnectionList(int size, int (*hash_func)(const char*),
                               int (*eq_func)(const char*, const char*))
    : IPHashTable(size, hash_func, eq_func)
{
    _domains = 0;
}

static int delete_string(char*, char* data)
{
    delete data;

    return 1;
}

ConnectionList::~ConnectionList()
{
    uninvolve();
}

int ConnectionList::interdomain_involved() const
{
    return _domains != NULL;
}

void ConnectionList::involve_domain(const char* domain_name)
{
    int len = strlen(domain_name)+1;
    char* name = new char[len];
    bcopy(domain_name, name, len);

    if (!_domains)
        _domains = IPList::Create();

    char* prev_domain = _domains->First();
    for(;prev_domain;prev_domain = _domains->Next()) 
        if (!strcmp(prev_domain, domain_name))
            return;

    _domains->Append(name);
}

void ConnectionList::uninvolve()
{
    if (_domains) {
        _domains->Iterate(delete_string, NULL);
        IPList::Delete(_domains);
    }
}

ConnectionNet::ConnectionNet()
{
    _connections = new IPHashTable(32, str_hash, str_eq);
}

static int delete_a_connection(char*, char* value)
{
    delete value;

    return 1;
}

static int delete_connections(char*, char* value)
{
    ConnectionList* conn_table = (ConnectionList*) value;

    conn_table->Iterate(delete_a_connection);
    delete conn_table;

    return 1;
}

ConnectionNet::~ConnectionNet()
{
    _connections->Iterate(delete_connections);
    delete _connections;
}

void ConnectionNet::connect(const char* initiator, const char* receptor)
{
    if (!update_net(initiator, receptor))
        update_net(receptor, initiator);
}

int ConnectionNet::update_net(const char* initiator, const char* receptor)
{
    ConnectionList* conn_table =
        (ConnectionList*) _connections->Find(initiator);
    if (!conn_table) {
        conn_table = new ConnectionList(32, str_hash, str_eq);
        _connections->Insert(initiator, strlen(initiator)+1,
                             (char*) conn_table);
    }

    if (conn_table->Find(receptor))
        return 1;

    int len = strlen(initiator)+1;
    char* entry = new char[len];
    bcopy(initiator, entry, len);
    conn_table->Insert(receptor, strlen(receptor)+1, entry);

    const char* colon = strchr(receptor, ':');
    if (colon) {
        int len = colon-receptor;
        char* domain = new char[len+1];
        bcopy(receptor, domain, len);
        domain[len] = '\0';
        conn_table->involve_domain(domain);
        delete domain;
    }

    return 0;
}

static ConnectionNet* _conn_net = NULL;
int disconnect_conn(char* receptor, char* initiator)
{
    _conn_net->unlink(receptor, initiator);

    return 1;
}

void ConnectionNet::disconnect(const char* doomed)
{
    ConnectionList* conn_table =
        (ConnectionList*) _connections->Remove(doomed);
    if (!conn_table)
        return;

    conn_table->uninvolve();
    _conn_net = this;
    conn_table->Iterate(disconnect_conn);
}

void ConnectionNet::unlink(const char* receptor, const char* initiator)
{
    ConnectionList* conn_table =
        (ConnectionList*) _connections->Find(receptor);
    if (!conn_table)
        return;

    conn_table->Remove(initiator);
}

int ConnectionNet::connected(const char* receptor, const char* initiator)
{
    ConnectionList* conn_table = (ConnectionList*) _connections->Find(receptor);
    if (!conn_table)
        return 0;

    return (!initiator) || conn_table->Find(initiator) != NULL;
}

static FILE* _dump_fp = NULL;
static int print_minor(char* key, char*)
{
    fprintf(_dump_fp, "\t%s\n", key);

    return 1;
}

static int print_major(char* key, char* value)
{
    fprintf(_dump_fp, "%s:\n", key);
    ((ConnectionList*) value)->Iterate(print_minor);

    return 1;
}

void ConnectionNet::dump(FILE* fp)
{
    _dump_fp = fp;
    _connections->Iterate(print_major);
}

void ConnectionNet::iterate(const char* mod_name,
                            int (*callback)(char*, char*, void*),
                            void* data)
{
    ConnectionList* conn_table =
        (ConnectionList*) _connections->Find(mod_name);
    if (!conn_table)
        return;

    conn_table->Iterate(callback, data);
}

static int unhook_domain(char* key, char*, void* data)
{
    char* domain_name = (char*) data;

    if (!bcmp(key, domain_name, strlen(domain_name)))
        return 0;

    return 1;
}
    
void ConnectionNet::disconnect_domain(const char* domain_name)
{
    char* name;
    while ((name = _connections->Iterate(unhook_domain, (void*) domain_name)))
        disconnect(name);
}

int ConnectionNet::interdomain(const char* name)
{
    ConnectionList* conn_table = (ConnectionList*) _connections->Find(name);
    if (!conn_table)
        return 0;
    return conn_table->interdomain_involved();
}

IPList* ConnectionNet::domains(const char* name)
{
    ConnectionList* conn_table = (ConnectionList*) _connections->Find(name);
    if (!conn_table)
        return 0;
    return conn_table->domains();
}

RegisterRequest::RegisterRequest(IPConnection* initiator, int instance,
                                 IPConnection* receptor, const char* conn_name,
                                 int id, IPMessage* msg)
    : IndirectRequest(initiator, instance, receptor, msg)
{
    int len = strlen(conn_name)+1;
    _conn_name = new char[len];
    bcopy(conn_name, _conn_name, len);
    _id = id;
}

RegisterRequest::~RegisterRequest()
{
    delete _conn_name;
}
 
void RegisterRequest::service(IPMessage* msg)
{
    IPCommunicator* comm = initiator()->Communicator();

    IPTTranslationStruct trans;
    trans.id = _id;
    trans.conn = _conn_name;
    trans.translated_id = *(int*) msg->FormattedData();
    
    comm->SendMessage(initiator(), comm->LookupMessage(IPTTranslationMsgNum),
                      (void*) &trans);
}

void RegisterRequest::clear(IPConnection* conn)
{
    if (conn != initiator()) {
        IPTTranslationStruct trans;
        trans.id = _id;
        trans.conn = _conn_name;
        trans.translated_id = -1;

        IPCommunicator* comm = initiator()->Communicator();
        comm->SendMessage(initiator(),
                          comm->LookupMessage(IPTTranslationMsgNum),
                          (void*) &trans);
    }
}

