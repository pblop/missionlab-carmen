///////////////////////////////////////////////////////////////////////////////
//
//                                 fakeconnection.h
//
// This header file defines a subclass of IPConnection used to hold a file
// descriptor.  This file descriptor cannot be sent or received from, but
// it can be acted upon when added as an administrative connection
// 
//
// Classes defined for export:
//    FakeConnection
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_fakeconnection_h
#define ip_fakeconnection_h

#include <ipt/connection.h>

class FakeConnection : public IPConnection {
  public:
    FakeConnection(const char* name, IPCommunicator* comm, int fd);
    virtual ~FakeConnection();

    virtual int Active();
    virtual int Send(IPMessage*);
    virtual IPMessage* Receive();
    virtual int DataAvailable();
};

#endif
