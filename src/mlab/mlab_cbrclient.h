/**********************************************************************
 **                                                                  **
 **                           mlab_cbrclient.h                       **
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
#ifndef MLAB_CBRCLIENT_H
#define MLAB_CBRCLIENT_H

/* $Id: mlab_cbrclient.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <string>
#include <vector>
#include <pthread.h>

#include "cbrclient.h"
#include "mission_design_types.h"
#include "cbrplanner_protocol.h"

using std::string;
using std::vector;

class MlabCBRClient : public CBRClient {

protected:
    CBRPlanner_OverlayInfo_t shOverlayInfo_;
    vector<CBRPlanner_LogfileInfo_t> shLogfileInfoList_;
    bool shOverlayInfoReady_;
    bool shLogfileInfoReady_;

    static pthread_mutex_t overlayInfoMutex_;
    static pthread_mutex_t logfileInfoMutex_;

    static const struct timespec RETRIEVE_OVERLAY_INFO_SLEEPTIME_NSEC_;
    static const struct timespec RETRIEVE_LOGFILE_INFO_SLEEPTIME_NSEC_;
    static const int RETRIEVE_OVERLAY_INFO_SLEEPTIME_USEC_;
    static const int RETRIEVE_LOGFILE_INFO_SLEEPTIME_USEC_;

    void retrieveOverlayInfoFromServer_(void);
    void retrieveLogfileInfoFromServer_(void);
    void readAndProcessServerData_(void);
    void sendStateInfo_(vector<CBRPlanner_StateInfo_t> stateInfoList);
    void retrieveLogfileInfoAcknowledgment_(void);

public:
    MlabCBRClient(void);
    MlabCBRClient(const symbol_table<rc_chain> &rcTable, int type);
    ~MlabCBRClient(void);
    CBRPlanner_OverlayInfo_t retrieveOverlayInfo(void);
    vector<CBRPlanner_LogfileInfo_t> retrieveLogfileInfoList(void);
    MMDRepairQuestion_t retrieveRepairQuestion(int lastSelectedOption);
    MMDRepairQuestion_t retrieveRepairFollowUpQuestion(void);
    void saveLogfileInfo(CBRPlanner_LogfileInfo_t logfileInfo);
    void reportCurrentState(vector<CBRPlanner_StateInfo_t> cbrStateInfoList);
    void replyRepairFollowUpQuestionToggle(int selectedOption);
    void replyRepairFollowUpQuestionAddMapPoint(double x, double y);
    void replyRepairFollowUpQuestionMoveMapPoint(
        double oldX,
        double oldY,
        double newX,
        double newY,
        double closeRange);
    void replyRepairFollowUpQuestionDeleteMapPoint(
        double x,
        double y,
        double closeRange);
    void replyRepairFollowUpQuestionStateName(
        vector<CBRPlanner_StateInfo_t> stateInfoList);
};

extern MlabCBRClient *gMlabCBRClient;

#endif
/**********************************************************************
 * $Log: mlab_cbrclient.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2005/02/07 23:12:36  endo
 * Mods from usability-2004
 *
 **********************************************************************/
