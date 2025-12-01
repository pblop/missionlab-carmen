#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef VXWORKS
#include <taskLib.h>
#else
#include "./libc.h"
#endif

#include <ipt/vxcommunicator.h>
#include <ipt/vxconnection.h>
#include <ipt/event.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/destination.h>
#include <ipt/internal_messages.h>

class VxDeactivatingEvent : public IPEvent {
  public:
    VxDeactivatingEvent(IPConnection* conn, VxConnection* other)
        : IPEvent(IPEvent::DeactivatingEventType)
            { _conn = conn; _other = other; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _conn;
    VxConnection* _other;
};

class VxNewConnectionEvent : public IPEvent {
  public:
    VxNewConnectionEvent(IPConnection* conn)
        : IPEvent(IPEvent::NewConnectionEventType)
            { _conn = conn; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _conn;
};

int VxDeactivatingEvent::Handle(IPCommunicator* comm)
{
    ((VxConnection*) _conn)->SetOther(_other);
    comm->DeactivateConnection(_conn);

    return 1;
}

int VxNewConnectionEvent::Handle(IPCommunicator* comm)
{
    /* do the connection administrata */
    comm->connect_notify(_conn);

    return 1;
}

VxConnection::VxConnection(const char* name, const char* host,
                           IPCommunicator* comm, VxConnection* other)
    : IPConnection("Vx", name, host, comm, -1)
{
    _other = other;
    _holder = NULL;
    _waiting_for_message = 0;
}

VxConnection::~VxConnection()
{
    if (_other)
        _other->RemoteDeactivate();
}

int VxConnection::Active()
{
    return (_other != NULL);
}

int VxConnection::Viable()
{
    return (_other != NULL);
}

int VxConnection::Send(IPMessage* msg)
{
    return _other->RemoteReceive(msg);
}

IPMessage* VxConnection::Receive()
{
    if (!_other || _waiting_for_message)
        return NULL;
    
    _waiting_for_message = 1;
    _msg_holder = NULL;
    
    while (!_msg_holder && _other) 
#ifdef VXWORKS
        taskDelay(1);
#else
        usleep(16667);
#endif

    _waiting_for_message = 0;
    return _msg_holder;
}

void VxConnection::Deactivate()
{
    if (_other) {
        _other = NULL;
        IPConnection::Deactivate();
    }
}

void VxConnection::RemoteDeactivate()
{
    IPEvent* event = new VxDeactivatingEvent(this, _other);
    _other = NULL;
    Communicator()->AddEvent(event);
}

int VxConnection::RemoteReceive(IPMessage* msg) 
{
    IPDestination* dest;
    IPMessageType* type = Communicator()->LookupMessage(msg->ID());
    IPMessage* new_msg;

    while (Allocated() > MAX_BACKUP)
        taskDelay(1);
    
    if (type) {
        dest = type->Destination();
        if (!dest)
            dest = Communicator()->Destination();
        
        new_msg = dest->MakeMessage(this, type, msg->Instance(),
                                    msg->SizeData());
    } else {
        new_msg = new IPMessage(msg->ID(), msg->Instance(), msg->SizeData());
        dest = Communicator()->Destination();
    }

    bcopy((char*) msg->Data(), (char*) new_msg->Data(), msg->SizeData());
    if (_holder)
        new_msg->Connection(_holder);
    else {
        _holder = Communicator()->LookupConnection(this->Name());
        if (_holder)
            new_msg->Connection(_holder);
        else
            new_msg->Connection(this);
    }
    
    if (_waiting_for_message) {
        _msg_holder = new_msg;
        return 1;
    }

    
    dest->ProcessMessage(this, new_msg);

    ((VxCommunicator*) Communicator())->FlagActivity();

    return 1;
}

void VxConnection::ActivateVx(VxConnection* other)
{
    SetOther(other);

    ((VxCommunicator*) Communicator())->FlagActivity();

    Communicator()->AddEvent(new VxNewConnectionEvent(this));
}

    
        


    
