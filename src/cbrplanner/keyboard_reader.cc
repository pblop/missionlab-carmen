/**********************************************************************
 **                                                                  **
 **                          keyboard_reader.cc                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module deals with processing of the user inputs via        **
 **  key entries.                                                    **
 **                                                                  **
 **  Copyright 2003 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: keyboard_reader.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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
#include <pthread.h>
#include <curses.h>

#include "keyboard_reader.h"
#include "windows.h"
#include "cbrplanner.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const struct timespec KeyboardReader::DAEMON_SLEEPTIME_NSEC_ = {0, 10000000}; // 10 msec.
const int KeyboardReader::DAEMON_USLEEPTIME_ = 10000; // 10 msec.
const string KeyboardReader::KEYBIND_TITLE_ = "Enter a key:";

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------
pthread_mutex_t KeyboardReader::readerMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for KeyboardReader class.
//-----------------------------------------------------------------------
KeyboardReader::KeyboardReader(KeyboardReader **keyboardReaderInstance) : self_(keyboardReaderInstance)
{
    if (keyboardReaderInstance != NULL)
    {
        *keyboardReaderInstance = this;
    }

    daemonThreadIsUp_ = false;

    pthread_mutex_init(&readerMutex_, NULL);

    displayDefaultKeybind();
}

//-----------------------------------------------------------------------
// Distructor for KeyboardReader class.
//-----------------------------------------------------------------------
KeyboardReader::~KeyboardReader(void)
{
    // Make sure to kill the deamon thread.
    if (daemonThreadIsUp_)
    {
        daemonThreadIsUp_ = false;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function starts up a thread to run the keyboard reader daemon.
//-----------------------------------------------------------------------
void *KeyboardReader::startDaemonThread_(void *keyboardReaderInstance)
{
    ((KeyboardReader *)keyboardReaderInstance)->daemonMainLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function is a daemon to start reading user inputs
//-----------------------------------------------------------------------
void KeyboardReader::daemonMainLoop_(void)
{
    daemonThreadIsUp_ = true;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Catch and proces the user input.
        catchAndProcessInput_();

        // Make sure termination was not requested.
        pthread_testcancel();

        // Sleep a little to reduce the CPU.
        //usleep(DAEMON_USLEEPTIME_);
        nanosleep(&DAEMON_SLEEPTIME_NSEC_, NULL);
    }
}

//-----------------------------------------------------------------------
// This function starts up the daemon to start reading user inputs
//-----------------------------------------------------------------------
void KeyboardReader::startDaemon(void)
{
    // Start the communication deamon thread.
    pthread_create(&daemonThread_, NULL, &startDaemonThread_, (void *)this);
}

//-----------------------------------------------------------------------
// This function reads the user input.
//-----------------------------------------------------------------------
void KeyboardReader::catchAndProcessInput_(void)
{
    char c;

    if (gWindows->displayDisabled())
    {
        return;
    }

    c = getch();

    switch(c) {

    case 'd':
    case 'D':
        if (gDebugger->debugEnabled())
        {
            gWindows->printfPlannerWindow("Debugger already enabled.\n");
        }
        else
        {
            gDebugger->enableDebug();
            gWindows->printfPlannerWindow("Debugger enabled.\n");
        }
        break;

    case 'l':
    case 'L':
        gCBRPlanner->loadCBRLibrary();
        displayDefaultKeybind();
        break;

    case 'q':
    case 'Q':
        if (gCBRPlanner->memoryChanged())
        {
            if (!(gCBRPlanner->confirmClear()))
            {
                displayDefaultKeybind();
                break;
            }
        }
        gWindows->printfPlannerWindow("Program terminated by the user.\n");
        gQuitProgram(0);
        break;

    case 's':
    case 'S':
        gCBRPlanner->saveCBRLibrary();
        displayDefaultKeybind();
        break;

    case 'x':
    case 'X':
        gWindows->printfPlannerWindow("Program terminated by the user.\n");
        gQuitProgram(0);
        break;
    }
}

//-----------------------------------------------------------------------
// This function display keyboard for the default keyboarding.
//-----------------------------------------------------------------------
void KeyboardReader::displayDefaultKeybind(void)
{
    gWindows->clearKeyboardWindow();
    gWindows->printfKeyboardWindow("d)ebug,  ");
    gWindows->printfKeyboardWindow("l)oad library,  ");
    gWindows->printfKeyboardWindow("s)ave library,  ");
    gWindows->printfKeyboardWindow("q)uit");
    gWindows->setKeyboardWindowTitle(KEYBIND_TITLE_);
}

/**********************************************************************
 * $Log: keyboard_reader.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
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
