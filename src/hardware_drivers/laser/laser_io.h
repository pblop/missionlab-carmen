#ifndef _LASER_IO_H_
#define _LASER_IO_H_

#include "data_structures.h"




#define PLAY_FORWARD               0
#define PLAY_REVERSE               1
#define PLAY_PAUSE                 2
#define PLAY_FROM_START            3
#define PLAY_SINGLE_FORWARD_FRAME  4
#define	PLAY_SINGLE_REVERSE_FRAME  5


#define GLOBAL_PLAY_MODE   0
#define RELATIVE_PLAY_MODE 1


void writeDataToFile(FILE *fp, rawLaserScan_t *rawScan);
void writeFixedDataToFile(FILE *fp, laserScan_t *lscan, double x, double t);
int writeDataToBinaryFile(FILE *fp, rawLaserScan_t *rawScan);
int readSeekRawLaserDataFromBinaryFile(FILE *fp, int offset, int isGlobal, rawLaserScan_t *rawScan);
int readRawLaserDataFromAsciiFile(FILE *fp, rawLaserScan_t *rawScan);

#endif
