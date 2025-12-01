/**********************************************************************
 **                                                                  **
 **                               gps.c                              **
 **                                                                  **
 **                                                                  **
 **  Written by:       William Halliburton                           **
 **  Original code by: Tom Collins                                   **
 **                                                                  **
 **  NovAtel GPS driver for HServer                                  **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gps.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include "gps.h"
#include "ipt_handler.h"

Gps::Gps(Gps** a) :
    Module((Module**) a, HS_MODULE_NAME_GPS),
    poseCalcID_(-1)
{
    version = NULL;
    save_packets = false;
    base_x = 0;
    base_y = 0;

    pthread_mutex_init(&savePacketMutex, NULL);
    pthread_mutex_init(&mutexStatsLock, NULL);

}

Gps::~Gps()
{
	pthread_mutex_destroy(&savePacketMutex);
	pthread_mutex_destroy(&mutexStatsLock);
}

void Gps::information()
{
	printTextWindow("Gps information");
    printfTextWindow("version: %s", version);
}


void Gps::SetSavePackets(bool bSave)
{
	pthread_cleanup_push((void(*)(void*)) pthread_mutex_unlock, (void*) &savePacketMutex);
	pthread_mutex_lock(&savePacketMutex);

	savePackets(bSave);
	save_packets = bSave;

	pthread_cleanup_pop(1);
}

void Gps::savePackets(bool start)
{
	if (start)
	{
		packetFd = open("./gpspackets.dat", O_RDWR | O_CREAT | O_APPEND, S_IRWXU);
		if (packetFd == -1)
		{
			printTextWindow("Error opening gps packet file");
		}
	}
	else
	{
		close(packetFd);
	}
}


int Gps::getRT20Status(void)
{
    int status;

    pthread_mutex_lock(&mutexStatsLock);
    status = rt20Status;
    pthread_mutex_unlock(&mutexStatsLock);

    return status;
}

void Gps::connectRemote()
{
    printTextWindow("GPS Got connection from gps relay");
    iptHandler->set_state(IPT_MOD_GPS, true, true);
}

void Gps::addSensors()
{
    partGps = new RobotPartGps();
    sensorGps = new Sensor(SENSOR_GPS, 0, NULL, 0, NULL, EnGpsInfoType_COUNT, stats,
                            NULL, &mutexStatsLock);
}

SuGpsStats Gps::GetStats()
{
    SuGpsStats suStats;

    pthread_mutex_lock(&mutexStatsLock);

    suStats.dLat                 = stats[EnGpsInfoType_LAT];
    suStats.dLon                 = stats[EnGpsInfoType_LON];
    suStats.dHeight              = stats[EnGpsInfoType_HEIGHT];
    suStats.dDirection           = stats[EnGpsInfoType_DIRECTION];
    suStats.dHorizontalSpeed     = stats[EnGpsInfoType_HORIZONTAL_SPEED];
    suStats.dVerticalSpeed       = stats[EnGpsInfoType_VERTICAL_SPEED];
    suStats.iNumSats             = (int) stats[EnGpsInfoType_NUM_SATS];
    suStats.afXyt[0]           = xyt[0];
    suStats.afXyt[1]           = xyt[1];
    suStats.afXyt[2]           = xyt[2];
    suStats.iRT20Status          = rt20Status;
    suStats.iRtcmMsgsRcvdAtGps   = 0;
    suStats.iRtcmMsgsFailedAtGps = 0;

    pthread_mutex_unlock(&mutexStatsLock);

    return suStats;
}



