/**********************************************************************
 **                                                                  **
 **                               jbox.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by: Yang Chien                                          **
 **                                                                  **
 **  Copyright 2003 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: jbox.h,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#ifndef JBOX_H
#define JBOX_H

#include "message.h"
#include "statusbar.h"
#include "hserver.h"
#include "module.h"
#include "gps.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <string> 
#include <fstream>
#include <vector>

#include <string.h>

using namespace std;

typedef enum JBoxNeighborType_t
{
    JBOX_NEIGHBOR_INDIRECT,
    JBOX_NEIGHBOR_DIRECT
};


typedef struct
{
    int ID;
    int signal_strength;    
    int pos_reading[7];
    double gps_reading[7];
    int num_hops;

	//if the node is direct neighbor, this value is set to be 1,
    // otherwise, it is set to be 0;
	int	neighbor_tag;

} JBOX_data;


typedef vector<JBOX_data> JBoxDataList_t;

class Jbox : public Module 
{
public:
	    
	Jbox(
        Jbox** a,
        int id,
        bool disableNetworkQuery,
        double gps_base_lat,
        double gps_base_lon,
        double gps_base_x,
        double gps_base_y,
        double gps_mperlat,
        double gps_mperlon);
    JBoxDataList_t getJBoxDataList(void);
	int nodes_in_network();
    int getSelfID(void);
    int get_hops(int id);
    int get_quality(int id);
    void get_pos(int* pos_array, int id);
    void get_gps(double* gps_array, int id);

	//The following two are query functions	
	int quality_to_neighbor(int id);	
	int GPS_of_node(double* gps_reading, int id);	
	int POS_of_node(int* pos_reading, int id);
    int HOPS_of_node( int id ); //used in the query

    static void connectToJbox( Jbox*& jbox );
	static void connectToJbox(
        Jbox*& jbox,
        int id,
        bool disableNetworkQuery,
        double gps_base_lat,
        double gps_base_lon,
        double gps_base_x,
        double gps_base_y,
        double gps_mperlat,
        double gps_mperlon);

	~Jbox();

//======================================================

protected:

	static void* JBOX_startMainThread(void* jbox_instance);
	
	pthread_t updateThread;
	void updateLoop(void);

	int axtoi(char *hexStg);
	int simple_parser1(int* int_arr, const char* str, const char* delimiter, int limit);
	int simple_parser2(double* double_arr, const char* str, const char* delimiter, int limit);
    int simple_parser3(int* int_arr, const char* str, const char* delimiter);

	FILE* open_command(const char* command);
	void str_man(int op);
	void error_report(const char* buffer);
	void error_report();
    void gps_update();
	void neighbor_list(JBoxDataList_t *jboxDataList);
	void link_quality(JBoxDataList_t *jboxDataList);
	void whole_pos(JBoxDataList_t *jboxDataList);
    void updateStatusBar_(void);
    void logData_(double logTime);

    // Shared. Should be protected by mutex.
    JBoxDataList_t shJBoxDataList;

    int selfID;
    bool disableQuery;

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

    const double GPS_BASE_LAT;
    const double GPS_BASE_LON;
    const double GPS_BASE_X;
    const double GPS_BASE_Y;
    const double GPS_M_PER_LAT;
    const double GPS_M_PER_LON;

    static const double OFFSET_MLAB_TO_GPS_HEADING;
    static const int INVALID_SIGNAL_STRENGTH;
    static const int INVALID_NUM_HOPS;
    static const int INVALID_NUM_NODES;
    static const int JBOX_UPDATE_THREAD_USLEEP;
    static const int JBOX_NON_QUERY_SLEEP_SEC;
};

inline int Jbox::getSelfID(void)
{
    return selfID;
}

extern Jbox *jbox;

#endif //for h file

/**********************************************************************
# $Log: jbox.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
