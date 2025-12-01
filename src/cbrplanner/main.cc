/**********************************************************************
 **                                                                  **
 **                               main.cc                            **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains the main function for CBRServer.             **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: main.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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
#include "cbrplanner.h"
#include "communicator.h"
#include "windows.h"
#include "keyboard_reader.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const bool DEBUG = false;
const string EMPTY_STRING = "";

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
CBRPlanner *gCBRPlanner = NULL;
Communicator *gCommunicator = NULL;
Windows *gWindows = NULL;
KeyboardReader *gKeyboardReader = NULL;
Debugger *gDebugger = NULL;

//---------------------------------------------------------------------
// This function kills the program.
//---------------------------------------------------------------------
void gQuitProgram(int exitStatus)
{
    if (gKeyboardReader != NULL)
    {
        delete gKeyboardReader;
    }

    if (gWindows != NULL)
    {
        if (exitStatus == -1)
        {
            gWindows->printfPlannerWindow("Abnormal termination.\n");
        }
        delete gWindows;
    }

    if (gCommunicator != NULL)
    {
        delete gCommunicator;
    }

    if (gCBRPlanner != NULL)
    {
        delete gCBRPlanner;
    }

    if (gDebugger != NULL)
    {
        delete gDebugger;
    }

    exit(exitStatus);
}

//---------------------------------------------------------------------
// This function displays usage.
//---------------------------------------------------------------------
void showUsageAndExit(char *binaryName)
{
    fprintf(stderr, "\nUsage: %s <option>\n\n", binaryName);
    fprintf(stderr, "Where <option> is:\n");
    fprintf(stderr, "  -s <socket name>  Use the specified socket name to initiate the\n");
    fprintf(stderr, "                    communication with CfgEdit/mlab.\n");
    fprintf(stderr, "  -l <library name> Load the specified CBR library.\n");
    fprintf(stderr, "                -d  Enable debug.\n");
    fprintf(stderr, "                -h  Show this usage.\n");
    fprintf(stderr, "\n");

    gQuitProgram(0);
}

//---------------------------------------------------------------------
// This function catches the signal.
//---------------------------------------------------------------------
void signalHandler(int sig)
{
    const bool DEBUG_SIGNAL_HANDLER = false;

    switch (sig) {

    case SIGABRT:
    case SIGSEGV:
    case SIGBUS:
        gQuitProgram(-1);
        break;

    // ENDO - gcc 3.4
    case SIGHUP:
        if (DEBUG_SIGNAL_HANDLER)
        {
            gWindows->printfPlannerWindow("signalHandler(): Ignoring SIGHUP.\n");
        }
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
    int c;
    bool validArgument = true;
    bool debug = DEBUG;
    string specifiedSocketName = EMPTY_STRING;
    string cbrlibraryName = EMPTY_STRING;

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
        while ((c = getopt(argc, argv, "s:l:hd")) != -1)
        {
            switch (c) {
            case 'h':
                validArgument = false;
                break;
            case 's':
                specifiedSocketName = optarg;
                break;
            case 'l':
                cbrlibraryName = optarg;
                break;
            case 'd':
                debug = true;
                break;
            }
        }

        if (!validArgument)
        {
            showUsageAndExit(argv[0]);
        }
    }

    // Instantiate the Debugger class.
    new Debugger(&gDebugger);

    // Instantiate the CBRPlanner class.
    new CBRPlanner(&gCBRPlanner);

    // Instantiate the Communicator class.
    new Communicator(&gCommunicator);

    // Instantiate the Windows class, and start windows.
    new Windows(&gWindows);
    gWindows->startWindows();

    // Instantiate the KeyboardReader class, and start the deamon.
    new KeyboardReader(&gKeyboardReader);
    gKeyboardReader->startDaemon();

    if (debug)
    {
        // Enable debugger.
        gDebugger->enableDebug();
    }

    // Start all the subsystems of the CBR Planner.
    gCBRPlanner->startSubsystems();

    // If user specified, load the CBR library.
    if (cbrlibraryName != EMPTY_STRING)
    {
        gCBRPlanner->loadCBRLibrary(cbrlibraryName);
    }

    // If user specified, use that socket name instead.
    if (specifiedSocketName != EMPTY_STRING)
    {
        gWindows->printfCommWindow("CBR Planner: Using the socket \"%s\".", specifiedSocketName.c_str());
        gCommunicator->setSocketName(specifiedSocketName);
    }

    // Start the communication daeman.
    gCommunicator->startDaemon();

    // Start a main loop until the program is terminated by the user.
    gCBRPlanner->mainLoop();

    return 0;
}

/**********************************************************************
 * $Log: main.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.5  2005/07/30 02:18:24  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/07/28 04:55:46  endo
 * 3D visualization improved more.
 *
 * Revision 1.3  2005/06/23 22:09:04  endo
 * Adding the mechanism to talk to CNP.
 *
 * Revision 1.2  2005/02/07 19:53:44  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:27  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/06 08:43:44  endo
 * Initial revision
 *
 *
 **********************************************************************/
