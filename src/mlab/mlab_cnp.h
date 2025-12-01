/**********************************************************************
 **                                                                  **
 **                             mlab_cnp.h                           **
 **                                                                  **
 **  Written by:       Yoichiro Endo                                 **
 **  Based on code by: Patrick Ulam                                  **
 **                                                                  **
 **  This class deals with CNP related functions within mlab.        **
 **                                                                  **
 **  Copyright 2005 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MLAB_CNP_H
#define MLAB_CNP_H

/* $Id: mlab_cnp.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <string>
#include <list>
#include <vector>
#include <map>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include "cnp_server_manager.h"

using std::string;
using std::list;
using std::vector;

typedef struct MlabCNP_RequestMess_t
{
    int TaskID;
    int Iteration;
    string TaskDesc;
    string Restriction;
};

typedef enum MlabCNP_TrackType_t {
    CNP_TRACK_NONE,
    CNP_TRACK_RED,
    CNP_TRACK_YELLOW
};

class MlabCNP {

    typedef struct InitCNPRobotConstraintsEntry_t {
        int robotID;
        vector<CNP_RobotConstraint> initCNPRobotConstraints;
    };

protected:
    XtAppContext xtApp_;
    CNP_Server_Manager *cnpServerManager_;
    list<CNP_Award> cnpAwardList_;
    vector<InitCNPRobotConstraintsEntry_t> initCNPRobotConstraintsList_;
    list<CNP_TaskConstraint>  currentCNPTaskConstraints_;
    int cnpTaskID_;

    static const string EMPTY_STRING_;
    static const string STRING_CNP_TRACK_RED_;
    static const string STRING_CNP_TRACK_YELLOW_;
    static const string STRING_CNP_TRACK_DEFAULT_REQUIREMENTS_;
    static const int CHECK_EXPIRE_LOOP_TIMER_MSEC_;

    void startCheckExpireLoop_(void);

    static XtTimerCallbackProc cbCheckExpire_(XtPointer clientData);

public:
    MlabCNP(void);
    MlabCNP(XtAppContext xtApp);
    ~MlabCNP(void);
    vector<CNP_RobotConstraint> getInitCNPRobotConstraints(int robotID);
    vector<CNP_TaskConstraint> getCurrentCNPTaskConstraints(void);
    list<CNP_TaskConstraint> getCurrentCNPTaskConstraintsList(void);
    bool checkForTask(int robotID, MlabCNP_RequestMess_t *mess);
    bool getAward(CNP_Award *currentAward);
    void cancelTask(CNP_Cancel canc);
    void completeTask(int taskid);
    void bidTask(CNP_Offer &offer);
    void injectTrackTask(int trackType);
    void saveInitCNPRobotConstraints(
        int robotID,
        vector<CNP_RobotConstraint> initCNPRobotConstraints);
    void saveCurrentCNPTaskConstraints(
        list<CNP_TaskConstraint> cnpTaskConstraints);
    void ReinjectTask(int tid);
};


extern MlabCNP *gMlabCNP;

#endif
/**********************************************************************
 * $Log: mlab_cnp.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/01 19:49:12  pulam
 * *** empty log message ***
 *
 * Revision 1.3  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.2  2006/01/12 20:32:44  pulam
 * cnp cleanup
 *
 * Revision 1.1  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 **********************************************************************/
