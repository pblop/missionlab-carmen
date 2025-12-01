///////////////////////////////////////////////////////////////////////////////
//
//                                 tcx_cover.cc
//
// This file implements functions for making IPT look like TCX for both C and
// C++ code
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
#include <sys/types.h>

#include "./libc.h"

#ifdef VXWORKS
#include <taskLib.h>
#include <taskVarLib.h>
#include <sockLib.h>
#else
#include <sys/time.h>
#endif

#include <ipt/tcx_cover.h>
#include <ipt/ipt.h>
#include <ipt/connection.h>
#include <ipt/format.h>
#include <ipt/message.h>
#include <ipt/time.h>
#include <ipt/parseformat.h>
#include <ipt/callbacks.h>
#include <ipt/filter.h>
#include <ipt/messagetype.h>

/* Callback class for converting TCX callbacks to IPT callbacks */
class TCXHandlerCallback : public IPHandlerCallback {
  public:
    TCXHandlerCallback(void (*proc)(TCX_REF_PTR, void*, void*),
                void* data)
        { _data = data; _proc = proc; }

    void Execute(IPMessage* msg)
        { (*_proc)((TCX_REF_PTR) msg, msg->FormattedData(1), _data); }

  private:
    void* _data;     // parameter data
    void (*_proc)(TCX_REF_PTR, void*, void*); // callback routine
};

/* Callback class for duplicating TCX connect/disconnect callbacks */
class TCXConnectCallback : public IPConnectionCallback {
  public:
    TCXConnectCallback(void (*proc)(TCX_MODULE_PTR, void*),
                       void* data)
        { _data = data; _proc = proc; }

    void Execute(IPConnection* conn)
        { (*_proc)((TCX_MODULE_PTR) conn, _data); }

  private:
    void* _data;
    void (*_proc)(TCX_MODULE_PTR, void*);
};

/* IPT variable for TCX cover functions */
extern IPCommunicator* _ip_communicator;

extern "C" {

void tcxInitialize(char* module, char* server)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
            printErr("Can't taskVarAdd\n");
            taskSuspend(0);
        }
#endif

        _ip_communicator = IPCommunicator::Instance(module, server);
    } else {
        _ip_communicator->printf("IPT(TCX): already initialized\n");
        return;
    }
}

#ifdef VXWORKS
void tcxInitializeInherited(char* task_name)
{
    if (_ip_communicator) {
        _ip_communicator->printf("IPT(TCX): already initialized\n");
        return;
    }
    
    int tid = taskNameToId(task_name);
    if (tid == ERROR) {
        _ip_communicator->printf("IPT(TCX): Error inheriting TCX variables from %s\n",
                task_name);
        return;
    }
    _ip_communicator->printf("IPT(TCX): Inheriting TCX communicator from %s (%x)\n",
           task_name, tid);

    if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
        printErr("Can't taskVarAdd\n");
        taskSuspend(0);
    }
    
    _ip_communicator = (IPCommunicator*) taskVarGet(tid, (int*) &_ip_communicator);
}

void tcxDeinitializeInherited()
{
    if (_ip_communicator) {
        _ip_communicator = NULL;
        taskVarDelete(0, (int*) &_ip_communicator);
    }
}

#endif

void tcxDeinitialize()
{
    if (_ip_communicator) {
        delete _ip_communicator;
        _ip_communicator = NULL;
#ifdef VXWORKS
        taskVarDelete(0, (int*) &_ip_communicator);
#endif
    }
}

TCX_MODULE_PTR tcxConnectModule(char* module)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return NULL;
    }

    IPConnection* res = _ip_communicator->Connect(module);

    if (!res || !res->Active())
        return NULL;
    else
        return (TCX_MODULE_PTR) res;
}

TCX_MODULE_PTR tcxConnectOptional(char* module)
{
    if (_ip_communicator) {
        IPConnection* res = _ip_communicator->Connect(module, IPT_OPTIONAL);
        
        if (!res || !res->Active())
            return NULL;
        else
            return (TCX_MODULE_PTR) res;
    } else {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return NULL;
    }
}

char* tcxModuleName(TCX_MODULE_PTR module)
{
    if (module)
        return (char*) ((IPConnection*) module)->Name();
    return NULL;
}
        
void tcxSendData(TCX_MODULE_PTR module, int id, int ins, void* data,
                 int len, TCX_FMT_PTR fmt)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    if (!module) {
        fprintf(stderr,
                "TCX Error (tcxSendData): sending to null connection\n");
        return;
    }

    IPFormat* format = (IPFormat*) fmt;

    if (format) {
        int size = format->bufferSize((char*) data);
        unsigned char* output = new unsigned char[size];
        format->encodeData((char*) data, (char*) output, 0);

        IPMessage out_msg(id, ins, size, output);
        _ip_communicator->SendMessage((IPConnection*) module, &out_msg);
        delete output;
    } else {
        IPMessage out_message(id, ins, len, (unsigned char*) data);
        _ip_communicator->SendMessage((IPConnection*) module, &out_message);
    }
}

int tcxRecvData(TCX_MODULE_PTR *module, int* id, int* ins, void* data,
                TCX_FMT_PTR fmt, int allowHnd, struct timeval *timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPConnection* desired_connection = NULL;
    IPMessageType* desired_type = NULL;
    if (module)
        desired_connection = *(IPConnection**) module;
    if (id && *id)
        desired_type = _ip_communicator->LookupMessage(*id);

    InstanceFilter inst_f;
    MsgFilter mf;
    IPFilter* f = NULL;
    if (ins && *ins) {
        inst_f = InstanceFilter(*ins, desired_connection, desired_type);
        f = &inst_f;
    } else if (desired_type || desired_connection) {
        mf = MsgFilter(desired_type, desired_connection);
        f = &mf;
    }

    if (allowHnd != ALL_HND)
        _ip_communicator->DisableHandlers();

    double down_time;
    if (!timeout)
        down_time = IPT_BLOCK;
    else
        down_time =
            IPTime((int) timeout->tv_sec, (int) timeout->tv_usec/1000).Value();
    IPMessage* msg = _ip_communicator->ReceiveMessage(f, down_time);

    if (allowHnd != ALL_HND)
        _ip_communicator->EnableHandlers();

    if (!msg)
        return -1;

    int num_read = msg->SizeData();
    if (data) {
        if (fmt) {
            if (id && !*id) {
                void* msg_data = msg->FormattedData(1);
                bcopy((char*) &msg_data, (char*) data, sizeof(void*));
            } else {
                msg->FormattedData(data, 1);
            }
        } else {
            if (id && !*id) {
                unsigned char* msg_data = new unsigned char[msg->SizeData()];
                bcopy((char*) msg->Data(), (char*) msg_data, msg->SizeData());
                bcopy((char*) &msg_data, (char*) data, sizeof(unsigned char*));
            } else
                bcopy((char*) msg->Data(), (char*) data, num_read);
        }
    }

    if (module && !*module)
        *module = (TCX_MODULE_PTR) msg->Connection();
    if (id && !*id)
        *id = msg->ID();
    if (ins && !*ins)
        *ins = msg->Instance();

    delete msg;

    return num_read;
}

TCX_FMT_PTR tcxParseFormat(char *format)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return NULL;
    }

    return (TCX_FMT_PTR) _ip_communicator->FormatParser()->Parse(format);
}

void tcxFree(char* msgName, void* data)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    IPMessageType* type = _ip_communicator->LookupMessage(msgName);
    if (!type) {
        _ip_communicator->printf("TCX error (tcxFree): unknown message name %s\n", msgName);
        return;
    }

    type->DeleteFormattedData(data, NULL, 0);
}

void tcxFreeReply(char* msgName, void* data)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    IPMessageType* type = _ip_communicator->LookupMessage(msgName);
    if (!type) {
        _ip_communicator->printf("TCX error (tcxFreeReply): unknown message name %s\n",
               msgName);
        return;
    }

    if (type->Formatter())
        type->Formatter()->freeDataElements((char*) data, 0, NULL, 0, NULL, 0);
}

void tcxFreeByRef(TCX_REF_PTR ref, void* data)
{
    ((IPMessage*) ref)->DeleteFormatted(data);
}

void tcxFreeData(TCX_FMT_PTR fmt, void* data)
{
    ((IPFormat*) fmt)->freeDataStructure((char*) data, NULL, 0);
}

void tcxRegisterMessages(TCX_REG_MSG_TYPE* msgArray, int size)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    int i;
    IPMessageSpec* specs = new IPMessageSpec[size+1];

    for (i=0;i<size;i++) {
        specs[i].name = msgArray[i].msgName;
        specs[i].format = msgArray[i].msgFormat;
    }
    specs[size].name = specs[size].format = NULL;
        
    _ip_communicator->RegisterMessages(specs);
}

void tcxRegisterHandlers(TCX_REG_HND_TYPE* handArray, int size)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    int i;

    for (i=0;i<size;i++) {
        IPMessageType* type =
            _ip_communicator->LookupMessage(handArray[i].msgName);
        if (!type) {
            _ip_communicator->printf("TCX Error (register): Unknown message type %s for handler %s\n",
                   handArray[i].msgName, handArray[i].hndName);
            continue;
        }

        _ip_communicator->
            RegisterHandler(type,
                            new TCXHandlerCallback((void (*) (TCX_REF_PTR,
                                                              void*,
                                                              void*))
                                                   handArray[i].hndProc,
                                                   handArray[i].hndData),
                            handArray[i].hndControl);
    }
}

int tcxSendMsg(TCX_MODULE_PTR module, char* msgName, void* data)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPMessageType* type = _ip_communicator->LookupMessage(msgName);
    if (!type) {
        _ip_communicator->printf("TCX error (tcxSendMsg): unknown message name %s\n", msgName);
        return -1;
    }

    return _ip_communicator->SendMessage((IPConnection*) module, type, data);
}

int tcxRecvLoop(struct timeval* timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    double down_time;
    if (!timeout)
        down_time = IPT_BLOCK;
    else
        down_time =
            IPTime((int) timeout->tv_sec, (int) timeout->tv_usec/1000).Value();

    int res = _ip_communicator->Sleep(down_time);
    if (!res)
        res = -1;
    return res;
}

void tcxQuery(TCX_MODULE_PTR module, char* msgName, void* data,
              char* replyMsgName, void* reply)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    IPMessageType* msgType = _ip_communicator->LookupMessage(msgName);
    if (!msgType) {
        _ip_communicator->printf("TCX Error (tcxQuery): Unknown message type %s\n", msgName);
        return;
    }
    IPMessageType* replyMsgType = _ip_communicator->LookupMessage(replyMsgName);
    if (!replyMsgType) {
        _ip_communicator->printf("TCX Error (tcxQuery, reply): Unknown message type %s\n",
               replyMsgName);
        return;
    }

    IPMessage* msg = _ip_communicator->Query((IPConnection*) module,
                                          msgType, data, replyMsgType);

    if (!msg) {
        _ip_communicator->printf("TCX Error: Query returned NULL\n");
        return;
    }

    msg->FormattedData(reply, 1);
        
    delete msg;

    return;
}

void tcxReply(TCX_REF_PTR ref, char* replyMsgName, void* reply)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    IPMessageType* replyMsgType = _ip_communicator->LookupMessage(replyMsgName);
    if (!replyMsgType) {
        _ip_communicator->printf("TCX Error (tcxReply): Unknown message type %s\n",
               replyMsgName);
        return;
    }

    _ip_communicator->Reply((IPMessage*) ref, replyMsgType, reply);
}

char* tcxMessageName(int id)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return NULL;
    }

    IPMessageType* msgType = _ip_communicator->LookupMessage(id);
    if (!msgType) {
        _ip_communicator->printf("TCX Error (tcxMessageName): Unknown message ID %d\n", id);
        return NULL;
    }

    return (char*) msgType->Name();
}

int tcxMessageId(char* name)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPMessageType* msgType = _ip_communicator->LookupMessage(name);
    if (!msgType) {
        _ip_communicator->printf("TCX Error (tcxMessageId): Unknown message name %d\n", name);
        return -1;
    }

    return msgType->ID();
}

TCX_MODULE_PTR tcxRefModule(TCX_REF_PTR ref)
{
    return (TCX_MODULE_PTR) ((IPMessage*) ref)->Connection();
}

int tcxRefId(TCX_REF_PTR ref)
{
    return ((IPMessage*) ref)->ID();
}

int tcxRefIns(TCX_REF_PTR ref)
{
    return ((IPMessage*) ref)->Instance();
}

int tcxRecvMsg(char* msgName, int *ins, void* data, struct timeval* timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPMessageType* msgType = _ip_communicator->LookupMessage(msgName);
    if (!msgType) {
        _ip_communicator->printf("TCX Error (tcxRecvMsg): Unknow message type %s\n", msgName);
        return 0;
    }

    MsgFilter mf;
    InstanceFilter inst_f;
    IPFilter* f = NULL;

    if (ins && *ins) {
        inst_f = InstanceFilter(*ins, NULL, msgType);
        f = &inst_f;
    } else if (msgType) {
        mf = MsgFilter(msgType, NULL);
        f = &mf;
    }

    double down_time;
    if (!timeout)
        down_time = IPT_BLOCK;
    else
        down_time =
            IPTime((int) timeout->tv_sec, (int) timeout->tv_usec/1000).Value();

    IPMessage* msg = _ip_communicator->ReceiveMessage(f, down_time);

    if (!msg)
        return -1;

    msg->FormattedData(data, 1);
        
    if (ins && !*ins)
        *ins = msg->Instance();

    delete msg;

    return 1;
}

int tcxRecvMsgE(TCX_MODULE_PTR *module, char *msgName, int *ins, 
                void *data, int allowHnd, struct timeval *timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPMessageType* msgType;

    if (msgName) {
        msgType = _ip_communicator->LookupMessage(msgName);
        if (!msgType) {
            _ip_communicator->printf("TCX Error (tcxRecvMsgE): Unknow message type %s\n",
                   msgName);
            return -1;
        }
    } else
        msgType = NULL;

    IPConnection* desired_connection;
    if (!module)
        desired_connection = NULL;
    else
        desired_connection = *(IPConnection**) module;

    MsgFilter mf;
    InstanceFilter inst_f;
    IPFilter* f = NULL;

    if (ins && *ins) {
        inst_f = InstanceFilter(*ins, desired_connection, msgType);
        f = &inst_f;
    } else if (msgType || desired_connection) {
        mf = MsgFilter(msgType, desired_connection);
        f = &mf;
    }

    double down_time;
    if (!timeout)
        down_time = IPT_BLOCK;
    else
        down_time =
            IPTime((int) timeout->tv_sec, (int) timeout->tv_usec/1000).Value();

    if (allowHnd != ALL_HND)
        _ip_communicator->DisableHandlers();

    IPMessage* msg = _ip_communicator->ReceiveMessage(f, down_time);

    if (allowHnd != ALL_HND)
        _ip_communicator->EnableHandlers();

    if (!msg)
        return -1;

    msg->FormattedData(data, 1);
        
    if (module && !*module)
        *module = (TCX_MODULE_PTR) msg->Connection();
    if (ins && !*ins)
        *ins = msg->Instance();
    
    delete msg;

    return 1;
}

int tcxRecvMsgNoHnd(char *msgName, int *ins, void *data,
                     struct timeval *timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    _ip_communicator->DisableHandlers();
    int res = tcxRecvMsg(msgName, ins, data, timeout);
    _ip_communicator->EnableHandlers();
    
    return res;
}

int tcxRecv(struct timeval* timeout)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    double down_time;
    if (!timeout)
        down_time = IPT_BLOCK;
    else
        down_time =
            IPTime((int) timeout->tv_sec, (int) timeout->tv_usec/1000).Value();

    int res = _ip_communicator->Idle(down_time);
    if (!res)
        res = -1;
    return res;
}

int tcxNRecvMsgE(TCX_MODULE_PTR *module, char **msgName, 
                 void *data, struct timeval *timeout, int *ins, int allowHnd)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return -1;
    }

    IPMessageType* msgType;
    if (msgName && *msgName) {
        msgType = _ip_communicator->LookupMessage(*msgName);
        if (!msgType) {
            _ip_communicator->printf("TCX Error (tcxNRecvMsgE): Unknow message type %s\n",
                   *msgName);
            return -1;
        }

        int id = msgType->ID();
        return tcxRecvData(module, &id, ins, data,
                           msgType->Formatter(), allowHnd, timeout);
    } else
        msgType = NULL;

    int id = 0;
    char* buf = NULL;
    int res = tcxRecvData(module, &id, ins, &buf, NULL, allowHnd, timeout);

    if (res < 0)
        return res;

    msgType = _ip_communicator->LookupMessage(id);
    if (!msgType) {
        if (msgName)
            *msgName = NULL;
        return res;
    }

    if (msgType->Formatter()) {
        if (buf) {
            char* msg_data = (char*)
                msgType->Formatter()->decodeData(buf, 0, 0, NULL,
                                                 BYTE_ORDER, ALIGN);
            bcopy((char*) &msg_data, (char*) data, sizeof(char*));
        }
    } else {
        *(char**) data = NULL;
    }

    if (msgName)
        *msgName = (char*) msgType->Name();

    return res;
}
    
char* tcxCurrentModuleName()
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return NULL;
    }

    return (char*) _ip_communicator->ModuleName();
}

int tcxTestActiveModule(TCX_MODULE_PTR module)
{
    return ((IPConnection*) module)->Active();
}

void tcxCloseAll()
{
    tcxDeinitialize();
}

void tcxSetConnectCallback(void (*connect_func)(TCX_MODULE_PTR, void*),
                           void* connect_data)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    _ip_communicator->AddConnectCallback(new TCXConnectCallback(connect_func,
                                                             connect_data));
}

void tcxSetDisconnectCallback(void (*func)(TCX_MODULE_PTR,
                                           void*),
                              void* data)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        _ip_communicator->printf("%x: ", taskIdSelf());
#endif
        _ip_communicator->printf("IPT(TCX) ERROR: not initialized\n");
        return;
    }

    _ip_communicator->AddDisconnectCallback(new TCXConnectCallback(func, data));
}

int tcxInitialized()
{
    return _ip_communicator != NULL;
}

int bufferSize(TCX_FMT_PTR fmt, void* data)
{
    return ((IPFormat*) fmt)->bufferSize((char*) data);
}

void encodeData(TCX_FMT_PTR fmt, void* in, unsigned char* out, int size)
{
    ((IPFormat*) fmt)->encodeData((char*) in, (char*) out, size);
}

char* decodeData(TCX_FMT_PTR fmt, unsigned char* in, int size)
{
    return (char*) ((IPFormat*) fmt)->decodeData((char*) in, size, 0, NULL,
                                                 BYTE_ORDER, ALIGN);
}

IPCommunicator* tcxCoverCommunicator()
{
    return _ip_communicator;
}

void tcxInitializeCommunicator(IPCommunicator* comm)
{
    if (!_ip_communicator) {
#ifdef VXWORKS
        if (taskVarAdd(0, (int*) &_ip_communicator) != OK) {
            printErr("Can't taskVarAdd\n");
            taskSuspend(0);
        }
#endif
        _ip_communicator = comm;
    }
}

TCX_REF_PTR tcxRefDup(TCX_REF_PTR ref, TCX_REF_PTR out)
{
    IPMessage* msg = (IPMessage*) ref;
    IPMessage* res = (IPMessage*) out;
    *res = *msg;

    return (TCX_REF_PTR) res;
}

}
