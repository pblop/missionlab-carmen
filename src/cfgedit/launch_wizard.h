/**********************************************************************
 **                                                                  **
 **                         launch_wizard.h                          **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  This is a header file for LaunchWizard class.                   **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef LAUNCH_WIZARD_H
#define LAUNCH_WIZARD_H

/* $Id: launch_wizard.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

// If a new entry is added, make sure to add the corresponding entry
// in LAUNCH_OPTION_NAMES_ below.
typedef enum LaunchWizardOptions_t {
    LAUNCH_WIZARD_OPTION_FSA,
    LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD,
    LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD,
    LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP,
    LAUNCH_WIZARD_OPTION_MISSION_REPAIR,
    NUM_LAUNCH_WIZARD_OPTIONS
};

class LaunchWizard {

    typedef struct LaunchOptionWindowOptionCallbackData_t {
        int optionNum;
        LaunchWizard *launchWizardInstance;
    };

    typedef struct LaunchOptionWindowOKButtonCallbackData_t {
        LaunchWizard *launchWizardInstance;
        Widget launchOptionWindow;
    };

    typedef struct InvalidOptionCombo_t {
        int optionA;
        int optionB;
    };

protected:
    Display *display_;
    Widget parentWidget_;
    XtAppContext appContext_;
    Pixel parentWidgetFg_;
    Pixel parentWidgetBg_;
    Widget launchOptionWindowOption_[NUM_LAUNCH_WIZARD_OPTIONS];
    bool launchOptionWindowDone_;
    bool launchOptions_[NUM_LAUNCH_WIZARD_OPTIONS];

    static const InvalidOptionCombo_t INVALID_OPTION_COMBOS_[];
    static const string EMPTY_STRING_;
    static const string LAUNCH_OPTION_WINDOW_OK_BUTTON_LABEL_;
    static const string LAUNCH_OPTION_WINDOW_TITLE_;
    static const string LAUNCH_OPTION_NAMES_[NUM_LAUNCH_WIZARD_OPTIONS];
    static const int LAUNCH_OPTION_WINDOW_WIDTH_;
    static const int LAUNCH_OPTION_WINDOW_LINEHEIGHT_;
    static const int NUM_INVALID_OPTION_COMBOS_;

    void createAndPopupLaunchOptionWindow_(void);
    void processLaunchOptionWindowOption_(int optionNum);
    void endWizard_(void);

    static void cbLaunchOptionWindowOption_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbLaunchOptionWindowOKButton_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);

public:
    LaunchWizard(
        Display *display,
        Widget parent,
        XtAppContext app);
    ~LaunchWizard(void);
    bool *runWizard(void);
    bool *launchOptions(void);
};

inline bool *LaunchWizard::launchOptions(void)
{
    return launchOptions_;
}

#endif
/**********************************************************************
 * $Log: launch_wizard.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 **********************************************************************/
