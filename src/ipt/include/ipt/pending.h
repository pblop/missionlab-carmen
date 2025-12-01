///////////////////////////////////////////////////////////////////////////////
//
//                                 pending.h
//
// This file delares the queued message classes used by the IPT server to
// avoid getting stuck inside message queries.
//
// Classes defined for export:
//    PendingRequest, PendingCallback, ...
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef pending_h
#define pending_h

#include <ipt/callbacks.h>

class IPMessageType;
class PendingRequest;
class ConnectionNet;
class IPServer;
class IPRoutingTable;
class IPRouting;

class PendingCallback : public IPHandlerCallback {
  public:
    PendingCallback();
    virtual ~PendingCallback();

    virtual void Execute(IPMessage*);

    virtual void pend(PendingRequest*);
    virtual void clear(IPConnection*);
    virtual void clear_all();
    virtual void query(IPConnection*, IPMessageType*, void*, PendingRequest*);

  private:
    IPList* _pending;
};

class PendingRequest {
  public:
    PendingRequest(IPConnection* target, IPMessage* message = 0);
    virtual ~PendingRequest();

    virtual void service(IPMessage*) = 0;
    virtual void clear(IPConnection*) = 0;
    virtual int related(IPConnection* conn) const { return conn == _target; }

    IPConnection* target() const { return _target; }
    IPMessage* message() const { return _message; }
    void set_message(IPMessage* msg) { _message = new IPMessage(*msg); }

  private:
    IPConnection* _target;
    IPMessage* _message;
};

class IndirectRequest : public PendingRequest {
  public:
    IndirectRequest(IPConnection* initiator, int instance,
                    IPConnection* receptor, IPMessage* msg = NULL)
        : PendingRequest(receptor, msg) { _initiator = initiator;
                                          _instance = instance; }

    virtual void service(IPMessage* msg);

    virtual int related(IPConnection* conn) const {
        return PendingRequest::related(conn) || conn == _initiator;
    }

    IPConnection* initiator() const { return _initiator; }
    int instance() const { return _instance; }

  private:
    IPConnection* _initiator;
    int _instance;
};

class RegisterRequest : public IndirectRequest {
  public:
    RegisterRequest(IPConnection* initiator, int instance,
                    IPConnection* receptor, const char* conn_name, int id,
                    IPMessage* msg = NULL);
    virtual ~RegisterRequest();
    
    virtual void service(IPMessage*);
    virtual void clear(IPConnection*);

  private:
    char* _conn_name;
    int _id;
};

#endif



