///////////////////////////////////////////////////////////////////////////////
//
//                                 forconnection.h
//
// This header file defines a subclass of IPConnection used to define 
// a container for other connections.  This is used to keep connection pointers
// used by the user constant even when the actual type of connection changes
// from, say TCPConnection to UnixConnection.
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

#ifndef for_connection_h
#define for_connection_h

#include <ipt/connection.h>

class IPRouter;

class ForConnection : public IPConnection {
  public:
    ForConnection(const char*, IPCommunicator*);
    virtual ~ForConnection();

    IPConnection* Body() const { return _body; }
    void Body(IPConnection*);

    virtual int FD() const;
    virtual const char* Name() const;
    virtual const char* Host() const;

    virtual IPMessage* GetMessage(IPConnection* = NULL);
    virtual void Activate(int, const char*);
    virtual void activate(const char*, IPConnection*);
    virtual void Deactivate();

    virtual int Active();
    virtual int Viable();
    virtual int Send(IPMessage*);
    virtual IPMessage* Receive();
    virtual int DataAvailable();

    virtual void Allocate(int b);
    virtual void Free(int b);
    virtual void Set(int b);
    virtual int Allocated() const;

    virtual int ConnectionNumber() const;

    virtual int ByteOrder() const;
    virtual int Alignment() const;
    virtual void SetByteOrder(int e);
    virtual void SetAlignment(int a);

    virtual int translate_id(int id);
    virtual void set_id_translator(IPTranslator* t);

  private:
    IPConnection* _body;
};

#endif
