///////////////////////////////////////////////////////////////////////////////
//
//                                 tcpcommunicator.h
//
// This file declares the classes needed for basic usage of TCP/IP only IPT
//
// Classes defined for export:
//    TCPCommunicator
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef tcpcommunicator_h
#define tcpcommunicator_h

#include <ipt/ipt.h>

class IPRouter;
class IPRouterTable;
class ForConnection;

/* This is the class to use when all TCP/IP connections are intended, i.e.,
   on a standard UNIX module */
class TCPCommunicator : public IPCommunicator {
  public:
    TCPCommunicator(const char* mod_name, const char* host_name = 0,
                    void (*)(TCPCommunicator*) = 0);
    TCPCommunicator(int);
    virtual ~TCPCommunicator();

    /* make a connection to another module */
    virtual IPConnection* Connect(const char*,
                                  int = IPT_REQUIRED);
    virtual IPConnection* DeclareConnection(const char*);
    
    virtual IPConnection* MakeConnection(const char* name, const char* host,
                                         int port, const char* this_name=NULL,
                                         int fail = 0);
    void AddConnection(IPConnection*);
    void ActivateConnection(IPConnection*);
    void DeactivateConnection(IPConnection*);

    virtual int WaitForInput(double);

    virtual void LogSend(IPConnection*, IPMessage*);
    virtual void LogReceive(IPConnection*, IPMessage*, const char* = NULL);

    virtual IPConnection* AddAdministrativeConnection(int fd,
                                                      IPConnectionCallback*,
                                                      IPConnectionCallback*);
    virtual void RemoveAdministrativeConnection(int fd);

    IPConnection* Server() const { return _server; }
    void close_socket_hand(IPConnection*);
    virtual int add_translator(int, IPConnection*);
    void add_router(IPRouter*, int, const char* = 0);
    void send_routes();
    virtual IPConnection* add_connection(const char* type, IPConnection* temp);

  protected:
    TCPCommunicator();
    virtual int get_message_id(const char*);
    virtual int* register_messages(int, char**);
    int register_connection(IPConnection*, const char* = 0);
    void set_translator(const char*, IPConnection*);

  private:
    void accept_new_connection_hand(IPConnection*);
    void initialize(int);
    IPTranslator* lookup_translator(const char* domain_name);
    IPConnection* make_connection(IPMessage*);
    int resolve_collision(ForConnection*, IPConnection*);
    void peer_disconnecting_hand(IPMessage*);
    void connection_routing_hand(IPMessage*);
    void translation_hand(IPMessage*);
    void server_disconnecting_hand(IPConnection*);
    void try_server(IPTimer*);
    void server_register();

  protected:
    fd_set* _connection_mask;  // Mask of active connection file descriptors
    int _port;

  private:
    IPConnection* _server;     // connection to an IPT server
    IPList* _translators;
    IPRouterTable* _routers;
};

#endif
