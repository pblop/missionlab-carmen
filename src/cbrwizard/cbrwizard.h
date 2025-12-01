/**********************************************************************
 **                                                                  **
 **                             cbrwizard.h                          **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This is a header file for cbrwizard.cc.                         **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef CBRWIZARD_H
#define CBRWIZARD_H

/* $Id: cbrwizard.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <vector>
#include <string>

#include "symbol.hpp"
#include "load_rc.h"
#include "mission_expert_types.h"
#include "cbrclient.h"

using std::vector;
using std::string;

class CBRWizard {

    typedef struct GeneralYesNoWindowCallbackData_t {
        bool result;
        CBRWizard *cbrWizardInstance;
    };

    typedef struct GeneralToggleWindowCallbackData_t {
        int result;
        CBRWizard *cbrWizardInstance;
        Widget generalToggleWindow;
    };

protected:
    Display *display_;
    XtAppContext appContext_;
    Pixel parentWidgetFg_;
    Pixel parentWidgetBg_;
    Widget parentWidget_;
    Widget generalMessageWindow_;
    symbol_table<rc_chain> rcTable_;
    CBRClient *cbrclient_;
    vector<MExpFeature_t> defaultGlobalFeatures_;
    vector<MExpFeature_t> defaultLocalFeatures_;
    vector<MExpMissionTask_t> defaultTaskList_;
    vector<MExpRobotConstraints_t> robotConstraintsList_;
    string featureFileName_;
    string featureStatusFileName_;
    string overlayFileName_;
    string defaultTaskName_;
    string runtimeCNPMissionManagerTaskName_;
    string runtimeCNPBidderTaskName_;
    string icarusTaskName_;
    string mexpRCFileName_;
    double defaultTaskWeight_;
    int cnpMode_;
    int missionSpecWizardType_;
    int globalFeatureRobotNumIndex_;
    int globalFeatureMissionTimeIndex_;
    int numTaskTypes_;
    int generalToggleWindowResult_;
    bool generalYesNoWindowResult_;
    bool generalYesNoWindowDone_;
    bool generalToggleWindowDone_;
    bool generalMessageWindowIsUp_;
    bool cbrClientInstantiated_;

    static const struct timespec EVENT_WAITER_SLEEP_TIME_NSEC_;
    static const string EMPTY_STRING_;
    static const string STRING_MISSION_TIME_;
    static const string STRING_NUM_ROBOTS_;
    static const string STRING_OVERLAY_NAME_;
    static const string GENERALTOGGLEWINDOW_TITLE_;
    static const double DEFAULT_TASK_WEIGHT_;
    static const int DEFAULT_GENERALTOGGLEWINDOW_WIDTH_;
    static const int DEFAULT_GENERALTOGGLEWINDOW_LINEHEIGHT_;

    Widget createGeneralYesNoWindow_(string question, Widget parentWidget = NULL);
    Widget createGeneralYesNoWindow_(const char *question, Widget parentWidget = NULL);
    void createAndPopupGeneralMessageWindow_(string message, Widget parentWidet = NULL);
    void createAndPopupGeneralToggleWindow_(string title, vector<string> options);
    void popdownGeneralMessageWindow_(void);
    void popupGeneralYesNoWindow_(Widget w);
    void sendGeneralToggleWindowResult_(int result);
    void sendGeneralYesNoWindowResult_(bool result);
    bool confirmUser_(
        string phrase,
        bool useOKOnly,
        Widget parentWidget = NULL);
    bool loadFeaturesFromRCFile_(void);

    static void clearFeature_(MExpFeature_t *feature);
    static void clearFeatureOption_(MExpFeatureOption_t *option);
    static void cbGeneralYesNoWindow_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbGeneralToggleWindow_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbGeneralMessageWindow_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);

    virtual bool initCBRClient_(void);

public:
    CBRWizard(void);
    CBRWizard(
        Display *display,
        Widget parent,
        XtAppContext app,
        int initCNPMode);
    string getMExpRCFileName(void);
    int missionSpecWizardType(void);
    bool confirmUser(
        string phrase,
        bool useOKOnly,
        Widget parentWidget);
    bool missionSpecWizardEnabled(void);
    bool setMissionSpecWizardType(int wizardType);
    virtual ~CBRWizard(void);
};

inline bool CBRWizard::confirmUser(
    string phrase,
    bool useOKOnly,
    Widget parentWidget)
{
    return (confirmUser_(phrase, useOKOnly, parentWidget));
}

inline bool CBRWizard::initCBRClient_(void) {return false;}

inline string CBRWizard::getMExpRCFileName(void)
{
    return mexpRCFileName_;
}

inline int CBRWizard::missionSpecWizardType(void)
{
    return missionSpecWizardType_;
}

#endif
/**********************************************************************
 * $Log: cbrwizard.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.7  2007/09/29 23:46:04  endo
 * Global feature can be now disabled.
 *
 * Revision 1.6  2007/09/28 15:56:13  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.5  2007/09/18 22:35:21  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.4  2007/05/15 18:47:32  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.3  2006/10/23 22:16:52  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.2  2006/09/13 19:05:28  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 07:39:22  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1  2006/01/30 02:53:50  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 **********************************************************************/
