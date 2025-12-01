/**********************************************************************
 **                                                                  **
 **                     robot_side_com.c                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: robot_side_com.c,v 1.1.1.1 2006/07/12 13:37:56 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <vector>
#include <string>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <math.h>
#include <malloc.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <time.h>

#include "ipt/ipt.h"
#include "ipt/message.h"
#include "ipt/connection.h"
#include "sensor_blackboard.h"
#include "gt_simulation.h"
#include "gt_ipt_msg_def.h"
#include "hclient.h"
#include "hserver/hserver.h"
#include "robot_side_com.h"
#include "FSA_status.h"
#include "lineutils.h"
#include "laser/laser_protocol.h"
#include "lmadjust.h"
#include "robot_cnp.h"
#include "cnl.h"
#include "CommBehavior.h"
#include "hserver/ipc_client.h"
#include "hserver/hserver_ipt.h"

using std::cerr;
using std::string;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string EMPTY_STRING = "";

//-----------------------------------------------------------------------
// DCM 03-29-00: Change to provide a consistent cycle execution time.
// This constant sets the duration of an execution cycle microseconds
// of wall clock time.  The value 100,000 microseconds is 1/10 second,
// allowing the robots to execute the control loop 10 times per second.
//-----------------------------------------------------------------------
long g_lCycleLength = 100000; // (in uSec) 1/10th seconds per cycle (i.e. 10Hz).
                             // This should never be set to 0 because it's used in division.
                             // If you don't want to wait, set it to 1.
long g_lOldCycleLength = g_lCycleLength; // jbl-visual

//-----------------------------------------------------------------------
// This variable is used to check the slowness of the communication
// between console and robot executable. See exec_ping_console()
// below.
//-----------------------------------------------------------------------
const double IPT_TIMEOUT = 100; // in millisecond

#define ZERO_TIME 0.05
#define IGNORE_SIGNALS

#define BUFFER 10
#define NUM_ESCLAVOS 10

//-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------
IPCommunicator* communicator;
IPConnection* console;
IPC_CONTEXT_PTR centralServerConnection = NULL;

// robot_name::robot_id is guaranteed to be a unique string
int robot_id = 1; // the id number of this robot
char robot_name[256]; // the name of this robot executable
int debug = false;
int report_current_state = false;
int save_learning_momentum_weights = false;
int save_sensor_readings = false;
int log_robot_data = false;
int current_state[FSA_MAX];  // tells what state (in what FSA) currently in.
int previous_state[FSA_MAX]; // tells what state (in what FSA) previoulsy in.
bool in_new_state[FSA_MAX];  // tells whether the robot (w.r.t. FSA) is in actual new state.
int robot_type = HOLONOMIC;
int run_type = SIMULATION;
int old_run_type = run_type;
int paused = false;
bool stopRobotFeedback = false;
char hserver_name[256] = "fred";
float danger_range = 0.5;  // For display purposes only
float drive_wait_angle = 15.0;
char* laserfit_machine_name = "columbus";
char *carmen_navigator_machine_name = NULL;

// DCM 01-12-00: Holds the latest copy of the sensor readings.
T_sensor_blackboard sensor_blackboard;

// when paused, robot steps one cycle when single_step_cycle is incremented
// It is automatically zeroed when unpaused.
int single_step_cycle = 0;
int current_step_cycle = 0;

// A global cycle count that is visible to the robot threads.
// It is incremented each thread execution cycle.
unsigned long current_cycle = 0;

char* halting = NULL;
int draw_obstacles = true;
int lurch_mode = false;
int use_reverse = false;
int wait_for_turn = true;
bool show_cycle_duration = false; // Do you want to see the actual cycle times?
bool marked_door_once = false;
bool unmarked_door_once = false;
int adjust_obstacles = false;

// Which sensors to use for obstacle readings and display
bool use_sonar = true;
bool use_laser = true;
bool use_cognachrome = true;
bool use_generic = false;
bool use_roomba = true;

bool multiple_hservers = false;
bool drawing_vector_field = false;
bool bPingHistory[100];
static vector<string> FSAs;
gt_Command* command = NULL;

// CNP related variables
bool enableRuntimeCNP = false;
RobotCNP *gRobotCNP = NULL;
const string STRING_CNP_TRACKRED = "TRACKRED";
const string STRING_CNP_TRACKYELLOW = "TRACKYELLOW";
const string CNP_NOTIFY_TASK_COMPLETED_MSG_PREFIX = "CNP_TASK_COMPLETED::";
const string CNP_NOTIFY_TASK_RENEGED_MSG_PREFIX = "CNP_TASK_RENEGED::";
const string CNP_NOTIFY_TASK_AUCTION_FAILED_MSG = "CNP_TASK_AUCTION_FAILED";

// Communication sensitive behavior related variables.
CommBehavior *csbInstance = NULL;
SensorDataUMWrap *csbSensorDataInstance = NULL;
bool csbInitialized = false;
const bool USE_CSB_NETWORK_MODEL = false;

// The number of robots that this robot is expected to be working
// with (including this robot itself).
int numExpectedRobots = 1;
vector<int> allowedRobotJBoxIDs;

//Variables anadidas parar realizar la coordinación entre varios robots.
IPConnection * robotLider = NULL;
IPConnection *robotsEsclavos[NUM_ESCLAVOS];
int numEsclavos = 0;
bool escuchoBroadCast = false;
robot_position posicionLider;
bool cambioEstado = false;
Vector posicionEsclavos;
Vector posicionIr;
int primeraSigoLider = 0;
int inicialNuevaTarea = 0;
Vector posicionAntLider;
int inicialEntrarHabitacion = 0;
Vector objetivoEntrarHabitacion;

float signalLevelAnt = 100.0;
float alfaFiltroWiFi = 0.8;

Point carmenAdvise;

//-----------------------------------------------------------------------
// exec_log_robot_data
//
//   * Robot Data Logging routine (saves robot data in <robotname>.log .)
//
//  Inputs:
//     v         = the velocity of the robot
//     rp        = position of the robot in the world coordinate.
//     rid       = robot ID number
//     state_num = what state the robot is in. (tells only for one FSA)
//
//-----------------------------------------------------------------------
void exec_log_robot_data(Vector v, robot_position rp, int rid, int state_num)
{
    REC_ReportedLogfile rec;
    timeval tvp;
    time_t currentTime;
    tm localTime;
    double absTime, runTime;
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];
    char *username = NULL;
    char *fsaList = NULL, *value;
    char *local = NULL, *start = NULL, *end = NULL;
    int i;
    static FILE *logfile = NULL;
    static char logfileName[1024];
    static double startLogTime = 0;
    static bool timeIsZero = true;
    static bool fileIsOpen = false;

    // make sure it is not in the halting process.
    if (halting)
    {
        if (fileIsOpen)
        {
            fclose(logfile);
        }
        return;
    }

    // Get the current time.
    gettimeofday(&tvp, NULL);
    absTime = (double)(tvp.tv_sec) + (((double)(tvp.tv_usec))/1000000.0);

    // Do this only the first time.
    if (timeIsZero)
    {
        username = getenv("USER");
        currentTime = time(NULL);
        strncpy(date, ctime(&currentTime), sizeof(date));
        localTime = *localtime(&currentTime);
        strftime(month, sizeof(month), "%m", &localTime);
        strftime(day,   sizeof(day), "%d", &localTime);
        strftime(year,  sizeof(year), "%Y", &localTime);
        strftime(hour,  sizeof(hour), "%H", &localTime);
        strftime(min,   sizeof(min), "%M", &localTime);
        strftime(sec,   sizeof(sec), "%S", &localTime);

        sprintf(
            logfileName,
            "%s-%s-%s%s%s-%s%s%s-robot%d.log",
            robot_name,
            username,
            month,
            day,
            year,
            hour,
            min,
            sec,
            robot_id);

        // open the log file
        if ((logfile = fopen(logfileName, "w")) == NULL)
        {
            fprintf(
                stderr,
                "Error: Unable to write to the logfile \"%s\".\n",
                logfileName);

            exit(1);
        }

        fileIsOpen = true;

        // Report the logfile name to console.
        rec.logfileName = strdup(logfileName);
        communicator->SendMessage(console, MSG_ReportedLogfile, &rec);
        free(rec.logfileName);

        // Build a list of the FSAs in this configuration
        fsaList = get_state("FSA_LIST");

        if(fsaList != NULL)
        {
            local = strdup(fsaList);
            start = local;

            while(*start)
            {
                // find separator blank
                end = start + 1;

                while(*end && *end != ' ')
                {
                    end++;
                }

                // replace blank with null
                *end = '\0';

                // add it to our list
                FSAs.push_back(string(start));

                // skip past to the next one, or the trailing null.
                start = end + 1;
            }

            free(local);
        }
        else
        {
            fprintf(stderr, "Error: FSA list not created.\n");
        }

        // Create the header.
        fprintf(logfile, "\n                  ");
        fprintf(logfile, "MissionLab Robot Executable Status Data\n\n");
        fprintf(
            logfile,
            "---------------------------------------------------------------------------------\n");

        // Dump the robot ID
        fprintf(logfile, "Robot ID: %d\n", robot_id);
        fprintf(
            logfile,
            "---------------------------------------------------------------------------------\n");

        // Dump the FSA Name-to-number conversion info.
        fprintf(logfile, "FSA ([Name:Number]): ");
        for(i = 0; i < (int)(FSAs.size()); i++)
        {
            fprintf(logfile, "[%s:%d]", FSAs[i].c_str(), i);
        }
        fprintf(logfile, "\n");
        fprintf(
            logfile,
            "---------------------------------------------------------------------------------\n");

        // Record the start time.
        startLogTime = absTime;
        fprintf(logfile, "Starting Time: %f\n", startLogTime);
        fprintf(
            logfile,
            "---------------------------------------------------------------------------------\n");
        fprintf(
            logfile,
            "      Time Position-X Position-Y    Heading Velocity-X Velocity-Y  RobotID (FSA#,State#)\n");
        fprintf(
            logfile,
            "---------------------------------------------------------------------------------\n");

        timeIsZero = false;
    }

    runTime = absTime - startLogTime;

    fprintf(
        logfile,
        "%10.3f %10.3f %10.3f %10.3f %10.3f %10.3f %8d ",
        runTime,
        rp.v.x,
        rp.v.y,
        rp.heading,
        v.x,
        v.y,
        rid);

    // Log state information for each of the FSAs.
    // The log format is: (FSA#,State#)
    // Where    FSA#   - The FSA number [0-n] that maps into the list of FSAs dumped in the header
    //          State# - The State number that the FSA was in this cycle.  A value of 999999 flags an error
    for(i = 0; i < (int)(FSAs.size()); i++)
    {
        // The value is formated as "A STATE" so we can just dump the string.
        value = get_state(FSAs[i].c_str());

        // If it is valid and active, go ahead and dump it
        if(value && value[0] == '1' && value[1] && value[2])
        {
            fprintf(logfile, "(%d,%s)", i, &value[2]);
        }
    }

    fprintf(logfile, "\n");
    fflush(logfile);
}

void LogSensorReadings(void)
{
    const char* szLOG_HEADER =
        "\n"
        "                  MissionLab Sensor Reading Data\n"
        "\n"
        "--------------------------------------------"
        "------------------------------------------\n"
        " Entry      Time   Position-X  Position-Y  Theta  Readings...\n"
        "--------------------------------------------"
        "------------------------------------------\n";

    // get the running time.
    static bool bTimeZero = true;
    timeval suCurrentTime;
    gettimeofday(&suCurrentTime, NULL);
    double dAbsTime = (double) suCurrentTime.tv_sec +
        (((double) suCurrentTime.tv_usec) / 1000000);

    static double dStartLogTime;
    static FILE* pfLogFile = NULL;
    static int iEntry;

    // make sure it is not in the halting process.
    if (halting)
    {
        if (pfLogFile != NULL)
        {
            fclose(pfLogFile);
	        pfLogFile = NULL;
        }
        return;
    }

    // open the log file
    if (pfLogFile == NULL)
    {
        // name the log file
        char szLogfileName[300];
        if (pfLogFile == NULL)
        {
            sprintf(szLogfileName, "%s%011ld.sensor-log",
                     robot_name, suCurrentTime.tv_sec);
        }

        if ((pfLogFile = fopen(szLogfileName, "w")) == NULL)
        {
            printf("Unable to write in %s!\n", szLogfileName);
            exit(1);
        }

        iEntry = 1;
    }

    // put a heading in the logfile if it is the first time.
    if (bTimeZero)
    {
        dStartLogTime = dAbsTime;

        fprintf(pfLogFile, szLOG_HEADER);

        bTimeZero = false;
    }

    double dRunTime = dAbsTime - dStartLogTime;

    // record the data in the file
    obs_reading reading;
    fprintf(pfLogFile, "%5d %10.3f %10.3f %10.3f %10.3f\n", iEntry, dRunTime,
             sensor_blackboard.x, sensor_blackboard.y, sensor_blackboard.theta);
    for (unsigned int i = 0; i < sensor_blackboard.sensed_objects.size(); i++)
    {
        reading = sensor_blackboard.sensed_objects[i];
        fprintf(pfLogFile, "    %10.3f %10.3f %10.3f\n",
                 reading.center.x, reading.center.y, reading.r );
    }
    fprintf(pfLogFile, "\n");
    iEntry++;

    fflush(pfLogFile);
}

/********* Debug message with timestamp *************/
void print_debug_msg(char* msg)
{
    static long start_time = 0;
    timeval tvp;

    if (!start_time)
    {
        gettimeofday(&tvp, NULL);
        start_time = tvp.tv_sec;
    }
    gettimeofday(&tvp, NULL);
    printf("%ld.%ld sec, %s\n", tvp.tv_sec-start_time, tvp.tv_usec, msg);
}

/****************************************************************************/

#ifndef IGNORE_SIGNALS

void sig_catch(int sig)
{
    char   msg[80];
    static had_error = false;

    if (had_error)
    {
        fprintf(stderr, "robot %d: Exiting with double fault\n", robot_id);
        abort();
        exit(1);
    }
    had_error = true;

    sprintf(msg, "caught signal %d", sig);
    fprintf(stderr, "robot %d: %s\n", robot_id, msg);

    halt(msg);
}

#endif

/******************* state database functions *****************/


const int SymbolTableSize = 1024;   // Size for symbol table (Closed hashing)
struct SymbolTableEntry
{
    char* key;
    char* value;
    int   val_buf_len;
};

static SymbolTableEntry SymbolTable[SymbolTableSize];


/************************************************************************
*                                                                       *
*                         int Hash(char*)                              *
*                                                                       *
************************************************************************/

/*
 * This routine will return an integer index into the SymbolTable. The
 * location that is returned must be then checked.  If the name is NULL then
 * the symbol is not in the table and should be inserted in this position.  If
 * this is the symbol then it was found at this loc.  Return of -1 means error.
 */

static int Hash(const char* str)
{
    int val;
    int loc;
    int start;

    val = 0;
    loc = 0;
    while (str[loc] != 0)
    {
        val += str[loc++];
    }

    loc = (val * 3) % SymbolTableSize;
    start = loc;

    while (SymbolTable[loc].key != NULL && (strcmp(SymbolTable[loc].key, str) != 0))
    {
        loc = (loc + 1) % SymbolTableSize;
        if (loc == start)
        {
            // Have gone through entire table and all locations are full!
            fprintf(stderr, "Error: The robot communication hash symbol table is full. \n"
                    "SymbolTableSize must be increased in the file robot_side_com.c\n");
            return -1;
        }
    }

    return loc;
}

/****************************************************************************
*                                                                           *
* int put_state(char* key, char* value);                                     *
*                                                                           *
* key:                                                                      *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* value:                                                                    *
*    Any text string.                                                       *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * add the parameter to the symbol table if it isn't already there.
 * If it is found then update its value.
 */

int put_state(char* key, char* value)
{
    int               loc;
    SymbolTableEntry* rec;

    if (debug)
    {
        fprintf(stderr, "robot(%d) put_state: '%s'='%s'\n", robot_id, key, value);
    }

    if ((loc = Hash(key)) < 0)
    {
        fprintf(stderr, "gt_update: Unable to hash parameter name '%s'\n", key);
        return -1;
    }

    rec = &SymbolTable[loc];

    if (rec->key == NULL)
    {
        // Not in table so add string
        if ((rec->key = (char*)calloc(strlen(key)+1, 1)) == NULL)
        {
            fprintf(stderr, "put_state: Unable to allocate memory for key '%s'\n", key);
            return -1;
        }

        // copy the name over
        strcpy(rec->key, key);
        rec->value = NULL;
    }

    if (rec->value != NULL &&
        (int)strlen(value) >= rec->val_buf_len)
    {
        // too small.  get rid of it
        free(rec->value);
        rec->value = NULL;
    }

    if (rec->value == NULL)
    {
        // alloc space for value
        rec->val_buf_len = strlen(value) + 1;
        if ((rec->value = (char*) malloc(rec->val_buf_len)) == NULL)
        {
            fprintf(stderr, "put_state: Unable to allocate memory for value '%s'\n", value);
            return -1;
        }
    }

    // remember the new value
    strcpy(rec->value, value);

    return 0;
}

/****************************************************************************
*                                                                           *
* int put_state_vector(char* context, char* name, Vector value);            *
*                                                                           *
* key: is formed via "context:name"                                         *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* value:                                                                    *
*    A vector.                                                              *
*                                                                           *
* Returns:                                                                  *
*    0: success                                                             *
*   -1: failure                                                             *
*                                                                           *
****************************************************************************/
/*
 * add the parameter to the symbol table if it isn't already there.
 * If it is found then update its value.
 */

int put_state_vector(char* context, char* name, Vector value)
{
    char var_name[256], var_val[256];

    sprintf(var_name, "%s:%s", context, name);
    sprintf(var_val, "%f, %f", value.x, value.y);
    return put_state(var_name, var_val);
}

int put_state_double(char* context, char* name, double value)
{
    char var_name[256], var_val[256];

    sprintf(var_name, "%s:%s", context, name);
    sprintf(var_val, "%f", value);
    return put_state(var_name, var_val);
}

/****************************************************************************
*                                                                           *
* char* get_state(char* key);                                               *
*                                                                           *
* key:                                                                      *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    NULL: failure                                                          *
*    else: pointer to the string value                                      *
*                                                                           *
****************************************************************************/
char* get_state(const char* key)
{
    int   loc;
    char* rtn = NULL;

    if ((loc = Hash(key)) < 0)
    {
        fprintf(stderr, "get_state: Unable to hash key '%s'\n", key);
        rtn = NULL;
    }
    else
    {
        rtn = SymbolTable[loc].value;
    }

    if (rtn == NULL)
    {
        rtn = "";
    }

    if (debug)
    {
        fprintf(stderr, "robot(%d) get_state: '%s'='%s'\n", robot_id, key, rtn);
    }

    return rtn;
}


/****************************************************************************
*                                                                           *
* char* get_state(char* context, char* name, Vector* retval);               *
*                                                                           *
* key: is formed via "context:name"                                         *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    1: success                                                             *
*    0: failure                                                             *
*                                                                           *
****************************************************************************/

int get_state_vector(char* context, char* name, Vector* retval)
{
    char  var_name[256];
    char  var_val[256];
    char* s;
    char* tok;

    sprintf(var_name, "%s:%s", context, name);

    if ((s = get_state(var_name)))
    {
        strcpy(var_val, s);
        if (!(tok = strtok(var_val, ", ")))
        {
            return 0;
        }
        retval->x = atof(tok);
        if (!(tok = strtok(NULL, ", ")))
        {
            return 0;
        }
        retval->y = atof(tok);
    }

    return 1;
}

int get_state_double(char* context, char* name, double* retval)
{
    char var_name[256];
    char* s;
    int stat = 0;

    sprintf(var_name, "%s:%s", context, name);
    if ((s = get_state(var_name)))
    {
        *retval = atof(s);
        stat = 1;
    }

    return stat;
}

/****************************************************************************
*                                                                           *
* char* return_state(char* key);                                            *
*                                                                           *
* key:                                                                      *
*    Any text string parameter name.  It is case and white space sensitive. *
*                                                                           *
* Returns:                                                                  *
*    NULL: failure                                                          *
*    else: pointer to allocated string value: User must free it.            *
*                                                                           *
* Like get_state only it deletes the entry from the database after reading  *
*                                                                           *
****************************************************************************/

char* return_state(char* key)
{
    int   loc;
    char* rtn = NULL;

    if ((loc = Hash(key)) < 0)
    {
        fprintf(stderr, "get_state: Unable to hash key '%s'\n", key);
        rtn = NULL;
    }
    else if (SymbolTable[loc].value != NULL)
    {
        rtn = SymbolTable[loc].value;
        SymbolTable[loc].value = NULL; // delete it
    }

    if (debug && (rtn != NULL))
    {
        fprintf(stderr, "robot(%d) return_state: '%s'='%s'\n", robot_id, key, rtn);
    }

    return rtn;
}

/****************************************************************************
* exec_ping_console                                                         *
*                                                                           *
* This function checks how long it takes to send and received the message   *
* between the console and robot executable.                                 *
*                                                                           *
****************************************************************************/
int exec_ping_console(double* ping_time)
{
    /*timeval tvp;
    double response_time = 0;
    double time_out = IPT_TIMEOUT / 1000.0;

    *ping_time = 1e10;

    REC_PingSend ping_send;
    ping_send.check = 1000*rand();

    // Get the time before pinging
    gettimeofday(&tvp, NULL);
    double start_time = (double) tvp.tv_sec + (((double) tvp.tv_usec) / 1000000);

    // Ping to console
    IPMessage* msg = communicator->Query(console, MSG_PingSend, &ping_send, MSG_PingBack, time_out);
    if (msg == NULL)
    {
        return false;
    }

    REC_PingBack* ping_back = (REC_PingBack*) msg->FormattedData();

    // Get the time after pinging
    gettimeofday(&tvp, NULL);
    double current_time = (double)tvp.tv_sec + (((double)tvp.tv_usec)/1000000.0);
    response_time = current_time - start_time;

    // Check to see if the message was actually returned properly.
    if (ping_back->check != ping_send.check)
    {
		delete msg;
        return false;
    }

    *ping_time = response_time * 1000; // ping_time is in millisecond


    // Se ha aumentado el IPT_TIMEOUT A 1000 (antes estaba en 100) con el fin de
    // evitar posibles errores por retrasos en la contestacion.
    if (*ping_time >= IPT_TIMEOUT)
    {
        *ping_time = 1e10;
	//Added by teny
	printf("Ping time above timeout\n");
		delete msg;
        return false;
    }
    delete msg;
    */


    return true;
}

/****************************************************************************
* network_to_console_too_slow                                               *
*                                                                           *
* This is a wrapper for exec_ping_console in order to check to see if the   *
* network to cosole is too slow or not.                                     *
*                                                                           *
****************************************************************************/
bool network_to_console_too_slow(bool ignore_simulation, const char *message)
{
    double ping_time;
    string message_string = "data";

    if (ignore_simulation && (run_type == SIMULATION))
    {
        // We don't need to worry about SIMULATION mode.
        return false;
    }

    // Si existen problemas de rotura de la tuberia,
    // se puede comentar esta parte para evitarlos. Estos problemas son debidos al retraso en la
    // contestacion al ping

    if (exec_ping_console(&ping_time))
    {
         //The ping arrived on time.
        return false;
    }

    // We have a network problem.
    if (debug)
    {
        if (message != NULL)
        {
            message_string = message;
        }

        fprintf(
            stderr,
            "Warning: Network too slow (%.2f ms). %s not sent to console.\n",
            ping_time,
            message);
    }

    return true;
}

// send a message to the console that a step has been taken
void SendStepTaken(void)
{
    static int iStep = 0;
    REC_StepTaken suStepTaken;
    suStepTaken.iRobotID = robot_id;
    suStepTaken.iStep = iStep;


    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_StepTaken"))
            {
                communicator->SendMessage(console, MSG_StepTaken, &suStepTaken);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_StepTaken, &suStepTaken);
    }

    iStep++;
}

void exec_get_battery_info(void)
{
    REC_BatteryInfo batt;
    int level = -1;

    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_BatteryInfo"))
            {
                if (hclient_get_battery(&level))
                {
                    batt.level = level;
                    batt.id = robot_id;
                    communicator->SendMessage(console, MSG_BatteryInfo, &batt);
                }
            }
        }
    }
}

/****************************************************************************
* end_of_cycle                                                              *
*                                                                           *
* This marks the end of a robot cycle and is where we check single step and *
* pause 								    *
****************************************************************************/
void end_of_cycle()
{
    int need_restart = false;

    exec_get_battery_info();

    communicator->Idle(ZERO_TIME);

    if (debug)
    {
        fprintf(stderr, "\n---------------------------- End of Cycle ------------------\n\n");
    }

    if (halting)
    {
        halt(halting);
    }

    // send a message to the console that a step has been taken
    SendStepTaken();

    // When did we start this cycle.
    static timeval start_time = {0, 0};
    int usage = 100;

    // when do we want to release?
    timeval end_time = start_time;
    end_time.tv_usec += g_lCycleLength;
    if (end_time.tv_usec >= 1000000)
    {
        end_time.tv_sec  += end_time.tv_usec / 1000000;
        end_time.tv_usec  = end_time.tv_usec % 1000000;
    }

    if (run_type == SIMULATION)
    {
        // What time is it now?
        timeval now_time;
        int rtn;
        if ((rtn = gettimeofday(&now_time, NULL)) != 0)
        {
            fprintf(stderr, "robot_side_com:end_of_cycle() - gettimeofday returned error %d\n", rtn);
        }

        // If we are ahead of schedule, then wait around a bit.
        else if (timercmp(&now_time, &end_time, <))
        {
            // How far away is the release time in microseconds?
            long yet_to_wait = (end_time.tv_sec - now_time.tv_sec) * 1000000;
            yet_to_wait += (long)end_time.tv_usec - (long)now_time.tv_usec;

            // Don't sleep if less than a millisecond.
            if (yet_to_wait > 1000)
            {
                if (show_cycle_duration)
                {
                    // What percentage of the cycle did we use up?
                    usage = (int)(((((double)(g_lCycleLength - yet_to_wait) /
                                     (double)g_lCycleLength)) * 100) + 0.5);
                }

                // Wait for that long before starting the next cycle.
                usleep(yet_to_wait);
            }
        }
    }

    // adjust the parameters for learning momentum
    AdjustLMValues();

    // Increment a global cycle count that is visible to the robot threads.
    current_cycle ++;

    // if requested, pause the robot and wait for the unpause msg
    // or a single step cycle message.
    if (paused && (run_type == REAL))
    {
        // Stop the robot
        hclient_stop();
        hclient_range_stop();
        need_restart = true;
    }

    if (save_sensor_readings)
    {
        LogSensorReadings();
    }

    while (paused)
    {
        // If have received a single step message, then run one cycle
        if (single_step_cycle > current_step_cycle)
        {
            current_step_cycle++;
            break;
        }

        // run message handlers
        communicator->Idle(ZERO_TIME);
        //tcxRecvData(NULL, NULL, NULL, NULL, NULL, ALL_HND, &timeout);

    }

    if (need_restart && (run_type == REAL))
    {
        hclient_range_start();
    }

    if (show_cycle_duration)
    {
        gettimeofday(&end_time, NULL);
        int secs = end_time.tv_sec - start_time.tv_sec;
        int usecs = (long) end_time.tv_usec - (long) start_time.tv_usec;
        if (usecs < 0)
        {
            usecs += 1000000;
            secs  -= 1;
        }

        // Throw out the errors from pausing and the first time.
        if (secs < 100)
        {
            fprintf(stderr, "\rCycle length:%2d.%6.6d sec. Load:%3d%%", secs, usecs, usage);
        }
    }

    // Remember when we started this cycle.
    gettimeofday(&start_time, NULL);

    if (robot_type == HSERVER)
    {
        hclient_update_sensors();
    }
}

/****************************************************************************
* exec_setxy set the robot's x and y position. Theta is left unchanged      *
*                                                                           *
****************************************************************************/
void exec_setxy(double x, double y)
{
    double xx, yy;
    double tt = 0;
    hclient_getxyt(&xx, &yy, &tt);
    hclient_setxyt(x, y, tt);
}


/****************************************************************************
* exec_setxyTheta set the robot's x and y position, and Theta.              *
* A command is sent to the mlab console to update the position of the       *
* robot on the simulator screen.                                            *
* This function can be very useful for correcting deadreconing errors       *
* or to fix the nasty bug of always starting the robot at the start point   *
* defined in the overlay file (just make the first state of you FSA         *
* a SETXYTHETA state). The robot will appear to jump in space. A small      *
* bug is that during this jump a robot trajectory is drawn.                 *
*                                                                           *
****************************************************************************/
void exec_setxyTheta(double x, double y, double theta)
{
    REC_RobotLocation rloc;

    if (run_type == REAL)
	{
        hclient_setxyt(x, y, theta);
	}
    else
	{
        rloc.x = x;
        rloc.y = y;
        rloc.robot_id = robot_id;
        rloc.heading = theta;

        if (debug)
        {
            fprintf(stderr, "exec_set_xyTheta(%d): robot loc=(%f %f) heading=%f\n",
                    rloc.robot_id, rloc.x, rloc.y, rloc.heading);
        }

        communicator->SendMessage(console, MSG_RobotLocation, &rloc);
	}
}

//-----------------------------------------------------------------------
void exec_set_camera_tracker_mode(int mode)
{
    if (robot_type == HSERVER)
    {
        hclient_set_camera_tracker_mode(mode);
    }
}

//-----------------------------------------------------------------------
// This function sets the mobility type of the robot.
//-----------------------------------------------------------------------
void exec_set_mobility_type(int mobilityType)
{
    REC_SetMobilityType rec;

    rec.robotID = robot_id;
    rec.mobilityType = mobilityType;

    communicator->SendMessage(console, MSG_SetMobilityType, &rec);
}

//-----------------------------------------------------------------------
void exec_spin_robot(float avel, float sf)
{
    exec_get_position();  // for display purposes

    // make spin available also in simulation
    REC_SpinRobot rec;

    rec.w = avel;
    rec.robot_id = robot_id;

    if (run_type == REAL)
    {
        hclient_drive(0);
        hclient_steer(avel);

        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_SpinRobot"))
            {
                communicator->SendMessage(console, MSG_SpinRobot, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_SpinRobot, &rec);
    }
}

/****************************************************************************
* exec_move_robot by the distance in v                                      *
*                                                                           *
* This marks the end of a robot cycle as far as single step & pause care    *
****************************************************************************/
void exec_move_robot(Vector v)
{
    float speed, theta;
    REC_MoveRobot rec;

    if (debug)
    {
        fprintf(stderr, "exec_move_robot(%.2f, %.2f)\n", v.x, v.y);
    }

    //exec_get_gps();

    // data logging
    if (log_robot_data)
    {
        // Due to the threading, right now, this robot-executable.cc cannot
        // tell which FSA (i.e. Wheel Actuator or Camera Actuator) it is in.
        // Thus, it only records the data for one fsa (FSA-1).

        int fsa = 1;
        int state_num = current_state[fsa];

        // get the position x, y, and heading of the robot.
        robot_position rp = exec_get_position();

        // store those in a file
        exec_log_robot_data(v, rp, robot_id, state_num);
    }

    // In simulation mode updates simulation.  In real mode, allows us to
    // draw the movement vector to the screen.
    rec.x = v.x;
    rec.y = v.y;
    rec.z = v.z;
    rec.robot_id = robot_id;

    if (run_type == REAL)
    {
        speed = len_2d(v);
        theta = RADIANS_TO_DEGREES(atan2(v.y, v.x));

        hclient_steer_toward_and_drive(speed, theta, drive_wait_angle, use_reverse);

        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_MoveRobot"))
            {
                communicator->SendMessage(console, MSG_MoveRobot, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_MoveRobot, &rec);
    }
}

void exec_update_console_lm_params()
{
    REC_UpdateLMParams rec;

    rec.iRobotID = robot_id;

    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_UpdateLMParams"))
            {
                communicator->SendMessage(console, MSG_UpdateLMParams, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_UpdateLMParams, &rec);
    }
}

/****************************************************************************
*                                                                           *
* exec_report_current_state                                                 *
*  * This routine will send back the information, containing what state     *
*    currently the robot is in, to mlab console.                            *
*                                                                           *
*  Inputs:                                                                  *
*     msg       = information about current state.                          *
*                                                                           *
****************************************************************************/

void exec_report_current_state(char* msg)
{
    REC_ReportedState rec;
    rec.message = msg;

    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_ReportedState"))
            {
                communicator->SendMessage(console, MSG_ReportedState, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_ReportedState, &rec);
    }

    if (debug)
    {
        fprintf(stderr, "exec_reported_current_state: msg = \n%s\n", msg);
    }
}


/****************************************************************************
* exec_get_position location of robot                                       *
*                                                                           *
****************************************************************************/

static robot_position last_pos;
static unsigned long last_updated_cycle = (unsigned long) -1;

robot_position exec_get_position()
{
    robot_position rtn;
    if (last_updated_cycle == current_cycle)
    {
        return last_pos;
    }

    REC_GetPosition request;
    REC_RobotLocation* location;

    if (run_type == SIMULATION)
    {
        request.robot_id = robot_id;
        location =
            (REC_RobotLocation*) communicator->QueryFormatted(
                console,
                MSG_GetPosition,
                &request,
                MSG_RobotLocation);

        rtn.v.x = location->x;
        rtn.v.y = location->y;
        rtn.v.z = location->z;
        rtn.heading = location->heading;
        free(location);
    }
    else
    {
        if (hclient_getxyt(&rtn.v.x, &rtn.v.y, &rtn.heading))
        {
            fprintf(stderr, "Error in hclient_getxyt\n");
        }
        rtn.v.z = 0;

        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_RobotLocation"))
            {
                REC_RobotLocation rob_loc;
                rob_loc.robot_id = robot_id;
                rob_loc.x = rtn.v.x;
                rob_loc.y = rtn.v.y;
                rob_loc.z = 0;
                rob_loc.heading = rtn.heading;
                communicator->SendMessage(console, MSG_RobotLocation, &rob_loc);
            }
        }
    }
    // Update the sensor blackboard with the latest position
    sensor_blackboard.x = rtn.v.x;
    sensor_blackboard.y = rtn.v.y;
    sensor_blackboard.theta = rtn.heading;

    if (debug)
    {
        fprintf(stderr, "GET_POSITION(%d): robot loc=(%.1f %.1f) heading=%.1f\n",
                robot_id, rtn.v.x, rtn.v.y, rtn.heading);
    }

    last_pos = rtn;
    last_updated_cycle = current_cycle;

    return rtn;
}

robot_position exec_get_robot_position(int rid)
{
    robot_position rtn;

    if (last_updated_cycle == current_cycle)
    {
        return last_pos;
    }

    REC_GetPosition request;
    REC_RobotLocation* location;

    if (run_type == SIMULATION)
    {
        request.robot_id = rid;
        location = (REC_RobotLocation*) communicator->
            QueryFormatted(console, MSG_GetPosition, &request, MSG_RobotLocation);
        rtn.v.x = location->x;
        rtn.v.y = location->y;
        rtn.v.z = location->z;
        rtn.heading = location->heading;
    }

    // Update the sensor blackboard with the latest position
    sensor_blackboard.x = rtn.v.x;
    sensor_blackboard.y = rtn.v.y;
    sensor_blackboard.theta = rtn.heading;

    if (debug)
    {
        fprintf(stderr, "GET_POSITION(%d): robot loc=(%.1f %.1f) heading=%.1f\n",
                 robot_id, rtn.v.x, rtn.v.y, rtn.heading);
    }

    last_pos = rtn;
    last_updated_cycle = current_cycle;

    return rtn;
}



/****************************************************************************
* exec_get_gps current gps position                                       *
****************************************************************************/

static gps_position last_gps_pos;
static unsigned long last_updated_gps_cycle = (unsigned long) -1;

gps_position exec_get_gps(void)
{
    double ping_time;
    int i;
    gps_position rtn;
    REC_GpsPosition gps_pos_sim;

    if (last_updated_gps_cycle == current_cycle)
    {
        return last_gps_pos;
    }

    REC_GetPosition request;
    REC_RobotLocation* location;

    if (run_type == SIMULATION)
    {
        request.robot_id = robot_id;
        location = (REC_RobotLocation*) communicator->QueryFormatted(
            console,
            MSG_GetPosition,
            &request,
            MSG_RobotLocation);
        gps_pos_sim.robot_id = robot_id;
        gps_pos_sim.latitude = location->x;
        gps_pos_sim.longitude = location->y;
        gps_pos_sim.direction = location->heading;
        gps_pos_sim.pingtime = 0.0;
        gps_pos_sim.pingloss = 0;
        rtn.latitude = location->x;
        rtn.longitude = location->y;
        rtn.direction = location->heading;

        char temp_str[1024];
        sprintf(temp_str, "%d", gps_pos_sim.pingloss);
        put_state("pingloss", temp_str);
        sprintf(temp_str, "%f", gps_pos_sim.pingtime);
        put_state("pingtime", temp_str);
        communicator->SendMessage(console, MSG_GpsPosition, &gps_pos_sim);
        delete location;
    }
    else
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_GpsPosition"))
            {
                int iGETGPS = hclient_get_gps(&rtn.latitude, &rtn.longitude, &rtn.direction);
                if (iGETGPS<0)
                {
                    fprintf(stderr, "Error in hclient_get_gps\n");
                }
                else if (iGETGPS == 0)
                {
                    request.robot_id = robot_id;
                    location =
                        (REC_RobotLocation*)communicator->QueryFormatted(
                            console,
                            MSG_GetPosition,
                            &request,
                            MSG_RobotLocation);
                    rtn.latitude = location->x;
                    rtn.longitude = location->y;
                    rtn.direction = location->heading;
                    delete location;
                }

                bPingHistory[99] = exec_ping_console(&ping_time);
                for (i=0;i<99;i++)
                {
                    bPingHistory[i]=bPingHistory[i+1];
                }

                if (bPingHistory[99])
                {
                    // Report current location to the console
                    REC_GpsPosition gps_pos;
                    gps_pos.robot_id = robot_id;
                    gps_pos.latitude = rtn.latitude;
                    gps_pos.longitude = rtn.longitude;
                    gps_pos.direction = rtn.direction;
                    gps_pos.pingtime = ping_time;
                    gps_pos.pingloss = 0;
                    for(i=0;i<100;i++)
                    {
                        if (!bPingHistory[i])
                        {
                            gps_pos.pingloss++;
                        }
                    }

                    char temp_str[1024];
                    sprintf(temp_str, "%d", gps_pos.pingloss);
                    put_state("pingloss", temp_str);
                    sprintf(temp_str, "%f", gps_pos.pingtime);
                    put_state("pingtime", temp_str);

                    communicator->SendMessage(console, MSG_GpsPosition, &gps_pos);
                }
            }
        }
    }

    if (debug)
    {
        fprintf(stderr, "GET_GPS(%d): gps pos=(lat: %.1f lon:%.1f) direction=%.1f\n",
                robot_id, rtn.latitude, rtn.longitude, rtn.direction);
    }

    last_gps_pos = rtn;
    last_updated_gps_cycle = current_cycle;

    return rtn;
}

/****************************************************************************
* exec_get_map_position location of robot                                   *
*                                                                           *
****************************************************************************/

Vector exec_get_map_position(char* name)
{
    REC_InquireMapLocation question;
    Vector v = ZERO_VECTOR;

    question.robot_id = robot_id;
    question.name = name;

    IPMessage* msg = communicator->Query(console, MSG_InquireMapLocation, &question, MSG_MapLocation);
    REC_MapLocation* response = (REC_MapLocation*) msg->FormattedData();

    if (response->valid)
    {
        v.x = response->pos.x;
        v.y = response->pos.y;
    }
    else
    {
        cerr << "Unknown map object '" << name << "'\n";
    }

    delete msg;
    return v;
}

int connectMachine(char* host, char* port)
{
    int sock, connectresult;
    sockaddr_in sockaddr;
    hostent* hp;

    printf("#Trying to connect..\n");
    if (isdigit(*host))
    {
        sockaddr.sin_addr.s_addr=inet_addr(host);
    }
    else
    {
        if ((hp = gethostbyname(host)) == NULL)
        {
            printf("#ERROR - UNKNOWN HOST.\n");
            return 0;
        }
        memcpy((char*) &sockaddr.sin_addr, hp->h_addr, sizeof(sockaddr.sin_addr));
    }

    if (isdigit(*port))
    {
        sockaddr.sin_port=htons(atoi(port));
    }
    else
    {
        printf("#THE PORT SHOULD BE A NUMBER.\n");
        return 0;
    }

    if ((sock = socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        printf("Error in socket\n");
    }
    sockaddr.sin_family = AF_INET;


    connectresult = connect(sock, (struct sockaddr*) &sockaddr, sizeof(sockaddr));

    if (connectresult)
    {
        close(sock);
        switch (errno)
        {
        case ECONNREFUSED:
            printf("#ERROR - CONNECTION REFUSED.\n");
            break;
        case ENETUNREACH:
            printf("#ERROR - THE NETWORK IS NOT REACHABLE FROM THIS HOST.\n");
            break;
        default:
            printf("#Couldn't connect\n");
        }
        return 0;
    }
    printf("#Connected\n");
    return sock;
}

typedef unsigned char byte_t;
int giLaserServerFd = 0;

int nextRemainingBytes(int fd, byte_t* bytes, int max)
{
    int cnum;

    do
    {
        cnum = read(fd, bytes, max);
    } while ((cnum == 0) || (cnum == -1));
    return cnum;
}

/****************************************************************************
* exec_get_raw_sonar 							    *
****************************************************************************/

raw_sonar_array exec_get_raw_sonar()
{
    float left, right;
    if (hclient_get_side_ranges(&left, &right))
    {
        halt("range read failed");
    }

    raw_sonar_array tmp(2);

    tmp.val[0].val = right;
    tmp.val[1].val = left;

    return tmp;
}

float exec_get_elevation(void) {
   REC_GetElevation rqst;
   float *ele;

   rqst.robot_id = robot_id;
   IPMessage* msg = communicator->Query(console, MSG_GetElevation,
                                        &rqst, MSG_CurElevation);
   ele = (float *) msg->FormattedData();
   delete msg;
   return *ele;
}

//-----------------------------------------------------------------------
// trigger for when a robot has recieved a task
//-----------------------------------------------------------------------
bool exec_cnp_got_a_bidding_task(void)
{
    REC_GetTask rqst;
    REC_GetTaskConstraints rqst2;
    REC_CurTask *taskforbid;
    REC_CNP_TaskConstraintsData *data;
    RobotCNP_TaskInfo_t currentBiddingTaskInfo;
    CNP_TaskConstraint cnpTaskConstraint;
    IPMessage* msg = NULL;
    list<CNP_TaskConstraint> cnpTaskConstraints;
    list<CNP_TaskConstraint>::const_iterator iterTC;
    char *buf = NULL;
    char *taskInfoNameData = NULL, *nameData = NULL, *valueData = NULL;
    char *taskInfoNameDataInitPtr = NULL, *nameDataInitPtr = NULL, *valueDataInitPtr = NULL;
    int i, numConstraints;
    int taskInfoNameSize, nameSize, valueSize;
    int taskInfoTotalNameSize, totalNameSize, totalValueSize;
    bool setSuccess = false;
    const CNP_TaskConstraint EMPTY_CNP_TASK_CONSTRAINT = {{0, string()}, string(), CNP_STRING, string()};
    const bool DEBUG_EXEC_CNP_GOT_A_BIDDING_TASK = false;

    if (gRobotCNP == NULL)
    {
        fprintf(
            stderr,
            "Error in exec_cnp_got_a_task() [robot%d]. gRobotCNP is NULL.\n",
            robot_id);
        return false;
    }

    // Check with console. The robot ID is required here because the console
    // has to check to see if this robot is in its exclusion list.
    rqst.robot_id = robot_id;
    msg = communicator->Query(console, MSG_GetTask, &rqst, MSG_CurTask);

    // Decode the data
    taskforbid = (REC_CurTask *)(msg->FormattedData());

    if (debug || DEBUG_EXEC_CNP_GOT_A_BIDDING_TASK)
    {
        fprintf(stderr, "exec_cnp_got_a_bidding_task()[robot%d]:\n", robot_id);
        fprintf(
            stderr,
            " TaskID: %d, Iteration: %d, TaskDesc %s, Restriction %s.\n",
            taskforbid->TaskID,
            taskforbid->Iteration,
            taskforbid->TaskDesc,
            taskforbid->Restriction);
    }

    // Basic error checking
    if (taskforbid->TaskID < 0)
    {
    	delete msg;
        return false;
    }

    // Copy the data.
    currentBiddingTaskInfo.taskID = taskforbid->TaskID;
    currentBiddingTaskInfo.iteration = taskforbid->Iteration;
    currentBiddingTaskInfo.taskDesc = taskforbid->TaskDesc;
    currentBiddingTaskInfo.restriction = taskforbid->Restriction;

    // Delete the message.
    delete msg;

    // Now, get the task constraints.
    rqst2.robotID = robot_id;
    msg = communicator->Query(
        console,
        MSG_GetTaskConstraints,
        &rqst2,
        MSG_CNP_TaskConstraintsData);

    // Get the formatted data.
    data = (REC_CNP_TaskConstraintsData *)msg->FormattedData();

    // Get some key numbers.
    numConstraints = data->numConstraints;
    taskInfoTotalNameSize = data->taskInfoTotalNameSize;
    totalNameSize = data->totalNameSize;
    totalValueSize = data->totalValueSize;

    // Allocate the size
    taskInfoNameData = new char[taskInfoTotalNameSize];
    nameData = new char[totalNameSize];
    valueData = new char[totalValueSize];

    // Copy the data
    memcpy(taskInfoNameData, data->taskInfoNameData, taskInfoTotalNameSize);
    memcpy(nameData, data->nameData, totalNameSize);
    memcpy(valueData, data->valueData, totalValueSize);

    // Remember the pointers
    taskInfoNameDataInitPtr = taskInfoNameData;
    nameDataInitPtr = nameData;
    valueDataInitPtr = valueData;

    // Copy the contents of the constraints
    for (i = 0; i < numConstraints; i++)
    {
        // Clear the previous values.
        cnpTaskConstraint = EMPTY_CNP_TASK_CONSTRAINT;

        // Copy the TaskInfo ID.
        cnpTaskConstraint.taskInfo.id = data->taskInfoIDs[i];

        // Copy the TaskInfo name.
        taskInfoNameSize = data->taskInfoNameSizes[i];
        buf = new char[taskInfoNameSize+1];
        memcpy(buf, taskInfoNameData, taskInfoNameSize);
        buf[taskInfoNameSize] = '\0';
        cnpTaskConstraint.taskInfo.name = buf;
        delete [] buf;
        taskInfoNameData += taskInfoNameSize;

        // Copy the constraint name.
        nameSize = data->nameSizes[i];
        buf = new char[nameSize+1];
        memcpy(buf, nameData, nameSize);
        buf[nameSize] = '\0';
        cnpTaskConstraint.strConstraintName = buf;
        delete [] buf;
        nameData += nameSize;

        // Copy the constraint type
        cnpTaskConstraint.constraintType = (CNP_ConstraintType)(data->types[i]);

        // Copy the constraint value
        valueSize = data->valueSizes[i];
        buf = new char[valueSize+1];
        memcpy(buf, valueData, valueSize);
        buf[valueSize] = '\0';
        cnpTaskConstraint.strConstraintValue = buf;
        delete [] buf;
        valueData += valueSize;

        // Put the constraint in the list
        cnpTaskConstraints.push_back(cnpTaskConstraint);
    }

    delete [] taskInfoNameDataInitPtr;
    delete [] nameDataInitPtr;
    delete [] valueDataInitPtr;
    delete msg;

    // Save it as the currently bidding task.
    setSuccess = gRobotCNP->setCurrentBiddingTask(
        currentBiddingTaskInfo,
        cnpTaskConstraints);

    if (debug || DEBUG_EXEC_CNP_GOT_A_BIDDING_TASK)
    {
        i = 0;

        for (iterTC = cnpTaskConstraints.begin();
             iterTC != cnpTaskConstraints.end();
             iterTC++)
        {

            fprintf(
                stderr,
                "[%d] {%d, %s, %s, %d, %s}\n",
                i++,
                iterTC->taskInfo.id,
                iterTC->taskInfo.name.c_str(),
                iterTC->strConstraintName.c_str(),
                iterTC->constraintType,
                iterTC->strConstraintValue.c_str());
        }

        fprintf(
            stderr,
            "gRobotCNP->setCurrentBiddingTask() = %s\n",
            setSuccess? "true" : "false");
    }

    return setSuccess;
}

//-----------------------------------------------------------------------
// This functions tells console that the task is canceled.
//-----------------------------------------------------------------------
void exec_cnp_cancel_task(bool isReneged, string msg)
{

    REC_CancelTask rqst;
    RobotCNP_TaskInfo_t currentExecutingTaskInfo;

    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_cancel_task(). gRobotCNP is NULL.\n");
        return;
    }

    currentExecutingTaskInfo = gRobotCNP->getCurrentExecutingTaskInfo();
    gRobotCNP->resetCurrentExecutingTask();

    rqst.TaskID = currentExecutingTaskInfo.taskID;
    rqst.Iteration = currentExecutingTaskInfo.iteration;
    rqst.RobotID = robot_id;
    rqst.Consensual = 1;
    rqst.IsReneged = isReneged;
    rqst.Info = strdup(msg.c_str()); // Reason for renegging.

    communicator->SendMessage(console, MSG_CancelTask, &rqst);

    free(rqst.Info);
}

// Obsolute: Use exec_notify_cnp_task_reneged() instad.
/*
//-----------------------------------------------------------------------
// reneg on a particular task
//-----------------------------------------------------------------------
void exec_cnp_reneg_task(void)
{
    char buf[2048];
    int i;

    string renegReason = EMPTY_STRING;
    list<CNP_TaskConstraint> tc = gRobotCNP->getCurrentBiddingTaskConstraints();
    list<CNP_TaskConstraint>::iterator tcit = tc.begin();

    // construct the new set of constraints
    while (tcit != tc.end()) {
        sprintf(buf, ":%d:", (int)tcit->constraintType);

        i = 0;

        while (true)
        {
            if (CONSTRAINT_DATABASE_KEY_MAP[i].constraintName == EMPTY_STRING)
            {
                break;
            }


            if ((*tcit).strConstraintName == CONSTRAINT_DATABASE_KEY_MAP[i].constraintName)
            {
                (*tcit).strConstraintValue = get_state(CONSTRAINT_DATABASE_KEY_MAP[i].dataBaseKey.c_str());
                break;
            }

            i++;
        }

        renegReason = renegReason + (*tcit).strConstraintName + buf + (*tcit).strConstraintValue;
        tcit++;
        if (tcit != tc.end()) {
            renegReason = renegReason + ",";
        }
    }

    exec_cnp_cancel_task(true, renegReason);
}
*/
//-----------------------------------------------------------------------
// This function notifies to everybody that the executing CNP task has
// been completed.
//-----------------------------------------------------------------------
void exec_notify_cnp_task_completed(void)
{
    RobotCNP_TaskInfo_t currentExecutingTaskInfo;
    string notifyMessage = CNP_NOTIFY_TASK_COMPLETED_MSG_PREFIX;

    if (gRobotCNP == NULL)
    {
        fprintf(
            stderr,
            "Error(%d): exec_notify_cnp_task_completed(). gRobotCNP is NULL.\n",
            robot_id);
        return;
    }

    // Get the executing task info.
    currentExecutingTaskInfo = gRobotCNP->getCurrentExecutingTaskInfo();

    // Tell mlab that the task is completed.
    exec_cnp_cancel_task(false, EMPTY_STRING);

    // Tell database in all robots that the task is completed.
    notifyMessage += currentExecutingTaskInfo.taskDesc;
    exec_broadcast_value((char *)(notifyMessage.c_str()), "1", THIS_ROBOT);
}

//-----------------------------------------------------------------------
// This function notifies to everybody that the executing CNP task has
// been reneged.
//-----------------------------------------------------------------------
void exec_notify_cnp_task_reneged(void)
{
    RobotCNP_TaskInfo_t wonTaskInfo;
    string renegReason = EMPTY_STRING;
    list<CNP_TaskConstraint> tc;
    list<CNP_TaskConstraint>::iterator tcit;
    string notifyMessage = CNP_NOTIFY_TASK_RENEGED_MSG_PREFIX;
    char buf[2048];
    int i;
    bool const DEBUG_EXEC_NOTIFY_CNP_TASK_RENEGED = false;

    if (gRobotCNP == NULL)
    {
        fprintf(
            stderr,
            "Error(%d): exec_notify_cnp_task_reneged(). gRobotCNP is NULL.\n",
            robot_id);
        return;
    }

    // First, compile the reneg reason.
    tc = gRobotCNP->getCurrentBiddingTaskConstraints();
    tcit = tc.begin();

    while (tcit != tc.end()) {
        sprintf(buf, ":%d:", (int)tcit->constraintType);

        i = 0;

        while (true)
        {
            if (CONSTRAINT_DATABASE_KEY_MAP[i].constraintName == EMPTY_STRING)
            {
                break;
            }


            if ((*tcit).strConstraintName == CONSTRAINT_DATABASE_KEY_MAP[i].constraintName)
            {
                (*tcit).strConstraintValue = get_state(CONSTRAINT_DATABASE_KEY_MAP[i].dataBaseKey.c_str());
                break;
            }

            i++;
        }

        renegReason = renegReason + (*tcit).strConstraintName + buf + (*tcit).strConstraintValue;
        tcit++;
        if (tcit != tc.end()) {
            renegReason = renegReason + ",";
        }
    }

    // Get the executing task info.
    wonTaskInfo = gRobotCNP->getWonTaskInfo();

    // Tell mlab about the reneging.
    exec_cnp_cancel_task(true, renegReason);

    // Tell database in all robots that the task is reneged.
    notifyMessage += wonTaskInfo.taskDesc;
    exec_broadcast_value((char *)(notifyMessage.c_str()), "1", THIS_ROBOT);

    if (debug || DEBUG_EXEC_NOTIFY_CNP_TASK_RENEGED)
    {
        fprintf(
            stderr,
            "exec_notify_cnp_task_reneged() [robot %d]: Message [%s] boradcasted.\n",
            robot_id,
            notifyMessage.c_str());
    }
}

//-----------------------------------------------------------------------
// This function checks to see if a specified CNP task has been completed.
// If selectWonCNPTask is true, the won task recorded in gRobotCNP is
// checked. Otherwise, specificCNPTaskName is checked.
//-----------------------------------------------------------------------
bool exec_cnp_task_completion_notified(
    bool selectWonCNPTask,
    string specificCNPTaskName)
{
    RobotCNP_TaskInfo_t wonTaskInfo;
    string notifyMessage = CNP_NOTIFY_TASK_COMPLETED_MSG_PREFIX;
    char *value = NULL;
    char *ptr = NULL;
    bool notified = false;
    bool DEBUG_EXEC_CNP_TASK_COMPLETION_NOTIFIED = false;

    if (selectWonCNPTask)
    {
        if (gRobotCNP == NULL)
        {
            fprintf(
                stderr,
                "Error(%d): exec_cnp_task_completion_notified(). gRobotCNP is NULL.\n",
                robot_id);
            return false;
        }

        // Get the won task name.
        wonTaskInfo = gRobotCNP->getWonTaskInfo();
        notifyMessage += wonTaskInfo.taskDesc;

    }
    else
    {
        notifyMessage += specificCNPTaskName;
    }


    // Checking the database entry "notifymesage" value.
    // Note: it uses "return_state", so that it deletes the entry
    //       from the database once it is read.
    if ((value = return_state((char *)(notifyMessage.c_str()))) != NULL)
    {
        // The key is in the database, so decode it
        notified = (const bool)(strtol(value, &ptr, 10));

        if(ptr == value || ptr == NULL || *ptr != '\0')
        {
            fprintf(
                stderr,
                "Error(%d): exec_cnp_task_completion_notified(): Badly formed string: [%s]\n",
                robot_id,
                notifyMessage.c_str());

            notified = false;
        }
    }
    else
    {
        // Not in the database.
        notified = false;
    }

    if (debug || DEBUG_EXEC_CNP_TASK_COMPLETION_NOTIFIED)
    {
        fprintf(
            stderr,
            "exec_cnp_task_completion_notified()[robot %d]. Message [%s] notified?: %s\n",
            robot_id,
            notifyMessage.c_str(),
            notified? "true" : "false");
    }

    // Found.
    return notified;
}

//-----------------------------------------------------------------------
// This function checks to see if a specified CNP task has been reneged.
// If selectWonCNPTask is true, the won task recorded in gRobotCNP is
// checked. Otherwise, specificCNPTaskName is checked.
//-----------------------------------------------------------------------
bool exec_cnp_task_reneging_notified(
    bool selectWonCNPTask,
    string specificCNPTaskName)
{
    RobotCNP_TaskInfo_t wonTaskInfo;
    string notifyMessage = CNP_NOTIFY_TASK_RENEGED_MSG_PREFIX;
    char *value = NULL;
    char *ptr = NULL;
    bool notified = false;
    bool DEBUG_EXEC_CNP_TASK_RENEGING_NOTIFIED = false;

    if (selectWonCNPTask)
    {
        if (gRobotCNP == NULL)
        {
            fprintf(
                stderr,
                "Error(%d): exec_cnp_task_reneging_notified(). gRobotCNP is NULL.\n",
                robot_id);
            return false;
        }

        // Get the won task name.
        wonTaskInfo = gRobotCNP->getWonTaskInfo();
        notifyMessage += wonTaskInfo.taskDesc;

    }
    else
    {
        notifyMessage += specificCNPTaskName;
    }


    // Checking the database entry "notifymesage" value.
    // Note: it uses "return_state", so that it deletes the entry
    //       from the database once it is read.
    if ((value = return_state((char *)(notifyMessage.c_str()))) != NULL)
    {
        // The key is in the database, so decode it
        notified = (const bool)(strtol(value, &ptr, 10));

        if(ptr == value || ptr == NULL || *ptr != '\0')
        {
            fprintf(
                stderr,
                "Error(%d): exec_cnp_task_reneging_notified(): Badly formed string: [%s]\n",
                robot_id,
                notifyMessage.c_str());

            notified = false;
        }
    }
    else
    {
        // Not in the database.
        notified = false;
    }

    if (debug || DEBUG_EXEC_CNP_TASK_RENEGING_NOTIFIED)
    {
        fprintf(
            stderr,
            "exec_cnp_task_reneging_notified()[robot %d]. Message [%s] notified?: %s\n",
            robot_id,
            notifyMessage.c_str(),
            notified? "true" : "false");
    }

    // Found.
    return notified;
}

//-----------------------------------------------------------------------
// This function registers that the CNP task that just won has been
// started.
//-----------------------------------------------------------------------
void exec_notify_cnp_won_task_started(void)
{
    if (gRobotCNP == NULL)
    {
        fprintf(
            stderr,
            "Error in exec_notify_cnp_won_task_started(). gRobotCNP is NULL.\n");
        return;
    }

    // Set this task as the won task.
    gRobotCNP->setWonTaskExecuted();
}

//-----------------------------------------------------------------------
// !TODO! this whole function should be rewritten
//-----------------------------------------------------------------------
bool exec_cnp_is_task_changed(void)
{
    return false;
}

//-----------------------------------------------------------------------
// A trigger that returns true if the auction is ended.
//-----------------------------------------------------------------------
bool exec_cnp_is_auction_ended(void)
{
    RobotCNP_TaskInfo_t currentBiddingTaskInfo;
    REC_GetAward rqst;
    REC_CurAward *resp;
    string notifyMessage;
    char *value = NULL;
    char *ptr = NULL;
    bool notified = false;
    const bool DEBUG_EXEC_CNP_IS_AUCTION_ENDED = false;

    if (gRobotCNP == NULL)
    {
        fprintf(
            stderr,
            "Error in exec_cnp_is_auction_ended() [robot %d]. gRobotCNP is NULL.\n",
            robot_id);
        return true;
    }

    // First, checking the database entry for auction failure
    // Note: it uses "return_state", so that it deletes the entry
    //       from the database once it is read.
    notifyMessage = CNP_NOTIFY_TASK_AUCTION_FAILED_MSG;
    if ((value = return_state((char *)(notifyMessage.c_str()))) != NULL)
    {
        // The key is in the database, so decode it
        notified = (const bool)(strtol(value, &ptr, 10));

        if(ptr == value || ptr == NULL || *ptr != '\0')
        {
            fprintf(
                stderr,
                "Error(%d): exec_cnp_is_auction_ended(): Badly formed string: [%s]\n",
                robot_id,
                notifyMessage.c_str());

            notified = false;
        }
    }
    else
    {
        // Not in the database.
        notified = false;
    }

    if (debug || DEBUG_EXEC_CNP_IS_AUCTION_ENDED)
    {
        fprintf(
            stderr,
            "exec_cnp_is_auction_ended() [robot %d]. Auction failure notified = %d\n",
            robot_id,
            notified);
    }

    if (notified)
    {
        // Note as the bidding lost.
        gRobotCNP->resetCurrentBiddingTask(ROBOTCNP_BIDDING_LOST);

        // The auction is failed, and it means that auction is ended.
        return true;
    }

    // Get the saved record.
    currentBiddingTaskInfo = gRobotCNP->getCurrentBiddingTaskInfo();

    // Clear the reneg flag.
    notifyMessage = CNP_NOTIFY_TASK_RENEGED_MSG_PREFIX;
    notifyMessage += currentBiddingTaskInfo.taskDesc;
    exec_broadcast_value((char *)(notifyMessage.c_str()), "0", THIS_ROBOT);

    if (debug || DEBUG_EXEC_CNP_IS_AUCTION_ENDED)
    {
        fprintf(
            stderr,
            "exec_cnp_is_auction_ended()[robot %d]. Message [%s] = 0 (flag clear) sent.\n",
            robot_id,
            notifyMessage.c_str());
    }

    // Marshall the query input.
    rqst.TaskID = currentBiddingTaskInfo.taskID;
    rqst.Iteration = currentBiddingTaskInfo.iteration;
    rqst.RobotID = robot_id;

    // Send the query to console.
    IPMessage* msg = communicator->Query(
        console,
        MSG_GetAward,
        &rqst,
        MSG_CurAward);

    resp = (REC_CurAward *)(msg->FormattedData());

    // Validate the ID.
    if (((resp->TaskID) < 0) || ((resp->RobotID) < 0))
    {
        if (debug || DEBUG_EXEC_CNP_IS_AUCTION_ENDED)
        {
            fprintf(
                stderr,
                "exec_cnp_is_auction_ended() [robot %d]. Auction not ended. Task[%d] Robot(%d)\n",
                robot_id,
                resp->TaskID,
                resp->RobotID);
        }

        delete msg;
        return false;
    }

    // Validate the robot ID.
    if (resp->RobotID != robot_id)
    {
        // Lost the bidding.
        gRobotCNP->resetCurrentBiddingTask(ROBOTCNP_BIDDING_LOST);

        if (debug || DEBUG_EXEC_CNP_IS_AUCTION_ENDED)
        {
            fprintf(
                stderr,
                "exec_cnp_is_auction_ended() [robot(%d)]: Auction lost (Awarded to robot(%d)).\n",
                robot_id,
                resp->RobotID);
        }

        delete msg;
        return true;
    }

    // It actually won
    gRobotCNP->resetCurrentBiddingTask(ROBOTCNP_BIDDING_WON);
    if (debug || DEBUG_EXEC_CNP_IS_AUCTION_ENDED)
    {
        fprintf(
            stderr,
            "exec_cnp_is_auction_ended() [robot(%d)]. Auction awarded.\n",
            robot_id);
    }

    delete msg;
    return true;
}

//-----------------------------------------------------------------------
// A trigger that returns true if the auction is failed.
//-----------------------------------------------------------------------
bool exec_cnp_is_auction_failed(void)
{
    REC_GetAward rqst;
    REC_CurAward *resp;
    IPMessage* msg = NULL;
    string notifyMessage;
    bool failed = false;
    const bool DEBUG_EXEC_CNP_IS_AUCTION_FAILED = false;

    if (gRobotCNP != NULL)
    {
        // Marshall the query input.
        rqst.TaskID = -2;
        rqst.Iteration = -2;
        rqst.RobotID = -2;

        // Send the query to console.
        msg = communicator->Query(
            console,
            MSG_GetAward,
            &rqst,
            MSG_CurAward);

        resp = (REC_CurAward *)(msg->FormattedData());

        if (debug || DEBUG_EXEC_CNP_IS_AUCTION_FAILED)
        {
            fprintf(
                stderr,
                "exec_cnp_is_auction_failed(): TaskID = %d Iteration = %d => RobotID = %d\n",
                rqst.TaskID,
                rqst.Iteration,
                resp->RobotID);
        }

        if ((resp->RobotID) == -2)
        {
            failed = true;
        }
    }
    else
    {
        fprintf(stderr, "Error in exec_cnp_is_auction_failled(). gRobotCNP is NULL.\a\n");
        failed = true;
    }

    if (failed)
    {
        // Tell database in all robots that the auction is failed.
        notifyMessage = CNP_NOTIFY_TASK_AUCTION_FAILED_MSG;
        exec_broadcast_value((char *)(notifyMessage.c_str()), "1", THIS_ROBOT);
    }

    if (debug || DEBUG_EXEC_CNP_IS_AUCTION_FAILED)
    {
        fprintf(
            stderr,
            "exec_cnp_is_auction_failed() [task(%d), robot(%d)]. Auction failed %d.\n",
            rqst.TaskID,
            robot_id,
            failed);
    }

    delete msg;
    return failed;
}

//-----------------------------------------------------------------------
// A trigger that returns true if a bidding task is lost
//
// Note: It requires that exec_cnp_is_auction_ended has to be executed
//       previously in order to update the bidding status.
//-----------------------------------------------------------------------
bool exec_cnp_is_task_lost(void)
{
    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_is_task_lost(). gRobotCNP is NULL.\n");
        return false;
    }

    return (gRobotCNP->biddingLost());
}


//-----------------------------------------------------------------------
// A trigger that returns true if a bidding task is won
//
// Note: It requires that exec_cnp_is_auction_ended has to be executed
//       previously in order to update the bidding status.
//-----------------------------------------------------------------------
bool exec_cnp_is_task_won(void)
{
    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_is_task_won(). gRobotCNP is NULL.\n");
        return false;
    }

    return (gRobotCNP->biddingWon());
}

//-----------------------------------------------------------------------
// A trigger that returns true if robot's won task is the task being
// specified.
//-----------------------------------------------------------------------
bool exec_cnp_won_task_is(string cnpTaskName)
{
    RobotCNP_TaskInfo_t wonTaskInfo;

    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_won_task_is(). gRobotCNP is NULL.\n");
        return false;
    }

    wonTaskInfo = gRobotCNP->getWonTaskInfo();

    if ((wonTaskInfo.taskDesc != EMPTY_STRING) &&
        (wonTaskInfo.taskDesc == cnpTaskName))
    {
        return true;
    }

    return false;
}

//-----------------------------------------------------------------------
// This function bids the task
//-----------------------------------------------------------------------
void exec_cnp_bid_on_task(void)
{
    RobotCNP_TaskInfo_t currentBiddingTaskInfo, wonTaskInfo;
    REC_BidTask rec;
    list<CNP_RobotConstraint> updatedRobotConstraints;
    string notifyMessage = CNP_NOTIFY_TASK_COMPLETED_MSG_PREFIX;
    double bid;
    const bool DEBUG_CNP_BID_ON_TASK = false;

    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_bid_on_task(). gRobotCNP is NULL.\n");
        return;
    }

    // First, make sure to delete the task completion key from database.
    wonTaskInfo = gRobotCNP->getWonTaskInfo();
    notifyMessage += wonTaskInfo.taskDesc;
    return_state((char *)(notifyMessage.c_str()));

    // Get the task info.
    currentBiddingTaskInfo = gRobotCNP->getCurrentBiddingTaskInfo();

    // Gather the updated robot constraints and save them.
    // Note: updatedRobotConstraints should be filled here.
    gRobotCNP->updateRobotConstraints(updatedRobotConstraints);

    // Compute the bid

    // gather robot specific information
    CNPRobotData mrd;
    mrd.Pos = exec_get_position();

    // gather task specific information
    bid = gRobotCNP->compTaskBid(mrd);

    // Marshall the input
    rec.TaskID = currentBiddingTaskInfo.taskID;
    rec.Iteration = currentBiddingTaskInfo.iteration;
    rec.RobotID = robot_id;
    rec.Bid = bid;
    rec.TaskAccept = (bid > 0)? 1 : 0;

    // Send them to the console.
    communicator->SendMessage(console, MSG_BidTask, &rec);

    if (debug || DEBUG_CNP_BID_ON_TASK)
    {
        fprintf(
            stderr,
            "exec_cnp_bid_on_task(): Robot(%d) - Bid [%f]\n",
            robot_id,
            bid);
    }
}

//-----------------------------------------------------------------------
// This function returns the track position based on the CNP task.
//-----------------------------------------------------------------------
Vector exec_cnp_get_track_pos(void)
{
    RobotCNP_TaskInfo_t currentExecutingTaskInfo;
    ObjectList objlist;
    Vector output;
    string targetColor;
    float tx, ty;
    int cnt;

    VECTOR_CLEAR(output);

    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_cnp_get_track_pos(). gRobotCNP is NULL.\n");
        return output;
    }

    currentExecutingTaskInfo = gRobotCNP->getCurrentExecutingTaskInfo();

    if (currentExecutingTaskInfo.taskDesc == STRING_CNP_TRACKRED)
    {
        targetColor = "red";
    }
    else if (currentExecutingTaskInfo.taskDesc == STRING_CNP_TRACKYELLOW)
    {
        targetColor = "yellow";
    }
    else
    {
        fprintf(stderr, "Warning: exec_cnp_get_track_pos(). Unknow target color.\n");
        return output;
    }

    objlist = exec_detect_objects(1000);
    for (cnt = 0; cnt < objlist.count; cnt++)
    {
        if (!strcmp(objlist.objects[cnt].objcolor, targetColor.c_str()))
        {
            tx = objlist.objects[cnt].x2;
            ty = objlist.objects[cnt].y2;
            break;
        }
    }

    output.x = tx;
    output.y = ty;
    output.z = 0;

    return output;
}

//-----------------------------------------------------------------------
bool exec_detect_nearest_doorway(Vector* pt1, Vector* pt2, int* time_stamp)
{
    if (run_type == SIMULATION)
    {
        // Find all doors nearby
        REC_DetectDoorway rqst;
        rqst.robot_id = robot_id;
        rqst.sensor_dir = 0;
        rqst.sensor_fov = 360;
        rqst.sensor_range = 100;
        IPMessage* msg = communicator->Query(console, MSG_DetectDoorway,
                                              &rqst, MSG_DetectDoorwayReply);
        DoorwayList* doors = (DoorwayList*) msg->FormattedData();
        if (doors->num_doors == 0) return false;
        float bdist = doors->doors[0].distFromRobot;
        int best = 0;
        for (int i = 1; i < doors->num_doors; i++)
        {
            if (doors->doors[i].distFromRobot < bdist)
            {
                best = i;
                bdist = doors->doors[i].distFromRobot;
            }
        }
        pt1->x = doors->doors[best].x1;
        pt1->y = doors->doors[best].y1;
        pt2->x = doors->doors[best].x2;
        pt2->y = doors->doors[best].y2;

        delete msg;
        return true;
    }
    else
    {
        laser_doorway_request_t rqst;
        byte_t pack[4096];
        byte_t* data;
        int len, n;
        laser_doorway_reply_t dway;

        if (!giLaserServerFd)
        {
            char port[256];
            sprintf(port, "%d", SERV_ROBOT_EXEC_PORT);
            if (!(giLaserServerFd = connectMachine(laserfit_machine_name, port)))
            {
                fprintf(stderr, "Unable to open laser connection @%s:%s\n", laserfit_machine_name, port);
                exit(-1);
            }
        }
        rqst.cmd = LASER_GET_DOORWAY_POS;
        rqst.direction = LASER_GET_DOORWAY_EITHER;
        write(giLaserServerFd, &rqst, sizeof(laser_doorway_request_t));
        data = pack;
        len = sizeof(laser_doorway_reply_t);
        while (len)
        {
            n = nextRemainingBytes(giLaserServerFd, data, len);
            len -= n;
            data += n;
        }
        data = pack;
        memcpy(&dway, data, sizeof(laser_doorway_reply_t));
        if (!dway.isdoor)
        {
            return false;
        }
        pt1->x = (dway.pt1x + 20)/100.0;
        pt1->y = dway.pt1y/100.0;
        pt2->x = (dway.pt2x + 20)/100.0;
        pt2->y = dway.pt2y/100.0;
        robot_position robot_loc = exec_get_position();
        rotate_z((*pt1), robot_loc.heading);
        rotate_z((*pt2), robot_loc.heading);

        pt1->x += robot_loc.v.x;
        pt1->y += robot_loc.v.y;
        pt2->x += robot_loc.v.x;
        pt2->y += robot_loc.v.y;
        *time_stamp = dway.time_stamp;
        printf("Doorway has width %f\n", sqrt(pow(pt1->x-pt2->x, 2) + pow(pt1->y-pt2->y, 2)));
        return true;
    }
}

//-----------------------------------------------------------------------
bool exec_detect_nearest_unmarked_doorway(Vector* pt1, Vector* pt2)
{
    int i, best;
    float bdist;
    if (run_type == SIMULATION)
    {
        // Find all doors nearby
        REC_DetectDoorway rqst;
        rqst.robot_id = robot_id;
        rqst.sensor_dir = 0;
        rqst.sensor_fov = 360;
        rqst.sensor_range = 100;
        IPMessage* msg = communicator->Query(console, MSG_DetectDoorway,
                                             &rqst, MSG_DetectDoorwayReply);
        DoorwayList* doors = (DoorwayList*) msg->FormattedData();
        if (doors->num_doors == 0) return false;
        best = 0;
        while (doors->doors[best].marked == 1) {
            best++;
            if (best >= doors->num_doors) return false;
        }
        bdist = doors->doors[best].distFromRobot;
        for (i = best+1; i < doors->num_doors; i++) {
            if (doors->doors[i].distFromRobot < bdist &&
                doors->doors[i].marked == 0) {
                best = i;
                bdist = doors->doors[i].distFromRobot;
            }
        }
        pt1->x = doors->doors[best].x1;
        pt1->y = doors->doors[best].y1;
        pt2->x = doors->doors[best].x2;
        pt2->y = doors->doors[best].y2;

        delete msg;
        return true;
    }
    else
    {
        return false; // Only implemented in simulation thus far
    }
}

//-----------------------------------------------------------------------
void exec_mark_nearest_doorway(void)
{
    if (run_type == SIMULATION)
    {
        REC_MarkDoorway rqst;
        rqst.robot_id = robot_id;
        rqst.mark_status = 1;
        communicator->SendMessage(console, MSG_MarkDoorway, &rqst);
    }
}

//-----------------------------------------------------------------------
void exec_unmark_nearest_doorway(void)
{
    if (run_type == SIMULATION)
    {
        REC_MarkDoorway rqst;
        rqst.robot_id = robot_id;
        rqst.mark_status = 0;
        communicator->SendMessage(console, MSG_MarkDoorway, &rqst);
    }
}

//-----------------------------------------------------------------------
bool exec_detect_unmarked_doorway(double hallway_width,
                                   double robot_length, int look_for, double max_dist)
{
    if (run_type == SIMULATION)
    {
        robot_position  loc = exec_get_position();
        REC_DetectDoorway rqst;
        rqst.robot_id = robot_id;
        if (look_for == 0)
        {
            rqst.sensor_dir = loc.heading + 270;
        }
        else
        {
            rqst.sensor_dir = loc.heading + 90;
        }
        while (rqst.sensor_dir >= 360)
        {
            rqst.sensor_dir -= 360;
        }
        rqst.sensor_fov = 12;
        rqst.sensor_range = max_dist;
        IPMessage* msg = communicator->Query(console, MSG_DetectDoorway,
                                             &rqst, MSG_DetectDoorwayReply);
        DoorwayList* doors = (DoorwayList*) msg->FormattedData();
        if (doors->num_doors == 0) return false;

        for (int i = 0; i < doors->num_doors; i++)
        {
            if (doors->doors[i].marked == 0)
            {
            	delete msg;
                return true;
            }
        }
        delete msg;
        return false;
    }
    // Unmarked doorway notion exists in simulation only
    else
    {
        return exec_detect_doorway(hallway_width, robot_length, look_for, max_dist);
    }
}

//-----------------------------------------------------------------------
bool exec_detect_doorway(
    double hallway_width,
    double robot_length,
    int look_for,
    double max_dist)
{
    bool            atDoorway;
    raw_sonar_array side_sonar(0);
    robot_position  loc = exec_get_position();
    static int      shorts[2] = {0, 0}, seen[2] = {0, 0}, longs[2] = {0, 0};
    static double   startx[2], starty[2], dx[2] = {0, 0}, dy[2] = {0, 0};
    double          dist, lensq;
    int             i = look_for;

    if (run_type == SIMULATION)
    {
        REC_DetectDoorway rqst;
        rqst.robot_id = robot_id;
        if (look_for == 0) rqst.sensor_dir = loc.heading + 270;
        else rqst.sensor_dir = loc.heading + 90;
        while (rqst.sensor_dir >= 360) rqst.sensor_dir -= 360;
        rqst.sensor_fov = 12;
        rqst.sensor_range = max_dist;
        IPMessage* msg = communicator->Query(console, MSG_DetectDoorway,
                                             &rqst, MSG_DetectDoorwayReply);
        DoorwayList* doors = (DoorwayList*) msg->FormattedData();
        if (doors->num_doors == 0)
        {
        	delete msg;
            return false;
        }

        delete msg;
        return true;
    }
    else if (1) //use Laser
    {
        laser_doorway_request_t rqst;
        byte_t pack[4096], *data;
        int len, n;
        laser_doorway_reply_t dway;

        if (!giLaserServerFd)
        {
            char port[256];
            sprintf(port, "%d", SERV_ROBOT_EXEC_PORT);
            if (!(giLaserServerFd = connectMachine(laserfit_machine_name, port)))
            {
                fprintf(stderr, "Unable to open laser connection @%s:%s\n", laserfit_machine_name, port);
                exit(-1);
            }
        }
        rqst.cmd = LASER_GET_DOORWAY_POS;
        rqst.direction = look_for;
        write(giLaserServerFd, &rqst, sizeof(laser_doorway_request_t));
        data = pack;
        len = sizeof(laser_doorway_reply_t);
        while (len)
        {
            n = nextRemainingBytes(giLaserServerFd, data, len);
            len -= n;
            data += n;
        }
        data = pack;
        memcpy(&dway, data, sizeof(laser_doorway_reply_t));
        if (dway.isdoor)
        {
            double x = (dway.pt1x / 100.0 + dway.pt2x / 100.0) / 2.0;
            double y = (dway.pt1y / 100.0 + dway.pt2y / 100.0) / 2.0;
            double dist = vlength(x, y);
            if (dist <= max_dist)
            {
                return true;
            }
        }
        return false;
    }
    else // Use Sonar
    {
        side_sonar = exec_get_raw_sonar();
        // If the side sonar is reading longer than the hallway_width
        // we assume that it is a door we are opposite
        atDoorway = (side_sonar.val[look_for].val > hallway_width)? true: false;

        // We are not near a doorway, record this as a short value
        // and zero the other counters
        if (!atDoorway)
        {
            shorts[i]++;
            longs[i] = seen[i] = 0;
            dx[i] = dy[i] = 0;
            return false;
        }

        // Otherwise we are near a doorway. We want to be next to the door
        // long enough to have travelled robot_length while observing the
        // doorway
        longs[i]++;
        if (longs[i] > 1)
        {
            dx[i] = fabs(startx[i] - loc.v.x);
            dy[i] = fabs(starty[i] - loc.v.y);
        }
        else
        {
            seen[i] = shorts[i];
            startx[i] = loc.v.x;
            starty[i] = loc.v.y;
            shorts[i] = 0;
        }
        dist = dx[i]*dx[i] + dy[i]*dy[i];
        lensq = robot_length*robot_length;

        // Enough clearance and atleast 4 short readings before reading longs
        if ((dist > lensq) && (seen[i] > 3))
        {
            // We found a doorway, restore counters for next time
            longs[i] = shorts[i] = 0;
            dx[i] = dy[i] = seen[i] = 0;
            return true;
        }
        return false;
    }
}

bool exec_detect_in_room(robot_position robot_loc)
{
    if (run_type == SIMULATION)
    {
        REC_DetectRoom rqst;
        rqst.robot_id = robot_id;
        rqst.robot_loc.v.x = robot_loc.v.x;
        rqst.robot_loc.v.y = robot_loc.v.y;
        rqst.robot_loc.heading = robot_loc.heading;
        IPMessage* msg = communicator->Query(console, MSG_DetectRoom,
                                             &rqst, MSG_DetectRoomReply);
        bool* ans = (bool*) msg->FormattedData();
        delete msg;
        return *ans;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------
bool exec_detect_in_hallway(robot_position robot_loc)
{
    if (run_type == SIMULATION)
    {
        REC_DetectHallway rqst;
        rqst.robot_id = robot_id;
        rqst.robot_loc.v.x = robot_loc.v.x;
        rqst.robot_loc.v.y = robot_loc.v.y;
        rqst.robot_loc.heading = robot_loc.heading;
        IPMessage* msg = communicator->Query(console, MSG_DetectHallway,
                                             &rqst, MSG_DetectHallwayReply);
        HallwayList *hways = (HallwayList*) msg->FormattedData();
        if (hways->num_hways == 0)
        {
        	delete msg;
            return false;
        }

        delete msg;
        return true;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------
bool exec_detect_alternate_hallway(robot_position robot_loc)
{
    if (run_type == SIMULATION)
    {
        REC_DetectHallway rqst;
        rqst.robot_id = robot_id;
        rqst.robot_loc.v.x = robot_loc.v.x;
        rqst.robot_loc.v.y = robot_loc.v.y;
        rqst.robot_loc.heading = robot_loc.heading;
        IPMessage* msg = communicator->Query(console, MSG_DetectHallway,
                                             &rqst, MSG_DetectHallwayReply);
        HallwayList *hways = (HallwayList*) msg->FormattedData();
        if (hways->num_hways == 0)
        {
        	delete msg;
            return false;
        }

		bool dev = (hways->num_hways > 1);
		delete msg;

        return dev;
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------
int exec_detect_hallway(robot_position robot_loc, Vector* start, Vector* end, double* width, int* time_stamp)
{
    if (run_type == SIMULATION)
    {
        static int timest;
        REC_DetectHallway rqst;
        rqst.robot_id = robot_id;
        rqst.robot_loc.v.x = robot_loc.v.x;
        rqst.robot_loc.v.y = robot_loc.v.y;
        rqst.robot_loc.v.z = 0;
        rqst.robot_loc.heading = robot_loc.heading;

        IPMessage* msg = communicator->Query(console, MSG_DetectHallway,
                                             &rqst, MSG_DetectHallwayReply);
        HallwayList *hways = (HallwayList*) msg->FormattedData();
        if (hways->num_hways > 0)
        {
            start->x = hways->hways[0].start.x;
            start->y = hways->hways[0].start.y;
            end->x = hways->hways[0].end.x;
            end->y = hways->hways[0].end.y;
            *width = hways->hways[0].width;

            *time_stamp = timest++;
            delete msg;
            return 1;
        }
        else
        {
        	delete msg;
            return 0;
        }
    }
    else
    {
        laser_simple_rec_t command;
        byte_t pack[4096], *data;
        int len, n;
        laser_hallway_reply_t hway;

        if (!giLaserServerFd)
        {
            char port[256];
            sprintf(port, "%d", SERV_ROBOT_EXEC_PORT);
            if (!(giLaserServerFd = connectMachine(laserfit_machine_name, port)))
            {
                fprintf(stderr, "Unable to open laser connection @%s:%s\n", laserfit_machine_name, port);
                exit(-1);
            }
        }
        command.cmd = LASER_GET_HALLWAY_INFO;
        write(giLaserServerFd, &command, sizeof(laser_simple_rec_t));
        data = pack;
        len = sizeof(laser_hallway_reply_t);
        while (len)
        {
            n = nextRemainingBytes(giLaserServerFd, data, len);
            len -= n;
            data += n;
        }
        data = pack;
        memcpy(&hway, data, sizeof(laser_hallway_reply_t));

        start->x = hway.forwardptx/100.0;
        start->y = hway.forwardpty/100.0;
        end->x   = hway.backwardptx/100.0;
        end->y   = hway.backwardpty/100.0;
        rotate_z((*start), robot_loc.heading);
        rotate_z((*end), robot_loc.heading);

        start->x += robot_loc.v.x;
        start->y += robot_loc.v.y;
        end->x += robot_loc.v.x;
        end->y += robot_loc.v.y;

        *width = hway.width/100;
        *time_stamp = hway.time_stamp;
        return 1;
    }
}

//-----------------------------------------------------------------------
int exec_detect_alternate_hallway_info_at_random(
    robot_position robot_loc,
    int seed,
    Vector* pt1,
    Vector* pt2,
    double* width,
    double* ddxx,
    double* ddyy)
{
    int idx;
    lineRec2d_t walls[4];
    point2d_t inters[4];
    double dx, dy, len;
    Vector mid, inpt;
    robot_position loc;

    if (run_type == SIMULATION)
    {
        REC_DetectHallway rqst;
        rqst.robot_id = robot_id;
        rqst.robot_loc.v.x = robot_loc.v.x;
        rqst.robot_loc.v.y = robot_loc.v.y;
        rqst.robot_loc.heading = robot_loc.heading;

        IPMessage* msg = communicator->Query(console, MSG_DetectHallway,
                                              &rqst, MSG_DetectHallwayReply);
        HallwayList *hways = (HallwayList*) msg->FormattedData();
        if (hways->num_hways == 0) return 0;
        if (hways->num_hways == 1) {
            delete msg;
            return 0;
        }
        // Choose at random a hallway among the alternates
        // idx zero is the current hallway, seed is a random number
        idx = seed % (hways->num_hways - 1);
        idx += 1;

        // Find the perpendicular slope to the hallway centerline
        dy = -1 * (hways->hways[idx].start.x - hways->hways[idx].end.x);
        dx = hways->hways[idx].start.y - hways->hways[idx].end.y;
        len = 1.0/sqrt(dx*dx + dy*dy);
        // Normalize dx and dy to 1/2 width
        dy = dy * len * hways->hways[idx].width / 2.0;
        dx = dx * len * hways->hways[idx].width / 2.0;
        // Now create the two lines the are the "walls" of the hallway
        setLineRec(hways->hways[idx].start.x + dx, hways->hways[idx].start.y + dy,
                    hways->hways[idx].end.x + dx, hways->hways[idx].end.y + dy,
                    &walls[0]);
        setLineRec(hways->hways[idx].start.x - dx, hways->hways[idx].start.y - dy,
                    hways->hways[idx].end.x - dx, hways->hways[idx].end.y - dy,
                    &walls[1]);
        // Now repeat for the current hallway index zero
        // Find the perpendicular slope to the hallway centerline
        dy = -1 * (hways->hways[0].start.x - hways->hways[0].end.x);
        dx = hways->hways[0].start.y - hways->hways[0].end.y;
        len = 1.0/sqrt(dx*dx + dy*dy);
        // Normalize dx and dy to 1/2 width
        dy = dy * len * hways->hways[0].width / 2.0;
        dx = dx * len * hways->hways[0].width / 2.0;
        // Now create the two lines the are the "walls" of the hallway
        setLineRec(hways->hways[0].start.x + dx, hways->hways[0].start.y + dy,
                    hways->hways[0].end.x + dx, hways->hways[0].end.y + dy,
                    &walls[2]);
        setLineRec(hways->hways[0].start.x - dx, hways->hways[0].start.y - dy,
                    hways->hways[0].end.x - dx, hways->hways[0].end.y - dy,
                    &walls[3]);
        // Store the four intersections
        intersectTwoLinesegs(&walls[2], &walls[0], &inters[0]);
        intersectTwoLinesegs(&walls[2], &walls[1], &inters[1]);
        intersectTwoLinesegs(&walls[3], &walls[0], &inters[2]);
        intersectTwoLinesegs(&walls[3], &walls[1], &inters[3]);
        idx = seed % 2;
        mid.x = (inters[idx * 2].x + inters[idx * 2 + 1].x) / 2.0;
        mid.y = (inters[idx * 2].y + inters[idx * 2 + 1].y) / 2.0;
        len = 1.0 / vlength(dx, dy);
        dx *= len;
        dy *= len;
        // Inpt is the point outside the current hallway
        // into what might be the alternate hallway if the
        // alternate hallway extends in both directions.
        inpt.x = mid.x + (idx ? -dx: dx);
        inpt.y = mid.y + (idx ? -dy: dy);
        loc.heading = atan2(inpt.y - mid.y, inpt.x - mid.x);
        loc.v.x = inpt.x;
        loc.v.y = inpt.y;
        // If the test point is not in a hallway return the other
        // opening
        if (exec_detect_in_hallway(loc) == false)
        {
            idx += 1;
            idx = idx % 2;
        }
        *ddxx = idx ? -dx: dx;
        *ddyy = idx ? -dy: dy;
        *width = hways->hways[0].width;
        pt1->x = inters[idx * 2].x;
        pt1->y = inters[idx * 2].y;
        pt2->x = inters[idx * 2 + 1].x;
        pt2->y = inters[idx * 2 + 1].y;

        delete msg;
        return 1;
    }
    return 0;
}

//-----------------------------------------------------------------------
static obs_array last_objlist;
static int objlist_last_updated_cycle = -1;
static float last_dist = 0;
obs_array  exec_detect_obstacles(float max_dist)
{
    if ((objlist_last_updated_cycle == (int) current_cycle) &&
        (last_dist >= max_dist))
    {
        return last_objlist;
    }

    obs_array out(0);
    REC_ObstacleList* rec;

    if (debug)
    {
        fprintf(stderr, "robot(%d):exec_detect_obs max %f\n",
                robot_id, max_dist);
    }

    if (run_type == SIMULATION)
    {

        REC_DetectObstacles request;

        request.robot_id = robot_id;
        request.max_dist = max_dist;

        IPMessage* msg = communicator->Query(console, MSG_DetectObstacles, &request, MSG_ObstacleList);
        rec = (REC_ObstacleList*) msg->FormattedData();

        obs_array tmp(rec->num_obs);

        for(int i = 0; i<rec->num_obs; i++)
        {
            tmp.val[i].center.x = rec->obs[i].x;
            tmp.val[i].center.y = rec->obs[i].y;
            tmp.val[i].center.z = rec->obs[i].z;
            tmp.val[i].r = rec->radius[i];
        }
        delete msg;
        out = tmp;
    }
    else // real robot
    {
        REC_SensorReadings sensormsg;
        sensor_reading_t* reading;
        int num_readings;
        int mask = 0;
        float ang;

        if (use_sonar) mask = mask|SENSOR_SONAR_MASK;
        if (use_laser) mask = mask|SENSOR_SICKLMS200_MASK;
        if (use_cognachrome) mask = mask|SENSOR_COGNACHROME_MASK;
        if (use_generic) mask = mask|SENSOR_GENERIC_MASK;
        num_readings = hclient_get_obstacles(mask, danger_range, &reading, adjust_obstacles);

        if ((draw_obstacles) && (!stopRobotFeedback))
        {
            if (!network_to_console_too_slow(true, "MSG_SensorReadings"))
            {
                sensormsg.robot_id = robot_id;
                sensormsg.num_readings = num_readings;
                sensormsg.reading = (REC_SensorReading*)reading;
                communicator->SendMessage(console, MSG_SensorReadings, &sensormsg);
            }
        }

        obs_array tmp(num_readings);
        Vector ptr;

        for(int i=0;i<num_readings;i++){
            ptr.x = reading[i].reading;
            ptr.y = 0;
            ang = reading[i].angle;
            rotate_z(ptr, ang);
            tmp.val[i].center.x = ptr.x;
            tmp.val[i].center.y = ptr.y;
            tmp.val[i].r = 0.0;  // need to do something smart here
        }
        out = tmp;
        if (num_readings)
        {
            free(reading);
        }
    }

    last_objlist = out;
    last_dist = max_dist;
    objlist_last_updated_cycle = current_cycle;

    // Update the sensor blackboard with the new list of detected objects
    // First, delete the existing readings.
    sensor_blackboard.sensed_objects.erase(
        sensor_blackboard.sensed_objects.begin(),
        sensor_blackboard.sensed_objects.end());
    // Next, add the new readings.
    for(int i=0; i<out.size; ++i)
    {
        sensor_blackboard.sensed_objects.push_back(out.val[i]);
    }

    return out;
}

//-----------------------------------------------------------------------
// This function attempts to place an object in a basket.
//-----------------------------------------------------------------------
void exec_drop_object_in_basket(int object_id, int basket_id)
{
    if (run_type == REAL)
    {
        return;
    }

    REC_DropInBasket rec;

    if (debug)
    {
        fprintf(stderr, "exec_drop_object_in_basket(%d) obj id=%d  basket id=%d\n",
                 robot_id, object_id, basket_id);
    }

    rec.robot_id = robot_id;
    rec.object_id = object_id;
    rec.basket_id = basket_id;
    communicator->SendMessage(console, MSG_DropInBasket, &rec);
}

//-----------------------------------------------------------------------
// This function identifies the vehicle type of the object (if the object
// is a robot).
//-----------------------------------------------------------------------
int exec_check_vehicle_type(int objectID)
{
    REC_CheckVehicleType req;
    REC_VehicleType *rec = NULL;
    IPMessage *msg = NULL;
    int vehicleType;

    if (run_type == REAL)
    {
        // Not supported by HServer yet.
        return UNKNOWN_ROBOT_MOBILITY_TYPE;
    }

    req.robotID = robot_id;
    req.objectID = objectID;

    // Send the query.
    msg = communicator->Query(
        console,
        MSG_CheckVehicleType,
        &req,
        MSG_VehicleType);
    rec = (REC_VehicleType *)msg->FormattedData();

    // Decode the vehicle type.
    vehicleType = rec->vehicleType;

    if (debug)
    {
        fprintf(
            stderr,
            "exec_check_vehicle_type(%d): Object(%d) Vehicle-Type = [%d]\n",
            robot_id,
            objectID,
            vehicleType);
    }

    delete msg;
    return vehicleType;

}

//-----------------------------------------------------------------------
// This function checks to see the target object is untrackable by this
// robot.
//-----------------------------------------------------------------------
bool exec_check_target_untrackable(int objectID)
{
    REC_CheckTargetUntrackable req;
    REC_TargetUntrackable *rec = NULL;
    IPMessage *msg = NULL;
    bool untrackable = false;

    if (run_type == REAL)
    {
        // Not supported by HServer yet.
        return true;
    }

    req.robotID = robot_id;
    req.objectID = objectID;

    // Send the query.
    msg = communicator->Query(
        console,
        MSG_CheckTargetUntrackable,
        &req,
        MSG_TargetUntrackable);
    rec = (REC_TargetUntrackable *)msg->FormattedData();

    // Decode the vehicle type.
    untrackable = rec->untrackable;

    if (debug)
    {
        fprintf(
            stderr,
            "exec_check_target_untrackable(%d): Object(%d) untrackable = [%d]\n",
            robot_id,
            objectID,
            untrackable);
    }

    delete msg;
    return untrackable;

}

/****************************************************************************
* exec_pickup_object                                                        *
*                                                                           *
* Attempt to pick up an object directly ahead of the robot                  *
****************************************************************************/

void exec_pickup_object(int object_id)
{
    if (run_type == REAL)
    {
        return;
    }

    REC_Pickup rec;

    if (debug)
    {
        fprintf(stderr, "exec_pickup_object(%d) id=%d\n", robot_id, object_id);
    }

    rec.robot_id = robot_id;
    rec.object_id = object_id;

    communicator->SendMessage(console, MSG_Pickup, &rec);
}

/****************************************************************************
* exec_probe_object                                                         *
*                                                                           *
* Attempt to pick up an object directly ahead of the robot                  *
****************************************************************************/

void exec_probe_object(int object_id)
{
    if (run_type == REAL)
    {
        return;
    }

    REC_Probe rec;

    if (debug)
    {
        fprintf(stderr, "exec_probe_object(%d) id=%d\n", robot_id, object_id);
    }

    rec.robot_id = robot_id;
    rec.object_id = object_id;
    communicator->SendMessage(console, MSG_Probe, &rec);
}

/****************************************************************************
*                                                                           *
* exec_send_email                                                           *
*                                                                           *
* Sends e-mail to whoever specified as "mailrecipnt".                       *
*                                                                           *
****************************************************************************/
int exec_send_email(char* mailrecipnt, char sbj[256], char* bodyfile)
{
    char cmd[1024];

    sprintf(cmd, "mail %s -s \"%s\" < %s", mailrecipnt, sbj, bodyfile);
    system(cmd);

    fprintf(stderr, "Mail sent to %s successfully.\n", mailrecipnt);
    return true;

}

int exec_frame_upload(char* name)
{
    int rtn = 0;
    if (robot_type == HSERVER)
    {
        rtn = hclient_frame_upload(1, name);
        rtn = !rtn;
    }
    return(rtn);
}

/****************************************************************************
* exec_alert                                                                *
*                                                                           *
* Warn user about the object found.                                         *
* Also sends email and attach the image captured if it was specified        *
****************************************************************************/

void exec_alert(char* alertsubject, char* alertmessage, bool use_email, char* mailrecipnt, bool attach_image)
{
    char sbj[256];
    char bdy[4096];
    char msg[4096];
    char* message;
    time_t cur_time;

    // subject line (or headline)
    sprintf(sbj, "%s", alertsubject);

    // get the time the object was found.
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];
    cur_time = time(NULL);
    strncpy(date, ctime(&cur_time), sizeof(date));
    tm local_time = *localtime(&cur_time);
    strftime(month, sizeof(month), "%m", &local_time);
    strftime(day, sizeof(day), "%d", &local_time);
    strftime(year, sizeof(year), "%Y", &local_time);
    strftime(hour, sizeof(hour), "%H", &local_time);
    strftime(min, sizeof(min), "%M", &local_time);
    strftime(sec, sizeof(sec), "%S", &local_time);

    // get the position of the robot
    robot_position robot_p = exec_get_position();

    // message body
    sprintf(bdy,  "%s\n\n\n", alertmessage);
    sprintf(bdy, "%sROBOT LOCATION (m)\n", bdy);
    sprintf(bdy, "%s%.1f, %.1f\n", bdy, robot_p.v.x, robot_p.v.y);
    sprintf(bdy, "%s\n", bdy);
    sprintf(bdy, "%sTIME\n", bdy);
    sprintf(bdy, "%s%s\n", bdy, date);

    // create pop-up message contents and send it to mlab
    REC_Alert rec;
    sprintf(msg, "%s\n\n\n%s", sbj, bdy);
    message = msg;
    rec.message = message;
    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_Alert"))
            {
                communicator->SendMessage(console, MSG_Alert, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_Alert, &rec);
    }

    if (debug)
    {
        fprintf(stderr, "exec_alert(%d): msg = \n%s\n", robot_id, msg);
    }

    if (use_email)
    {
        char bodyfilename[4096];
        sprintf(bodyfilename, "/tmp/alertmail_%d_%d", getpid(), robot_id);
        char* bodyfile = bodyfilename;
        unlink(bodyfile); // delete old one if there is any.
        FILE* mailbody;

        // mail body (write in a file).
        if ((mailbody = fopen(bodyfile, "w")) == NULL)
        {
            printf("Unable to write in %s!\n", bodyfile);
            exit(1);
        }
        fprintf(mailbody, "\n\n%s", bdy);
        fclose(mailbody);

        // attach the captured image to a file if there is any.
        if (attach_image)
        {
            char* username = getenv("USER");
            char* pwd = getenv("PWD");
            char image_id[4096], imagename[4096], imagefilename[4096];
            sprintf(image_id, "captured_image-%s-%s%s%s-%s%s%s-robot%d",
                    username, month, day, year, hour, min, sec, robot_id);
            sprintf(imagename, "%s.ppm", image_id);
            sprintf(imagefilename, "%s/%s", pwd, imagename);
            char* imagefile = strdup(imagefilename);

            // upload the captured image from camera on real robot
            if (robot_type == HSERVER)
            {
                unlink(imagefile); // delete old one
                fprintf(stderr, "Uploading the captured image...\n");
                if (exec_frame_upload(imagefile))
                    fprintf(stderr, "Image captured. Stored in %s.\n", imagefile);
            }

            struct stat stat_buf; // need the "struct" keyword here
            if ((stat(imagefile, &stat_buf)) == -1)
            {
                printf("sim_alert: No image file found. Image not attached.\n");
            }
            else // we have the image.
            {
                // first, encode the image into text. then, attach to the file.
                char cmd[1024];
                sprintf(cmd, "uuencode %s %s >> %s", imagefile, imagename, bodyfile);
                system(cmd);
            }
            if(imagefile)
            	free(imagefile);
            if(username)
            	free(username);
            if(pwd)
            	free(pwd);
        }

        // sending email
        exec_send_email(mailrecipnt, sbj, bodyfile);

        // delelte the bodyfile.
        unlink(bodyfile);
    }
}

/****************************************************************************
* exec_popup_telop_interface                                                *
*                                                                           *
* This function sends a message to the console to popup the telop interface *
* if it is not popped up yet.                                               *
****************************************************************************/

void exec_popup_telop_interface()
{
    REC_PopUpTelop rec;
    rec.popup = true;
    rec.robot_id = robot_id;
    communicator->SendMessage(console, MSG_PopUpTelop, &rec);

    if (debug)
    {
        fprintf(stderr, "exec_popup_telop_interface(%d)\n", robot_id);
    }
}

/****************************************************************************
* exec_ask_to_proceed                                                       *
*                                                                           *
* This function sends a message to the console to popup the proceed dialog. *
*                                                                           *
****************************************************************************/

void exec_ask_to_proceed()
{

    REC_AskToProceed rec;
    rec.ask = true;
    rec.robot_id = robot_id;

    communicator->SendMessage(console, MSG_AskToProceed, &rec);

    if (debug)
    {
        fprintf(stderr, "exec_ask_to_proceed(%d)\n", robot_id);
    }
}

/****************************************************************************
* exec_get_csb_comm_info                                                    *
*                                                                           *
* Returns the communication info for the communication sensitive behavior.  *
****************************************************************************/
CSB_CommInfoList_t exec_get_csb_comm_info(void)
{
    CSB_CommInfoList_t commInfoList;
    CSB_CommInfo_t commInfo;
    info_jbox_list_t jboxInfoList;
    vector<int>::iterator listIterator;
    robot_position selfRobotPos;
    vector<GTRobotPosID_t> posList;
    int i;
    const double SIM_MAX_DISTANCE = 50.0;
    const double SIM_MAX_SIGNAL_STRENGTH = 100.0;
    const bool USE_DUMMY = false;

    selfRobotPos = exec_get_position();

    if (run_type == SIMULATION)
    {
        if (USE_CSB_NETWORK_MODEL)
        {
            // Not yet re-integrated.
        }
        else if (USE_DUMMY)
        {
            static double theta = 0;
            commInfo.id = robot_id;
            commInfo.numHops = 0;
            commInfo.signalStrength = 50*(sin(theta) + 1.0);
            commInfo.nodeType = CSB_NODE_SELF;
            commInfo.heading = selfRobotPos.heading;
            commInfo.latitude = 0;
            commInfo.longitude = 0;
            commInfo.x = selfRobotPos.v.x;
            commInfo.y = selfRobotPos.v.y;
            commInfo.z = 0;
            theta += 0.1;

            if (theta > (2.0*M_PI))
            {
                theta = 0.0;
            }

            commInfoList.push_back(commInfo);
        }
        else
        {
            // First make up the data for self.
            commInfo.id = robot_id;
            commInfo.numHops = 0;
            commInfo.signalStrength = -1;
            commInfo.nodeType = CSB_NODE_SELF;
            commInfo.heading = selfRobotPos.heading;
            commInfo.latitude = 0;
            commInfo.longitude = 0;
            commInfo.x = selfRobotPos.v.x;
            commInfo.y = selfRobotPos.v.y;
            commInfo.z = 0;
            commInfoList.push_back(commInfo);

            // Get the information of all.
            posList = exec_get_sim_robots_pos(SIM_MAX_DISTANCE);

            for(i = 0; i < (int)(posList.size()); i++)
            {
                if (posList[i].id != robot_id)
                {
                    commInfo.id = posList[i].id;
                    commInfo.numHops = 1;

                    // Linear decay based solely on the distance.
                    commInfo.signalStrength =
                        SIM_MAX_SIGNAL_STRENGTH *
                        (SIM_MAX_DISTANCE -
                         sqrt(
                             pow((selfRobotPos.v.x - posList[i].x), 2) +
                             pow((selfRobotPos.v.y - posList[i].y), 2))) /
                        SIM_MAX_DISTANCE;

                    commInfo.nodeType = CSB_NODE_OTHER;
                    commInfo.heading = posList[i].heading;
                    commInfo.latitude = 0;
                    commInfo.longitude = 0;
                    commInfo.x = posList[i].x;
                    commInfo.y = posList[i].y;
                    commInfo.z = 0;
                    commInfoList.push_back(commInfo);
                }
            }
        }
    }
    else
    {
        hclient_get_jbox_info(&jboxInfoList);

        for (i = 0; i < (int)(jboxInfoList.size()); i++)
        {
            listIterator = find(
                allowedRobotJBoxIDs.begin(),
                allowedRobotJBoxIDs.end(),
                jboxInfoList[i].id);

            if (listIterator == allowedRobotJBoxIDs.end())
            {
                // This JBox is not a robot. Don't report.
                continue;
            }

            commInfo.id = jboxInfoList[i].id;
            commInfo.numHops = jboxInfoList[i].numHops;
            commInfo.signalStrength = (double)(jboxInfoList[i].signalStrength);
            commInfo.heading = jboxInfoList[i].gpsReading[GPS_HEADING];
            commInfo.latitude = jboxInfoList[i].gpsReading[GPS_LATITUDE];
            commInfo.longitude = jboxInfoList[i].gpsReading[GPS_LONGITUDE];
            commInfo.x = 0;
            commInfo.y = 0;
            commInfo.z = 0;

            if (commInfo.numHops < 0)
            {
                // Invalid number of hops. Don't report.
                continue;
            }
            else if (commInfo.numHops == 0)
            {
                // This info is about self.
                commInfo.nodeType = CSB_NODE_SELF;

                // Use the more reliable readings.
                commInfo.heading = selfRobotPos.heading;
                commInfo.x =  selfRobotPos.v.x;
                commInfo.y = selfRobotPos.v.y;
            }
            else
            {
                // This is perhaps another robot.
                commInfo.nodeType = CSB_NODE_OTHER;
            }

            commInfoList.push_back(commInfo);
        }
    }

    if (debug)
    {
        fprintf(stderr, "Signal Strength: %.2f\n", commInfo.signalStrength);
    }

   return commInfoList;
}

/****************************************************************************
* exec_get_csb_advise                                                       *
*                                                                           *
* This function executes the communication sensitive behavior, and returns  *
* the advise.                                                               *
****************************************************************************/

CSB_Advise_t exec_get_csb_advise(int method)
{
    CSB_Advise_t advise;
    CVectorConfidenceAdviceUMWrap *behaviorAdvise = NULL;

    // First, we have to make sure CSB is initialized already.
    // If not, initialize it.
    if (!exec_check_csb_initialized())
    {
        exec_init_csb();
    }

    SetAdviseMethod(csbInstance, method);

    // Now, execute the behavior.
    behaviorAdvise = Execute(csbInstance);
    advise.x = GetX(behaviorAdvise);
    advise.y = GetY(behaviorAdvise);
    advise.confidence = GetConfidence(behaviorAdvise);
    delete behaviorAdvise;
    behaviorAdvise = NULL;

    //if (debug)
    {
        fprintf(
            stderr,
            "advise <%f, %f>, confidence %f\n",
            advise.x,
            advise.y,
            advise.confidence);
    }

    return advise;
}

/****************************************************************************
* exec_init_csb                                                             *
*                                                                           *
* This function initializes the communication sensitive behavior.           *
****************************************************************************/

void exec_init_csb(void)
{
    info_jbox_list_t jboxInfoList;
    int i, selfID;
    bool selfJBoxFound = false;
    const int JBOX_SEARCH_SLEEP_USEC = 200000;

    if (csbInitialized)
    {
        return;
    }

    if (run_type == SIMULATION)
    {
        selfID = robot_id;
    }
    else
    {
        fprintf(stderr, "Acquiring Self JBox ID ...");

        while (!selfJBoxFound)
        {
            hclient_get_jbox_info(&jboxInfoList);

            for (i = 0; i < (int)(jboxInfoList.size()); i++)
            {
                if (jboxInfoList[i].numHops == 0)
                {
                    // The number of hops is 0. The JBox for this robot
                    // is found.
                    selfID = jboxInfoList[i].id;
                    selfJBoxFound = true;
                    break;
                }
            }

            if (selfJBoxFound)
            {
                break;
            }
            else
            {
                fprintf(stderr, ".");
                usleep(JBOX_SEARCH_SLEEP_USEC);
            }
        }

        fprintf(stderr, " found (%d).\n", selfID);
    }

    // Initialize GTechBehaivor
    csbInstance = createCommBehavior();
    Initialize(csbInstance, numExpectedRobots, selfID);
    Load(csbInstance, 0);

    // Initialize the sensor data class
    csbSensorDataInstance = createSensorDataUMWrap();

    csbInitialized = true;
}

/****************************************************************************
* exec_check_csb_initialized                                                *
*                                                                           *
* This function checks to see if the communication sensitive behavior was   *
* properly initialized.                                                     *
****************************************************************************/
bool exec_check_csb_initialized(void)
{
    return csbInitialized;
}

int exec_get_general_battery_level_carretilla()
{
	static float i = 60;
	if(run_type == REAL)
	{
		if((sensor[SENSOR_GENERIC].type!=-1) && (sensor[SENSOR_GENERIC].num_readings>20))
			return sensor[SENSOR_GENERIC].reading[20];
		else
			return 60;
	}
	else
	{
		i-=0.1;
		return i;
	}
}


int exec_get_servo_battery_level_carretilla()
{
	static float i = 8;
	if(run_type == REAL)
	{
		if((sensor[SENSOR_GENERIC].type!=-1) && (sensor[SENSOR_GENERIC].num_readings>21))
			return sensor[SENSOR_GENERIC].reading[21];
		else
			return 8;
	}
	else
	{
		i-=0.05;
		return i;
	}
}

void exec_set_signal_threshold_high(float threshold)
{
	if (!csbInitialized)
		return;

	SetSignalThresholdHigh(csbInstance, threshold);
}

void exec_set_signal_threshold_low(float threshold)
{
	if (!csbInitialized)
		return;

	SetSignalThresholdLow(csbInstance, threshold);
}


float exec_get_signal_level_carretilla(void)
{
	int i, j;
	float signalLevel;
	char line[200];
	FILE *f;
    REC_GetSignalStrength rec;
    rec.robotId = robot_id;

    if(run_type == REAL)
    	rec.filter = alfaFiltroWiFi;
    else
    	rec.filter = 0;

    REC_SignalStrength *signal = (REC_SignalStrength*) communicator->QueryFormatted(console,
    		MSG_GetSignalStrength, &rec, MSG_SignalStrength);

    if(run_type == REAL)
    {
    	system("iwconfig 2> /dev/null | grep \"Link Quality\" > salida.txt");
    	f = fopen("salida.txt", "r");
    	if(f!=NULL)
    	{
    		if(fgets(line, 200, f)!=NULL)
    		{
    			for(i=0;line[i]!='=';i++);
    			for(j=i;line[j]!='/';j++);
    			line[j]='\0';
    			signalLevel = atof(&line[i+1]);
    		}
    		else
    		{
    			signalLevel = 0;
    		}

    		fclose(f);
    		unlink("salida.txt");
    	}
    	else
    	{
    		signalLevel = 0;
    	}

    	printf("Señal real: %g Filtro: %g\n", signalLevel, alfaFiltroWiFi);

    	if(signalLevel==0)
    		signalLevel=signalLevelAnt;

    	signalLevel = alfaFiltroWiFi*signalLevelAnt+(1-alfaFiltroWiFi)*signalLevel;
    	signalLevelAnt = signalLevel;

    	printf("Señal remota: %g   --  Señal local: %g\n", signal->value, signalLevel);
    	signalLevel = (signalLevel < signal->value) ? signalLevel : signal->value;
    }
    else
    {
    	signalLevel = signal->value;
    }

    delete signal;

    //printf("SIGNAL LEVEL DEFINITIVO: %d\n", signalLevel);
    return signalLevel;
}

void exec_update_csb_carretilla(void)
{
    if (!csbInitialized)
    {
        // Initialize GTechBehaivor
        csbInstance = createCommBehavior();
        Initialize(csbInstance, numExpectedRobots, robot_id);
        Load(csbInstance, 0);

        // Initialize the sensor data class
        csbSensorDataInstance = createSensorDataUMWrap();

        printf("Cargando alfa del filtro de la señal WiFi (entre 0 y 1).\n");
        printf("Valores más grandes dan más peso a la señal anterior.\n");

        FILE *fichero = fopen("filtroPrimerOrden.cfg","r");
        if(fichero==NULL)
        {
        	printf("Archivo filtroPrimerOrden.cfg no encontrado!!\n");
        	printf("Se usará 0.8 para el alfa del filtro de primer orden de la señal WiFi\n");
        }
        else
        {
        	fscanf(fichero, "%f", &alfaFiltroWiFi);
        	printf("Usando %g como alfa del filtro de primer orden de la señal WiFi\n", alfaFiltroWiFi);
        }

        csbInitialized = true;
    }

    float signalLevel = exec_get_signal_level_carretilla();
    robot_position selfRobotPos = exec_get_position();

    printf("SIGNAL LEVEL PASADO A LA BIBLIOTECA: %d\n", signalLevel);
    SetSensorCommData(
                    csbSensorDataInstance,
                    robot_id, 				// Id
                    0, 						// Hops
                    -1, 					// SigStrength
                    selfRobotPos.heading, 	// Heading
                    selfRobotPos.v.x, 		// X
                    selfRobotPos.v.y, 		// Y
                    0); 					// Z

    SetSensorCommData(
                    csbSensorDataInstance,
                    2, 						// Id
                    1, 						// Hops
                    floor(signalLevel), 	// SigStrength
                    0, 						// Heading
                    0, 						// X
                    0, 						// Y
                    0); 					// Z

    UpdateSensoryData(csbInstance, csbSensorDataInstance);
}

/****************************************************************************
* exec_update_csb_sensor_data                                               *
*                                                                           *
* This function updates the sensor data for the communication sensitive     *
* behavior.                                                                 *
****************************************************************************/

void exec_update_csb_sensor_data(void)
{
    CSB_CommInfoList_t commInfoList;
    int i;

    // First, we have to make sure CSB is initialized already.
    // If not, initialize it.
    if (!exec_check_csb_initialized())
    {
        exec_init_csb();
    }

    // Get the communication info.
    commInfoList = exec_get_csb_comm_info();

    // Update sensor value.
    for (i = 0; i < (int)(commInfoList.size()); i++)
    {
        if ((run_type != SIMULATION) || USE_CSB_NETWORK_MODEL)
        {
            switch (commInfoList[i].nodeType) {

            case CSB_NODE_SELF:

                // The position information will be acquired directly from HServer
                SetSensorCommData(
                    csbSensorDataInstance,
                    commInfoList[i].id, // Id
                    commInfoList[i].numHops, // Hops
                    commInfoList[i].signalStrength, // SigStrength
                    commInfoList[i].heading, // Heading
                    commInfoList[i].x, // X
                    commInfoList[i].y, // Y
                    0); // Z

                if (debug)
                {
                    fprintf(
                        stderr,
                        "\nCSB Sensor:\nID: %d\nHops: %d\nSS: %f\nHead: %f\nX: %f\nY: %f\n",
                        commInfoList[i].id, // Id
                        commInfoList[i].numHops, // Hops
                        commInfoList[i].signalStrength, // SigStrength
                        commInfoList[i].heading, // Heading
                        commInfoList[i].x, // X
                        commInfoList[i].y); // Y
                }

                break;

            case CSB_NODE_OTHER:

                SetSensorCommDataTwo(
                    csbSensorDataInstance,
                    commInfoList[i].id, // Id
                    commInfoList[i].numHops, // Hops
                    commInfoList[i].signalStrength, // SigStrength
                    commInfoList[i].heading, // Heading
                    commInfoList[i].latitude, // Latitude
                    commInfoList[i].longitude); // Longititude

                if (debug)
                {
                    fprintf(
                        stderr,
                        "\nCSB Sensor:\nID: %d\nHops: %d\nSS: %f\nHead: %f\nLat: %f\nLon: %f\n",
                        commInfoList[i].id, // Id
                        commInfoList[i].numHops, // Hops
                        commInfoList[i].signalStrength, // SigStrength
                        commInfoList[i].heading, // Heading
                        commInfoList[i].latitude, // Latitude
                        commInfoList[i].longitude); // Longititude
                }

                break;
            }
        }
        else
        {
            SetSensorCommData(
                csbSensorDataInstance,
                commInfoList[i].id, // Id
                commInfoList[i].numHops, // Hops
                commInfoList[i].signalStrength, // SigStrength
                commInfoList[i].heading, // Heading
                commInfoList[i].x, // X
                commInfoList[i].y, // Y
                commInfoList[i].z); // Z

            if (debug)
            {
                fprintf(
                    stderr,
                    "\nCSB Sensor:\nID: %d\nHops: %d\nSS: %f\nHead: %f\nX: %f\nY: %f\n",
                    commInfoList[i].id, // Id
                    commInfoList[i].numHops, // Hops
                    commInfoList[i].signalStrength, // SigStrength
                    commInfoList[i].heading, // Heading
                    commInfoList[i].x, // X
                    commInfoList[i].y); // Y
            }
        }

        UpdateSensoryData(csbInstance, csbSensorDataInstance);
    }

}

/****************************************************************************
* exec_change_color                                                         *
*                                                                           *
* Change the color of an object which is ajacent to a robot                 *
****************************************************************************/

void exec_change_object_color(int object_id, char* new_color)
{
    REC_ChangeColor rec;
    REC_Ack* ack;

    rec.robot_id = robot_id;
    rec.object_id = object_id;
    rec.new_color = new_color;

    // We do a query here to syncronize the robot and make sure
    // that subsequent sensor readings will get the new color
    if (run_type != SIMULATION)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_ChangeColor"))
            {
                ack = (REC_Ack*) communicator->QueryFormatted(console, MSG_ChangeColor, &rec, MSG_Ack);
                delete ack;
            }
        }

    }
    else
    {
        ack = (REC_Ack*) communicator->QueryFormatted(console, MSG_ChangeColor, &rec, MSG_Ack);
        delete ack;
    }

    if (debug)
    {
        fprintf(stderr, "exec_pickup_object(%d) id=%d\n", robot_id, object_id);
    }
}

/****************************************************************************
* exec_change_robot_color                                                   *
*                                                                           *
* Change the color of an object which is ajacent to a robot                 *
****************************************************************************/

void exec_change_robot_color(char *new_color)
{
    REC_ChangeRobotColor rec;
    REC_Ack* ack;

    rec.robot_id = robot_id;
    rec.new_color = new_color;

    // We do a query here to syncronize the robot and make sure
    // that subsequent sensor readings will get the new color
    if (run_type != SIMULATION)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_ChangeRobotColor"))
            {
                ack = (REC_Ack*)communicator->QueryFormatted(
                    console,
                    MSG_ChangeRobotColor,
                    &rec,
                    MSG_Ack);
                delete ack;
            }
        }
    }
    else
    {
        ack = (REC_Ack*)communicator->QueryFormatted(
            console,
            MSG_ChangeRobotColor,
            &rec,
            MSG_Ack);
        delete ack;
    }
}

/****************************************************************************
* exec_add_object                                                           *
*                                                                           *
* Add a new object to the environment                                       *
****************************************************************************/

void exec_add_object(char* color, int style, Vector pos, double diameter)
{
    REC_AddObject rec;

    if (debug)
        fprintf(stderr, "exec_add_object(%d)\n", robot_id);

    rec.robot_id = robot_id;
    rec.color = color;
    rec.x = pos.x;
    rec.y = pos.y;
    rec.diameter = diameter;
    rec.style = style;

    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_AddObject"))
            {
                communicator->SendMessage(console, MSG_AddObject, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_AddObject, &rec);
    }
}

/****************************************************************************
*                                                                           *
****************************************************************************/

void halt(char* msg)
{
    REC_RobotDeath	rec;

    if (debug)
    {
        fprintf(stderr, "Robot %d halt(%s)\n", robot_id, msg);
    }

    if (run_type == REAL)
    {
        hclient_stop();
        hclient_range_stop();
        hclient_close_robot();
    }


    rec.robot_id = robot_id;
    rec.message = msg;
    communicator->SendMessage(console, MSG_RobotDeath, &rec);

    if (debug)
    {
        fprintf(stderr, "Robot %d exiting: %s\n", robot_id, msg);
    }

    exit(1);
}

//-----------------------------------------------------------------------
// This function sends a signal to console to terminate the entire
// mission. mlab process will be halted.
//-----------------------------------------------------------------------
void exec_terminate_mission(void)
{
    REC_TerminateMission rec;

    rec.robotID = robot_id;

    communicator->SendMessage(console, MSG_TerminateMission, &rec);
}

/****************************************************************************
*                                                                           *
****************************************************************************/
void finish_cycle_and_exit(char* msg)
{
    // Double halt
    if (halting)
    {
        fprintf(stderr, "Double finish_cycle_and_exit: Second message = '%s'\n", msg);
        halt(halting);
    }

    halting = strdup(msg);
}

/****************************************************************************
*                                                                           *
* exec_detect_robots returns location of the other visible robots            *
*                                                                           *
****************************************************************************/

RobotReadings exec_detect_robots(float max_dist)
{
    int                	i;
    REC_DetectRobots	rqst;

    rqst.robot_id = robot_id;
    rqst.max_dist = max_dist;

    if (run_type == REAL)
    {
        // Only supported in simulation.
        RobotReadings rtn(0);
        return rtn;
    }

    IPMessage* msg = communicator->Query(console, MSG_DetectRobots, &rqst, MSG_CurRobots);
    REC_CurRobots* rec = (REC_CurRobots*) msg->FormattedData();

    RobotReadings rtn(rec->num_robots);

    for(i = 0; i<rec->num_robots; i++)
    {
        rtn.readings[i].id = rec->robots[i].id;
        rtn.readings[i].v.x = rec->robots[i].v.x;
        rtn.readings[i].v.y = rec->robots[i].v.y;
        rtn.readings[i].v.z = rec->robots[i].v.z;
    }

    if (debug)
    {
        fprintf(stderr, "DETECT_ROBOTS(%d)\n", robot_id);
        for (i = 0; i < rtn.num_readings; i++)
        {
            fprintf(stderr, "   %d: (%.1f %.1f)\n",
                     rtn.readings[i].id,
                     rtn.readings[i].v.x,
                     rtn.readings[i].v.y);
        }
    }

    delete msg;
    return rtn;
}

//------------------------------------------------------------------------------
// Gets the robot position.
//------------------------------------------------------------------------------
vector<GTRobotPosID_t> exec_get_sim_robots_pos(double maxDist)
{
    vector<GTRobotPosID_t> posList;
    GTRobotPosID_t pos;
    REC_GetRobotsPos rqst;
    int i;

    rqst.robot_id = robot_id;
    rqst.max_dist = maxDist;

    if (run_type == REAL)
    {
        // Only supported in simulation.
        return posList;
    }

    IPMessage* msg = communicator->Query(
        console,
        MSG_GetRobotsPos,
        &rqst,
        MSG_RobotsPosReply);

    REC_RobotsPosReply* rec = (REC_RobotsPosReply*) msg->FormattedData();
    for(i = 0; i<rec->numRobots; i++)
    {
        pos.id = rec->robots[i].id;
        pos.x = rec->robots[i].x;
        pos.y = rec->robots[i].y;
        pos.z = rec->robots[i].z;
        pos.heading = rec->robots[i].heading;
        posList.push_back(pos);
    }

    if (debug)
    {
        fprintf(stderr, "robot(%d): exec_get_sim_robots_pos()\n", robot_id);
        for (i = 0; i < (int)(posList.size()); i++)
        {
            fprintf(
                stderr,
                "   %d: < %.1f %.1f %.1f >\n",
                posList[i].id,
                posList[i].x,
                posList[i].y,
                posList[i].heading);
        }
    }

    delete msg;
    return posList;
}


/****************************************************************************
*                                                                           *
* exec_detect_objects returns location of all visible objects               *
*                                                                           *
****************************************************************************/

ObjectList exec_detect_objects(float max_dist)
{
    int i;

    if (run_type == SIMULATION)
    {
        REC_DetectObjects	request;
        request.robot_id = robot_id;
        request.max_dist = max_dist;

        IPMessage* msg = communicator->Query(console, MSG_DetectObjects, &request, MSG_ObjectList);
        REC_ObjectList* rec = (REC_ObjectList*) msg->FormattedData();

        ObjectList rtn(rec->num_objects);

        for (i = 0; i < rec->num_objects; i++)
        {
            rtn.objects[i].setColor(rec->object[i].object_color);
            rtn.objects[i].id = rec->object[i].id;
            rtn.objects[i].objshape = (ObjectShapes) rec->object[i].object_shape;
            rtn.objects[i].x1 = rec->object[i].x1;
            rtn.objects[i].y1 = rec->object[i].y1;
            rtn.objects[i].r = rec->object[i].r;
            rtn.objects[i].x2 = rec->object[i].x2;
            rtn.objects[i].y2 = rec->object[i].y2;
            rtn.objects[i].ismovable = rec->object[i].ismovable;
        }

        delete msg;
        return rtn;
    }
    else if (robot_type == HSERVER)
    {
        sensor_reading_t *reading;
        int num_readings;
        int mask;
        float x, y;

        mask = SENSOR_COGNACHROME_MASK;
        num_readings = hclient_get_obstacles(mask, danger_range, &reading);

        ObjectList rtn(num_readings);
        for (i=0;i<num_readings;i++)
        {
            x = cos(reading[i].angle*(M_PI/180.0))*reading[i].reading;
            y = sin(reading[i].angle*(M_PI/180.0))*reading[i].reading;
            rtn.objects[i].id = NOID;
            rtn.objects[i].objshape = CircleObject;
            rtn.objects[i].x1 = x;
            rtn.objects[i].y1 = y;
            rtn.objects[i].r = 0.15;
            switch (reading[i].color)
            {
            case 0:
                rtn.objects[i].objcolor = "red";
                break;
            case 1:
                rtn.objects[i].objcolor = "green";
                break;
            case 2:
                rtn.objects[i].objcolor = "yellow";
                break;
            case 3:
                rtn.objects[i].objcolor = "black";
            }
        }

        if (num_readings > 0)
        {
            free(reading);
        }

        return(rtn);
    }
    return 0;
}

Vector exec_get_camera_heading()
{
    Vector heading;
    if (robot_type == HSERVER)
    {
        int pan=0, tilt=0;
        //hclient_get_camera_position(&pan, &tilt);
        heading.x = 1.0;
        heading.y = tan((pan*M_PI/180.0));
        heading.z = tan((tilt*M_PI/180.0));
    }
    else
    {
        heading.x = 0.0;
        heading.y = 0.0;
        heading.z = 0.0;
    }
    return heading;
}

void exec_move_camera(Vector direction)
{
    int theta_deg, psi_deg;
    float theta_rad, psi_rad;

    if (robot_type == HSERVER)
    {
        theta_rad = atan(direction.y / direction.x);
        psi_rad   = atan(direction.z / direction.x);

        theta_rad = DEGREES_FROM_RADIANS(theta_rad);
        psi_rad   = DEGREES_FROM_RADIANS(psi_rad  );

        //do rounding
        if (theta_rad < 0)
        {
            theta_deg = (int) (theta_rad - 0.5);
        }
        else
        {
            theta_deg = (int) (theta_rad + 0.5);
        }
        if (psi_rad < 0)
        {
            psi_deg = (int) (psi_rad - 0.5);
        }
        else
        {
            psi_deg = (int) (psi_rad + 0.5);
        }

        hclient_move_camera(theta_deg, psi_deg);
    }
}

void exec_set_turn_mode(int mode)
{
    wait_for_turn = mode;
}

/****************************************************************************
* exec_get_compass_heading:                                                 *
*                                                                           *
****************************************************************************/
int exec_get_compass_heading()
{
    int heading = 0;

    if (run_type == SIMULATION)
    {
        REC_GetPosition rec;
        REC_RobotPos  p;
        rec.robot_id = robot_id;
        heading = (int)rint(p.heading);
        heading = 360 - heading;
        heading += 90;
        deg_range_pm_360(heading);
    }
    else
    {
        //hclient_read_compass_heading(&heading);
    }
    return heading;
}

void exec_update_compass_window()
{
    REC_GetCompassHeading hd;
    int heading;

    heading = exec_get_compass_heading();
    hd.heading = heading;
}



/************************ SOUND RELATED STUFF ******************************/



/****************************************************************************
* exec_detect_sound:                                                        *
*                                                                           *
****************************************************************************/
#define         RAD             (3.1415926 / 180.0)
int exec_detect_sound()
{
    int volume = 0;

    if (run_type == SIMULATION)
    {
        // Do nothing the input is set by the mlab Sound Simulation dialog
    }
#if 0
    else
	{
        // For now it works only on the Nomad 200
        nomad200_detect_sound(&degrees, &volume);
        if (debug) printf("robot_side_com: deg=%d, vol=%d\n", degrees, volume);

        // We need to convert from degrees to corrdinates because of legacy issues
        sprintf(value, "%f", cos(degrees*RAD));
        put_state("sound_direction_x", value);
        if (debug) printf("robot_side_com2: d_x=%s, \n", value);

        sprintf(value, "%f", sin(degrees*RAD));
        put_state("sound_direction_y", value);
        if (debug) printf("robot_side_com2: d_y=%s, \n", value);


        sprintf(value, "%f", volume*1.0);
        put_state("sound_volume", value);

        if (debug)
        {
            printf("robot_side_com2: vol=%s, \n", value);
        }
        // Set the Database Double variables to the returned values
	}
#endif

    return volume;
}

/*
void exec_update_sound_sim_window()
{
    // Don't know how to implement this one yet
    REC_GetCompassHeading hd;
    int heading;

    heading = exec_get_compass_heading();
    hd.heading = heading;
}
*/


/****************************************************************************
* exec_talk() Talk using the Speech Synthesizer.                            *
* This finctionality is available for the NOmad 200 only.                   *
****************************************************************************/

void exec_talk(char* talkmessage)

{
    if (run_type == SIMULATION)
    {
        fprintf(stdout, "ROBOT SAYS: %s\n", talkmessage);
        fflush(stdout);
    }
}

/**************************** END SOUND STUFF  *******************************/



/****************************************************************************
* exec_put_console_state:                                                    *
*
*****************************************************************************/
void exec_put_console_state(char* parm, char* val)
{
    REC_PutState rec;

    // first, update our cache
    put_state(parm, val);

    rec.robot_id = robot_id;
    rec.key = parm;
    rec.value = val;

    if (run_type == REAL)
    {
        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_PutState"))
            {
                communicator->SendMessage(console, MSG_PutState, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_PutState, &rec);
    }

    if (debug)
    {
        fprintf(stderr, "PUT_CONSOLE_STATE: '%s'='%s'\n", parm, val);
    }
}

/****************************************************************************
* exec_broadcast_value:                                                     *
*                                                                           *
* Update the local database key with the new value. It will broadcast the   *
* value via either console (use -1 as the robot ID) or this robot.          *
*                                                                           *
****************************************************************************/
void exec_broadcast_value(char* key, char* value, int via)
{
    // first, update our cache
    put_state(key, value);

    if (via == MLAB_CONSOLE)
    {
        // Use the mlab console to broadcast this value.
        REC_PutState rec;

        rec.robot_id = BROADCAST_ROBOTID;
        rec.key = key;
        rec.value = value;

        if (run_type == REAL)
        {
            if (!stopRobotFeedback)
            {
                if (!network_to_console_too_slow(true, "MSG_PutState"))
                {
                    communicator->SendMessage(console, MSG_PutState, &rec);
                }
            }
        }
        else
        {
            communicator->SendMessage(console, MSG_PutState, &rec);
        }

        if (debug)
        {
            fprintf(stderr, "EXEC_BROADCAST_VALUE(%d) - via 'console'): '%s'='%s'\n",
                    robot_id, key, value);
        }
    }
    else
    {
        // Use this robot to broadcast this value.

        REC_NewState rec;

        rec.key = key;
        rec.value = value;
        communicator->Broadcast(MSG_NewState, &rec);

        if (debug)
        {
            fprintf(stderr, "EXEC_BROADCAST_VALUE(%d) - via 'robot(%d)': '%s'='%s'\n",
                    robot_id, robot_id, key, value);
        }
    }

}

/****************************************************************************
* exec_get_console_state:                                                    *
*                                                                           *
****************************************************************************/
void exec_get_console_state(char* parm, char* rtn, int len)
{
    char* val = get_state(parm);

    strncpy(rtn, val, len-1);
    rtn[len - 1] = '\0';

    if (debug)
    {
        fprintf(stderr, "GET_CONSOLE_STATE(%d): '%s'\n", robot_id, val);
    }
}

/****************************************************************************
* exec_reset_world                                                          *
****************************************************************************/
void exec_reset_world(char* new_overlay , int load_new_map)
{
    REC_ResetWorld rec;

    rec.new_overlay = new_overlay;
    rec.load_new_map = load_new_map;

    communicator->SendMessage(console, MSG_ResetWorld, &rec);

    if(debug)
    {
        fprintf(stderr,"RESETTING WORLD\n");
    }
}

/*==========================================================================*
*                                                                           *
* These commands are automatically called by the console when state         *
* information changes.                                                      *
*                                                                           *
****************************************************************************/

void new_command(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int i = 0;

    // free old record
    if (command != NULL)
    {
        if (command->path != NULL)
        {
            free(command->path);
        }

        if (command->unit != NULL)
        {
            free(command->unit);
        }

        if (command->phase_line != NULL)
        {
            free(command->phase_line);
        }

        if (command->phase_line_ack_msg != NULL)
        {
            free(command->phase_line_ack_msg);
        }

        if (command->completion_msg != NULL)
        {
            free(command->completion_msg);
        }

        free(command);
    }

    REC_NewCommand* rec = (REC_NewCommand*) msg->FormattedData();

    // allocate space for new command
    command = (gt_Command*) calloc(1, sizeof(gt_Command));

    // set all the path points
    command->num_path_points = rec->num_path_points;
    if (command->num_path_points)
    {
        command->path = (gt_Point*) calloc(command->num_path_points, sizeof(gt_Point));
        for(i = 0; i < command->num_path_points; i++)
        {
            command->path[i].x = rec->path[i].x;
            command->path[i].y = rec->path[i].y;
        }
    }
    else
    {
        command->path = NULL;
    }

    // set all units
    command->unit_size = rec->unit_size;
    if (command->unit_size)
    {
        command->unit = (int*) calloc(command->unit_size, sizeof(int));
        for(i = 0; i < command->unit_size; i++)
        {
            command->unit[i] = rec->unit[i];
        }
    }
    else
    {
        command->unit = NULL;
    }

    // set the phase lines
    command->num_pl_points = rec->num_pl_points;
    if (command->num_pl_points)
    {
        command->phase_line = (gt_Point*) calloc(command->num_pl_points, sizeof(gt_Point));
        for (i = 0; i < command->num_path_points; i++)
        {
            command->phase_line[i].x = rec->phase_line[i].x;
            command->phase_line[i].y = rec->phase_line[i].y;
        }
    }
    else
    {
        command->phase_line = NULL;
    }

    // set everything else
    command->goal.x = rec->goal.x;
    command->goal.y = rec->goal.y;
    command->behavior = rec->behavior;
    command->formation = rec->formation;
    command->technique = rec->technique;
    command->speed = rec->speed;
    command->width = rec->width;
    command->phase_line_given = rec->phase_line_given;
    command->time = rec->time;

    // strdup's MUST be deallocated with free().  Hopefully they will
    // be deallocated above when we get new command
    if(rec->phase_line_ack_msg!=NULL)
    	command->phase_line_ack_msg = strdup(rec->phase_line_ack_msg);
    else
    	command->phase_line_ack_msg = strdup("");

    command->wait_at_phase_line = rec->wait_at_phase_line;
    command->completion_given = rec->completion_given;

    // same note as above
    if(rec->completion_msg!=NULL)
    	command->completion_msg = strdup(rec->completion_msg);
    else
    	command->completion_msg = strdup("");

    command->freeze_on_completion = rec->freeze_on_completion;
    command->until_timeout_given = rec->until_timeout_given;
    command->robot_id = rec->robot_id;
    command->seq_num = rec->seq_num;
}


/****************************************************************************/

void new_state(IPCommunicator* comm, IPMessage* msg, void* data)
{
    REC_NewState* rec = (REC_NewState*) msg->FormattedData();

    if (debug)
    {
        fprintf(stderr, "robot(%d) new_state: '%s'='%s'\n", robot_id, rec->key, rec->value);
    }

    put_state(rec->key, rec->value);
}

/****************************************************************************/

void declare_ipt_client (IPCommunicator* comm, IPMessage* msg, void* data)
{
    REC_Declare_iptClient* rec = (REC_Declare_iptClient*) msg->FormattedData();

    int server_id = rec->server_id;

    if (debug)
    {
        fprintf(stderr, "robot(%d) declare_ipt_client: server_id (%d) server_name('%s')\n",
                 robot_id, server_id, rec->server_name);
    }

    if (server_id != robot_id)
    {
        iptClient(communicator, rec->server_name);
    }

}
/****************************************************************************/

void robot_pause(IPCommunicator* comm, IPMessage* msg, void* data)
{
    if (!drawing_vector_field)
    {
        REC_RobotPause* val = (REC_RobotPause *) msg->FormattedData();

        // clear single step counter and flag
        single_step_cycle = 0;
        current_step_cycle = 0;

        // update the paused flag
        paused = *val;

        if (debug)
        {
            fprintf(stderr,"robot(%d) is %s\n",robot_id, paused? "paused":"running");
        }
    }
}

/****************************************************************************/
void robot_feedback(IPCommunicator* comm, IPMessage* msg, void* data)
{
    REC_RobotFeedback *val = NULL;

    val = (REC_RobotFeedback *) msg->FormattedData();

    // update the paused flag
    stopRobotFeedback = *val;
}

/****************************************************************************/

void step_one_cycle(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_StepOneCycle* rec = (REC_StepOneCycle*) msg->FormattedData();

    // run the desired number of cycles (if paused)
    single_step_cycle += (int) *rec;
}

/****************************************************************************/

void robot_halt(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    fprintf(stderr, "Robot %d exiting: Received halt message\n", robot_id);
    if (robot_type == HSERVER)
    {
        hclient_close_robot();
    }
    abort();
    exit(1);
}

/****************************************************************************/

void robot_debug(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_RobotDebug* rec = (REC_RobotDebug*) msg->FormattedData();
    debug = (int) *rec;
}

/****************************************************************************/

void prepare_drawing_vector_field(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    if (!drawing_vector_field)
    {
        old_run_type = run_type;
        run_type = SIMULATION;
        g_lOldCycleLength = g_lCycleLength;
        g_lCycleLength = 1;
        drawing_vector_field = true;
    }
}

/****************************************************************************/

void done_drawing_vector_field(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    if (drawing_vector_field)
    {
        run_type = old_run_type;
        drawing_vector_field = false;
        g_lCycleLength = g_lOldCycleLength;
    }
}

/****************************************************************************/

void report_sensor_readings(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_SensorReadings suSensorReadingsMsg;
    sensor_reading_t* suReadings;
    int iNumReadings;
    int iMask = 0;

    if (run_type == SIMULATION)
    {
        // Only for the real robot.
        return;
    }

    if (use_sonar)
    {
        iMask |= SENSOR_SONAR_MASK;
    }
    if (use_laser)
    {
        iMask |= SENSOR_SICKLMS200_MASK;
    }
    if (use_cognachrome)
    {
        iMask |= SENSOR_COGNACHROME_MASK;
    }
    if (use_generic)
    {
          iMask |= SENSOR_GENERIC_MASK;
    }
    iNumReadings = hclient_get_obstacles(iMask, danger_range, &suReadings, adjust_obstacles);

    suSensorReadingsMsg.robot_id = robot_id;
    suSensorReadingsMsg.num_readings = iNumReadings;
    suSensorReadingsMsg.reading = (REC_SensorReading*) suReadings;

    if (!stopRobotFeedback)
    {
        if (!network_to_console_too_slow(true, "MSG_SensorReadings"))
        {
            communicator->SendMessage(console, MSG_SensorReadings, &suSensorReadingsMsg);
        }
    }

    if (iNumReadings > 0)
    {
        free(suReadings);
    }
}

void carmen_base_velocity_handler(MSG_INSTANCE msgInstance, void *callData, void *clientData)
{
	static float maxRobotVelocity = 0;
	carmen_base_velocity_message* rec = (carmen_base_velocity_message*)callData;
	FORMATTER_PTR formatter = IPC_msgInstanceFormatter(msgInstance);

	if(maxRobotVelocity==0)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		IPC_setContext(centralServerConnection);

		carmen_param_query_message query;
		carmen_param_response_double_message *response;

		query.timestamp 		= 0;
		query.host 			= "";
		query.module_name 	= "robot";
		query.variable_name 	= "max_t_vel";

		IPC_queryResponseData(CARMEN_PARAM_QUERY_DOUBLE_NAME, &query,
				(void **) &response, 5000);

		maxRobotVelocity = response->value;
		free(response->module_name);
		free(response->variable_name);
		free(response->host);
		free(response);

		if(context != NULL)
			IPC_setContext(context);
	}

	if(rec->tv == 0)
	{
		if(rec->rv == 0)
		{
			carmenAdvise.x = 0;
			carmenAdvise.y = 0;
		}
		else
		{
			carmenAdvise.x = -1; // cos(M_PI);
			carmenAdvise.y =  0; // sin(M_PI);
		}
	}
	else
	{
		double module = rec->tv / maxRobotVelocity;

		double angle = rec->rv * 180.0 / M_PI;
		if(angle>90.0)
			angle = 90.0;
		else if(angle<-90.0)
			angle = -90.0;

		angle = angle / 90.0;
		angle = asin(angle);

		carmenAdvise.x = module * cos(angle);
		carmenAdvise.y = module * sin(angle);

		//printf("TV = %g --- RV = %g --- Module = %g --- Angle = %g\n",
		//		rec->tv, rec->rv*180/M_PI, module, angle);
	}
	IPC_freeData(formatter, callData);
}


void obtener_sensores(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_DevolverSensores lecturas;
    //sensor_reading_t* suReadings;
    int iNumReadings;
    int iMask = 0xFFFF;
    //static int inicial = 0;

    sensor_t *sensores;


    if (run_type == SIMULATION)
    {
        // Only for the real robot.
        return;
    }

//     if (use_sonar)
//     {
//         iMask |= SENSOR_SONAR_MASK;
//     }
//     if (use_laser)
//     {
//         iMask |= SENSOR_SICKLMS200_MASK;
//     }
//     if (use_cognachrome)
//     {
//         iMask |= SENSOR_COGNACHROME_MASK;
//     }

//	hclient_get_sensor_readings(iMask,&iNumReadings, &sensores);
    sensores = sensor;
    iNumReadings = num_sensors;

//     iNumReadings = hclient_get_obstacles(iMask, danger_range, &suReadings, adjust_obstacles);



    lecturas.robotId = robot_id;
    lecturas.numLecturas = iNumReadings;
    lecturas.sensor = (REC_Sensor *)malloc(lecturas.numLecturas * sizeof(REC_Sensor));

    int j = 0;

    for(int i = 0; i < MAX_SENSORS ; i++){

	    //printf("Valor de sensores %d type = %d\n",i, sensores[i].type);

		if(sensores[i].type != -1){


			lecturas.sensor[j].type = sensores[i].type;
		    lecturas.sensor[j].num_readings = sensores[i].num_readings;

			// Cargar datos del laser (angulos y valores)
			lecturas.sensor[j].num_angles = sensores[i].num_angles;
			lecturas.sensor[j].angle = sensores[i].angle;


		    //lecturas.sensor[j].reading = (float *)malloc(lecturas.sensor[j].num_readings * sizeof(float));
		    //memcpy(lecturas.sensor[j].reading, sensores[i].reading, lecturas.sensor[j].num_readings);

			/*if(sensores[i].type == 15)
				for(int h=0;h<17;h++)
					printf("Sensor %d valor %f\n",h,sensores[i].reading[h]);*/

		    lecturas.sensor[j].reading = sensores[i].reading;
		    j ++;
	    }
    }

	//lecturas.sensor = (REC_Sensor *)sensores;

    //Reserva y copia de los 20 sensores para intentar evitar que la memoria se corrompa
    //memcpy(lecturas.sensor,(REC_Sensor *)sensores,sizeof(REC_Sensor) * 20);


    //printf("Numero de lecturas %d, numero de elementos del sensor %d\n", lecturas.numLecturas, lecturas.sensor[3].num_readings);
    //printf("Valores : %f %f %f\n", lecturas.sensor[3].reading[0],lecturas.sensor[3].reading[1],lecturas.sensor[3].reading[2]);

    if (!stopRobotFeedback)
    {
		time_t actualTime;


       if (!network_to_console_too_slow(true, "MSG_DevolverSensores"))
        {

		time ( &actualTime );
		printf("Enviando datos...%s\n",ctime (&actualTime));

               communicator->SendMessage(console, MSG_DevolverSensores, &lecturas);
	      //communicator->Reply(msg, MSG_DevolverSensores, &lecturas);

		time ( &actualTime );
		printf("Datos enviados...%s\n",ctime (&actualTime));
       }
    }


	/*for(int i=0; i< lecturas.numLecturas; i++)
		free(lecturas.sensor[i].reading);*/

	free(lecturas.sensor);


//     if (iNumReadings > 0)
//     {
//         free(suReadings);
//     }
}

//-----------------------------------------------------------------------
// Update the console with any config info. Also, this will ask the
// console to link robots, so that they can communicate each other.
//-----------------------------------------------------------------------
void exec_notify_config(void)
{
    REC_NotifyConfig cfg;
    cfg.robot_id = robot_id;
    cfg.robot_type = robot_type;
    cfg.run_type = run_type;

    communicator->SendMessage(console, MSG_NotifyConfig, &cfg);
}

//-----------------------------------------------------------------------
// This function sends (injects) the CNP task constraints to the mlab.
//-----------------------------------------------------------------------
void exec_send_cnp_task_constraints(
    vector<CNP_TaskConstraint> cnpTaskConstraints)
{
    REC_CNP_TaskConstraintsData rec;
    string infoNameData = EMPTY_STRING, nameData = EMPTY_STRING, valueData = EMPTY_STRING;
    string notifyMessage;
    int i;

    // First, tell database in all robots that the auction is not failed or reneged.
    notifyMessage = CNP_NOTIFY_TASK_AUCTION_FAILED_MSG;
    exec_broadcast_value((char *)(notifyMessage.c_str()), "0", THIS_ROBOT);

    // Get the number of the constraints.
    rec.numConstraints = cnpTaskConstraints.size();

    // Allocate the data sizes
    rec.taskInfoIDs = new int[rec.numConstraints];
    rec.taskInfoNameSizes = new int[rec.numConstraints];
    rec.nameSizes = new int[rec.numConstraints];
    rec.types = new int[rec.numConstraints];
    rec.valueSizes = new int[rec.numConstraints];
    rec.taskInfoTotalNameSize = 0;
    rec.totalNameSize = 0;
    rec.totalValueSize = 0;

    for (i = 0; i < (rec.numConstraints); i++)
    {
        // Get the info
        rec.taskInfoIDs[i] = cnpTaskConstraints[i].taskInfo.id;
        rec.taskInfoNameSizes[i] = cnpTaskConstraints[i].taskInfo.name.size();
        rec.taskInfoTotalNameSize += rec.taskInfoNameSizes[i];
        infoNameData += cnpTaskConstraints[i].taskInfo.name;

        // Get the name size and its content.
        rec.nameSizes[i] = cnpTaskConstraints[i].strConstraintName.size();
        rec.totalNameSize += rec.nameSizes[i];
        nameData += cnpTaskConstraints[i].strConstraintName;

        // Get the type.
        rec.types[i] = cnpTaskConstraints[i].constraintType;

        // Get the value size and its conent.
        rec.valueSizes[i] = cnpTaskConstraints[i].strConstraintValue.size();
        rec.totalValueSize += rec.valueSizes[i];
        valueData += cnpTaskConstraints[i].strConstraintValue;
    }

    // Copy the info
    rec.taskInfoNameData = new char[(rec.taskInfoTotalNameSize)+1];
    memcpy(rec.taskInfoNameData, infoNameData.c_str(), rec.taskInfoTotalNameSize);
    rec.taskInfoNameData[rec.taskInfoTotalNameSize] = '\0';

    // Copy the constraint name data.
    rec.nameData = new char[(rec.totalNameSize)+1];
    memcpy(rec.nameData, nameData.c_str(), rec.totalNameSize);
    rec.nameData[rec.totalNameSize] = '\0';

    // Copy the value data.
    rec.valueData = new char[(rec.totalValueSize)+1];
    memcpy(rec.valueData, valueData.c_str(), rec.totalValueSize);
    rec.valueData[rec.totalValueSize] = '\0';

    // Send to the task.
    communicator->SendMessage(console, MSG_CNP_TaskConstraintsData, &rec);

    // Clean up the data.
    delete [] rec.taskInfoIDs;
    delete [] rec.taskInfoNameSizes;
    delete [] rec.taskInfoNameData;
    delete [] rec.nameSizes;
    delete [] rec.nameData;
    delete [] rec.types;
    delete [] rec.valueSizes;
    delete [] rec.valueData;
}

//-----------------------------------------------------------------------
// This function retrieves the robot constraints from console.
//-----------------------------------------------------------------------
void exec_receive_cnp_robot_constraints(void)
{
    REC_CNP_RobotConstraintsRequest req;
    REC_CNP_RobotConstraintsData *rec = NULL;
    IPMessage *msg = NULL;
    CNP_RobotConstraint robotConstraint;
    list<CNP_RobotConstraint> cnpRobotConstraints;
    list<CNP_RobotConstraint>::const_iterator iterRC;
    int i, numConstraints;
    int totalNameSize, totalValueSize, nameSize, valueSize;
    char *nameData = NULL, *valueData = NULL;
    char *nameDataInitPtr = NULL, *valueDataInitPtr = NULL;
    char *buf = NULL;
    const bool DEBUG_CNP_ROBOT_CONSTRAINTS = false;

    if (gRobotCNP == NULL)
    {
        fprintf(stderr, "Error in exec_receive_cnp_robot_constraints(). gRobotCNP is NULL.\n");
        return;
    }

    // Request the robot constraints
    req.robotID = robot_id;
    msg = communicator->Query(
        console,
        MSG_CNP_RobotConstraintsRequest,
        &req,
        MSG_CNP_RobotConstraintsData);
    rec = (REC_CNP_RobotConstraintsData *)msg->FormattedData();

    // Get some key numbers
    numConstraints = rec->numConstraints;
    totalNameSize = rec->totalNameSize;
    totalValueSize = rec->totalValueSize;

    // Copy the data
    nameData = new char[totalNameSize+1];
    nameDataInitPtr = nameData;
    memcpy(nameData, rec->nameData, totalNameSize);

    valueData = new char[totalValueSize+1];
    valueDataInitPtr = valueData;
    memcpy(valueData, rec->valueData, totalValueSize);

    for (i = 0; i < numConstraints; i++)
    {
        // Get the constraint name
        nameSize = rec->nameSizes[i];
        buf = new char[nameSize+1];
        memcpy(buf, nameData, nameSize);
        buf[nameSize] = '\0';
        robotConstraint.strConstraintName = buf;
        delete [] buf;
        nameData += nameSize;

        // Copy the constraint type
        robotConstraint.constraintType = (CNP_ConstraintType)(rec->types[i]);

        // Copy the constraint value
        valueSize = rec->valueSizes[i];
        buf = new char[valueSize+1];
        memcpy(buf, valueData, valueSize);
        buf[valueSize] = '\0';
        robotConstraint.strConstraintValue = buf;
        delete [] buf;
        valueData += valueSize;

        cnpRobotConstraints.push_back(robotConstraint);
    }

    // Save the constraints.
    gRobotCNP->saveRobotConstraints(cnpRobotConstraints);

    if (debug || DEBUG_CNP_ROBOT_CONSTRAINTS)
    {
        fprintf(stderr, "robot(%d): CNP Robot Constraints\n", robot_id);

        i = 0;

        for (iterRC = cnpRobotConstraints.begin();
             iterRC != cnpRobotConstraints.end();
             iterRC++)
        {

            fprintf(
                stderr,
                " [%d] {%s, %s}\n",
                i++,
                iterRC->strConstraintName.c_str(),
                iterRC->strConstraintValue.c_str());
        }
    }

    // Clean up
    delete [] nameDataInitPtr;
    delete [] valueDataInitPtr;
    delete msg;
}

/****************************************************************************
*                                                                           *
*  report_state                                                             *
*   * This routine receives data from mlab-console to examine wheter        *
*     the robot should report what state currently it is in.                *
*                                                                           *
*  Inputs:                                                                  *
*     comm      = some IPT variable                                         *
*     msg       = some IPT variable                                         *
*     hnddata   = contains either true or false about reporting state.      *
*                                                                           *
****************************************************************************/
void report_state(IPCommunicator* comm, IPMessage* msg, void* hnddata)
{
    REC_ReportState* rec = (REC_ReportState*) msg->FormattedData();
    report_current_state = (int) *rec;
}

/****************************************************************************
* jboxArgsToIDs                                                                    *
*                                                                           *
****************************************************************************/
vector<int> jboxArgsToIDs(char *args)
{
    vector<int> idList;
    string argString, numString;
    int i;

    if (args == NULL)
    {
        return idList;
    }

    argString = args;
    numString = "";

    for (i = 0; i < (int)(argString.size()); i++)
    {
        if (argString[i] == ':')
        {
            idList.push_back(atoi(numString.c_str()));
            numString = "";
        }
        else
        {
            numString += argString[i];
        }
    }

    idList.push_back(atoi(numString.c_str()));

    return idList;
}

/****************************************************************************
* usage:                                                                    *
*                                                                           *
****************************************************************************/
void usage(char* name)
{
    fprintf(stderr, "usage: %s -i robot_id -c console_name [-n robot_name] [OPTION] "
             "[-v] [-w] -S -L [-x startx] [-y starty]\n\n", name);
    fprintf(stderr, "       -i nnn   : set the robot's ID (must be unique)\n");
    fprintf(stderr, "       -c       : name used to connect to the simulation\n\n");
    fprintf(stderr, "[OPTION]\n");
    fprintf(stderr, "       -h       : print this usage message\n");
    fprintf(stderr, "       -v       : verbose mode: turn on debugging\n");
    fprintf(stderr, "       -w       : wait for keypress before starting execution.\n");
    fprintf(stderr, "                  Allows attaching to thread with debugger\n");
    fprintf(stderr, "       -S       : reports what state the robot is in\n");
    fprintf(stderr, "       -L       : turn on the robot data logging\n");
    fprintf(stderr, "       -n str   : set the robot's name\n");
    fprintf(stderr, "       -s nnn   : set the robot's random number seed\n");
    fprintf(stderr, "       -t str   : set the IPT server's network address\n");
    fprintf(stderr, "       -u       : display the cpu usage\n");
    fprintf(stderr, "       -x nn.nn : set starting x location\n");
    fprintf(stderr, "       -y nn.nn : set starting y location\n");
    fprintf(stderr, "       -h nn.nn : set starting heading\n");
    fprintf(stderr, "       -m       : turn on learning momentum weight loggin\n");
    fprintf(stderr, "       -r       : turn on sensor reading logging\n");

    fprintf(stderr, "\n");
}


/* Disconnection callback */
void iptDisconnectCallback(IPConnection* conn, void* data)
{
    fprintf(stderr, "IPT: Lost connection with %s from %s\n", conn->Name(), conn->Host());

    const char * name = conn->Name();
    const char * hserverPrefix = "hserver";
    int tam = strlen(hserverPrefix);

    int isHserver = 1;
    for(int i=0; i<tam; i++)
    {
    	if(name[i] != hserverPrefix[i])
    	{
    		isHserver = 0;
    		break;
    	}
    }

    //Now, CARMEN modules can be connected to the mission.
    //This is to avoid exit if one CARMEN module disconnects
    if(isHserver)
    {
    	if (robot_type == HSERVER)
    	{
    		hclient_close_robot();
    	}
    	abort();
    	exit(-1);
    }
}


float modulo(Vector p1, Vector p2){
    return sqrt(pow(p1.x-p2.x,2) + pow(p1.y-p2.y,2));
}

/*Manejadora para el mensaje de broadCast enviado por el lider
*/
void conocer_robots(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
	robotLider = comm->Connect((char *)(*((char **)(msg->FormattedData()))));
	if(escuchoBroadCast == true){
        escuchoBroadCast = false;
    	sigo_lider();
    }
}

/*Manejadora para la contestación de los esclavos al mensaje de broadCast*/
void te_sigo(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    int rep = 0;
    IPConnection * connection = comm->Connect((char *)(*((char **)(msg->FormattedData()))));

    for(int i=0; i< numEsclavos; i++){
        if(robotsEsclavos[i] == connection)
            rep = 1;
    }
    if(rep == 0){
        robotsEsclavos[numEsclavos] = connection;
        numEsclavos ++;
    }
}

/*Manejadora del mensaje de cambio de estado*/
void mensaje_cambio_estado(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    cambioEstado = true;
}
/*Manejadora que devuelve la posición a la que debe ir el esclavo*/
void posicion_deseada_solicitud(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PosicionDeseadaContestacion rsp;
    rsp.x = posicionEsclavos.x;
    rsp.y = posicionEsclavos.y;
    rsp.z = posicionEsclavos.z;

    communicator->Reply(msg, MSG_PosicionDeseadaContestacion, &rsp);
}

/*Manejadora de la consulta de la posición del lider*/
void get_posicion_lider(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PosicionLider rsp;
    robot_position pos;

    pos = exec_get_position();
    rsp.posicion.x = pos.v.x;
    rsp.posicion.y = pos.v.y;
    rsp.posicion.z = pos.v.z;
    rsp.heading = pos.heading;

    communicator->Reply(msg, MSG_PosicionLider, &rsp);
}

/****************************************************************************
* init:                                                                     *
****************************************************************************/
void init(int argc, char** argv)
{
    REC_RobotBirth rec;
    double start_x = 0.0, start_y = 0.0, start_heading = 0.0;
    char keyPress;
    char our_ipt_module[256];
    char console_ipt_module[256];
    char hostname[100];
    char msg[256];
    char *ipt_home = NULL;
    char *value = NULL;
    int seed;
    int c, i;
    int numHandlers;
    bool wait_for_ack = false;
    extern char* optarg;
    extern int opterr;

    seed = getpid();

    // setup the defaults
    strcpy(console_ipt_module, DefaultConsoleName);

    strncpy(robot_name, argv[0], 200);
    robot_name[200] = '\0';

    // now see what they specified
    opterr = 0;
    robot_id = -1;

    for (i = 0; i < 100; i++)
    {
        bPingHistory[i] = true;
    }


    while ((c = getopt(argc, argv, "c:f:h:i:J:l:Lmn:Nrs:St:uvwx:y:")) != -1)
    {
        switch (c) {

        case 'c':
            strncpy(console_ipt_module, optarg, 200);
            console_ipt_module[200] = '\0';
            break;

        case 'f':
            stopRobotFeedback = !atoi(optarg);
            break;

        case 'h':
            sscanf(optarg, "%lf", &start_heading);
            break;

        case 'i':
            robot_id = (int) atof(optarg);
            break;

        case 'J':
            allowedRobotJBoxIDs = jboxArgsToIDs(optarg);
            break;

        case 'l':
            laserfit_machine_name = strdup(optarg);
            break;

        case 'L':
            log_robot_data = true;
            printf("Robot data logging enabled. Data will be recorded in %s.log\n", robot_name);
            break;

        case 'm':
            save_learning_momentum_weights = true;
            break;

        case 'n':
            strncpy(robot_name, optarg, 200);
            robot_name[200] = '\0';
            break;

        case 'N':
            enableRuntimeCNP = true;
            break;

        case 'r':
            save_sensor_readings = true;
            break;

        case 's':
            seed = atoi(optarg);
            break;

        case 'S':
            report_current_state = true;
            printf("Report current state enabled\n");
            break;

        case 't':
            ipt_home = strdup(optarg);
            break;

        case 'u':
            show_cycle_duration = true;
            break;

        case 'v':
            debug = true;
            printf("Debug enabled\n");
            break;

        case 'w':
            wait_for_ack = true;
            break;

        case 'x':
            sscanf(optarg, "%lf", &start_x);
            break;

        case 'y':
            sscanf(optarg, "%lf", &start_y);
            break;

        default:
            usage(argv[0]);
            break;
        }
    }

    if (robot_id == -1)
    {
        fprintf(stderr, "Error: unique robot_id must be specified\n");
        exit(1);
    }

    // start in pause state
    paused = true;

    // build a unique name for ourselves
    sprintf(our_ipt_module, "%s_%d_%d", robot_name, getpid(), robot_id);

    // if the user didn't specify a host machine, look in the environment
    if (ipt_home == NULL)
    {
        ipt_home = getenv("IPTHOST");
    }

    if (ipt_home == NULL)
    {
        ipt_home = getenv("TCXHOST");
    }

    // if none specified, the current machine will be the default
    if (ipt_home == NULL)
    {
        ipt_home = getenv("HOST");
    }

    // if HOST wasn't set either, use the hostname as the last resort
    if (ipt_home == NULL)
    {
        if (gethostname(hostname, 100) != 0)
        {
            fprintf(stderr, "Aborting: Unable to determine host for IPT server!\n");
            exit(1);
        }

        ipt_home = hostname;
    }

    // setup ipt connection
    fprintf(stderr, "Initialize IPT (%s, %s)\n", our_ipt_module, ipt_home);

    communicator = IPCommunicator::Instance(our_ipt_module, ipt_home);

    // connect to console module
    console = communicator->Connect(console_ipt_module, IPT_REQUIRED);

    // becomes a server to communicate with other robots.
    communicator->Server();

    // register message formats
    communicator->RegisterNamedFormatters(formatArray);

    // register message formats
    communicator->RegisterMessages(messageArray);

    // register handlers
    IPMsgHandlerSpec hndArray[] = {
        { MSG_NewCommand,             new_command,                  IPT_HNDL_ALL, NULL},
        { MSG_NewState,               new_state,                    IPT_HNDL_ALL, NULL},
        { MSG_Declare_iptClient,      declare_ipt_client,           IPT_HNDL_ALL, NULL},
        { MSG_RobotPause,             robot_pause,                  IPT_HNDL_ALL, NULL},
        { MSG_RobotFeedback,          robot_feedback,               IPT_HNDL_ALL, NULL},
        { MSG_StepOneCycle,           step_one_cycle,               IPT_HNDL_ALL, NULL},
        { MSG_RobotSuicide,           robot_halt,                   IPT_HNDL_ALL, NULL},
        { MSG_ReportState,            report_state,                 IPT_HNDL_ALL, NULL},
        { MSG_RobotDebug,             robot_debug,                  IPT_HNDL_ALL, NULL},
        { MSG_DrawingVectorField,     prepare_drawing_vector_field, IPT_HNDL_ALL, NULL},
        { MSG_DoneDrawingVectorField, done_drawing_vector_field,    IPT_HNDL_ALL, NULL},
        { MSG_ReportSensorReadings,   report_sensor_readings,       IPT_HNDL_ALL, NULL},
        { MSG_ConocerRobots,          conocer_robots,               IPT_HNDL_ALL, NULL},
        { MSG_TeSigo,                 te_sigo,                      IPT_HNDL_ALL, NULL},
        //{ MSG_PosicionLider,        posicion_lider,               IPT_HNDL_ALL, NULL},
        { MSG_CambioEstado,           mensaje_cambio_estado,        IPT_HNDL_ALL, NULL},
        { MSG_PosicionDeseadaSolicitud, posicion_deseada_solicitud, IPT_HNDL_ALL, NULL},
        { MSG_GetPosicionLider,       get_posicion_lider,           IPT_HNDL_ALL, NULL},

        //Añadido por alber y teny
		{ MSG_ObtenerSensores,	      obtener_sensores,  	    	IPT_HNDL_ALL, NULL},
    };

    numHandlers = (int)(sizeof(hndArray) / sizeof(IPMsgHandlerSpec));
    for(i = 0; i < numHandlers; i++)
    {
        IPMessageType* msg_type = communicator->LookupMessage(hndArray[i].msg_name);
        communicator->RegisterHandler(
            msg_type,
            hndArray[i].callback,
            hndArray[i].data,
            hndArray[i].context);
    }

    communicator->AddDisconnectCallback(iptDisconnectCallback, NULL);

    // notify the console that we are alive
    rec.start_x = start_x;
    rec.start_y = start_y;
    rec.start_z = 0;
    rec.start_heading = start_heading;
    rec.robot_id = robot_id;
    rec.port_name = our_ipt_module;
    rec.com_version = ComVersion;

    if (debug)
    {
        fprintf(
            stderr,
            "robot(%d): MSG_RobotBirth <%.1f %.1f> %s\n",
            rec.robot_id,
            rec.start_x,
            rec.start_y,
            rec.port_name);
    }

    communicator->SendMessage(console, MSG_RobotBirth, &rec);

    // Make sure pause is off before proceeding
    // to ensure that all robots are born before some start running
    // This solves a race condition in the signal handling code where
    // a robot could be born and die again before all robots started.

    while (paused)
    {
        // If have received a single step message, then go on and run one cycle
        if (single_step_cycle > current_step_cycle)
        {
            current_step_cycle++;
            break;
        }

        // run message handlers
        while (!(communicator->Idle(ZERO_TIME)));
    }

    // Set defaults
    run_type = SIMULATION;
    robot_type = HOLONOMIC;

    if ((value = return_state("wait_for_ack")) != NULL)
    {
        wait_for_ack = atoi(value);

        if (debug)
        {
            fprintf(stderr, "wait_for_ack = '%s' : %d\n", value, wait_for_ack);
        }
    }
    else if (debug)
    {
        fprintf(stderr, "Using default wait_for_ack = %d\n", wait_for_ack);
    }

    // Pause if requested before starting to aid debugging.
    if (wait_for_ack)
    {
        fprintf(stderr, "Robot %s starting at location %f, %f\n",
                 robot_name, start_x, start_y);

        fprintf(stderr, "Please press CR to continue");
        scanf("%c", &keyPress);
    }

    // Now the startup parameters have been sent, so we can get them
    if ((value = return_state(ROBOT_TYPE_MSG)) != NULL)
    {
        for(i = 0; i < NUM_ROBOT_TYPES; i++)
        {
            if (value == ROBOT_TYPE_NAMES[i])
            {
                robot_type = i;
                break;
            }
        }

        if (debug)
        {
            fprintf(stderr, "robot_type = '%s' : %d\n", value, robot_type);
        }

        if (i >= NUM_ROBOT_TYPES)
        {
            sprintf(msg, "Unknown robot type: '%s'='%s'", ROBOT_TYPE_MSG, value);
            halt(msg);
        }
    }
    else
    {
        fprintf(stderr, "Using default robot_type = 'HOLONOMIC'\n");
    }

    if ((value=return_state(RUN_TYPE_MSG)) != NULL)
    {
        for(i = 0; i < NUM_RUN_TYPES; i++)
        {
            if (value == RUN_TYPE_NAMES[i])
            {
                run_type = i;
                break;
            }
        }

        if (debug)
        {
            fprintf(stderr, "run_type = '%s' : %d\n", value, run_type);
        }

        if (i >= NUM_RUN_TYPES)
        {
            sprintf(msg, "Unknown run type: '%s'='%s'", RUN_TYPE_MSG, value);
            halt(msg);
        }
    }
    else
    {
        fprintf(stderr, "Using default run_type = 'SIMULATION'\n");
    }


    if (run_type == REAL)
    {
        if ((value=return_state("draw_obstacles")) != NULL)
        {
            draw_obstacles = atoi(value);
            if (debug)
            {
                fprintf(stderr, "draw_obstacles = '%s' : %d\n", value, draw_obstacles);
            }
        }
        else if (debug)
        {
            fprintf(stderr, "Using default draw_obstacles = %d\n", draw_obstacles);
        }

        if ((value=return_state("adjust_obstacles")) != NULL)
        {
            adjust_obstacles = atoi(value);

            if (debug)
            {
                fprintf(
                    stderr,
                    "adjust_obstacles = '%s' : %d\n",
                    value,
                    adjust_obstacles);
            }
        }
        else if (debug)
        {
            fprintf(stderr, "Using default adjust_obstacles = %d\n", adjust_obstacles);
        }

        if ((value=return_state("lurch_mode")) != NULL)
        {
            lurch_mode = atoi(value);
            if (debug)
            {
                fprintf(stderr, "lurch_mode = '%s' : %d\n", value, lurch_mode);
            }

            if (lurch_mode)
            {
                fprintf(stderr, "Using lurch_mode\n");
            }
        }
        else if (debug)
        {
            fprintf(stderr, "robot(%d): Using default lurch_mode = %d\n",
                     robot_id, lurch_mode);
        }

        if ((value=return_state("use_reverse")) != NULL)
        {
            use_reverse = atoi(value);

            if (debug)
            {
                fprintf(stderr, "use_reverse = '%s' : %d\n", value, use_reverse);
            }

            if (use_reverse)
            {
                fprintf(stderr, "Using reverse\n");
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default reverse = %s\n",
                robot_id,
                (use_reverse)? "ON": "OFF");
        }

        if ((value=return_state("wait_for_turn")) != NULL)
        {
            wait_for_turn = atoi(value);

            if (debug)
            {
                fprintf(stderr, "wait_for_turn = '%s' : %d\n", value, wait_for_turn);
            }
        }
        else if (debug)
        {
            fprintf(stderr, "robot(%d): Using default wait_for_turn = %s\n",
                     robot_id, (wait_for_turn)? "ON": "OFF");
        }

        if ((value=return_state("multiple_hservers")) != NULL)
        {
            multiple_hservers = atoi(value);

            if (debug)
            {
                fprintf(
                    stderr,
                    "multiple_hservers = '%s' : %d\n",
                    value,
                    multiple_hservers);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default multiple_hservers = %s\n",
                robot_id,
                (multiple_hservers)? "ON": "OFF");
        }

        if ((value=return_state("use_laser")) != NULL)
        {
            use_laser = atoi(value);

            if (debug)
            {
                fprintf(stderr, "use_laser = '%s' : %d\n", value, use_laser);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default use_laser = %s\n",
                robot_id,
                (use_laser)? "ON": "OFF");
        }

        if ((value=return_state("use_cognachrome")) != NULL)
        {
            use_cognachrome = atoi(value);

            if (debug)
            {
                fprintf(
                    stderr,
                    "use_cognachrome = '%s' : %d\n",
                    value,
                    use_cognachrome);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default use_cognachrome = %s\n",
                robot_id,
                (use_cognachrome)? "ON": "OFF");
        }

        if ((value=return_state("use_sonar")) != NULL)
        {
            use_sonar = atoi(value);

            if (debug)
            {
                fprintf(stderr, "use_sonar = '%s' : %d\n", value, use_sonar);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default use_sonar = %s\n",
                robot_id,
                (use_sonar)? "ON": "OFF");
        }

        if ((value=return_state("danger_range")) != NULL)
        {
            danger_range = atof(value);

            if (debug)
            {
                fprintf(stderr, "danger_range = '%s' : %f\n", value, danger_range);
            }
        }
        else if (debug)
        {
            fprintf(stderr, "robot(%d): Using default danger_range = %f\n",
                     robot_id, danger_range);
        }

        if ((value=return_state("drive_wait_angle")) != NULL)
        {
            drive_wait_angle = atof(value);

            if (debug)
            {
                fprintf(
                    stderr,
                    "drive_wait_angle = '%s' : %f\n",
                    value,
                    drive_wait_angle);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default drive_wait_angle = %f\n",
                robot_id,
                drive_wait_angle);
        }

        if ((value = return_state("hserver_name")) != NULL)
        {
            strcpy(hserver_name, value);

            if (debug)
            {
                fprintf(stderr, "hserver_name = %s\n", hserver_name);
            }
        }
        else if (debug)
        {
            fprintf(
                stderr,
                "robot(%d): Using default hserver_name = %s\n",
                robot_id,
                hserver_name);
        }

        if (hclient_open_robot(hserver_name, multiple_hservers))
        {
            halt("Error initializing robot hardware");
        }

        if (hclient_range_start())
        {
            halt("Error starting ultrasonics");
        }

        if (hclient_init_robot(start_x, start_y, start_heading))
        {
            char msg[80];
            sprintf(msg, "Error calling hclient_init_robot(%f, %f, %f)\n", start_x, start_y, 0.0);
            halt(msg);
        }

        hclient_update_sensors();

        if (debug)
        {
            fprintf(stderr, "Robot initialized\n");
        }
    }

#ifndef IGNORE_SIGNALS
    // Catch control-c and others to ensure that we kill the robots
    signal(SIGINT,  sig_catch);
    signal(SIGBUS,  sig_catch);
    signal(SIGHUP,  sig_catch);
    signal(SIGSEGV, sig_catch);
    signal(SIGTERM, sig_catch);
#endif

    // update the console with any config info
    exec_notify_config();

    if (enableRuntimeCNP)
    {
        gRobotCNP = new RobotCNP();
        exec_receive_cnp_robot_constraints();
    }

    if (debug)
    {
        fprintf(
            stderr,
            "Robot(%d): Birth at (%f %f) on %s\n",
            robot_id,
            start_x,
            start_y,
            gethostname(hostname, 100) ? "unknown" : hostname);
    }

    srand(seed);
}

/*---------------------------------------------------------------------

				CORRECTSIDE

	Determines if a point is on the correct side of a line
	(in this case using the right hand rule from pt1 to pt2)
	Planar case - PT3 to left of vector from pt1 to pt2
	IF POINT IS COLINEAR WITH LINE RETURNS false  (not inclusive)

------------------------------------------------------------------------*/
int correctside(double xx1, double yy1, double xx2, double yy2, double xx3, double yy3)
{
    const int RIGHT = 1;
    const int WRONG = 2;
    int flag;

     // Translate points to x1 = (0, 0)
    double x2 = xx2 - xx1;
    double y2 = yy2 - yy1;
    double x3 = xx3 - xx1;
    double y3 = yy3 - yy1;

    // special cases first
    if (x2 == 0) // slope not computable
    {
        if (y2 > 0)
        {
            flag = (x3 >= 0) ? WRONG : RIGHT;
        }
        else
        {
            flag = (x3 <= 0) ? WRONG : RIGHT;
        }
    }
    else if (y2 == 0)
    {
        if (x2 > 0)
        {
            flag = (y3 > 0) ? RIGHT : WRONG; // x3 cannot = 0 since x1=(0, 0)
        }
        else
        {
            flag = (y3 < 0) ? RIGHT : WRONG;
        }
    }
    // normal case
    else
    {
        double slope = y2 / x2;
        double new_x = y3 / slope;
        if (y2 > 0)
        {
            flag = (new_x <= x3) ? WRONG : RIGHT;
        }
        else
        {
            flag = (new_x >= x3) ? WRONG : RIGHT;
        }
    }

    return (flag == RIGHT);
}


void posicion_deseada_contestacion(IPCommunicator* comm, IPMessage* msg, void* hndData)
{
    REC_PosicionDeseadaContestacion* rec = (REC_PosicionDeseadaContestacion*) msg->FormattedData();
    posicionIr = *rec;
}

/*Calcula el vector de anvance del robot Roomba*/
Vector exec_calcular_vector_roomba(Vector v, double location_gain, double obstacle_gain, double sphere)
{
    REC_MoveRobot rec;
    struct timeval t1, t2;
    sensor_t *sensor;
    sensor_reading_t* reading;
    int num_readings;
    int mask = 0;
    float ang;
    robot_position rp;
    float margen = 0.2;
    Vector dev;
    static int estado = 0;
    static int lado = 0;
    static int flag = 0;
    static int mem = BUFFER;
    static double controlarY = 0;

    /**
     * Orden de almacenamiento de los datos provenientes de los sensores
     * 0 -> Bumper Izquierdo
     * 1 -> Bumper Derecho
     * 2 -> Cliff Izquierdo
     * 3 -> Cliff Frontal Izquierdo
     * 4 -> Cliff Frontal Derecho
     * 5 -> Cliff Derecho
     * 6 -> Sensor de pared
     * 7 -> Caida rueda Izquierda
     * 8 -> Caida rueda Central
     * 9 -> Caida rueda Derecha
     */

    /**
      * ESTADOS MAQ ESTADOS:
      * ===================
      * Estado 0: avanzar hacia objetivo
      * Estado 1: retroceder
      * Estado 2: impacto zona izquierda, girar hacia la derecha
      * Estado 3: impacto zona derecha, girar hacia la izquierda
      * Estado 4: impacto zona central, girar hacia el objetivo
      * Estado 5: rueda caida parada.
      *
      * VALORES FLAG EVENTOS:
      * ====================
      * 1: Evento izquierdo
      * 2: Evento derecho
      * 3: Evento central
      *
      **/

    if (debug)
    {
        fprintf(stderr, "exec_mover_roomba(%.2f, %.2f)\n", v.x, v.y);
    }

    rp = exec_get_position();

    if (run_type == REAL)
    {

        hclient_get_sensor_readings(0xFFFF, &num_readings, &sensor);

        for(int j=0; j< sensor[SENSOR_ROOMBA].num_readings; j++)
        {
            switch(j)
            {
                case 0:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        // Pasa al estado 1 (retroceder)
                        estado = 1;
                        // y se indica que el evento ha sucedido en la parte izquierda
                        flag = 1;
                    }
                    break;
                case 1:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        estado = 1;
                        flag = 2;
                    }
                    break;
                case 2:
                if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        estado = 1;
                        if(flag == 2)
                            flag = 3;
                        else
                            flag = 1;
                }
                break;
                case 3:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        estado = 1;
                        if(flag == 2)
                        flag = 3;
                        else
                            flag = 1;
                    }
                    break;
                case 4:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        estado = 1;
                        if(flag == 1 || flag == 3)
                            flag = 3;
                        else
                            flag = 2;
                    }
                    break;
                case 5:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        estado = 1;
                        if(flag == 1 || flag == 3)
                            flag = 3;
                        else
                            flag = 2;
                    }
                    break;
                case 6:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1){
                        if(estado == 0)
                            estado =6;
                    }
                    break;
                case 7:
                case 8:
                case 9:
                    if(sensor[SENSOR_ROOMBA].reading[j] < 0.1)
                        estado = 5;
                    break;
                }

        }

        lado = flag;

        /*La variable mem permite controlar el valor objetivo de la Y para que no vuelva bruscamente a su valor original.*/

        switch (estado)
        {
            case 0://Avanzar hacia el objetivo
                dev.x = v.x;
                dev.y = v.y;
                dev.z = 0;
                //printf("\n\n");
                break;

            case 1://Choque => Ir hacia atrás
                dev.x = -1.0;
                dev.y = 0.0;
                dev.z = 1.0;

                mem = BUFFER;
                if(lado == 1)
                    estado = 2;
                if(lado == 2)
                    estado = 3;
                if(lado == 3)
                    estado = 4;

                controlarY = v.y * 3;

                break;

            case 2: //Después de ir hacia atrás si el choque se ha producido en la zona izq ir girar hacia la derecha
                if(mem == BUFFER){
                    dev.x = 1.0;
                    dev.y = -1.0;
                    dev.z = 1.0;
                }else{
                    dev.x = v.x;
                    dev.y = v.y - controlarY / (BUFFER - mem);  // El valor de (Buffer - mem) se hace cada vez más grande y de forma que dev.y se aproxima a v.y
                    dev.z = 0.0;
                }
                mem --;
                if(mem == 0)
                    estado = 0;
                break;

            case 3: //Después de ir hacia atrás si el choque se ha producido en la zona derecha girar hacia la izq
                if(mem == BUFFER){
                    dev.x = 1.0;
                    dev.y = 1.0;
                    dev.z = 1.0;
                }else{
                    dev.x = v.x;
                    dev.y = v.y - controlarY / (BUFFER - mem);
                    dev.z = 0.0;
                }
                mem --;
                if(mem == 0)
                    estado = 0;
                break;
            case 4: //Después de ir hacia atrás si el choque se ha producido en la zona central girar hacia el objetivo
                if(mem == BUFFER){
                    dev.x = 1.0;
                    if(v.y != 0.0)
                        dev.y = v.y;
                    else
                        dev.y = 1.0; //Se tiene q poner aleatorio
                    dev.z = 1.0;
                }else{
                    dev.x = v.x;
                    dev.y = v.y - controlarY / (BUFFER - mem);
                    dev.z = 0.0;
                }
                mem --;
                if(mem == 0)
                    estado = 0;
                break;
            case 5: //Se ha caido alguna de las ruedas detener el movimiento
                dev.x = 0.0;
                dev.y = 0.0;
                dev.z = 0.0;
                estado = 0;
                break;
            case 6:
                if(v.x != 0){
                    dev.x = v.x;
                    dev.y = v.x * tan(RADIANS_TO_DEGREES(atan2(v.y, v.x) + 30));
                    dev.z = 0;;
                }else{
                    dev.x = v.x;
                    dev.y = v.y;
                    dev.z = 0;
                }
                estado = 0;
                break;
        }



        return dev;
    }else{
        return v;
    }
}


/*Determina el movimiento del robot Roomba
Como el movimiento de roomba se realiza 2 dimensiones, la tercera componente la utilizo para determinar si quiero que el robot vaya marcha atrás.*/
void exec_mover_roomba(Vector v){
    float speed, theta;
    REC_MoveRobot rec;
    int marchaAtras = 0;

    if (debug)
    {
        fprintf(stderr, "exec_mover_roomba(%.2f, %.2f)\n", v.x, v.y);
    }


    if (log_robot_data)
    {
        // Due to the threading, right now, this robot-executable.cc cannot
        // tell which FSA (i.e. Wheel Actuator or Camera Actuator) it is in.
        // Thus, it only records the data for one fsa (FSA-1).

        int fsa = 1;
        int state_num = current_state[fsa];

        // get the position x, y, and heading of the robot.
        robot_position rp = exec_get_position();

        // store those in a file
        exec_log_robot_data(v, rp, robot_id, state_num);
    }


    marchaAtras = v.z;
    rec.x = v.x;
    rec.y = v.y;
    rec.z = 0;
    rec.robot_id = robot_id;

    if (run_type == REAL)
    {
        if(marchaAtras == 1 && v.y == 0.0){
            speed = len_2d(v) * -1;
            theta = 0;
            hclient_steer_toward_and_drive(speed, theta, drive_wait_angle, false);
            usleep(200000);

        }else if(marchaAtras == 1){
            if(v.y < 0.0){
                speed = 0.0;
                theta = 310.0;
                hclient_steer_toward_and_drive(speed, theta, drive_wait_angle, false);
                usleep(1000000);

            }else{
                speed = 0.0;
                theta = 50.0;
                hclient_steer_toward_and_drive(speed, theta, drive_wait_angle, false);
                usleep(1000000);
            }
        }else{
            speed = len_2d(v);
            theta = RADIANS_TO_DEGREES(atan2(v.y, v.x));
            hclient_steer_toward_and_drive(speed, theta, drive_wait_angle, false);
        }

        if (!stopRobotFeedback)
        {
            if (!network_to_console_too_slow(true, "MSG_MoveRobot"))
            {
                communicator->SendMessage(console, MSG_MoveRobot, &rec);
            }
        }
    }
    else
    {
        communicator->SendMessage(console, MSG_MoveRobot, &rec);
    }
}

/*Calcula si valor se encuentra en el intervalo de confianza de centro*/
bool pertenece(double valor, double centro , double margen){
    if((valor >= centro-margen) && (valor <= centro+margen))
        return true;
    else
        return false;
}

/*Obtiene los valores de los sensores de Roomba*/
void exec_get_sensor_roomba(int *num_lecturas, float *lecturas){
    sensor_t *sensor;
    int num;

    hclient_get_sensor_readings(0xFFFF, &num, &sensor);
    *num_lecturas = sensor[SENSOR_ROOMBA].num_readings;

    for(int i=0; i < sensor[SENSOR_ROOMBA].num_readings; i++)
    {
        lecturas[i] = sensor[SENSOR_ROOMBA].reading[i];
    }
}

/*Activa los motores de limpieza*/
void exec_activar_motores_limpieza(bool valor)
{
    if (run_type == REAL)
        hclient_motores_aspirar(valor);
}

/*Envia un mensaje de broadCast indicando que es el lider a todos los dispositivos conectados a IPTServer*/
void enviar_broadCast(int num)
{
	char * moduleName = strdup(communicator->ModuleName());
    communicator->Broadcast(MSG_ConocerRobots, &moduleName);
    free(moduleName);
}

/*Activa la escucha de mensajes de broadCast*/
void escucho_broadCast(bool val){
    escuchoBroadCast = val;
}

void enviarPosicion(robot_position pos){
    REC_PosicionLider rec;
    rec.posicion.x = pos.v.x;
    rec.posicion.y = pos.v.y;
    rec.posicion.z = pos.v.z;
    rec.heading = pos.heading;
    for(int i=0; i< numEsclavos; i++){
        communicator->SendMessage(robotsEsclavos[i], MSG_PosicionLider, &rec);
    }
}

/*Realiza una consulta al lider para conocer su posición y la devuelve*/
robot_position posicion_lider(void)
{
    double time_out = IPT_TIMEOUT;

    if(robotLider == NULL)
    {
    	robot_position posicion = exec_get_position();
        posicionLider.v.x = posicion.v.x;
        posicionLider.v.y = posicion.v.y;
        posicionLider.v.z = posicion.v.z;
        posicionLider.heading = posicion.heading;
    }
    else
    {
        REC_GetPosicionLider rec= robot_id;
    	REC_PosicionLider* vec = (REC_PosicionLider *)communicator->QueryFormatted(robotLider, MSG_GetPosicionLider, &rec, MSG_PosicionLider, time_out);
        posicionLider.v.x = vec->posicion.x;
        posicionLider.v.y = vec->posicion.y;
        posicionLider.v.z = vec->posicion.z;
        posicionLider.heading = vec->heading;
        delete vec;
    }

    return posicionLider;
}

/*Devuelve la variable cambio de estado*/
bool cambio_estado(void){
    return cambioEstado;
}

/*Establece el valor de cambio de estado*/
void poner_cambio_estado(bool b){
    cambioEstado = b;
}

/*El lider envía un cambio de estado*/
void enviar_cambio_estado(int num){
    REC_CambioEstado rec;
    rec = 5;
    if(num >= 1)
        communicator->SendMessage(robotsEsclavos[0], MSG_CambioEstado, &rec);
    for(int i = 0; i < numEsclavos-1; i++)
        robotsEsclavos[i] = robotsEsclavos[i+1];
}

/*Devuelve el número de esclavos*/
int get_num_esclavos(void){
    return numEsclavos;
}

/*Decrementa el número de esclavos*/
void decrementar_num_esclavos(void){
    numEsclavos --;
}

/*Incrementa el número de esclavos*/
void incrementar_num_esclavos(void){
    numEsclavos ++;
}

/*El lider almacena la posición a la que se deben dirigir los esclavos*/
void almacenar_posicion(Vector p){
    posicionEsclavos = p;
}

/*Obtiene la posición objetivo solicitandosela al lider*/
Vector get_posicion_objetivo(void){
    double time_out = IPT_TIMEOUT;
    REC_PosicionDeseadaSolicitud rec;
    rec = 3;

    IPMessage* msg = communicator->Query(robotLider, MSG_PosicionDeseadaSolicitud, &rec, MSG_PosicionDeseadaContestacion, time_out);
    if (msg == NULL)
    {
        return exec_get_position().v;
    }
    REC_PosicionDeseadaContestacion* vec = (REC_PosicionDeseadaContestacion*) msg->FormattedData();
    posicionIr = *vec;
    return posicionIr;
}

/*Devuelve la posición objetivo*/
Vector posicion_objetivo(void){
    return posicionIr;
}

/*Devuelve true si hay un lider establecido y false en caso contrario*/
bool hay_lider(void){
    if(robotLider == NULL)
        return false;
    else
        return true;
}

/*Contestación al mensaje de broadcast proveniente del lider*/
void sigo_lider(void){
    if(robotLider != NULL){
    	char * moduleName = strdup(communicator->ModuleName());
    	communicator->SendMessage(robotLider, MSG_TeSigo, &moduleName);
    	free(moduleName);
    }
}

/*Función para saber si es la primera iteración*/
void set_primera(int val){
    primeraSigoLider = val;
}

/*Función para saber si es la primera iteración*/
int get_primera(void){
    return primeraSigoLider;
}

/*Función para saber si es la primera iteración*/
void set_inicial_nueva_tarea(int val){
    inicialNuevaTarea = val;
}

/*Función para saber si es la primera iteración*/
int get_inicial_nueva_tarea(void){
    return inicialNuevaTarea;
}

/*Devuelve la última posición conocida del lider*/
Vector posicion_ant_lider(void){
    return posicionAntLider;
}
/*Alamacena la última posición conocida del lider*/
void set_posicion_ant_lider(Vector v){
    posicionAntLider = v;
}

/*Calcula un punto en el interior de una habitación a partir de las coordenadas de la puerta y lo almacena en la variable global objetivoEntrarHabitacion*/
Vector posicion_habitacion(void){
    robot_position loc;
    Vector pt1, pt2, mid, inpt1, inpt2;
    boolean stat;
    int newtime;
    double dx, dy, len;
    double distancia = 0.7;

    loc = exec_get_position();

    if(inicialEntrarHabitacion == 0){
        stat = exec_detect_nearest_doorway(&pt1, &pt2, &newtime);
        if(stat == TRUE){
            inicialEntrarHabitacion ++;
            /*determina el punto medio*/
            mid.x = (pt1.x + pt2.x)/2.0;
            mid.y = (pt1.y + pt2.y)/2.0;
            /*calcula la pendiente de la recta*/
            dy = pt1.x - pt2.x;
            dx = pt1.y - pt2.y;
            dy *= -1;
            /*normaliza el vector*/
            len = 0.5/sqrt(dx*dx + dy*dy);
            dx *= len;
            dy *= len;
            /* calcula un punto de prueba */
            inpt1.x = mid.x + dx;
            inpt1.y = mid.y + dy;

            inpt2.x = mid.x - dx;
            inpt2.y = mid.y - dy;

            if(modulo(loc.v, inpt1) < modulo(loc.v, inpt2)){
                objetivoEntrarHabitacion.x = mid.x - dx * distancia;
                objetivoEntrarHabitacion.y = mid.y - dy * distancia;
            }else{
                objetivoEntrarHabitacion.x = mid.x + dx * distancia;
                objetivoEntrarHabitacion.y = mid.y + dy * distancia;
            }
        }else {
            fprintf(stderr, "No doorways around to enter\n");
            return loc.v;
        }
    }
    return objetivoEntrarHabitacion;
}


obs_array  exec_detect_obstacles_mio(float max_dist)
{
    if ((objlist_last_updated_cycle == (int) current_cycle) &&
         (last_dist >= max_dist))
    {
        return last_objlist;
    }

    obs_array out(0);
    REC_ObstacleList* rec;

    if (debug)
    {
        fprintf(stderr, "robot(%d):exec_detect_obs max %f\n",
                robot_id, max_dist);
    }
    if (run_type == SIMULATION)
    {

        REC_DetectObstacles request;

        request.robot_id = robot_id;
        request.max_dist = max_dist;

        IPMessage* msg = communicator->Query(console, MSG_DetectObstacles, &request, MSG_ObstacleList);
        rec = (REC_ObstacleList*) msg->FormattedData();

        obs_array tmp(rec->num_obs);

        for(int i = 0; i<rec->num_obs; i++)
        {
            tmp.val[i].center.x = rec->obs[i].x;
            tmp.val[i].center.y = rec->obs[i].y;
            tmp.val[i].center.z = rec->obs[i].z;
            tmp.val[i].r = rec->radius[i];
        }
        delete msg;
        out = tmp;
    }
    else // real robot
    {
        REC_SensorReadings sensormsg;
        sensor_reading_t* reading;
        int num_readings;
        int mask = 0;
        float ang;

        if (use_sonar) mask = mask|SENSOR_SONAR_MASK;
        if (use_laser) mask = mask|SENSOR_SICKLMS200_MASK;
        if (use_cognachrome) mask = mask|SENSOR_COGNACHROME_MASK;
        if (use_roomba) mask = mask|SENSOR_ROOMBA_MASK;
        num_readings = hclient_get_obstacles(mask, danger_range, &reading, adjust_obstacles);

//         if ((draw_obstacles) && (!stopRobotFeedback))
//         {
//             if (!network_to_console_too_slow(true, "MSG_SensorReadings"))
//             {
//                 sensormsg.robot_id = robot_id;
//                 sensormsg.num_readings = num_readings;
//                 sensormsg.reading = (REC_SensorReading*)reading;
//                 communicator->SendMessage(console, MSG_SensorReadings, &sensormsg);
//             }
//         }

        obs_array tmp(num_readings);
        Vector ptr;

        for(int i=0;i<num_readings;i++){
            ptr.x = reading[i].reading;
            ptr.y = 0;
            ang = reading[i].angle;
            rotate_z(ptr, ang);
            tmp.val[i].center.x = ptr.x;
            tmp.val[i].center.y = ptr.y;
            tmp.val[i].r = 0.0;  // need to do something smart here
        }
        out = tmp;
        if (num_readings)
        {
            free(reading);
        }
    }

    last_objlist = out;
    last_dist = max_dist;
    objlist_last_updated_cycle = current_cycle;

    // Update the sensor blackboard with the new list of detected objects
    // First, delete the existing readings.
    sensor_blackboard.sensed_objects.erase(
            sensor_blackboard.sensed_objects.begin(),
    sensor_blackboard.sensed_objects.end());
    // Next, add the new readings.
    for(int i=0; i<out.size; ++i)
    {
        sensor_blackboard.sensed_objects.push_back(out.val[i]);
    }

    return out;
}

void exec_send_advanced_telop(int joy_x, int joy_y, int slider1, int slider2, int joy_buttons)
{
    if (run_type == REAL)
    	hclient_send_advanced_telop(joy_x, joy_y, slider1, slider2, joy_buttons);
}

void exec_init_carmen_navigate()
{
	if(centralServerConnection == NULL)
	{
		IPC_CONTEXT_PTR context = IPC_getContext();
		const char *centralServer = hclient_get_central_server();
		if((centralServer == NULL) || (centralServer[0]=='\0'))
		{
			fprintf(stderr, "Error in exec_init_carmen_navigate: The robot doesn't have CARMEN features, check HServer and CARMEN modules\n");
			exit(1);
		}

		IPC_connectModule("robot", centralServer);
		printf("Connected with CARMEN at %s\n", centralServer);

		if(!IPC_isMsgDefined(CARMEN_NAVIGATOR_SET_GOAL_NAME))
		{
			fprintf(stderr, "Error in exec_init_carmen_navigate: The robot doesn't have CARMEN navigation features, check that navigator CARMEN module is running connected to %s\n", centralServer);
			exit(1);
		}

		IPC_subscribeData(MISSIONLAB_BASE_VELOCITY_NAME, carmen_base_velocity_handler, NULL);
		centralServerConnection = IPC_getContext();
		free((void *)centralServer);
		if(context != NULL)
			IPC_setContext(context);
	}
}

void exec_carmen_navigate(double x, double y)
{
	carmen_navigator_set_goal_message rec;
	carmen_navigator_go_message rec2;
	struct timeval tv;
	static double lastTime;
	double t;
	static char *host = getenv("HOSTNAME");;

	gettimeofday(&tv, NULL);
	t = tv.tv_sec + tv.tv_usec/1000000.0;

	if((t-lastTime)>1)
	{
		rec.x = x;
		rec.y = y;
		rec.timestamp = t;
		rec.host = host;
		rec2.timestamp = t;
		rec2.host = host;

		IPC_CONTEXT_PTR context = IPC_getContext();
		IPC_setContext(centralServerConnection);
		IPC_publishData(CARMEN_NAVIGATOR_SET_GOAL_NAME, &rec);
		IPC_publishData(CARMEN_NAVIGATOR_GO_NAME, &rec2);
		if(context != NULL)
			IPC_setContext(context);

		lastTime = t;
	}
}

void exec_read_carmen_advise()
{
	IPC_CONTEXT_PTR context = IPC_getContext();
	IPC_setContext(centralServerConnection);
	IPC_listenClear(100);
	if(context != NULL)
		IPC_setContext(context);
}


/**********************************************************************
 * $Log: robot_side_com.c,v $
 * Revision 1.1.1.1  2006/07/12 13:37:56  endo
 * MissionLab 7.0
 *
 * Revision 1.25  2006/07/11 17:15:58  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.24  2006/07/11 06:42:45  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.23  2006/06/21 20:45:17  endo
 * Missing CSB functions restored.
 *
 * Revision 1.22  2006/06/08 22:30:25  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.21  2006/05/15 06:26:46  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.20  2006/05/06 04:24:16  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.19  2006/05/02 04:20:39  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.17  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.16  2006/03/06 15:19:48  endo
 * robot_side_com.c
 *
 * Revision 1.15  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.14  2006/03/01 09:29:09  endo
 * Check-in for Type-I Intercept Experiment.
 *
 * Revision 1.13  2006/02/20 22:19:29  pulam
 * Bug fix involving broadcast of task completion
 *
 * Revision 1.12  2006/02/19 23:51:53  pulam
 * Changes made for experiments
 *
 * Revision 1.11  2006/02/19 17:57:20  endo
 * Experiment related modifications
 *
 * Revision 1.10  2006/01/30 03:02:24  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.9  2006/01/19 01:45:40  pulam
 * Cleaned up CNP code
 *
 * Revision 1.8  2006/01/10 06:09:02  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.7  2005/12/06 22:44:26  pulam
 * CNP hooks into the library
 *
 * Revision 1.6  2005/08/01 20:17:26  endo
 * CNP_TRACK improved.
 *
 * Revision 1.5  2005/04/08 01:55:53  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:32  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:46  pulam
 * addition of 3d visulation and handling of UUVs, USVs, UAVs
 *
 * Revision 1.2  2005/02/07 23:52:05  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:39  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.143  2003/04/06 13:20:20  endo
 * gcc 3.1.1
 *
 * Revision 1.142  2003/04/02 23:03:13  zkira
 * Added get_battery and get_gps
 *
 * Revision 1.141  2002/10/31 21:29:26  ebeowulf
 * Added exec_reset_world
 *
 * Revision 1.140  2002/10/31 19:57:12  ebeowulf
 * added exec_change_robot_color
 *
 * Revision 1.139  2002/07/04 23:19:50  endo
 * AdjustLMValues() moved up above "current_cycle ++;" to solve
 * the race condition against the vector field feature.
 *
 * Revision 1.138  2002/07/02 20:46:18  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.137  2002/04/04 18:33:42  blee
 * Added SendStepTaken() and a call to it in end_of_cycle().
 *
 * Revision 1.136  2001/12/23 20:31:46  endo
 * RH 7.1 porting.
 *
 * Revision 1.135  2000/12/02 21:35:19  sgorbiss
 * Added spin also in simulation
 *
 * Revision 1.134  2000/11/10 23:32:46  blee
 * added learning momentum and the ability to log sensor readings.
 *
 * Revision 1.133  2000/09/19 09:56:31  endo
 * robot_name changed to hserver_name to avoid confusion.
 *
 * Revision 1.132  2000/08/19 00:56:50  endo
 * exec_ping_console added.
 *
 * Revision 1.131  2000/08/15 20:55:53  endo
 * exec_ask_to_proceed added. 'l' option to pass the laserfit machine
 * name added.
 *
 * Revision 1.130  2000/08/12 20:08:05  endo
 * laser_protocol.h -> laser/laser_protocol.h
 *
 * Revision 1.129  2000/08/12 20:04:38  saho
 * Addede code for real world execution of the doorway and corridor
 * behaviors (not just simulation as it was so far).
 *
 * Revision 1.128  2000/08/12 06:55:08  endo
 * Alert fixed.
 *
 * Revision 1.127  2000/08/12 03:27:04  conrad
 * *** empty log message ***
 *
 * Revision 1.126  2000/08/11 23:17:20  endo
 * Rolled back to retrieve rev# 1.124.
 *
 * Revision 1.124  2000/07/02 02:01:22  conrad
 * added hserver support
 * .,
 *
 * Revision 1.123  2000/06/13 16:42:49  endo
 * exec_broadcast_value() was modified and declare_ipt_client()
 * was added to improve the communication capability of
 * robots. The robot can now broadcast information to other
 * robots by establishing server-clients relationships.
 *
 * Revision 1.122  2000/05/19 21:37:00  endo
 * exec_popup_telop_interface added.
 *
 * Revision 1.121  2000/04/25 08:54:04  jdiaz
 * the robot executable can be invoked with the -h option
 * specifying its initial heading
 *
 * Revision 1.120  2000/04/19 21:26:01  jdiaz
 * additional methods for setting and returning database variables
 *
 * Revision 1.119  2000/04/16 15:48:09  endo
 * alerted_once deleted. CYCLE_uSEC cranked up a little.
 *
 * Revision 1.118  2000/04/13 22:28:34  endo
 * Checked in for Doug.
 *
 * Revision 1.117  2000/03/30 16:36:33  saho
 * Added exec_talk and fixed robot_halt() to close the Nomads properly.
 *
 * Revision 1.116  2000/03/30 01:37:11  endo
 * simulation capability for spin added.
 *
 * Revision 1.115  2000/03/29 22:46:46  conrad
 * Fixed nomad 150 bug: added seperate functions for range read for the 150 and the 200.
 *  150 returns 16 reading and 200 return 32 readings.
 *  Simplified pioneed_open_robot so at to remove open_robot function and use generic_open_robot
 *  funtion throughout the file as other generics are used.
 *  Removed all use of ISLAPTOP. Not needed anymore.
 *  Under robot_halt changed pioneer_close_robot to generic_close_robot. No real change
 *  because it is wrapped in an if robot_type = PIONEERAT but this can be removed or
 *  modified for other robot types. No other robot type closes on mlab close except
 *  pioneer.
 *
 * Revision 1.114  2000/03/22 02:52:06  saho
 * Added exec_setxyTheta(double x, double y, double theta);
 *
 * Revision 1.113  2000/02/29 23:32:00  saho
 * Added: int exec_detect_sound(void);
 *
 * Revision 1.112  2000/02/29 00:03:55  jdiaz
 * mark doorway still
 *
 * Revision 1.111  2000/02/28 23:47:10  jdiaz
 * added detect unmarked doorway and detect nearest unmarked doorway
 *
 * Revision 1.110  2000/02/19 01:38:18  endo
 * int debug TRUE --> FALSE
 *
 * Revision 1.109  2000/02/18 22:36:13  conrad
 * 6.0 update
 *
 * Revision 1.108  2000/02/18 02:05:16  endo
 * exec_alert, exec_send_email added for the
 * Alert state.
 *
 * Revision 1.107  2000/02/10 05:48:19  endo
 * Checked in for Douglas C. MacKenzie.
 * He added a new startup flag (-w) to the robot executables
 * which simplifies debugging.
 * When the user starts a robot executable with the -w flag
 * it will pause very early in the startup code and wait
 * for the user to press a carriage return.  This allows the
 * user to attach to the robot process with a debugger such as
 * gdb. He also updated the usage statement to reflect
 * the current parameters.
 *
 * Revision 1.106  2000/02/07 05:05:21  jdiaz
 * added functions to support doorway hallway and room behaviors
 *
 * Revision 1.105  2000/01/19 18:06:35  endo
 * Code checked in for Doug. He added blackboard variable
 * feature.
 *
 * Revision 1.104  1999/12/18 09:29:55  endo
 * Data logging and Report current state capabilities
 * added.
 *
 * Revision 1.103  1999/12/16 22:28:37  mjcramer
 * fixed for RH6
 *
 * Revision 1.90  1999/08/11 21:45:22  jdiaz
 * added correctside function
 *
 * Revision 1.89  1999/07/17 18:10:18  mjcramer
 * Added exec_get_camera_heading
 *
 * Revision 1.88  1999/07/03 17:37:29  endo
 * 3D feature disabled due to the restore of gt_tcx_type_def.h.
 *
 * Revision 1.86  1999/06/29 01:10:36  mjcramer
 * Added laptop control and camera control
 *
 * Revision 1.85  1999/06/23 18:09:10  jdiaz
 * added exec_setxy
 *
 * Revision 1.84  1999/06/11 21:15:56  endo
 * *** empty log message ***
 *
 * Revision 1.83  1999/06/11  20:53:33  endo
 * *** empty log message ***
 *
 * Revision 1.82  1999/06/11  20:17:13  conrad
 * added Urban
 *
 * Revision 1.81  1999/06/07 04:11:16  jdiaz
 * *** empty log message ***
 *
 * Revision 1.80  1999/06/07 03:16:12  jdiaz
 * added exec_get_raw_sonar
 *
 * Revision 1.79  1999/06/07 03:06:36  endo
 * *** empty log message ***
 *
 * Revision 1.78  1999/05/31  01:08:51  conrad
 * *** empty log message ***
 *
 * Revision 1.78  1999/05/31  01:08:51  conrad
 * *** empty log message ***
 *
 * Revision 1.77  1999/05/17 16:55:24  jdiaz
 * support for reading the current compass heading.
 *
 * Revision 1.76  1999/04/27 09:30:27  conrad
 * Added support for kyle.
 *
 * Revision 1.73  1998/11/22 21:16:33  bas
 * Updated robot_side_com to support PIONEERAT.
 *
 * Revision 1.72  1997/06/10 20:32:42  tucker
 * update
 *
 * Revision 1.70  1997/03/27  20:23:22  zchen
 * add compass stuff
 *
 * Revision 1.69  1997/02/21  18:02:18  tucker
 * added nomad stuff
 *
 * Revision 1.68  1997/01/14  22:27:53  tucker
 * added calls to nomad library
 *
 * Revision 1.67  1996/12/19  20:26:25  tucker
 * removed debug = TRUE
 *
 * Revision 1.66  1996/11/19  22:02:08  kali
 * several small changes to integrate with hummer serial interface
 * including several calls to functions in hummer.c,
 * new code in getxy,
 * turn function does not try to put hummer in reverse,
 * and other very small changes
 *
 * Revision 1.65  1996/10/31  17:14:34  kali
 * modified stop function to call the correct hummer stop functions
 *
 * Revision 1.64  1996/10/04  21:17:28  doug
 * fixes for linux
 *
 * Revision 1.64  1996/10/01 13:02:25  doug
 * got linux version working
 *
 * Revision 1.63  1996/06/03  13:34:41  tucker
 * fixed bugs
 *
 * Revision 1.62  1996/05/30  03:46:00  tucker
 * made work with hummer.c
 *
 * Revision 1.61  1996/05/16  02:11:13  tucker
 * removed compiletime warnings
 *
 * Revision 1.60  1996/05/15  14:52:43  doug
 * fixing compiler warnings
 *
 * Revision 1.59  1996/05/14  21:46:50  tucker
 * fixed warnings
 *
 * Revision 1.58  1996/05/08  16:59:28  doug
 * ixing compile warnings
 *
 * Revision 1.57  1996/05/02  20:29:47  kali
 * changed pry_type to type_pry to avoid errors from cnl compiler
 *
 * Revision 1.56  1996/04/30  21:34:02  kali
 * added includes for kvh*.h
 *
 * Revision 1.55  1996/04/30  20:42:39  doug
 * fixed case statement
 *
 * Revision 1.54  1996/04/30  19:14:12  kali
 * exec_get_pry now returns shaft encoder heading also
 *
 * Revision 1.53  1996/04/30  17:58:18  kali
 * added exec_get_pry function
 * added compass type initialization to init function
 *
 * Revision 1.52  1996/03/20  00:30:45  doug
 * *** empty log message ***
 *
 * Revision 1.51  1996/03/19  23:15:01  doug
 * converted send/receive pairs to tcxQuery calls to make sure
 * are getting the correct response packets.
 *
 * Revision 1.50  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.49  1996/03/08  21:13:41  doug
 * seed the random number generator for the robot with either the process id
 * or a number sent in with the -s option
 *
 * Revision 1.48  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.47  1996/02/22  00:56:22  doug
 * *** empty log message ***
 *
 * Revision 1.46  1996/02/20  16:47:27  doug
 * made detect_objects work on the dennings
 *
 * Revision 1.45  1996/02/01  19:55:38  doug
 * added current_cycle as a global for the robot threads to check
 * how many times they have run
 *
 * Revision 1.44  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.43  1995/10/16  21:33:52  doug
 * *** empty log message ***
 *
 * Revision 1.42  1995/10/12  20:13:34  doug
 * Added pickup function
 *
 * Revision 1.41  1995/10/11  20:35:47  doug
 * *** empty log message ***
 *
 * Revision 1.40  1995/10/04  14:30:17  tucker
 * *** empty log message ***
 *
 * Revision 1.39  1995/08/31  14:58:24  kali
 * Now sends message back to simulation server reporting movement vector when
 * in real robot mode for debugging.
 *
 * Revision 1.38  1995/08/01  19:54:34  doug
 * support setting ultrasonic fire delay
 *
 * Revision 1.37  1995/07/08  19:54:18  doug
 * *** empty log message ***
 *
 * Revision 1.36  1995/06/30  16:59:40  doug
 * *** empty log message ***
 *
 * Revision 1.35  1995/06/27  12:50:53  doug
 * *** empty log message ***
 *
 * Revision 1.34  1995/06/25  17:11:58  doug
 * add support for ignoring ultrasonic readings
 * ./
 *
 * Revision 1.33  1995/05/08  19:32:52  jmc
 * Changed some externally visible references to TCX to IPT.  Added
 * checking for IPTHOST before TCXHOST.
 *
 * Revision 1.32  1995/04/26  22:59:52  doug
 * need to get rid of colons in tcx module name
 *
 * Revision 1.31  1995/04/26  22:02:40  doug
 * Converted from tcx to ipt.  Keep your fingers crossed!
 *
 * Revision 1.30  1995/04/21  20:09:15  doug
 * fixed problems with swapping coordinates
 *
 * Revision 1.29  1995/04/20  18:34:03  jmc
 * Corrected spelling.
 *
 * Revision 1.28  1995/04/19  21:51:57  jmc
 * Fixed a few insignificant problems that were pointed out by the
 * latest g++ compiler in -Wall mode.
 *
 * Revision 1.27  1995/04/03  20:37:52  jmc
 * Added copyright notice.
 *
 * Revision 1.26  1995/02/13  20:26:12  jmc
 * Tweaks to make -Wall compile quietly.
 *
 * Revision 1.25  1995/02/05  03:35:14  doug
 * Added ability to single step cycle through startup when paused
 * Fixed bug in return_data where it freed the key's value and then returned
 * a pointer to it.  Didn't show up on the sun's but did on Linux.
 *
 * Revision 1.24  1995/01/30  15:42:25  doug
 * porting to linux
 *
 * Revision 1.23  1995/01/10  19:32:58  doug
 * add notify_config msg
 *
 * Revision 1.22  1994/12/06  16:30:39  doug
 * getting dennings to run
 *
 * Revision 1.21  1994/11/29  17:05:07  doug
 * *** empty log message ***
 *
 * Revision 1.20  1994/11/18  00:50:04  doug
 * don't use -r robot_type but send it in the parameters
 *
 * Revision 1.19  1994/11/17  23:44:52  doug
 * fixed init code when running denning
 *
 * Revision 1.18  1994/11/16  22:30:01  doug
 * fixing halt problems
 *
 * Revision 1.17  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.16  1994/11/11  21:50:01  jmc
 * Changed some printfs to print only when debugging.
 *
 * Revision 1.15  1994/11/11  16:59:07  jmc
 * Cleaned up minor problems (unused variables, etc) to get rid of
 * make -Wall problems.
 *
 * Revision 1.14  1994/11/09  22:57:26  doug
 * make halt block until end of complete cycle so put_status will
 *  be able to end stop commands correctly
 *
 * Revision 1.13  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.13  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.12  1994/11/03  19:34:13  doug
 * Added single step cycle command
 *
 * Revision 1.11  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.10  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.9  1994/10/28  18:34:09  jmc
 * Removed code from new_command dealing with the deleted until_msg.
 *
 * Revision 1.8  1994/10/19  21:54:09  doug
 * *** empty log message ***
 *
 * Revision 1.7  1994/10/18  20:55:29  doug
 * fixing prototypes
 *
 * Revision 1.6  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.5  1994/10/11  20:12:50  doug
 * added tcxclose call
 *
 * Revision 1.4  1994/10/10  21:29:59  jmc
 * Added getting current host name for tcx host as a last resort.
 *
 * Revision 1.3  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.2  1994/10/03  21:13:47  doug
 * Switched to tcx
 *
 * Revision 1.1  1994/09/28  20:45:24  doug
 * Initial revision
 *
 **********************************************************************/
