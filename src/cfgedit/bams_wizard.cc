/**********************************************************************
 **                                                                  **
 **                            bams_wizard.cc                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the BamsWizard class.          **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: bams_wizard.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

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

#include "bams_wizard.h"
#include "globals.h"
#include "x_utils.h"
#include "string_utils.h"
#include "acdl_plus.h"
#include "sgig_management.h"
#include "acdl_to_xml_converter.h"
#include "xml_to_acdl_converter.h"
#include "EventLogging.h"

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const MissionSpecWizard::StepData_t BamsWizard::STEP_DATA_[NUM_STEPS] = 
{
    {STEP_GET_GIG_SURROGATE_DATA, "Obtain mission specs"},
    {STEP_CHECK_GIG_SURROGATE_DATA, "Check mission specs"},
    {STEP_SPECIFY_MISSION, "Specify a mission"},
    {STEP_CHOOSE_MISSION, "Choose a mission"},
    {STEP_REHEARSE_MISSION, "Rehearse a mission"},
    {STEP_SEND_MISSION, "Send a mission"},
    {STEP_COMPLETION, "Complete"}
};
const string BamsWizard::MAIN_WINDOW_TITLE_ = "UAV Wizard";
const string BamsWizard::MINI_WINDOW_TITLE_ = "UAV Wizard";
const string BamsWizard::INSTRUCTION_STARTUP_MESSAGE_ = "UAV Wizard";
const string BamsWizard::INSTRUCTION_GET_GIG_SURROGATE_DATA_MESSAGE_ = 
    "Welcome to UAV Wizard. Click \"Next\" button to obtain a current\nmission environment data from GIG surrogate.";
const string BamsWizard::INSTRUCTION_SPECIFY_MISSION_MESSAGE_ =
    "A mission data was obtained from GIG surrogate. Click \"Next\" button to\nspecify a mission using the area map.";
const string BamsWizard::INSTRUCTION_SPECIFY_MISSION_NO_DATA_MESSAGE_ =
    "Click \"Next\" button to specify a mission using the area map.";
const string BamsWizard::INSTRUCTION_LOADING_AREA_MAP_MESSAGE_ =
    "Running the Mission Expert...";
const string BamsWizard::INSTRUCTION_CHOOSE_MISSION_MESSAGE_ = 
    "The Mission Expert has found the following missions. Select one\nand click on \"Rehearse\" or \"Next\" button to rehearse the mission,\nor click on \"Detail\" button to see the details of the mission.\nClick on \"Back\" button to respecify the mission.";
const string BamsWizard::INSTRUCTION_REHEARSE_MISSION_MESSAGE_ =
    "Preparing the mission rehearsal...";
const string BamsWizard::INSTRUCTION_SEND_MISSION_MESSAGE_ =
    "Click on \"Next\" button to send this mission to GIG surrogate. To see\nthe XML mission, click on \"Detail\" button. To select another mission,\nclick on \"Back\" button.";
const string BamsWizard::INSTRUCTION_COMPLETION_SUCCESS_MESSAGE_ =
    "The mission was sent to GIG surrogate successfully. Click on \"Start Over\"\nto start a new mission specification.";
const string BamsWizard::INSTRUCTION_COMPLETION_FAILURE_MESSAGE_ =
    "The mission could not be sent to GIG surrogate. Click on \"Start Over\" to\nstart a new mission specification.";
const string BamsWizard::DATA_INVALID_SLECTION_MESSAGE_ =
    "Internal Error: Invalid mission number was selected.";
const string BamsWizard::DATA_SGIG_NOT_CONNECTED_MESSAGE_ =
    "Error: GIG surrogate is not connected.";
const string BamsWizard::DATA_SGIG_NO_ENV_DATA_MESSAGE_ =
    "Warning: The mission environment data not available.";

// Hook for event logging
extern EventLogging *gEventLogging;


//-----------------------------------------------------------------------
// Constructor for BamsWizard class.
//-----------------------------------------------------------------------
BamsWizard::BamsWizard(
    Display *display,
    Widget parent,
    XtAppContext app,
    MissionExpert *mexp,
    const symbol_table<rc_chain> &rcTable) :
    MissionSpecWizard(MISSION_SPEC_WIZARD_BAMS, display, parent, app, mexp, rcTable)
{
}

//-----------------------------------------------------------------------
// Distructor for BamsWizard class.
//-----------------------------------------------------------------------
BamsWizard::~BamsWizard(void)
{
}

//-----------------------------------------------------------------------
// This function starts the BAMS Wizard
//-----------------------------------------------------------------------
void BamsWizard::processFirstStep_(void)
{
    setStep_(STEP_GET_GIG_SURROGATE_DATA);
}

//-----------------------------------------------------------------------
// This function processes the specified number.
//-----------------------------------------------------------------------
void BamsWizard::processStep_(int stepNum)
{
    string xmlMission, envData, timeString, latString, lonString, ovlName, msg;
    int year, month, day, hour, minute, second;
    bool completed = false;

    switch (stepNum) {

    case STEP_GET_GIG_SURROGATE_DATA:
        clearDataWindowContent_();
        printfInstructionWindow(INSTRUCTION_GET_GIG_SURROGATE_DATA_MESSAGE_.c_str());
        resetDetailBtn_(false, false, false);
        resetNextBtn_(false, true);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, false);
        refreshMainWindow_();
	gEventLogging->log("STEP_GET_SGIG_DATA");
        break;

    case STEP_CHECK_GIG_SURROGATE_DATA:
        envData = sgig_get_environment_message();
        if (envData == EMPTY_STRING_)
        {
            createDataWindowScrolledText_(DATA_SGIG_NO_ENV_DATA_MESSAGE_);
            printfInstructionWindow(INSTRUCTION_SPECIFY_MISSION_NO_DATA_MESSAGE_.c_str());
            latString = EMPTY_STRING_;
            lonString = EMPTY_STRING_;
            ovlName = EMPTY_STRING_;

            checkCurrentTime_(&year, &month, &day, &hour, &minute, &second);
            timeString = Xml2Acdl::createACDLStyleDateTimeString(
                year,
                month,
                day,
                hour,
                minute,
                second);
        }
        else
        {
            timeString = Xml2Acdl::extractMissionDateTime(envData);
            latString = Xml2Acdl::extractMissionLatitude(envData);
            lonString = Xml2Acdl::extractMissionLongitude(envData);
            ovlName = findClosestOverlay_(atof(latString.c_str()), atof(lonString.c_str()));

            msg = "Mission Environment Data\n\n - Date/Time: " + 
                replaceCharInString(timeString, '+', ' ') +
                "\n\n - Latitude/Longitude: [" + 
                latString +
                ", " +
                lonString +
                "]";

            if (ovlName != EMPTY_STRING_)
            {
                msg += "\n\n   * Relevant Overlay: " + ovlName; 

                // Save in Mission Expert
                mexp_->saveOverlayFileName(ovlName);
            }

            createDataWindowScrolledText_(msg);
            printfInstructionWindow(INSTRUCTION_SPECIFY_MISSION_MESSAGE_.c_str());
        }

        mexp_->saveMissionTimeString(timeString);
        resetDetailBtn_(true, false, false);
        resetNextBtn_(false, true);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, true);
        refreshMainWindow_();
	gEventLogging->log("STEP_CHECK_SGIG_DATA");
        break;

    case STEP_SPECIFY_MISSION:
        retrievedMissions_.clear();
        mergingMissions_ = false;
        selectedCheckBox_ = -1;
        mexp_->runMlabMissionDesign();
        printfInstructionWindow(INSTRUCTION_LOADING_AREA_MAP_MESSAGE_.c_str());
        resetDetailBtn_(false, false, false);
        resetNextBtn_(false, false);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, true);
        refreshMainWindow_();
	gEventLogging->log("STEP_SPECIFY_MISSION");
        break;

    case STEP_CHOOSE_MISSION:
        createDataWindowMetadata_();
        printfInstructionWindow(INSTRUCTION_CHOOSE_MISSION_MESSAGE_.c_str());
        resetDetailBtn_(false, false, false);
        resetNextBtn_(false, false);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, true);
        refreshMainWindow_();
	gEventLogging->log("STEP_CHOOSE_MISSION");
        break;

    case STEP_REHEARSE_MISSION:
        clearDataWindowContent_();
        printfInstructionWindow(INSTRUCTION_REHEARSE_MISSION_MESSAGE_.c_str());
        resetDetailBtn_(false, false, false);
        resetNextBtn_(false, false);
        XtSetSensitive(mainWindowData_->closeBtn, false);
        XtSetSensitive(mainWindowData_->backBtn, false);
        refreshMainWindow_();
	gEventLogging->log("STEP_REHEARSE_MISSION");
        break;

    case STEP_SEND_MISSION:
        clearDataWindowContent_();
        if (selectedMission_ >= 0)
        {
            printfInstructionWindow(INSTRUCTION_SEND_MISSION_MESSAGE_.c_str());

            if (retrievedMissions_[selectedMission_].newUnsavedMission)
            {
                createDataWindowSaveCBRButton_();
            }
            else
            {
                if (!(retrievedMissions_[selectedMission_].usageCounterUpdated))
                {
                    updateMissionUsageCounter_();
                }

                if (!(retrievedMissions_[selectedMission_].feedbackSent))
                {
                    createDataWindowMissionFeedbackButton_();
                }
            }

            resetDetailBtn_(false, false, true);
        }
        else
        {
            printfInstructionWindow("Error: Invalid selectedMission_ = %d\n", selectedMission_);
            resetDetailBtn_(true, false, false);
        }
        resetNextBtn_(false, true);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, true);
        refreshMainWindow_();
	gEventLogging->log("STEP_SEND_MISSION");
        break;

    case STEP_COMPLETION:
        clearDataWindowContent_();
        completed = true;

        if (selectedMission_ >= 0)
        {
            if (!sgig_connected())
            {
                //createDataWindowScrolledText_(DATA_SGIG_NOT_CONNECTED_MESSAGE_);
                completed = false;
            }

            if (retrievedMissions_[selectedMission_].newUnsavedMission)
            {
                createDataWindowSaveCBRButton_();
            }
            else
            {
                if (!(retrievedMissions_[selectedMission_].usageCounterUpdated))
                {
                    updateMissionUsageCounter_();
                }

                if (!(retrievedMissions_[selectedMission_].feedbackSent))
                {
                    createDataWindowMissionFeedbackButton_();
                }
            }
        }
        else
        {
            createDataWindowScrolledText_(DATA_INVALID_SLECTION_MESSAGE_);
            completed = false;
        }

        if (completed)
        {
            xmlMission = ACDLPlus::acdlp2cimxmlText(retrievedMissions_[selectedMission_].acdlSolution);
            sgig_send_message(xmlMission);
            printfInstructionWindow(INSTRUCTION_COMPLETION_SUCCESS_MESSAGE_.c_str());
            resetDetailBtn_(false, false, true);
        }
        else
        {
            printfInstructionWindow(INSTRUCTION_COMPLETION_FAILURE_MESSAGE_.c_str());
            resetDetailBtn_(false, false, false);
        }

        resetNextBtn_(true, true);
        XtSetSensitive(mainWindowData_->closeBtn, true);
        XtSetSensitive(mainWindowData_->backBtn, true);
        refreshMainWindow_();
	gEventLogging->log("STEP_COMPLETED");
        break;
    }
}

//-----------------------------------------------------------------------
// This function is called when the user presses the Detail button.
//-----------------------------------------------------------------------
void BamsWizard::processDetailBtn_(void)
{
    string xmlMission;

    switch (currentStep_) {

    case STEP_GET_GIG_SURROGATE_DATA:
        break;

    case STEP_CHECK_GIG_SURROGATE_DATA:
        break;

    case STEP_SPECIFY_MISSION:
        break;

    case STEP_CHOOSE_MISSION:
        if ((selectedCheckBox_ >= 0) && (selectedCheckBox_ < (int)(metaDataDisplayLines_.size())))
        {
            popupMetadataDetailWindow_(metaDataDisplayLines_[selectedCheckBox_].number);
		gEventLogging->log("STEP_METADATA_VIEW");
        }
	
        break;

    case STEP_REHEARSE_MISSION:
        break;

    case STEP_SEND_MISSION:
    case STEP_COMPLETION:
        if (!detailShown_)
        {
            if (selectedMission_ >= 0)
            {
                xmlMission = ACDLPlus::acdlp2cimxmlText(retrievedMissions_[selectedMission_].acdlSolution);
                createDataWindowScrolledText_(xmlMission);
                resetDetailBtn_(true, true, true);
            }
        }
        else
        {
            clearDataWindowContent_();

            if (retrievedMissions_[selectedMission_].newUnsavedMission)
            {
                createDataWindowSaveCBRButton_();
            }
            else
            {
                if (!(retrievedMissions_[selectedMission_].usageCounterUpdated))
                {
                    updateMissionUsageCounter_();
                }

                if (!(retrievedMissions_[selectedMission_].feedbackSent))
                {
                    createDataWindowMissionFeedbackButton_();
                }
            }

            resetDetailBtn_(false, false, true);
        }
        break;
    }
}

//-----------------------------------------------------------------------
// This function is called when the user presses the Next button.
//-----------------------------------------------------------------------
void BamsWizard::processNextBtn_(void)
{
    switch (currentStep_) {

    case STEP_GET_GIG_SURROGATE_DATA:
    case STEP_CHECK_GIG_SURROGATE_DATA:
    case STEP_SPECIFY_MISSION:
    case STEP_REHEARSE_MISSION:
    case STEP_SEND_MISSION:
        setStep_(currentStep_ + 1);
        break;

    case STEP_CHOOSE_MISSION:
        if ((selectedCheckBox_ >= 0) && (selectedCheckBox_ < (int)(metaDataDisplayLines_.size())))
        {
            setSelectedMission_(metaDataDisplayLines_[selectedCheckBox_].number);
            setStep_(STEP_REHEARSE_MISSION);
        }
        break;

    case STEP_COMPLETION:
        mexp_->runMissionExpert(MEXP_RUNMODE_NEW);
        break;
    }
}

//-----------------------------------------------------------------------
// This function is called when the user presses the Back button.
//-----------------------------------------------------------------------
void BamsWizard::processBackBtn_(void)
{
    switch (currentStep_) {

    case STEP_GET_GIG_SURROGATE_DATA:
        // This is the first step.
        break;

    case STEP_CHECK_GIG_SURROGATE_DATA:
        setStep_(currentStep_ - 1);
        break;

    case STEP_SPECIFY_MISSION:
        setStep_(currentStep_ - 1);
        break;

    case STEP_CHOOSE_MISSION:
        abortLoadingMission_ = true;
        setStep_(currentStep_ - 1);
        break;

    case STEP_REHEARSE_MISSION:
    case STEP_SEND_MISSION:
        mexp_->resumeMissionExpert();
        break;

    case STEP_COMPLETION:
        setStep_(STEP_SEND_MISSION);
        break;

    }
}

/**********************************************************************
 * $Log: bams_wizard.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.10  2007/10/17 17:10:37  pulam
 * s/BAMS Wizard/UAV Wizard/
 *
 * Revision 1.9  2007/10/07 20:00:24  pulam
 * New Log Messages
 *
 * Revision 1.8  2007/09/28 15:54:58  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.7  2007/09/18 22:36:10  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.6  2007/08/10 15:14:59  endo
 * CfgEdit can now save the CBR library via its GUI.
 *
 * Revision 1.5  2007/08/09 19:18:26  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.4  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.3  2007/08/04 23:52:54  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.2  2007/07/18 22:44:49  endo
 * Filter function added to the meta data display.
 *
 * Revision 1.1  2007/05/15 18:50:00  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.17  2007/03/09 20:42:38  alanwags
 * findClosestOverlay_() updated to reflect GLOBAL_REF_POINT update.
 *
 * Revision 1.16  2007/03/05 19:35:37  endo
 * The bug on Back button fixed.
 *
 * Revision 1.15  2007/02/16 18:09:29  endo
 * METADATA_DISPLAY_FONT_NAME_ revised.
 *
 * Revision 1.14  2007/02/15 16:38:17  endo
 * Logic for the Back button fixed.
 *
 * Revision 1.13  2007/02/13 20:49:22  endo
 * Format of ratings changed.
 *
 * Revision 1.12  2007/02/13 11:01:12  endo
 * IcarusWizard can now display meta data.
 *
 * Revision 1.11  2006/10/23 22:14:53  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.10  2006/10/02 19:19:14  endo
 * Bug related to XtGrab fixed.
 *
 * Revision 1.9  2006/09/28 20:41:36  alanwags
 * Big font changed because it was not supported by a laptop.
 *
 * Revision 1.8  2006/09/26 18:30:27  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.7  2006/09/22 18:43:04  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.6  2006/09/22 17:57:17  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.5  2006/09/21 14:47:54  endo
 * ICARUS Wizard improved.
 *
 * Revision 1.4  2006/09/18 18:35:11  alanwags
 * cim_send_message() implemented.
 *
 * Revision 1.3  2006/09/15 22:37:39  endo
 * ICARUS Wizard compilation meter bars added.
 *
 * Revision 1.2  2006/09/15 13:58:36  endo
 * XmNnumRows commented.
 *
 * Revision 1.1  2006/09/13 19:03:48  endo
 * ICARUS Wizard implemented.
 *
 **********************************************************************/
