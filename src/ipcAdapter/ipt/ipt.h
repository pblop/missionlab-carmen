#ifndef ipt_h
#define ipt_h

#include <stdio.h>
#include <string.h>

//class IPMessageType;
//class IPConnection;
//class IPHandlerCallback;
//class IPMessage;
//class IPCommunicator;
//class IPConnectionCallback;

#include <ipt/message.h>
#include <ipt/connection.h>
#include <ipt/callbacks.h>
#include <ipt/spec.h>
#include <ipc.h>

using namespace std;

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
#define IPT_RECV_ALL  1

typedef const char IPMessageType;
//typedef const char IPConnection;


class IPCommunicator {
  public:

    /* create an appropriate instance of IPCommunicator */
    static IPCommunicator* Instance(const char* mod_name,
                                    const char* host_name=NULL);

    /* create a communicator with a module name connected to a host.  If the
       host_name is nil, look at the environment variable COMM_HOST */
    IPCommunicator(const char* mod_name, const char* host_name = NULL);

    /* destroy a communicator */
    virtual ~IPCommunicator();


    void RegisterMessages(IPMessageSpec*);
    void RegisterNamedFormatters(IPFormatSpec* specs);


    /* make a connection to another module */
    virtual IPConnection* Connect(const char* name,
                                  int = IPT_REQUIRED)
        { return new IPConnection(strdup(name)); }


    void RegisterHandler(IPMessageType* mess,
                         IPHandlerCallback* callback, int spec = IPT_HNDL_STD);
    void RegisterHandler(IPMessageType* mess,
                         void (*)(class IPCommunicator*, IPMessage*, void*),
                         void* callback_data = NULL,
                         int spec = IPT_HNDL_STD);

    /* Sends a message across a connection */
    int SendMessage(IPConnection*, const char* msg_name, void* data);


    /* routines for calling back when things connect and disconnect */
    void AddConnectCallback(IPConnectionCallback*);
    void AddConnectCallback(void (*)(IPConnection*, void*), void* = NULL);

    void AddDisconnectCallback(IPConnectionCallback*);
    void AddDisconnectCallback(void (*)(IPConnection*, void*), void* = NULL);

    void DisableHandler(IPMessageType*);

    /* loops for handling events */
    int Sleep(double time = IPT_BLOCK);
    int Idle(double time = IPT_BLOCK);


    /* lookup message types by name and ID */
    IPMessageType* LookupMessage(const char*);
    //IPMessageType* LookupMessage(int);

    /* Return the name of the machine the module is on */
    const char* ThisHost() const;
    /* Return the name of the module */
    const char* ModuleName() const { return _mod_name; }
    /* Return the name of the machine the server is on */
    const char* ServerHostName() const { return _host_name; }
    const char* DomainName() const { return _domain_name; }
    //void SetDomainName(char* name) { _domain_name = name; }



    IPMessage* Query(IPConnection*, const char*, void*,
                     const char*, double = IPT_BLOCK);


    void* QueryFormatted(IPConnection*, const char*, void*, const char*,
                         double = IPT_BLOCK);



    /* Reply to a message */
    void Reply(IPMessage*, const char*, void*);

    virtual int ScanForMessages(double);

    /* general server/client routines */
    void Server();

    void Broadcast(const char*, void*);


  private:
      IPMessage* QueryFormattedEx(IPConnection*, const char*, void*, const char*,
	                           double = IPT_BLOCK);
	  const char*  _mod_name;
	  const char* _host_name;
	  const char* _domain_name;
	  char* _this_host;
	  IPC_CONTEXT_PTR context;
};

extern "C" {

IPConnection* iptClient (IPCommunicator*, const char*);

}

#endif
