/**********************************************************************
 **                                                                  **
 **                           gt_playback.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_playback.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <vector>

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
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#include "bitmaps/playBtn.bit"
#include "bitmaps/stopBtn.bit"
#include "bitmaps/pauseBtn.bit"
#include "bitmaps/ffBtn.bit"
#include "bitmaps/rewBtn.bit"

#include "gt_playback.h"
#include "console.h"
#include "gt_console_db.h"
#include "gt_sim.h"
#include "gt_scale.h"
#include "mlab_cbrclient.h"
#include "cbrplanner_protocol.h"
#include "mission_design.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define MPB_CREATE_PIXMAP(name,fg,bg,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, fg, bg, \
        DefaultDepthOfScreen(XtScreen(parent)))
#define MPB_XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const double MlabPlayBack::ROBOT2DISPLAY_RATIO_ = 0.02;
const int MlabPlayBack::PLAYBIACK_NUM_STATUS_ = 5;
const int MlabPlayBack::DEFAULT_PLAYBACK_PANEL_HEIGHT_ = 120;
const int MlabPlayBack::DEFAULT_PLAYBACK_PANEL_WIDTH_ = 240;
const int MlabPlayBack::DEFAULT_PLAYBACK_PANEL_LEFT_OFFSET_ = 30;
const int MlabPlayBack::DEFAULT_PLAYBACK_PANEL_TOP_OFFSET_ = 100;
const int MlabPlayBack::DEFAULT_NUM_BUTTONS_ = 5;
const int MlabPlayBack::DEFAULT_LABEL_HEIGHT_ = 30;
const int MlabPlayBack::DEFAULT_BUTTON_HEIGHT_ = 40;
const int MlabPlayBack::DEFAULT_JOGSLIDER_HEIGHT_ = 50;
const int MlabPlayBack::PROCESS_THREADED_X_COMMAND_TIMER_MSEC_ = 10;
const string MlabPlayBack::EMPTY_STRING_ = "";
const string MlabPlayBack::PLAYBACK_PANEL_TITLE_ = "Logfile Playback Controller";
const string MlabPlayBack::LOG_ZEROTIME_KEY_ = "0.000";
const string MlabPlayBack::FSA_NUM_KEY_ = "([Name:Number]):";
const string MlabPlayBack::START_STATE_STRING_ = "Start";
const string MlabPlayBack::DEFAULT_STATE_STRING_ = "State";
const struct timespec MlabPlayBack::PLAYBACKLOOP_SLEEP_TIME_NSEC_ = {0, 2000000};

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------
pthread_mutex_t MlabPlayBack::threadedXCommandsMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MlabPlayBack::robotDataIndexMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MlabPlayBack::robotDataLoadMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MlabPlayBack::playbackStatusMutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t MlabPlayBack::playbackTimerMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------
MlabPlayBack::MlabPlayBack(
    XtAppContext app,
    Display *display,
    Widget parent,
    Widget drawingArea,
    double unitLength,
    int playbackMode)
{
    string overlayName;
    vector<string> logfileNames;

    MPBAppContext_ = app;
    MPBDisplay_ = display;
    MPBParent_ = parent;
    MPBDrawingArea_ = drawingArea;
    MPBDrawingUnitLength_ = unitLength;
    playbackMode_ = playbackMode;

    screenNumber_ = DefaultScreen(MPBDisplay_);
    displayWidth_ = DisplayWidth(MPBDisplay_, screenNumber_);
    displayHeight_ = DisplayHeight(MPBDisplay_, screenNumber_);

    XtVaGetValues(
        MPBParent_,
        XmNforeground, &MPBParentFg_,
        XmNbackground, &MPBParentBg_,
        NULL);

    // Initialize the variables.
    robotDataMax_ = 0;
    playbackPanelIsUp_ = false;
    shRobotDataLoaded_ = false;
    playbackThreadInstantiated_ = false;
    currentLogfile_ = NULL;
    shRobotDataIndex_ = 0;
    shPlaybackStatus_ = PAUSED;
    playbackOnPixmap_ = new Pixmap[PLAYBIACK_NUM_STATUS_];
    playbackOffPixmap_ = new Pixmap[PLAYBIACK_NUM_STATUS_];
    playbackISPixmap_ = new Pixmap[PLAYBIACK_NUM_STATUS_];
    buildBitmaps_(MPBParentFg_, MPBParentBg_, MPBParent_);
    playbackButton_w_ = new Widget[PLAYBIACK_NUM_STATUS_];
    shPlaybackTimer_.lastAbsTime = 0;
    shPlaybackTimer_.lastPlayTime = 0;
    mlabCBRClient_ = NULL;
    cbrClientInstantiated_ = false;
    oldLogfileStyle_ = false;
    shXCommandLoadNewMapList_.clear();
    shXCommandLoadLogfileList_.clear();
    shPendingThreadedXCommands_.clear();

    // Set the robot color.
    robotGC_ = gGCs.greenXOR;

    // Create the playback panel.
    createPlayBackPanel_();

    switch (playbackMode_) {

    case MPB_ENABLED:
        Pick_new_map();
        createLoadLogfileDialog_();
        break;

    case MPB_ENABLED_COMM_CBRSERVER:
        initCBRClient_();

        if (cbrClientInstantiated_)
        {
            // Separate process will be used for CBRClient. To
            // prevent conflicting with X, use XtAppAddTimeOut()
            // to handle the X related functions.
            XtToolkitThreadInitialize();
            startProcessThreadedXCommands_();

            // Load the overlay.
            overlayName = retrieveOverlay_();

            if (overlayName == EMPTY_STRING_)
            {
                pendXCommandPickNewMap_();
                
            }
            else
            {
                pendXCommandLoadNewMap_(overlayName);
            }

            // Load the logfile.
            logfileNames = retrieveLogfileList_();

            // Load the logfile immediately if there is only one robot.
            if (((int)(logfileNames.size()) == 0) ||
                ((int)(logfileNames.size()) > 1))
            {
                pendXCommandCreateLoadLogfileDialog_();
            }
            else if (logfileNames[0] == EMPTY_STRING_)
            {
                pendXCommandCreateLoadLogfileDialog_();
            }
            else
            {
                pendXCommandLoadLogfile_(logfileNames[0]);
            }

            // Creat the repair window.
            pendXCommandCreateRepairWindow_();
        }
        else
        {
            Pick_new_map();
            createLoadLogfileDialog_();
            gMMD->createRepairWindow();
        }

        break;

    default:
    case MPB_DISABLED:
        warn_userf("Playback mode disabled.");
        return;
    }

    // Start the thread.
    pthread_create(&playbackThread_, NULL, (void*(*)(void*))&startPlayBackThread_, this);
}

//-----------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------
MlabPlayBack::~MlabPlayBack(void)
{
    if (playbackThreadInstantiated_)
    {
        XtAppLock(MPBAppContext_);
        XLockDisplay(MPBDisplay_);

        pthread_cancel(playbackThread_);
        pthread_join(playbackThread_, NULL);
        playbackThreadInstantiated_ = false;

        XUnlockDisplay(MPBDisplay_);
        XtAppUnlock(MPBAppContext_);
    }

    if (currentLogfile_)
    {
        fclose(currentLogfile_);
        currentLogfile_ = NULL;
    }

    if (playbackButton_w_)
    {
        delete [] playbackButton_w_;
        playbackButton_w_ = NULL;
    }

    if (playbackOnPixmap_)
    {
        delete [] playbackOnPixmap_;
        playbackOnPixmap_ = NULL;
    }

    if (playbackOffPixmap_)
    {
        delete [] playbackOffPixmap_;
        playbackOffPixmap_ = NULL;
    }

    if (playbackISPixmap_)
    {
        delete [] playbackISPixmap_;
        playbackISPixmap_ = NULL;
    }

    if (mlabCBRClient_ != NULL)
    {
        delete mlabCBRClient_;
        mlabCBRClient_ = NULL;
    }

    robotDataList_.clear();
}

//-----------------------------------------------------------------------
// This function initializs the CBRClient class. 
//-----------------------------------------------------------------------
bool MlabPlayBack::initCBRClient_(void)
{
    bool initialized = false;

    mlabCBRClient_ = new MlabCBRClient(rc_table, CBRPLANNER_CLIENT_MLAB_MPB);

    initialized = mlabCBRClient_->initialize();

    if (!initialized)
    {
        warn_userf("CBRServer unreachable. Please make sure it is running.");
        return false;
    }

    cbrClientInstantiated_ = true;

    return true;
}

//-----------------------------------------------------------------------
// This function retrieves overlay info from CBRServer.
//-----------------------------------------------------------------------
string MlabPlayBack::retrieveOverlay_(void)
{
    CBRPlanner_OverlayInfo_t overlayInfo;

    overlayInfo = mlabCBRClient_->retrieveOverlayInfo();

    return (overlayInfo.name);
}

//-----------------------------------------------------------------------
// This function retrieves logfile info from CBRServer and load it.
//-----------------------------------------------------------------------
vector<string> MlabPlayBack::retrieveLogfileList_(void)
{
    vector<CBRPlanner_LogfileInfo_t> logfileInfoList;
    vector<string> logfileNames;
    int i;

    logfileInfoList = mlabCBRClient_->retrieveLogfileInfoList();

    for (i = 0; i < (int)(logfileInfoList.size()); i++)
    {
        logfileNames.push_back(logfileInfoList[i].name);
    }

    return logfileNames;
}

//-----------------------------------------------------------------------
// This function sends the current state info to the server.
//-----------------------------------------------------------------------
void MlabPlayBack::reportCurrentState_(vector<StateInfo_t> stateInfoList)
{
    CBRPlanner_StateInfo_t cbrStateInfo;
    vector<CBRPlanner_StateInfo_t> cbrStateInfoList;
    int i;

    for (i = 0; i < (int)(stateInfoList.size()); i++)
    {
        cbrStateInfo.fsaName = stateInfoList[i].fsaName;
        cbrStateInfo.stateName = stateInfoList[i].stateName;

        cbrStateInfoList.push_back(cbrStateInfo);
    }

    mlabCBRClient_->reportCurrentState(cbrStateInfoList);
}

//-----------------------------------------------------------------------
// Create the playback panel.
//-----------------------------------------------------------------------
void MlabPlayBack::createPlayBackPanel_(void)
{
    Widget mainFrame;
    Widget labelFrame, labelBox, label;
    Widget buttonsPanel, buttonFrame;
    Widget jogSliderFrame;
    vector<Widget> stackedWidget;
    char title[1024];
    int playbackPanelHeight, playbackPanelWidth;
    int posX, posY;
    int numButtons, numStackedWidget;
    int labelHeight, buttonHeight, buttonWidth, jogSliderHeight;
    int i, jogSliderValue, jogSliderMaxValue;

    playbackPanelHeight = DEFAULT_PLAYBACK_PANEL_HEIGHT_;
    playbackPanelWidth = DEFAULT_PLAYBACK_PANEL_WIDTH_;
    posX = DEFAULT_PLAYBACK_PANEL_LEFT_OFFSET_;
    posY = DEFAULT_PLAYBACK_PANEL_TOP_OFFSET_;
    numButtons = DEFAULT_NUM_BUTTONS_;
    labelHeight = DEFAULT_LABEL_HEIGHT_;
    buttonHeight = DEFAULT_BUTTON_HEIGHT_;
    buttonWidth = playbackPanelWidth/numButtons;
    jogSliderHeight = DEFAULT_JOGSLIDER_HEIGHT_;
    jogSliderValue = 0;
    jogSliderMaxValue = 1;

    // Popup shell.
    playbackPanel_w_ = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, MPBParent_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, playbackPanelHeight,
         XmNwidth, playbackPanelWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmRowColumnWidgetClass, playbackPanel_w_,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, playbackPanelHeight,
         XmNwidth, playbackPanelWidth,
         NULL);

    stackedWidget.push_back(mainFrame);

    // The frame for the label.
    labelFrame = XtVaCreateWidget
        ("labelFrame",
         xmFrameWidgetClass, mainFrame,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, labelHeight,
         XmNwidth, playbackPanelWidth,
         NULL);

    stackedWidget.push_back(labelFrame);

    // The widget for the main label.
    labelBox = XtVaCreateWidget
        ("labelBox",
         xmRowColumnWidgetClass, labelFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 1,
         XmNheight, labelHeight,
         XmNwidth, playbackPanelWidth,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label
    sprintf(title, PLAYBACK_PANEL_TITLE_.c_str());
    label = XtVaCreateWidget
        (title,
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, labelHeight,
         XmNwidth, playbackPanelWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    buttonsPanel = XtVaCreateWidget
        ("buttonsPanel",
         xmRowColumnWidgetClass, mainFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, numButtons,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, buttonHeight,
         XmNwidth, buttonWidth*numButtons,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelFrame,
         NULL);

    stackedWidget.push_back(buttonsPanel);

    // The "Rewind" buttons.
    posX = 0;
    posY = labelHeight;
    buttonFrame = createPlaybackButtons_(
        buttonsPanel,
        REWIND,
        posX,
        posY,
        buttonHeight,
        buttonWidth);
    stackedWidget.push_back(buttonFrame);
    stackedWidget.push_back(playbackButton_w_[REWIND]);

    // The "Stop" buttons.
    posX = playbackPanelWidth/numButtons;
    buttonFrame = createPlaybackButtons_(
        buttonsPanel,
        STOP,
        posX,
        posY,
        buttonHeight,
        buttonWidth);
    stackedWidget.push_back(buttonFrame);
    stackedWidget.push_back(playbackButton_w_[STOP]);

    // The "Play" buttons.
    posX = 2*playbackPanelWidth/numButtons;
    buttonFrame = createPlaybackButtons_(
        buttonsPanel,
        NORMAL_PLAY,
        posX,
        posY,
        buttonHeight,
        buttonWidth);
    stackedWidget.push_back(buttonFrame);
    stackedWidget.push_back(playbackButton_w_[NORMAL_PLAY]);

    // The "Pause" buttons.
    posX = 3*playbackPanelWidth/numButtons;
    buttonFrame = createPlaybackButtons_(
        buttonsPanel,
        PAUSED,
        posX,
        posY,
        buttonHeight,
        buttonWidth);
    stackedWidget.push_back(buttonFrame);
    stackedWidget.push_back(playbackButton_w_[PAUSED]);

    // The "Fast Forward" buttons.
    posX = 4*playbackPanelWidth/numButtons;
    buttonFrame = createPlaybackButtons_(
        buttonsPanel,
        FAST_FORWARD,
        posX,
        posY,
        buttonHeight,
        buttonWidth);
    stackedWidget.push_back(buttonFrame);
    stackedWidget.push_back(playbackButton_w_[FAST_FORWARD]);

    // The frame for the slider.
    jogSliderFrame = XtVaCreateWidget
        ("jogSliderFrame",
         xmFrameWidgetClass, mainFrame,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, jogSliderHeight,
         XmNwidth, playbackPanelWidth,
         NULL);

    stackedWidget.push_back(jogSliderFrame);

    // Slider
    jogSlider_w_ = XtVaCreateWidget
        ("",
         xmScaleWidgetClass, jogSliderFrame,
         XmNmaximum, jogSliderMaxValue,
         XmNminimum, 0,
         XmNvalue, jogSliderValue,
         XmNorientation, XmHORIZONTAL,
         XmNprocessingDirection, XmMAX_ON_RIGHT,
         XmNshowValue, false,
         XmNheight, jogSliderHeight,
         XmNwidth, playbackPanelWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, jogSliderFrame,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, jogSliderFrame,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, jogSliderFrame,
         NULL);
    XtAddCallback
        (jogSlider_w_,
         XmNvalueChangedCallback,
         (XtCallbackProc)cbChangeJogSlider_,
         (void *)this);
    XtAddCallback
        (jogSlider_w_,
         XmNdragCallback,
         (XtCallbackProc)cbChangeJogSlider_,
         (void *)this);

    stackedWidget.push_back(jogSlider_w_);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }

    XtPopup(playbackPanel_w_, XtGrabNone);

    // Turn on the color of the button for the currrent status. 
    XtVaSetValues(
        playbackButton_w_[shPlaybackStatus_],
        XmNlabelPixmap, playbackOnPixmap_[shPlaybackStatus_],
        NULL);

    playbackPanelIsUp_ = true;

    while (XtAppPending(MPBAppContext_))
    {
        XtAppProcessEvent(MPBAppContext_, XtIMAll);
    }
}

//-----------------------------------------------------------------------
// This function creates the buttons.
//-----------------------------------------------------------------------
Widget MlabPlayBack::createPlaybackButtons_(
    Widget buttonsPanel,
    PlayBackStatus_t status,
    int posX,
    int posY,
    int height,
    int width)
{
    Widget buttonFrame;
    PlayBackButtonCallbackData_t *callbackData;

    // The frame for the button.
    buttonFrame = XtVaCreateWidget
        ("buttonFrame",
         xmFrameWidgetClass, buttonsPanel,
         XmNx, posX,
         XmNy, posY,
         XmNheight, height,
         XmNwidth, width,
         NULL);

    // The "Fast Forward" button
    playbackButton_w_[status] = XtVaCreateWidget
        ("playbackButton_w_",
         xmPushButtonGadgetClass, buttonFrame,
         XmNlabelType, XmPIXMAP,
         XmNlabelPixmap, playbackOffPixmap_[status],
         XmNlabelInsensitivePixmap, playbackISPixmap_[status],
         XmNbackground, MPBParentBg_,
         XmNhighlightColor, MPBParentBg_,
         XmNborderColor, MPBParentBg_,
         XmNarmColor, MPBParentBg_,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, height,
         XmNwidth, width,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);
    callbackData = new PlayBackButtonCallbackData_t;
    callbackData->mlabPlayBackInstance = this;
    callbackData->status = status;
    XtAddCallback(
        playbackButton_w_[status],
        XmNactivateCallback,
        (XtCallbackProc)cbPlaybackButtonPushed_,
        callbackData);

    return buttonFrame;
}
//-----------------------------------------------------------------------
// This function creates the bitmaps.
//-----------------------------------------------------------------------
void MlabPlayBack::buildBitmaps_(Pixel fg, Pixel bg, Widget parent)
{
    playbackOnPixmap_[NORMAL_PLAY] = MPB_CREATE_PIXMAP(playBtn, gColorPixel.green4, bg, parent);
    playbackOffPixmap_[NORMAL_PLAY] = MPB_CREATE_PIXMAP(playBtn, gColorPixel.black, bg, parent);
    playbackISPixmap_[NORMAL_PLAY] = MPB_CREATE_PIXMAP(playBtn, gColorPixel.black, bg, parent);

    playbackOnPixmap_[PAUSED] = MPB_CREATE_PIXMAP(pauseBtn, gColorPixel.green4, bg, parent);
    playbackOffPixmap_[PAUSED] = MPB_CREATE_PIXMAP(pauseBtn, gColorPixel.black, bg, parent);
    playbackISPixmap_[PAUSED] = MPB_CREATE_PIXMAP(pauseBtn, gColorPixel.black, bg, parent);

    playbackOnPixmap_[FAST_FORWARD] = MPB_CREATE_PIXMAP(ffBtn, gColorPixel.green4, bg, parent);
    playbackOffPixmap_[FAST_FORWARD] = MPB_CREATE_PIXMAP(ffBtn, gColorPixel.black, bg, parent);
    playbackISPixmap_[FAST_FORWARD] = MPB_CREATE_PIXMAP(ffBtn, gColorPixel.black, bg, parent);

    playbackOnPixmap_[REWIND] = MPB_CREATE_PIXMAP(rewBtn, gColorPixel.green4, bg, parent);
    playbackOffPixmap_[REWIND] = MPB_CREATE_PIXMAP(rewBtn, gColorPixel.black, bg, parent);
    playbackISPixmap_[REWIND] = MPB_CREATE_PIXMAP(rewBtn, gColorPixel.black, bg, parent);

    playbackOnPixmap_[STOP] = MPB_CREATE_PIXMAP(stopBtn, gColorPixel.green4, bg, parent);
    playbackOffPixmap_[STOP] = MPB_CREATE_PIXMAP(stopBtn, gColorPixel.black, bg, parent);
    playbackISPixmap_[STOP] = MPB_CREATE_PIXMAP(stopBtn, gColorPixel.black, bg, parent);
}

//-----------------------------------------------------------------------
// This function creates the dialog for the user to load the logfile.
//-----------------------------------------------------------------------
void MlabPlayBack::createLoadLogfileDialog_(void)
{
    Arg wargs[3];
    int n = 0;

    XmString title = MPB_XSTRING("Select Logfile");
    XmString filterstr = MPB_XSTRING("*.log"); 

    XtSetArg(wargs[n], XmNdialogTitle, title); n++;
    XtSetArg(wargs[n], XmNpattern, filterstr); n++;

    loadLogfileDialog_w_ = XmCreateFileSelectionDialog
        (MPBParent_,
         "select-file",
         wargs,
         n);

    XmStringFree(title);
    XmStringFree(filterstr);

    XtUnmanageChild(XmFileSelectionBoxGetChild(loadLogfileDialog_w_,XmDIALOG_HELP_BUTTON));

    XtAddCallback(
        loadLogfileDialog_w_,
        XmNokCallback, 
        (XtCallbackProc)cbLoadLogfileDialogOK_,
        this);

    XtAddCallback(
        loadLogfileDialog_w_,
        XmNcancelCallback, 
        (XtCallbackProc)cbLoadLogfileDialogCancel_,
        NULL);
    
    XtManageChild(loadLogfileDialog_w_);
}

//-----------------------------------------------------------------------
// This is the main loop for thie playback process.
//-----------------------------------------------------------------------
void MlabPlayBack::playbackLoop_(void)
{
    playbackThreadInstantiated_ = true;

    while(true)
    {
        displayRobot_();
        pthread_testcancel();
        //usleep(DEFAULT_PLAYBACKLOOP_USLEEP_);
        nanosleep(&PLAYBACKLOOP_SLEEP_TIME_NSEC_, NULL);
        pthread_testcancel();
    }
}

//-----------------------------------------------------------------------
// This function updates the robot on the screen.
//-----------------------------------------------------------------------
void MlabPlayBack::displayRobot_(void)
{
    StateInfo_t stateInfo;
    timeval tv;
    gt_Point position;
    char msg[1024];
    double theta;
    double absTime, playTime, logTime;
    double lastAbsTime = 0, lastPlayTime = 0;
    int i, robotDataIndex;
    int playbackStatus;
    bool dataReady = false, pausePlayback = false, updateJogSlider = true;
    static vector<StateInfo_t> currentStateInfoList;
    static bool isFirstTime = true;

    // Make sure the log data is loaded.
    pthread_mutex_lock(&robotDataLoadMutex_);
    dataReady = shRobotDataLoaded_;
    pthread_mutex_unlock(&robotDataLoadMutex_);

    if (!dataReady)
    {
        return;
    }

    // Copy the log data.
    pthread_mutex_lock(&robotDataIndexMutex_);
    robotDataIndex = shRobotDataIndex_;
    pthread_mutex_unlock(&robotDataIndexMutex_);

    // Copy the playback status info.
    playbackStatus = getPlaybackStatus_();

    // Copy the playback timer info.
    pthread_mutex_lock(&playbackTimerMutex_);
    lastAbsTime = shPlaybackTimer_.lastAbsTime;
    lastPlayTime = shPlaybackTimer_.lastPlayTime;
    pthread_mutex_unlock(&playbackTimerMutex_);

    // Check the current time.
    gettimeofday(&tv, NULL);
    absTime = (double)tv.tv_sec + (((double)tv.tv_usec)/1000000);

    if (isFirstTime)
    {
        lastAbsTime = absTime;
        isFirstTime = false;
    }

    switch (playbackStatus) {

    case STOP:
        robotDataIndex = 0;
        logTime = robotDataList_[robotDataIndex].time;
        lastPlayTime = logTime;
        break;

    case NORMAL_PLAY:
        playTime = (absTime - lastAbsTime) + lastPlayTime;
        if (robotDataIndex < (robotDataMax_-1))
        {
            logTime = robotDataList_[robotDataIndex+1].time;

            if (logTime < playTime)
            {
                robotDataIndex++;
            }
        }
        else
        {
            // At the end of the log file. Pause it.
            pausePlayback = true;;
            updateJogSlider = false;
        }
        lastPlayTime = playTime;
        break;

    case REWIND:
        if (robotDataIndex > 0)
        {
            robotDataIndex--;
        }
        else
        {
            // At the beginning of the log file. Pause it.
            pausePlayback = true;;
            updateJogSlider = false;
        }
        logTime = robotDataList_[robotDataIndex].time;
        lastPlayTime = logTime;
        break;

    case FAST_FORWARD:
        if (robotDataIndex < (robotDataMax_-1))
        {
            robotDataIndex++;
        }
        else
        {
            // At the end of the log file. Pause it.
            pausePlayback = true;;
            updateJogSlider = false;
        }
        logTime = robotDataList_[robotDataIndex].time;
        lastPlayTime = logTime;
        break;

    default:
    case PAUSED:
        updateJogSlider = false;
        break;
    }

    // Remember the index for the next cycle.
    pthread_mutex_lock(&robotDataIndexMutex_);
    shRobotDataIndex_ = robotDataIndex;
    pthread_mutex_unlock(&robotDataIndexMutex_);

    // Remember the time for the next cycle.
    pthread_mutex_lock(&playbackTimerMutex_);
    shPlaybackTimer_.lastAbsTime = absTime;
    shPlaybackTimer_.lastPlayTime = lastPlayTime;
    pthread_mutex_unlock(&playbackTimerMutex_);

    // Display the robot.
    position.x = robotDataList_[robotDataIndex].posX;
    position.y = robotDataList_[robotDataIndex].posY;
    theta = robotDataList_[robotDataIndex].heading;

    XtAppLock(MPBAppContext_);
    XLockDisplay(MPBDisplay_);
    sim_robot_location(
        robotDataList_[robotDataIndex].id,
        position,
        theta,
        (GC *)&robotGC_);
    XUnlockDisplay(MPBDisplay_);
    XtAppUnlock(MPBAppContext_);

    if (!isSameStateInfoList_(
            currentStateInfoList,
            robotDataList_[robotDataIndex].stateInfoList))
    {
        currentStateInfoList = robotDataList_[robotDataIndex].stateInfoList;

        if (cbrClientInstantiated_)
        {
            pthread_testcancel();
            reportCurrentState_(currentStateInfoList);
            pthread_testcancel();
        }
        else
        {
            for (i = 0; i < (int)(currentStateInfoList.size()); i++)
            {
                sprintf(
                    msg,
                    "robot(%d) current state = %s\n",
                    robotDataList_[robotDataIndex].id,
                    currentStateInfoList[i].stateName.c_str());

                report_user(msg);
            }
        }
    }

    // Update the slider value
    if (updateJogSlider)
    {
        XtVaSetValues(
            jogSlider_w_,
            XmNvalue, robotDataIndex,
            NULL);
    }

    // Pause it if necessary.
    if (pausePlayback)
    {
        changePlaybackStatus_(PAUSED);
    }
}

//-----------------------------------------------------------------------
// This function returns the current playback status.
//-----------------------------------------------------------------------
int MlabPlayBack::getPlaybackStatus_(void)
{
    int playbackStatus;

    pthread_mutex_lock(&playbackStatusMutex_);
    playbackStatus = shPlaybackStatus_;
    pthread_mutex_unlock(&playbackStatusMutex_);

    return playbackStatus;
}

//-----------------------------------------------------------------------
// This function loads the log file specified by the user.
//-----------------------------------------------------------------------
void MlabPlayBack::loadLogfile_(string logfileName)
{
    FILE *logfile = NULL;
    string bufString;
    int status;
    char buf[1024];

    fsaInfoList_.clear();

    logfile = fopen(logfileName.c_str(), "r");

    if (logfile == NULL)
    {
        warn_user("Specified file cannot be opened.");
        return;
    }
    
    while(true)
    {
        status = fscanf(logfile, "%s", buf);

        if (status == EOF)
        {
            break;
        }

        bufString = buf;

        if (bufString == FSA_NUM_KEY_)
        {
            status = fscanf(logfile, "%s", buf);
            oldLogfileStyle_ = false;

            if (status != EOF)
            {
                bufString = buf;
                fsaInfoList_ = getFSAInfoListFromString_(bufString);

                if ((int)(fsaInfoList_.size()) == 0)
                {
                    warn_userf("Logfile could not be processed.");
                    fclose(logfile);
                    return;
                }
    
                break;
            }

            // It is already at the end of the file.
            warn_user("Logfile is empty.");
            fclose(logfile);
            return;
        }
        else if (bufString == LOG_ZEROTIME_KEY_)
        {
            oldLogfileStyle_ = true;
            break;
        }
    }

    if (!oldLogfileStyle_)
    {
        // Find the zero time.
        while(true)
        {
            status = fscanf(logfile, "%s", buf);

            if (status == EOF)
            {
                // It is already at the end of the file.
                warn_user("Logfile is empty.");
                fclose(logfile);
                return;
            }

            bufString = buf;

            if (bufString == LOG_ZEROTIME_KEY_)
            {
                break;
            }
        }
    }

    currentLogfileName_ = logfileName;

    if (currentLogfile_)
    {
        fclose(currentLogfile_);
    }

    currentLogfile_ = logfile;
    loadRobotData_();
}

//-----------------------------------------------------------------------
// This function loads the log file specified by the user.
//-----------------------------------------------------------------------
void MlabPlayBack::loadSelectedLogfile_(XmString value)
{
    char *logfileName = NULL;
    string bufString, logfileNameString;

    XmStringGetLtoR(value, XmSTRING_DEFAULT_CHARSET , &logfileName);
    logfileNameString = logfileName;

    loadLogfile_(logfileNameString);

    XtUnmanageChild(loadLogfileDialog_w_);
    XtFree(logfileName);
}

//-----------------------------------------------------------------------
// This function decodes state info from the string in the log data.
// The format in the log data is (FSA#,State#)(FSA#,State#)...
//-----------------------------------------------------------------------
vector<MlabPlayBack::StateInfo_t> MlabPlayBack::getStateInfoListFromString_(
    string stateInfoString)
{
    StateInfo_t stateInfo;
    vector<StateInfo_t> stateInfoList;
    string bufString, valueString;
    string::size_type index1, index2, index3;
    int fsaNum, stateNum;

    if (oldLogfileStyle_)
    {
        stateInfo.fsaName = EMPTY_STRING_;

        if (isdigit(stateInfoString.c_str()[0]))
        {
            stateNum = atoi(stateInfoString.c_str());
            stateInfo.stateName = stateNum2Name_(stateNum);
        }
        else
        {
            // Unsupported logfile format
            stateInfo.stateName = EMPTY_STRING_;
        }

        stateInfoList.push_back(stateInfo);
    }
    else
    {
        bufString = stateInfoString;

        while (true)
        {
            index1 = bufString.find("(");
            index2 = bufString.find(",");
            index3 = bufString.find(")");

            if ((index1 != string::npos) &&
                (index2 != string::npos) &&
                (index3 != string::npos))
            {
                index1++;
                valueString = bufString.substr(index1, index2-index1);
                fsaNum = atoi(valueString.c_str());
                stateInfo.fsaName = fsaNum2Name_(fsaNum);

                index2++;
                valueString = bufString.substr(index2, index3-index2);
                stateNum = atoi(valueString.c_str());
                stateInfo.stateName = stateNum2Name_(stateNum);

                index3++;
                bufString = bufString.substr(index3);

                stateInfoList.push_back(stateInfo);
            }
            else
            {
                break;
            }
        }
    }

    return stateInfoList;
}

//-----------------------------------------------------------------------
// This function decodes FSA info from the string in the log data.
// The format in the log data is [Name:Number][Name:Number]...
//-----------------------------------------------------------------------
vector<MlabPlayBack::FSAInfo_t> MlabPlayBack::getFSAInfoListFromString_(
    string fsaInfoString)
{
    FSAInfo_t fsaInfo;
    vector<FSAInfo_t> fsaInfoList;
    string bufString, valueString;
    string::size_type index1, index2, index3;

    bufString = fsaInfoString;

    while (true)
    {
        index1 = bufString.find("[");
        index2 = bufString.find(":");
        index3 = bufString.find("]");

        if ((index1 != string::npos) &&
            (index2 != string::npos) &&
            (index3 != string::npos))
        {
            index1++;
            valueString = bufString.substr(index1, index2-index1);
            fsaInfo.name = valueString;

            index2++;
            valueString = bufString.substr(index2, index3-index2);
            fsaInfo.number = atoi(valueString.c_str());

            index3++;
            bufString = bufString.substr(index3);

            fsaInfoList.push_back(fsaInfo);
        }
        else
        {
            break;
        }
    }

    return fsaInfoList;
}

//-----------------------------------------------------------------------
// This function checks to see if two state info lists are same.
//-----------------------------------------------------------------------
bool MlabPlayBack::isSameStateInfoList_(
    vector<StateInfo_t> stateInfoListA,
    vector<StateInfo_t> stateInfoListB)
{
    int i;

    // Check ths size.
    if (stateInfoListA.size() != stateInfoListB.size())
    {
        return false;
    }

    // Check the contents.
    for (i = 0; i < (int)(stateInfoListA.size()); i++)
    {
        if (stateInfoListA[i].fsaName != stateInfoListB[i].fsaName)
        {
            return false;
        }

        if (stateInfoListA[i].stateName != stateInfoListB[i].stateName)
        {
            return false;
        }
    }

    return true;
}

//-----------------------------------------------------------------------
// This function loads the robot data.
//-----------------------------------------------------------------------
void MlabPlayBack::loadRobotData_(void)
{
    PlayBackRobotData_t robotData;
    int status;
    float valTime, valPosX, valPosY, valHeading, valVelX, valVelY, valId;
    char valStateInfo[5120];
    string stateInfoString;

    // Get the first line
    status = fscanf(
        currentLogfile_,
        "%f %f %f %f %f %f %s",
        &valPosX,
        &valPosY,
        &valHeading,
        &valVelX,
        &valVelY,
        &valId,
        valStateInfo);
    stateInfoString = valStateInfo;
    valTime = 0.0;
    robotDataMax_ = 0;

    while (true)
    {
        robotData.time = valTime;
        robotData.posX = valPosX;
        robotData.posY = valPosY;
        robotData.heading = valHeading;
        robotData.velX = valVelX;
        robotData.velY = valVelY;
        robotData.id = (int)valId;
        robotData.stateInfoList = getStateInfoListFromString_(stateInfoString);
        robotDataList_.push_back(robotData);

        status = fscanf(
            currentLogfile_,
            "%f %f %f %f %f %f %f %s",
            &valTime,
            &valPosX,
            &valPosY,
            &valHeading,
            &valVelX,
            &valVelY,
            &valId,
            valStateInfo);
        stateInfoString = valStateInfo;

        if (status != EOF)
        {
            robotDataMax_++;
        }
        else
        {
            break;
        }
    }

    gt_Point pt;
    pt.x = robotDataList_[0].posX;
    pt.y = robotDataList_[0].posY;
    start_robot(
        NULL,
        NULL,
        robotDataList_[0].id,
        false,
        false,
        pt.x,
        pt.y,
        "blue");

    // Misc robot display setups.
    set_show_trails(false);
    set_scale_robots(false);
    set_robot_length(ROBOT2DISPLAY_RATIO_*(double)displayWidth_);

    sim_robot_location(
        robotDataList_[0].id,
        pt,
        robotDataList_[0].heading,
        (GC *)&robotGC_);

    // Update the jog slider bar.
    if (robotDataMax_ > 0)
    {
        XtVaSetValues(
            jogSlider_w_,
            XmNmaximum, robotDataMax_ - 1,
            NULL); 
    }

    pthread_mutex_lock(&robotDataLoadMutex_);
    shRobotDataLoaded_ = true;
    pthread_mutex_unlock(&robotDataLoadMutex_);
}
    
//-----------------------------------------------------------------------
// This function changes the status of the player based on the button
// press.
//-----------------------------------------------------------------------
void MlabPlayBack::changePlaybackStatus_(int newStatus)
{
    int oldStatus;

    pthread_mutex_lock(&playbackStatusMutex_);

    oldStatus = shPlaybackStatus_;

    XtVaSetValues(
        playbackButton_w_[oldStatus],
        XmNlabelPixmap, playbackOffPixmap_[oldStatus],
        NULL);

    shPlaybackStatus_ = newStatus;

    XtVaSetValues(
        playbackButton_w_[newStatus],
        XmNlabelPixmap, playbackOnPixmap_[newStatus],
        NULL);

    pthread_mutex_unlock(&playbackStatusMutex_);
}

//-----------------------------------------------------------------------
// This function reads the jog slider value.
//-----------------------------------------------------------------------
int MlabPlayBack::readJogSliderValue_(void)
{
    int newSliderValue;

    XtVaGetValues(
        jogSlider_w_,
        XmNvalue, &newSliderValue,
        NULL);

    return newSliderValue;
}

//-----------------------------------------------------------------------
// This function updates the robot data index.
//-----------------------------------------------------------------------
void MlabPlayBack::updateCurrentRobotDataIndex_(int newIndex)
{
    int currentStatus;

    // Update the index.
    pthread_mutex_lock(&robotDataIndexMutex_);
    shRobotDataIndex_ = newIndex;
    pthread_mutex_unlock(&robotDataIndexMutex_);

    // Update the playback timer info.
    pthread_mutex_lock(&playbackTimerMutex_);
    shPlaybackTimer_.lastPlayTime = robotDataList_[newIndex].time;;
    pthread_mutex_unlock(&playbackTimerMutex_);

    // If the current status is "Stop", change it to "Pause".
    currentStatus = getPlaybackStatus_();

    if (currentStatus == STOP)
    {
        changePlaybackStatus_(PAUSED);
    }
}

//-----------------------------------------------------------------------
// This callback function loads the log file specified by the user.
//-----------------------------------------------------------------------
void MlabPlayBack::cbLoadLogfileDialogOK_(
    Widget w,
    XtPointer clientData,
    XmFileSelectionBoxCallbackStruct *callData)
{
    MlabPlayBack *mlabPlayBackInstance = NULL;

    mlabPlayBackInstance = (MlabPlayBack *)clientData;
    mlabPlayBackInstance->loadSelectedLogfile_(callData->value);
}

//-----------------------------------------------------------------------
// This callback function just closes the Logfile Dialog.
//-----------------------------------------------------------------------
void MlabPlayBack::cbLoadLogfileDialogCancel_(
    Widget w,
    XtPointer clientData,
    XmFileSelectionBoxCallbackStruct *callData)
{
    XtUnmanageChild(w);
}

//-----------------------------------------------------------------------
// This callback function is called when one of the buttons was pressed.
//-----------------------------------------------------------------------
void MlabPlayBack::cbPlaybackButtonPushed_(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    MlabPlayBack *mlabPlayBackInstance;
    int newStatus;

    mlabPlayBackInstance = ((PlayBackButtonCallbackData_t *)clientData)->mlabPlayBackInstance;
    newStatus = ((PlayBackButtonCallbackData_t *)clientData)->status;

    mlabPlayBackInstance->changePlaybackStatus_(newStatus);
}

//-----------------------------------------------------------------------
// This callback function is called when the slider value is changed by
// the user.
//-----------------------------------------------------------------------
void MlabPlayBack::cbChangeJogSlider_(
    Widget w,
    XtPointer clientData,
    XmFileSelectionBoxCallbackStruct *callData)
{
    MlabPlayBack *mlabPlayBackInstance = NULL;
    int newSliderValue;

    mlabPlayBackInstance = (MlabPlayBack *)clientData;
    newSliderValue = mlabPlayBackInstance->readJogSliderValue_();
    mlabPlayBackInstance->updateCurrentRobotDataIndex_(newSliderValue);
}

//-----------------------------------------------------------------------
// This function starts the playback thread.
//-----------------------------------------------------------------------
void MlabPlayBack::startPlayBackThread_(void *MlabPlayBackInstance)
{
    ((MlabPlayBack *)MlabPlayBackInstance)->playbackLoop_();
}

//-----------------------------------------------------------------------
// This function translates the FSA number to its name.
//-----------------------------------------------------------------------
string MlabPlayBack::fsaNum2Name_(int number)
{
    string name;
    int i;

    for (i = 0; i < (int)(fsaInfoList_.size()); i++)
    {
        if (fsaInfoList_[i].number == number)
        {
            name = fsaInfoList_[i].name;
            break;
        }
    }

    return name;
}

//-----------------------------------------------------------------------
// This function translates the state number to its name.
//-----------------------------------------------------------------------
string MlabPlayBack::stateNum2Name_(int number)
{
    string name;
    char numBuf[1024];

    if (number == 0)
    {
        name = START_STATE_STRING_;
    }
    else
    {
        name = DEFAULT_STATE_STRING_;
        sprintf(numBuf, "%d", number);
        name += numBuf;
    }

    return name;
}

//-----------------------------------------------------------------------
// This function gets the current state info.
//-----------------------------------------------------------------------
vector<MlabPlayBack::StateInfo_t> MlabPlayBack::getCurrentStateInfoList(void)
{
    vector<StateInfo_t> stateInfoList;
    int currentIndex;

    // Get the current index.
    pthread_mutex_lock(&robotDataIndexMutex_);
    currentIndex = shRobotDataIndex_;
    pthread_mutex_unlock(&robotDataIndexMutex_);

    stateInfoList = robotDataList_[currentIndex].stateInfoList;

    return stateInfoList;
}

//-----------------------------------------------------------------------
// This function pends gMMD->createRepairWindow in the queue to be
// processed when X is available.
//-----------------------------------------------------------------------
void MlabPlayBack::pendXCommandCreateRepairWindow_(void)
{
    PendingThreadedXCommands_t cmd;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_CREATE_REPAIR_WINDOW;
    cmd.index = 0; // Index not used.
    shPendingThreadedXCommands_.push_back(cmd);

    pthread_cleanup_pop(1);
    //pthread_mutex_unlock(&threadedXCommandsMutex_);
}

//-----------------------------------------------------------------------
// This function pends load_new_map() in the queue to be processed when
// X is available.
//-----------------------------------------------------------------------
void MlabPlayBack::pendXCommandLoadNewMap_(string overlayName)
{
    PendingThreadedXCommands_t cmd;
    XCommandLoadNewMap_t data;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_LOAD_NEW_MAP;
    cmd.index = shXCommandLoadNewMapList_.size();
    shPendingThreadedXCommands_.push_back(cmd);

    data.overlayName = overlayName;
    shXCommandLoadNewMapList_.push_back(data);

	pthread_cleanup_pop(1);
    //pthread_mutex_unlock(&threadedXCommandsMutex_);
}

//-----------------------------------------------------------------------
// This function pends PickNewMap() in the queue to be processed when X
// is available.
//-----------------------------------------------------------------------
void MlabPlayBack::pendXCommandPickNewMap_(void)
{
    PendingThreadedXCommands_t cmd;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_PICK_NEW_MAP;
    cmd.index = 0; // Index not used.
    shPendingThreadedXCommands_.push_back(cmd);

	pthread_cleanup_pop(1);
    //pthread_mutex_unlock(&threadedXCommandsMutex_);
}

//-----------------------------------------------------------------------
// This function pends createLoadLogfileDialog_() in the queue to be
// processed when X is available.
//-----------------------------------------------------------------------
void MlabPlayBack::pendXCommandCreateLoadLogfileDialog_(void)
{
    PendingThreadedXCommands_t cmd;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_CREATE_LOAD_LOGFILE_DIALOG;
    cmd.index = 0; // Index not used.
    shPendingThreadedXCommands_.push_back(cmd);

	pthread_cleanup_pop(1);
    //pthread_mutex_unlock(&threadedXCommandsMutex_);
}

//-----------------------------------------------------------------------
// This function pends loadLogfile_() in the queue to be processed when
// X is available.
//-----------------------------------------------------------------------
void MlabPlayBack::pendXCommandLoadLogfile_(string logfileName)
{
    PendingThreadedXCommands_t cmd;
    XCommandLoadLogfile_t data;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_LOAD_LOGFILE;
    cmd.index = shXCommandLoadLogfileList_.size();
    shPendingThreadedXCommands_.push_back(cmd);

    data.logfileName = logfileName;
    shXCommandLoadLogfileList_.push_back(data);

	pthread_cleanup_pop(1);
    //pthread_mutex_unlock(&threadedXCommandsMutex_);
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending due to the
// multithreading.
//-----------------------------------------------------------------------
void MlabPlayBack::startProcessThreadedXCommands_(void)
{
    XtAppAddTimeOut(
        MPBAppContext_,
        PROCESS_THREADED_X_COMMAND_TIMER_MSEC_,
        (XtTimerCallbackProc)cbProcessThreadedXCommands_,
        this);
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending due to the
// multithreading.
//-----------------------------------------------------------------------
void MlabPlayBack::processThreadedXCommands_(void)
{
    PendingThreadedXCommands_t cmd;
    string overlayName, logfileName;
    int i, numCommands;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);
    XtAppLock(MPBAppContext_);
    XLockDisplay(MPBDisplay_);

    numCommands = shPendingThreadedXCommands_.size();

    for (i = 0; i < numCommands; i++)
    {
        cmd = shPendingThreadedXCommands_[i];

        switch (cmd.type) {

        case X_COMMAND_CREATE_REPAIR_WINDOW:
            gMMD->createRepairWindow();
            break;

        case X_COMMAND_LOAD_NEW_MAP:
            overlayName = shXCommandLoadNewMapList_[cmd.index].overlayName;
            load_new_map((char *)(overlayName.c_str()));
            break;

        case X_COMMAND_PICK_NEW_MAP:
            Pick_new_map();
            break;

        case X_COMMAND_CREATE_LOAD_LOGFILE_DIALOG:
            createLoadLogfileDialog_();
            break;

        case X_COMMAND_LOAD_LOGFILE:
            logfileName = shXCommandLoadLogfileList_[cmd.index].logfileName;
            loadLogfile_(logfileName);
            break;
        }
    }

    shPendingThreadedXCommands_.clear();
    shXCommandLoadNewMapList_.clear();
    shXCommandLoadLogfileList_.clear();

    XUnlockDisplay(MPBDisplay_);
    XtAppUnlock(MPBAppContext_);
	pthread_cleanup_pop(1);
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending due to the
// multithreading.
//-----------------------------------------------------------------------
XtTimerCallbackProc MlabPlayBack::cbProcessThreadedXCommands_(XtPointer clientData)
{
    MlabPlayBack *mlabPlayBackInstance = NULL;

    mlabPlayBackInstance = (MlabPlayBack *)clientData;

    mlabPlayBackInstance->processThreadedXCommands_();
    mlabPlayBackInstance->startProcessThreadedXCommands_();

    return false;
}

/**********************************************************************
 * $Log: gt_playback.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/06/28 14:00:26  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/06/19 20:35:46  endo
 * Initial revision
 *
 **********************************************************************/
