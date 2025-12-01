/**********************************************************************
 **                                                                  **
 **                          GT_MPPC_client.h                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: GT_MPPC_client.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef GT_MPPC_CLIENT_H
#define GT_MPPC_CLIENT_H

#define MAXDATASIZE 10000 // max number of bytes we can get at once  HAD TO CHANGE THIS
#define BACKLOG 10     // how many pending connections queue will hold

#include <vector>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h> 
#include <pthread.h>

using std::vector;
using std::string;


//this class is a thread-safe Meyer's singleton 


class GT_MPPC_client {

//public interface
public:

    static GT_MPPC_client* Instance();

    int SetupClientSocket();
    int SetupServerSocket();
    void Subscribe(string subscriptionType, string messageType, 
                   string dataType);
    void Unsubscribe(string subscriptionType, string messageType,
                     string dataType);
    void CloseSockets();

    char* GetCIMHostname() { return strCIMHostname; }
    void  SetCIMHostname( char* hstnm ) { strCIMHostname = hstnm; }
    int   GetCIMPortNumber() { return iCIMPortNumber; } 
    void  SetCIMPortNumber( int prt) { iCIMPortNumber = prt; }
    char* GetMPPCHostname() { return strMPPCHostname; }
    void  SetMPPCHostname( char* hstnm ) { strMPPCHostname = hstnm; }
    int   GetMPPCPortNumber() { return iMPPCPortNumber; } 
    void  SetMPPCPortNumber( int prt) { iMPPCPortNumber = prt; }

    void  AddSubscription( string sub ) { vecSubscriptions.push_back( sub ); }
    void  RemoveSubscription( string sub );
    string GetSubscription(int i) {return vecSubscriptions[i]; }
    int GetNumberOfSubscriptions() { return vecSubscriptions.size(); }
    bool GetClientSocketConnected()       { return bClientSocketConnected; }
    bool GetListeningToServerSocket()       { return bListeningToServerSocket; }
    bool SubscriptionExist( string st);
    void UnsubscribeAll();
    void ListenForMessages();
    void IncrementMessagesSent()            { iNumberOfMessagesSent++; }
    int  GetNumberOfMessagesSent()          { return iNumberOfMessagesSent; }
    string GetEnvironmentMessage();
    void SendString(char *msg);

//public data
public: 
    pthread_t MPPC_client_thread_;

protected:
    static void *startReaderThread_(void *Instance);
    void readerLoop_(void);

//internal methods
private:
  
    GT_MPPC_client(void);
    ~GT_MPPC_client(void)             {};
 
    void Sigchld_handler(int s);

    //  void ListenForMessages();
    void BuildLoiterObjective();
    void BuildPGMObjective();
    void BuildSubscription(const char *subscriptionType,const char *messageType,
                           char *msg_namespace,   const char *dataType,
                           int interval,char *operation);
    void SetClientSocketConnected(bool b) { bClientSocketConnected = b;}
    void SetListeningToServerSocket(bool b) { bListeningToServerSocket = b;}
    int ConnectTimeout(int sfd, struct sockaddr *addr, struct timeval *to);
 
 

//data members
private:
    int iClientSocket;
    int iServerSocket;
    int iConnectionSocket;
    int iUniqueNum;// = 0;
    char* strComponentName;

    char* strMPPCHostname;
    int iMPPCPortNumber;

    char* strCIMHostname;
    int   iCIMPortNumber;
  
    bool bClientSocketConnected;
    bool bListeningToServerSocket;
    vector<string> vecSubscriptions;
    int iNumberOfMessagesSent; 
    static GT_MPPC_client* smInstance;
    static pthread_mutex_t smMutex;
    static pthread_mutex_t smPortMutex_;
    static pthread_mutex_t smStringMutex;
    string strEnvironmentMessage;
    string strOtherMessage;

    static const string RCTABLE_CIM_HOSTNAME_STRING_;
    static const string RCTABLE_CIM_PORT_NUMBER_STRING_;
    static const string RCTABLE_MPPC_HOSTNAME_STRING_;
    static const string RCTABLE_MPPC_PORT_NUMBER_STRING_;
    static const char *DEFAULT_CIM_HOSTNAME_;
    static const char *DEFAULT_MPPC_HOSTNAME_;
    static const int DEFAULT_CIM_PORT_NUMBER_;
    static const int DEFAULT_MPPC_PORT_NUMBER_;
};

#endif

/**********************************************************************
 * $Log: GT_MPPC_client.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.10  2007/10/01 17:30:15  endo
 * Hostnames and port numbers can be now specified via .cfgeditrc
 *
 * Revision 1.9  2007/06/21 02:52:38  pulam
 * Changed connect code to use non-blocking sockets
 *
 * Revision 1.8  2006/10/23 22:18:00  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.7  2006/10/19 22:05:43  alanwags
 * Added code for recieving environment message
 *
 * Revision 1.6  2006/09/27 17:15:45  alanwags
 * Fixes for the messages and recieving messages
 *
 * Revision 1.5  2006/09/21 14:30:55  endo
 * Added CVS header. sigaction commented.
 *
 **********************************************************************/
