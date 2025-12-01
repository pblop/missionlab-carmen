/**********************************************************************
 **                                                                  **
 **                             debugger.cc                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module deals with outputting debugging info.               **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: debugger.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <string>

#include "debugger.h"
#include "windows.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string Debugger::DEBUG_FILENAME_EXTENSION_ = "out";
const string Debugger::EMPTY_STRING_ = "";

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constructor for Debugger class.
//-----------------------------------------------------------------------
Debugger::Debugger(Debugger **debuggerInstance) : self_(debuggerInstance)
{
    if (debuggerInstance != NULL)
    {
        *debuggerInstance = this;
    }

    debug_ = false;
    debugFilename_ = EMPTY_STRING_;
    debugFile_ = NULL;
    out_ = NULL;
    outputType_ = DEBUG_FILE;
}

//-----------------------------------------------------------------------
// Distructor for Debugger class.
//-----------------------------------------------------------------------
Debugger::~Debugger(void)
{
    if (debugFile_ != NULL)
    {
        fclose(debugFile_);
        debugFile_ = NULL;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function prints a formatted text string to the output.
//-----------------------------------------------------------------------
void Debugger::printfDebug(const char *format, ...)
{
    va_list args;

    if (!debug_)
    {
        return;
    }

    if (out_ == NULL)
    {
        setOutputType(outputType_);
    }

    va_start(args, format);
    vfprintf(out_, format, args);
    va_end(args);

    fflush(out_);
}

//-----------------------------------------------------------------------
// This prints debugging info to the specified output.
//-----------------------------------------------------------------------
void Debugger::printfDebug(int outputType, const char *format, ...)
{
    va_list args;
    int originalOutputType;

    if (!debug_)
    {
        return;
    }

    if (out_ == NULL)
    {
        setOutputType(outputType_);
    }

    // Remember the current output type.
    originalOutputType = outputType_;

    // Set the output as the one specified by the user.
    setOutputType(outputType);

    // Print the deubug.
    va_start(args, format);
    vfprintf(out_, format, args);
    va_end(args);

    fflush(out_);

    // Reset the output as it was before.
    setOutputType(originalOutputType);
}

//-----------------------------------------------------------------------
// This function prints a dotted line to the output.
//-----------------------------------------------------------------------
void Debugger::printfLine(void)
{
    printfDebug("-----------------------------------------------------------------------\n");
}

//-----------------------------------------------------------------------
// This function sets the output type.
//-----------------------------------------------------------------------
void Debugger::setOutputType(int outputType)
{
    outputType_ = outputType;

    switch (outputType) {

    case DEBUG_FILE:
        if (debugFile_ == NULL)
        {
            openDebugFile_();
        }
        out_ = debugFile_;
        break;
    case DEBUG_STDOUT:
        out_ = stdout;
        break;
    }
}

//-----------------------------------------------------------------------
// This function sets the default debug file name prefix.
//-----------------------------------------------------------------------
string Debugger::createDebugFilenamePrefix_(void)
{
    time_t cur_time;
    tm local_time;
    string prefix;
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];
    char *userName = NULL, buf[1024];

    // Get the user name.
    userName = getenv("USER");

    // Get the current date and time.
    cur_time = time(NULL);
    strncpy(date,ctime(&cur_time),sizeof(date));
    local_time = *localtime(&cur_time);
    strftime(month, sizeof(month), "%m", &local_time);
    strftime(day, sizeof(day), "%d", &local_time);
    strftime(year, sizeof(year), "%Y", &local_time);
    strftime(hour, sizeof(hour), "%H", &local_time);
    strftime(min, sizeof(min), "%M", &local_time);
    strftime(sec, sizeof(sec), "%S", &local_time);

    // Create the prefix.
    sprintf(
        buf,
        "cbrserver-debug-%s-%s%s%s-%s%s%s",
        userName,
        month,
        day,
        year,
        hour,
        min,
        sec);
    prefix = buf;

    return prefix;
}

//-----------------------------------------------------------------------
// This function sets the default debug file name.
//-----------------------------------------------------------------------
void Debugger::setDefaultDebugFilename_(void)
{
    string filename;

    filename = createDebugFilenamePrefix_();
    filename += ".";
    filename += DEBUG_FILENAME_EXTENSION_;

    debugFilename_ = filename;
    
    gWindows->printfPlannerWindow(
        "Debugging outputs saved in \"%s\".\n",
        debugFilename_.c_str());
}

//-----------------------------------------------------------------------
// This function opens the default debug file.
//-----------------------------------------------------------------------
void Debugger::openDebugFile_(void)
{
    if (debugFile_ != NULL)
    {
        // It seems to be open.
        return;
    }

    if (debugFilename_ == EMPTY_STRING_)
    {
        // No name has been yet specified. Assign the default name.
        setDefaultDebugFilename_();
    }

    
    // Open the file.
    debugFile_ = fopen(debugFilename_.c_str(), "w");

    if (debugFile_ == NULL)
    {
        fprintf(
            stderr,
            "Error: debug file \"%s\" failed to open.\n",
            debugFilename_.c_str());
        gQuitProgram(-1);
    }
}

//-----------------------------------------------------------------------
// This function enable the dubugging and set the output file.
//-----------------------------------------------------------------------
void Debugger::enableDebug(int outputType)
{
    debug_ = true;
    outputType_ = outputType;
}

//-----------------------------------------------------------------------
// This function prints out the plan sequence for debugging.
//-----------------------------------------------------------------------
void Debugger::dumpPlanSequence(CBRPlanner_PlanSequence_t planSequence)
{
    int planSize, parameterSize, agentSize, i, j, k;

    planSize = planSequence.size();

    printfLine();

    for (i = 0; i < planSize; i++)
    {
        printfLine();
        printfDebug(
            "Operator: Name %s\n",
            planSequence[i].name.c_str());
        printfDebug(
            "Operator: Index %s\n",
            planSequence[i].index.c_str());
        parameterSize = planSequence[i].parameters.size();
        for (j = 0; j < parameterSize; j++)
        {
            printfDebug(
                "Operator: Parm %s %s\n",
                planSequence[i].parameters[j].name.c_str(),
                planSequence[i].parameters[j].value.c_str());
        }

        agentSize = planSequence[i].preconditions.size();
        for (j = 0; j < agentSize; j++)
        {
            printfDebug(
                "Precondition: Name %s\n",
                planSequence[i].preconditions[j].name.c_str());
        }

        agentSize = planSequence[i].effects.size();
        for (j = 0; j < agentSize; j++)
        {
            printfDebug(
                "Effects: Name %s\n",
                planSequence[i].effects[j].name.c_str());
            printfDebug(
                "Effects: Index %s\n",
                planSequence[i].effects[j].index.c_str());

            parameterSize = planSequence[i].effects[j].parameters.size();
            for (k = 0; k < parameterSize; k++)
            {
                printfDebug(
                    "Effects: Parm %s %s\n",
                    planSequence[i].effects[j].parameters[k].name.c_str(),
                    planSequence[i].effects[j].parameters[k].value.c_str());
            }
        }
    }

    printfLine();
}

/**********************************************************************
 * $Log: debugger.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/02/07 20:08:18  endo
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
