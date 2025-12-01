///////////////////////////////////////////////////////////////////////////////
//
//                                 server.h
//
// This file declares the server class.  This class can be used to implement
// the central "operator" for IPT.  The server has two main purposes,
// first, to provide a consistent mapping between message name (strings) and
// message IDs (numbers).  Second, to arrange the connections between client
// modules.  The servers are set up so that several of them can be connected
// together.  Each server manages its own "domain" and each domain can have
// a name.  There should be one server which is the domain master, which 
// maintains and coordinates system wide things, such as interdomain message 
// IDs and how to get to all the domains
// 
//
// Classes defined for export:
//    IPServer
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef ip_server_h
#define ip_server_h

class IPConnection;
class IPList;
class ConnectionNet;
class DomainList;
class ConnectionRequest;
class PendingCallback;
class IPRoutingTable;
class IPRouting;
struct PeerDescriptor;
struct IPTRoutingTableStruct;
struct IPTRequestedRoutingTableStruct;

#include <ipt/tcpcommunicator.h>

struct IPDomainSpec {
    char* name;
    char* host;
};

/* The server class */
class IPServer : public TCPCommunicator {
  public:
    IPServer(const char*, const char*, const char*,
             IPDomainSpec*, const char*);
    virtual ~IPServer();

    void arrange_connection(IPConnection*, IPConnection*, int);
    void arrange_remote_connection(IPConnection*, IPConnection*, int);
    void clear_remote_connection(const char* dom_name, const char* mod_name);
    void fail_connection_request(const char*, IPMessage* msg);
    IPRouting* match_routings(IPRoutingTable*, IPRoutingTable*);
    int routing_reply(IPMessage*, const char*, IPRouting*, int=0);
    IPRoutingTable* add_routing(const char*, IPTRequestedRoutingTableStruct*);

  private:
    void read_message_file(FILE*);
    void mod_connecting(IPConnection*);
    void mod_disconnecting(IPConnection*);
    void req_connection_hand(IPMessage*);
    void register_message_hand(IPMessage*);
    void register_message_set_hand(IPMessage*);
    void req_domain_name_hand(IPMessage*);
    void server_flush_net_hand(IPMessage*);
    void remote_confirm_hand(IPMessage*);
    void remote_report_hand(IPMessage*);
    void logging_hand(IPMessage*);
    void deletion_hand(IPMessage*);
    void server_dump_hand(IPMessage*);
    void server_flush_hand(IPMessage*);
    void server_dump_net_hand(IPMessage*);
    void remote_clear_hand(IPMessage*);
    void request_translation_hand(IPMessage*);
    void peer_connecting_hand(IPMessage*);
    void routing_table_hand(IPMessage*);
    void routing_table_req_hand(IPMessage*);
    void report_connections_hand(IPMessage*);
    void reregister_messages_hand(IPMessage*);
    IPConnection* connect_to_domain(const char*);
    IPConnection* wait_for_connection(const char*);
    int register_global_message(const char*);
    int register_a_message(const char*);
    PeerDescriptor* add_peer(const char* name, const char* host);
    void remove_peer(const char* name);
    PeerDescriptor* lookup_peer(const char* name);
    void rename_peer(const char* old_name, const char* new_name);
    void peer_disconnecting(IPConnection*);
    IPRoutingTable* lookup_routing(const char* name);
    void add_routing(IPRoutingTable*);
    void remove_routing(const char*);
    IPConnection* lookup_connection(const char* name);
    int duplicate_request(IPConnection*, const char*);
    void remove_request(IPConnection*, const char*);

  private:
    IPList* _request_list;         // list of pending connection requests
    int _next_message_num;          // the next available message ID
    int _arranging_connection;     // true if in midst of arranging conn
    FILE* _log_fp;                 // place to log messages
    PendingCallback* _pending_register;
    PendingCallback* _pending_tables;
    PendingCallback* _pending_confirms;
    ConnectionNet* _conn_net;
    IPHashTable* _peer_table;
    IPHashTable* _routing_tables;
};

#endif
