/**********************************************************************
 **                                                                  **
 **                          jbox_mlab.c                             **
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

/* $Id: jbox_mlab.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */
#include <stdio.h>
#include <pthread.h>

#include "gt_sim.h"
#include "jbox_mlab.h"

JBOX_data  *single_jbox_record;
JBOX_data *jbdata;

// Mutex
pthread_mutex_t JBoxMlab::jboxDataMutex_ = PTHREAD_MUTEX_INITIALIZER;

// Constants
const int JBoxMlab::JBOX_UPDATE_THREAD_USLEEP = 100000;
const int JBoxMlab::INVALID_SIGNAL_STRENGTH = -1;
const int JBoxMlab::INVALID_NUM_HOPS = -1;
const int JBoxMlab::INVALID_NUM_NODES = -1;
const int JBoxMlab::OFFSET_GPS_TO_MLAB_HEADING = 90;

// In this constructor, the ID of this jbox and corresponding character strings (used to build up command lines)
// are initiated!
JBoxMlab::JBoxMlab(int id)
{
    timeval tv;

    gettimeofday(&tv, NULL);
    startTime_ = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000);
    shJBoxDataTimeStamp_ = 0;

    selfID = id;
    strcpy(j_prefix, "snmpwalk 192.168.");
    strcpy(j_gpsset, "snmpset 192.168.");

    sprintf(j_id, "%d", id);
    strcat(j_prefix, j_id);
    strcat(j_gpsset, j_id);
    strcat(j_prefix, ".1 public ");
    strcat(j_gpsset, ".1 robot enterprises.14.5.15.3.4.0 s ");
    
    strcpy(bk_prefix, j_prefix);
    strcpy(bk_gpsset, j_gpsset);
 
    strcpy(j_postfix, "enterprises.14.5.15.3.");
    strcpy(bk_postfix, "enterprises.14.5.15.3.");

    Load_BaseInfo();

    pthread_create(&readerThread_, NULL, &startReaderThread_, (void *)this);
}

JBoxMlab::~JBoxMlab()
{
  pthread_cancel(readerThread_);
  pthread_join(readerThread_, NULL);
}

// Similar function to atoi, but here it converts a hexadecimal digit string to an integer value
// If the digit string is "00xxx" (here x is from 0 ~ F), it is equal to "xxx"
int JBoxMlab::axtoi(char *hexStg) 
{
    int n = 0;         // position in string
    int m = 0;         // position in digit[] to shift
    int l;  	
    int count;         // loop index
    int intValue = 0;  // integer value of hex string
    int digit[128];    // hold values to convert
	
    l=strlen(hexStg);
    
    while (n < l) 
    {     	
	    if (hexStg[n]=='\0')
        {
	        break;
        }
     	
	    if (hexStg[n] > 0x29 && hexStg[n] < 0x40 ) //if 0 to 9
        {
            digit[n] = hexStg[n] & 0x0f;            //convert to int
        }
     	else if (hexStg[n] >='a' && hexStg[n] <= 'f') //if a to f
        {
            digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
        }
     	else if (hexStg[n] >='A' && hexStg[n] <= 'F') //if A to F
        {
            digit[n] = (hexStg[n] & 0x0f) + 9;      //convert to int
        }
     	else
        {
	        break;
        }
	    n++;
    }
    
    count = n;
    m = n-1;
    n = 0;
    
    while(n < count) 
    {
     	// digit[n] is value of hex digit at position n
    	// (m << 2) is the number of positions to shift
     	// OR the bits into return value
     	intValue = intValue | (digit[n] << (m << 2));
     	m--;   // adjust the position to set
     	n++;   // next digit to process
    }
    
    return (intValue);
}

// method to parse integer array, then return the total number of number converted and the numbers stored in the array int_arr
// str is the string needs parsing and delimiter is a string consists of all the delimiters;
// limit is the upper bound of conversion times to prevent unusual case
int JBoxMlab::simple_parser1(int* int_arr, const char* str, const char* delimiter, int limit)
{
    char *token, *cp;
    int total_comp_num;                                                                                                                                                            
    total_comp_num=0;

    cp = strdup (str);                // Make writable copy.                                                                                                                                                               
    token = strtok (cp, delimiter);      // token => "words" 
    while (token!=NULL)
    {
	    
        if ( (total_comp_num) < limit )
	    {   
	        int_arr[total_comp_num]=atoi(token);
	        total_comp_num++;
	    }

        token = strtok (NULL, delimiter);
    }

    return total_comp_num;
}
                                                                                                                                                             
// method to parse double string, then return the total number of number converted and the numbers stored in the array double_arr
// str is the string needs parsing and delimiter is a string consists of all the delimiters;
// limit is the upper bound of conversion times to prevent unusual case
int JBoxMlab::simple_parser2(double* double_arr, const char* str, const char* delimiter, int limit)
{

    char *token, *cp;
    
    int total_comp_num;                                                                                                                                                            
    total_comp_num=0;

    cp = strdup (str);                // Make writable copy.  

    token = strtok (cp, delimiter);      // token => "words" 
    while (token!=NULL)
    {	
	    if ( (total_comp_num) < limit )
	    {	
	        double_arr[total_comp_num]=atof(token);
	        total_comp_num++;
	    }

        token = strtok (NULL, delimiter);
    }
    return total_comp_num;
}
                                                                                                                                                             
// method to parse hexdecmial digit string, then return the total number of number converted and the numbers stored in the array int_arr
// str is the string needs parsing and delimiter is a string consists of all the delimiters;
// limit is the upper bound of conversion times to prevent unusual case
int JBoxMlab::simple_parser3(int* int_arr, const char* str, const char* delimiter)
{
    char *token, *cp;
    char temp[4];
    int total_comp_num;                                                                                                                                                            
    total_comp_num=0;

    cp = strdup (str);                // Make writable copy.  

    token = strtok (cp, delimiter);      // token => "words" 
    
    while (token!=NULL)
    {
        strcpy(temp, token);

        token = strtok (NULL, delimiter);
        if ( token==NULL )
	    {
	        fprintf(stderr, "Warning: JBoxMlab::simple_parser3(). A bad hexdecimal value.\n");
	        break;
	    }
        else
        {
            int_arr[total_comp_num]=axtoi( strcat(temp,token) );
            total_comp_num++;
            token = strtok (NULL, delimiter);
        }
    }

    return total_comp_num;
}

// function that sends out the command and open the pipe to receive feedback from that command
FILE* JBoxMlab::open_command(const char* command)
{
    FILE *fp;

    if( (fp = popen( command, "r" )) == NULL )
    {
        fprintf(stderr, "Error: JBoxMlab::open_command(). Popen failed.\n");
        quit_mlab();
    }

    return fp;

}

// The reason for the following function is that different commands correspond to different operation
// number and command lines, thus, those character-strings would be different.

void JBoxMlab::str_man(int op)
{
   // turn the op code into string type;
    sprintf(j_op, "%d", op);
	
    strcpy(j_prefix, bk_prefix);
    strcpy(j_postfix, bk_postfix);

    strcat(j_postfix, j_op);
    // After the following execuation, j_prefix would be the entire command
    strcat(j_prefix, j_postfix);
}

int JBoxMlab::nodes_in_network()
{
    int i;

    str_man(1);

    chk=open_command(j_prefix);

    if( fgets( buffer, 128, chk ) != NULL )
    {	
        // more robustness needed here if the returned is an arbitrary string!!		
	    i=atoi( (strstr(buffer, "=")+2) );
	    
        if ((i<1)||(i>128))
	    {
	        fprintf(stderr, "Warning: JBoxMlab::nodes_in_network(). JBox output corrupted.\n");
            return INVALID_NUM_NODES; 
	    }
    }
    else
    {
	    fprintf(stderr, "JBox Warning: nodes_in_network(). No valid response from JBox.\n");
        return INVALID_NUM_NODES;
    }
    
    fclose(chk);

    return i;
}

int JBoxMlab::get_hops(int id)
{
    int i;
    char node_id[4];

    str_man(6);

    strcat(j_prefix, ".1.4.");
    if ( atoi(j_id) == id )
    {    
        return 0;
    }
    else
    {
        strcat(j_prefix, j_id);
        sprintf(node_id, ".%d", id);
        strcat(j_prefix, node_id);
        strcat(j_prefix, ".1");
    
        chk=open_command(j_prefix);
                                                                                                                             
        if( fgets( buffer, 128, chk ) != NULL )
        {
            i=atoi( (strstr(buffer, "=")+2) );
        }
        else
        {
            fprintf(stderr, "Warning: JBoxMLab::get_hops(). No valid response from JBox.\n");
            return INVALID_NUM_HOPS;
        }
        fclose(chk);
        return i;
    }
}

int JBoxMlab::get_quality(int id)
{
    int i;
    char node_id[4];
                                                                                                                             
    str_man(9);
                                                                                                                             
    strcat(j_prefix, ".1.2.");
    if ( atoi(j_id) == id )
    {
        return INVALID_SIGNAL_STRENGTH;
    }
    else
    {
        sprintf(node_id, "%d", id);
        strcat(j_prefix, node_id);
        
        chk=open_command(j_prefix);
                                                                                                                             
        if( fgets( buffer, 128, chk ) != NULL )
        {
            // more robustness needed here if the returned is an arbitrary string!!
            i=atoi( (strstr(buffer, "=")+2) );
                                                                                                                             
            if ((i<1)||(i>120))
            {
                fprintf(stderr, "JBoxMlab::get_quality(). Invalid JBox output.\n");
                return INVALID_SIGNAL_STRENGTH;
            }
        }
        else
        {
            fprintf(stderr, "JBoxMlab::get_quality(). No valid response from JBox.\n");
            return INVALID_SIGNAL_STRENGTH;
        }
                                                                                                                             
        fclose(chk);
                                                                                                                             
        return i;
    }
}

void JBoxMlab:: get_pos(int* pos_array, int id)
{
    int i;
    char node_id[4];
                                                                                                                             
    str_man(11);
                                                                                                                             
    strcat(j_prefix, ".1.2.");
    
    sprintf(node_id, "%d", id);
    strcat(j_prefix, node_id);
        
    chk=open_command(j_prefix);
                                                                                                                             
    if( fgets( buffer, 128, chk ) != NULL )
    {
        i=simple_parser1(pos_array, strstr(buffer, "="), "= \"\n", 7);
    }
    else
    {
        fprintf(stderr, "JBoxMlab::get_pos(). No valid response from JBox.\n");
        memset(pos_array, 0x0, sizeof(int)*7);
    }

    fclose(chk);
}

void JBoxMlab::get_gps(double* gps_array, int id)
{
    int i;
    char node_id[4];
                                                                                                                             
    str_man(11);
                                                                                                                             
    strcat(j_prefix, ".1.3.");
                                                                                                                             
    sprintf(node_id, "%d", id);
    strcat(j_prefix, node_id);
                                                                                                                             
    chk=open_command(j_prefix);
                                                                                                                             
    if( fgets( buffer, 128, chk ) != NULL )
    {
        i=simple_parser2(gps_array, strstr(buffer, "="), "= \"\n", 7);
    }
    else
    {
        fprintf(stderr, "JBoxMlab::get_gps(). No valid response from JBox.\n");
        memset(gps_array, 0x0, sizeof(double)*7);
    }

    fclose(chk);
}

void JBoxMlab::neighbor_list(JBoxDataList_t *jboxDataList)
{
    JBOX_data single_jbox_record;
    int i, j, k;
    int node_array[40];
    int number_of_nodes;
    unsigned int l;

    number_of_nodes = nodes_in_network();
    
    // Copy self. The position won't be 
    single_jbox_record.ID = selfID;
    single_jbox_record.signal_strength = INVALID_SIGNAL_STRENGTH;
    jboxDataList->push_back(single_jbox_record);

    if ( number_of_nodes <= 1 )
    {
	    // No neighbor found
    }
    else
    {
	    str_man(2);    
	
	    chk=open_command(j_prefix);
	    
        i=0;
        
        while( !feof( chk ) )
	    {
	        if( fgets( buffer, 128, chk ) != NULL )
	        {
                j=simple_parser3(node_array, strstr(buffer, ":"), " :\n");
		        if (j==0)
		        {
		            fprintf(stderr, "JBoxMlab::neighbor_list(). JBox output corrupted.\n");
		        }
		        else
		        {
                    i++;
                    for (k=0; k<j; k++)
                    {
		                if ((node_array[k]<1)||(node_array[k]>128))
		                {
			                fprintf(stderr, "JBoxMlab::neighbor_list(). JBox output corrupted.\n");
		                }
		    
		                single_jbox_record.ID = node_array[k];
		                single_jbox_record.signal_strength = INVALID_SIGNAL_STRENGTH;
		                jboxDataList->push_back(single_jbox_record);
                    }
		        }
	        }
	    }
		
	    if (i==0)
        {
            fprintf(stderr, "JBoxMlab::neighbor_list(). No valid response from JBox.\n");
        }
    
        fclose(chk);
    }

    for ( l=0; l<jboxDataList->size(); l++)
    {
        (*jboxDataList)[l].num_hops = get_hops((*jboxDataList)[l].ID );

        if (debug)
        {
            fprintf(
                stderr,
                "JBoxMlab Debug: Node %d. Hop: %d.\n",
                (*jboxDataList)[l].ID,
                (*jboxDataList)[l].num_hops);
        }
    }
}

void JBoxMlab::link_quality(JBoxDataList_t *jboxDataList)
{
    unsigned int i;
    unsigned int data_list_size;
    
    data_list_size = jboxDataList->size();

    for ( i=0; i<data_list_size; i++ )
    {
        (*jboxDataList)[i].signal_strength = get_quality((*jboxDataList)[i].ID );
        
        if (debug)
        {
            fprintf(
                stderr,
                "Node %d. Link Quality (Signal Strength): %d.\n",
                (*jboxDataList)[i].ID,
                (*jboxDataList)[i].signal_strength); 
        }
    }
}

void JBoxMlab::whole_pos(JBoxDataList_t *jboxDataList)
{
    double temp_double_array[7];
    int temp_int_array[7];
    int id;
    unsigned int data_list_size;
    unsigned int i;
                                                                                                                             
    data_list_size = jboxDataList->size();

    for ( i=0; i<data_list_size; i++ )
    {
        id = (*jboxDataList)[i].ID;
        get_pos(temp_int_array, id);
        get_gps(temp_double_array, id);

        memcpy((*jboxDataList)[i].pos_reading, temp_int_array, sizeof(int)*7);
        memcpy((*jboxDataList)[i].gps_reading, temp_double_array, sizeof(double)*7);
    }
}

double JBoxMlab::getCurrentTime(void)
{
    timeval tv;
    double curTime, absTime;

    gettimeofday(&tv, NULL);
    absTime = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000);

    curTime = absTime - startTime_;

    return curTime;
}

void JBoxMlab::readerLoop_(void)
{
    JBoxDataList_t localJBoxList;
    double timeStamp;

    while (true)
    {
        // Clear the whole vector before updating
        localJBoxList.clear();

        // Check the neighbor list including the number of hops.
        neighbor_list(&localJBoxList);
        pthread_testcancel();

        // Check the signal strength.
        link_quality(&localJBoxList);
        pthread_testcancel();

        // Check the broadcasted position of the JBoxes.
        whole_pos(&localJBoxList);
        pthread_testcancel();

	// Get the current time stamp.
	timeStamp = getCurrentTime();

        // Copy the JBox list to the shared data.
        pthread_mutex_lock(&jboxDataMutex_);
        shJBoxDataList_ = localJBoxList;
        shJBoxDataTimeStamp_ = timeStamp;
        pthread_mutex_unlock(&jboxDataMutex_);  

         // Reduce CPU load
        usleep(JBOX_UPDATE_THREAD_USLEEP);
        pthread_testcancel();
    }
}

int JBoxMlab::quality_to_neighbor(int id)
{
    JBoxDataList_t localJBoxList;
    double timeStamp;
    int i, rt_sig_str;
    int size;
    
    localJBoxList = getJBoxDataList(&timeStamp);

    size = localJBoxList.size();
    rt_sig_str = -1;	
    
    for (i = 0; i < size; i++)
    {
	    if (localJBoxList[i].ID == id)
	    {
	        rt_sig_str = localJBoxList[i].signal_strength;
	        break;
	    }
    }    
    
    return rt_sig_str;
}

int JBoxMlab::POS_of_node(int* pos_reading_return, int id)
{   
    JBoxDataList_t localJBoxList;
    double timeStamp;
    int i, size;

    localJBoxList = getJBoxDataList(&timeStamp);

    size = localJBoxList.size();
    
    for (i = 0; i < size; i++)
    {
	    if (localJBoxList[i].ID == id)
	    {
            if (pos_reading_return != NULL)
            {
                memcpy(
                    pos_reading_return,
                    localJBoxList[i].pos_reading,
                    sizeof(int)*7);

                return 1;
            }
            else
            {
                fprintf(stderr, " Warning: JBoxMlab::Pos_of_node(). NULL input.\n");
                return -1;
            }

	        break;
	    }
    } 

    return -1;
}

int JBoxMlab::GPS_of_node(double* gps_reading_return, int id)
{
    JBoxDataList_t localJBoxList;
    double timeStamp;
    int i, size;

    localJBoxList = getJBoxDataList(&timeStamp);

    size = localJBoxList.size();
    
    for (i = 0; i < size; i++)
    {
	    if (localJBoxList[i].ID == id)
	    {
            if (gps_reading_return != NULL)
            {
                memcpy(
                    gps_reading_return,
                    localJBoxList[i].gps_reading,
                    sizeof(double)*7);

                return 1;
            }
            else
            {
                fprintf(stderr,"Warning: JBoxMlab::GPS_of_node(). NULL input.\n");
                return -1;
            }

	        break;
	    }
    } 

    return -1;
}

int JBoxMlab::HOPS_of_node( int id )
{
    JBoxDataList_t localJBoxList;
    double timeStamp;
    int i, size;

    localJBoxList = getJBoxDataList(&timeStamp);

    size = localJBoxList.size();
    
    for (i = 0; i < size; i++)
    {
	    if (localJBoxList[i].ID == id)
	    {
            return localJBoxList[i].num_hops;
	    }
    } 

    return -1;
}

void JBoxMlab::Load_BaseInfo(void)
{
    FILE *basein = NULL;
                                                                                                          
    basein = fopen("base_gps.txt","r");
                                                                                                          
    if (!basein)
    {
        fprintf(stderr, "Error: base_gps.txt not found in jbox gps conversion init\n");
        quit_mlab();
    }
    else
    {
                                                                                                          
        float fBase_X, fBase_Y, fBase_Lat, fBase_Long, fMPerLat, fMPerLong;
                                                                                                          
        fscanf(basein, "%f,%f", &fBase_X, &fBase_Y);
        fscanf(basein, "%f,%f", &fBase_Lat, &fBase_Long);
        fscanf(basein, "%f,%f", &fMPerLat, &fMPerLong);
        fclose(basein);
                                                                                                          
        Base_X = (double)fBase_X;
        Base_Y = (double)fBase_Y;
        Base_Lat = (double)fBase_Lat;
        Base_Long = (double)fBase_Long;
        MPerLat = (double)fMPerLat;
        MPerLong = (double)fMPerLong;
        
        fprintf(stderr, "baseX %f, baseY %f, MperLong %f\n",
                Base_X, Base_Y, MPerLong);
    }
}

float JBoxMlab::LongToX(double lng)
{
	return (float)(((lng - Base_Long) *MPerLong) + Base_X);
}

float JBoxMlab::LatToY(double lat)
{
	return (float)(((lat - Base_Lat) * MPerLat) + Base_Y);
}

/*
int JBoxMlab::readerLoop(void)
{   
    JBOX_data *jbd;
    int cnt, listsize;
    T_robot_state* cur;
    char buf[1024];
                                                                                                           
    updateJboxdata();

    listsize = jboxdata_sllist->Get_ListSize();
                                                                                
    for (cnt = 0; cnt < listsize; cnt++)
    {
                                                                                
        jbd = (JBOX_data *)jboxdata_sllist->Delete_Head();

        cur = find_robot_rec(jbd->ID);

        if (cur == NULL)
        {

            //start_robot(NULL, NULL, jbd->ID, 0, 0, LongToX(jbd->gps_reading[0]), LatToY(jbd->gps_reading[1]), "midnight blue");
            start_robot(NULL, NULL, jbd->ID, 0, 0, LongToX(jbd->gps_reading[1]), LatToY(jbd->gps_reading[0]), "midnight blue");
            //printf ("Added robot!\n");
        }
        else
        {
            cur->JBox_Update = true;
            sprintf(
                buf,
                "JBox ID: %d <Lat: %.3f, Lon: %.3f>\nX: %.1f, Y: %.1f\n",
                jbd->ID,
                jbd->gps_reading[0],
                jbd->gps_reading[1],
                LongToX(jbd->gps_reading[1]),
                LatToY(jbd->gps_reading[0]));

            report_user(buf);
            cur->loc.x =LongToX(jbd->gps_reading[1]);
            cur->loc.y = LatToY(jbd->gps_reading[0]);
            cur->heading = jbd->gps_reading[3]+90.0;

            //if (debug)
            {
                fprintf(stderr, buf);
            }
        }
    }
                                                                                
    cur = find_robot_rec(find_first_robot_rec());
                          
    while (cur != NULL)
    {
        if (cur->JBox_Update == true)
        {
            cur->object_ptr->change_color("midnight blue");
        }
        else
        {
            cur->object_ptr->change_color("yellow");
        }
        cur = cur->next;
    }
                                                                                
    draw_robots();
                                                                                
    cur = find_robot_rec(find_first_robot_rec());

    while (cur != NULL)
    {
        cur->JBox_Update = false;
        cur = cur->next;
    }

    // Return false, so that this work process will be kept looping.                                            
    return false;                                                                                               
}
*/

void *JBoxMlab::startReaderThread_(void *jboxInstance)
{
    ((JBoxMlab *)jboxInstance)->readerLoop_();

    return NULL;
}

JBoxDataList_t JBoxMlab::getJBoxDataList(double *timeStamp)
{
    JBoxDataList_t dataListCopy;
    double timeStampCopy;

    pthread_mutex_lock(&jboxDataMutex_);
    dataListCopy = shJBoxDataList_;
    timeStampCopy = shJBoxDataTimeStamp_;
    pthread_mutex_unlock(&jboxDataMutex_);
        
    *timeStamp = timeStampCopy;

    return dataListCopy;
}

/**********************************************************************
 * $Log: jbox_mlab.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/12 06:16:42  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.1  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.5  2004/04/21 04:59:27  endo
 * JBoxMlab improved.
 *
 **********************************************************************/

/*===============================================================================
*                                                                               *
*   Things added on 11/23/03 :                                                  *
*   Code is formatted according to Mobile Robot Lab Style Requirement           *
*===============================================================================*/  

/*===============================================================================
*										                                        *	
*   Things added on 11/16/03 :							                        *	
*   Basic feature of current sensor and how to add to hserver			        *
*										                                        *
*===============================================================================*/

/*===============================================================================
*										                                        *
*   Things added on 11/10/03 :							                        *	
*   Real test with jbox and some robustness issues are solved			        *
*										                                        *
*===============================================================================*/

/*===============================================================================
 *  Things added on 11/05/03 :							                        *
 *  1) popen is moved to a stand alone member function				            *
 *  2) strings are manipulate together						                    *
 *  3) First round robustness test						                        *
 *  4) function names added							                            *
 *										                                        *
================================================================================*/

/*===============================================================================
 *	Things added on 11/04/03 :						                            *
 *	1) popen function to deal with real data retrieve			                *
 *	2) a switch to control the debugging mode or actually running mode	        *
 *	3) further divide the function_prototype				                    *
 *										                                        *
================================================================================*/

/*===============================================================================
* 	Most of the modifications on this version are:				                *
*	1) the particular functions for specific gcc version			            *
*	2) transisiton to object oriented programming				                *
*	The code here is tested over both VC and gcc 2.96 (linux 7.3)		        *
*	Yang Chen		11/03/2003					                                *							
*										                                        *
================================================================================*/

/*===============================================================================
*	Things going to be added:						                            *							
*	Storing updated data on hserver and communicaiton with Robot Executable     *
*	via IPT protocol							                                *	
*	Yang Chen		11/16/2003					                                *
*										                                        *
================================================================================*/

/*===============================================================================
*	 updateJboxdata(void) is integrated into updateLoop(void)		            *
*	 The reason is that we need to clear and update the vector		            *
*	 at the same time.							                                *
*	 The global position info is retrieved by whole_pos()			            *
*	 We now can query a node's position by its node ID			                *
*	 Yang Chen		11/19/2003					                                *
================================================================================*/

