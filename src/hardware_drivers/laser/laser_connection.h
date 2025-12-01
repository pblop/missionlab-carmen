#ifndef LASER_CONNECTION_HEADER
#define LASER_CONNECTION_HEADER

typedef unsigned char byte_t;

int initServer(int port);
int waitForConnection(int sockfd);
int getHserverConnection(int port);
int nextRemainingBytes(int fd, byte_t *bytes, int max);
int getNextLaserReading(int fd, rawLaserScan_t *rawScan);

#endif
