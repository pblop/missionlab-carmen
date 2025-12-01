
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "mic.h"
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
vector<conInfo *> cons;


int main(int, char **)
{
   int our_port = 45129;
   int server_port = 45123;

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

   // fill in socket descripter record for server
   struct sockaddr_in server_socket;
   bzero((caddr_t)&server_socket, sizeof(server_socket));
   server_socket.sin_family = our_host->h_addrtype;
   bcopy(our_host->h_addr, (char *)&server_socket.sin_addr, our_host->h_length);
   server_socket.sin_port = server_port;

   // Attach to the server 
   if( connect(s, (struct sockaddr *)&server_socket, sizeof(server_socket)) < 0 )
   {
      perror("Can not connect");
      exit(4);
   }

   int msg = 0;
   while(1)
   {
      char buf[256];
      sprintf(buf,"message %d\n", msg++);
      int rtn = write(s, buf, strlen(buf)+1);
      if( rtn < 0 )
      {
         perror("Error writing data");
         exit(4);
      }
      sleep(1);
   }
}
}
