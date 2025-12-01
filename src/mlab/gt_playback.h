/**********************************************************************
 **                                                                  **
 **                           gt_playback.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by: Yoichiro Endo                                       **
 **                                                                  **
 **  Copyright 2003 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_playback.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_PLAYBACK_H
#define GT_PLAYBACK_H

#include <pthread.h>
#include <vector>
#include <string>

#include <Xm/Xm.h>

#include "mlab_cbrclient.h"

using std::vector;
using std::string;

class MlabCBRClient;

class MlabPlayBack {

public:
    typedef enum MlabPlayBackMode_t
    {
        MPB_DISABLED,
        MPB_ENABLED,
        MPB_ENABLED_COMM_CBRSERVER
    };

    typedef enum {
        PAUSED,
        NORMAL_PLAY,
        REWIND,
        FAST_FORWARD,
        STOP
    } PlayBackStatus_t;

    // Make sure this structure matches with CBRPlanner_StateInfo_t
    // in cbrplanner_protocol.h.
    typedef struct StateInfo_t
    {
        string fsaName;
        string stateName;
    };

protected:
    typedef struct FSAInfo_t {
        int number;
        string name;
    };

    typedef struct {
        double time;
        double posX;
        double posY;
        double heading;
        double velX;
        double velY;
        int id;
        vector<StateInfo_t> stateInfoList;
    } PlayBackRobotData_t;

    typedef struct {
        MlabPlayBack *mlabPlayBackInstance;
        int status;
    } PlayBackButtonCallbackData_t;

    typedef struct {
        double lastAbsTime;
        double lastPlayTime;
    } PlayBackTimer_t;

    typedef enum ThreadedXCommandsTypes_t {
        X_COMMAND_CREATE_REPAIR_WINDOW,
        X_COMMAND_LOAD_NEW_MAP,
        X_COMMAND_PICK_NEW_MAP,
        X_COMMAND_CREATE_LOAD_LOGFILE_DIALOG,
        X_COMMAND_LOAD_LOGFILE
    };

    typedef struct XCommandLoadNewMap_t {
        string overlayName;
    };

    typedef struct XCommandLoadLogfile_t {
        string logfileName;
    };

    typedef struct PendingThreadedXCommands_t {
        int type;
        int index;
    };

protected:
    XtAppContext MPBAppContext_;
    Widget MPBParent_;
    Widget MPBDrawingArea_;
    Widget playbackPanel_w_;
    Widget *playbackButton_w_;
    Widget loadLogfileDialog_w_;
    Widget jogSlider_w_;
    Display *MPBDisplay_;
    Pixel MPBParentFg_;
    Pixel MPBParentBg_;
    Pixmap *playbackOnPixmap_;
    Pixmap *playbackOffPixmap_;
    Pixmap *playbackISPixmap_;
    GC robotGC_;
    FILE *currentLogfile_;
    vector<PlayBackRobotData_t> robotDataList_;
    vector<FSAInfo_t> fsaInfoList_;
    vector<XCommandLoadNewMap_t> shXCommandLoadNewMapList_;
    vector<XCommandLoadLogfile_t> shXCommandLoadLogfileList_;
    vector<PendingThreadedXCommands_t> shPendingThreadedXCommands_;
    pthread_t playbackThread_;
    PlayBackTimer_t shPlaybackTimer_;
    MlabCBRClient *mlabCBRClient_;
    string currentLogfileName_;
    double MPBDrawingUnitLength_;
    int robotDataMax_;
    int shRobotDataIndex_;
    int shPlaybackStatus_;
    int playbackMode_;
    int screenNumber_;
    int displayWidth_;
    int displayHeight_;
    bool playbackPanelIsUp_;
    bool shRobotDataLoaded_;
    bool playbackThreadInstantiated_;
    bool cbrClientInstantiated_;
    bool oldLogfileStyle_;

    static pthread_mutex_t threadedXCommandsMutex_;
    static pthread_mutex_t robotDataIndexMutex_;
    static pthread_mutex_t robotDataLoadMutex_;
    static pthread_mutex_t playbackStatusMutex_;
    static pthread_mutex_t playbackTimerMutex_;

    static const struct timespec PLAYBACKLOOP_SLEEP_TIME_NSEC_;
    static const double ROBOT2DISPLAY_RATIO_;
    static const int PLAYBIACK_NUM_STATUS_;
    static const int DEFAULT_PLAYBACK_PANEL_HEIGHT_;
    static const int DEFAULT_PLAYBACK_PANEL_WIDTH_;
    static const int DEFAULT_PLAYBACK_PANEL_LEFT_OFFSET_;
    static const int DEFAULT_PLAYBACK_PANEL_TOP_OFFSET_;
    static const int DEFAULT_NUM_BUTTONS_;
    static const int DEFAULT_LABEL_HEIGHT_;
    static const int DEFAULT_BUTTON_HEIGHT_;
    static const int DEFAULT_JOGSLIDER_HEIGHT_;
    static const int PROCESS_THREADED_X_COMMAND_TIMER_MSEC_;
    static const string EMPTY_STRING_;
    static const string PLAYBACK_PANEL_TITLE_;
    static const string LOG_ZEROTIME_KEY_;
    static const string FSA_NUM_KEY_;
    static const string START_STATE_STRING_;
    static const string DEFAULT_STATE_STRING_;

    Widget createPlaybackButtons_(
        Widget buttonsPanel,
        PlayBackStatus_t status,
        int posX,
        int posY,
        int height,
        int width);
    vector<StateInfo_t> getStateInfoListFromString_(string stateInfoString);
    vector<FSAInfo_t> getFSAInfoListFromString_(string fsaInfoString);
    string fsaNum2Name_(int number);
    string stateNum2Name_(int number);
    string retrieveOverlay_(void);
    vector<string> retrieveLogfileList_(void);
    void createPlayBackPanel_(void);
    void buildBitmaps_(Pixel fg, Pixel bg, Widget parent);
    void createLoadLogfileDialog_(void);
    void loadRobotData_(void);
    void moveRobot_(void);
    void displayRobot_(void);
    void loadLogfile_(string logfileName);
    void loadSelectedLogfile_(XmString value);
    void changePlaybackStatus_(int newStatus);
    int readJogSliderValue_(void);
    int getPlaybackStatus_(void);
    void updateCurrentRobotDataIndex_(int newIndex);
    bool isSameStateInfoList_(
        vector<StateInfo_t> stateInfoListA,
        vector<StateInfo_t> stateInfoListB);
    bool initCBRClient_(void);
    void playbackLoop_(void);
    void reportCurrentState_(vector<StateInfo_t> stateInfoList);
    void startProcessThreadedXCommands_(void);
    void processThreadedXCommands_(void);
    void pendXCommandCreateRepairWindow_(void);
    void pendXCommandLoadNewMap_(string overlayName);
    void pendXCommandLoadLogfile_(string logfileName);
    void pendXCommandPickNewMap_(void);
    void pendXCommandCreateLoadLogfileDialog_(void);

    // Callbacks
    static XtTimerCallbackProc cbProcessThreadedXCommands_(XtPointer clientData);
    static void startPlayBackThread_(void *MlabPlayBackInstance);
    static void cbLoadLogfileDialogOK_(
        Widget w,
        XtPointer clientData,
        XmFileSelectionBoxCallbackStruct *call_data);
    static void cbLoadLogfileDialogCancel_(
        Widget w,
        XtPointer clientData,
        XmFileSelectionBoxCallbackStruct *callData);
    static void cbPlaybackButtonPushed_(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbChangeJogSlider_(
        Widget w,
        XtPointer clientData,
        XmFileSelectionBoxCallbackStruct *callData);

public:
    MlabPlayBack(
        XtAppContext app,
        Display *display,
        Widget parent,
        Widget drawingArea,
        double unitLength,
        int playbackMode);
    ~MlabPlayBack(void);
    vector<StateInfo_t> getCurrentStateInfoList(void);
    int getPlaybackStatus(void);
    void changePlaybackStatus(int newStatus);
};

inline void MlabPlayBack::changePlaybackStatus(int newStatus)
{
    changePlaybackStatus_(newStatus);
}

inline int MlabPlayBack::getPlaybackStatus(void)
{
    return getPlaybackStatus_();
}

extern MlabPlayBack *gPlayBack;
extern int gPlayBackMode;

#endif
/**********************************************************************
 * $Log: gt_playback.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
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
