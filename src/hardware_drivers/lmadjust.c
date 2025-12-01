#include <stdlib.h>	  // For random()
#include <stdio.h>	  // For debugging and converting doubles to strings
#include "lmadjust.h"
#include "gt_simulation.h"
#include "sensor_blackboard.h"

typedef struct status_struct {
    double avg_motion;
    double avg_affecting_obstacles;
    double avg_motion_to_goal;
    double distance_to_goal;
} status_struct;

double RandomDoubleRange( double dMin, double dMax );
int    RandomIntRange( int iMin, int iMax );
void AdjustVectorGains( double& dGoalGain, 
                        double& dObjectGain, 
                        double& dWanderGain );
void AdjustWanderPersistence( int& iPersistence );
void AdjustSphereOfInfluence( double& dSphereOfInfluence );

void SetDefaultParams();
void UpdateHistory();

void GetCurrentWeights( double& dGoalGain, 
                        double& dObjectGain, 
                        double& dWanderGain );
void StoreCurrentWeights( double dGoalGain, 
                          double dObjectGain, 
                          double dWanderGain );

void GetCurrentWanderPersistence( int& iPersistence );
void StoreCurrentWanderPersistence( int iPersistence );

void GetCurrentSphereOfInfluence( double& dSphereOfInfluence );
void StoreCurrentSphereOfInfluence( double dSphereOfInfluence );

int GetLMEnabled();
void SetLMEnabled( int iEnabled );

void LogWeightData( double dGoalGain, double dObjectGain, 
                    double dWanderGain, double dSphereOfInfluence, 
                    int iWanderPersistence );

inline void PutWithinBounds( double& dNum, double dLower, double dUpper );
inline void PutWithinBounds( int& iNum, int iLower, int iUpper );

EnMovementType GetMovementType();

const char* szNO_MOVEMENT_TXT                   = "lm_no_movement";
const char* szPROGRESS_TXT                      = "lm_progress";
const char* szNO_PROGRESS_WITH_OBSTACLES_TXT    = "lm_no_progress_with_obstacles";
const char* szNO_PROGRESS_WITHOUT_OBSTACLES_TXT = "lm_no_progress_without_obstacles";
const char* szG0AL_TXT                          = "lm_goal";
const char* szOBJECT_TXT                        = "lm_object";
const char* szNOISE_TXT                         = "lm_noise";
const char* szSPHERE_TXT                        = "lm_sphere";
const char* szNOISE_PERSISTENCE_TXT             = "lm_noise_persistence";


double history[WINDOW_SIZE]; // performance history
int    hist_offset = 0;
double distance_to_goal[WINDOW_SIZE];
int    affecting_obstacle_history[WINDOW_SIZE];

int    iFirstTime = TRUE;

status_struct status;

extern char robot_name[ 256 ]; // declared in robot_side_com.c
extern char* halting;          // declared in robot_side_com.c

SuDoubleRange g_aasuLMParams[ EnMovementType_COUNT ][ EnLMParamType_COUNT ];

void AdjustLMValues()
{
    static robot_position suLastPos;
    static int iHistoryCount = 0;

    // initialize the values with defaults
    double dGoalGain          = 1.0;
    double dObjectGain        = 1.0;
    double dWanderGain        = 0.0;
    double dSphereOfInfluence = 1.0;
    int    iPersistence       =  10;

    robot_position suCurrentPos = exec_get_position();

    if ( iFirstTime )
    {
        // store initial weights
        StoreCurrentWeights( dGoalGain, dObjectGain, dWanderGain );
        StoreCurrentSphereOfInfluence( dSphereOfInfluence );
        StoreCurrentWanderPersistence( iPersistence );

        SetDefaultParams();

        SetLMEnabled( TRUE );
        iFirstTime = FALSE;
        suLastPos = suCurrentPos;
    }

    history[ hist_offset ] = vlength( suCurrentPos.v.x - suLastPos.v.x,
                                      suCurrentPos.v.y - suLastPos.v.y );

    suLastPos = suCurrentPos;

    if( iHistoryCount == HISTORY_INTERVAL )
    {
        UpdateHistory();

        if ( GetLMEnabled() )
        {
            // get the current weights from the database
            GetCurrentWeights( dGoalGain, dObjectGain, dWanderGain );
            GetCurrentSphereOfInfluence( dSphereOfInfluence );
            GetCurrentWanderPersistence( iPersistence );

            // adjust weights dictating the robot behavior
            AdjustVectorGains( dGoalGain, dObjectGain, dWanderGain );
            AdjustSphereOfInfluence( dSphereOfInfluence );
            AdjustWanderPersistence( iPersistence );

            // write the gains back to the database.
            StoreCurrentWeights( dGoalGain, dObjectGain, dWanderGain );
            StoreCurrentSphereOfInfluence( dSphereOfInfluence );
            StoreCurrentWanderPersistence( iPersistence );

            if ( save_learning_momentum_weights )
            {
                LogWeightData( dGoalGain, dObjectGain, dWanderGain, 
                                 dSphereOfInfluence, iPersistence );
            }

            // tell the console to update any graphics
            exec_update_console_lm_params();
        }

        iHistoryCount = 0;
    }

    iHistoryCount++;
    hist_offset = ( hist_offset + 1 ) % WINDOW_SIZE;
}

void AdjustVectorGains( double& dGoalGain, 
                        double& dObjectGain, 
                        double& dWanderGain )
{
    SuDoubleRange* psuDelta;
    EnMovementType enMovement = GetMovementType();

    // Adjust the weights.
    psuDelta = &( g_aasuLMParams[ enMovement ][ EnLMParamType_GOAL ] );
    dGoalGain   += RandomDoubleRange( psuDelta->dLo, psuDelta->dHi );

    psuDelta = &( g_aasuLMParams[ enMovement ][ EnLMParamType_OBJECT ] );
    dObjectGain += RandomDoubleRange( psuDelta->dLo, psuDelta->dHi );

    psuDelta = &( g_aasuLMParams[ enMovement ][ EnLMParamType_NOISE ] );
    dWanderGain += RandomDoubleRange( psuDelta->dLo, psuDelta->dHi );

    // Enforce a maximum/minimum gain value.
    // It would be nice if we could learn these limits at some point.

    const double dNOISE_GAIN_UPPER_LIMIT = 5.0;
    const double dNOISE_GAIN_LOWER_LIMIT = 0.01;
    const double dGOAL_GAIN_UPPER_LIMIT = 2.0;
    const double dGOAL_GAIN_LOWER_LIMIT = 0.5;
    const double dOBJECT_GAIN_UPPER_LIMIT = 5.0;

    PutWithinBounds( dGoalGain, 
                     dGOAL_GAIN_LOWER_LIMIT, dGOAL_GAIN_UPPER_LIMIT );
    PutWithinBounds( dWanderGain, 
                     dNOISE_GAIN_LOWER_LIMIT, dNOISE_GAIN_UPPER_LIMIT );
    PutWithinBounds( dObjectGain, 
                     (dGoalGain + dWanderGain), dOBJECT_GAIN_UPPER_LIMIT );

    if ( debug )
    {
        printf("\n Average magnitude is %f\n", status.avg_motion);
        printf(" Distance to goal is %f\n", distance_to_goal[hist_offset]);
        printf(" Average motion to goal is %f\n", status.avg_motion_to_goal);
        printf(" Average affecting obstacles is %f\n", 
	       status.avg_affecting_obstacles);
        printf(" Goal gain %f\n", dGoalGain);
        printf(" Wander gain %f\n", dWanderGain);
        printf(" Obstacle gain %f\n", dObjectGain);
    }
}

void AdjustWanderPersistence( int& iPersistence )
{
    const int iMIN_PERSISTENCE = 1;
    const int iMAX_PERSISTENCE = 15;

    EnMovementType enMovement = GetMovementType();
    SuDoubleRange* psuDelta = &( g_aasuLMParams[ enMovement ][ EnLMParamType_NOISE_PERSISTENCE ] );
    iPersistence += RandomIntRange( (int) rint( psuDelta->dLo ), (int) rint( psuDelta->dHi ) );

    // set the lower bound of the persistence; you must have a 
    // positive persistence.
    PutWithinBounds( iPersistence, iMIN_PERSISTENCE, iMAX_PERSISTENCE );

    if ( debug )
    {
        printf( "\nCurrent persistence is: %d\n", iPersistence );
    }
}

void AdjustSphereOfInfluence( double& dSphereOfInfluence )
{
    const double dMIN_SPHERE_VALUE = 0.1;
    const double dMAX_SPHERE_VALUE = 5.0;

    EnMovementType enMovement = GetMovementType();
    SuDoubleRange* psuDelta = &( g_aasuLMParams[ enMovement ][ EnLMParamType_SPHERE ] );
    dSphereOfInfluence += RandomDoubleRange( psuDelta->dLo, psuDelta->dHi );

    PutWithinBounds( dSphereOfInfluence, 
                     dMIN_SPHERE_VALUE, dMAX_SPHERE_VALUE );

    if ( debug )
    {
        printf( "\nCurrent sphere of influence is: %f\n", dSphereOfInfluence );
    }
}

void SetDefaultParams()
{
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_GOAL].dLo   = -0.1;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_GOAL].dHi   =  0.0;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_OBJECT].dLo = -0.1;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_OBJECT].dHi =  0.0;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_NOISE].dLo  =  0.1;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_NOISE].dHi  =  0.5;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_SPHERE].dLo  = -0.5;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_SPHERE].dHi  =  0.0;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_NOISE_PERSISTENCE].dLo  =  0.0;
    g_aasuLMParams[EnMovementType_NO_MOVEMENT][EnLMParamType_NOISE_PERSISTENCE].dHi  =  1.0;

    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_GOAL].dLo   =  0.5;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_GOAL].dHi   =  1.0;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_OBJECT].dLo = -0.1;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_OBJECT].dHi =  0.0;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_NOISE].dLo  = -0.1;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_NOISE].dHi  =  0.0;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_SPHERE].dLo  = -0.5;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_SPHERE].dHi  =  0.0;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_NOISE_PERSISTENCE].dLo  = -1.0;
    g_aasuLMParams[EnMovementType_PROGRESS][EnLMParamType_NOISE_PERSISTENCE].dHi  =  0.0;

    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_GOAL].dLo   = -0.1;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_GOAL].dHi   =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_OBJECT].dLo =  0.1;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_OBJECT].dHi =  0.5;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_NOISE].dLo  =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_NOISE].dHi  =  0.5;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_SPHERE].dLo  =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_SPHERE].dHi  =  0.5;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_NOISE_PERSISTENCE].dLo  =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITH_OBSTACLES][EnLMParamType_NOISE_PERSISTENCE].dHi  =  1.0;

    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_GOAL].dLo   =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_GOAL].dHi   =  0.3;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_OBJECT].dLo = -0.1;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_OBJECT].dHi =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_NOISE].dLo  = -0.2;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_NOISE].dHi  =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_SPHERE].dLo  =  0.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_SPHERE].dHi  =  0.5;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_NOISE_PERSISTENCE].dLo  = -1.0;
    g_aasuLMParams[EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES][EnLMParamType_NOISE_PERSISTENCE].dHi  =  0.0;
}

EnMovementType GetMovementType()
{
    const double dMOVEMENT_THRESHOLD = 0.02;
    const double dPROGRESS_THRESHOLD = 0.025;
    const double dOBSTACLE_THRESHOLD = 1.5;

    EnMovementType enMovement = EnMovementType_NO_MOVEMENT;

    double motion_ratio = status.avg_motion_to_goal / 
                          status.avg_motion;

    // check to see if the robot is moving
    if ( status.avg_motion < dMOVEMENT_THRESHOLD )
    {
        enMovement = EnMovementType_NO_MOVEMENT;
    } 
    else
    {
      // check to see if the robot is moving towards the goal
        if ( motion_ratio > dPROGRESS_THRESHOLD )
        {
            enMovement = EnMovementType_PROGRESS;
        }
        else    // progress is not being made
        {
            // see if progress is being hindered by obstacles
            if ( status.avg_affecting_obstacles >= dOBSTACLE_THRESHOLD )
            {
	            enMovement = EnMovementType_NO_PROGRESS_WITH_OBSTACLES;
            }
            else
            {
	            enMovement = EnMovementType_NO_PROGRESS_WITHOUT_OBSTACLES;
            }
        }
    }

    if ( 0 )
    {
        printf( "\nAdjustWeights: status.avg_affecting_obstacles: %f\n"
                "AdjustWeights: status.avg_motion_to_goal: %f\n"
                "AdjustWeights: status.distance_to_goal: %f\n"
                "AdjustWeights: status.avg_motion: %f\n\n",
                status.avg_affecting_obstacles,
                status.avg_motion_to_goal,
                status.distance_to_goal,
                status.avg_motion );
    } 

    return enMovement;
}

void UpdateHistory()
{
    static int hist_step = 0;

    // give hist_step one up from hist_offset, but wrap around to zero 
    // if it becomes greater than WINDOW_SIZE */
    hist_step = (hist_offset + 1) % WINDOW_SIZE;
    status.avg_motion = 0.0;
    status.avg_motion_to_goal = 0.0;
    status.avg_affecting_obstacles = 0.0;
        
    // move from hist_step to the end of the array, then wrap from the 
    // beginning of the array to the one before where we started, so all 
    // array elements are covered. */
    while ( hist_step != hist_offset )
    {
        status.avg_motion += history[hist_step];
        if ( debug )
        {
            printf( "UpdateStatus in lmadjust.c: history[%d] = %f\n"
                    "       distance_to_goal[%d] = %f\n",
                    hist_step, history[hist_step],
                    hist_step, distance_to_goal[hist_step] );
        }
        status.avg_motion_to_goal += distance_to_goal[hist_step] -
                        distance_to_goal[ (hist_step + 1) % WINDOW_SIZE ];
        status.avg_affecting_obstacles += 
	                    (double) affecting_obstacle_history[hist_step];
        affecting_obstacle_history[hist_step] = 0;
        hist_step = (hist_step + 1) % WINDOW_SIZE;
    }
    // Get the last velocity
    status.avg_motion += history[hist_step];
    if ( debug )
    {
        printf( "UpdateStatus in lmadjust.c: history[%d] = %f\n"
                " distance_to_goal[%d] (final) = %f\n",
                hist_step, history[hist_step], 
                hist_step, distance_to_goal[hist_step]);
    }
    status.avg_affecting_obstacles += 
      (double) affecting_obstacle_history[hist_step];
    affecting_obstacle_history[hist_step] = 0;

    status.avg_motion /= (double) WINDOW_SIZE;
    status.avg_motion_to_goal /= (double) (WINDOW_SIZE - 1);
    status.avg_affecting_obstacles /= (double) WINDOW_SIZE;
    status.distance_to_goal = distance_to_goal[hist_offset];
}

void GetCurrentWeights( double& dGoalGain, 
			double& dObjectGain, 
			double& dWanderGain )
{
    const char* szValue = NULL;

    szValue = get_state( (char*) szMOVE_TO_GOAL_GAIN_LABEL );
    if ( szValue != NULL )
    {
        dGoalGain = strtod( szValue, NULL );
    }

    szValue = get_state( (char*) szOBJECT_GAIN_LABEL );
    if ( szValue != NULL )
    {
        dObjectGain = strtod( szValue, NULL );
    }

    szValue = get_state( (char*) szWANDER_GAIN_LABEL );
    if ( szValue != NULL )
    {
        dWanderGain = strtod( szValue, NULL );
    }
}

void StoreCurrentWeights( double dGoalGain, 
			  double dObjectGain, 
			  double dWanderGain )
{
    char szGain[ 30 ];

    sprintf( szGain, "%f", dGoalGain );
    exec_put_console_state( (char*) szMOVE_TO_GOAL_GAIN_LABEL, szGain );

    sprintf( szGain, "%f", dObjectGain );
    exec_put_console_state( (char*) szOBJECT_GAIN_LABEL, szGain );

    sprintf( szGain, "%f", dWanderGain );
    exec_put_console_state( (char*) szWANDER_GAIN_LABEL, szGain );
}

void GetCurrentWanderPersistence( int& iPersistence )
{
    const char* szValue = NULL;

    szValue = get_state( (char*) szWANDER_PERSISTENCE_LABEL );
    if ( szValue != NULL )
    {
        iPersistence = atoi( szValue );
    }
}

void StoreCurrentWanderPersistence( int iPersistence )
{
    char szPersistence[ 10 ];

    sprintf( szPersistence, "%d", iPersistence );
    exec_put_console_state( (char*) szWANDER_PERSISTENCE_LABEL, szPersistence );
}

void GetCurrentSphereOfInfluence( double& dSphereOfInfluence )
{
    const char* szValue = NULL;

    szValue = get_state( (char*) szSPHERE_OF_INFLUENCE_LABEL );
    if ( szValue != NULL )
    {
        dSphereOfInfluence = strtod( szValue, NULL );
    }
}

void StoreCurrentSphereOfInfluence( double dSphereOfInfluence )
{
    char szSphereOfInfluence[ 30 ];

    sprintf( szSphereOfInfluence, "%f", dSphereOfInfluence );
    exec_put_console_state( (char*) szSPHERE_OF_INFLUENCE_LABEL, szSphereOfInfluence );
}

void SetLMEnabled( int iEnabled )
{
    exec_put_console_state( (char*) szLM_ENABLED_LABEL, (char*) ( iEnabled ? "1" : "0" ) );
}

int GetLMEnabled()
{
    const char* szValue = NULL;
    int iEnabled = 0;

    szValue = get_state( (char*) szLM_ENABLED_LABEL );
    if ( szValue != NULL )
    {
        iEnabled = atoi( szValue );
    }

    return iEnabled;
}

// return a random double value bounded by dMin and dMax
double RandomDoubleRange( double dMin, double dMax )
{
    double dRange = dMax - dMin;
    double dRandomNum = (double) random() / RAND_MAX;

    return ( dMin + ( dRange * dRandomNum ) );
}

// return a random int value bound by iMin and iMax
int RandomIntRange( int iMin, int iMax )
{
    int iRange = iMax - iMin + 1; // must add 1 since iMax should be inclusive
    return ( random() % iRange ) + iMin;
}

// make sure dNum is within the bounds set by dLower and dUpper. If it is out 
// bounds, it is set to the nearest bound. dLower should be less than dUpper
void PutWithinBounds( double& dNum, double dLower, double dUpper )
{
    if ( dNum < dLower )
    {
        dNum = dLower;
    }
    if ( dNum > dUpper )
    {
        dNum = dUpper;
    }
}

// make sure iNum is within the bounds set by iLower and iUpper. If it is out 
// bounds, it is set to the nearest bound. dLower should be less than dUpper
void PutWithinBounds( int& iNum, int iLower, int iUpper )
{
    if ( iNum < iLower )
    {
        iNum = iLower;
    }
    if ( iNum > iUpper )
    {
        iNum = iUpper;
    }
}

void LogWeightData( double dGoalGain, double dObjectGain, 
                    double dWanderGain, double dSphereOfInfluence, 
                    int iWanderPersistence )
{
    const char* szLOG_HEADER = 
        "\n"
        "                  MissionLab Learning Momentum Weight Data\n"
        "\n"
        "--------------------------------------------"
        "------------------------------------------\n"
        " Entry      Time   Goal Gain  Object Gain  Wander Gain  "
        "Sphere Of Infl  Wander Persist\n"
        "--------------------------------------------"
        "------------------------------------------\n";

    // get the running time.
    static bool bTimeZero = TRUE;
    struct timeval suCurrentTime;
    gettimeofday( &suCurrentTime, NULL );
    double dAbsTime = (double) suCurrentTime.tv_sec + 
                         ( ( (double) suCurrentTime.tv_usec ) / 1000000 );

    static double dStartLogTime;
    static FILE* pfLogFile = NULL;
    static int iEntry;

    // make sure it is not in the halting process.
    if ( halting )
    {
        if ( pfLogFile != NULL )
        {
            fclose( pfLogFile );
	        pfLogFile = NULL;
        }
        return;
    }

    // open the log file
    if ( pfLogFile == NULL )
    {
        // name the log file
        char szLogfileName[300];
        if ( pfLogFile == NULL )
        {
            sprintf( szLogfileName, "%s%011ld.lm-log", 
                     robot_name, suCurrentTime.tv_sec );
        }

        if( ( pfLogFile = fopen( szLogfileName, "w" ) ) == NULL )
        {
            printf( "Unable to write in %s!\n", szLogfileName );
            exit( 1 );
        }
        iEntry = 1;
    }

    // put a heading in the logfile if it is the first time.
    if ( bTimeZero )
    {
        dStartLogTime = dAbsTime;

        fprintf( pfLogFile, szLOG_HEADER );

        bTimeZero = FALSE;
    }

    double dRunTime = dAbsTime - dStartLogTime;

    // record the data in the file
    fprintf( pfLogFile, "%5d %10.3f %10.3f %10.3f %12.3f %14.3f %12d\n",
             iEntry, dRunTime, dGoalGain, dObjectGain, dWanderGain, 
             dSphereOfInfluence, iWanderPersistence );
    iEntry++;

    fflush( pfLogFile );
}
