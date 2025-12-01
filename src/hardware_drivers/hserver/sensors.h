/**********************************************************************
 **                                                                  **
 **                             sensor.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  William Halliburton                                **
 **                                                                  **
 **  Sensor routines for HServer                                     **
 **                                                                  **
 **  Copyright 2000 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: sensors.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef SENSORS_H
#define SENSORS_H
 
#define SENSORS_WIN_X      8
#define SENSORS_WIN_Y      5
#define SENSORS_WIN_ROW    40
#define SENSORS_WIN_COL    60

#include <panel.h>
#include <pthread.h>
#include "hserver.h"

class Sensor {
    friend class Sensors;
  protected:
    // these are for locking the data so buffers aren't being written and 
    // read at the same time. Originally, there were only the mutexes, not 
    // the pointers. The pointers were added so that a mutex could be passed
    // in via the constructor to protect the buffers. The actual mutexes were
    // kept for backwards compatibility. If no mutex pointer is passed in, 
    // then the member mutexes will be used to lock the buffers. -- jbl
    pthread_mutex_t* m_pmutexAngles;
    pthread_mutex_t* m_pmutexReadings;
    pthread_mutex_t m_mutexAnglesLocal;
    pthread_mutex_t m_mutexReadingsLocal;

    int m_iType;
    int m_iLenExtra;
    char* m_pcExtra;
    int m_iNumAngles;
    float* m_pfAngles;
    int m_iNumReadings;
    float* m_pfReadings;
    int m_iIndex;

    static bool TryToLock( pthread_mutex_t* pMutex );
    static bool TryToUnlock( pthread_mutex_t* pMutex );

public:
    int getType(){ return m_iType; };
    int getLenExtra(){ return m_iLenExtra; };
    char* getExtra(){ return m_pcExtra; };
    int getAngleCount();
    int getReadingCount();
    void getAngleInfo( float*& pfAngles, int& iCount );
    void getReadingInfo( float*& pfReadings, int& iCount );
    void setAngleInfo( float* pfAngles, int iCount );
    void setReadingInfo( float* pfReadings, int iCount );

    Sensor(int type,int len_extra = 0,char *extra = NULL,int num_angles = 0, // jbl -- laser stuff
           float *angle = NULL, int num_readings = 0, float *reading = NULL, 
           pthread_mutex_t* pAngleMutex = NULL, pthread_mutex_t* pReadingsMutex = NULL );
    ~Sensor();
};


class Sensors
{
protected:
    int max_sensors;
    char *index;
    int num_sensors;
    WINDOW *window;
    PANEL *panel;
    int panelX,panelY,panelW,panelH;
    pthread_t updator;
    void capInt(int *i,int l,int u);
    void drawWindow();
    void movePanel(int xdif,int ydif);
    void movePanelKey(int c);
    int runKey(int c);
    void windowInit();

public:
    Sensor **latest_sensor;
    int numSensors(){return num_sensors;};
    void updator_thread();
    void show();
    int add(Sensor *sensor);
    void remove(int idx);
    Sensors(int max);
    ~Sensors();
};

// this will try first to lock a passed-in mutex, but if there isn't one 
// a local member mutex will be used.
inline bool Sensor::TryToLock( pthread_mutex_t* pMutex )
{
    if ( pMutex != NULL )
    {
        pthread_mutex_lock( pMutex );
    }

    return ( pMutex != NULL );
}

// this will try first to unlock a passed-in mutex, but if there isn't one 
// a local member mutex will be used.
inline bool Sensor::TryToUnlock( pthread_mutex_t* pMutex )
{
    if ( pMutex != NULL )
    {
        pthread_mutex_unlock( pMutex );
    }

    return ( pMutex != NULL );
}

extern Sensors *sensors;

#endif

/**********************************************************************
# $Log: sensors.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 06:01:03  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.4  2000/12/12 23:08:59  blee
# Changed member variable names in Sensor. Changed "private" to "protected".
# Added mutex pointer member variables. Added TryToLock() and TryToUnlock().
# Changed/added accessor members. Changed sensor constructor prototype.
#
# Revision 1.3  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
