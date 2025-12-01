///////////////////////////////////////////////////////////////////////////////
//
//                                 primcomm.h
//
// This header file defines routines used for setting up and monitoring TCP/IP
// socket based connections.  Many of these routines were taken directory from
// TCX
//
// Routines defined for export:
//    listenAtPort, connectAtPort, connectToModule, close_socket, 
//    data_at_socket, hosts_equal
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_primcomm_h
#define ip_primcomm_h

#define UNIX_SOCKET_NAME "/tmp/.ipt%s%d"

extern int listenAtPort(int& port, int& sd);
extern int connectAtPort(const char*, int, int&);
extern int connectToModule(const char*, const char*, int, int = 0);
extern void close_socket(int);
extern int data_at_socket(int);
extern int hosts_equal(const char*, const char*);
extern void protect_against_signals();
extern int socket_is_closed(int);
extern int accept_socket(int);

#ifndef VXWORKS
extern int connectToModule(const char*, int, const char*, int = 0);
extern int listenAtSocket(int port, int& sd, const char*);
extern int connectAtSocket(int port, int& sd, const char*);
extern void close_unix_port(int port, const char*);
#endif

#endif
