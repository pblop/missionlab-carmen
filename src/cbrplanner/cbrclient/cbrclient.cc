/**********************************************************************
 **                                                                  **
 **                              cbrclient.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the CBRClient class, whose     **
 **  job is to act as an interface between CfgEdit and CBRServer.    **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cbrclient.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/un.h>
#include <pthread.h>
#include <string>
#include <iostream>
#include <fstream>

#include "cbrclient.h"
#include "cbrplanner_protocol.h"
#include "EventLogging.h"

using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const string CBRClient::RCTABLE_SERVERSOCKET_STRING_= "CBRServerSocketName";
const string CBRClient::EMPTY_STRING_ = "";
const char CBRClient::CHAR_NULL_ = '\0';
const struct timespec CBRClient::READER_SLEEP_TIME_NSEC_ = {0, 1000000};

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------
pthread_mutex_t CBRClient::socketMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for CBRClient class.
//-----------------------------------------------------------------------
CBRClient::CBRClient(const symbol_table<rc_chain> &rcTable)
{
    shSocketFd_ = -1;
    rcTable_ = rcTable;
    readerThreadIsUp_ = false;

    pthread_mutex_init(&socketMutex_, NULL);
}

//-----------------------------------------------------------------------
// Distructor for CBRClient class.
//-----------------------------------------------------------------------
CBRClient::~CBRClient(void)
{
    pthread_mutex_lock(&socketMutex_);
    sendCommandToServer_(CBRPCOM_CMD_CLIENTEXIT);
    pthread_mutex_unlock(&socketMutex_);

    if (readerThreadIsUp_)
    {
        readerThreadIsUp_ = false;
        //pthread_mutex_lock(&socketMutex_);
        pthread_cancel(readerThread_);
        pthread_join(readerThread_, NULL );
        //pthread_mutex_unlock(&socketMutex_);
    }
}

//-----------------------------------------------------------------------
// This function initializes the CBRClient.
//-----------------------------------------------------------------------
bool CBRClient::initialize(void)
{
    ReaderThreadInput_t *data = NULL;
    bool status;

    pthread_mutex_init(&socketMutex_, NULL);

    // Connect to server.
    status = connectToServer_();

    if (!status)
    {
        fprintf(stderr, "Error(libcbrclient.a): CBRClient::initialize(). Failed to connect with the server.\n");
        return false;
    }
    
    // Set up the input for the thread.
    data = new ReaderThreadInput_t;
    data->cbrClientInstance = this;

    // Start the reader thread.
    pthread_create(&readerThread_, NULL, &startReaderThread_, (void *)data);

    return true;
}

//-----------------------------------------------------------------------
// This function connects to the server through a socket.
//-----------------------------------------------------------------------
bool CBRClient::connectToServer_(void)
{
    rc_chain *serverSocketNameList;
    sockaddr_un localAddress;
    char *serverSocketName = NULL;
    int status;
    char buf[UNIX_PATH_MAX];

    serverSocketNameList = (rc_chain *)rcTable_.get(RCTABLE_SERVERSOCKET_STRING_.c_str());

    if (serverSocketNameList != NULL)
    {
        serverSocketNameList->first(&serverSocketName);
    }

    if (serverSocketName == NULL)
    {
        // Create a default one using the user name.
        sprintf(buf, "/tmp/%s-cbrplanner.socket", getenv("USER"));
        serverSocketName = strdup(buf);
    }

    // Open the socket.
    shSocketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    if (shSocketFd_ < 0)
    {
        fprintf(stderr, "Error(libcbrclient.a): CBRClient::connectToServer(). Socket could not be opened.\n");
        return false;
    }

    // Connect to the socket.
    localAddress.sun_family = AF_UNIX;
    strcpy(localAddress.sun_path, serverSocketName);
    status = connect(shSocketFd_, (sockaddr *)&localAddress, sizeof(localAddress));

    if (status < 0)
    {
        fprintf(stderr, "Error(libcbrclient.a): CBRClient::initialize(). Connection to the server failed.\n");
        return false;
    }

    fprintf(stdout, "CBR Planner connected.\n");
    fflush(stdout);

    sendCommandToServer_(CBRPCOM_CMD_CLIENTINIT);
    sendClientInfo_(clientInfo_);

    return true;
}

//-----------------------------------------------------------------------
// This function starts up a thread that reads the server command.
//-----------------------------------------------------------------------
void *CBRClient::startReaderThread_(void *input)
{
    ReaderThreadInput_t *data = NULL;
    CBRClient *cbrClientInstance;

    // Unpack the data
    data = (ReaderThreadInput_t *)input;
    cbrClientInstance = data->cbrClientInstance;
    
    // Start the main loop for the connection
    cbrClientInstance->readerMainLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to read commands from the server.
//-----------------------------------------------------------------------
void CBRClient::readerMainLoop_(void)
{
    int i, numQueuedData = 0;

    readerThreadIsUp_ = true;

    while(true)
    {
        pthread_testcancel();

        pthread_mutex_lock(&socketMutex_);

        numQueuedData = pollServer_();

        // Read the arrived data, and process it based on
        // the instruction.
        for (i = 0; i < numQueuedData; i++)
        {
            readAndProcessServerData_();
        }

        pthread_mutex_unlock(&socketMutex_);

        if (numQueuedData == 0)
        {
            pthread_testcancel();

            nanosleep(&READER_SLEEP_TIME_NSEC_, NULL);
        }
    }
}

//-----------------------------------------------------------------------
// This function reads n bytes from the file descriptor.
//-----------------------------------------------------------------------
bool CBRClient::readNBytes_(const int fd, unsigned char *buf, const int n)
{
    int msgLength = -1, index = 0, remaining = 0;

    remaining = n;

    while (remaining > 0)
    {
        msgLength = read(fd, &buf[index], remaining);

        if (msgLength == -1)
        {
            fprintf(
                stderr,
                "Warning: CBRClient::readNBytes_(). Read error %d!\n",
                errno);

            return false;
        }

        index += msgLength;
        remaining -= msgLength;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function sends a command to the server.
//-----------------------------------------------------------------------
void CBRClient::sendCommandToServer_(const unsigned char cmd)
{
    unsigned char threeBytesBuf[3];

    threeBytesBuf[0] = CBRPCOM_SYNC_BYTE1;
    threeBytesBuf[1] = CBRPCOM_SYNC_BYTE2;
    threeBytesBuf[2] = cmd;

    write(shSocketFd_, (unsigned char *)&threeBytesBuf, 3);
}

//-----------------------------------------------------------------------
// This function sends a general data to the server.
//-----------------------------------------------------------------------
void CBRClient::sendDataToServer_(void *data, int dataSize)
{
    write(shSocketFd_, data, dataSize);
}

//-----------------------------------------------------------------------
// This function checks to see if any incoming data is ready to be
// delivered.
//-----------------------------------------------------------------------
int CBRClient::pollServer_(void)
{
    int numQueuedData;

    sendCommandToServer_(CBRPCOM_CMD_POLLING_DATA);

    numQueuedData = readInteger_(shSocketFd_);

    return numQueuedData;
}

//-----------------------------------------------------------------------
// This function reads an integer number.
//-----------------------------------------------------------------------
int CBRClient::readInteger_(const int fd)
{
    int number;

    readNBytes_(fd, (unsigned char *)&number, sizeof(int));

    return number;
}

//-----------------------------------------------------------------------
// This function reads a boolean value.
//-----------------------------------------------------------------------
bool CBRClient::readBoolean_(const int fd)
{
    bool value;

    readNBytes_(fd, (unsigned char *)&value, sizeof(bool));

    return value;
}

//-----------------------------------------------------------------------
// This function reads a charactor.
//-----------------------------------------------------------------------
char CBRClient::readChar_(const int fd)
{
    bool value;

    readNBytes_(fd, (unsigned char *)&value, 1);

    return value;
}

//-----------------------------------------------------------------------
// This function reads a string
//-----------------------------------------------------------------------
string CBRClient::readString_(const int fd)
{
    string value;
    char *data = NULL;
    int dataSize;

    // Read the length of the string.
    dataSize = readInteger_(fd);

    // Read the string.
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);
    data[dataSize] = CHAR_NULL_;
    value = data;

    return value;
}

//-----------------------------------------------------------------------
// This function writes an integer number.
//-----------------------------------------------------------------------
void CBRClient::writeInteger_(const int fd, int value)
{
    write(fd, &value, sizeof(int));
}

//-----------------------------------------------------------------------
// This function writes an double number.
//-----------------------------------------------------------------------
void CBRClient::writeDouble_(const int fd, double value)
{
    write(fd, &value, sizeof(double));
}

//-----------------------------------------------------------------------
// This function writes a boolean value.
//-----------------------------------------------------------------------
void CBRClient::writeBoolean_(const int fd, bool value)
{
    write(fd, &value, sizeof(bool));
}

//-----------------------------------------------------------------------
// This function writes a char value.
//-----------------------------------------------------------------------
void CBRClient::writeChar_(const int fd, char value)
{
    write(fd, &value, sizeof(char));
}

//-----------------------------------------------------------------------
// This function writes a string value.
//-----------------------------------------------------------------------
void CBRClient::writeString_(const int fd, string value)
{
    int dataSize;

    // Send the length of the string.
    dataSize = value.size();
    writeInteger_(fd, dataSize);

    // Send the data.
    write(fd, value.c_str(), dataSize);
}

//-----------------------------------------------------------------------
// This function sends the list of the state info to the server.
//-----------------------------------------------------------------------
void CBRClient::sendStateInfoList_(
    vector<CBRPlanner_StateInfo_t> stateInfoList)
{
    int i, numStateInfo;

    numStateInfo = stateInfoList.size();

    // Notify how many state info will be delivered.
    writeInteger_(shSocketFd_, numStateInfo);

    for (i = 0; i < numStateInfo; i++)
    {
        sendStateInfo_(stateInfoList[i]);
    }
}

//-----------------------------------------------------------------------
// This function sends the state info to the server.
//-----------------------------------------------------------------------
void CBRClient::sendStateInfo_(CBRPlanner_StateInfo_t stateInfo)
{
    int dataSize;

    // Send the FSA name.
    dataSize = stateInfo.fsaName.size();
    writeInteger_(shSocketFd_, dataSize);
    write(shSocketFd_, stateInfo.fsaName.c_str(), dataSize);

    // Send the State name.
    dataSize = stateInfo.stateName.size();
    writeInteger_(shSocketFd_, dataSize);
    write(shSocketFd_, stateInfo.stateName.c_str(), dataSize);
}

//-----------------------------------------------------------------------
// This function sends the state info to the server.
//-----------------------------------------------------------------------
void CBRClient::sendClientInfo_(CBRPlanner_ClientInfo_t clientInfo)
{
    int dataSize;

    // Send type.
    writeInteger_(shSocketFd_, clientInfo.type);

    // Send pid.
    writeInteger_(shSocketFd_, clientInfo.pid);

    // Send name.
    dataSize = clientInfo.name.size();
    writeInteger_(shSocketFd_, dataSize);
    write(shSocketFd_, clientInfo.name.c_str(), dataSize);
}

/**********************************************************************
 * $Log: cbrclient.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.4  2007/08/30 18:35:31  endo
 * Destructor improved.
 *
 * Revision 1.3  2007/08/10 15:15:39  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.2  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/06 04:23:54  endo
 * Destructor fixed.
 *
 * Revision 1.2  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1  2005/02/07 19:53:46  endo
 * Mods from usability-2004
 *
 * Revision 1.2  2003/04/06 08:48:14  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/13 01:08:36  endo
 * Initial revision
 *
 **********************************************************************/
