/**********************************************************************
 **                                                                  **
 **                             sensor.c                             **
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

/* $Id: sensors.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <string.h>
#include <stdlib.h>
#include <panel.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include "hserver.h"
#include "sensors.h"


Sensors *sensors;

Sensor::Sensor(
    int type,
    int len_extra/*=0*/,
    char *extra/*=NULL*/,
    int num_angles/*=0*/,
    float *angle/*=NULL*/,
    int num_readings/*=0*/,
    float *reading/*=NULL*/,
    pthread_mutex_t* pAngleMutex/*=NULL*/,
    pthread_mutex_t* pReadingsMutex/*=NULL*/)
{
    m_iType = type;
    m_iLenExtra = len_extra;
    m_pcExtra = extra;
    m_iNumAngles = num_angles;
    m_pfAngles = angle;
    m_iNumReadings = num_readings;
    m_pfReadings = reading;
    m_pmutexAngles = pAngleMutex;
    m_pmutexReadings = pReadingsMutex;
    pthread_mutex_init( &m_mutexAnglesLocal, NULL );
    pthread_mutex_init( &m_mutexReadingsLocal, NULL );
    m_iIndex = sensors->add( this );
}

Sensor::~Sensor()
{
    sensors->remove( m_iIndex );
}

int Sensor::getAngleCount()
{
    int iCount = 0;

    // aquire the angle lock
    if ( !TryToLock( m_pmutexAngles ) )
    {
        pthread_mutex_lock( &m_mutexAnglesLocal );
    }

    iCount = m_iNumAngles;

    // release the angle lock
    if ( !TryToUnlock( m_pmutexAngles ) )
    {
        pthread_mutex_unlock( &m_mutexAnglesLocal );
    }

    return iCount;
}

int Sensor::getReadingCount()
{
    int iCount = 0;

    // aquire the angle lock
    if ( !TryToLock( m_pmutexReadings ) )
    {
        pthread_mutex_lock( &m_mutexReadingsLocal );
    }

    iCount = m_iNumReadings;

    // release the angle lock
    if ( !TryToUnlock( m_pmutexReadings ) )
    {
        pthread_mutex_unlock( &m_mutexReadingsLocal );
    }

    return iCount;
}

void Sensor::getAngleInfo( float*& pfAngles, int& iCount )
{
    // aquire the angle lock
    if ( !TryToLock( m_pmutexAngles ) )
    {
        pthread_mutex_lock( &m_mutexAnglesLocal );
    }

    // make a copy of the angle info
    if ( m_iNumAngles > 0 )
    {
        iCount = m_iNumAngles;
        if ( ( pfAngles = new float[ m_iNumAngles ] ) != NULL )
        {
            memcpy( pfAngles, m_pfAngles, m_iNumAngles * sizeof( float ) );
        }
    }
    else
    {
        pfAngles = NULL;
        iCount = 0;
    }

    // release the angle lock
    if ( !TryToUnlock( m_pmutexAngles ) )
    {
        pthread_mutex_unlock( &m_mutexAnglesLocal );
    }
}

void Sensor::getReadingInfo( float*& pfReadings, int& iCount )
{
    // aquire the readings lock
    if ( !TryToLock( m_pmutexReadings ) )
    {
        pthread_mutex_lock( &m_mutexReadingsLocal );
    }

    // make a copy of the readings info
    if ( m_iNumReadings > 0 )
    {
        iCount = m_iNumReadings;
        if ( ( pfReadings = new float[ m_iNumReadings ] ) != NULL )
        {
            memcpy( pfReadings, m_pfReadings, m_iNumReadings * sizeof( float ) );
        }
    }
    else
    {
        pfReadings = NULL;
        iCount = 0;
    }

    // release the readings lock
    if ( !TryToUnlock( m_pmutexReadings ) )
    {
        pthread_mutex_unlock( &m_mutexReadingsLocal );
    }
}

// general note on setAngleInfo() and setReadingInfo()...
// If the actual sensor that this object represents did not
// pass in a mutex pointer to lock, then this object will
// use its local mutex to protect the buffer. If the sensor
// did pass in a mutex pointer, then it is up to that sensor
// to lock it. It knows when it's own data needs to be
// protected, and it's reasonable that it will want to lock
// the data while this change takes place, so if we also try
// to lock it here, we'll have deadlock. -- jbl

// see above general comment
void Sensor::setAngleInfo( float* pfAngles, int iCount )
{
    if ( m_pmutexAngles == NULL )
    {
        pthread_mutex_lock( &m_mutexAnglesLocal );
    }

    m_pfAngles = pfAngles;
    m_iNumAngles = iCount;

    if ( m_pmutexAngles == NULL )
    {
        pthread_mutex_unlock( &m_mutexAnglesLocal );
    }
}

// see above general comment
void Sensor::setReadingInfo( float* pfReadings, int iCount )
{
    if ( m_pmutexReadings == NULL )
    {
        pthread_mutex_lock( &m_mutexReadingsLocal );
    }

    m_pfReadings = pfReadings;
    m_iNumReadings = iCount;

    if ( m_pmutexReadings == NULL )
    {
        pthread_mutex_unlock( &m_mutexReadingsLocal );
    }
}

int Sensors::add( Sensor* sensor )
{
    int i;
    int idx = -1;
    if ( num_sensors == max_sensors )
    {
        return -1;
    }
    latest_sensor[ num_sensors ] = sensor;
    for( i = 0; i < max_sensors; i++ )
    {
        if ( index[ i ] == -1 )
        {
            idx = i;
            break;
        }
    }
    index[ idx ] = num_sensors;
    num_sensors++;
    return idx;
}

void Sensors::remove( int idx )
{
    int i, low;
    low = index[ idx ];
    index[ idx ] = -1;
    for( i = 0; i < max_sensors; i++ )
    {
        if ( index[ i ] > low )
        {
            index[i]--;
        }
        if ( ( i >= low ) && ( i < max_sensors - 1 ) )
        {
            latest_sensor[ i ] = latest_sensor[ i + 1 ];
        }
    }
    num_sensors--;
}

// This function displays the updated sensor information of the robot in the sensor window.
void Sensors::drawWindow(){
    char buf[100];
    char buf2[100];
    int line = 1;
    int i,j;

    pthread_cleanup_push((void(*)(void*))pthread_mutex_unlock, (void *) &screenMutex);
    pthread_mutex_lock( &screenMutex );
    werase(window);
    wmove(window,0,0);
    wborder(window,0,0,0,0,0,0,0,0);
    mvwaddstr( window, line++, 1, "Robot Sensors" );
    mvwaddstr( window, line++, 1, "" );

    for( i = 0; i < num_sensors; i++ )
    {
        switch( latest_sensor[i]->getType() )
        {
        case SENSOR_PANTILT_CAMERA:
            sprintf(buf,"%d.  pan/tilt  pan: %.0f tilt: %.0f zoom: %.0f focus %.0f",i,
                    latest_sensor[i]->m_pfAngles[0],latest_sensor[i]->m_pfAngles[1],
                    latest_sensor[i]->m_pfAngles[2],latest_sensor[i]->m_pfAngles[3]);
            break;
        case SENSOR_XYT:
            sprintf(buf,"%d.  Dead Reckoning: ",i);
            mvwaddstr(window,line++,1,buf);
            sprintf(buf,"    x: %.2f y: %.2f t: %.2f",
                    latest_sensor[i]->m_pfReadings[0],latest_sensor[i]->m_pfReadings[1],
                    latest_sensor[i]->m_pfReadings[2]);
            break;
        case SENSOR_ARM:
            sprintf(buf,"%d.  arm  angle: %.3f",i,latest_sensor[i]->m_pfAngles[0]);
            break;
        case SENSOR_SICKLMS200:
            sprintf(buf,"%d.  laser  num readings: %d",i,
                    latest_sensor[i]->getReadingCount() );
            break;
        case SENSOR_SONAR:
            float rdg;
            int num, num_rdgs;
            num_rdgs = latest_sensor[i]->getReadingCount();
            sprintf(buf,"%d.  Sonar Readings: ",i);
            mvwaddstr(window,line++,1,buf);
            strcpy(buf,"    ");
            num = 1;
            for(j=0;j<num_rdgs;j++)
            {
                rdg = latest_sensor[i]->m_pfReadings[j];
                if (rdg == (float)SONAR_READING_OUTSIDE_RANGE)
                    sprintf(buf2,"---- ");
                else
                    sprintf(buf2,"%.2f ",rdg);
                strcat(buf,buf2);

                if (num == 8)
                {
                    mvwaddstr(window,line++,1,buf);
                    strcpy(buf,"    ");
                    num = 0;
                }
                ++num;
            }
            break;

        case SENSOR_ROOMBA:
            num_rdgs = latest_sensor[i]->getReadingCount();
            sprintf(buf,"%d.  Roomba Readings: ",i);
            mvwaddstr(window,line++,1,buf);
            strcpy(buf,"    ");
            num = 1;
            for(j=0;j<num_rdgs;j++)
            {
                rdg = latest_sensor[i]->m_pfReadings[j];

                sprintf(buf2,"%.2f ",rdg);
                strcat(buf,buf2);

                if (num == 10)
                {
                    mvwaddstr(window,line++,1,buf);
                    strcpy(buf,"    ");
                    num = 0;
                }
                ++num;
            }
            break;

        case SENSOR_IR:
            sprintf(buf,"%d.  ir  num readings: %d",i,
                    latest_sensor[i]->getReadingCount());
            mvwaddstr(window,line++,1,buf);
            strcpy(buf,"    ");
            if (latest_sensor[i]->getReadingCount() > 8) num = 8;
            else num = latest_sensor[i]->getReadingCount();
            for(j=0;j<num;j++){
                sprintf(buf2,"%.2f ",latest_sensor[i]->m_pfReadings[j]);
                strcat(buf,buf2);
            }
            if (latest_sensor[i]->getReadingCount() > 8){
                mvwaddstr(window,line++,1,buf);
                strcpy(buf,"    ");
                num = latest_sensor[i]->getReadingCount() - 8;
                for(j=0;j<num;j++){
                    sprintf(buf2,"%.2f ",latest_sensor[i]->m_pfReadings[j+8]);
                    strcat(buf,buf2);
                }
            }
            break;
        case SENSOR_COGNACHROME:
            sprintf(buf,"%d.  cognachrome num readings: %d",i,
                    latest_sensor[i]->getReadingCount());
            break;
        case SENSOR_VIDEO:
            sprintf(buf,"%d.  video  num readings: %d",i,
                    latest_sensor[i]->getReadingCount());
            break;
        case SENSOR_COMPASS:
            sprintf(buf,"%d.  compass  angle: %f",i,latest_sensor[i]->m_pfAngles[0]);
            break;
        case SENSOR_GPS:
            sprintf( buf, "%d.  gps lat: %f  lon: %f  height: %f  tog: %f sats: %.0f",
                     i, latest_sensor[i]->m_pfReadings[0],
                     latest_sensor[i]->m_pfReadings[1],latest_sensor[i]->m_pfReadings[2],
                     latest_sensor[i]->m_pfReadings[3],latest_sensor[i]->m_pfReadings[4]);
            break;
        default:
            sprintf(buf,"%d.  Unknown sensor type",i);
        }
        mvwaddstr(window,line++,1,buf);
    }
    pthread_cleanup_pop(1);
}

static void start_thread(void *sensors_instance)
{
    ((Sensors*)sensors_instance)->updator_thread();
}

void Sensors::updator_thread()
{
    while( true )
    {
        pthread_testcancel();
        drawWindow();
        redrawWindows();
        usleep(500000);
    }
}

void Sensors::capInt( int* i, int l, int u )
{
    if ( *i < l ) *i = l;
    if ( *i > u ) *i = u;
}

void Sensors::movePanel( int xdif, int ydif )
{
    panelX += xdif;
    panelY += ydif;
    capInt( &panelX, 0, mainPanelX - SENSORS_WIN_COL );
    capInt( &panelY, 0, mainPanelY - SENSORS_WIN_ROW );
    move_panel( panel, panelY, panelX );
    redrawWindows();
}

void Sensors::movePanelKey( int c )
{
    switch( c )
    {
    case '1':
        movePanel(-1,1);
        break;
    case '2':
        movePanel(0,1);
        break;
    case '3':
        movePanel(1,1);
        break;
    case '4':
        movePanel(-1,0);
        break;
    case '5':
        break;
    case '6':
        movePanel(1,0);
        break;
    case '7':
        movePanel(-1,-1);
        break;
    case '8':
        movePanel(0,-1);
        break;
    case '9':
        movePanel(1,-1);
        break;
    }
}


int Sensors::runKey( int c )
{
    switch(c){
    case 'r':
        drawWindow();
        refreshScreen();
        break;
    case 'x':
    case 'Q':
        pthread_cancel(updator);
        pthread_join(updator,NULL);
        return(1);
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        movePanelKey(c);
        break;
    }
    return 0;
}


void Sensors::show()
{
    int c,done;

    done = false;
    show_panel(panel);
    drawWindow();
    redrawWindows();
    pthread_create( &updator, NULL, (void*(*)(void*))&start_thread, (void*) this );
    do
    {
        c = getch();
        done = runKey( c );
    } while ( !done );
    hide_panel(panel);
    redrawWindows();
    return;
}

void Sensors::windowInit()
{
    panelX = SENSORS_WIN_X;
    panelY = SENSORS_WIN_Y;
    panelH = SENSORS_WIN_ROW;
    panelW = SENSORS_WIN_COL;
    if ( panelH > screenY )
    {
        panelH = screenY;
    }
    if ( panelW > screenX )
    {
        panelW = screenX;
    }
    if ( panelX + panelW > screenX )
    {
        panelX = screenX - panelW;
    }
    if ( panelY + panelH > screenY )
    {
        panelY = screenY - panelH;
    }
    window = newwin( panelH, panelW, panelY, panelX );
    panel = new_panel( window );
    drawWindow();
}

Sensors::~Sensors()
{
    delete [] latest_sensor;
    delete [] index;
}

Sensors::Sensors( int max )
{
    num_sensors = 0;
    max_sensors = max;
    latest_sensor = new Sensor*[ max_sensors ];
    memset( latest_sensor, -1, sizeof(Sensor*) * max_sensors );
    index = new char[ max_sensors ];
    memset( index, -1, sizeof(char) * max_sensors );
    windowInit();
    hide_panel( panel );
}

/**********************************************************************
# $Log: sensors.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.3  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
# Revision 1.2  2006/05/15 03:06:53  endo
# gcc-3.4 upgrade
#
# Revision 1.1.1.1  2005/02/06 22:59:42  endo
# AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
#
# Revision 1.8  2002/01/16 22:40:54  ernest
# Initialize latest_sensor as precaution
#
# Revision 1.7  2001/03/08 23:32:08  blee
# Added initialization of member mutexes.
#
# Revision 1.6  2000/12/12 23:05:24  blee
# Changed member variable names and the constructor.
# Added getAngleCount, getReadingCount, getAngleInfo, getReadingInfo,
# setAngleInfo, and setReadingInfo. Made more multi-thread safe.
# Changed the case for SENSOR_SICKLMS200 in Sensors::drawWindow().
# Changed references in Sensors to Sensor member variables.
#
# Revision 1.5  2000/11/13 21:05:43  endo
# *** empty log message ***
#
# Revision 1.4  2000/11/13 20:09:23  endo
# ARTV-Jr class added to HServer.
#
# Revision 1.3  2000/09/19 03:49:46  endo
# RCS log added.
#
#
#**********************************************************************/
