/**********************************************************************
 **                                                                  **
 **                     laser_test.h                                 **
 **                                                                  **
 **  Test routines for the files in this directory.                  **
 **                                                                  **
 **                                                                  **
 **  Written by:  Jonathan Diaz  and                                 **
 **               Alexander Stoytchev                                **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: laser_test.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include <fcntl.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <math.h>
#include <stdio.h>

#include "laser_protocol.h"

void writeClient(char *s)
{
  printf("%s", s);
}
int connectMachine(char *host, char *port)
{
  int sock, connectresult;
  struct sockaddr_in sockaddr;
  struct hostent *hp;
  
  writeClient("#Trying to connect..\n");
  if(isdigit(*host))		
    sockaddr.sin_addr.s_addr=inet_addr(host);
  else {  
    if((hp=gethostbyname(host))==NULL) {
      writeClient("#ERROR - UNKNOWN HOST.\n");
      return 0;
    }
    memcpy((char *)&sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
  }
  
  if(isdigit(*port))  
    sockaddr.sin_port=htons(atoi(port)); 
  else { 
    printf("#THE PORT SHOULD BE A NUMBER.\n");
    return 0;
  }

  if((sock=socket(AF_INET, SOCK_STREAM, 0))<0) 
    writeClient("Error in socket\n");
  sockaddr.sin_family=AF_INET;
  

  connectresult=connect(sock, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
  
  if(connectresult) {
    close(sock);
    switch(errno) {
    case ECONNREFUSED:
      writeClient("#ERROR - CONNECTION REFUSED.\n");
      break;
    case ENETUNREACH:
      writeClient("#ERROR - THE NETWORK IS NOT REACHABLE FROM THIS HOST.\n");
      break;
    default:
      writeClient("#Couldn't connect\n");
    } 
    return 0;
  }
  writeClient("#Connected\n");
  return sock;
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

int main(int argc, char **argv)
{
  int fd;
  laser_simple_rec_t command;
  byte_t pack[4096], *data;
  int len, n;
  laser_hallway_reply_t hway;
  laser_doorway_reply_t dway;
  char port[256];

  sprintf(port, "%d", SERV_ROBOT_EXEC_PORT);
  if (!(fd = connectMachine("localhost", port))) exit(-1);

  do {
    command.cmd = LASER_GET_HALLWAY_INFO;
    write(fd, &command, sizeof(laser_simple_rec_t));
    data = pack;
    len = sizeof(laser_hallway_reply_t);
    while (len) {
      n = nextRemainingBytes(fd, data, len);
      len -= n;
      data += n;
    }
    data = pack;
    memcpy(&hway, data, sizeof(laser_hallway_reply_t));
    printf("hway %f %f %f %f %f\n", hway.width, hway.forwardptx,
	   hway.forwardpty, hway.backwardptx, hway.backwardpty);
    printf("angle %f\n", 180/M_PI*atan2(hway.forwardpty, hway.forwardptx));
    command.cmd = LASER_GET_DOORWAY_POS;
    write(fd, &command, sizeof(laser_simple_rec_t));
    data = pack;
    len = sizeof(laser_doorway_reply_t);
    while (len) {
      n = nextRemainingBytes(fd, data, len);
      len -= n;
      data += n;
    }
    data = pack;
    memcpy(&dway, data, sizeof(laser_doorway_reply_t));
    printf("dway %f %f %f %f\n", dway.pt1x, dway.pt1y,
	   dway.pt2x, dway.pt2y);
    usleep(50000);
  } while (1);

  return 1;
}

/**********************************************************************
 * $Log: laser_test.c,v $
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
