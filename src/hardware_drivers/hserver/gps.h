/**********************************************************************
 **                                                                  **
 **                               gps.h                              **
 **                                                                  **
 **                                                                  **
 **  Written by:     William Halliburton                             **
 **  With code from: Tom Collins                                     **
 **                                                                  **
 **  NovAtel GPS driver for HServer                                  **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gps.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef GPS_H
#define GPS_H

#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <string.h>
#include <termio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <math.h>

#include "robot_config.h"
#include "sensors.h"
#include "module.h"
#include "hserver_ipt.h"

using std::string;

// This macro takes a degree (d) and change its value to be inside
// the lower bound (l) and the higher bound (h).
#define GPS_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}

typedef unsigned char byte_t;

/* Default LAT and LON is at the roof of CRB */
#define DEFAULT_GPS_BASE_LAT   33.78135
#define DEFAULT_GPS_BASE_LON   -84.40034
#define DEFAULT_GPS_BASE_X     220
#define DEFAULT_GPS_BASE_Y     270

// recalculated the conversions factors using experimental methods -- jbl
//#define DEFAULT_GPS_M_PER_LON  92621.190
//#define DEFAULT_GPS_M_PER_LAT  110919.337
#define DEFAULT_GPS_M_PER_LON  89030.6
#define DEFAULT_GPS_M_PER_LAT  109220.0

#define LOG_SPHB       6
#define LOG_P20B       37
#define LOG_CDSB       39

struct bLogHeader
{
    byte_t sync[ 3 ];
    byte_t checksum;
    unsigned int id;
    unsigned int byte_count;
};

struct logP20B
{
    int week;
    double seconds;
    double lag;
    int num_sats;
    double lat;
    double lon;
    double height;
    double undulation;
    int datId;
    double latStdDev;
    double lonStdDev;
    double heightStdDev;
    int solutionStatus;
    int rt20Status;
    int fix;
    int cpuIdleTime;
    int monId;
};

struct logSPHB
{
    int week;
    double seconds;
    double hspeed;
    double tog;
    double vspeed;
    int solutionStatus;
};

struct logCDSB
{
    int week;
    int seconds;   // Yes, this is an int in this msg, but double in others
    int xonCOM1;
    int ctsCOM1;
    int parityErrorsCOM1;
    int overrunErrorsCOM1;
    int framingErrorsCOM1;
    int bytesReceivedCOM1;
    int bytesSentCOM1;
    int xonCOM2;
    int ctsCOM2;
    int parityErrorsCOM2;
    int overrunErrorsCOM2;
    int framingErrorsCOM2;
    int bytesReceivedCOM2;
    int bytesSentCOM2;
    int RTCAcrcFails;
    int RTCAchecksumFails;
    int RTCArecordsPassed;
    int RTCMparityFails;
    int RTCMchecksumFails;
    int RTCMrecordsPassed;
    int DCSAchecksumFails;
    int DCSArecordsPassed;
    int DCSBchecksumFails;
    int DCSBrecordsPassed;
};

typedef unsigned int uint;
struct rtcmHeader
{
    uint preamble  : 8;
    uint typeID    : 6;
    uint stationID : 10;
    uint parity    : 6;
    uint zcount    : 13;
    uint number    : 3;
    uint length    : 5;
    uint health    : 3;
    uint parity2   : 6;
};

struct SuGpsStats
{
    double dLat;
    double dLon;
    double dHeight;
    double dDirection;
    double dHorizontalSpeed;
    double dVerticalSpeed;
    int iNumSats;
    float afXyt[ 3 ];
    int iRT20Status;
    int iRtcmMsgsRcvdAtGps;
    int iRtcmMsgsFailedAtGps;
};

enum EnGpsInfoType
{
    EnGpsInfoType_LAT = 0,
    EnGpsInfoType_LON,
    EnGpsInfoType_HEIGHT,
    EnGpsInfoType_DIRECTION,
    EnGpsInfoType_HORIZONTAL_SPEED,
    EnGpsInfoType_VERTICAL_SPEED,
    EnGpsInfoType_NUM_SATS,
    EnGpsInfoType_COUNT     // this must be declared last
};

class Gps: public Module
{
protected:
	Sensor* sensorGps;
	RobotPartGps* partGps;
	int poseCalcID_;
	char* version;
	double m_per_deg_lon, m_per_deg_lat;
	bool save_packets;
	pthread_mutex_t savePacketMutex;
	pthread_mutex_t mutexStatsLock;
	int rt20Status;
	int packetFd;
	float stats[ EnGpsInfoType_COUNT ];
	float xyt[3];
	double base_x,base_y;

	void savePackets(bool start);
	void addSensors();

public:
	void information();
	void savePoseCalcID(int id);
	double GetBaseX() { return base_x; }
	double GetBaseY() { return base_y; }
	void AlterBaseXBy( double dDiff ) { base_x += dDiff; }
	void AlterBaseYBy( double dDiff ) { base_y += dDiff; }
	bool GetSavingPackets() { return save_packets; }
	void SetSavePackets( bool bSave );
	int getRT20Status(void);
	void connectRemote();

	virtual SuGpsStats GetStats();

    Gps(Gps** a);
    ~Gps();
};

inline void Gps::savePoseCalcID(int id)
{
    poseCalcID_ = id;
}

extern Gps* gps;

#endif

/**********************************************************************
# $Log: gps.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.2  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.3  2004/09/10 19:43:26  endo
# New PoseCalc integrated.
#
# Revision 1.2  2004/04/13 19:25:33  yangchen
# JBox module integrated.
#
# Revision 1.1.1.1  2004/02/27 22:30:50  endo
# Initial MARS 2020 revision
#
# Revision 1.6  2003/04/06 13:32:37  endo
# gcc 3.1.1
#
# Revision 1.5  2001/05/29 22:33:52  blee
# Made changes to incorporate gps data w/ other position info.
#
# Revision 1.4  2001/03/23 21:33:06  blee
# altered to use a config file
#
# Revision 1.3  2000/10/16 19:57:26  endo
# Read buffer problem fixed. Support for CDSB msg added.
#
# Revision 1.2  2000/09/19 03:11:10  endo
# RCS log added.
#
#
#**********************************************************************/
