#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "data_structures.h"
#include "laser_connection.h"

int giLaserAcceptFd;
extern int giHserverFd;

/* Create a socket to listen on */
int initServer(int port)
{
  struct sockaddr_in serv_addr;
  int sockfd;

  bzero((char *) &serv_addr, sizeof(serv_addr));
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(port);
  
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0)
    {
      fprintf(stderr, "can't open socket for writing\n");
      exit(1);
    }
  if (bind(sockfd, (struct sockaddr *) &serv_addr, 
           sizeof(serv_addr)) < 0)
    {
      fprintf(stderr, "cannot bind local address\n");
      exit(1);
    }
  listen(sockfd, 3);
  return sockfd;
}

/* Accept any waiting connection */
int waitForConnection(int sockfd)
{
  int addlen, fd;
  struct sockaddr client;

  addlen = sizeof(struct sockaddr);
  fd = accept(sockfd, &client, (socklen_t *)(&addlen));
  if (fd > 0) return fd;
  return 0;
}

int getHserverConnection(int port)
{
  int fd;
  giLaserAcceptFd = initServer(port);
  fprintf(stdout, "server started with socket: %d, waiting on first connection\n", giLaserAcceptFd);
  do {
    fd = waitForConnection(giLaserAcceptFd);
  } while(fd == 0);
  printf("connected\n");
  return fd;
}


/* Calls read until succesful. Anywhere between 1 and max bytes are returned or zero
   is returned if end-of-file is detected*/ 
int nextRemainingBytes(int fd, byte_t *bytes, int max)
{
  int cnum;

  do {
    cnum = read(fd, bytes, max);
  } while (cnum == -1);
  return cnum;
}

/* Function gets the next laser reading. If end of file is detected the
   function waits for a laser streamer to reconnect.*/
int getNextLaserReading(int fd, rawLaserScan_t *rawScan)
{
  byte_t pack[4096], *data;
  int len, n;

 TOP:
  data = pack;
  // This is the amount of data to be read. A timestamp, robot's x, y, theta and the laser readings 
  len = (sizeof(struct timeval)) + (sizeof(float)*3) + (sizeof(unsigned int) * 361);
  // Loop until enough data has been read
  while (len) {
    if (!(n = nextRemainingBytes(fd, data, len))) {
      // We lost the laser connection wait for someone to connect
      do {
	giHserverFd = waitForConnection(giLaserAcceptFd);
      } while(giHserverFd == 0);
      fd = giHserverFd;
      // We got a new connection start at the beginnning
      goto TOP;
    }
    // Some bytes were read possibly adjust len and data accordingly
    len -= n;
    data += n;
  }
  // Perform a memcpy to put the data into the rawScan structure
  data = pack;
  memcpy(&(rawScan->tstamp), data, sizeof(struct timeval));
  data += sizeof(struct timeval);
  memcpy(&(rawScan->x), data, sizeof(float));
  data += sizeof(float);
  memcpy(&(rawScan->y), data, sizeof(float));
  data += sizeof(float);
  memcpy(&(rawScan->t), data, sizeof(float));
  data += sizeof(float);
  memcpy(rawScan->dist, data, sizeof(unsigned int) * 361);
  
  return SUCCESS;
}
