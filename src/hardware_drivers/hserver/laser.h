/**********************************************************************
 **                                                                  **
 **                                laser.h                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  SICK LMS 200 driver for HServer                                 **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: laser.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef LASER_H
#define LASER_H

#include <vector>
#include <string>
#include "module.h"
#include "sensors.h"
#include "robot_config.h"
#include "hserver_ipt.h"
#include "ipc_client.h"

using std::vector;
using std::string;

#define SICK_STX           0x02
#define SICK_ACK           0x06
#define SICK_INIT          0x10
#define SICK_NACK_         0x15
#define SICK_BM            0x20
#define SICK_MWANF         0x30
#define SICK_SSANF         0x31
#define SICK_ERRANF        0x32
#define SICK_MMWANF        0x36
#define SICK_MWPANF        0x37
#define SICK_LMIKFGANF     0x38
#define SICK_EVCANF        0x39
#define SICK_TYPANF        0x3A
#define SICK_VARDEF        0x3B
#define SICK_MMWPANF       0x3F
#define SICK_FIELDKFG      0x40
#define SICK_FIELDCHG      0x41
#define SICK_SFANF         0x45
#define SICK_LMOD          0x46
#define SICK_DYNRECKFG     0x48
#define SICK_DYNSEGKFG     0x4B
#define SICK_SIMINDEX      0x4C
#define SICK_OUTPUTSET     0x4D
#define SICK_INPUTRD       0x4E
#define SICK_SIMINPUT      0x4F
#define SICK_BRPERMDEF     0x66
#define SICK_LASER         0x68
#define SICK_LMIKFGDEF     0x70
#define SICK_EVCDEF        0x72
#define SICK_LMSKFGANF     0x74
#define SICK_MWXYANF       0x76
#define SICK_LMSKFGDEF     0x77
#define SICK_CORRDEF       0x78
#define SICK_PWON          0x90
#define SICK_INIT_ACK      0x91
#define SICK_NACK          0x92
#define SICK_BMACK         0xA0
#define SICK_MW            0xB0
#define SICK_SS            0xB1
#define SICK_ERR           0xB2
#define SICK_MMW           0xB6
#define SICK_MWP           0xB7
#define SICK_LMIKFG        0xB8
#define SICK_EVC           0xB9
#define SICK_TYPE          0xBA
#define SICK_VARDEFACK     0xBB
#define SICK_MMWP          0xBF
#define SICK_FIELDACK      0xC0
#define SICK_FILEDCHGACK   0xC1
#define SICK_FIELDDAT      0xC5
#define SICK_LMODACK       0xC6
#define SICK_DYNRECACK     0xC8
#define SICK_DYNSEGACK     0xCB
#define SICK_SIMINDEXACK   0xCC
#define SICK_OUTPUTSETACK  0xCD
#define SICK_INPUTDATA     0xCE
#define SICK_SIMINPUTACK   0xCF
#define SICK_BRPERMACK     0xE6
#define SICK_LASERACK      0xE8
#define SICK_LMIKFGACK     0xF0
#define SICK_EVCACK        0xF2
#define SICK_LMSKFG        0xF4
#define SICK_MWXY          0xF6
#define SICK_LMSKFGAC      0xF7

#define STGM_STX    0x00
#define STGM_ADR    0x01
#define STGM_LENL   0x02
#define STGM_LENH   0x03
#define STGM_CMD    0x04
#define STGM_DATA   0x05

#define SICK_MWANF_SIZE       7
#define SICK_INIT_SIZE        5
#define SICK_BM_SIZE          6
#define SICK_LMSKFGDEF_SIZE   37

#define SMOD_INSTALLATION    0x00
#define SMOD_MON_CONTINUAL   0x24
#define SMOD_MON_REQUEST     0x25
#define SMOD_BAUD_9600       0x42
#define SMOD_BAUD_19200      0x41
#define SMOD_BAUD_38400      0x40
#define SMOD_BAUD_500K       0x48

#define SS_SW_VERSION   0
#define SS_MODE         7
#define SS_STATUS       8
#define SS_PROD_CODE    9
#define SS_VARIANT      17
#define SS_SCAN_ANGLE   106
#define SS_SSHOT_RES    108
#define SS_BAUD         115
#define SS_PERM_BAUD    118
#define SS_ACTIVE_FIELD 120
#define SS_UNIT         121
#define SS_LASER        122

#define SS_SW_VERSION_LEN  7
#define SS_PROD_CODE_LEN   8

#define SDAT_NUM_READINGS 181

#define SICK_FOV 180.0
#define SICK_RANGE 80.0

#define LASER_STREAMING_PORT 31400

typedef struct {
    int cmd;
} sick_config_t;

typedef unsigned char byte_t;
typedef unsigned int uint;


typedef struct {
    int len;
    byte_t *d;
} telegram_t;

class Laser : public Module
{
protected:
    static pthread_mutex_t m_mutexRanges;
    static pthread_mutex_t m_mutexAngles;
    static pthread_mutex_t m_mutexRawRanges;
    static int m_iStreamFd;
    static bool m_bStreaming;
    static Sensor* m_pclSensorLaser;
    static RobotPartLaser* m_pclPartLaser;
    static int m_bFirsttime;

    bool m_bConnected, m_bListenIpt;
    int m_iMaxRetries;
    bool m_bReaderRunning;
    pthread_t m_threadReader;
    pthread_t m_CarmenReaderThread;
    pthread_cond_t m_condReader;
    pthread_mutex_t m_mutexCond;
    pthread_mutex_t m_mutexSerial;
    int m_iSerialPortFd;
    int m_iMinReading;
    int m_iMinIndex;
    struct timeval m_oldtime;
    float m_fUpdateSpeed;
    bool m_bStreamIpt, m_bReceiveIpt, m_bReceiveCarmen;
    bool m_bContinual;
    string m_strPortStr;
    int m_iCurrentBuffer;
    int m_iNumReadings;
    int m_iNumBuffers;
    int m_iTelegramBufSize;
    byte_t* m_pbyteTelegramBuf;
    unsigned int* m_puRawRanges;
    float* m_pfSickRanges;
    float* m_pfCurrentSickRanges;
    float* m_pfAngles;    // angles after x and y offsets have been taken into account
    float* m_pfRawAngles; // angles in relation to the laser + the laser angle offset
    extra_laser_t m_suExtraInfo;
    IPHandlerCallback* m_pclIptCallback;
    IPCommunicator* m_pclCommunicator;
    IPConnection* m_pclLaserConnection;

    float m_fDegreeOffset;
    float m_fXOffset;
    float m_fYOffset;

    string m_strName; // this will distinguish different lasers

    // These will point to arrays that hold all information for all existing SICKs.
    // The number of elements should always be equal to m_iTotalReadings.
    static float* m_pfAllReadings;
    static float* m_pfAllAngles;
    static unsigned int* m_puAllRawRanges;
    static int m_iTotalReadings;
    static int m_iOverallMinReading;
    static float m_fAngleOfOverallMinReading;
    static string m_strStreamAddress;

    static const int MAX_TELEGRAM_SIZE;
    static const int WATCHDOG_CHECK_INTERVAL_SEC_;

    static void RecalculateMinReading(void);
    static bool ValidName(const string& strName);
    static float NormalizeAngle(float fAngle);

    float IndexToDegrees(int iIndex);
    void AddToOverallBuffer(void);
    void RemoveFromOverallBuffer(void);

    void AccountForXYOffset(float& fRange, float& fReadingAngle);
    void callbackLaserScan(IPMessage* message);
    void (*callback)(float*, int);
    int flushSerial(int fd);
    unsigned int createCRC(unsigned char* CommData, uint uLen);
    static int connectISocket(const char* addrstr, int port);
    string commandName(byte_t command); // jbl
    int initializeTelegram(telegram_t* tgm, uint command, uint len);
    int sendCommand(int command, uint arg = 0);
    void printTelegram(byte_t* buf, int len);
    void sendLaserStream(void);
    void sendLaserStreamIpt(void);
    void processMeasuredValues(byte_t* buf);
    void printPwonString(const char* str, int len);
    void printStatus(uint status);
    void printBmack(byte_t code);
    void printSensorStatus(byte_t* buf, uint len);
    int processTelegram(byte_t* buf, int len);
    void readTelegram(void);
    int changeBaud(uint baud);
    void sickClose(void);
    void sickContinual(void);
    void sickOneReading(void);
    int sickInitialize(void);
    void addSensor(void);
    bool readN(unsigned char* buf, int n);
    void convertAddress(char* address);
    int setupIpt(void);
    void SetXYOffsets(float fXOffset, float fYOffset);
    void shutdownIpt(void);
    void UpdateLaserPart(void);
    void updateStatusBar_(void);
    void logData_(double logTime);

public:
    void callbackCARMENLaserScan(carmen_laser_laser_message* rec);

    enum EnConnectType
    {
        EnConnectType_DIRECT                = 1,
        EnConnectType_DIRECT_AND_STREAM_IPT = 2,
        EnConnectType_LISTEN_IPT            = 3,
        EnConnectType_CARMEN				= 4
    };

    static const string m_strDEFAULT_NAME;

    void connectRemote(void);
    static void streamStart(void);
    void control(void);
    void reader_thread(void);
    void carmenLaserReaderThread(void);
    void waitNext(void);
    void setCallback(void (*callback)(float*, int));
    float* latest(void);
    Laser(
        EnConnectType enConnectType,
        const string& strPort,
        const string& strName,
        float fDegreeOffset,
        float fXOffset,
        float fYOffset);
    ~Laser(void);

    static void ConnectLaser(void);
    static void OverallMinReading(int& iReading, float& fAngle);
    static Laser* GetLaserByName(void);
    static void SetStreamAddress(const string& strAddress);
    static void SetStreamAddress(const char* szAddress);

    static vector<Laser*> m_clLaserList;
};

inline void Laser::SetXYOffsets(float fXOffset, float fYOffset)
{
    m_fXOffset = fXOffset;
    m_fYOffset = fYOffset;
}

inline void Laser::OverallMinReading(int& iReading, float& fAngle)
{
    iReading = m_iOverallMinReading;
    fAngle = m_fAngleOfOverallMinReading;
}

inline float* Laser::latest(void)
{
    return m_pfCurrentSickRanges;
}

inline float Laser::IndexToDegrees(int iIndex)
{
    return (-90.0 + ((float)iIndex / (float)(m_iNumReadings-1)) * 180.0) +
        m_fDegreeOffset;
}

inline void Laser::SetStreamAddress(const string& strAddress)
{
    m_strStreamAddress = strAddress;
}

#endif

/**********************************************************************
# $Log: laser.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.10  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.9  2002/08/21 21:23:40  kaess
# hserver crashes due to laser solved
#
# Revision 1.8  2002/01/16 21:54:48  ernest
# Added WatchDog function
#
# Revision 1.7  2001/05/31 17:40:45  blee
# changed readN to return a bool
#
# Revision 1.6  2001/03/23 21:25:25  blee
# altered to use a config file and allow for a laser's physical
# offset on a robot
#
# Revision 1.5  2000/12/12 22:58:08  blee
# Extensive updates were made to support multiple lasers.
#
# Revision 1.4  2000/09/22 20:30:27  blee
# Fixed bug that caused hserver to send old data to MissionLab
# part of the time.
#
# Revision 1.3  2000/09/19 03:43:50  endo
# Streaming bug fixed.
#
#
#**********************************************************************/
