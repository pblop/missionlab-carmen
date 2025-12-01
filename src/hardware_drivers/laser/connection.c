/**********************************************************************
 **                                                                  **
 **                     connection.c                                 **
 **                                                                  **
 **   Network routines for exchanging information between hserver    **
 **   and the corridor and doorway detection code.                   **
 **                                                                  **
 **  Written by:  Jonathan Diaz  and                                 **
 **               Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: connection.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

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

int waitForConnection(int sockfd)
{
  int addlen, fd;
  struct sockaddr client;

  addlen = sizeof(struct sockaddr);
  fd = accept(sockfd, &client, &addlen);
  if (fd > 0) return fd;
  return 0;
}

int getHserverConnection(int port)
{
  int sockfd, fd;
  sockfd = initServer(port);
  fprintf(stdout, "server started with socket: %d, waiting on first connection\n", sockfd);
  do {
    fd = waitForConnection(sockfd);
  } while(fd == 0);
  printf("connected\n");
  return fd;
}
typedef unsigned char byte_t;

int nextRemainingBytes(int fd, byte_t *bytes, int max)
{
  int cnum;

  do {
    cnum = read(fd, bytes, max);
  } while (cnum==0 || cnum==-1);
  return cnum;
}

int getNextLaserReading(int fd, rawLaserScan_t *rawScan)
{
  byte_t pack[4096], *data;
  int len, n;

  printf(".");
  fflush(stdout);
  data = pack;
  len = (sizeof(struct timeval)) + (sizeof(float)*3) + (sizeof(unsigned int) * 361);
  while (len) {
    n = nextRemainingBytes(fd, data, len);
    len -= n;
    data += n;
  }
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
  
  printf("%f %f %f\n", rawScan->x, rawScan->y, rawScan->t);
  fflush(stdout);
  return SUCCESS;
}

/**********************************************************************
 * $Log: connection.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:57  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2000/08/12 18:45:31  saho
 * Initial revision
 *
 **********************************************************************/

