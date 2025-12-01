/**********************************************************************
 **                                                                  **
 **                          mission_expert.cc                       **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_expert.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

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
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

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

#include "mission_expert.h"
#include "assistantDialog.h"
#include "configuration.hpp"
#include "symbol.hpp"
#include "circle.hpp"
#include "globals.h"
#include "popups.h"
#include "write_cdl.h"
#include "cfgedit_cbrclient.h"
#include "toolbar.h"
#include "EventLogging.h"
#include "file_utils.h"
#include "renumber_robots.h"
#include "load_cdl.h"
#include "cnp_types.h"
#include "make.h"
#include "run.h"
#include "cim_management.h"
#include "sgig_management.h"
#include "mission_spec_wizard_factory.h"
#include "convert_scale.h"
#include "string_utils.h"
#include "bitmaps/star.bit"
#include "bitmaps/empty_star.bit"

using std::string;
using std::vector;
using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define MEXP_CREATE_PIXMAP(name,fg,bg,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, fg, bg, \
        DefaultDepthOfScreen(XtScreen(parent)))

//-----------------------------------------------------------------------
// Mutex initialization
//-----------------------------------------------------------------------
pthread_mutex_t MissionExpert::threadedXCommandsMutex_ = PTHREAD_MUTEX_INITIALIZER;

//-----------------------------------------------------------------------
// Static
//-----------------------------------------------------------------------
LaunchWizard *MissionExpert::launchWizard_ = NULL;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
// Window sizes
const int MissionExpert::DEFAULT_FEATUREMENU_WIDTH_ = 400;
const int MissionExpert::DEFAULT_FEATUREMENU_LINEHEIGHT_ = 32;
const int MissionExpert::DEFAULT_STATUSWINDOW_WIDTH_ = 350;
const int MissionExpert::DEFAULT_STATUSWINDOW_LINEHEIGHT_ = 32;
const int MissionExpert::DEFAULT_STATUSWINDOW_TOPTITLE_HEIGHT_ = 45;
const int MissionExpert::DEFAULT_STATUSWINDOW_TEXT_HEIGHT_ = 165;
const int MissionExpert::DEFAULT_STATUSWINDOW_TEXT_NUMROWS_ = 10;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_ACTIONSPACE_HEIGHT_ = 64;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_EXTRA_HEIGHT_ = 20;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_HEIGHT_ = 64;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_ITEM_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_ITEM_LABEL_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_NAME_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_WIDTH_ = 600;
const int MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_ACTIONSPACE_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_HEIGHT_ = 32;
const int MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_WIDTH_ = 450;

// Misc setups
const bool MissionExpert::SEND_FEEDBACKS_TO_LIBRARY_ = false;
const bool MissionExpert::SEND_METADATA_TO_LIBRARY_ = true;
const bool MissionExpert::INCREMENTALLY_UPDATE_LIBRARY_ = false;

// Strings
const string MissionExpert::RCTABLE_DISABLE_LAUNCH_WIZARD_STRING_ = "DisableLaunchWizard";
const string MissionExpert::RCTABLE_DISABLE_PREMISSION_CNP_STRING_= "DisablePremissionCNP";
const string MissionExpert::RCTABLE_DISABLE_REPAIR_STRING_= "DisableMissionExpertRepair";
const string MissionExpert::RCTABLE_DISABLE_RUNTIME_CNP_STRING_= "DisableRuntimeCNP";
const string MissionExpert::RCTABLE_DISABLE_STRING_= "DisableMissionExpert";
const string MissionExpert::RCTABLE_DATAFILESDIR_STRING_= "MExpDataFilesDir";
const string MissionExpert::DEFAULT_FEATUREMENU_TITLE_ = "Component Feature";
const string MissionExpert::DEFAULT_FEATUREMENU_TASK_LABEL_ = "Task";
const string MissionExpert::DEFAULT_FEATUREMENU_SAVE_BUTTON_LABEL_ = "Save";
const string MissionExpert::DEFAULT_FEATUREMENU_CANCEL_BUTTON_LABEL_ = "Cancel";
const string MissionExpert::DEFAULT_STATUSWINDOW_TITLE_ = "Loadable Mission Summary       ";
const string MissionExpert::DEFAULT_STATUSWINDOW_RATING_LABEL_ = "Suitability Rating  ";
const string MissionExpert::DEFAULT_STATUSWINDOW_NAVIGATION_LABEL_ = "Choose a Mission    ";
const string MissionExpert::DEFAULT_STATUSWINDOW_PREV_BUTTON_LABEL_ = "<< Previous   ";
const string MissionExpert::DEFAULT_STATUSWINDOW_NEXT_BUTTON_LABEL_ = "  Next >>";
const string MissionExpert::DEFAULT_STATUSWINDOW_LOAD_BUTTON_LABEL_ = "Load";
const string MissionExpert::DEFAULT_STATUSWINDOW_CANCEL_BUTTON_LABEL_ = "Cancel";
const string MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_CANCEL_BUTTON_LABEL_ = "Cancel";
const string MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_ = "Comment this mission";
const string MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_ = "Rate this mission";
const string MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_NAME_LABEL_ = "Your Name:";
const string MissionExpert::DEFAULT_METADATA_ENTRY_WINDOW_SAVE_BUTTON_LABEL_ = "Save";
const string MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_CANCEL_BUTTON_LABEL_ = "Cancel";
const string MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_LABEL_ = "File Name:";
const string MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_ = "The contents of the CBR library have changed. Do you wan to save it?";
const string MissionExpert::DEFAULT_SAVE_CBR_LIBRARY_WINDOW_SAVE_BUTTON_LABEL_ = "Save";
const string MissionExpert::ACDL_STRING_AGENT_NAME_ = "AGENT_NAME";
const string MissionExpert::ACDL_STRING_TASK_INDEX_ = "TASK_INDEX";
const string MissionExpert::ACDL_STRING_START_ = "Start";
const string MissionExpert::CDL_STRING_MAXVELOCITY_ = "max_vel";
const string MissionExpert::CDL_STRING_BASEVELOCITY_ = "base_vel";
const string MissionExpert::CDL_STRING_START_SUB_MISSION_ = "StartSubMission";
const string MissionExpert::FEATURE_NAME_NUMROBOTS_ = "NumberOfRobots";
const string MissionExpert::FEATURE_NAME_MAXVELOCITY_ = "MaxVelocity";
const string MissionExpert::FEATURE_NAME_AGGRESSIVENESS_ = "Aggressiveness";
const string MissionExpert::CONFIRM_SUCCESSFUL_MISSION_PHRASE_ = "Was the mission successful?";
const string MissionExpert::CONFIRM_VIEW_ANYWAY_PHRASE_ = "Current mission was not generated by\nMission Expert.\n\nView anyway?";
const string MissionExpert::ASK_USE_MEXP_PHRASE_ = "Do you want to use Mission Expert?";
const string MissionExpert::ASK_DIAGNOSIS_PHRASE_ = "Do you want to diagnose the failed mission plan?";
const string MissionExpert::ASK_CNP_PHRASE_ = "Do you want MissionLab to automatically assign tasks to\nrobots using contract-net protocols (CNP)?";
const string MissionExpert::ASK_CNP_PHRASE_SIMPLE_ = "Is this CNP-based mission?";
const string MissionExpert::ASK_PREMISSION_CNP_PHRASE_ = "Do you want CNP to assign tasks to robots before\nthe mission is executed? (Premission-CNP)";
const string MissionExpert::ASK_PREMISSION_CNP_PHRASE_SIMPLE_ = "Premission-CNP?";
const string MissionExpert::ASK_RUNTIME_CNP_PHRASE_ = "Do you want CNP to assign tasks to robots during\nthe mission is being executed? (Runtime-CNP)";
const string MissionExpert::ASK_RUNTIME_CNP_PHRASE_SIMPLE_ = "Runtime-CNP?";
const string MissionExpert::REPAIR_NO_SOLUTION_PHRASE_ = "Mission Expert could not find a solution to your problem.\nHowever, the possible offending state was highlighted here.";
const string MissionExpert::REPAIR_SOLUTION_PHRASE_ = "Mission Expert found a solution to your problem.\nDo you want to apply the change?";
const string MissionExpert::FSASUMMARY_EMPTY_PHRASE_ = "This is an empty mission.";
const string MissionExpert::FSASUMMARY_DEFAULT_PHRASE_ = "States/Triggers Used in this mission:\n\n";
const string MissionExpert::STRING_ADD_ = "Add";
const string MissionExpert::METADATA_LABELS_[NUM_MEXP_METADATA_RATINGS] =
{
    "Stealth (1 = Not Stealthy; 5 = Very Stealthy)", // MEXP_METADATA_RATING_STEALTH
    "Efficiency (1 = Not Efficient; 5 = Very Efficient)", // MEXP_METADATA_RATING_EFFICIENCY
    "Preservation (1 = Poor Preservation; 5 = Excellent Preservation)", // MEXP_METADATA_RATING_PRESERVATION
    "Duration (1 = Short Duration; 5 = Long Duration)" // MEXP_METADATA_RATING_DURATION
};
const string MissionExpert::RCTABLE_DISABLE_MISSION_SPEC_WIZARD_STRINGS_[NUM_MISSION_SPEC_WIZARD_TYPES] =
{
    "DisableBamsWizard", // MISSION_SPEC_WIZARD_BAMS
    "DisableIcarusWizard" // MISSION_SPEC_WIZARD_ICARUS
};

// Others
const double MissionExpert::REPLAY_DISPLAY_RATIO_ = 0.5;
const char MissionExpert::CHAR_NULL_ = '\0';
const int MissionExpert::MAX_METADATA_RATING_ = 5;
const int MissionExpert::MAX_SUITABILITY_RATING_ = 5;
const int MissionExpert::EVENT_WAITER_SLEEP_TIME_USEC_ = 1000;
const int MissionExpert::PROCESS_THREADED_X_COMMAND_TIMER_MSEC_ = 10;

//-----------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constructor for the MissionExpert class.
//-----------------------------------------------------------------------
MissionExpert::MissionExpert(
    Display *display,
    Widget parent,
    XtAppContext app,
    const symbol_table<rc_chain> &rcTable,
    string mexpRCFileName) :
    CBRWizard(display, parent, app, CNP_MODE_DISABLED),
    featureMenuOption_w_(NULL),
    featureMenuTask_w_(NULL),
    globalFeaturesMenuOption_w_(NULL),
    metadataEntryWindowData_(NULL),
    cfgeditCBRClient_(NULL),
    missionSpecWizard_(NULL),
    saveCBRLibraryWindowData_(NULL),
    dataFilesDirectory_(EMPTY_STRING_),
    missionTimeString_(EMPTY_STRING_),
    numVisibleGlobalFeatures_(0),
    numVisibleLocalFeatures_(0),
    keepDataFiles_(false),
    featuresLoaded_(false),
    featureMenuIsUp_(false),
    disableLaunchWizard_(true),
    disableRepair_(true)
{
    rc_chain *dataFilesDirList = NULL;
    char *dataFilesDir = NULL;

    rcTable_ = rcTable;

    if (mexpRCFileName == EMPTY_STRING_)
    {
        mexpRCFileName_ = MEXP_DEFAULT_RC_FILENAME;
    }
    else
    {
        mexpRCFileName_ = mexpRCFileName;
    }

    createPixmaps_(parentWidgetFg_, parentWidgetBg_, parentWidget_);
    
    // Use the MissionExpert unless specified not to.
    if (check_bool_rc(rcTable_, RCTABLE_DISABLE_STRING_.c_str()))
    {
        disableMExp_ = true;
    }
    else
    {
        disableMExp_ = false;

        // Disable or enable the MissionExpert's repairment feature unless specified not to.
        if (check_bool_rc(rcTable_, RCTABLE_DISABLE_REPAIR_STRING_.c_str()))
        {
            disableRepair_ = true;
        }
        else
        {
            disableRepair_ = false;
        }

        // Disable or enable the MissionExpert's launch wizard feature unless specified
        // not to.
        if (check_bool_rc(rcTable_, RCTABLE_DISABLE_LAUNCH_WIZARD_STRING_.c_str()))
        {
            disableLaunchWizard_ = true;
        }
        else
        {
            disableLaunchWizard_ = false;
        }
    }

    // Get the directory for saving output data files from the .cfgeditrc file.
    dataFilesDirList = (rc_chain *)rcTable_.get(RCTABLE_DATAFILESDIR_STRING_.c_str());

    if (dataFilesDirList != NULL)
    {
        dataFilesDirList->first(&dataFilesDir);

        if (dataFilesDir != NULL)
        {
            // Directory specified.
            dataFilesDirectory_ = dataFilesDir;
            keepDataFiles_ = true;
        }
    }

    statusWindows_.textPos = 0;
    statusWindows_.isUp = false;
    statusWindows_.currentMissionId = -1;
    statusWindows_.selectedMissionId = -1;
    statusWindows_.starBox = new Widget[MAX_SUITABILITY_RATING_];

    initialize();

    XtToolkitThreadInitialize();
    startProcessThreadedXCommands_();
}

//-----------------------------------------------------------------------
// Destructor for the MissionExpert class.
//-----------------------------------------------------------------------
MissionExpert::~MissionExpert(void)
{
    if (cbrClientInstantiated_)
    {
        saveCBRLibraryFile();
    }

    if (!keepDataFiles_)
    {
        deleteCreatedDataFiles_();
    }

    delete [] statusWindows_.starBox;
    statusWindows_.starBox = NULL;
}

//-----------------------------------------------------------------------
// This function initializes the class variables.
//-----------------------------------------------------------------------
void MissionExpert::initialize(void)
{
    if (disableMExp_)
    {
        return;
    }

    if (!keepDataFiles_)
    {
        deleteCreatedDataFiles_();
    }

    if (overlayFileName_ != EMPTY_STRING_)
    {
        overlayFileName_ = EMPTY_STRING_;
    }

    if (missionTimeString_ != EMPTY_STRING_)
    {
        missionTimeString_ = EMPTY_STRING_;
    }

    if (featureMenuIsUp_)
    {
        closeFeatureMenu_();
    }

    generalYesNoWindowDone_ = false;
    dataFilesNamed_ = false;
    cbrClientInstantiated_ = false;
    shouldSaveMissionToCBRLibrary_ = false;
    missionSavedToCBRLibrary_ = false;
    missionLoadedFromCBRLibrary_ = false;
    currentCBRLibraryDataIndexList_.clear();
    shXCommandUpdateStatesWindowList_.clear();
    shXCommandHighlightStatesList_.clear();
    shPendingThreadedXCommands_.clear();
    mexpRunMode_ = MEXP_RUNMODE_NONE;
    missionSpecWizardType_ = MISSION_SPEC_WIZARD_DISABLED;
    lastHighlightedStateInfo_ = EMPTY_STATE_INFO_;
    cnpMode_ = CNP_MODE_DISABLED;

    initializeMetadataEntryWindowData_(false);
}   

//-----------------------------------------------------------------------
// This function initializes the metadata entry window data.
//-----------------------------------------------------------------------
void MissionExpert::initializeMetadataEntryWindowData_(bool keepUserName)
{
    string userName = EMPTY_STRING_;
    int i;

    if (metadataEntryWindowData_ != NULL)
    {
        if (keepUserName)
        {
            userName = metadataEntryWindowData_->userName;
        }

        if (metadataEntryWindowData_->windowIsUp)
        {
            closeMetadataEntryWindow_();
        }

        delete metadataEntryWindowData_;
        metadataEntryWindowData_ = NULL;
    }

    metadataEntryWindowData_ = new MetadataEntryWindowData_t;

    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        metadataEntryWindowData_->ratingBtns[i] = new Widget[MAX_METADATA_RATING_+1];
    }

    metadataEntryWindowData_->userName = userName;
    memset(
        (void *)(metadataEntryWindowData_->ratings),
        0x0,
        sizeof(int)*NUM_MEXP_METADATA_RATINGS);
    metadataEntryWindowData_->windowIsUp = false;
}

//-----------------------------------------------------------------------
// This function asks users if they want to use the Mission Expert.
//-----------------------------------------------------------------------
bool MissionExpert::askUseMissionExpert(void)
{
    int i;
    bool *launchOptions = NULL;
    bool useMExp = false;

    if (disableMExp_)
    {
        return false;
    }

    if (!disableLaunchWizard_)
    {
        if (launchWizard_ == NULL)
        {
            launchWizard_ = new LaunchWizard(display_, parentWidget_, appContext_);
        }

        launchOptions = launchWizard_->runWizard();

        if (launchOptions == NULL)
        {
            fprintf(
                stderr,
                "Error: MissionExpert::askUseMissionExpert(). launchWizard_->runWizard() returned NULL.\n\a");
            return false;
        }
        
        useMExp = true;
        disableRepair_ = true;

        for (i = 0; i < NUM_LAUNCH_WIZARD_OPTIONS; i++)
        {
            if (launchOptions[i])
            {
                switch (i) {

                case LAUNCH_WIZARD_OPTION_FSA:
                    useMExp = false;
                    break;

                case LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD:
                    break;

                case LAUNCH_WIZARD_OPTION_MISSION_SPEC_WIZARD:
                    askMissionSpecWizardType_();
                    break;

                case LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP:
                    cnpMode_ = CNP_MODE_PREMISSION;
                    break;

                case LAUNCH_WIZARD_OPTION_MISSION_REPAIR:
                    disableRepair_ = false;
                    break;
                }
            }
        }
    }
    else
    {
        gEventLogging->start(ASK_USE_MEXP_PHRASE_.c_str());

        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_CONFIRM_MISSION_EXPERT);

        confirmUser_(ASK_USE_MEXP_PHRASE_, false);

        gEventLogging->log(
            "%s = %d",
            ASK_USE_MEXP_PHRASE_.c_str(),
            generalYesNoWindowResult_);
        gEventLogging->end("%s", ASK_USE_MEXP_PHRASE_.c_str());

        gAssistantDialog->clearMessage();

        useMExp = generalYesNoWindowResult_;
    }

    return useMExp;
}

//-----------------------------------------------------------------------
// This function asks users if the mission was successful or not. The
// result will be reported to the server.
//-----------------------------------------------------------------------
void MissionExpert::confirmSuccessfulMission(void)
{
    if (disableMExp_)
    {
        return;
    }

    gEventLogging->start(CONFIRM_SUCCESSFUL_MISSION_PHRASE_.c_str());

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_SUCCESS_DIALOGUE);

    confirmUser_(CONFIRM_SUCCESSFUL_MISSION_PHRASE_, false);

    if (SEND_FEEDBACKS_TO_LIBRARY_)
    {
        if (generalYesNoWindowResult_)
        {
            // It was successful.
            cfgeditCBRClient_->savePositiveFeedback(currentCBRLibraryDataIndexList_);
        }
        else
        {
            // It was unsuccessful.
            cfgeditCBRClient_->saveNegativeFeedback(currentCBRLibraryDataIndexList_);
        }
    }

    gEventLogging->log(
        "%s = %d",
        CONFIRM_SUCCESSFUL_MISSION_PHRASE_.c_str(),
        generalYesNoWindowResult_);
    gEventLogging->end(CONFIRM_SUCCESSFUL_MISSION_PHRASE_.c_str());

    if (!generalYesNoWindowResult_)
    {
        askDiagnosis_();
    }
    else
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
    }
}

//-----------------------------------------------------------------------
// This function asks users if they want to diagnose the failed mission.
//-----------------------------------------------------------------------
void MissionExpert::askDiagnosis_(void)
{
    if (disableMExp_)
    {
        return;
    }

    gEventLogging->start(ASK_DIAGNOSIS_PHRASE_.c_str());

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_ASK_DIAGNOSIS);

    confirmUser_(ASK_DIAGNOSIS_PHRASE_, false);

    gEventLogging->log(
        "%s = %d",
        ASK_DIAGNOSIS_PHRASE_.c_str(),
        generalYesNoWindowResult_);
    gEventLogging->end(ASK_DIAGNOSIS_PHRASE_.c_str());

    if (generalYesNoWindowResult_)
    {
        mexpRunMode_ = MEXP_RUNMODE_REPLAY;
        //gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ASSEMBLAGE);
        runMlabReplay_();
    }
    else
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE_NO);
    }
}

//-----------------------------------------------------------------------
// This function asks users if they want to enable CNP mode.
//-----------------------------------------------------------------------
void MissionExpert::askCNPMode_(bool simpleCheck)
{
    string phrase;
    bool disablePremissionCNP = false;
    bool disableRuntimeCNP = false;
    const bool DEBUG_ASK_CNP_MODE = false;

    if (disableMExp_)
    {
        cnpMode_ = CNP_MODE_DISABLED;
        return;
    }

    // First, check .cfgeditrc.
    if (check_bool_rc(rcTable_, RCTABLE_DISABLE_PREMISSION_CNP_STRING_.c_str()))
    {
        disablePremissionCNP = true;
    }

    if (check_bool_rc(rcTable_, RCTABLE_DISABLE_RUNTIME_CNP_STRING_.c_str()))
    {
        disableRuntimeCNP = true;
    }

    if (disablePremissionCNP && disableRuntimeCNP)
    {
        // CNP totally disabled.
        cnpMode_ = CNP_MODE_DISABLED;
        return;
    }

    // Check to see whether the user wants to use CNP at all.
    if (simpleCheck)
    {
        phrase = ASK_CNP_PHRASE_SIMPLE_;
    }
    else
    {
        phrase = ASK_CNP_PHRASE_;
    }

    confirmUser_(phrase, false);

    if (!generalYesNoWindowResult_)
    {
        // CNP not wanted.
        cnpMode_ = CNP_MODE_DISABLED;
        return;
    }

    if (!disablePremissionCNP)
    {
        // Check to see whether the user wants to use premission-CNP.
        if (simpleCheck)
        {
            phrase = ASK_PREMISSION_CNP_PHRASE_SIMPLE_;
        }
        else
        {
            phrase = ASK_PREMISSION_CNP_PHRASE_;
        }

        confirmUser_(phrase, false);

        if (!generalYesNoWindowResult_)
        {
            disablePremissionCNP = true;
        }
        else
        {
            // Note:
            //  CNP_MODE_PREMISSION_AND_RUNTIME not yet supported.
            //  Once implemented, this is not necessary as both
            //  disablePremissionCNP and disableRuntimeCNP can
            //  be false.
            disableRuntimeCNP = true;
        }
    }

    if (!disableRuntimeCNP)
    {
        // Check to see whether the user wants to use runtime-CNP.
        if (simpleCheck)
        {
            phrase = ASK_RUNTIME_CNP_PHRASE_SIMPLE_;
        }
        else
        {
            phrase = ASK_RUNTIME_CNP_PHRASE_;
        }

        confirmUser_(phrase, false);

        if (!generalYesNoWindowResult_)
        {
            disableRuntimeCNP = true;
        }
    }

    if ((!disablePremissionCNP) && (!disableRuntimeCNP))
    {
        cnpMode_ = CNP_MODE_PREMISSION_AND_RUNTIME;
    }
    else if (!disablePremissionCNP)
    {
        cnpMode_ = CNP_MODE_PREMISSION;
    }
    else if (!disableRuntimeCNP)
    {
        cnpMode_ = CNP_MODE_RUNTIME;
    }
    else
    {
        warn_userf("CNP disabled.");

        while(gWarningUser)
        {
            // Run X
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

        cnpMode_ = CNP_MODE_DISABLED;
    }

    if (DEBUG_ASK_CNP_MODE)
    {
        fprintf(
            stderr,
            "MissionExpert::askCNPMode_(): CNP Mode(%d) [%s]\n",
            cnpMode_,
            CNP_MODE_STRING[cnpMode_].c_str());
    }
}

//-----------------------------------------------------------------------
// This function asks users what type of MissionSpecWizard to be enabled.
//-----------------------------------------------------------------------
void MissionExpert::askMissionSpecWizardType_(void)
{
    char buf[1024];
    string phrase;
    vector<int> enabledTypes;
    int i, wizardType;

    if (disableMExp_)
    {
        missionSpecWizardType_ = MISSION_SPEC_WIZARD_DISABLED;
        return;
    }

    for (i = 0; i < NUM_MISSION_SPEC_WIZARD_TYPES; i++)
    {
        if (check_bool_rc(rcTable_, RCTABLE_DISABLE_MISSION_SPEC_WIZARD_STRINGS_[i].c_str()))
        {
            // Disabled.
        }
        else
        {
            enabledTypes.push_back(i);
        }
    }

    if (enabledTypes.size() == 0)
    {
        missionSpecWizardType_ = MISSION_SPEC_WIZARD_DISABLED;
        return;
    }
    else if (enabledTypes.size() == 1)
    {
        missionSpecWizardType_ = enabledTypes.back();
        return;
    }

    for (i = 0; i < (int)(enabledTypes.size()); i++)
    {
        // Create the phrase.
        wizardType = enabledTypes[i];
        sprintf(buf, "Multiple Misssion-Specification Wizards were enabled.\n");
        sprintf(
            buf,
            "%sHowever, only one can be used.\n\nUse %s?",
            buf,
            MISSION_SPEC_WIZARD_NAMES[wizardType].c_str());
        phrase = buf;
        
        // Ask use.
        confirmUser_(phrase, false);

        if (generalYesNoWindowResult_)
        {
            missionSpecWizardType_ = wizardType;
            return;
        }
    }

    warn_userf("Misssion-Specification Wizard disabled.");

    while(gWarningUser)
    {
        // Run X
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

    missionSpecWizardType_ = MISSION_SPEC_WIZARD_DISABLED;
}

//-----------------------------------------------------------------------
// This function asks users if they want to use the Mission Expert.
//-----------------------------------------------------------------------
bool MissionExpert::confirmViewAnyway_(void)
{
    gEventLogging->start(CONFIRM_VIEW_ANYWAY_PHRASE_.c_str());
    confirmUser_(CONFIRM_VIEW_ANYWAY_PHRASE_, false);
    gEventLogging->log("View anyway = %d", generalYesNoWindowResult_);
    gEventLogging->end("Current mission was not generated by Mission Expert. View anyway?");

    return generalYesNoWindowResult_;
}

//-----------------------------------------------------------------------
// This function creates a unique prefix for output files, based on the
// user name, date, and time.
//-----------------------------------------------------------------------
string MissionExpert::createPrefix_(void)
{
    time_t cur_time;
    tm local_time;
    string prefix;
    char date[100], month[100], day[100], year[100], hour[100], min[100], sec[100];
    char *userName = NULL, buf[1024];

    // Get the user name.
    userName = getenv("USER");

    // Get the current date and time.
    cur_time = time(NULL);
    strncpy(date,ctime(&cur_time),sizeof(date));
    local_time = *localtime(&cur_time);
    strftime(month, sizeof(month), "%m", &local_time);
    strftime(day, sizeof(day), "%d", &local_time);
    strftime(year, sizeof(year), "%Y", &local_time);
    strftime(hour, sizeof(hour), "%H", &local_time);
    strftime(min, sizeof(min), "%M", &local_time);
    strftime(sec, sizeof(sec), "%S", &local_time);

    // Create the prefix.
    sprintf(buf, "mexp-%s-%s%s%s-%s%s%s",
            userName,
            month,
            day,
            year,
            hour,
            min,
            sec);
    prefix = buf;

    return prefix;
}

//-----------------------------------------------------------------------
// This function creates names for various files that will be used in
// Mission Expert.
//-----------------------------------------------------------------------
void MissionExpert::nameDataFiles_(void)
{
    string prefix = EMPTY_STRING_;
    string defaultPrefix;

    // Create a default prefix.
    defaultPrefix = createPrefix_();

    // Create the prefix of the file (including the directory path if appropriate)
    if (dataFilesDirectory_ != EMPTY_STRING_)
    {
        prefix = dataFilesDirectory_;
        prefix += "/";
    }
    prefix += defaultPrefix;

    // Create the feature file string to be sent to mlab.
    featureFileString_ = prefix;
    featureFileString_ += ".";
    featureFileString_ += MEXP_STRING_FEATURE;

    // Name the feature file.
    featureFileName_ = featureFileString_;
    featureFileName_ += ".";
    featureFileName_ += MEXP_EXTENSION_FEATUREFILENAME;

    // Name the feature status file.
    featureStatusFileName_ = featureFileString_;
    featureStatusFileName_ += ".";
    featureStatusFileName_ += MEXP_EXTENSION_FEATURESTATUSFILENAME;

    // Name the result file for retrieving.
    resultFileName_ = prefix;
    resultFileName_ += ".";
    resultFileName_ += MEXP_STRING_RETRIEVE;
    resultFileName_ += ".";
    resultFileName_ += MEXP_EXTENSION_CDL;

    // Name the case file for adding.
    missionPlanFileName_ = prefix;
    missionPlanFileName_ += ".";
    missionPlanFileName_ += MEXP_STRING_ADD;
    missionPlanFileName_ += ".";
    missionPlanFileName_ += MEXP_EXTENSION_ACDL;

    // Name the robot.
    robotName_ = defaultPrefix;
    robotName_ += MEXP_STRING_ROBOT;

    // Set the flag.
    dataFilesNamed_ = true;
}

//-----------------------------------------------------------------------
// This function creates names for various files that based on the
// use-selected data file.
//-----------------------------------------------------------------------
void MissionExpert::saveSpecifiedFeatureFileName(string featureFileName)
{
    string prefix = EMPTY_STRING_;
    string bufString, subBufString, targetString;
    string::size_type index;

    bufString = featureFileName;
    targetString = ".";
    targetString += MEXP_EXTENSION_FEATUREFILENAME;

    // Make sure that the extension is '.data'.
    index = bufString.size() - targetString.size();
    subBufString = bufString.substr(index, string::npos);

    if ((index < 0) || (subBufString != targetString))
    {
        fprintf(stderr, "Error: MissionExpert::saveSepcifiedFeatureFileName().\n");
        fprintf(stderr, "Filename does not contrain '%s'.\a\n", targetString.c_str());
        return;
    }

    // Get the string before the string
    bufString = bufString.substr(0, index);

    // Check again to see if the file name was ended as '.feature.data'.
    targetString = ".";
    targetString += MEXP_STRING_FEATURE;
    index = bufString.size() - targetString.size();
    subBufString = bufString.substr(index, string::npos);

    if ((index < 0) || (subBufString != targetString))
    {
        // It didn't end with '.feature.data'.
        prefix = bufString;
        featureFileString_ = prefix;
    }
    else
    {
        prefix = bufString.substr(0, index);
        featureFileString_ = prefix;
        featureFileString_ += ".";
        featureFileString_ += MEXP_STRING_FEATURE;
    }

    // Name the feature file.
    featureFileName_ = featureFileString_;
    featureFileName_ += ".";
    featureFileName_ += MEXP_EXTENSION_FEATUREFILENAME;

    // Name the feature status file.
    featureStatusFileName_ = featureFileString_;
    featureStatusFileName_ += ".";
    featureStatusFileName_ += MEXP_EXTENSION_FEATURESTATUSFILENAME;

    // Name the result file for retrieving.
    resultFileName_ = prefix;
    resultFileName_ += ".";
    resultFileName_ += MEXP_STRING_RETRIEVE;
    resultFileName_ += ".";
    resultFileName_ += MEXP_EXTENSION_CDL;

    // Name the case file for adding.
    missionPlanFileName_ = prefix;
    missionPlanFileName_ += ".";
    missionPlanFileName_ += MEXP_STRING_ADD;
    missionPlanFileName_ += ".";
    missionPlanFileName_ += MEXP_EXTENSION_ACDL;

    // Name the robot. Get rid of the directory path.
    index = prefix.find_last_of('/');

    if (index != string::npos)
    {
        // Get the string after '/'.
        prefix = prefix.substr(index+1, string::npos);
    }
    robotName_ = prefix;
    robotName_ += MEXP_STRING_ROBOT;

    // Set the flag.
    dataFilesNamed_ = true;
}

//-----------------------------------------------------------------------
// This function initializs the CBRClient class. 
//-----------------------------------------------------------------------
bool MissionExpert::initCBRClient_(void)
{
    bool initialized;

    cfgeditCBRClient_ = new CfgEditCBRClient(rcTable_);
    cbrclient_ = (CBRClient *)cfgeditCBRClient_;
    initialized = cbrclient_->initialize();

    if (!initialized)
    {
        warn_userf("CBRServer unreachable. Please make sure it is running.\n");
        return false;
    }

    cbrClientInstantiated_ = true;

    return true;
}

//-----------------------------------------------------------------------
// This function forks mlab in order for the user to select the mission.
//-----------------------------------------------------------------------
void MissionExpert::runMlabMissionDesign_(void)
{
    int pid, p;
    char *prog, *argv[32], buf[256];
    bool initialized = false, forkFailed = false;
    bool eventLogIsON;
    const char *eventLogfileName = gEventLogging->getLogfileName();
    config_fsa_lists_t fsaLists;

    if (disableMExp_)
    {
        return;
    }

    eventLogIsON = gEventLogging->eventLoggingIsON();

    if (!cbrClientInstantiated_)
    {
        // Instantiate the CBRClient first.
        initialized = initCBRClient_();

        if (!initialized)
        {
            // Something went wrong during the initialization.
            // Just return.
            return;
        }
    }

    popupStatusWindow_(
        0,
        "Searching for a mission. Please wait...",
        false,
        false,
        false,
        false,
        -1);

    // Set executable name
    p = 0;
    prog = strdup("mlab");
    argv[p++] = prog;
    argv[p++] = "-M";
    argv[p++] = strdup(featureFileString_.c_str());

    if (gAutomaticExecution)
    {
        argv[p++] = "-a";
    }

    if (gAssistantDialog->dialogIsUp())
    {
        argv[p++] = "-A";
    }

    // Usability log
    if(eventLogIsON)
    {
        argv[p++] = "-e";
        sprintf(buf,"%s", eventLogfileName);
        argv[p++] = strdup(buf);
        gEventLogging->start
            ("MExp fork mlab -M %s -e %s",
             featureFileString_.c_str(),
             eventLogfileName);
        gEventLogging->pause();
    }

    // User defined RC file
    if (special_rc_file)
    {
        sprintf(buf,"-c");
        argv[p++] = strdup(buf);
        sprintf(buf,"%s", rc_filename);
        argv[p++] = strdup(buf);
    }
      
    // CNP option.
    if (cnpMode_ != CNP_MODE_DISABLED)
    {
        sprintf(buf,"-N");
        argv[p++] = strdup(buf);
        sprintf(buf,"%s", CNP_MODE_STRING[cnpMode_].c_str());
        argv[p++] = strdup(buf);
    }

    // MissionSpecWizard type.
    if (missionSpecWizardEnabled())
    {
        if ((launchWizard_ != NULL) &&
            ((launchWizard_->launchOptions()[LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD]) ||
             (launchWizard_->launchOptions()[LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP])))
        {
            // Use Iconic CBR.
        }
        else
        {
            argv[p++] = "-U";
            sprintf(buf,"%s", MISSION_SPEC_WIZARD_NAMES[missionSpecWizardType_].c_str());
            argv[p++] = strdup(buf);
        }

        if (overlayFileName_ != EMPTY_STRING_)
        {
            sprintf(buf,"-o");
            argv[p++] = strdup(buf);
            sprintf(buf,"%s", overlayFileName_.c_str());
            argv[p++] = strdup(buf);
        }

        if (missionTimeString_ != EMPTY_STRING_)
        {
            sprintf(buf,"-t");
            argv[p++] = strdup(buf);
            sprintf(buf,"%s", missionTimeString_.c_str());
            argv[p++] = strdup(buf);
        }
    }

    // MExp RC file
    if (mexpRCFileName_ != MEXP_DEFAULT_RC_FILENAME)
    {
        sprintf(buf,"-E");
        argv[p++] = strdup(buf);
        sprintf(buf,"%s", mexpRCFileName_.c_str());
        argv[p++] = strdup(buf);
    }

    // Add NULL at the end of the command
    argv[p] = NULL;

    if ((pid = fork()) == 0)
    {
        // In child.
        if (execvp(prog, argv))
        {
            forkFailed = true;
        }      

        // Kill this child
        exit(2);
    }

    if ((pid == -1) || forkFailed)
    {
        gEventLogging->resume(false);
        gEventLogging->end("MExp fork mlab");
        fprintf(stderr, "Error(cfgedit): MissionExpert::runMlabMissionDesign_().\nUnable to fork %s\n", prog);
        config->quit();
    }
    else
    {
        startCheckForkedMlabStatus_(pid, eventLogIsON);
        return;
    }

    if(eventLogIsON)
    {
        gEventLogging->resume(false);
        gEventLogging->end("MExp fork mlab");
    }

    resumeMissionExpert_();
}

//-----------------------------------------------------------------------
// This function forks mlab in order for the user to replay the mission.
//-----------------------------------------------------------------------
void MissionExpert::runMlabReplay_(void)
{
    config_fsa_lists_t fsaLists;
    int pid, p;
    char *prog, *argv[32], buf[1024];
    char displayWidthBuf[1024], displayHeightBuf[1024], geometryBuf[1024];
    bool initialized = false, forkFailed = false;
    bool eventLogIsON;
    int screenNumber;
    double displayWidth, displayHeight;
    const char *eventLogfileName = gEventLogging->getLogfileName();
    const bool SCALE_MLAB = false;

    if (disableMExp_)
    {
        return;
    }

    eventLogIsON = gEventLogging->eventLoggingIsON();
    lastHighlightedStateInfo_ = EMPTY_STATE_INFO_;

    if (!cbrClientInstantiated_)
    {
        // Instantiate the CBRClient first.
        initialized = initCBRClient_();

        if (!initialized)
        {
            // Something went wrong during the initialization.
            // Just return.
            return;
        }
    }

    // Set executable name
    p = 0;
    prog = strdup("mlab");
    argv[p++] = prog;
    
    if (SCALE_MLAB)
    {
        // Set the geometry
        screenNumber = DefaultScreen(display_);

        displayWidth = (double)(DisplayWidth(display_, screenNumber));
        displayWidth *= REPLAY_DISPLAY_RATIO_;
        sprintf(displayWidthBuf, "%d", (int)displayWidth);

        displayHeight = (double)(DisplayHeight(display_, screenNumber));
        displayHeight *= REPLAY_DISPLAY_RATIO_;
        sprintf(displayHeightBuf, "%d", (int)displayHeight);

        sprintf(geometryBuf, "%dx%d+0-0", (int)displayWidth, (int)displayHeight);

        argv[p++] = "-geometry";
        argv[p++] = strdup(geometryBuf);
        argv[p++] = "-W";
        argv[p++] = strdup(displayWidthBuf);; 
        argv[p++] = "-H";
        argv[p++] = strdup(displayHeightBuf);
    }

    argv[p++] = "-B";
    argv[p++] = "-M";
    argv[p++] = strdup(featureFileString_.c_str());

    if (gAssistantDialog->dialogIsUp())
    {
        argv[p++] = "-A";
    }

    // Usability log
    if(eventLogIsON)
    {
        argv[p++] = "-e";
        sprintf(buf,"%s", eventLogfileName);
        argv[p++] = strdup(buf);
        gEventLogging->start
            ("MExp fork mlab -B -M %s -e %s",
             featureFileString_.c_str(),
             eventLogfileName);
        gEventLogging->pause();
    }

    // CNP option.
    if (cnpMode_ > CNP_MODE_DISABLED)
    {
        sprintf(buf,"-N");
        argv[p++] = strdup(buf);
        sprintf(buf,"%s", CNP_MODE_STRING[cnpMode_].c_str());
        argv[p++] = strdup(buf);
    }

    // Add NULL at the end of the command
    argv[p] = NULL;

    if ((pid = fork()) == 0)
    {
        // In child.
        if (execvp(prog, argv)) forkFailed = true;
      
        // Kill this child
        exit(2);
    }

    if ((pid == -1) || forkFailed)
    {
        gEventLogging->resume(false);
        gEventLogging->end("MExp fork mlab");
        fprintf(stderr, "Error(cfgedit): MissionExpert::runMlabMissionDesign_().\nUnable to fork %s\n", prog);
        config->quit();
    }
    else
    {
        startCheckForkedMlabStatus_(pid, eventLogIsON);
        return;
    }

    if(eventLogIsON)
    {
        gEventLogging->resume(false);
        gEventLogging->end("MExp fork mlab");
    }

    resumeMissionExpert_();
}

//-----------------------------------------------------------------------
// This function checks the feature status file being sent by mlab to see
// if reconfiguration was specified or not.
//-----------------------------------------------------------------------
int MissionExpert::checkFeatureStatusFile_(void)
{
    FILE *featureFile = NULL, *backupFile = NULL;
    char buf[1024];
    string bufString, backupFeatureFilename;
    int status = CANCEL;

    // Check the status.
    featureStatusFile_ = fopen(featureStatusFileName_.c_str(), "r");

    if (featureStatusFile_ == NULL)
    {
        return status;
    }
    else 
    {
        addCreatedDataFileList_(featureStatusFileName_);
    }

    if (fscanf(featureStatusFile_, "%s", buf) != EOF)
    {
        bufString = buf;

        if (bufString == MEXP_STRING_CONFIGURE)
        {
            // Make sure the feature file is readable.
            featureFile = fopen(featureFileName_.c_str(), "r");

            if (featureFile == NULL)
            {
                status = CANCEL;
            }
            else
            {
                fclose(featureFile);
                addCreatedDataFileList_(featureFileName_);
                status = CONFIGURE;
            }
        }
        else if (bufString == MEXP_STRING_REPAIR)
        {
            status = REPAIR;
        }
        else if (bufString == MEXP_STRING_REPAIR_FAILURE)
        {
            status = REPAIR_FAILURE;
        }
    }

    fclose(featureStatusFile_);

    // Remember backup file as well.
    backupFeatureFilename = featureFileName_;
    backupFeatureFilename += ".";
    backupFeatureFilename += MEXP_EXTENSION_BACKUP;
    backupFile = fopen(backupFeatureFilename.c_str(), "r");
    if (backupFile != NULL)
    {
        fclose(backupFile);
        backupFile = NULL;
        addCreatedDataFileList_(backupFeatureFilename);
    }

    return status;
}

//-----------------------------------------------------------------------
// This function retrieve the case from the CBRServer based on the featurees
// being specified by the user.
//-----------------------------------------------------------------------
bool MissionExpert::buildMission_(void)
{
    MExpCNPStatus_t cnpStatus;
    MExpRetrievedMission_t retrievedMission;
    string overlayFileName = EMPTY_STRING_;
    string summary;
    vector<MExpRetrievedMission_t> retrievedMissions;
    vector<MExpRobotIDs_t> robotIDList;
    int index, rating, selectedIndex;
    int featureStatus;
    bool missionLoaded = false;
    bool nextBtnIsSensitive = false;
    bool loadBtnIsSensitive = false;
    bool reload = false;
    bool addStartSubMission = true;

    // Check the feature status file being sent by mlab to see if the
    // mission has to be rebuilt with CBR.
    featureStatus = checkFeatureStatusFile_();
    if (featureStatus != CONFIGURE)
    {
        if ((featureStatus == CANCEL) &&
            missionSpecWizardEnabled() &&
            (missionSpecWizard_ != NULL))
        {
            missionSpecWizard_->setSpecificationCanceled();
        }

        return false;
    }

    // Load the features from the Mission Expert RC file.
    gEventLogging->start("MExp Building a mission");

    while (true)
    {
        if ((!featuresLoaded_) || reload)
        {
            featuresLoaded_ = loadFeaturesFromRCFile_();

            if (!featuresLoaded_)
            {
                warn_userf(
                    "Mission Expert RC file \"%s\" could not be loaded.\n",
                    mexpRCFileName_.c_str());
                return false;
            }
        }
    
        addStartSubMission = true;

        if (missionSpecWizardEnabled())
        {
            if ((launchWizard_ != NULL) &&
                ((launchWizard_->launchOptions()[LAUNCH_WIZARD_OPTION_ICONIC_CBR_WIZARD]) ||
                 (launchWizard_->launchOptions()[LAUNCH_WIZARD_OPTION_CBR_PREMISSION_CNP])))
            {
                // Use Iconic CBR.
            }
            else
            {
                addStartSubMission = false;
            }
        }

        // Retrieve the case from CBRServer via CBRClient.
        retrievedMissions = cfgeditCBRClient_->retrieveMissionPlan(
            featureFileName_,
            defaultTaskList_,
            defaultGlobalFeatures_,
            defaultLocalFeatures_,
            defaultTaskWeight_,
            MAX_SUITABILITY_RATING_,
            addStartSubMission,
            &overlayFileName,
            &cnpMode_,
            &robotIDList,
            &cnpStatus,
            &reload);

        if (!reload)
        {
            break;
        }
    }

    switch (cnpMode_) {

    case CNP_MODE_PREMISSION:
        switch (cnpStatus.status) {

        case MEXP_CNP_OUTPUT_NORMAL:
        case MEXP_CNP_OUTPUT_EXCESSIVE_ROBOT:
            // Do nothing.
            break;

        case MEXP_CNP_OUTPUT_INCOMPLETE_MAPPING:
        case MEXP_CNP_OUTPUT_EXCESSIVE_ROBOT_AND_INCOMPLETE_MAPPING:
            warn_userf("One or more tasks were not assigned due to\nthe lack of qualified robots.");
            break;

        case MEXP_CNP_OUTPUT_UNEXPECTED_ERROR:
            warn_userf("Unexpected error has occured when assigning\nthe tasks to the robots.");
            break;
        }
        break;
    }

    if (missionSpecWizardEnabled() && (missionSpecWizard_ != NULL))
    {
        retrievedMission = missionSpecWizard_->saveRetrievedMission(
            retrievedMissions,
            MAX_SUITABILITY_RATING_,
            &selectedIndex);

        closeStatusWindow_();

        if (selectedIndex < 0)
        {
            // Loading mission aborted.
            return false;
        }

        missionLoaded = loadRetrievedMission_(
            retrievedMission,
            overlayFileName);
    }
    else
    {
        if ((int)(retrievedMissions.size()) > 0)
        {
            retrievedMissionSummary_ = summarizeRetrievedMissions(retrievedMissions);

            // Load the summary of the first mission.
            index = 0;
            summary = retrievedMissionSummary_.fsaSummaries[index];

            if (summary != FSASUMMARY_EMPTY_PHRASE_)
            {
                loadBtnIsSensitive = true;
                rating = retrievedMissionSummary_.ratings[index];
            }
            else
            {
                loadBtnIsSensitive = false;
                rating = 0;
            }

            nextBtnIsSensitive = ((int)(retrievedMissions.size()) > 1)? true : false;

            pendXCommandUpdateStatusWindow_(
                rating,
                summary,
                false,
                nextBtnIsSensitive,
                loadBtnIsSensitive,
                true,
                index);

            gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_SUITABILITY);

            if (gAutomaticExecution)
            {
                selectedIndex = 0;
                closeStatusWindow_();

                // Load the first retrived mission.
                missionLoaded = loadRetrievedMission_(
                    retrievedMissions[selectedIndex],
                    overlayFileName);
            }
            else
            {
                // Loads the mission that was selected by the user from the summary
                // (i.e., wait for the user input to the status window).
                missionLoaded = loadRetrievedMissionChosenByUser_(
                    retrievedMissions,
                    overlayFileName,
                    &selectedIndex);
            }

            gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_MISSION_RETRIEVED);
        }
        else
        {
            // No mission was returned...
            pendXCommandUpdateStatusWindow_(
                0,
                "Sorry, no suitable mission was found.",
                false,
                false,
                false,
                true,
                -1);

            missionLoaded = false;
        }
    }

    if (missionLoaded)
    {
        missionLoadedFromCBRLibrary_ = true;
        shouldSaveMissionToCBRLibrary_ = false;

        currentRobotIDs_.robotIDs.clear();

        switch (cnpMode_) {

        case CNP_MODE_PREMISSION:
        case CNP_MODE_RUNTIME:
            currentRobotIDs_ = robotIDList[selectedIndex];
            break;
        }
    }

    gEventLogging->end("MExp Building a mission");

    return missionLoaded;
}

//-----------------------------------------------------------------------
// This function will attempt to fix the fault mission.
//-----------------------------------------------------------------------
bool MissionExpert::repairMission_(void)
{
    FILE *resultFile = NULL;
    Widget w;
    vector<MExpRetrievedMission_t> retrievedMissions;
    char *cdlSolution = NULL;
    int status;
    bool missionIsFixed = false, saved = false, hasStartSubMission = false;

    // Check the repair status file being sent by mlab
    status = checkFeatureStatusFile_();

    switch (status) {

    case CANCEL:
    case CONFIGURE:
        config->clearHighlightStates();
        return false;

    case REPAIR:
        break;

    case REPAIR_FAILURE:
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_NO_REPAIR_SOLUTION_FOUND);
        warn_userf(REPAIR_NO_SOLUTION_PHRASE_.c_str());
        return false;
    }

    // Check to see if the user wants to apply the change.
    w = createGeneralYesNoWindow_(REPAIR_SOLUTION_PHRASE_);
    popupGeneralYesNoWindow_(w);

    gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_REPAIR_SOLUTION_FOUND);

    while(!generalYesNoWindowDone_)
    {
        // Run X
        if (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }          
    }

    if (!generalYesNoWindowResult_)
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE_NO);
        return false;
    }

    if (!dataFilesNamed_)
    {
        nameDataFiles_();
    }

    // Save the mission to a file.
    saved = saveAbstractedMissionPlanToFile_(
        config->root(),
        missionPlanFileName_,
        &hasStartSubMission);

    if (!saved)
    {
        return false;
    }


    // Send the current mission to the server, and ask it to
    // fix the mission.
    retrievedMissions = cfgeditCBRClient_->fixMissionPlan(missionPlanFileName_);

    if ((int)(retrievedMissions.size()) == 0)
    {
        warn_userf("Mission could not be repaired.\n");
        return false;
    }

    // Open the result file.
    resultFile = fopen(resultFileName_.c_str(), "w");
    if (resultFile == NULL)
    {
        warn_userf("Mission could not be repaired.\n");
        return false;
    }
    else
    {
        addCreatedDataFileList_(resultFileName_);
    }

    // For this case, CBRServer should return only one solution.
    // Use the first elment in the list.
    cdlSolution = acdl2cdl(
        retrievedMissions[0].acdlSolution.c_str(),
        robotName_,
        &status);

    if ((cdlSolution != NULL) && (status != ACDL2CDL_STATUS_FAILURE))
    {
        if (status == ACDL2CDL_STATUS_EMPTYMISSION)
        {
            warn_userf("Retrieved fixed mission is empty.");
        }

        // Copy the solution to the result file.
        fprintf(resultFile, "%s", cdlSolution);

        missionIsFixed = true;
        currentCBRLibraryDataIndexList_ = retrievedMissions[0].dataIndexList;
    }
    else
    {
        warn_userf("Retrieved mission cound not be convered to CDL.");
        missionIsFixed = false;
    }
    
    fclose(resultFile);

    if (missionIsFixed)
    {
        missionLoadedFromCBRLibrary_ = true;
        shouldSaveMissionToCBRLibrary_ = false;
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE);
    }

    return missionIsFixed;
}

//-----------------------------------------------------------------------
// This creates a summary of the retrieved mission to be posted on the
// summary window.
//-----------------------------------------------------------------------
MExpRetrievedMissionSummary_t MissionExpert::summarizeRetrievedMissions(
    const vector<MExpRetrievedMission_t> retrievedMissions)
{
    FILE *tmpACDLFile = NULL;
    MExpRetrievedMissionSummary_t summary;
    const MExpRetrievedMissionSummary_t EMPTY_SUMMARY = {vector<int>(), vector<string>()};
    char tmpACDLFilename[256];
    char buf[1024], buf1[1024], buf2[1024];
    string bufString1, bufString2;
    string fsaSummary;
    string lastTaskIndex, fsaName;
    vector<string> fsaNames;
    int i, j, rating, numMissions = 0, count;
    int fd;
    vector<int> counts;
    bool nameFound = false;

    numMissions = retrievedMissions.size();

    if (numMissions > 0)
    {
        // Create a temporary file (to dump the FSA summay later).
        sprintf(tmpACDLFilename, "/tmp/%s-cfgedit-summarizeRetrievedMissions-acdl-XXXXXX", getenv("USER"));
        fd = mkstemp(tmpACDLFilename);
        unlink(tmpACDLFilename);
    }
    else
    {
        return EMPTY_SUMMARY;
    }

    for (i = 0; i < numMissions; i++)
    {
        // Copy rating
        rating = retrievedMissions[i].rating;
        summary.ratings.push_back(rating);

        // Get the FSA summary. Dump the ACDL code in a temporary file first.
        tmpACDLFile = fopen(tmpACDLFilename, "w");
        fprintf(tmpACDLFile, "%s", retrievedMissions[i].acdlSolution.c_str());
        fclose(tmpACDLFile);
        
        // Now, open up the file, again, for reading.
        tmpACDLFile = fopen(tmpACDLFilename, "r");
        
        fsaNames.clear();
        counts.clear();
        lastTaskIndex = EMPTY_STRING_;
    
        // Process the data.
        while (fscanf(tmpACDLFile, "%s %s", buf1, buf2) != EOF)
        {
            bufString1 = buf1;
            bufString2 = buf2;

            if (bufString1 == ACDL_STRING_TASK_INDEX_)
            {
                lastTaskIndex = bufString2;
            }

            if (bufString1 == ACDL_STRING_AGENT_NAME_)
            {
                // Check to see if this fsa name is for the Start state.
                // The start state is named "Stop", a little confusing for the
                // user to see in the summary.
                fsaName = (lastTaskIndex == ACDL_STRING_START_)? ACDL_STRING_START_ : bufString2;

                nameFound = false;

                for (j = 0; j < ((int)(fsaNames.size())); j++)
                {
                    if (fsaNames[j] == fsaName)
                    {
                        (counts[j])++;
                        nameFound = true;
                        break;
                    }
                }

                if (!nameFound)
                {
                    // New FSA name.
                    fsaNames.push_back(fsaName);
                    count = 1;
                    counts.push_back(count);
                }
            }
        }

        // Make sure the file is closed.
        fclose(tmpACDLFile);
        unlink(tmpACDLFilename);

        if (((int)(fsaNames.size())) == 0)
        {
            fsaSummary = FSASUMMARY_EMPTY_PHRASE_;
        }
        else
        {
            fsaSummary = FSASUMMARY_DEFAULT_PHRASE_;

            for (j = 0; j < ((int)(fsaNames.size())); j++)
            {
                sprintf(buf, "%s (x%d)\n", fsaNames[j].c_str(), counts[j]);
                fsaSummary += buf;
            }
        }

        summary.fsaSummaries.push_back(fsaSummary);
    }

    return summary;
}

//-----------------------------------------------------------------------
// This function runs the Mission Expert.
//-----------------------------------------------------------------------
void MissionExpert::runMissionExpert(int mode)
{
    if (disableMExp_)
    {
        return;
    }

    mexpRunMode_ = mode;

    switch (mexpRunMode_) {

    case MEXP_RUNMODE_NEW:
        gEventLogging->log("MExp Run Mode = New");

        if (overlayFileName_ != EMPTY_STRING_)
        {
            overlayFileName_ = EMPTY_STRING_;
        }

        nameDataFiles_();

        if ((disableLaunchWizard_) && (!missionSpecWizardEnabled()))
        {
            askMissionSpecWizardType_();
        }

        if (missionSpecWizardType_ != MISSION_SPEC_WIZARD_DISABLED)
        {
            if (disableLaunchWizard_)
            {
                cnpMode_ = CNP_MODE_DISABLED;
            }

            switch (missionSpecWizardType_) {

            case MISSION_SPEC_WIZARD_BAMS:
                if (!sgig_connected())
                {
                    sgig_run();
                }
                break;

            case MISSION_SPEC_WIZARD_ICARUS:
                if (!cim_connected())
                {
                    cim_run();
                }
                break;
            }

            if (missionSpecWizard_ == NULL)
            {
                missionSpecWizard_ = MissionSpecWizardFactory::createMissionSpecWizard(
                    missionSpecWizardType_,
                    display_,
                    parentWidget_,
                    appContext_,
                    this,
                    rcTable_);
            }

            missionSpecWizard_->setDisableRepair(disableRepair_);
            missionSpecWizard_->start();
        }
        else
        {
            if (disableLaunchWizard_)
            {
                askCNPMode_(false);
            }

            runMlabMissionDesign_();
        }

        break;

    case MEXP_RUNMODE_VIEW:
        gEventLogging->log("MExp Run Mode = View");

        if (shouldSaveMissionToCBRLibrary_)
        {
            saveSelectionToCBRLibrary(config->root());
        }

        if (!dataFilesNamed_)
        {
            if (confirmViewAnyway_())
            {
                nameDataFiles_();
            }
            else
            {
                return;
            }
        }

        runMlabMissionDesign_();
        break;

    case MEXP_RUNMODE_REPLAY:
        gEventLogging->log("MExp Run Mode = Replay");
        runMlabReplay_();
        break;

    default:
        return;
    }

}

//-----------------------------------------------------------------------
// This function runs the Mission Expert.
//-----------------------------------------------------------------------
void MissionExpert::resumeMissionExpert_(void)
{
    bool missionIsBuilt = false, missionIsFixed = false;

    if (disableMExp_)
    {
        return;
    }

    if (statusWindows_.isUp)
    {
        while (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
    }

    switch (mexpRunMode_) {

    case MEXP_RUNMODE_NEW:
    case MEXP_RUNMODE_VIEW:
        missionIsBuilt = buildMission_();

        if (missionIsBuilt)
        {
            config->load(resultFileName_.c_str());
            config->made_change();

            save_cdl_for_replay("mexp_retrieved_mission");

            if (missionSpecWizardEnabled() &&
                (missionSpecWizard_ != NULL))
            {
                missionSpecWizard_->setMissionLoaded();

                if (!(missionSpecWizard_->noRehearsal()))
                {
                    if (make())
                    {
                        run();
                        missionSpecWizard_->setDoneRehearsal();
                    }
                    else
                    {
                        missionSpecWizard_->setCompilationFailed();
                    }
                }                
            }
            else if (gAutomaticExecution)
            {
                if (make())
                {
                    run();
                    config->quit();
                }
            }
        }
        else
        {
            closeStatusWindow_();
        }

        break;

    case MEXP_RUNMODE_REPLAY:
        missionIsFixed = repairMission_();

        if (missionIsFixed)
        {
            config->load(resultFileName_.c_str());
            config->made_change();

            save_cdl_for_replay("mexp_retrieved_fixed_mission");
        }
        else
        {
            // Repairment is canceled. Revert the feature file.
            revertFile_(featureFileName_);
        }

        break;

    default:
        return;
    }
}

//-----------------------------------------------------------------------
// This function saves the selected cases to the CBR library.
//-----------------------------------------------------------------------
void MissionExpert::saveSelectionToCBRLibrary(Symbol * agent)
{
    bool saved = false, hasStartSubMission = false;

    if (disableMExp_)
    {
        return;
    }

    missionSavedToCBRLibrary_ = false;

    if (!dataFilesNamed_)
    {
        nameDataFiles_();
    }

    saved = saveAbstractedMissionPlanToFile_(
        agent,
        missionPlanFileName_,
        &hasStartSubMission);
    
    if (saved)
    {

        // Load the features from the Mission Expert RC file.
        //if (!featuresLoaded_)
        {
            askCNPMode_(true);

            featuresLoaded_ = loadFeaturesFromRCFile_();

            if (!featuresLoaded_)
            {
                warn_userf(
                    "Mission Expert RC file \"%s\" could not be loaded.\n",
                    mexpRCFileName_.c_str());
                return;
            }
        }

        // Assign temporary values for the task and global features used in
        // the feature menu window.
        createTemporaryTaskAndGlobalFeatures_();

        // Create and popup the feature menu window.
        popupFeatureMenu_(!hasStartSubMission);
    }
}

//-----------------------------------------------------------------------
// This function converts the selected mission to ACDL.
//-----------------------------------------------------------------------
string MissionExpert::getAbstractedMissionPlan(Symbol * agent)
{
    config_fsa_lists_t fsaLists;
    string tempCDLFilename, tempACDLFilename;
    string acdlMission = EMPTY_STRING_;
    char *buf = NULL;
    int dataSize;
    bool initialized = false, errors = false, saved = false;
    const char *tempFilelist[2];
    const bool REVERSE_ROBOT_ORDER = true;

    // Make sure it can talk to the CBR library.
    if (!cbrClientInstantiated_)
    {
        // Instantiate the CBRClient first.
        initialized = initCBRClient_();

        if (!initialized)
        {
            // Something went wrong during the initialization.
            // Just return.
            fprintf(
                stderr,
                "Error(cfgedit): MissionExpert::getAbstractedMissionPlan(): CBR client could not be initialized.\n");
            return EMPTY_STRING_;
        }

        // Upload the FSA names to the server
        fsaLists = getFSALists_();
        cfgeditCBRClient_->saveFSANames(fsaLists);
    }

    // Save the agent in a temporary file first, and then load it
    // again. This will allow the configuration tree to be ordered
    // correctly.
    tempCDLFilename = "/tmp/";
    tempCDLFilename += createPrefix_();
    tempCDLFilename += ".";
    tempCDLFilename += MEXP_EXTENSION_CDL;
    saved = save_workspace(agent, tempCDLFilename.c_str(), false, false);

    if (!saved)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionExpert::getAbstractedMissionPlan(): CDL could not be saved in a file.\n");
        return EMPTY_STRING_;
    }

    // Load the agent.
    tempFilelist[0] = tempCDLFilename.c_str();
    tempFilelist[1] = NULL;
    agent = load_cdl(tempFilelist, errors, false, false);

    // Save the agent in another temporary file.
    tempACDLFilename = "/tmp/";
    tempACDLFilename += createPrefix_();
    tempACDLFilename += ".";
    tempACDLFilename += MEXP_EXTENSION_ACDL;
    saved = save_abstracted_workspace(agent, tempACDLFilename.c_str());

    // Delete the temporary CDL file.
    unlink(tempCDLFilename.c_str());

    // Open the solution file to read.
    ifstream istr(tempACDLFilename.c_str());

    if (istr.bad()) 
    {
        fprintf
            (stderr,
             "Error(cfgedit): MissionExpert::getAbstractedMissionPlan(). file %s not readable",
             tempACDLFilename.c_str());
        return EMPTY_STRING_;
    }

    istr.seekg (0, ios::end);
    dataSize = istr.tellg();
    istr.seekg (0, ios::beg);

    // Read in the solution.
    buf = new char[dataSize+1];
    istr.read (buf, dataSize);

    // Append a null, to make it a char string.
    buf[dataSize] = CHAR_NULL_;

    // Close and delete the solution file.
    istr.close();
    unlink(tempCDLFilename.c_str());

    // Copy the solution.
    acdlMission = buf;
    delete [] buf;
    buf = NULL;

    if (REVERSE_ROBOT_ORDER)
    {
        // save_abstracted_workspace() above reverses the robot order. Reverse-back
        // the order using CBR.
        acdlMission = cfgeditCBRClient_->reverseRobotOrder(acdlMission);
    }

    return acdlMission;
}

//-----------------------------------------------------------------------
// This function merges multiple ACDL missions into one.
//-----------------------------------------------------------------------
string MissionExpert::getMergedAbstractedMissionPlan(
    vector<string> acdlMissions,
    int *numRobots)
{
    string acdlMission;

    acdlMission = cfgeditCBRClient_->getMergedAbstractedMissionPlan(acdlMissions, numRobots);

    return acdlMission;
}

//-----------------------------------------------------------------------
// This function saves the selected mission to the CBR library.
//-----------------------------------------------------------------------
bool MissionExpert::saveAbstractedMissionPlanToFile_(
    Symbol * agent,
    string fileName,
    bool *hasStartSubMission)
{
    FILE *outputFile = NULL;
    string acdlMissionPlan;

    if (fileName == EMPTY_STRING_)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionExpert::saveAbstractedMissionPlanToFile_(). fileName is empty.\n");
        return false;
    }

    // Get the ACDL data from the agent.
    acdlMissionPlan = getAbstractedMissionPlan(agent);

    if (acdlMissionPlan == EMPTY_STRING_)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionExpert::saveAbstractedMissionPlanToFile_(): ACDL mission is empty.\n");
        return false;
    }

    // Write the data in the file.
    outputFile = fopen(fileName.c_str(), "w");

    if (outputFile == NULL)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionExpert::saveAbstractedMissionPlanToFile_(): Output file [%s] could not be opened.\n",
            fileName.c_str());
        return false;
    }

    fprintf(outputFile, "%s", acdlMissionPlan.c_str());
    fclose(outputFile);

    *hasStartSubMission = hasAgentInMissionPlan_(
        fileName,
        CDL_STRING_START_SUB_MISSION_);

    addCreatedDataFileList_(fileName);

    return true;
}

//-----------------------------------------------------------------------
// This function creates a temporary task based on the default task.
//-----------------------------------------------------------------------
void MissionExpert::createTemporaryTaskAndGlobalFeatures_(void)
{
    int i, j;
    double maxVel, baseVel, aggressiveness;
    string taskName, maxVelValue, baseVelValue;
    MExpFeature_t localFeature, fixedGlobalFeature;
    vector<MExpFeature_t> localFeatures;
    vector<string> valueList;
    MExpMissionTask_t defaultTask;
    MExpFeatureOption_t option;

    // At first, create a list of local features with default values.
    localFeatures = defaultLocalFeatures_;

    // Next, find a task in the list that matches with the default task.
    defaultTask = defaultTaskList_[0];
    for (i = 0; i < numTaskTypes_; i++)
    {
        if (defaultTaskList_[i].name == defaultTaskName_)
        {
            defaultTask = defaultTaskList_[i];
            break;
        }
    }

    // Use the specified option for the default task if it is specified.
    for (i = 0; i < (int)(defaultTask.localFeatures.size()); i++)
    {
        for (j = 0; j < (int)(localFeatures.size()); j++)
        {
            if (defaultTask.localFeatures[i].name == localFeatures[j].name)
            {
                localFeatures[j].selectedOption = defaultTask.localFeatures[i].selectedOption;
            }
        }
    }

    // Assign values for the task.
    featureMenuUpTask_.localFeatures = localFeatures;
    featureMenuUpTask_.number = 0;
    featureMenuUpTask_.id = defaultTask.id;
    featureMenuUpTask_.name = defaultTask.name;
    featureMenuUpTask_.position.x = 0;
    featureMenuUpTask_.position.y = 0;
    featureMenuUpTask_.geo.latitude = g_ref_latitude;
    featureMenuUpTask_.geo.longitude = g_ref_longitude; 


    // Copy the values of the default global features to the menu specific one.
    featureMenuUpGlobalFeatures_.clear();
    fixedGlobalFeatures_.clear();

    for (i = 0; i < (int)(defaultGlobalFeatures_.size()); i++)
    {
        if (defaultGlobalFeatures_[i].name == FEATURE_NAME_NUMROBOTS_)
        {
            // The number of robots should not be a feature to save if it is for CNP.
            if (cnpMode_ == CNP_MODE_DISABLED)
            {
                fixedGlobalFeature = defaultGlobalFeatures_[i];

                // Assign the number of robots (from renumber_robots.h).
                fixedGlobalFeature.selectedOption = (float)num_robots; 
                fixedGlobalFeatures_.push_back(fixedGlobalFeature);
            }
        }
        else if (defaultGlobalFeatures_[i].name == FEATURE_NAME_MAXVELOCITY_)
        {
            fixedGlobalFeature = defaultGlobalFeatures_[i];

            // Get the max velocity from configuration.
            maxVelValue = config->root()->findData(CDL_STRING_MAXVELOCITY_);
            maxVel = atof(maxVelValue.c_str());

            fixedGlobalFeature.selectedOption = maxVel; 
            fixedGlobalFeatures_.push_back(fixedGlobalFeature);
        }
        else if (defaultGlobalFeatures_[i].name == FEATURE_NAME_AGGRESSIVENESS_)
        {
            fixedGlobalFeature = defaultGlobalFeatures_[i];

            // Get the max velocity from configuration.
            baseVelValue = (config->root()->findDataList(CDL_STRING_BASEVELOCITY_)).back();
            baseVel = atof(baseVelValue.c_str());

            aggressiveness = baseVel/maxVel;
            aggressiveness = (aggressiveness > 1.0)? 1.0 : aggressiveness;

            fixedGlobalFeature.selectedOption = aggressiveness;
            fixedGlobalFeatures_.push_back(fixedGlobalFeature);
        }
        else
        {
            featureMenuUpGlobalFeatures_.push_back(defaultGlobalFeatures_[i]);
        }
    }
}

//-----------------------------------------------------------------------
// This function creates a popup window for the metadata entry.
//-----------------------------------------------------------------------
void MissionExpert::updateMetadataRatingBtn_(Widget button)
{
    int i, j, index;
    bool ratingSet = false, buttonFound = false;

    if (metadataEntryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::updateMetadataRatingBtn_(). metadataEntryWindowData_ is NULL.\n");
        return;
    }

    index = 0;

    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        for (j = 0; j < MAX_METADATA_RATING_ + 1; j++)
        {
            if (metadataEntryWindowData_->ratingBtns[i][j] == button)
            {
                if (j != MAX_METADATA_RATING_)
                {
                    metadataEntryWindowData_->ratings[i] = j + 1;
                }
                else
                {
                    metadataEntryWindowData_->ratings[i] = 0;
                }

                index = j;
                buttonFound = true;
                break;
            }
        }

        if (buttonFound)
        {
            for (j = 0; j < MAX_METADATA_RATING_ + 1; j++)
            {
                ratingSet = false;

                if (j == index)
                {
                    ratingSet = true;
                }

                XtVaSetValues(
                    metadataEntryWindowData_->ratingBtns[i][j],
                    XmNset, ratingSet,
                    NULL);
            }
            break;
        }
    }
}

//-----------------------------------------------------------------------
// This function updates the metadata entry window's comment string.
//-----------------------------------------------------------------------
void MissionExpert::updateMetadataCommentTextWindowString_(void)
{
    char *text = NULL;

    if (metadataEntryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::updateMetadataCommentTextWindowString_(). metadataEntryWindowData_ is NULL.\n");
        return;
    }

    XtVaGetValues(
        metadataEntryWindowData_->commentTextWindow,
        XmNvalue, &text,
        NULL);

    if (text == NULL)
    {
        return;
    }

    metadataEntryWindowData_->commentTextWindowString = text;
}

//-----------------------------------------------------------------------
// This function updates the save-cbr-library window's filename string.
//-----------------------------------------------------------------------
void MissionExpert::updateSaveCBRLibraryFileWindowString_(void)
{
    char *text = NULL;

    if (saveCBRLibraryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::updateSaveCBRLibraryFileWindowString_().saveCBRLibraryWindowData_ is NULL.\n");
        return;
    }

    XtVaGetValues(
        saveCBRLibraryWindowData_->filenameTextWindow,
        XmNvalue, &text,
        NULL);

    if (text == NULL)
    {
        return;
    }

    saveCBRLibraryWindowData_->filenameTextWindowString = text;

    if (saveCBRLibraryWindowData_->filenameTextWindowString != EMPTY_STRING_)
    {
        XtSetSensitive(saveCBRLibraryWindowData_->saveBtn, true);
    }
    else
    {
        XtSetSensitive(saveCBRLibraryWindowData_->saveBtn, false);
    }
}

//-----------------------------------------------------------------------
// This function updates the metadata entry window's name string.
//-----------------------------------------------------------------------
void MissionExpert::updateMetadataNameTextWindowString_(void)
{
    char *text = NULL;

    if (metadataEntryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::updateMetadataNameTextWindowString_(). metadataEntryWindowData_ is NULL.\n");
        return;
    }

    XtVaGetValues(
        metadataEntryWindowData_->nameTextWindow,
        XmNvalue, &text,
        NULL);

    if (text == NULL)
    {
        return;
    }

    metadataEntryWindowData_->nameTextWindowString = text;
}

//-----------------------------------------------------------------------
// This function sends the mission feedback to the cbrserver.
//-----------------------------------------------------------------------
void MissionExpert::sendMissionFeedback(vector<int> dataIndexList)
{
    popupMetadataEntryWindow_(dataIndexList);
}

//-----------------------------------------------------------------------
// This function creates a popup window for the metadata entry.
//-----------------------------------------------------------------------
void MissionExpert::popupMetadataEntryWindow_(vector<int> dataIndexList)
{
    Widget mainFrame, mainBox;
    Widget instructionFrame, instructionBox, instructionWindow;
    Widget itemFrame, itenLabelWidget, itemBox, itemBtnFrame, itemBtnBox;
    Widget commentFrame, commentBox, commentLabelWidget, commentTextFrame;
    Widget nameBox, nameLabelWidget;
    Widget actionFrame, actionBox, saveBtn, cancelBtn;
    vector<Widget> stackedWidget;
    string instruction, commentLabel, nameLabel, saveLabel, cancelLabel;
    char label[1024];
    int numStackedWidget;
    int screenNumber;
    int posX, posY;
    int windowWidth, windowHeight;
    int itemFrameHeight, itemLabelHeight, itemHeight;
    int commentFrameHeight, commentLabelHeight, commentHeight;
    int nameHeight;
    int displayWidth, displayHeight;
    int instructionHeight, actionSpaceHeight;
    int i, j;
    bool ratingSet = false;

    initializeMetadataEntryWindowData_(true);
    metadataEntryWindowData_->windowIsUp = true;
    metadataEntryWindowData_->dataIndexList = dataIndexList;

    // Set up the instruction and labels.
    instruction = DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_;
    commentLabel = DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_;
    nameLabel = DEFAULT_METADATA_ENTRY_WINDOW_NAME_LABEL_;
    saveLabel = DEFAULT_METADATA_ENTRY_WINDOW_SAVE_BUTTON_LABEL_;
    cancelLabel = DEFAULT_METADATA_ENTRY_WINDOW_CANCEL_BUTTON_LABEL_;

    // Set up window sizes.
    windowWidth = DEFAULT_METADATA_ENTRY_WINDOW_WIDTH_;
    itemLabelHeight = DEFAULT_METADATA_ENTRY_WINDOW_ITEM_LABEL_HEIGHT_;
    itemHeight = DEFAULT_METADATA_ENTRY_WINDOW_ITEM_HEIGHT_;
    itemFrameHeight = itemLabelHeight + itemHeight;
    instructionHeight = DEFAULT_METADATA_ENTRY_WINDOW_INSTRUCTION_HEIGHT_;
    actionSpaceHeight = DEFAULT_METADATA_ENTRY_WINDOW_ACTIONSPACE_HEIGHT_;
    commentLabelHeight = DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_LABEL_HEIGHT_;
    commentHeight = DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_HEIGHT_;
    nameHeight = DEFAULT_METADATA_ENTRY_WINDOW_NAME_HEIGHT_;
    commentFrameHeight =
        commentLabelHeight +
        commentHeight +
        nameHeight +
        DEFAULT_METADATA_ENTRY_WINDOW_COMMENT_EXTRA_HEIGHT_;
    windowHeight =
        instructionHeight +
        (itemFrameHeight * NUM_MEXP_METADATA_RATINGS) +
        commentFrameHeight +
        actionSpaceHeight;

    // Set up the position.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth - windowWidth)/2;
    posY = (displayHeight - windowHeight)/2;

    // Popup shell.
    metadataEntryWindowData_->mainWindow = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, windowHeight,
         XmNwidth, windowWidth,
         NULL);


    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFormWidgetClass, metadataEntryWindowData_->mainWindow,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, windowHeight,
         XmNwidth, windowWidth+6,
         NULL);

    stackedWidget.push_back(mainFrame);

    mainBox = XtVaCreateWidget
        ("mainBox",
         xmRowColumnWidgetClass, mainFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, windowHeight,
         XmNwidth, windowWidth,
         NULL);

    stackedWidget.push_back(mainBox);

    // The frame for the instruction.
    instructionFrame = XtVaCreateWidget
        ("instructionFrame",
         xmFrameWidgetClass, mainBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, instructionHeight,
         XmNwidth, windowWidth,
         NULL);

    stackedWidget.push_back(instructionFrame);

    // The widget for the instruction.
    instructionBox = XtVaCreateWidget
        ("instructionBox",
         xmRowColumnWidgetClass, instructionFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, instructionHeight,
         XmNwidth, windowWidth,
         NULL);

    stackedWidget.push_back(instructionBox);

    // The instruction.
    instructionWindow = XtVaCreateWidget
        (instruction.c_str(),
         xmLabelGadgetClass, instructionBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, instructionHeight,
         XmNwidth, windowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, instructionBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, instructionBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, instructionBox,
         NULL);

    stackedWidget.push_back(instructionWindow);

    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        // The frame for the rating.
        itemFrame = XtVaCreateWidget
            ("itemFrame",
             xmFrameWidgetClass, mainBox,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, itemFrameHeight,
             XmNwidth, windowWidth,
             NULL);

        stackedWidget.push_back(itemFrame);

        // The create a row-column box.
        itemBox = XtVaCreateWidget
            ("itemBox",
             xmRowColumnWidgetClass, itemFrame,
             XmNpacking, XmPACK_COLUMN,
             XmNorientation, XmVERTICAL,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNnumColumns, 1,
             XmNheight, instructionHeight,
             XmNwidth, windowWidth,
             NULL);

        stackedWidget.push_back(itemBox);

        // The label for the rating.
        itenLabelWidget = XtVaCreateWidget
            (METADATA_LABELS_[i].c_str(),
             xmLabelGadgetClass, itemBox,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, itemLabelHeight,
             XmNwidth, windowWidth,
             XmNshadowType, XmSHADOW_ETCHED_OUT,
             XmNborderWidth, 0,
             XmNrecomputeSize, false,
             NULL);

        stackedWidget.push_back(itenLabelWidget);

        itemBtnFrame = XtVaCreateWidget
            ("itemBtnFrame",
             xmFrameWidgetClass, itemBox,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, itemHeight,
             XmNwidth, windowWidth,
             XmNborderWidth, 0,
             NULL);

        stackedWidget.push_back(itemBtnFrame);

        // The create a row-column box for the buttons.
        itemBtnBox = XtVaCreateWidget
            ("itemBtnBox",
             xmFormWidgetClass, itemBtnFrame,
             XmNalignment, XmALIGNMENT_CENTER,
             XmNrecomputeSize, false,
             XmNborderWidth, 0,
             XmNfractionBase, MAX_METADATA_RATING_ + 1,
             XmNrightAttachment, XmATTACH_WIDGET,
             XmNrightWidget, itemBtnFrame,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftWidget, itemBtnFrame,
             XmNtopAttachment, XmATTACH_WIDGET,
             XmNtopWidget, itemBtnFrame,
             XmNbottomAttachment, XmATTACH_WIDGET,
             XmNbottomWidget, itemBtnFrame,
             XmNresizable, false,
             NULL);

        stackedWidget.push_back(itemBtnBox);

        for (j = 0; j < (MAX_METADATA_RATING_ + 1); j++)
        {
            ratingSet = false;

            if (j == MAX_METADATA_RATING_)
            {
                sprintf(label, "[N/A]");

                if ((metadataEntryWindowData_->ratings[i]) == 0)
                {
                    ratingSet = true;
                }
            }
            else
            {
                sprintf(label, "%d", j + 1);

                if ((metadataEntryWindowData_->ratings[i]) == (j + 1))
                {
                    ratingSet = true;
                }
            }

            metadataEntryWindowData_->ratingBtns[i][j] = XtVaCreateWidget
                (label,
                 xmToggleButtonGadgetClass, itemBtnBox,
                 XmNradioBehavior, true,
                 XmNradioAlwaysOne, true,
                 XmNset, ratingSet,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNborderWidth, 0,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, j,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, j+1,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNhighlightOnEnter, false,
                 XmNhighlightColor, parentWidgetBg_,
                 NULL);

            stackedWidget.push_back(metadataEntryWindowData_->ratingBtns[i][j]);

            XtAddCallback
                (metadataEntryWindowData_->ratingBtns[i][j],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbChangeMetadataRatingBtn_,
                 this);
        }
    }

    // The frame for the comments.
    commentFrame = XtVaCreateWidget
        ("commentFrame",
         xmFrameWidgetClass, mainBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, commentFrameHeight,
         XmNwidth, windowWidth,
         XmNrecomputeSize, false,
         XmNresizable, false,
         NULL);

    stackedWidget.push_back(commentFrame);

    // The create a row-column box.
    commentBox = XtVaCreateWidget
        ("commentBox",
         xmRowColumnWidgetClass, commentFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, commentFrameHeight,
         XmNwidth, windowWidth,
         XmNrecomputeSize, false,
         XmNresizable, false,
         XmNresizeHeight, false,
         NULL);

    stackedWidget.push_back(commentBox);

    // The label for the comment.
    commentLabelWidget = XtVaCreateWidget
        (commentLabel.c_str(),
         xmLabelGadgetClass, commentBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, commentLabelHeight,
         XmNwidth, windowWidth,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNborderWidth, 0,
         XmNrecomputeSize, false,
         NULL);

    stackedWidget.push_back(commentLabelWidget);

    commentTextFrame = XtVaCreateWidget
        ("commentTextFrame",
         xmFrameWidgetClass, commentBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, commentHeight,
         XmNwidth, windowWidth,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(commentTextFrame);

    metadataEntryWindowData_->commentTextWindow = XtVaCreateWidget
        ("commentTextWindow",
         xmTextWidgetClass, commentTextFrame,
         XmNvalue, EMPTY_STRING_.c_str(),
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, commentTextFrame,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, commentTextFrame,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, commentTextFrame,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, commentTextFrame,
         XmNbackground, gCfgEditPixels.white,
         XmNheight, commentHeight,
         XmNborderWidth, 1,
         XmNhighlightThickness, 0,
         XmNeditMode, XmMULTI_LINE_EDIT,
         XmNrows, 2,
         XmNeditable, true,
         NULL);

    stackedWidget.push_back(metadataEntryWindowData_->commentTextWindow);

    XtAddCallback(
        metadataEntryWindowData_->commentTextWindow,
        XmNvalueChangedCallback,
        (XtCallbackProc)cbMetadataCommentTextWindowStringUpdate_,
        (void *)this);

    // The form for the name.
    nameBox = XtVaCreateWidget
        ("nameBox",
         xmFormWidgetClass, commentBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, nameHeight,
         XmNwidth, windowWidth-5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, commentBox,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, commentBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, commentBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, commentBox,
         XmNfractionBase, 6,
         XmNrecomputeSize, false,
         XmNresizable, false,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(nameBox);

    // The label for the name.
    nameLabelWidget = XtVaCreateWidget
        (nameLabel.c_str(),
         xmLabelGadgetClass, nameBox,
         XmNheight, nameHeight,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNborderWidth, 0,
         XmNrecomputeSize, false,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 3,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 4,
         NULL);

    stackedWidget.push_back(nameLabelWidget);

    metadataEntryWindowData_->nameTextWindow = XtVaCreateWidget
        ("nameTextWindow",
         xmTextWidgetClass, nameBox,
         XmNvalue, EMPTY_STRING_.c_str(),
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 4,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 6,
         XmNbackground, gCfgEditPixels.white,
         XmNheight, nameHeight,
         XmNborderWidth, 1,
         XmNhighlightThickness, 0,
         XmNrows, 1,
         XmNeditable, true,
         NULL);

    stackedWidget.push_back(metadataEntryWindowData_->nameTextWindow);

    XtAddCallback(
        metadataEntryWindowData_->nameTextWindow,
        XmNvalueChangedCallback,
        (XtCallbackProc)cbMetadataNameTextWindowStringUpdate_,
        (void *)this);

    // The frame for the action field.
    actionFrame = XtVaCreateWidget
        ("actionFrame",
         xmFrameWidgetClass, mainBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, actionSpaceHeight,
         XmNwidth, windowWidth,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, mainBox,
         NULL);

    stackedWidget.push_back(actionFrame);

    actionBox = XtVaCreateWidget
        ("actionBox",
         xmFormWidgetClass, actionFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, actionSpaceHeight,
         XmNwidth, windowWidth-5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, actionFrame,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, actionFrame,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, actionFrame,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, actionFrame,
         XmNfractionBase, 2,
         XmNrecomputeSize, false,
         XmNresizable, false,
         NULL);

    stackedWidget.push_back(actionBox);

    // The "Save" button.
    saveBtn = XtVaCreateWidget
        (saveLabel.c_str(),
         xmPushButtonGadgetClass, actionBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNborderWidth, 0,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 0,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 1,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, actionSpaceHeight,
         NULL);

    stackedWidget.push_back(saveBtn);

    XtAddCallback
        (saveBtn,
         XmNactivateCallback, 
         (XtCallbackProc)cbSaveMetadataEntries_,
         this);

    // The "Cancel" button
    cancelBtn = XtVaCreateWidget
        (cancelLabel.c_str(),
         xmPushButtonGadgetClass, actionBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNborderWidth, 0,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 1,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, actionSpaceHeight,
         NULL);

    stackedWidget.push_back(cancelBtn);

    XtAddCallback
        (cancelBtn,
         XmNactivateCallback, 
         (XtCallbackProc)cbCancelMetadataEntries_,
         this);

    metadataEntryWindowData_->poppedUpWidgets = stackedWidget;
    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
        XFlush(display_);
    }

    XtPopup (metadataEntryWindowData_->mainWindow, XtGrabNone);
    XFlush(display_);

    // Wait for the user to close the window.
    while(metadataEntryWindowData_->windowIsUp)
    {
        // Run X
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
}

//-----------------------------------------------------------------------
// This function closes the metadata entry window.
//-----------------------------------------------------------------------
void MissionExpert::closeMetadataEntryWindow_(void) 
{
    if (metadataEntryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::closeMetadataEntryWindow_(). metadataEntryWindowData_ is NULL.\n");
        return;
    }

    while ((int)(metadataEntryWindowData_->poppedUpWidgets.size()) > 0)
    {
        XtUnmanageChild(metadataEntryWindowData_->poppedUpWidgets.back());
        metadataEntryWindowData_->poppedUpWidgets.pop_back();
        XFlush(display_);
    }

    XtUnmanageChild(metadataEntryWindowData_->mainWindow);
    XFlush(display_);

    metadataEntryWindowData_->windowIsUp = false;
}

//-----------------------------------------------------------------------
// This function attempts to save the CBR library file.
//-----------------------------------------------------------------------
void MissionExpert::saveCBRLibraryFile(void)
{
    string cblFileName = EMPTY_STRING_;

    if (cfgeditCBRClient_ == NULL)
    {
        return;
    }

    if (cfgeditCBRClient_->checkCBRLibrarySave(&cblFileName))
    {
        popupSaveCBRLibraryWindow_(cblFileName);
    }
}

//-----------------------------------------------------------------------
// This function sends the entries specified in the metadata entry
// window to the cbrserver. 
//-----------------------------------------------------------------------
void MissionExpert::sendCBRLibraryFile_(void) 
{
    string filename;

    if (saveCBRLibraryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::sendCBRLibraryFile_(). saveCBRLibraryWindowData_ is NULL.\n");
        return;
    }

    // Copy the filename.
    filename = saveCBRLibraryWindowData_->filenameTextWindowString;

    // Send it.
    cfgeditCBRClient_->saveCBRLibraryFile(filename);
}

//-----------------------------------------------------------------------
// This function creates a popup window for saving the CBR library file.
//-----------------------------------------------------------------------
void MissionExpert::popupSaveCBRLibraryWindow_(string cblFileName)
{
    Widget mainFrame, mainBox;
    Widget instructionBox, instructionWindow;
    Widget filenameBox, filenameLabelWidget;
    Widget actionFrame, actionBox, cancelBtn;
    vector<Widget> stackedWidget;
    string instruction, filenameLabel, saveLabel, cancelLabel;
    int posX, posY, windowHeight, windowWidth;
    int displayHeight, displayWidth;
    int instructionHeight, actionSpaceHeight, filenameHeight;
    int screenNumber;
    int i, numStackedWidget;

    if (saveCBRLibraryWindowData_ != NULL)
    {
        closeSaveCBRLibraryWindow_();
    }

    saveCBRLibraryWindowData_ = new SaveCBRLibraryWindowData_t;
    saveCBRLibraryWindowData_->filenameTextWindowString = cblFileName;

    // Set up the instruction and labels.
    instruction = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_;
    filenameLabel = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_LABEL_;
    saveLabel = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_SAVE_BUTTON_LABEL_;
    cancelLabel = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_CANCEL_BUTTON_LABEL_;

    // Set up window sizes.
    windowWidth = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_WIDTH_;
    instructionHeight = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_INSTRUCTION_HEIGHT_;
    filenameHeight = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_FILENAME_HEIGHT_;
    actionSpaceHeight = DEFAULT_SAVE_CBR_LIBRARY_WINDOW_ACTIONSPACE_HEIGHT_;
    windowHeight =
        instructionHeight +
        filenameHeight +
        actionSpaceHeight;

    // Set up the position.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth - windowWidth)/2;
    posY = (displayHeight - windowHeight)/2;

    // Popup shell.
    saveCBRLibraryWindowData_->mainWindow = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, windowHeight,
         XmNwidth, windowWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFormWidgetClass, saveCBRLibraryWindowData_->mainWindow,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, windowHeight,
         XmNwidth, windowWidth+6,
         NULL);

    stackedWidget.push_back(mainFrame);

    mainBox = XtVaCreateWidget
        ("mainBox",
         xmRowColumnWidgetClass, mainFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, windowHeight,
         XmNwidth, windowWidth,
         NULL);

    stackedWidget.push_back(mainBox);

    // The frame for the instruction.

    // The widget for the instruction.
    instructionBox = XtVaCreateWidget
        ("instructionBox",
         xmRowColumnWidgetClass, mainBox,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         //XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNentryAlignment, XmALIGNMENT_BEGINNING,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, mainBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, mainBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, mainBox,
         XmNheight, instructionHeight,
         XmNwidth, windowWidth,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(instructionBox);

    // The instruction.
    instructionWindow = XtVaCreateWidget
        (instruction.c_str(),
         xmLabelGadgetClass, instructionBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, instructionHeight,
         XmNwidth, windowWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, instructionBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, instructionBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, instructionBox,
         NULL);

    stackedWidget.push_back(instructionWindow);

    // The form for the filename.
    filenameBox = XtVaCreateWidget
        ("filenameBox",
         xmFormWidgetClass, mainBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, filenameHeight,
         XmNwidth, windowWidth-5,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, mainBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, mainBox,
         XmNfractionBase, 6,
         XmNrecomputeSize, false,
         XmNresizable, false,
         XmNborderWidth, 0,
         NULL);

    stackedWidget.push_back(filenameBox);

    // The label for the filename.
    filenameLabelWidget = XtVaCreateWidget
        (filenameLabel.c_str(),
         xmLabelGadgetClass, filenameBox,
         XmNheight, filenameHeight,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNborderWidth, 0,
         XmNrecomputeSize, false,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 0,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 1,
         NULL);

    stackedWidget.push_back(filenameLabelWidget);

    saveCBRLibraryWindowData_->filenameTextWindow = XtVaCreateWidget
        ("filenameTextWindow",
         xmTextWidgetClass, filenameBox,
         XmNvalue, saveCBRLibraryWindowData_->filenameTextWindowString.c_str(),
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 1,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 6,
         XmNbackground, gCfgEditPixels.white,
         XmNheight, filenameHeight,
         XmNborderWidth, 1,
         XmNhighlightThickness, 0,
         XmNrows, 1,
         XmNeditable, true,
         NULL);

    stackedWidget.push_back(saveCBRLibraryWindowData_->filenameTextWindow);

    XtAddCallback(
        saveCBRLibraryWindowData_->filenameTextWindow,
        XmNvalueChangedCallback,
        (XtCallbackProc)cbSaveCBRLibraryFileWindowStringUpdate_,
        (void *)this);

    XtAddCallback
        (saveCBRLibraryWindowData_->filenameTextWindow,
         XmNactivateCallback, 
         (XtCallbackProc)cbSaveCBRLibraryFile_,
         this);

    // The frame for the action field.
    actionFrame = XtVaCreateWidget
        ("actionFrame",
         xmFrameWidgetClass, mainBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, actionSpaceHeight,
         XmNwidth, windowWidth,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, mainBox,
         NULL);

    stackedWidget.push_back(actionFrame);

    actionBox = XtVaCreateWidget
        ("actionBox",
         xmFormWidgetClass, actionFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, actionSpaceHeight,
         XmNwidth, windowWidth-5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, actionFrame,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, actionFrame,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, actionFrame,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, actionFrame,
         XmNfractionBase, 2,
         XmNrecomputeSize, false,
         XmNresizable, false,
         NULL);

    stackedWidget.push_back(actionBox);

    // The "Save" button.
    saveCBRLibraryWindowData_->saveBtn = XtVaCreateWidget
        (saveLabel.c_str(),
         xmPushButtonGadgetClass, actionBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNborderWidth, 0,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 0,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 1,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, actionSpaceHeight,
         NULL);

    stackedWidget.push_back(saveCBRLibraryWindowData_->saveBtn);

    XtAddCallback
        (saveCBRLibraryWindowData_->saveBtn,
         XmNactivateCallback, 
         (XtCallbackProc)cbSaveCBRLibraryFile_,
         this);

    if ((saveCBRLibraryWindowData_->filenameTextWindowString) == EMPTY_STRING_)
    {
        XtSetSensitive(saveCBRLibraryWindowData_->saveBtn, false);
    }

    // The "Cancel" button
    cancelBtn = XtVaCreateWidget
        (cancelLabel.c_str(),
         xmPushButtonGadgetClass, actionBox,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNborderWidth, 0,
         XmNleftAttachment, XmATTACH_POSITION,
         XmNleftPosition, 1,
         XmNrightAttachment, XmATTACH_POSITION,
         XmNrightPosition, 2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, actionSpaceHeight,
         NULL);

    stackedWidget.push_back(cancelBtn);

    XtAddCallback
        (cancelBtn,
         XmNactivateCallback, 
         (XtCallbackProc)cbCancelSaveCBRLibraryFile_,
         this);

    saveCBRLibraryWindowData_->poppedUpWidgets = stackedWidget;
    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
        XFlush(display_);
    }

    XtPopup (saveCBRLibraryWindowData_->mainWindow, XtGrabNone);
    XtAddGrab(saveCBRLibraryWindowData_->mainWindow, true, true);
    XFlush(display_);

    // Wait for the user to close the window.
    while(saveCBRLibraryWindowData_ != NULL)
    {
        // Run X
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
}

//-----------------------------------------------------------------------
// This function closes the window for saving the CBR library file.
//-----------------------------------------------------------------------
void MissionExpert::closeSaveCBRLibraryWindow_(void)
{
    SaveCBRLibraryWindowData_t *saveCBRLibraryWindowDataPtr = NULL;

    if (saveCBRLibraryWindowData_ == NULL)
    {
        return;
    }

    while ((int)(saveCBRLibraryWindowData_->poppedUpWidgets.size()) > 0)
    {
        XtUnmanageChild(saveCBRLibraryWindowData_->poppedUpWidgets.back());
        saveCBRLibraryWindowData_->poppedUpWidgets.pop_back();
        XFlush(display_);
    }

    XtPopdown(saveCBRLibraryWindowData_->mainWindow);
    XFlush(display_);

    saveCBRLibraryWindowDataPtr = saveCBRLibraryWindowData_;
    saveCBRLibraryWindowData_ = NULL;
    delete saveCBRLibraryWindowData_;
    saveCBRLibraryWindowData_ = NULL;
}

//-----------------------------------------------------------------------
// This function updates the usage counter in the CBR library.
//-----------------------------------------------------------------------
void MissionExpert::updateMissionUsageCounter(vector<int> dataIndexList)
{
    if (cfgeditCBRClient_ == NULL)
    {
        return;
    }

    cfgeditCBRClient_->updateMissionUsageCounter(dataIndexList);
}

//-----------------------------------------------------------------------
// This function creates a popup window for the feature menu to save the case. 
//-----------------------------------------------------------------------
void MissionExpert::popupFeatureMenu_(bool askTaskType)
{
    XmString sliderTitle;
    Widget mainFrame, main;
    Widget labelFrame, labelBox, label;
    Widget featureFrame, featureBox;
    Widget taskFrame, taskBox;
    Widget actionFrame, action;
    Widget buttonFrame, button;
    Widget saveFrame, saveBtn;
    Widget cancelFrame, cancelBtn;
    Widget polygonFrame, valueFrame;
    MExpMissionTask_t task;
    vector<Widget> stackedWidget;
    FeatureMenuTaskCallbackData_t *taskData;
    FeatureMenuCallbackData_t *featureOptionData;
    GlobalFeatureMenuCallbackData_t *globalFeatureData;
    string featureMenuTitle;
    string taskLabel;
    string saveLabel;
    string cancelLabel;
    char buf[1024];
    float value, conversion;
    int featureMenuHeight, featureMenuWidth;
    int featureMenuLineHeight, taskBoxHeight, taskBoxHeightOffset;
    int actionSpaceHeight, featureLabelHeight;
    int globalFeatureHeight, globalFeatureHeightTotal;
    int localFeatureHeight, localFeatureHeightTotal;
    int i, j, numOptions, line = 0;
    int set;
    int screenNumber;
    int posX, posY, displayWidth, displayHeight;
    int numStackedWidget;
    int numLocalFeatures, numGlobalFeatures;
    int numOptionRows;
    int maxValue, decimalPoints;
    const int NUM_OPTION_COLUMNS = 2;

    featureMenuIsUp_ = true;

    featureMenuTitle = DEFAULT_FEATUREMENU_TITLE_;
    taskLabel = DEFAULT_FEATUREMENU_TASK_LABEL_;
    saveLabel = DEFAULT_FEATUREMENU_SAVE_BUTTON_LABEL_;
    cancelLabel = DEFAULT_FEATUREMENU_CANCEL_BUTTON_LABEL_;

    // Setting the size of the window.
    featureMenuLineHeight = DEFAULT_FEATUREMENU_LINEHEIGHT_;

    // Set the action space (Save & Cancel) height.
    actionSpaceHeight = 3*(featureMenuLineHeight+3);

    // Set the height for the label of each feature.
    featureLabelHeight = featureMenuLineHeight+3;

    // Compute the total height of the global features.
    numGlobalFeatures = featureMenuUpGlobalFeatures_.size();
    globalFeatureHeightTotal = 0;
    numVisibleGlobalFeatures_ = 0;

    for (i = 0; i < numGlobalFeatures; i++)
    {
        //if ((featureMenuUpGlobalFeatures_[i].hide) ||
        //    (featureMenuUpGlobalFeatures_[i].nonIndex))
        if (featureMenuUpGlobalFeatures_[i].nonIndex)
        {
            continue;
        }

        numVisibleGlobalFeatures_++;
        numOptions = featureMenuUpGlobalFeatures_[i].options.size();
        numOptionRows = 
            (numOptions/NUM_OPTION_COLUMNS) + 
            ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
        globalFeatureHeightTotal += 
            (featureLabelHeight + (numOptionRows*(featureMenuLineHeight+3)));
    }

    // Compute the total height of the local features.
    numLocalFeatures = featureMenuUpTask_.localFeatures.size();
    localFeatureHeightTotal = 0;
    numVisibleLocalFeatures_ = 0;

    for (i = 0; i < numLocalFeatures; i++)
    {
        //if ((featureMenuUpTask_.localFeatures[i].hide) ||
        //    (featureMenuUpTask_.localFeatures[i].nonIndex))
        if (featureMenuUpTask_.localFeatures[i].nonIndex)
        {
            continue;
        }

        numVisibleLocalFeatures_++;

        switch (featureMenuUpTask_.localFeatures[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:
            numOptions = featureMenuUpTask_.localFeatures[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
            localFeatureHeightTotal += 
                (featureLabelHeight + (numOptionRows*(featureMenuLineHeight+3)));
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:
        case MEXP_FEATURE_OPTION_POLYGONS:
            localFeatureHeightTotal += 
                (2*(numOptionRows*(featureMenuLineHeight+3)));
            break;
        }
    }

    if (askTaskType && (numTaskTypes_ > 0))
    {
        taskBoxHeight = (1+numTaskTypes_)*(featureMenuLineHeight+2);
        taskBoxHeightOffset = 6;
    }
    else
    {
        taskBoxHeight = 0;
        taskBoxHeightOffset = 0;
    }

    featureMenuHeight = 
        (taskBoxHeight+taskBoxHeightOffset) + // Task
        featureLabelHeight + // Top label
        actionSpaceHeight + // Save & Cancel Buttons
        globalFeatureHeightTotal +
        localFeatureHeightTotal;
            
    // Set other lengths.
    featureMenuWidth = DEFAULT_FEATUREMENU_WIDTH_;

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth - featureMenuWidth)/2;
    posY = (displayHeight - featureMenuHeight)/2;
    
    // Popup shell.
    featureMenu_w_ = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, featureMenuHeight,
         XmNwidth, featureMenuWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFormWidgetClass, featureMenu_w_,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, featureMenuHeight,
         XmNwidth, featureMenuWidth+6,
         NULL);

    stackedWidget.push_back(mainFrame);

    main = XtVaCreateWidget
        ("main",
         xmRowColumnWidgetClass, mainFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, featureMenuHeight,
         XmNwidth, featureMenuHeight,
         NULL);

    stackedWidget.push_back(main);

    // The frame for the label.
    labelFrame = XtVaCreateWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, featureMenuLineHeight-2,
         XmNwidth, featureMenuWidth,
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
         XmNheight, featureMenuLineHeight,
         XmNwidth, featureMenuWidth,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    label = XtVaCreateWidget
        (featureMenuTitle.c_str(),
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, featureMenuLineHeight,
         XmNwidth, featureMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    line++;

    // It is a 2D array. Create the 1D part (feature) first.
    globalFeaturesMenuOption_w_ = new Widget*[numVisibleGlobalFeatures_];

    //  Create the global feature raws sequentially.
    for (i = 0; i < numGlobalFeatures; i++)
    {
        //if ((featureMenuUpGlobalFeatures_[i].hide) ||
        //    (featureMenuUpGlobalFeatures_[i].nonIndex))
        if (featureMenuUpGlobalFeatures_[i].nonIndex)
        {
            continue;
        }

        switch (featureMenuUpGlobalFeatures_[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:

            numOptions = featureMenuUpGlobalFeatures_[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);

            // Height = label + options
            globalFeatureHeight = 
                (featureLabelHeight + (numOptionRows*(featureMenuLineHeight+3)));

            // The frame for the feature.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 //XmNy, line*featureMenuLineHeight,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // N-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, numOptionRows+1,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 NULL);

            stackedWidget.push_back(featureBox);

            // Create the second dimention (option) of this array.
            globalFeaturesMenuOption_w_[i] = new Widget[numOptions];

            // The label column.
            label = XtVaCreateWidget
                (featureMenuUpGlobalFeatures_[i].name.c_str(),
                 xmLabelGadgetClass, featureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, featureMenuLineHeight,
                 XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureBox,
                 NULL);

            stackedWidget.push_back(label);
        
            // Fill the blanks in the label row.
            for (j = 0; j < (NUM_OPTION_COLUMNS-1); j++)
            {
                label = XtVaCreateWidget
                    (" ",
                     xmLabelGadgetClass, featureBox,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, featureMenuLineHeight,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS,
                     XmNtopAttachment, XmATTACH_WIDGET,
                     XmNtopWidget, featureBox,
                     NULL);

                stackedWidget.push_back(label);
            }
        
            for (j = 0; j < numOptions; j++)
            {
                globalFeatureData = new GlobalFeatureMenuCallbackData_t;
                globalFeatureData->globalFeaturesId = featureMenuUpGlobalFeatures_[i].id;
                globalFeatureData->optionValue = (float)j;
                globalFeatureData->missionExpertInstance = this;
                globalFeatureData->optionType = featureMenuUpGlobalFeatures_[i].optionType;

                value = ((int)(featureMenuUpGlobalFeatures_[i].selectedOption) == j)? 1.0 : 0.0;

                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, featureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, featureMenuLineHeight,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNborderWidth, 0,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                // Toggle features.
                globalFeaturesMenuOption_w_[i][j] = XtVaCreateWidget
                    (featureMenuUpGlobalFeatures_[i].options[j].value.c_str(),
                     xmToggleButtonGadgetClass, buttonFrame,
                     XmNradioBehavior, true,
                     XmNradioAlwaysOne, true,
                     XmNheight, featureMenuLineHeight-3,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNset, value,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNshadowThickness, 0,
                     XmNhighlightColor, parentWidgetBg_,
                     NULL);
                
                stackedWidget.push_back(globalFeaturesMenuOption_w_[i][j]);

                XtAddCallback
                    (globalFeaturesMenuOption_w_[i][j],
                     XmNvalueChangedCallback,
                     (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                     (void *)globalFeatureData);
            }

            if (numOptions%NUM_OPTION_COLUMNS)
            {
                // Fill the space
                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, featureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, featureMenuLineHeight,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNborderWidth, 0,
                     NULL);

                stackedWidget.push_back(buttonFrame);
            }
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:

            // Height = label + options
            globalFeatureHeight = 
                (2*(featureMenuLineHeight+3));

            // The frame for the feature.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 //XmNy, line*featureMenuLineHeight,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // N-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, 1,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 NULL);

            stackedWidget.push_back(featureBox);

            // Create the second dimention (option) of this array.
            globalFeaturesMenuOption_w_[i] = new Widget[numOptions];

            switch (featureMenuUpGlobalFeatures_[i].optionType) {

            case MEXP_FEATURE_OPTION_SLIDER100:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER100_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER10:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER10_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 10;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER1:
            default:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER1_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                decimalPoints = 2;
                conversion = 100.0;
                break;
            }

            // The label column.
            label = XtVaCreateWidget
                (featureMenuUpGlobalFeatures_[i].name.c_str(),
                 xmLabelGadgetClass, featureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, 2*featureMenuLineHeight,
                 XmNwidth, featureMenuWidth/2,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureBox,
                 NULL);

            stackedWidget.push_back(label);

            globalFeatureData = new GlobalFeatureMenuCallbackData_t;
            globalFeatureData->globalFeaturesId = featureMenuUpGlobalFeatures_[i].id;
            globalFeatureData->optionValue = featureMenuUpGlobalFeatures_[i].selectedOption;
            globalFeatureData->missionExpertInstance = this;
            globalFeatureData->optionType = featureMenuUpGlobalFeatures_[i].optionType;

            value  = globalFeatureData->optionValue*conversion;

            // Toggle features.
            globalFeaturesMenuOption_w_[i][0] = XtVaCreateWidget
                ("",
                 xmScaleWidgetClass, featureBox,
                 XmNmaximum, maxValue,
                 XmNminimum, 1,
                 XmNvalue, value,
                 XmNorientation, XmHORIZONTAL,
                 XmNprocessingDirection, XmMAX_ON_RIGHT,
                 XmNshowValue, true,
                 XmNdecimalPoints, decimalPoints,
                 XmNheight, 2*featureMenuLineHeight,
                 XmNwidth, 3*featureMenuWidth/7,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, label,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNtitleString, sliderTitle,
                 XmNhighlightColor, parentWidgetBg_,
                 NULL);

            stackedWidget.push_back(globalFeaturesMenuOption_w_[i][0]);

            XtAddCallback
                (globalFeaturesMenuOption_w_[i][0],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                 (void *)globalFeatureData);

            XmStringFree(sliderTitle);

            break;

        case MEXP_FEATURE_OPTION_POLYGONS:

            // Height = label + options
            globalFeatureHeight = 
                (2*(featureMenuLineHeight+3));

            // The frame for the feature.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 //XmNy, line*featureMenuLineHeight,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // N-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, 1,
                 XmNheight, globalFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 NULL);

            stackedWidget.push_back(featureBox);

            // Create the second dimention (option) of this array.
            globalFeaturesMenuOption_w_[i] = new Widget[numOptions];

            polygonFrame = XtVaCreateWidget(
                "polygonFrame",
                xmFormWidgetClass, featureBox,
                XmNheight, 2*featureMenuLineHeight,
                XmNwidth, featureMenuWidth,
                XmNborderWidth, 0,
                XmNfractionBase, 20,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNbottomAttachment, XmATTACH_WIDGET,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNrightAttachment, XmATTACH_WIDGET,
                NULL);

            stackedWidget.push_back(polygonFrame);

            // The label column.
            label = XtVaCreateWidget
                (featureMenuUpGlobalFeatures_[i].name.c_str(),
                 xmLabelGadgetClass, polygonFrame,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 0,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 8,
                 NULL);

            stackedWidget.push_back(label);

            valueFrame = XtVaCreateManagedWidget
                ("valueFrame",
                 xmFormWidgetClass, polygonFrame,
                 XmNshadowThickness, 0,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 8,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 13,
                 XmNverticalSpacing, featureMenuLineHeight/3,
                 XmNborderWidth, 0,
                 NULL);

            stackedWidget.push_back(valueFrame);

            // The label column.
            sprintf(buf, "%.0f", featureMenuUpGlobalFeatures_[i].selectedOption);
            globalFeaturesMenuOption_w_[i][0] = XtVaCreateWidget
                (buf,
                 xmTextWidgetClass, valueFrame,
                 XmNheight, featureMenuLineHeight,
                 XmNwidth, featureMenuWidth/4,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNvalue, buf,
                 XmNautoShowCursorPosition, false,
                 XmNcursorPositionVisible, false,
                 XmNeditable, false,
                 NULL);

            stackedWidget.push_back(globalFeaturesMenuOption_w_[i][0]);
            
            buttonFrame = XtVaCreateManagedWidget
                ("buttonFrame",
                 xmFormWidgetClass, polygonFrame,
                 XmNshadowThickness, 0,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 14,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 18,
                 XmNverticalSpacing, featureMenuLineHeight/3,
                 XmNborderWidth, 0,
                 NULL);

            stackedWidget.push_back(buttonFrame);

            button = XtVaCreateWidget
                (STRING_ADD_.c_str(),
                 xmPushButtonWidgetClass, buttonFrame,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNhighlightOnEnter, false,
                 XmNtraversalOn, false,
                 NULL);
            XtSetSensitive(button, false);

            stackedWidget.push_back(button);
            break;
        }
        line += 2;
    }

    // The frame for the task.
    if (askTaskType && (numTaskTypes_ > 0))
    {
        taskFrame = XtVaCreateWidget
            ("taskFrame",
             xmFrameWidgetClass, main,
             XmNy, line*featureMenuLineHeight+1,
             XmNheight, taskBoxHeight,
             XmNwidth, featureMenuWidth,
             NULL);

        stackedWidget.push_back(taskFrame);

        taskBox = XtVaCreateWidget
            ("taskBox",
             xmRowColumnWidgetClass, taskFrame,
             XmNpacking, XmPACK_COLUMN,
             XmNorientation, XmVERTICAL,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNnumColumns, 1,
             XmNheight, taskBoxHeight,
             XmNwidth, featureMenuWidth,
             NULL);

        stackedWidget.push_back(taskBox);

        // The label column.
        label = XtVaCreateWidget
            (taskLabel.c_str(),
             xmLabelGadgetClass, taskBox,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, featureMenuLineHeight,
             XmNwidth, featureMenuWidth,
             XmNtopAttachment, XmATTACH_WIDGET,
             XmNtopWidget, taskBox,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftWidget, taskBox,
             NULL);

        stackedWidget.push_back(label);

        line++;

        featureMenuTask_w_ = new Widget[numTaskTypes_];

        for (i = 0; i < numTaskTypes_; i++)
        {
            task = defaultTaskList_[i];

            set = (task.name == featureMenuUpTask_.name)? true : false;

            featureMenuTask_w_[i] = XtVaCreateWidget
                (task.name.c_str(),
                 xmToggleButtonGadgetClass, taskBox,
                 XmNradioBehavior, true,
                 XmNradioAlwaysOne, true,
                 XmNheight, featureMenuLineHeight,
                 XmNwidth, featureMenuWidth,
                 XmNset, set,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, label,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, taskBox,
                 NULL);

            stackedWidget.push_back(featureMenuTask_w_[i]);

            line++;

            taskData = new FeatureMenuTaskCallbackData_t;
            taskData->taskId = task.id;
            taskData->missionExpertInstance = this;
            XtAddCallback
                (featureMenuTask_w_[i],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbFeatureMenuTaskValue_,
                 (void *)taskData);

        }
    }

    featureMenuOption_w_ = new Widget*[numVisibleLocalFeatures_];

    //  Create the feature raws sequentially.
    for (i = 0; i < numLocalFeatures; i++)
    {
        //if ((featureMenuUpTask_.localFeatures[i].hide) ||
        //    (featureMenuUpTask_.localFeatures[i].nonIndex))
        if (featureMenuUpTask_.localFeatures[i].nonIndex)
        {
            continue;
        }

        switch (featureMenuUpTask_.localFeatures[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:

            numOptions = featureMenuUpTask_.localFeatures[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);

            // Height = label + options
            localFeatureHeight = 
                (featureLabelHeight + (numOptionRows*(featureMenuLineHeight+3)));

            // The frame for the feature.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 //XmNy, line*featureMenuLineHeight+2,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, localFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // N-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, numOptionRows+1,
                 //XmNnumColumns, NUM_OPTION_COLUMNS,
                 XmNheight, localFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 NULL);

            stackedWidget.push_back(featureBox);
        
            // Create the second dimention (option) of this array.
            featureMenuOption_w_[i] = new Widget[numOptions];

            // The label column.
            label = XtVaCreateWidget
                (featureMenuUpTask_.localFeatures[i].name.c_str(),
                 xmLabelGadgetClass, featureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, featureMenuLineHeight,
                 XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureBox,
                 NULL);

            stackedWidget.push_back(label);

            // Fill the blanks in the label row.
            for (j = 0; j < (NUM_OPTION_COLUMNS-1); j++)
            {
                label = XtVaCreateWidget
                    (" ",
                     xmLabelGadgetClass, featureBox,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, featureMenuLineHeight,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS,
                     XmNtopAttachment, XmATTACH_WIDGET,
                     XmNtopWidget, featureBox,
                     NULL);

                stackedWidget.push_back(label);
            }

            for (j = 0; j < numOptions; j++)
            {
                featureOptionData = new FeatureMenuCallbackData_t;
                featureOptionData->featureId = i;
                featureOptionData->optionValue = (float)j;
                featureOptionData->missionExpertInstance = this;
                featureOptionData->optionType = featureMenuUpTask_.localFeatures[i].optionType;

                value = ((int)(featureMenuUpTask_.localFeatures[i].selectedOption) == j)? 1.0 : 0.0;

                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, featureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, featureMenuLineHeight-3,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNborderWidth, 0,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                // Toggle features.
                featureMenuOption_w_[i][j] = XtVaCreateWidget
                    (featureMenuUpTask_.localFeatures[i].options[j].value.c_str(),
                     xmToggleButtonGadgetClass, buttonFrame,
                     XmNradioBehavior, true,
                     XmNradioAlwaysOne, true,
                     XmNheight, featureMenuLineHeight-3,
                     XmNwidth, featureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNset, value,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNshadowThickness, 0,
                     XmNhighlightColor, parentWidgetBg_,
                     NULL);

                stackedWidget.push_back(featureMenuOption_w_[i][j]);

                XtAddCallback
                    (featureMenuOption_w_[i][j],
                     XmNvalueChangedCallback,
                     (XtCallbackProc)cbFeatureMenuValue_,
                     (void *)featureOptionData);
            }

            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:

            // Height = label + options
            localFeatureHeight = 
                (2*(featureMenuLineHeight+3));

            // The frame for the feature.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 //XmNy, line*featureMenuLineHeight+2,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, localFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // N-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, 1,
                 //XmNnumColumns, NUM_OPTION_COLUMNS,
                 XmNheight, localFeatureHeight,
                 XmNwidth, featureMenuWidth,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 NULL);

            stackedWidget.push_back(featureBox);
        
            // Create the second dimention (option) of this array.
            featureMenuOption_w_[i] = new Widget[numOptions];

            switch (featureMenuUpTask_.localFeatures[i].optionType) {

            case MEXP_FEATURE_OPTION_SLIDER100:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER100_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER10:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER10_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 10;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER1:
            default:
                sliderTitle = XmStringCreateLtoR(
                    (String)MEXP_DEFAULT_SLIDER1_RANAGE_TITLE.c_str(),
                    XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                decimalPoints = 2;
                conversion = 100.0;
                break;
            }

            // The label column.
            label = XtVaCreateWidget
                (featureMenuUpTask_.localFeatures[i].name.c_str(),
                 xmLabelGadgetClass, featureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, 2*featureMenuLineHeight,
                 XmNwidth, featureMenuWidth/2,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureBox,
                 NULL);

            stackedWidget.push_back(label);

            featureOptionData = new FeatureMenuCallbackData_t;
            featureOptionData->featureId = i;
            featureOptionData->optionValue = featureMenuUpTask_.localFeatures[i].selectedOption;
            featureOptionData->missionExpertInstance = this;
            featureOptionData->optionType = featureMenuUpTask_.localFeatures[i].optionType;

            value = featureOptionData->optionValue*conversion;

            featureMenuOption_w_[i][0] = XtVaCreateWidget
                ("",
                 xmScaleWidgetClass, featureBox,
                 XmNmaximum, maxValue,
                 XmNminimum, 1,
                 XmNvalue, value,
                 XmNorientation, XmHORIZONTAL,
                 XmNprocessingDirection, XmMAX_ON_RIGHT,
                 XmNshowValue, true,
                 XmNdecimalPoints, decimalPoints,
                 XmNheight, 2*featureMenuLineHeight,
                 XmNwidth, 3*featureMenuWidth/7,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, label,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNtitleString, sliderTitle,
                 XmNhighlightColor, parentWidgetBg_,
                 NULL);

            stackedWidget.push_back(featureMenuOption_w_[i][0]);

            XtAddCallback
                (featureMenuOption_w_[i][0],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbFeatureMenuValue_,
                 (void *)featureOptionData);

            XmStringFree(sliderTitle);

            break;

        case MEXP_FEATURE_OPTION_POLYGONS:
            // To be implemented.
            break;
        }
        line += 2;
    }

    // The frame for the action field.
    actionFrame = XtVaCreateManagedWidget
        ("actionFrame",
         xmFormWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*featureMenuLineHeight,
         XmNwidth, featureMenuWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, main,
         NULL);

    stackedWidget.push_back(actionFrame);

    action = XtVaCreateWidget
        ("action",
         xmRowColumnWidgetClass, actionFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*featureMenuLineHeight,
         XmNwidth, featureMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(action);

    // The frame for the "Save" buttons.
    saveFrame = XtVaCreateWidget
        ("saveFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*featureMenuLineHeight-10,
         XmNwidth, featureMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, action,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(saveFrame);

    // The "Save" button.
    saveBtn = XtVaCreateWidget
        (saveLabel.c_str(),
         xmPushButtonGadgetClass, saveFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*featureMenuLineHeight-10,
         XmNwidth, featureMenuWidth/2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(saveBtn);

    XtAddCallback
        (saveBtn,
         XmNactivateCallback, 
         (XtCallbackProc)cbSaveCase_,
         this);

    // The frame for the "Cancel" buttons.
    cancelFrame = XtVaCreateWidget
        ("cancelFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNx, featureMenuWidth/2,
         XmNheight, 2*featureMenuLineHeight-10,
         XmNwidth, featureMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, action,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(cancelFrame);

    // The "Cancel" button
    cancelBtn = XtVaCreateWidget
        (cancelLabel.c_str(),
         xmPushButtonGadgetClass, cancelFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*featureMenuLineHeight-10,
         XmNwidth, featureMenuWidth/2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(cancelBtn);

    XtAddCallback
        (cancelBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbCancelSaveCase_,
         (void *)this);

    featureMenuWidgets_ = stackedWidget;
    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
        //XtManageChild(stackedWidget.back());
        //stackedWidget.pop_back();
        XFlush(display_);
    }

    XtPopup (featureMenu_w_, XtGrabNone);
    XFlush(display_);

    // Wait for the user to close the window.
    while(featureMenuIsUp_)
    {
        // Run X
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
}

//-----------------------------------------------------------------------
// This function changes the value of the case feature.
//-----------------------------------------------------------------------
void MissionExpert::changeFeatureMenuValue_(XtPointer clientData)
{
    int i, j;
    bool selected;
    int sliderBarValue;
    float optionValue;
    char optionValueStr[256];

    FeatureMenuCallbackData_t *data = (FeatureMenuCallbackData_t *)clientData;
    i = data->featureId;

    switch (data->optionType) {

    case MEXP_FEATURE_OPTION_TOGGLE:
        featureMenuUpTask_.localFeatures[i].selectedOption = data->optionValue;
        
        for (j = 0; j < ((int)(featureMenuUpTask_.localFeatures[i].options.size())); j++)
        {
            selected = ((int)(featureMenuUpTask_.localFeatures[i].selectedOption) == j)? true : false;

            XtVaSetValues(
                featureMenuOption_w_[i][j],
                XmNset, selected,
                NULL);
        }
        break;

    case MEXP_FEATURE_OPTION_SLIDER100:
    case MEXP_FEATURE_OPTION_SLIDER10:
    case MEXP_FEATURE_OPTION_SLIDER1:

        XtVaGetValues(
            featureMenuOption_w_[i][0],
            XmNvalue, &sliderBarValue,
            NULL);

        switch (data->optionType) {

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
            optionValue = (float)sliderBarValue;
            sprintf(optionValueStr, "%d", sliderBarValue);
            break;

        case MEXP_FEATURE_OPTION_SLIDER1:
        default:
            optionValue = (float)sliderBarValue/100.0;
        sprintf(optionValueStr, "%.2f", optionValue);
            break;
        }

        featureMenuUpTask_.localFeatures[i].selectedOption = optionValue;
        featureMenuUpTask_.localFeatures[i].options[0].value = optionValueStr;

        break;
    }
}

//-----------------------------------------------------------------------
// This function changes the value of the global feature.
//-----------------------------------------------------------------------
void MissionExpert::changeGlobalFeatureMenuValue_(XtPointer clientData)
{
    int i, id, index;
    bool selected;
    int sliderBarValue;
    float optionValue;
    char optionValueStr[256];

    GlobalFeatureMenuCallbackData_t *data = (GlobalFeatureMenuCallbackData_t *)clientData;
    id = data->globalFeaturesId;

    index = 0;

    for (i = 0; i < ((int)(featureMenuUpGlobalFeatures_.size())); i++)
    {
        if (featureMenuUpGlobalFeatures_[i].id == id)
        {
            index = i;
            break;
        }
    }

    switch (data->optionType) {

    case MEXP_FEATURE_OPTION_TOGGLE:
        featureMenuUpGlobalFeatures_[index].selectedOption = data->optionValue;
        
        for (i = 0; i < ((int)(featureMenuUpGlobalFeatures_[index].options.size())); i++)
        {
            selected = ((int)(featureMenuUpGlobalFeatures_[index].selectedOption) == i)? true : false;

            XtVaSetValues(
                globalFeaturesMenuOption_w_[index][i],
                XmNset, selected,
                NULL);
        }

        break;

    case MEXP_FEATURE_OPTION_SLIDER100:
    case MEXP_FEATURE_OPTION_SLIDER10:
    case MEXP_FEATURE_OPTION_SLIDER1:

        XtVaGetValues(
            globalFeaturesMenuOption_w_[index][0],
            XmNvalue, &sliderBarValue,
            NULL);

        switch (data->optionType) {

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
            optionValue = (float)sliderBarValue;
            sprintf(optionValueStr, "%d", sliderBarValue);
            break;

        case MEXP_FEATURE_OPTION_SLIDER1:
        default:
            optionValue = (float)sliderBarValue/100.0;
            sprintf(optionValueStr, "%.2f", optionValue);
            break;
        }

        featureMenuUpGlobalFeatures_[index].selectedOption = optionValue;
        featureMenuUpGlobalFeatures_[index].options[0].value = optionValueStr;

        break;

    case MEXP_FEATURE_OPTION_POLYGONS:
        break;
    }
}

//-----------------------------------------------------------------------
// This function changes the ID of the current task to be saved, and
// highlights the appropriate button of the case feature menu window.
//-----------------------------------------------------------------------
void MissionExpert::changeFeatureMenuTaskValue_(int taskId)
{
    int i;
    bool set;

    featureMenuUpTask_.id = taskId;

    for (i = 0; i < numTaskTypes_; i++)
    {
        if (defaultTaskList_[i].id == taskId)
        {
            featureMenuUpTask_.name = defaultTaskList_[i].name;
            break;
        }
    }

    for (i = 0; i < numTaskTypes_; i++)
    {
        set = (i == taskId)? true : false;

        XtVaSetValues(
            featureMenuTask_w_[i],
            XmNset, set,
            NULL);
    }
}
//-----------------------------------------------------------------------
// This function creates a status window and pops it up.
//-----------------------------------------------------------------------
void MissionExpert::popupStatusWindow_(
    int rating,
    string missionSummary,
    bool prevBtnIsSensitive,
    bool nextBtnIsSensitive,
    bool loadBtnIsSensitive,
    bool cancelBtnIsSensitive,
    int currentMissionId)
{
    Widget mainFrame, main;
    Widget labelFrame, labelBox, label;
    Widget ratingFrame;
    Widget textFrame;
    Widget navigationFrame, navigation;
    Widget prevFrame;
    Widget nextFrame;
    Widget actionFrame, action;
    Widget loadFrame;
    Widget cancelFrame;
    vector<Widget> stackedWidget;
    Pixmap pixmap;
    string titleBoxText;
    string windowTitle;
    string ratingLabel;
    string navigationLabel;
    string prevLabel;
    string nextLabel;
    string loadLabel;
    string cancelLabel;
    Arg wargs[16];
    char buf[1024];
    int i, n, line = 0;
    int screenNumber;
    int displayHeight, displayWidth;
    int posX, posY;
    int statusWindowHeight, statusWindowWidth;
    int statusWindowLineHeight;
    int toptitleHeight, textHeight;
    int numStackedWidget;
    int numTextRows;
    int maxRetrievedMissions;
    bool isSensitive = false;

    // Create the title box comment.
    if (currentMissionId > -1)
    {
        maxRetrievedMissions = retrievedMissionSummary_.fsaSummaries.size();
        sprintf(buf, " %d/%d", currentMissionId+1, maxRetrievedMissions);
        titleBoxText = buf;
    }
    else
    {
        titleBoxText = " N/A";
    }

    // Create other labels.
    windowTitle =  DEFAULT_STATUSWINDOW_TITLE_;
    ratingLabel = DEFAULT_STATUSWINDOW_RATING_LABEL_;
    navigationLabel = DEFAULT_STATUSWINDOW_NAVIGATION_LABEL_;
    prevLabel = DEFAULT_STATUSWINDOW_PREV_BUTTON_LABEL_;
    nextLabel = DEFAULT_STATUSWINDOW_NEXT_BUTTON_LABEL_;
    loadLabel = DEFAULT_STATUSWINDOW_LOAD_BUTTON_LABEL_;
    cancelLabel = DEFAULT_STATUSWINDOW_CANCEL_BUTTON_LABEL_;

    // Create dimensions of the window.
    statusWindowWidth = DEFAULT_STATUSWINDOW_WIDTH_;
    statusWindowLineHeight = DEFAULT_STATUSWINDOW_LINEHEIGHT_;
    toptitleHeight = DEFAULT_STATUSWINDOW_TOPTITLE_HEIGHT_;
    textHeight = DEFAULT_STATUSWINDOW_TEXT_HEIGHT_;
    numTextRows = DEFAULT_STATUSWINDOW_TEXT_NUMROWS_;
    statusWindowHeight = (int)(
        textHeight + 15 +
        (statusWindowLineHeight+5)*
        (1 + // Top label
         2 + // Navigation frame
         2) // Save & Cancel buttons;
        );

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = (displayWidth - statusWindowWidth)/2;
    posY = (displayHeight - statusWindowHeight)/2;
    
    // Popup shell.
    statusWindows_.popupShell = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, statusWindowHeight,
         XmNwidth, statusWindowWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFormWidgetClass, statusWindows_.popupShell,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, statusWindowHeight,
         XmNwidth, statusWindowWidth,
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
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNheight, statusWindowHeight,
         XmNwidth, statusWindowHeight,
         NULL);

    stackedWidget.push_back(main);

    // The frame for the label.
    labelFrame = XtVaCreateWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, toptitleHeight,
         XmNwidth, statusWindowWidth-5,
         NULL);

    stackedWidget.push_back(labelFrame);

    // The widget for the main label.
    labelBox = XtVaCreateWidget
        ("labelBox",
         xmRowColumnWidgetClass, labelFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmHORIZONTAL,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 2,
         XmNheight, toptitleHeight,
         XmNwidth, statusWindowWidth-5,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    label = XtVaCreateWidget
        (windowTitle.c_str(),
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, toptitleHeight,
         XmNwidth, 4*statusWindowWidth/5-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    statusWindows_.titleBox = XtVaCreateWidget
        ("",
         xmTextWidgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, toptitleHeight-5,
         XmNwidth, statusWindowWidth/5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, label,
         XmNvalue, titleBoxText.c_str(),
         XmNcursorPositionVisible, false,
         XmNeditable, false,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    stackedWidget.push_back(statusWindows_.titleBox);

    line++;

    // The frame for the status.
    ratingFrame = XtVaCreateWidget
        ("ratingFrame",
         xmFrameWidgetClass, main,
         XmNy, line*statusWindowLineHeight,
         XmNheight, statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         NULL);

    stackedWidget.push_back(ratingFrame);

    statusWindows_.ratingBox = XtVaCreateWidget
        ("ratingBox",
         xmRowColumnWidgetClass, ratingFrame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmHORIZONTAL,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, MAX_SUITABILITY_RATING_+1,
         XmNheight, statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(statusWindows_.ratingBox);

    // The label column.
    label = XtVaCreateWidget
        (ratingLabel.c_str(),
         xmLabelGadgetClass, statusWindows_.ratingBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, statusWindowLineHeight,
         XmNwidth, statusWindowWidth/2,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, statusWindows_.ratingBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, statusWindows_.ratingBox,
         NULL);

    stackedWidget.push_back(label);
        
    for (i = 0; i < MAX_SUITABILITY_RATING_; i++)
    {
        pixmap = (i < rating)? starPixmap_ : emptyStarPixmap_;

        // Rating box
        statusWindows_.starBox[i] = XtVaCreateWidget
            ("star",
             xmLabelGadgetClass, statusWindows_.ratingBox,
             XmNlabelType, XmPIXMAP,
             XmNlabelPixmap, pixmap,
             XmNtopAttachment, XmATTACH_WIDGET,
             XmNtopWidget, statusWindows_.ratingBox,
             XmNbottomAttachment, XmATTACH_WIDGET,
             XmNbottomWidget, statusWindows_.ratingBox,
             NULL);
        
        stackedWidget.push_back(statusWindows_.starBox[i]);
    }

    line ++;

    // The frame for the text field.
    textFrame = XtVaCreateManagedWidget
        ("textFrame",
         xmFormWidgetClass, main,
         XmNy, line*statusWindowLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 4*statusWindowLineHeight,
         XmNwidth, textHeight,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(textFrame);

    n = 0;
    XtSetArg(wargs[n], XmNscrollVertical, true); n++;
    XtSetArg(wargs[n], XmNscrollHorizontal, false); n++;
    XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(wargs[n], XmNeditable, false); n++;
    XtSetArg(wargs[n], XmNcursorPositionVisible, false); n++;
    XtSetArg(wargs[n], XmNwordWrap, true); n++;
    XtSetArg(wargs[n], XmNrows, numTextRows); n++;
    statusWindows_.textBox = XmCreateScrolledText(textFrame, "textBox", wargs, n);

    XmTextReplace(statusWindows_.textBox, 0, statusWindows_.textPos, NULL);
    statusWindows_.textPos = 0;
    XmTextInsert(statusWindows_.textBox, 0, (char *)(missionSummary.c_str()));
    statusWindows_.textPos += missionSummary.size();
    XtVaSetValues(
        statusWindows_.textBox,
        XmNcursorPosition, statusWindows_.textPos,
        NULL);
    XmTextShowPosition(statusWindows_.textBox, 0);

    XtVaSetValues(
        XtParent(statusWindows_.textBox),
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNhighlightOnEnter, false,
        XmNtraversalOn, false,
        NULL);

    stackedWidget.push_back(statusWindows_.textBox);

    // The frame for the navigation field.
    navigationFrame = XtVaCreateManagedWidget
        ("navigationFrame",
         xmFormWidgetClass, main,
         XmNy, line*statusWindowLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(navigationFrame);

    navigation = XtVaCreateWidget
        ("navigation",
         xmRowColumnWidgetClass, navigationFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(navigation);

    // The label column.
    label = XtVaCreateWidget
        (navigationLabel.c_str(),
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2-5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, navigation,
         NULL);

    stackedWidget.push_back(label);
        
    // The frame for the "Previous" buttons.
    prevFrame = XtVaCreateWidget
        ("prevFrame",
         xmFormWidgetClass, navigation,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, statusWindowLineHeight-5,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, navigation,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, navigation,
         NULL);

    stackedWidget.push_back(prevFrame);

    // The "Previous" button.
    statusWindows_.prevBtn = XtVaCreateWidget
        (prevLabel.c_str(),
         xmPushButtonGadgetClass, prevFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, statusWindowLineHeight-5,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);


    isSensitive = (prevBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.prevBtn, isSensitive);

    XtAddCallback
        (statusWindows_.prevBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbPrevBtnStatusWindow_,
         this);

    stackedWidget.push_back(statusWindows_.prevBtn);

    // A blank.
    label = XtVaCreateWidget
        (EMPTY_STRING_.c_str(),
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2-5,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, navigation,
         NULL);

    stackedWidget.push_back(label);
        
    // The frame for the "Next" buttons.
    nextFrame = XtVaCreateWidget
        ("nextFrame",
         xmFormWidgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNx, statusWindowWidth/2,
         XmNheight, statusWindowLineHeight-5,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, navigation,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, navigation,
         NULL);

    stackedWidget.push_back(nextFrame);

    // The "Next" button
    statusWindows_.nextBtn = XtVaCreateWidget
        (nextLabel.c_str(),
         xmPushButtonGadgetClass, nextFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, statusWindowLineHeight-5,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    isSensitive = (nextBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.nextBtn, isSensitive);

    XtAddCallback
        (statusWindows_.nextBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbNextBtnStatusWindow_,
         this);

    stackedWidget.push_back(statusWindows_.nextBtn);

    // The frame for the action field.
    actionFrame = XtVaCreateManagedWidget
        ("actionFrame",
         xmFormWidgetClass, main,
         XmNy, line*statusWindowLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(actionFrame);

    action = XtVaCreateWidget
        ("action",
         xmRowColumnWidgetClass, actionFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*statusWindowLineHeight,
         XmNwidth, statusWindowWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(action);

    // The frame for the "Load" buttons.
    loadFrame = XtVaCreateWidget
        ("loadFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, action,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(loadFrame);

    // The "Load" button.
    statusWindows_.loadBtn = XtVaCreateWidget
        (loadLabel.c_str(),
         xmPushButtonGadgetClass, loadFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    isSensitive = (loadBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.loadBtn, isSensitive);

    XtAddCallback
        (statusWindows_.loadBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbLoadBtnStatusWindow_,
         this);

    stackedWidget.push_back(statusWindows_.loadBtn);

    // The frame for the "Cancel" buttons.
    cancelFrame = XtVaCreateWidget
        ("cancelFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNx, statusWindowWidth/2,
         XmNheight, 2*statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, action,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(cancelFrame);

    // The "Cancel" button
    statusWindows_.cancelBtn = XtVaCreateWidget
        (cancelLabel.c_str(),
         xmPushButtonGadgetClass, cancelFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*statusWindowLineHeight-10,
         XmNwidth, statusWindowWidth/2,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    isSensitive = (cancelBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.cancelBtn, isSensitive);

    XtAddCallback
        (statusWindows_.cancelBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbCancelBtnStatusWindow_,
         (void *)this);

    stackedWidget.push_back(statusWindows_.cancelBtn);

    numStackedWidget = stackedWidget.size();

    if (missionSpecWizardEnabled() && (missionSpecWizard_ != NULL))
    {
        // Do not manage.
    }
    else
    {
        for (i = 0; i < numStackedWidget; i++)
        {
            XtManageChild(stackedWidget.back());
            stackedWidget.pop_back();
        }
    }

    statusWindows_.isUp = true;
    statusWindows_.currentMissionId = currentMissionId;
    statusWindows_.selectedMissionId = -1;

    gEventLogging->start(DEFAULT_STATUSWINDOW_TITLE_.c_str());

    if (missionSpecWizardEnabled() && (missionSpecWizard_ != NULL))
    {
        // Do not pop up.
    }
    else
    {
        XtPopup (statusWindows_.popupShell, XtGrabNone);
    }

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
}

//-----------------------------------------------------------------------
// This function updates the status windows.
//-----------------------------------------------------------------------
void MissionExpert::updateStatusWindow_(
    int rating,
    string missionSummary,
    bool prevBtnIsSensitive,
    bool nextBtnIsSensitive,
    bool loadBtnIsSensitive,
    bool cancelBtnIsSensitive,
    int currentMissionId)
{
    Pixmap pixmap, oldPixmap;
    string titleBoxText;
    char buf[1024];
    int i, maxRetrievedMissions;
    bool isSensitive = false;

    // Create the title box comment.
    if (currentMissionId > -1)
    {
        maxRetrievedMissions = retrievedMissionSummary_.fsaSummaries.size();
        sprintf(buf, " %d/%d", currentMissionId+1, maxRetrievedMissions);
        titleBoxText = buf;
    }
    else
    {
        titleBoxText = " N/A";
    }

    XtVaSetValues(
        statusWindows_.titleBox,
        XmNvalue, titleBoxText.c_str(),
        NULL);

    // Update rating.
    for (i = 0; i < MAX_SUITABILITY_RATING_; i++)
    {
        XtVaGetValues(
            statusWindows_.starBox[i],
            XmNlabelPixmap, &oldPixmap,
            NULL);

        XmDestroyPixmap(XtScreen(statusWindows_.popupShell), oldPixmap);

        while (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }

        pixmap = (i < rating)? starPixmap_ : emptyStarPixmap_;

        XtVaSetValues(
            statusWindows_.starBox[i],
            XmNlabelPixmap, pixmap,
            NULL);

        while (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
    }

    // Update the mission summary
    XmTextReplace(statusWindows_.textBox, 0, statusWindows_.textPos, NULL);
    statusWindows_.textPos = 0;
    XmTextInsert(statusWindows_.textBox, 0, (char *)(missionSummary.c_str()));
    statusWindows_.textPos += missionSummary.size();
    XtVaSetValues(
        statusWindows_.textBox,
        XmNcursorPosition, statusWindows_.textPos,
        NULL);
    XmTextShowPosition(statusWindows_.textBox, 0);

    // Update sensitivity of buttons.
    isSensitive = (prevBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.prevBtn, isSensitive);

    isSensitive = (nextBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.nextBtn, isSensitive);

    isSensitive = (loadBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.loadBtn, isSensitive);

    isSensitive = (cancelBtnIsSensitive)? true : false;
    XtSetSensitive(statusWindows_.cancelBtn, isSensitive);

    statusWindows_.currentMissionId = currentMissionId;

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
}

//-----------------------------------------------------------------------
// This function updates the status windows.
//-----------------------------------------------------------------------
void MissionExpert::pendXCommandUpdateStatusWindow_(
    int rating,
    string missionSummary,
    bool prevBtnIsSensitive,
    bool nextBtnIsSensitive,
    bool loadBtnIsSensitive,
    bool cancelBtnIsSensitive,
    int currentMissionId)
{
    PendingThreadedXCommands_t cmd;
    XCommandUpdateStatesWindow_t data;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);

    cmd.type = X_COMMAND_UPDATE_STATUS_WINDOW;
    cmd.index = shXCommandUpdateStatesWindowList_.size();
    shPendingThreadedXCommands_.push_back(cmd);

    data.rating = rating;
    data.missionSummary = missionSummary;
    data.prevBtnIsSensitive = prevBtnIsSensitive;
    data.nextBtnIsSensitive = nextBtnIsSensitive;
    data.loadBtnIsSensitive = loadBtnIsSensitive;
    data.cancelBtnIsSensitive = cancelBtnIsSensitive;
    data.currentMissionId = currentMissionId;
    shXCommandUpdateStatesWindowList_.push_back(data);

    pthread_cleanup_pop(1);
}

//-----------------------------------------------------------------------
// This function closes status window.
//-----------------------------------------------------------------------
void MissionExpert::closeStatusWindow_(void)
{
    Pixmap oldPixmap;
    int i;

    gEventLogging->end(DEFAULT_STATUSWINDOW_TITLE_.c_str());

    if (!(statusWindows_.isUp))
    {
        return;
    }

    statusWindows_.textPos = 0;
    statusWindows_.isUp = false;

    for (i = 0; i < MAX_SUITABILITY_RATING_; i++)
    {
        XtVaGetValues(
            statusWindows_.starBox[i],
            XmNlabelPixmap, &oldPixmap,
            NULL);

        XmDestroyPixmap(XtScreen(statusWindows_.popupShell), oldPixmap);

        while (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
    }

    XtDestroyWidget(statusWindows_.popupShell);

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
}

//-----------------------------------------------------------------------
// This function browse next loadable mission in the status window.
//-----------------------------------------------------------------------
void MissionExpert::changeStatusWindow_(int direction)
{
    int index, maxRetrievedMissions;
    bool prevBtnIsSensitive, nextBtnIsSensitive;

    index = statusWindows_.currentMissionId;
    maxRetrievedMissions = retrievedMissionSummary_.fsaSummaries.size();

    switch (direction) {

    case STATUS_WINDOW_PREV:
        if ((index-1) >= 0)
        {
            index--;
        }
        break;

    case STATUS_WINDOW_NEXT:
        if ((index+1) < maxRetrievedMissions)
        {
            index++;
        }
        break;
    }

    prevBtnIsSensitive = (index > 0)? true : false;
    nextBtnIsSensitive = ((index+1) < maxRetrievedMissions)? true : false;

    updateStatusWindow_(
        retrievedMissionSummary_.ratings[index],
        retrievedMissionSummary_.fsaSummaries[index],
        prevBtnIsSensitive,
        nextBtnIsSensitive,
        true,
        true,
        index);
}

//-----------------------------------------------------------------------
// This function loads the mission that was selected by the user from
// the summary (i.e., the user input to the status window).
//-----------------------------------------------------------------------
bool MissionExpert::loadRetrievedMissionChosenByUser_(
    vector<MExpRetrievedMission_t> retrievedMissions,
    string overlayFileName,
    int *selectedIndex)
{
    int numRetrievedMisssions = 0, selectedMissionId = -1;
    bool missionLoaded = false;

    numRetrievedMisssions = retrievedMissions.size();

    // Wait for the user input
    while (statusWindows_.isUp)
    {
        while (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }

        nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);
    }

    // Make sure a valid mission ID is selected.
    selectedMissionId = statusWindows_.selectedMissionId;

    if ((selectedMissionId < 0) || (selectedMissionId >= numRetrievedMisssions))
    {
        return false;
    }

    *selectedIndex = selectedMissionId;
    
    missionLoaded = loadRetrievedMission_(
        retrievedMissions[selectedMissionId],
        overlayFileName);

    return missionLoaded;
}

//-----------------------------------------------------------------------
// This function loads the retrived mission given an index.
//-----------------------------------------------------------------------
bool MissionExpert::loadRetrievedMission_(
    MExpRetrievedMission_t retrievedMission,
    string overlayFileName)
{
    FILE *resultFile = NULL;
    char *cdlSolution = NULL;
    int status;
    bool missionLoaded = false;

    // Open the result file.
    resultFile = fopen(resultFileName_.c_str(), "w");
    if (resultFile == NULL)
    {
        return false;
    }
    else
    {
        addCreatedDataFileList_(resultFileName_);
    }

    cdlSolution = acdl2cdl(
        retrievedMission.acdlSolution.c_str(),
        robotName_,
        &status);

    if ((cdlSolution != NULL) && (status != ACDL2CDL_STATUS_FAILURE))
    {
        if (status == ACDL2CDL_STATUS_EMPTYMISSION)
        {
            warn_userf("Retrieved mission is empty.");
        }

        // Copy the solution to the result file.
        fprintf(resultFile, "%s", cdlSolution);

        // Check the overlay file name.
        if (overlayFileName != EMPTY_STRING_)
        {
            overlayFileName_ = overlayFileName;
        }

        missionLoaded = true;
        currentCBRLibraryDataIndexList_ = retrievedMission.dataIndexList;
    }
    else
    {
        warn_userf("Retrieved mission cound not be convered to CDL.");
        missionLoaded = false;
    }
    
    fclose(resultFile);

    return missionLoaded;
}

//-----------------------------------------------------------------------
// This function saves the current feature and the closes the case feature
// menu window.
//-----------------------------------------------------------------------
void MissionExpert::saveCase_(void) 
{
    vector<MExpFeature_t> globalFeatures;
    int i;

    // Combine the global features.
    globalFeatures = fixedGlobalFeatures_;

    for (i = 0; i < (int)(featureMenuUpGlobalFeatures_.size()); i++)
    {
        globalFeatures.push_back(featureMenuUpGlobalFeatures_[i]);
    }

    currentCBRLibraryDataIndexList_ = cfgeditCBRClient_->saveMissionPlan(
        globalFeatures,
        featureMenuUpTask_,
        defaultTaskWeight_,
        missionPlanFileName_);

    closeFeatureMenu_();

    missionSavedToCBRLibrary_ = true;
    shouldSaveMissionToCBRLibrary_ = false;

    if (SEND_METADATA_TO_LIBRARY_)
    {
        popupMetadataEntryWindow_(currentCBRLibraryDataIndexList_);
    }
}

//-----------------------------------------------------------------------
// This function deletes the arrays of the instantiated widgets and
// closes the case feature menu window.
//-----------------------------------------------------------------------
void MissionExpert::closeFeatureMenu_(void)
{
    while ((int)(featureMenuWidgets_.size()) > 0)
    {
        //XtDestroyWidget(featureMenuWidgets_.back());
        XtUnmanageChild(featureMenuWidgets_.back());
        featureMenuWidgets_.pop_back();
        XFlush(display_);
    }

    //XtDestroyWidget(featureMenu_w_);
    XtUnmanageChild(featureMenu_w_);
    XFlush(display_);

    featureMenuIsUp_ = false;
}

//-----------------------------------------------------------------------
// This function sends the entries specified in the metadata entry
// window to the cbrserver. 
//-----------------------------------------------------------------------
void MissionExpert::saveMetadataEntries_(void) 
{
    MExpMetadata_t metadata;
    string comment;
    int i;

    if (metadataEntryWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionExpert::saveMetadataEntries_(). metadataEntryWindowData_ is NULL.\n");
        return;
    }

    // Copy the name.
    metadataEntryWindowData_->userName = metadataEntryWindowData_->nameTextWindowString;

    // Generate the metadata.
    time((time_t *)&(metadata.creationTimeSec));
    metadata.numUsage = -1;
    metadata.relevance = -1;

    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        metadata.ratings[i] = (double)(metadataEntryWindowData_->ratings[i]);
    }

    if ((metadataEntryWindowData_->commentTextWindowString) != EMPTY_STRING_)
    {
        comment = metadataEntryWindowData_->commentTextWindowString;

        if (metadataEntryWindowData_->userName != EMPTY_STRING_)
        {
            comment += " | ";
            comment += metadataEntryWindowData_->userName;
        }

        // Repace the special text in the comment ('"', " ", etc.)
        for (i = 0; i < NUM_MEXP_METADATA_SPECIAL_TEXTS; i++)
        {
            comment = replaceStringInString(
                comment,
                MEXP_METADATA_SPECIAL_TEXT_CONVERSION[i].specialText,
                MEXP_METADATA_SPECIAL_TEXT_CONVERSION[i].convertedText);
        }

        // Add quotations.
        comment = "\"" + comment + "\"";

        // Save the comment.
        metadata.userComments.push_back(comment);
    }

    // Send it.
    cfgeditCBRClient_->saveMetadata(
        metadataEntryWindowData_->dataIndexList,
        metadata);
}

//-----------------------------------------------------------------------
// This function deltes the data files which are created.
// Uncommenting "MExpDataFilesDir" in .cfgeditrc will prevent from
// the files being deleted.
//-----------------------------------------------------------------------
void MissionExpert::deleteCreatedDataFiles_(void)
{
    int i;

    for (i = 0; i < (int)(createdDataFileList_.size()); i++)
    {
        unlink(createdDataFileList_[i].c_str());
    }
}

//-----------------------------------------------------------------------
// This function adds the data file name into the list.
//-----------------------------------------------------------------------
void MissionExpert::addCreatedDataFileList_(string filename)
{
    createdDataFileList_.push_back(filename);
}

//-----------------------------------------------------------------------
// This function returns the name of the overlay file being specified
// earlier.
//-----------------------------------------------------------------------
string MissionExpert::getOverlayFileName(void)
{
    if (disableMExp_)
    {
        return EMPTY_STRING_;
    }

    return overlayFileName_;
}

//-----------------------------------------------------------------------
// This function returns the name of the feature file being specified
// earlier.
//-----------------------------------------------------------------------
string MissionExpert::getFeatureFileString(void)
{
    if (disableMExp_)
    {
        return EMPTY_STRING_;
    }

    return featureFileString_;
}

//-----------------------------------------------------------------------
// This function saves the name of the specifed overlay file.
//-----------------------------------------------------------------------
void MissionExpert::saveOverlayFileName(string overlayFileName)
{
    if (disableMExp_)
    {
        return;
    }

    if (overlayFileName != EMPTY_STRING_)
    {
        overlayFileName_ = overlayFileName;
    }
}

//-----------------------------------------------------------------------
// This function saves the mission time string.
//-----------------------------------------------------------------------
void MissionExpert::saveMissionTimeString(string missionTimeString)
{
    if (disableMExp_)
    {
        return;
    }

    if (missionTimeString != EMPTY_STRING_)
    {
        missionTimeString_ = missionTimeString;
    }
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending due to the
// multithreading.
//-----------------------------------------------------------------------
void MissionExpert::startProcessThreadedXCommands_(void)
{
    XtAppAddTimeOut(
        appContext_,
        PROCESS_THREADED_X_COMMAND_TIMER_MSEC_,
        (XtTimerCallbackProc)cbProcessThreadedXCommands_,
        this);
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending. X doesn't
// like to be called asynchronously. This function serves as a
// workaround to execute the X commands in multithreading.
//-----------------------------------------------------------------------
void MissionExpert::processThreadedXCommands_(void)
{
    PendingThreadedXCommands_t cmd;
    MExpStateInfo_t highlightedStateInfo;
    vector<MExpStateInfo_t> highlightedStateInfoList;
    int i, numCommands;
    bool highlighted = false;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);
    XtAppLock(appContext_);
    XLockDisplay(display_);

    /*
    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
    */

    numCommands = shPendingThreadedXCommands_.size();

    for (i = 0; i < numCommands; i++)
    {
        cmd = shPendingThreadedXCommands_[i];

        switch (cmd.type) {

        case X_COMMAND_UPDATE_STATUS_WINDOW:
            updateStatusWindow_(
                shXCommandUpdateStatesWindowList_[cmd.index].rating,
                shXCommandUpdateStatesWindowList_[cmd.index].missionSummary,
                shXCommandUpdateStatesWindowList_[cmd.index].prevBtnIsSensitive,
                shXCommandUpdateStatesWindowList_[cmd.index].nextBtnIsSensitive,
                shXCommandUpdateStatesWindowList_[cmd.index].loadBtnIsSensitive,
                shXCommandUpdateStatesWindowList_[cmd.index].cancelBtnIsSensitive,
                shXCommandUpdateStatesWindowList_[cmd.index].currentMissionId);
            break;

        case X_COMMAND_HIGHLIGHT_STATES:
            highlighted = false;
            highlightedStateInfo = highlightStates_(
                shXCommandHighlightStatesList_[cmd.index].stateInfoList,
                lastHighlightedStateInfo_,
                &highlighted);

            if (highlighted && (cbrclient_ != NULL))
            {
                highlightedStateInfoList.push_back(highlightedStateInfo);
                lastHighlightedStateInfo_ = highlightedStateInfo;
            }
            break;
        }
    }

    shPendingThreadedXCommands_.clear();
    shXCommandUpdateStatesWindowList_.clear();
    shXCommandHighlightStatesList_.clear();

    /*
    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }
    */

    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
	pthread_cleanup_pop(1);

    for (i = 0; i < (int)(highlightedStateInfoList.size()); i++)
    {
        cfgeditCBRClient_->saveHighlightedStateInfo(
            highlightedStateInfoList[i]);
    }
}

//-----------------------------------------------------------------------
// This function checks to see if the forked process is finished or not.
//-----------------------------------------------------------------------
bool MissionExpert::checkForkedProcessStatus_(int pid)
{
    int status, options, rtn;
    bool done = false;

    options = WNOHANG | WUNTRACED;
    rtn = waitpid(pid, &status, options);
      
    if ((rtn == pid) || (rtn == -1))
    {
        // process finished or died with signal
        done = true;
    }

    return done;
}

//-----------------------------------------------------------------------
// This function checks to see if the forked mlab is finished or not.
//-----------------------------------------------------------------------
void MissionExpert::startCheckForkedMlabStatus_(int pid, bool eventLogIsON)
{
    CheckForkedMlabStatusData_t *data;

    data = new CheckForkedMlabStatusData_t;
    data->missionExpertInstance = this;
    data->pid = pid;
    data->eventLogIsON = eventLogIsON;

    XtAppAddTimeOut(
        appContext_,
        EVENT_WAITER_SLEEP_TIME_USEC_,
        (XtTimerCallbackProc)cbCheckForkedMlabStatus_,
        data);
}

//-----------------------------------------------------------------------
// This function writes to the instruction window of the MissionSecWizard.
//-----------------------------------------------------------------------
void MissionExpert::printfMissionSpecWizardInstructionWindow(const char *format, ...)
{
    va_list args;
    char *buf = NULL;

    if (missionSpecWizard_ == NULL)
    {
        fprintf(stderr, "Error: MissionExpert::printfMissionSpecWizardInstructionWindow(). MissionSpecWizard not enabled.\n");
        return;
    }

    buf = new char[MissionSpecWizard::MAX_INSTRUCTION_WINDOW_BUFSIZE];    

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    missionSpecWizard_->printInstructionWindow(buf);

    delete [] buf;
    buf = NULL;
}

//-----------------------------------------------------------------------
// This function writes to the data window of the MissionSpecWizard.
//-----------------------------------------------------------------------
void MissionExpert::printfMissionSpecWizardDataWindow(const char *format, ...)
{
    va_list args;
    char *buf = NULL;

    if (missionSpecWizard_ == NULL)
    {
        fprintf(stderr, "Error: MissionExpert::printfMissionSpecWizardDataWindow(). MissionSpecWizard not enabled.\n");
        return;
    }

    buf = new char[MissionSpecWizard::MAX_DATA_WINDOW_BUFSIZE];    

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    missionSpecWizard_->printDataWindow(buf);

    delete [] buf;
    buf = NULL;
}

//-----------------------------------------------------------------------
// This function updates the progress bar.
//-----------------------------------------------------------------------
void MissionExpert::updateMissionSpecWizardProgressBar(double level)
{
    if (missionSpecWizard_ == NULL)
    {
        fprintf(stderr, "Error: MissionExpert::updateMissionSpecWizardMeter(). MissionSpecWizard not enabled.\n");
        return;
    }

    missionSpecWizard_->updateProgressBar(level);
}

//-----------------------------------------------------------------------
// This function clears the progress bar.
//-----------------------------------------------------------------------
void MissionExpert::clearMissionSpecWizardProgressBar(void)
{
    if (missionSpecWizard_ == NULL)
    {
        fprintf(stderr, "Error: MissionExpert::clearMissionSpecWizardProgressBar(). MissionSpecWizard not enabled.\n");
        return;
    }

    missionSpecWizard_->clearProgressBar();
}

//-----------------------------------------------------------------------
// This function process the X commands that are pending due to the
// multithreading.
//-----------------------------------------------------------------------
XtTimerCallbackProc MissionExpert::cbProcessThreadedXCommands_(XtPointer clientData)
{
    MissionExpert *missionExpertInstance = NULL;

    missionExpertInstance = (MissionExpert *)clientData;

    missionExpertInstance->processThreadedXCommands_();
    missionExpertInstance->startProcessThreadedXCommands_();

    return false;
}

//-----------------------------------------------------------------------
// This function checks to see if the forked process is finished or not.
//-----------------------------------------------------------------------
XtTimerCallbackProc MissionExpert::cbCheckForkedMlabStatus_(XtPointer clientData)
{
    CheckForkedMlabStatusData_t *data = NULL;
    MissionExpert *missionExpertInstance = NULL;
    int pid;
    bool eventLogIsON;

    data = (CheckForkedMlabStatusData_t *)clientData;
    missionExpertInstance = data->missionExpertInstance;
    pid = data->pid;
    eventLogIsON = data->eventLogIsON;

    delete data;
    data = NULL;

    if (missionExpertInstance->checkForkedProcessStatus_(pid))
    {
        if (eventLogIsON)
        {
            gEventLogging->resume(false);
            gEventLogging->end("MExp fork mlab");
        }

        missionExpertInstance->resumeMissionExpert_();

        //gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE_NO);
    }
    else
    {
        missionExpertInstance->startCheckForkedMlabStatus_(pid, eventLogIsON);
    }

    return false;
}

//-----------------------------------------------------------------------
// This callback function gets called when a toggle button for the option
// of case feature menu was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbFeatureMenuValue_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = ((FeatureMenuCallbackData_t *)clientData)->missionExpertInstance;
    missionExpertInstance->changeFeatureMenuValue_(clientData);
}

//-----------------------------------------------------------------------
// This callback function gets called when a toggle button for the option
// of global feature menu was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbChangeGlobalFeatureMenuValue_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = ((FeatureMenuCallbackData_t *)clientData)->missionExpertInstance;
    missionExpertInstance->changeGlobalFeatureMenuValue_(clientData);
}

//-----------------------------------------------------------------------
// This callback function gets called when a task button for the case
// feature menu was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbFeatureMenuTaskValue_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;
    int taskId;

    missionExpertInstance = ((FeatureMenuTaskCallbackData_t *)clientData)->missionExpertInstance;
    taskId = ((FeatureMenuTaskCallbackData_t *)clientData)->taskId;

    missionExpertInstance->changeFeatureMenuTaskValue_(taskId);
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Save" button of the
// case feature menu window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbSaveCase_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->saveCase_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Cancel" button of the
// case feature menu window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbCancelSaveCase_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->closeFeatureMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Previous" button of the
// status window window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbPrevBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->changeStatusWindow_(STATUS_WINDOW_PREV);
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Next" button of the
// status window window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbNextBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->changeStatusWindow_(STATUS_WINDOW_NEXT);
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Load" button of the
// status window window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbLoadBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->selectMissionForStatusWindow_();
    missionExpertInstance->closeStatusWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Cancel" button of the
// status window window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbCancelBtnStatusWindow_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->closeStatusWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the rating button of the
// metadata entry window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbChangeMetadataRatingBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->updateMetadataRatingBtn_(w);
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata entry window's
// user comment is typed in.
//-----------------------------------------------------------------------
void MissionExpert::cbMetadataCommentTextWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->updateMetadataCommentTextWindowString_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the save-cbr-library window's
// filename is typed in.
//-----------------------------------------------------------------------
void MissionExpert::cbSaveCBRLibraryFileWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->updateSaveCBRLibraryFileWindowString_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata entry window's
// user name is typed in.
//-----------------------------------------------------------------------
void MissionExpert::cbMetadataNameTextWindowStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->updateMetadataNameTextWindowString_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Save" button of the
// metadata entry window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbSaveMetadataEntries_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->saveMetadataEntries_();
    missionExpertInstance->closeMetadataEntryWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Save" button of the
// save-cbr-library window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbSaveCBRLibraryFile_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->sendCBRLibraryFile_();
    missionExpertInstance->closeSaveCBRLibraryWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Canecel" button of the
// save-cbr-library window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbCancelSaveCBRLibraryFile_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->closeSaveCBRLibraryWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Cancel" button of the
// metadata entry window was pressed.
//-----------------------------------------------------------------------
void MissionExpert::cbCancelMetadataEntries_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionExpert *missionExpertInstance;

    missionExpertInstance = (MissionExpert *)clientData;
    missionExpertInstance->closeMetadataEntryWindow_();
}

//-----------------------------------------------------------------------
// This function gathers all the names (and descriptions) of states and
// triggers.
//-----------------------------------------------------------------------
config_fsa_lists_t MissionExpert::getFSALists_(void)
{
    config_fsa_lists_t fsaLists;

    fsaLists.taskList = config->get_task_list();
    fsaLists.triggerList = config->get_trigger_list();

    return fsaLists;
}

//-----------------------------------------------------------------------
// This function creates pixmaps to be used, and put them in a list.
//-----------------------------------------------------------------------
void MissionExpert::createPixmaps_(Pixel fg, Pixel bg, Widget parent)
{
    Pixel yellowPixel, grayPixel;
    Colormap colorMap;
    XColor color, ignore;

    colorMap = DefaultColormap(display_, DefaultScreen(display_));

    // Create star.

    yellowPixel = fg;

    if (XAllocNamedColor(display_, colorMap, "yellow", &color, &ignore))
    {
        yellowPixel = color.pixel;
    }

    starPixmap_ = MEXP_CREATE_PIXMAP(star, yellowPixel, bg, parent);

    // Create empty star.

    grayPixel = fg;

    if (XAllocNamedColor(display_, colorMap, "gray40", &color, &ignore))
    {
        grayPixel = color.pixel;
    }

    emptyStarPixmap_ = MEXP_CREATE_PIXMAP(empty_star, grayPixel, bg, parent);
}

//-----------------------------------------------------------------------
// This function is called when the mission is modified by the user.
//-----------------------------------------------------------------------
void MissionExpert::madeChange(void)
{
    if (disableMExp_)
    {
        return;
    }

    if (missionSavedToCBRLibrary_ || missionLoadedFromCBRLibrary_)
    {
        if (INCREMENTALLY_UPDATE_LIBRARY_)
        {
            shouldSaveMissionToCBRLibrary_ = true;
        }
        else
        {
            //clearFeatureFile_();
        }
        currentCBRLibraryDataIndexList_.clear();
        missionSavedToCBRLibrary_ = false;
        missionLoadedFromCBRLibrary_ = false;
    }


    if (missionSpecWizardEnabled() && (missionSpecWizard_ != NULL))
    {
        missionSpecWizard_->notifyMissionModified();
    }
}

//-----------------------------------------------------------------------
// This function checks whether the user-feedback should be inquired.
//-----------------------------------------------------------------------
bool MissionExpert::shouldAskFeedback(void)
{
    if ((disableMExp_)||(!cbrClientInstantiated_)||(disableRepair_))
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------
// This function checks to see if the mission (ACDL) contains a specified
// agent.
//-----------------------------------------------------------------------
bool MissionExpert::hasAgentInMissionPlan_(string missionPlanFileName, string agentName)
{
    FILE *tmpACDLFile = NULL;
    char buf1[1024], buf2[1024];
    string bufString1, bufString2;
    bool found = false;
    
    // Now, open up the file, again, for reading.
    tmpACDLFile = fopen(missionPlanFileName.c_str(), "r");

    if (tmpACDLFile == NULL)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionExpert::hasAgentInMissionPlan_(). File [%s] could not be opened.\n",
            missionPlanFileName.c_str());
        return false;
    }

    // Process the data.
    while (fscanf(tmpACDLFile, "%s", buf1) != EOF)
    {
        bufString1 = buf1;

        if (bufString1 == ACDL_STRING_AGENT_NAME_)
        {
            if (fscanf(tmpACDLFile, "%s", buf2) != EOF)
            {
                bufString2 = buf2;

                if (bufString2 == agentName)
                {
                    found = true;
                    break;
                }
            }
        }
    }

    fclose(tmpACDLFile);

    return found;
}

//-----------------------------------------------------------------------
// This function highlights specified states. It returns the info of the
// state being highlighted.
//-----------------------------------------------------------------------
MExpStateInfo_t MissionExpert::highlightStates_(
    vector<MExpStateInfo_t> stateInfoList,
    MExpStateInfo_t lastHighlightedStateInfo,
    bool *highlighted)
{
    int i, numStateInfo;

    *highlighted = false;
    numStateInfo = stateInfoList.size();

    if (numStateInfo == 0)
    {
        // Nothing to highlight.
        config->clearHighlightStates();
        lastHighlightedStateInfo = EMPTY_STATE_INFO_;
    }
    else if (numStateInfo == 1)
    {
        // Only one state in the list. Highlight this one.
        config->highlightState(
            stateInfoList[0].fsaName,
            stateInfoList[0].stateName);
        lastHighlightedStateInfo = stateInfoList[0];
        *highlighted = true;
    }
    else
    {
        // Check to see if any of the states in the list have been
        // highlighted last time.
        for (i = 1; i < numStateInfo; i++)
        {
            if (lastHighlightedStateInfo.fsaName == stateInfoList[i].fsaName)
            {
                config->highlightState(
                    stateInfoList[i].fsaName,
                    stateInfoList[i].stateName);
                lastHighlightedStateInfo = stateInfoList[i];
                *highlighted = true;
                break;
            }
        }

        // Highlight the last one in the list.
        if (!highlighted)
        {
            config->highlightState(
                stateInfoList[numStateInfo-1].fsaName,
                stateInfoList[numStateInfo-1].stateName);
            lastHighlightedStateInfo = stateInfoList[numStateInfo-1];
            *highlighted = true;
        }
    }

    return lastHighlightedStateInfo;
}

//-----------------------------------------------------------------------
// This function highlights specified states.
//-----------------------------------------------------------------------
void MissionExpert::pendXCommandHighlightStates_(vector<MExpStateInfo_t> stateInfoList)
{
    PendingThreadedXCommands_t cmd;
    XCommandHighlightStates_t data;

    pthread_cleanup_push(
        (void(*)(void*))pthread_mutex_unlock,
        (void *)&threadedXCommandsMutex_);
    pthread_mutex_lock(&threadedXCommandsMutex_);
    XtAppLock(appContext_);
    XLockDisplay(display_);

    cmd.type = X_COMMAND_HIGHLIGHT_STATES;
    cmd.index = shXCommandHighlightStatesList_.size();
    shPendingThreadedXCommands_.push_back(cmd);

    data.stateInfoList = stateInfoList;
    shXCommandHighlightStatesList_.push_back(data);

    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
	pthread_cleanup_pop(1);
}

//-----------------------------------------------------------------------
// This function highlights specified states.
//-----------------------------------------------------------------------
void MissionExpert::highlightStates(vector<MExpStateInfo_t> stateInfoList)
{
    pendXCommandHighlightStates_(stateInfoList);
}

//-----------------------------------------------------------------------
// This function reverts a file from its backup file.
//-----------------------------------------------------------------------
int MissionExpert::revertFile_(string filename)
{
    FILE *backupFile = NULL, *origFile = NULL;
    string backupFilename;
    char cmdBuf[1024];
    int status;

    origFile = fopen(filename.c_str(), "r");
    if (origFile == NULL)
    {
        return REVERT_NO_ORIGINAL_FILE;
    }
    fclose(origFile);
    origFile = NULL;

    backupFilename = filename;
    backupFilename += ".";
    backupFilename += MEXP_EXTENSION_BACKUP;

    backupFile = fopen(backupFilename.c_str(), "r");
    if (backupFile == NULL)
    {
        return REVERT_NO_BACKUP_FILE;
    }

    fclose(backupFile);
    backupFile = NULL;

    sprintf(
        cmdBuf,
        "mv -f %s %s",
        backupFilename.c_str(),
        filename.c_str());

    status = system(cmdBuf);

    /*
    if (status == -1)
    {
        return REVERT_FAILURE;
    }
    */

    return REVERT_SUCCESS;
}

//-----------------------------------------------------------------------
// This function clears the contents of the feature file name.
//-----------------------------------------------------------------------
void MissionExpert::clearFeatureFile_(void)
{
    FILE *featureFile = NULL;

    if (disableMExp_)
    {
        return;
    }

    // Check to see if it exists at first.
    featureFile = fopen(featureFileName_.c_str(), "r");
    if (featureFile == NULL)
    {
        return;
    }
    fclose(featureFile);

    // Clear all the contents.
    featureFile = fopen(featureFileName_.c_str(), "w");
    if (featureFile == NULL)
    {
        return;
    }
    fclose(featureFile);
}

//-----------------------------------------------------------------------
// This function saves the logfile info.
//-----------------------------------------------------------------------
void MissionExpert::saveLogfileInfo(MExpLogfileInfo_t logfileInfo)
{
    if (disableMExp_)
    {
        return;
    }

    logfileInfoList_.push_back(logfileInfo);
}

//-----------------------------------------------------------------------
// This function clears the logfile info.
//-----------------------------------------------------------------------
void MissionExpert::clearLogfileInfoList(void)
{
    if (disableMExp_)
    {
        return;
    }

    logfileInfoList_.clear();
}

//-----------------------------------------------------------------------
// This function returns the list of the logfile info.
//-----------------------------------------------------------------------
vector<MExpLogfileInfo_t> MissionExpert::getLogfileInfoList(void)
{
    const vector<MExpLogfileInfo_t> EMPTY_LOGFILE_INFO_LIST;

    if (disableMExp_)
    {
        return EMPTY_LOGFILE_INFO_LIST;
    }

    return logfileInfoList_;
}

//-----------------------------------------------------------------------
// This function returns the robot id assigned by CNP etc.
//-----------------------------------------------------------------------
int MissionExpert::assignedRobotID(int index)
{
    int robotID = -1;

    if ((int)(currentRobotIDs_.robotIDs.size()) > index)
    {
        robotID = currentRobotIDs_.robotIDs[index];
    }

    return robotID;
}

//-----------------------------------------------------------------------
// This function pops up the MissionSpecWizard.
//-----------------------------------------------------------------------
void MissionExpert::popupMissionSpecWizard_(int wizardType)
{

    if (!setMissionSpecWizardType(wizardType))
    {
        fprintf(
            stderr,
            "Error: MissionExpert::popupMissionSpecWizard_(): Invalid wizard type: %d.\n",
            wizardType);
        return;
    }

    if (missionSpecWizard_ == NULL)
    {
        runMissionExpert(MEXP_RUNMODE_NEW);
    }
    else if (missionSpecWizard_->wizardType() != wizardType)
    {
        delete missionSpecWizard_;
        missionSpecWizard_ = NULL;
        runMissionExpert(MEXP_RUNMODE_NEW);
    }
    else
    {
        missionSpecWizard_->popupDialog();
    }
}

//-----------------------------------------------------------------------
// This function pops up the MissionSpecWizard.
//-----------------------------------------------------------------------
void MissionExpert::cbPopupMissionSpecWizard(Widget w, XtPointer clientData, XtPointer callbackData)
{
    if ((gMExp == NULL) || (gMExp->isDisabled()))
    {
        warn_userf("Mission Expert is not enabled. Enable Mission Expert\nafter restarting CfgEdit.");
        return;
    }

    gMExp->popupMissionSpecWizard_(clientData);
}

//-----------------------------------------------------------------------
// This function checks to see if the MissionSpecWizard dialog is up.
//-----------------------------------------------------------------------
bool MissionExpert::missionSpecWizardIsUp(void)
{
    return ((missionSpecWizard_ != NULL) && (missionSpecWizard_->dialogIsUp()));
}

/**********************************************************************
 * $Log: mission_expert.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.25  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.24  2007/09/18 22:36:11  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.23  2007/08/24 22:24:38  endo
 * Program termination process improved.
 *
 * Revision 1.22  2007/08/15 17:30:40  endo
 * A bug fix in saveAbstractedMissionPlanToFile_().
 *
 * Revision 1.21  2007/08/10 15:14:59  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.20  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.19  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.18  2007/08/04 23:52:54  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.17  2007/06/28 14:01:08  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.16  2007/06/28 12:23:11  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.15  2007/06/28 03:54:20  endo
 * For 06/28/2007 demo.
 *
 * Revision 1.14  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 * Revision 1.13  2007/05/15 18:50:00  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.12  2007/03/05 19:35:37  endo
 * The bug on Back button fixed.
 *
 * Revision 1.11  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.10  2007/01/29 15:08:35  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.9  2006/12/05 01:42:43  endo
 * cdl parser now accepts {& a b}.
 *
 * Revision 1.8  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.7  2006/09/26 18:30:27  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.6  2006/09/22 18:43:04  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.5  2006/09/22 17:57:17  endo
 * ICARUS Wizard improved.
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
 * Revision 1.16  2006/03/01 09:27:22  endo
 * Check-in for Type-I Intercept Experiment.
 *
 * Revision 1.15  2006/02/19 17:52:51  endo
 * Experiment related modifications
 *
 * Revision 1.14  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.13  2006/01/30 02:47:28  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.12  2005/10/21 00:30:16  endo
 * Event Log for GOMS Test.
 *
 * Revision 1.11  2005/09/22 21:42:53  endo
 * fixed a bug in loadFeaturesFromRCFile.
 *
 * Revision 1.10  2005/08/12 22:42:58  endo
 * More improvements for August demo.
 *
 * Revision 1.9  2005/08/12 21:49:58  endo
 * More improvements for August demo.
 *
 * Revision 1.8  2005/08/09 19:12:44  endo
 * Things improved for the August demo.
 *
 * Revision 1.7  2005/07/31 04:27:47  endo
 * *** empty log message ***
 *
 * Revision 1.6  2005/07/31 03:39:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.5  2005/07/27 20:36:39  endo
 * 3D visualization improved.
 *
 * Revision 1.4  2005/06/23 22:07:42  endo
 * Summary Window improved.
 *
 * Revision 1.3  2005/05/18 21:14:43  endo
 * AuRA.naval added.
 *
 * Revision 1.2  2005/02/07 22:25:26  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:35  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 08:50:37  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.2  2002/01/31 10:44:20  endo
 * Parameters chaged due to the change of max_vel and base_vel.
 *
 * Revision 1.1  2002/01/12 23:09:58  endo
 * Initial revision
 *
 **********************************************************************/
