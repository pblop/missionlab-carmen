///////////////////////////////////////////////////////////////////////////////
//
//                                 event.h
//
// This header file defines classes that can be put on a communicators 
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

#ifndef ip_event_h
#define ip_event_h

class IPCommunicator;
class IPMessage;
class IPConnection;
class IPHandlerCallback;
class IPPigeonHole;

/* Class for events.  Each event has a type and a routine to handle it.
   The type member variable is mainly used for debugging, it is not very
   important */
class IPEvent {
  public:
    /* Types of events */
    enum {
        MessageEventType,
        DataEventType,
        HandlerInvokedEventType,
        PigeonHoleEventType,
        DeactivatingEventType,
        NewConnectionEventType,
        AdministrativeEventType,
        ConnectHandlerEventType,
        DisconnectHandlerEventType
    };

    IPEvent(int t) { _type = t; }
    virtual ~IPEvent() {}

    int Type() const { return _type; }

    virtual int Handle(IPCommunicator*) = 0;
  
  private:
    int _type;     // message type 
};

/* Event used when an incoming normal message is detected. 
   Note: This is used rarely, if at all */
class MessageEvent : public IPEvent {
  public:
    MessageEvent(IPMessage* m) : IPEvent(MessageEventType)
        { _message = m; };

    IPMessage* Message() const { return _message; }
    void ResetMessage() { _message = NULL; }

    virtual int Handle(IPCommunicator*);

  private:
    IPMessage* _message;
};

/* Event used when a connection is noticed to have data available */
class DataEvent : public IPEvent {
  public:
    DataEvent(IPConnection* conn) : IPEvent(DataEventType)
        { _connection = conn; }
    
    IPConnection* Connection() const { return _connection; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _connection;
};

/* Event used when a handler is to be invoked */
class HandlerInvokedEvent : public IPEvent {
  public:
    HandlerInvokedEvent(IPMessage* m, IPHandlerCallback* cb)
        : IPEvent(HandlerInvokedEventType)
            { _message = m; _callback = cb; }
    virtual ~HandlerInvokedEvent();

    virtual int Handle(IPCommunicator*);

    IPMessage* Message() const { return _message; }
    void ResetMessage() { _message = NULL; }
    IPHandlerCallback* Callback() const { return _callback; }

  private:
    IPMessage* _message;
    IPHandlerCallback* _callback;
};

/* Event used when a pigeon hole is filled */
class PigeonHoleEvent : public IPEvent {
  public:
    PigeonHoleEvent(IPPigeonHole* p) : IPEvent(PigeonHoleEventType)
        { _hole = p; }

    virtual int Handle(IPCommunicator*);

    IPPigeonHole* Hole() const { return _hole; }

  private:
    IPPigeonHole* _hole;
};

/* Event used when a connection is noticed to have data available */
class AdministrativeEvent : public IPEvent {
  public:
    AdministrativeEvent(IPConnection* conn) : IPEvent(AdministrativeEventType)
        { _connection = conn; }
    
    IPConnection* Connection() const { return _connection; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _connection;
};

class ConnectionEvent : public IPEvent {
  public:
    ConnectionEvent(IPConnection* conn) : IPEvent(ConnectHandlerEventType)
        { _connection = conn; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _connection;
};

class DisconnectionEvent : public IPEvent {
  public:
    DisconnectionEvent(IPConnection* conn):IPEvent(DisconnectHandlerEventType)
        { _connection = conn; }

    virtual int Handle(IPCommunicator*);

  private:
    IPConnection* _connection;
};

#endif
