///////////////////////////////////////////////////////////////////////////////
//
//                                 message.cc
//
// This file implements the message class
//
// Classes defined for export:
//    IPMessage
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

#ifndef VXWORKS
#include "./libc.h"
#endif

#include <ipt/message.h>
#include <ipt/connection.h>
#include <ipt/messagetype.h>
#include <ipt/ipt.h>
#include <ipt/format.h>

/* Create a message with an type ID, and instance, and some data.  If
   data is 0 it allocates size bytes, otherwise it uses data as passed `
   Occasionally a message with a type that has not been registered will be
   created, that's why there is the option for creating a message with just a
   type ID.  This sets the _type field to 0. */
IPMessage::IPMessage(int type, int instance, int size, unsigned char* data)
{
    _id = type;
    _instance = instance;
    _size = size;
    _formatted_contents = 0;
    _formatted_data = NULL;

    _type = NULL;

    if (data) {
        _allocated = 0;
        _data = data;
    } else {
        _data = new unsigned char[size];
        _allocated = 1;
    }

    _connection = 0;
}

/* Create a message with a type, and instance, and some data.  If
   data is 0 it allocates size bytes, otherwise it uses data as passed */
IPMessage::IPMessage(IPMessageType* type, int instance, int size,
                     unsigned char* data)
{
    _instance = instance;
    _size = size;
    _type = type;
    if (_type)
        _id = type->ID();
    else
        _id = -1;

    if (data) {
        _allocated = 0;
        _data = data;
    } else {
        _data = new unsigned char[size];
        _allocated = 1;
    }

    _formatted_contents = 0;
    _formatted_data = NULL;
    _connection = 0;
}

/* Create a message with formatted data */
IPMessage::IPMessage(IPMessageType* type, int instance, void* formatted)
{
    _instance = instance;
    _type = type;
    if (_type)
        _id = type->ID();
    else
        _id = -1;

    if (!type) {
        fprintf(stderr,
                "IPT: Cannot create a message by type without a type\n");
        exit(-1);
    } else
        _allocated = type->CachedUnformatData(formatted, _size, _data);

    _formatted_contents = 0;
    _formatted_data = NULL;
    _connection = 0;
}

/* Create a message with the same data, type, and instance as another */
IPMessage::IPMessage(const IPMessage& msg)
{
    _type = msg._type;
    _instance = msg._instance;
    _id = msg._id;
    _connection = msg._connection;

    _allocated = 1;
    _size = msg._size;
    _data = new unsigned char[_size];
    bcopy((char*) msg._data, (char*) _data, _size);
    _formatted_contents = 0;
    _formatted_data = NULL;
}

/* deletes the memory associated with a message, if it was allocated in the
   creation routine in the first place */
IPMessage::~IPMessage()
{
    if (_type) {
        if (_formatted_data) {
            if (_formatted_contents) {
                _type->DeleteContents(_formatted_data, _data, _size);
            } else
                _type->DeleteFormattedData(_formatted_data, _data, _size);
        }
        if (!_allocated)
            _type->ReleaseCache(_data);
    }
    
    if (_allocated && _data)
    {
        // ENDO - MEM LEAK
        //delete _data;
        delete [] _data;
        _data = NULL; // ENDO
    }
}

/* Return the message ID. */
int IPMessage::ID() const
{
    return _id;
//    return _type ? _type->ID() : _id;
}

/* Create formatted datad from the data in the message and return it, if
   there is a type and the type has a message formatter */
void* IPMessage::FormattedData(int force_copy)
{
    if (force_copy) 
        return _type->FormatData(_data, 0, _connection);
    if (_formatted_data) {
        if (!_formatted_contents)
            return _formatted_data;
        _type->DeleteContents(_formatted_data, _data, _size);
        _formatted_data = NULL;
        _formatted_contents = 0;
    }
        
    if (_type) {
        _formatted_data = _type->FormatData(_data, _size, _connection);
        _formatted_contents = 0;
    }

    return _formatted_data;
}

/* Fill "data" with the formatted data gotten using the type's message 
   formatter (if there is one) on the messages data */
void IPMessage::FormattedData(void* data, int force_copy)
{
    if (force_copy) {
        _type->FormatData(_data, 0, data, _connection);
        return;
    }

    if (_formatted_data) {
        if (!_formatted_contents) {
            bcopy((char*) _formatted_data, (char*) data,
                  _type->Formatter()->dataStructureSize());
            return;
        } else
            _type->DeleteContents(_formatted_data, _data, _size);
    }
    
    if (_type) {
        _type->FormatData(_data, _size, data, _connection);
        _formatted_data = data;
        _formatted_contents = 1;
    }
}

/* Delete the formatted data created by this message's type's formatter */
void IPMessage::DeleteFormatted(void* data)
{
    if (_type) {
        _type->DeleteFormattedData(data, _data, _size);
        _formatted_data = NULL;
        _formatted_contents = 0;
    }
}

/* Delete the contents of the formatted data created by this message's type's
   formatter */
void IPMessage::DeleteContents(void* data)
{
    if (_type) {
        _type->DeleteContents(data, _data, _size);
        _formatted_data = NULL;
        _formatted_contents = 0;
    }
}

/* Encode a message with a message type */
IPMessageType* IPMessage::EncodeType(IPCommunicator* comm)
{
    if (!_type)
        _type = comm->LookupMessage(_id);

    return _type;
}

/* Unformatted message data can be set through this method.  If size
   is greater than the current size, the old data is deleted and 
   recreated.  If data is non-null, size bytes of it are copied into
   the current data buffer.  The current data buffer is returned */
unsigned char* IPMessage::SetData(int size, unsigned char* data)
{
    if (_formatted_data) {
        if (_formatted_contents) {
            _type->DeleteContents(_formatted_data, _data, _size);
            _formatted_contents = 0;
        } else
            _type->DeleteFormattedData(_formatted_data, _data, _size);
        _formatted_data = NULL;
    }

    if (size > _size) {
        if (_data) delete _data;
        _data = new unsigned char[size];
    }

    _size = size;
    if (data)
        bcopy((char*) data, (char*) _data, size);

    return _data;
}

int IPMessage::Print(int print_data)
{
    if (_type) 
        printf("Message %s: ", _type->Name());
    else
        printf("Message# %d: ", _id);

    printf("Instance# %d, Size %d", _instance, _size);
    if (_connection)
        printf(", Connection %s\n", _connection->Name());
    else
        printf("\n");

    if (print_data && _type && _type->Formatter()) {
        void* formatted = FormattedData();
        _type->Formatter()->PrintData(formatted);

        return 1;
    } else
        return 0;
}
