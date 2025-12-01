///////////////////////////////////////////////////////////////////////////////
//
//                                 filter.h
//
// This header file defines classes that can be used to filter messages in
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

#ifndef ip_filter_h
#define ip_filter_h

class IPConnection;
class IPMessageType;
class IPList;
class IPMessage;

/* base class for filtering.  Basically the virtual function Check returns
   1 if the message from the connection should be accepted, 0 if not */
class IPFilter {
  public:
    virtual int Check(IPMessage*, IPConnection*) = 0;
    virtual int Valid() { return 1; }
};

/* If message and connection are non-NULL, check to make sure instance message 
   type and connection match.  If message type is NULL, any message type will 
   do, if connection is NULL, any connectino will do */
class MsgFilter : public IPFilter {
  public:
    MsgFilter(IPMessageType* t, IPConnection* c= 0);
    MsgFilter() { _type = NULL; _conn = NULL; _old_conn_num = -1; }

    virtual int Check(IPMessage* m, IPConnection* c);
    virtual int Valid();

  private:
    IPConnection* _conn;
    int _old_conn_num;
    IPMessageType* _type;
};

/* Filter to check if the type, connection, and instance of the message to
   check match the parameters.  If connection and type are NULL, then they are
   subject to the same criteria as in MsgFilter */
class InstanceFilter : public IPFilter {
  public:
    InstanceFilter(int i, IPConnection* c, IPMessageType* type)
        { _instance = i; _conn = c; _type = type; }
    InstanceFilter() { _instance = -1; _conn = 0; _type = 0; }

    virtual int Check(IPMessage* m, IPConnection*);
    virtual int Valid();

  private:
    int _instance;
    IPConnection* _conn;
    IPMessageType* _type;
};

/* Used to do an And of several filters.  It maintains a list of filters 
   joined together in a list by the Add member function, and a message must
   pass all of the sub filters in order to be Checked positively */
class CompositeFilter {
  public:
    CompositeFilter();
    virtual ~CompositeFilter();

    void Add(IPFilter*);

    virtual int Check(IPMessage* m, IPConnection* c);
    virtual int Valid();

  private:
    IPList* _filters;
};

#endif
