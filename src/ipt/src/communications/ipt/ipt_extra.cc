///////////////////////////////////////////////////////////////////////////////
//
//                                 ipt.cc
//
// This file implements the some IPT frills for the IPCommunicator class.  
// frills manage the bookkeeping for a Publisher/Subscriber approach and a
// client server approach.  The publisher/subscriber approach means that
// a client module can subscribe on the publishing module to a published
// message type.  Whenever the publishing module decides to it can "publish"
// that message, and the message will go out to all subscribers.   The client
// server approach is more general, in that modules register with a server
// and then the server can "Broadcast" to them all.  The main advantage of
// using these frills is that the connection and disconnection is handled 
// cleanly
//
// Members of IPCommunicator defined for export
//    init_extras, delete_extras
//    DeclareSubscription, Subscribe, Publish
//    Server, AddClient, Client, Broadcast
//
// Classes defined for external use
//    IPPublication, IPClient
//
// Classes defined for internal use
//    IPTimedPublication, IPTimedClient
//    PublicationConnCallback, ServerDisconnectCallback, 
//    ServerCallback, ServerConnectCallback, IPSubscription,
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <ipt/ipt.h>
#include <ipt/callbacks.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/connection.h>
#include <ipt/hash.h>
#include <ipt/list.h>
#include <ipt/internal_messages.h>
#include <ipt/publish.h>
#include <ipt/time.h>
#include <ipt/parseformat.h>
#include <ipt/format.h>

#ifdef VXWORKS
#include <ipt/vxcommunicator.h>
#else
#include <ipt/unixcommunicator.h>
#include "./libc.h"
#endif

/* structure for subscribers to keep track of their subscriptions */
struct IPSubscription {
    IPConnection* publisher;
    IPMessageType* type;
    IPConnectionCallback* connect;
    IPConnectionCallback* disconnect;
};

struct IPServerRecord {
    IPServerRecord() {}
    ~IPServerRecord() { delete registration_message; }
    
    IPConnection* server;
    IPMessage* registration_message;
    int already_registered;
    IPConnectionCallback* connect;
    IPConnectionCallback* disconnect;
};

/* A publication service that causes IPTimedClients to be added to the
   publish list instead of IPClients, allowing the sending of information
   at timed intervals */
struct IPTimedPublication : public IPPublication {
  public:
    IPTimedPublication(IPConnectionCallback* cb) : IPPublication(cb) {}

    virtual int AddSubscriber(IPConnection* conn,
                              int size_info, unsigned char* info);
};

/* This subclass of IPClient only lets information be sent to a subscriber if
   it has not been sent in more than a minimum amount of time */
struct IPTimedClient : public IPClient {
  public:
    IPTimedClient(double interval, IPConnection* conn);

    virtual int Check();
    virtual int SendMessage(IPCommunicator* comm, IPMessage* msg);

  private:
    double _interval;
    IPTime _last_published;
};

/* class used by a subscriber to initiate a resubscription when a publisher
   reconnects */
class PublicationConnCallback : public IPConnectionCallback {
  public:
    PublicationConnCallback(IPMessageType* type,
                            int size = 0, unsigned char* info = 0);
    virtual ~PublicationConnCallback();

    virtual void Execute(IPConnection* conn) {
        printf("IPT: Resubscribing to %s published by %s\n",
               _type->Name(), conn->Name());
        while (1) {   // loop, because publisher might take a while to init.
            conn->Communicator()->Sleep(0.3);
            if (conn->Communicator()->Subscribe(conn, _type,
                                                _size_info, _info))
                break;
            printf("\tTrying again\n");
        }
    }

  private:
    IPMessageType* _type;   // message type that we are subscribed to
    int _size_info;
    unsigned char* _info;
};

/* Disconnection callback class used for either a Publisher or a Server 
   disconnecting from a subscriber/client.  It just initiates a non-blocking
   reconnection request to the IPServer which will cause this module to 
   connect to its publisher/server when the p/s restarts */
class ServerDisconnectCallback : public IPConnectionCallback {
  public:
    ServerDisconnectCallback(IPServerRecord* record = NULL)
        { _record = record; }
    virtual void Execute(IPConnection* conn) {
        if (_record)
            _record->already_registered = 0;
        conn->Communicator()->Connect(conn->Name(), IPT_NONBLOCKING);
    }

  private:
    IPServerRecord* _record;
};

/* Used by the server in response to a client registration message.  After
   the client is added to the internal list of clients, the optional user
   callback can be invoked */
class ServerCallback : public IPHandlerCallback {
  public:
    ServerCallback(IPHandlerCallback* cb) { _callback = cb; }

    virtual void Execute(IPMessage* m) {
        m->Connection()->Communicator()->AddClient(m->Connection());

        if (_callback)
            _callback->Execute(m);
    }

  private:
    IPHandlerCallback* _callback;   // optional user callback for registration
                                    // message
};

/* Used by a client to re-register with a server when the connection to 
   the server has been reestablished */
class ServerConnectCallback : public IPConnectionCallback {
  public:
    ServerConnectCallback(IPServerRecord* record) { _record = record; }
    virtual void Execute(IPConnection* server) {
        server->Communicator()->Client(server, _record->registration_message);
    }

  private:
    IPServerRecord* _record;
};

/* Create a publication with a subscription callback (optional) of cb */
IPPublication::IPPublication(IPConnectionCallback* cb)
{
    _cb = cb;
    _modules = IPList::Create();
}

/* Delete a publication */
IPPublication::~IPPublication()
{
    if (_cb)
        delete _cb;
    IPList::Delete(_modules);
}

/* Used as an interation function for Return.  Checks to see if the
   connection (passed in as "item) has the same name as "param" */
static int check_client_name(const char* param, const char* item)
{
    return (strcmp(((IPClient*) item)->Connection()->Name(), param) == 0);
}

/* add a subscriber "client" to the publication list for this message type */
int IPPublication::add_subscriber(IPClient* client)
{
    if (!client->Connection())
        return 0;

    if (!_modules->Return(check_client_name, client->Connection()->Name())) {
        _modules->Prepend((char*) client);
        return 1;
    } else
        return 0;
}    

/* Add a subscriber to a publication list, if that subscriber is not
   already on the list */
int IPPublication::AddSubscriber(IPConnection* conn, int, unsigned char*)
{
    return add_subscriber(new IPClient(conn));
}

int IPPublication::RemoveSubscriber(IPConnection* conn)
{
    return (_modules->Remove(check_client_name, conn->Name()) != NULL);
}

/* Publish a message "msg" using the communicator "comm" to all active modules
   in the publication list */
void IPPublication::Publish(IPCommunicator* comm, IPMessage* msg)
{
    for (IPClient* client = (IPClient*) _modules->First();client;
         client = (IPClient*) _modules->Next())
        if (client->Connection() && client->Connection()->Active() &&
            client->Check() && Check(client->Connection(), msg))
            client->SendMessage(comm, msg);
}

/* declare a subscription service for message type "type." The service will
   be managed by the publication instance "pub" */
void IPCommunicator::DeclareSubscription(IPMessageType* type, 
                                         IPPublication* pub)
{
    /* add publication to publication table */
    _publication_table->Insert((char*) &type, sizeof(IPMessageType*),
                               (char*) pub);
}

/* Declare that modules can subscribe to mesage type "type" on this module.
   If "cb" is non-NULL, it is a callback that will be invoked when modules
   subscribe to "type" */
void IPCommunicator::DeclareSubscription(IPMessageType* type, 
                                         IPConnectionCallback* cb)
{
    DeclareSubscription(type, new IPPublication(cb));
}

/* Convenience function for declaring a subscription with a message name 
   rather than a message type */
void IPCommunicator::DeclareSubscription(const char* msg_name,
                                         IPConnectionCallback* callback)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't publish unknown message %s\n", msg_name);
        return;
    }

    DeclareSubscription(type, callback);
}

/* DeclareTimed that modules can subscribe to mesage type "type" on this module.
   If "cb" is non-NULL, it is a callback that will be invoked when modules
   subscribe to "type" */
void IPCommunicator::DeclareTimedSubscription(IPMessageType* type, 
                                              IPConnectionCallback* cb)
{
    DeclareSubscription(type, new IPTimedPublication(cb));
}

/* Convenience function for declaring a subscription with a message name 
   rather than a message type */
void IPCommunicator::DeclareTimedSubscription(const char* msg_name,
                                              IPConnectionCallback* callback)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't publish unknown message %s\n", msg_name);
        return;
    }

    DeclareTimedSubscription(type, callback);
}

/* Publish "msg" to all subscribers of message type "type".  Note that
   "msg" does not have to be of type "type," but this is a possibly useful
   feature, not a bug */
void IPCommunicator::Publish(IPMessageType* type, IPMessage* msg)
{
    /* lookup type in the publication hash table */
    IPPublication* pub =
        (IPPublication*) _publication_table->Find((char*) &type);

    // if it is there, send the message to all modules in the publication list
    if (pub) 
        pub->Publish(this, msg);
    else
        printf("IPT:  %s not declared a publisher, doing nothing\n",
               type->Name());
}

/* Convenience function to create a message of type "type" with formatted data
   "data" and publish it */
void IPCommunicator::Publish(IPMessageType* type, void* data)
{
    IPMessage out(type, generate_instance_num(), data);
    Publish(type, &out);
}

/* Convenience function to create a message of type "type" with "size" bytes
   of unformatted data, "data" and publish it */
void IPCommunicator::Publish(IPMessageType* type, int size,
                             unsigned char* data)
{
    IPMessage out(type, generate_instance_num(), size, data);
    Publish(type, &out);
}

/* Convenience function to create a message with name "msg_name" with
   formatted data "data" and publish it */
void IPCommunicator::Publish(const char* msg_name, void* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't publish unknown message %s\n", msg_name);
        return;
    }

    Publish(type, data);
}

/* Convenience function to create a message of name "msg_name" with "size"
   bytes of unformatted data, "data" and publish it */
void IPCommunicator::Publish(const char* msg_name, int size,
                             unsigned char* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't publish unknown message %s\n", msg_name);
        return;
    }

    Publish(type, size, data);
}

/* Return the list of possible subscribers to the type "type" */
IPList* IPCommunicator::Subscribers(IPMessageType* type)
{
    /* lookup type in the publication hash table */
    IPPublication* pub =
        (IPPublication*) _publication_table->Find((char*) &type);

    if (!pub) {
        printf("IPT: Not a publisher for message %s\n", type->Name());
        return NULL;
    }

    return pub->Modules();
}

/* Return the possible subscribers to the message named "msg_name" */
IPList* IPCommunicator::Subscribers(const char* msg_name)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't look up subscribers for unknown message %s\n",
               msg_name);
        return NULL;
    }

    return Subscribers(type);
}

/* Return the number of subscribers to type "type" */
int IPCommunicator::NumSubscribers(IPMessageType* type)
{
    IPList* subscribers = Subscribers(type);
    if (!subscribers) 
        return 0;

    int num_active = 0;
    for (IPClient* s = (IPClient*) subscribers->First(); s;
         s = (IPClient*) subscribers->Next())
        if (s->Check() && s->Connection()->Active())
            num_active++;

    return num_active;
}

/* return the number of subscribers to the message named "msg_name" */
int IPCommunicator::NumSubscribers(const char* msg_name)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't look up subscribers for unknown message %s\n",
               msg_name);
        return 0;
    }

    return NumSubscribers(type);
}

/* Subscribe to a message of type "type" on the module connected by connectio
   "publisher."  If "resubscribe" is 1, this is a renewal, so don't add the
   redundant connect/disconnect callbacks */
int IPCommunicator::Subscribe(IPConnection* publisher, IPMessageType* type,
                              int size_info, unsigned char* info)
{
    IPTSubscriptionStruct sub;

    sub.msg_name = (char*) type->Name();
    sub.size_info = size_info;
    sub.info = info;
    
    while (1) {
        /* send the subscription message containing the message ID to pub. */
        IPMessage* msg = Query(publisher, LookupMessage(IPTSubscriptionMsgNum),
                               (void*) &sub,
                               LookupMessage(IPTSubscribedMsgNum));

        if (!msg) {
            if (type && publisher)
                printf("IPT: Subscription to %s (%s) failed, doing nothing\n",
                       type->Name(), publisher->Name());
            else
                printf("IPT: Subscription failed, invalid parameters, doing nothing\n");
            return 0;
        }
        
        int answer;
        bcopy((char*) msg->Data(), (char*) &answer, sizeof(int));
        delete msg;

        if (!answer) { // if bad response, print warning and continue 
            printf("IPT: Publisher %s claims no knowledge of message type %s\n",
                   publisher->Name(), type->Name());
            printf("\tTrying again\n");
            Idle(0.5);
        } else {
            if (!subscription_added(publisher, type)) {
                // add connect and disconnect callbacks for publisher
                IPConnectionCallback* connect =
                    new PublicationConnCallback(type);
                IPConnectionCallback* disconnect =
                    new ServerDisconnectCallback();
                publisher->AddDisconnectCallback(disconnect);
                publisher->AddConnectCallback(connect);
                add_subscription(publisher, type, connect, disconnect);
            }

            return 1;
        }
    }
}

/* Convenience function for connecting to a publisher named "module_name"
   and subscribing to message named "msg_name".  It returns a pointer to
   the publisher's connection */
IPConnection* IPCommunicator::Subscribe(const char* module_name,
                                        const char* msg_name,
                                        int size_info, unsigned char* info)
{
    IPConnection* res = Connect(module_name);
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't subscribe to unknown message %s\n", msg_name);
        return NULL;
    }

    if (!Subscribe(res, type, size_info, info))
        return NULL;
    return res;
}

/* Do a timed subscription to a publisher of message "type" with requested
   interval of "interval" */
int IPCommunicator::Subscribe(IPConnection* publisher, IPMessageType* type,
                              double interval)
{
    static IPFormat* time_format = 0;

    if (!time_format) 
        time_format =
            FormatParser()->Parse("float");

    float sub_int = interval;
    unsigned char info[sizeof(float)];
    time_format->encodeData((void*) &sub_int, (char*) info, 0);

    return Subscribe(publisher, type, sizeof(float), info);
}

/* Subscribe to the module named "module_name" for message named "msg_name" 
   and expect messgages at most every "interval" seconds */
IPConnection* IPCommunicator::Subscribe(const char* module_name,
                                        const char* msg_name,
                                        double interval)
{
    IPConnection* res = Connect(module_name);
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't subscribe to unknown message %s\n", msg_name);
        return NULL;
    }

    if (!Subscribe(res, type, interval))
        return NULL;
    return res;
}

/* Handler for subscription messages */
void IPCommunicator::subscription_hand(IPMessage* msg)
{
    int res;
    IPTSubscriptionStruct* sub = (IPTSubscriptionStruct*) msg->FormattedData();

    /* lookup message ID in hash table */
    IPMessageType* type = LookupMessage(sub->msg_name);
    if (type) {
        IPPublication* pub = (IPPublication*)
            _publication_table->Find((char*) &type);
        if (pub) {
            // If there, add message module to publication list
            pub->AddSubscriber(msg->Connection(), sub->size_info, sub->info);
            res = 1;
            Reply(msg, LookupMessage(IPTSubscribedMsgNum), sizeof(int),
                  (unsigned char*) &res);

            /* invoke publication connection callback, if any */
            if (pub->Callback())
                pub->Callback()->Execute(msg->Connection());
        }
    } else { //otherwise, return negative to module
        res = 0;
        Reply(msg, LookupMessage(IPTSubscribedMsgNum), sizeof(int),
              (unsigned char*) &res);
    }

    msg->DeleteFormatted(sub);
}

void IPCommunicator::Unsubscribe(IPConnection* publisher, IPMessageType* type)
{
    IPSubscription sub;
    sub.publisher = publisher;
    sub.type = type;
    
    IPSubscription* internal_sub =
        (IPSubscription*) _subscription_table->Remove((char*) &sub);
    if (!internal_sub) {
        printf("IPT: Not subscribed to %s on %s\n",
               publisher->Name(), type->Name());
        return;
    }

    publisher->RemoveConnectCallback(internal_sub->connect);
    publisher->RemoveDisconnectCallback(internal_sub->disconnect);

    delete internal_sub;

    if (publisher->Active()) {
        IPTUnsubscribeStruct us;
        us.msg_name = type->Name();
        us.subscriber = ModuleName();

        SendMessage(publisher, LookupMessage(IPTUnsubscribeMsgNum),
                    (void*) &us);
    }

    printf("IPT: Unsubscribing from %s on %s\n",
           type->Name(), publisher->Name());
}

void IPCommunicator::Unsubscribe(const char* publisher_name,
                                 const char* msg_name)
{
    IPConnection* publisher = LookupConnection(publisher_name);
    if (!publisher) {
        printf("IPT: Cannot unsubscribe from non-connection %s\n",
               publisher_name);
        return;
    }

    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Cannot unsubscribe from non-message %s\n",
               msg_name);
        return;
    }

    Unsubscribe(publisher, type);
}

void IPCommunicator::unsubscribe_hand(IPMessage* msg)
{
    IPTUnsubscribeStruct* us =
        (IPTUnsubscribeStruct*) msg->FormattedData();

    IPConnection* conn = LookupConnection(us->subscriber);
    IPMessageType* type = LookupMessage(us->msg_name);
    msg->DeleteFormatted(us);

    if (!conn) {
        printf("IPT: cannot unsubscribe unknown connection %s\n",
               conn->Name());
        return;
    }
    if (!type) {
        printf("IPT: cannot unsubscribe %s from unknown message type %s\n",
               conn->Name(), type->Name());
        return;
    }

    IPPublication* pub =
        (IPPublication*) _publication_table->Find((char*) &type);
    if (!pub) {
        printf("IPT: Cannot unsubscribe %s from %s since it's not published\n",
               conn->Name(), type->Name());
        return;
    }

    if (!pub->RemoveSubscriber(conn)) 
        printf("IPT:Cannot unsubscribe %s from %s since it never subscribed\n",
               conn->Name(), type->Name());
}

/* handler for remote subscription message.  An external module can cause
   this module to subscribe to yet another module */
void IPCommunicator::remote_subscription_hand(IPMessage* msg)
{
    IPTRemoteSubscriptionStruct* rs =
        (IPTRemoteSubscriptionStruct*) msg->FormattedData();

    printf("IPT: Remotely subscribing to %s on %s\n", rs->msg_name,
           rs->publisher);
    IPMessageType* type = RegisterMessage(rs->msg_name, NULL);
    IPConnection* conn = Connect(rs->publisher, IPT_REQUIRED);
    Subscribe(conn, type, rs->size_info, rs->info);

    msg->DeleteFormatted(rs);
}
    

/* Declares this module as a server.  All clients will register with a message
   of type "registration_type".  If "registration_type" is NULL, then we use
   a default of IPTServerRegisterMsg.  If callback "cb" is non-NULL, then we
   invoke it whenever a client registers */
void IPCommunicator::Server(IPMessageType* registration_type,
                            IPHandlerCallback* cb)
{
    if (!registration_type)
        registration_type = LookupMessage(IPTServerRegisterMsgNum);

    printf("IPT: Declared server with registration type %s\n",
           registration_type->Name());

    RegisterHandler(registration_type, new ServerCallback(cb));
}

/* Convenience function to declare this module a server.  If "msg_name" is
   non-NULL, it is the name of the message type to be the server registration
   message.  If it is null, it defaults to "IPTServerRegisterMsg".  "cb" is
   still the (optional) client registration callback */
void IPCommunicator::Server(const char* msg_name, IPHandlerCallback* cb)
{
    IPMessageType* type = NULL;
    if (msg_name) {
        type = LookupMessage(msg_name);
        if (!type) {
            printf("IPT: Can't serve with unknown mesage %s\n", msg_name);
            return;
        }
    }

    Server(type, cb);
}

/* Used as an interation function for Return.  Checks to see if the
   connection (passed in as "item) has the same name as "param" */
static int check_connection_name(const char* param, const char* item)
{
    return (strcmp(((IPConnection*) item)->Name(), param) == 0);
}

/* Add a client "client" to the client list */
void IPCommunicator::AddClient(IPConnection* client)
{
    if (!_clients->Return(check_connection_name, client->Name()))
        _clients->Prepend((char*) client);
}

/* Broadcast message "msg" to all active clients */
void IPCommunicator::Broadcast(IPMessage* msg)
{
    for (IPConnection* conn = (IPConnection*) _clients->First(); conn;
         conn = (IPConnection*) _clients->Next())
        if (conn->Active())
            SendMessage(conn, msg);
}

/* Broadcast message of type "type" and formatted data "data" to all clients */
void IPCommunicator::Broadcast(IPMessageType* type, void* data)
{
    IPMessage out(type, generate_instance_num(), data);
    Broadcast(&out);
}

/* Broadcast message of type "type" and "size" bytes of unformatted data in
   "data" to all clients */
void IPCommunicator::Broadcast(IPMessageType* type,
                               int size, unsigned char* data)
{
    IPMessage out(type, generate_instance_num(), size, data);
    Broadcast(&out);
}

/* Broadcast message with name "name" and formatted data "data" to all
   clients */
void IPCommunicator::Broadcast(const char* msg_name, void* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't broadcast unknown message %s\n", msg_name);
        return;
    }

    Broadcast(type, data);
}

/* Broadcast message with name "name" and "size" bytes of unformatted data in
   "data" to all clients */
void IPCommunicator::Broadcast(const char* msg_name,
                               int size, unsigned char* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't broadcast unknown message %s\n", msg_name);
        return;
    }

    Broadcast(type, size, data);
}

/* Make yourself a client of "server" with registration message reg_msg.
   "reconnect" is 1 if called internally to reconnect from a connect 
   callback */
void IPCommunicator::Client(IPConnection* server, IPMessage* reg_msg)
{
    if (!reg_msg) {
        reg_msg = new IPMessage(IPTServerRegisterMsgNum,
                                generate_instance_num(), 0, 0);
    } else
        reg_msg = new IPMessage(*reg_msg);

    IPServerRecord* record =
        (IPServerRecord*) _server_table->Find((char*) &server);
    if (!record) {
        record = new IPServerRecord;
        record->server = server;
        record->already_registered = 0;
        _server_table->Insert((char*) &server,
                              sizeof(IPConnection*), (char*) record);
        record->connect = new ServerConnectCallback(record);
        record->disconnect = new ServerDisconnectCallback(record);
        server->AddConnectCallback(record->connect);
        server->AddDisconnectCallback(record->disconnect);
    } else {
        if (record->already_registered) {
            delete reg_msg;
            return;
        }
        if (reg_msg != record->registration_message)
            delete record->registration_message;
        else {
            delete reg_msg;
            reg_msg = record->registration_message;
        }
    }
    record->registration_message = reg_msg;

    if (!record->already_registered && server->Active()) {
        SendMessage(server, reg_msg);
        printf("IPT: Declaring myself a client of %s\n", server->Name());
        record->already_registered = 1;
    }
}

IPMessage* IPCommunicator::RegistrationMessage(IPConnection* server)
{
    IPServerRecord* record =
        (IPServerRecord*) _server_table->Find((char*) &server);
    if (!record)
        return NULL;

    return record->registration_message;
}

/* Make yourself a client of module named "module_name" with registration 
   message msg. */
IPConnection* IPCommunicator::Client(const char* module_name, IPMessage* msg)
{
    IPConnection* conn = Connect(module_name);
    if (!conn) {
        printf("IPT: Error connecting to server %s\n", module_name);
        return NULL;
    }

    Client(conn, msg);

    return conn;
}

/* Make yourself a client of module named "module_name" with message named
   msg_name and formatted data "data" */
IPConnection* IPCommunicator::Client(const char* server_name,
                                     const char* msg_name, void* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't connect as client to %s with message %s\n",
               server_name, msg_name);
        return NULL;
    }
    IPMessage out(type, generate_instance_num(), data);
    return Client(server_name, &out);
}

/* Make yourself a client of module with message named
   msg_name and formatted data "data" */
void IPCommunicator::Client(IPConnection* module,
                            const char* msg_name, void* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't connect as client to %s with message %s\n",
               module->Name(), msg_name);
        return;
    }
    IPMessage out(type, generate_instance_num(), data);
    Client(module, &out);
}

/* Make yourself a client of module named "module_name" with message named
   msg_name and unformatted data */
IPConnection* IPCommunicator::Client(const char* server_name,
                                     const char* msg_name,
                                     int size, unsigned char* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't connect as client to %s with message %s\n",
               server_name, msg_name);
        return NULL;
    }
    IPMessage out(type, generate_instance_num(), size, data);
    return Client(server_name, &out);
}

/* Used as an iterator to delete all message types when deleting the
   publications hash table */
static int delete_publications(char*, char* data)
{
    delete (IPPublication*) data;

    return 1;
}

static int delete_subscriptions(char*, char* data)
{
    delete (IPSubscription*) data;

    return 1;
}

static int delete_servers(char*, char* data)
{
    delete (IPServerRecord*) data;

    return 1;
}

/* hash function for subscription structions */
static int sub_hash(const char* ptr)
{
    IPSubscription* sub = (IPSubscription*) ptr;
    return (long(sub->publisher) + long(sub->type)) >> 3;
}

/* equality functions for subscription structures */
static int sub_eq(const char* s1, const char* s2)
{
    IPSubscription* sub1 = (IPSubscription*) s1;
    IPSubscription* sub2 = (IPSubscription*) s2;

    return ((sub1->publisher == sub2->publisher) &&
            (sub1->type == sub2->type));
}

/* initialize the extra stuff for publisher/subscriber and client/server */
void IPCommunicator::init_extras()
{
    _publication_table = new IPHashTable(32, ptr_hash, int_eq);
    _server_table = new IPHashTable(32, ptr_hash, int_eq);
    _subscription_table = new IPHashTable(32, sub_hash, sub_eq);
    _clients = IPList::Create();
}

/* delete the extra stuff */
void IPCommunicator::delete_extras()
{
    _publication_table->Iterate(delete_publications);
    delete _publication_table;
    _subscription_table->Iterate(delete_subscriptions);
    delete _subscription_table;
    _server_table->Iterate(delete_servers);
    delete _server_table;
    IPList::Delete(_clients);
}

/* Create the appropriate IPCommunicator instance depending on the machine
   type */
IPCommunicator* IPCommunicator::Instance(const char* mod_name,
                                         const char* host_name)
{
    TCPCommunicator* res;
#ifdef VXWORKS
#ifndef NO_VXCONNECTION    
    res = new VxCommunicator(mod_name, host_name);    
#else
    res = new TCPCommunicator(mod_name, host_name);
#endif
#else

#if defined(SOLARIS) || defined(LYNX)
    res = new TCPCommunicator(mod_name, host_name);
#else
    res = new UnixCommunicator(mod_name, host_name);
#endif
#endif

    res->send_routes();

    return res;
}

/* Cause the module connected by "conn" to be subscribed to "type" with
   size_info bytes of information "info" */
void IPCommunicator::SubscribeConnection(IPConnection* conn,
                                         IPMessageType* type,
                                         int size_info, unsigned char* info)
{
    IPTRemoteSubscriptionStruct rss;
    rss.publisher = (char*) ModuleName();
    rss.msg_name = (char*) type->Name();
    rss.size_info = size_info;
    rss.info = info;

    SendMessage(conn, IPTRemoteSubscriptionMsgNum, &rss);
}

/* Connect to module named "conn_name" and cause it to be subscribe to 
   the message type named "msg_name" on this module with size_info bytes of 
   information "info" */
IPConnection* IPCommunicator::SubscribeConnection(const char* conn_name,
                                                  const char* msg_name,
                                                  int size_info,
                                                  unsigned char* info)
{
    IPTRemoteSubscriptionStruct rss;
    IPConnection* conn = Connect(conn_name);

    rss.publisher = (char*) ModuleName();
    rss.msg_name = (char*) msg_name;
    rss.size_info = size_info;
    rss.info = info;
    SendMessage(conn, IPTRemoteSubscriptionMsgNum, &rss);

    return conn;
}
    
/* used by subscribers to keep track of their subscriptions.  Add the fact
   that we are subscribing to publisher for type "type" to the internal 
   subscription table */
void IPCommunicator::add_subscription(IPConnection* publisher,
                                      IPMessageType *type,
                                      IPConnectionCallback* connect,
                                      IPConnectionCallback* disconnect)
{
    IPSubscription sub;
    sub.publisher = publisher;
    sub.type = type;
    sub.connect = connect;
    sub.disconnect = disconnect;
    
    if (_subscription_table->Find((char*) &sub))
        return;

    IPSubscription* new_sub = new IPSubscription;
    bcopy((char*) &sub, (char*) new_sub, sizeof(IPSubscription));
    
    _subscription_table->Insert((char*) &sub, sizeof(IPSubscription),
                                (char*) new_sub);
}

/* return 1 if we have already subscribed to this connection for this type */
int IPCommunicator::subscription_added(IPConnection* publisher,
                                       IPMessageType* type)
{
    IPSubscription sub;
    sub.publisher = publisher;
    sub.type = type;
    
    return (_subscription_table->Find((char*) &sub) != NULL);
}

/* Add a timed client to the publication list */
int IPTimedPublication::AddSubscriber(IPConnection* conn,
                                      int size_info, unsigned char* info)
{
    static IPFormat* time_format = 0;

    if (!time_format) 
        time_format =
            conn->Communicator()->FormatParser()->Parse("float");

    float interval;
    if (size_info != sizeof(float) )
        interval = 0.0;
    else {
        time_format->decodeData((char*) info, 0, sizeof(float),
                                (char*) &interval,
                                conn->ByteOrder(), conn->Alignment());
        if (interval < 0.0 || interval > 1000000.0) {
            fprintf(stderr,
                    "TimedPublication: Interval of %f from client %s is out of range\n", 
                    interval, conn->Name());
            interval = 0.0;
        }
    }

    return add_subscriber(new IPTimedClient(interval, conn));
}

/* create a timed client, and make sure the check will come up 1 the first
   time */
IPTimedClient::IPTimedClient(double interval, IPConnection* conn)
    : IPClient(conn)
{
    _interval = interval;

    _last_published =
        IPTime(IPTime::Current().Value() - 2.0*interval - 1.0);
}

/* Return true only if more than _interval seconds have elapsed since the
   last send */
int IPTimedClient::Check()
{
    double interval = (IPTime::Current() - _last_published).Value();

    return (interval >= _interval);
}

/* Send the message and record the publishing time */
int IPTimedClient::SendMessage(IPCommunicator* comm, IPMessage* msg)
{
    _last_published = IPTime::Current();
    return comm->SendMessage(Connection(), msg);
}

/* create a publication connect callback, given some extra information */
PublicationConnCallback::PublicationConnCallback(IPMessageType* type,
                                                 int size, unsigned char* info)
{
    _type = type;
    _size_info = size;
    if (_size_info) {
        _info = new unsigned char[size];
        bcopy((char*) info, (char*) _info, size);
    } else
        _info = NULL;
}

/* delete a publication callback and it's information (if any) */
PublicationConnCallback::~PublicationConnCallback()
{
    if (_size_info)
        delete _info;
}
