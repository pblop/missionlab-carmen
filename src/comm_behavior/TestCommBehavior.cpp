/**********************************************************************
 **                                                                  **
 **                       TestCommBehavior.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2004 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: TestCommBehavior.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <sys/types.h>
#include <string>

#include "CommBehavior.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int DEFAULT_HOPS = 0;
const double DEFAULT_SIG_STRENGTH = 0.5;
const double DEFAULT_HEADING = 0.0;
const double DEFAULT_X = 5.0;
const double DEFAULT_Y = 5.0;

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//---------------------------------------------------------------------
// This function kills the program.
//---------------------------------------------------------------------
void gQuitProgram(int exitStatus)
{

    exit(exitStatus);
}

//---------------------------------------------------------------------
// This function displays usage.
//---------------------------------------------------------------------
void showUsageAndExit(char *binaryName)
{
    fprintf(stderr, "\nThis program tests libCommBehavior.so.\n");
    fprintf(stderr, "Make sure you setenv LD_LIBRARY_PATH to where the library is.\n\n");

    gQuitProgram(0);
}

//---------------------------------------------------------------------
// This function catches the signal.
//---------------------------------------------------------------------
void signalHandler(int sig)
{
    switch (sig) {

    case SIGABRT:
    case SIGSEGV:
    case SIGBUS:
    case SIGHUP:
        gQuitProgram(-1);
        break;

    default:
        gQuitProgram(0);
        break;

    }
}

//-----------------------------------------------------------------------
// Main function for cbrpserver
//-----------------------------------------------------------------------
int main(int argc, char **argv)
{
    CommBehavior *commBehavior = NULL;
    SensorDataUMWrap *gtechSensorDataInput = NULL;
    CVectorConfidenceAdviceUMWrap *behaviorAdvise = NULL;
    int c;
    int hops = DEFAULT_HOPS;
    double sigStrength = DEFAULT_SIG_STRENGTH;
    double heading = DEFAULT_HEADING;
    double x = DEFAULT_X;
    double y = DEFAULT_Y;
    double adviseX, adviseY, adviseConfidence;
    bool validArgument = true;

    // Set the signal handler.
    signal(SIGINT, signalHandler);
    signal(SIGABRT, signalHandler);
    signal(SIGTERM, signalHandler);
    signal(SIGBUS, signalHandler);
    signal(SIGHUP, signalHandler);
    signal(SIGSEGV, signalHandler);

    // Get the command line arguments.
    if (argc > 1)
    {
        while ((c = getopt(argc, argv, "h")) != -1)
        {
            switch (c) {
            case 'h':
                validArgument = false;
                break;
            }
        }

        if (!validArgument)
        {
            showUsageAndExit(argv[0]);
        }
    }

    // Initialize CommBehaivor
    commBehavior = createCommBehavior();
    Initialize(commBehavior,1,0);
    Load(commBehavior,0);
    fprintf(stderr, "GTechBehaviorDLL initialized.\n");
    
    // Initialize the sensor data class
    gtechSensorDataInput = createSensorDataUMWrap();
    fprintf(stderr, "Sensor data class initialized.\n");

    while (true)
    {
        // Update sensor value.
        SetSensorCommData(
            gtechSensorDataInput,
            0, // Id
            hops, // Hops
            sigStrength, // SigStrength
            heading, // Heading
            x, // X
            y, // Y
            0); // Z

        UpdateSensoryData(commBehavior, gtechSensorDataInput);

        fprintf(stderr, "\nSensor value updated.\n");
        fprintf(stderr, "Hops: %d\n", hops);
        fprintf(stderr, "SigStrength: %d\n", hops);
        fprintf(stderr, "Heading: %f\n", heading);
        fprintf(stderr, "X: %f\n", x);
        fprintf(stderr, "Y: %f\n", y);
        sleep(1);

        
        // Execute the GTech behavior.
        behaviorAdvise = Execute(commBehavior);
        adviseX = GetX(behaviorAdvise);
        adviseY = GetY(behaviorAdvise);
        adviseConfidence = GetConfidence(behaviorAdvise);
        delete behaviorAdvise;
        behaviorAdvise = NULL;

        fprintf(stderr, "\nGTechBehavior executed.\n");
        fprintf(stderr, "X: %f\n", adviseX);
        fprintf(stderr, "Y: %f\n", adviseY);
        fprintf(stderr, "Confidence: %f\n", adviseConfidence);

        sleep(2);
    }

    return 0;
}
/**********************************************************************
 * $Log: TestCommBehavior.cpp,v $
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
 * Revision 1.2  2004/03/21 02:38:40  alanwags
 * *** empty log message ***
 *
 * Revision 1.1  2004/03/20 23:32:34  endo
 * A simple test program for GTechBehavior.
 *
 **********************************************************************/
