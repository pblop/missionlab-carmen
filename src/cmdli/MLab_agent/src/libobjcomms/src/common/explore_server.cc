
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "mic.h"
#include "deque"
#include "objcomms.h"
#include "rawMsg.h"
#include "MsgTypes.h"

namespace sara
{

#define MAXBUFSIZE 256

class conInfo
{
   public:
      conInfo() {addr_len = sizeof(addr);}
      int sd;
      struct sockaddr addr;
      int addr_len;
};
typedef deque<conInfo *> conInfoContainer;
conInfoContainer cons;


int main(int, char **)
{
   int our_port = 45123;

   /* define our local name */
   int s;
   if( (s = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
   {
      perror("RUNTIME_INIT: Can not create socket");
      exit(1);
   }

   /* assemble socket bindings */
   struct hostent *our_host;
   if( (our_host = gethostbyname("localhost")) == NULL )
   {
      perror("RUNTIME_INIT: Can not find host address");
      exit(2);
   }

   /* fill in socket descripter record for our socket */
   struct sockaddr_in our_socket;
   bzero((caddr_t)&our_socket, sizeof(our_socket));
   our_socket.sin_family = our_host->h_addrtype;
   bcopy(our_host->h_addr, (char *)&our_socket.sin_addr, our_host->h_length);
   our_socket.sin_port = our_port;

   /* bind socket to a global name */
   if( bind(s,(struct sockaddr *)&our_socket,sizeof(our_socket)) == -1 )
   {
      perror("RUNTIME_INIT: Can not bind");
      exit(3);
   }

   // Attach the socket as a server
   if( listen(s, 1000) < 0 )
   {
      perror("Can not listen");
      exit(4);
   }



   /* define our local name */
   int s2;
   if( (s2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 )
   {
      perror("RUNTIME_INIT: Can not create socket 2");
      exit(1);
   }

   /* assemble socket bindings */
   struct hostent *our_host2;
   if( (our_host2 = gethostbyname("localhost")) == NULL )
   {
      perror("RUNTIME_INIT: Can not find host address 2");
      exit(2);
   }

   /* fill in socket descripter record for our socket */
   struct sockaddr_in our_socket2;
   bzero((caddr_t)&our_socket2, sizeof(our_socket2));
   our_socket2.sin_family = our_host2->h_addrtype;
   bcopy(our_host2->h_addr, (char *)&our_socket2.sin_addr, our_host2->h_length);
   our_socket2.sin_port = our_port;

   /* bind socket to a global name */
   if( bind(s2,(struct sockaddr *)&our_socket2,sizeof(our_socket2)) == -1 )
   {
      perror("RUNTIME_INIT: Can not bind2");
      exit(3);
   }






   fd_set rfds;
   while(1)
   {
      // wait for something to arrive
      FD_ZERO(&rfds);
      FD_SET(s, &rfds);
      int highest = s;
      conInfoContainer::iterator it;
      for(it=cons.begin(); it!=cons.end(); ++it)
      {
         FD_SET((*it)->sd, &rfds);
	 if( (*it)->sd > highest )
	    highest = (*it)->sd;
      }

      int rtn = select(highest+1, &rfds, NULL, NULL, NULL);
      if(rtn < 0 )
      {
         perror("Error from select");
         exit(3);
      }
      if( rtn > 0 )
      {
         // check our main socket
	 if( FD_ISSET(s, &rfds) )
	 {
	    conInfo *newcon = new conInfo();
            newcon->sd = accept(s, &newcon->addr, (socklen_t *)&newcon->addr_len);
            cons.push_back( newcon );
	 }

	 // check our open connections
         bool restart;
         do
         {
            restart = false;
            for(it=cons.begin(); it!=cons.end(); ++it)
            {
	       // is there something here?
               if( FD_ISSET((*it)->sd, &rfds) )
	       {
                  unsigned char inbuf[MAXBUFSIZE];
                  int inlng;
                  if( (inlng = recv((*it)->sd,inbuf,MAXBUFSIZE,0))== -1 )
                  {
                     perror("Error in recvfrom");
                     exit(3);
                  }
                  if( inlng == 0 )
                  {
                     // eof: the client has closed the connection
                     close( (*it)->sd );
                     cons.erase( it );

                     // for some reason, if I delete the only record the loop continues
                     // with an invalid value and core dumps.  So, be conservative.
                     restart = true;
                     break;
                  }
                  else
                  {
                     fprintf(stderr,"received message '%s'\n", inbuf);
                  }
               }
	    }
         } while( restart );
      }
   }
}
}
