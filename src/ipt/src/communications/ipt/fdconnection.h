///////////////////////////////////////////////////////////////////////////////
//
//                                 fdconnection.h
//
// This header file defines subclasses of IPConnection used to define 
// connections between modules which are based on file descriptors (usually
// socket file descriptors).
//
// Classes defined for export:
//    FDConnection, TCPConnection, UnixConnection
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_connectionP_h
#define ip_connectionP_h

#include <ipt/connection.h>

#define BUF_SIZE 2048

// Connection class for using file descriptors to communicate between modules
// based on file descriptors
class FDConnection : public IPConnection {
  public:
    FDConnection(const char*, const char*, const char*, IPCommunicator*,
                 int fd=-1);

    virtual int Active();
    virtual int Viable();
    virtual int Send(IPMessage*);
    virtual IPMessage* Receive();

    virtual int DataAvailable();

    void SetData(FDConnection*);
    virtual void Deactivate();
    virtual void activate(const char*, IPConnection*);

  private:
    int read_string(const char*& msg_name, int& len);

  private:
    unsigned char _in_buffer[BUF_SIZE];   // buffer for input
    int _bytes_in_buffer;                 // how many bytes in input buffer
    int _buffer_index;                    // index into the input buffer
};

// class for communicating with TCP/IP sockets
class TCPConnection : public FDConnection {
  public:
    TCPConnection(const char*, const char*, IPCommunicator*,
                  int fd=-1);
};

// class for communicating with Unix sockets
class UnixConnection : public FDConnection {
  public:
    UnixConnection(const char*, const char*, IPCommunicator*,
                   int fd=-1);
};

#endif
