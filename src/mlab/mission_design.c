/**********************************************************************
 **                                                                  **
 **                          mission_design.c                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_design.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <limits>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

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

#include "mission_design.h"
#include "mission_expert.h"
#include "gt_console_windows.h"
#include "gt_console_db.h"
#include "gt_sim.h"
#include "gt_world.h"
#include "gt_scale.h"
#include "draw.h"
#include "console.h"
#include "file_utils.h"
#include "version.h"
#include "EventLogging.h"
#include "assistantDialog.h"
#include "mlab_cbrclient.h"
#include "cbrplanner_protocol.h"
#include "gt_playback.h"
#include "mission_design_types.h"
#include "string_utils.h"
#include "CNPConstraint.h"
#include "cnp_types.h"
#include "load_rc.h"
#include "convert_scale.h"
#include "bitmaps/BiohazardBtn.bit"
#include "bitmaps/CommunicationsBtn.bit"
#include "bitmaps/EOIRBtn.bit"
#include "bitmaps/HostageBtn.bit"
#include "bitmaps/IcarusBtn.bit"
#include "bitmaps/InspectBtn.bit"
#include "bitmaps/InterceptBtn.bit"
#include "bitmaps/LoiterBtn.bit"
#include "bitmaps/MADBtn.bit"
#include "bitmaps/MineBtn.bit"
#include "bitmaps/MineOceanBtn.bit"
#include "bitmaps/NavalReconBtn.bit"
#include "bitmaps/ObserveBtn.bit"
#include "bitmaps/OtherBtn.bit"
#include "bitmaps/SARImageBtn.bit"
#include "bitmaps/SearchAndRescueBtn.bit"
#include "bitmaps/SearchUAVBtn.bit"
#include "bitmaps/SearchUUVBtn.bit"
#include "bitmaps/SentryBtn.bit"
#include "bitmaps/ShoreProtectionBtn.bit"
#include "bitmaps/SteerpointBtn.bit"
#include "bitmaps/TrackBtn.bit"
#include "bitmaps/PolygonCursor.bit"
#include "bitmaps/PolygonCursorMask.bit"
#include "bitmaps/WaypointsBtn.bit"
#include "bitmaps/WeaponReleaseBtn.bit"

using std::numeric_limits;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define MMD_CREATE_PIXMAP(name,fg,bg,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, fg, bg, \
        DefaultDepthOfScreen(XtScreen(parent)))

#define MMD_CURSOR_PIXMAP(name,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, 1, 0, 1)

#define MMD_CREATE_CURSOR(name,mask,fg,bg,parent) XCreatePixmapCursor(XtDisplay(parent), \
        MMD_CURSOR_PIXMAP(name,parent), MMD_CURSOR_PIXMAP(mask,parent), \
        &(gXColors.black), &(gXColors.white), \
		name##_x_hot, name##_y_hot)

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
const int MlabMissionDesign::DEFAULT_PLACE_CIRCLE_DIAMETER_ = 15;
const int MlabMissionDesign::DEFAULT_PLACE_POLYGON_POINT_DIAMETER_ = 15;
const int MlabMissionDesign::DEFAULT_TOOLBOX_WIDTH_ = 165;
const int MlabMissionDesign::DEFAULT_TOOLBOX_XPOS_ = 50;
const int MlabMissionDesign::DEFAULT_TOOLBOX_YPOS_ = 150;
const int MlabMissionDesign::DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_ = 25;
const int MlabMissionDesign::DEFAULT_TOOLBOX_EXTRASPACE_HEIGHT_ = 40;
const int MlabMissionDesign::DEFAULT_TOOLBOX_BUTTON_HEIGHT_ = 70;
const int MlabMissionDesign::DEFAULT_TOOLBOX_BUTTON_WIDTH_ = 70;
const int MlabMissionDesign::DEFAULT_TOOLBOX_BUTTON_EXTRASPACE_HEIGHT_ = 5;
const int MlabMissionDesign::DEFAULT_TOOLBOX_NUM_COLUMNS_ = 2;
const int MlabMissionDesign::DEFAULT_REPAIRWINDOW_ACTIONAREA_HEIGHT_ = 70;
const int MlabMissionDesign::DEFAULT_REPAIRWINDOW_EXTRASPACE_HEIGHT_ = 5;
const int MlabMissionDesign::DEFAULT_REPAIRWINDOW_WIDTH_ = 165;
const int MlabMissionDesign::DEFAULT_REPAIRWINDOW_XPOS_ = 30;
const int MlabMissionDesign::DEFAULT_REPAIRWINDOW_YPOS_ = 250;
const int MlabMissionDesign::DEFAULT_RIGHTBTN_WIDTH_ = 400;
const int MlabMissionDesign::DEFAULT_RIGHTBTN_LINEHEIGHT_ = 32;
const int MlabMissionDesign::DEFAULT_RIGHTBTN_EDGEOFFSET_ = 20;
const int MlabMissionDesign::DEFAULT_RIGHTBTN_EXTRASPACE_HEIGHT_ = 0;
const int MlabMissionDesign::DEFAULT_ROBOTCONSTRAINT_WIDTH_ = 400;
const int MlabMissionDesign::DEFAULT_ROBOTCONSTRAINT_LINEHEIGHT_ = 32;
const int MlabMissionDesign::DEFAULT_ROBOTCONSTRAINT_EDGEOFFSET_ = 20;
const int MlabMissionDesign::DEFAULT_GLOBALFEATURE_WIDTH_ = 350;
const int MlabMissionDesign::DEFAULT_GLOBALFEATURE_LINEHEIGHT_ = 32;
const int MlabMissionDesign::DEFAULT_GLOBALFEATURE_XEDGEOFFSET_ = 50;
const int MlabMissionDesign::DEFAULT_GLOBALFEATURE_YPOS_ = 100;
const int MlabMissionDesign::DRAG_UPDATE_TIME_MSEC_ = 10;
const int MlabMissionDesign::MAX_CONSTRAINTS_DISPLAY_ = 5;
const int MlabMissionDesign::MAX_LOCAL_FEATURES_DISPLAY_ = 5;
const int MlabMissionDesign::TOOLBOX_EXTRASPACE_HEIGHT_RUNTIME_CNP_ = 60;
const int MlabMissionDesign::TOOLBOX_EXTRASPACE_HEIGHT_ICARUS_WIZARD_ = 60;
const int MlabMissionDesign::INVALID_TASK_ID_ = -1;
const int MlabMissionDesign::INVALID_TASK_NUMBER_ = -1;
const int MlabMissionDesign::INVALID_ROBOT_ID_ = -1;
const int MlabMissionDesign::INVALID_MOBILITY_TYPE_ = -1;
const string MlabMissionDesign::STRING_WAYPOINTS_TASK_ = "WaypointsTask";
const string MlabMissionDesign::STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_ = "RuntimeCNPMissionManagerTask";
const string MlabMissionDesign::STRING_RUNTIME_CNP_BIDDER_TASK_ = "RuntimeCNPBidderTask";
const string MlabMissionDesign::STRING_ICARUS_TASK_ = "IcarusTask";
const string MlabMissionDesign::STRING_VEHICLE_TYPES_ = "VEHICLE_TYPES";
const string MlabMissionDesign::STRING_ROBOT_ID_ = "ROBOT_ID";
const string MlabMissionDesign::STRING_ADD_ = "Add";
const string MlabMissionDesign::STRING_ANY_ = "Any";
const string MlabMissionDesign::STRING_DONE_ = "Done";
const string MlabMissionDesign::STRING_EDIT_ = "Edit";
const string MlabMissionDesign::TOOLBOX_OK_BUTTON_LABEL_ = "Finish";
const string MlabMissionDesign::TOOLBOX_CANCEL_BUTTON_LABEL_ = "Cancel & Return";
const string MlabMissionDesign::TOOLBOX_GLOBALSETTING_BUTTON_LABEL_ = "Global Settings";
const string MlabMissionDesign::TOOLBOX_ROBOTSETTING_BUTTON_LABEL_ = "Robot Settings";
const string MlabMissionDesign::TOOLBOX_MISSIONSETTING_BUTTON_LABEL_ = "Mission Settings";
const string MlabMissionDesign::REPAIRWINDOW_OK_BUTTON_LABEL_ = "Fix It!";
const string MlabMissionDesign::REPAIRWINDOW_CANCEL_BUTTON_LABEL_ = "Cancel & Return";
const string MlabMissionDesign::GLOBALFEATURE_TITLE_ = "Mission Preference  [Global Settings]";
const string MlabMissionDesign::CONFIRM_EMPTY_MISSION_PHRASE_= "This mission is empty.\n\nCancel this session and return to CfgEdit?";
const string MlabMissionDesign::CONFIRM_NO_ROBOT_CONSTRAINT_PERMISSION_PHRASE_ = "You do not have a permission to alter robot\nconstraints. Please check the RC file.";
const string MlabMissionDesign::CONFIRM_NO_RC_PHRASE_ = "Error: RC file could not be loaded. Returning to CfgEdit.";
const string MlabMissionDesign::CONFIRM_OFFENDING_PART_PHRASE_ = "Do you want to fix the part of the mission\nwhere the robot is now?";
const string MlabMissionDesign::CONFIRM_PROCEED_PHRASE_ = "OK to proceed?";
const string MlabMissionDesign::CONFIRM_DELETE_PHRASE_ = "OK to delete?";
const string MlabMissionDesign::CONFIRM_DEFAULT_INIT_INSTRUCTION_PHRASE_ = "Please choose a task from Toolbox and\nplace it on the map.";
const string MlabMissionDesign::CONFIRM_RUNTIME_CNP_INIT_INSTRUCTION_PHRASE_ = "Instruction:\n\n1) Adjust the mission setting for the CNP Mission Manager\n   that will be placed on StartPlace.\n\n2) Click a point in the map where you want a bidder robot\n   to be deployed.\n";
const string MlabMissionDesign::CONFIRM_RUNTIME_CNP_NO_DEPLOYABLE_ROBOT_PHRASE_ = "No more robot is available for deployment.";
const string MlabMissionDesign::CONFIRM_RUNTIME_CNP_NO_REMOVE_MISSION_MANAGER_ = "CNP Mission Manager cannot be deleted.";
const string MlabMissionDesign::CONFIRM_ENABLE_ICARUS_WIZARD_INIT_INSTRUCTION_PHRASE_ = "Instruction:\n\n1) Adjust the mission setting for the ICARUS task that will\n   be placed on StartPlace.\n\n2) Click \"Finish\" button to retrieve the relevant missions.\n";
const string MlabMissionDesign::CONFIRM_ENABLE_ICARUS_WIZARD_NO_REMOVE_TASK_ = "IcarusTask cannot be deleted.";
const string MlabMissionDesign::EXCEEDED_TASK_USAGE_PHRASE_ = "%s can be placed only %d time%s.\nDelete or move the existing one.";
const string MlabMissionDesign::ROBOTCONSTRAINT_TITLE_LABEL_ = "Robot Setting";
const string MlabMissionDesign::ROBOTCONSTRAINT_TITLE_LABEL_RUNTIME_CNP_ = "Select a robot to deploy";
const string MlabMissionDesign::ROBOTCONSTRAINT_ROBOT_NAVIGATION_LABEL_ = "Browse Robots       ";
const string MlabMissionDesign::ROBOTCONSTRAINT_CONSTRAINT_NAVIGATION_LABEL_ = "Browse Settings     ";
const string MlabMissionDesign::ROBOTCONSTRAINT_PREV_BUTTON_LABEL_ = "<< Previous   ";
const string MlabMissionDesign::ROBOTCONSTRAINT_NEXT_BUTTON_LABEL_ = "  Next >>";
const string MlabMissionDesign::RIGHTBTN_TITLE_LABEL_ = "Task Preference";
const string MlabMissionDesign::RIGHTBTN_TITLE_LABEL_RUNTIME_CNP_ = "Runtime-CNP Mission Manager";
const string MlabMissionDesign::RIGHTBTN_TITLE_LABEL_ICARUS_TASK_ = "ICARUS Task";
const string MlabMissionDesign::RIGHTBTN_PREFERNCE_NAVIGATION_LABEL_ = "Browse Settings     ";
const string MlabMissionDesign::RIGHTBTN_PREV_BUTTON_LABEL_ = "<< Previous   ";
const string MlabMissionDesign::RIGHTBTN_NEXT_BUTTON_LABEL_ = "  Next >>";
const string MlabMissionDesign::RCTABLE_DISALLOW_MOD_CNP_ROBOTS_ = "DisallowModCNPRobots";
const string MlabMissionDesign::RCTABLE_DISABLE_COLOR_BUTTONS_= "DisableMissionExpertColorButtons";
const char MlabMissionDesign::POLYGON_ENCODER_KEY_XY_START_ = '<';
const char MlabMissionDesign::POLYGON_ENCODER_KEY_XY_END_ = '>';
const char MlabMissionDesign::POLYGON_ENCODER_KEY_LATLON_START_ = '[';
const char MlabMissionDesign::POLYGON_ENCODER_KEY_LATLON_END_ = ']';
const char MlabMissionDesign::POLYGON_ENCODER_KEY_POINT_SEPARATOR_ = ',';
const char MlabMissionDesign::POLYGON_ENCODER_KEY_POINTS_SEPARATOR_ = ';';

//-----------------------------------------------------------------------
// Constructor for the MlabMissionDesign class.
//-----------------------------------------------------------------------
MlabMissionDesign::MlabMissionDesign(
    Display *display,
    Widget parent,
    XtAppContext app,
    Widget drawing_area,
    const symbol_table<rc_chain> &rcTable,
    string mexpRCFileName,
    char *featureFileString,
    char *missionTimeString,
    bool enableCBRClient,
    char **defaultOverlayName,
    int cnpMode) :
    CBRWizard(display, parent, app, cnpMode),
    featureStatusFile_(NULL),
    MMDDrawingArea_(drawing_area),
    globalFeatureMenu_w_(NULL),
    robotConstraintMenu_w_(NULL),
    currentEditPolygonButton_(NULL),
    mlabCBRClient_(NULL),
    placeCircleDiameter_(DEFAULT_PLACE_CIRCLE_DIAMETER_),
    placePolygonPointDiameter_(DEFAULT_PLACE_POLYGON_POINT_DIAMETER_),
    lastMouseClickedPointX_(0),
    lastMouseClickedPointY_(0),
    MMDCurrentTask_(INVALID_TASK_ID_),
    runtimeCNPMissionManagerTask_(INVALID_TASK_ID_),
    icarusTask_(INVALID_TASK_ID_),
    numVisibleTaskTypes_(0),
    numTasks_(0),
    placeCircleIndex_(0),
    upRuntimeCNPRobotID_(INVALID_ROBOT_ID_),
    selectedDeployedRuntimeCNPRobotID_(INVALID_ROBOT_ID_),
    windowsCreated_(false),
    rightBtnMenuIsUp_(false),
    robotConstraintMenuIsUp_(false),
    taskLifted_(false),
    viaPointLifted_(false),
    allowModCNPRobots_(false),
    savedTasksLoaded_(false),
    disableColorButtons_(false),
    isSpecifyingPolygon_(false),
    isEditingPolygon_(false)
{
    bool featuresLoaded = false;

    gEventLogging->start("MMD");

    rcTable_ = rcTable;

    if (mexpRCFileName == EMPTY_STRING_)
    {
        mexpRCFileName_ = MEXP_DEFAULT_RC_FILENAME;
    }
    else
    {
        mexpRCFileName_ = mexpRCFileName;
    }

    // Check RC file to see if color buttons should be disabled.
    if (check_bool_rc(rcTable, RCTABLE_DISABLE_COLOR_BUTTONS_.c_str()))
    {
        disableColorButtons_ = true;
    }

    MMDMissionAreaHeightPixels_ = drawing_area_height_pixels;

    featuresLoaded = loadFeaturesFromRCFile_();

    if (!featuresLoaded)
    {
        confirmUser_(CONFIRM_NO_RC_PHRASE_, true);
        quit_mlab();
    }
    
    // Default is RUN_MODE. The mode will be altered upon creating appropriate
    // windows.
    MMDMode_ = RUN_MODE;

    // Set the current task based on the default task ID.
    MMDCurrentTask_ = getDefaultCurrentTask_();

    // Create a list of global preference with default values.

    // Set the color for the toolbox buttons.
    MMDToolboxBtnBgColorSet_ = (disableColorButtons_)? gColorPixel.yellow : gColorPixel.white;
    MMDToolboxBtnBgColorUnset_ = (disableColorButtons_)? parentWidgetBg_ : gColorPixel.orange;

    if (featureFileString != NULL)
    {
        // Name the files to be used.
        featureFileName_ = featureFileString;
        featureFileName_ += ".";
        featureFileName_ += MEXP_EXTENSION_FEATUREFILENAME;

        featureStatusFileName_ = featureFileString;
        featureStatusFileName_ += ".";
        featureStatusFileName_ += MEXP_EXTENSION_FEATURESTATUSFILENAME;

        // Load the data in the feature file if there is any.
        loadData_();
    }

    // Save the date string.
    if (missionTimeString != NULL)
    {
        setGlobalFeatureMissionTime_(missionTimeString);
    }

    // Misc setup.
    //returnConfigureMission_ = MissionExpert::CANCEL;
    returnConfigureMission_ = MissionExpert::CONFIGURE;
    draggingCircle_.drawn = false;
    clearPolygon_(&specifyingPolygon_);

    // Save the overlay file name.
    if (overlayFileName_ != EMPTY_STRING_)
    {
        if (*defaultOverlayName != NULL)
        {
            free(*defaultOverlayName);
        }

        *defaultOverlayName = strdup(overlayFileName_.c_str());
    }

    if (enableCBRClient)
    {
        // Connect to CBRServer.
        initCBRClient_();
    }

    // Check RC file to see if CNP robot constraints can be altered.
    if (!(check_bool_rc(rcTable, RCTABLE_DISALLOW_MOD_CNP_ROBOTS_.c_str())))
    {
        allowModCNPRobots_ = true;
    }
}

//-----------------------------------------------------------------------
// Destructor for the MlabMissionDesign class.
//-----------------------------------------------------------------------
MlabMissionDesign::~MlabMissionDesign(void)
{
    if (featureStatusFileName_ != EMPTY_STRING_)
    {
        setFeatureFileStatus_();
    }

    if (windowsCreated_)
    {
        XtDestroyWidget(toolbox_w_);
        XtDestroyWidget(globalFeatureMenu_w_);
        windowsCreated_ = false;
    }
}

//-----------------------------------------------------------------------
// This function saves the specified features to the feature file.
//-----------------------------------------------------------------------
void MlabMissionDesign::saveData_(void)
{
    MExpFeature_t globalFeature;
    MExpMissionTask_t task;
    MExpFeature_t localFeature;
    MExpFeature_t constraint;
    MExpFeatureOption_t option;
    MExpTaskConstraints_t taskConstraints;
    MExpRobotConstraints_t robotConstraints;
    vector<MExpTaskConstraints_t> taskConstraintsList;
    vector<MExpRobotConstraints_t> robotConstraintsList;
    FILE *featureFile = NULL;
    int i, j, k;
    int numConstraints, numOptions;
    int status;
    bool skip = true;

    // If the file exists, copy it as a backup file.
    status = backupFile_(featureFileName_);
    if (status == BACKUP_FAILURE)
    {
        fprintf(stderr, "Warning (mlab): MlabMissionDesign::saveData_().\n");
        fprintf(
            stderr,
            "The feature status file %s could not be backed up.\n",
            featureFileName_.c_str());
    }
    
    featureFile = fopen(featureFileName_.c_str(), "w+");

    if (featureFile == NULL)
    {
        fprintf(stderr, "Warning (mlab): MlabMissionDesign::saveData_().\n");
        fprintf(stderr, "The feature file could not be opened.\n");
        returnConfigureMission_ = MissionExpert::CANCEL;
        return;
    }
    
    // Save the overlay file name.
    if (overlayFileName_ != EMPTY_STRING_)
    {
        fprintf(
            featureFile,
            "%s %s\n",
            MEXP_STRING_OVERLAY.c_str(),
            overlayFileName_.c_str());
    }

    // Write the CNP mode
    fprintf(
        featureFile,
        "%s %s\n",
        MEXP_STRING_CNP.c_str(),
        CNP_MODE_STRING[cnpMode_].c_str());
    fprintf(featureFile, "\n");

    for (i = 0; i < ((int)(defaultGlobalFeatures_.size())); i++)
    {
        // Save the global preference.
        globalFeature = defaultGlobalFeatures_[i];

        numOptions = globalFeature.options.size();

        // Special case. Overlay name for MissionSpecWizard.
        if (missionSpecWizardEnabled())
        {
            if (globalFeature.name == STRING_OVERLAY_NAME_)
            {
                for (j = 0; j < numOptions; j++)
                {
                    option = globalFeature.options[j];

                    if (option.value == overlayFileName_)
                    {
                        globalFeature.selectedOption = (float)j;
                        break;
                    }
                }
            }
        }

        fprintf(
            featureFile,
            "%s %d %.2f %d %d \"%s\"\n",
            MEXP_STRING_GLOBALFEATURE.c_str(),
            globalFeature.id,
            globalFeature.selectedOption,
            numOptions,
            globalFeature.optionType,
            globalFeature.name.c_str());

        for (j = 0; j < numOptions; j++)
        {
            option = globalFeature.options[j];
            fprintf(
                featureFile,
                "%s %d \"%s\"\n",
                MEXP_STRING_OPTION.c_str(),
                j,
                option.value.c_str());
        }
    }

    fprintf(featureFile, "\n");

    // Save the data specific to each task.
    for (i = 0; i < numTasks_; i++)
    {
        task = specifiedTaskList_[i];

        switch (cnpMode_) {

        case CNP_MODE_PREMISSION:
            taskConstraints.id = task.id;
            taskConstraints.name = task.name;
            taskConstraints.constraints.clear();
            break;
        }

        // Save the task name.
        fprintf(
            featureFile,
            "%s %d \"%s\"\n",
            MEXP_STRING_TASK.c_str(),
            task.id,
            task.name.c_str());

        // Save the position of the task.
        fprintf(
            featureFile,
            "%s %5.2f %5.2f\n",
            MEXP_STRING_COORDINATE.c_str(),
            task.position.x,
            task.position.y);

        fprintf(
            featureFile,
            "%s %5.6f %5.6f\n",
            MEXP_STRING_GEO_COORDINATE.c_str(),
            task.geo.latitude,
            task.geo.longitude);

        // Save the data specific to each feature.
        for (j = 0; j < ((int)(task.localFeatures.size())); j++)
        {
            localFeature = task.localFeatures[j];

            skip = true;

            switch (cnpMode_) {

            case CNP_MODE_PREMISSION:
                switch (localFeature.type) {

                case MEXP_FEATURE_LOCAL:
                    skip = false;
                    break;

                case MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT:
                    skip = false;
                    taskConstraints.constraints.push_back(localFeature);
                    break;

                case MEXP_FEATURE_TASK_CONSTRAINT:
                    taskConstraints.constraints.push_back(localFeature);
                    break;
                } // switch (localFeature.type)
                break;

            case CNP_MODE_RUNTIME:
                switch (localFeature.type) {

                case MEXP_FEATURE_LOCAL:
                case MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT:
                    skip = false;
                    break;

                case MEXP_FEATURE_RUNTIME_CNP_MISSION_MANAGER:
                    if (task.name == STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_)
                    {
                        skip = false;
                    }
                    break;

                case MEXP_FEATURE_RUNTIME_CNP_BIDDER:
                    if (task.name == STRING_RUNTIME_CNP_BIDDER_TASK_)
                    {
                        skip = false;
                    }
                    break;

                case MEXP_FEATURE_RUNTIME_CNP_BOTH:
                    skip = false;
                    break;
                } // switch (localFeature.type)
                break;

            default:
                switch (localFeature.type) {

                case MEXP_FEATURE_LOCAL:
                case MEXP_FEATURE_LOCAL_AND_TASK_CONSTRAINT:
                    skip = false;
                    break;
                } // switch (localFeature.type)
                break;
            } // switch (cnpMode_)

            if (skip)
            {
                continue;
            }

            numOptions = localFeature.options.size();
            fprintf(
                featureFile,
                "%s %d %.2f %d %d \"%s\"\n",
                MEXP_STRING_FEATURE.c_str(),
                localFeature.id,
                localFeature.selectedOption,
                numOptions,
                localFeature.optionType,
                localFeature.name.c_str());

            for (k = 0; k < numOptions; k++)
            {
                option = localFeature.options[k];
                fprintf(
                    featureFile,
                    "%s %d \"%s\"\n",
                    MEXP_STRING_OPTION.c_str(),
                    k,
                    option.value.c_str());
            }

        }
        fprintf(featureFile, "\n");

        switch (cnpMode_) {

        case CNP_MODE_PREMISSION:
            taskConstraintsList.push_back(taskConstraints);
            break;
        }
    }

    switch (cnpMode_) {

    case CNP_MODE_PREMISSION:
        for (i = 0; i < (int)(taskConstraintsList.size()); i++)
        {
            // Write the task number and the number of constraints.
            numConstraints = taskConstraintsList[i].constraints.size();
            fprintf(
                featureFile,
                "%s %d %d \"%s\"\n",
                MEXP_STRING_TASK_CONSTRAINTS.c_str(),
                taskConstraintsList[i].id,
                numConstraints,
                taskConstraintsList[i].name.c_str());
                

            // Save the data specific to each feature.
            for (j = 0; j < numConstraints; j++)
            {
                constraint = taskConstraintsList[i].constraints[j];

                numOptions = constraint.options.size();
                fprintf(
                    featureFile,
                    "%s %d %.2f %d %d \"%s\"\n",
                    MEXP_STRING_TASK_CONSTRAINT.c_str(),
                    constraint.id,
                    constraint.selectedOption,
                    numOptions,
                    constraint.optionType,
                    constraint.name.c_str());

                for (k = 0; k < numOptions; k++)
                {
                    option = constraint.options[k];
                    fprintf(
                        featureFile,
                        "%s %d \"%s\"\n",
                        MEXP_STRING_OPTION.c_str(),
                        k,
                        option.value.c_str());
                }

            }

            fprintf(featureFile, "\n");
        }

        fprintf(featureFile, "\n");

        // Do not "break" here yet. Add robot constraints, too.

    case CNP_MODE_RUNTIME:
        for (i = 0; i < (int)(robotConstraintsList_.size()); i++)
        {
            robotConstraints = robotConstraintsList_[i];

            // Save the robot ID.
            numConstraints = robotConstraints.constraints.size();
            fprintf(
                featureFile,
                "%s %d %d \"%s\"\n",
                MEXP_STRING_ROBOT_CONSTRAINTS.c_str(),
                robotConstraints.id,
                numConstraints,
                robotConstraints.name.c_str());

            // Save the data specific to each feature.
            for (j = 0; j < numConstraints; j++)
            {
                constraint = robotConstraints.constraints[j];

                numOptions = constraint.options.size(); 
                fprintf(
                    featureFile,
                    "%s %d %.2f %d %d \"%s\"\n",
                    MEXP_STRING_ROBOT_CONSTRAINT.c_str(),
                    constraint.id,
                    constraint.selectedOption,
                    numOptions,
                    constraint.optionType,
                    constraint.name.c_str());

                for (k = 0; k < numOptions; k++)
                {
                    option = constraint.options[k];
                    fprintf(
                        featureFile,
                        "%s %d \"%s\"\n",
                        MEXP_STRING_OPTION.c_str(),
                        k,
                        option.value.c_str());
                }
            }
            fprintf(featureFile, "\n");
        }
        break;
    } // switch (cnpMode_)

    // Save the data specific to each task.
    for (i = 0; i < (int)(viaPointList_.size()); i++)
    {
        fprintf(
            featureFile,
            "%s %5.2f %5.2f\n",
            MEXP_STRING_VIA_POINT.c_str(),
            viaPointList_[i].x,
            viaPointList_[i].y);
        fprintf(featureFile, "\n");
    }

    fclose(featureFile);
}

//-----------------------------------------------------------------------
// This function loads the task data from the feature file.
//-----------------------------------------------------------------------
bool MlabMissionDesign::loadData_(void)
{
    MExpMissionTask_t addedTask;
    MExpPoint2D_t viaPoint;
    MExpConstraints_t genericConstraints;
    MExpFeature_t constraint;
    MExpTaskConstraints_t taskConstraints;
    MExpRobotConstraints_t robotConstraints;
    MExpFeatureOption_t option;
    MMDPolygon_t poly;
    FILE *featureFile;
    vector<MExpTaskConstraints_t> taskConstraintsList;
    vector<MExpRobotConstraints_t> robotConstraintsList;
    string bufString1, bufString2, bufString3;
    string entryKey;
    char buf1[1024], buf2[1024], buf3[1024];
    float x, y, latitude, longitude;
    float globalFeatureSelectedOption, featureSelectedOption;
    int i, j;
    int taskId;
    int globalFeatureId, featureId, status;
    int constraintType, numConstraints;
    int numOptions;
    int optionType;
    int cnpMode = CNP_MODE_DISABLED;
    bool taskCreated = false, hadError = false;
    string errorMsg = EMPTY_STRING_;
    
    savedTasksLoaded_ = false;

    featureFile = fopen(featureFileName_.c_str(), "r");

    if (featureFile == NULL)
    {
        return false;
    }

    gEventLogging->start("MMD Loading the feature data from %s", featureFileName_.c_str());

    while(!feof(featureFile))
    {
        if (fscanf(featureFile, "%s", buf1) == EOF)
        {
            break;
        }

        bufString1 = buf1;

        if (bufString1 == MEXP_STRING_OVERLAY)
        {
            // Save the overlay name specified.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_OVERLAY";
                break;
            }
            overlayFileName_ = buf2;
        }
        else if (bufString1 == MEXP_STRING_CNP)
        {
            // Check the mode of CNP
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                break;
            }
            bufString2 = buf2;

            if (isdigit(bufString2[0]))
            {
                cnpMode = atoi(buf2);
            }
            else
            {
                for (i = 0; i < NUM_CNP_MODES; i++)
                {
                    if (bufString2 == CNP_MODE_STRING[i])
                    {
                        cnpMode = i;
                        break;
                    }
                }
            }

            if (cnpMode != cnpMode_)
            {
                // Inconsistent mode. Reload necessary data based on the new mode.
                fclose(featureFile);
                cnpMode_ = cnpMode;
                loadFeaturesFromRCFile_();
                MMDCurrentTask_ = getDefaultCurrentTask_();
                return (loadData_());
            }
        }
        else if (bufString1 == MEXP_STRING_GLOBALFEATURE)
        {
            // Get the global preference ID.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GLOBALFEATURE ID";
                break;
            }
            globalFeatureId = atoi(buf2);
            
            // Get the selected option for the global preference.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GLOBALFEATURE Selected Option";
                break;
            }
            globalFeatureSelectedOption = atof(buf2);

            defaultGlobalFeatures_[globalFeatureId].selectedOption = globalFeatureSelectedOption;

            // Get the number of options for the global preference.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GLOBALFEATURE numOptions";
                break;
            }
            numOptions = atoi(buf2);

            // Get the option type for the global preference.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GLOBALFEATURE optionType";
                break;
            }
            optionType = atoi(buf2);

            switch (optionType) {

            case MEXP_FEATURE_OPTION_POLYGONS:

                clearPolygon_(&poly);
                poly.featureType = MEXP_FEATURE_GLOBAL;
                poly.featureId = globalFeatureId;

                // Look for the polygon points store in the option value.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_GLOBALFEATURE feature name";
                    break;
                }
                // (Skip the featureName.)

                // Parse though the options
                for (i = 0; i < numOptions; i++)
                {
                    // Get the option string
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        errorMsg = "MEXP_STRING_GLOBALFEATURE Option Name (Read)";
                        break;
                    }
                    bufString2 = buf2;

                    if (bufString2 != MEXP_STRING_OPTION)
                    {
                        hadError = true;
                        errorMsg = "MEXP_STRING_GLOBALFEATURE Option Name (Check)";
                        break;
                    }

                    // Get the option number
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        errorMsg = "MEXP_STRING_GLOBALFEATURE Option Number";
                        break;
                    }

                    poly.number = atoi(buf2);

                    // Get the option value
                    if (fscanf(featureFile, "%s", buf2) == EOF)
                    {
                        hadError = true;
                        errorMsg = "MEXP_STRING_GLOBALFEATURE Option Value";
                        break;
                    }
                    bufString2 = buf2;

                    // Remove '"'
                    option.value = removeCharInString(bufString2, '\"');

                    if ((int)(defaultGlobalFeatures_[globalFeatureId].options.size()) <= i)
                    {
                        defaultGlobalFeatures_[globalFeatureId].options.push_back(option);
                    }
                    else
                    {
                        defaultGlobalFeatures_[globalFeatureId].options[i] = option;
                    }

                    if (option.value != MEXP_STRING_NA)
                    {
                        decodePolygon_(option.value, &poly);
                        specifiedPolygonList_.push_back(poly);
                    }
                }

                break;
            }

        }
        else if (bufString1 == MEXP_STRING_TASK)
        {
            // Get the task ID (e.g., Biohazrd, EOD, etc.)
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_TASK ID";
                break;
            }
            taskId = atoi(buf2);

            // Add a new task at <0, 0> for now.
            status = addNewTask_(0, 0, taskId, &addedTask);
            if (status == PLACETASK_FAILURE)
            {
                hadError = true;
                errorMsg = "PLACETASK_FAILURE";
                break;
            }
            taskCreated = true;
        }
        else if ((bufString1 == MEXP_STRING_COORDINATE) && (taskCreated))
        {
            // Get the X coordinate.
            if (fscanf(featureFile, "%f", &x) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_COORDINATE X";
                break;
            }
            specifiedTaskList_[numTasks_-1].position.x = (double)x;

            // Get the Y coordinate.
            if (fscanf(featureFile, "%f", &y) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_COORDINATE Y";
                break;
            }
            specifiedTaskList_[numTasks_-1].position.y = (double)y;
        }
        else if ((bufString1 == MEXP_STRING_GEO_COORDINATE) && (taskCreated))
        {
            // Get the X coordinate.
            if (fscanf(featureFile, "%f", &latitude) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GEO_COORDINATE LATITUDE";
                break;
            }
            specifiedTaskList_[numTasks_-1].geo.latitude = (double)latitude;

            // Get the Y coordinate.
            if (fscanf(featureFile, "%f", &longitude) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_GEO_COORDINATE LONGITUDE";
                break;
            }
            specifiedTaskList_[numTasks_-1].geo.longitude = (double)longitude;
        }
        else if ((bufString1 == MEXP_STRING_FEATURE) && (taskCreated))
        {
            // Get the feature ID.
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_FEATURE ID";
                break;
            }
            featureId = atoi(buf2);

            // Get the selected option for the feature.
            if (fscanf(featureFile, "%s", buf3) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_FEATURE Selected Option";
                break;
            }
            featureSelectedOption = atof(buf3);

            specifiedTaskList_[numTasks_-1].localFeatures[featureId].selectedOption = featureSelectedOption;
        }
        else if (bufString1 == MEXP_STRING_VIA_POINT)
        {
            // Get the X coordinate.
            if (fscanf(featureFile, "%f", &x) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_VIA_POINT X";
                break;
            }
            viaPoint.x = (double)x;

            // Get the Y coordinate.
            if (fscanf(featureFile, "%f", &y) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_VIA_POINT Y";
                break;
            }
            viaPoint.y = (double)y;

            viaPointList_.push_back(viaPoint);
        }
        else if ((bufString1 == MEXP_STRING_TASK_CONSTRAINTS) ||
                 (bufString1 == MEXP_STRING_ROBOT_CONSTRAINTS))
        {
            if (bufString1 == MEXP_STRING_TASK_CONSTRAINTS)
            {
                constraintType = MEXP_FEATURE_TASK_CONSTRAINT;
            }
            else if (bufString1 == MEXP_STRING_ROBOT_CONSTRAINTS)
            {
                constraintType = MEXP_FEATURE_ROBOT_CONSTRAINT;
            }

            genericConstraints.constraints.clear();

            // Get the task/robot ID
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINTS ID";
                break;
            }
            genericConstraints.id = atoi(buf2);

            // Get the number of constraints
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINTS Number of Constraints";
                break;
            }
            numConstraints = atoi(buf2);

            // Get the name of the task/robot
            if (fscanf(featureFile, "%s", buf2) == EOF)
            {
                hadError = true;
                errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINTS Name";
                break;
            }
            bufString2 = buf2;
            // Remove '"'
            bufString2 = removeCharInString(bufString2, '\"');
            genericConstraints.name = bufString2;

            for (i = 0; i < numConstraints; i++)
            {
                // Make sure it is the constraint first.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT (Read)";
                    break;
                }
                bufString2 = buf2;

                if ((bufString2 != MEXP_STRING_TASK_CONSTRAINT) &&
                    (bufString2 != MEXP_STRING_ROBOT_CONSTRAINT))
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT (Check) [";
                    errorMsg += bufString2;
                    errorMsg += "]";
                    break;
                }

                // Get the constraint ID.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT ID";
                    break;
                }
                constraint.id = atoi(buf2);

                // Get the option being selected.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Selected Option";
                    break;
                }
                constraint.selectedOption = atof(buf2);

                // Get the number of options.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Number of Option";
                    break;
                }
                numOptions = atoi(buf2);

                // Get the option types.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Option Type";
                    break;
                }
                constraint.optionType = atoi(buf2);

                // Get the constraint name.
                if (fscanf(featureFile, "%s", buf2) == EOF)
                {
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Name";
                    break;
                }
                bufString2 = buf2;
                // Remove '"'
                bufString2 = removeCharInString(bufString2, '\"');
                constraint.name = bufString2;

                switch (constraint.optionType) {

                case MEXP_FEATURE_OPTION_SLIDER100:
                case MEXP_FEATURE_OPTION_SLIDER10:
                case MEXP_FEATURE_OPTION_SLIDER1:
                    // Do nothing.
                    break;

                case MEXP_FEATURE_OPTION_TOGGLE:
                    // Parse though the options
                    for (j = 0; j < numOptions; j++)
                    {
                        // Get the option string
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Option Name (Read)";
                            break;
                        }
                        bufString2 = buf2;

                        if (bufString2 != MEXP_STRING_OPTION)
                        {
                            hadError = true;
                            errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Option Name (Check)";
                            break;
                        }

                        // Get the option number
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Option Number";
                            break;
                        }

                        // Get the option value
                        if (fscanf(featureFile, "%s", buf2) == EOF)
                        {
                            hadError = true;
                            errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Option Value";
                            break;
                        }
                    }
                    break;

                default:
                    // Unknown option type.
                    hadError = true;
                    errorMsg = "MEXP_STRING_TASK/ROBOT_CONSTRAINT Unknown Option Type";
                    break;
                }

                genericConstraints.constraints.push_back(constraint);
            }

            switch (constraintType) {

            case MEXP_FEATURE_TASK_CONSTRAINT:
                taskConstraintsList.push_back(
                    (MExpConstraints_t)genericConstraints);
                break;

            case MEXP_FEATURE_ROBOT_CONSTRAINT:
                robotConstraintsList.push_back(
                    (MExpConstraints_t)genericConstraints);
                break;
            }
        }
    }

    fclose(featureFile);

    // Insert the value back to the current data 
    while ((int)(taskConstraintsList.size()) > 0)
    {
        taskConstraints = taskConstraintsList.back();
        taskConstraintsList.pop_back();

        for (i = 0; i < (int)(specifiedTaskList_.size()); i++)
        {
            if (specifiedTaskList_[i].id == taskConstraints.id)
            {
                while ((int)(taskConstraints.constraints.size()) > 0)
                {
                    constraint = taskConstraints.constraints.back();
                    taskConstraints.constraints.pop_back();

                    // Copy the selected option.
                    for (j = 0; j < (int)(specifiedTaskList_[i].localFeatures.size()); j++)
                    {
                        if ((specifiedTaskList_[i].localFeatures[j].id)== (constraint.id))
                        {
                            specifiedTaskList_[i].localFeatures[j].selectedOption
                                = constraint.selectedOption;

                            break;
                        }
                    }
                }

                break;
            }
        }
    }

    while ((int)(robotConstraintsList.size()) > 0)
    {
        robotConstraints = robotConstraintsList.back();
        robotConstraintsList.pop_back();

        for (i = 0; i < (int)(robotConstraintsList_.size()); i++)
        {
            if (robotConstraintsList_[i].id == robotConstraints.id)
            {
                while ((int)(robotConstraints.constraints.size()) > 0)
                {
                    constraint = robotConstraints.constraints.back();
                    robotConstraints.constraints.pop_back();

                    // Copy the selected option.
                    for (j = 0; j < (int)(robotConstraintsList_[i].constraints.size()); j++)
                    {
                        if ((robotConstraintsList_[i].constraints[j].id) == (constraint.id))
                        {
                            robotConstraintsList_[i].constraints[j].selectedOption 
                                = constraint.selectedOption;

                            break;
                        }
                    }
                }

                break;
            }
        }
    }

    gEventLogging->end("MMD Loading the feature data");

    if (hadError)
    {
        fprintf(stderr, "Error(mlab): MlabMissionDesign::loadData_(). Feature file contains errror.\n\a");
        fprintf(stderr, "%s\n", errorMsg.c_str());
    }

    savedTasksLoaded_ = taskCreated;

    icarusTask_ = getTaskID_(STRING_ICARUS_TASK_);

    if (!missionSpecWizardEnabled())
    {
        switch (cnpMode_) {

        case CNP_MODE_RUNTIME:
            runtimeCNPMissionManagerTask_ = getTaskID_(STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_);

            // Deploy all the robots.
            for (i = 0; i < numTasks_; i++)
            {
                featureSelectedOption = getTaskFeatureSelectedOption_(
                    STRING_ROBOT_ID_,
                    specifiedTaskList_[i]);
                deployRuntimeCNPRobot_((int)featureSelectedOption, specifiedTaskList_[i]);
            }
            break;
        }
    }

    return taskCreated;
}

//-----------------------------------------------------------------------
// This function sets the mission time in the global feature.
//-----------------------------------------------------------------------
void MlabMissionDesign::setGlobalFeatureMissionTime_(string missionTimeString)
{
    MExpFeatureOption_t option;
    int index;

    if (globalFeatureMissionTimeIndex_ >= 0)
    {
        option.value = missionTimeString;

        index = globalFeatureMissionTimeIndex_;
        defaultGlobalFeatures_[index].options.clear();
        defaultGlobalFeatures_[index].options.push_back(option);
        defaultGlobalFeatures_[index].selectedOption = 0;
    }
}

//-----------------------------------------------------------------------
// This function saves the specified features to the output file.
//-----------------------------------------------------------------------
void MlabMissionDesign::setFeatureFileStatus_(void)
{
    featureStatusFile_ = fopen(featureStatusFileName_.c_str(), "w+");

    if (featureStatusFile_ == NULL)
    {
        fprintf(stderr, "Warning (mlab): MlabMissionDesign::setFeatureFileStatus_().\n");
        fprintf(
            stderr,
            "The feature status file %s could not be opened.\n",
            featureStatusFileName_.c_str());
        return;
    }

    switch (returnConfigureMission_) {

    case MissionExpert::CONFIGURE:
        saveData_();
        fprintf(featureStatusFile_, "%s", MEXP_STRING_CONFIGURE.c_str());
        gEventLogging->end("MMD");
        break;

    case MissionExpert::REPAIR:
        saveData_();
        fprintf(featureStatusFile_, "%s", MEXP_STRING_REPAIR.c_str());
        gEventLogging->end("Replay success");
        gEventLogging->end("MMD");
        break;

    case MissionExpert::REPAIR_FAILURE:
        fprintf(featureStatusFile_, "%s", MEXP_STRING_REPAIR_FAILURE.c_str());
        gEventLogging->end("Replay failure");
        gEventLogging->end("MMD");
        break;

    case MissionExpert::CANCEL:
    default:
        fprintf(featureStatusFile_, "%s", MEXP_STRING_CANCEL.c_str());
        gEventLogging->cancel("MMD");
        break;
    }

    fclose(featureStatusFile_);
}

//-----------------------------------------------------------------------
// This function creates the toolbox window.
//-----------------------------------------------------------------------
void MlabMissionDesign::createToolbox_(void)
{
    Widget toolbox_frame_w, toolbox_main_w, toolbox_menu_w, toolbox_action_w;
    Widget btnForm, okBtn, cancelBtn;
    Widget globalSettingBtn, robotSettingBtn, missionSettingBtn;
    MMDToolBoxSetCurrentTaskCallbackData_t *callbackData;
    MExpMissionTask_t task;
    vector<Widget> stackedWidget;
    Pixmap pixmap, insensitivePixmap;
    int i, numStackedWidget, numTaskBtnRows;
    int buttonHeight, buttonWidth;
    int toolboxHeight, toolboxWidth, toolboxXPos, toolboxYPos;
    int toolboxActionAreaHeight, extraSpaceHeight;

    // Get the task button dimensions.
    buttonHeight = DEFAULT_TOOLBOX_BUTTON_HEIGHT_;
    buttonWidth = DEFAULT_TOOLBOX_BUTTON_WIDTH_;

    // Get the action area height.
    toolboxActionAreaHeight = 3*(DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_);

    // Copy the number of the task types.
    numVisibleTaskTypes_ = numTaskTypes_;

    // Set the extra space height.
    extraSpaceHeight = DEFAULT_TOOLBOX_EXTRASPACE_HEIGHT_;

    if (missionSpecWizardEnabled())
    {
        // Add one more action button for Mission Setting if it is for
        // ICARUS Wizard.
        toolboxActionAreaHeight += DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_;

        // Do not show the task buttons when ICARUS Wizard is enabled.
        numVisibleTaskTypes_ = 0;

        // Adjust the extra space height.
        extraSpaceHeight = TOOLBOX_EXTRASPACE_HEIGHT_ICARUS_WIZARD_;
    }
    else
    {
        // Do not show the ICARUS task buttons when ICARUS Wizard is disabled.
        if (getTaskID_(STRING_ICARUS_TASK_) != INVALID_TASK_ID_)
        {
            numVisibleTaskTypes_ = (numVisibleTaskTypes_ == 0)?
                numVisibleTaskTypes_ : (numVisibleTaskTypes_ - 1);
        }

        switch (cnpMode_) {

        case CNP_MODE_PREMISSION:
            // Add one more action button for Robot Setting if it is for
            // premission-CNP.
            toolboxActionAreaHeight += DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_;
            break;

        case CNP_MODE_RUNTIME:
            // Add one more action button for Mission Setting if it is for
            // runtime-CNP.
            toolboxActionAreaHeight += DEFAULT_TOOLBOX_ACTIONBUTTON_HEIGHT_;

            // Do not show the task buttons when runtime-CNP.
            numVisibleTaskTypes_ = 0;

            // Adjust the extra space height.
            extraSpaceHeight = TOOLBOX_EXTRASPACE_HEIGHT_RUNTIME_CNP_;
            break;
        }
    }

    // Calculate the number of rows for the task buttons.
    numTaskBtnRows = (int)(numVisibleTaskTypes_/DEFAULT_TOOLBOX_NUM_COLUMNS_);
    if ((numVisibleTaskTypes_ % DEFAULT_TOOLBOX_NUM_COLUMNS_) > 0)
    {
        numTaskBtnRows++;
    }

    // Decide the height of the toolbox based on the number of rows of the button.
    toolboxHeight = (numTaskBtnRows*(buttonHeight+DEFAULT_TOOLBOX_BUTTON_EXTRASPACE_HEIGHT_)) +
        toolboxActionAreaHeight +
        extraSpaceHeight;

    // Also, other dimensions of the toolbox.
    toolboxWidth = DEFAULT_TOOLBOX_WIDTH_;
    toolboxXPos = DEFAULT_TOOLBOX_XPOS_;
    toolboxYPos = DEFAULT_TOOLBOX_YPOS_;
    
    // Popup shell
    toolbox_w_ = XtVaCreatePopupShell
        ("Toolbox",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNx, toolboxXPos,
         XmNy, toolboxYPos,
         XmNheight, toolboxHeight,
         XmNwidth, toolboxWidth,
         NULL);

    toolbox_frame_w = XtVaCreateWidget
        ("toolbox_frame_w",
         xmFrameWidgetClass, toolbox_w_,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNheight, toolboxHeight,
         XmNwidth, toolboxWidth,
         NULL);

    stackedWidget.push_back(toolbox_frame_w);

    toolbox_main_w = XtVaCreateWidget
        ("toolbox_main_w",
         xmRowColumnWidgetClass, toolbox_frame_w,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, toolboxHeight,
         XmNwidth, toolboxWidth,
         NULL);

    stackedWidget.push_back(toolbox_main_w);

    toolbox_menu_w = XtVaCreateWidget
        ("toolbox_menu_w",
         xmRowColumnWidgetClass, toolbox_main_w,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, DEFAULT_TOOLBOX_NUM_COLUMNS_,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, toolboxHeight-toolboxActionAreaHeight,
         XmNwidth, toolboxWidth,
         NULL);

    stackedWidget.push_back(toolbox_menu_w);

    // Task buttons
    task_btn_ = new Widget[numVisibleTaskTypes_];

    // Create buttons
    for (i = 0; i < numVisibleTaskTypes_; i++)
    {
        task = defaultTaskList_[i];

        if ((!missionSpecWizardEnabled()) &&
            (task.name == STRING_ICARUS_TASK_))
        {
            continue;
        }

        // The for for the button.
        btnForm = XtVaCreateManagedWidget
            ("btnForm",
             xmFormWidgetClass, toolbox_menu_w,
             XmNalignment, XmALIGNMENT_CENTER,
             XmNheight, buttonHeight,
             XmNwidth, buttonWidth,
             NULL);

        stackedWidget.push_back(btnForm);

        pixmap = getPixmapFromList_(task.buttonName, false);
        insensitivePixmap = getPixmapFromList_(task.buttonName, true);

        task_btn_[i] = XtVaCreateWidget
            (task.buttonName.c_str(),
             xmPushButtonWidgetClass, btnForm,
             XmNlabelType, XmPIXMAP,
             XmNlabelPixmap, pixmap,
             XmNlabelInsensitivePixmap, insensitivePixmap,
             XmNbackground, MMDToolboxBtnBgColorUnset_,
             XmNhighlightColor, parentWidgetBg_,
             XmNborderColor, MMDToolboxBtnBgColorUnset_,
             XmNarmColor, MMDToolboxBtnBgColorUnset_,
             NULL);

        stackedWidget.push_back(task_btn_[i]);

        callbackData = new MMDToolBoxSetCurrentTaskCallbackData_t;
        callbackData->task = task.id;
        callbackData->mlabMissionDesignInstance = this;
        XtAddCallback
            (task_btn_[i],
             XmNactivateCallback,
             (XtCallbackProc)cbSetCurrentTask_, 
             (void *)callbackData);
    }

    toolbox_action_w = XtVaCreateWidget
        ("toolbox_action_w",
         xmRowColumnWidgetClass, toolbox_main_w,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, toolbox_menu_w,
         XmNheight, toolboxActionAreaHeight,
         XmNwidth, toolboxWidth,
         NULL);

    stackedWidget.push_back(toolbox_action_w);

    // Global Setting button
    globalSettingBtn = XtVaCreateWidget
        (TOOLBOX_GLOBALSETTING_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, toolbox_action_w,
         XmNwidth, toolboxWidth,
         XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
         XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    stackedWidget.push_back(globalSettingBtn);

    XtAddCallback
        (globalSettingBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbToolboxGlobalSettingBtn_,
         (void *)this);

    // Robot Setting button
    if (missionSpecWizardEnabled())
    {
        missionSettingBtn = XtVaCreateWidget
            (TOOLBOX_MISSIONSETTING_BUTTON_LABEL_.c_str(),
             xmPushButtonWidgetClass, toolbox_action_w,
             XmNwidth, toolboxWidth,
             XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
             XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.orange,
             XmNhighlightOnEnter, false,
             XmNtraversalOn, false,
             NULL);

        stackedWidget.push_back(missionSettingBtn);

        XtAddCallback
            (missionSettingBtn,
             XmNactivateCallback,
             (XtCallbackProc)cbToolboxMissionSettingBtn_,
             (void *)this);
    }
    else
    {
        switch (cnpMode_) {

        case CNP_MODE_PREMISSION:
            robotSettingBtn = XtVaCreateWidget
                (TOOLBOX_ROBOTSETTING_BUTTON_LABEL_.c_str(),
                 xmPushButtonWidgetClass, toolbox_action_w,
                 XmNwidth, toolboxWidth,
                 XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                 XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
                 XmNhighlightOnEnter, false,
                 XmNtraversalOn, false,
                 NULL);

            stackedWidget.push_back(robotSettingBtn);

            XtAddCallback
                (robotSettingBtn,
                 XmNactivateCallback,
                 (XtCallbackProc)cbToolboxRobotSettingBtn_,
                 (void *)this);
            break;

        case CNP_MODE_RUNTIME:
            missionSettingBtn = XtVaCreateWidget
                (TOOLBOX_MISSIONSETTING_BUTTON_LABEL_.c_str(),
                 xmPushButtonWidgetClass, toolbox_action_w,
                 XmNwidth, toolboxWidth,
                 XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                 XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.orange,
                 XmNhighlightOnEnter, false,
                 XmNtraversalOn, false,
                 NULL);

            stackedWidget.push_back(missionSettingBtn);

            XtAddCallback
                (missionSettingBtn,
                 XmNactivateCallback,
                 (XtCallbackProc)cbToolboxMissionSettingBtn_,
                 (void *)this);
            break;
        }
    }

    // OK button
    okBtn = XtVaCreateWidget
        (TOOLBOX_OK_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, toolbox_action_w,
         XmNwidth, toolboxWidth,
         XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
         XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
         XmNhighlightOnEnter, true,
         XmNtraversalOn, true,
         NULL);

    stackedWidget.push_back(okBtn);

    callbackData = new MMDToolBoxSetCurrentTaskCallbackData_t;
    callbackData->task = MissionExpert::CONFIGURE;
    callbackData->mlabMissionDesignInstance = this;
    XtAddCallback
        (okBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbExit_,
         (void *)callbackData);

    // Cancel button
    cancelBtn = XtVaCreateWidget
        (TOOLBOX_CANCEL_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, toolbox_action_w,
         XmNwidth, toolboxWidth,
         XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
         XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    stackedWidget.push_back(cancelBtn);

    callbackData = new MMDToolBoxSetCurrentTaskCallbackData_t;
    callbackData->task = MissionExpert::CANCEL;
    callbackData->mlabMissionDesignInstance = this;
    XtAddCallback
        (cancelBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbExit_,
         (void *)callbackData);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }

    XtPopup (toolbox_w_, XtGrabNone);
    updateToolboxBtn_();

    gEventLogging->log("MMD Toolbox popped up.");
}

//-----------------------------------------------------------------------
// This function updates the backgroud of the buttons.
//-----------------------------------------------------------------------
void MlabMissionDesign::updateToolboxBtn_(void)
{
    int i;

    for (i = 0; i < numVisibleTaskTypes_; i++)
    {
        XtVaSetValues(task_btn_[i], XmNbackground, toolboxBtnBgColor_(i), NULL);
    }
}

//-----------------------------------------------------------------------
// This function changes the backgroud of the button based on the task.
//-----------------------------------------------------------------------
Pixel MlabMissionDesign::toolboxBtnBgColor_(int button)
{
    if (button == MMDCurrentTask_)
    {
        return MMDToolboxBtnBgColorSet_;
    }
    else
    {
        return MMDToolboxBtnBgColorUnset_;
    }
}

//-----------------------------------------------------------------------
// This function creates the repair window.
//-----------------------------------------------------------------------
void MlabMissionDesign::createRepairWindow(void)
{
    Widget repairwindow_frame_w, repairwindow_main_w, repairwindow_action_w;
    Widget okBtn, cancelBtn;
    MMDRepairWindowCallbackData_t *callbackData;
    vector<Widget> stackedWidget;
    int i, numStackedWidget;
    int repairwindowHeight, repairwindowWidth, repairwindowXPos, repairwindowYPos;
    int repairwindowActionAreaHeight;

    // Set the mode to be REPAIR_MODE.
    MMDMode_ = REPAIR_MODE;

    // Get the action area height.
    repairwindowActionAreaHeight = DEFAULT_REPAIRWINDOW_ACTIONAREA_HEIGHT_;

    // Decide the height of the window.
    repairwindowHeight = 
        repairwindowActionAreaHeight + 
        DEFAULT_REPAIRWINDOW_EXTRASPACE_HEIGHT_;

    // Also, other dimensions of the repairwindow.
    repairwindowWidth = DEFAULT_REPAIRWINDOW_WIDTH_;
    repairwindowXPos = DEFAULT_REPAIRWINDOW_XPOS_;
    repairwindowYPos = DEFAULT_REPAIRWINDOW_YPOS_;
    
    // Popup shell
    repairwindow_w_ = XtVaCreatePopupShell
         (" ",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNx, repairwindowXPos,
         XmNy, repairwindowYPos,
         XmNheight, repairwindowHeight,
         XmNwidth, repairwindowWidth,
         NULL);

    repairwindow_frame_w = XtVaCreateWidget
        ("repairwindow_frame_w",
         xmFrameWidgetClass, repairwindow_w_,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNheight, repairwindowHeight,
         XmNwidth, repairwindowWidth,
         NULL);

    stackedWidget.push_back(repairwindow_frame_w);

    repairwindow_main_w = XtVaCreateWidget
        ("repairwindow_main_w",
         xmRowColumnWidgetClass, repairwindow_frame_w,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, repairwindowHeight,
         XmNwidth, repairwindowWidth,
         NULL);

    stackedWidget.push_back(repairwindow_main_w);

    repairwindow_action_w = XtVaCreateWidget
        ("repairwindow_action_w",
         xmRowColumnWidgetClass, repairwindow_main_w,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, repairwindowActionAreaHeight,
         XmNwidth, repairwindowWidth,
         NULL);

    stackedWidget.push_back(repairwindow_action_w);

    // OK button
    okBtn = XtVaCreateWidget
        (REPAIRWINDOW_OK_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, repairwindow_action_w,
         XmNwidth, repairwindowWidth,
         XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
         XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
         XmNhighlightOnEnter, true,
         XmNtraversalOn, true,
         NULL);

    stackedWidget.push_back(okBtn);

    callbackData = new MMDRepairWindowCallbackData_t;
    callbackData->mode = MissionExpert::REPAIR;
    callbackData->mlabMissionDesignInstance = this;
    XtAddCallback
        (okBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbRepair_,
         (void *)callbackData);

    // Cancel button
    cancelBtn = XtVaCreateWidget
        (REPAIRWINDOW_CANCEL_BUTTON_LABEL_.c_str(),
         xmPushButtonWidgetClass, repairwindow_action_w,
         XmNwidth, repairwindowWidth,
         XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
         XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    stackedWidget.push_back(cancelBtn);

    callbackData = new MMDRepairWindowCallbackData_t;
    callbackData->mode = MissionExpert::CANCEL;
    callbackData->mlabMissionDesignInstance = this;
    XtAddCallback
        (cancelBtn,
         XmNactivateCallback,
         (XtCallbackProc)cbRepair_,
         (void *)callbackData);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }

    XtPopup (repairwindow_w_, XtGrabNone);

    while (XtAppPending(appContext_))
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    gEventLogging->start("Replay window");
    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_PLAYBACK);
}

//-----------------------------------------------------------------------
// This function sends the fixing information (entered by the user)
// to CBRServer.
//-----------------------------------------------------------------------
void MlabMissionDesign::repairMission_(void)
{
    int originalPlaybackStatus, questionStatus;
    int exitStatus = MissionExpert::CANCEL;

    // Pause the playback.
    originalPlaybackStatus = gPlayBack->getPlaybackStatus();
    gPlayBack->changePlaybackStatus(MlabPlayBack::PAUSED);

    if (!confirmUser_(CONFIRM_OFFENDING_PART_PHRASE_, false))
    {
        // The user did not confirm to fix the mission.
        // Resume with the original playback status.
        gPlayBack->changePlaybackStatus(originalPlaybackStatus);
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_PLAYBACK);

        return;
    }

    // Ask questions to get the solution of the problem.
    questionStatus = askRepairQuestions_();

    switch (questionStatus) {

    case MMDRQS_HAS_SOLUTION:
        askRepairFollowUpQuestion_();        
        exitStatus = MissionExpert::REPAIR;
        break;

    case MMDRQS_NO_SOLUTION:
        exitStatus = MissionExpert::REPAIR_FAILURE;
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_NO_REPAIR_SOLUTION_FOUND);
        break;

    default:
        exitStatus = MissionExpert::CANCEL;
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE_NO);
        gEventLogging->cancel("Replay window");
        break;
    }

    exit_(exitStatus);
}

//-----------------------------------------------------------------------
// This function asks questions to repair the mission.
//-----------------------------------------------------------------------
int MlabMissionDesign::askRepairQuestions_(void)
{
    MMDRepairQuestion_t repairQuestion;
    int lastSelectedOption = -1;
    
    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_REPAIR_GUIDE);

    while (true)
    {
        repairQuestion = mlabCBRClient_->retrieveRepairQuestion(lastSelectedOption);

        switch (repairQuestion.status) {

        case MMDRQS_HAS_SOLUTION:
        case MMDRQS_NO_SOLUTION:
            return repairQuestion.status;

        case MMDRQS_NORMAL_QUESTION:
            break;
        }

        createAndPopupGeneralToggleWindow_(
            repairQuestion.question,
            repairQuestion.options);

        // Wait for the answer.
        while(!generalToggleWindowDone_)
        {
            if (XtAppPending(appContext_))
            {
                XtAppProcessEvent(appContext_, XtIMAll);
            }
        }

        lastSelectedOption = generalToggleWindowResult_;
    }

    return (int)MMDRQS_NO_SOLUTION;
}

//-----------------------------------------------------------------------
// This function asks some follow up questions to repair the mission.
//-----------------------------------------------------------------------
void MlabMissionDesign::askRepairFollowUpQuestion_(void)
{
    MExpPoint2D_t viaPoint, *newAndOldViaPoints = NULL;
    MMDRepairQuestion_t repairQuestion;
    vector<MlabPlayBack::StateInfo_t> mpbStateInfoList;
    vector<CBRPlanner_StateInfo_t> stateInfoList;
    CBRPlanner_StateInfo_t stateInfo;
    double closeRange = 0;
    int i, stateInfoListSize;
    bool done = false;
    const bool REPLY_STATE_NAME = false;

    while (!done)
    {
        repairQuestion = mlabCBRClient_->retrieveRepairFollowUpQuestion();

        switch (repairQuestion.type) {

        case MMDRQS_TOGGLE:
            createAndPopupGeneralToggleWindow_(
                repairQuestion.question,
                repairQuestion.options);

            // Wait for the answer.
            while(!generalToggleWindowDone_)
            {
                XtAppProcessEvent(appContext_, XtIMAll);
            }

            mlabCBRClient_->replyRepairFollowUpQuestionToggle(
                generalToggleWindowResult_);
            break;

        case MMDRQS_ADD_MAP_POINT:
            viaPoint = askAndAddNewViaPoint_(repairQuestion.question);
            //gEventLogging->log("User asked to add a via point");
            mlabCBRClient_->replyRepairFollowUpQuestionAddMapPoint(
                viaPoint.x,
                viaPoint.y);
            break;

        case MMDRQS_MOD_MAP_POINT:
            newAndOldViaPoints = askAndMoveViaPoint_(
                repairQuestion.question,
                &closeRange);
            //gEventLogging->log("User asked to modify an existing point");//Lilia
            if (newAndOldViaPoints != NULL)
            {
                mlabCBRClient_->replyRepairFollowUpQuestionMoveMapPoint(
                    newAndOldViaPoints[OLD_VIA_POINT].x,
                    newAndOldViaPoints[OLD_VIA_POINT].y,
                    newAndOldViaPoints[NEW_VIA_POINT].x,
                    newAndOldViaPoints[NEW_VIA_POINT].y,
                    closeRange);
                delete [] newAndOldViaPoints;
                newAndOldViaPoints = NULL;
            }
            else
            {
                fprintf(
                    stderr,
                    "Error (mlab): MlabMissionDesign::askRepairFollowUpQuestion_().\n");
                fprintf(stderr, "Invalid memory allocation.\n");
                quit_mlab();
            }
            break;

        case MMDRQS_DEL_MAP_POINT:
            viaPoint = askAndDeleteViaPoint_(
                repairQuestion.question,
                &closeRange);
            gEventLogging->log("User asked to delete a waypoint");
            mlabCBRClient_->replyRepairFollowUpQuestionDeleteMapPoint(
                viaPoint.x,
                viaPoint.y,
                closeRange);
            break;

        case MMDRQS_STATE_NAME:
            // State name should already being sent to CBRServer via CfgEdit.
            // This part is now obsolete.
            if (REPLY_STATE_NAME)
            {
                mpbStateInfoList = gPlayBack->getCurrentStateInfoList();
                stateInfoList.clear();
                stateInfoListSize = mpbStateInfoList.size();
                if (stateInfoListSize == 0)
                {
                    break;
                }

                for (i = 0; i < stateInfoListSize; i++)
                {
                    stateInfo.fsaName = mpbStateInfoList[i].fsaName;
                    stateInfo.stateName = mpbStateInfoList[i].stateName;
                    stateInfoList.push_back(stateInfo);
                }

                mlabCBRClient_->replyRepairFollowUpQuestionStateName(stateInfoList);
            }
            break;

        default:
        case MMDRQS_NO_QUESTION:
            done = true;
            break;
        }
    }
}

//-----------------------------------------------------------------------
// This function adds a new via point on the overlay that was
// selected by the user.
//-----------------------------------------------------------------------
MExpPoint2D_t MlabMissionDesign::askAndAddNewViaPoint_(string question)
{
    MExpPoint2D_t viaPoint;

    viaPoint.x = lastMouseClickedPointX_;
    viaPoint.y = lastMouseClickedPointY_;

    while (true)
    {
        createAndPopupGeneralMessageWindow_(question);

        // Wait for the answer.
        while ((viaPoint.x == lastMouseClickedPointX_) ||
               (viaPoint.y == lastMouseClickedPointY_))
        {
            if (XtAppPending(appContext_))
            {
                XtAppProcessEvent(appContext_, XtIMAll);
            }
        }

        viaPoint.x = lastMouseClickedPointX_;
        viaPoint.y = lastMouseClickedPointY_;

        placeViaPoint_(viaPoint.x, viaPoint.y);
        //gEventLogging->log("user placed a new via point at %.2f, %.2f", viaPoint.x, viaPoint.y);
        popdownGeneralMessageWindow_();
                
        if (confirmUser_(CONFIRM_PROCEED_PHRASE_, false))
        {
            break;
        }

        clearLastViaPoint_();
    }

    return viaPoint;
}

//-----------------------------------------------------------------------
// This function asks user to drag an existing waypoint or via point
// to a desired location.
//-----------------------------------------------------------------------
MExpPoint2D_t *MlabMissionDesign::askAndMoveViaPoint_(
    string question,
    double *closeRange)
{
    MExpPoint2D_t viaPoint, *newAndOldPoints = NULL;
    MExpMissionTask_t task;
    double lastMouseClickedPointX, lastMouseClickedPointY;
    double radius = 0;
    int viaPointIndex = -1;
    bool isInsideTask = false, isInsideViaPoint = false;

    newAndOldPoints = new MExpPoint2D_t[NUM_MOD_VIA_POINTS];

    lastMouseClickedPointX = lastMouseClickedPointX_;
    lastMouseClickedPointY = lastMouseClickedPointY_;

    while (true)
    {
        createAndPopupGeneralMessageWindow_(question);

        while (true)
        {
            isInsideTask = false;
            isInsideViaPoint = false;

            // Wait for the answer.
            while ((lastMouseClickedPointX == lastMouseClickedPointX_) ||
                   (lastMouseClickedPointY == lastMouseClickedPointY_))
            {
                if (XtAppPending(appContext_))
                {
                    XtAppProcessEvent(appContext_, XtIMAll);
                }
            }

            lastMouseClickedPointX = lastMouseClickedPointX_;
            lastMouseClickedPointY = lastMouseClickedPointY_;

            isInsideTask = isInsideTask_(
                lastMouseClickedPointX,
                lastMouseClickedPointY,
                drawingUnitLength_,
                &task);

            if (isInsideTask)
            {
                if (task.name == STRING_WAYPOINTS_TASK_)
                {
                    break;
                }
            }

            isInsideViaPoint = isInsideViaPoint_(
                lastMouseClickedPointX,
                lastMouseClickedPointY,
                drawingUnitLength_,
                &viaPoint,
                &viaPointIndex);

            if (isInsideViaPoint)
            {
                break;
            }
        }

        popdownGeneralMessageWindow_();

        // Lift the waypoint / via point.
        if (isInsideTask)
        {
            newAndOldPoints[OLD_VIA_POINT] = task.position;
            radius = placeCircleDiameter_*drawingUnitLength_/2.0;
            liftTask_(task);
        }
        else if (isInsideViaPoint)
        {
            newAndOldPoints[OLD_VIA_POINT] = viaPoint;
            radius = placeCircleDiameter_*drawingUnitLength_/2.0;
            liftViaPoint_(viaPointIndex);
        }
        else
        {
            continue;
        }
                
        // Wait for the user to put down the waypoint / via point.
        while (draggingCircle_.dragging)
        {
            if (XtAppPending(appContext_))
            {
                XtAppProcessEvent(appContext_, XtIMAll);
            }
        }

        newAndOldPoints[NEW_VIA_POINT].x = lastMouseClickedPointX_;
        newAndOldPoints[NEW_VIA_POINT].y = lastMouseClickedPointY_;

        // The user put down the waypoint / via point. Ask for
        // the confirmation.
        if (confirmUser_(CONFIRM_PROCEED_PHRASE_, false))
        {
            // Answered "Yes".
            break;
        }

        // Restore the original position.
        if (isInsideTask)
        {
            liftTask_(task);
            putdownLiftedTask_(task.position.x, task.position.y);
        }
        else if (isInsideViaPoint)
        {
            liftViaPoint_(viaPointIndex);
            putdownLiftedViaPoint_(viaPoint.x, viaPoint.y);
        }
    }

    *closeRange = radius;

    return newAndOldPoints;
}

//-----------------------------------------------------------------------
// This function eliminates a waypoint or via point selected by the
// user.
//-----------------------------------------------------------------------
MExpPoint2D_t MlabMissionDesign::askAndDeleteViaPoint_(
    string question,
    double *closeRange)
{
    MExpPoint2D_t viaPoint, pointToDelete;
    MExpMissionTask_t task;
    double lastMouseClickedPointX, lastMouseClickedPointY;
    double radius;
    int viaPointIndex = -1;
    bool isInsideTask = false, isInsideViaPoint = false;
    bool confirmed = false;

    lastMouseClickedPointX = lastMouseClickedPointX_;
    lastMouseClickedPointY = lastMouseClickedPointY_;

    while (true)
    {
        createAndPopupGeneralMessageWindow_(question);

        while (true)
        {
            isInsideTask = false;
            isInsideViaPoint = false;

            // Wait for the answer.
            while ((lastMouseClickedPointX == lastMouseClickedPointX_) ||
                   (lastMouseClickedPointY == lastMouseClickedPointY_))
            {
                if (XtAppPending(appContext_))
                {
                    XtAppProcessEvent(appContext_, XtIMAll);
                }
            }

            lastMouseClickedPointX = lastMouseClickedPointX_;
            lastMouseClickedPointY = lastMouseClickedPointY_;

            isInsideTask = isInsideTask_(
                lastMouseClickedPointX,
                lastMouseClickedPointY,
                drawingUnitLength_,
                &task);

            if (isInsideTask)
            {
                if (task.name == STRING_WAYPOINTS_TASK_)
                {
                    break;
                }
            }

            isInsideViaPoint = isInsideViaPoint_(
                lastMouseClickedPointX,
                lastMouseClickedPointY,
                drawingUnitLength_,
                &viaPoint,
                &viaPointIndex);

            if (isInsideViaPoint)
            {
                break;
            }
        }

        popdownGeneralMessageWindow_();

        // Lift the waypoint / via point.
        if (isInsideTask)
        {
            pointToDelete = task.position;
            radius = placeCircleDiameter_*drawingUnitLength_/2.0;
        }
        else if (isInsideViaPoint)
        {
            pointToDelete = viaPoint;
            radius = placeCircleDiameter_*drawingUnitLength_/2.0;
        }
        else
        {
            continue;
        }
                
        // Mark the circle.
        XtAppLock(appContext_);
        XLockDisplay(display_);
        DrawFilledCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.redXOR);
        DrawFilledCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.whiteXOR);
        DrawCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.XOR);
        DrawCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.whiteXOR);
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);

        // The user clicked on the waypoint / via point. Ask for
        // the confirmation.
        confirmed = confirmUser_(CONFIRM_DELETE_PHRASE_, false);

        // Unmark the circle.
        XtAppLock(appContext_);
        XLockDisplay(display_);
        DrawFilledCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.redXOR);
        DrawFilledCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.whiteXOR);
        DrawCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.XOR);
        DrawCircle(pointToDelete.x, pointToDelete.y, radius, gGCs.whiteXOR);
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);

        if (confirmed)
        {
            break;
        }
    }

    if (isInsideTask)
    {
        removeTask_(task);
    }
    else if (isInsideViaPoint)
    {
        removeViaPoint_(viaPointIndex);
    }

    *closeRange = radius;

    return pointToDelete;
}

//-----------------------------------------------------------------------
// This function creates the toolbox window and its related widgets.
//-----------------------------------------------------------------------
void MlabMissionDesign::createWindows(void)
{
    MExpMissionTask_t task;
    bool showDefaultInstruction = true;

    // Starting the DESIGN_MODE.
    MMDMode_ = DESIGN_MODE;

    returnConfigureMission_ = MissionExpert::CANCEL;

    // This mode does not support the via points.
    // Clear all the via points.
    clearViaPoints_();

    createPixmapList_(
        (disableColorButtons_)? parentWidgetFg_ : gColorPixel.black,
        (disableColorButtons_)? parentWidgetBg_ : gColorPixel.orange,
        parentWidget_);

    createCursorList_(
        gColorPixel.black,
        gColorPixel.white,
        parentWidget_);

    createToolbox_();

    createGlobalFeatureMenu_();

    windowsCreated_ = true;

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_INITIAL);

    if (!savedTasksLoaded_)
    {
        if (missionSpecWizardEnabled())
        {
            confirmUser_(CONFIRM_ENABLE_ICARUS_WIZARD_INIT_INSTRUCTION_PHRASE_, true);
            placeIcarusTask_();
            showDefaultInstruction = false;
        }
        else
        {
            switch (cnpMode_) {

            case CNP_MODE_RUNTIME:
                confirmUser_(CONFIRM_RUNTIME_CNP_INIT_INSTRUCTION_PHRASE_, true);
                placeRuntimeCNPMissionManager_();
                showDefaultInstruction = false;
                break;
            }
        }

        if (showDefaultInstruction)
        {
            confirmUser_(CONFIRM_DEFAULT_INIT_INSTRUCTION_PHRASE_, true);
        }
    }
    else
    {
        if (missionSpecWizardEnabled())
        {
            confirmUser_(CONFIRM_ENABLE_ICARUS_WIZARD_INIT_INSTRUCTION_PHRASE_, true);

            if ((int)(specifiedTaskList_.size()) > 0)
            {
                task = specifiedTaskList_[0];

                if ((task.id) == icarusTask_)
                {
                    popUpRightBtnMenu_(task);
                }
            }
        }
    }

    if (gAutomaticExecution)
    {
        exit_(MissionExpert::CONFIGURE);
    }
}

//-----------------------------------------------------------------------
// This function creates a new task and adds it in the array.
//-----------------------------------------------------------------------
int MlabMissionDesign::addNewTask_(
    double taskPosX,
    double taskPosY,
    int taskType,
    MExpMissionTask_t *addedTask)
{
    MExpFeature_t localFeature;
    MExpMissionTask_t task, defaultTask;
    vector<MExpFeature_t> localFeatures;
    string taskName;
    int i, j, numSameTasks;

    // At first, create a list of local features with default values.
    localFeatures = defaultLocalFeatures_;

    // Next, find a task in the list that matches with this ID.
    defaultTask = defaultTaskList_[0];
    for (i = 0; i < numTaskTypes_; i++)
    {
        if (defaultTaskList_[i].id == taskType)
        {
            defaultTask = defaultTaskList_[i];
            break;
        }
    }

    // Check to see if the number of the placed tasks doesn't exceed
    // the maximum usage.
    if (defaultTask.maxUsage != MEXP_USAGE_INFINITY)
    {
        numSameTasks = 0;
        for (i = 0; i < (int)(specifiedTaskList_.size()); i++)
        {
            if (specifiedTaskList_[i].id == taskType)
            {
                numSameTasks++;
            }

            if (numSameTasks >= defaultTask.maxUsage)
            {
                warn_userf(
                    EXCEEDED_TASK_USAGE_PHRASE_.c_str(),
                    defaultTask.name.c_str(),
                    defaultTask.maxUsage,
                    (defaultTask.maxUsage > 1)? "s" : "");
                    
                return PLACETASK_FAILURE;
            }
        }
    }

    // Use the specified option for the default task if it is specified.
    for (i = 0; i < ((int)(defaultTask.localFeatures.size())); i++)
    {
        for (j = 0; j < ((int)(localFeatures.size())); j++)
        {
            if (defaultTask.localFeatures[i].name == localFeatures[j].name)
            {
                localFeatures[j].selectedOption = defaultTask.localFeatures[i].selectedOption;
            }
        }
    }

    task.localFeatures = localFeatures;
    task.number = numTasks_;
    task.id = defaultTask.id;
    task.name = defaultTask.name;
    task.position.x = taskPosX;
    task.position.y = taskPosY;
    global2geographic(
        task.position.x,
        task.position.y,
        &(task.geo.latitude),
        &(task.geo.longitude));
    specifiedTaskList_.push_back(task);

    *addedTask = task;

    numTasks_++;

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_TASK);

    return true;
}

//-----------------------------------------------------------------------
// This function places the specified task mark to the specified place
// in the overlay.
//-----------------------------------------------------------------------
void MlabMissionDesign::placeTask_(double taskPosX, double taskPosY, int taskType) 
{
    MExpMissionTask_t addedTask;
    string envOption = EMPTY_STRING_;
    string entryKey;
    int status, robotID;
    bool runtimeCNP = false;
    const int DEFAULT_START_CONSTRAINT_ID = 0;
    const int DEFAULT_START_PAGE = 1;

    // Check for the runtime-CNP status
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    // Add a new task.
    status = addNewTask_(taskPosX, taskPosY, taskType, &addedTask);

    if (status == PLACETASK_FAILURE)
    {
        // Task could not be added.
        return;
    }

    if (missionSpecWizardEnabled())
    {
        if (addedTask.name == STRING_ICARUS_TASK_)
        {
            popUpRightBtnMenu_(addedTask);
        }
    }
    else if (runtimeCNP)
    {
        // Get the available robot ID to add.
        robotID = getSmallestDeployableRuntimeCNPRobotID_();

        if (robotID == INVALID_ROBOT_ID_)
        {
            confirmUser_(CONFIRM_RUNTIME_CNP_NO_DEPLOYABLE_ROBOT_PHRASE_, true);

            // Remove the task
            specifiedTaskList_.pop_back();
            numTasks_--;
            return;
        }

        // Change the ROBOT_ID feature of the task.
        replaceTaskFeatureSelectedOption_(
            STRING_ROBOT_ID_,
            (float)(robotID),
            &(specifiedTaskList_[numTasks_-1]));

        // Add the robot ID to the deployed CNP robot list.
        addedTask = specifiedTaskList_[numTasks_-1];
        deployRuntimeCNPRobot_(robotID, addedTask);

        // Mark this robot as the selected deployed robot.
        selectedDeployedRuntimeCNPRobotID_ = robotID;

        // Pop up the robot constraint menu if it is for the bidder.
        // If it is for MissionManager, pop up righBtnMenu.
        if (addedTask.name == STRING_RUNTIME_CNP_BIDDER_TASK_)
        {
            upRobotConstraintsList_ = robotConstraintsList_;
            createRobotConstraintMenu_(
                robotID,
                DEFAULT_START_CONSTRAINT_ID,
                DEFAULT_START_PAGE);
        }
        else if (addedTask.name == STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_)
        {
            popUpRightBtnMenu_(addedTask);
        }
    }

    // Update the overlay.
    entryKey = generateTaskEntryKey_(addedTask);
    gt_add_passage_point(
        (char *)(entryKey.c_str()),
        taskPosX,
        taskPosY,
        placeCircleDiameter_*drawingUnitLength_);

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    gEventLogging->log(
        "MMD Task #%d [%s] added on <%.2f %.2f>",
        addedTask.number,
        addedTask.name.c_str(),
        addedTask.position.x,
        addedTask.position.y);

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT);
}

//-----------------------------------------------------------------------
// This function places a polygon point.
//-----------------------------------------------------------------------
void MlabMissionDesign::placePolygonPoint_(double x, double y) 
{
    MExpPoint2D_t pos;
    MExpLatLon_t geo;

    pos.x = x;
    pos.y = y;

    global2geographic(
        pos.x,
        pos.y,
        &(geo.latitude),
        &(geo.longitude));

    specifyingPolygon_.posList.push_back(pos);
    specifyingPolygon_.geoList.push_back(geo);
    specifyingPolygon_.dragging = true;
    dragPolygonPoint_();
}

//-----------------------------------------------------------------------
// This function places a circle to the point.
//-----------------------------------------------------------------------
void MlabMissionDesign::placeCircle_(double x, double y) 
{
    char entryKey[1024];
    char *username = NULL;
    int pid;

    // Create some label for this circle.
    pid = getpid();
    username = getenv("USER");
    sprintf(entryKey, "%s%d-%d*", username, pid, placeCircleIndex_);

    gt_add_passage_point(entryKey, x, y, placeCircleDiameter_*drawingUnitLength_);

    placeCircleIndex_++;
}

//-----------------------------------------------------------------------
// This function places a via point to the point.
//-----------------------------------------------------------------------
void MlabMissionDesign::placeViaPoint_(double x, double y) 
{
    MExpPoint2D_t viaPoint;
    string entryKey;
    int index;

    index = viaPointList_.size();

    // Create some label for this via point.
    entryKey = generateViaPointEntryKey_(index);

    // Display it.
    gt_add_passage_point(
        (char *)(entryKey.c_str()),
        x,
        y,
        placeCircleDiameter_*drawingUnitLength_);

    // Add it to the list.
    viaPoint.x = x;
    viaPoint.y = y;
    viaPointList_.push_back(viaPoint);
}

//-----------------------------------------------------------------------
// This function deletes last via point.
//-----------------------------------------------------------------------
void MlabMissionDesign::clearLastViaPoint_(void) 
{
    string entryKey;
    int index;
    int numPoints;

    numPoints = viaPointList_.size();

    if (numPoints == 0)
    {
        return;
    }

    index = numPoints - 1;
    entryKey = generateViaPointEntryKey_(index);
    gt_delete_db_entry((char *)(entryKey.c_str()));

    viaPointList_.pop_back();

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function deletes all the pasage points.
//-----------------------------------------------------------------------
void MlabMissionDesign::clearViaPoints_(void) 
{
    string entryKey;
    int i;

    for (i = 0; i < (int)(viaPointList_.size()); i++)
    {
        entryKey = generateViaPointEntryKey_(i);
        gt_delete_db_entry((char *)(entryKey.c_str()));
    }

    viaPointList_.clear();

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function removes a task from the list.
//-----------------------------------------------------------------------
void MlabMissionDesign::removeTask_(MExpMissionTask_t task)
{
    MExpMissionTask_t taskAdd, taskRemove;
    string entryKeyAdd, entryKeyRemove;
    int robotID, taskNum;
    int i;
    bool runtimeCNP = false;
    
    // Check for the runtime-CNP status
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    // Get the key for removing task.
    entryKeyRemove = generateTaskEntryKey_(task);
    
    if (missionSpecWizardEnabled())
    {
        if (task.id == icarusTask_)
        {
            confirmUser_(CONFIRM_ENABLE_ICARUS_WIZARD_NO_REMOVE_TASK_, true);
            return;
        }
    }
    else if (runtimeCNP)
    {
        if (task.id == runtimeCNPMissionManagerTask_)
        {
            confirmUser_(CONFIRM_RUNTIME_CNP_NO_REMOVE_MISSION_MANAGER_, true);
            return;
        }

        robotID = getDeployedRuntimeCNPRobotIDFromTaskNumber_(task.number);
        undeployRuntimeCNPRobot_(robotID);
    }

    gt_delete_db_entry((char *)(entryKeyRemove.c_str()));

    gEventLogging->log(
        "MMD Task #%d [%s] deleted",
        task.number,
        task.name.c_str());

    numTasks_--;

    if (numTasks_)
    {
        taskNum = task.number;
        for (i = taskNum; i < numTasks_; i++)
        {
            // Copy the one above to this one, and delete the one above.
            taskRemove = specifiedTaskList_[i+1];
            taskAdd = taskRemove;
            taskAdd.number = i;
            specifiedTaskList_[i] = taskAdd;

            if (runtimeCNP)
            {
                // Get the key for removing task.
                entryKeyRemove = generateTaskEntryKey_(taskRemove);

                // Get the robot ID.
                robotID = getDeployedRuntimeCNPRobotIDFromTaskNumber_(
                    taskRemove.number);

                // Remove from the deploy list, too.
                undeployRuntimeCNPRobot_(robotID);

                // Re-add the task with the new robot ID.
                deployRuntimeCNPRobot_(robotID, taskAdd);

                // Get the key for the adding task.
                entryKeyAdd = generateTaskEntryKey_(taskAdd);

                // Update the overlay.
                gt_delete_db_entry((char *)(entryKeyRemove.c_str()));
                gt_add_passage_point(
                    (char *)(entryKeyAdd.c_str()),
                    taskAdd.position.x,
                    taskAdd.position.y,
                    placeCircleDiameter_*drawingUnitLength_);
            }
            else
            {
                // Remove the old one from the overlay.
                entryKeyRemove = generateTaskEntryKey_(taskRemove);
                gt_delete_db_entry((char *)(entryKeyRemove.c_str()));

                // Add the new one to the overlay.
                entryKeyAdd = generateTaskEntryKey_(taskAdd);
                gt_add_passage_point(
                    (char *)(entryKeyAdd.c_str()),
                    taskAdd.position.x,
                    taskAdd.position.y,
                    placeCircleDiameter_*drawingUnitLength_);
            }

            gEventLogging->log(
                "MMD Renumbering Task #%d to #%d [%s]",
                taskRemove.number,
                taskAdd.number,
                taskAdd.name.c_str());
        }
    }
    else
    {
        // If we delete the last task, revert to the initial message.
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_INITIAL);
    }
  
    // Remove the last task in the array (vector).
    specifiedTaskList_.pop_back();

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function removes a via point from the list.
//-----------------------------------------------------------------------
void MlabMissionDesign::removeViaPoint_(int index)
{
    string entryKey;
    int i, index2, numViaPoints;
    
    // Delete the via point from the entry.
    entryKey = generateViaPointEntryKey_(index);
    gt_delete_db_entry((char *)(entryKey.c_str()));

    numViaPoints = viaPointList_.size() - 1 ;

    if (numViaPoints)
    {
        for (i = index; i < numViaPoints; i++)
        {
            // Copy the one above to this one.
            viaPointList_[i] = viaPointList_[i+1];

            // Make a mark on the overlay.
            entryKey = generateViaPointEntryKey_(i);
            gt_add_passage_point(
                (char *)(entryKey.c_str()),
                viaPointList_[i].x,
                viaPointList_[i].y,
                placeCircleDiameter_*drawingUnitLength_);

            // Delete the one above.
            index2 = i + 1;
            entryKey = generateViaPointEntryKey_(index2);
            gt_delete_db_entry((char *)(entryKey.c_str()));
        }
    }
  
    // Remove the last task in the array (vector).
    viaPointList_.pop_back();

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function lifts a task in order to be dragged.
//-----------------------------------------------------------------------
void MlabMissionDesign::liftTask_(MExpMissionTask_t task)
{
    string entryKey;

    liftedTask_ = task;
    taskLifted_ = true;

    // Delete the task from the entry.
    entryKey = generateTaskEntryKey_(task);
    gt_delete_db_entry((char *)(entryKey.c_str()));

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    // Start dragging a circle, following the pointer.
    draggingCircle_.radius = placeCircleDiameter_*drawingUnitLength_/2.0;
    draggingCircle_.dragging = true;
    dragCircle_();
}

//-----------------------------------------------------------------------
// This function lifts a via point in order to be dragged.
//-----------------------------------------------------------------------
void MlabMissionDesign::liftViaPoint_(int index)
{
    string entryKey;

    liftedViaPointIndex_ = index;
    viaPointLifted_ = true;

    // Delete the via point from the entry.
    entryKey = generateViaPointEntryKey_(index);
    gt_delete_db_entry((char *)(entryKey.c_str()));

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    // Start dragging a circle, following the pointer.
    draggingCircle_.radius = placeCircleDiameter_*drawingUnitLength_/2.0;
    draggingCircle_.dragging = true;
    dragCircle_();
}

//-----------------------------------------------------------------------
// This function puts down the dragged task.
//-----------------------------------------------------------------------
void MlabMissionDesign::putdownLiftedTask_(double x, double y)
{
    string entryKey;
    int robotID, taskNum;
    bool runtimeCNP = false;

    taskLifted_ = false;

    taskNum = liftedTask_.number;
    specifiedTaskList_[taskNum].position.x = x;
    specifiedTaskList_[taskNum].position.y = y;
    global2geographic(
        x,
        y,
        &(specifiedTaskList_[taskNum].geo.latitude),
        &(specifiedTaskList_[taskNum].geo.longitude));
    entryKey = generateTaskEntryKey_(liftedTask_);
    gt_add_passage_point(
        (char *)(entryKey.c_str()),
        x,
        y,
        placeCircleDiameter_*drawingUnitLength_);

    // Check for the runtime-CNP status
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if (runtimeCNP)
    {
        robotID = getDeployedRuntimeCNPRobotIDFromTaskNumber_(taskNum);
        undeployRuntimeCNPRobot_(robotID);
        deployRuntimeCNPRobot_(robotID, specifiedTaskList_[taskNum]);
    }

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    // Finish dragging
    draggingCircle_.dragging = false;
}

//-----------------------------------------------------------------------
// This function puts down the dragged via point.
//-----------------------------------------------------------------------
void MlabMissionDesign::putdownLiftedViaPoint_(double x, double y)
{
    string entryKey;

    viaPointLifted_ = false;

    viaPointList_[liftedViaPointIndex_].x = x;
    viaPointList_[liftedViaPointIndex_].y = y;

    entryKey = generateViaPointEntryKey_(liftedViaPointIndex_);
    gt_add_passage_point(
        (char *)(entryKey.c_str()),
        x,
        y,
        placeCircleDiameter_*drawingUnitLength_);

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    // Finish dragging
    draggingCircle_.dragging = false;
}

//-----------------------------------------------------------------------
// This function generates the entry key for the task used by
// gt_add_passage_point().
//-----------------------------------------------------------------------
string MlabMissionDesign::generateTaskEntryKey_(MExpMissionTask_t task)
{
    DeployedRuntimeCNPRobot_t deployedRobot;
    string entryKey;
    char buf[1024];
    int robotID;
    bool runtimeCNP = false;
    
    // Check for the runtime-CNP status.
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if (missionSpecWizardEnabled())
    {
        if (task.id == icarusTask_)
        {
            if (MMDMode_ == DESIGN_MODE)
            {
                sprintf(buf, " \n       ICARUS Task\n[Right-click for setting]\n");
            }
            else
            {
                sprintf(buf, " \nICARUS Task\n");
            }
        }
        else
        {
            sprintf(buf, "Task #%d [%s]", (task.number+1), task.name.c_str());
        }
    }
    else if (runtimeCNP)
    {
        robotID = getDeployedRuntimeCNPRobotIDFromTaskNumber_(task.number);

        if (task.id == runtimeCNPMissionManagerTask_)
        {
            sprintf(
                buf,
                " \nCNP Mission Manager\n[Robot ID: %d]\n",
                robotID);
        }
        else
        {
            sprintf(buf, "Robot #%d [Robot ID: %d]", task.number, robotID);
        }
    }
    else
    {
        sprintf(buf, "Task #%d [%s]", (task.number+1), task.name.c_str());
    }

    entryKey = (char *)buf;

    return entryKey;
}

//-----------------------------------------------------------------------
// This function generates the entry key for the via point used by
// gt_add_passage_point().
//-----------------------------------------------------------------------
string MlabMissionDesign::generateViaPointEntryKey_(int index)
{
    string entryKey;
    char buf[1024];
    
    sprintf(buf, "Via Point #%d", index);
    entryKey = (char *)buf;

    return entryKey;
}

//-----------------------------------------------------------------------
// This function generates the entry key for the polygon used by
// gt_add_boundary().
//-----------------------------------------------------------------------
string MlabMissionDesign::generatePolygonEntryKey_(MMDPolygon_t poly)
{
    string entryKey = EMPTY_STRING_;
    char buf[1024];

    switch (poly.featureType) {

    case MEXP_FEATURE_GLOBAL:
        sprintf(
            buf,
            "%s #%d",
            defaultGlobalFeatures_[poly.featureId].name.c_str(),
            (poly.number+1));
        entryKey = buf;
        break;

    default:
        entryKey = "Polygon";
        break;
    }

    return entryKey;
}

/*
//-----------------------------------------------------------------------
// This function redraw the task entry by clear them at first, then
// draw them again.
//-----------------------------------------------------------------------
void MlabMissionDesign::refreshTaskEntries(double unitLength) 
{
    eraseTaskEntries();
    drawTaskEntries(unitLength);
}

//-----------------------------------------------------------------------
// This function erases the task entries.
//-----------------------------------------------------------------------
void MlabMissionDesign::eraseTaskEntries(void)
{
    MExpMissionTask_t task;
    string entryKey;
    int i;

    // Delete the tasks from the entry.
    for (i = 0; i < numTasks_; i++)
    {
        task = specifiedTaskList_[i];
        entryKey = generateTaskEntryKey_(task);
        gt_delete_db_entry((char *)(entryKey.c_str()));
    }

    // Erase Via Points as well.
    for (i = 0; i < (int)(viaPointList_.size()); i++)
    {
        entryKey = generateViaPointEntryKey_(i);
        gt_delete_db_entry((char *)(entryKey.c_str()));
    }

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}
*/

//-----------------------------------------------------------------------
// This function draws the task entries
//-----------------------------------------------------------------------
void MlabMissionDesign::drawSpecifiedTaskList_(double unitLength)
{
    MExpMissionTask_t task;
    string entryKey;
    int i;

    drawingUnitLength_ = unitLength;

    // Draw tasks.
    for (i = 0; i < numTasks_; i++)
    {
        task = specifiedTaskList_[i];
        entryKey = generateTaskEntryKey_(task);
        gt_add_passage_point(
            (char *)(entryKey.c_str()),
            task.position.x,
            task.position.y,
            placeCircleDiameter_*drawingUnitLength_);
    } 

    // Draw Via Points as well.
    for (i = 0; i < (int)(viaPointList_.size()); i++)
    {
        entryKey = generateViaPointEntryKey_(i);
        gt_add_passage_point(
            (char *)(entryKey.c_str()),
            viaPointList_[i].x,
            viaPointList_[i].y,
            placeCircleDiameter_*drawingUnitLength_);
    }

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function erases the specified polygons.
//-----------------------------------------------------------------------
void MlabMissionDesign::eraseSpecifiedPolygons_(void)
{
    MMDPolygon_t poly;
    string entryKey = EMPTY_STRING_;
    int i;

    for (i = 0; i < (int)(specifiedPolygonList_.size()); i++)
    {
        poly = specifiedPolygonList_[i];
        entryKey = generatePolygonEntryKey_(poly);
        gt_delete_db_entry((char *)(entryKey.c_str()));
    }

    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);
}

//-----------------------------------------------------------------------
// This function draws the specified polygons.
//-----------------------------------------------------------------------
void MlabMissionDesign::drawSpecifiedPolygons_(void)
{
    MMDPolygon_t poly;
    gt_Point_list *startPt = NULL, *pt1 = NULL, *pt2 = NULL;
    string entryKey = EMPTY_STRING_;
    int i, j;

    for (i = 0; i < (int)(specifiedPolygonList_.size()); i++)
    {
        poly = specifiedPolygonList_[i];

        if ((int)(poly.posList.size()) < 1)
        {
            fprintf(stderr, "Warning: MlabMissionDesign::drawSpecifiedPolygons_(). Inadequate number of points.\n");
            continue;
        }

        startPt = new gt_Point_list;
        startPt->x = poly.posList[0].x;
        startPt->y = poly.posList[0].y;
        startPt->next = NULL;
        pt1 = startPt;

        // Create the link lists.
        for (j = 1; j < (int)(poly.posList.size()); j++)
        {
            pt2 = new gt_Point_list;
            pt2->x = poly.posList[j].x;
            pt2->y = poly.posList[j].y;
            pt2->next = NULL;
            pt1->next = pt2;
            pt1 = pt2;
        }

        if (pt1 != startPt)
        {
            pt2 = new gt_Point_list;
            pt2->x = startPt->x;
            pt2->y = startPt->y;
            pt2->next = NULL;
            pt1->next = pt2;
        
            // Update the overlay.
            entryKey = generatePolygonEntryKey_(poly);
            gt_add_boundary((char *)(entryKey.c_str()), startPt);
        }
    }
}

//-----------------------------------------------------------------------
// This function draws the overlay measures for MlabMissionDesign.
//-----------------------------------------------------------------------
void MlabMissionDesign::drawOverlayMeasures(double unitLength)
{
    drawSpecifiedTaskList_(unitLength);
    drawSpecifiedPolygons_();
}

//-----------------------------------------------------------------------
// This function creates the global preference menu window.
//-----------------------------------------------------------------------
void MlabMissionDesign::createGlobalFeatureMenu_(void)
{
    Widget mainFrame, main, labelFrame, labelBox, label;
    Widget featureFrame, featureBox, valueFrame, polygonFrame, sliderFrame;
    Widget buttonFrame, button;
    vector<Widget> stackedWidget;
    MMDGlobalFeatureMenuCallbackData_t *optionData;
    XmString sliderTitle;
    string anyBtnLabel;
    char buf[1024];
    float value, conversion;
    int i, j, line = 0;
    int screenNumber;
    int posX, posY, displayWidth, displayHeight;
    int leftPos;
    int globalFeatureMenuHeight, globalFeatureMenuWidth, globalFeatureMenuLineHeight;
    int globalFeatureMenuLabelHeight;
    //int numStackedWidget, numGlobalFeatures, numVisibleGlobalFeatures;
    int numStackedWidget, numGlobalFeatures;
    int maxValue, minValue, decimalPoints;
    int numOptions, numOptionRows;
    const int NUM_OPTION_COLUMNS = 2;

    // Setting the height of the window.
    globalFeatureMenuLineHeight = DEFAULT_GLOBALFEATURE_LINEHEIGHT_;
    globalFeatureMenuWidth = DEFAULT_GLOBALFEATURE_WIDTH_;

    // Set the height for the label of each feature.
    globalFeatureMenuLabelHeight = globalFeatureMenuLineHeight+3;
    
    // Count the number of visible global features.
    numGlobalFeatures = defaultGlobalFeatures_.size();
    globalFeatureMenuHeight = 0;

    // Initialize the height with the title height.
    globalFeatureMenuHeight = globalFeatureMenuLabelHeight;

    for (i = 0; i <  numGlobalFeatures; i++)
    {
        if (defaultGlobalFeatures_[i].hide)
        {
            continue;
        }
    
        // Do not show the number of robots if CNP is enabled.
        if ((cnpMode_ > CNP_MODE_DISABLED) &&
            (defaultGlobalFeatures_[i].name == STRING_NUM_ROBOTS_))
        {
            continue;
        }

        switch (defaultGlobalFeatures_[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:
            numOptions = defaultGlobalFeatures_[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
            globalFeatureMenuHeight += 
                (globalFeatureMenuLabelHeight + (numOptionRows*(globalFeatureMenuLineHeight+3)));
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:
        case MEXP_FEATURE_OPTION_POLYGONS:
            globalFeatureMenuHeight +=
                (2*(globalFeatureMenuLineHeight+6));
            break;
        }
    }

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    posX = displayWidth - globalFeatureMenuWidth - DEFAULT_GLOBALFEATURE_XEDGEOFFSET_;
    posY = DEFAULT_GLOBALFEATURE_YPOS_;

    // Popup shell.
    globalFeatureMenu_w_ = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, true,
         XmNx, posX,
         XmNy, posY,
         XmNheight, globalFeatureMenuHeight+20,
         XmNwidth, globalFeatureMenuWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateWidget
        ("mainFrame",
         xmFrameWidgetClass, globalFeatureMenu_w_,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, globalFeatureMenuHeight,
         XmNwidth, globalFeatureMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, globalFeatureMenu_w_,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, globalFeatureMenu_w_,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, globalFeatureMenu_w_,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, globalFeatureMenu_w_,
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
         XmNheight, globalFeatureMenuHeight,
         XmNwidth, globalFeatureMenuWidth,
         NULL);

    stackedWidget.push_back(main);

    // The widget for the main label.
    labelFrame = XtVaCreateWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, globalFeatureMenuLabelHeight,
         XmNwidth, globalFeatureMenuWidth,
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
         XmNheight, globalFeatureMenuLineHeight,
         XmNwidth, globalFeatureMenuWidth,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    label = XtVaCreateWidget
        (GLOBALFEATURE_TITLE_.c_str(),
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, globalFeatureMenuLineHeight,
         XmNwidth, globalFeatureMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);
    line++;

    stackedWidget.push_back(label);

    // It is a 2D array. Create the 1D part (feature) first.
    globalFeatureMenuOption_w_ = new Widget*[numGlobalFeatures];

    // Create the preference raws sequentially.
    for (i = 0; i < numGlobalFeatures; i++)
    {
        if (defaultGlobalFeatures_[i].id != i)
        {
            // Global features was not stored in order of their IDs.
            // Something went wroing. Warn it.
            fprintf(stderr, "Warning (mlab): MlabMissionDesign::createGlobalFeatureMenu_().\n");
            fprintf(stderr, "The order of global features corrupted.\n");
        }

        if (defaultGlobalFeatures_[i].hide)
        {
            continue;
        }

        // Do not show the number of robots if CNP is enabled.
        if ((cnpMode_ > CNP_MODE_DISABLED) &&
            (defaultGlobalFeatures_[i].name == STRING_NUM_ROBOTS_))
        {
            continue;
        }

        switch (defaultGlobalFeatures_[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:

            numOptions = defaultGlobalFeatures_[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);

            globalFeatureMenuHeight = 
                (globalFeatureMenuLabelHeight + (numOptionRows*(globalFeatureMenuLineHeight+3)));

            // The widget for the main label.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 XmNrecomputeSize, false,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // Two-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("globalFeature-featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 //XmNpacking, XmPACK_NONE,
                 XmNorientation, XmHORIZONTAL,
                 //XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNalignment, XmALIGNMENT_CENTER,
                 //XmNnumColumns, NUM_OPTION_COLUMNS,
                 XmNnumColumns, numOptionRows+1,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureFrame,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, featureFrame,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureFrame,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, featureFrame,
                 XmNrecomputeSize, false,
                 NULL);

            stackedWidget.push_back(featureBox);

            // Create the second dimention (option) of this array.
            globalFeatureMenuOption_w_[i] = new Widget[numOptions];

            // The label column.
            label = XtVaCreateWidget
                (defaultGlobalFeatures_[i].name.c_str(),
                 xmLabelGadgetClass, featureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureMenuLabelHeight,
                 XmNwidth, globalFeatureMenuWidth/NUM_OPTION_COLUMNS,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureBox,
                 NULL);

            stackedWidget.push_back(label);

            // Fill the blanks in the label row.
            for (j = 1; j < NUM_OPTION_COLUMNS; j++)
            {
                label = XtVaCreateWidget
                    (" ",
                     xmLabelGadgetClass, featureBox,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, globalFeatureMenuLabelHeight,
                     XmNwidth, globalFeatureMenuWidth/NUM_OPTION_COLUMNS,
                     XmNtopAttachment, XmATTACH_WIDGET,
                     XmNtopWidget, featureBox,
                     NULL);

                stackedWidget.push_back(label);
            }
            
            for (j = 0; j < numOptions; j++)
            {
                optionData = new MMDGlobalFeatureMenuCallbackData_t;
                optionData->globalFeatureId = i;
                optionData->optionValue = (float)j;
                optionData->mlabMissionDesignInstance = this;
                optionData->optionType = defaultGlobalFeatures_[i].optionType;

                if ((int)(defaultGlobalFeatures_[i].selectedOption) == j)
                {
                    value = 1.0;
                }
                else
                {
                    value = 0.0;
                }

                buttonFrame = XtVaCreateWidget
                    ("buttonFrame",
                     xmFormWidgetClass, featureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, globalFeatureMenuLineHeight-3,
                     XmNwidth, globalFeatureMenuWidth/NUM_OPTION_COLUMNS-10,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                // Toggle features.
                globalFeatureMenuOption_w_[i][j] = XtVaCreateWidget
                    (defaultGlobalFeatures_[i].options[j].value.c_str(),
                     xmToggleButtonGadgetClass, buttonFrame,
                     XmNradioBehavior, true,
                     XmNradioAlwaysOne, true,
                     XmNheight, globalFeatureMenuLineHeight-3,
                     XmNwidth, globalFeatureMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNset, (int)value,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNhighlightColor, parentWidgetBg_,
                     NULL);

                stackedWidget.push_back(globalFeatureMenuOption_w_[i][j]);

                XtAddCallback
                    (globalFeatureMenuOption_w_[i][j],
                     XmNvalueChangedCallback,
                     (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                     (void *)optionData);
            }

            if (numOptions%NUM_OPTION_COLUMNS)
            {
                // Fill the space
                buttonFrame = XtVaCreateWidget
                    ("buttonFrame",
                     xmFormWidgetClass, featureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, globalFeatureMenuLineHeight-3,
                     XmNwidth, globalFeatureMenuWidth/NUM_OPTION_COLUMNS-10,
                     NULL);

                stackedWidget.push_back(buttonFrame);
            }

            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:

            globalFeatureMenuHeight = (2*(globalFeatureMenuLineHeight+3));

            // The widget for the main label.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // Two-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("globalFeature-featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, 1,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureFrame,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, featureFrame,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureFrame,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, featureFrame,
                 NULL);

            stackedWidget.push_back(featureBox);

            if (defaultGlobalFeatures_[i].allowDisable)
            {
                numOptions = 2;
                leftPos = 12;
            }
            else
            {
                numOptions = 1;
                leftPos = 8;
            }

            // Create the second dimention (option) of this array.
            globalFeatureMenuOption_w_[i] = new Widget[numOptions];

            sliderFrame = XtVaCreateWidget(
                "sliderFrame",
                xmFormWidgetClass, featureBox,
                XmNheight, 2*globalFeatureMenuLineHeight,
                XmNwidth, globalFeatureMenuWidth,
                XmNborderWidth, 0,
                XmNfractionBase, 20,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNbottomAttachment, XmATTACH_WIDGET,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNrightAttachment, XmATTACH_WIDGET,
                NULL);

            stackedWidget.push_back(sliderFrame);

            switch (defaultGlobalFeatures_[i].optionType) {

            case MEXP_FEATURE_OPTION_SLIDER100:
                sliderTitle = XmStringCreateLtoR
                    ((String)MEXP_DEFAULT_SLIDER100_RANAGE_TITLE.c_str(),
                     XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                minValue = 1;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER10:
                sliderTitle = XmStringCreateLtoR
                    ((String)MEXP_DEFAULT_SLIDER10_RANAGE_TITLE.c_str(),
                     XmSTRING_DEFAULT_CHARSET);
                maxValue = 10;
                minValue = 1;
                decimalPoints = 0;
                conversion = 1.0;
                break;

            case MEXP_FEATURE_OPTION_SLIDER1:
            default:
                sliderTitle = XmStringCreateLtoR
                    ((String)MEXP_DEFAULT_SLIDER1_RANAGE_TITLE.c_str(),
                     XmSTRING_DEFAULT_CHARSET);
                maxValue = 100;
                minValue = 1;
                decimalPoints = 2;
                conversion = 100.0;
                break;
            }


            // The label column.
            label = XtVaCreateWidget
                (defaultGlobalFeatures_[i].name.c_str(),
                 xmLabelGadgetClass, sliderFrame,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 0,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 8,
                 NULL);

            stackedWidget.push_back(label);

            optionData = new MMDGlobalFeatureMenuCallbackData_t;
            optionData->globalFeatureId = i;
            optionData->optionValue = defaultGlobalFeatures_[i].selectedOption;
            optionData->mlabMissionDesignInstance = this;
            optionData->optionType = defaultGlobalFeatures_[i].optionType;

            value = optionData->optionValue*conversion;
            value = (value >= minValue)? value : minValue;

            // Slider bar.
            globalFeatureMenuOption_w_[i][0] = XtVaCreateWidget
                ("",
                 xmScaleWidgetClass, sliderFrame,
                 XmNmaximum, maxValue,
                 XmNminimum, minValue,
                 XmNvalue, (int)value,
                 XmNorientation, XmHORIZONTAL,
                 XmNprocessingDirection, XmMAX_ON_RIGHT,
                 XmNshowValue, true,
                 XmNdecimalPoints, decimalPoints,
                 XmNtitleString, sliderTitle,
                 XmNhighlightColor, parentWidgetBg_,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, leftPos,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 19,
                 NULL);

            XtAddCallback(
                globalFeatureMenuOption_w_[i][0],
                XmNvalueChangedCallback,
                (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                (void *)optionData);

            XmStringFree(sliderTitle);

            if (defaultGlobalFeatures_[i].allowDisable)
            {
                if (defaultGlobalFeatures_[i].selectedOption < 0)
                {
                    // Disabled.
                    anyBtnLabel = STRING_EDIT_.c_str();
                }
                else
                {
                    stackedWidget.push_back(globalFeatureMenuOption_w_[i][0]);
                    anyBtnLabel = STRING_ANY_.c_str();
                }

                buttonFrame = XtVaCreateWidget
                    ("buttonFrame",
                     xmFormWidgetClass, sliderFrame,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_CENTER,
                     XmNtopAttachment, XmATTACH_FORM,
                     XmNbottomAttachment, XmATTACH_FORM,
                     XmNleftAttachment, XmATTACH_POSITION,
                     XmNleftPosition, 8,
                     XmNrightAttachment, XmATTACH_POSITION,
                     XmNrightPosition, 11,
                     XmNverticalSpacing, globalFeatureMenuLineHeight/3,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                globalFeatureMenuOption_w_[i][1] = XtVaCreateWidget
                    (anyBtnLabel.c_str(),
                     xmPushButtonWidgetClass, buttonFrame,
                     XmNalignment, XmALIGNMENT_CENTER,
                     XmNtopAttachment, XmATTACH_FORM,
                     XmNbottomAttachment, XmATTACH_FORM,
                     XmNleftAttachment, XmATTACH_FORM,
                     XmNrightAttachment, XmATTACH_FORM,
                     XmNhighlightOnEnter, false,
                     XmNtraversalOn, false,
                     NULL);

                stackedWidget.push_back(globalFeatureMenuOption_w_[i][1]);

                optionData = new MMDGlobalFeatureMenuCallbackData_t;
                optionData->globalFeatureId = i;
                optionData->optionValue = defaultGlobalFeatures_[i].selectedOption;
                optionData->mlabMissionDesignInstance = this;
                optionData->optionType = defaultGlobalFeatures_[i].optionType;

                XtAddCallback
                    (globalFeatureMenuOption_w_[i][1],
                     XmNactivateCallback,
                     (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                     (void *)optionData);
            }
            else
            {
                stackedWidget.push_back(globalFeatureMenuOption_w_[i][0]);
            }
            break;

        case MEXP_FEATURE_OPTION_POLYGONS:

            globalFeatureMenuHeight = (2*(globalFeatureMenuLineHeight+3));

            // The widget for the main label.
            featureFrame = XtVaCreateWidget
                ("featureFrame",
                 xmFrameWidgetClass, main,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 NULL);

            stackedWidget.push_back(featureFrame);

            // Two-colum widget for the feature.
            featureBox = XtVaCreateWidget
                ("globalFeature-featureBox",
                 xmRowColumnWidgetClass, featureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNnumColumns, 1,
                 XmNheight, globalFeatureMenuHeight,
                 XmNwidth, globalFeatureMenuWidth,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, featureFrame,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, featureFrame,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, featureFrame,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, featureFrame,
                 NULL);

            stackedWidget.push_back(featureBox);

            // Create the second dimention (option) of this array.
            globalFeatureMenuOption_w_[i] = new Widget[1];

            polygonFrame = XtVaCreateWidget(
                "polygonFrame",
                xmFormWidgetClass, featureBox,
                XmNheight, 2*globalFeatureMenuLineHeight,
                XmNwidth, globalFeatureMenuWidth,
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
                (defaultGlobalFeatures_[i].name.c_str(),
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

            valueFrame = XtVaCreateWidget
                ("valueFrame",
                 xmFormWidgetClass, polygonFrame,
                 XmNshadowThickness, 0,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 8,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 10,
                 XmNverticalSpacing, globalFeatureMenuLineHeight/3,
                 NULL);

            stackedWidget.push_back(valueFrame);

            // The number of polygons
            sprintf(buf, "%.0f", defaultGlobalFeatures_[i].selectedOption);
            globalFeatureMenuOption_w_[i][0] = XtVaCreateWidget
                (buf,
                 xmTextWidgetClass, valueFrame,
                 XmNheight, globalFeatureMenuLineHeight,
                 XmNwidth, globalFeatureMenuWidth/4,
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

            stackedWidget.push_back(globalFeatureMenuOption_w_[i][0]);
            
            // Edit button
            buttonFrame = XtVaCreateWidget
                ("buttonFrame",
                 xmFormWidgetClass, polygonFrame,
                 XmNshadowThickness, 0,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 15,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 18,
                 XmNverticalSpacing, globalFeatureMenuLineHeight/3,
                 NULL);

            stackedWidget.push_back(buttonFrame);

            optionData = new MMDGlobalFeatureMenuCallbackData_t;
            optionData->globalFeatureId = i;
            optionData->optionValue = defaultGlobalFeatures_[i].selectedOption;
            optionData->mlabMissionDesignInstance = this;
            optionData->optionType = defaultGlobalFeatures_[i].optionType;

            button = XtVaCreateWidget
                (STRING_EDIT_.c_str(),
                 xmPushButtonWidgetClass, buttonFrame,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNhighlightOnEnter, false,
                 XmNtraversalOn, false,
                 NULL);

            if (defaultGlobalFeatures_[i].selectedOption > 0)
            {
                XtSetSensitive(button, true);
            }
            else
            {
                XtSetSensitive(button, false);
            }

            stackedWidget.push_back(button);

            XtAddCallback
                (button,
                 XmNactivateCallback,
                 (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                 (void *)optionData);

            // Add button
            buttonFrame = XtVaCreateWidget
                ("buttonFrame",
                 xmFormWidgetClass, polygonFrame,
                 XmNshadowThickness, 0,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 11,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 14,
                 XmNverticalSpacing, globalFeatureMenuLineHeight/3,
                 NULL);

            stackedWidget.push_back(buttonFrame);

            optionData = new MMDGlobalFeatureMenuCallbackData_t;
            optionData->globalFeatureId = i;
            optionData->optionValue = defaultGlobalFeatures_[i].selectedOption;
            optionData->mlabMissionDesignInstance = this;
            optionData->optionType = defaultGlobalFeatures_[i].optionType;
            optionData->extraDataList.push_back((void *)button); // Save "Edit" button.

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

            stackedWidget.push_back(button);

            XtAddCallback
                (button,
                 XmNactivateCallback,
                 (XtCallbackProc)cbChangeGlobalFeatureMenuValue_,
                 (void *)optionData);
            break;

        default:
            break;
        }

        line += 2;
    }

    numStackedWidget = stackedWidget.size();

    XtPopup (globalFeatureMenu_w_, XtGrabNone);

    for (i = 0; i < numStackedWidget; i++)
    {
        //XtManageChild(stackedWidget[i]);
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
    }
}

//-----------------------------------------------------------------------
// This function decode the polygon string with a format of:
// <pt1x,pt1y>[pt1lat,pt1lon];<pt2x,pt2y>[pt2lat,pt2lon];
//-----------------------------------------------------------------------
bool MlabMissionDesign::decodePolygon_(string polygonString, MMDPolygon_t *poly)
{
    MExpPoint2D_t pos;
    MExpLatLon_t geo;
    string bufString;
    string::size_type index;
    double x, y, lat, lon;

    poly->posList.clear();
    poly->geoList.clear();

    while (true)
    {
        // Find "<"
        index = polygonString.find(POLYGON_ENCODER_KEY_XY_START_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_XY_START_ not found.\n");
            return false;
        }

        polygonString = polygonString.substr(index+1, string::npos);

        // Find ","
        index = polygonString.find(POLYGON_ENCODER_KEY_POINT_SEPARATOR_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_POINT_SEPARATOR_ not found.\n");
            return false;
        }

        // Get x
        bufString = polygonString.substr(0, index);
        x = atof(bufString.c_str());

        polygonString = polygonString.substr(index+1, string::npos);

        // Find ">"
        index = polygonString.find(POLYGON_ENCODER_KEY_XY_END_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_XY_END_ not found.\n");
            return false;
        }

        // Get y
        bufString = polygonString.substr(0, index);
        y = atof(bufString.c_str());

        polygonString = polygonString.substr(index+1, string::npos);

        // Find "["
        index = polygonString.find(POLYGON_ENCODER_KEY_LATLON_START_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_LATLON_START_ not found.\n");
            return false;
        }

        polygonString = polygonString.substr(index+1, string::npos);

        // Find ","
        index = polygonString.find(POLYGON_ENCODER_KEY_POINT_SEPARATOR_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_POINT_SEPARATOR_ (2) not found.\n");
            return false;
        }

        // Get lat
        bufString = polygonString.substr(0, index);
        lat = atof(bufString.c_str());

        polygonString = polygonString.substr(index+1, string::npos);

        // Find "]"
        index = polygonString.find(POLYGON_ENCODER_KEY_LATLON_END_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_LATLON_END_ not found.\n");
            return false;
        }

        // Get lon
        bufString = polygonString.substr(0, index);
        lon = atof(bufString.c_str());

        polygonString = polygonString.substr(index+1, string::npos);

        // Find ";"
        index = polygonString.find(POLYGON_ENCODER_KEY_POINTS_SEPARATOR_);

        if (index == string::npos)
        {
            fprintf(
                stderr,
                "Error:  MlabMissionDesign::decodePolygon_(). POLYGON_ENCODER_KEY_POINTS_SEPARATOR_ not found.\n");
            return false;
        }

        // Save the point.
        pos.x = x;
        pos.y = y;
        poly->posList.push_back(pos);
        geo.latitude = lat;
        geo.longitude = lon;
        poly->geoList.push_back(geo);

        if (polygonString.size() <= 1)
        {
            // Done.
            break;
        }

        polygonString = polygonString.substr(index+1, string::npos);
    }

    return true;
}

//-----------------------------------------------------------------------
// This function encodes the polygon into a string with a format of:
// <pt1x,pt1y>[pt1lat,pt1lon];<pt2x,pt2y>[pt2lat,pt2lon];
//-----------------------------------------------------------------------
string MlabMissionDesign::encodePolygon_(MMDPolygon_t poly)
{
    string value = EMPTY_STRING_;
    char buf[1024];
    int i;

    if (poly.posList.size() != poly.geoList.size())
    {
        fprintf(stderr, "Error: MlabMissionDesign::encodePolygon_Points(). Corrupted polygon data.\n");
        return value;
    }

    for (i = 0; i < (int)(poly.posList.size()); i++)
    {
        sprintf(
            buf,
            "%c%5.2f%c%5.2f%c%c%5.6f%c%5.6f%c%c",
            POLYGON_ENCODER_KEY_XY_START_,
            poly.posList[i].x,
            POLYGON_ENCODER_KEY_POINT_SEPARATOR_,
            poly.posList[i].y,
            POLYGON_ENCODER_KEY_XY_END_,
            POLYGON_ENCODER_KEY_LATLON_START_,
            poly.geoList[i].latitude,
            POLYGON_ENCODER_KEY_POINT_SEPARATOR_,
            poly.geoList[i].longitude,
            POLYGON_ENCODER_KEY_LATLON_END_,
            POLYGON_ENCODER_KEY_POINTS_SEPARATOR_);

        value += buf;
    }

    return value;
}

//-----------------------------------------------------------------------
// This function clears the specifying polygon info.
//-----------------------------------------------------------------------
void MlabMissionDesign::clearPolygon_(MMDPolygon_t *poly)
{
    if (poly == NULL)
    {
        fprintf(stderr, "Warning: MlabMissionDesign::clearPolygon_(). Polygon is NULL.\n");
        return;
    }

    poly->posList.clear();
    poly->geoList.clear();
    memset(poly, 0x0, sizeof(MMDPolygon_t));
}

//-----------------------------------------------------------------------
// This function asks the user to specify a polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::specifyPolygon_(
    Widget callerWidget,
    Widget editerWidget,
    int featureType,
    int featureId,
    int number)
{
    if (rightBtnMenuIsUp_)
    {
        closeRightBtnMenu_();
    }

    if (robotConstraintMenuIsUp_)
    {
        closeRobotConstraintMenu_();
    }

    if (isSpecifyingPolygon_)
    {
        cancelSpecifyingPolygon_();
    }

    if (isEditingPolygon_)
    {
        finishEditingPolygon_();
    }

    XtSetSensitive(callerWidget, false);

    clearPolygon_(&specifyingPolygon_);
    specifyingPolygon_.callerWidget = callerWidget;
    specifyingPolygon_.editerWidget = editerWidget;
    specifyingPolygon_.featureType = featureType;
    specifyingPolygon_.featureId = featureId;
    specifyingPolygon_.number = number;
    isSpecifyingPolygon_ = true;

    XDefineCursor(
        XtDisplay(parentWidget_),
        XtWindow(parentWidget_),
        getCursorFromList_(CURSOR_POLYGON));
}

//-----------------------------------------------------------------------
// This function asks the user to edit a polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::editPolygon_(
    Widget callerWidget,
    int featureType,
    int featureId,
    int number)
{
    XmString label;

    if (rightBtnMenuIsUp_)
    {
        closeRightBtnMenu_();
    }

    if (robotConstraintMenuIsUp_)
    {
        closeRobotConstraintMenu_();
    }

    if (isSpecifyingPolygon_)
    {
        cancelSpecifyingPolygon_();
    }

    if (isEditingPolygon_)
    {
        finishEditingPolygon_();
    }

    label = XmStringCreateLtoR
        ((String)STRING_DONE_.c_str(),
         XmSTRING_DEFAULT_CHARSET);

    XtVaSetValues(
        callerWidget,
        XmNlabelString, label,
        XmNbackground, gColorPixel.yellow,
        NULL);
    XmStringFree(label);

    isEditingPolygon_ = true;
    currentEditPolygonButton_ = callerWidget;
    resetEditingPolygons_(featureType, featureId);

    XDefineCursor(
        XtDisplay(parentWidget_),
        XtWindow(parentWidget_),
        getCursorFromList_(CURSOR_HAND2));
}

//-----------------------------------------------------------------------
// This function resets the editing polygons.
//-----------------------------------------------------------------------
void MlabMissionDesign::resetEditingPolygons_(int featureType, int featureId)
{
    MExpPoint2D_t pos;
    MMDPolygon_t poly;
    double radius;
    int i, j;

    editingPolygonList_.clear();
    radius = placePolygonPointDiameter_*drawingUnitLength_/2.0;

    for (i = 0; i < (int)(specifiedPolygonList_.size()); i++)
    {
        poly = specifiedPolygonList_[i];

        if ((poly.featureType == featureType) &&
            (poly.featureId == featureId))
        {

            for (j = 0; j < (int)(poly.posList.size()); j++)
            {
                pos = poly.posList[j];
                DrawCircle(pos.x, pos.y, radius, gGCs.XOR);
                DrawCircle(pos.x, pos.y, radius, gGCs.whiteXOR);
            }

            editingPolygonList_.push_back(poly);
        }
    }

    if ((int)(editingPolygonList_.size()) == 0)
    {
        finishEditingPolygon_();
    }
}

//-----------------------------------------------------------------------
// This function completes the specifying polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::completeSpecifyingPolygon_(void)
{
    MExpPoint2D_t startPos, pos1, pos2;
    MExpFeatureOption_t featureOption;
    gt_Point_list *startPt = NULL, *pt1 = NULL, *pt2 = NULL;
    string entryKey = EMPTY_STRING_;
    char buf[1024];
    int i, id;

    specifyingPolygon_.dragging = false;
    isSpecifyingPolygon_ = false;

    // Wait until the dragPolygon_() finishes.
    while (specifyingPolygon_.drawn)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    // Restore the cursor.
    XDefineCursor(
        XtDisplay(parentWidget_),
        XtWindow(parentWidget_),
        getCursorFromList_(CURSOR_LEFT_PTR));

    // Restore the "Add" button.
    XtSetSensitive(specifyingPolygon_.callerWidget, true);

    if ((int)(specifyingPolygon_.posList.size()) < 1)
    {
        clearPolygon_(&specifyingPolygon_);
        return;
    }

    startPos = specifyingPolygon_.posList[0];
    pos1 = startPos;
    startPt = new gt_Point_list;
    startPt->x = startPos.x;
    startPt->y = startPos.y;
    startPt->next = NULL;
    pt1 = startPt;

    // Erase the temp lines and create the link lists.
    for (i = 1; i < (int)(specifyingPolygon_.posList.size()); i++)
    {
        pos2 = specifyingPolygon_.posList[i];

        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.XOR);
        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.whiteXOR);

        pt2 = new gt_Point_list;
        pt2->x = pos2.x;
        pt2->y = pos2.y;
        pt2->next = NULL;
        pt1->next = pt2;

        pos1 = pos2;
        pt1 = pt2;
    }

    // Refresh the screen.
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    if (pt1 != startPt)
    {
        pt2 = new gt_Point_list;
        pt2->x = startPt->x;
        pt2->y = startPt->y;
        pt2->next = NULL;
        pt1->next = pt2;
        
        switch (specifyingPolygon_.featureType) {

        case MEXP_FEATURE_GLOBAL:
            id = specifyingPolygon_.featureId;

            // Encode the polygon points in a string and save it in
            // the feature option value.
            featureOption.value = encodePolygon_(specifyingPolygon_);

            if ((defaultGlobalFeatures_[id].selectedOption == 0) &&
                (defaultGlobalFeatures_[id].options.size() > 0))
            {
                defaultGlobalFeatures_[id].options.clear();
            }

            defaultGlobalFeatures_[id].options.push_back(featureOption);

            // Selected option is the number of the polygon.
            defaultGlobalFeatures_[id].selectedOption =
                (double)defaultGlobalFeatures_[id].options.size();

            // Update the number of the polygons in the widget.
            sprintf(buf, "%d", defaultGlobalFeatures_[id].options.size());
            XtVaSetValues(
                globalFeatureMenuOption_w_[id][0],
                XmNvalue, buf,
                NULL);

            // Check to see if "Edit" button can be sensitive.
            if ((defaultGlobalFeatures_[id].options.size()) > 0)
            {
                XtSetSensitive(specifyingPolygon_.editerWidget, true);
            }
            else
            {
                XtSetSensitive(specifyingPolygon_.editerWidget, false);
            }

            XFlush(display_);

            break;
        }

        // Update the overlay.
        entryKey = generatePolygonEntryKey_(specifyingPolygon_);
        gt_add_boundary((char *)(entryKey.c_str()), startPt);

        // Add to the specified polygon list.
        specifiedPolygonList_.push_back(specifyingPolygon_);
    }

    clearPolygon_(&specifyingPolygon_);
}

//-----------------------------------------------------------------------
// This function cancels the specifying polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::cancelSpecifyingPolygon_(void)
{
    MExpPoint2D_t startPos, pos1, pos2;
    int i;

    specifyingPolygon_.dragging = false;
    isSpecifyingPolygon_ = false;

    while (specifyingPolygon_.drawn)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    XDefineCursor(
        XtDisplay(parentWidget_),
        XtWindow(parentWidget_),
        getCursorFromList_(CURSOR_LEFT_PTR));

    XtSetSensitive(specifyingPolygon_.callerWidget, true);

    if ((int)(specifyingPolygon_.posList.size()) < 1)
    {
        clearPolygon_(&specifyingPolygon_);
        return;
    }

    startPos = specifyingPolygon_.posList[0];
    pos1 = startPos;

    for (i = 1; i < (int)(specifyingPolygon_.posList.size()); i++)
    {
        pos2 = specifyingPolygon_.posList[i];

        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.XOR);
        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.whiteXOR);

        pos1 = pos2;
    }

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    clearPolygon_(&specifyingPolygon_);
}

//-----------------------------------------------------------------------
// This function cancels the editing polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::finishEditingPolygon_(void)
{
    XmString label;

    specifyingPolygon_.dragging = false;
    isEditingPolygon_ = false;

    while (specifyingPolygon_.drawn)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    if (currentEditPolygonButton_ != NULL)
    {
        label = XmStringCreateLtoR
            ((String)STRING_EDIT_.c_str(),
             XmSTRING_DEFAULT_CHARSET);

        XtVaSetValues(
            currentEditPolygonButton_,
            XmNlabelString, label,
            XmNbackground, parentWidgetBg_,
            NULL);
        XmStringFree(label);

        if (editingPolygonList_.size() == 0)
        {
            XtSetSensitive(currentEditPolygonButton_, false);
        }
        else
        {
            XtSetSensitive(currentEditPolygonButton_, true);
        }
    }

    currentEditPolygonButton_ = NULL;

    XDefineCursor(
        XtDisplay(parentWidget_),
        XtWindow(parentWidget_),
        getCursorFromList_(CURSOR_LEFT_PTR));

    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    clearPolygon_(&specifyingPolygon_);
    editingPolygonList_.clear();
}

//-----------------------------------------------------------------------
// This function changes the global preference menu value.
//-----------------------------------------------------------------------
void MlabMissionDesign::changeGlobalFeatureMenuValue_(Widget w, XtPointer client_data)
{
    MMDGlobalFeatureMenuCallbackData_t *data = NULL;
    XmString xmBuf;
    string buttonLabel;
    int i, j;
    bool selected;
    int sliderBarValue;
    float optionValue;
    char optionValueStr[256];
    char *buf = NULL;

    data = (MMDGlobalFeatureMenuCallbackData_t *)client_data;
    i = data->globalFeatureId;

    if (defaultGlobalFeatures_[i].id != i)
    {
        // Global features was not stored in order of their IDs.
        // Something went wrong. Warn it.
        fprintf(stderr, "Warning (mlab): MlabMissionDesign::changeGlobalFeatureMenuValue_().\n");
        fprintf(stderr, "The order of global features corrupted.\n");
        return;
    }

    switch (cnpMode_) {

    case CNP_MODE_PREMISSION:
        if (i == globalFeatureRobotNumIndex_)
        {
            warn_userf("CNP is enabled - the number of robots cannot be changed.");
            XtDestroyWidget(globalFeatureMenu_w_);
            createGlobalFeatureMenu_();
            return;
        }
        break;
    }

    switch (data->optionType) {
        
    case MEXP_FEATURE_OPTION_SLIDER100:
    case MEXP_FEATURE_OPTION_SLIDER10:
    case MEXP_FEATURE_OPTION_SLIDER1:

        XtVaGetValues(
            globalFeatureMenuOption_w_[i][0],
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

        if (w == globalFeatureMenuOption_w_[i][0])
        {
            defaultGlobalFeatures_[i].selectedOption = optionValue;
            defaultGlobalFeatures_[i].options[0].value = optionValueStr;

            gEventLogging->log(
                "MMD GlobalFeatureMenu %s %s slider changed",
                defaultGlobalFeatures_[i].name.c_str(),
                defaultGlobalFeatures_[i].options[0].value.c_str());
        }
        else if (w == globalFeatureMenuOption_w_[i][1])
        {
            // "Any" (or "Edit") button pressed.
            XtVaGetValues(
                w,
                XmNlabelString, &xmBuf,
                NULL);
            XmStringGetLtoR(
                xmBuf, 
                XmSTRING_DEFAULT_CHARSET,
                &buf);
            XmStringFree(xmBuf);

            buttonLabel = buf;

            if (buttonLabel == STRING_ANY_)
            {
                xmBuf = XmStringCreateLocalized((char *)(STRING_EDIT_.c_str()));

                XtVaSetValues(
                    w,
                    XmNlabelString, xmBuf,
                    NULL);

                XmStringFree(xmBuf);

                defaultGlobalFeatures_[i].selectedOption = MEXP_DISABLED_OPTION_VALUE;
                defaultGlobalFeatures_[i].options[0].value = MEXP_STRING_DISABLED;
                XtUnmanageChild(globalFeatureMenuOption_w_[i][0]);
            }
            else if (buttonLabel == STRING_EDIT_)
            {
                xmBuf = XmStringCreateLocalized((char *)(STRING_ANY_.c_str()));

                XtVaSetValues(
                    w,
                    XmNlabelString, xmBuf,
                    NULL);

                XmStringFree(xmBuf);

                defaultGlobalFeatures_[i].selectedOption = optionValue;
                defaultGlobalFeatures_[i].options[0].value = optionValueStr;
                XtManageChild(globalFeatureMenuOption_w_[i][0]);
            }
        }
        break;

    case MEXP_FEATURE_OPTION_TOGGLE:

        defaultGlobalFeatures_[i].selectedOption = data->optionValue;

        for (j = 0; j < ((int)(defaultGlobalFeatures_[i].options.size())); j++)
        {
            if ((int)(defaultGlobalFeatures_[i].selectedOption) == j)
            {
                selected = true;

                gEventLogging->log(
                    "MMD GlobalFeatureMenu %s %s toggled",
                    defaultGlobalFeatures_[i].name.c_str(),
                    defaultGlobalFeatures_[i].options[j].value.c_str());
            }
            else
            {
                selected = false;
            }

            XtVaSetValues(
                globalFeatureMenuOption_w_[i][j],
                XmNset, selected,
                NULL);
        }

        break;

    case MEXP_FEATURE_OPTION_POLYGONS:
        XtVaGetValues(
            w,
            XmNlabelString, &xmBuf,
            NULL);
        XmStringGetLtoR(
            xmBuf, 
            XmSTRING_DEFAULT_CHARSET,
            &buf);
        XmStringFree(xmBuf);

        buttonLabel = buf;

        if (buttonLabel == STRING_ADD_)
        {
            specifyPolygon_(
                w,
                (Widget)(data->extraDataList.back()),
                MEXP_FEATURE_GLOBAL,
                data->globalFeatureId,
                (int)(defaultGlobalFeatures_[i].selectedOption));
        }
        else if (buttonLabel == STRING_EDIT_)
        {
            editPolygon_(
                w,
                MEXP_FEATURE_GLOBAL,
                data->globalFeatureId,
                (int)(defaultGlobalFeatures_[i].selectedOption));
        }
        else if (buttonLabel == STRING_DONE_)
        {
            finishEditingPolygon_();
        }
        else
        {
            fprintf(
                stderr,
                "Warning: MlabMissionDesign::changeGlobalFeatureMenuValue_(). Unknown buttonLabel.\n");
        }
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------
// This routine creates a widget that contains list of features for the
// specified task. The user are allowed to change the values of the
// features through this widget.
//-----------------------------------------------------------------------
void MlabMissionDesign::createRightBtnMenu_(
    MExpMissionTask_t task,
    int startFeatureId,
    int page)
{
    XmString sliderTitle;
    Widget mainFrame, main, labelFrame, labelBox, label;
    Widget localFeatureFrame, localFeatureBox, buttonFrame;
    Widget applyFrame, apply, cancelFrame, cancel;
    Widget actionFrame, action;
    Widget prevFrame, prevBtn, nextFrame, nextBtn;
    Widget navigationFrame, navigation;
    Widget sliderFrame;
    vector<Widget> stackedWidget;
    vector<int> unhiddenFeatureIDs;
    MMDRightBtnMenuCallbackData_t *optionData;
    MMDNavRightBtnMenuCallbackData_t *navData;
    char title[1024];
    float value, conversion;
    int index, i, j, numOptions, line = 0;
    int screenNumber;
    int posX, posY, displayWidth, maxPos, displayHeight, offset;
    int rightBtnMenuHeight, rightBtnMenuWidth, rightBtnMenuLineHeight;
    int actionSpaceHeight, localFeatureHeight, localFeatureHeightTotal;
    int featureLabelHeight;
    int numStackedWidget, numVisibleLocalFeatures, numOptionRows, numLocalFeatures;
    int maxValue, decimalPoints;
    int unhiddenFeatureIDIndex = 0;
    const int NUM_OPTION_COLUMNS = 2;
    bool featurePrevBtnIsSensitive = false;
    bool featureNextBtnIsSensitive = false;
    bool runtimeCNP = false;

    // Check for the runtime-CNP status.
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    // Setting the height of the window.
    rightBtnMenuLineHeight = DEFAULT_RIGHTBTN_LINEHEIGHT_;

    // Set the action space (Apply & Cancel + Navigation) height.
    actionSpaceHeight = 5*(rightBtnMenuLineHeight+3);

    // Set the height for the label of each feature.
    featureLabelHeight = rightBtnMenuLineHeight+3;
    
    // Compute the total height of the features.
    numLocalFeatures = task.localFeatures.size();
    localFeatureHeightTotal = 0;
    numVisibleLocalFeatures = 0;
    
    // Preprocessed the feature info.
    for (i = 0; i < numLocalFeatures; i++)
    {
        if (task.localFeatures[i].hide)
        {
            if (i == startFeatureId)
            {
                startFeatureId++;
            }

            continue;
        }

        unhiddenFeatureIDs.push_back(i);

        if (i < startFeatureId)
        {
            featurePrevBtnIsSensitive = true;
            continue;
        }

        if (i == startFeatureId)
        {
            unhiddenFeatureIDIndex = (unhiddenFeatureIDs.size()) - 1;
        }

        numVisibleLocalFeatures++;

        if (numVisibleLocalFeatures > MAX_LOCAL_FEATURES_DISPLAY_)
        {
            numVisibleLocalFeatures = MAX_LOCAL_FEATURES_DISPLAY_;
            featureNextBtnIsSensitive = true;
            continue;
        }

        switch (task.localFeatures[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:
            numOptions = task.localFeatures[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
            localFeatureHeightTotal += 
                (featureLabelHeight + (numOptionRows*(rightBtnMenuLineHeight+3)));
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:
        case MEXP_FEATURE_OPTION_POLYGONS:
            localFeatureHeightTotal += (2*(rightBtnMenuLineHeight+3));
            break;
        }
    }
    
    // Finally, set the height of the window.
    rightBtnMenuHeight = 
        featureLabelHeight +
        localFeatureHeightTotal +
        actionSpaceHeight +
        DEFAULT_RIGHTBTN_EXTRASPACE_HEIGHT_;

    // Set other lengths.
    rightBtnMenuWidth = DEFAULT_RIGHTBTN_WIDTH_;
    offset = DEFAULT_RIGHTBTN_EDGEOFFSET_;

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    maxPos = displayWidth - rightBtnMenuWidth - offset;
    //posX = (lastMouseXRoot_ < maxPos)? lastMouseXRoot_ : maxPos;
    posX = (displayWidth - rightBtnMenuWidth)/2;
    posX = (posX < maxPos)? posX : maxPos;
    maxPos = displayHeight - rightBtnMenuHeight - offset;
    //posY = (lastMouseYRoot_ < maxPos)? lastMouseYRoot_ : maxPos;
    posY = (displayHeight - rightBtnMenuHeight)/2;
    posY = (posY < maxPos)? posY : maxPos;
    
    // Popup shell.
    rightBtnMenu_w_ = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, false,
         XmNx, posX,
         XmNy, posY,
         XmNheight, rightBtnMenuHeight,
         XmNwidth, rightBtnMenuWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateManagedWidget
        ("mainFrame",
         xmFrameWidgetClass, rightBtnMenu_w_,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, rightBtnMenuHeight,
         XmNwidth, rightBtnMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, rightBtnMenu_w_,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, rightBtnMenu_w_,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, rightBtnMenu_w_,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, rightBtnMenu_w_,
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
         XmNheight, rightBtnMenuHeight,
         XmNwidth, rightBtnMenuWidth,
         NULL);

    stackedWidget.push_back(main);

    // The frame for the label.
    labelFrame = XtVaCreateManagedWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*rightBtnMenuLineHeight-3,
         XmNwidth, rightBtnMenuWidth,
         NULL);

    stackedWidget.push_back(labelFrame);

    // The widget for the main label.
    labelBox = XtVaCreateManagedWidget
        ("labelBox",
         xmRowColumnWidgetClass, labelFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 1,
         XmNheight, 2*rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    if (missionSpecWizardEnabled() && (task.id == icarusTask_))
    {
        sprintf(title, RIGHTBTN_TITLE_LABEL_ICARUS_TASK_.c_str());
    }
    else if (runtimeCNP && (task.id == runtimeCNPMissionManagerTask_))
    {
        sprintf(title, RIGHTBTN_TITLE_LABEL_RUNTIME_CNP_.c_str());
    }
    else
    {
        sprintf(title, RIGHTBTN_TITLE_LABEL_.c_str());
    }

    label = XtVaCreateManagedWidget
        (title,
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    line++;

    if ((runtimeCNP && (task.id == runtimeCNPMissionManagerTask_)) ||
        (missionSpecWizardEnabled() && (task.id == icarusTask_)))
    {
        sprintf(title, "Mission Setting [Page %d]", page);
    }
    else
    {
        sprintf(
            title,
            "Task #%d - %s [Page %d]",
            (task.number+1),
            task.name.c_str(),
            page);
    }

    label = XtVaCreateManagedWidget
        (title,
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, label,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    line++;

    // It is a 2D array. Create the 1D part (feature) first.
    //rightBtnMenuOption_w_ = new Widget*[numVisibleLocalFeatures];
    rightBtnMenuOption_w_ = new Widget*[numLocalFeatures];

    // Create the feature raws sequentially.
    numVisibleLocalFeatures = 0;

    for (i = 0; i < numLocalFeatures; i++)
    {
        if (task.localFeatures[i].hide)
        {
            continue;
        }

        if (i < startFeatureId)
        {
            continue;
        }

        numVisibleLocalFeatures++;

        if (numVisibleLocalFeatures > MAX_LOCAL_FEATURES_DISPLAY_)
        {
            numVisibleLocalFeatures = MAX_LOCAL_FEATURES_DISPLAY_;
            break;
        }

        switch (task.localFeatures[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:

            numOptions = task.localFeatures[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);

            // Height = label + options
            localFeatureHeight = 
                (featureLabelHeight + (numOptionRows*(rightBtnMenuLineHeight+3)));

            // The frame for the feature.
            localFeatureFrame = XtVaCreateManagedWidget
                ("localFeatureFrame",
                 xmFrameWidgetClass, main,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, localFeatureHeight,
                 XmNwidth, rightBtnMenuWidth,
                 NULL);

            stackedWidget.push_back(localFeatureFrame);

            // N-colum widget for the feature.
            localFeatureBox = XtVaCreateManagedWidget
                ("rightBtnMenu-localFeatureBox",
                 xmRowColumnWidgetClass, localFeatureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 //XmNpacking, XmPACK_NONE,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_CENTER,
                 //XmNnumColumns, numOptions,
                 //XmNnumColumns, NUM_OPTION_COLUMNS,
                 XmNnumColumns, numOptionRows+1,
                 XmNheight, localFeatureHeight,
                 XmNwidth, rightBtnMenuWidth,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, localFeatureFrame,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, localFeatureFrame,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, localFeatureFrame,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, localFeatureFrame,
                 XmNrecomputeSize, false,
                 NULL);

            stackedWidget.push_back(localFeatureBox);

            // Create the second dimention (option) of this array.
            rightBtnMenuOption_w_[i] = new Widget[numOptions];

            // The label column.
            label = XtVaCreateManagedWidget
                (task.localFeatures[i].name.c_str(),
                 xmLabelGadgetClass, localFeatureBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, rightBtnMenuLineHeight,
                 XmNwidth, rightBtnMenuWidth/NUM_OPTION_COLUMNS,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, localFeatureBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, localFeatureBox,
                 NULL);

            stackedWidget.push_back(label);
            line++;

            // Fill the blanks in the label row.
            for (j = 1; j < NUM_OPTION_COLUMNS; j++)
            {
                label = XtVaCreateManagedWidget
                    (" ",
                     xmLabelGadgetClass, localFeatureBox,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, rightBtnMenuLineHeight,
                     XmNwidth, rightBtnMenuWidth/NUM_OPTION_COLUMNS,
                     XmNtopAttachment, XmATTACH_WIDGET,
                     XmNtopWidget, localFeatureBox,
                     NULL);

                stackedWidget.push_back(label);
            }
        
            for (j = 0; j < numOptions; j++)
            {
                optionData = new MMDRightBtnMenuCallbackData_t;
                optionData->featureId = i;
                optionData->optionValue = (float)j;
                optionData->mlabMissionDesignInstance = this;
                optionData->optionType = task.localFeatures[i].optionType;

                if ((int)(task.localFeatures[i].selectedOption) == j)
                {
                    value = 1.0;
                }
                else
                {
                    value = 0.0;
                }

                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, localFeatureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, rightBtnMenuLineHeight-3,
                     XmNwidth, rightBtnMenuWidth/NUM_OPTION_COLUMNS-10,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                // Toggle features.
                rightBtnMenuOption_w_[i][j] = XtVaCreateManagedWidget
                    (task.localFeatures[i].options[j].value.c_str(),
                     xmToggleButtonGadgetClass, buttonFrame,
                     XmNradioBehavior, true,
                     XmNradioAlwaysOne, true,
                     XmNheight, rightBtnMenuLineHeight-3,
                     XmNwidth, rightBtnMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNset, (int)value,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNhighlightColor, parentWidgetBg_,
                     NULL);

                stackedWidget.push_back(rightBtnMenuOption_w_[i][j]);

                XtAddCallback
                    (rightBtnMenuOption_w_[i][j],
                     XmNvalueChangedCallback,
                     (XtCallbackProc)cbChangeRightBtnMenuValue_,
                     (void *)optionData);
            }

            if (numOptions%NUM_OPTION_COLUMNS)
            {
                // Fill the space
                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, localFeatureBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, rightBtnMenuLineHeight-3,
                     XmNwidth, rightBtnMenuWidth/NUM_OPTION_COLUMNS-10,
                     NULL);

                stackedWidget.push_back(buttonFrame);
            }

            /*
              numOptions = task.localFeatures[i].options.size();
              numOptionRows = 
              (numOptions/NUM_OPTION_COLUMNS) + 
              ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
            */
            line += numOptionRows;
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:

            localFeatureHeight = 
                (2*(numOptionRows*rightBtnMenuLineHeight+3));

            // The frame for the feature.
            localFeatureFrame = XtVaCreateManagedWidget
                ("localFeatureFrame",
                 xmFrameWidgetClass, main,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, localFeatureHeight,
                 XmNwidth, rightBtnMenuWidth,
                 NULL);

            stackedWidget.push_back(localFeatureFrame);

            // N-colum widget for the feature.
            localFeatureBox = XtVaCreateManagedWidget
                ("rightBtnMenu-localFeatureBox",
                 xmRowColumnWidgetClass, localFeatureFrame,
                 XmNpacking, XmPACK_COLUMN,
                 XmNorientation, XmHORIZONTAL,
                 XmNalignment, XmALIGNMENT_CENTER,
                 //XmNnumColumns, numOptions,
                 XmNnumColumns, 1,
                 XmNheight, localFeatureHeight,
                 XmNwidth, rightBtnMenuWidth,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, localFeatureFrame,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, localFeatureFrame,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, localFeatureFrame,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, localFeatureFrame,
                 NULL);

            stackedWidget.push_back(localFeatureBox);

            // Create the second dimention (option) of this array.
            rightBtnMenuOption_w_[i] = new Widget[1];

            sliderFrame = XtVaCreateWidget(
                "sliderFrame",
                xmFormWidgetClass, localFeatureBox,
                XmNheight, 2*localFeatureHeight,
                XmNwidth, rightBtnMenuWidth,
                XmNborderWidth, 0,
                XmNfractionBase, 20,
                XmNtopAttachment, XmATTACH_WIDGET,
                XmNbottomAttachment, XmATTACH_WIDGET,
                XmNleftAttachment, XmATTACH_WIDGET,
                XmNrightAttachment, XmATTACH_WIDGET,
                NULL);

            stackedWidget.push_back(sliderFrame);

            switch (task.localFeatures[i].optionType) {

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
            label = XtVaCreateManagedWidget
                (task.localFeatures[i].name.c_str(),
                 xmLabelGadgetClass, sliderFrame,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 //XmNheight, 2*rightBtnMenuLineHeight,
                 //XmNwidth, rightBtnMenuWidth/2,
                 //XmNtopAttachment, XmATTACH_WIDGET,
                 //XmNtopWidget, localFeatureBox,
                 //XmNleftAttachment, XmATTACH_WIDGET,
                 //XmNleftWidget, localFeatureBox,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 0,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 8,
                 NULL);

            stackedWidget.push_back(label);

            optionData = new MMDRightBtnMenuCallbackData_t;
            optionData->featureId = i;
            optionData->optionValue = task.localFeatures[i].selectedOption;
            optionData->mlabMissionDesignInstance = this;
            optionData->optionType = task.localFeatures[i].optionType;

            value = optionData->optionValue*conversion;

            rightBtnMenuOption_w_[i][0] = XtVaCreateManagedWidget
                ("",
                 xmScaleWidgetClass, sliderFrame,
                 XmNmaximum, maxValue,
                 XmNminimum, 1,
                 XmNvalue, (int)value,
                 XmNorientation, XmHORIZONTAL,
                 XmNprocessingDirection, XmMAX_ON_RIGHT,
                 XmNshowValue, true,
                 XmNdecimalPoints, decimalPoints,
                 /*
                   XmNheight, 2*rightBtnMenuLineHeight,
                   XmNwidth, 3*rightBtnMenuWidth/7,
                   XmNtopAttachment, XmATTACH_WIDGET,
                   XmNtopWidget, localFeatureBox,
                   XmNrightAttachment, XmATTACH_WIDGET,
                   XmNrightWidget, localFeatureBox,
                   XmNbottomAttachment, XmATTACH_WIDGET,
                   XmNbottomWidget, localFeatureBox,
                 */
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_POSITION,
                 XmNleftPosition, 8,
                 XmNrightAttachment, XmATTACH_POSITION,
                 XmNrightPosition, 19,
                 XmNtitleString, sliderTitle,
                 XmNhighlightColor, parentWidgetBg_,
                 NULL);

            stackedWidget.push_back(rightBtnMenuOption_w_[i][0]);
            line += 2;

            XtAddCallback
                (rightBtnMenuOption_w_[i][0],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbChangeRightBtnMenuValue_,
                 (void *)optionData);

            XmStringFree(sliderTitle);
            break;

        case MEXP_FEATURE_OPTION_POLYGONS:
            // To be implemented.
            break;
        }
    }

    // The frame for the feature navigation field.
    navigationFrame = XtVaCreateManagedWidget
        ("navigationFrame",
         //xmFormWidgetClass, main,
         xmFrameWidgetClass, main,
         XmNy, line*rightBtnMenuLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(navigationFrame);
    line += 2;

    navigation = XtVaCreateWidget
        ("navigation",
         xmRowColumnWidgetClass, navigationFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(navigation);

    // The label column.
    sprintf(title, RIGHTBTN_PREFERNCE_NAVIGATION_LABEL_.c_str());
    label = XtVaCreateWidget
        (title,
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-5,
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
         XmNheight, rightBtnMenuLineHeight-5,
         XmNwidth, rightBtnMenuWidth/2-10,
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
    sprintf(title, RIGHTBTN_PREV_BUTTON_LABEL_.c_str());
    prevBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, prevFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, rightBtnMenuLineHeight-5,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(prevBtn, featurePrevBtnIsSensitive);

    if (featurePrevBtnIsSensitive)
    {
        navData = new MMDNavRightBtnMenuCallbackData_t;
        navData->page = page - 1;
        index = unhiddenFeatureIDIndex - MAX_LOCAL_FEATURES_DISPLAY_;

        if (index < 0)
        {
            index = 0;
            navData->page = 1;
        }

        navData->startFeatureId = unhiddenFeatureIDs[index];
        navData->mlabMissionDesignInstance = this;

        XtAddCallback
            (prevBtn,
             XmNactivateCallback,
             (XtCallbackProc)cbNavigateRightBtnMenu_,
             (void *)navData);

        // Make the button blue.
        XtVaSetValues(
            prevBtn,
            XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
            XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
            XmNhighlightOnEnter, false,
            XmNtraversalOn, false,
            NULL);
    }

    stackedWidget.push_back(prevBtn);

    // A blank.
    label = XtVaCreateWidget
        (EMPTY_STRING_.c_str(),
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-5,
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
         XmNx, rightBtnMenuWidth/2,
         XmNheight, rightBtnMenuLineHeight-5,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, navigation,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, navigation,
         NULL);

    stackedWidget.push_back(nextFrame);

    // The "Next" button
    sprintf(title, RIGHTBTN_NEXT_BUTTON_LABEL_.c_str());
    nextBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, nextFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, rightBtnMenuLineHeight-5,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(nextBtn, featureNextBtnIsSensitive);

    if (featureNextBtnIsSensitive)
    {
        navData = new MMDNavRightBtnMenuCallbackData_t;
        navData->page = page + 1;;
        index = unhiddenFeatureIDIndex + MAX_LOCAL_FEATURES_DISPLAY_;

        if (index >= (int)(unhiddenFeatureIDs.size()))
        {
            // Something went wrong.
            XtSetSensitive(nextBtn, false);
            delete navData;
            navData = NULL;
        }
        else
        {
            navData->startFeatureId = unhiddenFeatureIDs[index];
            navData->mlabMissionDesignInstance = this;

            XtAddCallback
                (nextBtn,
                 XmNactivateCallback,
                 (XtCallbackProc)cbNavigateRightBtnMenu_,
                 (void *)navData);

            // Make the button blue.
            XtVaSetValues(
                nextBtn,
                XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
                XmNhighlightOnEnter, false,
                XmNtraversalOn, false,
                NULL);
        }
    }

    stackedWidget.push_back(nextBtn);

    // The frame for the action field.
    actionFrame = XtVaCreateManagedWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNy, line*rightBtnMenuLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*rightBtnMenuLineHeight,
         XmNwidth, rightBtnMenuWidth,
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
         XmNheight, 2*rightBtnMenuLineHeight-5,
         XmNwidth, rightBtnMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(action);

    // The frame for the "Apply" buttons.
    applyFrame = XtVaCreateManagedWidget
        ("applyFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, action,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(applyFrame);

    // The "Apply" button.
    apply = XtVaCreateManagedWidget
        ("Apply",
         xmPushButtonGadgetClass, applyFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, true,
         XmNtraversalOn, true,
         NULL);

    // Make the button red.
    XtVaSetValues(
        applyFrame,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
        NULL);

    XtVaSetValues(
        apply,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
        NULL);

    stackedWidget.push_back(apply);

    XtAddCallback
        (apply,
         XmNactivateCallback,
         (XtCallbackProc)cbApplyRightBtnMenu_,
         (void *)this);

    // The frame for the "Cancel" buttons.
    cancelFrame = XtVaCreateManagedWidget
        ("cancelFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNx, rightBtnMenuWidth/2,
         XmNheight, 2*rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, action,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(cancelFrame);

    // The "Cancel" button
    cancel = XtVaCreateManagedWidget
        ("Cancel",
         xmPushButtonGadgetClass, cancelFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*rightBtnMenuLineHeight-10,
         XmNwidth, rightBtnMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    // Make the button green.
    XtVaSetValues(
        cancelFrame,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
        NULL);

    XtVaSetValues(
        cancel,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
        NULL);

    stackedWidget.push_back(cancel);

    XtAddCallback
        (cancel,
         XmNactivateCallback,
         (XtCallbackProc)cbCancelRightBtnMenu_,
         (void *)this);

    rightBtnMenuUpTask_ = task;

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
        //XtManageChild(stackedWidget[i]);
    }

    XtPopup (rightBtnMenu_w_, XtGrabNone);

    rightBtnMenuIsUp_ = true;

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_TASK_PROPERTIES);
}

//-----------------------------------------------------------------------
// This routine creates a widget that contains list of constraints for
// robots in order to be used by CNP.
//-----------------------------------------------------------------------
void MlabMissionDesign::createRobotConstraintMenu_(
    int robotId,
    int startConstraintId,
    int page)
{
    XmString sliderTitle;
    Widget mainFrame, main, labelFrame, labelBox, label;
    Widget constraintFrame, constraintBox, buttonFrame;
    Widget applyFrame, apply, cancelFrame, cancel;
    Widget actionFrame, action;
    Widget navigationFrame, navigation;
    Widget prevFrame, prevBtn, nextFrame, nextBtn;
    vector<Widget> stackedWidget;
    vector<MExpFeature_t> constraints;
    vector<int> unhiddenConstraintIDs;
    MMDRobotConstraintMenuCallbackData_t *optionData;
    MMDNavRobotConstraintMenuCallbackData_t *navData;
    string labelString;
    char title[1024];
    float value, conversion;
    int index, i, j, numOptions, line = 0;
    int screenNumber;
    int posX, posY, displayWidth, maxPos, displayHeight, offset;
    int robotConstraintMenuHeight, robotConstraintMenuWidth, robotConstraintMenuLineHeight;
    int actionSpaceHeight, constraintHeight, constraintHeightTotal;
    int constraintLabelHeight;
    int numStackedWidget, numVisibleConstraints, numOptionRows;
    int numRobots, numConstraints;
    int maxValue, decimalPoints;
    int unhiddenConstraintIDIndex = 0;
    const int NUM_OPTION_COLUMNS = 2;
    bool robotFound = false;
    bool robotPrevBtnIsSensitive = true;
    bool robotNextBtnIsSensitive = true;
    bool constraintPrevBtnIsSensitive = false;
    bool constraintNextBtnIsSensitive = false;
    bool runtimeCNP = false;

    // Check for the runtime-CNP status.
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    // Load the constraints for the specified robot.
    numRobots = upRobotConstraintsList_.size();

    for (i = 0; i < numRobots; i++)
    {
        if (upRobotConstraintsList_[i].id == robotId)
        {
            constraints = upRobotConstraintsList_[i].constraints;
            robotFound = true;
            break;
        }
    }

    // Set the sensitivity of Next and Prev buttons for robot navigation.
    if (robotFound)
    {
        if (i == 0)
        {
            robotPrevBtnIsSensitive = false;
        }

        if (i >= (numRobots-1))
        {
            robotNextBtnIsSensitive = false;
        }
    }
    else
    {
        robotPrevBtnIsSensitive = false;
        robotNextBtnIsSensitive = false;
    }

    // Setting the height of the window.
    robotConstraintMenuLineHeight = DEFAULT_ROBOTCONSTRAINT_LINEHEIGHT_;

    // Set the action space (Apply & Cancel + Navitation x 2) height.
    actionSpaceHeight = 7*(robotConstraintMenuLineHeight+3);

    // Set the height for the label of each feature.
    constraintLabelHeight = robotConstraintMenuLineHeight+3;
    
    // Compute the total height of the features.
    numConstraints = constraints.size();
    constraintHeightTotal = 0;
    numVisibleConstraints = 0;
    
    // Preprocessed the constraints info.
    for (i = 0; i < numConstraints; i++)
    {
        if (constraints[i].hide)
        {
            if (i == startConstraintId)
            {
                startConstraintId++;
            }

            continue;
        }

        unhiddenConstraintIDs.push_back(i);

        if (i < startConstraintId)
        {
            constraintPrevBtnIsSensitive = true;
            continue;
        }

        if (i == startConstraintId)
        {
            unhiddenConstraintIDIndex = (unhiddenConstraintIDs.size()) - 1;
        }

        numVisibleConstraints++;

        if (numVisibleConstraints > MAX_CONSTRAINTS_DISPLAY_)
        {
            numVisibleConstraints = MAX_CONSTRAINTS_DISPLAY_;
            constraintNextBtnIsSensitive = true;
            continue;
        }

        numOptions = constraints[i].options.size();
        numOptionRows = 
            (numOptions/NUM_OPTION_COLUMNS) + 
            ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
        constraintHeightTotal += 
            (constraintLabelHeight + (numOptionRows*(robotConstraintMenuLineHeight+3)));
    }
    
    // Finally, set the height of the window.
    robotConstraintMenuHeight = 
        constraintLabelHeight +
        constraintHeightTotal +
        actionSpaceHeight;

    // Set other lengths.
    robotConstraintMenuWidth = DEFAULT_ROBOTCONSTRAINT_WIDTH_;
    offset = DEFAULT_ROBOTCONSTRAINT_EDGEOFFSET_;

    // Make sure the window does go outside the desktop.
    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);
    maxPos = displayWidth - robotConstraintMenuWidth - offset;
    posX = (displayWidth - robotConstraintMenuWidth)/2;
    posX = (posX < maxPos)? posX : maxPos;
    maxPos = displayHeight - robotConstraintMenuHeight - offset;
    posY = (displayHeight - robotConstraintMenuHeight)/2;
    posY = (posY < maxPos)? posY : maxPos;

    // Popup shell.
    robotConstraintMenu_w_ = XtVaCreatePopupShell
        ("",
         xmDialogShellWidgetClass, parentWidget_, 
         XmNdeleteResponse, XmUNMAP,
         XmNallowShellResize, false,
         XmNx, posX,
         XmNy, posY,
         XmNheight, robotConstraintMenuHeight,
         XmNwidth, robotConstraintMenuWidth,
         NULL);

    // The biggest frame.
    mainFrame = XtVaCreateManagedWidget
        ("mainFrame",
         xmFrameWidgetClass, robotConstraintMenu_w_,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuHeight,
         XmNwidth, robotConstraintMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, robotConstraintMenu_w_,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, robotConstraintMenu_w_,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, robotConstraintMenu_w_,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, robotConstraintMenu_w_,
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
         XmNheight, robotConstraintMenuHeight,
         XmNwidth, robotConstraintMenuWidth,
         NULL);

    stackedWidget.push_back(main);

    // The frame for the label.
    labelFrame = XtVaCreateManagedWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*robotConstraintMenuLineHeight-3,
         XmNwidth, robotConstraintMenuWidth,
         NULL);

    stackedWidget.push_back(labelFrame);

    // The widget for the main label.
    labelBox = XtVaCreateManagedWidget
        ("labelBox",
         xmRowColumnWidgetClass, labelFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNnumColumns, 1,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth,
         NULL);

    stackedWidget.push_back(labelBox);

    // The main label.
    if (runtimeCNP)
    {
        sprintf(title, ROBOTCONSTRAINT_TITLE_LABEL_RUNTIME_CNP_.c_str());
    }
    else
    {
        sprintf(title, ROBOTCONSTRAINT_TITLE_LABEL_.c_str());
    }

    sprintf(title, "%s [Total: %d Robots]", title, numRobots);

    label = XtVaCreateManagedWidget
        (title,
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, labelBox,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    line++;

    if (robotFound)
    {
        sprintf(title, "Robot ID: %d [Page %d]", robotId, page);
    }
    else
    {
        sprintf(title, "Robot ID: %d - Unknown. Check the RC file.", robotId);
    }

    label = XtVaCreateManagedWidget
        (title,
         xmLabelGadgetClass, labelBox,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, label,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, labelBox,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, labelBox,
         NULL);

    stackedWidget.push_back(label);

    line++;

    // It is a 2D array. Create the 1D part (feature) first.
    //robotConstraintMenuOption_w_ = new Widget*[numVisibleConstraints];
    robotConstraintMenuOption_w_ = new Widget*[numConstraints];

    //  Create the constraint raws sequentially.
    numVisibleConstraints = 0;

    for (i = 0; i < numConstraints; i++)
    {
        if (constraints[i].hide)
        {
            continue;
        }

        if (i < startConstraintId)
        {
            continue;
        }

        numVisibleConstraints++;

        if (numVisibleConstraints > MAX_CONSTRAINTS_DISPLAY_)
        {
            numVisibleConstraints = MAX_CONSTRAINTS_DISPLAY_;
            break;
        }

        numOptions = constraints[i].options.size();
        numOptionRows = 
            (numOptions/NUM_OPTION_COLUMNS) + 
            ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);

        // Height = label + options
        constraintHeight = 
            (constraintLabelHeight + (numOptionRows*robotConstraintMenuLineHeight+3));

        // The frame for the feature.
        constraintFrame = XtVaCreateManagedWidget
            ("constraintFrame",
             xmFrameWidgetClass, main,
             XmNalignment, XmALIGNMENT_BEGINNING,
             XmNheight, constraintHeight,
             XmNwidth, robotConstraintMenuWidth,
             NULL);

        stackedWidget.push_back(constraintFrame);

        // N-colum widget for the feature.
        constraintBox = XtVaCreateManagedWidget
            ("robotConstraintMenu-constraintBox",
             xmRowColumnWidgetClass, constraintFrame,
             XmNpacking, XmPACK_COLUMN,
             XmNorientation, XmHORIZONTAL,
             XmNalignment, XmALIGNMENT_CENTER,
             XmNnumColumns, numOptions,
             XmNheight, constraintHeight,
             XmNwidth, robotConstraintMenuWidth,
             XmNtopAttachment, XmATTACH_FORM,
             XmNrightAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             NULL);

        stackedWidget.push_back(constraintBox);

        // Create the second dimention (option) of this array.
        robotConstraintMenuOption_w_[i] = new Widget[numOptions];

        switch (constraints[i].optionType) {

        case MEXP_FEATURE_OPTION_TOGGLE:

            // The label column.
            label = XtVaCreateManagedWidget
                (constraints[i].name.c_str(),
                 xmLabelGadgetClass, constraintBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, robotConstraintMenuLineHeight,
                 XmNwidth, robotConstraintMenuWidth/NUM_OPTION_COLUMNS,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, constraintBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, constraintBox,
                 NULL);

            stackedWidget.push_back(label);
            line++;

            // Fill the blanks in the label row.
            for (j = 0; j < (NUM_OPTION_COLUMNS-1); j++)
            {
                label = XtVaCreateManagedWidget
                    (" ",
                     xmLabelGadgetClass, constraintBox,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, robotConstraintMenuLineHeight,
                     XmNwidth, robotConstraintMenuWidth/NUM_OPTION_COLUMNS,
                     XmNtopAttachment, XmATTACH_WIDGET,
                     XmNtopWidget, constraintBox,
                     NULL);

                stackedWidget.push_back(label);
            }
        
            for (j = 0; j < numOptions; j++)
            {
                optionData = new MMDRobotConstraintMenuCallbackData_t;
                optionData->robotId = robotId;
                optionData->constraintId = i;
                optionData->optionValue = (float)j;
                optionData->mlabMissionDesignInstance = this;
                optionData->optionType = constraints[i].optionType;

                if ((int)(constraints[i].selectedOption) == j)
                {
                    value = 1.0;
                }
                else
                {
                    value = 0.0;
                }

                buttonFrame = XtVaCreateManagedWidget
                    ("buttonFrame",
                     xmFormWidgetClass, constraintBox,
                     XmNshadowThickness, 0,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNheight, robotConstraintMenuLineHeight-3,
                     XmNwidth, robotConstraintMenuWidth/NUM_OPTION_COLUMNS-10,
                     NULL);

                stackedWidget.push_back(buttonFrame);

                // Toggle features.
                robotConstraintMenuOption_w_[i][j] = XtVaCreateManagedWidget
                    (constraints[i].options[j].value.c_str(),
                     xmToggleButtonGadgetClass, buttonFrame,
                     XmNradioBehavior, true,
                     XmNradioAlwaysOne, true,
                     XmNheight, robotConstraintMenuLineHeight-3,
                     XmNwidth, robotConstraintMenuWidth/NUM_OPTION_COLUMNS-10,
                     XmNset, (int)value,
                     XmNalignment, XmALIGNMENT_BEGINNING,
                     XmNhighlightColor, parentWidgetBg_,
                     NULL);

                stackedWidget.push_back(robotConstraintMenuOption_w_[i][j]);

                XtAddCallback
                    (robotConstraintMenuOption_w_[i][j],
                     XmNvalueChangedCallback,
                     (XtCallbackProc)cbChangeRobotConstraintMenuValue_,
                     (void *)optionData);
            }
            numOptions = constraints[i].options.size();
            numOptionRows = 
                (numOptions/NUM_OPTION_COLUMNS) + 
                ((numOptions%NUM_OPTION_COLUMNS)? 1 : 0);
            line += numOptionRows;
            break;

        case MEXP_FEATURE_OPTION_SLIDER100:
        case MEXP_FEATURE_OPTION_SLIDER10:
        case MEXP_FEATURE_OPTION_SLIDER1:

            switch (constraints[i].optionType) {

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
            label = XtVaCreateManagedWidget
                (constraints[i].name.c_str(),
                 xmLabelGadgetClass, constraintBox,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, 2*robotConstraintMenuLineHeight,
                 XmNwidth, robotConstraintMenuWidth/2,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, constraintBox,
                 XmNleftAttachment, XmATTACH_WIDGET,
                 XmNleftWidget, constraintBox,
                 NULL);

            stackedWidget.push_back(label);

            optionData = new MMDRobotConstraintMenuCallbackData_t;
            optionData->robotId = robotId;
            optionData->constraintId = i;
            optionData->optionValue = constraints[i].selectedOption;
            optionData->mlabMissionDesignInstance = this;
            optionData->optionType = constraints[i].optionType;

            value = optionData->optionValue*conversion;

            robotConstraintMenuOption_w_[i][0] = XtVaCreateManagedWidget
                ("",
                 xmScaleWidgetClass, constraintBox,
                 XmNmaximum, maxValue,
                 XmNminimum, 1,
                 XmNvalue, (int)value,
                 XmNorientation, XmHORIZONTAL,
                 XmNprocessingDirection, XmMAX_ON_RIGHT,
                 XmNshowValue, true,
                 XmNdecimalPoints, decimalPoints,
                 XmNheight, 2*robotConstraintMenuLineHeight,
                 XmNwidth, 3*robotConstraintMenuWidth/7,
                 XmNtopAttachment, XmATTACH_WIDGET,
                 XmNtopWidget, constraintBox,
                 XmNrightAttachment, XmATTACH_WIDGET,
                 XmNrightWidget, constraintBox,
                 XmNbottomAttachment, XmATTACH_WIDGET,
                 XmNbottomWidget, constraintBox,
                 XmNtitleString, sliderTitle,
                 XmNhighlightColor, parentWidgetBg_,
                 NULL);

            stackedWidget.push_back(robotConstraintMenuOption_w_[i][0]);
            line += 2;

            XtAddCallback
                (robotConstraintMenuOption_w_[i][0],
                 XmNvalueChangedCallback,
                 (XtCallbackProc)cbChangeRobotConstraintMenuValue_,
                 (void *)optionData);

            XmStringFree(sliderTitle);
            break;
        }
    }

    // The frame for the constraint navigation field.
    navigationFrame = XtVaCreateManagedWidget
        ("navigationFrame",
         //xmFormWidgetClass, main,
         xmFrameWidgetClass, main,
         XmNy, line*robotConstraintMenuLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(navigationFrame);
    line += 2;

    navigation = XtVaCreateWidget
        ("navigation",
         xmRowColumnWidgetClass, navigationFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(navigation);

    // The label column.
    sprintf(title, ROBOTCONSTRAINT_CONSTRAINT_NAVIGATION_LABEL_.c_str());
    label = XtVaCreateWidget
        (title,
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-5,
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
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
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
    sprintf(title, ROBOTCONSTRAINT_PREV_BUTTON_LABEL_.c_str());
    prevBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, prevFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(prevBtn, constraintPrevBtnIsSensitive);

    if (constraintPrevBtnIsSensitive)
    {
        navData = new MMDNavRobotConstraintMenuCallbackData_t;
        navData->targetRobotId = robotId;
        navData->page = page - 1;
        index = unhiddenConstraintIDIndex - MAX_CONSTRAINTS_DISPLAY_;

        if (index < 0)
        {
            index = 0;
            navData->page = 1;
        }

        navData->startConstraindId = unhiddenConstraintIDs[index];
        navData->mlabMissionDesignInstance = this;

        XtAddCallback
            (prevBtn,
             XmNactivateCallback,
             (XtCallbackProc)cbNavigateConstraintMenu_,
             (void *)navData);
    }

    stackedWidget.push_back(prevBtn);

    // A blank.
    label = XtVaCreateWidget
        (EMPTY_STRING_.c_str(),
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-5,
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
         XmNx, robotConstraintMenuWidth/2,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, navigation,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, navigation,
         NULL);

    stackedWidget.push_back(nextFrame);

    // The "Next" button
    sprintf(title, ROBOTCONSTRAINT_NEXT_BUTTON_LABEL_.c_str());
    nextBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, nextFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(nextBtn, constraintNextBtnIsSensitive);

    if (constraintNextBtnIsSensitive)
    {
        navData = new MMDNavRobotConstraintMenuCallbackData_t;
        navData->targetRobotId = robotId;
        navData->page = page + 1;;
        index = unhiddenConstraintIDIndex + MAX_CONSTRAINTS_DISPLAY_;

        if (index >= (int)(unhiddenConstraintIDs.size()))
        {
            // Something went wrong.
            XtSetSensitive(nextBtn, false);
            delete navData;
            navData = NULL;
        }
        else
        {
            navData->startConstraindId = unhiddenConstraintIDs[index];
            navData->mlabMissionDesignInstance = this;

            XtAddCallback
                (nextBtn,
                 XmNactivateCallback,
                 (XtCallbackProc)cbNavigateConstraintMenu_,
                 (void *)navData);
        }
    }

    stackedWidget.push_back(nextBtn);

    // The frame for the robot navigation field.
    navigationFrame = XtVaCreateManagedWidget
        ("navigationFrame",
         xmFrameWidgetClass, main,
         XmNy, line*robotConstraintMenuLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth-5,
         XmNbottomAttachment, XmATTACH_WIDGET,
         NULL);

    stackedWidget.push_back(navigationFrame);
    line += 2;

    navigation = XtVaCreateWidget
        ("navigation",
         xmRowColumnWidgetClass, navigationFrame,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 2,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(navigation);

    // The label column.
    sprintf(title, ROBOTCONSTRAINT_ROBOT_NAVIGATION_LABEL_.c_str());
    label = XtVaCreateWidget
        (title,
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-5,
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
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
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
    sprintf(title, ROBOTCONSTRAINT_PREV_BUTTON_LABEL_.c_str());
    prevBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, prevFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(prevBtn, robotPrevBtnIsSensitive);

    if (robotPrevBtnIsSensitive)
    {
        navData = new MMDNavRobotConstraintMenuCallbackData_t;
        navData->targetRobotId = robotId - 1;
        navData->page = 1;
        navData->startConstraindId = 0;
        navData->mlabMissionDesignInstance = this;

        XtAddCallback
            (prevBtn,
             XmNactivateCallback,
             (XtCallbackProc)cbNavigateConstraintMenu_,
             (void *)navData);

        // Make the button blue.
        XtVaSetValues(
            prevFrame,
            XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
            XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
            NULL);

        XtVaSetValues(
            prevBtn,
            XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
            XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
            NULL);
    }

    stackedWidget.push_back(prevBtn);

    // A blank.
    label = XtVaCreateWidget
        (EMPTY_STRING_.c_str(),
         xmLabelGadgetClass, navigation,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-5,
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
         XmNx, robotConstraintMenuWidth/2,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, navigation,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, navigation,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, navigation,
         NULL);

    stackedWidget.push_back(nextFrame);

    // The "Next" button
    sprintf(title, ROBOTCONSTRAINT_NEXT_BUTTON_LABEL_.c_str());
    nextBtn = XtVaCreateWidget
        (title,
         xmPushButtonGadgetClass, nextFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    XtSetSensitive(nextBtn, robotNextBtnIsSensitive);

    if (robotNextBtnIsSensitive)
    {
        navData = new MMDNavRobotConstraintMenuCallbackData_t;
        navData->targetRobotId = robotId + 1;
        navData->page = 1;
        navData->startConstraindId = 0;
        navData->mlabMissionDesignInstance = this;

        XtAddCallback
            (nextBtn,
             XmNactivateCallback,
             (XtCallbackProc)cbNavigateConstraintMenu_,
             (void *)navData);

        // Make the button blue.
        XtVaSetValues(
            nextFrame,
            XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
            XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
            NULL);

        XtVaSetValues(
            nextBtn,
            XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
            XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.blue,
            NULL);
    }

    stackedWidget.push_back(nextBtn);

    // The frame for the action field.
    actionFrame = XtVaCreateManagedWidget
        ("labelFrame",
         xmFrameWidgetClass, main,
         XmNy, line*robotConstraintMenuLineHeight,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNheight, 2*robotConstraintMenuLineHeight,
         XmNwidth, robotConstraintMenuWidth,
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
         XmNheight, 2*robotConstraintMenuLineHeight-5,
         XmNwidth, robotConstraintMenuWidth-5,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    stackedWidget.push_back(action);

    // The frame for the "Apply" buttons.
    applyFrame = XtVaCreateManagedWidget
        ("applyFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, action,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(applyFrame);

    if (runtimeCNP)
    {
        if (isDeployedRuntimeCNPRobot_(robotId))
        {
            if (selectedDeployedRuntimeCNPRobotID_ == robotId)
            {
                labelString = "Selected by\nThis Robot";
            }
            else
            {
                labelString = "Selected by\nAnother Robot";
            }
        }
        else
        {
            labelString = "Select";
        }
    }
    else
    {
        labelString = "Apply";
    }

    // The "Apply" button.
    apply = XtVaCreateManagedWidget
        (labelString.c_str(),
         xmPushButtonGadgetClass, applyFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, true,
         XmNtraversalOn, true,
         NULL);

    if (runtimeCNP)
    {
        if(!robotFound)
        {
            XtSetSensitive(apply, false);
        }
        else if (isDeployedRuntimeCNPRobot_(robotId))
        {
            XtSetSensitive(apply, false);
        }
        else
        {
            // Make the button red.
            XtVaSetValues(
                applyFrame,
                XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
                NULL);

            XtVaSetValues(
                apply,
                XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
                NULL);
        }
    }
    else
    {
        if ((!robotFound) || (!allowModCNPRobots_))
        {
            XtSetSensitive(apply, false);
        }
        else
        {
            // Make the button red.
            XtVaSetValues(
                applyFrame,
                XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
                NULL);

            XtVaSetValues(
                apply,
                XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
                XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.red,
                NULL);
        }
    }

    stackedWidget.push_back(apply);

    XtAddCallback
        (apply,
         XmNactivateCallback,
         (XtCallbackProc)cbApplyRobotConstraintMenu_,
         (void *)this);

    // The frame for the "Cancel" buttons.
    cancelFrame = XtVaCreateManagedWidget
        ("cancelFrame",
         xmFormWidgetClass, action,
         XmNalignment, XmALIGNMENT_BEGINNING,
         XmNx, robotConstraintMenuWidth/2,
         XmNheight, 2*robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, action,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, action,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget, action,
         NULL);

    stackedWidget.push_back(cancelFrame);

    // The "Cancel" button
    if (runtimeCNP)
    {
        if ((isDeployedRuntimeCNPRobot_(robotId)) &&
            (selectedDeployedRuntimeCNPRobotID_ == robotId))
        {
            labelString = "OK";
        }
        else
        {
            labelString = "Close";
        }
    }
    else
    {
        if (allowModCNPRobots_)
        {
            labelString = "Cancel";
        }
        else
        {
            labelString = "OK";
        }
    }

    cancel = XtVaCreateManagedWidget
        (labelString.c_str(),
         xmPushButtonGadgetClass, cancelFrame,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNheight, 2*robotConstraintMenuLineHeight-10,
         XmNwidth, robotConstraintMenuWidth/2-10,
         XmNtopAttachment, XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNhighlightOnEnter, false,
         XmNtraversalOn, false,
         NULL);

    // Make the button green.
    XtVaSetValues(
        cancelFrame,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
        NULL);

    XtVaSetValues(
        cancel,
        XmNforeground, (disableColorButtons_)? parentWidgetFg_ : gColorPixel.white,
        XmNbackground, (disableColorButtons_)? parentWidgetBg_ : gColorPixel.green4,
        NULL);

    stackedWidget.push_back(cancel);

    XtAddCallback
        (cancel,
         XmNactivateCallback,
         (XtCallbackProc)cbCancelRobotConstraintMenu_,
         (void *)this);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget.back());
        stackedWidget.pop_back();
        //XtManageChild(stackedWidget[i]);
    }

    robotConstraintMenuIsUp_ = true;

    if (runtimeCNP)
    {
        // Mark this one as the popped-up runtime-CNP robot that the
        // user is seeing.
        upRuntimeCNPRobotID_ = robotId;
    }

    XtPopup (robotConstraintMenu_w_, XtGrabNone);
}

//-----------------------------------------------------------------------
// This function pops up the right-button menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::popUpRightBtnMenu_(MExpMissionTask_t task)
{
    const int DEFAULT_START_FEATURE_ID = 0;
    const int DEFAULT_START_CONSTRAINT_ID = 0;
    const int DEFAULT_START_PAGE = 1;
    int robotID;
    bool runtimeCNP = false;

    // Check for the runtime-CNP status.
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if ((runtimeCNP) &&
        (task.name == STRING_RUNTIME_CNP_BIDDER_TASK_))
    {

        // Runtime-CNP is ON. Do not pop up the regular RightBtnMenu.
        // Instead, pop up the robot constraint menu.
        robotID = getDeployedRuntimeCNPRobotIDFromTaskNumber_(task.number);

        if (robotID == INVALID_ROBOT_ID_)
        {
            fprintf(
                stderr,
                "Error: MlabMissionDesign::popUpRightBtnMenu_(). Robot ID not found.\a\n");
            return;
        }

        // Mark this robot as the selected deployed robot.
        selectedDeployedRuntimeCNPRobotID_ = robotID;

        // Pop up the robot constraint menu.
        upRobotConstraintsList_ = robotConstraintsList_;
        createRobotConstraintMenu_(
            robotID,
            DEFAULT_START_CONSTRAINT_ID,
            DEFAULT_START_PAGE);

        return;
    }

    createRightBtnMenu_(task, DEFAULT_START_FEATURE_ID, DEFAULT_START_PAGE);

    gEventLogging->start("MMD RightBtnMenu Task #%d [%s]", task.number, task.name.c_str());
}

//-----------------------------------------------------------------------
// This function pops up the right-button menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::popUpRobotConstraintMenu_(void)
{
    const int DEFAULT_ROBOT_ID = 1;
    const int DEFAULT_START_CONSTRAINT_ID = 0;
    const int DEFAULT_PAGE = 1;

    // Load the default.
    upRobotConstraintsList_ = robotConstraintsList_;

    createRobotConstraintMenu_(
        DEFAULT_ROBOT_ID,
        DEFAULT_START_CONSTRAINT_ID,
        DEFAULT_PAGE);
}

//-----------------------------------------------------------------------
// This function pops up the task menu for the runtime-CNP Mission
// Manager.
//-----------------------------------------------------------------------
void MlabMissionDesign::popUpRuntimeCNPMissionManagerMenu_(void)
{
    MExpMissionTask_t task;
    bool runtimeCNP = false, hadError = false;

    // Check for the runtime-CNP status.
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if (!runtimeCNP)
    {
        warn_userf("Runtime CNP not enabled.");
        return;
    }

    if ((int)(specifiedTaskList_.size()) > 0)
    {
        task = specifiedTaskList_[0];

        if ((task.id) != runtimeCNPMissionManagerTask_)
        {
            hadError = true;
        }
    }
    else
    {
        hadError = true;
    }

    if (hadError)
    {
        warn_userf("CNP Mission Manager could not be found.");
        return;
    }

    popUpRightBtnMenu_(task);
}

//-----------------------------------------------------------------------
// This function pops up the task menu for the Icarus Task.
//-----------------------------------------------------------------------
void MlabMissionDesign::popUpIcarusTaskMenu_(void)
{
    MExpMissionTask_t task;
    bool hadError = false;

    if (!missionSpecWizardEnabled())
    {
        warn_userf("ICARUS Wizard not enabled.");
        return;
    }

    if ((int)(specifiedTaskList_.size()) > 0)
    {
        task = specifiedTaskList_[0];

        if ((task.id) != icarusTask_)
        {
            hadError = true;
        }
    }
    else
    {
        hadError = true;
    }

    if (hadError)
    {
        warn_userf("ICARUS Task could not be found.");
        return;
    }

    popUpRightBtnMenu_(task);
}

//-----------------------------------------------------------------------
// This function chages the values of the right-button menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::changeRightBtnMenuValue_(XtPointer client_data)
{
    int i, j;
    bool selected;
    int sliderBarValue;
    float optionValue;
    char optionValueStr[256];

    MMDRightBtnMenuCallbackData_t *data = (MMDRightBtnMenuCallbackData_t *)client_data;
    i = data->featureId;

    switch (data->optionType) {

    case MEXP_FEATURE_OPTION_TOGGLE:
        
        rightBtnMenuUpTask_.localFeatures[i].selectedOption = data->optionValue;

        for (j = 0; j < ((int)(rightBtnMenuUpTask_.localFeatures[i].options.size())); j++)
        {
            if ((int)(rightBtnMenuUpTask_.localFeatures[i].selectedOption) == j)
            {
                selected = true;

                gEventLogging->log
                    ("MMD RightBtnMenu Task #%d [%s] %s %s toggled",
                     rightBtnMenuUpTask_.number,
                     rightBtnMenuUpTask_.name.c_str(),
                     rightBtnMenuUpTask_.localFeatures[i].name.c_str(),
                     rightBtnMenuUpTask_.localFeatures[i].options[j].value.c_str());
            }
            else
            {
                selected = false;
            }

            XtVaSetValues(
                rightBtnMenuOption_w_[i][j],
                XmNset, selected,
                NULL);
        }

        break;

    case MEXP_FEATURE_OPTION_SLIDER100:
    case MEXP_FEATURE_OPTION_SLIDER10:
    case MEXP_FEATURE_OPTION_SLIDER1:

        XtVaGetValues(
            rightBtnMenuOption_w_[i][0],
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

        rightBtnMenuUpTask_.localFeatures[i].selectedOption = optionValue;
        rightBtnMenuUpTask_.localFeatures[i].options[0].value = optionValueStr;

        gEventLogging->log
            ("MMD RightBtnMenu Task #%d [%s] %s adjusted to %s",
             rightBtnMenuUpTask_.number,
             rightBtnMenuUpTask_.name.c_str(),
             rightBtnMenuUpTask_.localFeatures[i].name.c_str(),
             rightBtnMenuUpTask_.localFeatures[i].options[0].value.c_str());

        break;
    }
}

//-----------------------------------------------------------------------
// This function replaces the selected option value of a feature in the
// specified task.
//-----------------------------------------------------------------------
bool MlabMissionDesign::replaceTaskFeatureSelectedOption_(
    string featureName,
    float selectedOption,
    MExpMissionTask_t *task)
{
    int i;
    bool replaced = false;

    if (task == NULL)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::replaceTaskFeatureSelectedOption_(). task is NULL.\a\n");
        return false;
    }

    for (i = 0; i < (int)(task->localFeatures.size()); i++)
    {
        if ((task->localFeatures[i].name) == featureName)
        {
            task->localFeatures[i].selectedOption = selectedOption;
            replaced = true;
            break;
        }
    }

    return replaced;
}

//-----------------------------------------------------------------------
// This function replaces the selected option value of a feature in the
// specified task in the list.
//-----------------------------------------------------------------------
bool MlabMissionDesign::replaceTaskFeatureSelectedOption_(
    int taskNumber,
    string featureName,
    float selectedOption,
    vector<MExpMissionTask_t> *taskList)
{
    int i;
    bool replaced = false;

    if (taskList == NULL)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::replaceTaskFeatureSelectedOption_(). taskList is NULL.\a\n");
        return false;
    }

    for (i = 0; i < (int)(taskList->size()); i++)
    {
        if (((*taskList)[i].number) == taskNumber)
        {
            replaced = replaceTaskFeatureSelectedOption_(
                featureName,
                selectedOption,
                &((*taskList)[i]));
            break;
        }
    }

    return replaced;
}

//-----------------------------------------------------------------------
// This function returns the selected option value of the feature in
// the task.
//-----------------------------------------------------------------------
float MlabMissionDesign::getTaskFeatureSelectedOption_(
    string featureName,
    MExpMissionTask_t task)
{
    float selectedOption = -1.0;
    int i;
    bool found = false;

    for (i = 0; i < (int)(task.localFeatures.size()); i++)
    {
        if ((task.localFeatures[i].name) == featureName)
        {
            selectedOption = task.localFeatures[i].selectedOption;
            found = true;
            break;
        }
    }

    if (!found)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::getTaskFeatureSelectedOption_(). Feature not found.\a\n");
    }

    return selectedOption;
}

//-----------------------------------------------------------------------
// This function chages the values of the robot constraint menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::changeRobotConstraintMenuValue_(XtPointer client_data)
{
    int i, j, k;
    int sliderBarValue;
    float optionValue;
    char optionValueStr[256];
    bool robotFound = false, selected = false;

    MMDRobotConstraintMenuCallbackData_t *data = (MMDRobotConstraintMenuCallbackData_t *)client_data;

    if (!allowModCNPRobots_)
    {
        confirmUser_(CONFIRM_NO_ROBOT_CONSTRAINT_PERMISSION_PHRASE_, true);
    }

    for (i = 0; i < (int)(upRobotConstraintsList_.size()); i++)
    {
        if (upRobotConstraintsList_[i].id == (data->robotId))
        {
            // Remember i.
            robotFound = true;
            break;
        }
    }

    if (!robotFound)
    {
        fprintf(stderr, "Error in changeRobotConstraintMenuValue_().\n");
        return;
    }

    j = data->constraintId;

    switch (data->optionType) {

    case MEXP_FEATURE_OPTION_TOGGLE:
        
        if (allowModCNPRobots_)
        {
            optionValue = data->optionValue;
            upRobotConstraintsList_[i].constraints[j].selectedOption = optionValue;
        }
        else
        {
            optionValue = upRobotConstraintsList_[i].constraints[j].selectedOption;
        }

        for (k = 0; k < ((int)(upRobotConstraintsList_[i].constraints[j].options.size())); k++)
        {
            if ((int)(optionValue) == k)
            {
                selected = true;
            }
            else
            {
                selected = false;
            }

            XtVaSetValues(
                robotConstraintMenuOption_w_[j][k],
                XmNset, selected,
                NULL);
        }

        break;

    case MEXP_FEATURE_OPTION_SLIDER100:
    case MEXP_FEATURE_OPTION_SLIDER10:
    case MEXP_FEATURE_OPTION_SLIDER1:

        if (allowModCNPRobots_)
        {
            XtVaGetValues(
                robotConstraintMenuOption_w_[j][0],
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

            upRobotConstraintsList_[i].constraints[j].selectedOption = optionValue;
            upRobotConstraintsList_[i].constraints[j].options[0].value = optionValueStr;
        }
        else
        {

            switch (data->optionType) {

            case MEXP_FEATURE_OPTION_SLIDER100:
            case MEXP_FEATURE_OPTION_SLIDER10:
                optionValue = upRobotConstraintsList_[i].constraints[j].selectedOption;
                break;

            case MEXP_FEATURE_OPTION_SLIDER1:
            default:
                optionValue = 100.0*(upRobotConstraintsList_[i].constraints[j].selectedOption);
                break;
            }

            sliderBarValue = (int)optionValue;

            XtVaSetValues(
                robotConstraintMenuOption_w_[j][0],
                XmNvalue, sliderBarValue,
                NULL);
        }

        break;
    }
}

//-----------------------------------------------------------------------
// This function navigates the values of the robot constraint menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::navigateRobotConstraintMenu_(XtPointer client_data)
{
    MMDNavRobotConstraintMenuCallbackData_t *data = (MMDNavRobotConstraintMenuCallbackData_t *)client_data;
    
    closeRobotConstraintMenu_();
    createRobotConstraintMenu_(
        data->targetRobotId,
        data->startConstraindId,
        data->page);
}

//-----------------------------------------------------------------------
// This function navigates the values of the robot constraint menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::navigateRightBtnMenu_(XtPointer client_data)
{
    MMDNavRightBtnMenuCallbackData_t *data = (MMDNavRightBtnMenuCallbackData_t *)client_data;
    
    closeRightBtnMenu_();
    createRightBtnMenu_(
        rightBtnMenuUpTask_,
        data->startFeatureId,
        data->page);
}

//-----------------------------------------------------------------------
// This function stores the "Up" task in the task array.
//-----------------------------------------------------------------------
void MlabMissionDesign::applyRightBtnMenu_(void) 
{
    int i;

    i = rightBtnMenuUpTask_.number;
    specifiedTaskList_[i] = rightBtnMenuUpTask_;

    gEventLogging->log("MMD RightBtnMenu Apply Change");

    closeRightBtnMenu_();

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT);
}

//-----------------------------------------------------------------------
// This function changes the robot constraint as specified by the user.
//-----------------------------------------------------------------------
void MlabMissionDesign::applyRobotConstraintMenu_(void) 
{
    DeployedRuntimeCNPRobot_t deployedRobot;
    MExpMissionTask_t task;
    string entryKeyAdd, entryKeyRemove;
    bool runtimeCNP = false;
    bool replaced = false;

    robotConstraintsList_= upRobotConstraintsList_;

    // Check for the runtime-CNP status
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if (runtimeCNP)
    {
        // Get the robot ID of the deployed and currently selected option.
        deployedRobot = getDeployedRuntimeCNPRobotFromRobotID_(
            selectedDeployedRuntimeCNPRobotID_);

        // Generate the key for the task entry to clear.
        entryKeyRemove = generateTaskEntryKey_(deployedRobot.task);

        // Update the specifiedTaskList_ as well as redraw it on the display.
        replaced = replaceTaskFeatureSelectedOption_(
            deployedRobot.task.number,
            STRING_ROBOT_ID_,
            (float)(upRuntimeCNPRobotID_),
            &specifiedTaskList_);

        if (!replaced)
        {
            fprintf(
                stderr,
                "Error: MlabMissionDesign::applyRobotConstraintMenu_(). Robot ID [%d] not replaced to [%d].\a\n",
                selectedDeployedRuntimeCNPRobotID_,
                upRuntimeCNPRobotID_);
        }

        // Clear the task from the overlay.
        gt_delete_db_entry((char *)(entryKeyRemove.c_str()));

        // Remove from the deploy list, too.
        undeployRuntimeCNPRobot_(selectedDeployedRuntimeCNPRobotID_);

        // Re-add the task with the new robot ID.
        deployRuntimeCNPRobot_(upRuntimeCNPRobotID_, deployedRobot.task);

        // Update the overlay
        entryKeyAdd = generateTaskEntryKey_(deployedRobot.task);
        gt_add_passage_point(
            (char *)(entryKeyAdd.c_str()),
            deployedRobot.task.position.x,
            deployedRobot.task.position.y,
            placeCircleDiameter_*drawingUnitLength_);

        // Clear the screen and redraw
        XtAppLock(appContext_);
        XLockDisplay(display_);
        clear_map();
        draw_world();
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);
    }

    closeRobotConstraintMenu_();
}

//-----------------------------------------------------------------------
// This function cancels the right button menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::cancelRightBtnMenu_(void) 
{
    gEventLogging->log("MMD RightBtnMenu Cancel Change");

    closeRightBtnMenu_();

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT);
}

//-----------------------------------------------------------------------
// This function cancels the robot constraint menu.
//-----------------------------------------------------------------------
void MlabMissionDesign::cancelRobotConstraintMenu_(void) 
{
    closeRobotConstraintMenu_();
}

//-----------------------------------------------------------------------
// This function closes the right-button menu window.
//-----------------------------------------------------------------------
void MlabMissionDesign::closeRightBtnMenu_(void) 
{
    delete [] rightBtnMenuOption_w_;
    XtDestroyWidget(rightBtnMenu_w_);
    rightBtnMenuIsUp_ = false;
    gEventLogging->end("MMD RightBtnMenu");
}

//-----------------------------------------------------------------------
// This function closes the robot constraint menu window.
//-----------------------------------------------------------------------
void MlabMissionDesign::closeRobotConstraintMenu_(void) 
{
    bool runtimeCNP = false;

    delete [] robotConstraintMenuOption_w_;
    XtDestroyWidget(robotConstraintMenu_w_);
    robotConstraintMenuIsUp_ = false;

    // Check for the runtime-CNP status
    runtimeCNP = (cnpMode_ == CNP_MODE_RUNTIME)? true : false;

    if (runtimeCNP)
    {
        upRuntimeCNPRobotID_ = INVALID_ROBOT_ID_;
    }
}

//-----------------------------------------------------------------------
// This function checks whether the particular point is inside the already-
// placed task.
//-----------------------------------------------------------------------
bool MlabMissionDesign::isInsideTask_(
    double x,
    double y,
    double unitLength,
    MExpMissionTask_t *task)
{
    int i;
    double dist, closestDist;
    MExpMissionTask_t closestTask;

    if (numTasks_ == 0)
    {
        return false;
    }

    // Find the closest waypoint
    closestTask = specifiedTaskList_[0];
    closestDist = pow((specifiedTaskList_[0].position.x - x),2) + pow((specifiedTaskList_[0].position.y - y),2);
    
    for (i = 1; i < numTasks_; i++)
    {
        dist = pow((specifiedTaskList_[i].position.x - x),2) + pow((specifiedTaskList_[i].position.y - y),2);

        if (dist < closestDist)
        {
            closestTask = specifiedTaskList_[i];
            closestDist = dist;
        }
    }

    // Too far from any waypoint
    closestDist = (sqrt(closestDist)) / unitLength;

    if (closestDist > placeCircleDiameter_)
    {
        return false;
    }

    *task = closestTask;
    return true;
}

//-----------------------------------------------------------------------
// This function checks whether the particular point is inside one of the
// polygon point.
//-----------------------------------------------------------------------
bool MlabMissionDesign::isAtEditingPolygonPoint_(
    double x,
    double y,
    double unitLength,
    MMDPolygon_t *selectedPoly)
{
    MMDPolygon_t poly, closestPoly;
    MExpPoint2D_t pos;
    int i, j;
    double dist, closestDist;

    closestDist = numeric_limits<double>::infinity();

    for (i = 0; i < (int)(editingPolygonList_.size()); i++)
    {
        poly = editingPolygonList_[i];

        for (j = 0; j < (int)(poly.posList.size()); j++)
        {
            pos = poly.posList[j];

            dist = pow((pos.x - x),2) + pow((pos.y - y),2);
            
            if (((i == 0) && (j == 0)) || (dist <= closestDist))
            {
                closestPoly = poly;
                closestPoly.selectedPosIndex = j;
                closestDist = dist;
            }
        }
    }

    // Check with the diameter of the point.
    closestDist = (sqrt(closestDist)) / unitLength;

    if (closestDist > placePolygonPointDiameter_)
    {
        return false;
    }

    
    *selectedPoly = closestPoly;

    return true;
}

//-----------------------------------------------------------------------
// This function checks whether the particular point is inside the already-
// placed via point.
//-----------------------------------------------------------------------
bool MlabMissionDesign::isInsideViaPoint_(
    double x,
    double y,
    double unitLength,
    MExpPoint2D_t *viaPoint,
    int *index)
{
    int i;
    double dist, closestDist;
    MExpPoint2D_t closestViaPoint;

    if ((int)(viaPointList_.size()) == 0)
    {
        *index = -1;
        return false;
    }

    // Find the closest via point.
    *index = 0;
    closestViaPoint = viaPointList_[0];
    closestDist = pow((viaPointList_[0].x - x),2) + pow((viaPointList_[0].y - y),2);
    
    for (i = 1; i < numTasks_; i++)
    {
        dist = pow((viaPointList_[i].x - x),2) + pow((viaPointList_[i].y - y),2);

        if (dist < closestDist)
        {
            closestViaPoint = viaPointList_[i];
            closestDist = dist;
            *index = i;
        }
    }

    // Too far from any waypoint
    closestDist = (sqrt(closestDist)) / unitLength;

    if (closestDist > placeCircleDiameter_)
    {
        *index = -1;
        return false;
    }

    *viaPoint = closestViaPoint;

    return true;
}

//-----------------------------------------------------------------------
// This function checks whether the particular point is inside the
// currently specifying polygon start point.
//-----------------------------------------------------------------------
bool MlabMissionDesign::isAtSpecifyingPolygonStartPoint_(
    double x,
    double y,
    double unitLength)
{
    double dist;

    if ((int)(specifyingPolygon_.posList.size()) < 1)
    {
        return false;
    }

    dist = sqrt(
        pow((specifyingPolygon_.posList[0].x - x),2) +
        pow((specifyingPolygon_.posList[0].y - y),2)) /
        unitLength;

    if (dist > placePolygonPointDiameter_)
    {
        return false;
    }

    return true;
}

//-----------------------------------------------------------------------
// This routine function is called when a mouse is clicked on the
// overlay.
//-----------------------------------------------------------------------
void MlabMissionDesign::mouseClick(
    double x,
    double y,
    XEvent *event,
    double unitLength) 
{
    MExpMissionTask_t task;
    MMDPolygon_t poly;
    bool isInsideTask = false, isAtPolyPoint = false;

    lastMouseXRoot_ = event->xbutton.x_root;
    lastMouseYRoot_ = event->xbutton.y_root;
    drawingUnitLength_ = unitLength;

    switch (MMDMode_) {

    case DESIGN_MODE:
        if (isSpecifyingPolygon_)
        {
            switch (event->xbutton.button) {

            case LEFT_BTN:
                if (isAtSpecifyingPolygonStartPoint_(x, y, drawingUnitLength_))
                {
                    completeSpecifyingPolygon_();
                }
                else
                {
                    placePolygonPoint_(x, y);
                }
                break;

            case MIDDLE_BTN:
                cancelSpecifyingPolygon_();
                break;

            case RIGHT_BTN:
                completeSpecifyingPolygon_();
                break;
            } // switch (event->xbutton.button)
        }
        else if (isEditingPolygon_)
        {
            isAtPolyPoint = isAtEditingPolygonPoint_(
                x,
                y,
                drawingUnitLength_,
                &poly);

            switch (event->xbutton.button) {

            case LEFT_BTN:
                if (isAtPolyPoint)
                {
                    liftPolygonVertex_(poly);
                }
                break;

            case MIDDLE_BTN:
                if (isAtPolyPoint)
                {
                    removePolygon_(poly);
                }
                break;

            case RIGHT_BTN:
                finishEditingPolygon_();
                break;

            } // switch (event->xbutton.button)
        }
        else
        {
            if (rightBtnMenuIsUp_)
            {
                closeRightBtnMenu_();
            }

            if (robotConstraintMenuIsUp_)
            {
                closeRobotConstraintMenu_();
            }

            isInsideTask = isInsideTask_(x, y, drawingUnitLength_, &task);

            switch (event->xbutton.button) {

            case LEFT_BTN:
                if (isInsideTask)
                {
                    liftTask_(task);
                }
                else
                {
                    placeTask_(x, y, MMDCurrentTask_);
                }
                break;

            case MIDDLE_BTN:
                if (isInsideTask)
                {
                    removeTask_(task);
                }
                break;

            case RIGHT_BTN:
                if (isInsideTask)
                {
                    popUpRightBtnMenu_(task);
                }
                break;
            } // switch (event->xbutton.button)
        }
        break;
    } // switch (MMDMode_)

    lastMouseClickedPointX_ = x;
    lastMouseClickedPointY_ = y;
}

//-----------------------------------------------------------------------
// This routine function is called when a mouse is released.
//-----------------------------------------------------------------------
void MlabMissionDesign::mouseRelease(
    double x,
    double y,
    XEvent *event,
    double unitLength) 
{
    lastMouseXRoot_ = event->xbutton.x_root;
    lastMouseYRoot_ = event->xbutton.y_root;
    drawingUnitLength_ = unitLength;

    switch (MMDMode_) {

    case DESIGN_MODE:
        if (isEditingPolygon_)
        {
            switch (event->xbutton.button) {

            case LEFT_BTN:
                if (specifyingPolygon_.dragging)
                {
                    unliftPolygonVertex_(x, y);
                }
                break;
            } // switch (event->xbutton.button)
        }
        else
        {
            switch (event->xbutton.button) {

            case LEFT_BTN:
                if (taskLifted_)
                {
                    putdownLiftedTask_(x, y);
                }
                break;
            } // switch (event->xbutton.button)
        }
        break;

    case REPAIR_MODE:
        switch (event->xbutton.button) {

        case LEFT_BTN:
            if (taskLifted_)
            {
                putdownLiftedTask_(x, y);
            }
            else if (viaPointLifted_)
            {
                putdownLiftedViaPoint_(x, y);
            }
            break;
        } // switch (event->xbutton.button)
        break;
    } // switch (MMDMode_)

    lastMouseClickedPointX_ = x;
    lastMouseClickedPointY_ = y;
}

//-----------------------------------------------------------------------
// This function sets the task.
//-----------------------------------------------------------------------
void  MlabMissionDesign::setCurrentTask_(int task)
{
    MMDCurrentTask_ = task;
    updateToolboxBtn_();

    gEventLogging->log("MMD Current Task set to %d", task);
}

//-----------------------------------------------------------------------
// This function stores the overlay name in the memory.
//-----------------------------------------------------------------------
void MlabMissionDesign::saveOverlayName(char *ovlFilename)
{
    char simpleFilename[1024];

    if (filename_has_directory(ovlFilename))
    {
        remove_directory(ovlFilename, simpleFilename);
        overlayFileName_ = simpleFilename;
    }
    else
    {
        overlayFileName_ = ovlFilename;
    }
}

//-----------------------------------------------------------------------
// This function creates cursor to be used, and put them in a list.
//-----------------------------------------------------------------------
void MlabMissionDesign::createCursorList_(Pixel fg, Pixel bg, Widget parent)
{
    MMDCursor_t cursor;

    // Allocate font cursors.
    cursor.id = CURSOR_LEFT_PTR;
    cursor.cursor = XCreateFontCursor(XtDisplay(parent), XC_left_ptr);
    cursorList_.push_back(cursor);

    cursor.id = CURSOR_WATCH;
    cursor.cursor = XCreateFontCursor(XtDisplay(parent), XC_watch);
    cursorList_.push_back(cursor);

    cursor.id = CURSOR_PIRATE;
    cursor.cursor = XCreateFontCursor(XtDisplay(parent), XC_pirate);
    cursorList_.push_back(cursor);

    cursor.id = CURSOR_HAND1;
    cursor.cursor = XCreateFontCursor(XtDisplay(parent), XC_hand1);
    cursorList_.push_back(cursor);

    cursor.id = CURSOR_HAND2;
    cursor.cursor = XCreateFontCursor(XtDisplay(parent), XC_hand2);
    cursorList_.push_back(cursor);

   // Create bitmap cursors 
    cursor.id = CURSOR_POLYGON;
    cursor.cursor = MMD_CREATE_CURSOR(PolygonCursor, PolygonCursorMask, fg, bg, parent);
    cursorList_.push_back(cursor);
}

//-----------------------------------------------------------------------
// This function creates pixmaps to be used, and put them in a list.
//-----------------------------------------------------------------------
void MlabMissionDesign::createPixmapList_(Pixel fg, Pixel bg, Widget parent)
{
    MMDPixmap_t p;

    // Biohazard Pixmap
    p.name = "BiohazardBtn";
    p.pixmap = MMD_CREATE_PIXMAP(BiohazardBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(BiohazardBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Communications Pixmap
    p.name = "CommunicationsBtn";
    p.pixmap = MMD_CREATE_PIXMAP(CommunicationsBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(CommunicationsBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // EOIR Pixmap
    p.name = "EOIRBtn";
    p.pixmap = MMD_CREATE_PIXMAP(EOIRBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(EOIRBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Hostage Pixmap
    p.name = "HostageBtn";
    p.pixmap = MMD_CREATE_PIXMAP(HostageBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(HostageBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Icarus Pixmap
    p.name = "IcarusBtn";
    p.pixmap = MMD_CREATE_PIXMAP(IcarusBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(IcarusBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Inspect Pixmap
    p.name = "InspectBtn";
    p.pixmap = MMD_CREATE_PIXMAP(InspectBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(InspectBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Intercept Pixmap
    p.name = "InterceptBtn";
    p.pixmap = MMD_CREATE_PIXMAP(InterceptBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(InterceptBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Loiter Pixmap
    p.name = "LoiterBtn";
    p.pixmap = MMD_CREATE_PIXMAP(LoiterBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(LoiterBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // MAD Pixmap
    p.name = "MADBtn";
    p.pixmap = MMD_CREATE_PIXMAP(MADBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(MADBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // EOD Pixmap
    p.name = "MineBtn";
    p.pixmap = MMD_CREATE_PIXMAP(MineBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(MineBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // MineOcean Pixmap
    p.name = "MineOceanBtn";
    p.pixmap = MMD_CREATE_PIXMAP(MineOceanBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(MineOceanBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // NavalRecon Pixmap
    p.name = "NavalReconBtn";
    p.pixmap = MMD_CREATE_PIXMAP(NavalReconBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(NavalReconBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Observe Pixmap
    p.name = "ObserveBtn";
    p.pixmap = MMD_CREATE_PIXMAP(ObserveBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(ObserveBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Other Pixmap
    p.name = "OtherBtn";
    p.pixmap = MMD_CREATE_PIXMAP(OtherBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(OtherBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // SARImage Pixmap
    p.name = "SARImageBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SARImageBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SARImageBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // SearchAndRescue Pixmap
    p.name = "SearchAndRescueBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SearchAndRescueBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SearchAndRescueBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // SearchUAV Pixmap
    p.name = "SearchUAVBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SearchUAVBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SearchUAVBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // SearchUUV Pixmap
    p.name = "SearchUUVBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SearchUUVBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SearchUUVBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Sentry Pixmap
    p.name = "SentryBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SentryBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SentryBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // ShoreProtection Pixmap
    p.name = "ShoreProtectionBtn";
    p.pixmap = MMD_CREATE_PIXMAP(ShoreProtectionBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(ShoreProtectionBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Steerpoint Pixmap
    p.name = "SteerpointBtn";
    p.pixmap = MMD_CREATE_PIXMAP(SteerpointBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(SteerpointBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Observe Pixmap
    p.name = "TrackBtn";
    p.pixmap = MMD_CREATE_PIXMAP(TrackBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(TrackBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // Waypoints Pixmap
    p.name = "WaypointsBtn";
    p.pixmap = MMD_CREATE_PIXMAP(WaypointsBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(WaypointsBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);

    // WeaponRelease Pixmap
    p.name = "WeaponReleaseBtn";
    p.pixmap = MMD_CREATE_PIXMAP(WeaponReleaseBtn, fg, bg, parent);
    p.insensitivePixmap = MMD_CREATE_PIXMAP(WeaponReleaseBtn, gColorPixel.gray80, bg, parent);
    pixmapList_.push_back(p);
}

//-----------------------------------------------------------------------
// This function returns pixmap (or insensitive pixmap) from the list.
//-----------------------------------------------------------------------
Pixmap MlabMissionDesign::getPixmapFromList_(string name, bool insensitive)
{
    Pixmap p;
    int i;

    for (i = 0; i < ((int)(pixmapList_.size())); i++)
    {
        if (pixmapList_[i].name == name)
        {
            if (insensitive)
            {
                p = pixmapList_[i].insensitivePixmap;
            }
            else
            {
                p = pixmapList_[i].pixmap;
            }
            
            break;
        }
    }

    return p;
}

//-----------------------------------------------------------------------
// This function returns cursor  from the list.
//-----------------------------------------------------------------------
Cursor MlabMissionDesign::getCursorFromList_(int id)
{
    Cursor cursor;
    int i;

    for (i = 0; i < ((int)(cursorList_.size())); i++)
    {
        if (cursorList_[i].id == id)
        {
            cursor = cursorList_[i].cursor;
            break;
        }
    }

    return cursor;
}

//-----------------------------------------------------------------------
// This function assigns the default task to be the ID for the default
// task.
//-----------------------------------------------------------------------
int MlabMissionDesign::getDefaultCurrentTask_(void)
{
    string taskName;
    int taskID;
    const bool DEBUG_GET_DEFAULT_CURRENT_TASK = false;

    if (missionSpecWizardEnabled())
    {
        taskName = icarusTaskName_;
    }
    else
    {
        switch (cnpMode_) {

        case CNP_MODE_RUNTIME:
            // For runtime-CNP, use bidder as the default.
            taskName = runtimeCNPBidderTaskName_;
            break;

        default:
            // Use the generic one.
            taskName = defaultTaskName_;
            break;
        }
    }

    taskID = getTaskID_(taskName);

    if (taskID == INVALID_TASK_ID_)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::getDefaultCurrentTask_(). Default task (%s) not found.\a\n",
            taskName.c_str());

        // Assign 0 then.
        taskID = 0;
    }

    if (DEBUG_GET_DEFAULT_CURRENT_TASK)
    {
        fprintf(
            stderr,
            "MlabMissionDesign::getDefaultCurrentTask_(). Default Task [%s] = %d.\n",
            taskName.c_str(),
            taskID);
    }

    return taskID;
}

//-----------------------------------------------------------------------
// This function retrieves the task ID from the task name.
//-----------------------------------------------------------------------
int MlabMissionDesign::getTaskID_(string taskName)
{
    int i, taskID;

    taskID = INVALID_TASK_ID_;

    for (i = 0; i < ((int)(defaultTaskList_.size())); i++)
    {
        if (defaultTaskList_[i].name == taskName)
        {
            taskID = defaultTaskList_[i].id;
            break;
        }
    }

    return taskID;
}

//-----------------------------------------------------------------------
// This function initializs the CBRClient class. 
//-----------------------------------------------------------------------
bool MlabMissionDesign::initCBRClient_(void)
{
    bool initialized = false;

    mlabCBRClient_ = new MlabCBRClient(rc_table, CBRPLANNER_CLIENT_MLAB_MMD);
    cbrclient_ = (CBRClient *)mlabCBRClient_;

    initialized = cbrclient_->initialize();

    if (!initialized)
    {
        warn_userf("CBRServer unreachable. Please make sure it is running.");
        return false;
    }

    cbrClientInstantiated_ = true;

    return true;
}

//-----------------------------------------------------------------------
// This function sends the logfile to CfgEdit (via CBRServer).
//-----------------------------------------------------------------------
void MlabMissionDesign::saveLogfile(string logfileName)
{
    CBRPlanner_LogfileInfo_t logfileInfo;

    if (!cbrClientInstantiated_)
    {
        return;
    }

    logfileInfo.name = logfileName;

    mlabCBRClient_->saveLogfileInfo(logfileInfo);
}

//-----------------------------------------------------------------------
// This function drags a circle, following the pointer.
//-----------------------------------------------------------------------
void MlabMissionDesign::dragCircle_(void)
{
    Window rootWindow, childWindow;
    double x, y, lastX, lastY, radius;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    radius = draggingCircle_.radius;

    if (draggingCircle_.drawn)
    {
        // Erase the previous one.
        lastX = draggingCircle_.lastPos.x;
        lastY = draggingCircle_.lastPos.y;
        DrawCircle(lastX, lastY, radius, gGCs.XOR);
        DrawCircle(lastX, lastY, radius, gGCs.whiteXOR);
        draggingCircle_.drawn = false;
    }

    if (!draggingCircle_.dragging)
    {
        XtAppLock(appContext_);
        XLockDisplay(display_);
        clear_map();
        draw_world();
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);
        return;
    }

    XQueryPointer(
        display_,
        XtWindow(drawing_area),
        &rootWindow,
        &childWindow,
        &rootX,
        &rootY,
        &winX,
        &winY,
        &mask);

    x = (winX * meters_per_pixel) + origin_x;
    y = (drawing_area_height_pixels * meters_per_pixel) -
        (winY * meters_per_pixel) + origin_y;

    DrawCircle(x, y, radius, gGCs.XOR);
    DrawCircle(x, y, radius, gGCs.whiteXOR);

    draggingCircle_.drawn = true;
    draggingCircle_.lastPos.x = x;
    draggingCircle_.lastPos.y = y;

    XtAppAddTimeOut(
        appContext_,
        DRAG_UPDATE_TIME_MSEC_,
        (XtTimerCallbackProc)cbDragCircle_,
        (void *)this);
}

//-----------------------------------------------------------------------
// This function drags a polygon point.
//-----------------------------------------------------------------------
void MlabMissionDesign::dragPolygonPoint_(void)
{
    Window rootWindow, childWindow;
    MExpPoint2D_t pos1, pos2;
    int rootX, rootY, winX, winY;
    unsigned int mask;

    if (specifyingPolygon_.posList.size() == 0)
    {
        return;
    }

    // Erase the previous one.
    if (specifyingPolygon_.drawn)
    {
        pos1 = specifyingPolygon_.posList.back();
        pos2 = specifyingPolygon_.lastMousePos;
        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.XOR);
        DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.whiteXOR);
        specifyingPolygon_.drawn = false;
    }

    if (!specifyingPolygon_.dragging)
    {
        XtAppLock(appContext_);
        XLockDisplay(display_);
        clear_map();
        draw_world();
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);
        return;
    }

    XQueryPointer(
        display_,
        XtWindow(drawing_area),
        &rootWindow,
        &childWindow,
        &rootX,
        &rootY,
        &winX,
        &winY,
        &mask);

    pos2.x = (winX * meters_per_pixel) + origin_x;
    pos2.y = (drawing_area_height_pixels * meters_per_pixel) -
        (winY * meters_per_pixel) + origin_y;

    pos1 = specifyingPolygon_.posList.back();
    DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.XOR);
    DrawLine(pos1.x, pos1.y, pos2.x, pos2.y, gGCs.whiteXOR);
    specifyingPolygon_.drawn = true;

    specifyingPolygon_.lastMousePos = pos2;

    XtAppAddTimeOut(
        appContext_,
        DRAG_UPDATE_TIME_MSEC_,
        (XtTimerCallbackProc)cbDragPolygonPoint_,
        (void *)this);
}

//-----------------------------------------------------------------------
// This function drags a polygon vertex.
//-----------------------------------------------------------------------
void MlabMissionDesign::liftPolygonVertex_(MMDPolygon_t poly) 
{
    // Clear the screen and redraw
    XtAppLock(appContext_);
    XLockDisplay(display_);
    clear_map();
    draw_world();
    XUnlockDisplay(display_);
    XtAppUnlock(appContext_);

    if ((int)(poly.posList.size()) > (poly.selectedPosIndex))
    {
        poly.lastMousePos = poly.posList[poly.selectedPosIndex];
        poly.dragging = true;
        poly.drawn = false;
    }
    else
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::liftPolygonVertex_(). Corrupted data.\n");
        return;
    }

    clearPolygon_(&specifyingPolygon_);
    specifyingPolygon_ = poly;

    dragPolygonVertex_();
}

//-----------------------------------------------------------------------
// This function puts down the dragged polygon vertex.
//-----------------------------------------------------------------------
void MlabMissionDesign::unliftPolygonVertex_(double x, double y) 
{
    MMDPolygon_t poly;
    MExpPoint2D_t pos;
    MExpLatLon_t geo;
    int i, index;

    specifyingPolygon_.dragging = false;

    // Wait until the dragPolygon_() finishes.
    while (specifyingPolygon_.drawn)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    index = specifyingPolygon_.selectedPosIndex;
    pos.x = x;
    pos.y = y;

    global2geographic(
        pos.x,
        pos.y,
        &(geo.latitude),
        &(geo.longitude));

    eraseSpecifiedPolygons_();

    // Update the data.
    for (i = 0; i < (int)(specifiedPolygonList_.size()); i++)
    {
        poly = specifiedPolygonList_[i];

        if ((poly.featureType == specifyingPolygon_.featureType) &&
            (poly.featureId == specifyingPolygon_.featureId) &&
            (poly.number == specifyingPolygon_.number))
        {
            if ((int)(poly.posList.size()) <= index)
            {
                fprintf(
                    stderr,
                    "Error: MlabMissionDesign::unliftPolygonVertex_(). Corrupted data.\n");
                return;
            }

            poly.posList[index] = pos;
            poly.geoList[index] = geo;
            specifiedPolygonList_[i] = poly;
            
            switch (poly.featureType) {

            case MEXP_FEATURE_GLOBAL:
                defaultGlobalFeatures_[poly.featureId].options[poly.number].value = 
                    encodePolygon_(poly);
                break;
            }

            break;
        }
    }

    drawSpecifiedPolygons_();

    resetEditingPolygons_(
        specifyingPolygon_.featureType,
        specifyingPolygon_.featureId);

    clearPolygon_(&specifyingPolygon_);
}

//-----------------------------------------------------------------------
// This function removes a polygon.
//-----------------------------------------------------------------------
void MlabMissionDesign::removePolygon_(MMDPolygon_t poly) 
{
    vector<MMDPolygon_t> newPolyList;
    vector<MExpFeatureOption_t> newOptions;
    MExpFeatureOption_t featureOption;
    MMDPolygon_t specifiedPoly;
    char buf[1024];
    int i;;

    specifyingPolygon_.dragging = false;

    // Wait until the dragPolygon_() finishes.
    while (specifyingPolygon_.drawn)
    {
        XtAppProcessEvent(appContext_, XtIMAll);
    }

    // Update the polygon data.
    for (i = 0; i < (int)(specifiedPolygonList_.size()); i++)
    {
        specifiedPoly = specifiedPolygonList_[i];

        if ((poly.featureType == specifiedPoly.featureType) &&
            (poly.featureId == specifiedPoly.featureId))
        {
            if (poly.number == specifiedPoly.number)
            {
                continue;
            }

            if (poly.number < specifiedPoly.number)
            {
                // Adjust the polygon number.
                specifiedPoly.number--;
            }

            // Add to the new list.
            newPolyList.push_back(specifiedPoly);
        }
        else
        {
            newPolyList.push_back(specifiedPoly);
        }
    }

    // Update feature data and the widget.
    newOptions.clear();
    switch (poly.featureType) {

    case MEXP_FEATURE_GLOBAL:

        for (i = 0; i < (int)(defaultGlobalFeatures_[poly.featureId].options.size()); i++)
        {
            if (i != poly.number)
            {
                newOptions.push_back(
                    defaultGlobalFeatures_[poly.featureId].options[i]);
            }
        }

        defaultGlobalFeatures_[poly.featureId].options = newOptions;
        defaultGlobalFeatures_[poly.featureId].selectedOption = newOptions.size();

        // Update the number of the polygons in the widget.
        sprintf(buf, "%d", newOptions.size());
        XtVaSetValues(
            globalFeatureMenuOption_w_[poly.featureId][0],
            XmNvalue, buf,
            NULL);
        break;

    default:
        break;
    }


    eraseSpecifiedPolygons_();
    specifiedPolygonList_ = newPolyList;
    drawSpecifiedPolygons_();

    clearPolygon_(&specifyingPolygon_);
    resetEditingPolygons_(poly.featureType, poly.featureId);
}

//-----------------------------------------------------------------------
// This function drags a polygon vertex.
//-----------------------------------------------------------------------
void MlabMissionDesign::dragPolygonVertex_(void) 
{
    MExpPoint2D_t pos, posL, posU;
    Window rootWindow, childWindow;
    int rootX, rootY, winX, winY;
    int numPoints;
    int index;
    unsigned int mask;

    numPoints = specifyingPolygon_.posList.size();

    // Pick 
    if (numPoints <= specifyingPolygon_.selectedPosIndex)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::dragPolygonVertex_(). Corrupted data.\n");
        return;
    }

    // Upper vertex
    index = specifyingPolygon_.selectedPosIndex + 1;
    index = (index < numPoints)? index : 0;
    posU = specifyingPolygon_.posList[index];

    // Lower vertex
    index = specifyingPolygon_.selectedPosIndex - 1;
    index = (index >= 0)? index : (numPoints - 1);
    posL = specifyingPolygon_.posList[index];

    // Erase the previous one.
    if (specifyingPolygon_.drawn)
    {
        pos = specifyingPolygon_.lastMousePos;
        DrawLine(pos.x, pos.y, posU.x, posU.y, gGCs.XOR);
        DrawLine(pos.x, pos.y, posU.x, posU.y, gGCs.whiteXOR);
        DrawLine(pos.x, pos.y, posL.x, posL.y, gGCs.XOR);
        DrawLine(pos.x, pos.y, posL.x, posL.y, gGCs.whiteXOR);
        specifyingPolygon_.drawn = false;
    }

    if (!specifyingPolygon_.dragging)
    {
        XtAppLock(appContext_);
        XLockDisplay(display_);
        clear_map();
        draw_world();
        XUnlockDisplay(display_);
        XtAppUnlock(appContext_);
        return;
    }

    XQueryPointer(
        display_,
        XtWindow(drawing_area),
        &rootWindow,
        &childWindow,
        &rootX,
        &rootY,
        &winX,
        &winY,
        &mask);

    pos.x = (winX * meters_per_pixel) + origin_x;
    pos.y = (drawing_area_height_pixels * meters_per_pixel) -
        (winY * meters_per_pixel) + origin_y;

    DrawLine(pos.x, pos.y, posU.x, posU.y, gGCs.XOR);
    DrawLine(pos.x, pos.y, posU.x, posU.y, gGCs.whiteXOR);
    DrawLine(pos.x, pos.y, posL.x, posL.y, gGCs.XOR);
    DrawLine(pos.x, pos.y, posL.x, posL.y, gGCs.whiteXOR);

    specifyingPolygon_.drawn = true;
    specifyingPolygon_.lastMousePos = pos;

    XtAppAddTimeOut(
        appContext_,
        DRAG_UPDATE_TIME_MSEC_,
        (XtTimerCallbackProc)cbDragPolygonVertex_,
        (void *)this);
}

//-----------------------------------------------------------------------
// This function quits mlab.
//-----------------------------------------------------------------------
void MlabMissionDesign::exit_(int exitMode)
{
    returnConfigureMission_ = exitMode;

    // If no task was placed, and the user is trying to configure it, verify if
    // the user is OK with just quit.
    if ((exitMode == MissionExpert::CONFIGURE) &&
        (numTasks_ == 0))
    {
        if (!confirmUser_(CONFIRM_EMPTY_MISSION_PHRASE_, false))
        {
            return;
        }
    }

    quit_mlab();
}

//-----------------------------------------------------------------------
// This function backs up a file.
//-----------------------------------------------------------------------
int MlabMissionDesign::backupFile_(string filename)
{
    FILE *origFile = NULL;
    char cmdBuf[1024];
    int status;

    origFile = fopen(filename.c_str(), "r");

    if (origFile == NULL)
    {
        return BACKUP_ORIGINAL_FILE;
    }

    fclose(origFile);
    origFile = NULL;

    sprintf(
        cmdBuf,
        "cp -f %s %s.%s",
        filename.c_str(),
        filename.c_str(),
        MEXP_EXTENSION_BACKUP.c_str());

    status = system(cmdBuf);

    return BACKUP_SUCCESS;
}

//-----------------------------------------------------------------------
// This function gets mobility type from the robot constraints
//-----------------------------------------------------------------------
int MlabMissionDesign::getMobilityTypeFromRobotConstraints(int robotId)
{
    MExpRobotConstraints_t robotConstraints;
    MExpFeature_t constraint;
    string mobilityTypeString;
    int i, j, k;
    int numConstraints, selectedOption;
    int mobilityType = INVALID_MOBILITY_TYPE_;

    for (i = 0; i < (int)(robotConstraintsList_.size()); i++)
    {
        robotConstraints = robotConstraintsList_[i];

        if ((robotConstraints.id) == robotId)
        {
            numConstraints = robotConstraints.constraints.size();

            for (j = 0; j < numConstraints; j++)
            {
                constraint = robotConstraints.constraints[j];

                if ((constraint.name) == STRING_VEHICLE_TYPES_)
                {
                    selectedOption = (int)(constraint.selectedOption);

                    if ((selectedOption >= 0) &&
                        (selectedOption < (int)(constraint.options.size())))
                    {
                        mobilityTypeString = 
                            constraint.options[selectedOption].value;
                        
                        mobilityType = UXV;

                        for (k = 0; k < NUM_ROBOT_MOBILITY_TYPES; k++)
                        {
                            if (mobilityTypeString == ROBOT_MOBILITY_TYPE_NAMES[k])
                            {
                                mobilityType = k;
                            }
                        }
                    }

                    break;
                }
            }

            break;
        }
    }

    return mobilityType;
}

//-----------------------------------------------------------------------
// This function retrieves the smallest ID of the robot in the CNP robot
// constraints list that is not being deployed yet.
//-----------------------------------------------------------------------
int MlabMissionDesign::getSmallestDeployableRuntimeCNPRobotID_(void)
{
    MExpRobotConstraints_t robotConstraints;
    int smallestRobotID;
    int i, j, numRobots;
    bool robotFound = false;

    numRobots = robotConstraintsList_.size();
    smallestRobotID = INT_MAX;

    for (i = 0; i < numRobots; i++)
    {
        robotConstraints = robotConstraintsList_[i];
        robotFound = false;

        for (j = 0; j < (int)(deployedRuntimeCNPRobotList_.size()); j++)
        {
            if ((robotConstraints.id) == deployedRuntimeCNPRobotList_[j].robotID)
            {
                robotFound = true;
                break;
            }
        }

        if (!robotFound)
        {
            if (smallestRobotID > robotConstraints.id)
            {
                smallestRobotID = robotConstraints.id;
            }
            break;
        }
    }

    if (smallestRobotID == INT_MAX)
    {
        // We did not find any undeployed robot.
        smallestRobotID = INVALID_ROBOT_ID_;
    }

    return smallestRobotID;
}

//-----------------------------------------------------------------------
// This function adds the runtime CNP MissionManager robot.
//-----------------------------------------------------------------------
void MlabMissionDesign::placeRuntimeCNPMissionManager_(void)
{
    gt_Point startPlace;
    double posX, posY;

    if (gtGetStartPlace(&startPlace) == GT_FAILURE)
    {
        warn_userf("StartPlace not found in the overlay. Runtime-CNP\nMissionManager could be placed.");
        return;
    }

    posX = startPlace.x;
    posY = startPlace.y;

    runtimeCNPMissionManagerTask_ = getTaskID_(STRING_RUNTIME_CNP_MISSION_MANAGER_TASK_);

    if (runtimeCNPMissionManagerTask_ == INVALID_TASK_ID_)
    {
        warn_userf("Configuration for Runtime-CNP MissionManager not found.");
        return;
    }

    placeTask_(posX, posY, runtimeCNPMissionManagerTask_);
}

//-----------------------------------------------------------------------
// This function adds the ICARUS task.
//-----------------------------------------------------------------------
void MlabMissionDesign::placeIcarusTask_(void)
{
    gt_Point startPlace;
    double posX, posY;

    if (gtGetStartPlace(&startPlace) == GT_FAILURE)
    {
        warn_userf("StartPlace not found in the overlay. Runtime-CNP\nMissionManager could be placed.");
        return;
    }

    posX = startPlace.x;
    posY = startPlace.y;

    icarusTask_ = getTaskID_(STRING_ICARUS_TASK_);

    if (icarusTask_ == INVALID_TASK_ID_)
    {
        warn_userf("Configuration for ICARUS task not found.");
        return;
    }

    placeTask_(posX, posY, icarusTask_);
}

//-----------------------------------------------------------------------
// This function adds a deployed robot to the deployed list
//-----------------------------------------------------------------------
void MlabMissionDesign::deployRuntimeCNPRobot_(int robotID, MExpMissionTask_t task)
{
    DeployedRuntimeCNPRobot_t deployedRuntimeCNPRobot;
    int i, numRobots;
    bool douplicated = false;
    
    // Check for duplication.
    for (i = 0; i < (int)(deployedRuntimeCNPRobotList_.size()); i++)
    {
        if ((deployedRuntimeCNPRobotList_[i].robotID) == robotID)
        {
            douplicated = true;
            break;
        }

        if ((deployedRuntimeCNPRobotList_[i].task.number) == task.number)
        {
            douplicated = true;
            break;
        }
    }

    if (douplicated)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::deployRuntimeCNPRobot_(). Invalid input. New entry not saved.\a\n");
        return;
    }

    deployedRuntimeCNPRobot.robotID = robotID;
    deployedRuntimeCNPRobot.task = task;
    deployedRuntimeCNPRobotList_.push_back(deployedRuntimeCNPRobot);

    // Save the number of the robots.
    numRobots = deployedRuntimeCNPRobotList_.size();

    if (globalFeatureRobotNumIndex_ >= 0)
    {
        defaultGlobalFeatures_[globalFeatureRobotNumIndex_].selectedOption = 
            (float)numRobots;
    }
    else
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::deployRuntimeCNPRobot_(). globalFeatureRobotNumIndex_ invalid.\a\n");
    }
}

//-----------------------------------------------------------------------
// This function removes a deployed robot from the deployed list
//-----------------------------------------------------------------------
void MlabMissionDesign::undeployRuntimeCNPRobot_(int robotID)
{
    DeployedRuntimeCNPRobot_t deployedRuntimeCNPRobot;
    vector<DeployedRuntimeCNPRobot_t> deployedRuntimeCNPRobotList;
    int i, numRobots;
    bool removed = false;

    for (i = 0; i < (int)(deployedRuntimeCNPRobotList_.size()); i++)
    {
        deployedRuntimeCNPRobot = deployedRuntimeCNPRobotList_[i];

        if ((deployedRuntimeCNPRobot.robotID) != robotID)
        {
            deployedRuntimeCNPRobotList.push_back(deployedRuntimeCNPRobot);
        }
        else
        {
            removed = true;
        }
    }

    deployedRuntimeCNPRobotList_ = deployedRuntimeCNPRobotList;

    if (!removed)
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::undeployRuntimeCNPRobot_(). Robot(%d) not removed.\a\n",
            robotID);
    }

    // Save the number of the robots.
    numRobots = deployedRuntimeCNPRobotList_.size();

    if (globalFeatureRobotNumIndex_ >= 0)
    {
        defaultGlobalFeatures_[globalFeatureRobotNumIndex_].selectedOption = 
            (float)numRobots;
    }
    else
    {
        fprintf(
            stderr,
            "Error: MlabMissionDesign::undeployRuntimeCNPRobot_(). globalFeatureRobotNumIndex_ invalid.\a\n");
    }
}

//-----------------------------------------------------------------------
// This function returns the robot ID of the deployed robot from the
// task number. Assumption: Every task number and robot ID matching is
// unique.
//-----------------------------------------------------------------------
int MlabMissionDesign::getDeployedRuntimeCNPRobotIDFromTaskNumber_(int taskNumber)
{
    DeployedRuntimeCNPRobot_t deployedRuntimeCNPRobot;
    int robotID = INVALID_ROBOT_ID_;
    int i;
    bool DEBBUG_GDRCNPRIDFTN = false;

    if (DEBBUG_GDRCNPRIDFTN)
    {
        fprintf(
            stderr,
            "MlabMissionDesign::getDeployedRuntimeCNPRobotIDFromTaskNumber_():\n");
        fprintf(
            stderr,
            " Finding Task(%d):\n",
            taskNumber);
    }

    for (i = 0; i < (int)(deployedRuntimeCNPRobotList_.size()); i++)
    {
        deployedRuntimeCNPRobot = deployedRuntimeCNPRobotList_[i];

        if (DEBBUG_GDRCNPRIDFTN)
        {
            fprintf(
                stderr,
                " - List [%d]: Task(%d)\n",
                i,
                deployedRuntimeCNPRobot.task.number);
        }

        if ((deployedRuntimeCNPRobot.task.number) == taskNumber)
        {
            robotID = deployedRuntimeCNPRobot.robotID;
            break;
        }
    }

    return robotID;
}

//-----------------------------------------------------------------------
// This function returns the deployed robot from the robot ID.
//-----------------------------------------------------------------------
MlabMissionDesign::DeployedRuntimeCNPRobot_t MlabMissionDesign::getDeployedRuntimeCNPRobotFromRobotID_(
    int robotID)
{
    DeployedRuntimeCNPRobot_t deployedRuntimeCNPRobot;
    int i;
    bool found = false;

    for (i = 0; i < (int)(deployedRuntimeCNPRobotList_.size()); i++)
    {

        if ((deployedRuntimeCNPRobotList_[i].robotID) == robotID)
        {
            deployedRuntimeCNPRobot = deployedRuntimeCNPRobotList_[i];
            found = true;
            break;
        }
    }

    if (!found)
    {
        deployedRuntimeCNPRobot.robotID = INVALID_ROBOT_ID_;
        deployedRuntimeCNPRobot.task.id = INVALID_TASK_ID_;
        deployedRuntimeCNPRobot.task.number = INVALID_TASK_NUMBER_;

        fprintf(
            stderr,
            "Error: MlabMissionDesign::getDeployedRuntimeCNPRobotFromRobotID_(). Bad Robot ID: %d.\a\n",
            robotID);
    }

    return deployedRuntimeCNPRobot;
}

//-----------------------------------------------------------------------
// This function returns true if specified robot ID is in the deployed
// list.
//-----------------------------------------------------------------------
bool MlabMissionDesign::isDeployedRuntimeCNPRobot_(int robotID)
{
    DeployedRuntimeCNPRobot_t deployedRuntimeCNPRobot;
    int i;
    bool deployed = false;

    for (i = 0; i < (int)(deployedRuntimeCNPRobotList_.size()); i++)
    {
        deployedRuntimeCNPRobot = deployedRuntimeCNPRobotList_[i];

        if ((deployedRuntimeCNPRobot.robotID) == robotID)
        {
            deployed = true;
            break;
        }
    }

    return deployed;
}

//-----------------------------------------------------------------------
// This function returns the robot constraints
//-----------------------------------------------------------------------
MExpRobotConstraints_t MlabMissionDesign::getRobotConstraints_(int robotId)
{
    MExpRobotConstraints_t robotConstraints;
    int i;

    for (i = 0; i < (int)(robotConstraintsList_.size()); i++)
    {
        if ((robotConstraintsList_[i].id) == robotId)
        {
            robotConstraints = robotConstraintsList_[i];
            break;
        }
    }

    return robotConstraints;
}

//-----------------------------------------------------------------------
// This function returns the robot constraints in the format specified
// in CNPConstraint.h.
//-----------------------------------------------------------------------
vector<CNP_RobotConstraint> MlabMissionDesign::getCNPRobotConstraints(int robotId)
{
    MExpRobotConstraints_t robotConstraints;
    MExpFeature_t constraint;
    CNP_RobotConstraint cnpRobotConstraint;
    vector<CNP_RobotConstraint> cnpRobotConstraints;
    int i, numConstraints, selectedOption;
    const CNP_RobotConstraint EMPTY_CNP_ROBOT_CONSTRAINT = {0, string(), CNP_STRING, string()};

    // Get the robot constraints in MExpConstraints_t format.
    robotConstraints = getRobotConstraints_(robotId);

    // Count the number of constraints.
    numConstraints = robotConstraints.constraints.size();

    for (i = 0; i < numConstraints; i++)
    {
        // Copy the data.
        constraint = robotConstraints.constraints[i];
        cnpRobotConstraint = EMPTY_CNP_ROBOT_CONSTRAINT;
        cnpRobotConstraint.iRobotID = robotId;
        cnpRobotConstraint.strConstraintName = constraint.name;

        selectedOption = (int)(constraint.selectedOption);

        if ((selectedOption >= 0) &&
            (selectedOption < (int)(constraint.options.size())))
        {
            cnpRobotConstraint.strConstraintValue = 
                constraint.options[selectedOption].value;

            // Value found. Save it.
            cnpRobotConstraints.push_back(cnpRobotConstraint);
        }
    }

    return cnpRobotConstraints;
}

//-----------------------------------------------------------------------
// This callback function sets the current task.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbSetCurrentTask_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;
    int task;

    mlabMissionDesignInstance = ((MMDToolBoxSetCurrentTaskCallbackData_t *)client_data)->mlabMissionDesignInstance;
    task = ((MMDToolBoxSetCurrentTaskCallbackData_t *)client_data)->task;

    mlabMissionDesignInstance->setCurrentTask_(task);
}

//-----------------------------------------------------------------------
// This callback function changes global preference menu value.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbChangeGlobalFeatureMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = ((MMDGlobalFeatureMenuCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mlabMissionDesignInstance->changeGlobalFeatureMenuValue_(w, client_data);
}

//-----------------------------------------------------------------------
// This callback function gets called when "Apply" button in the Right-
// Button Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbApplyRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;
    mlabMissionDesignInstance->applyRightBtnMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when "Apply" button in the Robot
// Constraint Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbApplyRobotConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;
    mlabMissionDesignInstance->applyRobotConstraintMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when "Cancel" button in the Right-
// Button Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbCancelRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;
    mlabMissionDesignInstance->cancelRightBtnMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when "Cancel" button in the Robot
// Constraiont Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbCancelRobotConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;
    mlabMissionDesignInstance->cancelRobotConstraintMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when a toggle button in the Right-
// Button Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbChangeRightBtnMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = ((MMDRightBtnMenuCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mlabMissionDesignInstance->changeRightBtnMenuValue_(client_data);
}

//-----------------------------------------------------------------------
// This callback function gets called when a toggle button in the Robot
// Constraint Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbChangeRobotConstraintMenuValue_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = ((MMDRobotConstraintMenuCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mlabMissionDesignInstance->changeRobotConstraintMenuValue_(client_data);
}

//-----------------------------------------------------------------------
// This callback function gets called when "Next" or "Prev" button in the
// Robot Constraint Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbNavigateConstraintMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = ((MMDNavRobotConstraintMenuCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mlabMissionDesignInstance->navigateRobotConstraintMenu_(client_data);
}

//-----------------------------------------------------------------------
// This callback function gets called when "Next" or "Prev" button in the
// Right Button Menu is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbNavigateRightBtnMenu_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = ((MMDNavRightBtnMenuCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mlabMissionDesignInstance->navigateRightBtnMenu_(client_data);
}

//-----------------------------------------------------------------------
// This callback function gets called when "Global Settings"
// button in the toolbar is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbToolboxGlobalSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;
    XtDestroyWidget(mlabMissionDesignInstance->globalFeatureMenu_w_);
    mlabMissionDesignInstance->createGlobalFeatureMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when "Robot Settings"
// button in the toolbar is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbToolboxRobotSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;

    if (mlabMissionDesignInstance->rightBtnMenuIsUp_)
    {
        mlabMissionDesignInstance->closeRightBtnMenu_();
    }

    if (mlabMissionDesignInstance->robotConstraintMenuIsUp_)
    {
        mlabMissionDesignInstance->closeRobotConstraintMenu_();
    }

    mlabMissionDesignInstance->popUpRobotConstraintMenu_();
}

//-----------------------------------------------------------------------
// This callback function gets called when "Mission Settings"
// button in the toolbar is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbToolboxMissionSettingBtn_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;

    mlabMissionDesignInstance = (MlabMissionDesign *)client_data;

    if (mlabMissionDesignInstance->rightBtnMenuIsUp_)
    {
        mlabMissionDesignInstance->closeRightBtnMenu_();
    }

    if (mlabMissionDesignInstance->robotConstraintMenuIsUp_)
    {
        mlabMissionDesignInstance->closeRobotConstraintMenu_();
    }

    if (mlabMissionDesignInstance->missionSpecWizardEnabled())
    {
        mlabMissionDesignInstance->popUpIcarusTaskMenu_();
    }
    else
    {
        mlabMissionDesignInstance->popUpRuntimeCNPMissionManagerMenu_();
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when either "Finish" or "Cancel &
// Return" button in the toolbar is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbExit_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;
    int exitMode;

    mlabMissionDesignInstance = ((MMDToolBoxSetCurrentTaskCallbackData_t *)client_data)->mlabMissionDesignInstance;
    exitMode = ((MMDToolBoxSetCurrentTaskCallbackData_t *)client_data)->task;

    mlabMissionDesignInstance->exit_(exitMode);
}

//-----------------------------------------------------------------------
// This callback function gets called when either "Fix It!" or "Cancel &
// Return" button in the repair window is pressed.
//-----------------------------------------------------------------------
void MlabMissionDesign::cbRepair_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    MlabMissionDesign *mlabMissionDesignInstance = NULL;
    int mode;

    mlabMissionDesignInstance = ((MMDRepairWindowCallbackData_t *)client_data)->mlabMissionDesignInstance;
    mode = ((MMDRepairWindowCallbackData_t *)client_data)->mode;

    if (mode == (MissionExpert::REPAIR))
    {
        mlabMissionDesignInstance->repairMission_();
    }
    else
    {
        mlabMissionDesignInstance->exit_(MissionExpert::CANCEL);
        gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_APPLY_CHANGE_NO);
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when a circle is dragged.
//-----------------------------------------------------------------------
XtTimerCallbackProc MlabMissionDesign::cbDragCircle_(XtPointer clientData)
{
    MlabMissionDesign *mlabMissionDesignInstance;

    mlabMissionDesignInstance = (MlabMissionDesign *)clientData;
    mlabMissionDesignInstance->dragCircle_();

    return false;
}

//-----------------------------------------------------------------------
// This callback function gets called when a polygon point is dragged.
//-----------------------------------------------------------------------
XtTimerCallbackProc MlabMissionDesign::cbDragPolygonPoint_(XtPointer clientData)
{
    MlabMissionDesign *mlabMissionDesignInstance;

    mlabMissionDesignInstance = (MlabMissionDesign *)clientData;
    mlabMissionDesignInstance->dragPolygonPoint_();

    return false;
}

//-----------------------------------------------------------------------
// This callback function gets called when a polygon vertex is dragged.
//-----------------------------------------------------------------------
XtTimerCallbackProc MlabMissionDesign::cbDragPolygonVertex_(XtPointer clientData)
{
    MlabMissionDesign *mlabMissionDesignInstance;

    mlabMissionDesignInstance = (MlabMissionDesign *)clientData;
    mlabMissionDesignInstance->dragPolygonVertex_();

    return false;
}

/**********************************************************************
 * $Log: mission_design.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.18  2007/09/29 23:44:44  endo
 * Global feature can be now disabled.
 *
 * Revision 1.17  2007/09/18 22:37:00  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.16  2007/06/28 03:52:36  endo
 * For 06/27/2007 demo.
 *
 * Revision 1.15  2007/05/15 18:45:03  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.14  2007/05/04 17:50:45  endo
 * ICARUS icons added.
 *
 * Revision 1.13  2007/02/13 11:03:04  endo
 * Aesthetic clean up.
 *
 * Revision 1.12  2007/02/08 20:22:25  endo
 * Polygon's Edit option added.
 *
 * Revision 1.11  2007/02/08 19:40:43  pulam
 * Added handling of zones (no-fly, weather)
 * Fixed up some of the icarus objectives to add turn limiting
 *
 * Revision 1.10  2007/01/29 15:09:42  endo
 * Minor code clean up.
 *
 * Revision 1.9  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.8  2006/10/24 22:03:47  endo
 * Last minute change for October demo.
 *
 * Revision 1.7  2006/10/23 22:19:10  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.6  2006/10/02 19:26:07  endo
 * ICARUS Loiter and Communication icons added.
 *
 * Revision 1.5  2006/10/02 19:20:40  endo
 * ICARUS Loiter and Communication icons added.
 *
 * Revision 1.4  2006/09/26 18:33:50  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.3  2006/09/21 14:19:19  endo
 * Default value for returnConfigureMission_ for RUN_MODE changed to CONFIGURE.
 *
 * Revision 1.2  2006/09/13 19:06:53  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.18  2006/07/10 06:49:00  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.17  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.16  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.15  2006/03/01 09:49:19  endo
 * Check-in for Type-I Intercept Experiment.
 *
 * Revision 1.14  2006/02/19 17:58:00  endo
 * Experiment related modifications
 *
 * Revision 1.13  2006/02/14 02:28:20  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.12  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.11  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.10  2005/10/21 00:29:30  endo
 * Event Log for GOMS Test.
 *
 * Revision 1.9  2005/08/22 22:17:22  endo
 * Robot ID is now displayed in Robot Settings window.
 *
 * Revision 1.8  2005/08/09 19:14:00  endo
 * Things improved for the August demo.
 *
 * Revision 1.7  2005/07/30 02:17:47  endo
 * 3D visualization improved.
 *
 * Revision 1.6  2005/07/27 20:38:10  endo
 * 3D visualization improved.
 *
 * Revision 1.5  2005/07/19 20:01:07  endo
 * Bug fix on task/robot constraints specification.
 *
 * Revision 1.4  2005/07/16 08:44:07  endo
 * CBR-CNP integration
 *
 * Revision 1.3  2005/05/18 21:09:48  endo
 * AuRA.naval added.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.4  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.3  2003/04/06 09:03:48  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.2  2002/01/31 10:51:28  endo
 * Parameters chaged due to the change of max_vel and base_vel.
 *
 * Revision 1.1  2002/01/12 22:54:39  endo
 * Initial revision
 *
 **********************************************************************/
