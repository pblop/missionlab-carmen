///////////////////////////////////////////////////////////////////////////////
//
//                                 filter.h
//
// This file implements classes that can be used to filter messages in
// IPCommunicator::ReceiveMessage
//
// Classes defined for export:
//    IPFilter, MsgFilter, InstanceFilter, CompositeFilter
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

#include <ipt/list.h>
#include <ipt/filter.h>
#include <ipt/connection.h>
#include <ipt/message.h>
#include <ipt/messagetype.h>

/* check message m from connection c against the stored mesage type and 
   connection.  If they match then return 1.  If the stored message type is
   NULL, any message type will do.  If the stored connection is NULL, any
   connection will do.  If both are NULL any message type or connection will
   do. */
int MsgFilter::Check(IPMessage* m, IPConnection* c)
{
    if (m == NULL || (_conn == NULL && _type == NULL))
        return 1;

    if (_type==NULL) 
        return (c == _conn);
    if (_conn == NULL)
        return (m->Type() == _type);

    return ((c == _conn) && (m->Type() == _type));
}

MsgFilter::MsgFilter(IPMessageType* t, IPConnection* c)
{
    _type = t;
    _conn = c;
    if (c)
        _old_conn_num = c->ConnectionNumber();
    else
        _old_conn_num = -1;
}

int MsgFilter::Valid()
{
    return (!_conn ||
            (_conn->Active() && _conn->ConnectionNumber() == _old_conn_num));
}

/* Checks for an instance number in "m" that matches the prestored instance
   number.  If the stored connection and message type are non-NULL, it checks
   to make sure that they match as well. */
int InstanceFilter::Check(IPMessage* m, IPConnection* c)
{
    return ((_conn == NULL || c == _conn) &&
            (_type == NULL || m->ID() == _type->ID()) &&
            (m->Instance() == _instance));
}

int InstanceFilter::Valid()
{
    return (!_conn || _conn->Active());
}

/* Create a composite filter */
CompositeFilter::CompositeFilter()
{
    _filters = IPList::Create();
}

/* Delete a composite filter */
CompositeFilter::~CompositeFilter()
{
    IPFilter* cur;
    for (cur = (IPFilter*) _filters->First(); cur;
         cur = (IPFilter*) _filters->Next())
        delete cur;
    
    IPList::Delete(_filters);
}

/* Add filter "filter" to the list of filters to be composited */
void CompositeFilter::Add(IPFilter* filter)
{
    _filters->Prepend((char*) filter);
}

/* Returns 1 if all the sub-filters return 1 */
int CompositeFilter::Check(IPMessage* m, IPConnection* c)
{
    IPFilter* cur;

    for (cur = (IPFilter*) _filters->First(); cur;
         cur = (IPFilter*) _filters->Next())
        if (!cur->Check(m, c))
            return 0;

    return 1;
}

/* Returns 1 if all the sub-filters return 1 */
int CompositeFilter::Valid()
{
    IPFilter* cur;

    for (cur = (IPFilter*) _filters->First(); cur;
         cur = (IPFilter*) _filters->Next())
        if (!cur->Valid())
            return 0;

    return 1;
}
