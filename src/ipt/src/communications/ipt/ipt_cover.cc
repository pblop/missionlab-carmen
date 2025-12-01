///////////////////////////////////////////////////////////////////////////////
//
//                                 ipt_cover.cc
//
// This file implements routines that allow C programs to use IPT
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

#ifdef VXWORKS
#include <taskLib.h>
#include <taskVarLib.h>
#else
#include "./libc.h"
#endif

#include <ipt/ipt.h>
#include <ipt/message.h>
#include <ipt/callbacks.h>
#include <ipt/connection.h>
#include <ipt/messagetype.h>
#include <ipt/publish.h>
#include <ipt/format.h>
#include <ipt/parseformat.h>
#include <ipt/list.h>
#include <ipt/timer.h>

/* the IPT communicator global used by the cover functions (IPT and TCX
   C cover functions */
IPCommunicator* _ip_communicator = NULL;

/* Callback sub-class used with cover function message handlers */
class OldCHandlerCallback : public IPHandlerCallback {
  public:
    OldCHandlerCallback(void (*func)(IPMessage*, void*),
                     void* param = 0)
        { _func = func; _param = param; }

    virtual void Execute(IPMessage* m)
        { (*_func)(m, _param); }

  private:
    void (*_func)(IPMessage*, void*);
    void* _param;
};

/* Callback sub-class used with cover function message handlers */
class CHandlerCallback : public IPHandlerCallback {
  public:
    CHandlerCallback(void (*func)(IPCommunicator*, IPMessage*, void*),
                     void* param = 0)
        { _func = func; _param = param; }

    virtual void Execute(IPMessage* m)
        { (*_func)(m->Connection()->Communicator(), m, _param); }

  private:
    void (*_func)(IPCommunicator*, IPMessage*, void*);
    void* _param;
};

/* Callback sub-class used with cover function connect/disconnect handlers  */
class OldCConnectionCallback : public IPConnectionCallback {
  public:
    OldCConnectionCallback(void (*func)(IPConnection*, void*),
                           void* data)
        { _func = func; _data = data; }

    virtual void Execute(IPConnection* conn)
        { (*_func)(conn, _data); }

  private:
    void (*_func)(IPConnection*, void*);
    void* _data;
};

/* Callback sub-class used with cover function connect/disconnect handlers  */
class CConnectionCallback : public IPConnectionCallback {
  public:
    CConnectionCallback(void (*func)(IPCommunicator*, IPConnection*, void*),
                        void* data)
        { _func = func; _data = data; }

    virtual void Execute(IPConnection* conn)
        { (*_func)(conn->Communicator(), conn, _data); }

  private:
    void (*_func)(IPCommunicator*, IPConnection*, void*);
    void* _data;
};

class C_Client : public IPClient {
  public:
    C_Client(IPConnection* conn, IPFormat* fmt,
             int size, unsigned char* info,
             int (*check)(void*, void*, void*),
             void* (*publish)(IPCommunicator*, IPMessage*, IPConnection*,
                              void*, void*, void*),
             void (*del)(void*),
             void* pub_data);
    virtual ~C_Client();

    virtual int Check();
    virtual int SendMessage(IPCommunicator* comm, IPMessage* msg);

  private:
    IPFormat* _format;
    void* _subscriber_data;
    int (*_check)(void*, void*, void*);
    void* (*_publish)(IPCommunicator*, IPMessage*, IPConnection*,
                     void*, void*, void*);
    void (*_delete)(void*);
    void* _publisher_data;
    void* _client_data;
};

class C_Publication : public IPPublication {
  public:
    C_Publication(IPConnectionCallback* cb,
                  IPFormat* fmt,
                  int (*check)(void*, void*, void*),
                  void* (*publish)(IPCommunicator*, IPMessage*, IPConnection*,
                                   void*, void*, void*),
                  void (*del)(void*),
                  void* data);

    virtual int AddSubscriber(IPConnection* conn,
                              int=0, unsigned char* = NULL);

  private:
    IPFormat* _format;
    void* _subscriber_data;
    int (*_check)(void*, void*, void*);
    void* (*_publish)(IPCommunicator*,
                      IPMessage*, IPConnection*, void*, void*, void*);
    void (*_delete)(void*);
    void* _data;
};

C_Publication::C_Publication(IPConnectionCallback* cb,
                             IPFormat* fmt,
                             int (*check)(void*, void*, void*),
                             void* (*publish)(IPCommunicator*,
                                              IPMessage*, IPConnection*,
                                              void*, void*, void*),
                             void (*del)(void*),
                             void* data) : IPPublication(cb)
{
    _format = fmt;
    _check = check;
    _publish = publish;
    _delete = del;
    _data = data;
}

int C_Publication::AddSubscriber(IPConnection* conn,
                                 int size, unsigned char* info)
{
    return add_subscriber(new C_Client(conn, _format, size, info,
                                       _check, _publish, _delete, _data));
}

C_Client::C_Client(IPConnection* conn, IPFormat* fmt,
                   int size_info, unsigned char* info,
                   int (*check)(void*, void*, void*),
                   void* (*publish)(IPCommunicator*, IPMessage*, IPConnection*,
                                    void*, void*, void*),
                   void (*del)(void*),
                   void* pub_data) : IPClient(conn)
{
    _format = fmt;
    if (fmt && size_info)
        _subscriber_data = _format->decodeData((char*) info, 0, 0, NULL,
                                               conn->ByteOrder(),
                                               conn->Alignment());
    else
        _subscriber_data = NULL;

    _check = check;
    _publish = publish;
    _delete = del;
    _publisher_data = pub_data;
    _client_data = NULL;
}

C_Client::~C_Client()
{
    if (_subscriber_data && _format)
        _format->freeDataStructure(_subscriber_data, NULL, 0);
    if (_delete && _client_data)
        (*_delete)(_client_data);
}


int C_Client::Check()
{
    if (_check)
        return (*_check)(_subscriber_data, _client_data, _publisher_data);
    else
        return 1;
}

int C_Client::SendMessage(IPCommunicator* comm, IPMessage* msg)
{
    if (_publish)
        _client_data = (*_publish)(comm, msg, Connection(), _subscriber_data,
                                   _client_data, _publisher_data);

    return IPClient::SendMessage(comm, msg);
}

extern "C" {

/* Base IPT cover functions */

/* Initialize IPT client with name "mod_name" and server on machine "host_name"
   If "host_name" is NULL, look at the environment variable "IPTHOST", if
   that is NULL, look at the current machine */
void IPTinitialize(char* mod_name, char* host_name)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
            printErr("Can't taskVarAdd LispMaster\n");
            taskSuspend(0);
        }
#endif
        _ip_communicator = IPCommunicator::Instance(mod_name, host_name);
    } else
        _ip_communicator->printf("IPT already initialized, ignoring\n");
}

IPCommunicator* iptCommunicatorInstance(char* mod_name, char* host_name)
{
    return IPCommunicator::Instance(mod_name, host_name);
}

void iptDeleteCommunicator(IPCommunicator* comm)
{
    delete comm;
}

/* Initialize the IPT cover functions with a given communicator, "comm" */
void IPTinitialize_with_communicator(IPCommunicator* comm)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
            printErr("Can't taskVarAdd LispMaster\n");
            taskSuspend(0);
        }
#endif
        _ip_communicator = comm;
    } else
        _ip_communicator->printf("IPT already initialized, ignoring\n");
}

#ifdef VXWORKS
/* This routine is for VxWorks only.  It initializes the cover communicator
   to be the same as the cover communicator for another task named 
   "task_name" */
void IPTinitialize_inherited(char* task_name)
{
    if (_ip_communicator) {
        _ip_communicator->printf("IPT(C): already initialized\n");
        return;
    }
    
    int tid = taskNameToId(task_name);
    if (tid == ERROR) {
        _ip_communicator->printf("IPT(C): Error inheriting TCX variables from %s\n",
                task_name);
        return;
    }
    printf("IPT(C): Inheriting TCX communicator from %s (%x)\n",
           task_name, tid);

    if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
        printErr("Can't taskVarAdd\n");
        taskSuspend(0);
    }
    
    _ip_communicator = (IPCommunicator*) taskVarGet(tid, (int*) &_ip_communicator);
}

/* This routine is for VxWorks only.  It cleans up after the inheritance
   process initialited by IPTinitialize_inherited */
void IPTdeinitialize_inherited()
{
    if (_ip_communicator) {
        _ip_communicator = NULL;
        taskVarDelete(0, (int*) &_ip_communicator);
    }
}

#endif

/* Get the IPT cover communicator */
IPCommunicator* IPTcommunicator()
{
    return _ip_communicator;
}

/* Register a message with a name and a format (which can be NULL) */
IPMessageType* iptRegisterMessage(IPCommunicator* comm,
                                  char* name, char* format)
{
    return comm->RegisterMessage(name, format);
}

/* Register an array of name/format pairs in "messages" */
void iptRegisterMessages(IPCommunicator* comm, IPMessageSpec* messages)
{
    comm->RegisterMessages(messages);
}

IPFormat* iptRegisterNamedFormatter(IPCommunicator* comm,
                                    char* name, char* format)
{
    return comm->RegisterNamedFormatter(name, format);
}

void iptRegisterNamedFormatters(IPCommunicator* comm, IPFormatSpec* specs)
{
    comm->RegisterNamedFormatters(specs);
}

/* Register a handler for message "msg_name" with callback routine "func"
   context specifier "spec" (which is ignored for now) and parameter "data" */
void iptRegisterHandler(IPCommunicator* comm, char* msg_name,
                        void (*func)(IPCommunicator*, IPMessage*, void*),
                        int spec, void* data)
{
    IPHandlerCallback* callback = new CHandlerCallback(func, data);
    comm->RegisterHandler(msg_name, callback, spec);
}

/* Register a handler for message "msg_name" with callback routine "func"
   context specifier "spec" (which is ignored for now) and parameter "data" */
void IPTregister_handler(char* msg_name,
                         void (*func)(IPMessage*, void*),
                         int spec, void* data)
{
    IPHandlerCallback* callback = new OldCHandlerCallback(func, data);
    _ip_communicator->RegisterHandler(msg_name, callback, spec);
}

/* Connect to module "name" and return a connection pointer to the new 
   connection to the module.  If "required" is 1, then block until module
   connects.  If "required" is 0, then if the module does not already exist
   on the system according to the server, return a connection pointer that is
   not active */
IPConnection* iptConnect(IPCommunicator* comm, char* name, int required)
{
    return comm->Connect(name, required);
}

/* Lookup message named "name" and return the type, or NULL if no message with
   that name is registered */
IPMessageType* iptLookupMessage(IPCommunicator* comm, char* name)
{
    return comm->LookupMessage(name);
}

/* Lookup message with the ID "id" and return the type, or NULL if no message
   with that ID is registered */
IPMessageType* iptMessageById(IPCommunicator* comm, int id)
{
    return comm->LookupMessage(id);
}

/* Receive a message from connection "conn" and name "msg_name" blocking
   for "timeout" seconds.  If conn is NULL, any connection will do, if
   msg_name is NULL, any message type will do.  If timeout is IPT_BLOCK (-1)
   block forever.  If no message is received within the alloted time or there
   is an error, return NULL, otherwise return the message that has been
   received succesfully */
IPMessage* iptReceiveMsg(IPCommunicator* comm,
                         IPConnection* conn, char* msg_name, double timeout)
{
    if (conn)
        comm = conn->Communicator();
    return comm->ReceiveMessage(conn, msg_name, timeout);
}

/* Lookup the connection to the module named "conn_name", return NULL if
   there is none */
IPConnection* iptConnection(IPCommunicator* comm, char* conn_name)
{
    return comm->LookupConnection(conn_name);
}

/* Cause the message type named "name" to be a pigeon holed message type */
void iptPigeonHole(IPCommunicator* comm, char* name)
{
    comm->PigeonHole(name);
}

/* Cause the message type "type" to be a pigeon holed message type */
void iptPigeonHoleType(IPCommunicator* comm, IPMessageType* type)
{
    comm->PigeonHole(type);
}

/* Add the connection callback routine "func" with parameter "data" */
void IPTadd_connect_callback(void (*func)(IPConnection*, void*),
                             void* data)
{
    _ip_communicator->AddConnectCallback(new OldCConnectionCallback(func,
                                                                    data));
}

/* Add the connection callback routine "func" with parameter "data" */
void iptAddConnectCallback(IPCommunicator* comm,
                           void (*func)(IPCommunicator*, IPConnection*, void*),
                           void* data)
{
    comm->AddConnectCallback(new CConnectionCallback(func, data));
}

/* Add the disconnection callback routine "func" with parameter "data" */
void IPTadd_disconnect_callback(void (*func)(IPConnection*, void*),
                                void* data)
{
    _ip_communicator->AddDisconnectCallback(new OldCConnectionCallback(func,
                                                                       data));
}

/* Add the disconnection callback routine "func" with parameter "data" */
void iptAddDisconnectCallback(IPCommunicator* comm,
                              void (*func)(IPCommunicator*, IPConnection*,
                                           void*),
                              void* data)
{
    comm->AddDisconnectCallback(new CConnectionCallback(func, data));
}

/* Return the name of the machine this client is on */
char* iptThisHost(IPCommunicator* comm)
{
    return (char*) comm->ThisHost();
}

/* Return the name of this module */
char* iptModuleName(IPCommunicator* comm)
{
    return (char*) comm->ModuleName();
}

/* Return the domain name of this client */
char* iptDomainName(IPCommunicator* comm)
{
    return (char*) comm->DomainName();
}

/* Disable all message handling */
void iptDisableHandlers(IPCommunicator* comm)
{
    comm->DisableHandlers();
}

/* Enable all message handling, checking to see if there are any pending
   messages that need to be handled */
void iptEnableHandlers(IPCommunicator* comm)
{
    comm->EnableHandlers();
}

/* Disable message handling for a particular type of message */
void iptDisableMsgHandler(IPCommunicator* comm, IPMessageType* type)
{
    comm->DisableHandler(type);
}

/* Enable message handling for a particular type of message, checking to see
   if there are any message of that type pending that need to be handled */
void iptEnableMsgHandler(IPCommunicator* comm, IPMessageType* type)
{
    comm->EnableHandler(type);
}

/* Query a module through connection "conn" with message named "query_msg_name"
   with formatted data "data" for a message of type "recv_msg_name" and
   return the message when it is received, or NULL if there has been an
   error */
IPMessage* iptQueryMsgForm(IPConnection* conn, char* query_msg_name,
                           void* data, char* recv_msg_name)
{
    return conn->Communicator()->Query(conn, query_msg_name, data,
                                       recv_msg_name);
}

/* Query a module through connection "conn" with message named "query_msg_name"
   with "size" bytes of unformatted data "data" for a message of type
   "recv_msg_name" and return the message when it is received, or NULL if there
   has been an error */
IPMessage* iptQueryMsgRaw(IPConnection* conn, char* query_msg_name,
                          int size, unsigned char* data,
                          char* recv_msg_name)
{
    return conn->Communicator()->Query(conn, query_msg_name, size, data,
                                       recv_msg_name);
}

/* Deinitialize IPT, deleting the internal communicator and thus closing all
   the connections gracefully */
void IPTdeinitialize()
{
    if (_ip_communicator) {
        delete _ip_communicator;
        _ip_communicator = NULL;
#ifdef VXWORKS
        taskVarDelete(0, (int*) &_ip_communicator);
#endif
    }
}

/* Do the IPT main loop, handling whatever comes our way */
void iptMainLoop(IPCommunicator* comm)
{
    comm->MainLoop();
}

/* Sleep for "time" seconds processing information as they come in.  This 
   routine will stay asleep for at least the alotted time no matter how much
   stuff happens. Returns 1 if any message processing was done and 0 if not */
int iptSleep(IPCommunicator* comm, double time)
{
    return comm->Sleep(time);
}

/* Sleep for "time" seconds processing information as they come in. As soon
   as any message processing is done return.  Returns 1 if any message 
   processing was done and 0 if not */
int iptIdle(IPCommunicator* comm, double time)
{
    return comm->Idle(time);
}

/* convenience cover functions with strings that avoid manipulating
   IPMessage's */

/* send a message through connection "conn" with name "msg_name" and 
   unformatted data of size "size" and contents "data" */
int iptSendRawMsg(IPConnection* conn, char* msg_name,
                  int size, unsigned char* data)
{
    return conn->Communicator()->SendMessage(conn, msg_name, size, data);
}

/* Receive a message named "msg_name" from connection "conn" and wait
   for "timeout" seconds (forever if IPT_BLOCK).  Up to "max_size" bytes of
   the unformatted data will be copied into "buffer".  The actual number
   of bytes copied will be returned.  If there were more than "max_size"
   bytes of data then max_size bytes of data will be copied and -1 will be
   returned */
int iptReceiveRawMsg(IPCommunicator* comm,
                     IPConnection* conn, char* msg_name,
                     int max_size, unsigned char* buffer, double timeout)
{
    IPMessageType* type;
    if (conn)
        comm = conn->Communicator();
    
    if (msg_name) {
        type = comm->LookupMessage(msg_name);
        if (!type) 
            comm->printf("IPT Warning: Receiving unknown message type %s\n",
                   msg_name);
    } else
        type = NULL;

    IPMessage* msg = comm->ReceiveMessage(conn, type, timeout);

    if (!msg)
        return 0;

    int size = msg->SizeData();
    unsigned char* data = msg->Data();
    int res;

    if (size < max_size) {
        bcopy((char*) data, (char*) buffer, size);
        res = size;
    } else {
        bcopy((char*) data, (char*) buffer, max_size);
        res = -1;
    }

    delete msg;

    return res;
}

/* Receive a message of name "msg_name" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
void* iptReceiveFormMsg(IPCommunicator* comm,
                        IPConnection* conn, char* msg_name,
                        double timeout)
{
    if (conn)
        comm = conn->Communicator();
    return comm->ReceiveFormatted(conn, msg_name, timeout);
}

/* Receive a message of name "msg_name" from connection "conn" waiting for
   "timeout" seconds (wait forever if IPT_BLOCK) and return the formatted
   data, or NULL if the time ran out or there was an error */
int iptReceiveFormMsgContents(IPCommunicator* comm,
                              IPConnection* conn, char* msg_name,
                              void* msg_data,
                              double timeout)
{
    if (conn)
        comm = conn->Communicator();
    return comm->ReceiveFormatted(conn, msg_name, msg_data,
                                              timeout);
}

/* Free the formatted data "formatted" according to the format in the message
   named "msg_name" */
void iptFreeData(IPCommunicator* comm, char* msg_name, void* formatted)
{
    comm->DeleteFormatted(msg_name, formatted);
}

void iptFreeContents(IPCommunicator* comm, char* msg_name, void* formatted)
{
    comm->DeleteContents(msg_name, formatted);
}

/* Send message named "msg_name" through connection "conn" with formatted
   data "data" */
int iptSendFormMsg(IPConnection* conn, char* msg_name, void* data)
{
    return conn->Communicator()->SendMessage(conn, msg_name, data);
}

/* Query through connection "conn" with a message named "query_msg_name"
   and formatted data "query_data" and wait for 
   a message named "recv_msg_name".  Return the formatted data or void
   if the time ran out or there was an error */
int iptQueryFormContents(IPConnection* conn,
                         char* query_msg_name, void* query_data,
                         char* recv_msg_name, void* reply_data)
{
    return conn->Communicator()->QueryFormatted(conn,
                                                query_msg_name, query_data,
                                                recv_msg_name, reply_data);
}

/* Query through connection "conn" with a message named "query_msg_name"
   and formatted data "query_data" and wait for 
   a message named "recv_msg_name".  Return the formatted data or void
   if the time ran out or there was an error */
void* iptQueryForm(IPConnection* conn, char* query_msg_name, void* query_data,
                   char* recv_msg_name)
{
    return conn->Communicator()->QueryFormatted(conn,
                                                query_msg_name, query_data,
                                                recv_msg_name);
}

/* Reply to message "m" with message named "reply_name" and formatted 
   data "reply_data */
void iptReplyForm(IPMessage* m, char* reply_name, void* reply_data)
{
    m->Connection()->Communicator()->Reply(m, reply_name, reply_data);
}

/* Reply to message "m" with message named "reply_name" and unformatted data
   of size "size" and contents "data" */
void iptReplyRaw(IPMessage* m, char* reply_name,
                 int size, unsigned char* data)
{
    m->Connection()->Communicator()->Reply(m, reply_name, size, data);
}

/* Cover functions for manipulating IPMessages */

/* Get the size of the unformatted data contained in "msg" */
int iptMessageSizeData(IPMessage* msg)
{
    return msg->SizeData();
}

/* Get the contents of the unformatted data contained in "msg" */
unsigned char* iptMessageData(IPMessage* msg)
{
    return msg->Data();
}

/* Get the instance number of "msg" */
int iptMessageInstance(IPMessage* msg)
{
    return msg->Instance();
}

/* Get the message ID of "msg" */
int iptMessageId(IPMessage* msg)
{
    return msg->ID();
}

/* Create formatted data from "msg" */
void* iptMessageFormData(IPMessage* msg)
{
    return msg->FormattedData();
}

/* Create a copy of formatted data from "msg" */
void* iptMessageFormCopy(IPMessage* msg)
{
    return msg->FormattedData(1);
}

/* Create formatted data and put it in "res" */
void iptMessageFormContents(IPMessage* msg, void* res)
{
    msg->FormattedData(res);
}

/* Create a copy of formatted data and put it in "res" */
void iptMessageFormContentsCopy(IPMessage* msg, void* res)
{
    msg->FormattedData(res, 1);
}

/* Delete formatted data "data" according to "msg"'s message type */
void iptMessageDeleteForm(IPMessage* msg, void* data)
{
    msg->DeleteFormatted(data);
}

/* Delete the contents of formatted data "data" (i.e., delete the elements
   but not the pointer "data" itself, used when iptMessageFormContents
   was used to generate "data" */
void iptMessageDeleteContents(IPMessage* msg, void* data)
{
    msg->DeleteContents(data);
}

/* Return the message type of "msg" */
IPMessageType* iptMessageType(IPMessage* msg)
{
    return msg->Type();
}

/* Return the message name of "msg" */
char* iptMessageName(IPMessage* msg)
{
    IPMessageType* type = msg->Type();
    if (!type)
        return NULL;

    return (char*) type->Name();
}

/* Return the connection that "msg" came through if it did come through one */
IPConnection* iptMessageConnection(IPMessage* msg)
{
    return msg->Connection();
}

void iptSetMessageConnection(IPMessage* msg, IPConnection* conn)
{
    msg->Connection(conn);
}

/* Delete message and its unformatted data */
void iptMessageDelete(IPMessage* msg)
{
    delete msg;
}

/* print a message header (and the data, optionally) */
void iptMessagePrint(IPMessage* msg, int print_data)
{
    msg->Print(print_data);
}

/* routines for dealying with connections */

/* Add a callback routine "func" and parameter "data" that gets called whenever
   connection "conn" becomes active */
void iptConnectionAddConnectCallback(IPConnection* conn,
                                     void (*func)(IPCommunicator*,
                                                  IPConnection*, void*),
                                     void* data)
{
    if (conn)
        conn->AddConnectCallback(new CConnectionCallback(func, data));
}

/* Add a callback routine "func" and parameter "data" that gets called whenever
   connection "conn" becomes inactive */
void iptConnectionAddDisconnectCallback(IPConnection* conn,
                                        void (*func)(IPCommunicator*,
                                                     IPConnection*, void*),
                                        void* data)
{
    if (conn)
        conn->AddDisconnectCallback(new CConnectionCallback(func, data));
}

/* Add a callback routine "func" and parameter "data" that gets called whenever
   connection "conn" becomes active */
void IPTconnection_add_connect_callback(IPConnection* conn,
                                        void (*func)(IPConnection*, void*),
                                        void* data)
{
    if (conn)
        conn->AddConnectCallback(new OldCConnectionCallback(func, data));
}

/* Add a callback routine "func" and parameter "data" that gets called whenever
   connection "conn" becomes inactive */
void IPTconnection_add_disconnect_callback(IPConnection* conn,
                                           void (*func)(IPConnection*, void*),
                                           void* data)
{
    if (conn)
        conn->AddDisconnectCallback(new OldCConnectionCallback(func, data));
}

/* Get the file descriptor associated with connection "conn", -1 if there is
   no file descriptor associated */
int iptConnectionFd(IPConnection* conn)
{
    if (conn)
        return conn->FD();
    else
        return -1;
}

IPCommunicator* iptConnectionCommunicator(IPConnection* conn)
{
    if (conn)
        return conn->Communicator();
    else
        return NULL;
}

/* Return the name of the module connected to by "conn" */
char* iptConnectionName(IPConnection* conn)
{
    if (conn)
        return (char*) conn->Name();
    else
        return NULL;
}

/* Return the name of the machine on which the module connected to by "conn"
   is */
char* iptConnectionHost(IPConnection* conn)
{
    if (conn)
        return (char*) conn->Host();
    else
        return NULL;
}

/* Return 1 if connection "conn" is active, 0 if not */
int iptConnectionActive(IPConnection* conn)
{
    if (conn)
        return conn->Active();
    else
        return 0;
}

/* Duplicate message "msg" in "out" */
IPMessage* iptMessageDuplicate(IPMessage* msg, IPMessage* out)
{
    *out = *msg;

    return out;
}

/* Create a copy of message "msg" and return it */
IPMessage* iptMessageCopy(IPMessage* msg)
{
    return new IPMessage(*msg);
}

/* ipt extra functions */

void iptDeclareSubscription(IPCommunicator* comm, char* message_name,
                            void (*func) (IPCommunicator*,
                                          IPConnection*, void*),
                            void* data)
{
    IPConnectionCallback* cb = NULL;
    if (func) 
        cb = new CConnectionCallback(func, data);
    comm->DeclareSubscription(message_name, cb);
}

void iptDeclareTimedSubscription(IPCommunicator* comm, char* message_name,
                                 void (*func) (IPCommunicator*,
                                               IPConnection*, void*),
                                 void* data)
{
    IPConnectionCallback* cb = NULL;
    if (func) 
        cb = new CConnectionCallback(func, data);
    comm->DeclareTimedSubscription(message_name, cb);
}

void iptPublishForm(IPCommunicator* comm, char* message_name, void* data)
{
    comm->Publish(message_name, data);
}

void iptPublishRaw(IPCommunicator* comm,
                   char* message_name, int size, unsigned char* data)
{
    comm->Publish(message_name, size, data);
}

int iptNumberSubscribers(IPCommunicator* comm, char* message_name)
{
    return comm->NumSubscribers(message_name);
}

IPConnection* iptSubscribe(IPCommunicator* comm,
                           char* module_name, char* message_name)
{
    return comm->Subscribe(module_name, message_name);
}

void iptUnsubscribe(IPCommunicator* comm,
                    char* module_name, char* message_name)
{
    comm->Unsubscribe(module_name, message_name);
}

void iptServer(IPCommunicator* comm,
               char* reg_msg_name, void (*callback)(IPCommunicator*,
                                                    IPMessage*, void*),
               void* param)
{
    IPHandlerCallback* cb = NULL;
    if (callback) 
        cb = new CHandlerCallback(callback, param);
    comm->Server(reg_msg_name, cb);
}

void iptBroadcastForm(IPCommunicator* comm, char* msg_name, void* data)
{
    comm->Broadcast(msg_name, data);
}

void iptBroadcastRaw(IPCommunicator* comm,
                     char* msg_name, int size, unsigned char* data)
{
    comm->Broadcast(msg_name, size, data);
}

IPConnection* iptClient(IPCommunicator* comm, char* server_name)
{
    return comm->Client(server_name);
}

IPConnection* iptClientForm(IPCommunicator* comm,
                            char* server_name, char* msg_name, void* data)
{
    IPMessageType* type = comm->LookupMessage(msg_name);
    if (!type) {
        comm->printf("ipt(C): Can't connect as client to %s with message %s\n",
               server_name, msg_name);
        return NULL;
    }
    IPMessage out(type, comm->generate_instance_num(), data);
    return comm->Client(server_name, &out);
}

IPConnection* iptClientRaw(IPCommunicator* comm,
                           char* server_name, char* msg_name,
                           int size, unsigned char* data)
{
    IPMessageType* type = comm->LookupMessage(msg_name);
    if (!type) {
        comm->printf("ipt(C): Can't connect as client to %s with message %s\n",
                     server_name, msg_name);
        return NULL;
    }
    IPMessage out(type, comm->generate_instance_num(), size, data);
    return comm->Client(server_name, &out);
}

IPConnection* iptSubscribeFull(IPCommunicator* comm,
                               char* module_name, char* message_name,
                               char* fmt, void* data)
{
    IPFormat* format = comm->FormatParser()->Parse(fmt);
    int size = format->bufferSize(data);
    unsigned char* buffer = new unsigned char[size];

    format->encodeData(data, (char*) buffer, 0);

    IPConnection* res = comm->Subscribe(module_name, message_name,
                                        size, buffer);
    IPResource::unref(format);

    return res;
}

void iptDeclareSubscriptionFull(IPCommunicator* comm, char* message_name,
                                char* fmt,
                                int (*check)(void*, void*, void*),
                                void* (*publish)(IPCommunicator*,
                                                 IPMessage*, IPConnection*,
                                                 void*, void*, void*),
                                void (*del)(void*),
                                void (*callback)(IPCommunicator*,
                                                 IPConnection*, void*),
                                void* cb_data)

{
    IPFormat* format = comm->FormatParser()->Parse(fmt);
    IPPublication* pub =
        new C_Publication(new CConnectionCallback(callback, cb_data),
                          format, check, publish, del, cb_data);

    comm->DeclareSubscription(comm->LookupMessage(message_name), pub);
}

IPConnection* iptSubscribeConnection(IPCommunicator* comm,
                                     char* conn, char* msg_name)
{
    return comm->SubscribeConnection(conn, msg_name);
}

void iptIterateConnections(IPCommunicator* comm,
                           void (*func)(IPCommunicator*, IPConnection*, void*),
                           void* data)
{
    CConnectionCallback cb(func, data);
    comm->IterateConnections(&cb);
}

void iptAddAdministrativeConnection(IPCommunicator* comm, int fd,
                                    void (*activity)(IPCommunicator*,
                                                     IPConnection*, void*),
                                    void (*close_it)(IPCommunicator*,
                                                     IPConnection*, void*),
                                    void* data)
{
    IPConnectionCallback* act_cb =
        (activity ? new CConnectionCallback(activity, data) :
         (IPConnectionCallback*) NULL);
    IPConnectionCallback* close_cb =
        (close_it ? new CConnectionCallback(close_it, data) :
         (IPConnectionCallback*) NULL);
    comm->AddAdministrativeConnection(fd, act_cb, close_cb);
}

void iptRemoveAdministrativeConnection(IPCommunicator* comm, int fd)
{
    comm->RemoveAdministrativeConnection(fd);
}

IPMessage* iptPostponedMessage(IPCommunicator* comm, const char* msg_name)
{
    IPMessageType* type = comm->LookupMessage(msg_name);
    if (!type) {
        comm->printf("IPT(C): Cannot find postponed for unknown type %s\n",
               msg_name);
        return NULL;
    }

    return type->Postponed();
}

IPMessage* iptQueryMsgFormTO(IPConnection* conn, char* query_msg_name,
                             void* data, char* recv_msg_name,
                             double timeout)
{
    return conn->Communicator()->Query(conn, query_msg_name, data,
                                       recv_msg_name, timeout);
}

IPMessage* iptQueryMsgRawTO(IPConnection* conn, char* query_msg_name,
                            int size, unsigned char* data,
                            char* recv_msg_name, double timeout)
{
    return conn->Communicator()->Query(conn, query_msg_name, size, data,
                                       recv_msg_name, timeout);
}

int iptQueryFormContentsTO(IPConnection* conn,
                           char* query_msg_name, void* query_data,
                           char* recv_msg_name, void* reply_data,
                           double timeout)
{
    return conn->Communicator()->QueryFormatted(conn,
                                                query_msg_name, query_data,
                                                recv_msg_name, reply_data,
                                                timeout);
}

void* iptQueryFormTO(IPConnection* conn,
                     char* query_msg_name, void* query_data,
                     char* recv_msg_name, double timeout)
{
    return conn->Communicator()->QueryFormatted(conn,
                                                query_msg_name, query_data,
                                                recv_msg_name, timeout);
}

IPMessage* iptNewMessageRaw(IPMessageType* type, int instance,
                            IPConnection* conn, int size, unsigned char* data)
{
    IPMessage* res = new IPMessage(type, instance, size, data);
    res->Connection(conn);
    return res;
}

IPMessage* iptNewMessageForm(IPMessageType* type, int instance,
                             IPConnection* conn, void* form_data)
{
    IPMessage* res = new IPMessage(type, instance, form_data);
    res->Connection(conn);
    return res;
}

IPTimer* iptAddTimer(IPCommunicator* comm,
                     double interval, void (*callback)(IPCommunicator*,
                                                       IPTimer*, void*),
                     void* callback_data)
{
    return comm->AddTimer(interval,
                          new IPRoutineTimerCallback(comm,
                                                     callback, callback_data));
}

IPTimer* iptAddOneShot(IPCommunicator* comm,
                       double interval, void (*callback)(IPCommunicator*,
                                                         IPTimer*, void*),
                       void* callback_data)
{
    return comm->AddOneShot(interval,
                            new IPRoutineTimerCallback(comm, callback,
                                                       callback_data));
}

void iptRemoveTimer(IPCommunicator* comm, IPTimer* timer)
{
    comm->RemoveTimer(timer);
}

double iptTimerInterval(IPTimer* timer)
{
    return timer->interval();
}

double iptTimerTimeLeft(IPTimer* timer)
{
    return timer->time_left();
}

int iptTimerOneShot(IPTimer* timer)
{
    return timer->one_shot();
}

/* Message type cover functions */

IPFormat* iptMessageTypeFormat (IPMessageType* t)
{
    return t->Formatter();
}

char* iptMessageTypeName (IPMessageType* t)
{
    return (char*) t->Name();
}


/* IPFormat cover functions */
int iptFormatDataStructureSize (IPFormat* f)
{
    return f->dataStructureSize();
}

int iptFormatBufferSize (IPFormat* f, void* dataStruct)
{
    return f->bufferSize(dataStruct);
}

void iptFormatEncodeData (IPFormat* f, void* dataStruct, char* buf)
{
    f->encodeData(dataStruct, buf, 0);
}

void* iptFormatDecodeData (IPFormat* f, char* buf, int b_size,
                           void* dataStruct, int byte_order, int alignment)
{
    return f->decodeData(buf, 0, b_size, (char*) dataStruct,
                         byte_order, alignment);
}

void iptFormatFreeData (IPFormat* f, void* dataStruct, char* buffer, int bsize)
{
    f->freeDataStructure(dataStruct, buffer, bsize);
}

void iptFormatFreeDataElements (IPFormat* f, void* dataStruct,
                                char* buffer, int bsize)
{
    f->freeDataElements((char*) dataStruct, 0, buffer, bsize, NULL, 0);
}

int iptFormatFlatStructure (IPFormat* f)
{
    return f->sameFixedSizeDataBuffer();
}

}
