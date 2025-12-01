/**********************************************************************
 **                                                                  **
 **                              cbrclient.h                         **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for cbrclient.cc.                         **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CBRCLIENT_H
#define CBRCLIENT_H

/* $Id: cbrclient.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <string>
#include <vector>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Xthreads.h>

#include "cbrplanner_protocol.h"
#include "symbol_table.hpp"
#include "load_rc.h"

using std::string;
using std::vector;

class CBRClient {

    typedef struct ReaderThreadInput_t {
        int fd;
        CBRClient *cbrClientInstance;
    };

protected:
    CBRPlanner_ClientInfo_t clientInfo_;
    symbol_table<rc_chain> rcTable_;
    pthread_t readerThread_;
    int shSocketFd_;
    bool readerThreadIsUp_;

    static pthread_mutex_t socketMutex_;

    static const struct timespec READER_SLEEP_TIME_NSEC_;
    static const string RCTABLE_SERVERSOCKET_STRING_;
    static const string EMPTY_STRING_;
    static const char CHAR_NULL_;

    string readString_(const int fd);
    char readChar_(const int fd);
    int pollServer_(void);
    int readInteger_(const int fd);
    void readerMainLoop_(void);
    void sendClientInfo_(CBRPlanner_ClientInfo_t clientInfo);
    void sendCommandToServer_(const unsigned char cmd);
    void sendDataToServer_(void *data, int dataSize);
    void sendStateInfoList_(vector<CBRPlanner_StateInfo_t> stateInfoList);
    void sendStateInfo_(CBRPlanner_StateInfo_t stateInfo);
    void writeBoolean_(const int fd, bool value);
    void writeChar_(const int fd, char value);
    void writeDouble_(const int fd, double value);
    void writeInteger_(const int fd, int value);
    void writeString_(const int fd, string value);
    bool connectToServer_(void);
    bool readBoolean_(const int fd);
    bool readNBytes_(const int fd, unsigned char *buf, const int n);

    static void *startReaderThread_(void *input);

    virtual void readAndProcessServerData_(void);

public:
    CBRClient(void);
    CBRClient(const symbol_table<rc_chain> &rcTable);
    virtual ~CBRClient(void);
    bool initialize(void);
    void lockSocketMutex(void);
    void unlockSocketMutex(void);
};

inline void CBRClient::readAndProcessServerData_(void) {}

inline void CBRClient::lockSocketMutex(void)
{
    pthread_mutex_lock(&socketMutex_);
}

inline void CBRClient::unlockSocketMutex(void)
{
    pthread_mutex_unlock(&socketMutex_);
}

#endif
/**********************************************************************
 * $Log: cbrclient.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/09/28 15:56:56  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 00:54:55  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2006/01/30 02:50:35  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1  2005/02/07 19:53:46  endo
 * Mods from usability-2004
 *
 * Revision 1.3  2003/04/06 15:21:25  endo
 * gcc 3.1.1
 *
 * Revision 1.2  2003/04/06 08:48:14  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/13 01:08:36  endo
 * Initial revision
 *
 **********************************************************************/
