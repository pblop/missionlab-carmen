///////////////////////////////////////////////////////////////////////////////
//
//                                 forconnection.cc
//
// This file implements the forwarded connection class.  This class can be
// used to contain other types of connections.
//
// Classes defined for export:
//    ForConnection
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

#include <ipt/forconnection.h>
#include <ipt/message.h>

/* create an empty forwarded connection named "name" with communicator
   "comm" */
ForConnection::ForConnection(const char* name, IPCommunicator* comm)
    : IPConnection("Forwarded", name, NULL, comm, -1)
{
    _body = NULL;
}

/* delete a forwarded connection, and the contained connection if there is
   one */
ForConnection::~ForConnection()
{
    if (_body)
        delete _body;
}

/* Set the forwarded connection body to "conn" */
void ForConnection::Body(IPConnection* conn)
{
    if (_body)
        delete _body;
    _body = conn;
    if (translator())
        _body->set_id_translator(translator());
}

/* forwarded routines */

int ForConnection::FD() const
{
    if (_body)
        return _body->FD();
    else
        return -1;
}

const char* ForConnection::Name() const
{
    if (_body)
        return _body->Name();
    else
        return IPConnection::Name();
}

const char* ForConnection::Host() const
{
    if (_body)
        return _body->Host();
    else
        return NULL;
}

int ForConnection::Active()
{
    if (_body)
        return _body->Active();
    else
        return 0;
}

int ForConnection::Viable()
{
    if (_body)
        return _body->Viable();
    else
        return 0;
}

int ForConnection::Send(IPMessage* msg)
{
    if (_body) {
        return _body->Send(msg);
    } else
        return -1;
}

IPMessage* ForConnection::Receive()
{
    if (_body)
        return _body->Receive();
    else
        return NULL;
}

int ForConnection::DataAvailable()
{
    if (_body)
        return _body->DataAvailable();
    else
        return 0;
}

IPMessage* ForConnection::GetMessage(IPConnection*)
{
    if (_body) {
        IPMessage* msg = _body->GetMessage(this);
        return msg;
    } else
        return NULL;
}

void ForConnection::Activate(int sd, const char* host)
{
    if (_body)
        _body->Activate(sd, host);
}

void ForConnection::activate(const char* host, IPConnection* conn)
{
    if (_body)
        _body->activate(host, conn);
}

void ForConnection::Deactivate()
{
    if (_body)
        _body->Deactivate();
}

void ForConnection::Allocate(int b)
{
    if (_body)
        _body->Allocate(b);
}

void ForConnection::Free(int b)
{
    if (_body)
        _body->Free(b);
}

void ForConnection::Set(int b)
{
    if (_body)
        _body->Set(b);
}

int ForConnection::Allocated() const
{
    if (_body)
        return _body->Allocated();
    else
        return 0;
}

int ForConnection::ByteOrder() const
{
    if (_body)
        return _body->ByteOrder();
    else
        return IPConnection::ByteOrder(); 
}

int ForConnection::Alignment() const
{
    if (_body)
        return _body->Alignment();
    else
        return IPConnection::Alignment();
}


void ForConnection::SetByteOrder(int b)
{
    if (_body)
        _body->SetByteOrder(b);
}

void ForConnection::SetAlignment(int a)
{
    if (_body)
        _body->SetAlignment(a);
}

int ForConnection::translate_id(int id)
{
    if (_body)
        return _body->translate_id(id);
    else
        return id;
}

void ForConnection::set_id_translator(IPTranslator* t)
{
    IPConnection::set_id_translator(t);
    if (_body)
        _body->set_id_translator(t);
}

int ForConnection::ConnectionNumber() const
{
    if (_body)
        return _body->ConnectionNumber();
    else
        return IPConnection::ConnectionNumber();
}

