/**********************************************************************
 **                                                                  **
 **                              cbrwizard.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the CBRWozard class, including **
 **  common functions between MissionExpert (CfgEdit) and Mlab-      **
 **  MissionDesign (mlab).                                           **
 **                                                                  **
 **  Copyright 2006 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cbrwizard.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

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

#include "cbrwizard.h"
#include "file_utils.h"
#include "mission_expert.h"
#include "cnp_types.h"
#include "mission_spec_wizard_types.h"

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const struct timespec CBRWizard::EVENT_WAITER_SLEEP_TIME_NSEC_ = {0, 1000000};
const string CBRWizard::EMPTY_STRING_ = "";
const string CBRWizard::STRING_MISSION_TIME_ = "MissionTime";
const string CBRWizard::STRING_NUM_ROBOTS_ = "NumberOfRobots";
const string CBRWizard::STRING_OVERLAY_NAME_ = "OverlayName";
const string CBRWizard::GENERALTOGGLEWINDOW_TITLE_ = "Please select one of the following:";
const double CBRWizard::DEFAULT_TASK_WEIGHT_ = 1.0;
const int CBRWizard::DEFAULT_GENERALTOGGLEWINDOW_WIDTH_ = 600;
const int CBRWizard::DEFAULT_GENERALTOGGLEWINDOW_LINEHEIGHT_ = 32;

//-----------------------------------------------------------------------
// Constructor for CBRWizard class.
//-----------------------------------------------------------------------
CBRWizard::CBRWizard(
    Display *display,
    Widget parent,
    XtAppContext app,
    int initCNPMode) :
    display_(display),
    appContext_(app),
    parentWidget_(parent),
    cbrclient_(NULL),
    overlayFileName_(EMPTY_STRING_),
    runtimeCNPMissionManagerTaskName_(EMPTY_STRING_),
    runtimeCNPBidderTaskName_(EMPTY_STRING_),
    icarusTaskName_(EMPTY_STRING_),
    mexpRCFileName_(MEXP_DEFAULT_RC_FILENAME),
    defaultTaskWeight_(DEFAULT_TASK_WEIGHT_),
    cnpMode_(initCNPMode),
    missionSpecWizardType_(MISSION_SPEC_WIZARD_DISABLED),
    globalFeatureRobotNumIndex_(-1),
    globalFeatureMissionTimeIndex_(-1),
    numTaskTypes_(0),
    generalYesNoWindowDone_(false),
    generalToggleWindowDone_(false),
    generalMessageWindowIsUp_(false),
    cbrClientInstantiated_(false)
{
    XtVaGetValues(
        parentWidget_,
        XmNforeground, &parentWidgetFg_,
        XmNbackground, &parentWidgetBg_,
        NULL);
}

//-----------------------------------------------------------------------
// Distructor for CBRWizard class.
//-----------------------------------------------------------------------
CBRWizard::~CBRWizard(void)
{
    if (cbrclient_ != NULL)
    {
        delete cbrclient_;
        cbrclient_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function creates the window that simply asks a "Yes" / "No"
// question. The question statement is the input.
//-----------------------------------------------------------------------
Widget CBRWizard::createGeneralYesNoWindow_(string question, Widget parentWidget)
{
    return (createGeneralYesNoWindow_(question.c_str(), parentWidget));
}

//-----------------------------------------------------------------------
// This function creates the window that simply asks a "Yes" / "No"
// question. The question statement is the input.
//-----------------------------------------------------------------------
Widget CBRWizard::createGeneralYesNoWindow_(const char *question, Widget parentWidget)
{
    XmString yes_str, no_str, msg_str, title_str;
    Arg wargs[16];
    int n=0;
    GeneralYesNoWindowCallbackData_t *callBackData;
    Widget generalYesNoWindow;

    parentWidget = (parentWidget == NULL)? parentWidget_ : parentWidget;

    msg_str = XmStringCreateLtoR((char *)question, XmSTRING_DEFAULT_CHARSET);
    yes_str = XmStringCreateLocalized("  Yes  ");
    no_str = XmStringCreateLocalized("  No  ");
    title_str = XmStringCreateLocalized(" ");

    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(wargs[n], XmNmessageString, msg_str); n++; 
    XtSetArg(wargs[n], XmNokLabelString, yes_str); n++;
    XtSetArg(wargs[n], XmNhelpLabelString, no_str); n++;
    XtSetArg(wargs[n], XmNdialogTitle, title_str); n++;
    generalYesNoWindow = XmCreateMessageDialog(parentWidget, "", wargs, n);

    // "Yes" button
    callBackData = new GeneralYesNoWindowCallbackData_t;
    callBackData->cbrWizardInstance = this;
    callBackData->result = true;

    XtAddCallback
        (generalYesNoWindow,
         XmNokCallback,
         (XtCallbackProc)cbGeneralYesNoWindow_,
         (void *)callBackData);

    // "No" button
    callBackData = new GeneralYesNoWindowCallbackData_t;
    callBackData->cbrWizardInstance = this;
    callBackData->result = false;
    XtAddCallback
        (generalYesNoWindow,
         XmNhelpCallback,
         (XtCallbackProc)cbGeneralYesNoWindow_,
         (void *)callBackData);

    XtUnmanageChild(XmMessageBoxGetChild(generalYesNoWindow, XmDIALOG_CANCEL_BUTTON));

    XmStringFree(msg_str);
    XmStringFree(yes_str);
    XmStringFree(no_str);

    return generalYesNoWindow;
}

//-----------------------------------------------------------------------
// This function popups the window above.
//-----------------------------------------------------------------------
void CBRWizard::popupGeneralYesNoWindow_(Widget w)
{
    generalYesNoWindowResult_ = false;
    generalYesNoWindowDone_ = false;
    XtManageChild(w);
}

//-----------------------------------------------------------------------
// This function sets the "Yes" / "No" variable according to the answer
// given by the user.
//-----------------------------------------------------------------------
void CBRWizard::sendGeneralYesNoWindowResult_(bool result)
{
    generalYesNoWindowResult_ = result;
    generalYesNoWindowDone_ = true;
}

//-----------------------------------------------------------------------
// This function asks the user to confirm the specified phrase.
//-----------------------------------------------------------------------
bool CBRWizard::confirmUser_(
    string phrase,
    bool useOKOnly,
    Widget parentWidget)
{
    Widget w;
    XmString ok;

    w = createGeneralYesNoWindow_(phrase.c_str(), parentWidget);

    if (useOKOnly)
    {
        ok = XmStringCreateLocalized("  OK  ");
        XtVaSetValues(w, XmNokLabelString, ok, NULL);
        XtUnmanageChild(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
        XmStringFree(ok);
    }

    popupGeneralYesNoWindow_(w);

    // Wait for the answer.
    while(!generalYesNoWindowDone_)
    {
        if (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
        else
        {
            if (cbrclient_ != NULL)
            {
                cbrclient_->lockSocketMutex();
            }

            nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);

            if (cbrclient_ != NULL)
            {
                cbrclient_->unlockSocketMutex();
            }
        }
    }

    return generalYesNoWindowResult_;
}

//-----------------------------------------------------------------------
// This function creates a genetic message window without any button.
//-----------------------------------------------------------------------
void CBRWizard::createAndPopupGeneralMessageWindow_(string message, Widget parentWidget)
{
    XmString msg_str, title_str;
    Arg wargs[16];
    int n = 0;

    if (generalMessageWindowIsUp_)
    {
        popdownGeneralMessageWindow_();
    }

    msg_str = XmStringCreateLtoR(
        (char *)(message.c_str()),
        XmSTRING_DEFAULT_CHARSET);
    title_str = XmStringCreateLocalized(" ");

    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
    XtSetArg(wargs[n], XmNmessageString, msg_str); n++; 
    XtSetArg(wargs[n], XmNdialogTitle, title_str); n++;
    generalMessageWindow_ = XmCreateMessageDialog(parentWidget_, "", wargs, n);

    XtAddCallback
        (generalMessageWindow_,
         XmNokCallback,
         (XtCallbackProc)cbGeneralMessageWindow_,
         (void *)this);

    XtUnmanageChild(
        XmMessageBoxGetChild(generalMessageWindow_, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(
        XmMessageBoxGetChild(generalMessageWindow_, XmDIALOG_HELP_BUTTON));

    XmStringFree(msg_str);
    XmStringFree(title_str);

    generalMessageWindowIsUp_ = true;

    XtManageChild(generalMessageWindow_);
}

//-----------------------------------------------------------------------
// This function creates the global preference menu window.
//-----------------------------------------------------------------------
void CBRWizard::createAndPopupGeneralToggleWindow_(
    string title,
    vector<string> options)
{
    Widget generalToggleWindow;
    Widget main_frame, main, label_frame, label_box, label;
    Widget option_frame, option_box;
    Widget *generalToggleWindowOption = NULL;
    vector<Widget> stackedWidget;
    GeneralToggleWindowCallbackData_t *callbackData;
    int i, line = 0;
    int screenNumber;
    int posX, posY, displayWidth, displayHeight;
    int generalToggleWindowLineHeight;
    int generalToggleWindowWidth, generalToggleWindowHeight;
    int numStackedWidget, numOptions;

    // Initialize the variable.
    generalToggleWindowResult_ = -1;
    generalToggleWindowDone_ = false;

    // Setting the height of the window.
    numOptions = options.size();
    generalToggleWindowLineHeight = DEFAULT_GENERALTOGGLEWINDOW_LINEHEIGHT_;
    generalToggleWindowHeight = (1+numOptions)*(generalToggleWindowLineHeight+2)+20;
    generalToggleWindowWidth = DEFAULT_GENERALTOGGLEWINDOW_WIDTH_;

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth-generalToggleWindowWidth)/2;
    posY = (displayHeight-generalToggleWindowHeight)/2;

    // Popup shell.
    generalToggleWindow = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, generalToggleWindowHeight,
         XmNwidth, generalToggleWindowWidth,
         NULL);

    // The biggest frame.
    main_frame = XtVaCreateWidget
        ("main_frame",
         xmFrameWidgetClass, generalToggleWindow,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, generalToggleWindowHeight,
         XmNwidth, generalToggleWindowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, generalToggleWindow,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, generalToggleWindow,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, generalToggleWindow,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, generalToggleWindow,
         NULL);

    stackedWidget.push_back(main_frame);

    main = XtVaCreateWidget
        ("main",
         xmRowColumnWidgetClass, main_frame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, generalToggleWindowHeight,
         XmNwidth, generalToggleWindowWidth+10,
         NULL);

    stackedWidget.push_back(main);

    // The widget for the main label.
    label_frame = XtVaCreateWidget
        ("label_frame",
         //xmFrameWidgetClass, main,
         xmFormWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, generalToggleWindowLineHeight,
         XmNwidth, generalToggleWindowWidth+10,
         NULL);

    stackedWidget.push_back(label_frame);

    // The widget for the main label.
    label_box = XtVaCreateWidget
        ("label_box",
         xmRowColumnWidgetClass, label_frame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNisAligned, true,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 1,
         XmNheight, generalToggleWindowLineHeight,
         XmNwidth, generalToggleWindowWidth,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(label_box);

    if (title == EMPTY_STRING_)
    {
        title = GENERALTOGGLEWINDOW_TITLE_;
    }

    // The main label.
    label = XtVaCreateWidget
        (title.c_str(),
         xmLabelGadgetClass, label_box,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, generalToggleWindowLineHeight,
         XmNwidth, generalToggleWindowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, label_box,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, label_box,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, label_box,
         NULL);
    line++;

    stackedWidget.push_back(label);

    // It is a 2D array. Create the 1D part (feature) first.
    generalToggleWindowOption = new Widget[numOptions];

    //  Create the option raws sequentially.
    for (i = 0; i < numOptions; i++)
    {
        // The widget for the main label.
        option_frame = XtVaCreateWidget
            ("option_frame",
             xmFrameWidgetClass, main,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, generalToggleWindowLineHeight-2,
             XmNwidth, generalToggleWindowWidth-10,
             NULL);

        stackedWidget.push_back(option_frame);

        // Two-colum widget for the option.
        option_box = XtVaCreateWidget
            ("option_box",
             xmRowColumnWidgetClass, option_frame,
             XmNpacking, XmPACK_COLUMN,
             XmNorientation, XmHORIZONTAL,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNnumColumns, 1,
             XmNheight, generalToggleWindowLineHeight,
             XmNwidth, generalToggleWindowWidth-10,
             XmNtopAttachment, XmATTACH_FORM,
             XmNrightAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             NULL);

        stackedWidget.push_back(option_box);

        // Create the second dimention (option) of this array.
        generalToggleWindowOption = new Widget[numOptions];
        generalToggleWindowOption[i] = XtVaCreateWidget
            (options[i].c_str(),
             xmToggleButtonGadgetClass, option_box,
             XmNradioBehavior, true,
             XmNradioAlwaysOne, true,
             XmNheight, generalToggleWindowLineHeight,
             XmNwidth, generalToggleWindowWidth-10,
             XmNset, false,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNhighlightColor, parentWidgetBg_,
             NULL);

        stackedWidget.push_back(generalToggleWindowOption[i]);

        callbackData = new GeneralToggleWindowCallbackData_t;
        callbackData->cbrWizardInstance = this;
        callbackData->result = i;
        callbackData->generalToggleWindow = generalToggleWindow;
        XtAddCallback
            (generalToggleWindowOption[i],
             XmNvalueChangedCallback,
             (XtCallbackProc)cbGeneralToggleWindow_,
             (void *)callbackData);

        line++;
    }
    
    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }

    XtPopup(generalToggleWindow, XtGrabNone);
}

//-----------------------------------------------------------------------
// This function saves the result of the generalToggleWindow.
//-----------------------------------------------------------------------
void CBRWizard::sendGeneralToggleWindowResult_(int result)
{
    generalToggleWindowResult_ = result;
    generalToggleWindowDone_ = true;
}

//-----------------------------------------------------------------------
// This function popups the window above.
//-----------------------------------------------------------------------
void CBRWizard::popdownGeneralMessageWindow_(void)
{
    if (generalMessageWindowIsUp_)
    {
        generalMessageWindowIsUp_ = false;
        XtUnmanageChild(generalMessageWindow_);
        XtDestroyWidget(generalMessageWindow_);
    }
}

//-----------------------------------------------------------------------
// This function clears the specified feature options.
//-----------------------------------------------------------------------
void CBRWizard::clearFeatureOption_(MExpFeatureOption_t *option)
{
    option->value = EMPTY_STRING_;
}

//-----------------------------------------------------------------------
// This function clears the specified feature.
//-----------------------------------------------------------------------
void CBRWizard::clearFeature_(MExpFeature_t *feature)
{
    feature->id = 0;
    feature->type = 0;
    feature->nonIndex = false;
    feature->hide = false;
    feature->allowDisable = false;
    feature->weight = 0;
    feature->name = EMPTY_STRING_;
    feature->optionType = 0;
    feature->options.clear();
    feature->selectedOption = 0;
}

//-----------------------------------------------------------------------
// This function loads the features from the RC file.
//-----------------------------------------------------------------------
bool CBRWizard::loadFeaturesFromRCFile_(void)
{
    MExpFeature_t feature;
    MExpFeatureOption_t featureOption;
    MExpMissionTask_t task;
    MExpFeature_t constraint;
    MExpFeatureOption_t constraintOption;
    MExpRobotConstraints_t robotConstraints;
    //const MExpFeature_t EMPTY_FEATURE;
    //const MExpFeatureOption_t EMPTY_FEATURE_OPTION;
    const MExpMissionTask_t EMPTY_TASK = {0, 0, 0, string(), string(), vector<MExpFeature_t>(), 0, 0};
    const MExpFeature_t EMPTY_CONSTRAINT = {0 ,0, 0, 0, 0, 0, string(), 0, vector<MExpFeatureOption_t>(), 0};
    const MExpFeatureOption_t EMPTY_CONSTRAINT_OPTION = {string()};
    const MExpRobotConstraints_t EMPTY_ROBOT_CONSTRAINTS =  {0, string(), vector<MExpFeature_t>()};
    FILE *rcFile = NULL;
    string rcFilename;
    string bufString1, bufString2;
    char buf1[1024], buf2[1024], buf3[1024], buf[1024];
    int index, numRobots;
    int constraintType = -1;
    bool isRuntimeCNPTask = false;
    bool isIcarusTask = false;

    // First, find the RC file.
    rcFilename = find_file_in_envpath(mexpRCFileName_);

    if (rcFilename == EMPTY_STRING_)
    {
        fprintf(
            stderr,
            "Error(mlab): RC file (%s) could not be loaded.\n",
            rcFilename.c_str());
        return false;
    }

    // Load the RC file.
    rcFile = fopen(rcFilename.c_str(), "r");

    // Initialize the vectors.
    defaultGlobalFeatures_.clear();
    defaultLocalFeatures_.clear();
    defaultTaskList_.clear();
    robotConstraintsList_.clear();

    while (fscanf(rcFile, "%s", buf1) != EOF)
    {
        bufString1 = buf1;

        if (bufString1[0] == '#')
        {
            // It's a comment.
            while ((fgetc(rcFile) != '\n') && (!feof(rcFile)))
            {
                // Keep reading until next line.
            }
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_KEY)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            bufString2 = buf2;

            if (bufString2 == MEXP_RC_STRING_FEATURE_BEGIN)
            {
                //feature = EMPTY_FEATURE;
                //featureOption = EMPTY_FEATURE_OPTION;
                CBRWizard::clearFeature_(&feature);
                CBRWizard::clearFeatureOption_(&featureOption);
            }
            else if (bufString2 == MEXP_RC_STRING_FEATURE_END)
            {
                switch (cnpMode_) {

                case CNP_MODE_PREMISSION:
                    switch (feature.type) {
                    
                    case MEXP_FEATURE_GLOBAL:
                        feature.id = defaultGlobalFeatures_.size();
                        defaultGlobalFeatures_.push_back(feature);

                        // Remember the index for the feature that specifies
                        // number of robots
                        if (feature.name == STRING_NUM_ROBOTS_)
                        {
                            globalFeatureRobotNumIndex_ = feature.id;
                        }
                        else if (feature.name == STRING_MISSION_TIME_)
                        {
                            globalFeatureMissionTimeIndex_ = feature.id;
                        }
                        break;

                    case MEXP_FEATURE_LOCAL:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;

                    case MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;

                    case MEXP_FEATURE_TASK_CONSTRAINT:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;
                    } // switch (feature.type)
                    break;

                case CNP_MODE_RUNTIME:
                    switch (feature.type) {
                    
                    case MEXP_FEATURE_GLOBAL:
                        feature.id = defaultGlobalFeatures_.size();
                        defaultGlobalFeatures_.push_back(feature);

                        // Remember the index for the feature that specifies
                        // number of robots
                        if (feature.name == STRING_NUM_ROBOTS_)
                        {
                            globalFeatureRobotNumIndex_ = feature.id;
                        }
                        else if (feature.name == STRING_MISSION_TIME_)
                        {
                            globalFeatureMissionTimeIndex_ = feature.id;
                        }
                        break;

                    case MEXP_FEATURE_RUNTIME_CNP_MISSION_MANAGER:
                    case MEXP_FEATURE_RUNTIME_CNP_BIDDER:
                    case MEXP_FEATURE_RUNTIME_CNP_BOTH:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;
                    } // switch (feature.type)
                    break;

                default:
                    switch (feature.type) {
                    
                    case MEXP_FEATURE_GLOBAL:
                        feature.id = defaultGlobalFeatures_.size();
                        defaultGlobalFeatures_.push_back(feature);

                        // Remember the index for the feature that specifies
                        // number of robots
                        if (feature.name == STRING_NUM_ROBOTS_)
                        {
                            globalFeatureRobotNumIndex_ = feature.id;
                        }
                        else if (feature.name == STRING_MISSION_TIME_)
                        {
                            globalFeatureMissionTimeIndex_ = feature.id;
                        }
                        break;

                    case MEXP_FEATURE_LOCAL:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;

                    case MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT:
                        feature.id = defaultLocalFeatures_.size();
                        defaultLocalFeatures_.push_back(feature);
                        break;
                    } // switch (feature.type)
                    break;
                }
            }
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_TYPE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.type = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_NAME)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.name = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_NON_INDEX)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.nonIndex = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_HIDE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.hide = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_WEIGHT)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.weight = atof(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_ALLOW_DISABLE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.allowDisable = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_OPTION_TYPE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.optionType = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_SELECTED_OPTION)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            feature.selectedOption = atof(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_FEATURE_OPTION)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            featureOption.value = buf2;
            feature.options.push_back(featureOption);
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_KEY)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            bufString2 = buf2;

            if (bufString2 == MEXP_RC_STRING_TASK_BEGIN)
            {
                task = EMPTY_TASK;
                isRuntimeCNPTask = false;
                isIcarusTask = false;
            }
            else if (bufString2 == MEXP_RC_STRING_TASK_END)
            {
                // ENDO ???
                //if (missionSpecWizardEnabled())
                //{
                    if (isIcarusTask)
                    {
                        task.id = defaultTaskList_.size();
                        defaultTaskList_.push_back(task);
                        numTaskTypes_ = defaultTaskList_.size();
                    }
                    //}
                else
                {
                    switch (cnpMode_) {

                    case CNP_MODE_RUNTIME:
                        if (isRuntimeCNPTask)
                        {
                            task.id = defaultTaskList_.size();
                            defaultTaskList_.push_back(task);
                            numTaskTypes_ = defaultTaskList_.size();
                        }
                        break;

                    default:
                        if (!isRuntimeCNPTask)
                        {
                            task.id = defaultTaskList_.size();
                            defaultTaskList_.push_back(task);
                            numTaskTypes_ = defaultTaskList_.size();
                        }
                        break;
                    }
                }
            }
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_NAME)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            task.name = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_BUTTON_NAME)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            task.buttonName = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_MAX_USAGE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            bufString2 = buf2;

            if (bufString2 == MEXP_STRING_INFINITY)
            {
                task.maxUsage = MEXP_USAGE_INFINITY;
            }
            else
            {
                task.maxUsage = atoi(buf2);
            }
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_RUNTIME_CNP)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            isRuntimeCNPTask = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_ICARUS)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            isIcarusTask = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_TASK_SPECIFIC_SELECTED_OPTION)
        {
            if (fscanf(rcFile, "%s %s", buf2, buf3) == EOF)
            {
                break;
            }
            
            feature.name = buf2;
            feature.selectedOption = atof(buf3);
            task.localFeatures.push_back(feature);
        }
        else if (bufString1 == MEXP_RC_STRING_DEFAULT_TASK)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            defaultTaskName_ = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_DEFAULT_TASK_WEIGHT)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            defaultTaskWeight_ = atof(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_RUNTIME_CNP_MISSION_MANAGER_TASK)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            runtimeCNPMissionManagerTaskName_ = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_RUNTIME_CNP_BIDDER_TASK)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            runtimeCNPBidderTaskName_ = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_ICARUS_TASK)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            icarusTaskName_ = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_ROBOT_CONSTRAINTS_KEY)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            bufString2 = buf2;

            if (bufString2 == MEXP_RC_STRING_ROBOT_CONSTRAINTS_BEGIN)
            {
                robotConstraints = EMPTY_ROBOT_CONSTRAINTS;
                constraintType = MEXP_FEATURE_ROBOT_CONSTRAINT;
            }
            else if (bufString2 == MEXP_RC_STRING_ROBOT_CONSTRAINTS_END)
            {
                switch (cnpMode_) {

                case CNP_MODE_PREMISSION:
                    robotConstraintsList_.push_back(robotConstraints);

                    // Based on this, set the number of robots in the
                    // Global Setting window.
                    if (globalFeatureRobotNumIndex_ >= 0)
                    {
                        index = globalFeatureRobotNumIndex_;
                        numRobots = robotConstraintsList_.size();
                        defaultGlobalFeatures_[index].selectedOption = (float)numRobots;
                    }
                    break;

                case CNP_MODE_RUNTIME:
                    robotConstraintsList_.push_back(robotConstraints);
                    break;
                }
                constraintType = -1;
            }
        }
        else if (bufString1 == MEXP_RC_STRING_ROBOT_CONSTRAINTS_ROBOT_ID)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            robotConstraints.id = atoi(buf2);
            
            sprintf(buf, "robot%d", robotConstraints.id);
            robotConstraints.name = buf;
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_KEY)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            bufString2 = buf2;

            if (bufString2 == MEXP_RC_STRING_CONSTRAINT_BEGIN)
            {
                constraint = EMPTY_CONSTRAINT;
                constraint.type = constraintType;

                switch (constraintType) {

                case MEXP_FEATURE_ROBOT_CONSTRAINT:
                    constraint.id = robotConstraints.constraints.size();
                    break;
                }
            }
            else if (bufString2 == MEXP_RC_STRING_CONSTRAINT_END)
            {
                switch (constraintType) {

                case MEXP_FEATURE_ROBOT_CONSTRAINT:
                    robotConstraints.constraints.push_back(constraint);
                    break;
                }
            }
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_NAME)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }

            constraint.name = buf2;
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_HIDE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }

            constraint.hide = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_OPTION_TYPE)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            constraint.optionType = atoi(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_SELECTED_OPTION)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            constraint.selectedOption = atof(buf2);
        }
        else if (bufString1 == MEXP_RC_STRING_CONSTRAINT_OPTION)
        {
            if (fscanf(rcFile, "%s", buf2) == EOF)
            {
                break;
            }
            
            constraintOption.value = buf2;
            constraint.options.push_back(constraintOption);
        }
    }

    return true;
}

//-----------------------------------------------------------------------
// This function returns true if some type of MissionSpecWizard is enabled.
//-----------------------------------------------------------------------
bool CBRWizard::missionSpecWizardEnabled(void)
{
    bool enabled = false;

    enabled = (missionSpecWizardType_ == MISSION_SPEC_WIZARD_DISABLED)? false : true;

    return enabled;
}

//-----------------------------------------------------------------------
// This function sets the type of MissionSpecWizard.
//-----------------------------------------------------------------------
bool CBRWizard::setMissionSpecWizardType(int wizardType)
{
    if ((wizardType < MISSION_SPEC_WIZARD_DISABLED) ||
        (wizardType >= NUM_MISSION_SPEC_WIZARD_TYPES))
    {
        fprintf(
            stderr,
            "Error: CBRWizard::setMissionSpecWizardType(): Invalid wizard type: %d.\n",
            wizardType);

        missionSpecWizardType_ = MISSION_SPEC_WIZARD_DISABLED;
        return false;
    }

    missionSpecWizardType_ = wizardType;
    return true;
}

//-----------------------------------------------------------------------
// This callback function sets the "Yes" / "No" variable according to the
// answer given by the user.
//-----------------------------------------------------------------------
void CBRWizard::cbGeneralYesNoWindow_(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    CBRWizard *cbrWizardInstance = NULL;
    bool result;

    cbrWizardInstance = ((GeneralYesNoWindowCallbackData_t *)clientData)->cbrWizardInstance;
    result = ((GeneralYesNoWindowCallbackData_t *)clientData)->result;

    cbrWizardInstance->sendGeneralYesNoWindowResult_(result);
    XtDestroyWidget(w);
}

//-----------------------------------------------------------------------
// This callback function gets called when the user press one of the
// options in the generalToggleWindow.
//-----------------------------------------------------------------------
void CBRWizard::cbGeneralToggleWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    CBRWizard *cbrWizardInstance = NULL;
    Widget generalToggleWindow;
    int result;

    cbrWizardInstance = ((GeneralToggleWindowCallbackData_t *)clientData)->cbrWizardInstance;
    result = ((GeneralToggleWindowCallbackData_t *)clientData)->result;
    generalToggleWindow = ((GeneralToggleWindowCallbackData_t *)clientData)->generalToggleWindow;

    cbrWizardInstance->sendGeneralToggleWindowResult_(result);

    XtDestroyWidget(generalToggleWindow);
}

//-----------------------------------------------------------------------
// This callback function for the generalMessageWindow_.
//-----------------------------------------------------------------------
void CBRWizard::cbGeneralMessageWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    CBRWizard *cbrWizardInstance = NULL;

    cbrWizardInstance = (CBRWizard *)clientData;
    cbrWizardInstance->popdownGeneralMessageWindow_();
}

/**********************************************************************
 * $Log: cbrwizard.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.9  2007/09/29 23:46:04  endo
 * Global feature can be now disabled.
 *
 * Revision 1.8  2007/09/28 15:56:13  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.7  2007/09/18 22:35:21  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.6  2007/06/28 12:24:20  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.5  2007/06/01 04:41:54  endo
 * LaunchWizard implemented.
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
 * Revision 1.1  2006/01/30 02:53:50  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 **********************************************************************/
