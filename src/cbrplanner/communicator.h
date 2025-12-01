/**********************************************************************
 **                                                                  **
 **                           communicator.h                         **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia 30332-0415                                     **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

/* $Id: communicator.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <string>
#include <vector>
#include <pthread.h>

#include "cbrplanner_protocol.h"
#include "cbrplanner_domain.h"
#include "domain_manager.h"
#include "memory_manager.h"
#include "repair_plan.h"

using std::string;
using std::vector;

class Communicator {
    
    typedef struct PrivateConnectionInfo_t
    {
        int fd;
        CBRPlanner_ClientInfo_t clientInfo;
        pthread_t connectionThread;
        bool connectionThreadIsUp;
    };

    typedef struct PrivateConnectionThreadInput_t
    {
        int fd;
        Communicator *communicatorInstance;
    };


    typedef struct QueuedData_t
    {
        int clientType;
        char cmd;
        int dataSize;
        char *data;
    };


protected:
    Communicator **self_;

    vector<PrivateConnectionInfo_t> shPrivateConnectionInfoList_;
    vector<QueuedData_t> shQueuedDataList_;
    string socketName_;
    pthread_t daemonThread_;
    int socketFd_;
    bool daemonThreadIsUp_;

    static pthread_mutex_t privateConnectionInfoListMutex_;
    static pthread_mutex_t queuedDataMutex_;

    static const char CHAR_NULL_;
    static const int SOCKET_BACKLOG_SIZE_;
    static const int SELECT_WAIT_SEC_;
    static const int MAX_SYNC_ERROR_COUNT_;
    static const int DUMMY_DATA_SIZE_;

    CBRPlanner_ClientInfo_t readClientInfo_(const int fd);
    CBRPlanner_ClientInfo_t getClientInfo_(const int fd);
    CBRPlanner_LogfileInfo_t readLogfileInfo_(const int fd);
    CBRPlanner_OverlayInfo_t readOverlayInfo_(const int fd);
    CBRPlanner_ACDLMissionPlan_t readACDLMissionPlan_(const int fd);
    CBRPlanner_FSANames_t readFSANames_(const int fd);
    CBRPlanner_StateInfo_t readStateInfo_(const int fd);
    MemoryManager::Features_t readFeatures_(const int fd);
    pthread_t getConnectionThread_(const int fd);
    vector<CBRPlanner_ExtraData_t> readExtraDataList_(const int fd);
    vector<CBRPlanner_StateInfo_t> readStateInfoList_(const int fd);
    vector<MemoryManager::Constraints_t> readConstraints_(const int fd);
    vector<int> readDataIndexList_(const int fd);
    char readChar_(const int fd);
    string readString_(const int fd);
    double readDouble_(const int fd);
    int waitForNewConnection_(int count);
    int readInteger_(const int fd);
    int getClientFd_(const int clientType);
    bool readBoolean_(const int fd);
    bool readAndProcessPrivateConnectionData_(const int fd);
    bool readNBytes_(const int fd, unsigned char *buf, const int n);
    bool syncToClient_(const int fd);
    void setupSocket_(void);
    void daemonMainLoop_(void);
    void startPrivateConnection_(const int fd);
    void privateConnectionMainLoop_(const int fd);
    void setConnectionThreadIsUp_(const int fd, const bool isUp);
    void saveConnectionThread_(const int fd, const pthread_t connectionThread);
    void disconnectPrivateConnection_(const int fd);
    void disconnectAllPrivateConnections_(void);
    void updateClientInfo_(const int fd, const CBRPlanner_ClientInfo_t clientInfo);
    void writeACDLMissionPlans_(
        const int fd,
        vector<CBRPlanner_ACDLMissionPlan_t> acdlPlans);
    void writeACDLMissionPlan_(
        const int fd,
        CBRPlanner_ACDLMissionPlan_t acdlPlan);
    void writeRobotIDList_(
        const int fd,
        vector<CBRPlanner_RobotIDs_t> robotIDList);
    void writeCNPStatus_(
        const int fd,
        CBRPlanner_CNPStatus_t cnpStatus);
    void writeInteger_(const int fd, int value);
    void writeBoolean_(const int fd, bool value);
    void writeChar_(const int fd, char value);
    void writeString_(const int fd, string value);
    void writeDataIndexList_(const int fd, vector<int> dataIndexList);
    void returnQueuedData_(int fd, QueuedData_t queuedData);
    void queueLogfileInfo_(
        CBRPlanner_LogfileInfo_t logfileInfo,
        int clientType,
        char command);
    void queueOverlayInfo_(
        CBRPlanner_OverlayInfo_t overlayInfo,
        int clientType,
        char command);
    void queueRequestInfo_(int clientType, char command);
    void sendOffQueuedData_(int fd);
    void queueStateInfo_(
        vector<CBRPlanner_StateInfo_t> stateInfoList,
        int clientType,
        char command);
    void writeRepairQuestion_(
        const int fd,
        RepairQuestion_t repairQuestion);
    void queueSimpleCommand_(
        int clientType,
        char command);

    static void *startDaemonThread_(void *communicatorInstance);
    static void *startPrivateConnectionThread_(void *input);

public:
    Communicator(void);
    Communicator(Communicator **self);
    ~Communicator(void);
    void startDaemon(void);
    void setSocketName(string name);
};

inline void Communicator::setSocketName(string name)
{
    socketName_ = name;
}

extern Communicator *gCommunicator;

#endif
/**********************************************************************
 * $Log: communicator.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
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
 * Revision 1.6  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
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
