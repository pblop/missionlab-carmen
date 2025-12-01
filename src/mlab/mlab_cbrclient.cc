/**********************************************************************
 **                                                                  **
 **                          mlab_cbrclient.cc                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for cbrclient.cc.                         **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mlab_cbrclient.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

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
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/un.h>
#include <pthread.h>
#include <iostream>
#include <fstream>

#include "mission_design_types.h"
#include "mlab_cbrclient.h"
#include "cbrplanner_protocol.h"
#include "EventLogging.h"

using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const struct timespec MlabCBRClient::RETRIEVE_OVERLAY_INFO_SLEEPTIME_NSEC_ = {0, 1000000}; // 1 msec.
const struct timespec MlabCBRClient::RETRIEVE_LOGFILE_INFO_SLEEPTIME_NSEC_ = {0, 1000000}; // 1 msec.
const int MlabCBRClient::RETRIEVE_OVERLAY_INFO_SLEEPTIME_USEC_ = 1000; // 1 msec
const int MlabCBRClient::RETRIEVE_LOGFILE_INFO_SLEEPTIME_USEC_ = 1000; // 1 msec

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------
pthread_mutex_t MlabCBRClient::overlayInfoMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MlabCBRClient::logfileInfoMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for MlabCBRClient class.
//-----------------------------------------------------------------------
MlabCBRClient::MlabCBRClient(const symbol_table<rc_chain> &rcTable, int type) :
    CBRClient(rcTable)
{
    clientInfo_.type = type;
    clientInfo_.pid = getpid();
    clientInfo_.name = "mlab";

    shOverlayInfoReady_ = false;
    shLogfileInfoReady_ = false;

    pthread_mutex_init(&overlayInfoMutex_, NULL);
    pthread_mutex_init(&logfileInfoMutex_, NULL);
}

//-----------------------------------------------------------------------
// Distructor for MlabCBRClient class.
//-----------------------------------------------------------------------
MlabCBRClient::~MlabCBRClient(void)
{
}

//-----------------------------------------------------------------------
// This function retrieves overlay info from the server.
//-----------------------------------------------------------------------
CBRPlanner_OverlayInfo_t MlabCBRClient::retrieveOverlayInfo(void)
{
    CBRPlanner_OverlayInfo_t overlayInfo;
    bool overlayInfoReady = false;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that overlay info needs
    // to be delivered.
    sendCommandToServer_(CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE);

    pthread_mutex_unlock(&socketMutex_);

    // Wait until the overlay to be ready.
    while (true)
    {
        pthread_testcancel();

        pthread_mutex_lock(&overlayInfoMutex_);

        if (shOverlayInfoReady_)
        {
            overlayInfoReady = shOverlayInfoReady_;
            overlayInfo = shOverlayInfo_;
            shOverlayInfoReady_ = false;
        }
 
        pthread_mutex_unlock(&overlayInfoMutex_);

        if (overlayInfoReady)
        {
            break;
        }

        pthread_testcancel();

        nanosleep(&RETRIEVE_OVERLAY_INFO_SLEEPTIME_NSEC_, NULL);
    }

    return overlayInfo;
}

//-----------------------------------------------------------------------
// This function retrieves logfile info from the server.
//-----------------------------------------------------------------------
vector<CBRPlanner_LogfileInfo_t> MlabCBRClient::retrieveLogfileInfoList(void)
{
    vector<CBRPlanner_LogfileInfo_t> logfileInfoList;
    bool logfileInfoReady = false;

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that logfile info needs
    // to be delivered.
    sendCommandToServer_(CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE);

    pthread_mutex_unlock(&socketMutex_);

    // Wait until the logfile to be ready.
    while (true)
    {
        pthread_testcancel();

        pthread_mutex_lock(&logfileInfoMutex_);

        if (shLogfileInfoReady_)
        {
            logfileInfoReady = shLogfileInfoReady_;
            logfileInfoList = shLogfileInfoList_;
            shLogfileInfoList_.clear();
            shLogfileInfoReady_ = false;
        }
 
        pthread_mutex_unlock(&logfileInfoMutex_);

        if (logfileInfoReady)
        {
            break;
        }

        pthread_testcancel();

        nanosleep(&RETRIEVE_LOGFILE_INFO_SLEEPTIME_NSEC_, NULL);
    }

    return logfileInfoList;
}

//-----------------------------------------------------------------------
// This function sends the logfile info to CfgEdit via CBRServer.
//-----------------------------------------------------------------------
void MlabCBRClient::saveLogfileInfo(CBRPlanner_LogfileInfo_t logfileInfo)
{
    int dataSize;
    char *data = NULL;

    dataSize = logfileInfo.name.size();
    data = new char[dataSize+1];
    sprintf(data, logfileInfo.name.c_str());

    pthread_mutex_lock(&socketMutex_);

    // First, send a flag to notify the server that logfile info will be
    // delivered.
    sendCommandToServer_(CBRPCOM_CMD_LOGFILE_INFO_SAVE);

    writeInteger_(shSocketFd_, dataSize);
    write(shSocketFd_, (unsigned char *)data, dataSize);

    pthread_mutex_unlock(&socketMutex_);

    if (data != NULL)
    {
        delete [] data;
    }
}

//-----------------------------------------------------------------------
// This function retrieves the repair question.
//-----------------------------------------------------------------------
MMDRepairQuestion_t MlabCBRClient::retrieveRepairQuestion(int lastSelectedQuestion)
{
    MMDRepairQuestion_t repairQuestion;
    string option;
    int i, dataSize, numOptions;
    char *data = NULL;
    
    pthread_mutex_lock(&socketMutex_);

    // First, send the flag.
    sendCommandToServer_(CBRPCOM_CMD_REPAIR_Q_RETRIEVE);

    // Send the last selected question.
    writeInteger_(shSocketFd_, lastSelectedQuestion);

    // Get the status.
    repairQuestion.status = readInteger_(shSocketFd_);

    // Get the type.
    repairQuestion.type = readInteger_(shSocketFd_);

    // Get the question
    dataSize = readInteger_(shSocketFd_);
    data = new char[dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);
    data[dataSize] = '\0';
    repairQuestion.question = data;
    delete [] data;
    data = NULL;

    // Get the number of options.
    numOptions = readInteger_(shSocketFd_);

    // Get the options.
    for (i = 0; i < numOptions; i++)
    {
        dataSize = readInteger_(shSocketFd_);
        data = new char[dataSize+1];
        readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);
        data[dataSize] = '\0';
        option = data;
        delete [] data;
        data = NULL;

        repairQuestion.options.push_back(option);
    }

    pthread_mutex_unlock(&socketMutex_);

    return repairQuestion;
}

//-----------------------------------------------------------------------
// This function retrieves the repair question.
//-----------------------------------------------------------------------
MMDRepairQuestion_t MlabCBRClient::retrieveRepairFollowUpQuestion(void)
{
    MMDRepairQuestion_t repairQuestion;
    string option;
    int i, dataSize, numOptions;
    char *data = NULL;
    
    pthread_mutex_lock(&socketMutex_);

    // First, send the flag.
    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_RETRIEVE);

    // Get the status.
    repairQuestion.status = readInteger_(shSocketFd_);

    // Get the type.
    repairQuestion.type = readInteger_(shSocketFd_);

    // Get the question
    dataSize = readInteger_(shSocketFd_);
    data = new char[dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);
    data[dataSize] = '\0';
    repairQuestion.question = data;
    delete [] data;
    data = NULL;

    // Get the number of options.
    numOptions = readInteger_(shSocketFd_);

    // Get the options.
    for (i = 0; i < numOptions; i++)
    {
        dataSize = readInteger_(shSocketFd_);
        data = new char[dataSize+1];
        readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);
        data[dataSize] = '\0';
        option = data;
        delete [] data;
        data = NULL;

        repairQuestion.options.push_back(option);
    }

    pthread_mutex_unlock(&socketMutex_);

    return repairQuestion;
}

//-----------------------------------------------------------------------
// This function sends the answer (toggle) to the server.
//-----------------------------------------------------------------------
void MlabCBRClient::replyRepairFollowUpQuestionToggle(int selectedOption)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_TOGGLE_REPLY);
    writeInteger_(shSocketFd_, selectedOption);
    
    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends the answer (map point) to the server.
//-----------------------------------------------------------------------
void MlabCBRClient::replyRepairFollowUpQuestionAddMapPoint(double x, double y)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_ADD_MAP_POINT_REPLY);
    writeDouble_(shSocketFd_, x);
    writeDouble_(shSocketFd_, y);
    
    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends the answer (a pair of map points) to the server.
//-----------------------------------------------------------------------
void MlabCBRClient::replyRepairFollowUpQuestionMoveMapPoint(
    double oldX,
    double oldY,
    double newX,
    double newY,
    double closeRange)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_MOD_MAP_POINT_REPLY);
    writeDouble_(shSocketFd_, oldX);
    writeDouble_(shSocketFd_, oldY);
    writeDouble_(shSocketFd_, newX);
    writeDouble_(shSocketFd_, newY);
    writeDouble_(shSocketFd_, closeRange);
    
    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends the answer (map point) to the server.
//-----------------------------------------------------------------------
void MlabCBRClient::replyRepairFollowUpQuestionDeleteMapPoint(
    double x,
    double y,
    double closeRange)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_DEL_MAP_POINT_REPLY);
    writeDouble_(shSocketFd_, x);
    writeDouble_(shSocketFd_, y);
    writeDouble_(shSocketFd_, closeRange);
    
    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends the answer (state info) to the server.
//-----------------------------------------------------------------------
void MlabCBRClient::replyRepairFollowUpQuestionStateName(
    vector<CBRPlanner_StateInfo_t> stateInfoList)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_STATENAME_REPLY);

    sendStateInfoList_(stateInfoList);

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function sends the state info to CfgEdit via CBRServer.
//-----------------------------------------------------------------------
void MlabCBRClient::reportCurrentState(
    vector<CBRPlanner_StateInfo_t> stateInfoList)
{
    pthread_mutex_lock(&socketMutex_);

    sendCommandToServer_(CBRPCOM_CMD_REPORT_CURRENT_STATE);

    sendStateInfoList_(stateInfoList);

    pthread_mutex_unlock(&socketMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves overlay info from the server.
//-----------------------------------------------------------------------
void MlabCBRClient::retrieveOverlayInfoFromServer_(void)
{
    CBRPlanner_OverlayInfo_t overlayInfo;
    int dataSize;
    char *data = NULL;

    dataSize = readInteger_(shSocketFd_);
    data = new char[dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);

    data[dataSize] = '\0';
    overlayInfo.name = data;

    delete [] data;
    data = NULL;

    pthread_mutex_lock(&overlayInfoMutex_);

    shOverlayInfo_ = overlayInfo;
    shOverlayInfoReady_ = true;

    pthread_mutex_unlock(&overlayInfoMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves logfile info from the server.
//-----------------------------------------------------------------------
void MlabCBRClient::retrieveLogfileInfoFromServer_(void)
{
    CBRPlanner_LogfileInfo_t logfileInfo;
    int dataSize;
    char *data = NULL;

    dataSize = readInteger_(shSocketFd_);
    data = new char[dataSize+1];
    readNBytes_(shSocketFd_, (unsigned char *)data, dataSize);

    logfileInfo.name = data;
    logfileInfo.name[dataSize] = '\0';
    delete [] data;
    data = NULL;

    pthread_mutex_lock(&logfileInfoMutex_);

    shLogfileInfoList_.push_back(logfileInfo);

    pthread_mutex_unlock(&logfileInfoMutex_);
}

//-----------------------------------------------------------------------
// This function retrieves logfile info acknowledgment from the server,
// indicating that all the logfiles have been arrived.
//-----------------------------------------------------------------------
void MlabCBRClient::retrieveLogfileInfoAcknowledgment_(void)
{
    readInteger_(shSocketFd_);

    pthread_mutex_lock(&logfileInfoMutex_);
    shLogfileInfoReady_ = true;
    pthread_mutex_unlock(&logfileInfoMutex_);
}

//-----------------------------------------------------------------------
// This function read and process the server data.
//-----------------------------------------------------------------------
void MlabCBRClient::readAndProcessServerData_(void)
{
    unsigned char oneByteBuf[1];
    const bool DEBUG_READ_AND_PROCESS_SERVER_DATA = false;

    readNBytes_(shSocketFd_, (unsigned char *)&oneByteBuf, 1);

    if (DEBUG_READ_AND_PROCESS_SERVER_DATA)
    {
        fprintf(
            stderr,
            "MlabCBRClient::readAndProcessServerData_(): Byte [%x] receieved.\n",
            oneByteBuf[0]);
    }

    switch (oneByteBuf[0]) {

    case CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE:
        retrieveOverlayInfoFromServer_();
        break;

    case CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE:
        retrieveLogfileInfoFromServer_();
        break;

    case CBRPCOM_CMD_LOGFILE_INFO_ACKNOWLEDGE:
        retrieveLogfileInfoAcknowledgment_();
        break;

    default:
        break;
    }
}

/**********************************************************************
 * $Log: mlab_cbrclient.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.2  2005/12/02 21:16:14  endo
 * Memory leak fix.
 *
 * Revision 1.1  2005/02/07 23:12:36  endo
 * Mods from usability-2004
 *
 * Revision 1.2  2003/04/06 08:48:14  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/13 01:08:36  endo
 * Initial revision
 *
 **********************************************************************/
