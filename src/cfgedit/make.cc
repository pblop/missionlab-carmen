/**********************************************************************
 **                                                                  **
 **                              make.cc                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: make.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <memory.h>
#include <malloc.h>
#include <stdarg.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/RowColumn.h>
#include <Xm/MainW.h>
#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/DrawingA.h>
#include <Xm/ArrowBG.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/PushB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>

#include "make.h"
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
#include "renumber_robots.h"
#include "cfgedit_common.h"
#include "EventLogging.h"
#include "mission_expert.h"
#include "assistantDialog.h"

extern Widget main_window;

extern int sys_nerr;

static char name_prefix[256];

Widget logWindow[NUM_MAKE_WINDOWS];
Widget textField[NUM_MAKE_WINDOWS];
Widget detail_button=NULL;
Widget hide_detail=NULL;
Widget cancelButton[NUM_MAKE_WINDOWS];
XmTextPosition wpr_position[NUM_MAKE_WINDOWS];

int dot_count=0;
bool create_detail=false;
bool compile_finish=false;
bool abort_make = false;
bool detailedLogWindowIsUp = false;

// These globals are used in the run command
bool have_fresh_make = false;
char *robot_names[MAX_ROBOTS];
char *cmd_list_name;

void createSimpleMakeWindow(Widget parent);
void createDetailedMakeWindow(Widget parent);

/**********************************************************************
 **                                                                  **
 **                       constants and macros                       **
 **                                                                  **
 **********************************************************************/

#define MAXARGS         20

#define RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

const int MAX_MAKE_DOTCOUNT = 30;
const int NUM_SIMPLE_MAKE_ROWS = 25;
const int NUM_SIMPLE_MAKE_COLUMNS = 60;
const int NUM_DETAILED_MAKE_ROWS = 40;
const int NUM_DETAILED_MAKE_COLUMNS = 130;
const int SIMPLE_WINDOW_WIDTH = 475;
const int SIMPLE_WINDOW_HEIGHT = 500;
const int DETAILED_WINDOW_WIDTH = 750;
const int DETAILED_WINDOW_HEIGHT = 550;
const long SLEEP_TIME_NANOSEC = 100000000;

/*-----------------------------------------------------------------------*/
static void abort_make_cb_compile(Widget w, void *data, void* call_data)
{
   abort_make = true;
}

static void abort_make_cb_detail(Widget w, void *data, void* call_data)
{
    abort_make = true;
    XtUnmanageChild(logWindow[DETAILED_MAKE_WINDOW]);
    XtManageChild(logWindow[SIMPLE_MAKE_WINDOW]);
}

static void ok_make_cb_compile(Widget w, void *data, void* call_data)
{
    abort_make = true;
    XtUnmanageChild(logWindow[DETAILED_MAKE_WINDOW]);
    XtUnmanageChild(logWindow[SIMPLE_MAKE_WINDOW]);
}

/*-----------------------------------------------------------------------*/
/*for the Detail button of logging_window1*/
void detailAll(Widget w, void *data, void* call_data)
{
  if (!create_detail)
  {
      createDetailedMakeWindow(main_window);
      create_detail=true;
  }
  else
  {
      XtManageChild(textField[DETAILED_MAKE_WINDOW]);
      XtManageChild(logWindow[DETAILED_MAKE_WINDOW]);
      detailedLogWindowIsUp = true;
  }
}

/*-----------------------------------------------------------------------*/
/*for the Hide Detail button of logging_window2*/
void hideDetail(Widget w, void *data, void* call_data)
{
    XtManageChild(logWindow[SIMPLE_MAKE_WINDOW]);
    XtUnmanageChild(logWindow[DETAILED_MAKE_WINDOW]);
    detailedLogWindowIsUp = false;
}

/*-----------------------------------------------------------------------*/
void createSimpleMakeWindow(Widget parent)
{
   Arg args[MAXARGS];
   Cardinal argcount = 0;
   XmString title = XSTRING("Make Log");

   // Create the simple make window
   argcount = 0;
   XtSetArg(args[argcount], XmNdialogTitle, title); argcount++;
   XtSetArg(args[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); argcount++;
   XtSetArg(args[argcount], XmNautoUnmanage,false); argcount++;

   logWindow[SIMPLE_MAKE_WINDOW] = XmCreateMessageDialog(
       parent,
       "logWindow[SIMPLE_MAKE_WINDOW]",
       args,
       argcount);

   XmStringFree(title);

   XtUnmanageChild(XmMessageBoxGetChild(logWindow[SIMPLE_MAKE_WINDOW], XmDIALOG_HELP_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(logWindow[SIMPLE_MAKE_WINDOW], XmDIALOG_OK_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(logWindow[SIMPLE_MAKE_WINDOW], XmDIALOG_CANCEL_BUTTON));

   XmString cancel=XmStringCreateLocalized("Cancel");

   cancelButton[SIMPLE_MAKE_WINDOW] = XtVaCreateManagedWidget
       ("",
	xmPushButtonWidgetClass, logWindow[SIMPLE_MAKE_WINDOW],
	XmNlabelType, XmSTRING,
	XmNlabelString, cancel,
	NULL);

   XmStringFree(cancel);

   XtAddCallback(
       cancelButton[SIMPLE_MAKE_WINDOW],
       XmNactivateCallback,
       (XtCallbackProc)
       abort_make_cb_compile,
       NULL);

   XmString detail = XmStringCreateLocalized("Detail");

   detail_button = XtVaCreateManagedWidget(
       "",
       xmPushButtonWidgetClass, logWindow[SIMPLE_MAKE_WINDOW],
       XmNlabelType, XmSTRING,
       XmNlabelString, detail,
       NULL);

   XmStringFree(detail);

   XtAddCallback(
       detail_button,
       XmNactivateCallback,
       (XtCallbackProc) detailAll,
       NULL);

   // Create the text window
   argcount = 0;
   XtSetArg(args[argcount], XmNrows, NUM_SIMPLE_MAKE_ROWS); argcount++;
   XtSetArg(args[argcount], XmNcolumns, NUM_SIMPLE_MAKE_COLUMNS); argcount++;
   XtSetArg(args[argcount], XmNeditable, False); argcount++;
   XtSetArg(args[argcount], XmNeditMode, XmMULTI_LINE_EDIT); argcount++;
   XtSetArg(args[argcount], XmNwordWrap, False); argcount++;
   XtSetArg(args[argcount], XmNscrollHorizontal, True); argcount++;
   XtSetArg(args[argcount], XmNblinkRate, 0); argcount++;
   XtSetArg(args[argcount], XmNautoShowCursorPosition, True); argcount++;
   XtSetArg(args[argcount], XmNcursorPositionVisible, False); argcount++;

   textField[SIMPLE_MAKE_WINDOW] = XmCreateScrolledText(
       logWindow[SIMPLE_MAKE_WINDOW],
       "textField[SIMPLE_MAKE_WINDOW]",
       args, argcount);

   XtVaSetValues(
       logWindow[SIMPLE_MAKE_WINDOW],
       XmNwidth, SIMPLE_WINDOW_WIDTH,
       XmNheight, SIMPLE_WINDOW_HEIGHT,
       NULL);

   if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
   {
       // MissionSpecWizard in up. Do not manage these windows.
       gMExp->printfMissionSpecWizardInstructionWindow("Compiling the mission. Please wait...");
       gMExp->updateMissionSpecWizardProgressBar(0.0);
   }
   else
   {
       XtManageChild(textField[SIMPLE_MAKE_WINDOW]);
       XtManageChild(logWindow[SIMPLE_MAKE_WINDOW]);
   }

   // Reset cursor
   wpr_position[SIMPLE_MAKE_WINDOW] = 0;
   gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_COMPILE);
}

void createDetailedMakeWindow(Widget parent)
{
    Arg args[MAXARGS];
    Cardinal argcount = 0;
    XmString title = XSTRING("Make Log");

    // Create the dialog
    argcount = 0;
    XtSetArg(args[argcount], XmNdialogTitle, title); argcount++;
    XtSetArg(args[argcount], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); argcount++;
    XtSetArg(args[argcount],XmNautoUnmanage,false);

    logWindow[DETAILED_MAKE_WINDOW] = XmCreateMessageDialog(
	parent,
	"logWindow[DETAILED_MAKE_WINDOW]",
	args,
	argcount);

    XmStringFree(title);

    XtUnmanageChild(XmMessageBoxGetChild(logWindow[DETAILED_MAKE_WINDOW], XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(logWindow[DETAILED_MAKE_WINDOW], XmDIALOG_OK_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(logWindow[DETAILED_MAKE_WINDOW], XmDIALOG_CANCEL_BUTTON));

    XmString cancel=XmStringCreateLocalized("Cancel");
    cancelButton[DETAILED_MAKE_WINDOW] = XtVaCreateManagedWidget(
	"",
	xmPushButtonWidgetClass, logWindow[DETAILED_MAKE_WINDOW],
	XmNlabelType, XmSTRING,
	XmNlabelString, cancel,
	NULL);

    XtAddCallback(
	cancelButton[DETAILED_MAKE_WINDOW],
	XmNactivateCallback,
	(XtCallbackProc)abort_make_cb_detail,
	NULL);

    XmString detail = XmStringCreateLocalized("Hide Detail");

    hide_detail = XtVaCreateManagedWidget(
	"",
	xmPushButtonWidgetClass, logWindow[DETAILED_MAKE_WINDOW],
	XmNlabelType, XmSTRING,
	XmNlabelString, detail,
	NULL);

    XtAddCallback(
	hide_detail,
	XmNactivateCallback,
	(XtCallbackProc)hideDetail,
	NULL);

    XmStringFree(detail);

    // Create the text window
    argcount = 0;
    XtSetArg(args[argcount], XmNrows, NUM_DETAILED_MAKE_ROWS); argcount++;
    XtSetArg(args[argcount], XmNcolumns, NUM_DETAILED_MAKE_COLUMNS); argcount++;
    XtSetArg(args[argcount], XmNeditable, False); argcount++;
    XtSetArg(args[argcount], XmNeditMode, XmMULTI_LINE_EDIT); argcount++;
    XtSetArg(args[argcount], XmNwordWrap, False); argcount++;
    XtSetArg(args[argcount], XmNscrollHorizontal, True); argcount++;
    XtSetArg(args[argcount], XmNblinkRate, 0); argcount++;
    XtSetArg(args[argcount], XmNautoShowCursorPosition, True); argcount++;
    XtSetArg(args[argcount], XmNcursorPositionVisible, False); argcount++;

    textField[DETAILED_MAKE_WINDOW] = XmCreateScrolledText(
	logWindow[DETAILED_MAKE_WINDOW],
	"textField[DETAILED_MAKE_WINDOW]",
	args,
	argcount);

    XtVaSetValues(
	logWindow[DETAILED_MAKE_WINDOW],
	XmNwidth, DETAILED_WINDOW_WIDTH,
	XmNheight, DETAILED_WINDOW_HEIGHT,
	NULL);

    // Reset cursor
    wpr_position[DETAILED_MAKE_WINDOW] = 0;
    create_detail=true;
}


/*-----------------------------------------------------------------------*/
static void
add_to_log(int indicator,const char *msg)
{
    string logMsg = msg;

    switch (indicator) {

    case SIMPLE_MAKE_WINDOW:
	if (dot_count == MAX_MAKE_DOTCOUNT)
	{
	    logMsg.insert(0, "\n");
	    dot_count = 0;
	}
	dot_count++;
	XmTextInsert(textField[SIMPLE_MAKE_WINDOW], wpr_position[SIMPLE_MAKE_WINDOW], (char *)logMsg.c_str());
	wpr_position[SIMPLE_MAKE_WINDOW] += logMsg.size();
	XtVaSetValues(textField[SIMPLE_MAKE_WINDOW], XmNcursorPosition, wpr_position[SIMPLE_MAKE_WINDOW], NULL);
	XmTextShowPosition(textField[SIMPLE_MAKE_WINDOW], wpr_position[SIMPLE_MAKE_WINDOW]);
	break;

    case DETAILED_MAKE_WINDOW:
	XmTextInsert(textField[DETAILED_MAKE_WINDOW], wpr_position[DETAILED_MAKE_WINDOW], (char *)logMsg.c_str());
	wpr_position[DETAILED_MAKE_WINDOW] += logMsg.size();
	XtVaSetValues(textField[DETAILED_MAKE_WINDOW], XmNcursorPosition, wpr_position[DETAILED_MAKE_WINDOW], NULL);
	XmTextShowPosition(textField[DETAILED_MAKE_WINDOW], wpr_position[DETAILED_MAKE_WINDOW]);
	break;
    }
}


/*-----------------------------------------------------------------------*/
/*
static void
add_to_log_cr(int indicator,const char *msg)
{
   add_to_log(indicator,msg);
   add_to_log(indicator,"\n");
}
*/
/*-----------------------------------------------------------------------*/
static void
add_to_logf(int indicator,const char *fmt,...)
{
   va_list args;
   char msg[2048];

   va_start(args, fmt);
   vsprintf(msg, fmt, args);
   va_end(args);
   add_to_log(indicator,msg);
}


/*-----------------------------------------------------------------------*/
// make: called to generate robot executables
bool make(void)
{
    int i;

    gEventLogging->start("Make");
    if(!verify(true, false))
    {
        gEventLogging->status("Verify Error");
        gEventLogging->cancel("Make");
        return false;
    }

    if (gMExp->isEnabled())
    {
        if (gMExp->shouldSaveMissionToCBRLibrary())
        {
            gMExp->saveSelectionToCBRLibrary(config->root());
        }
    }

    // Renumber the robots to get consistent names
    renumber_robots();

    cmd_list_name = NULL;
    char *cdl_filename = NULL;
    bool had_error = false;

    abort_make = false;

    createSimpleMakeWindow(main_window);
    createDetailedMakeWindow(main_window);

    // Build a unique prefix for tmp files
    sprintf(name_prefix, "/tmp/CfgEdit%d", getpid());
    cdl_filename = strdupcat(strdup(name_prefix), ".cdl");

    add_to_log(SIMPLE_MAKE_WINDOW,"Writing configuration ");
    add_to_logf(DETAILED_MAKE_WINDOW,"Making configuration %s:\n",
                config->configuration_name());

    // Write the file out as CDL
    add_to_log(SIMPLE_MAKE_WINDOW," .");
    add_to_logf(DETAILED_MAKE_WINDOW,"Writing configuration to %s\n", cdl_filename);
    int good = save_workspace(config->root(), cdl_filename, debug_save, false);

    if (!good)
    {
        add_to_log(SIMPLE_MAKE_WINDOW," .");
        add_to_log(DETAILED_MAKE_WINDOW,"Make aborted: Error writing tmp file\n");
        had_error = true;
    }

    // Create a communications pipe to talk to the child processes
    int pid;
    int fd[2];

    if (!had_error)
    {
        if (pipe(fd) < 0)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," .");
            /*
              add_to_log(DETAILED_MAKE_WINDOW,"Unable to create communication pipe:");
              if (errno < sys_nerr)
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
              }
              else
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
              }
            */
            add_to_log(DETAILED_MAKE_WINDOW,"Unable to create communication pipe.\n");
            had_error = true;
        }

        if (fcntl(fd[0], F_SETFL, FNDELAY) == -1)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," .");
            /*
              add_to_log(DETAILED_MAKE_WINDOW,"Unable to set communication pipe to non-blocking:");
              if (errno < sys_nerr)
              {
              add_to_log_cr(SIMPLE_MAKE_WINDOW,sys_errlist[errno]);
              add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
              }
              else
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
              }
            */
            add_to_log(DETAILED_MAKE_WINDOW,"Unable to set communication pipe to non-blocking.\n");
            had_error = true;
        }
    }

    if (!had_error)
    {
        add_to_log(SIMPLE_MAKE_WINDOW," Done");
    }
    else
    {
        add_to_log(SIMPLE_MAKE_WINDOW," Failed");
    }

    // Run CDL
    char *robot_cnl_names[MAX_ROBOTS];
    char *robot_cc_names[MAX_ROBOTS];
    int num_robots = 0;

    for(int a=0; a<MAX_ROBOTS; a++)
    {
        robot_cnl_names[a] = NULL;
        robot_cc_names[a] = NULL;
        robot_names[a] = NULL;
    }

    add_to_log(SIMPLE_MAKE_WINDOW,"\n\nPhase I");
    add_to_log(DETAILED_MAKE_WINDOW,"\n\nPhase I: CDL Compilation:");
    dot_count=0;

    if (!had_error)
    {
        if ((pid = fork()) == 0)
        {
            // Redirect stderr back to parent
            if (dup2(fd[1], STDERR_FILENO) == -1)
            {
                perror("CDL: unable to redirect stderr back to caller");
            }
            else if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
            {
                perror("CDL: Unable to config communication pipe");
                exit(1);
            }
            else if (fcntl(fd[1], F_SETFD, 0) == -1)
            {
                perror("CDL: Unable to config communication pipe");
                exit(1);
            }
            else
            {
                char *prog;
                char *argv[20];
                int p = 0;

                /* in child */

                // Set executable name
                prog = "cdl";
                argv[p++] = prog;

                argv[p++] = "-p";

                if( special_rc_file )
                {
                    char buf[2048];
                    sprintf(buf,"-c%s", rc_filename);
                    argv[p++] = strdup(buf);
                }

                argv[p++] = "-o";
                argv[p++] = name_prefix;

                // Load the parm list
                argv[p++] = cdl_filename;

                // Mark end of parm list
                argv[p] = NULL;

                // Show the user
                char *cmd = strdup(prog);

                for (i = 1; i < p; i++)
                {
                    cmd = strdupcat(cmd, " ");
                    cmd = strdupcat(cmd, argv[i]);
                }
                fprintf(stderr, "\n%s\n", cmd);
                free(cmd);

                if (execvp(prog, argv) == -1)
                {
                    char msg[256];

                    sprintf(msg, "Unable to exec %s", prog);
                    perror(msg);
                }
            }

            close(fd[1]);

            // Kill this child
            exit(2);
        }

        if (pid == -1)
        {

            add_to_log(SIMPLE_MAKE_WINDOW, " .");
            /*
              add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork:");
              if (errno < sys_nerr)
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
              }
              else
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
              }
            */
            add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork.\n");
            had_error = true;
        }
        else
        {
            // wait for compile to finish
            char buf[256];
            char queue[2048];
            int pos = 0;

            bool done = false;
            bool empty_pipe = false;
            bool cdl_to_cnl = false;
            while (!done || !empty_pipe )
            {
                if (!cdl_to_cnl)
                {
                    // Try to read from the com pipe
                    int size = read(fd[0], buf, sizeof(buf));

                    if (size > 0)
                    {
                        for (i = 0; i < size; i++)
                        {
                            queue[pos] = buf[i]; pos++;

                            if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                            {
                                queue[pos] = '\0';

                                char buf[2048];

                                if (sscanf(queue, "Writing robot %s", buf) == 1)
                                {
                                    robot_cnl_names[num_robots++] = strdup(buf);
                                }
                                else if (sscanf(queue, "Writing command list to %s", buf) == 1)
                                {
                                    cmd_list_name = strdup(buf);
                                }
                                else if (sscanf(queue, "CDL compiler has generated %s CNL file(s).", buf) == 1)
                                {
                                    // CDL finished successfully. It is safe to move on to CNL.
                                    cdl_to_cnl = true;
                                    empty_pipe = true;
                                }
                                else if (sscanf(queue, "Error%s", buf) == 1)
                                {
                                    // CDL finished unsuccessfully. Compilation will be aborted at
                                    // a later routine.
                                    cdl_to_cnl = true;
                                    empty_pipe = true;
                                }


                                add_to_log(SIMPLE_MAKE_WINDOW," .");
                                //add_to_log(SIMPLE_MAKE_WINDOW,queue);
                                add_to_log(DETAILED_MAKE_WINDOW,queue);
                                pos = 0;
                            }
                        }
                        if (!cdl_to_cnl) empty_pipe = false;
                    }
                    else if( size < 0 )
                    {
                        empty_pipe = true;
                    }
                }

                // Run X
                if (XtAppPending(app))
                {
                    XtAppProcessEvent(app, XtIMAll);
                }

                if (abort_make)
                {
                    had_error = true;
                    add_to_log(SIMPLE_MAKE_WINDOW," .");
                    add_to_log(DETAILED_MAKE_WINDOW,"Compilation canceled by user\n");
                    kill(pid, SIGTERM);
                    done = true;
                    empty_pipe = true;
                    continue;
                }

                if (cdl_to_cnl)
                {
                    int statusp;
                    int options =  WNOHANG | WUNTRACED;
                    int rtn = waitpid(pid, &statusp, options);

                    if (rtn == pid)
                    {
                        done = true;

                        if (WIFEXITED(statusp))
                        {
                            int exit_status = WEXITSTATUS(statusp);

                            if (exit_status != 0)
                            {
                                add_to_log(SIMPLE_MAKE_WINDOW," .");
                                add_to_logf(DETAILED_MAKE_WINDOW,"CDL compiler exited with status %d\n",
                                            exit_status);
                                had_error = true;
                            }
                        }
                        else if (WIFSIGNALED(statusp))
                        {
                            // Error
                            add_to_log(SIMPLE_MAKE_WINDOW," .");
                            add_to_logf(DETAILED_MAKE_WINDOW,"CDL compiler died with signal: %d\n",
                                        WTERMSIG(statusp));
                            had_error = true;
                        }
                    }
                    else if (rtn == -1)
                    {
                        // Error
                        add_to_log(SIMPLE_MAKE_WINDOW,"Received signal from CDL compiler");
                        /*
                          add_to_log(DETAILED_MAKE_WINDOW,"Received signal from CDL compiler");
                          if (errno < sys_nerr)
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
                          }
                          else
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
                          }
                        */
                        add_to_log(DETAILED_MAKE_WINDOW,"Received signal from CDL compiler.\n");
                        done = true;
                        had_error = true;

                        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
                        {
                            gMExp->printfMissionSpecWizardInstructionWindow("Error: Compilation failed.");
                            gMExp->printfMissionSpecWizardDataWindow("Received signal from CDL compiler.");
                        }
                    }
                }
            }

            // Dump the last message if waiting
            if (pos > 0)
            {
                queue[pos] = '\n'; pos++;
                queue[pos] = '\0';
                add_to_log(SIMPLE_MAKE_WINDOW," .");
                add_to_log(DETAILED_MAKE_WINDOW,queue);

            }
        }
    } // Finished running CDL

    add_to_log(SIMPLE_MAKE_WINDOW," Done");

    if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()) && (!had_error))
    {
        gMExp->updateMissionSpecWizardProgressBar(1.0/(double)(num_robots*2+1));
    }

    // For each robot file
    for (int robot = 0; robot < num_robots; robot++)
    {
        add_to_logf(SIMPLE_MAKE_WINDOW, "\n\n----------------- Creating Robot%d ------------------\n",robot+1);
        add_to_logf(DETAILED_MAKE_WINDOW, "\n\n------------------------------------ Creating Robot%d ------------------------------------\n",
                    robot+1);
        add_to_log(SIMPLE_MAKE_WINDOW, "\nPhase II");
        add_to_log(DETAILED_MAKE_WINDOW, "\nPhase II: CNL Compilation:");
        dot_count=0;

        if( had_error )
        {
            break;
        }

        // Run CNL compiler
        if ((pid = fork()) == 0)
        {
            // Redirect stderr back to parent
            if (dup2(fd[1], STDERR_FILENO) == -1)
            {
                perror("CNL: unable to redirect stderr back to caller");
            }
            else if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
            {
                perror("CNL: Unable to config communication pipe");
                exit(1);
            }
            else if (fcntl(fd[1], F_SETFD, 0) == -1)
            {
                perror("CNL: Unable to config communication pipe");
                exit(1);
            }
            else
            {
                char *prog;
                char *argv[20];
                int p = 0;

                /* in child */

                // Set executable name
                prog = "cnl";
                argv[p++] = prog;

                // Load the parm list
                argv[p++] = robot_cnl_names[robot];

                // Extract the compile options
                rc_chain *val = (rc_chain *)rc_table.get(CFLAGS_NAME);
                if( val )
                {
                    char *str;
                    void *pos = val->first(&str);
                    while( pos )
                    {
                        argv[p++] = strdup(str);
                        pos = val->next(&str, pos);
                    }
                }

                // Mark end of parm list
                argv[p] = NULL;

                // Show the user
                char *cmd = strdup(prog);

                for (i = 1; i < p; i++)
                {
                    cmd = strdupcat(cmd, " ");
                    cmd = strdupcat(cmd, argv[i]);
                }
                fprintf(stderr, "\n%s\n", cmd);
                free(cmd);

                if (execvp(prog, argv) == -1)
                {
                    /*
                      fprintf(stderr, "Unable to exec the cnl compiler: ");
                      if (errno < sys_nerr)
                      {
                      fprintf(stderr, "%s\n", sys_errlist[errno]);
                      }
                      else
                      {
                      fprintf(stderr, "Unknown error number: %d\n", errno);
                      }
                    */
                    fprintf(stderr, "Unable to exec the cnl compiler.\n");
                }
            }

            close(fd[1]);

            // Kill this child
            exit(2);
        }

        if (pid == -1)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," .");
            /*
              add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork:");
              if (errno < sys_nerr)
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
              }
              else
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
              }
            */
            add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork.\n");
            had_error = true;
            break;
        }
        else
        {
            // wait for compile to finish
            char buf[256];
            char queue[2048];
            int pos = 0;

            bool done = false;
            bool empty_pipe = false;
            while (!done || !empty_pipe)
            {
                // Try to read from the com pipe
                int size = read(fd[0], buf, sizeof(buf));

                if (size > 0)
                {
                    for (i = 0; i < size; i++)
                    {
                        queue[pos] = buf[i]; pos++;

                        if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                        {
                            queue[pos] = '\0';
                            add_to_log(SIMPLE_MAKE_WINDOW," .");
                            add_to_log(DETAILED_MAKE_WINDOW,queue);
                            pos = 0;
                        }
                    }
                    empty_pipe = false;
                }
                else if( size < 0 )
                {
                    empty_pipe = true;
                }

                // Run X
                if (XtAppPending(app))
                    XtAppProcessEvent(app, XtIMAll);

                if (abort_make)
                {
                    had_error = true;
                    add_to_log(SIMPLE_MAKE_WINDOW," .");
                    add_to_log(DETAILED_MAKE_WINDOW,"Compilation canceled by user\n");
                    kill(pid, SIGTERM);
                    done = true;
                    empty_pipe = true;
                    continue;
                }

                if( !done )
                {
                    int statusp;
                    int options = WNOHANG | WUNTRACED;
                    int rtn = waitpid(pid, &statusp, options);

                    if (rtn == pid)
                    {
                        done = true;

                        if (WIFEXITED(statusp))
                        {
                            int exit_status = WEXITSTATUS(statusp);

                            if (exit_status != 0)
                            {
                                add_to_log(SIMPLE_MAKE_WINDOW," .");
                                add_to_logf(DETAILED_MAKE_WINDOW,"CNL compiler exited with status %d\n",
                                            exit_status);
                                had_error = true;
                            }
                        }
                        else if (WIFSIGNALED(statusp))
                        {
                            // Error
                            add_to_log(SIMPLE_MAKE_WINDOW," .");
                            add_to_logf(DETAILED_MAKE_WINDOW,"CNL compiler died with signal: %d\n",
                                        WTERMSIG(statusp));
                            had_error = true;
                        }
                    }
                    else if (rtn == -1)
                    {
                        // Error
                        add_to_log(SIMPLE_MAKE_WINDOW," .");
                        /*
                          add_to_log(DETAILED_MAKE_WINDOW,"Received signal from CNL compiler");
                          if (errno < sys_nerr)
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
                          }
                          else
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
                          }
                        */
                        add_to_log(DETAILED_MAKE_WINDOW,"Received signal from CNL compiler.\n");
                        done = true;
                        had_error = true;

                        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
                        {
                            gMExp->printfMissionSpecWizardInstructionWindow("Error: Compilation failed.");
                            gMExp->printfMissionSpecWizardDataWindow("Received signal from CNL compiler.");
                        }
                    }
                }
            }

            // Dump the last message if waiting
            if (pos > 0)
            {
                queue[pos] = '\n'; pos++;
                queue[pos] = '\0';
                add_to_log(SIMPLE_MAKE_WINDOW," .");
                add_to_log(DETAILED_MAKE_WINDOW,queue);
            }
        }

        if(had_error)
        {
            break;
        }

        // Run gcc C++ compiler
        add_to_log(SIMPLE_MAKE_WINDOW," Done");
        add_to_log(SIMPLE_MAKE_WINDOW,"\nPhase III");
        add_to_log(DETAILED_MAKE_WINDOW,"\nPhase III: C++ Compilation: \n");
        dot_count=0;

        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()) && (!had_error))
        {
            gMExp->updateMissionSpecWizardProgressBar((double)((robot+1)*2)/(double)(num_robots*2+1));
        }

        // Extract out the robot name
        robot_names[robot] = strdup(robot_cnl_names[robot] + strlen(name_prefix));
        char *end = strchr(robot_names[robot], '.');
        if (end != NULL)
            *end = '\0';

        // build the .cc name of the source file
        int len = strlen(robot_cnl_names[robot]);
        if( len < 4 )
        {
            fprintf(stderr,"Illegal robot file name '%s'\n",
                    robot_cnl_names[robot]);
            exit(1);
        }

        // ends in .cnl so change it to .cc
        robot_cc_names[robot] = strdup(robot_cnl_names[robot]);
        robot_cc_names[robot][len-1] = '\0';
        robot_cc_names[robot][len-2] = 'c';


        if ((pid = fork()) == 0)
        {
            // Redirect stderr back to parent
            if (dup2(fd[1], STDERR_FILENO) == -1)
            {
                perror("GCC: unable to redirect stderr back to caller");
            }
            else if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
            {
                perror("GCC: Unable to config communication pipe");
                exit(1);
            }
            else if (fcntl(fd[1], F_SETFD, 0) == -1)
            {
                perror("GCC: Unable to config communication pipe");
                exit(1);
            }
            else
            {
                char *prog;
                char *argv[20];
                int p = 0;

                /* in child */

                // Set executable name
                prog = "gcc";
                argv[p++] = prog;

                // Extract the compile options
                rc_chain *val = (rc_chain *)rc_table.get(CFLAGS_NAME);
                if( val )
                {
                    char *str;
                    void *pos = val->first(&str);
                    while( pos )
                    {
                        argv[p++] = strdup(str);
                        pos = val->next(&str, pos);
                    }
                }

                // Add file name to the parm list
                argv[p++] = robot_cc_names[robot];

                // Extract the link options
                val = (rc_chain *)rc_table.get(LDFLAGS_NAME);
                if( val )
                {
                    char *str;
                    void *pos = val->first(&str);
                    while( pos )
                    {

                        // Insert the CNL link libraries where indicated.
                        if( strcmp(str,CNL_LIBS_FLAG) == 0 )
                        {
                            // Extract the link options
                            rc_chain *val = (rc_chain *)rc_table.get(CNL_LIBRARIES);
                            if( val )
                            {
                                char *str;
                                void *pos = val->first(&str);
                                while( pos )
                                {
                                    char *string = strdup(str);
                                    char *start = strrchr(string,'/');
                                    if( start )
                                    {
                                        // Insert a NULL
                                        *start = '\0';

                                        // Add a -L to the directory
                                        argv[p++] = strdupcat(strdup("-L"), string);

                                        // Bump past the slash
                                        start++;

                                        // Add a -l for the lib name
                                        argv[p++] = strdupcat(strdup("-l"), start);
                                    }
                                    else if( strlen(str) )
                                        argv[p++] = strdupcat(strdup("-l"), str);

                                    pos = val->next(&str, pos);
                                }
                            }
                        }
                        else
                        {
                            argv[p++] = strdup(str);
                        }

                        pos = val->next(&str, pos);
                    }
                }

                // Set output name
                argv[p++] = "-o";
                argv[p++] = robot_names[robot];

                // Mark end of parm list
                argv[p] = NULL;

                // Show the user
                char *cmd = strdup(prog);

                for (i = 1; i < p; i++)
                {
                    cmd = strdupcat(cmd, " ");
                    cmd = strdupcat(cmd, argv[i]);
                }
                fprintf(stderr, "\n%s\n", cmd);
                free(cmd);

                if (execvp(prog, argv) == -1)
                {
                    /*
                      fprintf(stderr, "Unable to exec the gcc compiler: ");
                      if (errno < sys_nerr)
                      {
                      fprintf(stderr, "%s\n", sys_errlist[errno]);
                      }
                      else
                      {
                      fprintf(stderr, "Unknown error number: %d\n", errno);
                      }
                    */
                    fprintf(stderr, "Unable to exec the gcc compiler.\n");
                }
            }

            close(fd[1]);

            // Kill this child
            exit(2);
        }

        if (pid == -1)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," .");
            /*
              add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork:");
              if (errno < sys_nerr)
              {
              add_to_log_cr(SIMPLE_MAKE_WINDOW,sys_errlist[errno]);
              add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
              }
              else
              {
              add_to_log(SIMPLE_MAKE_WINDOW," .");
              add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
              }
            */
            add_to_log(DETAILED_MAKE_WINDOW,"Unable to fork.\n");
            had_error = true;
            continue;
        }
        else
        {
            // wait for compile to finish
            char buf[256];
            char queue[2048];
            int pos = 0;

            bool done = false;
            bool empty_pipe = false;

            struct timespec sleep_delay = {0, SLEEP_TIME_NANOSEC};

            while (!done)
            {
                // Try to read from the com pipe
                int size = read(fd[0], buf, sizeof(buf));

                if (size > 0)
                {
                    for (i = 0; i < size; i++)
                    {
                        queue[pos] = buf[i]; pos++;

                        if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                        {
                            queue[pos] = '\0';
                            add_to_log(SIMPLE_MAKE_WINDOW," .");
                            add_to_log(DETAILED_MAKE_WINDOW,queue);
                            pos = 0;
                        }
                    }
                    empty_pipe = false;
                }
                else if( size < 0 )
                {
                    empty_pipe = true;
                }

                // Run X
                if (XtAppPending(app))
                    XtAppProcessEvent(app, XtIMAll);

                if (abort_make)
                {
                    had_error = true;
                    add_to_log(SIMPLE_MAKE_WINDOW," .");
                    add_to_log(DETAILED_MAKE_WINDOW,"Compilation canceled by user\n");
                    kill(pid, SIGTERM);
                    done = true;
                    empty_pipe = true;
                    continue;
                }

                if( !done )
                {
                    int statusp;
                    int options = WNOHANG | WUNTRACED;
                    int rtn = waitpid(pid, &statusp, options);

                    if (rtn == pid)
                    {
                        done = true;

                        if (WIFEXITED(statusp))
                        {
                            int exit_status = WEXITSTATUS(statusp);

                            if (exit_status != 0)
                            {
                                add_to_log(SIMPLE_MAKE_WINDOW," .");
                                add_to_logf(
                                    DETAILED_MAKE_WINDOW,
                                    "GCC compiler exited with status %d\n",
                                    exit_status);
                                done = true;
                                had_error = true;
                            }
                        }
                        else if (WIFSIGNALED(statusp))
                        {
                            // Error
                            add_to_log(SIMPLE_MAKE_WINDOW," .");
                            add_to_logf(
                                DETAILED_MAKE_WINDOW,
                                "GCC compiler died with signal: %d\n",
                                WTERMSIG(statusp));
                            done = true;
                            had_error = true;
                        }
                    }
                    else if (rtn == -1)
                    {
                        // Error
                        add_to_log(SIMPLE_MAKE_WINDOW," .");
                        /*
                          add_to_log(DETAILED_MAKE_WINDOW,"Received signal from GCC compiler");
                          if (errno < sys_nerr)
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_log_cr(DETAILED_MAKE_WINDOW,sys_errlist[errno]);
                          }
                          else
                          {
                          add_to_log(SIMPLE_MAKE_WINDOW," .");
                          add_to_logf(DETAILED_MAKE_WINDOW,"Unknown error number: %d\n", errno);
                          }
                        */
                        add_to_log(DETAILED_MAKE_WINDOW,"Received signal from GCC compiler.\n");
                        done = true;
                        had_error = true;

                        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
                        {
                            gMExp->printfMissionSpecWizardInstructionWindow("Error: Compilation failed.");
                            gMExp->printfMissionSpecWizardDataWindow("Received signal from GCC compiler.");
                        }
                    }
                }

                // sleep for 1/10 second to unload the CPU so the compilers can run
                nanosleep(&sleep_delay,NULL);
            }

            // Dump the last message if waiting
			while (!empty_pipe) {
				int size = read(fd[0], buf, sizeof(buf));
				if (size > 0) {
					for (i = 0; i < size; i++) {
						queue[pos] = buf[i];
						pos++;

						if (buf[i] == '\n' || pos >= (int) sizeof(queue)) {
							queue[pos] = '\0';
							add_to_log(SIMPLE_MAKE_WINDOW, " .");
							add_to_log(DETAILED_MAKE_WINDOW, queue);
							pos = 0;
						}
					}
					empty_pipe = false;
				} else if (size < 0) {
					empty_pipe = true;
				}
			}

        }

        if (abort_make)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," Canceled");
        }
        else if (had_error)
        {
            add_to_log(SIMPLE_MAKE_WINDOW," Failed");
        }
        else
        {
            add_to_log(SIMPLE_MAKE_WINDOW," Done");

            if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
            {
                gMExp->updateMissionSpecWizardProgressBar((double)(1+((robot+1)*2))/(double)(num_robots*2+1));
            }
        }
    }

    if(had_error)
    {
        add_to_log(SIMPLE_MAKE_WINDOW, "\n\n<<<<<<<<<<<<<<< COMPILATION FAILED >>>>>>>>>>>>>>>>\n");
        add_to_log(DETAILED_MAKE_WINDOW, "\n\n<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< COMPILATION FAILED >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
        gEventLogging->status("Make Error");

        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
        {
            gMExp->printfMissionSpecWizardInstructionWindow("Error: Compiling failed.");
        }
    }
    else
    {
        add_to_log(SIMPLE_MAKE_WINDOW, "\n\n------------ Compiling finished successfully ------------\n");
        add_to_log(DETAILED_MAKE_WINDOW, "\n\n----------------------------- Compilation finished sucessfully ------------------------------\n");

        gEventLogging->status("Make Success");

        if ((gMExp != NULL) && (gMExp->missionSpecWizardEnabled()))
        {
            gMExp->printfMissionSpecWizardInstructionWindow("Compilation successful.");
            gMExp->updateMissionSpecWizardProgressBar(1.0);
        }
    }
    add_to_log(SIMPLE_MAKE_WINDOW,"\nPress OK to continue");
    add_to_log(DETAILED_MAKE_WINDOW,"\nPress OK to continue");
    fprintf(stdout, "\a");
    fflush(stdout);

    // Get user OK before deleting log box
    XmString ok = XmStringCreateLocalized("OK");

    for (i = 0; i < NUM_MAKE_WINDOWS; i++)
    {
        XtRemoveAllCallbacks(cancelButton[i], XmNactivateCallback);

        XtAddCallback(
            cancelButton[i],
            XmNactivateCallback,
            (XtCallbackProc) ok_make_cb_compile,
            NULL);

        XtVaSetValues(
            cancelButton[i],
            XmNlabelString, ok,
            NULL);
    }

    XmStringFree(ok);

    XtVaSetValues(
        logWindow[SIMPLE_MAKE_WINDOW],
        XmNwidth, SIMPLE_WINDOW_WIDTH,
        XmNheight, SIMPLE_WINDOW_HEIGHT,
        NULL);

    XtVaSetValues(
        logWindow[DETAILED_MAKE_WINDOW],
        XmNwidth, DETAILED_WINDOW_WIDTH,
        XmNheight, DETAILED_WINDOW_HEIGHT,
        NULL);

    struct timespec sleep_delay = {0, SLEEP_TIME_NANOSEC};

    if (((gMExp != NULL) && (gMExp->missionSpecWizardEnabled())) ||
        gAutomaticExecution)
    {
        abort_make = true;
        XtUnmanageChild(logWindow[DETAILED_MAKE_WINDOW]);
        XtUnmanageChild(logWindow[SIMPLE_MAKE_WINDOW]);
    }

    while (!abort_make)
    {
        // Run X
        if (XtAppPending(app))
        {
            XtAppProcessEvent(app, XtIMAll);
        }
        else
        {
            // sleep for 1/10 second to unload the CPU
            nanosleep(&sleep_delay,NULL);
        }
    }

    if(!had_error)
    {
        have_fresh_make = true;
        XtSetSensitive(run_button, true);
    }

    // Clean up files
    {
        if (cdl_filename) unlink(cdl_filename);

        for (int robot = 0; robot < num_robots; robot++)
        {
            unlink(robot_cnl_names[robot]);
            //unlink(robot_cc_names[robot]);
        }
    }

    //close_logging_window();
    compile_finish = true;

    if (config->currentPageIsFSA())
    {
        // It is the FSA level.
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_ASSEMBLAGE);
    }
    else
    {
        gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_TOP_LEVEL);
    }

    gEventLogging->end("Make");

    return (!had_error);
}


///////////////////////////////////////////////////////////////////////
// $Log: make.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.6  2007/06/28 03:54:20  endo
// For 06/28/2007 demo.
//
// Revision 1.5  2007/05/15 18:50:00  endo
// BAMS Wizard implemented.
//
// Revision 1.4  2006/09/21 14:47:54  endo
// ICARUS Wizard improved.
//
// Revision 1.3  2006/09/15 22:37:40  endo
// ICARUS Wizard compilation meter bars added.
//
// Revision 1.2  2006/09/13 19:03:48  endo
// ICARUS Wizard implemented.
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.5  2006/02/14 02:27:18  endo
// gAutomaticExecution flag and its capability added.
//
// Revision 1.4  2006/01/10 06:07:39  endo
// AO-FNC Type-I check-in.
//
// Revision 1.3  2005/02/08 20:51:53  endo
// Sleep added in order to reduce the CPU load (Fix by Doug).
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.42  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.41  2003/04/06 09:00:07  endo
// Updated for CBR Wizard Prototype II.
//
// Revision 1.40  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.39  2000/07/07 21:21:00  endo
// *** empty log message ***
//
// Revision 1.38  2000/07/07 21:18:11  endo
// Disabled debugging statement.
//
// Revision 1.37  2000/07/07 20:30:35  lxy
// simplify the userinterface of compile option of cfgedit.
//
// Revision 1.35  2000/04/13 21:45:52  endo
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.34  2000/03/20 00:24:51  endo
// The bug of CfgEdit failing to compile multiple robots
// was fixed.
//
// Revision 1.33  1999/12/16 21:01:48  endo
// rh-6.0 porting.
//
// Revision 1.32  1998/11/03  01:53:28  endo
// The problem of cfgedit failing to make "output" for 2nd time
// was fixed by new boolean variable "cdl_to_cnl". "cdl_to_cnl"
// assures the program not going to "waitpid" until the queue
// "Writing robot /tmp/..." is received and not going back to
// the while-loop after the "waitpid" confirms that the child is
// exited. With (currently) unknown reason, the queue occasionary
// (or 2nd time compiling) brings up the queue after the child
// is exited, and messes up the sequence in "while(!dome ||
// ! empty_pipe) because the program was assuming otherwise.
//
// Revision 1.31  1998/11/03  01:14:03  endo
// A GNU library function, "execvp" returns a value '-1' if
// there is a error in the process. Thus, "if (execvp(prog, argv)"
// was changed to "if (execvp(prog, argv) == -1". Then, it can
// provide a proper error message to users.
//
// Revision 1.30  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.29  1996/10/04  20:58:17  doug
// changes to get to version 1.0c
//
// Revision 1.29  1996/10/01 13:00:11  doug
// went to version 1.0c
//
// Revision 1.28  1996/05/07 19:17:03  doug
// fixing compile warnings
//
// Revision 1.27  1996/03/13  01:55:23  doug
// *** empty log message ***
//
// Revision 1.26  1996/03/06  23:39:17  doug
// *** empty log message ***
//
// Revision 1.25  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.24  1996/02/27  05:01:12  doug
// *** empty log message ***
//
// Revision 1.23  1996/02/18  00:03:13  doug
// binding is working better
//
// Revision 1.22  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.21  1996/02/07  17:51:51  doug
// *** empty log message ***
//
// Revision 1.20  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.19  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.18  1996/01/17  18:47:01  doug
// *** empty log message ***
//
// Revision 1.17  1995/11/21  23:10:50  doug
// *** empty log message ***
//
// Revision 1.16  1995/11/14  15:46:12  doug
// fixed so compiles under linux
//
// Revision 1.15  1995/10/31  19:22:01  doug
// handle text field changes in loose focus callback
//
// Revision 1.14  1995/10/30  23:06:43  doug
// *** empty log message ***
//
// Revision 1.13  1995/10/11  22:03:40  doug
// *** empty log message ***
//
// Revision 1.12  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.11  1995/09/01  21:14:27  doug
// linking and copying glyphs works across pages
//
// Revision 1.10  1995/09/01  16:14:56  doug
// Able to run double wander config
//
// Revision 1.9  1995/08/24  19:14:32  doug
// *** empty log message ***
//
// Revision 1.8  1995/08/24  16:03:13  doug
// *** empty log message ***
//
// Revision 1.7  1995/08/22  18:03:48  doug
// cdl and cnl now work
//
// Revision 1.6  1995/08/22  13:53:35  doug
// cdl exec works
//
// Revision 1.5  1995/08/18  23:03:26  doug
// *** empty log message ***
//
//
///////////////////////////////////////////////////////////////////////
