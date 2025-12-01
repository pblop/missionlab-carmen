///////////////////////////////////////////////////////////////////////////////
//
//                                 libc.h
//
// Declares prototypes for lame compilers like g++ that don't build prototypes
// right on sun work stations.  Also declares some stuff to make compilation 
// go well on SGI machines
//
//  "1995, Carnegie Mellon University. All Rights Reserved." This
//  software is made available for academic and research purposes only. No
//  commercial license is hereby granted.  Copying and other reproduction is
//  authorized only for research, education, and other non-commercial
//  purposes.  No warranties, either expressed or implied, are made
//  regarding the operation, use, or results of the software.
//
///////////////////////////////////////////////////////////////////////////////

#ifndef VXWORKS

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef P_

#if defined(__STDC__) || defined(__cplusplus)
# define P_(s) s
#else
# define P_(s) ()
#endif

#endif

#ifdef WITH_PTHREADS
#include <pthread.h>
#endif

#if defined(LINUX) || defined(LYNX)
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <signal.h>
typedef void (*SIG_PF)();
#endif

#ifdef SGI
#include <unistd.h>
#include <strings.h>
#include <signal.h>
#include <sys/ioctl.h>
#endif

#ifdef IRIX5
#include <osfcn.h>
#include <sys/uio.h>
void usleep(unsigned);
#endif

#ifdef SUN4
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <signal.h>

int gethostname(char*, int);
#endif

#ifdef SOLARIS
#define bcopy(a,b,n) 	memcpy((b),(a),(n))
#define bzero(a,n) 	memset((a),0,(n))
#define bcmp(a,b,n)	memcmp((a),(b),(n))

int usleep(unsigned int useconds);

#include <unistd.h>
#include <sys/filio.h>

#endif

#if defined(FORCE_SIG_PF)
typedef void    SIG_FUNC_TYP (int);
typedef SIG_FUNC_TYP*   SIG_PF;
#endif

#if defined(NEED_PROTOTYPES)

void bcopy P_((const void *, void *, int));
int bcmp P_((const void *, const void *, int));
void bzero P_((void *, int));
struct sockaddr;
int accept P_((int, struct sockaddr *, int *) );
int bind P_((int, struct sockaddr *, int) );
int connect P_((int, const struct sockaddr *, int) );
int getpeername P_((int, struct sockaddr *, int *) );
int getsockname P_((int, struct sockaddr *, int *) );
int getsockopt P_((int, int, int, char *, int *) );
int listen P_((int, int) );
int recv P_((int, char *, int, int) );
int recvfrom P_((int, char *, int, int, struct sockaddr *, int *) );
int send P_((int, const char *, int, int) );
int sendto P_((int, const char *, int, int, const struct sockaddr *, int) );
int setsockopt P_((int, int, int, const char *, int) );
int socket P_((int, int, int) );
int recvmsg P_((int, struct msghdr *, int) );
int sendmsg P_((int, struct msghdr *, int) );
int socketpair P_((int, int, int, int *) );

struct fd_set;
struct timeval;
struct timezone;
int select P_((int, fd_set*, fd_set*, fd_set*, const struct timeval*) );
int gettimeofday P_((timeval*, struct timezone*) );

unsigned sleep P_((unsigned) );
int      ioctl P_((int, int, ...) );
int  usleep P_((unsigned));
int writev P_((int, struct iovec*, int));

#if !defined(FORCE_SIG_PF)
typedef void    SIG_FUNC_TYP (int);
typedef SIG_FUNC_TYP*   SIG_PF;
#endif

#endif 

#if defined(__cplusplus)
}
#endif

#endif
