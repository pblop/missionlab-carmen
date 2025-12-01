///////////////////////////////////////////////////////////////////////////////
//
//                                 conn_net.h
//
// This file also declares the class for maintaining the connection graph
//
// Classes defined for export:
//    ConnectionNet
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef conn_net_h
#define conn_net_h

class IPHashTable;
class IPList;

class ConnectionNet {
  public:
    ConnectionNet();
    ~ConnectionNet();

    void connect(const char* initiator, const char* receptor);
    void disconnect(const char*);
    void disconnect_domain(const char*);
    int connected(const char* initiator, const char* receptor);
    void dump(FILE*);
    void iterate(const char*, int (*)(char*, char*, void*),
                 void*);
    int interdomain(const char*);
    IPList* domains(const char*);

    void unlink(const char*, const char*);

  private:
    int update_net(const char*, const char*);

  private:
    IPHashTable* _connections;
};

#endif
