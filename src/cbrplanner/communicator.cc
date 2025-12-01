/**********************************************************************
 **                                                                  **
 **                           communicator.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This module serves as an interface to the client by using       **
 **  a socket-based communication with it.                           **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: communicator.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/un.h>
#include <string>

#include "communicator.h"
#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "cbrplanner.h"
#include "domain_manager.h"
#include "memory_manager.h"
#include "repair_plan_types.h"
#include "repair_plan.h"
#include "windows.h"
#include "debugger.h"

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
extern void gQuitProgram(int exitStatus); // Defined in main.cc

//-----------------------------------------------------------------------
// Constanns
//-----------------------------------------------------------------------
const char Communicator::CHAR_NULL_ = '\0';
const int Communicator::SOCKET_BACKLOG_SIZE_ = 5;
const int Communicator::SELECT_WAIT_SEC_ = 10;
const int Communicator::MAX_SYNC_ERROR_COUNT_ = 5;
const int Communicator::DUMMY_DATA_SIZE_ = 1;

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------
pthread_mutex_t Communicator::privateConnectionInfoListMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t Communicator::queuedDataMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor for CBRPlanner class.
//-----------------------------------------------------------------------
Communicator::Communicator(Communicator **communicatorInstance) : self_(communicatorInstance)
{
    if (communicatorInstance != NULL)
    {
        *communicatorInstance = this;
    }

    socketName_ = "";
    socketFd_ = -1;
    daemonThreadIsUp_ = false;

    pthread_mutex_init(&privateConnectionInfoListMutex_, NULL);
    pthread_mutex_init(&queuedDataMutex_, NULL);
}
//-----------------------------------------------------------------------
// Distructor for Communicator class.
//-----------------------------------------------------------------------
Communicator::~Communicator(void)
{
    // Close all the private connections.
    disconnectAllPrivateConnections_();

    // Make sure to kill the deamon thread.
    if (daemonThreadIsUp_)
    {
        pthread_cancel(daemonThread_);
        pthread_join(daemonThread_, NULL );
        daemonThreadIsUp_ = false;
    }

    if (self_ != NULL)
    {
        *self_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function starts up the daemon to initiate the communication with
// client.
//-----------------------------------------------------------------------
void Communicator::startDaemon(void)
{
    char buf[UNIX_PATH_MAX];

    if (socketName_ == "")
    {
        // The name of the socket is not yet assigned. Create it using
        // the user name.
        sprintf(buf, "/tmp/%s-cbrplanner.socket", getenv("USER"));
        socketName_ = buf;
    }

    // Delete the old socket.
    unlink(socketName_.c_str());

    // Open the socket and bind the name to it.
    setupSocket_();

    // Start the communication deamon thread.
    pthread_create(&daemonThread_, NULL, &startDaemonThread_, (void *)this );
}

//-----------------------------------------------------------------------
// This function starts up a thread to run the communication daemon.
//-----------------------------------------------------------------------
void *Communicator::startDaemonThread_(void *communicatorInstance)
{
    ((Communicator *)communicatorInstance)->daemonMainLoop_();

    return NULL;
}

//-----------------------------------------------------------------------
// This function starts up the daemon to initiate the communication with
// CfgEdit / mlab.
//-----------------------------------------------------------------------
void Communicator::daemonMainLoop_(void)
{
    int count = 0;
    int fd = -1;

    daemonThreadIsUp_ = true;

    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Wait for a client connection at the socket.
        fd = waitForNewConnection_(count);

        // Make sure termination was not requested.
        pthread_testcancel();

        // Start another thread specific to this connection.
        startPrivateConnection_(fd);

        count++;
    }
}

//-----------------------------------------------------------------------
// This function opens a socket and bind the given name to it. It returns
// the file descriptor of the socket.
//-----------------------------------------------------------------------
void Communicator::setupSocket_(void)
{
    int status;
    sockaddr_un localAddress;

    // Open the socket.
    socketFd_ = socket(AF_UNIX, SOCK_STREAM, 0);

    if (socketFd_ < 0)
    {
        gWindows->printfCommWindow(
            "Error: Communicator::setupSocket_()[%s]. Socket could not be opened [%s].\n",
            __FILE__,
            strerror(errno));
        gQuitProgram(-1);
    }

    // Bind the name to a socket.
    localAddress.sun_family = AF_UNIX;
    strcpy(localAddress.sun_path, socketName_.c_str());
    status = bind(socketFd_, (sockaddr *)&localAddress, sizeof(localAddress));

    if (status < 0)
    {
        gWindows->printfCommWindow(
            "Error: Communicator::setupSocket_()[%s]. \"%s\" could not be binded to the socket [%s].\n",
            __FILE__,
            socketName_.c_str(),
            strerror(errno));
        gQuitProgram(-1);
    }
}

//-----------------------------------------------------------------------
// This function listens the socket for a client connection. After the
// communication is established and it returns a new file descriptor to
// talks to the cilent privately.
//-----------------------------------------------------------------------
int Communicator::waitForNewConnection_(int count)
{
    int newFd = -1, status = -1;
    sockaddr_un remoteAddress;
    socklen_t len;

    // Listen the socket for connection.
    status = listen(socketFd_, SOCKET_BACKLOG_SIZE_);

    if (status < 0)
    {
        gWindows->printCommWindow("Error: Listening socket failed.\n");
        gQuitProgram(-1);
    }

    gWindows->printfCommWindow(
        "Waiting for %s client connection.\n",
        (count > 0)? "another" : "the first");

    // Wait for the connection with CfgEdit
    newFd = accept(socketFd_, (sockaddr *)&remoteAddress, &len);

    if (newFd < 0)
    {
        gWindows->printCommWindow("Error: Connection to the client failed.\n");
        gQuitProgram(-1);
    }

    gWindows->clearCommWindow();
    gWindows->printCommWindow("Connection to the client established.\n");

    return newFd;
}

//-----------------------------------------------------------------------
// This function starts another thread to talk only to this connection.
//-----------------------------------------------------------------------
void Communicator::startPrivateConnection_(const int fd)
{
    PrivateConnectionThreadInput_t *data = NULL;
    PrivateConnectionInfo_t connectionInfo;
    pthread_t newThread;

    // First, add this fd to the connection info list.
    connectionInfo.fd = fd;
    connectionInfo.connectionThreadIsUp = false;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    shPrivateConnectionInfoList_.push_back(connectionInfo);
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    // Set up the input for the thread.
    data = new PrivateConnectionThreadInput_t;
    data->fd = fd;
    data->communicatorInstance = this;

    // Start the new thread to talk to this connection.
    pthread_create(&newThread, NULL, &startPrivateConnectionThread_, (void *)data);

    saveConnectionThread_(fd, newThread);
}

//-----------------------------------------------------------------------
// This function starts up a thread to communicate with this connection
// privately.
//-----------------------------------------------------------------------
void *Communicator::startPrivateConnectionThread_(void *input)
{
    PrivateConnectionThreadInput_t *data = NULL;
    int fd;
    Communicator *communicatorInstance;

    // Unpack the data
    data = (PrivateConnectionThreadInput_t *)input;
    fd = data->fd;
    communicatorInstance = data->communicatorInstance;
    
    // Start the main loop for the connection
    communicatorInstance->privateConnectionMainLoop_(fd);

    return NULL;
}

//-----------------------------------------------------------------------
// This function loops around to read commands from the client.
//-----------------------------------------------------------------------
void Communicator::privateConnectionMainLoop_(const int fd)
{
    setConnectionThreadIsUp_(fd, true);
    
    while(true)
    {
        // Make sure termination was not requested.
        pthread_testcancel();

        // Read the arrived data, and process it based on the instruction.
        readAndProcessPrivateConnectionData_(fd);
    }
}

//-----------------------------------------------------------------------
// This function reads the data sent by the client, and process it
// according to the instruction.
//-----------------------------------------------------------------------
bool Communicator::readAndProcessPrivateConnectionData_(const int fd)
{
    CBRPlanner_ClientInfo_t clientInfo;
    CBRPlanner_ACDLMissionPlan_t acdlPlan;
    CBRPlanner_FSANames_t fsaNames;
    CBRPlanner_OverlayInfo_t overlayInfo;
    CBRPlanner_LogfileInfo_t logfileInfo;
    CBRPlanner_StateInfo_t highlightedStateInfo;
    CBRPlanner_CNPStatus_t cnpStatus;
    OffendingState_t offendingState;
    RepairQuestion_t repairQuestion;
    vector<CBRPlanner_StateInfo_t> stateInfoList;
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans;
    vector<CBRPlanner_RobotIDs_t> robotIDList;
    vector<CBRPlanner_ExtraData_t> extraDataList;
    vector<MemoryManager::Constraints_t> genericConstraintsList;
    vector<MemoryManager::TaskConstraints_t> taskConstraintsList;
    vector<MemoryManager::RobotConstraints_t> robotConstraintsList;
    vector<string> acdlMissions;
    vector<int> dataIndexList;
    MemoryManager::Features_t features;
    string filename, acdlMission;
    unsigned char oneByteBuf[1];
    double oldX, oldY, newX, newY;
    double closeRange;
    int i, maxRating, index, numLogfileInfo, numMissions, numRobots;
    int selectedOption, lastSelectedOption;
    bool synced = false, received = false, fixed = false;
    bool addStartSubMission = true;
    const bool DEBUG_READ_AND_PROCESS_PRIVATE_CONNECTION_DATA = false;
    static long int debugReadAndProcessPrivateConnectionDataCounter = 0;

    // Make sure that it is synced to the client.
    synced = syncToClient_(fd);

    if (!synced)
    {
        // Not synced. Something went wrong. Disconnect it.
        disconnectPrivateConnection_(fd);
        return false;
    }

    // Now it is synced. Get a command byte.
    received = readNBytes_(fd, (unsigned char *)&oneByteBuf, 1);

    if (DEBUG_READ_AND_PROCESS_PRIVATE_CONNECTION_DATA)
    {
        gWindows->printfPlannerWindow(
            "[%ld] Byte(%x) receieved.\n",
            debugReadAndProcessPrivateConnectionDataCounter++,
            oneByteBuf[0]);
    }

    switch (oneByteBuf[0]) {

    case CBRPCOM_CMD_CLIENTINIT:
        clientInfo = readClientInfo_(fd);
        updateClientInfo_(fd, clientInfo);
        break;

    case CBRPCOM_CMD_CLIENTEXIT:
        disconnectPrivateConnection_(fd);
        break;

    case CBRPCOM_CMD_FSANAMES:
        fsaNames = readFSANames_(fd);
        gCBRPlanner->updateFSANames(fsaNames);
        break;

    case CBRPCOM_CMD_MISSIONPLAN_SAVE:
        acdlPlan = readACDLMissionPlan_(fd);
        features = readFeatures_(fd);
        dataIndexList = gCBRPlanner->saveNewMissionPlan(
            acdlPlan,
            features);
        writeDataIndexList_(fd, dataIndexList);
        break;

    case CBRPCOM_CMD_MISSIONPLAN_RETRIEVE:
        features = readFeatures_(fd);
        maxRating = readInteger_(fd);
        addStartSubMission = readBoolean_(fd);
        acdlPlans = gCBRPlanner->getACDLMissionPlans(features, maxRating, addStartSubMission);
        writeACDLMissionPlans_(fd, acdlPlans);
        break;

    case CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_PREMISSION:
        features = readFeatures_(fd);
        maxRating = readInteger_(fd);
        addStartSubMission = readBoolean_(fd);
        genericConstraintsList = readConstraints_(fd);
        taskConstraintsList = (vector<MemoryManager::TaskConstraints_t>)genericConstraintsList;
        genericConstraintsList = readConstraints_(fd);
        robotConstraintsList = (vector<MemoryManager::RobotConstraints_t>)genericConstraintsList;
        acdlPlans = gCBRPlanner->getACDLMissionPlansPremissionCNP(
            features,
            maxRating,
            addStartSubMission,
            taskConstraintsList,
            robotConstraintsList);
        writeACDLMissionPlans_(fd, acdlPlans);
        break;

    case CBRPCOM_CMD_MISSIONPLAN_RETRIEVE_CNP_RUNTIME:
        features = readFeatures_(fd);
        maxRating = readInteger_(fd);
        addStartSubMission = readBoolean_(fd);
        genericConstraintsList = readConstraints_(fd);
        robotConstraintsList = (vector<MemoryManager::RobotConstraints_t>)genericConstraintsList;
        acdlPlans = gCBRPlanner->getACDLMissionPlansRuntimeCNP(
            features,
            maxRating,
            addStartSubMission,
            robotConstraintsList);
        writeACDLMissionPlans_(fd, acdlPlans);
        break;

    case CBRPCOM_CMD_ROBOT_ID_LIST_RETRIEVE:
        robotIDList = gCBRPlanner->getCurrentRobotIDList();
        writeRobotIDList_(fd, robotIDList);
        break;

    case CBRPCOM_CMD_CNP_STATUS_RETRIEVE:
        cnpStatus = gCBRPlanner->getCurrentCNPStatus();
        writeCNPStatus_(fd, cnpStatus);
        break;

    case CBRPCOM_CMD_MISSIONPLAN_FIX:
        acdlPlan = readACDLMissionPlan_(fd);
        acdlPlan = gCBRPlanner->repairMissionPlan(acdlPlan, &fixed);
        if (fixed)
        {
            writeBoolean_(fd, true);
            writeACDLMissionPlan_(fd, acdlPlan);
        }
        else
        {
            writeBoolean_(fd, false);
        }
        break;

    case CBRPCOM_CMD_SELECTED_MISSION:
        index = readInteger_(fd);
        break;

    case CBRPCOM_CMD_POSITIVE_FEEDBACK:
        dataIndexList = readDataIndexList_(fd);
        gCBRPlanner->givePositiveFeedback(dataIndexList);
        break;

    case CBRPCOM_CMD_NEGATIVE_FEEDBACK:
        dataIndexList = readDataIndexList_(fd);
        gCBRPlanner->giveNegativeFeedback(dataIndexList);
        break;

    case CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE:
        clientInfo = getClientInfo_(fd);

        switch (clientInfo.type) {

        case CBRPLANNER_CLIENT_CFGEDIT:
            overlayInfo = readOverlayInfo_(fd);
            queueOverlayInfo_(
                overlayInfo,
                CBRPLANNER_CLIENT_MLAB_MPB,
                CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE);
            break;

        case CBRPLANNER_CLIENT_MLAB_MPB:
            queueRequestInfo_(
                CBRPLANNER_CLIENT_CFGEDIT,
                CBRPCOM_CMD_OVERLAY_INFO_RETRIEVE);
            break;
        }
        break;

    case CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE:
        clientInfo = getClientInfo_(fd);

        switch (clientInfo.type) {

        case CBRPLANNER_CLIENT_CFGEDIT:
            numLogfileInfo = readInteger_(fd);
            for (i = 0; i < numLogfileInfo; i++)
            {
                logfileInfo = readLogfileInfo_(fd);
                queueLogfileInfo_(
                    logfileInfo,
                    CBRPLANNER_CLIENT_MLAB_MPB,
                    CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE);
            }
            queueSimpleCommand_(
                CBRPLANNER_CLIENT_MLAB_MPB,
                CBRPCOM_CMD_LOGFILE_INFO_ACKNOWLEDGE);
            break;

        case CBRPLANNER_CLIENT_MLAB_MPB:
            queueRequestInfo_(
                CBRPLANNER_CLIENT_CFGEDIT,
                CBRPCOM_CMD_LOGFILE_INFO_RETRIEVE);
            break;
        }
        break;

    case CBRPCOM_CMD_LOGFILE_INFO_SAVE:
        logfileInfo = readLogfileInfo_(fd);
        queueLogfileInfo_(
            logfileInfo,
            CBRPLANNER_CLIENT_CFGEDIT,
            CBRPCOM_CMD_LOGFILE_INFO_SAVE);
        break;

    case CBRPCOM_CMD_POLLING_DATA:
        sendOffQueuedData_(fd);
        break;

    case CBRPCOM_CMD_REPORT_CURRENT_STATE:
        stateInfoList = readStateInfoList_(fd);
        queueStateInfo_(
            stateInfoList,
            CBRPLANNER_CLIENT_CFGEDIT,
            CBRPCOM_CMD_REPORT_CURRENT_STATE);
        break;

    case CBRPCOM_CMD_REPAIR_Q_RETRIEVE:
        lastSelectedOption = readInteger_(fd);
        repairQuestion = gCBRPlanner->getRepairQuestion(lastSelectedOption);
        writeRepairQuestion_(fd, repairQuestion);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_RETRIEVE:
        repairQuestion = gCBRPlanner->getRepairFollowUpQuestion();
        writeRepairQuestion_(fd, repairQuestion);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_TOGGLE_REPLY:
        selectedOption = readInteger_(fd);
        gCBRPlanner->putRepairFollowUpQuestionToggleAnswer(selectedOption);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_ADD_MAP_POINT_REPLY:
        newX = readDouble_(fd);
        newY = readDouble_(fd);
        gCBRPlanner->putRepairFollowUpQuestionAddMapPointAnswer(
            newX,
            newY);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_MOD_MAP_POINT_REPLY:
        oldX = readDouble_(fd);
        oldY = readDouble_(fd);
        newX = readDouble_(fd);
        newY = readDouble_(fd);
        closeRange = readDouble_(fd);
        gCBRPlanner->putRepairFollowUpQuestionModMapPointAnswer(
            oldX,
            oldY,
            newX,
            newY,
            closeRange);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_DEL_MAP_POINT_REPLY:
        oldX = readDouble_(fd);
        oldY = readDouble_(fd);
        closeRange = readDouble_(fd);
        gCBRPlanner->putRepairFollowUpQuestionDelMapPointAnswer(
            oldX,
            oldY,
            closeRange);
        break;

    case CBRPCOM_CMD_REPAIR_FOLLOWUP_Q_STATENAME_REPLY:
        stateInfoList = readStateInfoList_(fd);
        // Note: this case is obsolete as the state index will be
        // sent via CfgEdit (rather than mlab).
        break;

    case CBRPCOM_CMD_HIGHLIGHTED_STATE:
        highlightedStateInfo = readStateInfo_(fd);
        offendingState.description = highlightedStateInfo.stateName;
        gCBRPlanner->updateOffendingState(offendingState);
        break;

    case CBRPCOM_CMD_EXTRADATA_LIST:
        dataIndexList = readDataIndexList_(fd);
        extraDataList = readExtraDataList_(fd);
        gCBRPlanner->saveExtraDataList(dataIndexList, extraDataList);
        break;
  
    case CBRPCOM_CMD_USAGE_COUNTER_UPDATE:
        dataIndexList = readDataIndexList_(fd);
        gCBRPlanner->updateMissionUsageCounter(dataIndexList);
        break;

    case CBRPCOM_CMD_CHECK_CBR_LIBRARY_FILE_SAVE:
        writeBoolean_(fd, gCBRPlanner->memoryChanged());
        writeString_(fd, gCBRPlanner->getCBRLibraryName());
        break;

    case CBRPCOM_CMD_SAVE_CBR_LIBRARY_FILE:
        filename = readString_(fd);
        gCBRPlanner->saveCBRLibrary(filename);
        break;

    case CBRPCOM_CMD_REVERSE_ROBOT_ORDER:
        acdlMission = readString_(fd);
        acdlMission = gCBRPlanner->reverseRobotOrder(acdlMission); 
        writeString_(fd, acdlMission);
        break;

    case CBRPCOM_CMD_MERGE_MISSIONS:
        numMissions = readInteger_(fd);
        for (i = 0; i < numMissions; i++)
        {
            acdlMission = readString_(fd);
            acdlMissions.push_back(acdlMission);
        }
        acdlMission = gCBRPlanner->mergeMissions(acdlMissions, &numRobots); 
        writeInteger_(fd, numRobots);
        writeString_(fd, acdlMission);
        break;
  }

    return true;
}

//-----------------------------------------------------------------------
// This function makes sure that the server and the client are syncronized.
// Two keys are used to check the syncronization.
//-----------------------------------------------------------------------
bool Communicator::syncToClient_(const int fd)
{
    int errorCount = 0;
    unsigned char oneByteBuf[1];
    bool synced = false, received = false;


    while (!synced)
    {
        received = readNBytes_(fd, (unsigned char *)&oneByteBuf, 1);

        if (received)
        {
            if (oneByteBuf[0] == CBRPCOM_SYNC_BYTE1)
            {
                // The first sync byte is OK. Check the second one.
                received = readNBytes_(fd, (unsigned char *)&oneByteBuf, 1);

                if (received)
                {
                    if (oneByteBuf[0] == CBRPCOM_SYNC_BYTE2)
                    {
                        // The second sync byte matched as well. Assuming
                        // this is a right data.
                        synced = true;
                        break;
                    }
                    else
                    {
                        // It seems, it was not the header. Continue to read.
                        continue;
                    }
                }
                else
                {
                    // Read error occured.
                    errorCount++;
                }
            }
            else
            {
                // It seems, it was not the header. Continue to read.
                continue;
            }
        }
        else
        {
            // Read error occured.
            errorCount++;
        }

        // Check if the errors were occured too often.
        if (errorCount > MAX_SYNC_ERROR_COUNT_)
        {
            gWindows->printCommWindow("WARNING - Max Sync count exceeded!\n");
            break;
        }
    }

    return synced;
}

//-----------------------------------------------------------------------
// This function reads n bytes from the file descriptor.
//-----------------------------------------------------------------------
bool Communicator::readNBytes_(const int fd, unsigned char *buf, const int n)
{
    int msgLength = -1, index = 0, remaining = 0;

    remaining = n;

    while (remaining > 0)
    {
        msgLength = read(fd, &buf[index], remaining);

        if (msgLength == 0)
        {
            disconnectPrivateConnection_(fd);
        }
        else if (msgLength <= 0)
        {
            gWindows->printfPlannerWindow(
                "WARNING: Communicator::readNBytes_(). Read error %d.\n",
                errno);

            disconnectPrivateConnection_(fd);
        }

        index += msgLength;
        remaining -= msgLength;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function updates the status of the thread in the info list.
//-----------------------------------------------------------------------
void Communicator::setConnectionThreadIsUp_(const int fd, const bool isUp)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    int i, listSize;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].fd == fd)
        {
            privateConnectionInfoList[i].connectionThreadIsUp = isUp;
            break;
        }
    }

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    shPrivateConnectionInfoList_ = privateConnectionInfoList;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);
}

//-----------------------------------------------------------------------
// This function updates the status of the thread in the info list.
//-----------------------------------------------------------------------
void Communicator::saveConnectionThread_(const int fd, const pthread_t connectionThread)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    int i, listSize;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].fd == fd)
        {
            privateConnectionInfoList[i].connectionThread = connectionThread;
            break;
        }
    }

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    shPrivateConnectionInfoList_ = privateConnectionInfoList;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);
}

//-----------------------------------------------------------------------
// This function gets the connectionThread stored in the info list.
//-----------------------------------------------------------------------
pthread_t Communicator::getConnectionThread_(const int fd)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    pthread_t connectionThread;
    int i, listSize;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].fd == fd)
        {
            connectionThread = privateConnectionInfoList[i].connectionThread;
            break;
        }
    }

    return connectionThread;
}

//-----------------------------------------------------------------------
// This function gets the connectionThread stored in the info list.
//-----------------------------------------------------------------------
CBRPlanner_ClientInfo_t Communicator::getClientInfo_(const int fd)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    CBRPlanner_ClientInfo_t clientInfo;
    int i, listSize;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].fd == fd)
        {
            clientInfo = privateConnectionInfoList[i].clientInfo;
            break;
        }
    }

    return clientInfo;
}

//-----------------------------------------------------------------------
// This function gets the file descriptor for the specified client type.
//-----------------------------------------------------------------------
int Communicator::getClientFd_(const int clientType)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    int i, listSize;
    int fd;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].clientInfo.type == clientType)
        {
            fd = privateConnectionInfoList[i].fd;
            break;
        }
    }

    return fd;
}

//-----------------------------------------------------------------------
// This function disconnects the private connection.
//-----------------------------------------------------------------------
void Communicator::disconnectPrivateConnection_(const int fd)
{
    CBRPlanner_ClientInfo_t clientInfo;
    pthread_t connectionThread;

    // First, report the disconnection to the user
    clientInfo = getClientInfo_(fd);
    gWindows->printfCommWindow(
        "Client %s (PID %d) disconnected.\n",
        //clientInfo.name, clientInfo.pid);
        clientInfo.name.c_str(), clientInfo.pid);

    // Update the info list.
    setConnectionThreadIsUp_(fd, false);

    // ENDO - gcc 3.4
    close(fd);

    // Finally, quit the thread.
    connectionThread = getConnectionThread_(fd);
    pthread_cancel(connectionThread);
    pthread_join(connectionThread, NULL);
}

//-----------------------------------------------------------------------
// This function disconnects all of the private connections.
//-----------------------------------------------------------------------
void Communicator::disconnectAllPrivateConnections_(void)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    int i, listSize;
    
    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        if (privateConnectionInfoList[i].connectionThreadIsUp)
        {
            pthread_cancel(privateConnectionInfoList[i].connectionThread);
            pthread_join(privateConnectionInfoList[i].connectionThread, NULL);

            privateConnectionInfoList[i].connectionThreadIsUp = false;
        }
    }

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    shPrivateConnectionInfoList_ = privateConnectionInfoList;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);
}

//-----------------------------------------------------------------------
// This function reads the client info being received
//-----------------------------------------------------------------------
CBRPlanner_ClientInfo_t Communicator::readClientInfo_(const int fd)
{
    CBRPlanner_ClientInfo_t clientInfo;
    int dataSize;
    char *data = NULL;

    // Read type.
    clientInfo.type = readInteger_(fd);

    // Read pid.
    clientInfo.pid = readInteger_(fd);

    if (clientInfo.type >= NUM_CBRPLANNER_CLIENT_TYPES)
    {
        gWindows->printfPlannerWindow(
            "Error: Corrupted Client Info: Type = %d, PID = %d.\a\n",
            clientInfo.type,
            clientInfo.pid);
        fflush(stdout);
        clientInfo.type = CBRPLANNER_CLIENT_UNKNOWN;
        exit(0);
    }

    // Read name.
    dataSize = readInteger_(fd);
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);
    data[dataSize] = CHAR_NULL_;
    clientInfo.name = data;
    delete [] data;
    data = NULL;

    gWindows->printfCommWindow(
        "Client is %s (PID %d).\n",
        clientInfo.name.c_str(),
        clientInfo.pid);
    fflush(stdout);

    return clientInfo;
}

//-----------------------------------------------------------------------
// This function updates the client list.
//-----------------------------------------------------------------------
void Communicator::updateClientInfo_(const int fd, const CBRPlanner_ClientInfo_t clientInfo)
{
    vector<PrivateConnectionInfo_t> privateConnectionInfoList;
    int i, listSize;

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    privateConnectionInfoList = shPrivateConnectionInfoList_;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);

    listSize = privateConnectionInfoList.size();

    for (i = 0; i < listSize; i++)
    {
        pthread_testcancel();

        if (privateConnectionInfoList[i].fd == fd)
        {
            privateConnectionInfoList[i].clientInfo = clientInfo;
            break;
        }
    }

    pthread_mutex_lock(&privateConnectionInfoListMutex_);
    shPrivateConnectionInfoList_ = privateConnectionInfoList;
    pthread_mutex_unlock(&privateConnectionInfoListMutex_);
}

//-----------------------------------------------------------------------
// This function reads the mission plan being received.
//-----------------------------------------------------------------------
CBRPlanner_ACDLMissionPlan_t Communicator::readACDLMissionPlan_(const int fd)
{
    CBRPlanner_ACDLMissionPlan_t acdlPlan;

    // Get the mission plan type.
    acdlPlan.type = readInteger_(fd);

    // Get the mission plan rating.
    acdlPlan.rating = readInteger_(fd);

    // Get the data size.
    acdlPlan.dataSize = readInteger_(fd);

    // Get the data itself.
    acdlPlan.data = new char[(acdlPlan.dataSize)+1];
    readNBytes_(fd, (unsigned char *)(acdlPlan.data), acdlPlan.dataSize);
    acdlPlan.data[acdlPlan.dataSize] = CHAR_NULL_;

    return acdlPlan;
}

//-----------------------------------------------------------------------
// This function reads the extra data being received.
//-----------------------------------------------------------------------
vector<CBRPlanner_ExtraData_t> Communicator::readExtraDataList_(const int fd)
{
    CBRPlanner_ExtraData_t extraData;
    vector<CBRPlanner_ExtraData_t> extraDataList;
    string value;
    char *buf = NULL;
    int i, j, listSize, keySize, valueSize;

    // Get the size of the list.
    listSize = readInteger_(fd);

    for (i = 0; i < listSize; i++)
    {
        // Read the size of the key
        keySize = readInteger_(fd);
        
        // Read the key.
        buf = new char[keySize+1];
        readNBytes_(fd, (unsigned char *)buf, keySize);
        buf[keySize] = CHAR_NULL_;
        extraData.key = buf;
        delete [] buf;
        buf = NULL;

        // Read the format of the data.
        extraData.format = readInteger_(fd);

        // Read the number of values.
        extraData.numDataEntries = readInteger_(fd);

        for (j = 0; j < extraData.numDataEntries; j++)
        {
            // Read the size of the value.
            valueSize = readInteger_(fd);

            // Read the value.
            buf = new char[valueSize+1];
            readNBytes_(fd, (unsigned char *)buf, valueSize);
            buf[valueSize] = CHAR_NULL_;
            value = buf;
            extraData.dataEntries.push_back(value);
            delete [] buf;
            buf = NULL;
        }

        extraDataList.push_back(extraData);
        extraData.dataEntries.clear();
    }

    return extraDataList;
}

//-----------------------------------------------------------------------
// This function writes the mission plan to the client.
//-----------------------------------------------------------------------
void Communicator::writeACDLMissionPlans_(
    const int fd,
    vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans)
{
    int i, numPlans;

    // First, send the number of the plans.
    numPlans = acdlPlans.size();
    writeInteger_(fd, numPlans);

    // Next, send the mission plan. The mission plan data is composed of the type flag,
    // data size, and the data itselt.
    for (i = 0; i < numPlans; i++)
    {
        writeACDLMissionPlan_(fd, acdlPlans[i]);
    }

    return;
}

//-----------------------------------------------------------------------
// This function writes the robot IDs to the client.
//-----------------------------------------------------------------------
void Communicator::writeRobotIDList_(
    const int fd,
    vector<CBRPlanner_RobotIDs_t> robotIDList)
{
    int i, j, listSize, numIDs, id;

    // First, send the size of the list.
    listSize = robotIDList.size();
    writeInteger_(fd, listSize);

    // Send the robot IDs
    for (i = 0; i < listSize; i++)
    {
        numIDs = robotIDList[i].robotIDs.size();
        writeInteger_(fd, numIDs);

        for (j = 0; j < numIDs; j++)
        {
            id = robotIDList[i].robotIDs[j];
            writeInteger_(fd, id);
        }
    }

    return;
}

//-----------------------------------------------------------------------
// This function writes the CNP status to the client.
//-----------------------------------------------------------------------
void Communicator::writeCNPStatus_(
    const int fd,
    CBRPlanner_CNPStatus_t cnpStatus)
{
    int status, dataSize;

    // First, send the status.
    status = cnpStatus.status;
    writeInteger_(fd, status);

    // Send the message size, then the message itself. 
    dataSize = cnpStatus.errorMsg.size();
    writeInteger_(fd, dataSize);

    if (dataSize > 0)
    {
        write(fd, cnpStatus.errorMsg.c_str(), dataSize);
    }

    return;
}

//-----------------------------------------------------------------------
// This function writes the mission plan to the client.
//-----------------------------------------------------------------------
void Communicator::writeACDLMissionPlan_(
    const int fd,
    CBRPlanner_ACDLMissionPlan_t acdlPlan)
{
    int i, j, dataIndexListSize;

    // Send the mission type.
    writeInteger_(fd, acdlPlan.type);

    // Send the rating.
    writeInteger_(fd, acdlPlan.rating);

    // Send the META-Data
    writeInteger_(fd, acdlPlan.extraDataList.size());
    
    for (i = 0; i < (int)(acdlPlan.extraDataList.size()); i++)
    {
        writeInteger_(fd,acdlPlan.extraDataList[i].key.size());
        write(
            fd,
            acdlPlan.extraDataList[i].key.c_str(), 
            acdlPlan.extraDataList[i].key.size());

        writeInteger_(fd,acdlPlan.extraDataList[i].format);
        writeInteger_(fd,acdlPlan.extraDataList[i].numDataEntries);

        for (j = 0; j < acdlPlan.extraDataList[i].numDataEntries; j++)
        {
            writeInteger_(
                fd,
                acdlPlan.extraDataList[i].dataEntries[j].size());
            write(
                fd,
                acdlPlan.extraDataList[i].dataEntries[j].c_str(), 
                acdlPlan.extraDataList[i].dataEntries[j].size());
        }
    }

    // Send the list of the data indexes.
    dataIndexListSize = acdlPlan.dataIndexList.size();
    writeInteger_(fd, dataIndexListSize);

    for (i = 0; i < dataIndexListSize; i++)
    {
        writeInteger_(fd, acdlPlan.dataIndexList[i]);
    }

    // Send the number of the robots.
    writeInteger_(fd, acdlPlan.numRobots);

    // Send the data.
    writeInteger_(fd, acdlPlan.dataSize);
    write(fd, acdlPlan.data, acdlPlan.dataSize);

    return;
}

//-----------------------------------------------------------------------
// This function writes a repair question to the client.
//-----------------------------------------------------------------------
void Communicator::writeRepairQuestion_(
    const int fd,
    RepairQuestion_t repairQuestion)
{
    int i, dataSize, numOptions;

    // Send the status.
    writeInteger_(fd, repairQuestion.status);

    // Send the type.
    writeInteger_(fd, repairQuestion.type);

    // Send the question.
    dataSize = repairQuestion.question.size();
    writeInteger_(fd, dataSize);
    write(fd, repairQuestion.question.c_str(), dataSize);

    // Send the number of the options.
    numOptions = repairQuestion.options.size();
    writeInteger_(fd, numOptions);

    // Send the options.
    for (i = 0; i < numOptions; i++)
    {
        dataSize = repairQuestion.options[i].size();
        writeInteger_(fd, dataSize);
        write(fd, repairQuestion.options[i].c_str(), dataSize);
    }

    return;
}

//-----------------------------------------------------------------------
// This function reads an integer number.
//-----------------------------------------------------------------------
int Communicator::readInteger_(const int fd)
{
    int number;

    readNBytes_(fd, (unsigned char *)&number, sizeof(int));

    return number;
}

//-----------------------------------------------------------------------
// This function reads a double number.
//-----------------------------------------------------------------------
double Communicator::readDouble_(const int fd)
{
    double number;

    readNBytes_(fd, (unsigned char *)&number, sizeof(double));

    return number;
}

//-----------------------------------------------------------------------
// This function reads a boolean value.
//-----------------------------------------------------------------------
bool Communicator::readBoolean_(const int fd)
{
    bool value;

    readNBytes_(fd, (unsigned char *)&value, sizeof(bool));

    return value;
}

//-----------------------------------------------------------------------
// This function reads a charactor.
//-----------------------------------------------------------------------
char Communicator::readChar_(const int fd)
{
    bool value;

    readNBytes_(fd, (unsigned char *)&value, 1);

    return value;
}

//-----------------------------------------------------------------------
// This function reads a string.
//-----------------------------------------------------------------------
string Communicator::readString_(const int fd)
{
    string value;
    int valueSize;
    char *buf = NULL;

    valueSize = readInteger_(fd);
    buf = new char[valueSize + 1];
    readNBytes_(fd, (unsigned char *)buf, valueSize);
    buf[valueSize] = CHAR_NULL_;
    value = buf;
    delete [] buf;
    buf = NULL;

    return value;
}

//-----------------------------------------------------------------------
// This function writes an integer number.
//-----------------------------------------------------------------------
void Communicator::writeInteger_(const int fd, int value)
{
    write(fd, &value, sizeof(int));
}

//-----------------------------------------------------------------------
// This function writes a boolean value.
//-----------------------------------------------------------------------
void Communicator::writeBoolean_(const int fd, bool value)
{
    write(fd, &value, sizeof(bool));
}

//-----------------------------------------------------------------------
// This function writes a char value.
//-----------------------------------------------------------------------
void Communicator::writeChar_(const int fd, char value)
{
    write(fd, &value, sizeof(char));
}

//-----------------------------------------------------------------------
// This function writes a string value.
//-----------------------------------------------------------------------
void Communicator::writeString_(const int fd, string value)
{
    int valueSize;

    valueSize = value.size();
    writeInteger_(fd, valueSize);
    write(fd, value.c_str(),valueSize);
}

//-----------------------------------------------------------------------
// This function reads the features being received.
//-----------------------------------------------------------------------
MemoryManager::Features_t Communicator::readFeatures_(const int fd)
{
    MemoryManager::Feature_t feature;
    MemoryManager::Features_t features;
    int i;
    int numFeatures, nameSize, valueSize;
    int formatType;
    char *name = NULL, *value = NULL;
    const bool DEBUG = true;

    // Get the number of features.
    numFeatures = readInteger_(fd);

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("RECEIVED FEATURES:\n");
    }

    for (i = 0; i < numFeatures; i++)
    {
        // Get the length of the name.
        nameSize = readInteger_(fd); 

        // Get the name.
        name = new char[nameSize+1];
        readNBytes_(fd, (unsigned char *)name, nameSize);
        name[nameSize] = CHAR_NULL_;
        feature.name = name;
        delete [] name;
        name = NULL;
	
        // Get the length of the value.
        valueSize = readInteger_(fd);

        // Get the value.
        value = new char[valueSize+1];
        readNBytes_(fd, (unsigned char *)value, valueSize);
        value[valueSize] = CHAR_NULL_;
        feature.value = value;
        delete [] value;
        value = NULL;

        // Get the nonIndex flag.
        feature.nonIndex = readBoolean_(fd);

        // Get the weight.
        feature.weight = readDouble_(fd);

        // Get the format type
        formatType = readInteger_(fd);

        switch (formatType) {

        case CBRPLANNER_FORMAT_STRING:
            feature.formatType = MemoryManager::FORMAT_STRING;
            break;

        case CBRPLANNER_FORMAT_INT:
            feature.formatType = MemoryManager::FORMAT_INT;
            break;

        case CBRPLANNER_FORMAT_DOUBLE:
            feature.formatType = MemoryManager::FORMAT_DOUBLE;
            break;

        case CBRPLANNER_FORMAT_BOOLEAN:
            feature.formatType = MemoryManager::FORMAT_BOOLEAN;
            break;

        case CBRPLANNER_FORMAT_NA:
        default:
            feature.formatType = MemoryManager::FORMAT_NA;
            break;
        }

        features.push_back(feature);

        if (DEBUG)
        {
            gDebugger->printfDebug("%s  ", feature.name.c_str());
            gDebugger->printfDebug("%s  ", feature.value.c_str());
            gDebugger->printfDebug("%d  ", feature.nonIndex);
            gDebugger->printfDebug("%.2f\n", feature.weight);
        }
    }

    if (DEBUG)
    {
        gDebugger->printfLine();
    }

    return features;
}

//-----------------------------------------------------------------------
// This function reads the constraints being received.
//-----------------------------------------------------------------------
vector<MemoryManager::Constraints_t> Communicator::readConstraints_(const int fd)
{
    MemoryManager::Constraints_t genericConstraints;
    MemoryManager::Constraint_t constraint;
    vector<MemoryManager::Constraints_t> genericConstraintsList;
    int i, j;
    int listSize, numConstraints, nameSize, valueSize;
    int formatType;
    char *name = NULL, *value = NULL;
    const bool DEBUG = true;

    // Get the number of features.
    listSize = readInteger_(fd);

    if (DEBUG)
    {
        gDebugger->printfLine();
        gDebugger->printfDebug("RECEIVED CONSTRAINTS:\n");
    }

    for (i = 0; i < listSize; i++)
    {
        genericConstraints.constraints.clear();

        // Get the task/robot ID.
        genericConstraints.id = readInteger_(fd); 

        // Get the length of the name.
        nameSize = readInteger_(fd); 

        // Get the name.
        name = new char[nameSize+1];
        readNBytes_(fd, (unsigned char *)name, nameSize);
        name[nameSize] = CHAR_NULL_;
        genericConstraints.name = name;
        delete [] name;
        name = NULL;
	
        // Get the number of constraints.
        numConstraints = readInteger_(fd); 

        if (DEBUG)
        {
            gDebugger->printfLine();
            gDebugger->printfDebug("Name: %s\n", genericConstraints.name.c_str());
            gDebugger->printfDebug("ID: %d\n", genericConstraints.id);
        }

        for (j = 0; j < numConstraints; j++)
        {
            // Get the length of the name.
            nameSize = readInteger_(fd); 

            // Get the name.
            name = new char[nameSize+1];
            readNBytes_(fd, (unsigned char *)name, nameSize);
            name[nameSize] = CHAR_NULL_;
            constraint.name = name;
            delete [] name;
            name = NULL;
	
            // Get the length of the value.
            valueSize = readInteger_(fd);

            // Get the value.
            value = new char[valueSize+1];
            readNBytes_(fd, (unsigned char *)value, valueSize);
            value[valueSize] = CHAR_NULL_;
            constraint.value = value;
            delete [] value;
            value = NULL;

            // Get the ID
            constraint.id = readInteger_(fd);

            // Get the format type
            formatType = readInteger_(fd);

            switch (formatType) {

            case CBRPLANNER_FORMAT_STRING:
                constraint.formatType = MemoryManager::FORMAT_STRING;
                break;

            case CBRPLANNER_FORMAT_INT:
                constraint.formatType = MemoryManager::FORMAT_INT;
                break;

            case CBRPLANNER_FORMAT_DOUBLE:
                constraint.formatType = MemoryManager::FORMAT_DOUBLE;
                break;

            case CBRPLANNER_FORMAT_BOOLEAN:
                constraint.formatType = MemoryManager::FORMAT_BOOLEAN;
                break;

            case CBRPLANNER_FORMAT_NA:
            default:
                constraint.formatType = MemoryManager::FORMAT_NA;
                break;
            }

            genericConstraints.constraints.push_back(constraint);

            if (DEBUG)
            {
                gDebugger->printfDebug("%s  ", constraint.name.c_str());
                gDebugger->printfDebug("%s  ", constraint.value.c_str());
                gDebugger->printfDebug("%d  ", constraint.id);
                gDebugger->printfDebug("%d\n", constraint.formatType);
            }
        }

        genericConstraintsList.push_back(genericConstraints);
    }

    if (DEBUG)
    {
        gDebugger->printfLine();
    }

    return genericConstraintsList;
}

//-----------------------------------------------------------------------
// This function reads the FSA list info being received.
//-----------------------------------------------------------------------
CBRPlanner_FSANames_t Communicator::readFSANames_(const int fd)
{
    CBRPlanner_FSANames_t fsaNames;
    string taskName, triggerName;
    bool received = false;
    int i, numTasks, numTriggers, nameSize;
    char *name = NULL;

    // Get the number of tasks.
    received = readNBytes_(fd, (unsigned char *)&(numTasks), sizeof(int));

    for (i = 0; i < numTasks; i++)
    {
        // Get the length of the task name;
        received = readNBytes_(fd, (unsigned char *)&(nameSize), sizeof(int));

        // Get the task name;
        name = new char[nameSize+1];
        received = readNBytes_(fd, (unsigned char *)name, nameSize);
        name[nameSize] = CHAR_NULL_;
        taskName = name;
        fsaNames.taskNames.push_back(taskName);
        delete [] name;
        name = NULL;
    }

    // Get the number of triggers.
    received = readNBytes_(fd, (unsigned char *)&(numTriggers), sizeof(int));

    for (i = 0; i < numTriggers; i++)
    {
        // Get the length of the trigger name;
        received = readNBytes_(fd, (unsigned char *)&(nameSize), sizeof(int));

        // Get the trigger name;
        name = new char[nameSize+1];
        received = readNBytes_(fd, (unsigned char *)name, nameSize);
        name[nameSize] = CHAR_NULL_;
        triggerName = name;
        fsaNames.triggerNames.push_back(triggerName);
        delete [] name;
        name = NULL;
    }

    return fsaNames;
}

//-----------------------------------------------------------------------
// This function reads the list of data indexes.
//-----------------------------------------------------------------------
vector<int> Communicator::readDataIndexList_(const int fd)
{
    vector<int> dataIndexList;
    int i, dataIndexListSize;
    int dataIndex;
    bool received = false;
    
    // Get the size of the list.
    received = readNBytes_(fd, (unsigned char *)&(dataIndexListSize), sizeof(int));

    for (i = 0; i < dataIndexListSize; i++)
    {
        received = readNBytes_(fd, (unsigned char *)&(dataIndex), sizeof(int));
        dataIndexList.push_back(dataIndex);
    }

    return dataIndexList;
}

//-----------------------------------------------------------------------
// This function sends the list of data indexes.
//-----------------------------------------------------------------------
void Communicator::writeDataIndexList_(const int fd, vector<int> dataIndexList)
{
    int i, dataIndexListSize;
    
    dataIndexListSize = dataIndexList.size();
    write(fd, &dataIndexListSize, sizeof(int));

    for (i = 0; i < dataIndexListSize; i++)
    {
        write(fd, &(dataIndexList[i]), sizeof(int));
    }
}

//-----------------------------------------------------------------------
// This function sends the overlay info to the client.
//-----------------------------------------------------------------------
void Communicator::returnQueuedData_(int fd, QueuedData_t queuedData)
{
    writeChar_(fd, queuedData.cmd);
    writeInteger_(fd, queuedData.dataSize);

    if (queuedData.dataSize > 0)
    {
        write(fd, queuedData.data, queuedData.dataSize);
        delete [] queuedData.data;
        queuedData.data = NULL;
    }
}

//-----------------------------------------------------------------------
// This function reads the logfile info being received.
//-----------------------------------------------------------------------
CBRPlanner_LogfileInfo_t Communicator::readLogfileInfo_(const int fd)
{
    CBRPlanner_LogfileInfo_t logfileInfo;
    int dataSize;
    char *data = NULL;

    readNBytes_(fd, (unsigned char *)&dataSize, sizeof(int));
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);

    data[dataSize] = CHAR_NULL_;
    logfileInfo.name = data;

    delete [] data;
    data = NULL;

    return logfileInfo;
}

//-----------------------------------------------------------------------
// This function reads the overlay info being received.
//-----------------------------------------------------------------------
CBRPlanner_OverlayInfo_t Communicator::readOverlayInfo_(const int fd)
{
    CBRPlanner_OverlayInfo_t overlayInfo;
    int dataSize;
    char *data = NULL;

    readNBytes_(fd, (unsigned char *)&dataSize, sizeof(int));
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);

    data[dataSize] = CHAR_NULL_;
    overlayInfo.name = data;

    delete [] data;
    data = NULL;

    return overlayInfo;
}

//-----------------------------------------------------------------------
// This function sends the logfile info to CfgEdit.
//-----------------------------------------------------------------------
void Communicator::queueLogfileInfo_(
    CBRPlanner_LogfileInfo_t logfileInfo,
    int clientType,
    char command)
{
    QueuedData_t queuedData;

    queuedData.clientType = clientType;
    queuedData.cmd = command;
    queuedData.dataSize = logfileInfo.name.size();
    queuedData.data = new char[(queuedData.dataSize)+1];
    sprintf(queuedData.data, logfileInfo.name.c_str());
    queuedData.data[queuedData.dataSize] = CHAR_NULL_;

    pthread_mutex_lock(&queuedDataMutex_);
    shQueuedDataList_.push_back(queuedData);
    pthread_mutex_unlock(&queuedDataMutex_);
}

//-----------------------------------------------------------------------
// This function queues up a simple command to CfgEdit.
//-----------------------------------------------------------------------
void Communicator::queueSimpleCommand_(
    int clientType,
    char command)
{
    QueuedData_t queuedData;

    queuedData.clientType = clientType;
    queuedData.cmd = command;
    queuedData.dataSize = 0;
    queuedData.data = NULL;

    pthread_mutex_lock(&queuedDataMutex_);
    shQueuedDataList_.push_back(queuedData);
    pthread_mutex_unlock(&queuedDataMutex_);
}

//-----------------------------------------------------------------------
// This function sends the state info to CfgEdit.
//-----------------------------------------------------------------------
void Communicator::queueStateInfo_(
    vector<CBRPlanner_StateInfo_t> stateInfoList,
    int clientType,
    char command)
{
    QueuedData_t queuedData;
    int i, numStateInfo, nameSize;
    char *dataStart = NULL;

    queuedData.clientType = clientType;
    queuedData.cmd = command;

    numStateInfo = stateInfoList.size();

    // Compute the space for the FSA names. First, start with the integer
    // space for indicating the size of the list.
    queuedData.dataSize = sizeof(int);

    for (i = 0; i < numStateInfo; i++)
    {
        // Size of the FSA name.
        queuedData.dataSize += sizeof(int);

        // FSA name itself.
        queuedData.dataSize += stateInfoList[i].fsaName.size();

        // Size of the state name.
        queuedData.dataSize += sizeof(int);

        // State name itself.
        queuedData.dataSize += stateInfoList[i].stateName.size();
    }

    // Allocate memory to the data.
    queuedData.data = new char[queuedData.dataSize];
    dataStart = queuedData.data;

    // Copy the size of the list.
    memcpy(queuedData.data, (char *)&numStateInfo, sizeof(int));
    queuedData.data += sizeof(int);

    for (i = 0; i < numStateInfo; i++)
    {
        // Copy the FSA names (after its size).
        nameSize = stateInfoList[i].fsaName.size();
        memcpy(queuedData.data, (char *)&nameSize, sizeof(int));
        queuedData.data += sizeof(int);
        memcpy(queuedData.data, stateInfoList[i].fsaName.c_str(), nameSize);
        queuedData.data += nameSize;

        // Copy the state names (after its size).
        nameSize = stateInfoList[i].stateName.size();
        memcpy(queuedData.data, (char *)&nameSize, sizeof(int));
        queuedData.data += sizeof(int);
        memcpy(queuedData.data, stateInfoList[i].stateName.c_str(), nameSize);
        queuedData.data += nameSize;
    }

    // Set the data pointer to be its start.
    queuedData.data = dataStart;

    pthread_mutex_lock(&queuedDataMutex_);
    shQueuedDataList_.push_back(queuedData);
    pthread_mutex_unlock(&queuedDataMutex_);
}

//-----------------------------------------------------------------------
// This function sends the overlay info to CfgEdit.
//-----------------------------------------------------------------------
void Communicator::queueOverlayInfo_(
    CBRPlanner_OverlayInfo_t overlayInfo,
    int clientType,
    char command)
{
    QueuedData_t queuedData;

    queuedData.clientType = clientType;
    queuedData.cmd = command;
    queuedData.dataSize = overlayInfo.name.size();
    queuedData.data = new char[(queuedData.dataSize)+1];
    sprintf(queuedData.data, overlayInfo.name.c_str());
    queuedData.data[queuedData.dataSize] = CHAR_NULL_;

    pthread_mutex_lock(&queuedDataMutex_);
    shQueuedDataList_.push_back(queuedData);
    pthread_mutex_unlock(&queuedDataMutex_);
}

//-----------------------------------------------------------------------
// This function requests the specified info from CfgEdit.
//-----------------------------------------------------------------------
void Communicator::queueRequestInfo_(int clientType, char command)
{
    QueuedData_t queuedData;

    queuedData.clientType = clientType;
    queuedData.cmd = command;
    queuedData.dataSize = DUMMY_DATA_SIZE_;
    queuedData.data = new char[DUMMY_DATA_SIZE_];

    pthread_mutex_lock(&queuedDataMutex_);
    shQueuedDataList_.push_back(queuedData);
    pthread_mutex_unlock(&queuedDataMutex_);
}

//-----------------------------------------------------------------------
// This function sends the queued data
//-----------------------------------------------------------------------
void Communicator::sendOffQueuedData_(int fd)
{
    CBRPlanner_ClientInfo_t clientInfo;
    vector<QueuedData_t> thisQueuedDataList, nextQueuedDataList;
    int i, numQueuedData;

    clientInfo = getClientInfo_(fd);
    numQueuedData = 0;

    pthread_mutex_lock(&queuedDataMutex_);

    for (i = 0; i < (int)(shQueuedDataList_.size()); i++)
    {
        if (shQueuedDataList_[i].clientType == clientInfo.type)
        {
            thisQueuedDataList.push_back(shQueuedDataList_[i]);
        }
        else
        {
            nextQueuedDataList.push_back(shQueuedDataList_[i]);
        }
    }

    shQueuedDataList_ = nextQueuedDataList;
        
    pthread_mutex_unlock(&queuedDataMutex_);

    pthread_testcancel();

    numQueuedData = thisQueuedDataList.size();
    writeInteger_(fd, numQueuedData);

    for (i = 0; i < numQueuedData; i++)
    {
        returnQueuedData_(fd, thisQueuedDataList[i]);
    }
}

//-----------------------------------------------------------------------
// This function reads the (multiple) state info being received.
//-----------------------------------------------------------------------
vector<CBRPlanner_StateInfo_t> Communicator::readStateInfoList_(const int fd)
{
    CBRPlanner_StateInfo_t stateInfo;
    vector<CBRPlanner_StateInfo_t> stateInfoList;
    int i, numStateInfo;

    numStateInfo = readInteger_(fd);

    for (i = 0; i < numStateInfo; i++)
    {
        stateInfo = readStateInfo_(fd);
        stateInfoList.push_back(stateInfo);
    }

    return stateInfoList;
}

//-----------------------------------------------------------------------
// This function reads the single state info being received.
//-----------------------------------------------------------------------
CBRPlanner_StateInfo_t Communicator::readStateInfo_(const int fd)
{
    CBRPlanner_StateInfo_t stateInfo;
    int dataSize;
    char *data = NULL;

    // Read the FSA name.
    dataSize = readInteger_(fd);
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);
    data[dataSize] = CHAR_NULL_;
    stateInfo.fsaName = data;
    delete [] data;
    data = NULL;

    // Read the state name.
    dataSize = readInteger_(fd);
    data = new char[dataSize+1];
    readNBytes_(fd, (unsigned char *)data, dataSize);
    data[dataSize] = CHAR_NULL_;
    stateInfo.stateName = data;
    delete [] data;
    data = NULL;

    return stateInfo;
}

/**********************************************************************
 * $Log: communicator.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.8  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.7  2007/09/06 19:01:18  endo
 * Adding strerror() to the socket errors.
 *
 * Revision 1.6  2007/08/10 15:15:39  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.5  2007/08/06 22:08:47  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.4  2007/08/04 23:53:59  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.3  2007/02/11 22:40:38  nadeem
 * Added the code to send the META-DATA from the cbrplanner to cfgedit_cbrclient
 *
 * Revision 1.2  2006/09/26 18:30:59  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.10  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.9  2006/03/05 23:20:50  endo
 * readNBytes_()
 *
 * Revision 1.8  2006/03/05 20:33:41  endo
 * CPU bug fixed.
 *
 * Revision 1.7  2006/02/19 17:51:59  endo
 * Experiment related modifications
 *
 * Revision 1.6  2006/01/30 02:50:34  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.5  2005/08/12 21:48:30  endo
 * More improvements for August demo.
 *
 * Revision 1.4  2005/07/31 03:41:40  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.3  2005/07/16 08:49:21  endo
 * CBR-CNP integration
 *
 * Revision 1.2  2005/02/07 19:53:43  endo
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
