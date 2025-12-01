#include <stdio.h>
#include <string.h>

#ifdef VXWORKS
#include <taskLib.h>
#include <selectLib.h>
#include <sockLib.h>
#else
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "./libc.h"
#endif

#include <ipt/vxcommunicator.h>
#include <ipt/vxconnection.h>
#include <ipt/forconnection.h>
#include <ipt/fdconnection.h>
#include <ipt/internal_messages.h>
#include <ipt/list.h>
#include <ipt/event.h>
#include <ipt/primcomm.h>
#include <ipt/message.h>
#include <ipt/format.h>
#include <ipt/route.h>

class VxRouter : public IPRouter {
  public:
    VxRouter(VxCommunicator* comm);

    virtual IPConnection* copy_connection(const char*, const char*,
                                          IPConnection*);

  protected:
    virtual IPConnection* make_connection(const char* name,
                                          const char* sub_type,
                                          const char* parameters);
    virtual void activate_connection(IPConnection* conn, const char* sub_type,
                                     const char* parameters);

  private:
    VxCommunicator* _vx_comm;
};

VxRouter::VxRouter(VxCommunicator* comm) : IPRouter(comm, "Vx", IPT_HOSTNAME)
{
    _vx_comm = comm;

    char buffer[100];
    sprintf(buffer, "%x", comm);
    routing()->set_parameters(buffer);
    routing()->set_sub_type(comm->ThisHost());
}

IPConnection* VxRouter::make_connection(const char* name,
                                        const char*, const char* parameters)
{
    VxCommunicator* other;
    if (sscanf(parameters, "%x", &other) != 1) {
        _vx_comm->printf("IPT(VxRouter): Error reading parameters %s\n",
                         parameters);
        return NULL;
    }

    VxConnection* res = new VxConnection(name, _vx_comm->ThisHost(),
                                         _vx_comm);
    VxConnection* connected = other->AddVxConnection(res);
    if (connected)
        res->SetOther(connected);
    else {
        delete res;
        res = NULL;
    }

    return res;
}

void VxRouter::activate_connection(IPConnection* conn, const char*,
                                   const char* parameters)
{
    VxCommunicator* other;
    if (sscanf(parameters, "%x", &other) != 1) {
        _vx_comm->printf("IPT(VxRouter): Error reading parameters %s\n",
                               parameters);
        return;
    }

    VxConnection* connected = other->AddVxConnection((VxConnection*) conn);
    if (connected) {
        ((VxConnection*) conn)->SetOther(connected);
    }
}

IPConnection* VxRouter::copy_connection(const char* name,
                                        const char* host, IPConnection* conn)
{
    if (strcmp(conn->Type(), "Vx"))
        return NULL;

    VxConnection* vx_conn = (VxConnection*) conn;
    if (!vx_conn->Other())
        return NULL;

    VxConnection* res = new VxConnection(name, host, _vx_comm, 
                                         vx_conn->Other());

    res->SetByteOrder(conn->ByteOrder());
    res->SetAlignment(conn->Alignment());

    return res;
}

VxCommunicator::VxCommunicator(const char* mod_name, const char* host_name)
    : TCPCommunicator(mod_name, host_name)
{
    _message_sem = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
    _hole_sem = semBCreate(SEM_Q_PRIORITY, SEM_FULL);
    _event_sem = semBCreate(SEM_Q_PRIORITY, SEM_FULL);

    _activity = 0;

    add_router(new VxRouter(this), IPT_ROUTE_HIGHEST);
}

int VxCommunicator::WaitForInput(double timeout)
{
    int ticks = (int) (timeout/60.0);
    int cur_ticks = 0;

    while (1) {
        IPList* connections = AdministrativeConnections();
        IPConnection* conn = (IPConnection*) connections->First();
        for (;conn;conn = (IPConnection*) connections->Next())
            if (conn->Active() && conn->Viable() && conn->DataAvailable()) {
                AddEvent(new AdministrativeEvent(conn));
                _activity = 1;
            }

        connections = Connections();
        conn = (IPConnection*) connections->First();
        for (;conn;conn = (IPConnection*) connections->Next()) {
            if (conn->Active() && conn->FD() != -1) {
                if (!conn->Viable()) 
                    DeactivateConnection(conn);
                else if (conn->Allocated() < MAX_BACKUP &&
                         conn->DataAvailable()) {
                    AddEvent(new DataEvent(conn));
                    _activity = 1;
                }
            }
        }
        
        if (_activity)
            break;
#ifdef VXWORKS        
        taskDelay(1);
#else
        usleep(16667);
#endif

        cur_ticks++;
        if (timeout != -1.0 && cur_ticks > ticks)
            break;
    }

    int res = _activity;
    _activity = 0;
    return res;
}

VxConnection* VxCommunicator::AddVxConnection(VxConnection* conn)
{
    const char* name = conn->Communicator()->ModuleName();
    ForConnection* res = (ForConnection*) LookupConnection(name);

    if (!res) 
        res = (ForConnection*) DeclareConnection(name);
    else {
        if (res->Active())
            return NULL;

        if (!strcmp(res->Body()->Type(), "Vx")) {
            ((VxConnection*) res)->ActivateVx(conn);
            return (VxConnection*) res->Body();
        }
    }

    VxConnection* body = new VxConnection(name,
                                          conn->Communicator()->ThisHost(),
                                          this);
    res->Body(body);

    body->ActivateVx(conn);
    return body;
}

void VxCommunicator::AddEvent(IPEvent* event)
{
    semTake(_event_sem, WAIT_FOREVER);
    IPCommunicator::AddEvent(event);
    semGive(_event_sem);
}

IPEvent* VxCommunicator::pop_event()
{
    IPList* events = Events();
    int taken = 0;
    if (events->Length() == 1) {
        taken = 1;
        semTake(_event_sem, WAIT_FOREVER);
    }
    IPEvent* res = IPCommunicator::pop_event();
    if (taken)
        semGive(_event_sem);

    return res;
}

void VxCommunicator::QueueMessage(IPMessage* message)
{
    semTake(_message_sem, WAIT_FOREVER);
    IPCommunicator::QueueMessage(message);
    semGive(_message_sem);
}

IPMessage* VxCommunicator::pop_message()
{
    IPList* messages = MessageQueue();
    int taken = 0;
    if (messages->Length() == 1) {
        semTake(_message_sem, WAIT_FOREVER);
        taken = 1;
    }
    IPMessage* res = IPCommunicator::pop_message();
    if (taken)
        semGive(_message_sem);

    return res;
}

void VxCommunicator::QueuePigeonHole(IPPigeonHole* hole)
{
    semTake(_hole_sem, WAIT_FOREVER);
    IPCommunicator::QueuePigeonHole(hole);
    semGive(_hole_sem);
}

IPPigeonHole* VxCommunicator::pop_hole()
{
    IPList* holes = PigeonHoleQueue();
    int taken = 0;
    if (holes->Length() == 1) {
        semTake(_hole_sem, WAIT_FOREVER);
        taken = 1;
    }
    IPPigeonHole* res = IPCommunicator::pop_hole();
    if (taken)
        semGive(_hole_sem);

    return res;
}

IPMessage* VxCommunicator::remove_message(int (*func)(const char*,
                                                      const char*),
                                          const char* param, int all)
{
    int i;
    char* cur = MessageQueue()->First();
    for (i=1;i <= MessageQueue()->Length();i++) {
        if ((*func)(param, cur)) {
            if (i == MessageQueue()->Length()) {
                semTake(_message_sem, WAIT_FOREVER);
                MessageQueue()->Remove(cur);
                semGive(_message_sem);
            } else
                MessageQueue()->Remove(cur);

            if (!all)
                return (IPMessage*) cur;
        }
        cur = MessageQueue()->Next();
    }

    return NULL;
}

IPPigeonHole* VxCommunicator::remove_hole(int (*func)(const char*,
                                                      const char*),
                                          const char* param, int all)
{
    int i;
    char* cur = PigeonHoleQueue()->First();
    for (i=1;i <= PigeonHoleQueue()->Length();i++) {
        if ((*func)(param, cur)) {
            if (i == PigeonHoleQueue()->Length()) {
                semTake(_hole_sem, WAIT_FOREVER);
                PigeonHoleQueue()->Remove(cur);
                semGive(_hole_sem);
            } else
                PigeonHoleQueue()->Remove(cur);

            if (!all)
                return (IPPigeonHole*) cur;
        }
        cur = PigeonHoleQueue()->Next();
    }

    return NULL;
}



