#ifndef ip_route_h
#define ip_route_h

#define IPT_ROUTE_HIGHEST 1
#define IPT_ROUTE_LOWEST 2
#define IPT_ROUTE_AFTER 4

class IPCommunicator;
class IPConnection;
class IPList;
class IPHashTable;

class IPRouting {
  public:
    IPRouting();
    ~IPRouting();

    void set_routing(const char*, const char*, int, const char*);
    void set_sub_type(const char*);
    void set_parameters(const char*);
    int matches(const IPRouting*) const;
    const char* type() const { return _type; }
    const char* sub_type() const { return _sub_type; }
    const char* parameters() const { return _parameters; }
    int comparison() const { return _comparison; }

  private:
    char* _type;
    char* _sub_type;
    int _comparison;  // 0 = non, 1 = same, 2 = different, 3 = same hosts
    char* _parameters;
};

class IPRoutingTable {
  public:
    IPRoutingTable(IPConnection* conn, int num_routes, int free_conn = 0);
    ~IPRoutingTable();

    IPConnection* connection() const { return _connection; }
    IPRouting* table() const { return _table; }
    int num_routes() const { return _num_routes; }
    IPRouting* route(int index) const { return &_table[index] ; }

  private:
    int _num_routes;
    IPRouting* _table;
    IPConnection* _connection;
    int _free_conn;
};

class IPRouter {
  public:
    IPRouter(IPCommunicator* comm, const char* type, int comparison);
    virtual ~IPRouter();

    IPConnection* route_connection(const char* name,
                                   const char* sub_type,
                                   const char* parameters);
    virtual void activate_connection(IPConnection* conn,
                             const char* sub_type,
                             const char* parameters) = 0;
    virtual IPConnection* copy_connection(const char*, const char*,
                                          IPConnection*) = 0;

    IPCommunicator* communicator() const { return _communicator; }
    IPRouting* routing() const { return _routing; }
    const char* type() const { return _routing->type(); }

  protected:
    virtual IPConnection* make_connection(const char* name,
                                          const char* sub_type,
                                          const char* parameters) = 0;

  private:
    IPCommunicator* _communicator;
    IPRouting* _routing;
};

class IPRouterTable {
  public:
    IPRouterTable();
    ~IPRouterTable();

    void add_router(IPRouter*, int = IPT_ROUTE_HIGHEST, IPRouter* = 0);
    IPRouter* lookup_router(const char*) const;
    IPRouter* router(int i) const;
    int num_routers() const;

  private:
    IPList* _routers;
    IPHashTable* _table;
};

class TCPRouter : public IPRouter {
  public:
    TCPRouter(IPCommunicator* comm, int port);

    virtual IPConnection* copy_connection(const char*, const char*,
                                          IPConnection*);
    int port() const { return _port; }

  protected:
    virtual IPConnection* make_connection(const char* name,
                                          const char* sub_type,
                                          const char* parameters);
    virtual void activate_connection(IPConnection* conn, const char* sub_type,
                                     const char* parameters);

  private:
    void accept_new_connection_hand(IPConnection* conn);
    void close_socket_hand(IPConnection* conn);

  private:
    int _port;
};

#endif
