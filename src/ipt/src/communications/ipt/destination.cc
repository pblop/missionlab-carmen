///////////////////////////////////////////////////////////////////////////////
//
//                                 destination.cc
//
// This file implements  classes used to define destinations for messages.
// Destinations are associated the IPMessageType, i.e. each type has one.
// Whenever a IPMessage is received, it is sent to the IPDestination declared 
// by its message type.  The IPDestination then stores the message whereever
// and however it deems appropriate.  This mechanism makes things like "pigeon
// holed" messages possible, since there can be one mechanism for most
// messages, but a special mechanism for pigeon holed messages just by changing
// classes of a message type's destination
//
// Classes defined for export:
//    IPDestination, DefaultDestination, PigeonHoleDestination
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
#include <ipt/connection.h>
#include <ipt/destination.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/event.h>
#include <ipt/pigeonhole.h>

/* The standard message making just uses the message creator to make
   the appropriate message */
IPMessage* IPDestination::MakeMessage(IPConnection*, IPMessageType* type,
                                      int instance, int size)
{
    return new IPMessage(type, instance, size, type->Cache(size));
}

/* The default destination for messages.  If the message is handled, then
   a handled message event is put on "conn"'s communicator's event queue,
   other wise the message is put on the communicator's message queue */
int DefaultDestination::ProcessMessage(IPConnection* conn, IPMessage* msg)
{
    IPMessageType* type = msg->Type();
    IPCommunicator* comm = conn->Communicator();

    conn->Allocate(msg->SizeData());
    if (type && type->Handler() && comm->HandlersActive() &&
        type->HandlerActive()) {
        comm->AddEvent(new HandlerInvokedEvent(msg, type->Handler()));
    } else {
        comm->QueueMessage(msg);
        if (comm->Logging())
            comm->LogReceive(conn, msg, "Queued");
    }

    return 1;
}

/* Process a pigeon hole'd message.  The implementation of ProcessMessage
   for pigeon hole'd messages means this simply consists of marking the
   pigeon hole as full and making sure the communicator knows that this
   pigeon hole has been filled */
int PigeonHoleDestination::ProcessMessage(IPConnection* conn, IPMessage* msg)
{
    IPPigeonHole* hole = conn->PigeonHole(msg->Type());

    int already_full = hole->Filled();
    hole->Fill(msg);

    if (conn->Communicator()->Logging())
        conn->Communicator()->LogReceive(conn, msg, "Pigeon");

    if (!already_full) {
        conn->Communicator()->AddEvent(new PigeonHoleEvent(hole));
    }

    return 1;
}

/* PigeonHoleDestination use a special process message to void additional 
   new'ing and delete'ing.  We return the message associated with the
   pigeon hole with the messages parameters set appropriately and the 
   message's data are set to the appropriate size */
IPMessage* PigeonHoleDestination::MakeMessage(IPConnection* conn,
                                              IPMessageType* type,
                                              int instance,
                                              int size)
{
    IPPigeonHole* hole = conn->PigeonHole(type);

    IPMessage* contents = hole->Contents();
    if (!contents) {
        contents = new IPMessage(type, instance, size);
        hole->Set(contents);
    } else {
        contents->SetData(size);
        contents->SetInstance(instance);
        contents->SetType(type);
    }

    return contents;
}
            
FixedDestination::FixedDestination(int max_size, unsigned char* data)
{
    _max_size = max_size;
    _default_data = data;
}

IPMessage* FixedDestination::MakeMessage(IPConnection* conn,
                                         IPMessageType* type,
                                         int instance,
                                         int size)
{
    if (size > _max_size)
        return DefaultDestination::MakeMessage(conn, type, instance, size);
    else
        return new IPMessage(type, instance, size, _default_data);
}
