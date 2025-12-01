/**************************************************************************

                                 ipt.h

 This file declares the classes needed for basic usage of IPT.  Unless you
 use very basic functionality, you often have to include message.h for 
 processing messages.  Other include files (connection.h, format.h, etc.)
 can be included as needed.  This file is meant for inclusion by both C and
 C++ programs.

 Classes defined for export:
    IPCommunicator

  "1995, Carnegie Mellon University. All Rights Reserved." This
  software is made available for academic and research purposes only. No
  commercial license is hereby granted.  Copying and other reproduction is
  authorized only for research, education, and other non-commercial
  purposes.  No warranties, either expressed or implied, are made
  regarding the operation, use, or results of the software.

**************************************************************************/

#ifndef ipt_h
#define ipt_h

#ifndef NULL
#define NULL 0
#endif

#define IPT_BLOCK -1.0
#define IPT_MAJOR_VERSION 8
#define IPT_MINOR_VERSION 3
#define IPT_SUB_VERSION 0
#define IPT_REQUIRED 1
#define IPT_OPTIONAL 0
#define IPT_NONBLOCKING 2
#define IPT_HNDL_STD 1
#define IPT_HNDL_ALL 0
#define IPT_HNDL_NONE -1

/* for backward compatibility */
#define IPT_RECV_ALL  1

#if defined(__cplusplus)

class IPMessageType;
class IPConnection;
class IPHandlerCallback;
class IPMessage;
class IPCommunicator;
class IPFilter;
class IPConnectionCallback;
class IPTimerCallback;
class IPTimer;
class IPHashTable;
class IPFormatParser;
class IPList;
class IPDestination;
class IPEvent;
class IPPigeonHole;
class IPPublication;
class IPFormat;
class IPTranslator;
class IPTimerTable;
struct IPTConnectionInfoStruct;
#ifdef __linux__
//struct __fd_set;
//typedef __fd_set fdset;
#else
struct fd_set;
#endif

#include <ipt/spec.h>
#include <stdio.h>

/* The base class for IPT.  Specifies the syntax for the basic operations of
   sending, receiving, handling, and administrating messages */
class IPCommunicator {
  public:
    /* constants used to set contexts for handlers.  Note:  included for
       backward compatibility */
    static const int ReceiveAll;
    static const int ReceiveOnce;
    static const int ReceiveNone;

    /* create an appropriate instance of IPCommunicator */
    static IPCommunicator* Instance(const char* mod_name,
                                    const char* host_name=NULL);
    static void initializeThreads(int parallelism_level = 1);
    static int getParallelismLevel() { return _parallelism_level; }

    /* create a communicator with a module name connected to a host.  If the
       host_name is nil, look at the environment variable COMM_HOST */
    IPCommunicator(const char* mod_name, const char* host_name = NULL);

    /* destroy a communicator */
    virtual ~IPCommunicator();

    /* register a message with a name and a format, create it and add message
       name to internal message hash table */
    IPMessageType* RegisterMessage(const char*, const char*);
    void RegisterMessages(IPMessageSpec*);
    IPFormat* RegisterNamedFormatter(const char*, const char*);
    void RegisterNamedFormatters(IPFormatSpec* specs);

    /* make a connection to another module */
    virtual IPConnection* Connect(const char*,
                                  int = IPT_REQUIRED)
        { return NULL; }

    /* declare an empty connection that only has a name */
    virtual IPConnection* DeclareConnection(const char*)
        { return NULL; }
    
    /* register a handler to react to an incoming message with name msg_name
       name the handler hand_name.  The handler uses callback to respond.  When
       to handle the message is controlled by handler_spec */
    void RegisterHandler(const char* msg_name, 
                         IPHandlerCallback* callback, int spec = IPT_HNDL_STD);
    /* same as above, but use a message instead of message name */
    void RegisterHandler(IPMessageType* mess, 
                         IPHandlerCallback* callback, int spec = IPT_HNDL_STD);

    /* register a routine handler rather than a class handler */
    // ENDO
    /*
    void RegisterHandler(IPMessageType* mess,
                         void (*)(IPCommunicator*, IPMessage*, void*),
                         void* callback_data = NULL, 
                         int spec = IPT_HNDL_STD);
    void RegisterHandler(const char* msg_name,
                         void (*)(IPCommunicator*, IPMessage*, void*),
                         void* callback_data = NULL, 
                         int spec = IPT_HNDL_STD);
    */
    void RegisterHandler(IPMessageType* mess,
                         void (*)(class IPCommunicator*, IPMessage*, void*),
                         void* callback_data = NULL, 
                         int spec = IPT_HNDL_STD);
    void RegisterHandler(const char* msg_name,
                         void (*)(class IPCommunicator*, IPMessage*, void*),
                         void* callback_data = NULL, 
                         int spec = IPT_HNDL_STD);
    /* get and set handlers status.  When handlers are disabled _no_ handlers
       work, including the ones necessary to connect modules */
    int HandlersActive() const { return _handlers_active; }
    void DisableHandlers() { _handlers_active = 0; }
    void EnableHandlers();

    /* Get the next available message */
    IPMessage* ReceiveMessage(double timeout = IPT_BLOCK);
    /* Get the next available message that passes the filter */
    IPMessage* ReceiveMessage(IPFilter* filter, double timeout = IPT_BLOCK);
    /* convenience functions for ReceiveMessage*/
    IPMessage* ReceiveMessage(IPConnection*, IPMessageType*,
                              double timeout = IPT_BLOCK);
    IPMessage* ReceiveMessage(IPConnection* conn, const char* msg_name,
                              double timeout = IPT_BLOCK);

    /* Convenience functions for receiving formatted data */
    void* ReceiveFormatted(IPConnection* conn, IPMessageType*, 
                           double timeout = IPT_BLOCK);
    void* ReceiveFormatted(IPConnection* conn, const char*, 
                           double timeout = IPT_BLOCK);
    int ReceiveFormatted(IPConnection* conn, IPMessageType*,
                          void*, double timeout = IPT_BLOCK);
    int ReceiveFormatted(IPConnection* conn, const char*,
                          void*, double timeout = IPT_BLOCK);

    /* Sends a message across a connection */
    int SendMessage(IPConnection*, IPMessage*); // the workhorse
    /* convenience functions */
    int SendMessage(IPConnection*, const char* msg_name, void* data);
    int SendMessage(IPConnection*, const char* msg_name, int, unsigned char*);
    int SendMessage(IPConnection*, int msg_num, void* data);
    int SendMessage(IPConnection*, IPMessageType*, void*);
    int SendMessage(IPConnection*, IPMessageType*, int, unsigned char*);

    /* The result of calling this function on a message specifier will be that
       the result of a GetMessage or the data passed in on a callback in result
       of a message of type msg_type will be garaunteed to have the most recent
       message of that type from a connection
     */
    void PigeonHole(IPMessageType* msg_type);
    void PigeonHole(const char* msg_name);

    /* Same as above, except instead of a pigeon hole for each connection,
       there will be one pigeon hole for any message from any connection, so
       GetMessage and handlers dealing with the message type will be the most
       recent message of that type period.  Not implemented now */
    void SpecialPigeonHole(IPMessageType* msg_type);
    void SpecialPigeonHole(const char* msg_name);

    /* routines for calling back when things connect and disconnect */
    void AddConnectCallback(IPConnectionCallback*);
    void AddConnectCallback(void (*)(IPConnection*, void*), void* = NULL);
    void InvokeConnectCallbacks(IPConnection* res);
    void RemoveConnectCallback(IPConnectionCallback*);
    void AddDisconnectCallback(IPConnectionCallback*);
    void AddDisconnectCallback(void (*)(IPConnection*, void*), void* = NULL);
    void InvokeDisconnectCallbacks(IPConnection* res);
    void RemoveDisconnectCallback(IPConnectionCallback*);
    void connect_notify(IPConnection* conn);
    void disconnect_notify(IPConnection* conn);

    /* Timer operations */
    IPTimer* AddTimer(double interval, IPTimerCallback*);
    // ENDO
    /*
    IPTimer* AddTimer(double interval, void(*)(IPCommunicator*, IPTimer*,
                                               void*),
                      void*);
    */
    IPTimer* AddTimer(double interval, void(*)(class IPCommunicator*, IPTimer*,
                                               void*),
                      void*);

    IPTimer* AddOneShot(double interval, IPTimerCallback*);
    // ENDO
    /*
    IPTimer* AddOneShot(double interval, void(*)(IPCommunicator*, IPTimer*,
                                                 void*),
                      void*);
    */
    IPTimer* AddOneShot(double interval, void(*)(class IPCommunicator*, IPTimer*,
                                                 void*),
                      void*);

    void RemoveTimer(IPTimer*);

    /* loops for handling events */
    void MainLoop();
    int Sleep(double time = IPT_BLOCK);
    int Idle(double time = IPT_BLOCK);
    void Finish() { _finished = 1; }
    void Restart() { _finished = 0; }
    int Finished() const { return _finished; }

    /* process one set of events until timeout is reached */ 
    int Process(double timeout = IPT_BLOCK);

    /* lookup a connection by name */
    IPConnection* LookupConnection(const char*);

    /* lookup message types by name and ID */
    IPMessageType* LookupMessage(const char*);
    IPMessageType* LookupMessage(int);

    /* Return the name of the machine the module is on */
    const char* ThisHost() const;
    /* Return the name of the module */
    const char* ModuleName() const { return _mod_name; }
    /* Return the name of the machine the server is on */
    const char* ServerHostName() const { return _host_name; }
    const char* DomainName() const { return _domain_name; }
    void SetDomainName(char* name) { _domain_name = name; }

    /* disable and enable handlers for a particular message type */
    void DisableHandler(IPMessageType*);
    void EnableHandler(IPMessageType*);
    void DisableHandler(const char*);
    void EnableHandler(const char*);

    /* Query for a message */
    IPMessage* Query(IPConnection*, IPMessage*, IPMessageType*,
                     double = IPT_BLOCK);
    IPMessage* Query(IPConnection*, IPMessageType*, void*, IPMessageType*,
                     double = IPT_BLOCK);
    IPMessage* Query(IPConnection*, IPMessageType*, int, unsigned char*,
                     IPMessageType*, double = IPT_BLOCK);
    IPMessage* Query(IPConnection*, int, void*, int, double=IPT_BLOCK);
    IPMessage* Query(IPConnection*, const char*, int, unsigned char*,
                     const char*, double = IPT_BLOCK);
    IPMessage* Query(IPConnection*, const char*, void*,
                     const char*, double = IPT_BLOCK);
    void* QueryFormatted(IPConnection*, const char*, void*, const char*,
                         double = IPT_BLOCK);
    int QueryFormatted(IPConnection*, const char*, void*, const char*, void*,
                       double = IPT_BLOCK);

    /* Reply to a message */
    void Reply(IPMessage*, IPMessageType*, void*);
    void Reply(IPMessage*, IPMessageType*, int, unsigned char*);
    void Reply(IPMessage*, const char*, void*);
    void Reply(IPMessage*, const char*, int, unsigned char*);
    void Reply(IPMessage*, int, void*);

    /* I wouldn't use these functions publicly, but they are there.  I make
       them public because they can be used to customize operations from
       outside the class */
    virtual void AddConnection(IPConnection*);
    virtual void DeactivateConnection(IPConnection*);
    virtual void AddEvent(IPEvent*);
    virtual void QueueMessage(IPMessage*);
    virtual void QueuePigeonHole(IPPigeonHole*);

    virtual IPEvent* pop_event();
    virtual IPMessage* pop_message();
    virtual IPMessage* remove_message(int (*)(const char*, const char*),
                                      const char*, int = 0);
    virtual IPPigeonHole* pop_hole();
    virtual IPPigeonHole* remove_hole(int (*)(const char*, const char*),
                                      const char*, int = 0);

    int Execute(IPMessage* msg);

    IPList* Connections() const { return _connection_list; }
    IPList* Events() const { return _event_list; }
    IPList* MessageQueue() const { return _message_queue; }
    IPList* PigeonHoleQueue() const { return _hole_queue; }
    IPDestination* Destination() const { return _default_destination; }
    void ModuleName(const char*);
    IPFormatParser* FormatParser() const { return _parser; }
    IPHashTable* MessageTable() const { return _message_table; }

    void IterateConnections(IPConnectionCallback*);
    IPList* AdministrativeConnections() const { return _administrative_list; }
    virtual IPConnection* AddAdministrativeConnection(int fd,
                                                      IPConnectionCallback*,
                                                      IPConnectionCallback*);
    virtual void RemoveAdministrativeConnection(int fd);

    IPMessage* WaitForMsg(IPConnection*, int);

    virtual int ScanForMessages(double);
    virtual void ProcessEvents();
    virtual int WaitForInput(double);

    IPMessage* NextReceivedMessage();
    IPMessage* ScanReceivedMessages(IPFilter*);

    void Rename(IPConnection*, const char* new_name);

    /* publication/subscription routines */
    void DeclareSubscription(IPMessageType*, IPPublication*);
    void DeclareSubscription(IPMessageType*, IPConnectionCallback* = NULL);
    void DeclareSubscription(const char*, IPConnectionCallback* = NULL);
    void DeclareTimedSubscription(IPMessageType*, IPConnectionCallback* = 0);
    void DeclareTimedSubscription(const char*, IPConnectionCallback* = NULL);
    void Publish(IPMessageType*, IPMessage*);
    void Publish(IPMessageType*, void*);
    void Publish(IPMessageType*, int, unsigned char*);
    void Publish(const char*, void*);
    void Publish(const char*, int, unsigned char*);
    int Subscribe(IPConnection*, IPMessageType*, int = 0, unsigned char* = 0);
    IPConnection* Subscribe(const char*, const char*,
                            int = 0, unsigned char* = 0);
    int Subscribe(IPConnection*, IPMessageType*, double interval);
    IPConnection* Subscribe(const char*, const char*, double interval);
    IPList* Subscribers(IPMessageType*);
    IPList* Subscribers(const char*);
    int NumSubscribers(IPMessageType*);
    int NumSubscribers(const char*);
    void SubscribeConnection(IPConnection*, IPMessageType*,
                             int = 0, unsigned char* = 0);
    IPConnection* SubscribeConnection(const char*, const char*,
                                      int = 0, unsigned char* = 0);
    void Unsubscribe(IPConnection*, IPMessageType*);
    void Unsubscribe(const char*, const char*);
    
    /* general server/client routines */
    void Server(IPMessageType* registration_type = 0,
                IPHandlerCallback* cb = 0);
    void Server(const char* msg_name, IPHandlerCallback* cb = 0);
    void AddClient(IPConnection*);
    void Broadcast(IPMessage*);
    void Broadcast(IPMessageType*, void*);
    void Broadcast(IPMessageType*, int, unsigned char*);
    void Broadcast(const char*, void*);
    void Broadcast(const char*, int, unsigned char*);
    void Client(IPConnection*, IPMessage* = 0);
    IPConnection* Client(const char*, IPMessage* = 0);
    void Client(IPConnection*, const char*, void*);
    IPConnection* Client(const char*, const char*, void*);
    IPConnection* Client(const char*, const char*, int, unsigned char*);
    IPMessage* RegistrationMessage(IPConnection*);
    IPList* Clients() const { return _clients; }

    /* memory management convenience routines */
    void DeleteFormatted(const char* msg_name, void* data);
    void DeleteContents(const char* msg_name, void* data);

    /* logging routines */
    int Logging() const { return _logging; }
    virtual void LogSend(IPConnection*, IPMessage*);
    virtual void LogReceive(IPConnection*, IPMessage*, const char* = 0);

    /* routines for setting the hooks for when we do blocking I/O */
    void SetStartBlockingAction(void (*)(void*), void* = NULL);
    void SetEndBlockingAction(void (*)(void*), void* = NULL);
    void StartBlocking();
    void EndBlocking();

    /* Used to generate a "unique" instance number.  If you want to 
       construct your own messages you should use this function */
    int generate_instance_num() { return _instance_num++; }

    int printf(const char* ...);
    virtual int add_translator(int, IPConnection*);
    virtual IPConnection* add_connection(const char*, IPConnection*)
        { return NULL; }

    void enable_disconnect_handlers() { _disconn_handlers_active = 1; }
    void disable_disconnect_handlers() { _disconn_handlers_active = 0; }
    int status_disconnect_handlers() const
        { return _disconn_handlers_active; };

  protected:
    IPCommunicator();
    
    IPMessageType* register_message(int, const char*, const char*);
    virtual int get_message_id(const char*) { return -1; }
    virtual int* register_messages(int, char**) { return 0; }
    virtual void check_message_queues();

  private:
    void register_built_in_messages();
    void subscription_hand(IPMessage*);
    void unsubscribe_hand(IPMessage*);
    void remote_subscription_hand(IPMessage*);
    void initialize();
    void delete_extras();
    void init_extras();
    int subscription_added(IPConnection* publisher, IPMessageType* type);
    void add_subscription(IPConnection* publisher, IPMessageType* type,
                          IPConnectionCallback*, IPConnectionCallback*);

  private:
    IPHashTable* _message_table;       // table of message types hashed by name
    IPHashTable* _id_to_message_table; // table of types hashed by ID
    IPHashTable* _connection_table;    // table of connections hashed by name
    IPList* _connection_list;          // list of connections
    IPList* _administrative_list;      // list of administrative connections
    char* _mod_name;                   // the module name
    char* _host_name;                  // the servers machine name
    char* _domain_name;                // the servers domain name
    IPFormatParser* _parser;           // the parser used to format messages
    IPList* _event_list;               // the event queue
    int _instance_num;                 // the current message instance number
    char* _this_host;                  // the modules machine name
    IPList* _message_queue;            // the queue of unhandled messages
    IPList* _hole_queue;        // the queue of unhandled pigeonholed messages
    IPDestination* _default_destination;  // destination for plain messages
    int _handlers_active;              // true if handlers are to be invoked
    IPList* _connect_callbacks;        // list of callbacks on connection
    IPList* _disconnect_callbacks;     // list of callbacks on disconnection
    IPHashTable* _publication_table;   // table of publications
    IPHashTable* _subscription_table;   // table of publications
    IPList* _clients;                  // list of clients
    int _logging;                     // true if logging message headers
    IPHashTable* _server_table;        // table of servers we are clients of
    FILE* _out_fp;                     // file pointer used for output
    int _disconn_handlers_active;      // if true, suppress disconnect handling
    IPTimerTable* _timer_table;        // table of timers
    void (*_start_blocking_action)(void*);  // start blocking callback
    void *_start_blocking_data;        // client data for the above
    void (*_end_blocking_action)(void*);    // end blocking callback
    void *_end_blocking_data;          // client data for the above
    int _finished;             // true if we are doing no more processing

    static int _parallelism_level;  // 0 is no parallelism, 1 is multiple IPT's
                                    // on multiple threads, 2 would be one IPT
                                    // on multiple threads
};

extern "C" {

#else

#define IPMESSAGE_SIZE 32

/* Aliasing for C code */
typedef void IPMessageType;
typedef void IPConnection;
typedef void IPCommunicator;
typedef void IPFormat;
typedef void IPTimer;
typedef char IPMessage[IPMESSAGE_SIZE];

#include <ipt/spec.h>

#endif

/* base IPT cover functions */
/* initialize IPT with module name and a host machine name (if host machine
   name is NULL, looks for name of host machine from environment variable
   IPTHOST, if IPTHOST is not set assumes current machine is host machine) */
IPCommunicator* iptCommunicatorInstance P_((char* mod_name, char* host_name));
void iptDeleteCommunicator P_((IPCommunicator*));

/* register a message with name msg_name.  format is the format string ala
   TCX.  Pass format with value NULL for no format specified.  */
IPMessageType* iptRegisterMessage P_((IPCommunicator*,
                                      char* msg_name, char* format));
void iptRegisterMessages P_((IPCommunicator*, IPMessageSpec* messages));
IPFormat* iptRegisterNamedFormatter P_((IPCommunicator*, char*, char*));
void iptRegisterNamedFormatters P_((IPCommunicator*, IPFormatSpec*));

/* Connect to module with name "name".  Set required to 1 if you want to block
   until the module attaches */
IPConnection* iptConnect P_((IPCommunicator*, char* name, int required));

void iptRegisterHandler P_((IPCommunicator*, char* msg_name,
                            void (*callback) P_((IPCommunicator*,
                                                 IPMessage*, void*)),
                            int context, void* data));

void iptMainLoop P_((IPCommunicator*));
int iptSleep P_((IPCommunicator*, double));
int iptIdle P_((IPCommunicator*, double));
IPMessageType* iptLookupMessage P_((IPCommunicator*, char* name));
IPMessageType* iptMessageById P_((IPCommunicator*, int id));
IPMessage* iptReceiveMsg P_((IPCommunicator*,
                             IPConnection* conn, char* msg_name, 
                             double timeout));
IPConnection* iptConnection P_((IPCommunicator*, char* conn_name));
void iptPigeonHole P_((IPCommunicator*, char* msg_name));
void iptPigeonHoleType P_((IPCommunicator*, IPMessageType* type));
void iptAddConnectCallback P_((IPCommunicator*,
                               void (*func) P_((IPCommunicator*,
                                                IPConnection*, void*)),
                               void* data));
void iptAddDisconnectCallback P_((IPCommunicator*,
                                  void (*func) P_((IPCommunicator*,
                                                   IPConnection*, void*)),
                                  void* data));
void iptIterateConnections P_((IPCommunicator* comm,
                               void (*func) P_((IPCommunicator*,
                                                IPConnection*, void*)),
                               void* data));
void iptAddAdministrativeConnection P_((IPCommunicator* comm, int fd,
                                        void (*activity) P_((IPCommunicator*,
                                                             IPConnection*,
                                                             void*)),
                                        void (*close_it) P_((IPCommunicator*,
                                                             IPConnection*,
                                                             void*)),
                                        void* data));
void iptRemoveAdministrativeConnection P_((IPCommunicator* comm, int fd));
char* iptThisHost P_((IPCommunicator*));
char* iptModuleName P_((IPCommunicator*));
char* iptDomainName P_((IPCommunicator*));
void iptDisableHandlers P_((IPCommunicator*));
void iptEnableHandlers P_((IPCommunicator*));
void iptDisableMsgHandler P_((IPCommunicator*, IPMessageType* type));
void iptEnableMsgHandler P_((IPCommunicator*, IPMessageType* type));
IPMessage* iptQueryMsgForm P_((IPConnection* conn, char* query_msg_name,
                               void* data, char* recv_msg_name));
IPMessage* iptQueryMsgRaw P_((IPConnection* conn, char* query_msg_name,
                              int size, unsigned char* data,
                              char* recv_msg_name));


/* convenience functions to avoid dealing with IPMessage */

/* Send a message of type "type" to connection "conn".  The data for the
   message will have length "size" and be stored in array "data" */
int iptSendRawMsg P_((IPConnection* conn, char* type,
                      int size, unsigned char* data));

/* Receive a message from connection conn of type type.  Puts the data
   into "buffer" and returns the number of bytes received.  If the number
   of bytes received is greater than "max_size" it returns -1, and all
   the data that fits is copied into "buffer".  If "conn" is NULL it waits for
   messages of type "type" from any connection.  If "type" is NULL it waits
   for messages of any type from any connection.  If "timeout" is -1.0 it
   blocks forever, otherwise it returns after "timeout" seconds */
int iptReceiveRawMsg P_((IPCommunicator*, IPConnection* conn, char* type,
                         int max_size, unsigned char* buffer,
                         double timeout));

int iptSendFormMsg P_((IPConnection* conn, char* msg_name, void* data));
void* iptReceiveFormMsg P_((IPCommunicator*,
                            IPConnection* conn, char* msg_name,
                            double timeout));
int iptReceiveFormMsgContents P_((IPCommunicator*,
                                  IPConnection* conn, char* msg_name,
                                  void* reply_data, double timeout));
void* iptQueryForm P_((IPConnection*, char*, void*, char*));
int iptQueryFormContents P_((IPConnection*, char*, void*, char*, void*));
void iptReplyForm P_((IPMessage* m, char*, void*));
void iptReplyRaw P_((IPMessage* m, char*, int, unsigned char*));
void iptFreeData P_((IPCommunicator*, char* msg_name, void* formatted));
void iptFreeContents P_((IPCommunicator*, char* msg_name, void* formatted));
IPMessage* iptPostponedMessage P_((IPCommunicator*, const char*));

/* IPMessage cover functions */
IPConnection* iptMessageConnection P_((IPMessage* m));
void iptSetMessageConnection P_((IPMessage* m, IPConnection* conn));
int iptMessageSizeData P_((IPMessage* msg));
unsigned char* iptMessageData P_((IPMessage* msg));
int iptMessageInstance P_((IPMessage* msg));
int iptMessageId P_((IPMessage* msg));
void* iptMessageFormData P_((IPMessage* msg));
void* iptMessageFormCopy P_((IPMessage* msg));
void iptMessageFormContents P_((IPMessage* msg, void* res));
void iptMessageFormContentsCopy P_((IPMessage* msg, void* res));
void iptMessageDeleteForm P_((IPMessage* msg, void* data));
void iptMessageDeleteContents P_((IPMessage* msg, void* data));
IPMessageType* iptMessageType P_((IPMessage* msg));
char* iptMessageName P_((IPMessage* msg));
void iptMessageDelete P_((IPMessage* msg));
void iptMessagePrint P_((IPMessage* msg, int print_data));
IPMessage* iptMessageDuplicate P_((IPMessage* msg, IPMessage* out));
IPMessage* iptMessageCopy P_((IPMessage* msg));

/* IPMessageType cover functions */
IPFormat* iptMessageTypeFormat P_((IPMessageType* t));
char* iptMessageTypeName P_((IPMessageType* t));

/* IPFormat cover functions */
int iptFormatDataStructureSize P_((IPFormat* f));
int iptFormatBufferSize P_((IPFormat* f, void* dataStruct));
void iptFormatEncodeData P_((IPFormat* f, void* dataStruct, char* buf));
void* iptFormatDecodeData P_((IPFormat* f, char* buf, int b_size,
                              void* dataStruct,
                              int byte_order, int alignment));
void iptFormatFreeData P_((IPFormat* f, void* dataStruct,
                           char* buf, int buf_size));
void iptFormatFreeDataElements P_((IPFormat* f, void* dataStruct,
                                   char* buf, int buf_size));
int iptFormatFlatStructure P_((IPFormat* f));

/* IPConnection cover functions */
void iptConnectionAddConnectCallback P_((IPConnection* conn,
                                         void (*func) P_((IPCommunicator*,
                                                          IPConnection*,
                                                          void*)),
                                         void* data));
void iptConnectionAddDisconnectCallback P_((IPConnection* conn,
                                            void (*func) P_((IPCommunicator*,
                                                             IPConnection*,
                                                             void*)),
                                            void* data));
int iptConnectionFd P_((IPConnection* conn));
IPCommunicator* iptConnectionCommunicator P_((IPConnection* conn));
char* iptConnectionName P_((IPConnection* conn));
char* iptConnectionHost P_((IPConnection* conn));
int iptConnectionActive P_((IPConnection* conn));
void iptIterateConnections P_((IPCommunicator*,
                               void (*)(IPCommunicator*, IPConnection*, void*),
                               void*));

/* ipt extra functions */
void iptDeclareSubscription P_((IPCommunicator*, char*,
                                void (*func) P_((IPCommunicator*,
                                                 IPConnection*, void*)),
                                void* data));
void iptDeclareTimedSubscription P_((IPCommunicator*, char*,
                                     void (*func) P_((IPCommunicator*,
                                                      IPConnection*, void*)),
                                     void* data));
void iptPublishForm P_((IPCommunicator*, char*, void*));
void iptPublishRaw P_((IPCommunicator*, char*, int, unsigned char*));
IPConnection* iptSubscribe P_((IPCommunicator*, char*, char*));
IPConnection* iptSubscribeFull P_((IPCommunicator*, char*, char*,
                                   char*, void*));
void iptDeclareSubscriptionFull P_((IPCommunicator*, char*, char*,
                                    int (*)(void*, void*, void*),
                                    void* (*)(IPCommunicator*, IPMessage*,
                                              IPConnection*,
                                              void*, void*, void*),
                                    void (*)(void*),
                                    void (*)(IPCommunicator*, IPConnection*,
                                             void*),
                                    void*));
IPConnection* iptSubscribeConnection P_((IPCommunicator*, char*, char*));
void iptUnsubscribe P_((IPCommunicator*, char*, char*));
int iptNumberSubscribers P_((IPCommunicator*, char*));
void iptServer P_((IPCommunicator*, char*,
                   void (*callback) P_((IPCommunicator*, IPMessage*, void*)),
                   void*));
void iptBroadcastForm P_((IPCommunicator*, char*, void*));
void iptBroadcastRaw P_((IPCommunicator*, char*, int, unsigned char*));
IPConnection* iptClient P_((IPCommunicator*, char*));
IPConnection* iptClientForm P_((IPCommunicator*, char*, char*, void*));
IPConnection* iptClientRaw P_((IPCommunicator*,
                               char*, char*, int, unsigned char*));

/* timed out query functions */
IPMessage* iptQueryMsgFormTO P_((IPConnection* conn, char* query_msg_name,
                                 void* data, char* recv_msg_name,
                                 double timeout));
IPMessage* iptQueryMsgRawTO P_((IPConnection* conn, char* query_msg_name,
                              int size, unsigned char* data,
                              char* recv_msg_name, double timeout));
void* iptQueryFormTO P_((IPConnection*, char*, void*, char*, double));
int iptQueryFormContentsTO P_((IPConnection*, char*, void*, char*, void*,
                               double));

/* creating messages */
IPMessage* iptNewMessageRaw P_((IPMessageType* type, int instance,
                                IPConnection* conn,
                                int size, unsigned char* data));
IPMessage* iptNewMessageForm P_((IPMessageType* type, int instance,
                                 IPConnection* conn,
                                 void* form_data));

/* dealing with timers */
IPTimer* iptAddTimer P_((IPCommunicator* comm,
                         double interval, void (*callback)(IPCommunicator*,
                                                           IPTimer*, void*),
                         void* callback_data));
IPTimer* iptAddOneShot P_((IPCommunicator* comm,
                           double interval, void (*callback)(IPCommunicator*,
                                                             IPTimer*, void*),
                           void* callback_data));
void iptRemoveTimer P_((IPCommunicator* comm, IPTimer* timer));
double iptTimerInterval P_((IPTimer*));
int iptTimerOneShot P_((IPTimer*));
double iptTimerTimeLeft P_((IPTimer*));

/* these are kept for backwards compatibility with pre-v4.0 code */
extern IPCommunicator* _ip_communicator;

void IPTinitialize P_((char* mod_name, char* host_machine));
void IPTinitialize_with_communicator P_((IPCommunicator* comm));
IPCommunicator* IPTcommunicator ();
void IPTdeinitialize();
#ifdef VXWORKS
void IPTinitialize_inherited P_((char* task_name));
void IPTdeinitialize_inherited();
#endif
void IPTregister_handler P_((char* msg_name,
                             void (*callback) P_((IPMessage*, void*)),
                             int context, void* data));
void IPTadd_connect_callback P_((void (*func) P_((IPConnection*, void*)),
                                 void* data));
void IPTadd_disconnect_callback P_((void (*func) P_((IPConnection*, void*)),
                                    void* data));
void IPTconnection_add_connect_callback P_((IPConnection* conn,
                                            void (*func) P_((IPConnection*,
                                                             void*)),
                                            void* data));
void IPTconnection_add_disconnect_callback P_((IPConnection* conn,
                                               void (*func) P_((IPConnection*,
                                                                void*)),
                                               void* data));
void IPTclear_list_cache();

/* iptMessageFormContentsCopy was misnamed before v6.4 */
#define iptMessageFormContentsData iptMessageFormContentsCopy

/* macros for backward compatibility with prev v4.0 versions */
#define IPTregister_message(msg_name, format) \
	iptRegisterMessage(_ip_communicator,(msg_name),(format))
#define IPTregister_messages(messages) \
	iptRegisterMessages(_ip_communicator,(messages))
#define IPTconnect(name, required) \
	iptConnect(_ip_communicator,(name),(required))
#define IPTmain_loop() iptMainLoop(_ip_communicator)
#define IPTsleep(sleep_time) iptSleep(_ip_communicator, (sleep_time))
#define IPTidle(sleep_time) iptIdle(_ip_communicator, (sleep_time))
#define IPTlookup_message(name) iptLookupMessage(_ip_communicator,(name))
#define IPTmessage_by_id(id) iptMessageById(_ip_communicator,(id))
#define IPTreceive_msg(conn, msg_name, timeout) \
	iptReceiveMsg(_ip_communicator,(conn),(msg_name),(timeout))
#define IPTconnection(conn_name) iptConnection(_ip_communicator,(conn_name))
#define IPTpigeon_hole(msg_name) iptPigeonHole(_ip_communicator,(msg_name))
#define IPTpigeon_hole_type(type) iptPigeonHoleType(_ip_communicator,(type))
#define IPTthis_host() iptThisHost(_ip_communicator)
#define IPTmodule_name() iptModuleName(_ip_communicator)
#define IPTdomain_name() iptDomainName(_ip_communicator)
#define IPTdisable_handlers() iptDisableHandlers(_ip_communicator)
#define IPTenable_handlers() iptEnableHandlers(_ip_communicator)
#define IPTdisable_msg_handler(type) \
	iptDisableMsgHandler(_ip_communicator,(type))
#define IPTenable_msg_handler(type) \
	iptEnableMsgHandler(_ip_communicator, (type))
#define IPTquery_msg_form(conn, query_msg_name, data, recv_msg_name) \
	iptQueryMsgForm((conn), (query_msg_name), (data), (recv_msg_name))
#define IPTquery_msg_raw(conn, query_msg_name, size, data, recv_msg_name) \
	iptQueryMsgRaw((conn), (query_msg_name), (size), (data), (recv_msg_name))
#define IPTsend_raw_msg(conn, type, size, data) \
	iptSendRawMsg((conn), (type), (size), (data))
#define IPTreceive_raw_msg(conn, type, max_size, buffer, timeout) \
	iptReceiveRawMsg(_ip_communicator, (conn), (type), (max_size), (buffer), \
		(timeout))
#define IPTsend_form_msg(conn, msg_name, data) \
	iptSendFormMsg((conn), (msg_name), (data))
#define IPTreceive_form_msg(conn, msg_name, timeout) \
	iptReceiveFormMsg(_ip_communicator, (conn), (msg_name), (timeout))
#define IPTreceive_form_msg_contents(conn, msg_name, reply_data, timeout) \
	iptReceiveFormMsgContents(_ip_communicator, (conn), (msg_name), \
		(reply_data), (timeout))
#define IPTquery_form(conn, query_msg_name, query_data, recv_msg_name) \
	iptQueryForm((conn), (query_msg_name), (query_data), (recv_msg_name))
#define IPTquery_form_contents(conn, query_msg_name, query_data, recv_msg_name, reply_data) \
		iptQueryFormContents((conn), (query_msg_name), (query_data), \
			(recv_msg_name), (reply_data))
#define IPTreply_form(m, msg_name, data) iptReplyForm((m), (msg_name), (data))
#define IPTreply_raw(m, msg_name, size, data) \
	iptReplyRaw((m), (msg_name), (size), (data))
#define IPTfree_data(msg_name, formatted) \
	iptFreeData(_ip_communicator, (msg_name), (formatted))
#define IPTfree_contents(msg_name, formatted) \
	iptFreeContents(_ip_communicator, (msg_name), (formatted))
#define IPTmessage_connection(m) iptMessageConnection(m)
#define IPTmessage_size_data(msg) iptMessageSizeData(msg)
#define IPTmessage_data(msg) iptMessageData(msg)
#define IPTmessage_instance(msg) iptMessageInstance(msg)
#define IPTmessage_ID(msg) iptMessageId(msg)
#define IPTmessage_form_data(msg) iptMessageFormData(msg)
#define IPTmessage_form_copy(msg) iptMessageFormCopy(msg)
#define IPTmessage_form_contents(msg, res) iptMessageFormContents((msg), (res))
#define IPTmessage_form_contents_copy(msg, res) \
	iptMessageFormContentsCopy((msg), (res))
#define IPTmessage_delete_form(msg, data) iptMessageDeleteForm((msg), (data))
#define IPTmessage_delete_contents(msg, data) \
	iptMessageDeleteContents((msg), (data))
#define IPTmessage_type(msg) iptMessageType(msg)
#define IPTmessage_name(msg) iptMessageName(msg)
#define IPTmessage_delete(msg) iptMessageDelete(msg)
#define IPTmessage_print(msg, print_data) iptMessagePrint((msg), (print_data))
#define IPTmessage_duplicate(msg, out) iptMessageDuplicate((msg), (out))
#define IPTmessage_copy(msg) iptMessageCopy(msg)
#define IPTconnection_fd(conn) iptConnectionFd(conn)
#define IPTconnection_name(conn) iptConnectionName(conn)
#define IPTconnection_host(conn) iptConnectionHost(conn)
#define IPTconnection_active(conn) iptConnectionActive(conn)
#define IPTiterate_connections(func, data) \
	iptIterateConnections(_ip_communicator, (func), (data))
#define IPTdeclare_subscription(message_name, func, data) \
	iptDeclareSubscription(_ip_communicator, (message_name), (func), (data))
#define IPTdeclare_timed_subscription(msg_name, func, data) \
	iptDeclareTimedSubscription(_ip_communicator, (msg_name), (func), (data))
#define IPTpublish_form(msg_name, data) \
	iptPublishForm(_ip_communicator, (msg_name), (data))
#define IPTpublish_raw(msg_name, size, data) \
	iptPublishRaw(_ip_communicator, (msg_name), (size), (data))
#define IPTsubscribe(mod_name, msg_name) \
	iptSubscribe(_ip_communicator, (mod_name), (msg_name))
#define IPTsubscribe_connection(conn, msg_name) \
	iptSubscribeConnection(_ip_communicator, (conn), (msg_name))
#define IPTunsubscribe(mod_name, msg_name) \
	iptUnsubscribe(_ip_communicator, (mod_name), (msg_name))
#define IPTnumber_subscribers(msg_name) \
	iptNumberSubscribers(_ip_communicator, (msg_name))
#define IPTserver(msg_name, callback, data) \
	iptServer(_ip_communicator, (msg_name), (callback), (data))
#define IPTbroadcast_form(msg_name, data) \
	iptBroadcastForm(_ip_communicator, (msg_name), (data))
#define IPTbroadcast_raw(msg_name, size, data) \
	iptBroadcastRaw(_ip_communicator, (msg_name), (size), (data))
#define IPTclient(server_name) iptClient(_ip_communicator, (server_name))
#define IPTclient_form(server_name, msg_name, data) \
	iptClientForm(_ip_communicator, (server_name), (msg_name), (data))
#define IPTclient_raw(server_name, msg_name, size, data) \
	iptClientRaw(_ip_communicator, (server_name), (msg_name), (size), (data))

#if defined(__cplusplus)
}
#endif


#endif
