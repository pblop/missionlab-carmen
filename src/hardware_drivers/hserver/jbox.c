/**********************************************************************
 **                                                                  **
 **                               jbox.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by: Yang Chien                                          **
 **                                                                  **
 **  Copyright 2003 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: jbox.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

#include <sys/timeb.h>

#include "HServerTypes.h"
#include "jbox.h"
#include "LogManager.h"
#include "PoseCalculatorInterface.h"

pthread_mutex_t jbox_data_mutex;

// Constants
const double Jbox::OFFSET_MLAB_TO_GPS_HEADING = -90;
const int Jbox::JBOX_UPDATE_THREAD_USLEEP = 100000;
const int Jbox::INVALID_SIGNAL_STRENGTH = -1;
const int Jbox::INVALID_NUM_HOPS = -1;
const int Jbox::INVALID_NUM_NODES = -1;
const int Jbox::JBOX_NON_QUERY_SLEEP_SEC = 3;

void* Jbox::JBOX_startMainThread(void* jbox_instance)
{
    ((Jbox*)jbox_instance)->updateLoop();

    return NULL;
}


// In this constructor, the ID of this jbox and corresponding character strings (used to build up command lines)
// are initiated!
Jbox::Jbox(
    Jbox**a,
    int id,
    bool disableNetworkQuery,
    double gps_base_lat,
    double gps_base_lon,
    double gps_base_x,
    double gps_base_y,
    double gps_mperlat,
    double gps_mperlon) :
    Module ((Module**) a, HS_MODULE_NAME_JBOX),
    selfID(id),
    disableQuery(disableNetworkQuery),
    GPS_BASE_LAT(gps_base_lat),
    GPS_BASE_LON(gps_base_lon),
    GPS_BASE_X(gps_base_x),
    GPS_BASE_Y(gps_base_y),
    GPS_M_PER_LAT(gps_mperlat),
    GPS_M_PER_LON(gps_mperlon)
{
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

    // It is crucial to initalize the pthread mutex here since this is a public available variable
    // even after jbox object is destructed, it is still there. So each time a new object of jbox
    // is constructed, we need to re-initialize it.
    pthread_mutex_init(&jbox_data_mutex, NULL);
    
    refreshScreen();

    pthread_create(&updateThread, NULL, &JBOX_startMainThread, (void*) this );
    
    updateStatusBar_();
}

Jbox::~Jbox(void)
{
    pthread_cancel(updateThread);
    pthread_join(updateThread, NULL);
    printTextWindow( "JBox disconnected" );
}

// Similar function to atoi, but here it converts a hexadecimal digit string to an integer value
// If the digit string is "00xxx" (here x is from 0 ~ F), it is equal to "xxx"
int Jbox::axtoi(char *hexStg) 
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
int Jbox::simple_parser1(int* int_arr, const char* str, const char* delimiter, int limit)
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
int Jbox::simple_parser2(double* double_arr, const char* str, const char* delimiter, int limit)
{

    char *token, *cp;
    //char temp1[20];
    //char temp2[20];
    int total_comp_num;                                                                                                                                                            
    total_comp_num=0;

    cp = strdup (str);                // Make writable copy.  

    token = strtok (cp, delimiter);      // token => "words" 
    while (token!=NULL)
    {	
	    if ( (total_comp_num) < limit )
	    {
            //printTextWindow(token);	
	        double_arr[total_comp_num]=atof(token);
            //strcpy(temp1, token);
            //double_arr[total_comp_num]=atof(temp1);
            //sprintf(temp2, "%f\n", double_arr[total_comp_num]);
            //printTextWindow(temp2);
	        
            total_comp_num++;
	    }

        token = strtok (NULL, delimiter);
    }
    return total_comp_num;
}
                                                                                                                                                             
// method to parse hexdecmial digit string, then return the total number of number converted and the numbers stored in the array int_arr
// str is the string needs parsing and delimiter is a string consists of all the delimiters;
// limit is the upper bound of conversion times to prevent unusual case
//int Jbox::simple_parser3(int* int_arr, const char* str, const char* delimiter, int limit)
int Jbox::simple_parser3(int* int_arr, const char* str, const char* delimiter)
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
	        printfTextWindow("JBox Warning: a bad hexdecimal value.");
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
FILE* Jbox::open_command(const char* command)
{
    FILE *fp;

    if( (fp = popen( command, "r" )) == NULL )
    {
        error_report("open_command(). Popen failed.");
    }

    return fp;

}

// The reason for the following function is that different commands correspond to different operation
// number and command lines, thus, those character-strings would be different.

void Jbox::str_man(int op)
{
    // turn the op code into string type;
    sprintf(j_op, "%d", op);
	
    strcpy(j_prefix, bk_prefix);
    strcpy(j_postfix, bk_postfix);

    strcat(j_postfix, j_op);
    // After the following execuation, j_prefix would be the entire command
    strcat(j_prefix, j_postfix);
    
}


void Jbox::error_report(const char* buffer)
{
    if (buffer != NULL)
    {
        printfTextWindow("JBox Error: %s", buffer);
    }

    printfTextWindow("JBox terminated.");
    delete this;
    redrawWindows();
}

int Jbox::nodes_in_network()
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
	        printfTextWindow("JBox Warning: nodes_in_network(). JBox output corrupted.");
            return INVALID_NUM_NODES; 
	    }
    }
    else
    {
	    printfTextWindow("JBox Warning: nodes_in_network(). No valid response from JBox.");
        return INVALID_NUM_NODES;
    }
    
    fclose(chk);

    return i;
}

int Jbox::get_hops( int id )
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
            printfTextWindow("JBox Warning: get_hops(). No valid response from JBox.");
            return INVALID_NUM_HOPS;
        }
        fclose(chk);
        return i;
    }
}

int Jbox::get_quality( int id )
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
                printfTextWindow("JBox Warning: get_quality(). Invalid JBox output.");
                return INVALID_SIGNAL_STRENGTH;
            }
        }
        else
        {
            printfTextWindow("JBox Warning: get_quality(). No valid response from Jbox.");
            return INVALID_SIGNAL_STRENGTH;
        }
                                                                                                                             
        fclose(chk);
                                                                                                                             
        return i;
    }
}

void Jbox:: get_pos(int* pos_array, int id)
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
        printfTextWindow("JBox Warning: get_pos(). No valid response from Jbox.");
        memset(pos_array, 0x0, sizeof(int)*7);
    }

    fclose(chk);
}

void Jbox::get_gps(double* gps_array, int id)
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
        printfTextWindow("JBox Warning: get_gps(). No valid response from Jbox.");
        memset(gps_array, 0x0, sizeof(double)*7);
    }

    fclose(chk);
}

void Jbox::neighbor_list(JBoxDataList_t *jboxDataList)
{
    JBOX_data single_jbox_record;
    int i, j, k, l;
    int node_array[40];
    int number_of_nodes;

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
		            printfTextWindow("JBox Warning: neighbor_list(). JBox output corrupted.");
		        }
		        else
		        {
                    i++;
                    for (k=0; k<j; k++)
                    {
		                if ((node_array[k]<1)||(node_array[k]>128))
		                {
			                printfTextWindow("JBox Warning: neighbor_list(). JBox output corrupted.");
		                }
		    
		                single_jbox_record.ID = node_array[k];
		                single_jbox_record.signal_strength = INVALID_SIGNAL_STRENGTH;
                        single_jbox_record.neighbor_tag = JBOX_NEIGHBOR_DIRECT;
		                jboxDataList->push_back(single_jbox_record);
                    }
		        }
	        }
	    }
		
	    if (i==0)
        {
            printfTextWindow("JBox Warning: neighbor_list(). No valid response from Jbox.");
        }
    
        fclose(chk);
    }

    if ( (int)(jboxDataList->size()) < number_of_nodes )
    {
        str_man(11);
        strcat(j_prefix, ".1.1");
        chk=open_command(j_prefix);
        i=0;
        while( !feof( chk ) )
        {
            if( fgets( buffer, 128, chk ) != NULL )
            {
                i=atoi( (strstr(buffer, "=")+2) );
                k=0;
                for (j = 0; j < (int)(jboxDataList->size()); j++)
                {
                    if ( (*jboxDataList)[j].ID == i )
                    {   
                        k=1;
                        break;
                    }
                }
                if ( !k )
                {
                    single_jbox_record.ID = i;
                    single_jbox_record.signal_strength = INVALID_SIGNAL_STRENGTH;
                    single_jbox_record.neighbor_tag = JBOX_NEIGHBOR_INDIRECT;
                    jboxDataList->push_back(single_jbox_record);
                }
            }
        }
        fclose(chk);        
    }

    for ( l = 0;  l < (int)(jboxDataList->size()); l++ )
    {
        (*jboxDataList)[l].num_hops = get_hops((*jboxDataList)[l].ID );

        if ( report_level >= HS_REP_LEV_DEBUG )
        {
            printfTextWindow(
                "JBox Debug: Node %d. Hop: %d.",
                (*jboxDataList)[l].ID,
                (*jboxDataList)[l].num_hops);
        }
    }
}

void Jbox::link_quality(JBoxDataList_t *jboxDataList)
{
    unsigned int i;
    unsigned int data_list_size;
    
    data_list_size = jboxDataList->size();

    for ( i=0; i<data_list_size; i++ )
    {
        switch ( (*jboxDataList)[i].neighbor_tag ) {

        case JBOX_NEIGHBOR_DIRECT:
            (*jboxDataList)[i].signal_strength = get_quality((*jboxDataList)[i].ID );
            break;

        case JBOX_NEIGHBOR_INDIRECT:
        default:
            (*jboxDataList)[i].signal_strength = INVALID_SIGNAL_STRENGTH;
            break;
        }

        if ( report_level >= HS_REP_LEV_DEBUG )
        {
            printfTextWindow(
                "Node %d. Link Quality (Signal Strength): %d.",
                (*jboxDataList)[i].ID,
                (*jboxDataList)[i].signal_strength); 
        }
    }
}

void Jbox::gps_update()
{
    struct timeb tp;
    HSPose_t pose;
    double lat;
    double lon;
    double heading;
    unsigned short millitm = 0;
    char temp[100];

    if (gPoseCalc == NULL)
    {
        return;
    }

    gPoseCalc->getPose(pose);

    lon = ((pose.loc.x - GPS_BASE_X)/GPS_M_PER_LON) + GPS_BASE_LON;
    lat = ((pose.loc.y - GPS_BASE_Y)/GPS_M_PER_LAT) + GPS_BASE_LAT;
    heading = pose.rot.yaw + OFFSET_MLAB_TO_GPS_HEADING;

    if (ftime(&tp) == 0);
    {
        millitm = tp.millitm;
    }

    sprintf(
        temp,
        "\"%f %f %d %d %d %d %d\"",
        lat,
        lon,
        (int)(pose.loc.z),
        (int)heading,
        (int)(pose.rot.pitch),
        (int)(pose.rot.roll),
        millitm);

    strcpy(j_gpsset, bk_gpsset);
    strcat(j_gpsset, temp);
    
    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        printTextWindow(j_gpsset);
    }

    chk=open_command(j_gpsset);
    fclose(chk);
}
    
void Jbox::whole_pos(JBoxDataList_t *jboxDataList)
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

//===============================================================================
void Jbox::updateStatusBar_(void)
{
    JBoxDataList_t localJBoxList;
    int i, numJBoxes;
    char buf[100];
    
    statusStr[0] = 0;
    
    strcat(statusStr, "JBox: ");

    localJBoxList = getJBoxDataList();

    numJBoxes = localJBoxList.size();

    sprintf(buf, "%c ", statusbarSpinner_->getStatus());
    strcat(statusStr, buf);

    if (disableQuery)
    {
        strcat(statusStr, "[network query disabled]");
    }
    else
    {
        if (numJBoxes > 0)
        {
            strcat(statusStr, "ID:");

            for (i = 0; i < numJBoxes; i++)
            {	
                sprintf(buf, " %d", localJBoxList[i].ID);
                strcat(statusStr, buf);
            }
        }
        else
        {
            strcat(statusStr, " [connecting]");
        }

    }

    statusbar->update(statusLine);

}

void Jbox::updateLoop(void)
{
    JBoxDataList_t localJBoxList;
    
    if ( report_level >= HS_REP_LEV_DEBUG )
    {
        printfTextWindow("JBox Debug: updateLoop().");
    }
    
    while (true)
    {
        // Make sure termination is not started
        pthread_testcancel();
        
        // Reduce CPU load
        usleep(JBOX_UPDATE_THREAD_USLEEP);
        pthread_testcancel();
        
        // Update GPS value.
        gps_update();
        pthread_testcancel();

        // Clear the whole vector before updating
        localJBoxList.clear();

        if (!disableQuery)
        {
            // Check the neighbor list including the number of hops.
            neighbor_list(&localJBoxList);
            pthread_testcancel();

            // Check the signal strength.
            link_quality(&localJBoxList);
            pthread_testcancel();

            // Check the broadcasted position of the JBoxes.
            whole_pos(&localJBoxList);
            pthread_testcancel();
        }
        else
        {
            sleep(JBOX_NON_QUERY_SLEEP_SEC);
        }

        // Copy the JBox list to the shared data.
        pthread_mutex_lock(&jbox_data_mutex);
        shJBoxDataList = localJBoxList;
        pthread_mutex_unlock(&jbox_data_mutex);  

        // Log the data if necessary.
        if (gLogManager != NULL)
        {
            manageLog_(updateThread);
        }

        // Update the status bar display
        updateStatusBar_();
    }	
}

int Jbox::quality_to_neighbor(int id)
{
    JBoxDataList_t localJBoxList;
    int i, rt_sig_str;
    int size;
    
    localJBoxList = getJBoxDataList();

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

int Jbox::POS_of_node(int* pos_reading_return, int id)
{   
    JBoxDataList_t localJBoxList;
    int i, size;

    localJBoxList = getJBoxDataList();

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
                printfTextWindow("JBox Warning: NULL input to POS_of_node().");
                return -1;
            }

	        break;
	    }
    } 

    return -1;
}

int Jbox::GPS_of_node(double* gps_reading_return, int id)
{
    JBoxDataList_t localJBoxList;
    int i, size;

    localJBoxList = getJBoxDataList();

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
                printfTextWindow("JBox Warning: NULL input to GPS_of_node().");
                return -1;
            }

	        break;
	    }
    } 

    return -1;
}

int Jbox::HOPS_of_node( int id )
{
    JBoxDataList_t localJBoxList;
    int i, size;

    localJBoxList = getJBoxDataList();

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

JBoxDataList_t Jbox::getJBoxDataList(void)
{
    JBoxDataList_t dataListCopy;

    pthread_mutex_lock(&jbox_data_mutex);
    dataListCopy = shJBoxDataList;
    pthread_mutex_unlock(&jbox_data_mutex);
        
    return dataListCopy;
}

void Jbox::connectToJbox( Jbox*& jbox )
{
    //unsigned const int ECHO_LINE = 4;
    string jboxIDStr;

    printfTextWindow("\n");
    printfTextWindow("JBox can only be started through the command line option.\n");
    printfTextWindow("Restart hserver with '-J <jbox_section in .hserverrc>'.\n");

    return;
}

void Jbox::connectToJbox(
    Jbox*& jbox,
    int jboxID,
    bool disableNetworkQuery,
    double gps_base_lat,
    double gps_base_lon,
    double gps_base_x,
    double gps_base_y,
    double gps_mperlat,
    double gps_mperlon)

{
    new Jbox(
        &jbox,
        jboxID,
        disableNetworkQuery,
        gps_base_lat,
        gps_base_lon,
        gps_base_x,
        gps_base_y,
        gps_mperlat,
        gps_mperlon);
}

void Jbox::logData_(double logTime)
{
    JBoxDataList_t jboxDataList;
    int i, numJBoxes;

    if (logfile_ != NULL)
    {
        jboxDataList = getJBoxDataList();
        numJBoxes = jboxDataList.size();

        fprintf(logfile_, "%f %d", logTime, numJBoxes);

        for (i = 0; i < numJBoxes; i++)
        {
            fprintf(
                logfile_,
                " %d %d %.2f %f %f %f %f %f %f %d %d",
                jboxDataList[i].ID,
                jboxDataList[i].signal_strength,
                jboxDataList[i].gps_reading[0],
                jboxDataList[i].gps_reading[1],
                jboxDataList[i].gps_reading[2],
                jboxDataList[i].gps_reading[3],
                jboxDataList[i].gps_reading[4],
                jboxDataList[i].gps_reading[5],
                jboxDataList[i].gps_reading[6],
                jboxDataList[i].num_hops,
                jboxDataList[i].neighbor_tag);
        }

        fprintf(logfile_, "\n");
        fflush(logfile_);
    }
}

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
* 1) Remove the part using file feed-in for debugging
* 2) adding the node itself in the display as well as in the vector
* things need to be taken care of : use -J (id) instead of a fixed/embedded ID
* 3) the GPS updating problem (incorrect format)  is solved, the messageDrawWindow()
* is modified, a double* dmsgData is added to receive the double array data correctly.
  
* things need to be taken care of 
* 1) GPS info sent to Jbox;  done, but 
    a) one of the jbox became abnormal-functioned, after reboot the jbox, the problem is gone.
    b) the hserver ran into seg fault several times and then became normal. (?)  
* 2) use -J (id) instead of a fixed/embedded ID
* 3) the exact updating interval;
* Yang Chen 04/06/2004
===============================================================================*/

/*===============================================================================
* 1) simple_parser3() as well as neighbor_list() are modified
* The reason is that the assumption of the feedback format for neighbor list
* query is wrong.
* Yang Chen 04/05/2004
===============================================================================*/

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

/**********************************************************************
# $Log: jbox.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:56  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:52  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
