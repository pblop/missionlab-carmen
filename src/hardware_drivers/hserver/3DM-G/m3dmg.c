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
 * Tel: 802-862-6629 
 * Fax: 802-863-4093
 *--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------
 * m3dmg.c
 *
 * A test application which excercises the functions of the 3DM-G
 * sensor device. This program should be compiled and linked as follows:
 *
 *  Win32 platforms:
 *      cl m3dmg.c m3dmgAdapter.c m3dmgErrors.c m3dmgSerialWin.c -o m3dmg
 *
 *  Linux platform (with gcc) :
 *      gcc -DLINUX_OS m3dmg.c m3dmgAdapter.c m3dmgErrors.c m3dmgSerialLinux.c -o m3dmg
 *--------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string>

#include "m3dmgErrors.h"
#include "m3dmgSerial.h"
#include "m3dmgAdapter.h"
#include "m3dmgUtils.h"

using std::string;

void Compass3DMG_Printf(const char *format, ...)
{
    va_list args;
    char buf[4096];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    fprintf(stderr, buf);
}

int main(int argc, char **argv) {

    int portNum3DMG, deviceNum3DMG;
    int i, j;

/* variables used to retieve and store sensor data */
    int sn;
    float temp, ticks;
    float mag[3];       /*  magetic */
    float accel[3];     /*  acceleration */
    float angRate[3];   /*  angular rate */
    char *axis[] = { "X", "Y", "Z"};
    float quat[4];      /*  quaternions */
    float xform[3][3];  /*  transformation matrix */
    float roll, pitch, yaw;
    short address, evalue;  /* EEPROM address and data value */

    //float raw[30];

    char *fw;
    int errorCode;
    int tryPortNum = 3; // That is /dev/ttyR2;

    string portString, portNumString;

    /* open a port, map a device */
	printf("3DM-G C-Serial port test\n");

    if (argc > 1) {

        //tryPortNum = atoi(argv[1]);
        portString = argv[1];
        portNumString = portString[portString.size()-1];
        tryPortNum = atoi(portNumString.c_str()) + 1;

        if (tryPortNum < 1 || tryPortNum > 9) {
            printf("usage:  m3dmg <portNumber>\n");
            printf("        valid ports are 1..9, default is 1\n");
            exit(1);
        }

        portNum3DMG = m3dmg_openPort(portString, 38400, 8, 0, 1);
    }
    else
    {
        portNum3DMG = m3dmg_openPort(tryPortNum, 38400, 8, 0, 1);
    }

    if (portNum3DMG<0) {
        printf("port open failed.\n");
        printf("Comm error %d, %s.\n", portNum3DMG, explainError(portNum3DMG));
        return -1;
    }
    printf("Port is open as #%d - OK.\n", portNum3DMG);

    // map device - this is required!
    deviceNum3DMG = m3dmg_mapDevice(1, portNum3DMG);
    if (deviceNum3DMG <= 0) {
        printf("Could not map the Device to a Port\n");
        printf("%s\n", explainError(deviceNum3DMG));
        return -1;
    }
    printf("Device Number is #: %d\n\n", deviceNum3DMG);

    // get serial number
    errorCode = m3dmg_getSerialNumber(deviceNum3DMG, &sn);
    if (errorCode != M3D_OK)
        printf("S/N Error - : %s\n", explainError(errorCode));
    else
        printf("Serial #: %d\n", sn);
    printf("\n");

    // get firmware number (as a string)
    fw = (char *) malloc( (size_t)12 );
    errorCode = m3dmg_getFirmwareVersion(deviceNum3DMG, fw);
    printf("Firmware ");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else
        printf("#: %s\n", fw);
    printf("\n");

    // temperature
    errorCode = m3dmg_getTemperature(deviceNum3DMG, &temp);
    printf("Temperature ");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else
        printf("%3.2f degrees C.\n", temp);
    printf("\n");

    // timer ticks in seconds
    errorCode = m3dmg_getTimerSeconds(deviceNum3DMG, &ticks);
    printf("Timer seconds since last rollover\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else
        printf(" -> %f\n", ticks);
    printf("\n");

    // raw sensor data
    errorCode = m3dmg_getRawSensorOutput(deviceNum3DMG, mag, accel, angRate);
    printf("Raw sensor data\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("\tMag\t\tAccel\t\tAng Rate\n");
        for (i=0; i<3; i++) {
            printf("  %s\t: %f\t%f\t%f\n", axis[i], mag[i], accel[i], angRate[i]);
        }
    }
    printf("\n");

    // instantaneous vector
    errorCode = m3dmg_getVectors(deviceNum3DMG, mag, accel, angRate, M3D_INSTANT);
    printf("Instantaneous Vectors\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("\tMag\t\tAccel\t\tAng Rate\n");
        for (i=0; i<3; i++) {
            printf("  %s\t: %f\t%f\t%f\n", axis[i], mag[i], accel[i], angRate[i]);
        }
    }
    printf("\n");

    // gyro-stabilized vector
    errorCode = m3dmg_getVectors(deviceNum3DMG, mag, accel, angRate, M3D_STABILIZED);
    printf("Stabilized Vectors\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("\tMag\t\tAccel\t\tAng Rate\n");
        for (i=0; i<3; i++) {
            printf("  %s\t: %f\t%f\t%f\n", axis[i], mag[i], accel[i], angRate[i]);
        }
    }
    printf("\n");

    // instantaneous quaternion
    errorCode = m3dmg_getQuaternions(deviceNum3DMG, quat, M3D_INSTANT);
    printf("Instantaneous Quaternions\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        for (i=0; i<4; i++) {
            printf("  %d : %6.4f\n", i, quat[i]);
        }
    }
    printf("\n");

    // gyro-stabilized quaternion
    errorCode = m3dmg_getQuaternions(deviceNum3DMG, quat, M3D_STABILIZED);
    printf("Stabilized Quaternions\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        for (i=0; i<4; i++) {
            printf("  %d : %6.4f\n", i, quat[i]);
        }
    }
    printf("\n");

    // instantaneous orientation matrix
    errorCode = m3dmg_getOrientMatrix(deviceNum3DMG, &xform[0], M3D_INSTANT);
    printf("Instantaneous orientation matrix\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("-M-\t1\t\t2\t\t3\n");
        for (i=0; i<3; i++) {
            printf("  %d :", i+1);
            for (j=0;j<3;j++) {
                printf("\t%f", xform[i][j]);
            }
            printf("\n");
        }
    }
    printf("\n");

    // Euler angles - instantaneous
    errorCode = m3dmg_getEulerAngles(deviceNum3DMG, &pitch, &roll, &yaw, M3D_INSTANT);
    printf("Instantaneous Euler angles\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("Pitch : %4.2f\n", pitch);
        printf("Roll  : %4.2f\n", roll);
        printf("Yaw   : %4.2f\n", yaw);
    }
    printf("\n");

    // Euler angles - gyro-stabilized
    errorCode = m3dmg_getEulerAngles(deviceNum3DMG, &pitch, &roll, &yaw, M3D_STABILIZED);
    printf("Stabilized Euler angles\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("Pitch : %4.2f\n", pitch);
        printf("Roll  : %4.2f\n", roll);
        printf("Yaw   : %4.2f\n", yaw);
    }
    printf("\n");


    // quaternions and vectors together (gyro-stabilized)
    errorCode = m3dmg_getGyroStabQuatVectors(deviceNum3DMG, 
                                             quat, 
                                             mag, accel, angRate);
    printf("Stabilized Quaternions & Vectors\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        for (i=0; i<4; i++) {
            printf("  %d : %6.4f\n", i, quat[i]);
        }
        printf("\tMag\t\tAccel\t\tAng Rate\n");
        for (i=0; i<3; i++) {
            printf("  %s\t: %f\t%f\t%f\n", axis[i], mag[i], accel[i], angRate[i]);
        }
    }
    printf("\n");

    // gyro-stabilized orientation matrix
    errorCode = m3dmg_getOrientMatrix(deviceNum3DMG, &xform[0], M3D_STABILIZED);
    printf("Stabilized orientation matrix\n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("-M-\t1\t\t2\t\t3\n");
        for (i=0; i<3; i++) {
            printf("  %d :", i+1);
            for (j=0;j<3;j++) {
                printf("\t%f", xform[i][j]);
            }
            printf("\n");
        }
    }
    printf("\n");

    // read EEPROM value
    address = 134;  // address of the serial number 
    errorCode = m3dmg_getEEPROMValue(deviceNum3DMG, address, &evalue);
    printf("EEPROM read \n");
    if (errorCode < 0)
        printf("Error - : %s\n", explainError(errorCode));
    else {
        printf("value at addr. %d : %u \n", address, evalue);
    }

    // Keep displaying the angles.

    printf("\n===========================================================\n\n");

    printf("Stabilized Euler angles\n");
    while(true)
    {
        // Euler angles - gyro-stabilized
        errorCode = m3dmg_getEulerAngles(deviceNum3DMG, &pitch, &roll, &yaw, M3D_STABILIZED);
        if (errorCode < 0)
            printf("Error - : %s\n", explainError(errorCode));
        else {
            printf("Pitch/Roll/Yaw : <%4.2f, %4.2f, %4.2f>\n", pitch, roll, yaw);
        }
        usleep(250000);
    }

    // free allocated memory
    free(fw);

    // close device
    m3dmg_closeDevice(deviceNum3DMG);
	return(0);
}

/*-------------- end of m3dmg.c ----------------------*/
