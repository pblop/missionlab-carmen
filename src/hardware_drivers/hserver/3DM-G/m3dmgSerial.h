/*--------------------------------------------------------------------------
 *
 * 3DM-G Interface Software
 *
 * (c) 2003 Microstrain, Inc.
 * All rights reserved.
 *
 * www.microstrain.com
 * 310 Hurricane Lane, Suite 4
 * Williston, VT 05495 USA
 * Tel: 802-862-6629 tel
 * Fax: 802-863-4093 fax
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * m3dmgSerial.h
 *
 * definitions and prototype declarations for the serial port functions.
 *--------------------------------------------------------------------------*/

#ifndef M3DMGAD_SERIAL_H
#define M3DMGAD_SERIAL_H

#include <string>

using std::string;
#define MAX_PORT_NUM 9

#define M3D_COMM_NOPARITY 0
#define M3D_COMM_ODDPARITY 1
#define M3D_COMM_EVENPARITY 2

#define M3D_COMM_ONESTOPBIT 1
#define M3D_COMM_TWOSTOPBITS 2

/*--------------------------------------------------------------------------
 * function prototypes
 *--------------------------------------------------------------------------*/

int openPort(int);
void closePort(int);
int setCommParameters(int, int, int, int, int);
int setCommTimeouts(int, int, int);
int sendData(int, char*, int);
int receiveData(int, char*, int);

extern string gPortBase;

#endif

/*-------------- end of m3dmgSerial.h ----------------------*/
