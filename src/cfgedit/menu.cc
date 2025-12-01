/**********************************************************************
 **                                                                  **
 **                              menu.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: menu.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <assert.h>
#include <signal.h>
#include <string>
#include <iostream>
#include <fstream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/FileSB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/CascadeBG.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>

#include "buildbitmaps.h"
#include "buildmenu.h"
#include "list.hpp"
#include "load_cdl.h"
#include "popups.h"
#include "design.h"
#include "help.h"
#include "write_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "slot_record.hpp"
#include "glyph.hpp"
#include "fsa.hpp"
#include "transition.hpp"
#include "circle.hpp"
#include "binding.h"
#include "reporterror.h"
#include "toolbar.h"
#include "edit.h" 
#include "callbacks.h"
#include "version.h"
#include "make.h"
#include "analyze.h"
#include "cim_management.h"
#include "sgig_management.h"
#include "run.h"
#include "utilities.h"
#include "cfgedit_common.h"
#include "library_manager.h"
#include "verify.h"
#include "so_movement.h"
#include "EventLogging.h"
#include "renumber_robots.h"
#include "assistantDialog.h"
#include "mission_expert.h"
#include "file_utils.h"
#include "x_utils.h"
#include "acdl_plus.h"

using std::string;
using std::ifstream;
using std::ios;

//-----------------------------------------------------------------------
// Macros and Constatns
//-----------------------------------------------------------------------
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

#define	MAXARGS	20
#define	FSA_DETAIL_LABEL_SHOW  "Show state/trigger parameters"
#define	FSA_DETAIL_LABEL_HIDE  "Hide state/trigger parameters"

const String translations =
   "Shift <Btn1Down>:   da_mouse_cb(1shift_click)\n\
    <Btn1Down>:         da_mouse_cb(1down)\n\
    Shift <Btn2Down>:   da_mouse_cb(2shift_click)\n\
    <Btn2Down>:         da_mouse_cb(2down)\n\
    <Btn1Up>:           da_mouse_cb(up)\n\
    <Btn1Motion>:       da_mouse_cb(btnmove)\n\
    <Btn3Down>:         da_mouse_cb(3down)\n\
    <MouseMoved>:       da_mouse_cb(move)";
static const int DEFAULT_LINE_WIDTH = 2;
static const int CIRCLE_LINE_WIDTH = 2;
static const int TRANS_LINE_WIDTH = 2;
static const int FILE_ITEM_OPEN_INDEX = 4;
static const bool SCREEN_CAPTURE = false;
static const string EMPTY_STRING = "";

// Font related
string FONTNAME_CLEAN_BOLD_14 = "-*-clean-bold-r-*-*-14-*-*-*-*-*-*-*";
string FONTNAME_CLEAN_BOLD_12 = "-*-clean-bold-r-*-*-12-*-*-*-*-*-*-*";
string FONTNAME_CLEAN_MEDIUM_12 = "-*-clean-medium-r-*-*-12-*-*-*-*-*-*-*";
string gFONTTAG_CLEAN_BOLD_14 = "CLEAN_BOLD_14";
string gFONTTAG_CLEAN_BOLD_12 = "CLEAN_BOLD_12";
string gFONTTAG_CLEAN_MEDIUM_12 = "CLEAN_MEDIUM_12";

//-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------
// X and Xm related
XtAppContext app;
XtTranslations mouse_translations;
XmFontList gCfgEditFontList = NULL;
XColor black_color, white_color;
Font gCfgEditDefaultFont;
CfgEditGCTypes_t gCfgEditGCs;
CfgEditPixelTypes_t gCfgEditPixels;

// Drawing area related
int gDisplayHeight = 1000;
int gDisplayWidth = 1200;

// Widgets
Widget up_button = NULL;
Widget run_button = NULL;
Widget analyze_button = NULL;
Widget new_state_button = NULL;
Widget new_trans_button = NULL;
Widget new_waypoint_button = NULL;
Widget new_planner_button = NULL;
Widget new_oper_button = NULL;
Widget new_prim_button = NULL;
Widget new_agent_button = NULL;
Widget new_trigger_button = NULL;
Widget new_ibp_button = NULL;
Widget new_obp_button = NULL;
Widget bind_button = NULL;
Widget unbind_button = NULL;
Widget output_button = NULL;
Widget start_over_button = NULL;
Widget drawing_area;
Widget cur_page_label;
Widget cur_arch_label;
Widget cur_file_label;
Widget menu_bar;
Widget top_level;
Widget main_window;
Widget scrolled_window;

// Flags
bool place_constant = false;
bool copy_object = false;
bool push_up_input = false;
bool gDisableImportSymbol = false;
bool debug_save = false;
bool backup_files = true;
bool disableWaypointsFunction = false;
bool disablePathPlanner = false;
bool disableHoneywellRTAnalyzer = false;
bool verbose = false;
bool SHOW_SLIDER_VALUES = false;
bool special_rc_file = false;
bool gAutomaticExecution = false;

// Dialog
dialog_rec new_design_dialog;
dialog_rec save_as_dialog;
dialog_rec write_sub_dialog;

// Next three variables are coupled, and will be used to
// hide/show the details of parameters for the state/trigger.
// In other words, if the value of one variable is changed, the
// other two have to be changed.
bool hide_FSA_detail = true;
bool currently_FSA_detail_shown = false;
char *FSA_detail_label_default = FSA_DETAIL_LABEL_SHOW;

// Determine which behavior and trigger names will be shown to the user
// on the pull down menus using this bit mask based on the ARCH_xxx_BIT 
// constants defined in design.h  
// Each bit set intersects the list with that architecture. 
// Default of no bits set means all names visible.
// All bits set means only names occuring in all architectures are available.
unsigned long name_visibility = 0;

// Overlay
bool gOverlayPicked = false;
string gPickedOverlayName = EMPTY_STRING;

// Others
char *configuration_name = NULL;
EDIT_STATES edit_state = ES_NORMAL;
configuration *config = NULL;
symbol_table <rc_chain> rc_table(47);
SymbolList loaded_libraries;
char *current_filename = NULL;
const char *rc_filename = RC_FILENAME;
USER_PRIVILEGES user_privileges;
MissionExpert *gMExp = NULL;
EventLogging *gEventLogging = NULL;
AssistantDialog *gAssistantDialog = NULL;
PopUpWindows *gPopWin = NULL;
int debug_load_cdl = 0;
bool gWarningUser = false;

//-----------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------
static void save_file_cb(void);
static void save_file_as_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct * fcb);
static void write_sub_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct * fcb);
static void edit_cb(Widget w, int item_num);
static void FSA_detail_cb(Widget w);
static void exit_cb(Widget w);
static void cbSendCurrentMissionToCIM(Widget w, XtPointer clientData, XtPointer callbackData);
static void cbSendCurrentMissionToSGIG(Widget w, XtPointer clientData, XtPointer callbackData);


//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
// Table of architecture names and values
struct architecture_tbl architecture_table[] = {
    {"AuRA", ARCH_AuRA_BIT},
    {"AuRA.urban", ARCH_AuRA_urban_BIT},
    {"AuRA.naval", ARCH_AuRA_naval_BIT},
    {"UGV", ARCH_UGV_BIT},  
    {NULL, 0} 
};

// Table of privilege names and values
static struct priv_tbl
{
    char *name;
    USER_PRIVILEGES  priv;
} privilege_table[] = {
    {"Execute", EXECUTE_PRIV},
    {"Modify", MODIFY_PRIV},  
    {"Edit", EDIT_PRIV},    
    {"Create", CREATE_PRIV},
    {"Library", LIBRARY_PRIV},
    {"RealRobots", REALROBOTS_PRIV},
    {"MExpModifyCBRLib", MEXP_MODIFY_CBRLIB_PRIV},
    {NULL, 0} 
};

// Define the fallback resources
static String fallback_resources[] =
{
   "cfgedit.geometry: ",
   "cfgedit*ScrolledWindow.background: LightSteelBlue3",
   "cfgedit*CommandBar.background: LightSteelBlue3",
   "cfgedit*CommandBarLabel.background: LightSteelBlue3",
   "cfgedit*GlyphLabel.background: white",
   "cfgedit*DrawingArea.background: white",
   "cfgedit*Constant.background: white",
   "cfgedit*List.background: white",
   "cfgedit*.background: LightSteelBlue2",
   "cfgedit*.borderWidth: 1",
   "cfgedit*.borderColor: black",
   "cfgedit*selectColor: gold1",
   NULL, // This entry will be overwritten below if B/W display
   NULL
};

// Define the menu structure
static MenuItem file_items[] =
{
    // Note: If you add an item above "Open CDL file", make sure to modify
    // FILE_ITEM_OPEN_INDEX above as well.

    {"File Items", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"New", &xmPushButtonGadgetClass,
     'N', NULL, NULL, (XtCallbackProc) open_dialog_box, (XtPointer) & new_design_dialog, NULL, NULL},

    {"Open CDL file", &xmPushButtonGadgetClass,
     'O', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunOpenCDLFileDialog), (void *)gPopWin, NULL, NULL},

    {"Save Configuration", &xmPushButtonGadgetClass,
     'S', "Ctrl<Key>s", "Ctrl+s", (XtCallbackProc) save_file_cb, NULL, NULL, NULL},

    {"Save Configuration As ...", &xmPushButtonGadgetClass,
     'A', NULL, NULL, (XtCallbackProc) open_dialog_box, (XtPointer) & save_as_dialog, NULL, NULL},

    {"Write Current Subtree ...", &xmPushButtonGadgetClass,
     'A', NULL, NULL, (XtCallbackProc) open_dialog_box, (XtPointer) & write_sub_dialog, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Load Mission Expert data file", &xmPushButtonGadgetClass,
     'M', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunOpenMExpDataFileDialog), (void *)gPopWin, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Quit", &xmPushButtonGadgetClass,
     'Q', "Ctrl<Key>q", "Ctrl+q", (XtCallbackProc) exit_cb, 0, NULL, NULL},

    {NULL}
};

static MenuItem edit_items[] =
{
    {"Edit Items", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Copy", &xmPushButtonGadgetClass,
     'C',"Ctrl<Key>C","Ctrl+C",(XtCallbackProc)set_mode,(void *)COPY,NULL,NULL},

    {"Duplicate", &xmPushButtonGadgetClass,
     'D',"Ctrl<Key>D","Ctrl+D",(XtCallbackProc)set_mode,(void *)DUPLICATE, NULL, NULL},

    {"Cut", &xmPushButtonGadgetClass,
     'X',"Ctrl<Key>X","Ctrl+X",(XtCallbackProc)set_mode,(void *)CUT, NULL, NULL},

    {"Paste", &xmPushButtonGadgetClass,
     'V',"Ctrl<Key>V","Ctrl+V",(XtCallbackProc)set_mode,(void *)PASTE,NULL,NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Delete", &xmPushButtonGadgetClass,
     'a',"<Key>Delete","Del",(XtCallbackProc)set_mode,(void *)DELETE,NULL,NULL},

    //{"Undo", &xmPushButtonGadgetClass,
    //'Z',"Ctrl<Key>Z","Ctrl+Z",(XtCallbackProc)set_mode,(void *)UNDO,NULL,NULL},

    {NULL}
};

static MenuItem layout_items[] =
{
    {"Layout Items", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Redraw", &xmPushButtonGadgetClass,
     'R',"Ctrl<Key>L","Ctrl+L",(XtCallbackProc)edit_cb, (void *) 2, NULL, NULL},

    {"Cancel", &xmPushButtonGadgetClass,
     'C',"<Key>Escape","Esc",(XtCallbackProc)cancel_cb, NULL ,NULL,NULL},

    {FSA_detail_label_default, &xmPushButtonGadgetClass,
     '\0',NULL,NULL,(XtCallbackProc)FSA_detail_cb, (void *) currently_FSA_detail_shown, NULL, NULL},

    {"Link selections into paste buffer", &xmPushButtonGadgetClass,
     'A', NULL, NULL, (XtCallbackProc)set_mode, (void *)LINK, NULL, NULL},

    //DCM 12-4-99: Reworded since now toggles input parms between constants and links
    {"Toggle input between constant and link", &xmPushButtonGadgetClass,
     'A', NULL, NULL, (XtCallbackProc)set_mode, (void *)CVT_INPUT, NULL, NULL},

    {"Push up input", &xmPushButtonGadgetClass,
     'P', NULL, NULL, (XtCallbackProc)set_mode, (void *)PUSHUP, NULL, NULL},

    {"Group selected component", &xmPushButtonGadgetClass,
     'G', NULL, NULL, (XtCallbackProc)set_mode, (void *)GROUP_COMPONENT, NULL, NULL},

    {NULL}
};


static MenuItem configure_items[] =
{
    {"Configuration Settings", &xmLabelGadgetClass,
     '\0',NULL,NULL,NULL,NULL,NULL,NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {" ", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Restrict available primitives", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Must occur in AuRA", &xmToggleButtonGadgetClass,
     '\0',NULL,NULL,(XtCallbackProc)configure_nv_toggle_cb,(void *)ARCH_AuRA_BIT,NULL,NULL},

    {"Must occur in AuRA.urban", &xmToggleButtonGadgetClass,
     '\0',NULL,NULL,(XtCallbackProc)configure_nv_toggle_cb,(void *)ARCH_AuRA_urban_BIT,NULL,NULL},

    {"Must occur in AuRA.naval", &xmToggleButtonGadgetClass,
     '\0',NULL,NULL,(XtCallbackProc)configure_nv_toggle_cb,(void *)ARCH_AuRA_naval_BIT,NULL,NULL},

    {"Must occur in UGV", &xmToggleButtonGadgetClass,
     '\0',NULL,NULL,(XtCallbackProc)configure_nv_toggle_cb,(void *)ARCH_UGV_BIT,NULL,NULL},

    {NULL}
};

static MenuItem library_items[] =
{
   {"Library Manager", &xmLabelGadgetClass,
    '\0', NULL, NULL, NULL, NULL, NULL, NULL},

   {"_sep1", &xmSeparatorGadgetClass,
    '\0', NULL, NULL, NULL, NULL, NULL, NULL},

   {"Import library component", &xmPushButtonGadgetClass,
    'I', NULL, NULL, (XtCallbackProc)set_mode,(void *)IMPORT_LIB_FNC,NULL,NULL},

   {"Add Selected Component to Library", &xmPushButtonGadgetClass,
    'A', NULL, NULL, (XtCallbackProc)set_mode,(void *)ADD_TO_LIB_FNC,NULL,NULL},

   {"Remove Component from Library", &xmPushButtonGadgetClass,
    'R', NULL, NULL, (XtCallbackProc)set_mode,(void *)DELETE_FROM_LIB_FNC,NULL,NULL},

   {NULL}
};

static MenuItem binding_items[] =
{
    {"Architecture Binding", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Bind", &xmPushButtonGadgetClass,
     'B', NULL, NULL, (XtCallbackProc) bind_cb, NULL, NULL, NULL},

    {"Unbind", &xmPushButtonGadgetClass,
     'U', NULL, NULL, (XtCallbackProc) unbind_cb, NULL, NULL, NULL},

    {NULL}
};

static MenuItem missionExpert_items[] =
{
    {"Mission Expert", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Create a New Mission Using Mission Expert", &xmPushButtonGadgetClass,
     'C', NULL, NULL, (XtCallbackProc)set_mode,(void *)MEXP_NEW_MISSION,NULL,NULL},

    {"View the Map-Based Mission with Mlab", &xmPushButtonGadgetClass,
     'V', NULL, NULL, (XtCallbackProc)set_mode,(void *)MEXP_VIEW_MAP_MISSION,NULL,NULL},

    {"Add This Mission to CBR Library", &xmPushButtonGadgetClass,
     'M', NULL, NULL, (XtCallbackProc)set_mode,(void *)MEXP_ADD_MISSION_TO_CBRLIB,NULL,NULL},

    {"Replay Mission", &xmPushButtonGadgetClass,
     'R', NULL, NULL, (XtCallbackProc)set_mode,(void *)MEXP_REPLAY_MISSION,NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Setup and Subscribe to CIM", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cim_run, NULL, NULL, NULL},

    {"Send the Current Mission to CIM", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cbSendCurrentMissionToCIM, NULL, NULL, NULL},

    {"Popup ICARUS Wizard", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)(gMExp->cbPopupMissionSpecWizard), (void *)MISSION_SPEC_WIZARD_ICARUS, NULL, NULL},    

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Setup and Subscribe to SGIG", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)sgig_run, NULL, NULL, NULL},

    {"Send the Current Mission to SGIG", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cbSendCurrentMissionToSGIG, NULL, NULL, NULL},

    {"Popup UAV Wizard", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)(gMExp->cbPopupMissionSpecWizard), (void *)MISSION_SPEC_WIZARD_BAMS, NULL, NULL},

    // Not yet fully supported.
    //{"Add Selected Component to CBR Library", &xmPushButtonGadgetClass,
    // 'S', NULL, NULL, (XtCallbackProc)set_mode,(void *)MEXP_ADD_SELECTION_TO_CBRLIB,NULL,NULL},

    {NULL}
};


// old menu items for cim/sgig
/*
static MenuItem cim_items[] =
{
    {"CIM Management", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Setup and Subscribe to CIM", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cim_run, NULL, NULL, NULL},

    {"Send the Current Mission to CIM", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cbSendCurrentMissionToCIM, NULL, NULL, NULL},

    {"Popup ICARUS Wizard", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)(gMExp->cbPopupIcarusWizard), NULL, NULL, NULL},

    {NULL}
};

static MenuItem sgig_items[] =
{
    {"SGIG Management", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Setup and Subscribe to SGIG", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)sgig_run, NULL, NULL, NULL},

    {"Send the Current Mission to SGIG", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)cbSendCurrentMissionToCIM, NULL, NULL, NULL},

    {"Popup UAV Wizard", &xmPushButtonGadgetClass,
     '\0', NULL, NULL,  (XtCallbackProc)(gMExp->cbPopupIcarusWizard), NULL, NULL, NULL},

    {NULL}
};
*/


static MenuItem tools_items[] =
{
    {"Tools", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Convert ACDL+ code into CIM XML", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunACDLP2CIMXMLDialog), (void *)gPopWin, NULL, NULL},

    {"Convert ACDL+ code into CMDL", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunACDLP2CMDLDialog), (void *)gPopWin, NULL, NULL},

    {"Convert ACDL+ code into CMDLi", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunACDLP2CMDLIDialog), (void *)gPopWin, NULL, NULL},

    {"Convert ACDL+ code into CDL", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunACDLP2CDLDialog), (void *)gPopWin, NULL, NULL},

    {"Convert CIM XML code into ACDL+", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunCIMXML2ACDLPDialog), (void *)gPopWin, NULL, NULL},

    {"Convert CMDL code into ACDL+", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunCMDL2ACDLPDialog), (void *)gPopWin, NULL, NULL},

    {"Convert CMDLi code into ACDL+", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunCMDLI2ACDLPDialog), (void *)gPopWin, NULL, NULL},

    {"Convert CDL code into ACDL+", &xmPushButtonGadgetClass,
     '\0', NULL, NULL, (XtCallbackProc)(gPopWin->cbRunCDL2ACDLPDialog), (void *)gPopWin, NULL, NULL},

    {NULL}
};

static MenuItem help_items[] =
{
    {"Help Items", &xmLabelGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"Assistant Dialog", &xmPushButtonGadgetClass,
     'A', NULL, NULL, (XtCallbackProc)(cbPopupAssistantDialog), 0, NULL, NULL},

    {"_sep1", &xmSeparatorGadgetClass,
     '\0', NULL, NULL, NULL, NULL, NULL, NULL},

    {"About", &xmPushButtonGadgetClass,
     'b', NULL, NULL, (XtCallbackProc) help_cb, 0, NULL, NULL},

    {"Copyright", &xmPushButtonGadgetClass,
     'C', NULL, NULL, (XtCallbackProc) help_callback, (void *) HELP_PAGE_copyright, NULL, NULL},

    //{"Index", &xmPushButtonGadgetClass,
    //'I', NULL, NULL, (XtCallbackProc) index_callback, (void *) 0, NULL, NULL},

    {NULL}
};

//-------------------------------------------------------------
void gSavePickedOverlayName(string overlayName)
{
    gOverlayPicked = true;
    gPickedOverlayName = overlayName;
}

//-------------------------------------------------------------
static void new_design_cb(void)
{
   config->new_design();
}

//-------------------------------------------------------------
static void init_dialog_records(void)
{
   // Define the new_design dialog structure
   dialog_rec l_new_design_dialog =
   {(DIALOG_BLDR)XmCreateQuestionDialog,
    {
       {XmNmessageString, "Are you sure you want to delete the current design?"},
       {XmNokLabelString, "Yes"},
       {XmNcancelLabelString, "Cancel"},
       {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {NULL, 0}
    },
    {
       {XmNokCallback, (XtCallbackProc) new_design_cb, NULL},
       {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_new_design},
       {NULL, NULL, NULL}
    }
   };

   // Define the new_design dialog structure
   dialog_rec l_save_as_dialog =
   {(DIALOG_BLDR) XmCreatePromptDialog,
    {
       {XmNmessageString, "Enter the file name"},
       {XmNokLabelString, "Save"},
       {XmNcancelLabelString, "Cancel"},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {NULL, 0}
    },
    {
       {XmNokCallback, (XtCallbackProc) save_file_as_cb, (XtPointer) PASS_WIDGET},
       {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_save_as},
       {NULL, NULL, NULL}
    }
   };

   // Define the new_design dialog structure
   dialog_rec l_write_sub_dialog =
   {(DIALOG_BLDR) XmCreatePromptDialog,
    {
       {XmNmessageString, "Enter the file name"},
       {XmNokLabelString, "Save"},
       {XmNcancelLabelString, "Cancel"},
       {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
       {NULL, 0}
    },
    {
    {XmNokCallback, (XtCallbackProc) write_sub_cb, (XtPointer) PASS_WIDGET},
       {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_save_as},
       {NULL, NULL, NULL}
    }
   };

   new_design_dialog = l_new_design_dialog;
   save_as_dialog = l_save_as_dialog;
   write_sub_dialog = l_write_sub_dialog;
}

//-------------------------------------------------------------
// called when the level changes to select which components are allowed
//-------------------------------------------------------------
void reset_placement_list(void)
{
    bool edit_mode = (system_mode == EDIT_MODE);
    bool is_fsa = config->this_page()->is_fsa();
    bool cfg_empty = (config->root()->children.len() == 0);
    bool is_read_only = config->is_read_only_page();

    if(edit_mode)
    {
	if(is_fsa)
	{
	    XtUnmanageChild(new_prim_button);
	    XtUnmanageChild(new_agent_button);
	    XtUnmanageChild(new_trigger_button);
	    XtUnmanageChild(new_ibp_button);

	    XtUnmanageChild(new_oper_button);
	    XtUnmanageChild(new_obp_button);

	    XtManageChild(new_state_button);
	    XtManageChild(new_trans_button);
	    XtManageChild(new_waypoint_button);	
	    XtManageChild(new_planner_button);
	    XtManageChild(start_over_button);
	}
	else
	{
	    XtUnmanageChild(new_state_button);
	    XtUnmanageChild(new_trans_button);
	    XtUnmanageChild(new_waypoint_button);
	    XtUnmanageChild(new_planner_button);

	    XtManageChild(new_oper_button);
	    XtManageChild(new_obp_button);
	    XtManageChild(new_prim_button);
	    XtManageChild(new_agent_button);
	    XtManageChild(new_trigger_button);
	    XtManageChild(new_ibp_button);
	    XtManageChild(start_over_button);
	}

	if ((gDisableImportSymbol) && (is_read_only))
	{
	    XtSetSensitive(new_oper_button, false);
	    XtSetSensitive(new_agent_button, false);
	    XtSetSensitive(new_ibp_button, false);
	    XtSetSensitive(new_obp_button, false);
	    XtSetSensitive(new_trigger_button, false);
	    XtSetSensitive(new_prim_button, false);
	    XtSetSensitive(new_state_button, false);
	    XtSetSensitive(new_trans_button, false);
	    XtSetSensitive(new_waypoint_button, false);
	    XtSetSensitive(new_planner_button, false);
	}
	else
	{
	    XtSetSensitive(new_oper_button, true);
	    XtSetSensitive(new_agent_button, true);
	    XtSetSensitive(new_ibp_button, true);
	    XtSetSensitive(new_obp_button, true);
	    XtSetSensitive(new_trigger_button, true);
	    XtSetSensitive(new_prim_button, true);
	    XtSetSensitive(new_state_button, true);
	    XtSetSensitive(new_trans_button, true);
	    XtSetSensitive(new_waypoint_button, true);
	    XtSetSensitive(new_planner_button, true);
	}

    }

    XtSetSensitive(output_button, edit_mode && !cfg_empty);
    XtSetSensitive(binding_items[2].widget, edit_mode);
    XtSetSensitive(binding_items[3].widget, edit_mode && config->arch_is_bound());

    if (disableWaypointsFunction)
    {
        XtSetSensitive(new_waypoint_button, false);
    }
    else
    {
        XtSetSensitive(new_waypoint_button, edit_mode && !cfg_empty);
    }

    if (disablePathPlanner)
    {
        XtSetSensitive(new_planner_button, false);
    }
    else
    {
        XtSetSensitive(new_planner_button, edit_mode && !cfg_empty);
    }

    if (disableHoneywellRTAnalyzer)
    {
        XtSetSensitive(analyze_button, false);
    }
    else
    {
        XtSetSensitive(analyze_button, edit_mode && !cfg_empty);
    }
}

//-----------------------------------------------------------------------
// called when a file is loaded or cleared
//-----------------------------------------------------------------------
void reset_file_save(Boolean val)
{
   XtSetSensitive(file_items[FILE_ITEM_OPEN_INDEX].widget, val);
}

//-----------------------------------------------------------------------
// called when the binding changes 
//-----------------------------------------------------------------------
void reset_binding_list()
{
#if 0
   // for each button on the placement menu, determine if it is available or not
   int slot = 2;		// First two slots on menu are label and seperator

   bool arch_is_bound = config->arch_is_bound();

   // unbind arch ----------------------------------------------------
   bool is_on = false;

   if (arch_is_bound)
   {
      is_on = true;
   }

   XtSetSensitive(bind_items[slot++].widget, is_on);

   // bind arch -----------------------------------------------
   is_on = false;

   if (!arch_is_bound)
   {
      is_on = true;
   }

   XtSetSensitive(bind_items[slot++].widget, is_on);
#endif
}

//-----------------------------------------------------------------------
// called when the filename changes
//-----------------------------------------------------------------------
void change_filename(char *filename)
{
    char *config_name = NULL, *fileonly = NULL, *slash = NULL, *root = NULL;

    if (current_filename)
    {
        free(current_filename);
        current_filename = NULL;
    }

    if (filename)
    { 
        current_filename = strdup(filename);
    }

    // Update the menu bar
    if (current_filename)
    {
        slash = strrchr(current_filename, '/');

        if (slash != NULL)
        {
            fileonly = slash + 1;
        }
        else
        {
            fileonly = current_filename;
        }

        config_name = fileonly;
    }
    else
    {
        config_name = "UNTITLED";
    }

    XmString str = XmStringCreateLocalized(config_name);
    XtVaSetValues(
        cur_file_label,
        XmNlabelString, str,
        NULL);
    XmStringFree(str);

    // Get just the root filename without the extension
    root = strtok(strdup(config_name),".");

    if(root == NULL)
    {
        root = config_name;
    }

    config->rename(root);
}

//-----------------------------------------------------------------------
// This opens a CDL file.
//-----------------------------------------------------------------------
void open_cdl_file(string filename)
{
    char simpleFilename[256], msg[256];

    if (config->load((char *)(filename.c_str())))
    {
        // Unable to load input file
        fprintf(
            stderr,
            "Error(cfgedit): open_cdl_file(). '%s' could not be opened.\a\n",
            filename.c_str());
        return;
    }
    else
    {
        // Reset filename
        change_filename((char *)(filename.c_str()));

        reset_file_save(true);

        if (filename_has_directory((char *)(filename.c_str())))
        {
            remove_directory((char *)(filename.c_str()), simpleFilename);
            sprintf(msg, "%s_loaded", simpleFilename);
        }
        else
        {
            sprintf(msg, "%s_loaded", filename.c_str());
        }

        save_cdl_for_replay(msg);
    }
}

//-----------------------------------------------------------------------/
void erase_drawing_area(void)
{
    XFillRectangle(
        XtDisplay(drawing_area),
        XtWindow(drawing_area),
        //eraseGC,
        gCfgEditGCs.erase,
        0,
        0,
        DRAWING_WIDTH,
        DRAWING_HEIGHT);
}

//-----------------------------------------------------------------------
// edit_cb: called when an item on the the edit menu is selected
//-----------------------------------------------------------------------
static void edit_cb(Widget w, int item_num)
{
    place_constant = false;
    copy_object = false;
    push_up_input = false;

    edit_state = ES_NORMAL;

    switch (item_num) {

    case 0:
        place_constant = true;
        break;

    case 2:
        config->regen_page();
        break;

    case 3:
        if (config->this_page()->is_fsa())
            config->this_page()->this_fsa()->adding_transition();
        break;

    case 4:
        copy_object = true;
        break;

    case 5:
        push_up_input = true;

        // Unfocus any text widgets so won't grab the existing one
        XmProcessTraversal(drawing_area, XmTRAVERSE_CURRENT);
        break;

    case 6:
        edit_state = ES_LINK_OBJECT;
        break;

    case 7:
        edit_state = ES_MOVE_OBJECT;
        break;
    }
}

//------------------------------------------------------------------
static void save_file_as_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct * fcb)
{
    char *filename;
    int len;

    /* unexpose the file selection dialog */
    XtUnmanageChild(w);


    XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET, &filename);

    gEventLogging->start("Save As [%s]", filename);

    /* check filename */
    if (filename == NULL)
    {
        warn_userf("Invalid File Name");
        gEventLogging->cancel("Save As");
        return;
    }

    len = strlen(filename);

    if (len <= 0)
    {
        warn_userf("Invalid File Name");
        gEventLogging->cancel("Save As");
        return;
    }

    /* check if need to add .cdl */
    if (len < 5 || strcmp(&filename[len - 4], ".cdl") != 0)
    {
        /* make a copy of the filename with the extension */
        char *rtn = (char *) malloc(len + 5);

        strcpy(rtn, filename);
        strcpy(&rtn[len], ".cdl");
        free(filename);
        filename = rtn;
    }

    FILE *file;

    if ((file = fopen(filename, "r")) != NULL)
    {
        fclose(file);
        char msg[256];
        char ok_string[256];

        if (backup_files)
        {
            sprintf(msg, "Warning: file %s already exists!  OK to backup existing file?", filename);
            sprintf(ok_string, "Backup then save");
        }
        else
        {
            sprintf(msg, "Warning: file %s already exists!  OK to overwrite?", filename);
            sprintf(ok_string, "Overwrite");
        }

        // Define the new_design dialog structure
        dialog_rec file_exists_dialog =
            {(DIALOG_BLDR) XmCreateQuestionDialog,
             {
                 {XmNmessageString, msg},
                 {XmNokLabelString, ok_string},
                 {XmNcancelLabelString, "Cancel"},
                 {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                 {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                 {NULL, 0}
             },
             {
                 {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_file},
                 {NULL, NULL, NULL}}
            };

        gEventLogging->start("%s", msg);

        if (wait_dialog_box(w, &file_exists_dialog, NULL))
        {
            gEventLogging->log("%s = OK", msg);
            gEventLogging->end("%s", msg);
        }
        else
        {
            gEventLogging->log("%s = Cancel", msg);
            gEventLogging->end("%s", msg);
            gEventLogging->cancel("Save As");
            return;
        }
    }

    // Reset to this name
    change_filename(filename);
    reset_file_save(true);

    int good = save_workspace(config->root(), filename, debug_save, backup_files);

    if (!good)
    {
        warn_userf("Error writing output file '%s'!", filename);
        gEventLogging->cancel("Save As");
        XtFree(filename);
    }
    else
    {
        // Mark drawing as clean
        renumber_robots();
        config->did_save();
        gEventLogging->end("Save As");
    }
}

//-----------------------------------------------------------------------

static void write_sub_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct * fcb)
{
    /* unexpose the file selection dialog */
    XtUnmanageChild(w);

    char *filename;

    XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET, &filename);

    /* check filename */
    if (filename == NULL)
        return;
    int len = strlen(filename);

    if (len <= 0)
        return;

    /* check if need to add .cdl */
    if (len < 5 || strcmp(&filename[len - 4], ".cdl") != 0)
    {
        /* make a copy of the filename with the extension */
        char *rtn = (char *) malloc(len + 5);

        strcpy(rtn, filename);
        strcpy(&rtn[len], ".cdl");
        free(filename);
        filename = rtn;
    }

    FILE *file;

    if ((file = fopen(filename, "r")) != NULL)
    {
        fclose(file);
        char msg[256];
        char ok_string[256];

        if (backup_files)
        {
            sprintf(msg, "Warning: file %s already exists!  OK to backup existing file?", filename);
            sprintf(ok_string, "Backup then save");
        }
        else
        {
            sprintf(msg, "Warning: file %s already exists!  OK to overwrite?", filename);
            sprintf(ok_string, "Overwrite");
        }


        // Define the new_design dialog structure
        dialog_rec file_exists_dialog =
            {(DIALOG_BLDR) XmCreateQuestionDialog,
             {
                 {XmNmessageString, msg},
                 {XmNokLabelString, ok_string},
                 {XmNcancelLabelString, "Cancel"},
                 {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                 {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                 {NULL, 0}
             },
             {
                 {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_file},
                 {NULL, NULL, NULL}}
            };

        if (!wait_dialog_box(w, &file_exists_dialog, NULL))
            return;
    }

    int good = save_workspace(config->this_page()->this_agent(), filename, debug_save, backup_files);

    if (!good)
    {
        warn_userf("Error writing output file '%s'!", filename);
        XtFree(filename);
    }
}

//-----------------------------------------------------------------------
// This function sends the current mission to CIM.
//-----------------------------------------------------------------------
static void cbSendCurrentMissionToCIM(Widget w, XtPointer clientData, XtPointer callbackData)
{
    string tmpACDLFilenameString;
    string acdlText;
    string xmlMission;
    char tmpACDLFilename[256];
    char *data = NULL;
    int dataSize = 0;
    int fd;
    bool saved = false;

    sprintf(tmpACDLFilename, "/tmp/%s-cbSendCurrentMissionToCIM-acdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpACDLFilename);
    unlink(tmpACDLFilename);
    tmpACDLFilenameString = tmpACDLFilename;

    // Save the ACDL.
    saved = save_abstracted_workspace(config->root(), tmpACDLFilenameString.c_str());

    if (!saved)
    {
        warn_userf("Error: Work space could not be saved.");
        return;
    }

    // Now, read the file as a string.
    ifstream acdlFile(tmpACDLFilenameString.c_str());

    // Find the data size.
    acdlFile.seekg (0, ios::end);
    dataSize = acdlFile.tellg();
    acdlFile.seekg (0, ios::beg);

    // To append NULL at the end.
    dataSize++; 

    // Read in the data.
    data = new char[dataSize];
    acdlFile.read (data, dataSize-1);
    
    // Append a null, to make it a char string.
    data[dataSize-1] = '\0';

    // Close the file.
    acdlFile.close();

    // Copy it as a string.
    acdlText = data;

    // Delete the char*.
    delete [] data;
    data = NULL;

    // Remove the temporary files.
    unlink(tmpACDLFilenameString.c_str());

    // Now, convert the ACDL to XML.
    xmlMission = ACDLPlus::acdlp2cimxmlText(acdlText);
    
    // Send the XML mission.
    cim_send_message(xmlMission);
}


//-----------------------------------------------------------------------
// This function sends the current mission to SGIG.
//-----------------------------------------------------------------------
static void cbSendCurrentMissionToSGIG(Widget w, XtPointer clientData, XtPointer callbackData)
{
    string tmpACDLFilenameString;
    string acdlText;
    string xmlMission;
    char tmpACDLFilename[256];
    char *data = NULL;
    int dataSize = 0;
    int fd;
    bool saved = false;

    sprintf(tmpACDLFilename, "/tmp/%s-cbSendCurrentMissionToCIM-acdl-XXXXXX", getenv("USER"));
    fd = mkstemp(tmpACDLFilename);
    unlink(tmpACDLFilename);
    tmpACDLFilenameString = tmpACDLFilename;

    // Save the ACDL.
    saved = save_abstracted_workspace(config->root(), tmpACDLFilenameString.c_str());

    if (!saved)
    {
        warn_userf("Error: Work space could not be saved.");
        return;
    }

    // Now, read the file as a string.
    ifstream acdlFile(tmpACDLFilenameString.c_str());

    // Find the data size.
    acdlFile.seekg (0, ios::end);
    dataSize = acdlFile.tellg();
    acdlFile.seekg (0, ios::beg);

    // To append NULL at the end.
    dataSize++; 

    // Read in the data.
    data = new char[dataSize];
    acdlFile.read (data, dataSize-1);
    
    // Append a null, to make it a char string.
    data[dataSize-1] = '\0';

    // Close the file.
    acdlFile.close();

    // Copy it as a string.
    acdlText = data;

    // Delete the char*.
    delete [] data;
    data = NULL;

    // Remove the temporary files.
    unlink(tmpACDLFilenameString.c_str());

    // Now, convert the ACDL to XML.
    xmlMission = ACDLPlus::acdlp2cimxmlText(acdlText);
    
    // Send the XML mission.
    sgig_send_message(xmlMission);
}

//-----------------------------------------------------------------------

static void save_file_cb(void)
{
    gEventLogging->start("Save File [%s]", current_filename);

    if (current_filename == NULL)
    {
        warn_userf("Internal Error: no filename loaded");
        gEventLogging->cancel("Save File");
        return;
    }

    int good = save_workspace(config->root(), current_filename, debug_save, backup_files);

    if (!good)
    {
        warn_userf("Error writing output file '%s'!", current_filename);
        gEventLogging->cancel("Save File");
    }
    else
    {
        // Mark drawing as clean
        renumber_robots();
        config->did_save();
        gEventLogging->end("Save File");
    }
}

//-----------------------------------------------------------------------

static void FSA_detail_cb(Widget w)
{
    XmString   str;

    if (layout_items[4].callback_data)
    {
        // Hide the details
        hide_FSA_detail = true;

        // Update the menu bar, so that it will show the appropriate
        // label next time.
        str = XmStringCreateLocalized (FSA_DETAIL_LABEL_SHOW);

        // Next time, when FSA_detail_cb is called, it will
        // show the details instead.
        layout_items[4].callback_data = (void *)false;
    }
    else
    {   
        hide_FSA_detail = false;
        str = XmStringCreateLocalized (FSA_DETAIL_LABEL_HIDE);
        layout_items[4].callback_data = (void *)true;
    }

    config->regen_page();

    XtVaSetValues(w,
                  XmNlabelString, str,
                  NULL);
    XmStringFree (str);

    XtManageChild(menu_bar);
}

//-----------------------------------------------------------------------
void sig_catch(int sig)
{
    if (config != NULL)
    {
        config->quit();
    }
    else
    {
        exit(0);
    }
}

//-----------------------------------------------------------------------
static void exit_cb(Widget w)
{
    bool confirmed = false;

    if (config->needs_save())
    {
        confirmed = gPopWin->quitConfirm(w);

        if (!confirmed)
        {
            // Not confirmed. Don't quit.
            return;
        }
    }

    config->quit();
}

//-----------------------------------------------------------------------
void AddSystemGrabs(void)
{
   // Also keep the cancel button active.
   // XtAddGrab(advanced_items[3].widget, false, false);
}

//-----------------------------------------------------------------------
int main(int argc, char *argv[])
{
    XtActionsRec actions[1];
    Display *display = NULL, *dpy = NULL;
    Visual *vis = NULL;
    Colormap cmap;
    Widget fileCascade_w, editCascade_w, layoutCascade_w, configureCascade_w;
    Widget libraryCascade_w, bindingCascade_w, mexpCascade_w, toolsCascade_w, helpCascade_w;;
    XmString labelStr;
    FILE *chk = NULL;
    Symbol *arch;
    GTList<char *> startup_events;
    string mexpFeatureFileName = EMPTY_STRING;
    string mexpRCFileName = EMPTY_STRING;
    char simpleFilename[256], msg[256], buf[2048], title[256], geo[50];
    char *fullname = NULL;
    char *log_event = NULL, *event = NULL, *filename = NULL, *dir = NULL;
    char *str = NULL;
    void *p = NULL, *cur = NULL;
    int fg, bg;
    int monochrome = false, screen_num, scr;
    int c;
    int index;
    int rtn, pos;
    bool showInitDialog = true;
    bool loadMExpFeatureFile = false;
    bool valid = false;
    bool errors = false;

    gEventLogging = new EventLogging();

    // Load the dialog records
    init_dialog_records();

    // ************************************************************
    while ((c = getopt(argc, argv, "ac:e:E:lM:sv")) != EOF)
    {
        switch (c) {

        case 'a':
            gAutomaticExecution = true;
            break;

        case 'c':
            rc_filename = optarg;
            special_rc_file = true;
            cerr << "Using configuration file " << rc_filename << '\n';
            break;

        case 'e':
            log_event = strdup(optarg);
            cerr << "Logging startup event '" << log_event << "'\n";
            startup_events.append(log_event);
            break;

        case 'E':
            mexpRCFileName = optarg;
            break;

        case 'l':
            debug_load_cdl++;
            cerr << "Debug level of cdl parser set to " << debug_load_cdl << '\n';
            break;

        case 'M':
            loadMExpFeatureFile = true;
            mexpFeatureFileName = optarg;
            break;

        case 's':
            debug_save = true;
            cerr << "Debugging of cdl code generator enabled\n";
            break;

        case 'v':
            cerr << "Verbose mode selected\n";
            verbose = true;

            for(int i=0; i<argc; i++)
            {
                fprintf(stderr,"%s ",argv[i]);
            }

            fprintf(stderr,"\n");
            break;
        }
    }

    signal(SIGINT, sig_catch);

    if(load_rc(rc_filename, &rc_table, special_rc_file, verbose, false) == 2)
    {
        cerr << "Warning: Didn't find " << rc_filename << '\n';
    }

    // Init the boolean globals
    {
        int val = check_bool_rc(rc_table, BACKUP_FILES_NAME);

        if (val >= 0)
            backup_files = val == 1;

        val = check_bool_rc(rc_table, "ShowSliderValues");

        if (val >= 0)
            SHOW_SLIDER_VALUES = val == 1;

        val = check_bool_rc(rc_table, "HideStateTriggerParameters");

        if (val)
        {
            hide_FSA_detail = true;
            layout_items[4].label = FSA_DETAIL_LABEL_SHOW;
            layout_items[4].callback_data = (void *)false;
        }
        else
        {
            hide_FSA_detail = false;
            layout_items[4].label = FSA_DETAIL_LABEL_HIDE;
            layout_items[4].callback_data = (void *)true;
        }

        if (check_bool_rc(rc_table, "DisableWaypointsFunction"))
        {
            disableWaypointsFunction = true;
        }
        else
        {
            disableWaypointsFunction = false;
        }

        if (check_bool_rc(rc_table, "DisablePathPlanner"))
        {
            disablePathPlanner = true;
        }
        else
        {
            disablePathPlanner = false;
        }

        if (check_bool_rc(rc_table, "DisableHoneywellRTAnalyzer"))
        {
            disableHoneywellRTAnalyzer = true;
        }
        else
        {
            disableHoneywellRTAnalyzer = false;
        }
    }

    // Set the privileges
    index = 0;
    user_privileges = 0;
    while(privilege_table[index].name)
    {
        rtn = check_value_rc(
            rc_table, 
            USER_PRIVILEGES_NAME, 
            privilege_table[index].name);

        if(rtn == 1)
        {
            user_privileges |= privilege_table[index].priv;
        }
        index++;
    }

    // Restrict the name space visibility
    index = 0;
    name_visibility = 0;
    while(architecture_table[index].name)
    {
        rtn = check_value_rc(
            rc_table, 
            VISIBLE_NAMES_NAME, 
            architecture_table[index].name);

        if(rtn == 1)
        {
            name_visibility |= architecture_table[index].bit;
        }
        index++;
    }

    // make sure that we're running under X
    display = XOpenDisplay(NULL);

    if (display == NULL)
    {
        fprintf(stderr, "Error opening X display! \n(maybe need to do 'setenv DISPLAY host:0.0')\n");
        exit(1);
    }

    // Create default font.
    gCfgEditDefaultFont = createAndAppendFont(
        display,
        (char *)(FONTNAME_CLEAN_BOLD_14.c_str()),
        XmSTRING_DEFAULT_CHARSET,
        &gCfgEditFontList);

    // Append some other fonts to the list.
    createAndAppendFont(
        display,
        (char *)(FONTNAME_CLEAN_BOLD_14.c_str()),
        (char *)(gFONTTAG_CLEAN_BOLD_14.c_str()),
        &gCfgEditFontList);

    createAndAppendFont(
        display,
        (char *)(FONTNAME_CLEAN_BOLD_12.c_str()),
        (char *)(gFONTTAG_CLEAN_BOLD_12.c_str()),
        &gCfgEditFontList);

    createAndAppendFont(
        display,
        (char *)(FONTNAME_CLEAN_MEDIUM_12.c_str()),
        (char *)(gFONTTAG_CLEAN_MEDIUM_12.c_str()),
        &gCfgEditFontList);

    // Determine if on a B/W or color display to set resources 
    Visual *myvis = DefaultVisual(display, DefaultScreen(display));

    // figure out the background and foreground for various types of systems 
    if (myvis->map_entries > 2)
    {
        /* It's a color display */
        // fprintf(stderr,"color display\n");
    }
    else
    {
        // It's a B/W display
        // fprintf(stderr,"B/W display\n");

        // Add a fallback resource which will make the shadows black
        pos = sizeof(fallback_resources) / sizeof(fallback_resources[0]) - 2;

        fallback_resources[pos++] = "cfgedit*.bottomShadowColor: black";
    }

    // create the top-level shell widget and initialize the toolkit
    //  (Note: Run this first to strip off X-related options)
    screen_num = DefaultScreen(display);

    gDisplayHeight = DisplayHeight(display, screen_num);
    gDisplayWidth = DisplayWidth(display, screen_num);

    if (!SCREEN_CAPTURE)
    {
        sprintf(
            geo,
            "cfgedit.geometry: %dx%d+10+0",
            gDisplayWidth - 30,
            gDisplayHeight - 50);
    }
    else
    {
        sprintf(geo, "cfgedit.geometry: 1200x800");
    }

    fallback_resources[0] = geo;

    sprintf(title, " cfgedit  v%s   (c) Georgia Institute of Technology", version_str);
    top_level = XtVaAppInitialize
        (&app, "cfgedit", NULL, 0,
         &argc, argv, fallback_resources,
         XmNtitle, title,
         XmNdefaultFontList, gCfgEditFontList,
         NULL);


    // ************************************************************

    if (verbose)
    {
        fprintf(stderr, "\nRC List:\n");

        const char *key;
        const rc_chain *val;

        valid = rc_table.first(&key, &val);

        while (valid)
        {
            fprintf(stderr, "\t%s\t=\t", key);

            p = val->first(&str);

            while (p)
            {
                fprintf(stderr, "%s", str);
                p = val->next(&str, p);

                if (p)
                {
                    fprintf(stderr, ",");
                }
            }
            fprintf(stderr, "\n");

            valid = rc_table.next(&key, &val);
        };

        fprintf(stderr, "\n");

        // Report the privileges
        index = 0;
        fprintf(stderr, "User Privileges:\n");

        while(privilege_table[index].name)
        {
            fprintf(
                stderr,
                "\t%s\t:\t%s\n",
                privilege_table[index].name,
                user_privileges & privilege_table[index].priv ? "True" : "False");
   
            index++;
        }
    }

    // Add actions
    actions[0].string = "da_mouse_cb";
    actions[0].proc = (XtActionProc)da_mouse_cb;
    XtAppAddActions(app, actions, 1);


    // create the main window widget.
    main_window = XtVaCreateManagedWidget
        ("cfgedit", xmMainWindowWidgetClass, top_level,
         XmNwidth, WINDOW_WIDTH,
         XmNheight, WINDOW_HEIGHT,
         NULL);

    // Retrieve the background and foreground colors from the widget's resources.
    {
        dpy = XtDisplay(main_window);
        scr = DefaultScreen(dpy);
        vis = DefaultVisual(dpy, scr);
        cmap = DefaultColormap(dpy, scr);

        if(!XParseColor(dpy, cmap, "black",&black_color))
        {
            cerr << "Unable to allocate black XColor\n";
        }

        if(!XParseColor(dpy, cmap, "white",&white_color))
        {
            cerr << "Unable to allocate white XColor\n";
        }

        // figure out the background and foreground for various types of systems 
        if (vis->map_entries > 2)
        {
            // It's a color display
            XtVaGetValues(
                main_window,
                XmNforeground, &fg,
                XmNbackground, &bg,
                NULL);

            gCfgEditPixels.black = fg;
            gCfgEditPixels.blue = createPixel(dpy, cmap, "blue");
            gCfgEditPixels.red = createPixel(dpy, cmap, "red");
            gCfgEditPixels.yellow = createPixel(dpy, cmap, "yellow");
            gCfgEditPixels.green = createPixel(dpy, cmap, "green");
            //gCfgEditPixels.select = createPixel(dpy, cmap, "light steel blue");
            gCfgEditPixels.select = createPixel(dpy, cmap, "DarkSeaGreen2");
            gCfgEditPixels.insensitive = createPixel(dpy, cmap, "gray80");
            gCfgEditPixels.smoke = createPixel(dpy, cmap, "white");
            gCfgEditPixels.plum = createPixel(dpy, cmap, "plum");
            gCfgEditPixels.darkseagreen2 = createPixel(dpy, cmap, "darkseagreen2");
            gCfgEditPixels.snow4 = createPixel(dpy, cmap, "snow4");
            gCfgEditPixels.white = createPixel(dpy, cmap, "white");
            gCfgEditPixels.khaki = createPixel(dpy, cmap, "khaki");
            gCfgEditPixels.darkslateblue = createPixel(dpy, cmap, "darkslateblue");
            gCfgEditPixels.gray95 = createPixel(dpy, cmap, "gray95");
            gCfgEditPixels.gray90 = createPixel(dpy, cmap, "gray90");
            gCfgEditPixels.gray85 = createPixel(dpy, cmap, "gray85");
            gCfgEditPixels.gray80 = createPixel(dpy, cmap, "gray80");
            gCfgEditPixels.gray75 = createPixel(dpy, cmap, "gray75");
            gCfgEditPixels.gray60 = createPixel(dpy, cmap, "gray60");
            gCfgEditPixels.gray65 = createPixel(dpy, cmap, "gray65");
            gCfgEditPixels.gray55 = createPixel(dpy, cmap, "gray55");
            gCfgEditPixels.gray50 = createPixel(dpy, cmap, "gray50");
            gCfgEditPixels.gray45 = createPixel(dpy, cmap, "gray45");
            gCfgEditPixels.gray40 = createPixel(dpy, cmap, "gray40");
            gCfgEditPixels.gray35 = createPixel(dpy, cmap, "gray35");
            gCfgEditPixels.gray30 = createPixel(dpy, cmap, "gray30");
            gCfgEditPixels.gray25 = createPixel(dpy, cmap, "gray25");
            gCfgEditPixels.gray20 = createPixel(dpy, cmap, "gray20");
            gCfgEditPixels.gray15 = createPixel(dpy, cmap, "gray15");
            gCfgEditPixels.gray10 = createPixel(dpy, cmap, "gray10");
            gCfgEditPixels.gray5 = createPixel(dpy, cmap, "gray5");
            gCfgEditPixels.XORFg = gCfgEditPixels.red;
            gCfgEditPixels.XORBg = bg;
        }
        else
        {
            // It's a monochrome display
            XtVaGetValues(
                main_window,
                XmNforeground, &fg,
                XmNbackground, &bg,
                NULL);

            monochrome = true;

            gCfgEditPixels.black = fg;
            gCfgEditPixels.blue = fg;
            gCfgEditPixels.red = fg;
            gCfgEditPixels.yellow = fg;
            gCfgEditPixels.green = fg;
            gCfgEditPixels.select = fg;
            gCfgEditPixels.insensitive = bg;
            gCfgEditPixels.smoke = bg;
            gCfgEditPixels.plum = fg;
            gCfgEditPixels.darkseagreen2 = fg;
            gCfgEditPixels.snow4 = fg;
            gCfgEditPixels.white = bg;
            gCfgEditPixels.khaki = bg;
            gCfgEditPixels.darkslateblue = fg;
            gCfgEditPixels.gray95 = bg;
            gCfgEditPixels.gray90 = bg;
            gCfgEditPixels.gray85 = bg;
            gCfgEditPixels.gray80 = bg;
            gCfgEditPixels.gray75 = bg;
            gCfgEditPixels.gray70 = bg;
            gCfgEditPixels.gray65 = bg;
            gCfgEditPixels.gray60 = bg;
            gCfgEditPixels.gray55 = bg;
            gCfgEditPixels.gray50 = fg;
            gCfgEditPixels.gray45 = fg;
            gCfgEditPixels.gray40 = fg;
            gCfgEditPixels.gray35 = fg;
            gCfgEditPixels.gray30 = fg;
            gCfgEditPixels.gray25 = fg;
            gCfgEditPixels.gray20 = fg;
            gCfgEditPixels.gray15 = fg;
            gCfgEditPixels.gray10 = fg;
            gCfgEditPixels.gray5 = fg;
            gCfgEditPixels.XORFg = bg;
            gCfgEditPixels.XORBg = fg;
        }
    }

    BuildBitMaps(fg, bg, gCfgEditPixels.red);

    menu_bar = XmCreateMenuBar(main_window, "Menubar", NULL, 0);

    XtManageChild(menu_bar);

    fileCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "File", 'F', file_items);
    editCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Edit", 'E', edit_items);
    layoutCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Layout", 'A', layout_items);
    configureCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Configure", 'C', configure_items);
    libraryCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Libraries", 'L', library_items);
    bindingCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Binding", 'B', binding_items); 
    mexpCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Expert", 'x', missionExpert_items);
    
    // old menu interface to cim.sgig
    //cimCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "CIM Managment", 'M', cim_items);
    //cimCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "SGIG Managment", 'M', sgig_items);

    toolsCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Tools", 'T', tools_items);
    helpCascade_w = BuildMenu(menu_bar, XmMENU_PULLDOWN, "Help", 'H', help_items);

    // Init the configuration menu to match the settings read from the cfg file
    if(name_visibility & ARCH_AuRA_BIT)
    {
        XmToggleButtonSetState(configure_items[4].widget, true, false);
    }
    if(name_visibility & ARCH_AuRA_urban_BIT)
    {
        XmToggleButtonSetState(configure_items[5].widget, true, false);
    }
    if(name_visibility & ARCH_AuRA_naval_BIT)
    {
        XmToggleButtonSetState(configure_items[6].widget, true, false);
    }
    if(name_visibility & ARCH_UGV_BIT)
    {
        XmToggleButtonSetState(configure_items[7].widget, true, false);
    }

    // Resize toggles
    XtVaSetValues(
        configure_items[4].widget,
        XmNindicatorSize, 15,
        NULL);

    XtVaSetValues(
        configure_items[5].widget,
        XmNindicatorSize, 15,
        NULL);

    XtVaSetValues(
        configure_items[6].widget,
        XmNindicatorSize, 15,
        NULL);

    XtVaSetValues(
        configure_items[7].widget,
        XmNindicatorSize, 15,
        NULL);

    if(!(user_privileges & LIBRARY_PRIV))
    {
        // User does not have a privilege to modify the CDL library
        // content.
        XtSetSensitive(library_items[3].widget, false);
        XtSetSensitive(library_items[4].widget, false);
    }

    // Let the menu bar know the last button is the help button
    // THE "HELP" MENU MUST BE THE LAST ONE INSTALLED FOR THIS TO WORK!
    {
        WidgetList buttons;
        Cardinal num_buttons;

        XtVaGetValues(menu_bar,
                      XmNchildren, &buttons,

                      XmNnumChildren, &num_buttons,
                      NULL);
        XtVaSetValues(menu_bar,
                      XmNmenuHelpWidget, (XtArgVal) buttons[num_buttons - 1],
                      NULL);
    }

    // Help Index it too outdated
    //XtSetSensitive(help_items[4].widget, false);

    // create a Form widget to be the work area.
    Widget work_area = XtCreateManagedWidget(
        "WorkArea", xmFormWidgetClass, main_window, NULL, 0);

    XtManageChild(work_area);

    // create the command bar form
    Widget command_bar = XtVaCreateManagedWidget(
        "CommandBar", xmFormWidgetClass, work_area,
        XmNtopAttachment, XmATTACH_FORM,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);

    XtManageChild(command_bar);

    {
        labelStr = XmStringCreateLocalized("File: ");
        Widget label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_FORM,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);

        labelStr = XmStringCreateLocalized("UNTITLED");
        cur_file_label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftWidget, label,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);
    }

    {
        Widget sep = XtVaCreateManagedWidget
            ("", xmSeparatorGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftOffset, 5,
             XmNleftWidget, cur_file_label,
             XmNorientation, XmVERTICAL,
             NULL);

        labelStr = XmStringCreateLocalized("Architecture: ");
        Widget label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftOffset, 5,
             XmNleftWidget, sep,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);

        labelStr = XmStringCreateLocalized("free");
        cur_arch_label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftWidget, label,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);
    }

    {
        Widget sep = XtVaCreateManagedWidget
            ("", xmSeparatorGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftOffset, 5,
             XmNleftWidget, cur_arch_label,
             XmNorientation, XmVERTICAL,
             NULL);

        labelStr = XmStringCreateLocalized("Current Page: ");
        Widget label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftOffset, 5,
             XmNleftWidget, sep,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);

        labelStr = XmStringCreateLocalized("Configuration UNTITLED");

        cur_page_label = XtVaCreateManagedWidget
            ("CommandBarLabel", xmLabelGadgetClass,
             command_bar,
             XmNtopAttachment, XmATTACH_FORM,
             XmNbottomAttachment, XmATTACH_FORM,
             XmNleftAttachment, XmATTACH_WIDGET,
             XmNleftOffset, 5,
             XmNleftWidget, label,
             XmNlabelString, labelStr,
             XmNalignment, XmALIGNMENT_BEGINNING,
             NULL);

        XmStringFree(labelStr);
    }

    // Create toolbar
    Widget toolbar = XtVaCreateManagedWidget
        ("",
         xmRowColumnWidgetClass, work_area,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, command_bar,
         XmNleftAttachment, XmATTACH_FORM,
         //	XmNpacking, XmPACK_COLUMN,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         //	XmNentryAlignment, XmALIGNMENT_CENTER,
         XmNentryAlignment, XmALIGNMENT_BEGINNING,
         NULL);

    // ************* Tools ****************

    // Create name
    labelStr = XmStringCreateLocalized("Navigation");
    XtVaCreateManagedWidget
        ("", 
         xmLabelGadgetClass, toolbar,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);

    // Create frame
    Widget tool_frame = XtVaCreateManagedWidget
        ("",
         xmFrameWidgetClass, toolbar,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         NULL);

    // Create toolbar
    Widget tools = XtVaCreateManagedWidget
        ("",
         xmRowColumnWidgetClass, tool_frame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         NULL);

    // ********* left column ***********

    // Add the up button
    labelStr = XmStringCreateLocalized("Up 1 Level");
    up_button = XtVaCreateManagedWidget
        ("Button",
         xmPushButtonWidgetClass, tools,
         XmNlabelType, XmSTRING,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);
    XtAddCallback(
        up_button,
        XmNactivateCallback,
        (XtCallbackProc)set_mode, 
        (void *)MOVE_UP);


    // ************* Construction ****************

    // Create name
    labelStr = XmStringCreateLocalized("Construction");
    XtVaCreateManagedWidget
        ("", 
         xmLabelGadgetClass, toolbar,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);

    // Create frame
    Widget construct_frame = XtVaCreateManagedWidget
        ("",
         xmFrameWidgetClass, toolbar,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         NULL);

    // Create the arrangement rowcol
    Widget construct = XtVaCreateManagedWidget
        ("",
         xmRowColumnWidgetClass, construct_frame,
         XmNpacking, XmPACK_TIGHT,
         XmNorientation, XmVERTICAL,
         XmNnumColumns, 1,
         XmNisAligned, True,
         XmNentryAlignment, XmALIGNMENT_CENTER,
         NULL);

    // Add the start over button.
    // This Start Over button will quit the current configuration
    // and start new configuration.
    labelStr = XmStringCreateLocalized("Start Over");
    start_over_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);

    XtAddCallback(start_over_button,
                  XmNactivateCallback,
                  (XtCallbackProc) set_mode,
                  (void *)START_OVER);

    // ********* left column ***********
    // Add the new_state button
    new_state_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmPIXMAP,
        XmNlabelPixmap, pm_new_state,
        XmNlabelInsensitivePixmap, ispm_new_state,
        NULL);

    XtAddCallback(
        new_state_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode, 
        (void *)PLACE_STATE);
    
    // Add the new_trans button
    new_trans_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmPIXMAP,
        XmNlabelPixmap, pm_new_trans,
        XmNlabelInsensitivePixmap, ispm_new_trans,
        NULL);

    XtAddCallback(
        new_trans_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode,
        (void *)START_CONNECT_TRANSITION);

    // Add the new_waypoint button
    labelStr = XmStringCreateLocalized("Waypoints");
    new_waypoint_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);
    XtAddCallback(
        new_waypoint_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode,
        (void *)WAYPOINT_DESIGN);

    if (disableWaypointsFunction)
    {
        XtSetSensitive(new_waypoint_button, false);
    }

    /*planner*/
    // Add the new planner button
    labelStr= XmStringCreateLocalized("Path Plan");
    new_planner_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);

    XtAddCallback(
        new_planner_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode,
        (void *)PLANNER_DESIGN);

    if (disablePathPlanner)
    {
        XtSetSensitive(new_planner_button, false);
    }

    // Add  new_operator button
    labelStr = XmStringCreateLocalized("Operator");
    new_oper_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);
    XtAddCallback(
        new_oper_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode,
        (void *)NEW_OPERATOR);

    // Add  new_agent button
    labelStr = XmStringCreateLocalized("Agent");
    new_agent_button = XtVaCreateManagedWidget
        ("Button",
         xmPushButtonWidgetClass, construct,
         XmNlabelType, XmSTRING,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);
    XtAddCallback
        (new_agent_button,
         XmNactivateCallback,
         (XtCallbackProc) set_mode,
         (void *)NEW_AGENT);

    labelStr = XmStringCreateLocalized("IBP");
    new_ibp_button = XtVaCreateManagedWidget
        ("Button",
         xmPushButtonWidgetClass, construct,
         XmNlabelType, XmSTRING,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);
    XtAddCallback
        (new_ibp_button,
         XmNactivateCallback,
         (XtCallbackProc) set_mode,
         (void *)NEW_IBP);

    // ********** right column ***********

    // Add an output binding point button
    labelStr = XmStringCreateLocalized("OBP");
    new_obp_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, construct,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);

    XtAddCallback(
        new_obp_button,
        XmNactivateCallback,
        (XtCallbackProc) set_mode,
        (void *)NEW_OBP);
    
    // Add  new_trigger button
    labelStr = XmStringCreateLocalized("Trigger");
    new_trigger_button = XtVaCreateManagedWidget
        ("Button",
         xmPushButtonWidgetClass, construct,
         XmNlabelType, XmSTRING,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);
    XtAddCallback
        (new_trigger_button,
         XmNactivateCallback,
         (XtCallbackProc) set_mode,
         (void *)NEW_TRIGGER);
   
    // Add  new_primitive button
    labelStr = XmStringCreateLocalized("Primitive");
    new_prim_button = XtVaCreateManagedWidget
        ("Button",
         xmPushButtonWidgetClass, construct,
         XmNlabelType, XmSTRING,
         XmNlabelString, labelStr,
         NULL);
    XmStringFree(labelStr);
    XtAddCallback
        (new_prim_button,
         XmNactivateCallback,
         (XtCallbackProc) set_mode,
         (void *)NEW_PRIMITIVE);

    // ************* Execution ****************

    // Create name
    labelStr = XmStringCreateLocalized("Execution");
    XtVaCreateManagedWidget("", 
                            xmLabelGadgetClass, toolbar,
                            XmNlabelString, labelStr,
                            NULL);
    XmStringFree(labelStr);

    // Create frame
    Widget exec_frame = XtVaCreateManagedWidget(
        "",
        xmFrameWidgetClass, toolbar,
        XmNshadowType, XmSHADOW_ETCHED_OUT,
        NULL);

    // Create the arrangement rowcol
    Widget exec = XtVaCreateManagedWidget(
        "",
        xmRowColumnWidgetClass, exec_frame,
        XmNpacking, XmPACK_TIGHT,
        XmNorientation, XmVERTICAL,
        XmNnumColumns, 1,
        XmNisAligned, True,
        XmNentryAlignment, XmALIGNMENT_CENTER,
        NULL);

    // ********* left column ***********

    // Add the analyze button
    labelStr = XmStringCreateLocalized("Analyze");
    analyze_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, exec,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);

    XtAddCallback(
        analyze_button,
        XmNactivateCallback,
        (XtCallbackProc) analyze,
        NULL);

    if (disableHoneywellRTAnalyzer) 
    {
        XtSetSensitive(analyze_button, false);
    }

    // Add the output button
    labelStr = XmStringCreateLocalized("Compile");
    output_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, exec,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);

    XtAddCallback(
        output_button,
        XmNactivateCallback,
        (XtCallbackProc)make,
        NULL);

    // Add the execute button
    labelStr = XmStringCreateLocalized("Run");
    run_button = XtVaCreateManagedWidget(
        "Button",
        xmPushButtonWidgetClass, exec,
        XmNlabelType, XmSTRING,
        XmNlabelString, labelStr,
        NULL);
    XmStringFree(labelStr);
    XtAddCallback(
        run_button,
        XmNactivateCallback,
        (XtCallbackProc)run,
        NULL);


    /* ---------------------------------------------------------------- */
    scrolled_window = XtVaCreateManagedWidget
        ("ScrolledWindow",
         xmScrolledWindowWidgetClass, work_area,
         XmNtopAttachment, XmATTACH_WIDGET,
         XmNtopWidget, command_bar,
         XmNleftAttachment, XmATTACH_WIDGET,
         XmNleftWidget, toolbar,
         XmNrightAttachment, XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNscrollBarDisplayPolicy, XmAS_NEEDED,
         XmNscrollingPolicy, XmAUTOMATIC,
         XmNvisualPolicy, XmCONSTANT,
         NULL);

    mouse_translations = XtParseTranslationTable(translations);

    // Create the drawing area
    drawing_area = XtVaCreateWidget(
        "DrawingArea",
        xmDrawingAreaWidgetClass, scrolled_window,
        XmNresizePolicy, XmNONE,
        XmNwidth, DRAWING_WIDTH,
        XmNheight, DRAWING_HEIGHT,
        XmNtranslations, mouse_translations,
        NULL);

    // Can't override Btn1Motion due to bug in X11R4, so replace entire table
    // above in the drawing_area def.
    // Add the required translations for the mouse movements
    //  XtOverrideTranslations(drawing_area, XtParseTranslationTable(translations));

    // add a callback for expose events.
    XtAddCallback(drawing_area, XmNexposeCallback, (XtCallbackProc)refresh_screen, NULL);

    XtManageChild(drawing_area);


    /* ---------------------------------------------------------------- */

    /* attach the menubar and the drawing_area area to the main window */
    XmMainWindowSetAreas(main_window, menu_bar, NULL, NULL, NULL, work_area);

    /* ---------------------------------------------------------------- */

    // get colors used by drawing_area for connections
    XtVaGetValues(
        drawing_area,
        XmNforeground, &fg,
        XmNbackground, &bg,
        NULL);

    // define a black GC
    gCfgEditGCs.black = createGC(
        fg,
        bg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        false);

    // define a GC for the glyphs when they are not selected
    gCfgEditPixels.glyphFg = gCfgEditPixels.red;
    gCfgEditPixels.glyphBg = gCfgEditPixels.smoke;
    gCfgEditGCs.glyph = createGC(
        gCfgEditPixels.glyphFg,
        gCfgEditPixels.glyphBg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        false);

    // define a GC for the circles when they are not selected
    gCfgEditPixels.stateFg = gCfgEditPixels.black;
    gCfgEditPixels.stateBg = bg;
    gCfgEditGCs.state = createGC(
        gCfgEditPixels.stateFg,
        gCfgEditPixels.stateBg,
        main_window,
        CIRCLE_LINE_WIDTH,
        false,
        false);

    // define a GC for selection
    gCfgEditGCs.select = createGC(
        gCfgEditPixels.select,
        bg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        false);

    // define a GC for selection
    gCfgEditGCs.highlight = createGC(
        gCfgEditPixels.darkseagreen2,
        bg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        false);

    // define a GC to draw the state transitions
    gCfgEditPixels.transitionFg = gCfgEditPixels.black;
    gCfgEditPixels.transitionBg = bg;
    gCfgEditGCs.transition = createGC(
        gCfgEditPixels.transitionFg,
        gCfgEditPixels.transitionBg,
        main_window,
        TRANS_LINE_WIDTH,
        true,
        false);

    // define an XOR GC.
    gCfgEditGCs.XOR = createGC(
        gCfgEditPixels.XORFg,
        gCfgEditPixels.XORBg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        true);

    // define a GC to erase
    gCfgEditGCs.erase = createGC(
        bg,
        fg,
        main_window,
        DEFAULT_LINE_WIDTH,
        false,
        false);

    /* ------------------------------------------------------------- */

    /* create the dialog boxes */
    create_error_dialog(main_window);
    create_help_dialog(main_window);

    /* ------------------------------------------------------------- */

    /* Realize all widgets */
    XtRealizeWidget(top_level);

    /* ------------------------------------------------------------- */

    // Disable file save, no filename loaded
    current_filename = NULL;
    reset_file_save(false);

    /* ------------------------------------------------------------- */

    erase_drawing_area();

    /* ------------------------------------------------------------- */

    // Init the help subsystem
    init_help();

    // Instantiate AssistantDialog
    gAssistantDialog = new AssistantDialog(
        main_window,
        rc_table,
        ASSISTANT_INIT_POPUP_RCVALUE);

    if (gAssistantDialog->isDiabled())
    {
        XtSetSensitive(help_items[2].widget, false);
    }

    /* ------------------------------------------------------------- */

    config = new configuration;
    gPopWin = new PopUpWindows(main_window, app);
    gMExp = new MissionExpert(
        display,
        main_window,
        app,
        rc_table,
        mexpRCFileName);

    // Disabled buttons if if necessary.

    if ((gMExp->isDisabled()) ||
        (check_bool_rc(rc_table, "HideMissionExpertMenu")))
    {
        // Disabled the "Expert" button in the menu.
        XtSetSensitive(mexpCascade_w, false);
    }
    else if (gMExp->repairDisabled())
    {
        // Disabled the playback button in the Expert menu.
        XtSetSensitive(missionExpert_items[5].widget, false);
    }

    if(!(user_privileges & MEXP_MODIFY_CBRLIB_PRIV))
    {
        // User does not have a privilege to modify the CBRServer's
        // library content.
        XtSetSensitive(missionExpert_items[4].widget, false);
    }


    reset_placement_list();

    /* ------------------------------------------------------------- */


    // Init the parser
    if (init_cdl(debug_load_cdl > 2))
    {
        fprintf(stderr, "Internal Error: unable to initialize the parser!\n");
        exit(1);
    }

    /* ------------------------------------------------------------- */
    // Load any libraries that were specified

    if (verbose)
    {
        fprintf(stderr, "\nLoading libraries:\n");
    }

    // Extract the library options
    const rc_chain *files = rc_table.get(CDL_LIBRARIES);

    if (files)
    {
        p = files->first(&str);

        while (p)
        {
            fullname = strdupcat(strdup(str), ".gen");

            chk = fopen(fullname, "r");

            if (chk == NULL)
            {
                cerr << "Warning: Didn't find " << fullname << '\n';
            }
            else
            {
                fclose(chk);

                if (verbose)
                    cerr << "Loading " << fullname << " ...";

                const char *namelist[2];

                namelist[0] = fullname;
                namelist[1] = NULL;

                Symbol *library_header = new Symbol(LIBRARY_HEADER);
                library_header->name = strdup(str);

                Symbol *library_rec = new Symbol(LIBRARY_RECORD);
                library_rec->name = strdup(fullname);
                library_rec->arch = free_arch;
                load_cdl(namelist, errors, true, debug_load_cdl > 1);
                library_header->children.append(library_rec);

                if (verbose)
                {
                    cerr << " Done\n";
                }
                free(fullname);

                // Try to load any architecture specific versions of the library
                cur = defined_architectures.first(&arch);

                while (cur)
                {
                    fullname = strdupcat(strdup(str), ".", arch->name);

                    chk = fopen(fullname, "r");

                    if (chk)
                    {
                        fclose(chk);

                        if (verbose)
                        {
                            cerr << "Loading " << fullname << " ...";
                        }
                        const char *namelist[2];

                        namelist[0] = fullname;
                        namelist[1] = NULL;

                        Symbol *library_rec = new Symbol(LIBRARY_RECORD);
                        library_rec->name = strdup(fullname);
                        library_rec->arch = arch;
                        load_cdl(namelist, errors, true, debug_load_cdl > 1);
                        library_header->children.append(library_rec);

                        if (verbose)
                        {
                            cerr << " Done\n";
                        }
                        free(fullname);
                    }

                    cur = defined_architectures.next(&arch, cur);
                }

                // Remember the libraries that we load
                loaded_libraries.append(library_header);
            }

            p = files->next(&str, p);
        }
    }

    /* ------------------------------------------------------------- */
    // Mark that any new objects are at the user level
    set_user_class();

    // Can't run an empty config	
    XtSetSensitive(run_button, false);
    /* ------------------------------------------------------------- */

    // Enable event logging if specified
    files = rc_table.get("EventLogDir");
    if (files && files->first(&dir))
    {
        gEventLogging->initialize(dir);

        // Dump any startup events specified on the command line
        while(!startup_events.isempty())
        {
            event = startup_events.get();
            sprintf(buf,"%s \"%s\"", strtok(event," "), strtok(NULL," "));

            gEventLogging->status(buf);
        }
    }

    /* ------------------------------------------------------------- */
    // if specified a filename, then try to load it
    if (argc > optind)
    {
        filename = argv[optind];

        fprintf(stderr, "Loading %s ...", filename);

        if (config->load(filename))
        {
            fprintf(stderr, " Unable to load file\n");
        }
        else
        {
            fprintf(stderr, " Done\n");

            if (filename_has_directory(filename))
            {
                remove_directory(filename, simpleFilename);
                sprintf(msg, "%s_loaded", simpleFilename);
            }
            else
            {
                sprintf(msg, "%s_loaded", filename);
            }
            save_cdl_for_replay(msg);

            // Remember the filename
            change_filename(filename);
            reset_file_save(true);
            showInitDialog = false;
        }
    }
    else if (loadMExpFeatureFile)
    {
        gMExp->saveSpecifiedFeatureFileName(mexpFeatureFileName);
        gMExp->runMissionExpert(MEXP_RUNMODE_VIEW);
        showInitDialog = false;
    }

    // Get any names that have been defined
    config->create_name_lists();

    // Mark doesn't need save
    config->did_save();

    // Pick a good set
    reset_placement_list();

    // Make the drawing area get keyboard events.
    XmProcessTraversal(drawing_area, XmTRAVERSE_CURRENT);

    // Put X into syncronous mode for debugging
    //XSynchronize(XtDisplay(main_window), true);

    if(showInitDialog)
    {
        gPopWin->runInitialDialog();
    }

    // assert(malloc_verify());
    /* start the main event-handling loop */
    XtAppMainLoop(app);
}

///////////////////////////////////////////////////////////////////////
// $Log: menu.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.15  2007/10/17 17:10:37  pulam
// s/BAMS Wizard/UAV Wizard/
//
// Revision 1.14  2007/09/07 23:10:03  endo
// The overlay name is now remembered when the coordinates are picked from an overlay.
//
// Revision 1.13  2007/08/10 15:14:59  endo
// CfgEdit can now save the CBR library via its GUI.
//
// Revision 1.12  2007/07/17 21:53:51  endo
// Meta data sorting function added.
//
// Revision 1.11  2007/05/15 18:50:00  endo
// BAMS Wizard implemented.
//
// Revision 1.10  2007/05/14 20:40:49  pulam
// Added interface for SGIG
//
// Revision 1.9  2006/10/02 19:19:14  endo
// Bug related to XtGrab fixed.
//
// Revision 1.8  2006/09/26 18:30:27  endo
// ICARUS Wizard integrated with Lat/Lon.
//
// Revision 1.7  2006/09/21 14:47:54  endo
// ICARUS Wizard improved.
//
// Revision 1.6  2006/09/20 22:08:28  alanwags
// Add Meyers singleton pattern for actual cim communications
//
// Revision 1.5  2006/09/13 19:03:48  endo
// ICARUS Wizard implemented.
//
// Revision 1.4  2006/08/25 20:12:39  alanwags
// Moved the GT client object
//
// Revision 1.3  2006/08/19 17:01:13  alanwags
// Added cim management menu
//
// Revision 1.2  2006/07/26 18:07:47  endo
// ACDLPlus class added.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.12  2006/07/10 21:47:39  endo
// GUI Clean-up.
//
// Revision 1.11  2006/07/10 17:47:34  endo
// Clean-up for MissionLab 7.0 release.
//
// Revision 1.10  2006/07/10 06:09:43  endo
// Clean-up for MissionLab 7.0 release.
//
// Revision 1.9  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.8  2006/02/19 17:52:51  endo
// Experiment related modifications
//
// Revision 1.7  2006/02/14 02:27:18  endo
// gAutomaticExecution flag and its capability added.
//
// Revision 1.6  2006/01/30 02:47:28  endo
// AO-FNC CBR-CNP Type-I check-in.
//
// Revision 1.5  2005/10/21 00:30:16  endo
// Event Log for GOMS Test.
//
// Revision 1.4  2005/08/09 19:12:44  endo
// Things improved for the August demo.
//
// Revision 1.3  2005/05/18 21:14:43  endo
// AuRA.naval added.
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.137  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.136  2002/05/06 17:59:14  endo
// "Delete" functionality added in the edit menu.
//
// Revision 1.135  2002/01/31 10:44:20  endo
// Parameters chaged due to the change of max_vel and base_vel.
//
// Revision 1.134  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.133  2001/12/23 20:12:06  endo
// RH 7.1 porting.
//
// Revision 1.132  2001/09/14 01:34:12  ebeowulf
// Filled the global variables for display size
//
// Revision 1.131  2001/02/01 20:06:55  endo
// Modification to display the robot ID and name in the
// "Individual Robot" glyph was added.
//
// Revision 1.130  2000/04/16 16:00:43  endo
// Fixed toolbar not showng Planner button when it is
// not supposed to.
//
// Revision 1.129  2000/04/13 21:47:30  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.128  2000/03/23 20:03:44  endo
// Made cfgedit to turn off or on path_planner based on
//  the flag specified in .cfgeditrc.
//
// Revision 1.127  2000/03/22 05:38:18  saho
// *** empty log message ***
//
// Revision 1.126  2000/03/22 02:10:13  saho
// Added code for the new Planner button in cfgedit.
//
// Revision 1.125  2000/03/13 23:39:59  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.124  2000/02/15 22:27:12  sapan
// *** empty log message ***
//
// Revision 1.122  2000/02/10 04:40:01  endo
// Checked in for Douglas C. MacKenzie.
// He got rid of warning signs upon compilation.
//
// Revision 1.121  2000/02/03 00:07:20  endo
// *** empty log message ***
//
// Revision 1.119  2000/02/02 23:42:21  jdiaz
// waypoints support
//
// Revision 1.118  2000/01/20 03:09:22  endo
// Code checked in for doug. A feature for
// cfgedit to allow toggling constants to
// input allows was added.
//
// Revision 1.117  1999/12/18 10:08:30  endo
// hide_FSA_detail capability added.
//
// Revision 1.116  1999/12/18 00:04:23  endo
// Display width restored.
//
// Revision 1.115  1999/12/16 21:07:01  endo
// rh-6.0 porting.
//
// Revision 1.114  1999/09/03 15:31:21  endo
// ARCH_AuRA_urban_BIT stuff added.
//
// Revision 1.113  1999/03/08 20:44:55  dj
// modified button structure, added analyze button
//
// Revision 1.112  1999/02/22 05:29:30  dj
// No more compiler warnings
//
// Revision 1.111  1999/02/19 23:16:25  dj
// fixed some typos
//
// Revision 1.110  1999/02/19  16:09:21  dj
// Moved bind/unbind buttons to the menu bar.  The window automatically
// fills the screen now.  Cleaned up some unused code.
//
// Revision 1.109  1998/11/12 19:45:46  endo
// Buttons renamed, so that they are easy to be understood.
//
// Revision 1.108  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.107  1996/12/06  16:15:23  doug
// respelled loose as lose
//
// Revision 1.106  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.106  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.105  1996/06/02 16:25:59  doug
// added ability to group nodes
//
// Revision 1.104  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.103  1996/05/02  22:58:26  doug
// *** empty log message ***
//
// Revision 1.102  1996/04/11  04:07:42  doug
// *** empty log message ***
//
// Revision 1.101  1996/04/06  23:54:05  doug
// *** empty log message ***
//
// Revision 1.100  1996/03/12  17:45:14  doug
// *** empty log message ***
//
// Revision 1.99  1996/03/09  01:09:23  doug
// *** empty log message ***
//
// Revision 1.98  1996/03/08  00:46:56  doug
// *** empty log message ***
//
// Revision 1.97  1996/03/06  23:39:17  doug
// *** empty log message ***
//
// Revision 1.96  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.95  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.94  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.93  1996/02/28  03:56:24  doug
// *** empty log message ***
//
// Revision 1.92  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.91  1996/02/26  05:01:33  doug
// *** empty log message ***
//
// Revision 1.90  1996/02/22  00:55:17  doug
// adding popups to fsa states/trans
//
// Revision 1.89  1996/02/20  22:52:24  doug
// adding EditParms
//
// Revision 1.88  1996/02/20  20:00:22  doug
// *** empty log message ***
//
// Revision 1.87  1996/02/19  21:57:05  doug
// library components and permissions now work
//
// Revision 1.86  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.85  1996/02/17  17:05:53  doug
// *** empty log message ***
//
// Revision 1.84  1996/02/16  00:07:18  doug
// *** empty log message ***
//
// Revision 1.83  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.82  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.81  1996/02/04  23:21:52  doug
// *** empty log message ***
//
// Revision 1.80  1996/02/04  17:11:48  doug
// *** empty log message ***
//
// Revision 1.79  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.78  1996/02/01  04:04:30  doug
// *** empty log message ***
//
// Revision 1.77  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.76  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.75  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.74  1996/01/27  00:09:24  doug
// added grouped lists of parameters
//
///////////////////////////////////////////////////////////////////////
