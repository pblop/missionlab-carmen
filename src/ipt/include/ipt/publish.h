///////////////////////////////////////////////////////////////////////////////
//
//                                 publish.h
//
// This header file defines classes used in publishing and subscribing.
//
// Classes defined for export:
//    IPClient, IPPublication, 
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_publish_h
#define ip_publish_h

class IPConnection;
class IPConnectionCallback;
class IPList;
class IPCommunicator;
class IPMessage;

// the default "client" structure used when a subscriber attaches.  It returns
// a "Check" of 1, meaning always send every message, and uses the standard
// SendMessage to send a message
class IPClient {
  public:
    IPClient(IPConnection* conn) { _conn = conn; }

    virtual int Check() { return 1; }
    virtual int SendMessage(IPCommunicator* comm, IPMessage* msg)
        { return comm->SendMessage(_conn, msg); }

    IPConnection* Connection() const { return _conn; }

  private:
    IPConnection* _conn;     // the client connection
};

/* A publication has a list of modules and a callback function.  The modules
   are the subscribers to a piece of information and the (optional) callback
   function will be invoked by the IPCommunicator when a new Subscriber signs
   on (i.e. is added to the list of modules */
class IPPublication {
  public:
    IPPublication(IPConnectionCallback* cb);
    virtual ~IPPublication();

    IPList* Modules() const { return _modules; }
    IPConnectionCallback* Callback() const { return _cb; }

    virtual int AddSubscriber(IPConnection* conn,
                              int=0, unsigned char* = NULL);
    virtual int RemoveSubscriber(IPConnection* conn);
    virtual void Publish(IPCommunicator* comm, IPMessage* msg);
    virtual int Check(IPConnection*, IPMessage*) { return 1; }

  protected:
    int add_subscriber(IPClient* client);

  private:
    IPList* _modules;          // list of subscribers
    IPConnectionCallback* _cb; // subscription callback 
};

#endif
