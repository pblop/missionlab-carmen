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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Base class for inter-module IPT connections
class IPConnection {
  public:
    IPConnection(const char* name)
    { _name = strdup(name); _host = strdup("undefined"); _fd = 1;	 }

    ~IPConnection()
    {free(_name); free(_host);}

    int FD() const { return _fd; }
    const char* Name() const { return _name; }
    const char* Host() const { return _host; }

  private:
    char* _name;                   // name of the module connected to
    int _fd;                       // file descriptor of the connection
    char* _host;                   // name of the host the module is on
};

#endif
