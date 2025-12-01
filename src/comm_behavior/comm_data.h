/**********************************************************************
 **                                                                  **
 **                           comm_data.h                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **  describes the format of the comm sensor data.                   **
 **                                                                  **
 **  duplicate data structures (point, vector) because the need to   **
 **  be utilized by non-missionlab code                              **
 **                                                                  **
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* $Id: comm_data.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef COMM_DATA_H
#define COMM_DATA_H


#define MAX_ROBOTS 100

struct Comm_Vector {
  float x;
  float y;
  float z;
};  

struct gt_LatLong_and_Hop {
	int id;
	double lat;
	double lng;
	int hops;
	double signal_strength;
};

struct gt_Pos_and_Hop {
    int     id;
    Comm_Vector  v;
    double    hops;
    double    signal_strength;
};


class Comm_Sensor_Data
{
public:
    int                  num_readings;
    gt_Pos_and_Hop* readings;

    Comm_Sensor_Data( int num = 0 )
    {
        num_readings = 0;
        readings = NULL;

        if ( num > 0 )
        {
            num_readings = num;
            readings = new gt_Pos_and_Hop[ num ];
        }
    }

    Comm_Sensor_Data( const Comm_Sensor_Data& list )
    {
        num_readings = 0;
        readings = NULL;

        *this = list;
    }

    /* Assignment operator.  We have to really careful about this memory stuff. */
    Comm_Sensor_Data& operator=( const Comm_Sensor_Data& list )
    {
        if ( this != &list )
        {
            cleanup();

            if( list.num_readings > 0 )
            {
                num_readings = list.num_readings;
                readings = new gt_Pos_and_Hop[ num_readings ];
                for( int i = 0; i < num_readings; i++ )
                {
                    readings[ i ].v.x = list.readings[ i ].v.x;
                    readings[ i ].v.y = list.readings[ i ].v.y;
                    readings[ i ].v.z = list.readings[ i ].v.z;
                    readings[ i ].v.z = list.readings[ i ].v.z;
                    readings[ i ].id = list.readings[ i ].id;
                    readings[ i ].hops = list.readings[ i ].hops;
                    readings[ i ].signal_strength = list.readings[ i ].signal_strength;
                }
            }
        }

        return *this;
    }

    /* Important:  We need to delete the allocated memory when we fall out of scope. */
    ~Comm_Sensor_Data()
    {
        cleanup();
    }

protected:

    void cleanup()
    {
        num_readings = 0;
        if ( readings != NULL )
        {
            delete [] readings;
            readings = NULL;
        }
    }
};


#endif

/**********************************************************************
 * $Log: comm_data.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
