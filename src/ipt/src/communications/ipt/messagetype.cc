///////////////////////////////////////////////////////////////////////////////
//
//                                 messagetype.cc
//
// This file implements the class that holds a message's type information, 
// i.e., whether (and how) it is handled, whether (and how) it is formatted,
// what its destination is, what its type name is, etc.
//
// Classes defined for export:
//    IPMessageType
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

#include <ipt/messagetype.h>
#include <ipt/format.h>
#include <ipt/callbacks.h>
#include <ipt/list.h>
#include <ipt/connection.h>

#include "./libc.h"

/* Create a message type with name "name", format "format" and message ID
   "id" */
IPMessageType::IPMessageType(const char* name, IPFormat* format, int id)
{
    int len = strlen(name)+1;
    
    _name = new char[len];
    bcopy(name, _name, len);
    _id = id;
    _format = format;
    _handler_active = 1;
    _handler_invoked = 0;
    _postponed_messages = IPList::Create();
    _destination = NULL;
    _callback = NULL;
    _handler_context = 0;
    _size_cache = _cache_in_use = 0;
    _cache = NULL;
}

/* delete a message type */
IPMessageType::~IPMessageType()
{
    delete _name;
    IPList::Delete(_postponed_messages);

    IPResource::unref(_format);

    if (_callback) {
        IPResource::unref(_callback);
    	_callback = NULL;
	}
    if (_cache)	{
        delete _cache;
    	_cache = NULL;
	}
}

/* take unformatted data "input", and create formatted data and return it.
   If there is no formatter for this type, return NULL */
void* IPMessageType::FormatData(unsigned char* input, int size_input,
                                IPConnection* conn)
{
    if (_format) {
        if (!input) {
            fprintf(stderr, "IPT Fatal Error: Message type %s expects some data\n",
                    _name);
            exit(-1);
        }
        if (conn)
            return _format->decodeData((char*) input, 0, size_input, NULL,
                                       conn->ByteOrder(), conn->Alignment());
        else
            return _format->decodeData((char*) input, 0, size_input, NULL,
                                       BYTE_ORDER, ALIGN);
    } else return NULL;
}

/* take unformatted data "input", and create formatted data and return it.
   If there is no formatter for this type, return NULL */
void IPMessageType::FormatData(unsigned char* input, int size_input,
                               void* output, IPConnection* conn)
{
    if (_format) {
        if (!input) {
            fprintf(stderr, "IPT Fatal Error: Message type %s expects some data\n",
                    _name);
            exit(-1);
        }
        if (conn)
            _format->decodeData((char*) input, 0, size_input, (char*) output,
                                conn->ByteOrder(), conn->Alignment());
        else
            _format->decodeData((char*) input, 0, size_input, (char*) output,
                                BYTE_ORDER, ALIGN);
    } 
}

/* Take formatted data "input" and create unformatted output buffer "output", 
   copy the data into it, and put the number of bytes in "output_size".
   If formatter is NULL, do nothing */
int IPMessageType::UnformatData(void* input,
                                int& output_size, unsigned char*& output)
{
    if (!_format) {
        output_size = 0;
        output = NULL;
        return 0;
    }
    output_size = _format->bufferSize((char*) input);
    if (_format->sameFixedSizeDataBuffer()) {
        output = (unsigned char*) input;
        return 0;
    } else {
        output = new unsigned char[output_size];
        _format->encodeData((char*) input, (char*) output, 0);
        return 1;
    }
}

/* Take formatted data "input" and create unformatted output buffer "output", 
   copy the data into it, and put the number of bytes in "output_size".
   If formatter is NULL, do nothing */
int IPMessageType::CachedUnformatData(void* input,
                                      int& output_size, unsigned char*& output)
{
    if (!_format) {
        output_size = 0;
        output = NULL;
        return 0;
    }
    output_size = _format->bufferSize((char*) input);
    if (_format->sameFixedSizeDataBuffer()) {
        output = (unsigned char*) input;
        return 0;
    } else {
        int allocated = 0;
        output = Cache(output_size);
        if (!output) {
            output = new unsigned char[output_size];
            allocated = 1;
        }
        _format->encodeData((char*) input, (char*) output, 0);
        return allocated;
    }
}

/* Take formatted data "input", unformat it into bytes, and copy the result
   into "output", which had better have the right number of bytes.  If there
   is no formatter for this type, do nothing */
void IPMessageType::RawUnformatData(void* input, unsigned char* output)
{
    if (_format)
        _format->encodeData((char*) input, (char*) output, 0);
}

/* Delete the formatted data "data" according to the type's message formatter,
   if there is one */
void IPMessageType::DeleteFormattedData(void* data,
                                        unsigned char* buffer, int bsize)
{
    if (_format)
        _format->freeDataStructure((char*) data, (char*) buffer, bsize);
}

void IPMessageType::DeleteContents(void* data,
                                   unsigned char* buffer, int bsize)
{
    if (_format)
        _format->freeDataElements((char*) data, 0, (char*) buffer, bsize,
                                  NULL, 0);
}

/* Set the message type's handler to "callback" and the context to "context" */
void IPMessageType::Handler(IPHandlerCallback* callback, int context)
{
    if (_callback)
        IPResource::unref(_callback);
    _callback = callback;
    IPResource::ref(_callback);
    _handler_context = context;
}

/* Postpone message "msg" */
void IPMessageType::Postpone(IPMessage* msg)
{
    _postponed_messages->Append((char*) msg);
}

/* Get the earliest postponed message, remove it from the postponed list */
IPMessage* IPMessageType::Postponed()
{
    return (IPMessage*) _postponed_messages->Pop();
}

void IPMessageType::SetFormat(IPFormat* fmt)
{
    if (!fmt)
        return;

    if (_format && _format != fmt)
        IPResource::unref(_format);
    _format = fmt;
}

unsigned char* IPMessageType::Cache(int num_bytes)
{
    if (_cache_in_use)
        return NULL;

    _cache_in_use = 1;

    if (num_bytes <= _size_cache) 
        return _cache;
    if (_cache)
        delete [] _cache;
    _cache = new unsigned char[num_bytes];
    _size_cache = num_bytes;

    return _cache;
}

void IPMessageType::ReleaseCache(unsigned char* cache)
{
    if (_cache_in_use && _cache == cache)
        _cache_in_use = 0;
}

int IPMessageType::GetCache(int& num_bytes, unsigned char*& cache)
{
    num_bytes = _size_cache;
    cache = _cache;
    return 1;
}
