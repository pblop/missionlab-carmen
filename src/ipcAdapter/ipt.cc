#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>

#include "ipt/ipt.h"
#include <ipc.h>
#include <pthread.h>

IPCommunicator::IPCommunicator(const char* mod_name, const char* host_name)
{
	context = IPC_getContext();
    IPC_setCapacity(4);
	_mod_name = strdup(mod_name);
	_host_name = strdup(host_name);
    _this_host = new char[100];
    gethostname(_this_host, 99);
    _this_host[99] = '\0';
}

/* Create the appropriate IPCommunicator instance depending on the machine
   type */
IPCommunicator* IPCommunicator::Instance(const char* mod_name,
                                         const char* host_name)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
    if(IPC_connectModule(mod_name, host_name)==IPC_OK)
    {
    	IPCommunicator *comm = new IPCommunicator(mod_name, host_name);
    	if(lastContext)
    		IPC_setContext(lastContext);
    	return comm;
    }
    else
    {
    	return NULL;
    }
}

void IPCommunicator::RegisterNamedFormatters(IPFormatSpec* specs)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
    for (int i=0;specs[i].name;i++)
		IPC_defineFormat(specs[i].name,specs[i].format);
	if(lastContext)
		IPC_setContext(lastContext);
}


/* Register a set of messages specified by the name/format array "specs".
   The final element of this array should be {NULL, NULL} */
void IPCommunicator::RegisterMessages(IPMessageSpec* specs)
{
	char *msgName;
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	for (int i=0; specs[i].name!=NULL; i++)
	{
		msgName = (char *)malloc(sizeof(char)*(strlen(specs[i].name)+strlen(_mod_name)+2));
		sprintf(msgName,"%s_%s",_mod_name, specs[i].name);
		IPC_defineMsg(specs[i].name, IPC_VARIABLE_LENGTH, specs[i].format);
		IPC_defineMsg(msgName, IPC_VARIABLE_LENGTH, specs[i].format);
		free(msgName);
	}
	if(lastContext)
		IPC_setContext(lastContext);
}

/* Look up a message type by name in the message type hash table */
IPMessageType* IPCommunicator::LookupMessage(const char* name)
{
    return name;
}



/* delete an IPCommunicator.  Deactivates all active connections, and deletes
   all allocated memory */
IPCommunicator::~IPCommunicator()
{
}


void _handlerCallbackAdapter(MSG_INSTANCE msgRef, BYTE_ARRAY callData, void *clientData)
{
	IPHandlerCallbackAdapter *adapter = (IPHandlerCallbackAdapter *)clientData;
	adapter->Execute(msgRef, callData);
}

/* Register a handler on messages of type "mess".  The callback is specified
   by "callback".  "spec" is one of IPT_HNDL_STD or IPT_HNDL_ALL.

   Once the handler is registered, the unhandled messages queue is checked
   for messages of that type.  If there are any, the handler specified by
   "callback" is invoked for each message */
void IPCommunicator::RegisterHandler(IPMessageType* type,
                                     IPHandlerCallback* callback,
                                     int spec)
{
	char *msgName = (char *)malloc(sizeof(char)*(strlen(type)+strlen(_mod_name)+2));
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	sprintf(msgName,"%s_%s",_mod_name, type);
	IPC_subscribe(type,_handlerCallbackAdapter,new IPHandlerCallbackAdapter(callback));
	IPC_subscribe(msgName,_handlerCallbackAdapter,new IPHandlerCallbackAdapter(callback));
	free(msgName);
	if(lastContext)
		IPC_setContext(lastContext);
}



/* Convenience routine for registering a handler on a message type with a
   routine "func" rather than an instance of class IPHandlerCallback */
void IPCommunicator::RegisterHandler(IPMessageType* type,
                                     void (*callback)(IPCommunicator*,
                                                  IPMessage*, void*),
                                     void* data,
                                     int spec)
{
	char *msgName = (char *)malloc(sizeof(char)*(strlen(type)+strlen(_mod_name)+2));
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	sprintf(msgName,"%s_%s",_mod_name, type);
	IPC_subscribe(type,_handlerCallbackAdapter,new IPHandlerCallbackAdapter(this, callback, data));
	IPC_subscribe(msgName,_handlerCallbackAdapter,new IPHandlerCallbackAdapter(this, callback, data));
	free(msgName);
	if(lastContext)
		IPC_setContext(lastContext);
}





/* Create a message with name "msg_name" and formatted data "data" and
   send it to connection "conn" */
int IPCommunicator::SendMessage(IPConnection* conn, const char* msg_name,
                                void *data)
{
	int dev;
	char *msgName = (char *)malloc(sizeof(char)*(strlen(conn->Name())+strlen(msg_name)+2));
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	sprintf(msgName,"%s_%s",conn->Name(), msg_name);
    dev = IPC_publishData(msgName,data);
    free(msgName);
	if(lastContext)
		IPC_setContext(lastContext);
    return dev;
}


/* This routine processes all pending events, checks the connections for fresh
   input, and processes the events that result from that fresh input.  It
   waits for "timeout" seconds for new input, and returns 1 if it gets some
   and 0 if not */
int IPCommunicator::ScanForMessages(double timeout)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
    IPC_listenWait(timeout*1000);
	if(lastContext)
		IPC_setContext(lastContext);
	return 1;
}


/* Disable the handling of a message type */
void IPCommunicator::DisableHandler(IPMessageType* type)
{
	char *msgName = (char *)malloc(sizeof(char)*(strlen(type)+strlen(_mod_name)+2));
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	sprintf(msgName,"%s_%s",_mod_name, type);
	IPC_unsubscribe(type,NULL);
	IPC_unsubscribe(msgName,NULL);
	free(msgName);
	if(lastContext)
		IPC_setContext(lastContext);
}

void _connectCallbackAdapter(const char *moduleName, void *clientData)
{
	IPConnectionCallbackAdapter *adapter = (IPConnectionCallbackAdapter *)clientData;
	adapter->Execute(moduleName);
}

/* Add a new connection callback.  The callback "cb" will be invoked
   when any module connects to this module */
void IPCommunicator::AddConnectCallback(IPConnectionCallback* callback)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_subscribeConnect(_connectCallbackAdapter,new IPConnectionCallbackAdapter(callback));
	if(lastContext)
		IPC_setContext(lastContext);
}

void IPCommunicator::AddConnectCallback(void (*callback)(IPConnection*, void*), void* data)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_subscribeConnect(_connectCallbackAdapter,new IPConnectionCallbackAdapter(callback, data));
	if(lastContext)
		IPC_setContext(lastContext);
}


/* Add a new disconnection callback.  The callback "cb" will be invoked
   when any module disconnects from this module */
void IPCommunicator::AddDisconnectCallback(IPConnectionCallback* callback)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_subscribeDisconnect(_connectCallbackAdapter,new IPConnectionCallbackAdapter(callback));
	if(lastContext)
		IPC_setContext(lastContext);
}

void IPCommunicator::AddDisconnectCallback(void (*callback)(IPConnection*, void*), void* data)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_subscribeDisconnect(_connectCallbackAdapter,new IPConnectionCallbackAdapter(callback, data));
	if(lastContext)
		IPC_setContext(lastContext);
}


/* Sleep for "timeout" seconds, responding to handlers.  Returns 1 if any
   message process was done during the sleep, and 0 if not */
int IPCommunicator::Sleep(double timeout)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_listenWait(timeout*1000);
	if(lastContext)
		IPC_setContext(lastContext);
	return 1;
}

/* Sleep for "timeout" seconds, or until the first message processing is done.
   Returns 1 if any message processing was done during the idle, 0 if not */
int IPCommunicator::Idle(double timeout)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_listenWait(timeout*1000);
	if(lastContext)
		IPC_setContext(lastContext);
    return 1;
}

IPMessage* IPCommunicator::QueryFormattedEx(IPConnection* conn,
                                     const char* query_msg_name,
                                     void* query_data,
                                     const char* recv_msg_name,
                                     double timeout)
{
	void *replyData = NULL;
	IPMessage *replyMsg;
	IPC_VARCONTENT_TYPE varcontent;
	FORMATTER_PTR formatter, replyFormatter;
	BYTE_ARRAY reply;
	IPC_RETURN_TYPE dev;
	IPC_CONTEXT_PTR lastContext = IPC_getContext();

	IPC_setContext(context);
	formatter = IPC_msgFormatter(query_msg_name);
	replyFormatter = IPC_msgFormatter(recv_msg_name);
	IPC_marshall(formatter, query_data, &varcontent);

	if(timeout==IPT_BLOCK)
		timeout = IPC_WAIT_FOREVER;
	else
		timeout *= 1000;

	dev = IPC_queryResponseVC(query_msg_name, &varcontent, &reply, timeout);
	if(dev!=IPC_OK)
	{
		printf("Error en QueryResponseVC: \nQuery: %s\nResponse: %s\n", query_msg_name, recv_msg_name);
		if(dev == IPC_Error)
			IPC_perror("Descripción del error");
		else if(dev == IPC_Timeout)
			printf("Descripción del error: TIMEOUT\n");

		if(lastContext)
			IPC_setContext(lastContext);

		return NULL;
	}

	IPC_unmarshall(replyFormatter, reply, &replyData);
	IPC_freeByteArray(varcontent.content);

	replyMsg = new IPMessage(reply, replyData, replyFormatter);

	if(lastContext)
		IPC_setContext(lastContext);

	return replyMsg;
}

IPMessage* IPCommunicator::Query(IPConnection* conn,
								 const char* query_msg_name,
                                 void* query_data,
                                 const char* recv_msg_name,
                                 double timeout)
{
	IPMessage* replyMsg = NULL;

	char *msgName = (char *)malloc(sizeof(char)*(strlen(conn->Name())+strlen(query_msg_name)+2));
	sprintf(msgName,"%s_%s",conn->Name(), query_msg_name);
	replyMsg = QueryFormattedEx(conn, msgName, query_data, recv_msg_name, timeout);
	free(msgName);

	return replyMsg;
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
	IPMessage* replyMsg = NULL;

	char *msgName = (char *)malloc(sizeof(char)*(strlen(conn->Name())+strlen(query_msg_name)+2));
	sprintf(msgName,"%s_%s",conn->Name(), query_msg_name);
	replyMsg = QueryFormattedEx(conn, msgName, query_data, recv_msg_name, timeout);
	free(msgName);

	return replyMsg->FormattedData();
}



/* convenience function for replying by message name with formatted data */
void IPCommunicator::Reply(IPMessage* msg, const char* msg_name, void* data)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_respondData(msg->getMsgInstance(),msg_name,data);
	if(lastContext)
		IPC_setContext(lastContext);
}



const char* IPCommunicator::ThisHost() const
{
    return _this_host;
}


/* Declares this module as a server.  All clients will register with a message
   of type "registration_type".  If "registration_type" is NULL, then we use
   a default of IPTServerRegisterMsg.  If callback "cb" is non-NULL, then we
   invoke it whenever a client registers */
void IPCommunicator::Server()
{

}


/* Broadcast message with name "name" and formatted data "data" to all
   clients */
void IPCommunicator::Broadcast(const char* msg_name, void* data)
{
	IPC_CONTEXT_PTR lastContext = IPC_getContext();
	IPC_setContext(context);
	IPC_publishData(msg_name,data);
	if(lastContext)
		IPC_setContext(lastContext);
}


IPConnection* iptClient(IPCommunicator* comm, const char* server_name)
{
    return new IPConnection(server_name);
}
