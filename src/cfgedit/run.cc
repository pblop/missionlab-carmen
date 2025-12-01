/**********************************************************************
 **                                                                  **
 **                              run.cc                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: run.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <memory.h>
#include <malloc.h>
#include <stdarg.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/DrawingA.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/SeparatoG.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/ScrolledW.h>

#include "gt_robot_type.h"
#include "design.h"
#include "popups.h"
#include "help.h"
#include "prototypes.h"
#include "configuration.hpp"
#include "globals.h"
#include "screen_object.hpp"
#include "page.hpp"
#include "write_cdl.h"
#include "utilities.h"
#include "verify.h"
#include "cfgedit_common.h"
#include "renumber_robots.h"
#include "mission_expert.h"
#include "EventLogging.h"
#include "toolbar.h"
#include "mission_expert.h"
#include "assistantDialog.h"
#include "cnp_types.h"

using std::string;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
#define DEFAULT_OVERLAY_NAME_STR "Empty.ovl"
#define DEFAULT_HSERVER_NAME_STR "fred"
#define MAXARGS         20
const string EMPTY_STRING = "";
const string STRING_UNKNOWN = "(Unknown)";
const string STRING_EXTRA_ROBOT = "extra_robot";
const string STRING_DESIRED_ROBOT_ID = "desired_robot_id";
const int EVENT_WAITER_SLEEP_TIME_USEC = 100;
const int RC_STRING_FALSE = 0;
const int RC_STRING_TRUE = 1;
const int RC_STRING_UNSPECIFIED = -1;
const int RC_STRING_INVALID = -2;
const bool SHOW_NO_RUN_OPTION = false;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define RES_CONVERT(res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
static struct robot_options {
    Widget main_form;
    Widget *toggles;
    Widget w_remote_host;
    Widget w_color;
    Widget w_hserver_name;
    Widget ftp_toggle;
    int host_ppp;
    char **names;
    int num_toggles;
    int choice;	// Simulation is 0, others index into list
    int should_ftp; // FTP the robot executable
    char *remote_host_str; // Name of the remote host machine to execute the robot-executable
    char *color_str; // Color of the robot
    char *hserver_name_str; // ID that the robot executable uses to connect HServer via IPT
} robot_recs[MAX_ROBOTS];

//-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------
extern char *cmd_list_name;
extern bool gOverlayPicked;
extern string gPickedOverlayName;

Widget run_window;
Widget overlay_toggle;
Widget w_overlay_name;
Widget w_pause_toggle;
Widget w_logdata_toggle;
Widget w_show_robot_status_toggle;
Widget w_robot_feedback_toggle;
Widget w_cmdli_name;
Widget w_enable_3d_toggle;
Widget w_enable_runtime_cnp_toggle;
Widget w_norun_mlab_toggle;
Widget w_expiration_timer;
Widget w_console_jbox_id;

Widget run_local_dialog;

char simtext[] = "SimulationRobotSettings";
char *remotehost_dir = strdup(".");
rc_chain *remotehost_dir_ptr;

int use_ppp = false;

// Remember these variables, so that if the users run the mission again,
// they don't have to respecify.
int first_time = true;
int ask_default_overlay = false;     // Ask the users to choose the overlay from the popup menu .
int norun_mlab = false;              // Do not fork mlab. Just create an updated CMDL file instead.
int pause_execution = false;         // mlab waits for the users' confirmation to execute the robot-executable.
int log_data = false;                // Enable the data logging for the robot. Not the usability log.
int show_robot_status = false;       // Enable mlab to show the current status of the robot.
int enable_runtime_cnp = false;      // Enable runtime-CNP
int enable_3d = false;               // Enable mlab to do 3D
bool stop_robot_feedback = false;    // Disable the feedback from the robot to mlab.
char *overlay_name_str = NULL;       // Use this overlay.
int activateMlabMissionDesign = false;
int cnpMode = CNP_MODE_DISABLED;
string expiration_timer_value = STRING_UNKNOWN;
string cmdli_name_str = STRING_UNKNOWN;
string cmdli_name_text = EMPTY_STRING;
string console_jbox_id_str = STRING_UNKNOWN;
string console_jbox_id_text = EMPTY_STRING;
string mexpRCFileName = MEXP_DEFAULT_RC_FILENAME;
bool useCustomMExpRCFile = false;

static bool doneRunDialog;
static bool okRunDialog;

//-----------------------------------------------------------------------
static void done_run(void)
{
    gEventLogging->log("Run-button in Run-Dialog pressed.");
    doneRunDialog = true;
    okRunDialog = true;
}

//-----------------------------------------------------------------------
static void done_cancel(void)
{
    gEventLogging->log("Cancel-button in Run-Dialog pressed.");
    doneRunDialog = true;
    okRunDialog = false;
}

//-----------------------------------------------------------------------
static void create_run_popup(Widget parent)
{
    rc_chain *val = NULL;
    rc_chain *hostval = NULL;
    rc_chain *hserver_name_val = NULL;
    rc_chain *colorval = NULL;
    rc_chain *ovlval = NULL;
    XmString label;
    Widget super_frm;
    Widget frm;
    Widget scroll;
    Widget widgetAbove;
    Arg args[MAXARGS];
    Cardinal argcount = 0;
    string mexpOverlayName;
    char buf[1024];
    char *str = NULL; 
    int Winheight = 320;  // These numbers control the window size and were chosen
    int Winwidth = 290;   // by experimenting with different sizes
    int i, index;
    int set;
    void *p = NULL;

    sprintf(buf,"Configuration %s runtime options",config->configuration_name());
    XmString title = XSTRING(buf);
    XmString ok = XSTRING("Run");
    XmString abort_msg = XSTRING("Cancel");
    static XtCallbackRec ok_cb_list[] =
        {
            {(XtCallbackProc) done_run, NULL},
            {(XtCallbackProc) NULL, NULL}
        };
    static XtCallbackRec cancel_cb_list[] =
        {
            {(XtCallbackProc) done_cancel, NULL},
            {(XtCallbackProc) NULL, NULL}
        };
  
    // Create the dialog
    argcount = 0;
    XtSetArg(args[argcount], XmNdialogTitle, title);
    argcount++;
    XtSetArg(args[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);
    argcount++;
    XtSetArg(args[argcount], XmNokLabelString, ok);
    argcount++;
    XtSetArg(args[argcount], XmNcancelLabelString, abort_msg);
    argcount++;
    XtSetArg(args[argcount], XmNokCallback, ok_cb_list);
    argcount++;
    XtSetArg(args[argcount], XmNcancelCallback, cancel_cb_list);
    argcount++;
    run_window = XmCreateMessageDialog(parent, "log_window", args, argcount);
  
    XtUnmanageChild(XmMessageBoxGetChild(run_window, XmDIALOG_HELP_BUTTON));
    XmStringFree(title);
    XmStringFree(ok);
 
    super_frm = XtVaCreateManagedWidget("frm", xmFormWidgetClass, run_window, NULL);


    //if there is more than one robot, we can double the window size.
    //  however, if the window is too large, it runs off the screen, hence the
    //  need for a scrolling window
    if (num_robots>1)
    {
        Winheight = (int)(0.5*(float)gDisplayHeight);
    }

    scroll = XtVaCreateManagedWidget (
        "scrolled_w",
        xmScrolledWindowWidgetClass, super_frm,
        XmNwidth, Winwidth,
        XmNheight, Winheight,
        XmNscrollBarDisplayPolicy, XmSTATIC,
        XmNscrollingPolicy, XmAUTOMATIC,
        NULL);
  
    frm = XtVaCreateManagedWidget("frm", xmFormWidgetClass, scroll, NULL);
  
    for(i = 0; i < num_robots; i++)
    {
        // Make a form for this robot's options
        robot_recs[i].main_form = XtVaCreateManagedWidget(
            "main_form", 
            xmFormWidgetClass, frm, 
            XmNtopAttachment, (i>0)? XmATTACH_WIDGET : XmATTACH_FORM,
            XmNtopWidget, robot_recs[i-1].main_form,
            XmNtopOffset, (i>0)? 10 : 10,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            XmNwidth, Winwidth-35,
            XmNheight, 270,
            NULL);
      
        sprintf(buf,"Options for %s", RobotName(i+1));
        XmString label = XmStringCreateLocalized(buf);
        Widget w = XtVaCreateManagedWidget(
            "robot_name", 
            xmLabelGadgetClass, robot_recs[i].main_form, 
            XmNlabelString, label,
            NULL);
        XmStringFree(label);
      
        w = XtVaCreateManagedWidget(
            "w", 
            xmSeparatorGadgetClass, robot_recs[i].main_form, 
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, w,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
      
        Widget rc = XtVaCreateManagedWidget(
            "rc",
            xmRowColumnWidgetClass, robot_recs[i].main_form,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, w,
            XmNradioBehavior, true,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            NULL);
      
        label = XmStringCreateLocalized("Simulated robot");

        set = false;
        if (first_time || (robot_recs[i].choice == 0))
        {
            set = true;
        }

        w = XtVaCreateManagedWidget(
            "", 
            xmToggleButtonGadgetClass, rc, 
            XmNlabelString, label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, w,
            XmNset, set,
            NULL);
        XmStringFree(label);
      
        // Create a push button for each of real robots we know about
        val = (rc_chain *)rc_table.get("robots");
      
        // Allocate memory for the buttons
        robot_recs[i].num_toggles = 1 + val->len();
        robot_recs[i].toggles = new Widget[robot_recs[i].num_toggles];
        robot_recs[i].names = new char *[robot_recs[i].num_toggles];
      
        // Remember the simulation choice
        index = 0;
        robot_recs[i].names[index] = RobotName(i+1);
        robot_recs[i].toggles[index++] = w;
      
        if(val)
        {
            p = val->first(&str);

            while(p)
            {
                label = XmStringCreateLocalized(str);
                set = false;
                if (robot_recs[i].choice == index)
                {
                    set = true;
                }
                w = XtVaCreateManagedWidget(
                    "w", 
                    xmToggleButtonGadgetClass, rc,
                    XmNlabelString, label,
                    XmNtopAttachment, XmATTACH_WIDGET,
                    XmNtopWidget, w,
                    XmNset, set,
                    NULL);
                XmStringFree(label);
	      
                robot_recs[i].names[index] = strdup(str);
                robot_recs[i].toggles[index++] = w;
	      
                p = val->next(&str, p);
            }
        }
      
        sprintf(buf, "FTP Robot Executable");
        label = XmStringCreateLocalized(buf);
        robot_recs[i].ftp_toggle =  XtVaCreateManagedWidget(
            "ftp_toggle",
            xmToggleButtonWidgetClass,	robot_recs[i].main_form,
            XmNlabelString, label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, rc,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment, XmATTACH_FORM,
            XmNalignment, XmALIGNMENT_BEGINNING,
            XmNheight, 20,
            XmNset, robot_recs[i].should_ftp,
            NULL);
        XmStringFree(label);

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "Remote Host Computer");
        label = XmStringCreateLocalized(buf);
        w = XtVaCreateManagedWidget(
            "remote_label", 
            xmLabelGadgetClass, 	robot_recs[i].main_form, 
            XmNlabelString, 	label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, robot_recs[i].ftp_toggle,
            NULL);
        XmStringFree(label);
      
        robot_recs[i].w_remote_host = XtVaCreateManagedWidget(
            "remote_host", 
            xmTextWidgetClass, robot_recs[i].main_form, 
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, w,
            XmNleftAttachment, XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
      
        hostval = (rc_chain *)rc_table.get("DefaultHost");

        // If the user ran the mission already, use the same host machine name
        if (robot_recs[i].remote_host_str != NULL)
        {
            XmTextSetString(
                robot_recs[i].w_remote_host,
                robot_recs[i].remote_host_str);
        }
        else if (hostval != NULL)
        {
            // If it is the firt time to run the mission, check with rc file
            hostval->first(&str);
            XmTextSetString(robot_recs[i].w_remote_host,str);
        }

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "HServer Name");
        label = XmStringCreateLocalized(buf);
        w = XtVaCreateManagedWidget(
            "hserver-name-w", 
            xmLabelGadgetClass, 	robot_recs[i].main_form, 
            XmNlabelString, 	label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, robot_recs[i].w_remote_host,
            NULL);
        XmStringFree(label);
      
        robot_recs[i].w_hserver_name = XtVaCreateManagedWidget(
            "hserver-name", 
            xmTextWidgetClass, robot_recs[i].main_form, 
            XmNtopAttachment,	XmATTACH_WIDGET,
            XmNtopWidget,		w,
            XmNleftAttachment,	XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
      
        hserver_name_val = (rc_chain *)rc_table.get("DefaultHServerID");

        // If the user ran the mission already, use the same host machine name
        if (robot_recs[i].hserver_name_str != NULL)
        {
            XmTextSetString(
                robot_recs[i].w_hserver_name,
                robot_recs[i].hserver_name_str);
        }

        // If it is the firt time to run the mission, check with rc file
        else if (hserver_name_val != NULL)
        {
            hserver_name_val->first(&str);
            XmTextSetString(robot_recs[i].w_hserver_name,str);
        }
        // Otherwise, use some hard-coded value.
        else
        {
            robot_recs[i].hserver_name_str = strdup(DEFAULT_HSERVER_NAME_STR);
            XmTextSetString(
                robot_recs[i].w_hserver_name,
                robot_recs[i].hserver_name_str);
        }

        memset(buf, 0, sizeof(buf));
        sprintf(buf, "Color");
        label = XmStringCreateLocalized(buf);
        w = XtVaCreateManagedWidget(
            "color_label", 
            xmLabelGadgetClass, robot_recs[i].main_form, 
            XmNlabelString, label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, robot_recs[i].w_hserver_name,
            NULL);
        XmStringFree(label);
      
        robot_recs[i].w_color = XtVaCreateManagedWidget(
            "color", 
            xmTextWidgetClass, robot_recs[i].main_form, 
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, w,
            XmNleftAttachment,	XmATTACH_FORM,
            XmNrightAttachment,	XmATTACH_FORM,
            NULL);
      
        colorval = (rc_chain *)rc_table.get("DefaultColor");

        // If the user ran the mission already, use the same color
        if (robot_recs[i].color_str != NULL)
        {
            XmTextSetString(robot_recs[i].w_color, robot_recs[i].color_str);
        }
        else if (colorval != NULL)
        {
            // If it is the firt time to run the mission, check with rc file
            colorval->first(&str);
            XmTextSetString(robot_recs[i].w_color, str);
        }
    }

    // This widget will create an option for the user whether to ask or
    // not ask overlay upon the execution of the mlab.
    sprintf(buf, "Ask for Overlay");
    label = XmStringCreateLocalized(buf);
    overlay_toggle = XtVaCreateManagedWidget(
        "overlay_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString,	label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, robot_recs[num_robots-1].main_form,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment,	XmATTACH_FORM,
        XmNheight, 20,
        XmNset, ask_default_overlay,
        NULL);
    XmStringFree(label);

    // This widger will create an option for the user whether to 
    // run mlab
    if (SHOW_NO_RUN_OPTION)
    {
        sprintf(buf, "Write to CMDL Only");
        label = XmStringCreateLocalized(buf);
        w_norun_mlab_toggle = XtVaCreateManagedWidget(
            "w_norun_mlab_toggle",
            xmToggleButtonWidgetClass, super_frm,
            XmNlabelString,	label,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, overlay_toggle,
            XmNleftAttachment, XmATTACH_FORM,
            XmNalignment, XmALIGNMENT_BEGINNING,
            XmNrightAttachment, XmATTACH_FORM,
            XmNheight, 20,
            XmNset, norun_mlab,
            NULL);
        XmStringFree(label);
        widgetAbove = w_norun_mlab_toggle;
    }
    else
    {
        widgetAbove = overlay_toggle;
    }

    // This widget will create an option for the user whether to pause
    // before the robot starts upon the execution of the mlab.
    sprintf(buf, "Pause Before Execution");
    label = XmStringCreateLocalized(buf);
    w_pause_toggle = XtVaCreateManagedWidget(
        "w_pause_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString,	label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, widgetAbove,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment,	XmATTACH_FORM,
        XmNheight, 20,
        XmNset, pause_execution,
        NULL);
    XmStringFree(label);

    // This widget will create an option for the user whether to log the
    // robot data.
    sprintf(buf, "Log Robot Data");
    label = XmStringCreateLocalized(buf);
    if (first_time)
    {
        switch(check_bool_rc(rc_table, "LogRobotData")) {

        case RC_STRING_TRUE:
            log_data = true;
            break;

        case RC_STRING_FALSE:
        case RC_STRING_UNSPECIFIED:
        case RC_STRING_INVALID:
            log_data = false;
            break;
        }
    }
    w_logdata_toggle = XtVaCreateManagedWidget(
        "w_logdata_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString,	label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_pause_toggle,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment,	XmATTACH_FORM,
        XmNheight, 20,
        XmNset, log_data,
        NULL);
    XmStringFree(label);

    // This widget will create an option for the user whether to show the
    // current status of the robot.
    sprintf(buf, "Show Robot Status");
    label = XmStringCreateLocalized(buf);

    if (first_time)
    {
        switch (check_bool_rc(rc_table, "ShowRobotStatus")) {

        case RC_STRING_TRUE:
            show_robot_status = true;
            break;

        case RC_STRING_FALSE:
        case RC_STRING_UNSPECIFIED:
        case RC_STRING_INVALID:
            show_robot_status = false;
            break;
        }
    }

    w_show_robot_status_toggle = XtVaCreateManagedWidget(
        "w_show_robot_status_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_logdata_toggle,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment, XmATTACH_FORM,
        XmNheight, 20,
        XmNset, show_robot_status,
        NULL);
    XmStringFree(label);

    // This widget will create an option to disable the feedback from
    // the robot to mlab.
    /*
    sprintf( buf, "Cut-Off Feedback to Mlab");
    label = XmStringCreateLocalized(buf);
    */
    if (first_time) 
    {
        switch (check_bool_rc(rc_table, "CutOffFeedbackToRobot")) {

        case RC_STRING_TRUE:
            stop_robot_feedback = true;
            break;

        case RC_STRING_FALSE:
        case RC_STRING_UNSPECIFIED:
        case RC_STRING_INVALID:
            stop_robot_feedback = false;
            break;
        }
    }
    /*
    w_robot_feedback_toggle = XtVaCreateManagedWidget(
        "w_robot_feedback_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_show_robot_status_toggle,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment,	XmATTACH_FORM,
        XmNheight, 20,
        XmNset, stop_robot_feedback,
        NULL);
    XmStringFree(label);
    */

    // This widget will create an option for the user whether to enable 3D.
    sprintf(buf, "Enable Runtime-CNP");
    label = XmStringCreateLocalized(buf);

    if (gMExp != NULL)
    {
        cnpMode = gMExp->getCNPMode();

        switch (cnpMode) {

        case CNP_MODE_RUNTIME:
        case CNP_MODE_PREMISSION_AND_RUNTIME:
            enable_runtime_cnp = true;
        }

        mexpRCFileName = gMExp->getMExpRCFileName();

        if (mexpRCFileName != MEXP_DEFAULT_RC_FILENAME)
        {
            useCustomMExpRCFile = true;
        }
    }

    w_enable_runtime_cnp_toggle = XtVaCreateManagedWidget(
        "w_enable_runtime_cnp_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_show_robot_status_toggle,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment, XmATTACH_FORM,
        XmNheight, 20,
        XmNset, enable_runtime_cnp,
        NULL);
    XmStringFree(label);

    // This widget will create an option for the user whether to enable 3D.
    sprintf(buf, "Enable 3D");
    label = XmStringCreateLocalized(buf);

    w_enable_3d_toggle = XtVaCreateManagedWidget(
        "w_enable_3d_toggle",
        xmToggleButtonWidgetClass, super_frm,
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_enable_runtime_cnp_toggle,
        XmNleftAttachment, XmATTACH_FORM,
        XmNalignment, XmALIGNMENT_BEGINNING,
        XmNrightAttachment, XmATTACH_FORM,
        XmNheight, 20,
        XmNset, enable_3d,
        NULL);
    XmStringFree(label);

    // This widget is for the user to type in the console-JBox.
    // At first, it uses the one specified in .cfgeditrc.
    /*
    sprintf(buf, "CMDLi Dialog Input File");
    label = XmStringCreateLocalized(buf);
    Widget w = XtVaCreateManagedWidget(
        "cmdli-dialog-input-file-label", 
        xmLabelGadgetClass, super_frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_robot_feedback_toggle,
        NULL);
    XmStringFree(label);

    w_cmdli_name = XtVaCreateManagedWidget(
        "cmdli-name", 
        xmTextWidgetClass, super_frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);
    */
    rc_chain *cmdliNameValue = (rc_chain *)rc_table.get("CMDLiDialogInputFile");

    if (cmdli_name_str != EMPTY_STRING)
    {
        if (cmdli_name_str == STRING_UNKNOWN)
        {
            if (cmdliNameValue != NULL)
            {
                // First time. Check with RC file.
                char *str;
                cmdliNameValue->first(&str);
                //XmTextSetString(w_cmdli_name, str);
                cmdli_name_text = str;
            }
        }
        else
        {
            // If the user ran the mission already, use the same file.
            //XmTextSetString(w_cmdli_name, (char *)(cmdli_name_str.c_str()));
            cmdli_name_text = cmdli_name_str;
        }
    }
    else
    {
        //XmTextSetString(w_cmdli_name, (char *)(cmdli_name_str.c_str()));
        cmdli_name_text = cmdli_name_str;
    }

    // This widget is for the user to type in the console-JBox.
    // At first, it uses the one specified in .cfgeditrc.
    /*
    sprintf( buf, "Console-JBox ID" );
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "console-jbox-id-label", 
        xmLabelGadgetClass, super_frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_overlay_name,
        NULL);
    XmStringFree(label);

    w_console_jbox_id = XtVaCreateManagedWidget(
        "console-jbox-id", 
        xmTextWidgetClass, super_frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);
    */
    rc_chain *conoleJBoxIDValue = (rc_chain *)rc_table.get("ConsoleJBoxID");

    if (console_jbox_id_str != EMPTY_STRING)
    {
        if (console_jbox_id_str == STRING_UNKNOWN)
        {
            if (conoleJBoxIDValue != NULL)
            {
                // First time. Check with RC file.
                char *str;
                conoleJBoxIDValue->first(&str);
                //XmTextSetString(w_console_jbox_id, str);
                console_jbox_id_text = str;
            }
        }
        else
        {
            // If the user ran the mission already, use the same id.
            //XmTextSetString(w_console_jbox_id, (char *)(console_jbox_id_str.c_str()));
            console_jbox_id_text = console_jbox_id_str;
        }
    }
    else
    {
        //XmTextSetString(w_console_jbox_id, (char *)(console_jbox_id_str.c_str()));
        console_jbox_id_text = console_jbox_id_str;
    }

    // This widget is for the user to type in the default overlay.
    // At first, it uses the overlay specified in .cfgeditrc.
    sprintf(buf, "Default Overlay");
    label = XmStringCreateLocalized(buf);
    Widget w = XtVaCreateManagedWidget(
        "default_overlay_label", 
        xmLabelGadgetClass, super_frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_enable_3d_toggle,
        NULL);
    XmStringFree(label);

    w_overlay_name = XtVaCreateManagedWidget(
        "overlay_name", 
        xmTextWidgetClass, super_frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment,	XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    ovlval = (rc_chain *)rc_table.get("DefaultOverlay");

    /*
    if (gWaypointOverlayName != NULL)
    {
        if (overlay_name_str != NULL)
        {
            free(overlay_name_str);
            overlay_name_str = NULL;
        }

        overlay_name_str = strdup(gWaypointOverlayName);
    }
    */

    if ((gOverlayPicked) && (gPickedOverlayName != EMPTY_STRING))
    {
        if (overlay_name_str != NULL)
        {
            free(overlay_name_str);
            overlay_name_str = NULL;
        }

        overlay_name_str = strdup(gPickedOverlayName.c_str());
    }

    // If MissionExpert is running, load the overlay which was defined in the class.
    activateMlabMissionDesign = false;

    if (gMExp != NULL)
    {
        mexpOverlayName = gMExp->getOverlayFileName();

        if (mexpOverlayName != EMPTY_STRING)
        {
            if (overlay_name_str != NULL)
            {
                free(overlay_name_str);
                overlay_name_str = NULL;
            }

            overlay_name_str = strdup(mexpOverlayName.c_str());
            activateMlabMissionDesign = true;
        }
    }

    // If the user ran the mission already, use the same overlay.
    if (overlay_name_str != NULL)
    {
        XmTextSetString(w_overlay_name, overlay_name_str);
    }

    // If it is the firt time to run the mission, check with rc file.
    else if (ovlval != NULL)
    {
        ovlval->first(&str);
        XmTextSetString(w_overlay_name,str);
    }
    else
    {
        // Otherwise, use some hard-coded value.
        overlay_name_str = strdup(DEFAULT_OVERLAY_NAME_STR);
        XmTextSetString(w_overlay_name,overlay_name_str);
    }

    // This widget is for the user to type in the value for
    // the mission expiration time.
    sprintf(buf, "Mission Expiration Time (sec)");
    label = XmStringCreateLocalized(buf);
    w = XtVaCreateManagedWidget(
        "expiration-timer-label", 
        xmLabelGadgetClass, super_frm, 
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w_overlay_name,
        NULL);
    XmStringFree(label);

    w_expiration_timer = XtVaCreateManagedWidget(
        "expiration-timer", 
        xmTextWidgetClass, super_frm, 
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, w,
        XmNleftAttachment, XmATTACH_FORM,
        XmNrightAttachment,	XmATTACH_FORM,
        NULL);

    rc_chain *expirationTimerValue = (rc_chain *)rc_table.get("MissionExpirationTime");

    if (expiration_timer_value != EMPTY_STRING)
    {
        if (expiration_timer_value == STRING_UNKNOWN)
        {
            if (expirationTimerValue != NULL)
            {
                // First time. Check with RC file.
                char *str;
                expirationTimerValue->first(&str);
                XmTextSetString(w_expiration_timer, str);
            }
        }
        else
        {
            // If the user ran the mission already, use the same id.
            XmTextSetString(
                w_expiration_timer,
                (char *)(expiration_timer_value.c_str()));
        }
    }
    else
    {
        XmTextSetString(
            w_expiration_timer,
            (char *)(expiration_timer_value.c_str()));
    }

    XtManageChild(frm);
    XtManageChild(super_frm);
    XtManageChild(overlay_toggle);
    XtManageChild(w_pause_toggle);
    XtManageChild(w_logdata_toggle);
    XtManageChild(w_show_robot_status_toggle);
    //XtManageChild(w_robot_feedback_toggle);
    //XtManageChild(w_cmdli_name);
    //XtManageChild(w_console_jbox_id);
    XtManageChild(w_overlay_name);
    XtManageChild(w_expiration_timer);
    XtManageChild(run_window);
  
    doneRunDialog = false;
    gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_RUN);
}

/*-----------------------------------------------------------------------*/
static void
close_run_window(int update)
{
    if(update)
    {
        // Find the bit that is set.
        for(int i=0; i<num_robots; i++)
        {
            // Simulation by default
            robot_recs[i].choice = 0;

            // Check to see which toggle is set, and that is the choice
            for(int j=0; j<robot_recs[i].num_toggles; j++)
            {
                if(XmToggleButtonGadgetGetState(robot_recs[i].toggles[j]))
                {
                    robot_recs[i].choice = j;
                    break;
                }
            }
        }
    }

    XtUnmanageChild(run_window);

    if (config->currentPageIsFSA())
    {
        // It is the FSA level.
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ASSEMBLAGE);
    }
    else
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
    }

}

int is_online(char *machine)
{
  int sockfd;
  struct hostent *hp;
  struct sockaddr_in host;
  int machine_online = true;

  if ((hp = gethostbyname(machine)) == (struct hostent *) 0) {
    machine_online = false;
    printf("Unable to find host %s.\n", machine);
  }
  if(machine_online) {
    host.sin_family = AF_INET;
    memcpy((char *) &host.sin_addr, (char *) hp->h_addr, hp->h_length);
    host.sin_port = htons(21);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1) {
      machine_online = false;
    }

    if(machine_online && (connect(sockfd, (struct sockaddr *) &host, sizeof(host)) != 0)) {
      machine_online = false;
      printf("Unable to connect to %s.\n", machine);
    } else {
      close(sockfd);
    }
  }
  if(machine_online) {
    printf("Remote host \"%s\" is reachable.\n", machine);
 }
  return machine_online;
}

int send_remote_robot_exe(char *machine, char *file)
{
  FILE *netrcfile;
  char netrcfile_name[256];
  char *home = getenv("HOME");
  sprintf(netrcfile_name, "%s/.netrc", home);

  // Check to see if ".netrc" file is in the user's home directory.
  if((netrcfile = fopen(netrcfile_name, "r")) == NULL)
  {
    fprintf(stderr, "Warning: \"$HOME/.netrc\" does not exist. The FTP program tries to\n");
    fprintf(stderr, "access the remote machine automatically using \".netrc\". If it does\n");
    fprintf(stderr, "not exist, the FTP expects the password to be typed in, and you\n");
    fprintf(stderr, "cannot you cannot run CfgEdit in the background (i.e., don't use \"&\").\n\n");
  }
  else fclose(netrcfile);
  
  FILE *outfile;
  char *ftpscript = "./tmp.ftpscript";
  // Open a file to write a script.
  if((outfile = fopen(ftpscript, "w")) == NULL) {
    printf("Unable to make script!\n");
    return false;
  }

  remotehost_dir_ptr = (rc_chain *)rc_table.get("RemoteHostRobotDir");
  if (remotehost_dir_ptr != NULL)
    remotehost_dir_ptr->first(&remotehost_dir);

  fprintf(outfile, "\ncd %s\nverbose\nhash\nbin\nsend %s\nchmod 777 %s\nbye\n", remotehost_dir, file, file);
  fclose(outfile);
  char ftpcommand[30] = "ftp";
  char *ftparg[4] = { "ftp", "-d", machine, NULL };

  int filedes;
  if((filedes = open(ftpscript, O_RDONLY)) < 0) {
    printf("Unable to open script!\n");
  }

  int newpid, status;
  if((newpid = fork()) == 0) {

    if((dup2(filedes, STDIN_FILENO)) < 0) {
      printf("Unable to set file descriptor!\n");
    }
    printf("Transferring file %s to %s.\n", file, machine);
    printf("Exec %s %s.\n", ftparg[0], ftparg[1]);
    execvp(ftpcommand, ftparg);
    perror("An error occurred in the FTP.\n");
    exit(1);
  } else {
    waitpid(newpid, &status, 0);
    close(filedes);
    unlink(ftpscript);
  }

  fprintf(stderr, "FTP session finished.\n");
  return true;
}

int start_ppp_service(char *machine) {
  int newpid;
  char command[25] = "/usr/sbin/ppp-to";
  char *args[5];
  int p = 0;
  int status;
  int i;

  if((newpid = fork()) == 0) {

    args[p++] = command;
    args[p++] = machine;
    args[p] = NULL;

    printf("Starting PPP service.\n");

    printf("Attempting to execute");
    for(i=0; i<p; i++) {
      printf(" %s", args[i]);
    }
    printf("\n");

    execvp(command, args);
    perror("An error occurred while starting PPP.\n");
  } else {

    waitpid(newpid, &status, 0);
  }

  printf("PPP service started.\n");
  return true;

}

int stop_ppp_service() {
  int newpid;
  char command[25] = "kill";
  char *args[5];
  int p = 0;
  int status;
  int i;

  if((newpid = fork()) == 0) {

    args[p++] = command;
    args[p++] = "`cat /var/run/ppp0.pid`";
    args[p] = NULL;

    printf("Starting PPP service.\n");

    printf("Attempting to execute");
    for(i=0; i<p; i++) {
      printf(" %s", args[i]);
    }
    printf("\n");

    execvp(command, args);
    perror("An error occurreth whilst thou stopeth PPP.\n");
  } else {

    waitpid(newpid, &status, 0);
  }

  printf("PPP service stopped.\n");
  return true;
}

int remote_setup(char *machine, char *file, int *host_ppp)
{
  if(is_online(machine))
  {
    int ret = send_remote_robot_exe(machine, file);
    if(ret == 0)
    {
      printf("Transfer of %s to %s FAILED - exiting...\n",file, machine);
      return 0;
    }
  }
  else if (use_ppp)
  {
    *host_ppp = 1;
    start_ppp_service(machine);
    int ret = send_remote_robot_exe(machine, file);
    if(ret == 0)
    {
      printf("Transfer of %s to %s FAILED - exiting...\n",file, machine);
      return 0;
    }
  }
  else return 0;
  
  return 1;
}

/*-----------------------------------------------------------------------*/
void run_AuRA(void)
{
    FILE *file = NULL, *script = NULL;
    string featureFileString = EMPTY_STRING;
    rc_chain *extraRobotsList = NULL, *extraRobotStartPlaceList = NULL;
    double dx, dy;
    int i, len, num, robot;
    int assignedRobotID;
    int pid, statusp, options, rtn;
    int argc;
    int jboxID;
    char msg[256];
    char buf[2048];
    char *prog = NULL;
    char *argv[32];
    char *cmd_list = NULL;
    char *str = NULL, *tmp = NULL;
    char *scriptFilename = NULL;
    char *robotname = NULL, *unitRobotName = NULL;
    char *firstRobotStartPlace = NULL, *extraRobotStartPlace = NULL;
    char *pos = NULL;
    void *p = NULL;
    bool using_hserver = false, logging_on = false, runMMD = false, useTimer = false;
    bool popupCMDLiDialog = false;
    bool connectJBox = false;
    bool done = false;


    if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
    {
        gMExp->clearMissionSpecWizardProgressBar();
        gMExp->printfMissionSpecWizardInstructionWindow("Running the simulator...");
    }

    // Popup a  window to let the user configure the run
    if(user_privileges & REALROBOTS_PRIV)
    {
        create_run_popup(main_window);
      
        if (((gMExp != NULL) &&
             (gMExp->isRunning()) &&
             (gMExp->missionSpecWizardEnabled())) || 
            gAutomaticExecution)
        {
            done_run();
        }

        while(!doneRunDialog)
        {
            XtAppProcessEvent(app, XtIMAll);
        }

        close_run_window(okRunDialog);

        if(!okRunDialog)
        {
            gEventLogging->cancel("Run-Dialog");
            return;
        }
    }
    else
    {
        // Build the default robot records info array
        for(i = 0; i < num_robots; i++)
        {
            // Allocate memory for the default data
            robot_recs[i].num_toggles = 1;
            robot_recs[i].toggles = NULL;
            // ENDO - gcc 3.4
            //robot_recs[i].names = new (char *)[robot_recs[i].num_toggles];
            robot_recs[i].names = new char *[robot_recs[i].num_toggles];
	  
            // Load the simulation choice
            robot_recs[i].names[0] = RobotName(i+1);
        }
    }

    // Load commands list file
    if(cmd_list_name)
    {
        file = fopen(cmd_list_name, "r");

        if(file == NULL)
        {
            perror("Unable to open file for mlab script ");
        }
        else
        {
            fseek(file, 0, 2);
            len = ftell(file); 
            fseek(file, 0, 0);
            if(len < 0)
            {
                perror("Changes lost: Unable to get length of temporary file!!");
            }
            cmd_list = new char[len+1];
	
            num = fread(cmd_list,1,len,file);
            if(num != len)
                warn_userf("Tried to read %d bytes but only got %d from temporary file",len,num);

            cmd_list[len] = '\0';
        }
    }
  
    // Write a suitable mlab script file
    scriptFilename = (char *)(config->configuration_name());
    script = fopen(scriptFilename, "w");
    if(script == NULL)
    {
        // Error
        perror("Unable to open file for mlab script");
        gEventLogging->status("Unable to open file for mlab script");
        gEventLogging->cancel("Run-Dialog");
        return;
    }
    else
    {
        overlay_name_str = XmTextGetString(w_overlay_name);
      
        if (gMExp)
        {
            string filename = overlay_name_str;
            gMExp->saveOverlayFileName(filename);
        }

        fprintf(script,"MISSION NAME \"%s\"\n", scriptFilename);
        fprintf(script,"OVERLAY \"%s\"\n",overlay_name_str);
      
        // Get some mlab setup from the dialog
        ask_default_overlay = XmToggleButtonGetState(overlay_toggle);
        if (SHOW_NO_RUN_OPTION)
        {
            norun_mlab = XmToggleButtonGetState(w_norun_mlab_toggle);
        }
        pause_execution = XmToggleButtonGetState(w_pause_toggle);
        log_data = XmToggleButtonGetState(w_logdata_toggle);
        show_robot_status = XmToggleButtonGetState(w_show_robot_status_toggle);
        //stop_robot_feedback = XmToggleButtonGetState(w_robot_feedback_toggle);
        enable_runtime_cnp = XmToggleButtonGetState(w_enable_runtime_cnp_toggle);
        enable_3d = XmToggleButtonGetState(w_enable_3d_toggle);
        expiration_timer_value = XmTextGetString(w_expiration_timer);
        //cmdli_name_str = XmTextGetString(w_cmdli_name);
        cmdli_name_str = cmdli_name_text;
        //console_jbox_id_str = XmTextGetString(w_console_jbox_id);
        console_jbox_id_str = console_jbox_id_text;

        if (console_jbox_id_str != EMPTY_STRING)
        {
            jboxID = atoi(console_jbox_id_str.c_str());

            if (isdigit(console_jbox_id_str[0]))
            {
                // Check the first char to see if it is a number.
                // Technically it may fail if 2nd or later char
                // is non-number.
                connectJBox = true;
            }
            else
            {
                fprintf(
                    stderr,
                    "Warning: Invalid Console-JBox ID: %s.\n",
                    console_jbox_id_str.c_str());
                console_jbox_id_str = EMPTY_STRING;
            }
        }

        if (expiration_timer_value != EMPTY_STRING)
        {
            if (isdigit(expiration_timer_value[0]))
            {
                // Check the first char to see if it is a number.
                // Technically it may fail if 2nd or later char
                // is non-number.
                useTimer = true;
            }
            else
            {
                fprintf(
                    stderr,
                    "Warning: Invalid MissionExpirationTimer value: %s.\n",
                    expiration_timer_value.c_str());
                expiration_timer_value = EMPTY_STRING;
            }
        }

        if (cmdli_name_str != EMPTY_STRING)
        {
            popupCMDLiDialog = true;
        }

        for (robot = 0; robot < num_robots; robot++)
        {
            robotname = RobotName(robot+1);

            // Get some robot setup from the dialog
            robot_recs[robot].should_ftp = XmToggleButtonGetState(robot_recs[robot].ftp_toggle);
            robot_recs[robot].remote_host_str = XmTextGetString(robot_recs[robot].w_remote_host);
            robot_recs[robot].color_str = XmTextGetString(robot_recs[robot].w_color);
            robot_recs[robot].hserver_name_str = XmTextGetString(robot_recs[robot].w_hserver_name);

            using_hserver = true;
            if (robot_recs[robot].choice == 0)
            {
                using_hserver = false;
            }

            if(robot_recs[robot].remote_host_str && robot_recs[robot].remote_host_str[0] != '\0')
            {
                robot_recs[robot].host_ppp = 0;

                if(robot_recs[robot].should_ftp &&
                   !remote_setup(robot_recs[robot].remote_host_str,robotname,&(robot_recs[robot].host_ppp)))
                {
                    norun_mlab = true;
                    warn_userf("Unable to setup %s on %s.\n", robotname, robot_recs[robot].remote_host_str);
                }

                remotehost_dir_ptr = (rc_chain *)rc_table.get("RemoteHostRobotDir");
                if (remotehost_dir_ptr != NULL) remotehost_dir_ptr->first(&remotehost_dir);

                fprintf(script,"NEW-ROBOT %s \"%s/%s\" \"%s\" \"%s\" (\n",
                        robotname,
                        remotehost_dir,
                        robotname,
                        robot_recs[robot].color_str,
                        robot_recs[robot].remote_host_str);
            }
            else
            {
                fprintf(script,"NEW-ROBOT %s \"%s\" \"%s\" \"\" (\n",
                        robotname,
                        robotname,
                        robot_recs[robot].color_str);
            }
  
            // Default is false, which is simulation
            if(robot_recs[robot].choice > 0)
            {
                fprintf(script," robot_type = %s,\n", ROBOT_TYPE_NAMES[HSERVER].c_str());
                fprintf(script," run_type = %s,\n", RUN_TYPE_NAMES[REAL].c_str());
                fprintf(script," hserver_name = \"%s\"", robot_recs[robot].hserver_name_str);
            }
            else
            {
                fprintf(script," robot_type = %s,\n", ROBOT_TYPE_NAMES[HOLONOMIC].c_str());
                fprintf(script," run_type = %s", RUN_TYPE_NAMES[SIMULATION].c_str());
            }

            if (gMExp != NULL)
            {
                assignedRobotID = gMExp->assignedRobotID(robot);

                if (assignedRobotID > 0)
                {
                    fprintf(
                        script,
                        ",\n %s = %d",
                        STRING_DESIRED_ROBOT_ID.c_str(),
                        assignedRobotID);
                }
            }

            // Add any extra commands from user rc file
            rc_chain *val;
            if (robot_recs[robot].choice == 0)
                val = (rc_chain *)rc_table.get(simtext);
            else
                val = (rc_chain *)rc_table.get(robot_recs[robot].names[robot_recs[robot].choice]);

            if(val)
            {
                p = val->first(&str); 
                while(p)
                {
                    fprintf(script, ",\n %s", str);
                    p = val->next(&str, p);
                }
            }
            free(robotname);
            //XtFree(host);
            fprintf(script,")\n\n");
        }
      

        extraRobotsList = (rc_chain *)rc_table.get("ExtraRobots");

        if(extraRobotsList)
        {
            p = extraRobotsList->first(&str); 

            while(p)
            {
                // delete any \ characters from the string.
                while((pos = strchr(str,'\\')))
                {
                    // Shift stuff left one char to overwrite the backslash
                    while(*pos)
                    {
                        *pos = *(pos+1);
                        pos++;
                    }
                }
	      
                fprintf(script, "NEW-ROBOT %s\n\n", str);
                p = extraRobotsList->next(&str, p);
            }
        }
      
        extraRobotStartPlaceList = (rc_chain *)rc_table.get("ExtraRobotStartPlace");

        if(extraRobotStartPlaceList)
        {
            extraRobotStartPlaceList->first(&str); 

            if (str != NULL)
            {
                extraRobotStartPlace = strdup(str);
            }
        }
      
        fprintf(script,"UNIT <all>");
        for (robot = 0; robot < num_robots; robot++)
        {
            unitRobotName = RobotName(robot+1);
            fprintf(script," %s", unitRobotName);
            free(unitRobotName);
        }

        // Add any extra robots from the rc file
        if(extraRobotsList)
        {
            if (extraRobotStartPlace != NULL)
            {
                fprintf(script,"\nUNIT <extra>");
            }

            p = extraRobotsList->first(&str); 

            while(p)
            {
                tmp = strdup(str);
                fprintf(script, " %s", strtok(tmp," "));
                free(tmp);
                p = extraRobotsList->next(&str, p);
            }
        }
        fprintf(script,"\n\n");
      
        // Dump any misc settings from user rc file
        rc_chain *miscRobotSettingsList = (rc_chain *)rc_table.get("MiscRobotSettings");
        if(miscRobotSettingsList)
        {
            p = miscRobotSettingsList->first(&str); 

            while(p)
            {
                fprintf(script, "%s\n", str);
                p = miscRobotSettingsList->next(&str, p);
            }
        }
      
        fprintf(script,"\n\n");
        fprintf(script,"COMMAND LIST:\n");

        if(cmd_list != NULL && *cmd_list != '\0')
        {
            fprintf(script,cmd_list);
        }
        else
        {
            firstRobotStartPlace = strdup("StartPlace");

            // Find it in the RC file.
            rc_chain *firstRobotStartPlaceList = (rc_chain *)rc_table.get("FirstRobotStartPlace");

            if(firstRobotStartPlaceList)
            {
                firstRobotStartPlaceList->first(&str); 

                if(str != NULL)
                {
                    free(firstRobotStartPlace);
                    firstRobotStartPlace = strdup(str);
                }
            }
	    
            dx = 0;
            dy = 1.5;
            check_dbl_rc(rc_table, "RobotStart_dx", &dx);
            check_dbl_rc(rc_table, "RobotStart_dy", &dy);

            fprintf(script," 0. UNIT all START %s %f %f\n", firstRobotStartPlace, dx, dy);

            if ((extraRobotsList != NULL) && (extraRobotStartPlace != NULL))
            {
                check_dbl_rc(rc_table, "ExtraRobotStart_dx", &dx);
                check_dbl_rc(rc_table, "ExtraRobotStart_dy", &dy);
                fprintf(script," AND UNIT extra START %s %f %f\n", extraRobotStartPlace, dx, dy);
            }

            if (firstRobotStartPlace != NULL)
            {
                free(firstRobotStartPlace);
                firstRobotStartPlace = NULL;
            }
        }
        fclose(script);

        if (extraRobotStartPlace != NULL)
        {
            free(extraRobotStartPlace);
            extraRobotStartPlace = NULL;
        }
    }
  
    if (norun_mlab)
    {
        return;
    } 

    logging_on = gEventLogging->eventLoggingIsON();
    gEventLogging->pause();

    if (activateMlabMissionDesign)
    {
        featureFileString = gMExp->getFeatureFileString();

        if (featureFileString != EMPTY_STRING)
        {
            runMMD = true;
            gMExp->clearLogfileInfoList();
        }
    }

    // Set executable name
    argc = 0;
    prog = "mlab";
    argv[argc++] = prog;
      
    if (ask_default_overlay)
    {
        argv[argc++] = "-R";
    }
    else
    {
        argv[argc++] = "-r";
    }

    if (runMMD)
    {
        argv[argc++] = "-M";
        argv[argc++] = strdup(featureFileString.c_str());
        argv[argc++] = "-L";

        if (gMExp->missionSpecWizardEnabled())
        {
            argv[argc++] = "-U";
            sprintf(buf, "%d", gMExp->missionSpecWizardType());
            argv[argc++] = strdup(buf);
        }
    }

    if (cnpMode > CNP_MODE_DISABLED)
    {
        argv[argc++] = "-N";
        sprintf(buf,"%s", CNP_MODE_STRING[cnpMode].c_str());
        argv[argc++] = strdup(buf);
    }
    else if (enable_runtime_cnp)
    {
        argv[argc++] = "-N";
        sprintf(buf,"%s", CNP_MODE_STRING[CNP_MODE_RUNTIME].c_str());
        argv[argc++] = strdup(buf);
    }

    if (pause_execution)
    {
        argv[argc++] = "-P";
    }

    // Logging robot data
    if (log_data)
    {
        argv[argc++] = "-L";
    }

    // Enable mlab to show the current status
    if (show_robot_status)
    {
        argv[argc++] = "-S";
    }

    // Cut off the feedback from the robot during the execution.
    if (stop_robot_feedback)
    {
        argv[argc++] = "-f";
        argv[argc++] = "0";
    }

    // Cut off the feedback from the robot during the execution.
    if (popupCMDLiDialog)
    {
        argv[argc++] = "-i";
        argv[argc++] = strdup(cmdli_name_str.c_str());
    }

    // Connect to the console-JBox.
    if (connectJBox)
    {
        argv[argc++] = "-J";
        argv[argc++] = strdup(console_jbox_id_str.c_str());
    }

    // Enable mlab to do 3D
    if (enable_3d)
    {
        argv[argc++] = "-3";
    }

    // Usability log
    if(logging_on)
    {
        argv[argc++] = "-e";
        sprintf(buf,"%s", gEventLogging->getLogfileName());
        argv[argc++] = strdup(buf);
    }
      
    // Set the expiration timer
    if(useTimer)
    {
        argv[argc++] = "-x";
        argv[argc++] = strdup(expiration_timer_value.c_str());
    }

    if(special_rc_file)
    {
        sprintf(buf, "-c");
        argv[argc++] = strdup(buf);
        sprintf(buf,"%s", rc_filename);
        argv[argc++] = strdup(buf);
    }
      
    if (useCustomMExpRCFile)
    {
        sprintf(buf, "-E");
        argv[argc++] = strdup(buf);
        sprintf(buf,"%s", mexpRCFileName.c_str());
        argv[argc++] = strdup(buf);
    }

    argv[argc++] = strdup(scriptFilename);
      
    // Mark end of parm list
    argv[argc] = NULL;
      
    if(verbose)
    {
        fprintf(stderr,"execvp(%s",prog);

        for(i = 0; i < argc; i++)
        {
            fprintf(stderr," %s", argv[i]);
        }

        fprintf(stderr,")\n");
    }
      
    if ((pid = fork()) == 0)
    {
        // In child
        if (execvp(prog, argv))
        {
            sprintf(msg, "Unable to exec %s", prog);
            perror(msg);
        }
      
        // Kill this child
        exit(2);
    }
  
    if (pid == -1)
    {
        gEventLogging->resume(false);
        gEventLogging->status("Unable to fork mlab");
        perror("Unable to fork mlab");
    }
    else
    {
        // wait for mlab to finish
        done = false;
      
        while (!done)
        {
            // Run X
            if (XtAppPending(app))
            {
                XtAppProcessEvent(app, XtIMAll);
            }

            options = WNOHANG | WUNTRACED;
            rtn = waitpid(pid, &statusp, options);
      
            if (rtn == pid)
            {
                done = true; // mlab finished
            }
            else if (rtn == -1)
            {
                done = true; // mlab died with signal
            }

            usleep(EVENT_WAITER_SLEEP_TIME_USEC);
        }
    }
  
    if(logging_on)
    {
        gEventLogging->resume(false);
        gEventLogging->end("Run-Dialog");
    }
}

//-----------------------------------------------------------------------
    void run_UGV(void)
    {
        // Try to run the sausages simulator
        int rtn = system("SAUSAGES_run");

        if (rtn & 0xff != 0)
        {
            warn_userf("The SAUSAGES simulation package is not installed");
            return;
        }
    }

//-----------------------------------------------------------------------
// called to execute robot executables in missionlab
//-----------------------------------------------------------------------
    void run(void)
    {
        gEventLogging->start("Run-Dialog");

        if (config->arch_is_AuRA() ||
            config->arch_is_AuRA_urban() ||
            config->arch_is_AuRA_naval())
        {
            run_AuRA();
        }
        else if(config->arch_is_UGV())
        {
            run_UGV();
        }
        else
        {
            warn_user("Unknown architecture type in RUN");
        }

        first_time = false;

        if ((gMExp->shouldAskFeedback()) && 
            okRunDialog)
        {
            if ((gMExp != NULL) && (gMExp->missionSpecWizardIsUp()))
            {
                gMExp->printfMissionSpecWizardInstructionWindow(
                    "Starting automated mission repair...");
            }

            gMExp->confirmSuccessfulMission();
        }
    }


///////////////////////////////////////////////////////////////////////
// $Log: run.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.8  2007/09/18 22:36:12  endo
// Mission time windows in ICARUS objectives can be now updated based on the environment message.
//
// Revision 1.7  2007/09/07 23:10:03  endo
// The overlay name is now remembered when the coordinates are picked from an overlay.
//
// Revision 1.6  2007/06/28 03:54:20  endo
// For 06/28/2007 demo.
//
// Revision 1.5  2007/05/15 18:50:01  endo
// BAMS Wizard implemented.
//
// Revision 1.4  2007/03/09 22:26:08  endo
// The bug that prevents Run dialog to be shown even if ICARUS Wizard is not running is fixed.
//
// Revision 1.3  2006/10/24 22:03:13  endo
// Last minute change for October demo.
//
// Revision 1.2  2006/09/13 19:03:49  endo
// ICARUS Wizard implemented.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.14  2006/07/11 17:14:04  endo
// ConsoleJBoxID added.
//
// Revision 1.13  2006/07/11 16:28:44  endo
// CMDLi Panel merged from MARS 2020.
//
// Revision 1.12  2006/07/11 06:41:50  endo
// Cut-Off Feedback functionality merged from MARS 2020.
//
// Revision 1.11  2006/05/15 01:23:28  endo
// gcc-3.4 upgrade
//
// Revision 1.10  2006/05/02 04:21:03  endo
// TrackTask improved for the experiment.
//
// Revision 1.9  2006/02/19 17:52:51  endo
// Experiment related modifications
//
// Revision 1.8  2006/02/14 02:27:18  endo
// gAutomaticExecution flag and its capability added.
//
// Revision 1.7  2006/01/30 02:47:28  endo
// AO-FNC CBR-CNP Type-I check-in.
//
// Revision 1.6  2005/10/21 00:30:16  endo
// Event Log for GOMS Test.
//
// Revision 1.5  2005/07/31 03:39:43  endo
// Robot ID specified by CNP can be now recognized by mlab during execution.
//
// Revision 1.4  2005/07/27 20:36:39  endo
// 3D visualization improved.
//
// Revision 1.3  2005/05/18 21:14:43  endo
// AuRA.naval added.
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:34  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.48  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.47  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.46  2001/09/14 01:34:37  ebeowulf
// Added scroll bars to the run window
//
// Revision 1.45  2000/09/19 09:51:02  endo
// Run dialog re-organized.
//
// Revision 1.44  2000/07/26 20:07:57  endo
// The capability of MissionLab being able to FTP robot executable
// was broken, and the mechanism of executing the robot executable
// on a remote machine was hard coded to use /home/demo. This
// modification fixes those.
//
// Revision 1.43  2000/07/02 00:58:43  conrad
// rechecked in after goahead
//
// Revision 1.41  2000/06/27 21:58:01  conrad
// Added color,robot name,and do_not_run_mlab
//
// Revision 1.40  2000/02/25 21:59:24  endo
// Some widget bug (small height and wrong attachement)
// fixed. "Ask for Overlay" and "Pause" was changed to ask
// once instead of asking for each robot.
//
// Revision 1.39  2000/02/14 05:42:52  conrad
// Changed run dialog box to include default overlay field,
// pause_before_execution box, ask_for_overlay box.
// On selection of robot type the overlay and host field is changed to settings
// in .cfgeditrc under each robot heading.
//
// Revision 1.37  1999/09/03 19:56:33  endo
// *** empty log message ***
//
// Revision 1.36  1999/09/03 19:54:59  endo
// run_AuRA(), run_UGV() created to organize run().
//
// Revision 1.35  1999/07/15 18:17:49  dj
// made the run dialog box prettier
//
// Revision 1.34  1999/07/15 18:14:08  dj
// changed execution of ppp from "pppd" executable to "ppp-to" script
//
// Revision 1.33  1999/07/03 22:01:00  dj
// added ability for user to specify FTP, and moved FTP and PPP setup to cfgedit
//
// Revision 1.32  1999/06/29 03:08:13  mjcramer
// Added remote machine name to run window
//
// Revision 1.31  1999/03/05 21:18:07  endo
// *** empty log message ***
//
// Revision 1.30  1999/02/16  17:42:02  endo
// Changed: "Empty._ovl" --> "Empty.ovl"
//
// Revision 1.29  1999/02/16  16:33:51  endo
// Changed back, so that "Empty._ovl" is the default overlay,
// and runs mlab with "-R" option: pop-up menu asks user for
// the selection of overlays.
//
// Revision 1.28  1998/11/03  03:37:53  endo
// *** empty log message ***
//
// Revision 1.27  1998/11/03  03:18:26  endo
// The "-R" option for mlab was (temporary) replaced with "-r"
// because when  X-window is using twm, mlab fails to display
// pop-up menu. After this pop-up menu problem was solved,
// the "-R" option may be replaced again, so that users can
// have a option to choose which overlay to use. Currently,
// only "World_B.ovl" can be used.
//
// Revision 1.26  1998/11/03  00:41:36  endo
// The generated cmdl file uses "World_B.ovl" as the default
// overlay instead of the previous "empty._ovl".
//
// Revision 1.25  1998/11/03  00:34:05  endo
// Not specifying the color of NEW-ROBOT was problem of the mlab
// not being able to run the robot. Thus, color "blue" was added to
// the cmdl generated by this cfgedit.
// 
//
// Revision 1.24  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.23  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.23  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.22  1996/06/07 18:32:00  doug
// fixed problem running robots in simulation
//
// Revision 1.21  1996/06/04  21:31:54  doug
// fixed run dialog box so asks about specific robots instead of tty's
// ./
//
// Revision 1.20  1996/06/04  20:12:56  doug
// *** empty log message ***
//
// Revision 1.19  1996/05/28  15:14:38  doug
// changed to also run the SAUSAGES simulator if it is installed
//
// Revision 1.18  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.17  1996/03/08  20:43:47  doug
// *** empty log message ***
//
// Revision 1.16  1996/03/08  00:46:56  doug
// *** empty log message ***
//
// Revision 1.15  1996/03/06  23:39:17  doug
// *** empty log message ***
//
// Revision 1.14  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.13  1996/03/04  22:52:12  doug
// *** empty log message ***
//
// Revision 1.12  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.11  1996/02/29  01:48:49  doug
// *** empty log message ***
//
// Revision 1.10  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.9  1996/02/20  22:52:24  doug
// adding EditParms
//
// Revision 1.8  1996/02/07  17:51:51  doug
// *** empty log message ***
//
// Revision 1.7  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.6  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.5  1995/11/14  15:46:12  doug
// fixed so compiles under linux
//
// Revision 1.4  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.3  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.2  1995/09/01  16:14:56  doug
// Able to run double wander config
//
// Revision 1.1  1995/08/24  19:14:45  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////
