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
 * m3dmgSerialLinux.c
 *
 * Serial port interface for the 3DM-G Sensor device. 
 *
 * This module is specific to the Linux platform and 
 * and has been tested on Linux Redhat 7.3 running on Dell 2300 hardware.
 * For a Windows version (95,98,NT,2000,XP), please use m3dmgSerialWin.c
 *
 * Note: This code is POSIX compliant, but we have found timing problems
 * running this on the Solaris platform with the sensor, which does not
 * implement hardware flow control.
 *--------------------------------------------------------------------------*/
#ifndef DEBUG
#define DEBUG 0
#endif

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <string.h>

#include "m3dmgSerial.h"
#include "m3dmgErrors.h"

extern void Compass3DMG_Printf(const char *format, ...);

/*  port Handles in use */
int portHandles[MAX_PORT_NUM];
char portName[20];

/*--------------------------------------------------------------------------
 * openPort
 *
 * parameters:  portNum3DMG  : a port number from 1 to MAX_PORT_NUM
 *
 * returns:     a port handle or MM3D_COMM_FAILED.
 *--------------------------------------------------------------------------*/

int openPort(int portNum3DMG) {

/* Depending on your OS, the portBase and portList will vary. 
*  Some common schemes are:
 *   Linux:         /dev/ttyS0, /dev/ttyS1
 *   IRIS:          /dev/ttyf1, /dev/ttyf2
 *   HP-UX:         /dev/tty1p0, /dev/tty2p0 
 *   Digital UNIX:  /dev/tty01, /dev/tty02 
 */

#ifdef LINUX_OS
    char *portList[] = { "0", "1", "2", "3", "4", "5", "6", "7", "8"};
    char *portBase = "/dev/ttyR";
    //char *portBase = "/dev/ttyS";
#endif

#ifdef SOLARIS_OS
    char *portList[] = { "a", "b", "c", "e", "f", "g", "h", "i", "j"};
    char *portBase = "/dev/tty";
#endif

#ifdef HPUX_OS
    char *portList[] = { "1p0", "2p0", "3p0", "4p0", "5p0", "6p0", "7p0", "8p0", "9p0"};
    char *portBase = "/dev/tty";
#endif

    int portHandle;
	//    char *portName;
	//    portName = (char *) malloc((size_t) 20);

    if (portNum3DMG<0 || portNum3DMG > MAX_PORT_NUM) {
        return M3D_COMM_INVALID_PORTNUM;
    }

    if (gPortBase == "")
    {
        gPortBase = portBase;
    }


    //strcat(portName, portBase);
    strcat(portName, gPortBase.c_str());
    strcat(portName, portList[portNum3DMG-1]);

    if (DEBUG)
    {
      Compass3DMG_Printf("Opening the port: %s", portName);
    }

    portHandle = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
    // portHandle = open(portName, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);

    if (portHandle <0 ) {
        Compass3DMG_Printf("Failed to open %s\n", portName);
        return M3D_COMM_FAILED;
    }

    if (DEBUG)
    {
      Compass3DMG_Printf("Port opened. FD = %d,\n", portHandle);
    }

    portHandles[portNum3DMG-1] = portHandle;
    return portNum3DMG;
}

/*--------------------------------------------------------------------------
 * closePort
 *
 * parameters:  portNum3DMG :  closes the port corresponding to this port number.
 *--------------------------------------------------------------------------*/

void closePort(int portNum3DMG) {
    if (portNum3DMG>0 && portNum3DMG<=MAX_PORT_NUM) {
        close(portHandles[portNum3DMG-1]);
    }
}

/*--------------------------------------------------------------------------
 * setCommParameters
 *
 * parameters:  portNum3DMG  : a serial port number
 *              baudrate : the communication flow rate (speed).
 *              charsize : the character size (7,8 bits)
 *              parity   : 0=none, 1=odd, 2=even
 *              stopbits : the number of stop bits (1 or 2)
 *
 * returns:     M3D_COMM_OK if the settings succeeded.
 *              M3D_COMM_FAILED if there was an error.
 *--------------------------------------------------------------------------*/

int setCommParameters(int portNum3DMG,
                      int baudrate,
                      int dataBits,
                      int parity,
                      int stopBits ) {

    struct termios portOptions;
    int portHandle;
    int status;
    int BAUD, DATABITS, PARITY, PARITYON, STOPBITS;

    portHandle = portHandles[portNum3DMG-1];
    /*  get port options for speed, etc. */
    tcgetattr(portHandle, &portOptions);
    tcflush(portHandle, TCIFLUSH);

    /* baudrate settings */
    switch (baudrate) {
    case 38400:
        BAUD = B38400;
        break;
    case 19200:
        BAUD  = B19200;
        break;
    case 9600:
        BAUD  = B9600;
        break;
    default:
        BAUD = B38400;
    }

    /* databits settings */
    switch (dataBits) {
    case 8:
        DATABITS = CS8;
        break;
    case 7:
        DATABITS = CS7;
        break;
    default:
        DATABITS = CS8;
    }

    /* stop bits */
    switch (stopBits) {
    case 1:
        STOPBITS = 0;
        break;
    case 2:
        STOPBITS = CSTOPB;
        break;
    default:
        STOPBITS = 0;

    }

    /* parity */
    switch (parity) {
    case 0: /* no parity */
        PARITYON = 0;
        PARITY = 0;
        break;
    case 1: /* odd parity */
        PARITYON = PARENB;
        PARITY = PARODD;
        break;
    case 2: /* event parity */
        PARITYON = PARENB;
        PARITY = 0;
        break;
    default: /* none */
        PARITYON = 0;
        PARITY = 0;
    }

    /* We are not setting CRTSCTS which turns on hardware flow control */
    portOptions.c_cflag = BAUD | DATABITS | STOPBITS | PARITYON | PARITY | CLOCAL | CREAD;

    /*  set for non-canonical (raw processing, no echo, etc.) */
    portOptions.c_iflag = IGNPAR; /* ignore parity check */
    portOptions.c_oflag = 0; /* raw output */
    portOptions.c_lflag = 0; /* raw input  */

    cfsetospeed(&portOptions, BAUD);  /* redundant with the cflag setting, above */
    cfsetispeed(&portOptions, BAUD);
    status = tcsetattr(portHandle, TCSANOW, &portOptions);

    if (status != 0)
        return M3D_COMM_FAILED;
    else
        return M3D_COMM_OK;
}

/*--------------------------------------------------------------------------
 * setCommTimeouts
 *
 * parameters:  readTimeOut - timeout for single char and total read.
 *              writeTimeOut - timeout for single char and total write.
 *
 * Does nothing since we are not using blocking IO.
 *--------------------------------------------------------------------------*/

int setCommTimeouts( int portNum3DMG, 
                     int readTimeout, 
                     int writeTimeout) {

    return M3D_COMM_OK;
}

/*--------------------------------------------------------------------------
 * sendData
 *
 * parameters:  portNum3DMG  - a serial port number (1..n)
 *              command  - a single byte command
 *              commandLength - the length of the command buffer
 *                              which is the number of bytes to send
 *
 * returns:     COMM_OK if write/read succeeded
 *              COMM_WRITE_ERROR if there was an error writing to the port
 *--------------------------------------------------------------------------*/

int sendData(int portNum3DMG, char* command, int commandLength)
{
    //int bytesWritten;
    int status;
    int portHandle;

    portHandle = portHandles[portNum3DMG-1];

    /* write command to the serial port */
    status = write(portHandle, &command[0], commandLength);
    if (status >=0) {
        return M3D_COMM_OK;
    } else {
        return M3D_COMM_WRITE_ERROR; 
    }
}

/*--------------------------------------------------------------------------
 * receiveData
 *
 * parameters:  portNum3DMG  - a serial port number
 *              response - a pointer to a character buffer to hold the response
 *              responseLength - the # of bytes of the expected response.
 *
 * returns:     COMM_OK if write/read succeeded
 *              COMM_READ_ERROR if there was an reading from the port
 *              COMM_RLDLEN_ERROR if the length of the response did not match
 *                              the number of returned bytes.
 *--------------------------------------------------------------------------*/

int receiveData(int portNum3DMG, char *response, int responseLength) {

    int n, bytesRead, attempts;
    char inchar;
    //int status;
    int portHandle;
    int portCount;
    int maxPorts;
    struct timeval timeout;
    fd_set readfs;    /* file descriptor set */
    //struct termios portOptions;

    int TIMEOUT = 50000;      /* time to wait for port to respond, in microseconds */
    int MAXATTEMPTS = 200;    /* maximum number of attempts to read characters */
    int WAITCHARTIME = 1000;  /* time to wait for a char to arrive. */

    portHandle = portHandles[portNum3DMG-1];

    /* select will wait for port to respond or timeout */
    maxPorts = portHandle+1;
    timeout.tv_usec = TIMEOUT;  /* microseconds */
    timeout.tv_sec  = 0;        /* seconds */
    FD_ZERO(&readfs);
    FD_SET(portHandle, &readfs);  /* set testing for portHandle */
    if (DEBUG) Compass3DMG_Printf("waiting for port to respond\n");
    portCount = select(maxPorts, &readfs, NULL, NULL, &timeout);  /* block until input becomes available */
    if ((portCount==0) || (!FD_ISSET(portHandle, &readfs))) {
        if (DEBUG) Compass3DMG_Printf(" - timeout expired!\n");
        return M3D_COMM_RDTIMEOUT_ERROR;
    }
    if (DEBUG) Compass3DMG_Printf("time remaining %d ms.\n", timeout.tv_usec/1000);

    /* Read data into the response buffer.
     * until we get enough data or exceed the maximum
     * number of attempts
     */
    bytesRead = 0;
    attempts = 0;
    while (bytesRead < responseLength && attempts++ < MAXATTEMPTS) {
        n = read(portHandle, &inchar, 1);
        //if (DEBUG) Compass3DMG_Printf(".", n, inchar);
        if (n == 1)
            response[bytesRead++] = inchar;
        else
            usleep(WAITCHARTIME);  /* sleep a while for next byte. */
    }
    if (DEBUG) Compass3DMG_Printf("\nattempts %d", attempts);
    if (DEBUG) Compass3DMG_Printf("\nreceiveData: bytes read: %d   expected: %d\n", bytesRead, responseLength);

    if (bytesRead != responseLength) {
        return M3D_COMM_RDLEN_ERROR;
    }
    else
        return M3D_COMM_OK;
}

/*-------------- end of m3dmgSerialLinux.c ----------------------*/
