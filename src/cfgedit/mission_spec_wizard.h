/**********************************************************************
 **                                                                  **
 **                      mission_spec_wizard.h                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains a header for the MissionSpecWizard class.     **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
#ifndef MISSION_SPEC_WIZARD_H
#define MISSION_SPEC_WIZARD_H

/* $Id: mission_spec_wizard.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <vector>
#include <string>

#include "symbol.hpp"
#include "load_rc.h"
#include "mission_expert_types.h"
#include "mission_expert.h"

using std::vector;
using std::string;

class MissionExpert;

class MissionSpecWizard {

protected:
    typedef struct MainWindowData_t {
        Widget popupShell;
        Widget instructionWindow;
        Widget dataWindow;
        Widget closeBtn;
        Widget detailBtn;
        Widget nextBtn;
        Widget backBtn;
        vector<Widget> windows;
        bool windowIsUp;
    };

    typedef struct MiniWindowData_t {
        Widget popupShell;
        vector<Widget> windows;
    };

    typedef struct StatusRow_t {
        Widget form;
        Widget label;
        int h;
        int w;
    };

    typedef struct StepData_t {
        int stepNum;
        string description;
    };

    // If you add an entry to this enum, make sure that METADATA_DISPLAY_CONTENTS_LABELS_[]
    // and METADATA_DISPLAY_CONTENTS_RIGHTPOS_ are updated as well.
    typedef enum MetaDataDisplayContents_t {
        METADATA_DISPLAY_CHECK_BOX,
        METADATA_DISPLAY_NUMBER,
        METADATA_DISPLAY_NUM_ROBOTS,
        METADATA_DISPLAY_STEALTH_RATING,
        METADATA_DISPLAY_EFFICIENCY_RATING,
        METADATA_DISPLAY_PRESERVATION_RATING,
        METADATA_DISPLAY_DURATION_RATING,
        METADATA_DISPLAY_RELEVANCE,
        METADATA_DISPLAY_USER_COMMENTS,
        METADATA_DISPLAY_NUM_USAGE,
        METADATA_DISPLAY_BUTTONS,
        NUM_METADATA_DISPLAY_CONTENTS
    };

    // If you add an entry to this enum, make sure that METADATA_DISPLAY_DISPLAY_LABELS_[]
    // is updated as well.
    typedef enum MetaDataDisplayButtons_t {
        METADATA_DISPLAY_BTN_DETAIL,
        METADATA_DISPLAY_BTN_REHERASE,
        METADATA_DISPLAY_BTN_SEND,
        NUM_METADATA_DISPLAY_BTNS
    };

    // If you add an entry to this enum, make sure that
    // METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_ is updated as well.
    typedef enum MetaDataFilterLineContents_t {
        METADATA_FILTER_LINE_SPACE1,
        METADATA_FILTER_LINE_MERGE_BUTTON,
        METADATA_FILTER_LINE_SPACE2,
        METADATA_FILTER_LINE_TYPE_IN,
        METADATA_FILTER_LINE_SPACE3,
        METADATA_FILTER_LINE_FILTER_BUTTON,
        METADATA_FILTER_LINE_SPACE4,
        METADATA_FILTER_LINE_CLEAR_BUTTON,
        METADATA_FILTER_LINE_SPACE5,
        NUM_METADATA_FILTER_LINE_CONTENTS
    };

    typedef struct MetaDataDisplayLine_t {
        int number;
        Widget checkBox;
        Widget btn[NUM_METADATA_DISPLAY_BTNS];
    };

    typedef struct MetadataDetailWindowOKBtnCallbackData_t {
        int selectedMission;
        MissionSpecWizard *missionSpecWizardInstance;
    };

    typedef struct MetadataDetailWindowAssemblageBtnCallbackData_t {
        int selectedMission;
        MissionSpecWizard *missionSpecWizardInstance;
    };

    typedef struct MetaDataDisplayLineActionBtnCallbackData_t {
        int selectedMission;
        int buttonNum;
        MissionSpecWizard *missionSpecWizardInstance;
    };

    typedef struct MetaDataReorderRequestCallbackData_t {
        int metadataOrderReference;
        MissionSpecWizard *missionSpecWizardInstance;
    };

protected:
    Display *display_;
    XtAppContext appContext_;
    Widget actionWindow_;
    Widget dataWindowContent_;
    Widget instructionWindow_;
    Widget metadataDetailWindow_;
    Widget metadataFilterTypeInBox_;
    Widget parentWidget_;
    Widget *progressBarIncs_;
    Pixel parentWidgetFg_;
    Pixel parentWidgetBg_;
    Pixmap starPixmap_;
    Pixmap emptyStarPixmap_;
    XmFontList instructionFontList_;
    XmFontList metadataDisplayFontList_;
    XmFontList statusNormalFontList_;
    XmFontList statusBoldFontList_;
    Font defaultInstructionFont_;
    Font defaultStatusNormalFont_;
    Font defaultStatusBoldFont_;
    Font defaultMetadataDisplayFont_;
    MainWindowData_t *mainWindowData_;
    MiniWindowData_t *miniWindowData_;
    vector<MetaDataDisplayLine_t> metaDataDisplayLines_;
    vector<MExpRetrievedMission_t> retrievedMissions_;
    vector<StatusRow_t> statusRows_;
    vector<int> metadataOrder_;
    vector<int> metadataFilterRejectList_;
    string metadataFilterString_;
    symbol_table<rc_chain> rcTable_;
    MissionExpert *mexp_;
    int currentStep_;
    int maxSuitabilityRating_;
    int selectedCheckBox_;
    int selectedMission_;
    int wizardType_;
    int metadataOrderReference_;
    bool abortLoadingMission_;
    bool detailShown_;
    bool disableColorButtons_;
    bool metadataDetailWindowIsUp_;
    bool missionLoaded_;
    bool noRehearsal_;
    bool disableRepair_;
    bool waiting_;
    bool windowsInitialized_;
    bool reverseMetadataOrder_;
    bool missionModified_;
    bool mergingMissions_;

    static const struct timespec EVENT_WAITER_SLEEP_TIME_NSEC_;
    static const string DETAIL_BUTTON_LABEL_DETAIL_;
    static const string DETAIL_BUTTON_LABEL_HIDE_;
    static const string EMPTY_STRING_;
    static const string INSTRUCTION_FONT_NAME_;
    static const string INSTRUCTION_SPECIFICATION_CANCEL_MESSAGE_;
    static const string METADATA_DISPLAY_FONT_NAME_;
    static const string MINI_WINDOW_MAXIMIZE_LABEL_;
    static const string MISSION_FEEDBACK_BUTTON_LABEL_;
    static const string NEXT_BUTTON_LABEL_NEXT_;
    static const string NEXT_BUTTON_LABEL_RESTART_;
    static const string RCTABLE_DISABLE_COLOR_BUTTONS_;
    static const string RESPECIFY_BUTTON_LABEL_;
    static const string SAVECBR_BUTTON_LABEL_;
    static const string STATUS_NORMAL_FONT_NAME_;
    static const string STATUS_BOLD_FONT_NAME_;
    static const string MERGE_CANCEL_BUTTON_LABEL_;
    static const string MERGE_MISSIONS_BUTTON_LABEL_;
    static const string MERGE_MISSIONS_INSTRUCTION_;
    static const string METADATA_DETAIL_WINDOW_TITLE_;
    static const string METADATA_DETAIL_WINDOW_OK_LABEL_;
    static const string METADATA_DETAIL_WINDOW_CANCEL_LABEL_;
    static const string METADATA_DETAIL_WINDOW_ASSEMBLAGE_LABEL_;
    static const string METADATA_DISPLAY_CONTENTS_LABELS_[NUM_METADATA_DISPLAY_CONTENTS];
    static const string METADATA_DISPLAY_BUTTON_LABELS_[NUM_METADATA_DISPLAY_BTNS];
    static const string METADATA_DETAIL_WINDOW_RATING_NAMES_[NUM_MEXP_METADATA_RATINGS];
    static const string METADATA_FILTER_BUTTON_LABEL_;
    static const string METADATA_FILTER_CLEAR_BUTTON_LABEL_;
    static const string STRING_NA_;
    static const string SLIDER_CLIP_WINDOW_NAME_;
    static const string STRING_GLOBAL_REF_POINT_;
    static const double DATAWINDOW2CONTENTS_WIDTH_RATIO_;
    static const double DISPLAY2META_HEIGHT_RATIO_;
    static const double DISPLAY2META_WIDTH_RATIO_;
    static const double DISPLAY2MINI_WIDTH_RATIO_;
    static const double DISPLAY2MINI_HEIGHT_RATIO_;
    static const double DISPLAY2MINI_X_OFFSET_RATIO_;
    static const double DISPLAY2MINI_Y_OFFSET_RATIO_;
    static const double MAIN2ACTION_HEIGHT_RATIO_;
    static const double MAIN2ACTION_X_OFFSET_RATIO_;
    static const double MAIN2ACTION_EXTRA_SPACE_RATIO_;
    static const double MAIN2DATA_HEIGHT_RATIO_;
    static const double MAIN2DATA_X_OFFSET_RATIO_;
    static const double MAIN2DATA_Y_OFFSET_RATIO_;
    static const double MAIN2INSTRUCTION_HEIGHT_RATIO_;
    static const double MAIN2INSTRUCTION_X_OFFSET_RATIO_;
    static const double MAIN2INSTRUCTION_Y_OFFSET_RATIO_;
    static const double MAIN2MISSIONFEEDBACK_HEIGHT_RATIO_;
    static const double MAIN2PROGRESS_HEIGHT_RATIO_;
    static const double MAIN2RESPECIFY_HEIGHT_RATIO_;
    static const double MAIN2SAVECBR_HEIGHT_RATIO_;
    static const double MAIN2STATUS_WIDTH_RATIO_;
    static const double MAIN2STATUS_HEIGHT_RATIO_;
    static const double MAIN2STATUS_X_OFFSET_RATIO_;
    static const double MAIN2STATUS_Y_OFFSET_RATIO_;
    static const double MAIN2STATUS_EXTRA_SPACE_RATIO_;
    static const double MINI2ACTION_WIDTH_RATIO_;
    static const double MINI2ACTION_HEIGHT_RATIO_;
    static const double MINI2ACTION_OFFSET_X_RATIO_;
    static const double MINI2ACTION_OFFSET_Y_RATIO_;
    static const int NUM_PROGRESS_BAR_STEPS_;
    static const int EVENT_WAITER_SLEEP_TIME_MSEC_;
    static const int METADATA_DISPLAY_CONTENTS_RIGHTPOS_[NUM_METADATA_DISPLAY_CONTENTS];
    static const int METADATA_DISPLAY_LINE_HEIGHT_;
    static const int METADATA_DISPLAY_LINE_EXTRA_HEIGHT_;
    static const int METADATA_DISPLAY_LINE_Y_OFFSET_;
    static const int METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[NUM_METADATA_FILTER_LINE_CONTENTS];
    static const int METADATA_FILTER_LINE_HEIGHT_;
    static const int MISSION_RATING_INDEX_2_METADATA_DISPLAY_INDEX_[NUM_MEXP_METADATA_RATINGS];

    virtual string instructionStartUpMessage_(void) = 0;
    virtual string mainWindowTitle_(void) = 0;
    virtual string miniWindowTitle_(void) = 0;
    virtual StepData_t *stepData_(void) = 0;
    virtual int numSteps_(void) = 0;
    virtual int stepChooseMission_(void) = 0;
    virtual int stepCompletion_(void) = 0;
    virtual int stepRehearsal_(void) = 0;
    virtual int stepSpecifyMission_(void) = 0;
    virtual void processBackBtn_(void) = 0;
    virtual void processDetailBtn_(void) = 0;
    virtual void processFirstStep_(void) = 0;
    virtual void processNextBtn_(void) = 0;
    virtual void processStep_(int stepNum) = 0;

    string findClosestOverlay_(float latitude, float longitude);
    string generateMetadataDetailWindowText_(int selectedMission);
    void addNewMetadataEntryForNewUnsavedMission_(
        int numRobots,
        string acdlMission);
    void checkCurrentTime_(
        int *year,
        int *month,
        int *day,
        int *hour,
        int *minute,
        int *second);
    void checkMetadataDisplayLineCheckBoxForRetrievedMission_(int retrievedMissionIndex);
    void clearDataWindowContent_(void);
    void clearMetadataFilter_(void);
    void closeMainWindow_(void);
    void closeMiniWindow_(void);
    void createDataWindowMetadata_(void);
    void createDataWindowMissionFeedbackButton_(void);
    void createDataWindowProgressBar_(void);
    void createDataWindowRespecifyButton_(void);
    void createDataWindowSaveCBRButton_(void);
    void createDataWindowScrolledText_(string text);
    void createFonts_(void);
    void createMainWindow_(void);
    void createMiniWindow_(void);
    void createPixmaps_(void);
    void filterMetadata_(void);
    void initializeWindows_(void);
    void mergeTwoMissions_(int missionIndexA, int missionIndexB);
    void orderMetadata_(void);
    void popupMetadataDetailWindow_(int selectedMission);
    void popupMiniWindow_(void);
    void processMergeButton_(Widget mergeBtn);
    void processMetadataDisplayLineActionBtn_(int selectedMission, int buttonNum);
    void processMetadataDetailWindowAssemblageBtn_(int selectedMission);
    void refreshMainWindow_(void);
    void resetDetailBtn_(bool detailShown, bool labelHideDetail, bool sensitive);
    void resetNextBtn_(bool labelStartOver, bool sensitive);
    void setMetadataOrder_(int orderReference);
    void setSelectedMission_(int selectedMission);
    void setStep_(int stepNum);
    void startWaiter_(void);
    void stopWaiter_(void);
    void updateMetadataDisplayLineCheckBox_(int selectedCheckBox);
    void updateMetadataFilterString_(void);
    void updateMissionUsageCounter_(void);
    void waitAndShowLoadedMissionDetail_(void);
    void waiterLoop_(void);
    bool sendMissionFeedbackToCBR_(void);
    bool sendMissionToCBR_(void);

    static Widget getTopShell_(Widget w);
    static XtTimerCallbackProc cbWaitAndShowLoadedMissionDetail_(XtPointer clientData);
    static void cbBackBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbCloseBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbDetailBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMaximizeBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMergeBtnPress_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataDisplayLineActionBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataDisplayLineCheckBox_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataDetailWindowOKBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataDetailWindowCancelBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataDetailWindowAssemblageBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataFilter_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataFilterClear_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataFilterStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMetadataReorder_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbNextBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbMissionFeedbackBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbRespecifyBtn_(Widget w, XtPointer clientData, XtPointer callbackData);
    static void cbSaveCBRBtn_(Widget w, XtPointer clientData, XtPointer callbackData);

public:
    static const int MAX_DATA_WINDOW_BUFSIZE;
    static const int MAX_INSTRUCTION_WINDOW_BUFSIZE;

    MissionSpecWizard(void);
    MissionSpecWizard(
        int wizardType,
        Display *display,
        Widget parent,
        XtAppContext app,
        MissionExpert *mexp,
        const symbol_table<rc_chain> &rcTable);
    virtual ~MissionSpecWizard(void);

    MExpRetrievedMission_t saveRetrievedMission(
        vector<MExpRetrievedMission_t> retrievedMissions,
        int maxSuitabilityRating,
        int *selectedMission);
    int wizardType(void);
    void notifyMissionModified(void);
    void popupDialog(void);
    void printDataWindow(char *str);
    void printfDataWindow(const char *format, ...);
    void printInstructionWindow(char *str);
    void printfInstructionWindow(const char *format, ...);
    void setCompilationFailed(void);
    void setDoneRehearsal(void);
    void setMissionLoaded(void);
    void setSpecificationCanceled(void);
    void start(void);
    void updateProgressBar(double level);
    void clearProgressBar(void);
    void setDisableRepair(bool disableRepair);
    bool dialogIsUp(void);
    bool noRehearsal(void);
};

inline int MissionSpecWizard::wizardType(void)
{
    return wizardType_;
}

inline void MissionSpecWizard::setSelectedMission_(int selectedMission)
{
    selectedMission_ = selectedMission;
}

inline void MissionSpecWizard::setMissionLoaded(void)
{
    missionLoaded_ = true;
}

inline bool MissionSpecWizard::noRehearsal(void)
{
    return noRehearsal_;
}

inline void MissionSpecWizard::setDisableRepair(bool disableRepair)
{
    disableRepair_ = disableRepair;
}

#endif
/**********************************************************************
 * $Log: mission_spec_wizard.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.9  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.8  2007/09/18 22:36:12  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.7  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.6  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.5  2007/08/04 23:52:55  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.4  2007/07/18 22:44:49  endo
 * Filter function added to the meta data display.
 *
 * Revision 1.3  2007/07/17 21:53:51  endo
 * Meta data sorting function added.
 *
 * Revision 1.2  2007/06/28 03:54:20  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.1  2007/05/15 18:50:01  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.8  2007/03/05 19:35:37  endo
 * The bug on Back button fixed.
 *
 * Revision 1.7  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.6  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.5  2006/09/22 18:43:04  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.4  2006/09/22 17:57:17  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.3  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.2  2006/09/15 22:37:40  endo
 * ICARUS Wizard compilation meter bars added.
 *
 * Revision 1.1  2006/09/13 19:03:48  endo
 * ICARUS Wizard implemented.
 *
 **********************************************************************/
