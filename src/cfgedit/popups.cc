/**********************************************************************
 **                                                                  **
 **                             popups.cc                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: popups.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

#include <Xm/Xm.h>

#include <Xm/ArrowBG.h>
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/FileSB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/CascadeBG.h>
#include <Xm/SelectioB.h>
#include <Xm/ScrolledW.h>
#include <Xm/Frame.h>
#include <Xm/ToggleBG.h>

#include <memory.h>
#include <malloc.h>
#include <sys/types.h>
#include <stdarg.h>

#include "load_cdl.h"
#include "design.h"
#include "version.h"
#include "globals.h"
#include "EventLogging.h"
#include "popups.h"
#include "configuration.hpp"
#include "help.h"
#include "mission_expert.h"
#include "toolbar.h"
#include "file_utils.h"
#include "assistantDialog.h"
#include "acdl_plus.h"
#include "string_utils.h"

//-----------------------------------------------------------------------
// Macros and Constatns
//-----------------------------------------------------------------------
#define	MAXARGS	20

#define	RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

const struct timespec PopUpWindows::EVENT_WAITER_SLEEP_TIME_NSEC_ = {0, 1000000};
const string PopUpWindows::EMPTY_STRING_ = "";

 //-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------
static Widget error_dialog;
static Widget help_dialog;
//extern Widget drawing_area;

//-----------------------------------------------------------------------
// Constructor for the PopUpWindows class.
//-----------------------------------------------------------------------
PopUpWindows::PopUpWindows(Widget parent, XtAppContext app) :
    generalMessageWindowIsUp_(false),
    generalYesNoWindowDone_(false)
{
    PopWinParent_ = parent;
    PopWinAppContext_ = app;
}

//-----------------------------------------------------------------------
// This function creates a initial dialog for CfgEdit.
//-----------------------------------------------------------------------
void PopUpWindows::createInitialDialog_(void)
{
    Arg wargs[16];
    XmString load_str, new_str, quit_str, title, msg_str;
    string msg ="Please select one of the following options.";
    int n;

    load_str = XmStringCreateLocalized("Load Robot");
    new_str = XmStringCreateLocalized("New Robot");
    quit_str = XmStringCreateLocalized("Quit");
    title = XmStringCreateLocalized("Welcome to CfgEdit");
    msg_str = XmStringCreateLtoR((char *)(msg.c_str()), XmSTRING_DEFAULT_CHARSET);

    n = 0;
    XtSetArg(wargs[n], XmNdialogTitle, title); n++;
    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(wargs[n], XmNokLabelString, new_str); n++;
    XtSetArg(wargs[n], XmNcancelLabelString, load_str); n++;
    XtSetArg(wargs[n], XmNhelpLabelString, quit_str); n++;
    XtSetArg(wargs[n], XmNmessageString, msg_str); n++;
    InitialDialog_w_ = XmCreateMessageDialog(PopWinParent_, "", wargs, n);

    XtAddCallback
        (InitialDialog_w_,
         XmNokCallback,
         (XtCallbackProc)InitializeDialog_cb,
         (void *)IDB_NEW);
    XtAddCallback
        (InitialDialog_w_,
         XmNcancelCallback,
         (XtCallbackProc)InitializeDialog_cb,
         (void *)IDB_LOAD);
    XtAddCallback
        (InitialDialog_w_,
         XmNhelpCallback,
         (XtCallbackProc)InitializeDialog_cb,
         (void *)IDB_CANCEL);

    XmStringFree(load_str);
    XmStringFree(title);
    XmStringFree(new_str);
    XmStringFree(quit_str);
    XmStringFree(msg_str);
}

//-----------------------------------------------------------------------
// This function popups the initial dialog.
//-----------------------------------------------------------------------
void PopUpWindows::popupInitialDialog_(void)
{
   PopWinIDBDone_ = FALSE;
   XtManageChild(InitialDialog_w_);
}
//-----------------------------------------------------------------------
// This function runs the initial dialog.
//-----------------------------------------------------------------------
void PopUpWindows::runInitialDialog(void)
{
    const rc_chain *files;
    char *filename;
    char simpleFilename[256], msg[256];

    gEventLogging->start("Initial Dialog");

    createInitialDialog_();
    popupInitialDialog_();

    gAssistantDialog->showPredefinedMessage(ASSISTANT_CFGEDIT_INITIAL_SELECTION);

    // Wait for the answer.
    while(!PopWinIDBDone_) XtAppProcessEvent(PopWinAppContext_, XtIMAll);

    // Event logging
    if (gEventLogging->eventLoggingIsON())
    {
        switch (PopWinIDBResult_) {

        case IDB_CANCEL:
            gEventLogging->log("Initial Dialog = Cancel");
            break;

        case IDB_LOAD:
            gEventLogging->log("Initial Dialog = Load Robot");
            break;

        case IDB_NEW:
        default:
            gEventLogging->log("Initial Dialog = New Robot");
        }

        gEventLogging->end("Initial Dialog");
    }

    gAssistantDialog->clearMessage();

    // user selected quit
    switch (PopWinIDBResult_) {

    case IDB_CANCEL:
        config->quit();
        break;

    case IDB_LOAD:
        runOpenFileDialog(OPEN_FILE_CDL);
        break;

    case IDB_NEW:
    default:
        if (gMExp->askUseMissionExpert())
        {
            gMExp->runMissionExpert(MEXP_RUNMODE_NEW);
        }
        else
        {
            // Create a new config: Optionally load a default config
            // If specified a default startup file, load it.
            files = rc_table.get("DefaultConfiguration");
            if (!(files && files->first(&filename))) break;

            fprintf(stderr, "Loading default configuration %s ...", filename);
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
            }
        }
        break;
    }
}

//-----------------------------------------------------------------------
// This function saves the user's input.
//-----------------------------------------------------------------------
void PopUpWindows::sendInitialDialogResult(int result)
{
    PopWinIDBResult_ = result;
    PopWinIDBDone_ = TRUE;
}

//-----------------------------------------------------------------------
// This function asks the users whether they want to discard their workplace
// before quiting CfgEdit.
//-----------------------------------------------------------------------
bool PopUpWindows::quitConfirm(Widget w)
{
    // Define the exit_design dialog structure
    dialog_rec exit_dialog =
        {
            (DIALOG_BLDR) XmCreateQuestionDialog,
            {
                {XmNmessageString, "Exit and discard workspace?"},
                {XmNokLabelString, "Yes"},
                {XmNcancelLabelString, "Cancel"},
                {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                {NULL, 0}
            },
            {
                {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_exit},
                {NULL, NULL, NULL}
            }
        };

    gEventLogging->start("Exit and discard workspace?");

    if (wait_dialog_box(w, &exit_dialog, NULL))
    {
        gEventLogging->log("Exit and discard workspace? = Yes");
        gEventLogging->end("Exit and discard workspace?");
    }
    else
    {
        gEventLogging->log("Exit and discard workspace? = Cancel");
        gEventLogging->end("Exit and discard workspace?");
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------
// This function asks the users whether they want to discard their workplace.
//-----------------------------------------------------------------------
bool PopUpWindows::discardConfirm(Widget w)
{
    dialog_rec discard_dialog =
        {
            (DIALOG_BLDR) XmCreateQuestionDialog,
            {
                {XmNmessageString, "Discard the current workspace?"},
                {XmNokLabelString, "Discard"},
                {XmNcancelLabelString, "Cancel"},
                {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                {NULL, 0}
            },
            {
                {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_discard},
                {NULL, NULL, NULL}
            }
        };

    gEventLogging->start("Discard the current workspace?");

    if (wait_dialog_box(w, &discard_dialog, NULL))
    {
        gEventLogging->log("Discard the current workspace? = Discard");
        gEventLogging->end("Discard the current workspace?");
    }
    else
    {
        gEventLogging->log("Discard the current workspace? = Cancel");
        gEventLogging->end("Discard the current workspace?");
        return FALSE;
    }

    return TRUE;
}

//-----------------------------------------------------------------------
// This function creates a genetic message window without any button.
//-----------------------------------------------------------------------
void PopUpWindows::createAndPopupGeneralMessageWindow(string message)
{
    XmString msg_str, title_str;
    Arg wargs[16];
    int n = 0;

    if (generalMessageWindowIsUp_)
    {
        popdownGeneralMessageWindow_();
    }

    msg_str = XmStringCreateLtoR(
        (char *)(message.c_str()),
        XmSTRING_DEFAULT_CHARSET);
    title_str = XmStringCreateLocalized(" ");

    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
    XtSetArg(wargs[n], XmNmessageString, msg_str); n++; 
    XtSetArg(wargs[n], XmNdialogTitle, title_str); n++;
    generalMessageWindow_ = XmCreateMessageDialog(PopWinParent_, "", wargs, n);

    XtAddCallback
        (generalMessageWindow_,
         XmNokCallback,
         (XtCallbackProc)cbGeneralMessageWindow_,
         (void *)this);

    XtUnmanageChild(
        XmMessageBoxGetChild(generalMessageWindow_, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(
        XmMessageBoxGetChild(generalMessageWindow_, XmDIALOG_HELP_BUTTON));

    XmStringFree(msg_str);
    XmStringFree(title_str);

    generalMessageWindowIsUp_ = true;

    XtManageChild(generalMessageWindow_);
}

//-----------------------------------------------------------------------
// This function popups the window above.
//-----------------------------------------------------------------------
void PopUpWindows::popdownGeneralMessageWindow_(void)
{
    if (generalMessageWindowIsUp_)
    {
        generalMessageWindowIsUp_ = false;
        XtUnmanageChild(generalMessageWindow_);
        XtDestroyWidget(generalMessageWindow_);
    }
}

//-----------------------------------------------------------------------
// This function creates the window that simply asks a "Yes" / "No"
// question. The question statement is the input.
//-----------------------------------------------------------------------
Widget PopUpWindows::createGeneralYesNoWindow_(string question)
{
    return (createGeneralYesNoWindow_(question.c_str()));
}

//-----------------------------------------------------------------------
// This function creates the window that simply asks a "Yes" / "No"
// question. The question statement is the input.
//-----------------------------------------------------------------------
Widget PopUpWindows::createGeneralYesNoWindow_(const char *question)
{
    XmString yes_str, no_str, msg_str, title_str;
    Arg wargs[16];
    int n=0;
    GeneralYesNoWindowCallbackData_t *callBackData;
    Widget generalYesNoWindow;

    msg_str = XmStringCreateLtoR((char *)question, XmSTRING_DEFAULT_CHARSET);
    yes_str = XmStringCreateLocalized("  Yes  ");
    no_str = XmStringCreateLocalized("  No  ");
    title_str = XmStringCreateLocalized(" ");

    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); n++;
    XtSetArg(wargs[n], XmNmessageString, msg_str); n++; 
    XtSetArg(wargs[n], XmNokLabelString, yes_str); n++;
    XtSetArg(wargs[n], XmNhelpLabelString, no_str); n++;
    XtSetArg(wargs[n], XmNdialogTitle, title_str); n++;
    generalYesNoWindow = XmCreateMessageDialog(PopWinParent_, "", wargs, n);

    // "Yes" button
    callBackData = new GeneralYesNoWindowCallbackData_t;
    callBackData->popWinInstance = this;
    callBackData->result = true;
    XtAddCallback
        (generalYesNoWindow,
         XmNokCallback,
         (XtCallbackProc)cbGeneralYesNoWindow_,
         (void *)callBackData);

    // "No" button
    callBackData = new GeneralYesNoWindowCallbackData_t;
    callBackData->popWinInstance = this;
    callBackData->result = false;
    XtAddCallback
        (generalYesNoWindow,
         XmNhelpCallback,
         (XtCallbackProc)cbGeneralYesNoWindow_,
         (void *)callBackData);

    XtUnmanageChild(XmMessageBoxGetChild(generalYesNoWindow, XmDIALOG_CANCEL_BUTTON));

    XmStringFree(msg_str);
    XmStringFree(yes_str);
    XmStringFree(no_str);

    return generalYesNoWindow;
}

//-----------------------------------------------------------------------
// This function popups the window above.
//-----------------------------------------------------------------------
void PopUpWindows::popupGeneralYesNoWindow_(Widget w)
{
    generalYesNoWindowResult_ = false;
    generalYesNoWindowDone_ = false;
    XtManageChild(w);
}

//-----------------------------------------------------------------------
// This function sets the "Yes" / "No" variable according to the answer
// given by the user.
//-----------------------------------------------------------------------
void PopUpWindows::sendGeneralYesNoWindowResult_(bool result)
{
    generalYesNoWindowResult_ = result;
    generalYesNoWindowDone_ = true;
}

//-----------------------------------------------------------------------
// This function asks the user to confirm the specified phrase.
//-----------------------------------------------------------------------
bool PopUpWindows::confirmUser(string phrase, bool useOKOnly)
{
    Widget w;
    XmString ok;

    w = createGeneralYesNoWindow_(phrase.c_str());

    if (useOKOnly)
    {
        ok = XmStringCreateLocalized("  OK  ");
        XtVaSetValues(w, XmNokLabelString, ok, NULL);
        XtUnmanageChild(XmMessageBoxGetChild(w, XmDIALOG_HELP_BUTTON));
        XmStringFree(ok);
    }

    popupGeneralYesNoWindow_(w);

    // Wait for the answer.
    while(!generalYesNoWindowDone_)
    {
        if (XtAppPending(PopWinAppContext_))
        {
            XtAppProcessEvent(PopWinAppContext_, XtIMAll);
        }
        else
        {
            nanosleep(&EVENT_WAITER_SLEEP_TIME_NSEC_, NULL);
        }
    }

    return generalYesNoWindowResult_;
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for CDL.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunOpenCDLFileDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_CDL);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for Mission Expert data.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunOpenMExpDataFileDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_MEXP_DATA);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog.
//-----------------------------------------------------------------------
void PopUpWindows::runOpenFileDialog(int openFileType)
{
    string patternString = EMPTY_STRING_;
    OpenFileDialogCallBackData_t *data = NULL;

    if (config->needs_save())
    {
        // Define the exit_design dialog structure
        dialog_rec exit_dialog =
            {(DIALOG_BLDR) XmCreateQuestionDialog,
             {
                 {XmNmessageString, "Overwrite workspace and lose changes?"},
                 {XmNokLabelString, "Yes"},
                 {XmNcancelLabelString, "Cancel"},
                 {XmNdefaultButtonType, XmDIALOG_CANCEL_BUTTON},
                 {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
                 {NULL, 0}
             },
             {
                 {XmNhelpCallback, (XtCallbackProc) help_callback, (XtPointer) HELP_PAGE_overwrite_workspace},
                 {NULL, NULL, NULL}}
            };

        gEventLogging->start("Overwrite workspace and lose changes?");

        // don't give it a widget to cache the dialog with since collides with
        // The file selection dialog.  Can't have two on one widget.
        if (wait_dialog_box(NULL, &exit_dialog, NULL))
        {
            gEventLogging->log("Overwrite workspace and lose changes? = Yes");
            gEventLogging->end("Overwrite workspace and lose changes?");
        }
        else
        {
            gEventLogging->log("Overwrite workspace and lose changes? = Cancel");
            gEventLogging->end("Overwrite workspace and lose changes?");
            return;
        }
    }

    // Define the select_file dialog structure
    
    switch (openFileType) {

    case OPEN_FILE_CDL:
        patternString = "*.cdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_CDL;
        break;

    case OPEN_FILE_MEXP_DATA:
        patternString = "*.data";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_MEXP_DATA;
        break;

    case OPEN_FILE_ACDLP2CIMXML:
        patternString = "*.acdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_ACDLP2CIMXML;
        break;

    case OPEN_FILE_ACDLP2CMDL:
        patternString = "*.acdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_ACDLP2CMDL;
        break;

    case OPEN_FILE_ACDLP2CMDLI:
        patternString = "*.acdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_ACDLP2CMDLI;
        break;

    case OPEN_FILE_ACDLP2CDL:
        patternString = "*.acdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_ACDLP2CDL;
        break;

    case OPEN_FILE_CIMXML2ACDLP:
        patternString = "*.xml";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_CIMXML2ACDLP;
        break;

    case OPEN_FILE_CMDL2ACDLP:
        patternString = "*.cmdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_CMDL2ACDLP;
        break;

    case OPEN_FILE_CMDLI2ACDLP:
        patternString = "*.cmdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_CMDLI2ACDLP;
        break;

    case OPEN_FILE_CDL2ACDLP:
        patternString = "*.cdl";
        data = new OpenFileDialogCallBackData_t;
        data->openFileType = OPEN_FILE_CDL2ACDLP;
        break;

    default:
        fprintf(
            stderr,
            "Error: PopUpWindows::runOpenFileDialog(). Unknown open file type %d.\a\n",
            openFileType);
        return;
        break;
    }

    dialog_rec dialog =
        {(DIALOG_BLDR) XmCreateFileSelectionDialog,
         {
             {XmNdialogTitle, "Select Input File"},
             {XmNpattern, (patternString.c_str())},
             {XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL},
             {NULL, 0}
         },
         {
             //{XmNokCallback, (XtCallbackProc)file_open_ok_cb, NULL},
             {XmNokCallback, (XtCallbackProc)cbOpenFile_, (void *)data},
             {XmNcancelCallback, (XtCallbackProc)XtUnmanageChild, NULL},
             {XmNhelpCallback, (XtCallbackProc)help_callback, (XtPointer)HELP_PAGE_select_file},
             {NULL, NULL, NULL}
         }
        };

    // Don't cache the widget, so will rescan the directories each time
    open_dialog_box(NULL, &dialog, NULL);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog.
//-----------------------------------------------------------------------
void PopUpWindows::cbOpenFile_(
    Widget w,
    caddr_t clientData,
    XmFileSelectionBoxCallbackStruct *fileSelectData)
{
    OpenFileDialogCallBackData_t *data = NULL;
    string filename, filename2, origExtension, newExtension;
    string bufString;
    char *value;
    char buf[1024];
    int openFileType;

    // Copy the data.
    XmStringGetLtoR(fileSelectData->value, XmSTRING_DEFAULT_CHARSET, &value);
    filename = value;
    data = (OpenFileDialogCallBackData_t *)clientData;
    openFileType = data->openFileType;
    delete data;

    // Close the dialog
    XtUnmanageChild(w);

    switch (openFileType) {

    case OPEN_FILE_CDL:
        open_cdl_file(filename);
        break;

    case OPEN_FILE_MEXP_DATA:
        gMExp->saveSpecifiedFeatureFileName(filename);
        gMExp->runMissionExpert(MEXP_RUNMODE_VIEW);
        break;

    case OPEN_FILE_ACDLP2CIMXML:
        origExtension = ".acdl";
        newExtension = ".xml";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::acdlp2cimxml(filename, filename2))
        {
            sprintf(buf, "CIM XML saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "CIM XML could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_ACDLP2CMDL:
        origExtension = ".acdl";
        newExtension = ".cmdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::acdlp2cmdl(filename, filename2))
        {
            sprintf(buf, "CMDL saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "CMDL could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_ACDLP2CMDLI:
        origExtension = ".acdl";
        newExtension = ".cmdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::acdlp2cmdli(filename, filename2))
        {
            sprintf(buf, "CMDLi saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "CMDLi could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_ACDLP2CDL:
        origExtension = ".acdl";
        newExtension = ".cdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::acdlp2cdl(filename, filename2))
        {
            sprintf(buf, "CDL saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "CDL could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_CIMXML2ACDLP:
        origExtension = ".xml";
        newExtension = ".acdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::cimxml2acdlp(filename, filename2))
        {
            sprintf(buf, "ACDL+ saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "ACDL+ could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_CMDL2ACDLP:
        origExtension = ".cmdl";
        newExtension = ".acdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::cmdl2acdlp(filename, filename2))
        {
            sprintf(buf, "ACDL+ saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "ACDL+ could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_CMDLI2ACDLP:
        origExtension = ".cmdl";
        newExtension = ".acdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::cmdli2acdlp(filename, filename2))
        {
            sprintf(buf, "ACDL+ saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "ACDL+ could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;

    case OPEN_FILE_CDL2ACDLP:
        origExtension = ".cdl";
        newExtension = ".acdl";
        filename2 = replaceStringInString(filename, origExtension, newExtension);
        if (ACDLPlus::cdl2acdlp(filename, filename2))
        {
            sprintf(buf, "ACDL+ saved in \'%s\'.", filename2.c_str());
        }
        else
        {
            sprintf(buf, "ACDL+ could not be saved.");
        }
        bufString = buf;
        gPopWin->createAndPopupGeneralMessageWindow(bufString);
        break;
    }
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for acdlp2cimxml.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunACDLP2CIMXMLDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_ACDLP2CIMXML);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for acdlp2cmdl.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunACDLP2CMDLDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_ACDLP2CMDL);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for acdlp2cmdli.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunACDLP2CMDLIDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_ACDLP2CMDLI);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for acdlp2cdl.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunACDLP2CDLDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_ACDLP2CDL);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for cimxml2acdlp.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunCIMXML2ACDLPDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_CIMXML2ACDLP);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for cmdl2acdlp.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunCMDL2ACDLPDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_CMDL2ACDLP);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for cmdli2acdlp.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunCMDLI2ACDLPDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_CMDLI2ACDLP);
}

//-----------------------------------------------------------------------
// This function pops up the Open File dialog for cdl2acdlp.
//-----------------------------------------------------------------------
void PopUpWindows::cbRunCDL2ACDLPDialog(
    Widget w,
    XtPointer clientData,
    XtPointer callbackData)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)callbackData;
    popWinInstance->runOpenFileDialog(OPEN_FILE_CDL2ACDLP);
}

//-----------------------------------------------------------------------
// This callback function sets the "Yes" / "No" variable according to the
// answer given by the user.
//-----------------------------------------------------------------------
void PopUpWindows::cbGeneralYesNoWindow_(
    Widget w,
    XtPointer client_data,
    XtPointer callback_data)
{
    PopUpWindows *popWinInstance = NULL;
    bool result;

    popWinInstance = ((GeneralYesNoWindowCallbackData_t *)client_data)->popWinInstance;
    result = ((GeneralYesNoWindowCallbackData_t *)client_data)->result;

    popWinInstance->sendGeneralYesNoWindowResult_(result);
    XtDestroyWidget(w);
}

//-----------------------------------------------------------------------
// This callback function for the generalMessageWindow_.
//-----------------------------------------------------------------------
void PopUpWindows::cbGeneralMessageWindow_(Widget w, XtPointer client_data, XtPointer callback_data)
{
    PopUpWindows *popWinInstance = NULL;

    popWinInstance = (PopUpWindows *)client_data;
    popWinInstance->popdownGeneralMessageWindow_();
}

//-----------------------------------------------------------------------
void InitializeDialog_cb(Widget w, XtPointer client_data, XtPointer callback_data)
{  
    int64_t result = (int64_t)client_data;

    gPopWin->sendInitialDialogResult(result);
    XtDestroyWidget(XtParent(w)); 
}  

//-----------------------------------------------------------------------
void create_help_dialog(Widget parent)
{
    XmString ok    = XSTRING("   OK   ");
    XmString title = XSTRING("Help: About");
    Arg wargs[2];
    int n=0;

    Display *display;
    Window window;
    Font big_font;
    XFontStruct *big_font_struct;
    char big_font_name[] = "-*-*-bold-i-*-*-20-*-*-*-*-*-*-*";
    GC erase_gc;
    GC draw_gc;
    Widget label;
    Pixmap about_pixmap;
    Dimension width, height;
    XmString about_console;
    char about_console_str[4096];
    char *about_console_fmt = 
        "                version %s\n\
\n\
Director: \n\
\n\
Ronald C. Arkin \n\
\n\
\n\
Original Author: \n\
\n\
Douglas C. MacKenzie \n\
\n\
\n\
Contributers:  \n\
\n\
Khaled S. Ali            Tucker R. Balch\n\
Robert R. Burridge       Jonathan M. Cameron\n\
Zhong Chen               Thomas R. Collins\n\
Michael J. Cramer        Jonathan F. Diaz\n\
Brian A. Ellenberger     Yoichiro Endo\n\
William C. Halliburton   Jung-Hoon Hwang\n\
David H. Johnson         Michael Kaess\n\
Zsolt Kira               Michael James Langford\n\
James B. Lee             Xinyu Li\n\
Richard Liston           Douglas C. MacKenzie\n\
Eric B. Martinson        Sapan D. Mehta\n\
Ananth Ranganathan       Antonio Sgorbissa\n\
Bradley A. Singletary    Alexander Stoytchev\n\
Benjamin Wong            Donghua Xu\n\
\n\
\n\
Mobile Robot Laboratory         \n\
College of Computing            \n\
Georgia Institute of Technology \n\
Atlanta, GA 30332-0280          \n\
\n\
Please send questions or suggestions to \n\
mlab@cc.gatech.edu \n\
\n\
Copyright (c) 1994 - 2006 \n\
ALL RIGHTS RESERVED.  \n\
See [Copyright] under the [Help] menu for \n\
details. \n\
\n\
Compiled on: \n\
\n\
%s";

    /* create the basic dialog */
    XtSetArg(wargs[n], XmNokLabelString, ok);   n++;
    XtSetArg(wargs[n], XmNdialogTitle, title);  n++;
    help_dialog = XmCreateMessageDialog(parent, "Help", wargs, n);
    XmStringFree(ok);

    /* get rid of the cancel and help buttons */
    XtUnmanageChild(XmMessageBoxGetChild(help_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(help_dialog, XmDIALOG_HELP_BUTTON));
      
    /* Now construct the message and figure out how big the pixmap should be */
    {
        Dimension msg_height, msg_width;

        sprintf(about_console_str, about_console_fmt, version_str, compile_time());
        about_console = XSTRING(about_console_str);

        XmStringExtent(gCfgEditFontList, about_console, &msg_width, &msg_height);

        width = msg_width;
        height = msg_height + 30;
    }

    /* create a pixmap for the message label */
    display = XtDisplay(help_dialog);
    window = RootWindowOfScreen(XtScreen(help_dialog));
    about_pixmap = XCreatePixmap(display, RootWindowOfScreen(XtScreen(help_dialog)), 
                                 width, height, DefaultDepthOfScreen(XtScreen(help_dialog)));

    // create the GC's
    {
        XGCValues xgcv;
        int bg, fg;
        int scr = DefaultScreen(display);
        Visual *myvis = DefaultVisual(display,scr);
        if (myvis->map_entries > 2) {
            XtVaGetValues(help_dialog,
                          XmNforeground, &fg,
                          XmNbackground, &bg,
                          NULL);
        }
        else {
            fg = BlackPixel(display, scr);
            bg = WhitePixel(display, scr);
        }
        xgcv.foreground = bg;
        xgcv.background = bg;
        erase_gc = XtGetGC(help_dialog, GCForeground | GCBackground, &xgcv);
        xgcv.foreground = fg;
        xgcv.background = bg;
        draw_gc = XtGetGC(help_dialog, GCForeground | GCBackground, &xgcv);
    }

    /* clear the pixmap */
    XFillRectangle(display, about_pixmap, erase_gc, 0, 0, width, height);

    /* Create the big font */
    big_font_struct = XLoadQueryFont(display, big_font_name);
    if (big_font_struct == NULL)
    {
        fprintf(stderr, "Error unable to load big font (%s)\n", big_font_name);
        big_font = gCfgEditDefaultFont;
    }
    else
    {
        big_font = big_font_struct->fid;
    }
   
    /* Write the nice big label */
    {
        int label_width;

        XSetFont(display, draw_gc, big_font);
        label_width = XTextWidth(big_font_struct, "Configuration Editor", strlen("Configuration Editor"));
        XDrawString(display, about_pixmap, draw_gc, (int)((width - label_width)/2), 25,
                    "Configuration Editor", strlen("Configuration Editor"));
    }
      
    /* Now draw the rest of the message */
    //XSetFont(display, draw_gc, default_font);
    XSetFont(display, draw_gc, gCfgEditDefaultFont);

    XmStringDraw(
        display,
        about_pixmap,
        //default_fonts,
        gCfgEditFontList,
        about_console,
        draw_gc,
        0,
        30,
        width,
        XmALIGNMENT_BEGINNING,
        XmSTRING_DIRECTION_L_TO_R,
        NULL);

    XmStringFree(about_console);

    /* Now register the pixmap */
    label = XmMessageBoxGetChild(help_dialog, XmDIALOG_MESSAGE_LABEL);
    XtVaSetValues(label,
                  XmNlabelType, XmPIXMAP,
                  XmNlabelPixmap, about_pixmap,
                  NULL);

    /* restore the system default font */
    XSetFont(display, gCfgEditGCs.black, gCfgEditDefaultFont);
}



//-----------------------------------------------------------------------
void warn_user(char *msg)
{
    Widget msg_label;
    XmString str, title;
    XmString old_msg;
    XmString new_msg;

    gEventLogging->status("warn_user() \"%s\"",msg);

    title = XSTRING("Warning!");

    XtVaSetValues(error_dialog, XmNdialogTitle, title, NULL);

    if (gWarningUser) {

        // Add this message to the message already being displayed
        XtVaGetValues(error_dialog, XmNmessageString, &old_msg, NULL);
        str = XSTRING("\n\n");
        new_msg = XmStringConcat(old_msg, str);
        XmStringFree(str);
        XmStringFree(old_msg);
        str = XSTRING(msg);
        new_msg = XmStringConcat(new_msg, str);
        XmStringFree(str);
        msg_label = XmMessageBoxGetChild(error_dialog, XmDIALOG_MESSAGE_LABEL);
        XtUnmanageChild(msg_label);
        XtUnmanageChild(error_dialog);
        XtVaSetValues(error_dialog, XmNmessageString, new_msg, NULL);
        XtManageChild(msg_label);
        XtManageChild(error_dialog);
        XmStringFree(new_msg);
        XtVaSetValues(error_dialog, XmNwidth, 600, NULL);  /* Not sure why I need this; maybe
                                                              it forces correct resizing? */
    }
    else
    {
        XtVaSetValues(error_dialog, RES_CONVERT(XmNmessageString, msg), NULL);
        XtManageChild(error_dialog);
        gWarningUser = true;
    }

    XmStringFree(title);
}


//-----------------------------------------------------------------------
void warn_userf(const char *fmt, ...)
{
    va_list args;
    char msg[512];

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    warn_user(msg);
}

//-----------------------------------------------------------------------
static void warn_user_ok_cb(void )
{
    XtUnmanageChild(error_dialog);
    gWarningUser = false;
}

//-----------------------------------------------------------------------
void create_error_dialog(Widget parent)
{
    Arg             args[MAXARGS];
    Cardinal        argcount = 0;
    XmString title = XSTRING("Error!");
    XmString ok    = XSTRING("   OK   ");
    static XtCallbackRec ok_cb_list[] = {
        {(XtCallbackProc) warn_user_ok_cb, NULL},
        {(XtCallbackProc) NULL, NULL}};

    XtSetArg(args[argcount], XmNdialogTitle, title);     argcount++;
    XtSetArg(args[argcount], XmNdialogStyle, 
             XmDIALOG_FULL_APPLICATION_MODAL);           argcount++;
    XtSetArg(args[argcount], XmNokLabelString, ok);      argcount++;
    XtSetArg(args[argcount], XmNokCallback, ok_cb_list); argcount++;
    error_dialog = XmCreateErrorDialog(parent, "error_dialog", args, argcount);
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_HELP_BUTTON));
    XmStringFree(title);
    XmStringFree(ok);
    gWarningUser = false;
}

//-----------------------------------------------------------------------
// called when an item on the help menu is selected
//-----------------------------------------------------------------------
void help_cb(Widget w, int item_num)
{
    XtManageChild(help_dialog);
}

//-----------------------------------------------------------------------
// Callback for some of the menu items declared in the MenuItem struct.
// The client data is a function that creates some sort of Motif dialog.
// Since the function returns this dialog, associate it with the menu
// item via XmNuserData so we (a) don't have a to keep a global and
// (b) don't have to repeatedly create one.
//-----------------------------------------------------------------------
void open_dialog_box(Widget w, dialog_rec *rec, XmAnyCallbackStruct *cbs)
{
    if(rec == NULL || rec->func == NULL)
    {
        warn_user("Internal error: invalid parameter record passed to open_dialog_box");
        return;
    }

    // initialize in case get values doesn't return anything sensible.
    Widget dialog = NULL;

    // first see if this menu item's dialog has been created yet
    if(w != NULL)
    {
        XtVaGetValues(w, XmNuserData, &dialog, NULL);
    }

    if (!dialog) 
    {
        Arg args[MAX_ARGS+1];
        Cardinal argcount = 0;

        if( w != NULL )
        {
            // Give the dialog's title the menu item label 
            XtSetArg(args[argcount], XmNtitle, XtName(w));   argcount++;
        }

        // Load the argument array
        for(int i=0; rec->args[i].name != NULL && i < MAX_ARGS; i++)
        {
            // DCM: This is a really bad idea, but I like it.

            // Special case: Fixup string types since had problems allocating
            // Strings in the array.
            if( strstr(rec->args[i].name, "String") ||
                strstr(rec->args[i].name, "Title")  ||
                strstr(rec->args[i].name, "pattern") )
            {
                XmString str= XmStringCreateLtoR((char *)(rec->args[i].value), XmSTRING_DEFAULT_CHARSET);
                XtSetArg(args[argcount], rec->args[i].name, str);   argcount++;
            }
            else
            {
               XtSetArg(args[argcount], rec->args[i].name, rec->args[i].value);   argcount++;
            }
        }

        /* create the dialog. 
         * Use the toplevel as the parent because we don't want the
         * parent of a dialog to be a menu item!
         *
         */
        dialog = (*rec->func)(top_level, "dialog", args, argcount);

        for(int i=0; rec->cbs[i].name != NULL && i<MAX_CBs; i++)
        {
            if( rec->cbs[i].data == PASS_WIDGET )
                XtAddCallback(dialog, rec->cbs[i].name, rec->cbs[i].proc, w);
            else
                XtAddCallback(dialog, rec->cbs[i].name, rec->cbs[i].proc, rec->cbs[i].data);
        }

        /* store the newly created dialog in the XmNuserData for the menu
         * item for easy retrieval next time. (see get-values above.)
         */
        if( w != NULL )
            XtVaSetValues(w, XmNuserData, dialog, NULL);
    }


    XtManageChild(dialog);

    /* call XtPopup() for good form... */
    XtPopup(XtParent(dialog), XtGrabNone);
    /* If the dialog was already open, XtPopup does nothing.  In
     * this case, at least make sure the window is raised to the top
     * of the window tree (or as high as it can get).
     */
    XRaiseWindow(XtDisplay(dialog), XtWindow(XtParent(dialog)));
}

/************************************************************************
* Pop up a dialog box and then wait for an answer.  
* Return true if OK button pressed.
* Return false if Cancel button pressed.
************************************************************************/
static int wdb_result;
static int wdb_done;
static char *wsb_result;

static void wdb_ok(Widget w)
{
    wdb_result = true;
    wdb_done = true;
    XtPopdown(XtParent(w));
}

static void wdb_cancel(Widget w)
{
    wdb_result = false;
    wdb_done = true;
    XtPopdown(XtParent(w));
}

static void wsb_ok(Widget w, dialog_rec *rec, XmSelectionBoxCallbackStruct *cbs)
{
    wdb_result = true;
    wdb_done = true;
    XmStringGetLtoR(cbs->value, XmSTRING_DEFAULT_CHARSET, &wsb_result);
    XtPopdown(XtParent(w));
}

static S_cbs cb_1 = {XmNokCallback, (XtCallbackProc)wdb_ok, NULL};
static S_cbs cb_3 = {XmNokCallback, (XtCallbackProc)wsb_ok, NULL};
static S_cbs cb_2 = {XmNcancelCallback, (XtCallbackProc)wdb_cancel, NULL};
static S_cbs cb_null = {NULL, NULL, NULL};

int wait_dialog_box(Widget w, dialog_rec *rec, XmAnyCallbackStruct *cbs)
{
    int i;

    // add the call backs to set the done flags
    for(i=0; rec->cbs[i].name != NULL && i<MAX_CBs; i++)
    {
        // NONE
    }

    if( i+3 >= MAX_CBs )
    {
        fprintf(stderr,"Internal Error: Expand callback structure S_cbs\n");
        exit(1);
    }

    rec->cbs[i++] = cb_1;
    rec->cbs[i++] = cb_2;
    rec->cbs[i++] = cb_null;

    wdb_result = false;
    wdb_done = false;

    open_dialog_box(w, rec, cbs);

    // Wait for the answer
    while( !wdb_done )
    {
        XtAppProcessEvent(app, XtIMAll);
    }

    return wdb_result;
}

int wait_SelectionBox(Widget w, dialog_rec *rec, char **result)
{
    int i;

    // add the call backs to set the done flags
    for(i=0; rec->cbs[i].name != NULL && i<MAX_CBs; i++)
    {
        // NONE
    }

    if( i+3 >= MAX_CBs )
    {
        fprintf(stderr,"Internal Error: Expand callback structure S_cbs\n");
        exit(1);
    }

    rec->cbs[i++] = cb_3;
    rec->cbs[i++] = cb_2;
    rec->cbs[i++] = cb_null;

    wdb_result = false;
    wdb_done = false;
    wsb_result = NULL;

    open_dialog_box(w, rec, NULL);

    // Wait for the answer
    while(!wdb_done)
    {
        XtAppProcessEvent(app, XtIMAll);
    }

    if(wdb_result && wsb_result)
    {
        *result = wsb_result;
    }
    else
    {
        *result = NULL;
    }

    return wdb_result;
}



///////////////////////////////////////////////////////////////////////
// $Log: popups.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.3  2006/08/02 21:37:33  endo
// acdlp2cdl() implemented.
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
// Revision 1.6  2006/07/12 09:06:05  endo
// Preparing for MissionLab 7.0 release.
//
// Revision 1.5  2006/02/14 02:27:18  endo
// gAutomaticExecution flag and its capability added.
//
// Revision 1.4  2006/01/30 02:47:28  endo
// AO-FNC CBR-CNP Type-I check-in.
//
// Revision 1.3  2005/08/12 21:49:59  endo
// More improvements for August demo.
//
// Revision 1.2  2005/02/07 22:25:26  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:33  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.29  2003/04/06 16:14:03  endo
// *** empty log message ***
//
// Revision 1.28  2003/04/06 12:43:34  endo
// *** empty log message ***
//
// Revision 1.27  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.26  2002/01/12 23:23:16  endo
// Mission Expert functionality added.
//
// Revision 1.25  2001/12/23 22:22:22  endo
// Preparing for 5.0 release.
//
// Revision 1.24  2000/08/01 22:51:04  endo
// Copyright and Contributers updated.
//
// Revision 1.23  2000/04/13 21:48:11  endo
// Checked in for Doug.
// This patch extends MissionLab to allow the user to import read-only
// library code.  When the user attempts to change something which is
// read-only, cfgedit pops up a dialog box and asks if it should import
// the object so it can be edited.  If OK'd, it imports the object.
//
// This fixes the problem with FSA's (and other assemblages) not being
// editable when they are loaded from the library.
//
// Revision 1.22  2000/03/14 00:00:58  endo
// The "Start Over" button was added to CfgEdit.
//
// Revision 1.21  1999/08/12 18:13:34  endo
// *** empty log message ***
//
// Revision 1.20  1999/08/12 02:25:57  endo
// *** empty log message ***
//
// Revision 1.19  1997/02/14 16:53:35  zchen
// *** empty log message ***
//
// Revision 1.18  1996/05/07  19:17:03  doug
// fixing compile warnings
//
// Revision 1.17  1996/03/05  22:55:37  doug
// *** empty log message ***
//
// Revision 1.16  1996/03/01  00:47:46  doug
// *** empty log message ***
//
// Revision 1.15  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.14  1996/01/20  23:40:47  doug
// Split nodes code was completely rewritten to present multiple choices
//
// Revision 1.13  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.12  1995/06/29  18:10:06  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
