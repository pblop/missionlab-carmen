/**********************************************************************
 **                                                                  **
 **                      mission_spec_wizard.cc                      **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  This file contains functions for the MissionSpecWizard class.   **
 **                                                                  **
 **  Copyright 2007 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: mission_spec_wizard.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <unistd.h>

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
#include <Xm/DrawnB.h>

#include "mission_spec_wizard.h"
#include "globals.h"
#include "x_utils.h"
#include "string_utils.h"
#include "bitmaps/star.bit"
#include "bitmaps/empty_star.bit"
#include "renumber_robots.h"
#include "EventLogging.h"

using std::sort;
using std::transform;
using std::tolower;
using std::sqrt;
using std::pow;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define MISSION_SPEC_WIZARD_CREATE_PIXMAP(name,fg,bg,parent) XCreatePixmapFromBitmapData(XtDisplay(parent),\
        RootWindowOfScreen(XtScreen(parent)),\
        (char *)name##_bits, name##_width, name##_height, fg, bg, \
        DefaultDepthOfScreen(XtScreen(parent)))

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
const struct timespec MissionSpecWizard::EVENT_WAITER_SLEEP_TIME_NSEC_ = {0, 1000000};
const string MissionSpecWizard::DETAIL_BUTTON_LABEL_DETAIL_ = "Detail";
const string MissionSpecWizard::DETAIL_BUTTON_LABEL_HIDE_ = "Hide Detail";
const string MissionSpecWizard::EMPTY_STRING_ = "";
//const string MissionSpecWizard::INSTRUCTION_FONT_NAME_ = "-*-fixed-bold-r-*-*-20-*-*-*-*-*-*-*";
const string MissionSpecWizard::INSTRUCTION_FONT_NAME_ = "-*-fixed-*-r-*-*-20-*-*-*-*-*-*-*";
const string MissionSpecWizard::INSTRUCTION_SPECIFICATION_CANCEL_MESSAGE_ =
    "Mission specification canceled.";
//const string MissionSpecWizard::METADATA_DISPLAY_FONT_NAME_ = "-*-*-*-r-*-*-10-*-*-*-*-*-*-*";
const string MissionSpecWizard::METADATA_DISPLAY_FONT_NAME_ = "-*-fixed-*-r-*-*-10-*-*-*-*-*-*-*";
const string MissionSpecWizard::MINI_WINDOW_MAXIMIZE_LABEL_ = "Back to Wizard";
const string MissionSpecWizard::MISSION_FEEDBACK_BUTTON_LABEL_ = "Rate and Comment";
const string MissionSpecWizard::NEXT_BUTTON_LABEL_NEXT_ = "Next";
const string MissionSpecWizard::NEXT_BUTTON_LABEL_RESTART_ = "Start Over";
const string MissionSpecWizard::RCTABLE_DISABLE_COLOR_BUTTONS_= "DisableMissionExpertColorButtons";
const string MissionSpecWizard::RESPECIFY_BUTTON_LABEL_ = "Respecify";
const string MissionSpecWizard::SAVECBR_BUTTON_LABEL_ = "Save this mission for a futre use";
const string MissionSpecWizard::STATUS_NORMAL_FONT_NAME_ = "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*";
const string MissionSpecWizard::STATUS_BOLD_FONT_NAME_ = "-*-fixed-bold-r-*-*-14-*-*-*-*-*-*-*";
const string MissionSpecWizard::METADATA_DETAIL_WINDOW_TITLE_ = "Mission Summary";
const string MissionSpecWizard::METADATA_DETAIL_WINDOW_OK_LABEL_ = "Choose";
const string MissionSpecWizard::METADATA_DETAIL_WINDOW_CANCEL_LABEL_ = "Cancel";
const string MissionSpecWizard::METADATA_DETAIL_WINDOW_ASSEMBLAGE_LABEL_ = "Assemblage";
const string MissionSpecWizard::METADATA_DISPLAY_CONTENTS_LABELS_[NUM_METADATA_DISPLAY_CONTENTS] =
{
    "Select", // METADATA_DISPLAY_CHECK_BOX
    "Mission No.", // METADATA_DISPLAY_NUMBER
    "# Robots", // METADATA_DISPLAY_STEALTH_RATING
    "Stealth", // METADATA_DISPLAY_STEALTH_RATING
    "Efficiency", // METADATA_DISPLAY_EFFICIENCY_RATING
    "Preserve", // METADATA_DISPLAY_PRESERVATION_RATING
    "Duration", // METADATA_DISPLAY_DURATION_RATING
    "Relevence", // METADATA_DISPLAY_RELEVANCE
    "Comments", // METADATA_DISPLAY_USER_COMMENTS
    "# Usage", // METADATA_DISPLAY_NUM_USAGE_
    "Action", // METADATA_DISPLAY_BUTTONS
};
const string MissionSpecWizard::METADATA_DISPLAY_BUTTON_LABELS_[NUM_METADATA_DISPLAY_BTNS] =
{
    "Detail", // METADATA_DISPLAY_BTN_DETAIL
    "Rehearse", // METADATA_DISPLAY_BTN_REHERASE
    "Send" // METADATA_DISPLAY_BTN_SEND
};
const string MissionSpecWizard::METADATA_DETAIL_WINDOW_RATING_NAMES_[NUM_MEXP_METADATA_RATINGS] =
{
    "Stealth", // MEXP_METADATA_RATING_STEALTH
    "Efficiency", // MEXP_METADATA_RATING_EFFICIENCY
    "Assets Preservation", // MEXP_METADATA_RATING_PRESERVATION
    "Time on Duration" // MEXP_METADATA_RATING_DURATION
};
const string MissionSpecWizard::MERGE_CANCEL_BUTTON_LABEL_ = "Normal Select";
const string MissionSpecWizard::MERGE_MISSIONS_BUTTON_LABEL_ = "Merge Missions";
const string MissionSpecWizard::MERGE_MISSIONS_INSTRUCTION_ = "Select two missions to merge.";
const string MissionSpecWizard::METADATA_FILTER_BUTTON_LABEL_ = "Filter";
const string MissionSpecWizard::METADATA_FILTER_CLEAR_BUTTON_LABEL_ = "Clear";
const string MissionSpecWizard::STRING_NA_ = "[N/A]";
const string MissionSpecWizard::SLIDER_CLIP_WINDOW_NAME_ = "ClipWindow";
const string MissionSpecWizard::STRING_GLOBAL_REF_POINT_ = "GLOBAL_REF_POINT";

const double MissionSpecWizard::DATAWINDOW2CONTENTS_WIDTH_RATIO_ = 0.96;
const double MissionSpecWizard::DISPLAY2META_HEIGHT_RATIO_ = 0.7;
const double MissionSpecWizard::DISPLAY2META_WIDTH_RATIO_ = 0.5;
const double MissionSpecWizard::DISPLAY2MINI_WIDTH_RATIO_ = 0.15;
const double MissionSpecWizard::DISPLAY2MINI_HEIGHT_RATIO_ = 0.075;
const double MissionSpecWizard::DISPLAY2MINI_X_OFFSET_RATIO_ = 0.1;
const double MissionSpecWizard::DISPLAY2MINI_Y_OFFSET_RATIO_ = 0.1;
const double MissionSpecWizard::MAIN2ACTION_HEIGHT_RATIO_ = 0.1;
const double MissionSpecWizard::MAIN2ACTION_X_OFFSET_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2ACTION_EXTRA_SPACE_RATIO_ = 0.025;
const double MissionSpecWizard::MAIN2DATA_HEIGHT_RATIO_ = 0.6;
const double MissionSpecWizard::MAIN2DATA_X_OFFSET_RATIO_ = 0.01;
const double MissionSpecWizard::MAIN2DATA_Y_OFFSET_RATIO_ = 0.01;
const double MissionSpecWizard::MAIN2INSTRUCTION_HEIGHT_RATIO_ = 0.1;
const double MissionSpecWizard::MAIN2INSTRUCTION_X_OFFSET_RATIO_ = 0.02;
const double MissionSpecWizard::MAIN2INSTRUCTION_Y_OFFSET_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2MISSIONFEEDBACK_HEIGHT_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2PROGRESS_HEIGHT_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2RESPECIFY_HEIGHT_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2SAVECBR_HEIGHT_RATIO_ = 0.05;
const double MissionSpecWizard::MAIN2STATUS_WIDTH_RATIO_ = 0.2;
const double MissionSpecWizard::MAIN2STATUS_HEIGHT_RATIO_ = 0.85;
const double MissionSpecWizard::MAIN2STATUS_X_OFFSET_RATIO_ = 0.01;
const double MissionSpecWizard::MAIN2STATUS_Y_OFFSET_RATIO_ = 0.04;
const double MissionSpecWizard::MAIN2STATUS_EXTRA_SPACE_RATIO_ = 0.05;
const double MissionSpecWizard::MINI2ACTION_WIDTH_RATIO_ = 0.33;
const double MissionSpecWizard::MINI2ACTION_HEIGHT_RATIO_ = 0.6;
const double MissionSpecWizard::MINI2ACTION_OFFSET_X_RATIO_ = 0.0;
const double MissionSpecWizard::MINI2ACTION_OFFSET_Y_RATIO_ = 0.15;
const int MissionSpecWizard::NUM_PROGRESS_BAR_STEPS_ = 20;
const int MissionSpecWizard::EVENT_WAITER_SLEEP_TIME_MSEC_ = 10;
const int MissionSpecWizard::METADATA_DISPLAY_CONTENTS_RIGHTPOS_[NUM_METADATA_DISPLAY_CONTENTS] =
{
    3, // METADATA_DISPLAY_CHECK_BOX (3)
    7, // METADATA_DISPLAY_NUMBER (4)
    11, // METADATA_DISPLAY_NUM_ROBOTS (4)
    15, // METADATA_DISPLAY_STEALTH_RATING (4)
    19, // METADATA_DISPLAY_EFFICIENCY_RATING (4)
    23, // METADATA_DISPLAY_PRESERVATION_RATING (4)
    27, // METADATA_DISPLAY_DURATION_RATING (4)
    31, // METADATA_DISPLAY_RELEVANCE (4)
    35, // METADATA_DISPLAY_USER_COMMENTS (4)
    39, // METADATA_DISPLAY_NUM_USAGE (4)
    51 // METADATA_DISPLAY_BUTTONS (12)
};
const int MissionSpecWizard::METADATA_DISPLAY_LINE_HEIGHT_ = 40;
const int MissionSpecWizard::METADATA_DISPLAY_LINE_EXTRA_HEIGHT_ = 10;
const int MissionSpecWizard::METADATA_DISPLAY_LINE_Y_OFFSET_ = 10;
const int MissionSpecWizard::METADATA_FILTER_LINE_HEIGHT_ = 30;
const int MissionSpecWizard::METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[NUM_METADATA_FILTER_LINE_CONTENTS] =
{
    1, // METADATA_FILTER_LINE_SPACE1 (1)
    9, // METADATA_FILTER_LINE_MERGE_BUTTON (8)
    23, // METADATA_FILTER_LINE_SPACE2 (14)
    47, // METADATA_FILTER_LINE_TYPE_IN (24)
    48, // METADATA_FILTER_LINE_SPACE3 (1)
    54, // METADATA_FILTER_LINE_FILTER_BUTTON (6)
    55, // METADATA_FILTER_LINE_SPACE4 (1)
    61, // METADATA_FILTER_LINE_FILTER_CLEAR_BUTTON (6)
    62 // METADATA_FILTER_LINE_SPACE5 (1)
};
const int MissionSpecWizard::MISSION_RATING_INDEX_2_METADATA_DISPLAY_INDEX_[NUM_MEXP_METADATA_RATINGS] =
{
    METADATA_DISPLAY_STEALTH_RATING, // MEXP_METADATA_RATING_STEALTH
    METADATA_DISPLAY_EFFICIENCY_RATING, // MEXP_METADATA_RATING_EFFICIENCY
    METADATA_DISPLAY_PRESERVATION_RATING, // MEXP_METADATA_RATING_PRESERVATION
    METADATA_DISPLAY_DURATION_RATING // MEXP_METADATA_RATING_DURATION
};

// Public
const int MissionSpecWizard::MAX_DATA_WINDOW_BUFSIZE = 10240;
const int MissionSpecWizard::MAX_INSTRUCTION_WINDOW_BUFSIZE = 5120;


// allow use of event logging
extern EventLogging *gEventLogging;


//-----------------------------------------------------------------------
// Constructor for MissionSpecWizard class.
//-----------------------------------------------------------------------
MissionSpecWizard::MissionSpecWizard(
    int wizardType,
    Display *display,
    Widget parent,
    XtAppContext app,
    MissionExpert *mexp,
    const symbol_table<rc_chain> &rcTable) :
    display_(display),
    appContext_(app),
    actionWindow_(NULL),
    dataWindowContent_(NULL),
    metadataDetailWindow_(NULL),
    metadataFilterTypeInBox_(NULL),
    parentWidget_(parent),
    progressBarIncs_(new Widget[NUM_PROGRESS_BAR_STEPS_]),
    instructionFontList_(NULL),
    metadataDisplayFontList_(NULL),
    statusNormalFontList_(NULL),
    statusBoldFontList_(NULL),
    mainWindowData_(NULL),
    miniWindowData_(NULL),
    mexp_(mexp),
    currentStep_(0),
    //mainWindowStatus_(MAINWINDOW_STATUS_DESTROYED),
    maxSuitabilityRating_(0),
    selectedCheckBox_(-1),
    selectedMission_(-1),
    wizardType_(wizardType),
    abortLoadingMission_(false),
    detailShown_(false),
    disableColorButtons_(false),
    metadataDetailWindowIsUp_(false),
    missionLoaded_(false),
    noRehearsal_(false),
    disableRepair_(true),
    waiting_(false),
    windowsInitialized_(false),
    reverseMetadataOrder_(false),
    missionModified_(false),
    mergingMissions_(false)
{
    int i;

    XtVaGetValues(
        parentWidget_,
        XmNforeground, &parentWidgetFg_,
        XmNbackground, &parentWidgetBg_,
        NULL);

    rcTable_ = rcTable;

    // Check RC file to see if color buttons should be disabled.
    if (check_bool_rc(rcTable_, RCTABLE_DISABLE_COLOR_BUTTONS_.c_str()))
    {
        disableColorButtons_ = true;
    }

    for (i = 0; i < NUM_PROGRESS_BAR_STEPS_; i++)
    {
        progressBarIncs_[i] = NULL;
    }
}

//-----------------------------------------------------------------------
// Distructor for MissionSpecWizard class.
//-----------------------------------------------------------------------
MissionSpecWizard::~MissionSpecWizard(void)
{
    if (progressBarIncs_ != NULL)
    {
        delete [] progressBarIncs_;
        progressBarIncs_ = NULL;
    }
}

//-----------------------------------------------------------------------
// This function initializes the interface.
// NOTE: Do not call this within the constructor. Otherwise, a pure
// virtual method will be called and the progam will abort.
//-----------------------------------------------------------------------
void MissionSpecWizard::initializeWindows_(void)
{
    createFonts_();
    createPixmaps_();
    popupDialog();

    windowsInitialized_ = true;
}

//-----------------------------------------------------------------------
// This function starts the wizard
//-----------------------------------------------------------------------
void MissionSpecWizard::start(void)
{
    if (!windowsInitialized_)
    {
        initializeWindows_();
    }

    processFirstStep_();
}

//-----------------------------------------------------------------------
// This function creates fonts.
//-----------------------------------------------------------------------
void MissionSpecWizard::createFonts_(void)
{
    defaultInstructionFont_ = createAndAppendFont(
        display_,
        (char *)(INSTRUCTION_FONT_NAME_.c_str()),
        XmSTRING_DEFAULT_CHARSET,
        &instructionFontList_);

    defaultStatusNormalFont_ = createAndAppendFont(
        display_,
        (char *)(STATUS_NORMAL_FONT_NAME_.c_str()),
        XmSTRING_DEFAULT_CHARSET,
        &statusNormalFontList_);

    defaultStatusBoldFont_ = createAndAppendFont(
        display_,
        (char *)(STATUS_BOLD_FONT_NAME_.c_str()),
        XmSTRING_DEFAULT_CHARSET,
        &statusBoldFontList_);

    defaultMetadataDisplayFont_ = createAndAppendFont(
        display_,
        (char *)(METADATA_DISPLAY_FONT_NAME_.c_str()),
        XmSTRING_DEFAULT_CHARSET,
        &metadataDisplayFontList_);
}

//-----------------------------------------------------------------------
// This function creates pixmaps to be used, and put them in a list.
//-----------------------------------------------------------------------
void MissionSpecWizard::createPixmaps_(void)
{
    Pixel yellowPixel, grayPixel;
    Colormap colorMap;
    XColor color, ignore;

    colorMap = DefaultColormap(display_, DefaultScreen(display_));

    // Create a star.

    yellowPixel = parentWidgetFg_;

    if (XAllocNamedColor(display_, colorMap, "gold", &color, &ignore))
    {
        yellowPixel = color.pixel;
    }

    starPixmap_ = MISSION_SPEC_WIZARD_CREATE_PIXMAP(star, yellowPixel, gCfgEditPixels.white, parentWidget_);

    // Create an empty star.

    grayPixel = parentWidgetFg_;

    if (XAllocNamedColor(display_, colorMap, "gray40", &color, &ignore))
    {
        grayPixel = color.pixel;
    }

    emptyStarPixmap_ = MISSION_SPEC_WIZARD_CREATE_PIXMAP(empty_star, grayPixel, gCfgEditPixels.white, parentWidget_);
}

//-----------------------------------------------------------------------
// This function refreshes windows.
//-----------------------------------------------------------------------
void MissionSpecWizard::refreshMainWindow_(void)
{
    int i;

    if (mainWindowData_ == NULL)
    {
        return;
    }

    XFlush(display_);
    XmUpdateDisplay(parentWidget_);
    XmUpdateDisplay(mainWindowData_->popupShell);

    for (i = 0; i < (int)(mainWindowData_->windows.size()); i++)
    {
        XmUpdateDisplay(mainWindowData_->windows[i]);
    }
}

//-----------------------------------------------------------------------
// This function clears the content of the data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::clearDataWindowContent_(void)
{
    WidgetList children;
    int i, numChildren;

    if (dataWindowContent_ != NULL)
    {
        XtVaGetValues(
            mainWindowData_->dataWindow,
            XmNchildren, &children,
            XmNnumChildren, &numChildren,
            NULL);

        for (i = 0; i < numChildren; i++)
        {
            XtUnmanageChild(children[i]);
        }

        XtDestroyWidget(dataWindowContent_);
        dataWindowContent_ = NULL;
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function attaches the scrolled text to the data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowScrolledText_(string text)
{
    Arg args[100];
    int n;

    clearDataWindowContent_();

    n = 0;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNvalue, text.c_str()); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    XtSetArg(args[n], XmNbackground, gCfgEditPixels.white); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNmappedWhenManaged, true); n++;

    dataWindowContent_ = XmCreateScrolledText(
        mainWindowData_->dataWindow,
        "dataWindowContent_",
        args,
        n);

    XtManageChild (dataWindowContent_);
}

//-----------------------------------------------------------------------
// This function filters the meta data based on the filtering string.
//-----------------------------------------------------------------------
void MissionSpecWizard::filterMetadata_(void)
{
    MExpRetrievedMission_t retrievedMission;
    string::size_type index;
    string comment, acdlSolution, filterString, subString;
    vector<string> filterStrings;
    int i, j, k;
    int selectedRetrievedMissionIndex = -1;
    bool found = false;
    const char WHITE_CHAR = ' ';
    const int WHITE_CHAR_LENGTH = 1;

    if ((selectedCheckBox_ >= 0) &&
        (selectedCheckBox_ < (int)(metaDataDisplayLines_.size())))
    {
        selectedRetrievedMissionIndex = metaDataDisplayLines_[selectedCheckBox_].number;
    }

    metadataFilterRejectList_.clear();
    filterString = metadataFilterString_;

    // Change the string to lower case.
    transform(
        filterString.begin(),
        filterString.end(),
        filterString.begin(), 
        (int(*)(int))tolower);

    while (true)
    {
        if (filterString.size() == 0)
        {
            break;
        }

        // Erase leading white space.
        while (filterString[0] == WHITE_CHAR)
        {
            filterString.erase(0, WHITE_CHAR_LENGTH);
        }

        if (filterString.size() == 0)
        {
            break;
        }

        // Extract a word before a white space.
        index = filterString.find(WHITE_CHAR);

        if (index == string::npos)
        {
            // At the end, save the last word.
            filterStrings.push_back(filterString);
            break;
        }

        // Save the word before the white space.
        subString = filterString.substr(0, index);
        filterStrings.push_back(subString);

        // Extract the string after the white space.
        filterString = filterString.substr(index + WHITE_CHAR_LENGTH, string::npos);
    }

    for (i = 0; i < (int)(retrievedMissions_.size()); i++)
    {
        retrievedMission = retrievedMissions_[i];

        for (j = 0; j < (int)(filterStrings.size()); j++)
        {
            filterString = filterStrings[j];
            found = false;

            // Find the string in the user comments.
            for (k = 0; k < (int)(retrievedMission.metadata.userComments.size()); k++)
            {
                comment = retrievedMission.metadata.userComments[k];

                // Change the string to lower case.
                transform(
                    comment.begin(),
                    comment.end(),
                    comment.begin(), 
                    (int(*)(int))tolower);

                index = comment.find(filterString);

                if (index != string::npos)
                {
                    // Found the string.
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                // Find the string in the mission.
                acdlSolution = retrievedMission.acdlSolution;

                // Change the string to lower case.
                transform(
                    acdlSolution.begin(),
                    acdlSolution.end(),
                    acdlSolution.begin(), 
                    (int(*)(int))tolower);

                index = acdlSolution.find(filterString);

                if (index != string::npos)
                {
                    // Found the string.
                    found = true;
                }
            }

            if (!found)
            {
                metadataFilterRejectList_.push_back(retrievedMission.index);
                break;
            }
        }
    }

    orderMetadata_();
    createDataWindowMetadata_();
    checkMetadataDisplayLineCheckBoxForRetrievedMission_(selectedRetrievedMissionIndex);
}

//-----------------------------------------------------------------------
// This function clears the filter of the meta.
//-----------------------------------------------------------------------
void MissionSpecWizard::clearMetadataFilter_(void)
{
    metadataFilterString_ = EMPTY_STRING_;

    if (metadataFilterTypeInBox_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionSpecWizard::clearMetadataFilter_(). metadataFilterTypeInBox_ is NULL.\n");
        return;
    }

    XtVaSetValues(
        metadataFilterTypeInBox_,
        XmNvalue, (char *)(metadataFilterString_.c_str()),
        NULL);

    filterMetadata_();
}

//-----------------------------------------------------------------------
// This function updates the meta data filer string.
//-----------------------------------------------------------------------
void MissionSpecWizard::updateMetadataFilterString_(void)
{
    char *typeInBoxValue = NULL;

    if (metadataFilterTypeInBox_ == NULL)
    {
        fprintf(
            stderr,
            "Error: MissionSpecWizard::filterMetadata_(). metadataFilterTypeInBox_ is NULL.\n");
        return;
    }

    XtVaGetValues(
        metadataFilterTypeInBox_,
        XmNvalue, &typeInBoxValue,
        NULL);

    if (typeInBoxValue == NULL)
    {
        return;
    }

    metadataFilterString_ = typeInBoxValue;
}

//-----------------------------------------------------------------------
// This function sets the meta data display order.
//-----------------------------------------------------------------------
void MissionSpecWizard::setMetadataOrder_(int orderReference)
{
    int selectedRetrievedMissionIndex = -1;

    if ((selectedCheckBox_ >= 0) &&
        (selectedCheckBox_ < (int)(metaDataDisplayLines_.size())))
    {
        selectedRetrievedMissionIndex = metaDataDisplayLines_[selectedCheckBox_].number;
    }

    if (orderReference != metadataOrderReference_)
    {
        reverseMetadataOrder_ = false;
        metadataOrderReference_ = orderReference;
    }
    else
    {
        reverseMetadataOrder_ = (reverseMetadataOrder_)? false : true;
    }

    orderMetadata_();
    createDataWindowMetadata_();
    checkMetadataDisplayLineCheckBoxForRetrievedMission_(selectedRetrievedMissionIndex);
}

//-----------------------------------------------------------------------
// This function (re)orders the meta data. Use MetaDataDisplayContents_t
// as orderReference.
//-----------------------------------------------------------------------
void MissionSpecWizard::orderMetadata_(void)
{
    vector<MExpRetrievedMission_t> retrievedMissions;
    int i, j, numRetrievedMissions, ratingIndex;
    bool rejected = false;

    metadataOrder_.clear();
    retrievedMissions = retrievedMissions_;
    numRetrievedMissions = retrievedMissions.size();

    switch (metadataOrderReference_) {

    case METADATA_DISPLAY_NUMBER:
        if (!reverseMetadataOrder_)
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isAscendingIndex);
        }
        else
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isDescendingIndex);
        }
        break;

    case METADATA_DISPLAY_STEALTH_RATING:
    case METADATA_DISPLAY_EFFICIENCY_RATING:
    case METADATA_DISPLAY_PRESERVATION_RATING:
    case METADATA_DISPLAY_DURATION_RATING:
        ratingIndex = -1;
        for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
        {
            if (MISSION_RATING_INDEX_2_METADATA_DISPLAY_INDEX_[i] == metadataOrderReference_)
            {
                ratingIndex = i;
                break;
            }
        }

        if (ratingIndex >= 0)
        {
            switch (ratingIndex) {

            case MEXP_METADATA_RATING_STEALTH:
                if (!reverseMetadataOrder_)
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isDescendingRatingStealth);
                }
                else
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isAscendingRatingStealth);
                }
                break;

            case MEXP_METADATA_RATING_EFFICIENCY:
                if (!reverseMetadataOrder_)
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isDescendingRatingEfficiency);
                }
                else
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isAscendingRatingEfficiency);
                }
                break;

            case MEXP_METADATA_RATING_PRESERVATION:
                if (!reverseMetadataOrder_)
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isDescendingRatingPreservation);
                }
                else
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isAscendingRatingPreservation);
                }
                break;

            case MEXP_METADATA_RATING_DURATION:
                if (!reverseMetadataOrder_)
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isDescendingRatingDuration);
                }
                else
                {
                    sort(
                        retrievedMissions.begin(),
                        retrievedMissions.end(),
                        MExpRetrievedMission_t::isAscendingRatingDuration);
                }
                break;
            }
        }
        break;


    case METADATA_DISPLAY_RELEVANCE:
        if (!reverseMetadataOrder_)
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isDescendingRelevance);
        }
        else
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isAscendingRelevance);
        }
        break;

    case METADATA_DISPLAY_USER_COMMENTS:
        if (!reverseMetadataOrder_)
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isDescendingNumUserComments);
        }
        else
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isAscendingNumUserComments);
        }
        break;

    case METADATA_DISPLAY_NUM_USAGE:
        if (!reverseMetadataOrder_)
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isDescendingNumUsage);
        }
        else
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isAscendingNumUsage);
        }
        break;

    case METADATA_DISPLAY_NUM_ROBOTS:
        if (!reverseMetadataOrder_)
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isDescendingNumRobots);
        }
        else
        {
            sort(
                retrievedMissions.begin(),
                retrievedMissions.end(),
                MExpRetrievedMission_t::isAscendingNumRobots);
        }
        break;

    default:
        // Do nothing.
        break;
    }

    for (i = 0; i < numRetrievedMissions; i++)
    {
        rejected = false;

        for (j = 0; j < (int)(metadataFilterRejectList_.size()); j++)
        {
            if ((retrievedMissions[i].index) == metadataFilterRejectList_[j])
            {
                rejected = true;
                break;
            }
        }

        if (!rejected)
        {
            metadataOrder_.push_back(retrievedMissions[i].index);
        }
    }
}

//-----------------------------------------------------------------------
// This function writes to the data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::printDataWindow(char *str)
{
    createDataWindowScrolledText_(str);
}

//-----------------------------------------------------------------------
// This function writes to the data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::printfDataWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_DATA_WINDOW_BUFSIZE];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printDataWindow(buf);
}

//-----------------------------------------------------------------------
// This function writes to the instruction window.
//-----------------------------------------------------------------------
void MissionSpecWizard::printInstructionWindow(char *str)
{
    if (mainWindowData_ == NULL)
    {
        fprintf(
            stderr,
            "Error(cfgedit): MissionSpecWizard::printInstructionWindow(). mainWindowData_ is NULL.\n");
        return;
    }

    XtVaSetValues(
        mainWindowData_->instructionWindow,
        XmNvalue, str,
        NULL);
}

//-----------------------------------------------------------------------
// This function writes to the instruction window.
//-----------------------------------------------------------------------
void MissionSpecWizard::printfInstructionWindow(const char *format, ...)
{
    va_list args;
    char buf[MAX_INSTRUCTION_WINDOW_BUFSIZE];

    va_start(args, format);
    vsprintf(buf, format, args);
    va_end(args);

    printInstructionWindow(buf);
}

//-----------------------------------------------------------------------
// This function save retrieved missions.
//-----------------------------------------------------------------------
MExpRetrievedMission_t MissionSpecWizard::saveRetrievedMission(
    vector<MExpRetrievedMission_t> retrievedMissions,
    int maxSuitabilityRating,
    int *selectedMission)
{
    MExpRetrievedMission_t mission;
    vector<MExpRetrievedMission_t> unsavedMissions;
    int i;

    for (i = 0; i < (int)(retrievedMissions_.size()); i++)
    {
        if (retrievedMissions_[i].newUnsavedMission)
        {
            unsavedMissions.push_back(retrievedMissions_[i]);
        }
    }

    retrievedMissions_ = retrievedMissions;

    for (i = 0; i < (int)(unsavedMissions.size()); i++)
    {
        retrievedMissions_.push_back(unsavedMissions[i]);
    }

    maxSuitabilityRating_ = maxSuitabilityRating;
    selectedMission_ = -1;
    noRehearsal_ = false;
    missionLoaded_ = false;
    abortLoadingMission_ = false;
    missionModified_ = false;

    // Reset the meta data.
    metadataFilterString_ = EMPTY_STRING_;
    reverseMetadataOrder_ = false;
    metadataOrderReference_ = -1;
    metadataFilterRejectList_.clear();
    orderMetadata_();

    // Make the user to choose a mission.
    setStep_(stepChooseMission_());

    while ((selectedMission_ < 0) && (!abortLoadingMission_))
    {
        if (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
        else
        {
            nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);
        }
    }

    *selectedMission = selectedMission_;

    if (selectedMission_ >= 0)
    {
        mission = retrievedMissions_[selectedMission_];
    }

    return mission;
}

//-----------------------------------------------------------------------
// This function creates progress data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowProgressBar_(void)
{
    vector<Widget> stackedWidget;
    int screenNum;
    int displayHeight, displayWidth;
    int h, w;
    int i, numStackedWidget;

    clearDataWindowContent_();

    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    h = (int)(MAIN2PROGRESS_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    dataWindowContent_ = XtVaCreateWidget(
        "dataWindowContent_",
        xmFormWidgetClass, mainWindowData_->dataWindow,
        XmNheight, h,
        XmNwidth, w,
        XmNborderWidth, 0,
        XmNfractionBase, NUM_PROGRESS_BAR_STEPS_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    stackedWidget.push_back(dataWindowContent_);

    for (i = 0; i < NUM_PROGRESS_BAR_STEPS_; i++)
    {
        progressBarIncs_[i] = XtVaCreateManagedWidget(
            "",
            xmPushButtonGadgetClass, dataWindowContent_,
            XmNtopAttachment, XmATTACH_FORM,
            XmNbottomAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_POSITION,
            XmNleftPosition, i,
            XmNrightAttachment, XmATTACH_POSITION,
            XmNrightPosition, i+1,
            XmNshowAsDefault, false,
            XmNdefaultButtonShadowThickness, 1,
            XmNborderWidth, 1,
            XmNhighlightThickness, 0,
            NULL);

        stackedWidget.push_back(progressBarIncs_[i]);
    }

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function updates the progress bar. level = 0 is no bars,
// level = 1 is full bars.
//-----------------------------------------------------------------------
void MissionSpecWizard::updateProgressBar(double level)
{
    Pixel pixel;
    int i, numColoredBars;

    if (dataWindowContent_ == NULL)
    {
        createDataWindowProgressBar_();
    }

    numColoredBars = (int)(level*(double)NUM_PROGRESS_BAR_STEPS_);

    for (i = 0; i < NUM_PROGRESS_BAR_STEPS_; i++)
    {
        if (progressBarIncs_[i] != NULL)
        {
            pixel = (i <= numColoredBars)?
                gCfgEditPixels.darkslateblue : parentWidgetBg_;
            
            XtVaSetValues(
                progressBarIncs_[i],
                XmNbackground, pixel,
                NULL);
        }
        else
        {
            fprintf(
                stderr,
                "Error(cfgedit): MissionSpecWizard::updateProgressData_(). progressBarIncs_ is NULL.\n"); 
        }
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function clears the progress bar.
//-----------------------------------------------------------------------
void MissionSpecWizard::clearProgressBar(void)
{
    clearDataWindowContent_();
}

//-----------------------------------------------------------------------
// This function creates the respecify button in the data window.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowRespecifyButton_(void)
{
    Widget respecifyBtn;
    vector<Widget> stackedWidget;
    int screenNum;
    int displayHeight, displayWidth;
    int h, w;
    int i, numStackedWidget;

    clearDataWindowContent_();

    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    h = (int)(MAIN2RESPECIFY_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    dataWindowContent_ = XtVaCreateWidget(
        "dataWindowContent_",
        xmFormWidgetClass, mainWindowData_->dataWindow,
        XmNheight, h,
        XmNwidth, w,
        XmNborderWidth, 0,
        XmNfractionBase, 4,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    stackedWidget.push_back(dataWindowContent_);

    respecifyBtn = XtVaCreateManagedWidget(
        RESPECIFY_BUTTON_LABEL_.c_str(),
        xmPushButtonGadgetClass, dataWindowContent_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 0,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 1,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        XmNbackground, gCfgEditPixels.plum,
        NULL);

    stackedWidget.push_back(respecifyBtn);

    XtAddCallback(
        respecifyBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbRespecifyBtn_,
        this);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function creates the mission a button for saving a new case.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowSaveCBRButton_(void)
{
    Widget saveCBRBtn;
    vector<Widget> stackedWidget;
    int screenNum;
    int displayHeight, displayWidth;
    int h, w;
    int i, numStackedWidget;

    clearDataWindowContent_();

    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    h = (int)(MAIN2SAVECBR_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    dataWindowContent_ = XtVaCreateWidget(
        "dataWindowContent_",
        xmFormWidgetClass, mainWindowData_->dataWindow,
        XmNheight, h,
        XmNwidth, w,
        XmNborderWidth, 0,
        XmNfractionBase, 3,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    stackedWidget.push_back(dataWindowContent_);

    saveCBRBtn = XtVaCreateManagedWidget(
        SAVECBR_BUTTON_LABEL_.c_str(),
        xmPushButtonGadgetClass, dataWindowContent_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 0,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 1,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        XmNbackground, parentWidgetBg_,
        NULL);

    stackedWidget.push_back(saveCBRBtn);

    XtAddCallback(
        saveCBRBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbSaveCBRBtn_,
        this);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function creates the mission feedback button.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowMissionFeedbackButton_(void)
{
    Widget feedbackBtn;
    vector<Widget> stackedWidget;
    int screenNum;
    int displayHeight, displayWidth;
    int h, w;
    int i, numStackedWidget;

    clearDataWindowContent_();

    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    h = (int)(MAIN2MISSIONFEEDBACK_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    dataWindowContent_ = XtVaCreateWidget(
        "dataWindowContent_",
        xmFormWidgetClass, mainWindowData_->dataWindow,
        XmNheight, h,
        XmNwidth, w,
        XmNborderWidth, 0,
        XmNfractionBase, 4,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    stackedWidget.push_back(dataWindowContent_);

    feedbackBtn = XtVaCreateManagedWidget(
        MISSION_FEEDBACK_BUTTON_LABEL_.c_str(),
        xmPushButtonGadgetClass, dataWindowContent_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 0,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 1,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        XmNbackground, parentWidgetBg_,
        NULL);

    stackedWidget.push_back(feedbackBtn);

    XtAddCallback(
        feedbackBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbMissionFeedbackBtn_,
        this);

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function is called when the "Rate and Comment" (mission feedback)
// button is pressed.
//-----------------------------------------------------------------------
bool MissionSpecWizard::sendMissionFeedbackToCBR_(void)
{
    bool feedbackSent = false;

    if (selectedMission_ >= 0)
    {
        mexp_->sendMissionFeedback(retrievedMissions_[selectedMission_].dataIndexList);
        retrievedMissions_[selectedMission_].feedbackSent = true;
        feedbackSent = true;
    }

    return feedbackSent;
}

//-----------------------------------------------------------------------
// This function is called when the save-cbr button is pressed.
//-----------------------------------------------------------------------
bool MissionSpecWizard::sendMissionToCBR_(void)
{
    bool missionSent = false;

    if (selectedMission_ >= 0)
    {
        mexp_->saveSelectionToCBRLibrary(config->root());

        while (mexp_->featureMenuIsUp())
        {
            if (XtAppPending(appContext_))
            {
                XtAppProcessEvent(appContext_, XtIMAll);
            }
            else
            {
                nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);
            }
        }

        if (mexp_->missionSavedToCBRLibrary())
        {
            retrievedMissions_[selectedMission_].newUnsavedMission = false;
            missionSent = true;
        }
    }

    return missionSent;
}

//-----------------------------------------------------------------------
// This function updates the mission usage counter in the CBR library.
//-----------------------------------------------------------------------
void MissionSpecWizard::updateMissionUsageCounter_(void)
{
    if (selectedMission_ >= 0)
    {
        mexp_->updateMissionUsageCounter(retrievedMissions_[selectedMission_].dataIndexList);
        retrievedMissions_[selectedMission_].usageCounterUpdated = true;
    }
}

//-----------------------------------------------------------------------
// This function is called when the mission specification is cancled.
//-----------------------------------------------------------------------
void MissionSpecWizard::setSpecificationCanceled(void)
{
    printfInstructionWindow(INSTRUCTION_SPECIFICATION_CANCEL_MESSAGE_.c_str());
    createDataWindowRespecifyButton_();
}

//-----------------------------------------------------------------------
// This function creates the mini window.
//-----------------------------------------------------------------------
void MissionSpecWizard::createMiniWindow_(void)
{
    vector<Widget> stackedWidget;
    Widget miniForm, maximizeBtn;
    Widget pane, frame, form;
    int screenNum;
    int displayHeight, displayWidth, miniWidth, miniHeight;
    int h, w, x, y;
    int i, numStackedWidget;

    if (miniWindowData_ == NULL)
    {
        miniWindowData_ = new MiniWindowData_t;
    }

    // Get the display dimension.
    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    miniHeight = (int)(DISPLAY2MINI_HEIGHT_RATIO_*(double)displayHeight);
    miniWidth = (int)(DISPLAY2MINI_WIDTH_RATIO_*(double)displayWidth);
    x = displayWidth -
        miniWidth -
        (int)(DISPLAY2MINI_X_OFFSET_RATIO_*(double)displayWidth);
    y = displayHeight -
        miniHeight -
        (int)(DISPLAY2MINI_Y_OFFSET_RATIO_*(double)displayHeight);

    miniWindowData_->popupShell = XtVaCreatePopupShell(
        "MissionSpecWizardMini",
        xmDialogShellWidgetClass, parentWidget_, 
        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
        XmNtitle, miniWindowTitle_().c_str(),
        XmNx, x,
        XmNy, y,
        XmNheight, miniHeight,
        XmNwidth, miniWidth,
        XmNrecomputeSize, false,
        NULL);

    frame = XtVaCreateWidget(
        "frame",
        xmFrameWidgetClass, miniWindowData_->popupShell,
        XmNshadowType, XmSHADOW_ETCHED_OUT,
        XmNheight, miniHeight,
        XmNwidth, miniWidth,
        NULL);

    stackedWidget.push_back(frame);

    miniForm = XtVaCreateWidget(
        "miniForm",
        xmFormWidgetClass, frame,
        XmNbackground, gCfgEditPixels.snow4,
        XmNborderWidth, 0,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, frame,
        XmNbottomAttachment, XmATTACH_WIDGET,
        XmNbottomWidget, frame,
        XmNleftAttachment, XmATTACH_WIDGET,
        XmNleftWidget, frame,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, frame,
        NULL);

    stackedWidget.push_back(miniForm);

    w = miniWidth;
    h = (int)(MINI2ACTION_HEIGHT_RATIO_*(double)miniHeight);
    x = (int)(MINI2ACTION_OFFSET_X_RATIO_*(double)miniWidth);
    y = (int)(MINI2ACTION_OFFSET_Y_RATIO_*(double)miniHeight);

    pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, miniForm,
        XmNsashWidth, 2,
        XmNsashHeight, 2,
        XmNy, y,
        XmNborderWidth, 0,
        XmNbackground, gCfgEditPixels.snow4,
        NULL);

    stackedWidget.push_back(pane);

    form = XtVaCreateWidget(
        "form",
        xmFormWidgetClass, pane, 
        XmNheight, h,
        XmNwidth, w,  
        XmNfractionBase, 10,
        XmNborderWidth, 0,
        XmNbackground, gCfgEditPixels.snow4,
        XmNrecomputeSize, false,
        NULL);

    stackedWidget.push_back(form);

    maximizeBtn = XtVaCreateWidget(
        MINI_WINDOW_MAXIMIZE_LABEL_.c_str(),
        xmPushButtonGadgetClass, form,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 1,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 9,
        XmNdefaultButtonShadowThickness, 0,
        XmNhighlightThickness, 0,
        XmNborderWidth, 0,
        XmNforeground, gCfgEditPixels.yellow,
        XmNbackground, gCfgEditPixels.snow4,
        XmNrecomputeSize, false,
        XmNshowAsDefault, true,
        NULL);

    stackedWidget.push_back(maximizeBtn);

    XtAddCallback(
        maximizeBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbMaximizeBtn_,
        this);

    // Manage the windows.
    miniWindowData_->windows = stackedWidget;
    numStackedWidget = stackedWidget.size();

    XtPopup(miniWindowData_->popupShell, XtGrabNone);
    XtAddGrab(miniWindowData_->popupShell, false, false);

    for (i = numStackedWidget - 1; i >= 0; i--)
    {
        XtManageChild(stackedWidget[i]);
    }
}

//-----------------------------------------------------------------------
// This function creates the main window.
//-----------------------------------------------------------------------
void MissionSpecWizard::createMainWindow_(void)
{
    StatusRow_t statusRow;
    vector<Widget> stackedWidget;
    Widget mainForm, statusWindow;
    Widget pane, frame, form;
    char label[1024];
    int screenNum;
    int displayHeight, displayWidth;
    int h, w, x, y;
    int i, numStackedWidget, numSteps;

    if (mainWindowData_ == NULL)
    {
        mainWindowData_ = new MainWindowData_t;
    }

    mainWindowData_->windowIsUp = true;

    // Get the display dimension.
    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    mainWindowData_->popupShell = XtVaCreatePopupShell(
        "MissionSpecWizardMain",
        xmDialogShellWidgetClass, parentWidget_, 
        XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
        XmNtitle, mainWindowTitle_().c_str(),
        NULL);

    frame = XtVaCreateWidget(
        "frame",
        xmFrameWidgetClass, mainWindowData_->popupShell,
        XmNshadowType, XmSHADOW_ETCHED_OUT,
        XmNheight, displayHeight,
        XmNwidth, displayWidth,  
        NULL);

    stackedWidget.push_back(frame);

    mainForm = XtVaCreateWidget(
        "mainForm",
        xmFormWidgetClass, frame,
        NULL);

    stackedWidget.push_back(mainForm);

    x = (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);
    y = (int)(MAIN2STATUS_Y_OFFSET_RATIO_*(double)displayHeight);
    h = (int)(MAIN2STATUS_HEIGHT_RATIO_*(double)displayHeight);
    w = (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth);

    pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, mainForm,
        XmNsashWidth, 2,
        XmNsashHeight, 2,
        XmNx, x,
        XmNy, y,
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(pane);

    form = XtVaCreateWidget(
        "form",
        xmFormWidgetClass, pane, 
        XmNheight, h,
        XmNwidth, w,  
        NULL);

    stackedWidget.push_back(form);

    statusWindow = XtVaCreateWidget
        ("statusWindow",
         xmRowColumnWidgetClass, form,
         XmNpacking, XmPACK_COLUMN,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, true,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNheight, h,
         XmNwidth, w,
         XmNrightAttachment, XmATTACH_FORM,
         XmNleftAttachment, XmATTACH_FORM,
         XmNtopAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNbackground, gCfgEditPixels.snow4,
         NULL);

    stackedWidget.push_back(statusWindow);
    
    numSteps = numSteps_();

    if (numSteps > 0)
    {
        h = (int)(((MAIN2STATUS_HEIGHT_RATIO_*(double)displayHeight) -
                   (MAIN2STATUS_EXTRA_SPACE_RATIO_*(double)displayHeight)) /
                  (double)numSteps);

        statusRow.h = h;
        statusRow.w = w;

        for (i = 0; i < numSteps; i++)
        {
            statusRow.form = XtVaCreateWidget
                ("statusRow.form",
                 xmFormWidgetClass, statusWindow,
                 XmNalignment, XmALIGNMENT_CENTER,
                 XmNheight, statusRow.h,
                 XmNwidth, statusRow.w,
                 XmNborderWidth, 0,
                 XmNbackground, gCfgEditPixels.snow4,
                 XmNrecomputeSize, false,
                 NULL);

            stackedWidget.push_back(statusRow.form);

            sprintf(label, "  %d. %s", i+1, stepData_()[i].description.c_str());

            statusRow.label = XtVaCreateWidget
                (label,
                 xmLabelGadgetClass, statusRow.form,
                 XmNalignment, XmALIGNMENT_BEGINNING,
                 XmNheight, statusRow.h,
                 XmNwidth, statusRow.w,
                 XmNrightAttachment, XmATTACH_FORM,
                 XmNleftAttachment, XmATTACH_FORM,
                 XmNtopAttachment, XmATTACH_FORM,
                 XmNbottomAttachment, XmATTACH_FORM,
                 XmNbackground, gCfgEditPixels.snow4,
                 XmNborderWidth, 0,
                 XmNfontList, statusNormalFontList_,
                 XmNforeground, gCfgEditPixels.white,
                 XmNbackground, gCfgEditPixels.snow4,
                 XmNrecomputeSize, false,
                 NULL);

            stackedWidget.push_back(statusRow.label);

            statusRows_.push_back(statusRow);
        }
    }

    x = (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) +
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) +
        (int)(MAIN2INSTRUCTION_X_OFFSET_RATIO_*(double)displayWidth);
    y = (int)(MAIN2INSTRUCTION_Y_OFFSET_RATIO_*(double)displayHeight);
    h = (int)(MAIN2INSTRUCTION_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        x - 
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, mainForm,
        XmNx, x,
        XmNy, y,
        XmNsashShadowThickness, 0,
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(pane);

    form = XtVaCreateWidget(
        "form",
        xmFormWidgetClass, pane,
        XmNleftWidget, statusWindow,
        XmNheight, h,
        XmNwidth, w,  
        XmNborderWidth, 0,
        XmNbackground, gCfgEditPixels.red,
        NULL);

    stackedWidget.push_back(form);

    mainWindowData_->instructionWindow = XtVaCreateManagedWidget(
        "MissionSpecWizardInstruction",
        xmTextWidgetClass, form,
        XmNvalue, instructionStartUpMessage_().c_str(),
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNborderWidth, 0,
        XmNcursorPositionVisible, false,
        XmNwordWrap, true,
        XmNeditable, false,
        XmNshadowThickness, 0,
        XmNhighlightThickness, 0,
        XmNfontList, instructionFontList_,
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(mainWindowData_->instructionWindow);

    x = (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) +
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) +
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth);
    y = (int)(MAIN2INSTRUCTION_Y_OFFSET_RATIO_*(double)displayHeight) +
        (int)(MAIN2INSTRUCTION_HEIGHT_RATIO_*(double)displayHeight) +
        (int)(MAIN2DATA_Y_OFFSET_RATIO_*(double)displayHeight);
    h = (int)(MAIN2DATA_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        x - 
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, mainForm,
        XmNx, x,
        XmNy, y,
        XmNsashShadowThickness, 0,
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(pane);

    mainWindowData_->dataWindow = XtVaCreateWidget(
        "MissionSpecWizardData",
        xmFormWidgetClass, pane,
        XmNleftWidget, statusWindow,
        XmNheight, h,
        XmNwidth, w,  
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(mainWindowData_->dataWindow);

    x = (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) +
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) +
        (int)(MAIN2ACTION_X_OFFSET_RATIO_*(double)displayWidth);
    y = (int)(MAIN2INSTRUCTION_Y_OFFSET_RATIO_*(double)displayHeight) +
        (int)(MAIN2INSTRUCTION_HEIGHT_RATIO_*(double)displayHeight) +
        (int)(MAIN2DATA_HEIGHT_RATIO_*(double)displayHeight) +
        (int)(MAIN2ACTION_EXTRA_SPACE_RATIO_*(double)displayHeight);
    h = (int)(MAIN2ACTION_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        x - 
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    pane = XtVaCreateWidget(
        "pane",
        xmPanedWindowWidgetClass, mainForm,
        XmNx, x,
        XmNy, y,
        XmNsashShadowThickness, 0,
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(pane);

    actionWindow_ = XtVaCreateWidget(
        "MissionSpecWizardAction",
        xmFormWidgetClass, pane, 
        XmNfractionBase, 6,
        XmNheight, h,
        XmNwidth, w,  
        XmNborderWidth, 0,
        NULL);

    stackedWidget.push_back(actionWindow_);

    mainWindowData_->closeBtn = XtVaCreateManagedWidget(
        "Close",
        xmPushButtonGadgetClass, actionWindow_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 0,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 1,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        NULL);

    XtAddCallback(
        mainWindowData_->closeBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbCloseBtn_,
        this);

    stackedWidget.push_back(mainWindowData_->closeBtn);

    mainWindowData_->detailBtn = XtVaCreateManagedWidget(
        "Detail",
        xmPushButtonGadgetClass, actionWindow_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 2,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 3,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        NULL);

    XtAddCallback(
        mainWindowData_->detailBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbDetailBtn_,
        this);

    stackedWidget.push_back(mainWindowData_->detailBtn);

    mainWindowData_->backBtn = XtVaCreateManagedWidget(
        "Back",
        xmPushButtonGadgetClass, actionWindow_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 3,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 4,
        XmNshowAsDefault, false,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        NULL);

    XtAddCallback(
        mainWindowData_->backBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbBackBtn_,
        this);

    stackedWidget.push_back(mainWindowData_->backBtn);

    mainWindowData_->nextBtn = XtVaCreateManagedWidget(
        "Next",
        xmPushButtonGadgetClass, actionWindow_,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 4,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNrightPosition, 5,
        XmNshowAsDefault, true,
        XmNdefaultButtonShadowThickness, 1,
        XmNborderWidth, 1,
        XmNhighlightThickness, 0,
        NULL);

    XtAddCallback(
        mainWindowData_->nextBtn,
        XmNactivateCallback,
        (XtCallbackProc)cbNextBtn_,
        this);

    stackedWidget.push_back(mainWindowData_->nextBtn);

    // Manage the windows.
    mainWindowData_->windows = stackedWidget;
    numStackedWidget = stackedWidget.size();

    XtPopup(mainWindowData_->popupShell, XtGrabNone);
    XtAddGrab(mainWindowData_->popupShell, true, true);

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    stackedWidget.clear();
}

//-----------------------------------------------------------------------
// This function checks to see if the main window is up.
//-----------------------------------------------------------------------
bool MissionSpecWizard::dialogIsUp(void)
{
    if (mainWindowData_ == NULL)
    {
        return false;
    }

    return (mainWindowData_->windowIsUp);
}

//-----------------------------------------------------------------------
// This function pops up the main window.
//-----------------------------------------------------------------------
void MissionSpecWizard::popupDialog(void)
{
    if (miniWindowData_ != NULL)
    {
        closeMiniWindow_();
    }

    if (mainWindowData_ == NULL)
    {
        createMainWindow_();
        return;
    }

    if (!(mainWindowData_->windowIsUp))
    {
        XtPopup(mainWindowData_->popupShell, XtGrabNone);
        XtAddGrab(mainWindowData_->popupShell, true, true);

        if (metadataDetailWindowIsUp_)
        {
            if (missionModified_)
            {
                metadataDetailWindowIsUp_ = false;
                XtDestroyWidget(metadataDetailWindow_);

                addNewMetadataEntryForNewUnsavedMission_(
                    num_robots,
                    mexp_->getAbstractedMissionPlan(config->root()));
            }
            else
            {
                XtManageChild(metadataDetailWindow_);
            }
        }
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function pops up the mini window.
//-----------------------------------------------------------------------
void MissionSpecWizard::popupMiniWindow_(void)
{
    closeMainWindow_();
    createMiniWindow_();
}

//-----------------------------------------------------------------------
// This function shuts down the main window.
//-----------------------------------------------------------------------
void MissionSpecWizard::closeMainWindow_(void)
{
    XtPopdown(mainWindowData_->popupShell);
    XFlush(display_);

    mainWindowData_->windowIsUp = false;
}

//-----------------------------------------------------------------------
// This function shuts donw the mini window.
//-----------------------------------------------------------------------
void MissionSpecWizard::closeMiniWindow_(void)
{
    int i;

    if (miniWindowData_ == NULL)
    {
        return;
    }

    XtRemoveGrab(miniWindowData_->popupShell);
    XmUpdateDisplay(mainWindowData_->popupShell);
    XFlush(display_);

    for (i = 0; i < (int)(miniWindowData_->windows.size()); i++)
    {
        XtDestroyWidget(miniWindowData_->windows[i]);
    }

    XtDestroyWidget(miniWindowData_->popupShell);
    delete miniWindowData_;
    miniWindowData_ = NULL;
}

//-----------------------------------------------------------------------
// This function loops around and catches event from the Widgets until
// waiting_ flag becomes false.
//-----------------------------------------------------------------------
void MissionSpecWizard::waiterLoop_(void)
{
    refreshMainWindow_();

    while(waiting_)
    {
        if (XtAppPending(appContext_))
        {
            XtAppProcessEvent(appContext_, XtIMAll);
        }
        else
        {
            nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);
        }
    }
}

//-----------------------------------------------------------------------
// This function starts the waiter loop.
//-----------------------------------------------------------------------
void MissionSpecWizard::startWaiter_(void)
{
    if (waiting_)
    {
        // Already running.
        return;
    }

    waiting_ = true;
    waiterLoop_();
}

//-----------------------------------------------------------------------
// This function stops the waiter loop.
//-----------------------------------------------------------------------
void MissionSpecWizard::stopWaiter_(void)
{
    waiting_ = false;
}

//-----------------------------------------------------------------------
// This function resets the detail button.
//-----------------------------------------------------------------------
void MissionSpecWizard::resetDetailBtn_(
    bool detailShown,
    bool labelHideDetail,
    bool sensitive)
{
    XmString label;

    detailShown_ = detailShown;

    if (labelHideDetail)
    {
        label = XmStringCreateLocalized((char *)(DETAIL_BUTTON_LABEL_HIDE_.c_str()));
    }
    else
    {
        label = XmStringCreateLocalized((char *)(DETAIL_BUTTON_LABEL_DETAIL_.c_str()));
    }


    XtVaSetValues(
        mainWindowData_->detailBtn,
        XmNlabelString, label,
        NULL);

    XmStringFree(label);

    XtSetSensitive(mainWindowData_->detailBtn, sensitive);
}

//-----------------------------------------------------------------------
// This function resets the next button.
//-----------------------------------------------------------------------
void MissionSpecWizard::resetNextBtn_(bool labelStartOver, bool sensitive)
{
    XmString label;

    if (labelStartOver)
    {
        label = XmStringCreateLocalized((char *)(NEXT_BUTTON_LABEL_RESTART_.c_str()));
    }
    else
    {
        label = XmStringCreateLocalized((char *)(NEXT_BUTTON_LABEL_NEXT_.c_str()));
    }


    XtVaSetValues(
        mainWindowData_->nextBtn,
        XmNlabelString, label,
        NULL);

    XmStringFree(label);

    XtSetSensitive(mainWindowData_->nextBtn, sensitive);
}

//-----------------------------------------------------------------------
// This function closes the main window and opens up the mini window
// to show the FSA mission in CfgEdit.
//-----------------------------------------------------------------------
void MissionSpecWizard::waitAndShowLoadedMissionDetail_(void)
{
    if (missionLoaded_)
    {
        popupMiniWindow_();
        setSelectedMission_(-1);
        mexp_->resumeMissionExpert();
    }
    else
    {
        XtAppAddTimeOut(
            appContext_,
            EVENT_WAITER_SLEEP_TIME_MSEC_,
            (XtTimerCallbackProc)cbWaitAndShowLoadedMissionDetail_,
            this);
    }
}

//-----------------------------------------------------------------------
// This function sets the step number to be specified one.
//-----------------------------------------------------------------------
void MissionSpecWizard::setStep_(int stepNum)
{
    vector<Widget> stackedWidget;
    int i, index = -1;
    int numSteps, numStackedWidget;

    numSteps = numSteps_();

    for (i = 0; i < numSteps; i++)
    {
        if (stepData_()[i].stepNum == stepNum)
        {
            index = i;
            break;
        }
    }
    
    if (index == -1)
    {
        printfInstructionWindow("Internal Error: Unknown step.");
        return;
    }

    // Update the status window.
    for (i = 0; i < numSteps; i++)
    {
        if (i == index)
        {
            XtUnmanageChild(statusRows_[i].label);
            XtUnmanageChild(statusRows_[i].form);

            XtVaSetValues(
                statusRows_[i].form,
                XmNheight, statusRows_[i].h,
                XmNwidth, statusRows_[i].w,
                XmNrecomputeSize, false,
                NULL);

            XtVaSetValues(
                statusRows_[i].label,
                XmNfontList, statusBoldFontList_,
                XmNheight, statusRows_[i].h,
                XmNwidth, statusRows_[i].w,
                XmNforeground, gCfgEditPixels.yellow,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrecomputeSize, false,
                NULL);

            stackedWidget.push_back(statusRows_[i].form);
            stackedWidget.push_back(statusRows_[i].label);
        }
        else if (stepData_()[i].stepNum == currentStep_)
        {
            XtUnmanageChild(statusRows_[i].label);
            XtUnmanageChild(statusRows_[i].form);

            XtVaSetValues(
                statusRows_[i].form,
                XmNheight, statusRows_[i].h,
                XmNwidth, statusRows_[i].w,
                XmNrecomputeSize, false,
                NULL);

            XtVaSetValues(
                statusRows_[i].label,
                XmNfontList, statusNormalFontList_,
                XmNheight, statusRows_[i].h,
                XmNwidth, statusRows_[i].w,
                XmNforeground, gCfgEditPixels.white,
                XmNrightAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNrecomputeSize, false,
                NULL);

            stackedWidget.push_back(statusRows_[i].form);
            stackedWidget.push_back(statusRows_[i].label);
        }
    }

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    currentStep_ = stepNum;

    // Take care other stuff based on the step number.
    processStep_(stepNum);
}

//-----------------------------------------------------------------------
// This function is called when the compilation is failed.
//-----------------------------------------------------------------------
void MissionSpecWizard::setCompilationFailed(void)
{
    XtSetSensitive(mainWindowData_->closeBtn, true);
}

//-----------------------------------------------------------------------
// This function is called when the rehearsal is finished.
//-----------------------------------------------------------------------
void MissionSpecWizard::setDoneRehearsal(void)
{
    setStep_(stepRehearsal_() + 1);
}

//-----------------------------------------------------------------------
// This function is called when the mission is modified at its FSA level.
//-----------------------------------------------------------------------
void MissionSpecWizard::notifyMissionModified(void)
{
    missionModified_ = true;
}

//-----------------------------------------------------------------------
// This function merges two missions into one.
//-----------------------------------------------------------------------
void MissionSpecWizard::mergeTwoMissions_(int missionIndexA, int missionIndexB)
{
    vector<string> acdlMissions;
    string acdlMission;
    int numRobots;

    mergingMissions_ = false;

    acdlMissions.push_back(retrievedMissions_[missionIndexA].acdlSolution);
    acdlMissions.push_back(retrievedMissions_[missionIndexB].acdlSolution);

    acdlMission = mexp_->getMergedAbstractedMissionPlan(
        acdlMissions,
        &numRobots);

    addNewMetadataEntryForNewUnsavedMission_(
        numRobots,
        acdlMission);
}

//-----------------------------------------------------------------------
// This function adds a new metadata entry for the new unsaved modified
// mission.
//-----------------------------------------------------------------------
void MissionSpecWizard::addNewMetadataEntryForNewUnsavedMission_(
    int numRobots,
    string acdlMission)
{
    MExpRetrievedMission_t retrievedMission;

    // Create an empty entry.
    retrievedMission.index = retrievedMissions_.size();
    retrievedMission.rating = -1;
    retrievedMission.numRobots = numRobots;
    retrievedMission.acdlSolution = acdlMission;
    time((time_t *)&(retrievedMission.metadata.creationTimeSec));
    retrievedMission.metadata.numUsage = -1;
    retrievedMission.metadata.relevance = -1;
    memset(&(retrievedMission.metadata.ratings), 0x0, sizeof(double)*NUM_MEXP_METADATA_RATINGS);
    retrievedMission.feedbackSent = false;
    retrievedMission.usageCounterUpdated = false;
    retrievedMission.newUnsavedMission = true;
    retrievedMissions_.push_back(retrievedMission);

    // Update the display.
    clearMetadataFilter_();
    orderMetadata_();
    createDataWindowMetadata_();
    checkMetadataDisplayLineCheckBoxForRetrievedMission_(retrievedMission.index);
}

//-----------------------------------------------------------------------
// This function creates the scrolled mission data.
//-----------------------------------------------------------------------
void MissionSpecWizard::createDataWindowMetadata_(void)
{
    MetaDataDisplayLine_t metaDataDisplayLine;
    MetaDataDisplayLineActionBtnCallbackData_t *actionBtnCallbackData = NULL;
    MetaDataReorderRequestCallbackData_t *reorderRequestCallbackData = NULL;
    Widget mainForm, lineForm, cellForm, buttonForm;
    Widget clipWindow, cell, labelGadget;
    Widget filterBtn, clearBtn, mergeBtn;
    Pixel entryFg, entryBg;
    vector<Widget> stackedWidget;
    string labelString;
    char label[1024];
    int screenNum;
    int displayHeight, displayWidth;
    int h, w;
    int filterLineHeight;
    int i, j, k, index;
    int missionIndex;
    int rightPos, leftPos, topPos, bottomPos;
    int numStackedWidget;
    const int FRACTION_BASE_100 = 100;
    const int FRACTION_BASE_3 = 3;

    clearDataWindowContent_();
    metaDataDisplayLines_.clear();

    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    h = (int)(MAIN2DATA_HEIGHT_RATIO_*(double)displayHeight);
    w = displayWidth - 
        (int)(MAIN2STATUS_WIDTH_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2DATA_X_OFFSET_RATIO_*(double)displayWidth) -
        (int)(MAIN2STATUS_X_OFFSET_RATIO_*(double)displayWidth);

    dataWindowContent_ = XtVaCreateManagedWidget
        ("dataWindowContent_",
         xmScrolledWindowWidgetClass, mainWindowData_->dataWindow,
         XmNheight, h,
         XmNwidth, w,
         XmNscrollBarDisplayPolicy, XmSTATIC,
         XmNscrollingPolicy, XmAUTOMATIC,
         XmNbackground, gCfgEditPixels.white,
         NULL);

    stackedWidget.push_back(dataWindowContent_);

    filterLineHeight = METADATA_FILTER_LINE_HEIGHT_ + METADATA_DISPLAY_LINE_EXTRA_HEIGHT_;

    h = 
        filterLineHeight +
        ((METADATA_DISPLAY_LINE_HEIGHT_ + METADATA_DISPLAY_LINE_EXTRA_HEIGHT_)*(metadataOrder_.size() + 1));

    // Look for ClipWindow in the scrolled window, so that it can be paited white.
    // (Otherwise, it becomes the regular background color.
    clipWindow = XtNameToWidget(
        dataWindowContent_,
        (String)(SLIDER_CLIP_WINDOW_NAME_.c_str()));

    XtVaSetValues(
        clipWindow,
        XmNbackground, gCfgEditPixels.white,
        NULL);

    w = (int)(((double)w)*DATAWINDOW2CONTENTS_WIDTH_RATIO_);

    mainForm = XtVaCreateWidget(
        "mainForm",
        xmFormWidgetClass, dataWindowContent_,
        XmNheight, h,
        XmNwidth, w,
        XmNborderWidth, 0,
        XmNbackground, gCfgEditPixels.white,
        XmNrightAttachment, XmATTACH_POSITION,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNtopAttachment, XmATTACH_POSITION,
        XmNbottomAttachment, XmATTACH_POSITION,
        XmNresizable, false,
        NULL);

    stackedWidget.push_back(mainForm);    

    // Filter
    topPos = 
        (int)((float)(FRACTION_BASE_100*
                      (METADATA_DISPLAY_LINE_Y_OFFSET_))/(float)h);

    bottomPos =
        (int)((float)(FRACTION_BASE_100*
                      (METADATA_FILTER_LINE_HEIGHT_ +
                       METADATA_DISPLAY_LINE_Y_OFFSET_))/(float)h);
        
    lineForm = XtVaCreateManagedWidget
        ("lineForm",
         xmFormWidgetClass, mainForm,
         XmNalignment, XmALIGNMENT_CENTER,
         XmNwidth, w,
         XmNrecomputeSize, false,
         XmNborderWidth, 0,
         XmNbackground, gCfgEditPixels.white,
         XmNfractionBase, FRACTION_BASE_100,
         XmNrightAttachment, XmATTACH_WIDGET,
         XmNrightWidget, mainForm,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, mainForm,
         XmNtopAttachment, XmATTACH_POSITION,
         XmNtopPosition, topPos,
         XmNbottomAttachment, XmATTACH_POSITION,
         XmNbottomPosition, bottomPos,
         XmNresizable, false,
         NULL);

    stackedWidget.push_back(lineForm);

    for (i = 0; i < NUM_METADATA_FILTER_LINE_CONTENTS; i++)
    {
        leftPos = (i == 0)?
            0 : (FRACTION_BASE_100*METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[i-1])/
            METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[NUM_METADATA_FILTER_LINE_CONTENTS-1];

        rightPos = (FRACTION_BASE_100*METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[i])/
            METADATA_FILTER_LINE_CONTENTS_RIGHTPOS_[NUM_METADATA_FILTER_LINE_CONTENTS-1];

        switch (i) {

        case METADATA_FILTER_LINE_MERGE_BUTTON:

            if (mergingMissions_)
            {
                labelString = MERGE_CANCEL_BUTTON_LABEL_;
            }
            else
            {
                labelString = MERGE_MISSIONS_BUTTON_LABEL_;
            }

            mergeBtn = XtVaCreateManagedWidget(
                (char *)labelString.c_str(),
                xmPushButtonGadgetClass, lineForm,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNshowAsDefault, false,
                XmNhighlightThickness, 0,
                XmNfontList, metadataDisplayFontList_,
                XmNbackground, gCfgEditPixels.gray90,
                NULL);

            stackedWidget.push_back(mergeBtn);

            XtAddCallback(
                mergeBtn,
                XmNactivateCallback,
                (XtCallbackProc)cbMergeBtnPress_,
                (void *)this);
            break;

        case METADATA_FILTER_LINE_TYPE_IN:
            cell = XtVaCreateManagedWidget(
                "cell",
                xmFormWidgetClass, lineForm,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbackground, gCfgEditPixels.white,
                XmNborderWidth, 0,
                XmNborderColor, gCfgEditPixels.white,
                XmNrecomputeSize, false,
                XmNbackground, gCfgEditPixels.white,
                NULL);

            stackedWidget.push_back(cell);

            metadataFilterTypeInBox_ = XtVaCreateManagedWidget(
                "metadataFilterTypeInBox_",
                xmTextWidgetClass, cell,
                XmNvalue, metadataFilterString_.c_str(),
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_FORM,
                XmNrightAttachment, XmATTACH_FORM,
                XmNfontList, metadataDisplayFontList_,
                XmNbackground, gCfgEditPixels.white,
                XmNborderWidth, 1,
                XmNhighlightThickness, 0,
                XmNrows, 1,
                XmNeditable, true,
                NULL);

            stackedWidget.push_back(metadataFilterTypeInBox_);

            XtAddCallback(
                metadataFilterTypeInBox_,
                XmNvalueChangedCallback,
                (XtCallbackProc)cbMetadataFilterStringUpdate_,
                (void *)this);

            XtAddCallback(
                metadataFilterTypeInBox_,
                XmNactivateCallback,
                (XtCallbackProc)cbMetadataFilter_,
                (void *)this);
            break;

        case METADATA_FILTER_LINE_FILTER_BUTTON:
            filterBtn = XtVaCreateManagedWidget(
                (char *)(METADATA_FILTER_BUTTON_LABEL_.c_str()),
                xmPushButtonGadgetClass, lineForm,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNshowAsDefault, false,
                XmNhighlightThickness, 0,
                XmNfontList, metadataDisplayFontList_,
                XmNbackground, gCfgEditPixels.gray90,
                NULL);
            stackedWidget.push_back(filterBtn);

            XtAddCallback(
                filterBtn,
                XmNactivateCallback,
                (XtCallbackProc)cbMetadataFilter_,
                (void *)this);
            break;

        case METADATA_FILTER_LINE_CLEAR_BUTTON:
            clearBtn = XtVaCreateManagedWidget(
                (char *)(METADATA_FILTER_CLEAR_BUTTON_LABEL_.c_str()),
                xmPushButtonGadgetClass, lineForm,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNshowAsDefault, false,
                XmNhighlightThickness, 0,
                XmNfontList, metadataDisplayFontList_,
                XmNbackground, gCfgEditPixels.gray90,
                NULL);
            stackedWidget.push_back(clearBtn);

            XtAddCallback(
                clearBtn,
                XmNactivateCallback,
                (XtCallbackProc)cbMetadataFilterClear_,
                (void *)this);
            break;

        default:
            cell = XtVaCreateManagedWidget(
                "cell",
                xmFormWidgetClass, lineForm,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNtopAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNbackground, gCfgEditPixels.white,
                //XmNborderWidth, 1,
                XmNborderWidth, 0,
                XmNborderColor, gCfgEditPixels.white,
                XmNrecomputeSize, false,
                XmNbackground, gCfgEditPixels.white,
                NULL);

            stackedWidget.push_back(cell);

            break;
        }
    }

    // Display meta data lines.
    for (i = 0; i < (int)(metadataOrder_.size() + 1); i++)
    {
        //missionIndex = i - 1;
        missionIndex = (i == 0)? (-1) : metadataOrder_[i-1];

        topPos = 
            (int)((float)(FRACTION_BASE_100*
                          (filterLineHeight +
                           (i*METADATA_DISPLAY_LINE_HEIGHT_) +
                           METADATA_DISPLAY_LINE_Y_OFFSET_))/(float)h);

        bottomPos =
            (int)((float)(FRACTION_BASE_100*
                          (filterLineHeight +
                           ((i+1)*METADATA_DISPLAY_LINE_HEIGHT_) +
                           METADATA_DISPLAY_LINE_Y_OFFSET_))/(float)h);
        
        lineForm = XtVaCreateManagedWidget
            ("lineForm",
             xmFormWidgetClass, mainForm,
             XmNalignment, XmALIGNMENT_CENTER,
             XmNwidth, w,
             XmNrecomputeSize, false,
             XmNborderWidth, 0,
             XmNbackground, gCfgEditPixels.white,
             XmNfractionBase, FRACTION_BASE_100,
             XmNrightAttachment, XmATTACH_WIDGET,
             XmNrightWidget, mainForm,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftWidget, mainForm,
             XmNtopAttachment, XmATTACH_POSITION,
             XmNtopPosition, topPos,
             XmNbottomAttachment, XmATTACH_POSITION,
             XmNbottomPosition, bottomPos,
             XmNresizable, false,
             NULL);

        stackedWidget.push_back(lineForm);

        for (j = 0; j < NUM_METADATA_DISPLAY_CONTENTS; j++)
        {
            leftPos = (j == 0)?
                0 : (FRACTION_BASE_100*METADATA_DISPLAY_CONTENTS_RIGHTPOS_[j-1])/
                METADATA_DISPLAY_CONTENTS_RIGHTPOS_[NUM_METADATA_DISPLAY_CONTENTS-1];

            rightPos = (FRACTION_BASE_100*METADATA_DISPLAY_CONTENTS_RIGHTPOS_[j])/
                METADATA_DISPLAY_CONTENTS_RIGHTPOS_[NUM_METADATA_DISPLAY_CONTENTS-1];


            cell = XtVaCreateManagedWidget(
                (char *)METADATA_DISPLAY_CONTENTS_LABELS_[j].c_str(),
                xmFrameWidgetClass, lineForm,
                XmNalignment, XmALIGNMENT_CENTER,
                XmNtopAttachment, XmATTACH_FORM,
                XmNbottomAttachment, XmATTACH_FORM,
                XmNleftAttachment, XmATTACH_POSITION,
                XmNleftPosition, leftPos,
                XmNrightAttachment, XmATTACH_POSITION,
                XmNrightPosition, rightPos,
                XmNbackground, gCfgEditPixels.white,
                //XmNborderWidth, 1,
                XmNborderWidth, 0,
                XmNborderColor, gCfgEditPixels.black,
                XmNrecomputeSize, false,
                NULL);

            stackedWidget.push_back(cell);

            if (i == 0)
            {
                // Label line

                switch (j) {

                case METADATA_DISPLAY_NUMBER:
                case METADATA_DISPLAY_NUM_ROBOTS:
                case METADATA_DISPLAY_STEALTH_RATING:
                case METADATA_DISPLAY_EFFICIENCY_RATING:
                case METADATA_DISPLAY_PRESERVATION_RATING:
                case METADATA_DISPLAY_DURATION_RATING:
                case METADATA_DISPLAY_RELEVANCE:
                case METADATA_DISPLAY_USER_COMMENTS:
                case METADATA_DISPLAY_NUM_USAGE:
                    labelGadget = XtVaCreateManagedWidget(
                        (char *)METADATA_DISPLAY_CONTENTS_LABELS_[j].c_str(),
                        xmPushButtonWidgetClass, cell,
                        XmNfontList, metadataDisplayFontList_,
                        XmNalignment, XmALIGNMENT_CENTER,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNrightWidget, cell,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNleftWidget, cell,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, cell,
                        XmNbottomAttachment, XmATTACH_WIDGET,
                        XmNbottomWidget, cell,
                        XmNborderWidth, 0,
                        XmNhighlightThickness, 0,
                        XmNdefaultButtonShadowThickness, 0,
                        XmNforeground, gCfgEditPixels.white,
                        XmNbackground, gCfgEditPixels.gray40,
                        XmNborderColor, gCfgEditPixels.gray40,
                        XmNrecomputeSize, false,
                        XmNshowAsDefault, false,
                        NULL);

                    stackedWidget.push_back(labelGadget);

                    reorderRequestCallbackData = new MetaDataReorderRequestCallbackData_t;
                    reorderRequestCallbackData->metadataOrderReference = j;
                    reorderRequestCallbackData->missionSpecWizardInstance = this;

                    XtAddCallback(
                        labelGadget,
                        XmNactivateCallback,
                        (XtCallbackProc)cbMetadataReorder_,
                        (void *)reorderRequestCallbackData);
                    break;

                default:
                    labelGadget = XtVaCreateManagedWidget(
                        (char *)METADATA_DISPLAY_CONTENTS_LABELS_[j].c_str(),
                        xmLabelGadgetClass, cell,
                        XmNfontList, metadataDisplayFontList_,
                        XmNalignment, XmALIGNMENT_CENTER,
                        XmNrightAttachment, XmATTACH_WIDGET,
                        XmNrightWidget, cell,
                        XmNleftAttachment, XmATTACH_WIDGET,
                        XmNleftWidget, cell,
                        XmNtopAttachment, XmATTACH_WIDGET,
                        XmNtopWidget, cell,
                        XmNbottomAttachment, XmATTACH_WIDGET,
                        XmNbottomWidget, cell,
                        XmNborderWidth, 0,
                        XmNforeground, gCfgEditPixels.white,
                        XmNbackground, gCfgEditPixels.gray40,
                        XmNrecomputeSize, false,
                        NULL);

                    stackedWidget.push_back(labelGadget);

                    break;
                }
            }
            else
            {
                if ((i % 2))
                {
                    entryBg = gCfgEditPixels.white;
                }
                else
                {
                    entryBg = gCfgEditPixels.gray90;
                }

                if (!retrievedMissions_[missionIndex].newUnsavedMission)
                {
                    entryFg = parentWidgetFg_;
                }
                else
                {
                    entryFg = gCfgEditPixels.red;
                }

                switch (j) {

                case METADATA_DISPLAY_NUMBER:

                    sprintf(label, "%d", retrievedMissions_[missionIndex].index + 1);

                    if (!retrievedMissions_[missionIndex].newUnsavedMission)
                    {
                        entryFg = parentWidgetFg_;
                    }
                    else
                    {
                        sprintf(label, "%s [New]", label);
                        entryFg = gCfgEditPixels.red;
                    }

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_NUM_ROBOTS:

                    if (retrievedMissions_[missionIndex].numRobots >= 0)
                    {
                        sprintf(
                            label,
                            "%d",
                            retrievedMissions_[missionIndex].numRobots);
                    }
                    else
                    {
                        sprintf(label, "%s", STRING_NA_.c_str());
                    }

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_CHECK_BOX:
                    cellForm = XtVaCreateManagedWidget
                        ("cellForm",
                         xmFormWidgetClass, cell,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrecomputeSize, false,
                         XmNborderWidth, 0,
                         XmNbackground, entryBg,
                         XmNfractionBase, FRACTION_BASE_3,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNresizable, false,
                         NULL);

                    stackedWidget.push_back(cellForm);

                    sprintf(label, "%d", i);
                    metaDataDisplayLine.checkBox = XtVaCreateManagedWidget
                        (label,
                         xmToggleButtonGadgetClass, cellForm,
                         XmNfontList, metadataDisplayFontList_,
                         XmNradioBehavior, true,
                         XmNradioAlwaysOne, true,
                         XmNset, false,
                         XmNalignment, XmALIGNMENT_BEGINNING,
                         XmNhighlightColor, entryBg,
                         XmNforeground, entryBg,
                         XmNbackground, entryBg,
                         XmNborderWidth, 1,
                         XmNleftAttachment, XmATTACH_POSITION,
                         XmNleftPosition, 1,
                         XmNrightAttachment, XmATTACH_POSITION,
                         XmNrightPosition, 3,
                         XmNtopAttachment, XmATTACH_FORM,
                         XmNbottomAttachment, XmATTACH_FORM,
                         NULL);

                    stackedWidget.push_back(metaDataDisplayLine.checkBox);

                    XtAddCallback(
                        metaDataDisplayLine.checkBox,
                        XmNvalueChangedCallback,
                        (XtCallbackProc)cbMetadataDisplayLineCheckBox_,
                        this);
                    break;

                case METADATA_DISPLAY_STEALTH_RATING:
                case METADATA_DISPLAY_EFFICIENCY_RATING:
                case METADATA_DISPLAY_PRESERVATION_RATING:
                case METADATA_DISPLAY_DURATION_RATING:
                    index = -1;
                    sprintf(label, "%s", STRING_NA_.c_str());

                    for (k = 0; k < NUM_MEXP_METADATA_RATINGS; k++)
                    {
                        if (MISSION_RATING_INDEX_2_METADATA_DISPLAY_INDEX_[k] == j)
                        {
                            index = k;
                            break;
                        }
                    }

                    if (index >= 0)
                    {
                        if (retrievedMissions_[missionIndex].metadata.ratings[index] > 0)
                        {
                            sprintf(
                                label,
                                "%.0f",
                                retrievedMissions_[missionIndex].metadata.ratings[index]);
                        }
                    }

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_RELEVANCE:

                    if (retrievedMissions_[missionIndex].metadata.relevance >= 0)
                    {
                        sprintf(
                            label,
                            "%.1f%%",
                            100.0*retrievedMissions_[missionIndex].metadata.relevance);
                    }
                    else
                    {
                        sprintf(label, "%s", STRING_NA_.c_str());
                    }

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_USER_COMMENTS:

                    sprintf(
                        label,
                        "%d",
                        retrievedMissions_[missionIndex].metadata.userComments.size());

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_NUM_USAGE:

                    if (retrievedMissions_[missionIndex].metadata.numUsage >= 0)
                    {
                        sprintf(
                            label,
                            "%d",
                            retrievedMissions_[missionIndex].metadata.numUsage);
                    }
                    else
                    {
                        sprintf(label, "%s", STRING_NA_.c_str());
                    }

                    labelGadget = XtVaCreateManagedWidget
                        (label,
                         xmLabelGadgetClass, cell,
                         XmNfontList, metadataDisplayFontList_,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNborderWidth, 0,
                         XmNforeground, entryFg,
                         XmNbackground, entryBg,
                         XmNrecomputeSize, false,
                         NULL);

                    stackedWidget.push_back(labelGadget);
                    break;

                case METADATA_DISPLAY_BUTTONS:
                    cellForm = XtVaCreateManagedWidget
                        ("cellForm",
                         xmFormWidgetClass, cell,
                         XmNalignment, XmALIGNMENT_CENTER,
                         XmNrecomputeSize, false,
                         XmNborderWidth, 0,
                         XmNbackground, entryBg,
                         XmNfractionBase, NUM_METADATA_DISPLAY_BTNS,
                         XmNrightAttachment, XmATTACH_WIDGET,
                         XmNrightWidget, cell,
                         XmNleftAttachment, XmATTACH_WIDGET,
                         XmNleftWidget, cell,
                         XmNtopAttachment, XmATTACH_WIDGET,
                         XmNtopWidget, cell,
                         XmNbottomAttachment, XmATTACH_WIDGET,
                         XmNbottomWidget, cell,
                         XmNresizable, false,
                         NULL);

                    stackedWidget.push_back(cellForm);

                    for (k = 0; k < NUM_METADATA_DISPLAY_BTNS; k++)
                    {
                        buttonForm = XtVaCreateManagedWidget
                            ("buttonForm",
                             xmFormWidgetClass, cellForm,
                             XmNshadowThickness, 0,
                             XmNalignment, XmALIGNMENT_CENTER,
                             XmNtopAttachment, XmATTACH_FORM,
                             XmNbottomAttachment, XmATTACH_FORM,
                             XmNleftAttachment, XmATTACH_POSITION,
                             XmNleftPosition, k,
                             XmNrightAttachment, XmATTACH_POSITION,
                             XmNrightPosition, k+1,
                             XmNforeground, entryFg,
                             XmNbackground, entryBg,
                             NULL);

                        stackedWidget.push_back(buttonForm);

                        metaDataDisplayLine.btn[k] = XtVaCreateManagedWidget(
                            (char *)METADATA_DISPLAY_BUTTON_LABELS_[k].c_str(),
                            xmPushButtonGadgetClass, buttonForm,
                            XmNfontList, metadataDisplayFontList_,
                            XmNshowAsDefault, true,
                            XmNdefaultButtonShadowThickness, 1,
                            XmNborderWidth, 1,
                            XmNforeground, entryFg,
                            XmNbackground, entryBg,
                            XmNborderColor, entryBg,
                            XmNhighlightThickness, 0,
                            XmNtopAttachment, XmATTACH_FORM,
                            XmNbottomAttachment, XmATTACH_FORM,
                            XmNleftAttachment, XmATTACH_FORM,
                            XmNrightAttachment, XmATTACH_FORM,
                            NULL);

                        stackedWidget.push_back(metaDataDisplayLine.btn[k]);

                        actionBtnCallbackData = new MetaDataDisplayLineActionBtnCallbackData_t;
                        actionBtnCallbackData->selectedMission = missionIndex;
                        actionBtnCallbackData->buttonNum = k;
                        actionBtnCallbackData->missionSpecWizardInstance = this;

                        XtAddCallback(
                            metaDataDisplayLine.btn[k],
                            XmNactivateCallback,
                            (XtCallbackProc)cbMetadataDisplayLineActionBtn_,
                            (void *)actionBtnCallbackData);
                    }
                    break;
                }
            }
        }

        if (i > 0)
        {
            metaDataDisplayLine.number = missionIndex;
            metaDataDisplayLines_.push_back(metaDataDisplayLine);
        }
    }

    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    refreshMainWindow_();
}

//-----------------------------------------------------------------------
// This function generates a text for meta data detail window.
//-----------------------------------------------------------------------
string MissionSpecWizard::generateMetadataDetailWindowText_(int selectedMission)
{
    MExpRetrievedMission_t retrievedMission;
    MExpRetrievedMissionSummary_t summary;
    string text = EMPTY_STRING_, comment;
    char buf[1024];
    int i, j;

    retrievedMission = retrievedMissions_[selectedMission];

    // Mission #
    sprintf(buf, "Mission Number: #%d\n", (selectedMission + 1));
    text += buf;

    // Creation Time
    if (retrievedMission.metadata.creationTimeSec > 0)
    {
        sprintf(
            buf,
            "Created on: %s",
            ctime((time_t *)&(retrievedMission.metadata.creationTimeSec)));
        text += buf;

        if (retrievedMission.newUnsavedMission)
        {
            text[text.size() - 1] = ' ';
            text += "(*NEW*)\n";
        }
    }

    text += "\n";
    text += "\n";

    // # of Robots
    text += "Number of Robots: ";
    if (retrievedMission.numRobots >= 0)
    {
        sprintf(
            buf,
            "%d\n",
            retrievedMission.numRobots);
    }
    else
    {
        sprintf(buf, "%s\n", STRING_NA_.c_str());
    }
    text += buf;
    text += "\n";

    // Ratings
    sprintf(
        buf,
        "Average Ratings (1 = Lowest; %d = Highest):\n",
        maxSuitabilityRating_);
    text += buf;
    for (i = 0; i < NUM_MEXP_METADATA_RATINGS; i++)
    {
        sprintf(buf, " - %s: ", METADATA_DETAIL_WINDOW_RATING_NAMES_[i].c_str());
        text += buf;

        if (retrievedMission.metadata.ratings[i] > 0)
        {
            sprintf(
                buf,
                "%.0f\n",
                retrievedMission.metadata.ratings[i]);
        }
        else
        {
            sprintf(buf, "%s\n", STRING_NA_.c_str());
        }
        text += buf;
    }
    text += "\n";
    text += "\n";

    // Relevance
    text += "Computed Relevance: ";
    if (retrievedMission.metadata.relevance >= 0)
    {
        sprintf(
            buf,
            "%.1f%%\n",
            100.0*retrievedMission.metadata.relevance);
    }
    else
    {
        sprintf(buf, "%s\n", STRING_NA_.c_str());
    }
    text += buf;
    text += "\n";

    // # of Usage
    text += "Number of Usage: ";
    if (retrievedMission.metadata.numUsage >= 0)
    {
        sprintf(
            buf,
            "%d\n",
            retrievedMission.metadata.numUsage);
    }
    else
    {
        sprintf(buf, "%s\n", STRING_NA_.c_str());
    }
    text += buf;
    text += "\n";

    // Comments
    text += "\n";
    text += "\n";
    text += "Comments:";
    if (retrievedMission.metadata.userComments.size() > 0)
    {
        text += "\n";

        for (i = 0; i < (int)(retrievedMission.metadata.userComments.size()); i++)
        {
            text += "\n";
            comment = retrievedMission.metadata.userComments[i];

            for (j = 0; j < NUM_MEXP_METADATA_SPECIAL_TEXTS; j++)
            {
                comment = replaceStringInString(
                    comment,
                    MEXP_METADATA_SPECIAL_TEXT_CONVERSION[j].convertedText,
                    MEXP_METADATA_SPECIAL_TEXT_CONVERSION[j].specialText);
            }

            text += comment;
            text += "\n";
        }
    }
    else
    {
        sprintf(buf, " %s\n", STRING_NA_.c_str());
        text += buf;
    }
    text += "\n";

    // Summary
    text += "\n\n";
    summary = MissionExpert::summarizeRetrievedMissions(retrievedMissions_);
    text += summary.fsaSummaries[selectedMission]; 

    return text;
}

//-----------------------------------------------------------------------
// This function checks the metadata check box for the specified retrieved
// mission.
//-----------------------------------------------------------------------
void MissionSpecWizard::checkMetadataDisplayLineCheckBoxForRetrievedMission_(
    int retrievedMissionIndex)
{
    int i;
    bool selected = false;
    bool noSelection = true;

    selectedCheckBox_ = -1;

    for (i = 0; i < (int)(metaDataDisplayLines_.size()); i++)
    {
        if (metaDataDisplayLines_[i].number == retrievedMissionIndex)
        {
            selected = true;
            noSelection = false;
            selectedCheckBox_ = i;
        }
        else
        {
            selected = false;
        }

        XtVaSetValues(
            metaDataDisplayLines_[i].checkBox,
            XmNset, selected,
            NULL);
    }

    XtSetSensitive(mainWindowData_->nextBtn, ((noSelection)? false : true));
    XtSetSensitive(mainWindowData_->detailBtn, ((noSelection)? false : true));
}

//-----------------------------------------------------------------------
// This function gets called when the "Merge Missions" button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::processMergeButton_(Widget mergeBtn)
{
    XmString label;

    mergingMissions_ = (!mergingMissions_);

    if (mergingMissions_)
    {
        label = XmStringCreateLocalized((char *)(MERGE_CANCEL_BUTTON_LABEL_.c_str()));
    }
    else
    {
        label = XmStringCreateLocalized((char *)(MERGE_MISSIONS_BUTTON_LABEL_.c_str()));
    }


    XtVaSetValues(
        mergeBtn,
        XmNlabelString, label,
        NULL);

    XmStringFree(label);

    if (mergingMissions_)
    {
        mexp_->confirmUser(
            MERGE_MISSIONS_INSTRUCTION_,
            true,
            mainWindowData_->popupShell);
    }
}

//-----------------------------------------------------------------------
// This function updates the check boxes when one of them is clicked.
//-----------------------------------------------------------------------
void MissionSpecWizard::updateMetadataDisplayLineCheckBox_(int selectedCheckBox)
{
    char phrase[1024];
    int i;
    bool select;
    bool noSelection = true, merge = false;

    if (selectedCheckBox >= (int)(metaDataDisplayLines_.size()))
    {
        printfInstructionWindow("Error: Invalid selectedCheckBox = %d\n", selectedCheckBox);
        return;
    }

    if ((mergingMissions_) &&
        (selectedCheckBox >= 0) &&
        (selectedCheckBox_ >= 0) &&
        (selectedCheckBox != selectedCheckBox_))
    {
        sprintf(
            phrase,
            "Merge missions %d and %d?",
            (metaDataDisplayLines_[selectedCheckBox_].number + 1),
            (metaDataDisplayLines_[selectedCheckBox].number + 1));

        merge = mexp_->confirmUser(phrase, false, mainWindowData_->popupShell);

        if (merge)
        {
            mergeTwoMissions_(
                metaDataDisplayLines_[selectedCheckBox_].number,
                metaDataDisplayLines_[selectedCheckBox].number);

            return;
        }
    }

    for (i = 0; i < (int)(metaDataDisplayLines_.size()); i++)
    {
        if (i == selectedCheckBox)
        {
            if (i == selectedCheckBox_)
            {
                // Already selected last time. Unselect it.
                select = false;
            }
            else
            {
                select = true;
                noSelection = false;
                selectedCheckBox_ = i;
            }
        }
        else
        {
            select = false;
        }

        XtVaSetValues(
            metaDataDisplayLines_[i].checkBox,
            XmNset, select,
            NULL);
    }

    if (noSelection)
    {
        selectedCheckBox_ = -1;
    }

    XtSetSensitive(mainWindowData_->nextBtn, ((noSelection)? false : true));
    XtSetSensitive(mainWindowData_->detailBtn, ((noSelection)? false : true));
}

//-----------------------------------------------------------------------
// This function gets called when the Assemblage button for the meta
// data detail window was pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::processMetadataDetailWindowAssemblageBtn_(int selectedMission)
{
    XtUnmanageChild(metadataDetailWindow_);
    XFlush(display_);

    noRehearsal_ = true;
    setSelectedMission_(selectedMission);
    waitAndShowLoadedMissionDetail_();
}

//-----------------------------------------------------------------------
// This callback function gets called when one of the action buttons is
// pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::processMetadataDisplayLineActionBtn_(int selectedMission, int buttonNum)
{
    switch (buttonNum) {
        
    case METADATA_DISPLAY_BTN_DETAIL:
        popupMetadataDetailWindow_(selectedMission);
        break;

    case METADATA_DISPLAY_BTN_REHERASE:
        selectedCheckBox_ = -1;
        setSelectedMission_(selectedMission);
        setStep_(stepRehearsal_());
        break;

    case METADATA_DISPLAY_BTN_SEND:
        noRehearsal_ = true;
        selectedCheckBox_ = -1;
        setSelectedMission_(selectedMission);
        setStep_(stepCompletion_());
	gEventLogging->log("Sent Mission");
        break;
    }
}

//-----------------------------------------------------------------------
// This function creates the metadata detail window.
//-----------------------------------------------------------------------
void MissionSpecWizard::popupMetadataDetailWindow_(int selectedMission)
{
    MetadataDetailWindowOKBtnCallbackData_t *okCallbackData = NULL;
    MetadataDetailWindowAssemblageBtnCallbackData_t *assemblageCallbackData = NULL;
    vector<Widget> stackedWidget;
    Widget frame, scrolledWindow;
    Arg args[100];
    XmString title, okLabel, cancelLabel, assemblageLabel;
    string text;
    int screenNum;
    int displayHeight, displayWidth, windowWidth, windowHeight;
    int i, numStackedWidget;
    int n;

    if (selectedMission < 0)
    {
        fprintf(
            stderr,
            "Error: MissionSpecWizard::popupMetadataDetailWindow_(). Invalid selected mission (%d).\n",
            selectedMission_);

        return;
    }

    // Get the display dimension.
    screenNum = DefaultScreen(display_);
    displayHeight = DisplayHeight(display_, screenNum);
    displayWidth = DisplayWidth(display_, screenNum);

    windowHeight = (int)(DISPLAY2META_HEIGHT_RATIO_*(double)displayHeight);
    windowWidth = (int)(DISPLAY2META_WIDTH_RATIO_*(double)displayWidth);

    // Create labels
    title = XmStringCreateLtoR(
        (String)METADATA_DETAIL_WINDOW_TITLE_.c_str(),
        XmSTRING_DEFAULT_CHARSET);
    okLabel = XmStringCreateLtoR(
        (String)METADATA_DETAIL_WINDOW_OK_LABEL_.c_str(),
        XmSTRING_DEFAULT_CHARSET);
    cancelLabel = XmStringCreateLtoR(
        (String)METADATA_DETAIL_WINDOW_CANCEL_LABEL_.c_str(),
        XmSTRING_DEFAULT_CHARSET);
    assemblageLabel = XmStringCreateLtoR(
        (String)METADATA_DETAIL_WINDOW_ASSEMBLAGE_LABEL_.c_str(),
        XmSTRING_DEFAULT_CHARSET);

    // Create the message dialog.
    n = 0;
    XtSetArg(args[n], XmNdialogTitle, title); n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(args[n], XmNokLabelString, okLabel); n++;
    XtSetArg(args[n], XmNcancelLabelString, cancelLabel); n++;
    XtSetArg(args[n], XmNhelpLabelString, assemblageLabel); n++;

    metadataDetailWindow_ = XmCreateMessageDialog(
        mainWindowData_->popupShell,
        "metadataDetailWindow_",
        args,
        n);
  
    XmStringFree(title);
    XmStringFree(okLabel);
    XmStringFree(cancelLabel);
    XmStringFree(assemblageLabel);

    okCallbackData = new MetadataDetailWindowOKBtnCallbackData_t;
    okCallbackData->selectedMission = selectedMission;
    okCallbackData->missionSpecWizardInstance = this;

    XtAddCallback(
        metadataDetailWindow_,
        XmNokCallback,
        (XtCallbackProc)cbMetadataDetailWindowOKBtn_,
        (void *)okCallbackData);

    XtAddCallback(
        metadataDetailWindow_,
        XmNcancelCallback,
        (XtCallbackProc)cbMetadataDetailWindowCancelBtn_,
        this);

    assemblageCallbackData = new MetadataDetailWindowAssemblageBtnCallbackData_t;
    assemblageCallbackData->selectedMission = selectedMission;
    assemblageCallbackData->missionSpecWizardInstance = this;

    XtAddCallback(
        metadataDetailWindow_,
        XmNhelpCallback,
        (XtCallbackProc)cbMetadataDetailWindowAssemblageBtn_,
        (void *)assemblageCallbackData);

    frame = XtVaCreateWidget(
        "frame",
        xmFrameWidgetClass, metadataDetailWindow_,
        XmNshadowType, XmSHADOW_ETCHED_OUT,
        XmNheight, windowHeight,
        XmNwidth, windowWidth,
        NULL);

    stackedWidget.push_back(frame);

    // Create Text
    text = generateMetadataDetailWindowText_(selectedMission);

    n = 0;
    XtSetArg(args[n], XmNscrollVertical, true); n++;
    XtSetArg(args[n], XmNscrollHorizontal, false); n++;
    XtSetArg(args[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNwordWrap, true); n++;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); n++;
    XtSetArg(args[n], XmNeditable, false); n++;
    XtSetArg(args[n], XmNcursorPositionVisible, false); n++;
    XtSetArg(args[n], XmNbackground, gCfgEditPixels.white); n++;
    XtSetArg(args[n], XmNborderWidth, 0); n++;
    XtSetArg(args[n], XmNmappedWhenManaged, true); n++;
    XtSetArg(args[n], XmNvalue, text.c_str()); n++;

    scrolledWindow = XmCreateScrolledText(
        frame,
        "scrolledWindow",
        args,
        n);

    stackedWidget.push_back(scrolledWindow);

    // Manage the windows.
    numStackedWidget = stackedWidget.size();

    for (i = 0; i < numStackedWidget; i++)
    {
        XtManageChild(stackedWidget[i]);
    }

    XtManageChild(metadataDetailWindow_);

    stackedWidget.clear();

    metadataDetailWindowIsUp_ = true;
}

//-----------------------------------------------------------------------
// Move up a widget hierarchy until a shell widget is reached
//-----------------------------------------------------------------------
Widget MissionSpecWizard::getTopShell_(Widget w)
{
    while ((w != NULL) && (!XtIsWMShell(w)))
    {
        w = XtParent(w);
    }

    return w;
}

//-----------------------------------------------------------------------
// This function finds an overlay file that is closest to the specified
// geographic coordinates.
//-----------------------------------------------------------------------
string MissionSpecWizard::findClosestOverlay_(float latitude, float longitude)
{
    FILE *outputFile = NULL;
    string overlayName = EMPTY_STRING_, latString = EMPTY_STRING_, lonString = EMPTY_STRING_;
    string overlayNameKeep = EMPTY_STRING_;
    string cmd;
    string bufString1, bufString2;
    string::size_type index;
    char tmpFileName[512];
    char buf1[1024], buf2[1024], buf3[1024], buf4[1024];
    float ovlLatitude, ovlLongitude, diff, smallestDiff;
    int fd;

    // Name the temporary files that will be used in this function.
    sprintf(tmpFileName, "/tmp/%s-bamsWizard-findClosestOverlay_-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpFileName);
    unlink(tmpFileName);

    // Run grep
    cmd = "grep " + STRING_GLOBAL_REF_POINT_ + " *.ovl >& ";
    cmd += ((char *)tmpFileName);
    system(cmd.c_str());

    // If success, grep should return:
    // "overlayfilename.ovl:GLOBAL_REF_POINT x y lat lon".
    outputFile = fopen(tmpFileName, "r");
    
    smallestDiff = sqrt(2 * pow(360.0, 2));

    while (fscanf(outputFile, "%s", buf1) != EOF)
    {
        bufString1 = buf1;
        bufString2 = buf2;

        if (bufString1 == "grep:")
        {
            // Error.
            break;
        }
        else if (bufString1[0] == '-')
        {
            // It's a comment.
            while ((fgetc(outputFile) != '\n') && (!feof(outputFile)))
            {
                // Keep reading until next line.
            }
        }
        else
        {
            overlayName = bufString1;

            if (overlayName[overlayName.size() - 1] == ':')
            {
                // "overlayfilename.ovl: GLOBAL_REF_POINT lat lon" format
                overlayName = overlayName.substr(0, overlayName.size() - 1);

                if (fscanf(outputFile, "%s %s %s", buf2, buf3, buf4) == EOF)
                {
                    fprintf(stderr, "Warning: MissionSpecWizard::findClosestOverlay_(). Control charactor ':' scanf failed.\n");
                    continue;
                }

                latString = buf3;
                lonString = buf4;
            }
            else
            {
                // "overlayfilename.ovl:GLOBAL_REF_POINT lat lon" format
                index = overlayName.find(':');

                if (index == string::npos)
                {
                    fprintf(
                        stderr,
                        "Warning: MissionSpecWizard::findClosestOverlay_(). Control charactor ':' not found in [%s].\n",
                        overlayName.c_str());
                    continue;
                }

                overlayName = overlayName.substr(0, index);

                if (fscanf(outputFile, "%s %s", buf2, buf3) == EOF)
                {
                    fprintf(stderr, "Warning: MissionSpecWizard::findClosestOverlay_(). Control charactor ':' scanf failed.\n");
                    continue;
                }

                latString = buf2;
                lonString = buf3;
            }

            ovlLatitude = atof(latString.c_str());
            ovlLongitude = atof(lonString.c_str());
            diff = sqrt(pow(ovlLatitude - latitude, 2) + pow(ovlLongitude - longitude, 2));

            if (diff <= smallestDiff)
            {
                overlayNameKeep = overlayName;
                smallestDiff = diff;
            }

            continue;
        }
    }

    fclose(outputFile);
    unlink(tmpFileName);

    return overlayNameKeep;
}

//-----------------------------------------------------------------------
// This function checks the current time.
//-----------------------------------------------------------------------
void MissionSpecWizard::checkCurrentTime_(
    int *year,
    int *month,
    int *day,
    int *hour,
    int *minute,
    int *second)
{
    struct tm *tmStruct = NULL;
    time_t currentTime;
    const int EPOCH_YEAR = 1900;
    const int MONTH_OFFSET = 1;

    currentTime = time(NULL);
    tmStruct = localtime(&currentTime);

    *year = tmStruct->tm_year + EPOCH_YEAR;
    *month = tmStruct->tm_mon + MONTH_OFFSET;
    *day = tmStruct->tm_mday;
    *hour = tmStruct->tm_hour;
    *minute = tmStruct->tm_min;
    *second = tmStruct->tm_sec;
}

//-----------------------------------------------------------------------
// This function shows loaded FSA mission. This function waits until
// the mission is loaded in CfgEdit. We use XtTimerCallbackProc instead
// of XtAppProcessEvent because it conflicts with saveRetrievedMission_().
//-----------------------------------------------------------------------
XtTimerCallbackProc MissionSpecWizard::cbWaitAndShowLoadedMissionDetail_(XtPointer clientData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->waitAndShowLoadedMissionDetail_();

    return false;
}

//-----------------------------------------------------------------------
// This callback function gets called when Back button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbBackBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->processBackBtn_();
}

//-----------------------------------------------------------------------
// This callback function gets called when Close button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbCloseBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->closeMainWindow_();
}

//-----------------------------------------------------------------------
// This callback function gets called when Detail button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbDetailBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->processDetailBtn_();
}

//-----------------------------------------------------------------------
// This callback function gets called when Next button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbNextBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->processNextBtn_();
}

//-----------------------------------------------------------------------
// This callback function gets called when Respecify button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbRespecifyBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecInstance = NULL;

    missionSpecInstance = (MissionSpecWizard *)clientData;
    missionSpecInstance->setStep_(missionSpecInstance->stepSpecifyMission_());
}

//-----------------------------------------------------------------------
// This callback function gets called when "Rate and Comment"  button is
// pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMissionFeedbackBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecInstance = NULL;

    missionSpecInstance = (MissionSpecWizard *)clientData;
    
    if (missionSpecInstance->sendMissionFeedbackToCBR_())
    {
        XtSetSensitive(w, false);
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when the save-cbr  button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbSaveCBRBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecInstance = NULL;

    missionSpecInstance = (MissionSpecWizard *)clientData;

    if (missionSpecInstance->sendMissionToCBR_())
    {
        XtSetSensitive(w, false);
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when the maximize button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMaximizeBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->popupDialog();
}

//-----------------------------------------------------------------------
// This callback function gets called when one of the action buttons is
// pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataDisplayLineActionBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MetaDataDisplayLineActionBtnCallbackData_t *data = NULL;
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    data = (MetaDataDisplayLineActionBtnCallbackData_t *)clientData;
    missionSpecWizardInstance = data->missionSpecWizardInstance;

    missionSpecWizardInstance->processMetadataDisplayLineActionBtn_(
        data->selectedMission,
        data->buttonNum);
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata's "Merge Missions"
// button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMergeBtnPress_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->processMergeButton_(w);
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata's "Filter"
// button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataFilter_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->filterMetadata_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata's "Clear"
// button is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataFilterClear_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->clearMetadataFilter_();
}

//-----------------------------------------------------------------------
// This callback function gets called when the metadata's "Filter"
// box is typed in.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataFilterStringUpdate_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->updateMetadataFilterString_();
}

//-----------------------------------------------------------------------
// This callback function gets called when one the metadata's label
// is pressed in order to reorder the display.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataReorder_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MetaDataReorderRequestCallbackData_t *data = NULL;
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    data = (MetaDataReorderRequestCallbackData_t *)clientData;
    missionSpecWizardInstance = data->missionSpecWizardInstance;

    missionSpecWizardInstance->setMetadataOrder_(data->metadataOrderReference);
}

//-----------------------------------------------------------------------
// This callback function gets called when the check box is pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataDisplayLineCheckBox_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;
    int i;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;

    for (i = 0; i < (int)(missionSpecWizardInstance->metaDataDisplayLines_.size()); i++)
    {
        if (missionSpecWizardInstance->metaDataDisplayLines_[i].checkBox == w)
        {
            missionSpecWizardInstance->updateMetadataDisplayLineCheckBox_(i);
            break;
        }
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when the OK button for the meta
// data detail window was pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataDetailWindowOKBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MetadataDetailWindowOKBtnCallbackData_t *data = NULL;
    MissionSpecWizard *missionSpecWizardInstance = NULL;
    int selectedMission;

    data = (MetadataDetailWindowOKBtnCallbackData_t *)clientData;
    missionSpecWizardInstance = data->missionSpecWizardInstance;
    selectedMission = data->selectedMission;

    missionSpecWizardInstance->metadataDetailWindowIsUp_ = false;

    if (selectedMission != (missionSpecWizardInstance->selectedCheckBox_))
    {
        missionSpecWizardInstance->updateMetadataDisplayLineCheckBox_(selectedMission);
    }
    else
    {
        missionSpecWizardInstance->checkMetadataDisplayLineCheckBoxForRetrievedMission_(selectedMission);
    }
}

//-----------------------------------------------------------------------
// This callback function gets called when the Cancel button for the meta
// data detail window was pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataDetailWindowCancelBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MissionSpecWizard *missionSpecWizardInstance = NULL;

    missionSpecWizardInstance = (MissionSpecWizard *)clientData;
    missionSpecWizardInstance->metadataDetailWindowIsUp_ = false;
    missionSpecWizardInstance->updateMetadataDisplayLineCheckBox_(-1);
}

//-----------------------------------------------------------------------
// This callback function gets called when the Assemblage button for the
// meta data detail window was pressed.
//-----------------------------------------------------------------------
void MissionSpecWizard::cbMetadataDetailWindowAssemblageBtn_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    MetadataDetailWindowAssemblageBtnCallbackData_t *data = NULL;
    MissionSpecWizard *missionSpecWizardInstance = NULL;
    int selectedMission;

    data = (MetadataDetailWindowAssemblageBtnCallbackData_t *)clientData;
    missionSpecWizardInstance = data->missionSpecWizardInstance;
    selectedMission = data->selectedMission;

    missionSpecWizardInstance->processMetadataDetailWindowAssemblageBtn_(selectedMission);
}

/**********************************************************************
 * $Log: mission_spec_wizard.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.11  2007/10/07 20:00:24  pulam
 * New Log Messages
 *
 * Revision 1.10  2007/09/28 15:54:59  endo
 * Merge capability added to MissionSpecWizard.
 *
 * Revision 1.9  2007/09/18 22:36:12  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.8  2007/08/09 19:18:27  endo
 * MissionSpecWizard can now saves a mission if modified by the user
 *
 * Revision 1.7  2007/08/06 22:07:05  endo
 * Automatic update of metadata NumUsage implemented.
 *
 * Revision 1.6  2007/08/04 23:52:55  endo
 * Rate-&-Comment-Mission feature added to MissionSpecWizard.
 *
 * Revision 1.5  2007/07/19 14:50:03  endo
 * A bug fix for the filter function.
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
