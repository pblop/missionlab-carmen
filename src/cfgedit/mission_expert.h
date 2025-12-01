/**********************************************************************
 **                                                                  **
 **                           mission_expert.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_expert.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#ifndef MISSION_EXPERT_H
#define MISSION_EXPERT_H

#include <stdio.h>
#include <time.h>
#include <string>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>
#include <pthread.h>

#include "symbol.hpp"
#include "load_rc.h"
#include "mission_expert_types.h"
#include "configuration.hpp"
#include "cbrwizard.h"
#include "mission_spec_wizard.h"
#include "mission_spec_wizard_types.h"
#include "launch_wizard.h"

using std::string;
using std::vector;

class CfgEditCBRClient;
class MissionSpecWizard;

// Extentions of some of the output files
const string MEXP_EXTENSION_FEATUREFILENAME = "data";
const string MEXP_EXTENSION_FEATURESTATUSFILENAME = "status";
const string MEXP_EXTENSION_CDL = "cdl";
const string MEXP_EXTENSION_ACDL = "acdl";
const string MEXP_EXTENSION_BACKUP = "backup";

// Range of the slider bar
const string MEXP_DEFAULT_SLIDER1_RANAGE_TITLE = "[0.01 - 1.00]";
const string MEXP_DEFAULT_SLIDER10_RANAGE_TITLE = "[1 - 10]";
const string MEXP_DEFAULT_SLIDER100_RANAGE_TITLE = "[1 - 100]";

// Some strings
const string MEXP_STRING_OVERLAY = "overlay";
const string MEXP_STRING_TASK = "task";
const string MEXP_STRING_COORDINATE = "coordinate";
const string MEXP_STRING_FEATURE = "feature";
const string MEXP_STRING_OPTION = "option";
const string MEXP_STRING_CANCEL = "cancel";
const string MEXP_STRING_CONFIGURE = "configure";
const string MEXP_STRING_REPAIR = "repair";
const string MEXP_STRING_REPAIR_FAILURE = "repair-failure";
const string MEXP_STRING_YES = "Yes";
const string MEXP_STRING_NO = "No";
const string MEXP_STRING_RETRIEVE = "retrieve";
const string MEXP_STRING_ADD = "add";
const string MEXP_STRING_GLOBALFEATURE = "global-feature";
const string MEXP_STRING_VIA_POINT = "via-point";
const string MEXP_STRING_ROBOT = "Robot";
const string MEXP_STRING_INFINITY = "inf";
const string MEXP_STRING_DISABLED = "disabled";
const string MEXP_STRING_NA = "N/A";
const string MEXP_STRING_TASK_CONSTRAINTS = "task-constraints";
const string MEXP_STRING_TASK_CONSTRAINT = "task-constraint";
const string MEXP_STRING_ROBOT_CONSTRAINTS = "robot-constraints";
const string MEXP_STRING_ROBOT_CONSTRAINT = "robot-constraint";
const string MEXP_STRING_CNP = "cnp";
const string MEXP_STRING_GEO_COORDINATE = "geo-coordinate";

// RC file name (default).
const string MEXP_DEFAULT_RC_FILENAME = ".mission_expertrc";

// RC file vocabulary
const string MEXP_RC_STRING_FEATURE_KEY = "<FEATURE";
const string MEXP_RC_STRING_FEATURE_BEGIN = "BEGIN>";
const string MEXP_RC_STRING_FEATURE_END = "END>";
const string MEXP_RC_STRING_FEATURE_TYPE = "FEATURE-TYPE";
const string MEXP_RC_STRING_FEATURE_NAME = "FEATURE-NAME";
const string MEXP_RC_STRING_FEATURE_NON_INDEX = "FEATURE-NON-INDEX";
const string MEXP_RC_STRING_FEATURE_HIDE = "FEATURE-HIDE";
const string MEXP_RC_STRING_FEATURE_WEIGHT = "FEATURE-WEIGHT";
const string MEXP_RC_STRING_FEATURE_ALLOW_DISABLE = "FEATURE-ALLOW-DISABLE";
const string MEXP_RC_STRING_FEATURE_OPTION_TYPE = "FEATURE-OPTION-TYPE";
const string MEXP_RC_STRING_FEATURE_SELECTED_OPTION = "FEATURE-SELECTED-OPTION";
const string MEXP_RC_STRING_FEATURE_OPTION = "FEATURE-OPTION";
const string MEXP_RC_STRING_TASK_KEY = "<TASK";
const string MEXP_RC_STRING_TASK_BEGIN = "BEGIN>";
const string MEXP_RC_STRING_TASK_END = "END>";
const string MEXP_RC_STRING_TASK_NAME = "TASK-NAME";
const string MEXP_RC_STRING_TASK_BUTTON_NAME = "TASK-BUTTON-NAME";
const string MEXP_RC_STRING_TASK_SPECIFIC_SELECTED_OPTION = "TASK-SPECIFIC-SELECTED-OPTION";
const string MEXP_RC_STRING_TASK_MAX_USAGE = "TASK-MAX-USAGE";
const string MEXP_RC_STRING_TASK_RUNTIME_CNP = "TASK-RUNTIME-CNP";
const string MEXP_RC_STRING_TASK_ICARUS = "TASK-ICARUS";
const string MEXP_RC_STRING_DEFAULT_TASK = "DEFAULT-TASK";
const string MEXP_RC_STRING_DEFAULT_TASK_WEIGHT = "DEFAULT-WEIGHT";
const string MEXP_RC_STRING_RUNTIME_CNP_MISSION_MANAGER_TASK = "RUNTIME-CNP-MISSION-MANAGER-TASK";
const string MEXP_RC_STRING_RUNTIME_CNP_BIDDER_TASK = "RUNTIME-CNP-BIDDER-TASK";
const string MEXP_RC_STRING_ICARUS_TASK = "ICARUS-TASK";
const string MEXP_RC_STRING_GLOBAL = "GLOBAL";
const string MEXP_RC_STRING_LOCAL = "LOCAL";
const string MEXP_RC_STRING_TOGGLE = "TOGGLE";
const string MEXP_RC_STRING_SLIDER = "SLIDER";
const string MEXP_RC_STRING_CONSTRAINT_KEY = "<CONSTRAINT";
const string MEXP_RC_STRING_CONSTRAINT_BEGIN = "BEGIN>";
const string MEXP_RC_STRING_CONSTRAINT_END = "END>";
const string MEXP_RC_STRING_CONSTRAINT_TYPE = "CONSTRAINT-TYPE";
const string MEXP_RC_STRING_CONSTRAINT_NAME = "CONSTRAINT-NAME";
const string MEXP_RC_STRING_CONSTRAINT_HIDE = "CONSTRAINT-HIDE";
const string MEXP_RC_STRING_CONSTRAINT_OPTION_TYPE = "CONSTRAINT-OPTION-TYPE";
const string MEXP_RC_STRING_CONSTRAINT_SELECTED_OPTION = "CONSTRAINT-SELECTED-OPTION";
const string MEXP_RC_STRING_CONSTRAINT_OPTION = "CONSTRAINT-OPTION";
const string MEXP_RC_STRING_ROBOT_CONSTRAINTS_KEY = "<ROBOT-CONSTRAINTS";
const string MEXP_RC_STRING_ROBOT_CONSTRAINTS_BEGIN = "BEGIN>";
const string MEXP_RC_STRING_ROBOT_CONSTRAINTS_END = "END>";
const string MEXP_RC_STRING_ROBOT_CONSTRAINTS_ROBOT_ID = "ROBOT-CONSTRAINTS-ROBOT-ID";

// Misc setups.
const float MEXP_DISABLED_OPTION_VALUE = -1.0;
const int MEXP_USAGE_INFINITY = 0;

class MissionExpert : public CBRWizard {

public:
    typedef enum FeatureStatus_t {
        CANCEL,
        CONFIGURE,
        REPAIR,
        REPAIR_FAILURE
    };

protected:
    typedef enum StatusWindowNavigation_t {
        STATUS_WINDOW_PREV,
        STATUS_WINDOW_NEXT
    };

    typedef enum ThreadedXCommandsTypes_t {
        X_COMMAND_UPDATE_STATUS_WINDOW,
        X_COMMAND_HIGHLIGHT_STATES
    };

    typedef enum RevertStatus_t {
        REVERT_FAILURE,
        REVERT_SUCCESS,
        REVERT_NO_ORIGINAL_FILE,
        REVERT_NO_BACKUP_FILE
    };

    typedef struct FeatureMenuCallbackData_t {
        int featureId;
        int optionType;
        float optionValue;
        MissionExpert *missionExpertInstance;
    };

    typedef struct GlobalFeatureMenuCallbackData_t {
        int globalFeaturesId;
        int optionType;
        float optionValue;
        MissionExpert *missionExpertInstance;
    };

    typedef struct FeatureMenuTaskCallbackData_t {
        int taskId;
        MissionExpert *missionExpertInstance;
    };

    typedef struct StatusWindows_t {
        Widget popupShell;
        Widget titleBox;
        Widget textBox;
        Widget prevBtn;
        Widget nextBtn;
        Widget loadBtn;
        Widget cancelBtn;
        Widget ratingBox;
        Widget *starBox;
        int textPos;
        bool isUp;
        int currentMissionId;
        int selectedMissionId;
    };

    typedef struct XCommandUpdateStatesWindow_t {
        int rating;
        string missionSummary;
        bool prevBtnIsSensitive;
        bool nextBtnIsSensitive;
        bool loadBtnIsSensitive;
        bool cancelBtnIsSensitive;
        int currentMissionId;
    };

    typedef struct XCommandHighlightStates_t {
        vector<MExpStateInfo_t> stateInfoList;
    };

    typedef struct PendingThreadedXCommands_t {
        int type;
        int index;
    };

    typedef struct CheckForkedMlabStatusData_t {
        MissionExpert *missionExpertInstance;
        int pid;
        bool eventLogIsON;
    };

    typedef struct MetadataEntryWindowData_t {
        Widget commentTextWindow;
        Widget mainWindow;
        Widget nameTextWindow;
        Widget *ratingBtns[NUM_MEXP_METADATA_RATINGS];
        vector<Widget> poppedUpWidgets;
        vector<int> dataIndexList;
        string commentTextWindowString;
        string nameTextWindowString;
        string userName;
        int ratings[NUM_MEXP_METADATA_RATINGS];
        bool windowIsUp;
    };

    typedef struct SaveCBRLibraryWindowData_t {
        Widget mainWindow;
        Widget filenameTextWindow;
        Widget saveBtn;
        vector<Widget> poppedUpWidgets;
        string filenameTextWindowString;
    };

protected:
    FILE *featureStatusFile_;
    FILE *resultFile_;
    Widget featureMenu_w_;
    Widget **featureMenuOption_w_;
    Widget *featureMenuTask_w_;
    Widget **globalFeaturesMenuOption_w_;
    Widget *globalFeaturesMenuTask_w_;
    StatusWindows_t statusWindows_;
    Pixmap starPixmap_;
    Pixmap emptyStarPixmap_;
    MExpMissionTask_t featureMenuUpTask_;
    MExpRetrievedMissionSummary_t retrievedMissionSummary_;
    MExpStateInfo_t lastHighlightedStateInfo_;
    MExpRobotIDs_t currentRobotIDs_;
    MetadataEntryWindowData_t *metadataEntryWindowData_;
    CfgEditCBRClient *cfgeditCBRClient_;
    MissionSpecWizard *missionSpecWizard_;
    SaveCBRLibraryWindowData_t *saveCBRLibraryWindowData_;
    vector<MExpFeature_t> featureMenuUpGlobalFeatures_;
    vector<MExpFeature_t> fixedGlobalFeatures_;
    vector<MExpLogfileInfo_t> logfileInfoList_;
    vector<XCommandUpdateStatesWindow_t> shXCommandUpdateStatesWindowList_;
    vector<XCommandHighlightStates_t> shXCommandHighlightStatesList_;
    vector<PendingThreadedXCommands_t> shPendingThreadedXCommands_;
    vector<Widget> featureMenuWidgets_;
    vector<string> createdDataFileList_;
    vector<int> currentCBRLibraryDataIndexList_;
    string featureFileString_;
    string resultFileName_;
    string missionPlanFileName_;
    string robotName_;
    string dataFilesDirectory_;
    string missionTimeString_;
    int mexpRunMode_;
    int numVisibleGlobalFeatures_;
    int numVisibleLocalFeatures_;
    bool disableMExp_;
    bool dataFilesNamed_;
    bool keepDataFiles_;
    bool featuresLoaded_;
    bool featureMenuIsUp_;
    bool shouldSaveMissionToCBRLibrary_;
    bool missionSavedToCBRLibrary_;
    bool missionLoadedFromCBRLibrary_;
    bool disableLaunchWizard_;
    bool disableRepair_;

    MExpStateInfo_t EMPTY_STATE_INFO_; 

    static LaunchWizard *launchWizard_;
    static pthread_mutex_t threadedXCommandsMutex_;

    static const string RCTABLE_DISABLE_LAUNCH_WIZARD_STRING_;
    static const string RCTABLE_DISABLE_PREMISSION_CNP_STRING_;
    static const string RCTABLE_DISABLE_REPAIR_STRING_;
    static const string RCTABLE_DISABLE_RUNTIME_CNP_STRING_;
    static const string RCTABLE_DISABLE_STRING_;
    static const string RCTABLE_DATAFILESDIR_STRING_;
    static const string DEFAULT_FEATUREMENU_TITLE_;
    static const string DEFAULT_FEATUREMENU_TASK_LABEL_;
    static const string DEFAULT_FEATUREMENU_SAVE_BUTTON_LABEL_;
    static const string DEFAULT_FEATUREMENU_CANCEL_BUTTON_LABEL_;
    static const string DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_;
    static const string DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_;
    static const string DEFAULT_METADATA_ENTRY_WINDOW_NAME_LABEL_;
    static const string DEFAULT_METADATA_ENTRY_WINDOW_SAVE_BUTTON_LABEL_;
    static const string DEFAULT_METADATA_ENTRY_WINDOW_CANCEL_BUTTON_LABEL_;
    static const string DEFAULT_SAVE_CBR_LIBRARY_WINDOW_CANCEL_BUTTON_LABEL_;
    static const string DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_LABEL_;
    static const string DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_;
    static const string DEFAULT_SAVE_CBR_LIBRARY_WINDOW_SAVE_BUTTON_LABEL_;
    static const string DEFAULT_STATUSWINDOW_TITLE_;
    static const string DEFAULT_STATUSWINDOW_RATING_LABEL_;
    static const string DEFAULT_STATUSWINDOW_NAVIGATION_LABEL_;
    static const string DEFAULT_STATUSWINDOW_PREV_BUTTON_LABEL_;
    static const string DEFAULT_STATUSWINDOW_NEXT_BUTTON_LABEL_;
    static const string DEFAULT_STATUSWINDOW_LOAD_BUTTON_LABEL_;
    static const string DEFAULT_STATUSWINDOW_CANCEL_BUTTON_LABEL_;
    static const string ACDL_STRING_AGENT_NAME_;
    static const string ACDL_STRING_TASK_INDEX_;
    static const string ACDL_STRING_START_;
    static const string CDL_STRING_MAXVELOCITY_;
    static const string CDL_STRING_BASEVELOCITY_;
    static const string CDL_STRING_START_SUB_MISSION_;
    static const string FEATURE_NAME_NUMROBOTS_;
    static const string FEATURE_NAME_MAXVELOCITY_;
    static const string FEATURE_NAME_AGGRESSIVENESS_;
    static const string CONFIRM_SUCCESSFUL_MISSION_PHRASE_;
    static const string CONFIRM_VIEW_ANYWAY_PHRASE_;
    static const string ASK_USE_MEXP_PHRASE_;
    static const string ASK_DIAGNOSIS_PHRASE_;
    static const string ASK_CNP_PHRASE_;
    static const string ASK_CNP_PHRASE_SIMPLE_;
    static const string ASK_PREMISSION_CNP_PHRASE_;
    static const string ASK_PREMISSION_CNP_PHRASE_SIMPLE_;
    static const string ASK_RUNTIME_CNP_PHRASE_;
    static const string ASK_RUNTIME_CNP_PHRASE_SIMPLE_;
    static const string REPAIR_NO_SOLUTION_PHRASE_;
    static const string REPAIR_SOLUTION_PHRASE_;
    static const string FSASUMMARY_EMPTY_PHRASE_;
    static const string FSASUMMARY_DEFAULT_PHRASE_;
    static const string STRING_ADD_;
    static const string METADATA_LABELS_[NUM_MEXP_METADATA_RATINGS];
    static const string RCTABLE_DISABLE_MISSION_SPEC_WIZARD_STRINGS_[NUM_MISSION_SPEC_WIZARD_TYPES];
    static const double REPLAY_DISPLAY_RATIO_;
    static const int DEFAULT_FEATUREMENU_WIDTH_;
    static const int DEFAULT_FEATUREMENU_LINEHEIGHT_;
    static const int DEFAULT_STATUSWINDOW_WIDTH_;
    static const int DEFAULT_STATUSWINDOW_LINEHEIGHT_;
    static const int DEFAULT_STATUSWINDOW_TOPTITLE_HEIGHT_;
    static const int DEFAULT_STATUSWINDOW_TEXT_HEIGHT_;
    static const int DEFAULT_STATUSWINDOW_TEXT_NUMROWS_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_ACTIONSPACE_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_EXTRA_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_ITEM_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_ITEM_LABEL_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_NAME_HEIGHT_;
    static const int DEFAULT_METADATA_ENTRY_WINDOW_WIDTH_;
    static const int DEFAULT_SAVE_CBR_LIBRARY_WINDOW_ACTIONSPACE_HEIGHT_;
    static const int DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_HEIGHT_;
    static const int DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_HEIGHT_;
    static const int DEFAULT_SAVE_CBR_LIBRARY_WINDOW_WIDTH_;
    static const int MAX_METADATA_RATING_;
    static const int MAX_SUITABILITY_RATING_;
    static const int EVENT_WAITER_SLEEP_TIME_USEC_;
    static const int PROCESS_THREADED_X_COMMAND_TIMER_MSEC_;
    static const char CHAR_NULL_;
    static const bool SEND_FEEDBACKS_TO_LIBRARY_;
    static const bool SEND_METADATA_TO_LIBRARY_;
    static const bool INCREMENTALLY_UPDATE_LIBRARY_;
    
    MExpStateInfo_t highlightStates_(
        vector<MExpStateInfo_t> stateInfoList,
        MExpStateInfo_t lastHighlightedStateInfo,
        bool *highlighted);
    config_fsa_lists_t getFSALists_(void);
    string createPrefix_(void);
    MExpStateInfo_t highlightStates_(vector<MExpStateInfo_t> stateInfoList);
    int checkFeatureStatusFile_(void);
    int revertFile_(string filename);
    void closeMetadataEntryWindow_(void);
    void createTemporaryTaskAndGlobalFeatures_(void);
    void initializeMetadataEntryWindowData_(bool keepUserName);
    void nameDataFiles_(void);
    void runMlabMissionDesign_(void);
    void runMlabReplay_(void);
    void runCBRMExp_(void);
    void readCBRMExpResult_(void);
    void popupFeatureMenu_(bool askTaskType);
    void popupMetadataEntryWindow_(vector<int> dataIndexList);
    void popupStatusWindow_(
        int rating,
        string missionSummary,
        bool prevBtnIsSensitive,
        bool nextBtnIsSensitive,
        bool loadBtnIsSensitive,
        bool cancelBtnIsSensitive,
        int currentMissionId);
    void updateStatusWindow_(
        int rating,
        string missionSummary,
        bool prevBtnIsSensitive,
        bool nextBtnIsSensitive,
        bool loadBtnIsSensitive,
        bool cancelBtnIsSensitive,
        int currentMissionId);
    void pendXCommandUpdateStatusWindow_(
        int rating,
        string missionSummary,
        bool prevBtnIsSensitive,
        bool nextBtnIsSensitive,
        bool loadBtnIsSensitive,
        bool cancelBtnIsSensitive,
        int currentMissionId);
    void selectMissionForStatusWindow_(void);
    void closeStatusWindow_(void);
    void closeSaveCBRLibraryWindow_(void);
    void changeStatusWindow_(int direction);
    void changeFeatureMenuValue_(XtPointer clientData);
    void changeGlobalFeatureMenuValue_(XtPointer clientData);
    void changeFeatureMenuTaskValue_(int taskId);
    void saveCase_(void);
    void closeFeatureMenu_(void);
    void deleteCreatedDataFiles_(void);
    void addCreatedDataFileList_(string filename);
    void createPixmaps_(Pixel fg, Pixel bg, Widget parent);
    void askDiagnosis_(void);
    void startProcessThreadedXCommands_(void);
    void pendXCommandHighlightStates_(vector<MExpStateInfo_t> stateInfoList);
    void processThreadedXCommands_(void);
    void startCheckForkedMlabStatus_(int pid, bool eventLogIsON);
    void clearFeatureFile_(void);
    void askCNPMode_(bool simpleCheck);
    void askMissionSpecWizardType_(void);
    void resumeMissionExpert_(void);
    void saveMetadataEntries_(void);
    void sendCBRLibraryFile_(void);
    void updateMetadataCommentTextWindowString_(void);
    void updateMetadataNameTextWindowString_(void);
    void updateMetadataRatingBtn_(Widget button);
    void updateSaveCBRLibraryFileWindowString_(void);
    bool confirmViewAnyway_();
    bool initCBRClient_(void);
    bool buildMission_(void);
    bool repairMission_(void);
    bool loadRetrievedMissionChosenByUser_(
        vector<MExpRetrievedMission_t> retrievedMissions,
        string overlayFileName,
        int *selectedIndex);
    bool loadRetrievedMission_(
        MExpRetrievedMission_t retrievedMission,
        string overlayFileName);
    bool hasAgentInMissionPlan_(string agentName);
    bool hasAgentInMissionPlan_(string missionPlanFileName, string agentName);
    bool isSameStateInfo_(MExpStateInfo_t stateInfoA, MExpStateInfo_t stateInfoB);
    bool checkForkedProcessStatus_(int pid);
    bool saveAbstractedMissionPlanToFile_(Symbol * agent, string fileName, bool *hasStartSubMission);

    // Callbacks
    static XtTimerCallbackProc cbProcessThreadedXCommands_(XtPointer clientData);
    static XtTimerCallbackProc cbCheckForkedMlabStatus_(XtPointer clientData);
    static void cbCancelBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbCancelMetadataEntries_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbCancelSaveCase_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbCancelSaveCBRLibraryFile_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbChangeGlobalFeatureMenuValue_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbChangeMetadataRatingBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbFeatureMenuTaskValue_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbFeatureMenuValue_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbLoadBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataCommentTextWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataNameTextWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbNextBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbPrevBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbSaveCase_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbSaveCBRLibraryFile_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbSaveCBRLibraryFileWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbSaveMetadataEntries_(Widget w, XtPointer clientData, XtPointer callbackData);

public:
    MissionExpert(
        Display *display,
        Widget parent,
        XtAppContext app,
        const symbol_table<rc_chain> &rcTable,
        string mexpRCFileName);
    ~MissionExpert(void);
    string getAbstractedMissionPlan(Symbol * agent);
    string getMergedAbstractedMissionPlan(
        vector<string> acdlMissions,
        int *numRobots);
    string getOverlayFileName(void);
    string getFeatureFileString(void);
    vector<MExpLogfileInfo_t> getLogfileInfoList(void);
    int assignedRobotID(int index);
    int getCNPMode(void);
    int maxSuitabilityRating(void);
    void initialize(void);
    void confirmSuccessfulMission(void);
    void runMissionExpert(int mode);
    void resumeMissionExpert(void);
    void saveOverlayFileName(string overlayFileName);
    void saveMissionTimeString(string missionDateString);
    void saveSelectionToCBRLibrary(Symbol * agent);
    void madeChange(void);
    void saveLogfileInfo(MExpLogfileInfo_t logfileInfo);
    void clearLogfileInfoList(void);
    void highlightStates(vector<MExpStateInfo_t> stateInfoList);
    void saveSpecifiedFeatureFileName(string featureFileName);
    void printfMissionSpecWizardInstructionWindow(const char *format, ...);
    void printfMissionSpecWizardDataWindow(const char *format, ...);
    void updateMissionSpecWizardProgressBar(double level);
    void clearMissionSpecWizardProgressBar(void);
    void popupMissionSpecWizard_(int wizardType);
    void popupSaveCBRLibraryWindow_(string cblFileName);
    void runMlabMissionDesign(void);
    void saveCBRLibraryFile(void);
    void sendMissionFeedback(vector<int> dataIndexList);
    void updateMissionUsageCounter(vector<int> dataIndexList);
    bool askUseMissionExpert(void);
    bool isDisabled(void);
    bool isEnabled(void);
    bool isRunning(void);
    bool repairDisabled(void);
    bool featureMenuIsUp(void);
    bool missionSavedToCBRLibrary(void);
    bool missionSpecWizardIsUp(void);
    bool shouldSaveMissionToCBRLibrary(void);
    bool shouldAskFeedback(void);

    static void cbPopupMissionSpecWizard(Widget w, XtPointer clientData, XtPointer callbackData);
    static MExpRetrievedMissionSummary_t summarizeRetrievedMissions(
        const vector<MExpRetrievedMission_t> retrievedMissions);
};

inline int MissionExpert::getCNPMode(void)
{
    return cnpMode_;
}

inline bool MissionExpert::isDisabled(void)
{
    return disableMExp_;
}

inline bool MissionExpert::isEnabled(void)
{
    return (!disableMExp_);
}

inline bool MissionExpert::isRunning(void)
{
    return (mexpRunMode_ != MEXP_RUNMODE_NONE);
}

inline bool MissionExpert::repairDisabled(void)
{
    return disableRepair_;
}

inline bool MissionExpert::shouldSaveMissionToCBRLibrary(void)
{
    return shouldSaveMissionToCBRLibrary_;
}

inline void MissionExpert::selectMissionForStatusWindow_(void)
{
    statusWindows_.selectedMissionId =  statusWindows_.currentMissionId;
}

inline void MissionExpert::runMlabMissionDesign(void)
{
    runMlabMissionDesign_();
}

inline void MissionExpert::resumeMissionExpert(void)
{
    resumeMissionExpert_();
}

inline bool MissionExpert::isSameStateInfo_(
    MExpStateInfo_t stateInfoA,
    MExpStateInfo_t stateInfoB)
{
    return ((stateInfoA.fsaName == stateInfoB.fsaName) &&
            (stateInfoA.stateName == stateInfoB.stateName));
}

inline int MissionExpert::maxSuitabilityRating(void)
{
    return MAX_SUITABILITY_RATING_;
}

inline bool MissionExpert::featureMenuIsUp(void)
{
    return featureMenuIsUp_;
}

inline bool MissionExpert::missionSavedToCBRLibrary(void)
{
    return missionSavedToCBRLibrary_;
}

extern MissionExpert *gMExp;

#endif

/**********************************************************************
 * $Log: mission_expert.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.19  2007/09/29 23:45:06  endo
 * Global feature can be now disabled.
 *
 * Revision 1.18  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.17  2007/09/18 22:36:12  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.16  2007/08/10 15:14:59  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.15  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.14  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.13  2007/08/04 23:52:54  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.12  2007/06/28 03:54:20  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.11  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 * Revision 1.10  2007/05/15 18:50:01  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.9  2007/03/09 22:26:08  endo
 * The bug that prevents Run dialog to be shown even if ICARUS Wizard is not running is fixed.
 *
 * Revision 1.8  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.7  2007/01/29 15:08:35  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.6  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.5  2006/09/26 18:30:27  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.4  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.3  2006/09/15 22:37:40  endo
 * ICARUS Wizard compilation meter bars added.
 *
 * Revision 1.2  2006/09/13 19:03:48  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.8  2006/05/15 01:23:28  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.7  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.6  2006/01/30 02:47:28  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.5  2005/08/09 19:12:44  endo
 * Things improved for the August demo.
 *
 * Revision 1.4  2005/07/31 03:39:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.3  2005/07/16 08:47:49  endo
 * CBR-CNP integration
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:35  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 12:25:34  endo
 * gcc 3.1.1
 *
 * Revision 1.2  2003/04/06 08:50:37  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/12 23:09:58  endo
 * Initial revision
 *
 **********************************************************************/
