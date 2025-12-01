/////////////////////////////////////////////////////////////////////
//  laser_protocol.c
// 


#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
//#include <linux/un.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>

#include <pthread.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "laser_protocol.h"
#include "data_structures.h"
#include "laser_connection.h"
#define DEBUG  0
#define DEBUG1 0
#define DEBUG2 0
#define DEBUG3 0

#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

int sock_fd;
int Debug = FALSE;

//---------------

extern int giHserverFd;
extern int giLaserAcceptFd;
extern laser_hallway_reply_t gpHwayInfo;
extern pthread_mutex_t gHwayStructMutex;

extern laser_doorway_reply_t gpDwayRightPos;
extern laser_doorway_reply_t gpDwayLeftPos; 
extern laser_doorway_reply_t gpDwayEitherPos;

extern pthread_mutex_t gDwayRightStructMutex;
extern pthread_mutex_t gDwayLeftStructMutex; 
extern pthread_mutex_t gDwayEitherStructMutex;

extern void initAlg(void);

int giRobotExecFd, giLserverPort, giRobotExecAcceptFd;

// Send the current hallway info off on file descriptor sock_fd
void runLaserGetHallwayInfo(int sock_fd)
{
  pthread_mutex_lock(&gHwayStructMutex);
  gpHwayInfo.cmd = LASER_GET_HALLWAY_INFO;
  write(sock_fd, &gpHwayInfo, sizeof(laser_hallway_reply_t));

  pthread_mutex_unlock(&gHwayStructMutex);
}

/* Send the requested doorway info. The request structure has one
   parameter direction which should be one of right, left or either. */
void runLaserGetDoorwayPos(int sock_fd, laser_doorway_request_t *dway)
{
  // Return the doorway on the requested side. 
  switch (dway->direction) {
  case LASER_GET_DOORWAY_RIGHT:
    pthread_mutex_lock(&gDwayRightStructMutex);
    gpDwayRightPos.cmd = LASER_GET_DOORWAY_POS;
    write(sock_fd, &gpDwayRightPos, sizeof(laser_doorway_reply_t));
    pthread_mutex_unlock(&gDwayRightStructMutex);
    break;
  case LASER_GET_DOORWAY_LEFT: 
    pthread_mutex_lock(&gDwayLeftStructMutex);
    gpDwayLeftPos.cmd = LASER_GET_DOORWAY_POS;
    write(sock_fd, &gpDwayLeftPos, sizeof(laser_doorway_reply_t));
    pthread_mutex_unlock(&gDwayLeftStructMutex);
    break;
  case LASER_GET_DOORWAY_EITHER: 
    pthread_mutex_lock(&gDwayEitherStructMutex);
    gpDwayEitherPos.cmd = LASER_GET_DOORWAY_POS;
    write(sock_fd, &gpDwayEitherPos, sizeof(laser_doorway_reply_t));
    pthread_mutex_unlock(&gDwayEitherStructMutex);
    break;
  default:
    printf("Unknown direction in laserGetDoorwayPostion ==> direction = %d\n", dway->direction);
    exit(-1);
  } 
}
void runLaserCloseChannel(int lsock, int fsock)
{
  close(lsock);
  close(fsock);
  exit(-1);    // this kills the LServer  process
}



/**********************************************************************/
/* readCommand() parses the command stream comming from robot_side_com*/
/* over a socket.                                                     */  
/* It interprets the command and runs the appropriate function which  */
/* may or may not write something back on the socket                  */
/**********************************************************************/

int readCommand(int sock_fd)
{
  int len, n;
  static unsigned char rbuf[4096];
  unsigned char *buf;
  laser_simple_rec_t command;
  laser_doorway_request_t dway;

  /* Read the first sizeof(laser_simple_rec_t) bytes which gives the command type.
     If end-of-file is encountered we return failure */
  buf = rbuf;
  len = sizeof(laser_simple_rec_t);
  while (len) {
    if (!(n = nextRemainingBytes(sock_fd, buf, len))) 
      return FAILURE;
    len -= n;
    buf += n;
  }

  memcpy(&command, rbuf, sizeof(laser_simple_rec_t));

  if (DEBUG){
    fprintf(stderr, "Lserver Command Recieved #%x:\n", command.cmd);
  }
  // Run the corresponding function
  switch (command.cmd) {
  case LASER_GET_HALLWAY_INFO:
    runLaserGetHallwayInfo(sock_fd);
    break;   
  case LASER_GET_DOORWAY_POS:
    /* Here we need to read the rest of the laser_doorway_request_t structure.
       If end-of-file is encountered we return failure */
    len = sizeof(laser_doorway_request_t) - sizeof(laser_simple_rec_t);
    while (len) {
      if (!(n = nextRemainingBytes(sock_fd, buf, len)))
	return FAILURE;
      len -= n;
      buf += n;
    }
    buf = rbuf;
    memcpy(&dway, buf, sizeof(laser_doorway_request_t));
    runLaserGetDoorwayPos(sock_fd, &dway);    
    break;   
  case LASER_SYNC_CMD:   
    break;
  case LASER_CLOSE_CHANNEL_CMD:
    break;
  default:
    fprintf(stderr, "LServer: Unknown Command: %x\n",command.cmd);
    break;
  }
  
  return SUCCESS;
}

/* ================================== */

void signal_catcher(int sig){
  switch(sig) {
  case SIGINT:
    runLaserCloseChannel(giLaserAcceptFd, giRobotExecAcceptFd); 
    break;
  }
}
// Entry point for the robot_exec_reader thread
void robot_exec_reader_thread(void){
  /* Read commands from the robot. If the connection is lost we wait
     for a new connection. */
  giRobotExecFd = waitForConnection(giRobotExecAcceptFd);
  while(1){
    if (!readCommand(giRobotExecFd)) 
      giRobotExecFd = waitForConnection(giRobotExecAcceptFd);
  }
}

int startLServer(int hserver_port, int lserver_port)
{

  pthread_t robot_exec_reader;
  
  // Let the laser connect if we are using it
  if (REAL_LASER_READ)
    giHserverFd = getHserverConnection(hserver_port);

  // Initialization
  initAlg();

  giLserverPort = lserver_port;
  
  // Start the server that will wait in another thread for robot_side_com to connect
  giRobotExecAcceptFd = initServer(giLserverPort);
  fprintf(stdout, "server started with socket: %d, waiting on first connection\n", giRobotExecAcceptFd);
    
  pthread_create(&robot_exec_reader,NULL,(void * (*)(void *)) &robot_exec_reader_thread,NULL);
  
  signal(SIGINT,signal_catcher);

  return SUCCESS;
} 


