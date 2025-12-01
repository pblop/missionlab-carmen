///////////////////////////////////////////////////////////////////////////////
//
//                                 primcomm.cc
//
// This file implements routines used for setting up and monitoring TCP/IP
// socket based connections.  Many of these routines were taken directory from
// TCX
//
// Routines defined for export:
//    listenAtPort, connectAtPort, connectToModule, close_socket, 
//    data_at_socket, hosts_equal
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>

#define SIGPIPE 13    
#define TCP_SOCKET_BUFFER (16*1024)
#define UNIX_SOCKET_BUFFER (32*1024)

#ifdef VXWORKS

#include <sockLib.h>
#include <hostLib.h>
#include <taskLib.h>
#include <ioLib.h>
#include <iosLib.h>
#include <sigLib.h>
#include <in.h>
#include <types.h>
#include <net/socketvar.h>

#else

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#ifndef LINUX
#include <osfcn.h>
#endif
#include <arpa/inet.h>
#include <sys/un.h>

#include "./libc.h"

#endif

#include <ipt/primcomm.h>
#include <ipt/hash.h>

#if !defined(LYNX) && !defined(LINUX)
extern "C" unsigned long inet_addr(const char*);
extern "C" int shutdown(int, int);
#endif

#ifdef LINUX
typedef unsigned int Length_Type;
#else
typedef int Length_Type;
#endif


#define TCP 6      /* from etc/protocols or getprotoent call */
#define BACKLOG 5  /* backlog for listen is pretty much limited to 5 anyway */

/**********************************************************************
 *
 *  FUNCTION:  int listenAtPort(int *port, int *sd)
 *
 *  DESCRIPTION: 
 *
 *		Create a socket for listening for new connections at a given
 * port value. If the value of port is 0, return the value chosen in port.
 *
 * Return 1 on success, else 0.
 *
 *********************************************************************/

int listenAtPort(int& port, int& sd)
{
    int value;
    int reuse, length;
    struct sockaddr_in server;

    /* printf("listenAtPort: %d\n", port);*/

    bzero((char *)&server, sizeof(struct sockaddr_in));

    server.sin_port = htons(port);

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

    if ((sd = socket(AF_INET, SOCK_STREAM, TCP)) < 0)
        return 0;

    reuse = 1;

    if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
                    (char*) &reuse, sizeof(int))) < 0)
        return 0;

    reuse = 1;

#ifndef VXWORKS
    value = TCP_SOCKET_BUFFER;
    if ((setsockopt(sd, TCP, TCP_NODELAY,
		    (char *)&value, sizeof(int))) < 0)
      return 0;
    setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(int));
#endif

    if ((bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in))) < 0)
        return 0;

    if (listen(sd, BACKLOG) < 0)
        return 0;

    /* find what port is if not set */
    if (!port) {
        length = sizeof(server);
        if (getsockname(sd, (struct sockaddr*) &server,
                        (Length_Type *) &length) < 0)
            return 0;

        port = ntohs(server.sin_port);
    }

    return 1;
}

static IPHashTable* NameTable = NULL;

/**********************************************************************
 *
 *  FUNCTION:  int connectAtPort(char *machine, int port, int *sd)
 *
 *  DESCRIPTION: 
 *
 *		Establish a socket connection to the specified machine
 * at a specific port and store the created socket descriptor sd.
 *
 * Return 1 on success, else 0. 
 *
 *********************************************************************/

int connectAtPort(const char *machine, int port, int& sd)
{
    if (!NameTable) 
        NameTable = new IPHashTable(32, str_hash, str_eq);
    
#ifndef VXWORKS
    struct hostent *hp;
#endif

    int value;
    struct sockaddr_in server;

    bzero((char *)&server, sizeof(struct sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    unsigned long stored_addr = (unsigned long) NameTable->Find(machine);
    if (!stored_addr) {
        if (isdigit(machine[0])) {
            server.sin_addr.s_addr = inet_addr((char*) machine);
        } else {
#ifndef VXWORKS
            if ((hp = gethostbyname((char*) machine)) == NULL) {
                return 0;
            }
            bcopy(hp->h_addr, &server.sin_addr.s_addr, hp->h_length);
#else
            if ((server.sin_addr.s_addr = hostGetByName((char*) machine)) == ERROR) {
                return 0;
            }
#endif
        }
        bcopy(&server.sin_addr.s_addr, &stored_addr, sizeof(struct in_addr));
        NameTable->Insert(machine, strlen(machine)+1, (char*) stored_addr);
    } else
        bcopy(&stored_addr, &server.sin_addr.s_addr, sizeof(struct in_addr));

    if ((sd = socket(AF_INET, SOCK_STREAM, TCP)) < 0) {
        /*  fprintf(stderr, "connectAtPort: ERROR: socket\n");*/
        return 0;
    }

    /******* 17-aug-91: fedor: causes NeXT machine to fail connecting **
      if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0)) < 0)
    return 0;
    *****/

#ifndef applec
    { int item = 1;
    if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, 
                    (char *)&item, sizeof(int))) < 0) {
        close(sd);
        return 0;
    }
    }
#endif /* applec */

#ifndef VXWORKS
    value = TCP_SOCKET_BUFFER;
    if ((setsockopt(sd, TCP, TCP_NODELAY, (char*) &value, sizeof(int))) < 0) {
        close(sd);
        return 0;
    }

    setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(int));

#endif

    /* 24-Aug-90: fedor: VxWorks sockaddr_in is defined differently
     this should be checked to make a VxWorks version. */
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        /* fprintf(stderr, "connectAtPort: ERROR: connect\n");*/
        close(sd);
        return 0;
    }

    return 1;
}

/**********************************************************************
 *
 *  FUNCTION:  int connectToModule(char *mod, char *host, int port)
 *
 *  DESCRIPTION: 
 *
 *             Iterate connectAtPort
 *
 * Return the socket descriptor of the new connection.
 *
 *********************************************************************/

int connectToModule(const char *mod, const char *host, int port, int fail)
{
    int sd;

    if (!connectAtPort(host, port, sd)) {
        if (fail)
            return -1;
        fprintf(stderr, "Looking for %s on %s ", mod, host);
        fflush(stderr);

        while (!connectAtPort(host, port, sd)) {
            fprintf(stderr, ".");
            fflush(stderr);
#ifdef VXWORKS
            taskDelay(60*2);
#else
            sleep(2);
#endif
        }
        fprintf(stderr, "found.\n");
    }

    return sd;
}

int accept_socket(int fd)
{
    /* accept the connection */
    struct sockaddr addr;
    int len = sizeof(struct sockaddr);
    int sd = accept(fd, &addr, (Length_Type*) &len);
    
#ifndef VXWORKS
    int value = TCP_SOCKET_BUFFER;
    if ((setsockopt(sd, TCP, TCP_NODELAY, (char*) &value, sizeof(int))) < 0)
        return 0;

    setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(int));
#endif

    return sd;
}

/* close socket with file descriptor "fd" */
void close_socket(int fd)
{
    if (fd == -1)
        return;
    
    shutdown(fd, 2);
    close(fd);
}

/* return the number of bytes available at the socket with file descriptor
   "sd" */
int data_at_socket(int sd)
{
#ifdef _MIPS_SIM_ABI64
    int num_bytes = 0;
#else
    long num_bytes = 0;
#endif

    int res = ioctl(sd, FIONREAD, &num_bytes);
    if (res < 0)
        return res;

    return num_bytes;
}

/* Return the address of the machine with name "machine", 0 for error */
unsigned long host_addr(const char* machine)
{
    long addr = inet_addr((char*) machine);
    if (addr != -1)
        return addr;
#ifndef VXWORKS
    struct hostent *hp;

    if ((hp = gethostbyname((char*) machine)) == NULL) {
        return 0;
    }

    bcopy(hp->h_addr, &addr, sizeof(unsigned long));
#else
    if ((addr = hostGetByName((char*) machine)) == ERROR) {
      return 0;
    }
#endif

    return addr;
}

/* Return 1 if machine "h1" is the same a machine "h2" */
int hosts_equal(const char* h1, const char* h2)
{
    unsigned long addr1 = host_addr(h1);
    unsigned long addr2 = host_addr(h2);

    return (addr1 && addr2 && addr1 == addr2);
}

#ifdef VXWORKS
/* void sigHnd(int sig, int code, SIGCONTEXT *scp) */
void sigHnd(int sig)
#else
void sigHnd(int sig)
#endif
{
  fprintf(stderr, "IPT: sigHnd: Got signal: %d\n", sig);
}

void protect_against_signals()
{
#ifdef VXWORKS
    SIGVEC pVec;

    pVec.sv_handler = sigHnd;
    pVec.sv_mask = 0;
    pVec.sv_flags = 0;
    sigvec(SIGPIPE, &pVec, NULL);
#else
    // Typecast is no longer needed or correct in RedHat 7.1 (endo)
    //signal(SIGPIPE, (SIG_PF) sigHnd);
    signal(SIGPIPE, sigHnd);
#endif
}

int socket_is_closed(int fd)
{
#ifdef VXWORKS

    struct socket* so = (struct socket*) iosFdValue(fd);
    if ((int) so == -1)
        return 1;

    return so->so_state & SS_CANTRCVMORE;
#else
    return fd != -1;
#endif
}
    
#ifndef VXWORKS

int connectToModule(const char *mod, int port, const char* user_id, int fail)
{
    int sd;

    if (!connectAtSocket(port, sd, user_id)) {
        if (fail)
            return -1;
        
        fprintf(stderr, "Looking for %s on this machine ", mod);
        fflush(stderr);

        while (!connectAtSocket(port, sd, user_id)) {
            fprintf(stderr, ".");
            fflush(stderr);
            sleep(2);
        }
        fprintf(stderr, "found.\n");
    }

    return sd;
}

void close_unix_port(int port, const char* user_id)
{
  struct sockaddr_un unix_server;
  
  bzero((char *)&unix_server, sizeof(struct sockaddr_un));
  unix_server.sun_family = AF_UNIX;
  sprintf(unix_server.sun_path,UNIX_SOCKET_NAME,user_id, port);
  
  /* Get rid of old links. */
  unlink(unix_server.sun_path);
  
}

/******************************************************************************
 *
 * FUNCTION: int connectAtSocket(const char *machine, int port, int *sd)
 *
 * DESCRIPTION: Connect at the unix socket.
 *
 * INPUTS: none.
 *
 * OUTPUTS: char *
 *
 *****************************************************************************/

int connectAtSocket(int port, int& sd, const char* user_id)
{
  int value;
  struct sockaddr_un unix_server;
  
  bzero((char *)&unix_server, sizeof(struct sockaddr_un));
  unix_server.sun_family = AF_UNIX;
  sprintf(unix_server.sun_path,UNIX_SOCKET_NAME,user_id,port);
  
  if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    return 0;
  
  /******* 17-aug-91: fedor: causes NeXT machine to fail connecting **
    if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)0, 0)) < 0)
    return 0;
    *****/
  
  /******
    item = 0;
    if ((setsockopt(sd, SOL_SOCKET, SO_LINGER, &item, sizeof(int))) < 0)
    return 0;
    *****/
  
  value = UNIX_SOCKET_BUFFER;
  setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(int));
  setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(int));
  
  /* 24-Aug-90: fedor: VxWorks sockaddr_in is defined differently
   * this should be checked to make a VxWorks version.
   */
  if (connect(sd, (struct sockaddr *)&unix_server, sizeof(unix_server)) < 0) {
    close(sd);
    return 0;
  }
  
  return 1;
}

/**********************************************************************
 *
 *  FUNCTION:  int listenAtSocket(int *port, int *sd)
 *
 *  DESCRIPTION: 
 *
 *		Create a socket for listening for new connections at a given
 * port value. If the value of port is 0, return the value chosen in port.
 *
 * Return 1 on success, else 0.
 *
 * NOTES:
 * 7-Jun-91: fedor: may need some changes for vxworks 
 *
 *********************************************************************/

int listenAtSocket(int port, int& sd, const char* user_id)
{
  int reuse;
  struct sockaddr_un unix_server;
  
  bzero((char *)&unix_server, sizeof(struct sockaddr_un));
  unix_server.sun_family = AF_UNIX;
  sprintf(unix_server.sun_path,UNIX_SOCKET_NAME,user_id,port);
  
  /* Get rid of old links. */
  unlink(unix_server.sun_path);
  
  if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
    return 0;
  }
  
  reuse = 1;
  
#ifndef applec
  if ((setsockopt(sd, SOL_SOCKET, SO_REUSEADDR,
		  (char *)&reuse, sizeof(int))) < 0) {
    return 0;
  }
#endif /* applec */
  
  reuse = 1;
  
#if !defined(VXWORKS) && !defined(applec)
  { int value = UNIX_SOCKET_BUFFER;
    setsockopt(sd, SOL_SOCKET, SO_SNDBUF, (char *)&value, sizeof(int));
    setsockopt(sd, SOL_SOCKET, SO_RCVBUF, (char *)&value, sizeof(int));
  }
#endif /* !VXWORKS && !applec */
  
  if ((bind(sd, (struct sockaddr *)&unix_server,
	    sizeof(unix_server))) < 0)
    {
      return 0;
    }
  
  if (listen(sd, BACKLOG) < 0) {
    return 0;
  }
  
  return 1;
}

#endif

#ifdef SGI

#ifdef IRIX5

#include <limits.h>
extern "C" void usleep(unsigned usec)
{
    sginap(long(double(usec)/1000000.0 * double(CLK_TCK)));
}

#endif

#endif
