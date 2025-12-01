/**********************************************************************
 **                                                                  **
 **                          launch_wizard.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the LaunchWizard class.        **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: launch_wizard.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>
#include <X11/Xthreads.h>

#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include <Xm/ArrowBG.h>
#include <Xm/BulletinB.h>
#include <Xm/CascadeB.h>   
#include <Xm/CascadeBG.h>   
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/MenuShell.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>

#include "launch_wizard.h"
#include "globals.h"
#include "popups.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const LaunchWizard::InvalidOptionCombo_t LaunchWizard::INVALID_OPTION_COMBOS_[] =
{
    {LAUNCH_WIZARD_OPTION_FSA, LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD},
    {LAUNCH_WIZARD_OPTION_FSA, LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD},
    {LAUNCH_WIZARD_OPTION_FSA, LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP},
    {LAUNCH_WIZARD_OPTION_FSA, LAUNCH_WIZARD_OPTION_MISSION_REPAIR},
    {LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD, LAUNCH_WIZARD_OPTION_FSA},
    //{LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD, LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD},
    {LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD, LAUNCH_WIZARD_OPTION_FSA},
    //{LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD, LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD},
    {LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD, LAUNCH_WIZARD_OPTION_MISSION_REPAIR},
    {LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP, LAUNCH_WIZARD_OPTION_FSA},
    {LAUNCH_WIZARD_OPTION_MISSION_REPAIR, LAUNCH_WIZARD_OPTION_FSA},
    {LAUNCH_WIZARD_OPTION_MISSION_REPAIR, LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD}
};
const string LaunchWizard::EMPTY_STRING_ = "";
const string LaunchWizard::LAUNCH_OPTION_WINDOW_OK_BUTTON_LABEL_ = "Launch";
const string LaunchWizard::LAUNCH_OPTION_WINDOW_TITLE_ = "Please select all of the options that you want to turn on:";
const string LaunchWizard::LAUNCH_OPTION_NAMES_[NUM_LAUNCH_WIZARD_OPTIONS] = {
    "Visual Programming Interface", // LAUNCH_WIZARD_OPTION_FSA,
    "Iconic CBR Wizard Interface", // LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD,
    "Geo-Driven Mission Specification Wizard", // LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD,
    "CBR + Premission CNP Module", // LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP,
    "Automated Mission Repair Mechanism" // LAUNCH_WIZARD_OPTION_MISSION_REPAIR,
};

const int LaunchWizard::LAUNCH_OPTION_WINDOW_WIDTH_ = 500;
const int LaunchWizard::LAUNCH_OPTION_WINDOW_LINEHEIGHT_ = 32;
const int LaunchWizard::NUM_INVALID_OPTION_COMBOS_ = sizeof(INVALID_OPTION_COMBOS_)/sizeof(InvalidOptionCombo_t);

//-----------------------------------------------------------------------
// Constructor for LaunchWizard class.
//-----------------------------------------------------------------------
LaunchWizard::LaunchWizard(
    Display *display,
    Widget parent,
    XtAppContext app) :
    display_(display),
    parentWidget_(parent),
    appContext_(app),
    launchOptionWindowDone_(false)
{
    XtVaGetValues(
        parentWidget_,
        XmNforeground, &parentWidgetFg_,
        XmNbackground, &parentWidgetBg_,
        NULL);

    memset(launchOptions_, 0x0, sizeof(bool)*NUM_LAUNCH_WIZARD_OPTIONS);
}

//-----------------------------------------------------------------------
// Distructor for LaunchWizard class.
//-----------------------------------------------------------------------
LaunchWizard::~LaunchWizard(void)
{
}

//-----------------------------------------------------------------------
// This function pops up the launch wizard
//-----------------------------------------------------------------------
bool *LaunchWizard::runWizard(void)
{
    createAndPopupLaunchOptionWindow_();

    // Wait for the answer.
    while(!launchOptionWindowDone_)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    return launchOptions_;
}

//-----------------------------------------------------------------------
// This function creates the general toggle window.
//-----------------------------------------------------------------------
void LaunchWizard::createAndPopupLaunchOptionWindow_(void)
{
    Widget launchOptionWindow;
    Widget mainFrame, main, labelFrame, labelBox, label;
    Widget optionFrame, optionBox, actionFrame, button, spacerFrame;
    vector<Widget> stackedWidget;
    LaunchOptionWindowOptionCallbackData_t *optionCallbackData;
    LaunchOptionWindowOKButtonCallbackData_t *okButtonCallbackData;
    int i, line = 0;
    int screenNumber;
    int posX, posY, displayWidth, displayHeight;
    int launchOptionWindowLineHeight;
    int launchOptionWindowWidth, launchOptionWindowHeight;
    int numStackedWidget, numOptions;

    // Initialize the variable.
    launchOptionWindowDone_ = false;

    // Setting the height of the window.
    numOptions = NUM_LAUNCH_WIZARD_OPTIONS;
    launchOptionWindowLineHeight = LAUNCH_OPTION_WINDOW_LINEHEIGHT_;
    launchOptionWindowHeight = (3+numOptions)*(launchOptionWindowLineHeight+2)+10;
    launchOptionWindowWidth = LAUNCH_OPTION_WINDOW_WIDTH_;

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth-launchOptionWindowWidth)/2;
    posY = (displayHeight-launchOptionWindowHeight)/2;

    // Popup shell.
    launchOptionWindow = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, launchOptionWindowHeight,
         XmNwidth, launchOptionWindowWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFrameWidgetClass, launchOptionWindow,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, launchOptionWindowHeight,
         XmNwidth, launchOptionWindowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, launchOptionWindow,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, launchOptionWindow,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, launchOptionWindow,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, launchOptionWindow,
         NULL);

    stackedWidget.push_back(mainFrame);

    main = XtVaCreateWidget
        ("main",
         xmRowColumnWidgetClass, mainFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, launchOptionWindowHeight,
         XmNwidth, launchOptionWindowWidth+10,
         NULL);

    stackedWidget.push_back(main);

    // The widget for the main label.
    labelFrame = XtVaCreateWidget
        ("labelFrame",
         //xmFrameWidgetClass, main,
         xmFormWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, launchOptionWindowLineHeight,
         XmNwidth, launchOptionWindowWidth+10,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(labelFrame);

    // The widget for the main label.
    labelBox = XtVaCreateWidget
        ("labelBox",
         xmRowColumnWidgetClass, labelFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNisAligned, true,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 1,
         XmNheight, launchOptionWindowLineHeight,
         XmNwidth, launchOptionWindowWidth,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    label = XtVaCreateWidget
        (LAUNCH_OPTION_WINDOW_TITLE_.c_str(),
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, launchOptionWindowLineHeight,
         XmNwidth, launchOptionWindowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);
    line++;

    stackedWidget.push_back(label);

    //  Create the option raws sequentially.
    for (i = 0; i < numOptions; i++)
    {
        // The widget for the main label.
        optionFrame = XtVaCreateWidget
            ("optionFrame",
             //xmFrameWidgetClass, main,
             xmFormWidgetClass, main,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, launchOptionWindowLineHeight-2,
             XmNwidth, launchOptionWindowWidth-10,
             XmNborderWidth, 0,
             NULL);

        stackedWidget.push_back(optionFrame);

        // Two-colum widget for the option.
        optionBox = XtVaCreateWidget
            ("optionBox",
             xmRowColumnWidgetClass, optionFrame,
             XmNpacking, XmPACK_COLUMN,
             XmNorientation, XmHORIZONTAL,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNnumColumns, 1,
             XmNheight, launchOptionWindowLineHeight,
             XmNwidth, launchOptionWindowWidth-10,
             XmNtopAttachment, XmATTACH_FORM,
             XmNrightAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNborderWidth, 0,
             NULL);

        stackedWidget.push_back(optionBox);

        // Create the second dimention (option) of this array.
        launchOptionWindowOption_[i] = XtVaCreateWidget
            (LAUNCH_OPTION_NAMES_[i].c_str(),
             xmToggleButtonGadgetClass, optionBox,
             XmNradioBehavior, true,
             XmNradioAlwaysOne, true,
             XmNheight, launchOptionWindowLineHeight,
             XmNwidth, launchOptionWindowWidth-10,
             XmNset, launchOptions_[i],
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNhighlightColor, parentWidgetBg_,
             NULL);

        stackedWidget.push_back(launchOptionWindowOption_[i]);

        optionCallbackData = new LaunchOptionWindowOptionCallbackData_t;
        optionCallbackData->launchWizardInstance = this;
        optionCallbackData->optionNum = i;
        XtAddCallback
            (launchOptionWindowOption_[i],
             XmNvalueChangedCallback,
             (XtCallbackProc)cbLaunchOptionWindowOption_,
             (void *)optionCallbackData);

        line++;
    }
    
    spacerFrame = XtVaCreateWidget
        ("spacerFrame",
         xmSeparatorGadgetClass, main,
         XmNwidth, launchOptionWindowWidth+10,
         NULL);

    stackedWidget.push_back(spacerFrame);

    // The widget for the action buttons
    actionFrame = XtVaCreateWidget
        ("actionFrame",
         xmFormWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, launchOptionWindowLineHeight*2,
         XmNwidth, launchOptionWindowWidth+10,
         XmNborderWidth, 0,
         XmNfractionBase, 3,
         NULL);

    stackedWidget.push_back(actionFrame);

    button = XtVaCreateManagedWidget
        ((char *)LAUNCH_OPTION_WINDOW_OK_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, actionFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 1,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 2,
         XmNtraversalOn, false,
         XmNborderWidth, 2,
         XmNborderColor, gCfgEditPixels.white,
         NULL);

    stackedWidget.push_back(button);

    okButtonCallbackData = new LaunchOptionWindowOKButtonCallbackData_t;
    okButtonCallbackData->launchWizardInstance = this;
    okButtonCallbackData->launchOptionWindow = launchOptionWindow;
    XtAddCallback
        (button,
         XmNactivateCallback,
         (XtCallbackProc)cbLaunchOptionWindowOKButton_,
         (void *)okButtonCallbackData);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }

    XtPopup(launchOptionWindow, XtGrabNone);
}

//-----------------------------------------------------------------------
// This function process the launch option window's opion press.
//-----------------------------------------------------------------------
void LaunchWizard::processLaunchOptionWindowOption_(int optionNum)
{
    Widget option;
    char buf[1024];
    int i, conflictingOptionNum;
    bool set = false, invalid = false;

    option = launchOptionWindowOption_[optionNum];

    XtVaGetValues(
        option,
        XmNset, &set,
        NULL);

    if (set)
    {
        for (i = 0; i < NUM_INVALID_OPTION_COMBOS_; i++)
        {
            if (INVALID_OPTION_COMBOS_[i].optionA == optionNum)
            {
                if (launchOptions_[INVALID_OPTION_COMBOS_[i].optionB])
                {
                    invalid = true;
                    conflictingOptionNum = INVALID_OPTION_COMBOS_[i].optionB; 
                    break;
                }
            }
        }
    }

    if (invalid)
    {
        sprintf(
            buf,
            "Invalid Selection:\n\"%s\" conflicts with \"%s\".",
            LAUNCH_OPTION_NAMES_[optionNum].c_str(),
            LAUNCH_OPTION_NAMES_[conflictingOptionNum].c_str());

        sprintf(
            buf,
            "%s\nPlease unselect \"%s\" first.",
            buf,
            LAUNCH_OPTION_NAMES_[conflictingOptionNum].c_str());
        warn_userf(buf);

        XtVaSetValues(
            option,
            XmNset, false,
            NULL);
        return;
    }

    launchOptions_[optionNum] = set;
}

//-----------------------------------------------------------------------
// This function saves the result of the launchOptionWindow.
//-----------------------------------------------------------------------
void LaunchWizard::endWizard_(void)
{
    int i;
    bool hasEnabledOption = false;

    // If none of then was selected, use FSA.
    for (i = 0; i < NUM_LAUNCH_WIZARD_OPTIONS; i++)
    {
        if (launchOptions_[i])
        {
            hasEnabledOption = true;
            break;
        }
    }

    if (!hasEnabledOption)
    {
        launchOptions_[LAUNCH_WIZARD_OPTION_FSA] = true;
    }

    launchOptionWindowDone_ = true;
}

//-----------------------------------------------------------------------
// This callback function gets called when the user press one of the
// options in the launchOptionWindow.
//-----------------------------------------------------------------------
void LaunchWizard::cbLaunchOptionWindowOption_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    LaunchWizard *launchWizardInstance = NULL;
    int optionNum;

    launchWizardInstance = ((LaunchOptionWindowOptionCallbackData_t *)clientData)->launchWizardInstance;
    optionNum = ((LaunchOptionWindowOptionCallbackData_t *)clientData)->optionNum;
    launchWizardInstance->processLaunchOptionWindowOption_(optionNum);
}

//-----------------------------------------------------------------------
// This callback function gets called when OK button in the
// launchOptionWindow.
//-----------------------------------------------------------------------
void LaunchWizard::cbLaunchOptionWindowOKButton_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    LaunchWizard *launchWizardInstance = NULL;
    Widget launchOptionWindow;

    launchWizardInstance = ((LaunchOptionWindowOKButtonCallbackData_t *)clientData)->launchWizardInstance;
    launchOptionWindow = ((LaunchOptionWindowOKButtonCallbackData_t *)clientData)->launchOptionWindow;
    launchWizardInstance->endWizard_();
    XtDestroyWidget(launchOptionWindow);
}

/**********************************************************************
 * $Log: launch_wizard.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/09/07 23:10:03  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 * Revision 1.3  2007/06/28 12:23:11  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.2  2007/06/28 03:54:20  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.1  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 **********************************************************************/
