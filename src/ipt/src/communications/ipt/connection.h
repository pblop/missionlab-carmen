///////////////////////////////////////////////////////////////////////////////
//
//                                 connection.h
//
// This header file defines the base class for connections between modules
// using IPT.  Messages are sent across and received through these connections.
// Every connection has associated with it the name of the module that the
// connection goes to, the machine that that module is on, and a file 
// descriptor.  This file descriptor may or may not be relevant for every class
// of connection.
//
// Classes defined for export:
//    IPConnection
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_connection_h
#define ip_connection_h

class IPList;
class IPTranslator;
class IPConnectionCallback;
class IPMessage;
class IPCommunicator;
class IPPigeonHole;
class IPMessageType;
class IPHashTable;

// Base class for inter-module IPT connections
class IPConnection {
  public:
    IPConnection(const char*, const char*, const char*, IPCommunicator*,
                 int fd=-1);
    virtual ~IPConnection();

    void AddConnectCallback(IPConnectionCallback*);
    // ENDO
    //void AddConnectCallback(void (*)(IPConnection*, void*), void* = NULL);
    void AddConnectCallback(void (*)(class IPConnection*, void*), void* = NULL);
    void RemoveConnectCallback(IPConnectionCallback*);
    void InvokeConnectCallbacks();
    IPList* ConnectCallbacks() const { return _connect_callbacks; }

    void AddDisconnectCallback(IPConnectionCallback*);
    // ENDO
    //void AddDisconnectCallback(void (*)(IPConnection*, void*), void* = NULL);
    void AddDisconnectCallback(void (*)(class IPConnection*, void*), void* = NULL);
    void RemoveDisconnectCallback(IPConnectionCallback*);
    void InvokeDisconnectCallbacks();
    IPList* DisconnectCallbacks() const { return _disconnect_callbacks; }

    virtual int FD() const { return _fd; }
    IPCommunicator* Communicator() const { return _communicator; }
    virtual const char* Name() const { return _name; }
    virtual const char* Host() const { return _host; }

    // ENDO
    //virtual IPMessage* GetMessage(IPConnection* = NULL);
    virtual IPMessage* GetMessage(class IPConnection* = NULL);
    virtual void Activate(int, const char*);
    virtual void Deactivate();
    // ENDO
    //virtual void activate(const char*, IPConnection*);
    virtual void activate(const char*, class IPConnection*);

    virtual int Active() = 0;
    virtual int Viable() { return Active(); }
    virtual int Send(IPMessage*) = 0;
    virtual IPMessage* Receive() = 0;
    virtual int DataAvailable() = 0;

    IPPigeonHole* PigeonHole(IPMessageType*);

    virtual void SetName(const char* name);

    virtual void Allocate(int b) { _backed_up+= b; }
    virtual void Free(int b)  { _backed_up-=b; }
    virtual void Set(int b) { _backed_up = b; }
    virtual int Allocated() const { return _backed_up; }

    virtual int ConnectionNumber() const { return _connection_number; }

    const char* Type() const { return _type; }
    void SetType(const char* t) { _type = t; }

    virtual int ByteOrder() const { return _byte_order; }
    virtual int Alignment() const { return _alignment; }
    virtual void SetByteOrder(int e) { _byte_order = e; }
    virtual void SetAlignment(int a) { _alignment = a; }

    virtual int translate_id(int id);
    virtual void set_id_translator(IPTranslator* t) { _id_translator = t; }
    IPTranslator* translator() const { return _id_translator; }

  protected:
    int _fd;                       // file descriptor of the connection
    char* _host;                   // name of the host the module is on
    IPCommunicator* _communicator; // the connection's communicator
    
  private:
    IPList* _connect_callbacks;    // list of connection callbacks
    IPList* _disconnect_callbacks; // list of disconection callbackes
    char* _name;                   // name of the module connected to
    IPHashTable* _pigeon_holes;    // hash table of connection's pigeon holes
    int _backed_up;                // count of clogging bytes
    const char* _type;             // type of the connection
    int _byte_order;                   // byte order of connected module
    int _alignment;                // alignment of connected module
    IPTranslator* _id_translator; // message ID translator
    int _connection_number;       // unique (for this process) identifier

    // used to generate the _connection_numbers
    static int _last_connection_number;
};

#endif    
