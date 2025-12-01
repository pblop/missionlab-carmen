/**********************************************************************
 **                                                                  **
 **                              analyze.cc                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: analyze.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

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
#include <iostream>
#include <fstream>

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


#include <memory.h>
#include <malloc.h>
#include <stdarg.h>


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

using std::ifstream;

extern Widget main_window;

extern int sys_nerr;


static char name_prefix[256];


Widget analyze_log_window = NULL;
Widget analyze_text_w = NULL;

// These globals are used in the run command
bool have_fresh_analyze = false;
char *analyze_robot_names[MAX_ROBOTS];
char *analyze_cmd_list_name;

extern "C" {
int ioctl( int fd, int request, caddr_t arg);
}


/**********************************************************************
 **                                                                  **
 **                       constants and macros                       **
 **                                                                  **
 **********************************************************************/

#define MAXARGS         20

#define RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

static bool abort_analyze = false;

/*-----------------------------------------------------------------------*/
static void
abort_analyze_cb()
{
   abort_analyze = true;
}


static XmTextPosition wpr_position = 0;

/*-----------------------------------------------------------------------*/
static void
analyze_logging_window(Widget parent)
{
   Arg args[MAXARGS];
   Cardinal argcount = 0;
   XmString title = XSTRING("Analysis Log");
   XmString ok = XSTRING("   OK   ");
   XmString abort_msg = XSTRING("   Abort   ");
   static XtCallbackRec abort_cb_list[] =
   {
      {(XtCallbackProc) abort_analyze_cb, NULL},
      {(XtCallbackProc) NULL, NULL}};

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
   XtSetArg(args[argcount], XmNcancelCallback, abort_cb_list);
   argcount++;
   XtSetArg(args[argcount], XmNokCallback, abort_cb_list);
   argcount++;
   analyze_log_window = XmCreateMessageDialog(parent, "analyze_log_window", args, argcount);

   XtUnmanageChild(XmMessageBoxGetChild(analyze_log_window, XmDIALOG_HELP_BUTTON));
   XtSetSensitive(XmMessageBoxGetChild(analyze_log_window, XmDIALOG_OK_BUTTON), false);
   XmStringFree(title);
   XmStringFree(ok);

   // Create the text window
   argcount = 0;
   XtSetArg(args[argcount], XmNrows, 30);
   argcount++;
   XtSetArg(args[argcount], XmNcolumns, 120);
   argcount++;
   XtSetArg(args[argcount], XmNeditable, False);
   argcount++;
   XtSetArg(args[argcount], XmNeditMode, XmMULTI_LINE_EDIT);
   argcount++;
   XtSetArg(args[argcount], XmNwordWrap, False);
   argcount++;
   XtSetArg(args[argcount], XmNscrollHorizontal, True);
   argcount++;
   XtSetArg(args[argcount], XmNblinkRate, 0);
   argcount++;
   XtSetArg(args[argcount], XmNautoShowCursorPosition, True);
   argcount++;
   XtSetArg(args[argcount], XmNcursorPositionVisible, False);
   argcount++;
   analyze_text_w = XmCreateScrolledText(analyze_log_window, "analyze_text_w", args, argcount);

   XtManageChild(analyze_text_w);
   XtManageChild(analyze_log_window);

   // Reset cursor
   wpr_position = 0;
}

/*-----------------------------------------------------------------------*/
static void
close_logging_window()
{
   XtUnmanageChild(analyze_log_window);
}

/*-----------------------------------------------------------------------*/
static void
add_to_log(const char *msg)
{
   XmTextInsert(analyze_text_w, wpr_position, (char *)msg);
   wpr_position += strlen(msg);
   XtVaSetValues(analyze_text_w, XmNcursorPosition, wpr_position, NULL);
   XmTextShowPosition(analyze_text_w, wpr_position);
}

/*-----------------------------------------------------------------------*/
static void
add_to_log_cr(const char *msg)
{
   add_to_log(msg);
   add_to_log("\n");
}

/*-----------------------------------------------------------------------*/
static void
add_to_logf(const char *fmt,...)
{
   va_list args;
   char msg[2048];

   va_start(args, fmt);
   vsprintf(msg, fmt, args);
   va_end(args);
   add_to_log(msg);
}


/*-----------------------------------------------------------------------*/
// analyze: called to generate robot executables
void
analyze(void)
{
    gEventLogging->start("Analysis");

    if( !verify(true, false) )
    {
        gEventLogging->status("Verify Error");
        gEventLogging->cancel("Analysis");
        return;
    }

    // Renumber the robots to get consistent names
    renumber_robots();

    analyze_cmd_list_name = NULL;
    char *cdl_filename = NULL;
    bool had_error = false;

    abort_analyze = false;

    analyze_logging_window(main_window);

    // Build a unique prefix for tmp files
    sprintf(name_prefix, "/tmp/CfgEdit%d", getpid());
    cdl_filename = strdupcat(strdup(name_prefix), ".cdl");

    add_to_logf("Making configuration %s\n", config->configuration_name());

    // Write the file out as CDL
    add_to_logf("Writing configuration to %s\n", cdl_filename);
    int good = save_workspace(config->root(), cdl_filename, debug_save, false);

    if (!good)
    {
        add_to_log("Analysis aborted: Error writing tmp file\n");
        had_error = true;
    }

    // Create a communications pipe to talk to the child processes
    int pid;
    int fd[2];

    if (!had_error)
    {
        if (pipe(fd) < 0)
        {
            /*
            add_to_log("Unable to create communication pipe:");
            if (errno < sys_nerr)
            {
                add_to_log_cr(sys_errlist[errno]);
            }
            else
            {
                add_to_logf("Unknown error number: %d\n", errno);
            }
            */
            add_to_log("Unable to create communication pipe.\n");
            had_error = true;
        }

        if (fcntl(fd[0], F_SETFL, FNDELAY) == -1)
        {
            /*
            add_to_log("Unable to set communication pipe to non-blocking:");
            if (errno < sys_nerr)
            {
                add_to_log_cr(sys_errlist[errno]);
            }
            else
            {
                add_to_logf("Unknown error number: %d\n", errno);
            }
            */
            add_to_log("Unable to set communication pipe to non-blocking.\n");
            had_error = true;
        }

    }

    //*************************** run CDL *****************************
        char *robot_cnl_names[MAX_ROBOTS];
        char *robot_anz_names[MAX_ROBOTS];

        int num_robots = 0;
        for(int a=0; a<MAX_ROBOTS; a++)
        {
            robot_cnl_names[a] = NULL;
            robot_anz_names[a] = NULL;
            analyze_robot_names[a] = NULL;
        }

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
                    int i;

                    // in child

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
                /*
                add_to_log("Unable to fork:");
                if (errno < sys_nerr)
                {
                    add_to_log_cr(sys_errlist[errno]);
                }
                else
                {
                    add_to_logf("Unknown error number: %d\n", errno);
                }
                */
                add_to_log("Unable to fork.\n");
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
                            for (int i = 0; i < size; i++)
                            {
                                queue[pos++] = buf[i];

                                if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                                {
                                    queue[pos] = '\0';

                                    char buf[2048];

                                    if (sscanf(queue, "Writing robot %s", buf) == 1)
                                    {
                                        robot_cnl_names[num_robots++] = strdup(buf);
                                        cdl_to_cnl = true;
                                        empty_pipe = true;
                                    }
                                    else if (sscanf(queue, "Writing command list to %s", buf) == 1)
                                    {
                                        analyze_cmd_list_name = strdup(buf);
                                    }

                                    add_to_log(queue);
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
                        XtAppProcessEvent(app, XtIMAll);

                    if (abort_analyze)
                    {
                        had_error = true;
                        add_to_log("Analysis terminated by user\n");
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
                                    add_to_logf("CDL compiler exited with status %d\n",
                                                exit_status);
                                    had_error = true;
                                }
                            }
                            else if (WIFSIGNALED(statusp))
                            {
                                // Error
                                add_to_logf("CDL compiler died with signal: %d\n",
                                            WTERMSIG(statusp));
                                had_error = true;
                            }
                        }
                        else if (rtn == -1)
                        {
                            // Error
                            /*
                            add_to_log("Received signal from CDL compiler");

                            if (errno < sys_nerr)
                            {
                                add_to_log_cr(sys_errlist[errno]);
                            }
                            else
                            {
                                add_to_logf("Unknown error number: %d\n", errno);
                            }
                            */
                            add_to_log("Received signal from CDL compiler.\n");
                            had_error = true;
                        }
                    }
                }

                // Dump the last message if waiting
                if (pos > 0)
                {
                    queue[pos++] = '\n';
                    queue[pos] = '\0';
                    add_to_log(queue);
                }
            }
        }

        //+++++++++++++++++++++++ Finished running CDL +++++++++++++++++++++++++++++

        //**************** for each robot file **************************
            for (int robot = 0; robot < num_robots; robot++)
            {
                if( had_error )
                    break;

                //**************** run Real Time Analyzer **************************

                        // Extract out the robot name

                        analyze_robot_names[robot] = strdup(robot_cnl_names[robot] + strlen(name_prefix));
                        char *end = strchr(analyze_robot_names[robot], '.');
                        if (end != NULL)
                            *end = '\0';

                        // build the .anz name of the source file
                        int len = strlen(robot_cnl_names[robot]);
                        if( len < 4 )
                        {
                            fprintf(stderr,"Illegal robot file name '%s'\n",
                                    robot_cnl_names[robot]);
                            exit(1);
                        }

                        // ends in .cnl so change it to .anz
                        robot_anz_names[robot] = strdup(robot_cnl_names[robot]);
                        robot_anz_names[robot][len-1] = 'z';
                        robot_anz_names[robot][len-2] = 'n';
                        robot_anz_names[robot][len-3] = 'a';


                        if ((pid = fork()) == 0)
                        {
                            // Redirect stderr back to parent
                            if (dup2(fd[1], STDERR_FILENO) == -1)
                            {
                                perror("RTA: unable to redirect stderr back to caller");
                            }
                            else if (fcntl(STDERR_FILENO, F_SETFD, 0) == -1)
                            {
                                perror("RTA: Unable to config communication pipe");
                                exit(1);
                            }
                            else if (fcntl(fd[1], F_SETFD, 0) == -1)
                            {
                                perror("RTA: Unable to config communication pipe");
                                exit(1);
                            }
                            else
                            {
                                char *prog;
                                char *argv[20];
                                int p = 0;
                                int i;

                                // in child

                                // Set executable name
                                prog = "rta";
                                argv[p++] = prog;

                                // Add input file name to parm list
                                argv[p++] = robot_cnl_names[robot];

                                // Add output file name to the parm list
                                argv[p++] = robot_anz_names[robot];

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
                                    fprintf(stderr, "Unable to exec the rta: ");
                                    if (errno < sys_nerr)
                                    {
                                        fprintf(stderr, "%s\n", sys_errlist[errno]);
                                    }
                                    else
                                    {
                                        fprintf(stderr, "Unknown error number: %d\n", errno);
                                    }
                                    */
                                    fprintf(stderr, "Unable to exec the rta.\n");
                                }
                            }

                            close(fd[1]);

                            // Kill this child
                            exit(2);
                        }

                        if (pid == -1)
                        {
                            /*
                            add_to_log("Unable to fork:");
                            if (errno < sys_nerr)
                            {
                                add_to_log_cr(sys_errlist[errno]);
                            }
                            else
                            {
                                add_to_logf("Unknown error number: %d\n", errno);
                            }
                            */
                            add_to_log("Unable to fork.\n");
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
                            while (!done)
                            {
                                // Try to read from the com pipe
                                int size = read(fd[0], buf, sizeof(buf));

                                if (size > 0)
                                {
                                    for (int i = 0; i < size; i++)
                                    {
                                        queue[pos++] = buf[i];

                                        if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                                        {
                                            queue[pos] = '\0';
                                            add_to_log(queue);
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

                                if (abort_analyze)
                                {
                                    had_error = true;
                                    add_to_log("Analysis terminated by user\n");
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
                                                add_to_logf("RTA compiler exited with status: %d\n",
                                                            exit_status);
                                                had_error = true;
                                            }
                                        }
                                        else if (WIFSIGNALED(statusp))
                                        {
                                            // Error
                                            add_to_logf("RTA compiler died with signal: %d\n",
                                                        WTERMSIG(statusp));
                                            had_error = true;
                                        }
                                    }
                                    else if (rtn == -1)
                                    {
                                        // Error
                                        /*
                                        add_to_log("Received signal from RTA compiler");

                                        if (errno < sys_nerr)
                                        {
                                            add_to_log_cr(sys_errlist[errno]);
                                        }
                                        else
                                        {
                                            add_to_logf("Unknown error number: %d\n", errno);
                                        }
                                        */
                                        add_to_log("Received signal from RTA compiler.\n");
                                        had_error = true;
                                    }
                                }
                            }

                            // Dump the last message if waiting
                            if (pos > 0)
                            {
                                queue[pos++] = '\n';
                                queue[pos] = '\0';
                                add_to_log(queue);
                            }

                            if( !had_error)
                            {

                                ifstream data_file( robot_anz_names[robot]);
                                if( data_file.bad())
                                {
                                    add_to_log_cr( "Unable to open analysis results file.");
                                    had_error = true;
                                }
                                else
                                {
                                    add_to_log_cr( "\nAnalysis Results: ");
                                    while( !data_file.eof())
                                    {
                                        data_file.getline( buf, sizeof(buf));
                                        add_to_log_cr( buf);
                                    }
                                }
                            }

                        }
            }

            // Get user OK before deleting log box
            XtSetSensitive(XmMessageBoxGetChild(analyze_log_window, XmDIALOG_OK_BUTTON), true);
            XtSetSensitive(XmMessageBoxGetChild(analyze_log_window, XmDIALOG_CANCEL_BUTTON), false);


            if( had_error )
            {
                // wait for any final messages
                char buf[256];
                char queue[2048];
                int pos = 0;

                bool done = false;
                bool empty_pipe = false;
                while (!done)
                {
                    // Try to read from the com pipe
                    int size = read(fd[0], buf, sizeof(buf));

                    if (size > 0)
                    {
                        for (int i = 0; i < size; i++)
                        {
                            queue[pos++] = buf[i];

                            if (buf[i] == '\n' || pos >= (int)sizeof(queue))
                            {
                                queue[pos] = '\0';
                                add_to_log(queue);
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

                    if (abort_analyze)
                    {
                        had_error = true;
                        add_to_log("Analysis terminated by user\n");
                        kill(pid, SIGTERM);
                        done = true;
                        empty_pipe = true;
                        continue;
                    }

                }

                // Dump the last message if waiting
                if (pos > 0)
                {
                    queue[pos++] = '\n';
                    queue[pos] = '\0';
                    add_to_log(queue);
                }

                add_to_log("\n<<<<<<<<<<<<<<< ERROR IN ANALYSIS >>>>>>>>>>>>>>>>>\n");
                gEventLogging->status("Analysis Error");
            }
            else
            {
                add_to_log("\n--------- Analysis finished sucessfully -----------\n");
                gEventLogging->status("Analysis Success");
            }
            add_to_log("\nPress OK to continue");

            while (!abort_analyze)
            {
                // Run X
                if (XtAppPending(app))
                    XtAppProcessEvent(app, XtIMAll);
            }

            if( !had_error )
            {
                have_fresh_analyze = true;
                XtSetSensitive(run_button, true);
            }

            // Clean up files
            {
                if (cdl_filename)
                    unlink(cdl_filename);

                for (int robot = 0; robot < num_robots; robot++)
                {
                    unlink(robot_cnl_names[robot]);
                    unlink(robot_anz_names[robot]);
                }
            }

            close_logging_window();
            gEventLogging->end("Analysis");
}
