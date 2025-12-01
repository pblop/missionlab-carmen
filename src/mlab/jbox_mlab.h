/**********************************************************************
 **                                                                  **
 **                          jbox_mlab.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by: Yang Chen                                           **
 **  Modified by: Patrick Ulam and Yoichiro Endo                     **
 **                                                                  **
 **  Copyright 2004 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: jbox_mlab.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#ifndef JBOX_MLAB_H
#define JBOX_MLAB_H

//#include "message.h"

//#include "module.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <Xm/Xm.h>

#include <iostream>
#include <string> 
#include <fstream>
//#include <vector>

using namespace std;

typedef struct
{
    int ID;
    int signal_strength;    
    int pos_reading[7];
    double gps_reading[7];
    int num_hops;
} JBOX_data;

typedef vector<JBOX_data> JBoxDataList_t;

class JBoxMlab {

public:
    typedef enum
    {
        GPS_LATITUDE,
        GPS_LONGITUDE,
        GPS_ALTITUDE,
        GPS_HEADING,
        GPS_PITCH,
        GPS_ROLL,
        GPS_TIME,
        GPS_NUM_ELEMENTS // Keep this in the end of the list.
    
    } GPSElements_t;

    JBoxMlab(int id);	
    JBoxDataList_t getJBoxDataList(double *timeStamp);
	
	int nodes_in_network();
    int get_hops(int id);
    int get_quality(int id);
    void get_pos(int* pos_array, int id);
    void get_gps(double* gps_array, int id);

	//The following two are query functions	
	int quality_to_neighbor(int id);	
	int GPS_of_node(double* gps_reading, int id);	
	int POS_of_node(int* pos_reading, int id);
    int HOPS_of_node( int id ); //used in the query

	float LongToX(double lng);
    float LatToY(double lat);
    double gpsHeadToMlabHead(double gpsHeading);


	~JBoxMlab();

//======================================================

private:

	int axtoi(char *hexStg);
	int simple_parser1(int* int_arr, const char* str, const char* delimiter, int limit);
	int simple_parser2(double* double_arr, const char* str, const char* delimiter, int limit);
    int simple_parser3(int* int_arr, const char* str, const char* delimiter);
    double getCurrentTime(void);

	FILE* open_command(const char* command);
	void str_man(int op);
    void Load_BaseInfo(void);
	void neighbor_list(JBoxDataList_t *jboxDataList);
	void link_quality(JBoxDataList_t *jboxDataList);
	void whole_pos(JBoxDataList_t *jboxDataList);
    void readerLoop_(void);

    static void *startReaderThread_(void *jboxInstance);

    // Shared.
    JBoxDataList_t shJBoxDataList_;
    double shJBoxDataTimeStamp_;

    pthread_t readerThread_;

    int selfID;

	char buffer[128];
	FILE   *chk;
	
	char j_id[4]; 
	char j_op[4];

	char j_prefix[128];
	char j_postfix[128];

	char bk_prefix[128];
	char bk_postfix[128];

    char j_gpsset[128];
    char bk_gpsset[128];

    double Base_X;
    double Base_Y;
    double Base_Lat;
    double Base_Long;
    double MPerLat;
    double MPerLong;

    double startTime_;

    static const int JBOX_UPDATE_THREAD_USLEEP;
    static const int INVALID_SIGNAL_STRENGTH;
    static const int INVALID_NUM_HOPS;
    static const int INVALID_NUM_NODES;
    static const int OFFSET_GPS_TO_MLAB_HEADING;

    static pthread_mutex_t jboxDataMutex_;
};

inline double JBoxMlab::gpsHeadToMlabHead(double gpsHeading)
{
    return (gpsHeading + OFFSET_GPS_TO_MLAB_HEADING);
}

extern JBoxMlab *gJBoxMlab;

#endif //for h file

