///////////////////////////////////////////////////////////////////////////////
//
//                                 connection.cc
//
// This file implements classes used to define connections between modules
// using IPT.  Messages are sent across and received through these connections.
// Every connection has associated with it the name of the module that the
// connection goes to, the machine that that module is on, and a file 
// descriptor.  This file descriptor may or may not be relevant for every class
// of connection.
//
// Classes defined for export:
//    IPConnection, FDConnection, TCPConnection, UnixConnection, FakeConnection
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
#include <errno.h>

#ifdef VXWORKS
#include <ioLib.h>
#else
#include "./libc.h"
#endif

#include <ipt/list.h>
#include <ipt/callbacks.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>
#include <ipt/connection.h>
#include <ipt/ipt.h>
#include <ipt/primcomm.h>
#include <ipt/destination.h>
#include <ipt/hash.h>
#include <ipt/pigeonhole.h>
#include <ipt/fdconnection.h>
#include <ipt/internal_messages.h>
#include <ipt/format.h>
#include <ipt/fakeconnection.h>
#include <ipt/translator.h>
#include <ipt/form_macros.h>

/* Used as an iterator to delete all routers when deleting the routing hash 
   table */
static int delete_pigeon_holes(char*, char* data)
{
	delete (IPPigeonHole*) data;

    return 1;
}

/* Used as an iterator to delete all callbacks when deleting the list */
static int delete_callbacks(char*, char* data)
{
	if(data) {
        IPResource::unref((IPConnectionCallback*) data);
		data = NULL;
	}

    return 1;
}

/* returns true if two messages ID's are the same, used in message hashing */
static int message_type_eq(const char* t1, const char* t2)
{
    return ((*(IPMessageType**) t1)->ID() == (*(IPMessageType**) t2)->ID());
}

/* hash functions for messages, simply the message ID */
static int message_type_hash(const char* t)
{
    return (*(IPMessageType**) t)->ID();
}

int IPConnection::_last_connection_number = 0;

/* Create a connection between this module and the module named "name" on
   machine "host" using IPCommunicator "comm" and file descriptor "fd".
   The type of the connection is an arbitrary string that should be constant
   and unique for each sub-class */
IPConnection::IPConnection(const char* type,
                           const char* name, const char* host,
                           IPCommunicator* comm, int fd)
{
    int len;
    if (name) {
        len = strlen(name) + 1;
        _name = new char[len];
        bcopy((char*) name, _name, len);
    } else
        _name = NULL;

    if (host) {
        len = strlen(host) + 1;
        _host = new char[len];
        bcopy((char*) host, _host, len);
    } else
        _host = NULL;

    _connection_number = _last_connection_number++;
    _fd = fd;
    _communicator = comm;
    _connect_callbacks = IPList::Create();
    _disconnect_callbacks = IPList::Create();
    _pigeon_holes = new IPHashTable(10, message_type_hash, message_type_eq);
    _backed_up = 0;
    _type = type;
    _byte_order = BYTE_ORDER_UNDEFINED;
    _alignment = ALIGN_UNDEFINED;
    _id_translator = NULL;
}

/* Delete a connection */
IPConnection::~IPConnection()
{
    IPConnection* conn = _communicator->LookupConnection(Name());
    if (!conn)
        conn = this;

    // ENDO - MEM LEAK
    /*
    if (_name)
        delete _name;
    if (_host)
        delete _host;
    */
    if (_name)
    {
        delete [] _name;
        _name = NULL;
    }

    if (_host)
    {
        delete [] _host;
        _host = NULL;
    }


    _pigeon_holes->Iterate(delete_pigeon_holes);
	delete _pigeon_holes;

    _connect_callbacks->Iterate(delete_callbacks, (char*) conn);
    IPList::Delete(_connect_callbacks);

    _disconnect_callbacks->Iterate(delete_callbacks, (char*) conn);
    IPList::Delete(_disconnect_callbacks);
}

/* Invoke a connectin callback.  Used with IPList */
static int invoke_callback(char* conn, char* callback)
{
    ((IPConnectionCallback*) callback)->Execute((IPConnection*) conn);

    return 1;
}

/* Add a callback to the connection callbacks.  When a module connects 
   and activates or reactivates this connection, the routine defined in
   callback will be called */
void IPConnection::AddConnectCallback(IPConnectionCallback* callback)
{
    _connect_callbacks->Prepend((char*) callback);
    IPResource::ref(callback);
}

void IPConnection::AddConnectCallback(void (*func)(IPConnection*, void*),
                                        void* data)
{
    AddConnectCallback(new IPRoutineConnectionCallback(func, data));
}

/* Remove a callback from the connection callbacks */
void IPConnection::RemoveConnectCallback(IPConnectionCallback* callback)
{
    _connect_callbacks->Remove((char*) callback);
    IPResource::ref(callback);
}

/* Invoke all of the connectino callbacks for a connection */
void IPConnection::InvokeConnectCallbacks()
{
    IPConnection* conn = _communicator->LookupConnection(Name());
    if (!conn)
        conn = this;
    
    _connect_callbacks->Iterate(invoke_callback, (char*) conn);
}

/* Add a callback to the disconnetion callbacks.  When a module deactivates
   this connection, call the routine defined in the callback */
void IPConnection::AddDisconnectCallback(IPConnectionCallback* callback)
{
    _disconnect_callbacks->Prepend((char*) callback);
    IPResource::ref(callback);
}

void IPConnection::AddDisconnectCallback(void (*func)(IPConnection*, void*),
                                        void* data)
{
    AddDisconnectCallback(new IPRoutineConnectionCallback(func, data));
}

/* Remove a callback from the disconnectino callbacks */
void IPConnection::RemoveDisconnectCallback(IPConnectionCallback* callback)
{
    _disconnect_callbacks->Remove((char*) callback);
    IPResource::unref(callback);
}

/* Invoke all of the disconnection callbacks */
void IPConnection::InvokeDisconnectCallbacks()
{
    if (!_communicator->status_disconnect_handlers())
        return;

    IPConnection* conn = _communicator->LookupConnection(Name());
    if (!conn)
        conn = this;

    _disconnect_callbacks->Iterate(invoke_callback, (char*) conn);
}

/* Receive a message over the connection.  If a message is received, process
   it through to the message types's destination.  If the message type has
   no destination, process it through the connection's communicator's default
   destination */
IPMessage* IPConnection::GetMessage(IPConnection* set_conn)
{
    IPMessage* msg = Receive();
    if (!msg)
        return NULL;

    if (!set_conn)
        msg->Connection(this);
    else
        msg->Connection(set_conn);

    IPMessageType* type = msg->Type();
    int res;
    if (!type || !type->Destination())
        res = Communicator()->Destination()->ProcessMessage(this, msg);
    else 
        res = type->Destination()->ProcessMessage(this, msg);

    if (!res)
        return NULL;
    else
        return msg;
}

/* Deactivate a connection, calling all pertinent callbacks 
   Note that just calling this is usually not effective, you should call
   IPCommunicator::Deactivate so that the communicator bookkeeping can be
   done */
void IPConnection::Deactivate()
{
    _communicator->printf("Deactivating connection %s\n", _name);
    _communicator->disconnect_notify(this);
}

/* Create (or find) a pigeon hole for message type "type" in this connection */
IPPigeonHole* IPConnection::PigeonHole(IPMessageType* type)
{
    IPPigeonHole* res = (IPPigeonHole*) _pigeon_holes->Find((char*) &type);
    if (res)
        return res;
    res = new IPPigeonHole(type);
    _pigeon_holes->Insert((char*) &type, sizeof(IPMessageType*), (char*) res);

    return res;
}

/* Set (or reset) the connection's name to "name" */
void IPConnection::SetName(const char* name)
{
    if (_name)
        delete _name;
    
    int len = strlen(name) + 1;
    _name = new char[len];
    bcopy((char*) name, _name, len);
}

/* Activate a connection with a new file descriptor and
   host name.  If the connection is currently active, deactivate it first,
   unless the new file descriptor and host match the old, in which case do
   nothing */
void IPConnection::Activate(int fd, const char* host)
{
    if ((_fd == fd) && !strcmp(_host, host))
        return;
        
    if (_fd != -1)
        Communicator()->DeactivateConnection(this);
    _fd = fd;
    if (_host)
        delete _host;
    int len = strlen(host)+1;
    _host = new char[len];
    bcopy(host, _host, len);
}

void IPConnection::activate(const char* host, IPConnection* conn)
{
    if ((_fd == conn->_fd) && !strcmp(_host, host))
        return;
        
    if (_fd != -1)
        Communicator()->DeactivateConnection(this);

    _fd = conn->_fd;
    if (_host)
        delete _host;
    int len = strlen(host)+1;
    _host = new char[len];
    bcopy(host, _host, len);
}

int IPConnection::translate_id(int id)
{
    if (!_id_translator || id < LOWEST_MESSAGE_NUMBER)
        return id;
    int res = _id_translator->translation(id);
    if (!res)
        return Communicator()->add_translator(id, this);

    return res;
}

/* Create a file descriptor based connection to the module named "name" on 
   machine "host" with communicator "comm" file descriptor.  Type will be
   "type" */
FDConnection::FDConnection(const char* type, const char* name,
                           const char* host, IPCommunicator* comm, int fd)
    : IPConnection(type, name, host, comm, fd)
{
    _bytes_in_buffer = 0;
	bzero((char *)_in_buffer,BUF_SIZE); 
}

/* Write the data of message "msg" to the socket with descriptor "fd" using
   output buffer "buffer" (which has a constant size of BUF_SIZE bytes) */
static int write_message(IPMessage* msg, int id, int fd)
{
    struct iovec iov[3];
    int buffer[3];
    int iocnt = 1;

    // ENDO - MEM LEAK
    memset(iov, 0x0, sizeof(iovec)*3); // Added by ENDO

    buffer[0] = INT_TO_NET_INT(id);
    buffer[1] = INT_TO_NET_INT(msg->Instance());
    buffer[2] = INT_TO_NET_INT(msg->SizeData());
    iov[0].iov_base = (caddr_t) buffer;
    iov[0].iov_len = 3*sizeof(int);

    if (id == -1 && msg->Type()) {
        iov[1].iov_base = (caddr_t) msg->Type()->Name();
        iov[1].iov_len = strlen(msg->Type()->Name())+1;
        iocnt = 2;
    }

    iov[iocnt].iov_base = (caddr_t) msg->Data();
    iov[iocnt].iov_len = msg->SizeData();
    iocnt++;

    return writev(fd, iov, iocnt);
}

/* Send the data associated with "msg" out across the connection's socket to
   the other module */
int FDConnection::Send(IPMessage* msg)
{
    if (!Active()) {
        _communicator->printf("FDConnection %s: sending to inactive connection\n", Name());
        return -1;
    }
    int res = write_message(msg, translate_id(msg->ID()), _fd);

    if (res < 0) {
        _communicator->printf("FDConnection %s: Connection deactivated during send\n",
               Name());
        Communicator()->DeactivateConnection(this);
    }

    return res;
}

/* Receive a message across the connection's socket and return it.  This is
   the tricky part of message transaction for FD connections */
IPMessage* FDConnection::Receive()
{
    unsigned char* ptr;

    /* first check to see if there is data already in the input buffer */
    if (_bytes_in_buffer >= int(3*sizeof(int))) {
        ptr = _in_buffer + _buffer_index;
    } else {  /* no? get some from the socket */
        while (_bytes_in_buffer < int(3*sizeof(int))) {
            if (_bytes_in_buffer) {
                /* if data needs to be gotten, but there is already data in the
                   buffer, move the data to the beginning of the buffer before
                   reading */
                if (_buffer_index) {
                    bcopy((char*) _in_buffer+_buffer_index, (char*) _in_buffer,
                          _bytes_in_buffer);
                    _buffer_index = 0;
                }
                int count = read(_fd, (char*) _in_buffer+_bytes_in_buffer,
                                 BUF_SIZE-_bytes_in_buffer);
                if (count < 0) {
                    Communicator()->DeactivateConnection(this);
                    return NULL;
                }

                _bytes_in_buffer += count;
            } else {
                /* otherwise, just read right in */
                _bytes_in_buffer = read(_fd, (char*) _in_buffer, BUF_SIZE);
                _buffer_index = 0;
                if (_bytes_in_buffer <= 0) {
                    Communicator()->DeactivateConnection(this);
                    return NULL;
                }
            }
        }
        ptr = _in_buffer;
    }

    /* Get the message header from the input buffer */
    int id, instance, size;
    bcopy((char*) ptr, (char*) &id, sizeof(int));
    ptr += sizeof(int);
    bcopy((char*) ptr, (char*) &instance, sizeof(int));
    ptr += sizeof(int);
    bcopy((char*) ptr, (char*) &size, sizeof(int));
    ptr += sizeof(int);
    _buffer_index += 3*sizeof(int);
    _bytes_in_buffer -= 3*sizeof(int);

    id = NET_INT_TO_INT(id);
    instance = NET_INT_TO_INT(instance);
    size = NET_INT_TO_INT(size);

    /* Create the message whose header we have read.  This can be
       done using destinations to make things like pigeonholes work
       very efficiently */
    IPMessage* msg;
    if (_communicator) {
        IPMessageType* type;
        if (id == -1) {
            const char* msg_name;
            int len;
            if (read_string(msg_name, len)) {
                type = _communicator->LookupMessage(msg_name);
                if (!type)
                    printf("IPT: Unknown message type '%s' received\n",
                           msg_name);
                _buffer_index += len;
                ptr = _in_buffer + _buffer_index;
                _bytes_in_buffer -= len;
            } else {
                printf("IPT: Invalid message name received\n");
                type = NULL;
            }
        } else
            type = _communicator->LookupMessage(id);

        if (type) {
            IPDestination* dest = type->Destination();
            if (!dest)
                dest = Communicator()->Destination();
            msg = dest->MakeMessage(this, type, instance, size);
        } else
            msg = new IPMessage(id, instance, size);
    } else
        msg = new IPMessage(id, instance, size);
    unsigned char* dest = msg->Data();

    /* if all the message data is in the buffer, just copy and return the
       new message */
    if (size < _bytes_in_buffer) {
        bcopy((char*) ptr, (char*) dest, size);
        _buffer_index += size;
        _bytes_in_buffer -= size;

        return msg;
    }

    /* other wise, get all data in the buffer, and... */
    bcopy((char*) ptr, (char*) dest, _bytes_in_buffer);
    dest += _bytes_in_buffer;
    size -= _bytes_in_buffer;
    _bytes_in_buffer = 0;
    
    /* read in the rest straight into the message data */
    int count;
    while (size) {
        count = read(_fd, (char*) dest, size);
        if (count <= 0) {
            if (errno == EINTR)
                continue;
            Communicator()->DeactivateConnection(this);
            delete msg;
            return NULL;
        }

        size -= count;
        dest += count;
    }

    return msg;
}

int FDConnection::read_string(const char*& msg_name, int& len)
{
    msg_name = (char*) _in_buffer + _buffer_index;
    if (_bytes_in_buffer) {
        const char* ptr = msg_name;
        len = 0;
        for (; len++ < _bytes_in_buffer;  ptr++)
            if (!*ptr)
                return 1;

        bcopy((char*) _in_buffer+_buffer_index, (char*) _in_buffer,
              _bytes_in_buffer);
    } else 
        len = 1;
    
    _buffer_index = 0;
    msg_name = (char*) _in_buffer;

    int count = read(_fd, (char*) _in_buffer+_bytes_in_buffer,
                     BUF_SIZE-_bytes_in_buffer);
    if (count < 0) {
        Communicator()->DeactivateConnection(this);
        return 0;
    }

    const char* ptr = (char*) _in_buffer+_bytes_in_buffer;
    _bytes_in_buffer += count;
    for (; len <= _bytes_in_buffer; ptr++, len++)
        if (!*ptr)
            return 1;

    return 0;
}

/* Deactivate a FD connection.  Note that just calling this is usually not
   effective, you should call IPCommunicator::Deactivate so that the 
   communicator bookkeeping can be done */
void FDConnection::Deactivate()
{
    if (_fd != -1) {
        close_socket(_fd);

        IPConnection::Deactivate();
        _fd = -1;
    }
}
    
int FDConnection::Active()
{
    return (_fd != -1);
}

int FDConnection::Viable()
{
    return ((_fd != -1) && !socket_is_closed(_fd));
}

/* Checks the internal buffer and the socket for available data across the
   connection.  Returns true if there is data available */
int FDConnection::DataAvailable()
{
    return ((_bytes_in_buffer != 0) ||
            ((_fd != -1) && 
             (Allocated() <= MAX_BACKUP) &&
             data_at_socket(_fd) > 0));
}

/* Set the data pending in this connection to be the same as the data pending
   in connection "conn." */
void FDConnection::SetData(FDConnection* conn)
{
    if (conn->_bytes_in_buffer) {
        _bytes_in_buffer = conn->_bytes_in_buffer;
        _buffer_index = conn->_buffer_index;
        bcopy((char*) conn->_in_buffer+_buffer_index,
              (char*) _in_buffer+_buffer_index, _bytes_in_buffer);
    }
}

void FDConnection::activate(const char* host, IPConnection* conn)
{
    IPConnection::activate(host, conn);
    SetData((FDConnection*) conn);
}

TCPConnection::TCPConnection(const char* name,
                             const char* host, IPCommunicator* comm, int fd)
    : FDConnection("TCP", name, host, comm, fd)
{}

UnixConnection::UnixConnection(const char* name,
                             const char* host, IPCommunicator* comm, int fd)
    : FDConnection("Unix", name, host, comm, fd)
{}

FakeConnection::FakeConnection(const char* name, IPCommunicator* comm, int fd)
    : IPConnection("Fake", name, "Unknown", comm, fd)
{}

FakeConnection::~FakeConnection()
{
    InvokeDisconnectCallbacks();
}

int FakeConnection::Active()
{
    return FD() != -1;
}

int FakeConnection::Send(IPMessage*)
{
    _communicator->printf("IPT: Cannot send to fake connection\n");

    return 0;
}

IPMessage* FakeConnection::Receive()
{
    _communicator->printf("IPT: Cannot receive from fake connection\n");

    return NULL;
}

int FakeConnection::DataAvailable()
{
    return ((_fd != -1) && (data_at_socket(_fd) > 0));
}
