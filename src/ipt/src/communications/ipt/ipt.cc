///////////////////////////////////////////////////////////////////////////////
//
//                                 ipt.cc
//
// This file implements the base class for the IPT communications package.
// I call it IPT because it uses the "back end" of TCX for reliable, flexible,
// and expandable InterProcess message based communication.
//
// Classes defined for export:
//    IPCommunicator
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
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#ifdef VXWORKS
#include <taskLib.h>
#include <hostLib.h>
#else
#include "./libc.h"
#endif

#include <ipt/ipt.h>
#include <ipt/internal_messages.h>
#include <ipt/primcomm.h>
#include <ipt/callbacks.h>
#include <ipt/messagetype.h>
#include <ipt/message.h>
#include <ipt/hash.h>
#include <ipt/parseformat.h>
#include <ipt/format.h>
#include <ipt/connection.h>
#include <ipt/list.h>
#include <ipt/event.h>
#include <ipt/filter.h>
#include <ipt/time.h>
#include <ipt/destination.h>
#include <ipt/pigeonhole.h>
#include <ipt/formconfig.h>
#include <ipt/fakeconnection.h>
#include <ipt/translator.h>
#include <ipt/timertable.h>
#include <ipt/timer.h>
#include <pthread.h>

pthread_mutex_t sendMutex = PTHREAD_MUTEX_INITIALIZER;

int IPCommunicator::_parallelism_level = 0;

const int IPCommunicator::ReceiveAll = IPT_HNDL_ALL;
const int IPCommunicator::ReceiveOnce = IPT_HNDL_STD;
const int IPCommunicator::ReceiveNone = IPT_HNDL_NONE;

declareHandlerCallback(IPCommunicator);
implementHandlerCallback(IPCommunicator);

/* routine used when an IPT error occurs. */
static void ipt_error(const char* string)
{
    fprintf(stderr, "IPT Error: %s\n", string);
}

/* Create an IPCommunicator with a module name and host name for a server */
IPCommunicator::IPCommunicator(const char* mod_name, const char* host_name)
{
    if (!mod_name) {
        fprintf(stderr, "IPT Error: No module name\n");
        exit(-1);
    }

    initialize();

    int len = strlen(mod_name)+1;
    _mod_name = new char[len];
    bcopy(mod_name, _mod_name, len);

    if (!host_name) {
        host_name = getenv("IPTHOST");
        if (!host_name) {
            host_name = _this_host;
        }
    }

    printf("\t\t\tIPT Client\n");
    printf("\t\t\tVersion %d.%d.%d\n", IPT_MAJOR_VERSION, IPT_MINOR_VERSION,
           IPT_SUB_VERSION);
    printf("\t\tModule Name: %s\n", mod_name);
    printf("\t\tServer Host Name: %s\n", host_name);

    len = strlen(host_name)+1;
    _host_name = new char[len];
    bcopy(host_name, _host_name, len);
}

/* Create an IPCommunicator that has no module name and does not have an 
   intended server host machine name */
IPCommunicator::IPCommunicator()
{
    initialize();

    _mod_name = NULL;
    _host_name = NULL;
    _domain_name = NULL;
}

/* initialize all the things that need initialized in IPT */
void IPCommunicator::initialize()
{
    _start_blocking_action = _end_blocking_action = NULL;
    _message_table = new IPHashTable(100, str_hash, str_eq);
    _id_to_message_table = new IPHashTable(100, int_hash, int_eq);
    _connection_table = new IPHashTable(32, str_hash, str_eq);
    _connection_list = IPList::Create();
    _administrative_list = IPList::Create();
    _event_list = IPList::Create();
    _message_queue = IPList::Create();
    _hole_queue = IPList::Create();
    _parser = new IPFormatParser();
    _default_destination = new DefaultDestination();
    _instance_num = 1;
    _handlers_active = 1;
    _connect_callbacks = IPList::Create();
    _disconnect_callbacks = IPList::Create();
    _disconn_handlers_active = 1;
    _timer_table = new IPTimerTable();
    _finished = 0;

    _this_host = new char[100];

    char* host_ptr = getenv("IPTMACHINE");
    if (host_ptr)
        strcpy(_this_host, host_ptr);
    else {
        gethostname(_this_host, 99);
        _this_host[99] = '\0';
    }
    _domain_name = NULL;

    register_built_in_messages();

    init_extras();
    if (getenv("IPTLOGGING"))
        _logging = 1;
    else
        _logging = 0;

    char* output_name = getenv("IPTOUTPUT");
    if (!output_name)
        _out_fp = stdout;
    else {
        if (!strcmp(output_name, "none")) 
            _out_fp = fopen("/dev/null", "w");
        else if (!strcmp(output_name, "stdout"))
            _out_fp = stdout;
        else
            _out_fp = fopen(output_name, "w");
        if (!_out_fp)
            _out_fp = stdout;
    }
}

/* Used as an iterator to delete all message types when deleting the message
   type hash table */
static int delete_message_types(char*, char* data)
{
	if(data) {
    	delete (IPMessageType*) data;
		data = NULL;
	}

    return 1;
}

/* delete an IPCommunicator.  Deactivates all active connections, and deletes
   all allocated memory */
IPCommunicator::~IPCommunicator()
{
    IPMessage* message;
    for (message = (IPMessage*) _message_queue->First(); message;
         message = (IPMessage*) _message_queue->Next())
        delete message;

    IPList::Delete(_message_queue); // <---- must delete extant messages
    IPList::Delete(_hole_queue);
    
    IPConnection* conn;
    for (conn = (IPConnection*) _connection_list->First(); conn;
         conn = (IPConnection*) _connection_list->Next()) {
        conn->Deactivate();
        delete conn;
    }

    enable_disconnect_handlers();
    for (conn = (IPConnection*) _administrative_list->First(); conn;
         conn = (IPConnection*) _administrative_list->Next()) {
        delete conn;
    }

    IPEvent* event;
    for (event = (IPEvent*) _event_list->First(); event;
         event = (IPEvent*) _event_list->Next())
        delete event;
    IPList::Delete(_event_list);
    
    if (_mod_name)
        delete _mod_name;
    if (_host_name)
        delete _host_name;
    if (_domain_name)
        delete _domain_name;
    delete _this_host;

    _message_table->Iterate(delete_message_types);
    delete _message_table; 
    delete _id_to_message_table; 
    delete _connection_table;
    IPList::Delete(_connection_list);
    IPList::Delete(_administrative_list);

    IPConnectionCallback* cb;
    for (cb = (IPConnectionCallback*) _connect_callbacks->First(); cb;
         cb = (IPConnectionCallback*) _connect_callbacks->Next())
        IPResource::unref(cb);
    IPList::Delete(_connect_callbacks);

    for (cb = (IPConnectionCallback*) _disconnect_callbacks->First(); cb;
         cb = (IPConnectionCallback*) _disconnect_callbacks->Next())
        IPResource::unref(cb);
    IPList::Delete(_disconnect_callbacks);

    delete_extras();

    delete _parser;
    delete _default_destination;

    delete _timer_table;
    IPResource::clear();
}

/* Register the built in messages.  The built in messages are those that
   need no "name" and whose ID's are known a priori to be the same by all
   IPCommunicators, whether they be servers or clients */
void IPCommunicator::register_built_in_messages()
{
    register_message(IPTVersionInfoMsgNum, IPT_VERSION_INFO_MSG,
                     IPT_VERSION_INFO_FORM);
    register_message(IPTModuleConnectingMsgNum, IPT_MODULE_CONNECTING_MSG,
                     IPT_MODULE_CONNECTING_FORM);
    register_message(IPTRequestConnectionMsgNum, IPT_REQUEST_CONNECTION_MSG,
                     IPT_REQUEST_CONNECTION_FORM);
    register_message(IPTRegisterMessageMsgNum, IPT_REGISTER_MESSAGE_MSG,
                     IPT_REGISTER_MESSAGE_FORM);
    register_message(IPTRegisterMessageSetMsgNum, IPT_REGISTER_MESSAGE_SET_MSG,
                     IPT_REGISTER_MESSAGE_SET_FORM);
    register_message(IPTMessageRegisteredMsgNum, IPT_MESSAGE_REGISTERED_MSG,
                     IPT_MESSAGE_REGISTERED_FORM);
    register_message(IPTMessageSetRegisteredMsgNum,
                     IPT_MESSAGE_SET_REGISTERED_MSG,
                     IPT_MESSAGE_SET_REGISTERED_FORM);
    register_message(IPTSubscriptionMsgNum,
                     IPT_SUBSCRIPTION_MSG, IPT_SUBSCRIPTION_FORM);
    register_message(IPTSubscribedMsgNum,
                     IPT_SUBSCRIBED_MSG, IPT_SUBSCRIBED_FORM);
    register_message(IPTRemoteSubscriptionMsgNum,
                     IPT_REMOTE_SUBSCRIPTION_MSG,
                     IPT_REMOTE_SUBSCRIPTION_FORM);
    register_message(IPTServerRegisterMsgNum,
                     IPT_SERVER_REGISTER_MSG, IPT_SERVER_REGISTER_FORM);
    register_message(IPTLoggingMsgNum,
                     IPT_LOGGING_MSG, IPT_LOGGING_FORM);
    register_message(IPTUnsubscribeMsgNum,
                     IPT_UNSUBSCRIBE_MSG, IPT_UNSUBSCRIBE_FORM);
    register_message(IPTDeletionActionMsgNum,
                     IPT_DELETION_ACTION_MSG, IPT_DELETION_ACTION_FORM);
    register_message(IPTRequestTranslationMsgNum, IPT_REQUEST_TRANSLATION_MSG,
                     IPT_REQUEST_TRANSLATION_FORM);
    register_message(IPTTranslationMsgNum, IPT_TRANSLATION_MSG,
                     IPT_TRANSLATION_FORM);

    RegisterHandler(LookupMessage(IPTSubscriptionMsgNum),
                    new HandlerCallback(IPCommunicator)
                    (this,&IPCommunicator::subscription_hand));
    RegisterHandler(LookupMessage(IPTRemoteSubscriptionMsgNum),
                    new HandlerCallback(IPCommunicator)
                    (this, &IPCommunicator::remote_subscription_hand));
    RegisterHandler(LookupMessage(IPTUnsubscribeMsgNum),
                    new HandlerCallback(IPCommunicator)
                    (this, &IPCommunicator::unsubscribe_hand));
}

/* Create a message type with ID, id, name, msg_name, and format specified by 
   msg_format (which can be NULL to indicate no format needed), and enter the
   message type into the appropriate hash tables */
IPMessageType* IPCommunicator::register_message(int id, const char* msg_name,
                                                const char* msg_format)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (type) {
        printf("IPT: Warning, already defined message %s\n", msg_name);
        return type;
    }
    
    IPFormat* format;
    if (msg_format)
        format = _parser->Parse(msg_format);
    else
        format = NULL;

    type = new IPMessageType(msg_name, format, id);
    _message_table->Insert(msg_name, strlen(msg_name)+1, (char*) type);
    if (id != -1)
        _id_to_message_table->Insert((char*) &id, sizeof(int), (char*) type);

    return type;
}

/* Lookup a connection by name in the connection hash table */
IPConnection* IPCommunicator::LookupConnection(const char* name)
{
    return (IPConnection*) _connection_table->Find(name);
}

/* Look up a message type by name in the message type hash table */
IPMessageType* IPCommunicator::LookupMessage(const char* name)
{
    return (IPMessageType*) _message_table->Find(name);
}

/* Look up a message type by ID in the message type hash table */
IPMessageType* IPCommunicator::LookupMessage(int id)
{
    return (IPMessageType*) _id_to_message_table->Find((char*) &id);
}

/* Register a handler on messages of type "mess".  The callback is specified 
   by "callback".  "spec" is one of IPT_HNDL_STD or IPT_HNDL_ALL.

   Once the handler is registered, the unhandled messages queue is checked
   for messages of that type.  If there are any, the handler specified by
   "callback" is invoked for each message */
void IPCommunicator::RegisterHandler(IPMessageType* mess,
                                     IPHandlerCallback* callback,
                                     int spec)
{
    if (!mess) {
        printf("IPT: Warning, cannot register a NULL message, doing nothing\n");
        return;
    }
    mess->Handler(callback, spec);

    check_message_queues();
}

/* Convenience routine for registering a handler on a named message.  It does
   nothing if "msg_name" is not a registered message name */
void IPCommunicator::RegisterHandler(const char* msg_name,
                                     IPHandlerCallback* callback,
                                     int spec)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (type) 
        RegisterHandler(type, callback, spec);
    else 
        printf("IPT: Cannot handle for unknown message %s\n", msg_name);
}

/* Convenience routine for registering a handler on a message type with a 
   routine "func" rather than an instance of class IPHandlerCallback */
void IPCommunicator::RegisterHandler(IPMessageType* type,
                                     void (*func)(IPCommunicator*,
                                                  IPMessage*, void*),
                                     void* data,
                                     int spec)
{
    IPHandlerCallback* cb = new IPRoutineHandlerCallback(this, func, data);

    RegisterHandler(type, cb, spec);
}

/* register routine handler by message name */
void IPCommunicator::RegisterHandler(const char* msg_name,
                                     void (*func)(IPCommunicator*, IPMessage*,
                                                  void*),
                                     void* data,
                                     int spec)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Cannot register handler for unknown type %s\n", msg_name);
        return;
    }

    RegisterHandler(type, func, data, spec);
}

/* Adds connection "conn" as a new connection */
void IPCommunicator::AddConnection(IPConnection* conn)
{
    if (LookupConnection(conn->Name())) {
        return;
    }

    _connection_table->Insert(conn->Name(), strlen(conn->Name())+1,
                              (char*) conn);
    _connection_list->Prepend((char*) conn);
}

/* Send message "msg" to connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, IPMessage* msg)
{
    if (!conn || !msg) {
        ipt_error("Invalid parameters to SendMessage, doing nothing");
        return -1;
    }

    if (_logging) 
        LogSend(conn, msg);
    
    return conn->Send(msg);
}

/* Create a message with name "msg_name" and formatted data "data" and 
   send it to connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, const char* msg_name,
                                void *data)
{
    if (!conn) {
        ipt_error("Invalid connection for SendMessage, doing nothing");
        return -1;
    }

    IPMessageType* type = LookupMessage(msg_name);

    if (!type) {
        ipt_error("Unknown message name for SendMessage, doing nothing");
        return -1;
    }

    IPMessage msg(type, generate_instance_num(), data);
    return SendMessage(conn, &msg);
}

/* Create a message with name "msg_name" and unformatted data of size "size"
   and value "data" and send it to connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, const char* msg_name,
                                int size, unsigned char* data)
{
    if (!conn) {
        ipt_error("Invalid connection for SendMessage, doing nothing");
        return -1;
    }

    IPMessageType* type = LookupMessage(msg_name);

    if (!type) {
        ipt_error("Unknown message name for SendMessage, doing nothing");
        return -1;
    }

    IPMessage msg(type, generate_instance_num(), size, data);
    return SendMessage(conn, &msg);
}

/* Create a message with ID "msg_name" and formatted data "data" and send it
   through connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, int msg_num,
                                void *data)
{
    if (!conn) {
        ipt_error("Invalid connection for SendMessage, doing nothing");
        return -1;
    }

    IPMessageType* type = LookupMessage(msg_num);

    if (!type) {
        ipt_error("Unknown message ID for SendMessage, doing nothing");
        return -1;
    }

    IPMessage msg(type, generate_instance_num(), data);
    return SendMessage(conn, &msg);
}

/* Create a message with type "type" and formatted data "data" and send it
   through connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, IPMessageType* type,
                                void *data)
{
    if (!conn) {
        ipt_error("Invalid connection for SendMessage, doing nothing");
        return -1;
    }

    IPMessage msg(type, generate_instance_num(), data);
    return SendMessage(conn, &msg);
}

/* Create a message with type "type" and unformatted data "data" (of size
   "size") and send it through connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, IPMessageType* type,
                                int size, unsigned char* data)
{
    if (!conn) {
        ipt_error("Invalid connection for SendMessage, doing nothing");
        return -1;
    }

    IPMessage msg(type, generate_instance_num(), size, data);
    return SendMessage(conn, &msg);
}

/* Deactivate connection "conn" */
void IPCommunicator::DeactivateConnection(IPConnection* conn)
{
    if (conn)
        conn->Deactivate();
    else 
        ipt_error("Invalid connection for Deactivate, doing nothing");
}

/* Return the next message on the internal unhandled message queues */
IPMessage* IPCommunicator::NextReceivedMessage()
{
    IPPigeonHole* hole = pop_hole();

    if (!hole) {
        IPMessage* res = pop_message();
        if (res)
            res->Connection()->Free(res->SizeData());
        return res;
    } else {
        hole->Dequeue();
        return hole->Empty();
    }
}

/* Wait for the next received message from any connection of any type for
   "timeout" seconds.  If "timeout" is IPT_BLOCK, wait forever.  The routine
   returns the message, or NULL if there was no message or was an error */
IPMessage* IPCommunicator::ReceiveMessage(double timeout)
{
    IPMessage* msg = NextReceivedMessage();

    if (msg)
        return msg;

    ScanForMessages(timeout);

    IPPigeonHole* hole = pop_hole();

    if (!hole) {
        IPMessage* res = pop_message();
        if (res)
            res->Connection()->Free(res->SizeData());
        return res;
    } else
        return hole->Empty();
}

/* For a totally handler based system, this can be the main loop.  It only
   returns if someone sets _finished to 1 */
void IPCommunicator::MainLoop()
{
    while (1) {
        if (Finished())
            return;
        ScanForMessages(IPT_BLOCK);
    }
}

/* Used as an iterator over the message queue.  Returns true if a filter
   matches the current message */
static int match_msg_filter(const char* param, const char* item)
{
    IPMessage* msg = (IPMessage*) item;
    IPFilter* filter = (IPFilter*) param;
    
    return filter->Check(msg, msg->Connection());
}
            
/* Used as an iterator over the pigeon hole queue.  Returns true if a filter
   matches message of a current pigeon hole */
static int match_hole_filter(const char* param, const char* item)
{
    IPPigeonHole* hole = (IPPigeonHole*) item;
    IPFilter* filter = (IPFilter*) param;
    IPMessage* msg = hole->Contents();

    return msg && filter->Check(msg, msg->Connection());
}

/* Scans the internal queues of unhandled messages for a message that
   matches "filter".  Returns the matching message, or NULL, if there was
   no matching message */
IPMessage* IPCommunicator::ScanReceivedMessages(IPFilter* filter)
{
    if (!filter)
        return NextReceivedMessage();
    
    IPPigeonHole* hole = remove_hole(match_hole_filter, (char*) filter);
    if (hole) {
        hole->Dequeue();
        return hole->Empty();
    }

    IPMessage* res = remove_message(match_msg_filter, (char*) filter);
    if (res)
        res->Connection()->Free(res->SizeData());
    return res;
}

/* Returns the next message that matches "filter" that comes in in the next
   "timeout" seconds.  If "timeout" is IPT_BLOCK, there is no time limit.
   Returns NULL if no such message comes in the allotted time or there is an
   error.  */
IPMessage* IPCommunicator::ReceiveMessage(IPFilter* filter, double timeout)
{
    if (_finished)
        return NULL;

    if (!filter)
        return ReceiveMessage(timeout);

    ProcessEvents();
    IPMessage* msg = ScanReceivedMessages(filter);
    if (msg)
        return msg;

    IPTime start = IPTime::Current();
    int blocking = 0;

    if (timeout == IPT_BLOCK)
        blocking = 1;

    while (1) {
        if (_finished)
            return NULL;

        ScanForMessages(timeout);
        msg = ScanReceivedMessages(filter);
        if (msg)
            return msg;
        if (!filter->Valid())
            return NULL;

        if (!blocking) {
            IPTime now = IPTime::Current();
            timeout -= (now-start).Value();
            if (timeout <= 0)
                return NULL;
            start = now;
        }
    }
}

/* Convenience routine for getting a message of type "type" from conneciton
   "conn".  If "conn" is NULL, it means accept from any connection and if 
   "type" is NULL, it means accept messages of any type */
IPMessage* IPCommunicator::ReceiveMessage(IPConnection* conn,
                                          IPMessageType* type,
                                          double timeout)
{
    int state = 0;
    if (type && type->Handler()) {
        state = type->HandlerActive();
        if (state)
            DisableHandler(type);
    }

    if (type || conn) {
        MsgFilter f(type, conn);
        IPMessage* msg = ReceiveMessage(&f, timeout);
        if (state)
            EnableHandler(type);
        return msg;
    } else
        return ReceiveMessage(timeout);
}

/* Convenience routine for getting a message with name "msg_name" from 
   connection "conn". If "conn" is NULL, it means accept from any connection
   and if "msg_name" is NULL, it means accept messages of any type */
IPMessage* IPCommunicator::ReceiveMessage(IPConnection* conn,
                                          const char* msg_name,
                                          double timeout)
{
    IPMessageType* t = NULL;
    if (msg_name) {
        t = LookupMessage(msg_name);
        if (!t) {
            printf("IPT Warning: Cannot receive unknown message %s\n",
                   msg_name);
            return NULL;
        }
    }

    return ReceiveMessage(conn, t, timeout);
}

/* Receive a message of name "msg_name" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
void* IPCommunicator::ReceiveFormatted(IPConnection* conn,
                                       const char* msg_name, double timeout)
{
    IPMessage* msg = ReceiveMessage(conn, msg_name, timeout);
    if (!msg)
        return NULL;

    void* res = msg->FormattedData(1);
    delete msg;

    return res;
}

/* Receive a message of name "msg_name" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
int IPCommunicator::ReceiveFormatted(IPConnection* conn,
                                     const char* msg_name, void* msg_data,
                                     double timeout)
{
    IPMessage* msg = ReceiveMessage(conn, msg_name, timeout);
    if (!msg)
        return 0;

    msg->FormattedData(msg_data, 1);
    delete msg;

    return 1;
}

/* Receive a message of type "type" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
void* IPCommunicator::ReceiveFormatted(IPConnection* conn, IPMessageType* type,
                                       double timeout)
{
    IPMessage* msg = ReceiveMessage(conn, type, timeout);
    if (!msg)
        return NULL;

    void* res = msg->FormattedData(1);
    delete msg;

    return res;
}

/* Receive a message of type "type" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
int IPCommunicator::ReceiveFormatted(IPConnection* conn, IPMessageType* type,
                                     void* msg_data,
                                     double timeout)
{
    IPMessage* msg = ReceiveMessage(conn, type, timeout);
    if (!msg)
        return 0;

    msg->FormattedData(msg_data, 1);
    delete msg;

    return 1;
}

/* This routine implements the default method for waiting for input from
   all connections for "timeout" seconds.  If "timeout" is IPT_BLOCK there is
   no time limit on the wait.  

   This routine will most likely be overridden in the descendents of
   IPCommunicator.  It just checks all active connections for data,
   and for every one with data it puts an event on the event queue.  As soon
   as data is availble it returns 1.  0 is returned if the timeout expires */
int IPCommunicator::WaitForInput(double timeout)
{
    IPTime start = IPTime::Current();
    int blocking = 0;
    if (timeout == IPT_BLOCK)
        blocking = 1;

    while (1) {
        if (Finished())
            return 0;

        int res = 0;
        IPConnection* conn = (IPConnection*) Connections()->First();
        for (;conn;conn = (IPConnection*) Connections()->Next())
            if (conn->Active() && conn->DataAvailable()) {
                AddEvent(new DataEvent(conn));
                res = 1;
            }

        if (res)
            return 1;
        
        if (!blocking && (IPTime::Current() - start).Value() >= timeout)
            return 0;

#ifdef VXWORKS
        taskDelay(1);
#else
        usleep(16667);
#endif
    }
}

/* This routine processes all pending events, checks the connections for fresh
   input, and processes the events that result from that fresh input.  It 
   waits for "timeout" seconds for new input, and returns 1 if it gets some
   and 0 if not */
int IPCommunicator::ScanForMessages(double timeout)
{
    if (_finished)
        return 0;

    ProcessEvents();

    double next_firing = _timer_table->TimeLeft();

    int res;
    if (next_firing == -1.0 || (next_firing >= timeout && timeout != -1.0)) {
        res = WaitForInput(timeout);
        if (next_firing == 0.0) 
            _timer_table->FireTimers();
    } else {
        if (timeout == -1.0) {
            while (1) {
                res = WaitForInput(next_firing);
                _timer_table->FireTimers();
                if (res) 
                    break;
                next_firing = _timer_table->TimeLeft();
            }
        } else {
            IPTime start, now;
            while (timeout >= next_firing && next_firing != -1.0) {
                start = IPTime::Current();
                res = WaitForInput(next_firing);
                if (_finished)
                    return 0;
                _timer_table->FireTimers();

                if (res) 
                    break;

                now = IPTime::Current();
                timeout -= (now - start).Value();
                if (timeout < 0.0)
                    timeout = 0.0;
                next_firing = _timer_table->TimeLeft();
                start = now;
            }
            if (!res && timeout != 0.0) 
                res = WaitForInput(timeout);
        }
    }

    ProcessEvents();

    return res;
}

/* Process all pending events */
void IPCommunicator::ProcessEvents()
{
    IPEvent* event;

    while ((event = pop_event())) {
        event->Handle(this);
        delete event;
    }
}

/* Add an event to the event queue */
void IPCommunicator::AddEvent(IPEvent* event)
{
    _event_list->Append((char*) event);
}

IPEvent* IPCommunicator::pop_event()
{
    return (IPEvent*) _event_list->Pop();
}

/* Add a message to the unhandled messages queue */
void IPCommunicator::QueueMessage(IPMessage* msg)
{
    _message_queue->Append((char*) msg);
}

/* Add a pigeon hole to the unhandled active pigeon hole queue */
void IPCommunicator::QueuePigeonHole(IPPigeonHole* ph)
{
    _hole_queue->Append((char*) ph);
    ph->Enqueue();
}

IPMessage* IPCommunicator::pop_message()
{
    return (IPMessage*) _message_queue->Pop();
}

IPPigeonHole* IPCommunicator::pop_hole()
{
    return (IPPigeonHole*) _hole_queue->Pop();
}

IPMessage* IPCommunicator::remove_message(int (*func)(const char*,
                                                      const char*),
                                          const char* param, int all)
{
    if (all) {
        _message_queue->RemoveAll(func, param);
        return NULL;
    } else
        return (IPMessage*) _message_queue->Remove(func, param);
}

IPPigeonHole* IPCommunicator::remove_hole(int (*func)(const char*,
                                                      const char*),
                                          const char* param, int all)
{
    if (all) {
        _hole_queue->RemoveAll(func, param);
        return NULL;
    } else
        return (IPPigeonHole*) _hole_queue->Remove(func, param);
}

/* Wait for a message with ID msg_name from connection "conn" forever */
IPMessage* IPCommunicator::WaitForMsg(IPConnection* conn, int msg_num)
{
    if (!conn)
        return NULL;

    MsgFilter filter(LookupMessage(msg_num), conn);

    while (1) {
        IPMessage* msg = ReceiveMessage(&filter, IPT_BLOCK);
        if (msg)
            return msg;
    }
}

/* Reset the module name to "name" */
void IPCommunicator::ModuleName(const char* name)
{
    if (_mod_name)
        delete _mod_name;
    int len = strlen(name) + 1;
    _mod_name = new char[len];
    bcopy(name, _mod_name, len);
}

IPTimer* IPCommunicator::AddTimer(double interval, IPTimerCallback* timer)
{
    IPTimer* res = new IPTimer(interval, timer, 0);
    _timer_table->Add(res);
    return res;
}

IPTimer* IPCommunicator::AddOneShot(double interval, IPTimerCallback* timer)
{
    IPTimer* res = new IPTimer(interval, timer, 1);
    _timer_table->Add(res);
    return res;
}

void IPCommunicator::RemoveTimer(IPTimer* timer)
{
    _timer_table->Remove(timer);
}

/* Add a new connection callback.  The callback "cb" will be invoked
   when any module connects to this module */
void IPCommunicator::AddConnectCallback(IPConnectionCallback* cb)
{
    _connect_callbacks->Append((char*) cb);
    IPResource::ref(cb);
}

void IPCommunicator::AddConnectCallback(void (*func)(IPConnection*, void*),
                                        void* data)
{
    AddConnectCallback(new IPRoutineConnectionCallback(func, data));
}

/* Invoke all the connection callbacks on a connection */
void IPCommunicator::InvokeConnectCallbacks(IPConnection* res)
{
    IPConnectionCallback* cb =
        (IPConnectionCallback*) _connect_callbacks->First();

    res = LookupConnection(res->Name());
    if (!res)
        return;
    
    for(;cb;cb = (IPConnectionCallback*) _connect_callbacks->Next())
        cb->Execute(res);
}

void IPCommunicator::RemoveConnectCallback(IPConnectionCallback* cb)
{
    _connect_callbacks->Remove((char*) cb);
    IPResource::unref(cb);
}

/* Add a new disconnection callback.  The callback "cb" will be invoked
   when any module disconnects from this module */
void IPCommunicator::AddDisconnectCallback(IPConnectionCallback* cb)
{
    _disconnect_callbacks->Append((char*) cb);
    IPResource::ref(cb);
}

void IPCommunicator::AddDisconnectCallback(void (*func)(IPConnection*, void*),
                                           void* data)
{
    AddDisconnectCallback(new IPRoutineConnectionCallback(func, data));
}

/* Invoke all the disconnection callbacks on a connection */
void IPCommunicator::InvokeDisconnectCallbacks(IPConnection* res)
{
    if (!_disconn_handlers_active)
        return;
    
    IPConnectionCallback* cb =
        (IPConnectionCallback*) _disconnect_callbacks->First();

    res = LookupConnection(res->Name());
    if (!res)
        return;

    for(;cb;cb = (IPConnectionCallback*) _disconnect_callbacks->Next())
        cb->Execute(res);
}

void IPCommunicator::RemoveDisconnectCallback(IPConnectionCallback* cb)
{
    _disconnect_callbacks->Remove((char*) cb);
    IPResource::unref(cb);
}

/* This is an iterator used to update the message types of any unregistered
   messages that might have been received and which are languishing in the
   unhandled message queue */
static int update_msg_type(char* param, char* item)
{
    IPMessageType* type = (IPMessageType*) param;
    IPMessage* msg = (IPMessage*) item;

    if (!msg->Type() && msg->ID() == type->ID())
        msg->EncodeType(msg->Connection()->Communicator());

    return 1;
}

/* Register a message with name "name" and format "format_string".  This format
   could be NULL to indicate an unformatted message */
IPMessageType* IPCommunicator::RegisterMessage(const char* name, 
                                               const char* format_string)
{
    IPMessageType* res = LookupMessage(name);
    if (res) {
        if (format_string) {
            if (!res->Formatter())
                res->SetFormat(_parser->Parse(format_string));
            else
                printf("Warning: Duplicate message type definitions for %s\n",
                       name);
        }
        return res;
    }
    
    int id = get_message_id(name);
    if (id < 0)
        return NULL;

    IPMessageType* type = register_message(id, name, format_string);
    _message_queue->Iterate(update_msg_type, (char*) type);

    return type;
}

IPFormat* IPCommunicator::RegisterNamedFormatter(const char* name,
                                                 const char* format)
{
    IPFormat* fmt = _parser->Parse(format);
    _parser->addFormatToTable(name, fmt);

    return fmt;
}

void IPCommunicator::RegisterNamedFormatters(IPFormatSpec* specs)
{
    int i;
    for (i=0;specs[i].name;i++) 
        RegisterNamedFormatter(specs[i].name, specs[i].format);
}

/* Register a set of messages specified by the name/format array "specs".
   The final element of this array should be {NULL, NULL} */
void IPCommunicator::RegisterMessages(IPMessageSpec* specs)
{
    int i;
    for (i=0;specs[i].name; i++);
    int n = i;

    if (n==0)
        return;

    int state = HandlersActive();  // We don't want to handle any messages 
    if (state)                     // while registering messages themselves
        DisableHandlers();

    char** names = new char*[n];
    for (i=0;i<n;i++) 
        names[i] = specs[i].name;
    int* ids = register_messages(n, names);
    if (!ids)
        return;
    IPMessageType* type;
    for (i=0;i<n;i++) {
        type = LookupMessage(ids[i]);
        if (!type) {
            type = register_message(ids[i], specs[i].name, specs[i].format);
            _message_queue->Iterate(update_msg_type, (char*) type);
        } else {
            if (specs[i].format) {
                if (!type->Formatter())
                    type->SetFormat(_parser->Parse(specs[i].format));
                else
                    printf("Warning: Duplicate message type definitions for %s\n",
                           specs[i].name);
            } 
        }
    }

    // ENDO - MEM LEAK
    //delete ids;
    //delete names;
    delete [] ids;
    delete [] names;

    if (state)
        EnableHandlers();
}

/* Sleep for "timeout" seconds, responding to handlers.  Returns 1 if any
   message process was done during the sleep, and 0 if not */
int IPCommunicator::Sleep(double timeout)
{
    if (timeout == IPT_BLOCK) {
        MainLoop();
        return 0;
    }

    IPTime start = IPTime::Current();
    int res;
    
    while (1) {
        if (Finished())
            return 0;

        res = ScanForMessages(timeout);

        IPTime now = IPTime::Current();
        timeout -= (now-start).Value();
        if (timeout <= 0) 
            break;
        start = now;
    } 

    return res;
}

/* Sleep for "timeout" seconds, or until the first message processing is done.
   Returns 1 if any message processing was done during the idle, 0 if not */
int IPCommunicator::Idle(double timeout)
{
    IPTime start = IPTime::Current();
    int res;
    
    while (1) {
        res = ScanForMessages(timeout);

        if (res)
            return res;
        if (_finished)
            return 0;

        if (timeout != IPT_BLOCK) {
            IPTime now = IPTime::Current();
            timeout -= (now-start).Value();
            if (timeout <= 0)
                break;
            start = now;
        }
    }

    return res;
}

/* Enable the handling of messages.  This routine looks through the unhandled
   message queues and invokes the handlers of any messages that have languished
   there */
void IPCommunicator::EnableHandlers()
{
    _handlers_active = 1;

    check_message_queues();
}

/* An iterator to check for handleable messages in the unhandled message 
   queue */
static int message_check_handlers(const char* param, const char* item)
{
    IPCommunicator* ipt = (IPCommunicator*) param;
    IPMessage* msg = (IPMessage*) item;

    IPMessageType* type = msg->Type();
    if (type && type->Handler() && type->HandlerActive()) {
        ipt->AddEvent(new HandlerInvokedEvent(msg, type->Handler()));
        return 1;
    }

    return 0;
}

/* An iterator to check for handleable pigeonholed messages in the unhandled 
   pigeonholed message queue */
static int ph_check_handlers(const char* param, const char* item)
{
    IPCommunicator* ipt = (IPCommunicator*) param;
    IPPigeonHole* ph = (IPPigeonHole*) item;

    IPMessageType* type = ph->Type();
    if (type && type->Handler() && type->HandlerActive()) {
        ipt->AddEvent(new PigeonHoleEvent(ph));
        return 1;
    }

    return 0;
}

/* Check the unhandled message and pigeonholed message queues for messages that
   are now handleable.  For each of those messages, remove them from the queue
   and notify the event queue to handle them */
void IPCommunicator::check_message_queues()

{
    if (!_handlers_active)
        return;

    remove_message(message_check_handlers, (char*) this, 1);
    remove_hole(ph_check_handlers, (char*) this, 1);
}

/* Disable the handling of a message type */    
void IPCommunicator::DisableHandler(IPMessageType* type)
{
    type->DisableHandler();
}

/* Enable the handling of a message type, and make sure that there are no
   handlable messages in the unhandled message queues */
void IPCommunicator::EnableHandler(IPMessageType* type)
{
    type->EnableHandler();
    check_message_queues();
}

void IPCommunicator::DisableHandler(const char* msg_name)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Cannot disable unknown message type %s\n", msg_name);
        return;
    }

    DisableHandler(type);
}

void IPCommunicator::EnableHandler(const char* msg_name)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Cannot enable unknown message type %s\n", msg_name);
        return;
    }

    EnableHandler(type);
}

/* Iterator to be used when a message type is declared a pigeon hole,
   it processes any mesasges to be pigeon holed that are hanging around
   in the unhandled message queue */
static int clean_up_pigeons(const char* param, const char* item)
{
    IPMessage* msg = (IPMessage*) item;
    IPMessageType* type = (IPMessageType*) param;

    if (type && type == msg->Type() && type->Destination()) {
        type->Destination()->ProcessMessage(msg->Connection(), msg);
        return 1;
    }

    return 0;
}

/* Declare a type as a pigeon holed message type.  Makes sure to clean
   up the unhandled message queue */
void IPCommunicator::PigeonHole(IPMessageType* msg_type)
{
    IPDestination* dest = new PigeonHoleDestination();
    msg_type->Destination(dest);

    if (msg_type->Destination())
        remove_message(clean_up_pigeons, (char*) msg_type, 1);
}

/* Convenience function to pigeon hole on a message name */
void IPCommunicator::PigeonHole(const char* msg_name)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (type) 
        PigeonHole(type);

}

/* Queries through connection "conn" with a message "out" and waits for
   "timeout" seconds for a message of type "type" with the same instance
   number as "out" */
IPMessage* IPCommunicator::Query(IPConnection* conn, IPMessage* out,
                                 IPMessageType* type,
                                 double timeout)
{
    if (!conn || !type) {
        ipt_error("Invalid parameters for Query, doing nothing");
        return NULL;
    }
    if (!conn->Active()) {
        ipt_error("Querying to inactive connection, doing nothing");
        return NULL;
    }

    pthread_mutex_lock(&sendMutex); ///

    if (_logging) 
        LogSend(conn, out);
    
    conn->Send(out);

    int state = 0;
    if (type->Handler()) {
        state = type->HandlerActive();
        if (state)
            DisableHandler(type);
    }
    InstanceFilter f(out->Instance(), conn, type);
    IPMessage* msg = ReceiveMessage(&f, timeout);
    if (state)
        EnableHandler(type);

    pthread_mutex_unlock(&sendMutex); ///

    return msg;
}

/* Convenience function that creates the query from "out_type" and formatted
   data "out_data" */
IPMessage* IPCommunicator::Query(IPConnection* conn, IPMessageType* out_type,
                                 void* out_data, IPMessageType* in_type,
                                 double timeout)
{
    if (!out_type) {
        ipt_error("Invalid type for query");
        return NULL;
    }
    
    IPMessage out(out_type, generate_instance_num(), out_data);

    return Query(conn, &out, in_type, timeout);
}

/* Convenience function that creates the query from "out_type" and unformatted
   data "out" of size "size_data" */
IPMessage* IPCommunicator::Query(IPConnection* conn, IPMessageType* out_type,
                                 int size_data, unsigned char* data,
                                 IPMessageType* in_type, double timeout)
{
    if (!out_type) {
        ipt_error("Invalid type for query");
        return NULL;
    }
    
    IPMessage out(out_type, generate_instance_num(), size_data, data);

    return Query(conn, &out, in_type, timeout);
}

/* Convenience function that creates the query from the message type ID
   "message_num" and the formatted data "out_data" and waits for the message
   type with ID "in_num" for "timeout" seconds */
IPMessage* IPCommunicator::Query(IPConnection* conn, int message_num,
                                 void* out_data, int in_num, double timeout)
{
    IPMessageType* type = LookupMessage(message_num);
    if (!type) {
        ipt_error("Invalid type for query");
        return NULL;
    }
    
    IPMessage out(type, generate_instance_num(), out_data);

    IPMessageType* in_type = LookupMessage(in_num);
    if (!in_type) {
        ipt_error("Invalid type for query");
        return NULL;
    }
    
    return Query(conn, &out, in_type, timeout);
}

IPMessage* IPCommunicator::Query(IPConnection* conn, const char* query_name,
                                 int size, unsigned char* data,
                                 const char* reply_name, double timeout)
{
    IPMessageType* query_type = LookupMessage(query_name);
    if (!query_type) {
        ipt_error("Unknown message type for query");
        return NULL;
    }
    IPMessageType* reply_type = LookupMessage(reply_name);
    if (!reply_type) {
        ipt_error("Unknown message type for query reply");
        return NULL;
    }

    return Query(conn, query_type, size, data, reply_type, timeout);
}

IPMessage* IPCommunicator::Query(IPConnection* conn, const char* query_name,
                                 void* data,
                                 const char* reply_name, double timeout)
{
    IPMessageType* query_type = LookupMessage(query_name);
    if (!query_type) {
        ipt_error("Unknown message type for query");
        return NULL;
    }
    IPMessageType* reply_type = LookupMessage(reply_name);
    if (!reply_type) {
        ipt_error("Unknown message type for query reply");
        return NULL;
    }

    return Query(conn, query_type, data, reply_type, timeout);
}

/* Query through connection "conn" with a message named "query_msg_name"
   and formatted data "query_data" and wait for 
   a message named "recv_msg_name".  Return the formatted data or void
   if the time ran out or there was an error */
void* IPCommunicator::QueryFormatted(IPConnection* conn,
                                     const char* query_msg_name,
                                     void* query_data,
                                     const char* recv_msg_name,
                                     double timeout)
{
    IPMessageType* query_type = LookupMessage(query_msg_name);
    if (!query_type) {
        printf("IPT: Invalid message name in query: %s\n", query_msg_name);
        return NULL;
    }

    IPMessageType* recv_type = LookupMessage(recv_msg_name);
    if (!query_type) {
        printf("IPT: Invalid message name in query: %s\n", recv_msg_name);
	return NULL;
    }
    

    IPMessage* msg =
        Query(conn, query_type, query_data, recv_type, timeout);

    if (!msg)
	return NULL;
    

    void* res = msg->FormattedData();
    delete msg;
    
    return res;
}

/* Query through connection "conn" with a message named "query_msg_name"
   and formatted data "query_data" and wait for 
   a message named "recv_msg_name".  Return the formatted data or void
   if the time ran out or there was an error */
int IPCommunicator::QueryFormatted(IPConnection* conn,
                                   const char* query_msg_name,void* query_data,
                                   const char* recv_msg_name, void* reply_data,
                                   double timeout)
{
    IPMessageType* query_type = LookupMessage(query_msg_name);
    if (!query_type) {
        printf("IPT: Invalid message name in query: %s\n", query_msg_name);
        return 0;
    }

    IPMessageType* recv_type = LookupMessage(recv_msg_name);
    if (!query_type) {
        printf("IPT: Invalid message name in query: %s\n", recv_msg_name);
        return 0;
    }
    

    IPMessage* msg = Query(conn, query_type, query_data, recv_type, timeout);

    if (!msg)
        return 0;

    msg->FormattedData(reply_data, 1);
    delete msg;

    return 1;
}

/* Replies to a message "msg" by sending a message of type "type" with
   formatted data "data" to "msg"'s connection with "msg"'s instance
   number */
void IPCommunicator::Reply(IPMessage* msg, IPMessageType* type, void* data)
{
    if (!type) {
        ipt_error("Invalid type for reply");
        return;
    }
    
    IPMessage out(type, msg->Instance(), data);

    if (_logging) 
        LogSend(msg->Connection(), &out);
    
    msg->Connection()->Send(&out);
}

/* Replies to a message "msg" by sending a message with type ID "id" with
   formatted data "data" to "msg"'s connection with "msg"'s instance
   number */
void IPCommunicator::Reply(IPMessage* msg, int id, void* data)
{
    IPMessageType* type = LookupMessage(id);
    if (!type) {
        ipt_error("Replying with invalid type");
        return;
    }
    IPMessage out(type, msg->Instance(), data);

    msg->Connection()->Send(&out);
}

/* Replies to a message "msg" by sending a message of type "type" with
   unformatted data "data" (of size "size") to "msg"'s connection with "msg"'s
   instance number */
void IPCommunicator::Reply(IPMessage* msg, IPMessageType* type, int size,
                           unsigned char* data)
{
    if (!type) {
        ipt_error("Replying with invalid type");
        return;
    }
    
    IPMessage out(type, msg->Instance(), size, data);

    msg->Connection()->Send(&out);
}

/* convenience function for replying by message name with formatted data */
void IPCommunicator::Reply(IPMessage* msg, const char* msg_name, void* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't reply with unknown message %s\n", msg_name);
        return;
    }

    Reply(msg, type, data);
}

/* convenience function for replying by message name with unformatted data */
void IPCommunicator::Reply(IPMessage* msg, const char* msg_name,
                           int size, unsigned char* data)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("IPT: Can't reply with unknown message %s\n", msg_name);
        return;
    }

    Reply(msg, type, size, data);
}

/* Renames connection "conn" to "new_name" */
void IPCommunicator::Rename(IPConnection* conn, const char* new_name)
{
    if (!strcmp(conn->Name(), new_name))
        return;

    _connection_table->Remove(conn->Name());
    conn->SetName(new_name);
    _connection_table->Insert(conn->Name(), strlen(conn->Name())+1,
                              (char*) conn);
}

/* Executes the handler for "msg", if any.  If the handler is already invoked,
   postpone it until after the handling is finished */
int IPCommunicator::Execute(IPMessage* msg)
{
    IPMessageType* type = msg->Type();
    if (!type || !type->Handler() || type->HandlerContext() == IPT_HNDL_NONE ||
        msg->Connection()->ByteOrder() == BYTE_ORDER_UNDEFINED) {
        return -1; 
    }
    
    if (type->HandlerContext() == IPT_HNDL_STD && type->HandlerInvoked()) {
        type->Postpone(msg);
        return 1;
    } else {
        type->HandlerInvoked(1);
        type->Handler()->Execute(msg);
        type->HandlerInvoked(0);

        if (msg->formatted_data_store() && msg->formatted_contents()) {
            printf("IPT: Handler for %s used formatted contents and did not clean up\n", msg->Type()->Name());
            printf("IPT: When unpacking just the contents of a message you must either\n");
            printf("IPT:   1) Clean up using either the DeleteContents member function\n");
            printf("IPT:      or the iptMessageDeleteContents cover function\n");
            printf("IPT:   2) Force copying of the contents (either using the optional argument to\n");
            printf("IPT:      FormattedData or the iptMessageFormContentsCopy, and once again,\n");
            printf("IPT:      then you are responsible for freeing the memory\n");
        }

        while ((msg = type->Postponed()))
            AddEvent(new HandlerInvokedEvent(msg, type->Handler()));

        return 0;
    }
}

/* Delete formatted data "formatted" according to the formatter of the message
   named "msg_name" */
void IPCommunicator::DeleteFormatted(const char* msg_name, void* formatted)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("Unknown message name, cannot delete formatted data\n");
        return;
    }

    if (formatted) 
        type->DeleteFormattedData(formatted, NULL, 0);
}

/* Delete the contentes of formatted data "formatted" according to the
   formatter of the message named "msg_name" */
void IPCommunicator::DeleteContents(const char* msg_name, void* formatted)
{
    IPMessageType* type = LookupMessage(msg_name);
    if (!type) {
        printf("Unknown message name, cannot delete formatted data\n");
        return;
    }

    if (formatted)
        type->DeleteContents(formatted, NULL, 0);
}

/* Log the sending of message "msg" to connection "conn" to the standard out */
void IPCommunicator::LogSend(IPConnection* conn, IPMessage* msg)
{
    if (msg->Type()) 
        printf("%s: Sent %s (ID %d, instance %d, size %d) to %s\n",
               ModuleName(), msg->Type()->Name(), msg->ID(),
               msg->Instance(), msg->SizeData(), conn->Name());
    else
        printf("%s: Sent untyped message (ID %d, instance %d, size %d) to %s\n",
               ModuleName(), msg->ID(), msg->Instance(),
               msg->SizeData(), conn->Name());
}

/* Log reception of message "msg" from connection "conn" to the standard out */
void IPCommunicator::LogReceive(IPConnection* conn, IPMessage* msg,
                             const char* extra)
{
    if (msg->Type()) 
        printf("%s: Received %s (ID %d, instance %d, size %d)",
               ModuleName(), msg->Type()->Name(), msg->ID(),
               msg->Instance(), msg->SizeData());
    else
        printf("%s: Received untyped message (ID %d, instance %d, size %d)",
               ModuleName(), msg->ID(), msg->Instance(),
               msg->SizeData());

    if (conn)
        printf(" from %s", conn->Name());
    if (extra)
        printf(" (%s)\n", extra);
    else
        printf("\n");
}

void IPCommunicator::IterateConnections(IPConnectionCallback* cb)
{
    IPConnection* conn =
        (IPConnection*) AdministrativeConnections()->First();
    for (; conn; conn = (IPConnection*) AdministrativeConnections()->Next())
        cb->Execute(conn);

    conn =
        (IPConnection*) Connections()->First();
    for (; conn; conn = (IPConnection*) Connections()->Next())
        cb->Execute(conn);
}    

IPConnection* IPCommunicator::
AddAdministrativeConnection(int fd,
                            IPConnectionCallback* activity_cb,
                            IPConnectionCallback* disconnect_cb)
{
    IPConnection* conn = new FakeConnection("Administrative", this, fd);
    if (activity_cb)
        conn->AddConnectCallback(activity_cb);
    if (disconnect_cb)
        conn->AddDisconnectCallback(disconnect_cb);

    _administrative_list->Prepend((char*) conn);

    return conn;
}

static int remove_by_fd(const char* param, const char* item)
{
    IPConnection* conn = (IPConnection*) item;
    int fd = (long) param;

    return (fd == conn->FD());
}

void IPCommunicator::RemoveAdministrativeConnection(int fd)
{
    IPConnection* removed = (IPConnection*)
        _administrative_list->Remove(remove_by_fd, (const char*) fd);

    if (removed)
        delete removed;
}
            
int IPCommunicator::printf(const char* fmt ...)
{
    va_list ap;
    va_start(ap, fmt);
    int res = vfprintf(_out_fp, fmt, ap);
    va_end(ap);

    return res;
}

int IPCommunicator::add_translator(int id, IPConnection*)
{
    return id;
}

void IPCommunicator::connect_notify(IPConnection* conn)
{
    conn = LookupConnection(conn->Name());
    AddEvent(new ConnectionEvent(conn));
}

void IPCommunicator::disconnect_notify(IPConnection* conn)
{
    conn = LookupConnection(conn->Name());
    AddEvent(new DisconnectionEvent(conn));
}

void IPCommunicator::initializeThreads(int level)
{
    if (_parallelism_level > level)
        return;
    
    IPList::initThreads();
    IPResource::initThreads();
}

void IPCommunicator::SetStartBlockingAction(void (*cb)(void*), void* data)
{
    _start_blocking_action = cb;
    _start_blocking_data = data;
}

void IPCommunicator::StartBlocking()
{
    if (_start_blocking_action)
        (*_start_blocking_action)(_start_blocking_data);
}

void IPCommunicator::SetEndBlockingAction(void (*cb)(void*), void* data)
{
    _end_blocking_action = cb;
    _end_blocking_data = data;
}

void IPCommunicator::EndBlocking()
{
    if (_end_blocking_action)
        (*_end_blocking_action)(_end_blocking_data);
}

const char* IPCommunicator::ThisHost() const 
{
    return _this_host; 
}

IPTranslator::IPTranslator(const char* domain_name)
{
    int len = strlen(domain_name)+1;
    _domain_name = new char[len];
    bcopy(domain_name, _domain_name, len);
    _table = new IPHashTable(200, int_hash, int_eq);
}

IPTranslator::~IPTranslator()
{
    delete _domain_name;
    delete _table;
}

int IPTranslator::translation(int id)
{
    return (long) _table->Find((char*) &id);
}

void IPTranslator::make_translation(int id, int translated)
{
    _table->Remove((char*) &id);
    _table->Insert((char*) &id, sizeof(int), (char*) translated);
}

void IPTranslator::clear_translations()
{
    delete _table;
    _table = new IPHashTable(200, int_hash, int_eq);
}    

