///////////////////////////////////////////////////////////////////////////////
//
//                                 event.cc
//
// This file implements classes that can be put on a communicator's 
// event queue and acted upon
//
// Classes defined for export:
//    IPEvent, MessageEvent, DataEvent, HandlerInvokedEvent, PigeonHoleEvent,
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

#include <ipt/ipt.h>
#include <ipt/list.h>
#include <ipt/event.h>
#include <ipt/connection.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/callbacks.h>
#include <ipt/pigeonhole.h>

/* Handle a message event by queue the message to comm's message queue */
int MessageEvent::Handle(IPCommunicator* comm)
{
    comm->MessageQueue()->Append((char*) _message);

    return 1;
}

/* Handle an active connection event by sucking all the messages possible off
   of the active connection */
int DataEvent::Handle(IPCommunicator*)
{
    while (_connection->DataAvailable())
        _connection->GetMessage();

    return 1;
}

HandlerInvokedEvent::~HandlerInvokedEvent()
{
    if (_message)
        delete _message;
}

/* Handle a handler event by executing the handler */
int HandlerInvokedEvent::Handle(IPCommunicator* comm)
{
    _message->Connection()->Free(_message->SizeData());

    int instance;
    if (comm->Logging()) {
      instance = _message->Instance();
      comm->LogReceive(_message->Connection(), _message, "Handling");
    }
    int res = comm->Execute(_message);
    if (res == 1)
        _message = NULL;
    if (comm->Logging()) {
      printf("\tInstance %d, ", instance);
      if (res == 0)
        printf("Handled\n");
      else if (res == -1)
        printf("Ignored\n");
      else if (res == 1)
        printf("Postponed\n");
    }

    return 1;
}

/* Handle a full pigeon hole. If the pigeon hole type has a message handler,
   execute it, otherwise make sure the communicator knows about the
   active hole */
int PigeonHoleEvent::Handle(IPCommunicator* comm)
{
    IPMessageType* type = _hole->Type();

    if (type->Handler()) {
        IPMessage* msg = _hole->Empty();

        _hole->HandlerActive(1);
        comm->Execute(msg);
        _hole->HandlerActive(0);

        if (_hole->Filled())
            comm->AddEvent(new PigeonHoleEvent(_hole));
    } else if (!_hole->Enqueued()) {
        comm->QueuePigeonHole(_hole);
    }

    return 1;
}
        
int AdministrativeEvent::Handle(IPCommunicator*)
{
    _connection->InvokeConnectCallbacks();

    return 1;
}

int ConnectionEvent::Handle(IPCommunicator* comm)
{
    if (_connection->Active()) {
        _connection->InvokeConnectCallbacks();
        comm->InvokeConnectCallbacks(_connection);
    }

    return 1;
}

int DisconnectionEvent::Handle(IPCommunicator* comm)
{
    if (_connection && !_connection->Active()) {
        _connection->InvokeDisconnectCallbacks();
        comm->InvokeDisconnectCallbacks(_connection);
    }

    return 1;
}


