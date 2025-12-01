/**********************************************************************
 **                                                                  **
 **                         GT_MPPC_client.cpp                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: GT_MPPC_client.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#include "GT_MPPC_client.h"
#include "load_rc.h"
#include <string>
#include <fcntl.h>

const string GT_MPPC_client::RCTABLE_CIM_HOSTNAME_STRING_ = "CIMHostname";
const string GT_MPPC_client::RCTABLE_CIM_PORT_NUMBER_STRING_ = "CIMPortNumber";
const string GT_MPPC_client::RCTABLE_MPPC_HOSTNAME_STRING_ = "MPPCHostname";
const string GT_MPPC_client::RCTABLE_MPPC_PORT_NUMBER_STRING_ = "MPPCPortNumber";
const char *GT_MPPC_client::DEFAULT_CIM_HOSTNAME_ = "127.0.0.1";
const char *GT_MPPC_client::DEFAULT_MPPC_HOSTNAME_ = "127.0.0.1";
const int GT_MPPC_client::DEFAULT_CIM_PORT_NUMBER_ = 7654;
const int GT_MPPC_client::DEFAULT_MPPC_PORT_NUMBER_ = 4567;

extern symbol_table<rc_chain> rc_table;

GT_MPPC_client* GT_MPPC_client::smInstance = NULL;
pthread_mutex_t GT_MPPC_client::smMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t GT_MPPC_client::smPortMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t GT_MPPC_client::smStringMutex = PTHREAD_MUTEX_INITIALIZER;

GT_MPPC_client* GT_MPPC_client::Instance()
{
    GT_MPPC_client *mppc = NULL;

    // Copy first.
    pthread_mutex_lock(&smMutex);
    mppc = smInstance;
    pthread_mutex_unlock(&smMutex);

    if (mppc == NULL)
    {
        // smInstance assigned inside the constructor.
        new GT_MPPC_client();

        // Copy again.
        pthread_mutex_lock(&smMutex);
        mppc = smInstance;
        pthread_mutex_unlock(&smMutex);

        if (mppc == NULL)
        {
            fprintf(stderr, "Error: GT_MPPC_client::Instance(). The instance could not be assigned.\n");
        }
    }

    return mppc;
}

/* 
 * helper function
 */ 
void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
GT_MPPC_client::GT_MPPC_client() 
{
    rc_chain *rcChain = NULL;
    char *buf = NULL;

    pthread_mutex_lock(&smMutex);
    smInstance = this;
    pthread_mutex_unlock(&smMutex);
 	
    iUniqueNum = 0;
    strComponentName ="MPPC";

    rcChain = (rc_chain *)rc_table.get(RCTABLE_MPPC_HOSTNAME_STRING_.c_str());

    if (rcChain != NULL)
    {
        rcChain->first(&strMPPCHostname);
    }

    if (strMPPCHostname == NULL)
    {
        strMPPCHostname = (char *)DEFAULT_MPPC_HOSTNAME_;
    }

    rcChain = (rc_chain *)rc_table.get(RCTABLE_MPPC_PORT_NUMBER_STRING_.c_str());

    if (rcChain != NULL)
    {
        rcChain->first(&buf);
    }

    if (buf == NULL)
    {
        iMPPCPortNumber = DEFAULT_MPPC_PORT_NUMBER_;
    }
    else
    {
        iMPPCPortNumber = atoi(buf);
    }

    rcChain = (rc_chain *)rc_table.get(RCTABLE_CIM_HOSTNAME_STRING_.c_str());

    if (rcChain != NULL)
    {
        rcChain->first(&strCIMHostname);
    }

    if (strCIMHostname == NULL)
    {
        strCIMHostname = (char *)DEFAULT_CIM_HOSTNAME_;
    }

    rcChain = (rc_chain *)rc_table.get(RCTABLE_CIM_PORT_NUMBER_STRING_.c_str());

    if (rcChain != NULL)
    {
        rcChain->first(&buf);
    }

    if (buf == NULL)
    {
        iCIMPortNumber = DEFAULT_CIM_PORT_NUMBER_;
    }
    else
    {
        iCIMPortNumber = atoi(buf);
    }

    bClientSocketConnected=false;
    bListeningToServerSocket=false;
    vecSubscriptions.empty(); 
    iNumberOfMessagesSent=0;	

    pthread_mutex_lock(&smStringMutex);
    strEnvironmentMessage = "";
    strOtherMessage = "";  
    pthread_mutex_unlock(&smStringMutex);

    pthread_create(&MPPC_client_thread_, NULL, &startReaderThread_, (void*)this);
}

//-----------------------------------------------------------------------
// This function starts the reader thread.
//-----------------------------------------------------------------------

void *GT_MPPC_client::startReaderThread_(void *vInstance)
{
  ((GT_MPPC_client*) vInstance)->ListenForMessages();
  return NULL;
}


//-----------------------------------------------------------------------
// This function returns the environment message.
//-----------------------------------------------------------------------
std::string GT_MPPC_client::GetEnvironmentMessage(void)
{
    std::string envMsg;

    pthread_mutex_lock(&smStringMutex);
    envMsg = strEnvironmentMessage;
    pthread_mutex_unlock(&smStringMutex);

    return envMsg;
}

/* 
 * Sends a string to the socket defined by the global variable clientSocket. 
 * clientSocket must have been initialized prior to calling this.
 */
void GT_MPPC_client::SendString(char *msg){

  int len = strlen(msg);
  IncrementMessagesSent();

  pthread_mutex_lock(&smPortMutex_);

  if (send(iClientSocket, msg, len, 0) == -1){
    perror("send");  
    pthread_mutex_unlock(&smPortMutex_);
    return;
  }

  if (send(iClientSocket, "\n\n", 2, 0) == -1){
    perror("send");	
    pthread_mutex_unlock(&smPortMutex_);
    return;
  } 	

  pthread_mutex_unlock(&smPortMutex_);
}

/*
 * Setup client socket (The socket connecting to the CIM)
 */
int GT_MPPC_client::SetupClientSocket(){
  
  struct hostent *he;
  struct sockaddr_in their_addr; //  connector's address information 

  // Get the CIM Hostname and port
  if ((he=gethostbyname(strCIMHostname)) == NULL) {  // get the host info 
    herror("gethostbyname");
    return -1;
  }

  // Open the socket to the CIM
  if ((iClientSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }
  their_addr.sin_family = AF_INET;    // host byte order 
  their_addr.sin_port = htons(iCIMPortNumber);  // short, network byte order 
  their_addr.sin_addr = *((struct in_addr *)he->h_addr);
  memset(&(their_addr.sin_zero), '\0', 8);  // zero the rest of the struct 

  //if (connect(iClientSocket, (struct sockaddr *)&their_addr,
//	      sizeof(struct sockaddr)) == -1) {

  timeval to;
  to.tv_sec = 5;
  to.tv_usec = 0;
  if (ConnectTimeout(iClientSocket, (struct sockaddr *)&their_addr, &to) == -1) {
    perror("connect");
    return -1;
  }
  SetClientSocketConnected( true );

  return 0;
}

int GT_MPPC_client::ConnectTimeout(int sfd, struct sockaddr *addr, struct timeval *to) {
	int ret;


	int sockflags = fcntl(sfd, F_GETFL, 0);
	fcntl(sfd, F_SETFL, sockflags | O_NONBLOCK);
	
	if ((ret = connect(sfd, addr, sizeof(struct sockaddr))) < 0) {
		if (errno != EINPROGRESS) {
			return -1;
		}
	}
	if (ret != 0) {
		fd_set rset, wset;
	
		FD_ZERO(&rset);
		FD_ZERO(&wset);
		FD_SET(sfd, &rset);
		FD_SET(sfd, &wset);

		if ((ret = select(sfd + 1, &rset, &wset, NULL, to)) == 0) {
			close(sfd);
			errno = ETIMEDOUT;
			return -1;
		}

		fcntl(sfd, F_SETFL, sockflags);

	}
	return 0;
}


void GT_MPPC_client::CloseSockets(){

  close(iClientSocket);
  close(iServerSocket);
}

/*
 * Setup server socket (The socket receiving data from the CIM)
 */
int GT_MPPC_client::SetupServerSocket(){

  int yes = 1;
  struct sockaddr_in my_addr;    // my address information
  struct sigaction sa;

  if ((iServerSocket = socket(PF_INET, SOCK_STREAM, 0)) == -1){
    perror("socket");
    return -1;
  }

  if (setsockopt(iServerSocket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1){
    perror("setsockopt");
    return -1;
  }

  my_addr.sin_family = AF_INET;         // host byte order
  my_addr.sin_port = htons(iMPPCPortNumber);     // short, network byte order
  my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
  memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

  if (bind(iServerSocket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))== -1) {
    perror("bind");
	return -1;
  }

  if (listen(iServerSocket, BACKLOG) == -1) {
    perror("listen");
    return -1;
  }

  sa.sa_handler = sigchld_handler; // reap all dead processes
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;

  /* Commented because of the confliction with waitpid().
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    return -1;
  }
  */
  SetListeningToServerSocket( true );
  return 0;
}

/*
 * Will listen for incoming messages on the server socket
 */

void GT_MPPC_client::ListenForMessages(){
  
    std::string strMessage;
    int iMessageRecievedNum=0;
    socklen_t sin_size;
    struct sockaddr_in their_addr;
    const bool DEBUG_ENV_MESSAGE = true;

    while(true){ 
        pthread_testcancel();

        if(bClientSocketConnected == true ) {
            sin_size = sizeof(struct sockaddr_in);
	 
            if ((iConnectionSocket = accept(iServerSocket, (struct sockaddr *)&their_addr,&sin_size)) == -1){
                perror("accept");
                usleep(1000);
                continue;
            }
 
            iMessageRecievedNum++;

            char message[MAXDATASIZE];
            int messageSize = 0;
            char buffer[1];
            int numNewLines = 0;
            
            //if (!fork())
            { // this is the child process
      
                while (numNewLines <2 && recv(iConnectionSocket, buffer, 1, 0) > 0)
                {
                    if (buffer[0] == '\n')
                    {
                        numNewLines = numNewLines +1;
                    }
                    else
                    {
                        numNewLines = 0;
                    }

                    if (messageSize < MAXDATASIZE-1)
                    {
                        message[messageSize] = buffer[0];
                        messageSize = messageSize +1;
                    }
                }//end while
   
                message[messageSize] = '\0';


            }//end if  
            close(iConnectionSocket);
            pthread_testcancel();

            //std::string strMessage( message );
            strMessage = message;

            if (!strMessage.empty())
            {
                pthread_mutex_lock(&smStringMutex);
                if(strMessage.find("message_type>Environment") != string::npos)
                {
                    strEnvironmentMessage = strMessage;
                }
                else
                {
                    strOtherMessage = strMessage;
                }
		
                pthread_mutex_unlock(&smStringMutex);

                if (DEBUG_ENV_MESSAGE)
                {
                    fprintf(
                        stderr,
                        "\nGT_MPPC_client::ListenForMessages(): strEnvironmentMessage = [%s]\n\n",
                        strEnvironmentMessage.c_str());
                }
            }

        }//end if
        usleep(1000);
    }//end while
}

/*
 * This function will build a subscription message from the
 * parameters provided.
 */
void GT_MPPC_client::BuildSubscription(const char *subscriptionType,
		       const char *messageType,
		       char *msg_namespace,
		       const char *dataType,
		       int interval,
		       char *operation){

  char msg[1000];
  char tmp[1000];

  strcpy(msg,"<subscription_object>");
  strcat(msg,"<priority>3</priority>");

  strcat(msg,"<timestamp>");
  sprintf(tmp, "%ld", time(NULL));
  strcat(msg, tmp);
  strcat(msg, "</timestamp>");

  strcat(msg, "<message_number>");
  sprintf(tmp, "%d", iUniqueNum);
  iUniqueNum = iUniqueNum+1;
  strcat(msg, tmp);
  strcat(msg, "</message_number>");

  strcat(msg, "<message_type>");
  strcat(msg, "SubscriptionRequest");
  strcat(msg, "</message_type>");
  
  strcat(msg, "<source>");
  strcat(msg, strComponentName);
  strcat(msg, "</source>");

  strcat(msg, "<operation>");
  strcat(msg, operation);
  strcat(msg, "</operation>");

  strcat(msg, "<content>");
  
  strcat(msg, "<subscription type=\"");
  strcat(msg, subscriptionType);
  strcat(msg, "\">");
  
  strcat(msg, "<subscriber>");
  
  strcat(msg, "<name>");
  strcat(msg, strComponentName);
  strcat(msg, "</name>");
  
  strcat(msg, "<contact>");
  
  strcat(msg, "<ip>");
  strcat(msg, strMPPCHostname);
  strcat(msg, "</ip>");

  strcat(msg, "<port>");
  sprintf(tmp, "%d", iMPPCPortNumber);
  strcat(msg, tmp);
  strcat(msg, "</port>");

  strcat(msg, "<transmissiontype>socket-tcp</transmissiontype>");
  
  strcat(msg, "<datatype>");
  strcat(msg, dataType);
  strcat(msg, "</datatype>");
  
  strcat(msg, "</contact>");
  
  strcat(msg, "</subscriber>");
  
  strcat(msg, "<message_type>");
  strcat(msg, messageType);
  strcat(msg, "</message_type>");
  
  strcat(msg, "<namespace>");
  strcat(msg, msg_namespace);
  strcat(msg, "</namespace>");
 
  strcat(msg, "<interval>");
  sprintf(tmp, "%d", interval);
  strcat(msg, tmp);
  strcat(msg, "</interval>");
  strcat(msg, "</subscription>");
  strcat(msg, "</content>");
  strcat(msg, "</subscription_object>");

  SendString(msg);
}

/* 
 * Subscribes to the CIM 
 */
void GT_MPPC_client::Subscribe(std::string subscriptionType, std::string messageType, 
							   std::string dataType){

  BuildSubscription(subscriptionType.c_str(),messageType.c_str(),  
					"", dataType.c_str(), 
					1, "add");

  std::string strSubscript = subscriptionType + ";";
  strSubscript += messageType + ";";
  strSubscript += dataType;
  AddSubscription( strSubscript );  
}

/*
 * This function will build an unsubscription message from the
 * parameters provided.
 */
void GT_MPPC_client::Unsubscribe(std::string subscriptionType,
		 std::string messageType, std::string dataType){

  BuildSubscription(subscriptionType.c_str(), messageType.c_str(), 
					"", dataType.c_str(),
					0, "delete");
  
  std::string strSubscript = subscriptionType + ";";
  strSubscript += messageType + ";";
  strSubscript += dataType;
  RemoveSubscription( strSubscript );  
}

/*
 * Removes a subscription from the subscription list
 *
 */
void GT_MPPC_client::RemoveSubscription(std::string st) {

  std::vector< std::string >::iterator iter;
  for(iter =vecSubscriptions.begin();iter!=vecSubscriptions.end(); iter++) {
	if((*iter) == st ) {
	  vecSubscriptions.erase( iter);
	  break;
	}
  }
}

/*
 * Checks if a subscription exists
 *
 */
bool GT_MPPC_client::SubscriptionExist( std::string st) {	
  for(size_t i=0;i<vecSubscriptions.size(); i++) {
	if(vecSubscriptions[i] == st )
	  return true;
  }
  return false;
}

/*
 * Unsubscribe from all current subscriptions
 *
 */
void GT_MPPC_client::UnsubscribeAll(){

  for(size_t i=0;i<vecSubscriptions.size(); i++) {
	std::string strSubscript = vecSubscriptions[i];
	RemoveSubscription( strSubscript );

	char* pch; 
	pch = strtok((char*)strSubscript.c_str(),";");
	std::string strSubscription = pch;
	pch = strtok (NULL, ";");
	std::string strMessage = pch;
	pch = strtok (NULL, ";");
	std::string strDataType = pch;
	
	BuildSubscription(strSubscript.c_str(), strMessage.c_str(), 
					"", strDataType.c_str(),
					0, "delete");

  } 
}



/*     
int main(int argc, char *argv[])
{

  //If your running the cim on a local machine then names are localhost and ports are cimport 7654 this port 4567 
  if (argc != 5) {
    fprintf(stderr,"usage: client cimHostname cimPort thisHostname thisPort\n");
	fprintf(stderr,"Using default parameters instead\ncimHostname=localhost cimPort=7654 thisHostname=localhost thisPort=4567\n");	
	// Setup the server socket
	setupServerSocket();
	// Setup the client socket
   	setupClientSocket("localhost", 7654);
  }
  else {
	 // Set this hostname and port num
	thisHost = argv[3];
	thisPort = atoi(argv[4]);
 
	// Setup the server socket
	setupServerSocket();

	// Setup the client socket
	setupClientSocket(argv[1], atoi(argv[2]));
  }
  
  // Send a string to the CIM
  subscribe("onetime","AllEntities","","xmlstring",1); 
  subscribe("onetime","AvailableAssets","","xmlstring",1);
  fprintf(stderr,"BUILDING OBJECTIVE\n");
  buildLoiterObjective();
  fprintf(stderr,"BUILDING PGM OBJECTIVE\n");
  buildPGMObjective();
  sleep(5);
  fprintf(stderr,"Sleep over\n");	
  unsubscribe("onetime","AllEntities","","xmlstring");

  //sendString("Hello World");

  // Listen for responses
  listenForMessages();

  // The code will never reach here unless a listen fails

  // Close the sockets
  close(clientSocket);
  close(serverSocket);
  
  return 0;
} 
*/

/**********************************************************************
 * $Log: GT_MPPC_client.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.13  2007/10/01 17:30:15  endo
 * Hostnames and port numbers can be now specified via .cfgeditrc
 *
 * Revision 1.12  2007/08/17 17:49:33  alanwags
 * Revised XML for latest schema
 *
 * Revision 1.11  2007/06/21 02:52:38  pulam
 * Changed connect code to use non-blocking sockets
 *
 * Revision 1.10  2006/10/23 22:18:00  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.9  2006/10/19 22:06:24  alanwags
 * Added code for recieving environment message
 *
 * Revision 1.8  2006/10/12 16:10:43  alanwags
 * Ft. Worth changes for integration excercise
 *
 * Revision 1.7  2006/10/02 18:31:26  alanwags
 * Closing socket allows for reading of additional lines
 *
 * Revision 1.6  2006/09/27 17:15:45  alanwags
 * Fixes for the messages and recieving messages
 *
 * Revision 1.5  2006/09/21 14:30:55  endo
 * Added CVS header. sigaction commented.
 *
 **********************************************************************/
