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
 * m3dmgAdapter.h
 *
 * definitions for the 3dm-g sensor device
 * The continuous mode functions are not supported by this adapter.
 *--------------------------------------------------------------------------*/

#ifndef M3DMG_ADAPETER_H
#define M3DMG_ADAPETER_H

#include <string.h>
#include <stdlib.h>

using std::string;

#define CMD_RAW_SENSOR          0x01
#define CMD_GYRO_VECTOR         0x02
#define CMD_INSTANT_VECTOR      0x03
#define CMD_INSTANT_QUAT        0x04
#define CMD_GYRO_QUAT           0x05
#define CMD_CAPTURE_GYRO_BIAS   0x06
#define CMD_TEMPERATURE         0x07
#define CMD_SEND_EEPROM         0x08
#define CMD_PROG_EEPROM         0x09
#define CMD_INSTANT_OR_MATRIX   0x0A
#define CMD_GYRO_OR_MATRIX      0x0B
#define CMD_GYRO_QUAT_VECTOR    0x0C
#define CMD_INSTANT_EULER       0x0D
#define CMD_GYRO_EULER          0x0E
#define CMD_SET_CONTINUOUS      0x10
#define CMD_FIRWARE_VERSION     0xF0 
#define CMD_SERIAL_NUMBER       0xF1

#define M3D_INSTANT    1
#define M3D_STABILIZED 2
#define M3D_GYROSCALE_ADDRESS 130
#define M3D_GYROGAINSCALE 64

#define MAX_DEVICENUM 16

/*--------------------------------------------------------------------------
 * Sensor communication function prototypes.
 *--------------------------------------------------------------------------*/

int m3dmg_mapDevice(int, int);
int m3dmg_sendCommand(int , char, char *, int);
void m3dmg_closeDevice(int);
int m3dmg_openPort(int, int, int, int, int);
int m3dmg_openPort(string, int, int, int, int);

/*--------------------------------------------------------------------------
 * Sensor Function prototypes
 *
 * The following abbreviations are used to shorten
 * function names:
 *                Instant = Instantaneous
 *                Orient  = Orientation
 *                Stab    = Stabilized
 *                Quat    = Quaternion
 *--------------------------------------------------------------------------*/

int m3dmg_initGyroScale(int);

int m3dmg_getSerialNumber(int, int *);
int m3dmg_getFirmwareVersion(int, char *);
int m3dmg_getTemperature(int, float *);
int m3dmg_getTimerSeconds(int, float *);

int m3dmg_getRawSensorOutput(int, float[], float[], float[]);
int m3dmg_getVectors(int, float[], float[], float[], int);

int m3dmg_getQuaternions(int, float[], int);
int m3dmg_getOrientMatrix(int, float[][3], int);
int m3dmg_getEulerAngles(int, float *, float *, float *, int);

int m3dmg_captureGyroBias(int);

int m3dmg_setEEPROMValue(int, short, short *);
int m3dmg_getEEPROMValue(int, short, short *);

int m3dmg_getGyroStabQuatVectors(int, float[], float[], float[], float[]);

#endif

/*-------------- end of m3dmgAdapter.h ----------------------*/
