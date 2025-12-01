/**********************************************************************
 **                                                                  **
 **                        assiatantDialog.cc                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
/* $Id: assistantDialog.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//---------------------------------------------------------------------
// Includes
//---------------------------------------------------------------------
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <signal.h>
#include <stdarg.h>
#include <string>
#include <fstream>
#include <iostream>

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

#include "assistantDialog.h"
#include "load_rc.h"

using std::ifstream;

//---------------------------------------------------------------------
// Predefined Assistant Message and Its File.
//---------------------------------------------------------------------
const AssistantDialog::AssistantDialog_predefinedMessageFile_t AssistantDialog::MESSAGE_FILE_[] =
{
    {ASSISTANT_CFGEDIT_INITIAL_SELECTION, "ASSISTANT_CFGEDIT_INITIAL_SELECTION.txt"},
    {ASSISTANT_CFGEDIT_CONFIRM_MISSION_EXPERT, "ASSISTANT_CFGEDIT_CONFIRM_MISSION_EXPERT.txt"},
    {ASSISTANT_CFGEDIT_CONFIRM_NOSAVE, "ASSISTANT_CFGEDIT_CONFIRM_NOSAVE.txt"},
    {ASSISTANT_MLAB_SELECT_OVERLAY, "ASSISTANT_MLAB_SELECT_OVERLAY.txt"},
    {ASSISTANT_MLAB_MISSION_DESIGN_INITIAL, "ASSISTANT_MLAB_MISSION_DESIGN_INITIAL.txt"},
    // ECC
    {ASSISTANT_MLAB_MISSION_DESIGN_TASK, "ASSISTANT_MLAB_MISSION_DESIGN_TASK.txt"},
    {ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT, "ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT.txt"}, //Lilia
    {ASSISTANT_MLAB_MISSION_DESIGN_TASK_PROPERTIES, "ASSISTANT_MLAB_MISSION_DESIGN_TASK_PROPERTIES.txt"}, //Lilia
    {ASSISTANT_MLAB_PLAYBACK, "ASSISTANT_MLAB_PLAYBACK.txt"}, //Lilia
    {ASSISTANT_MLAB_REPAIR_GUIDE, "ASSISTANT_MLAB_REPAIR_GUIDE.txt"}, //Lilia
    {ASSISTANT_MLAB_MISSION_SUCCESS_DIALOGUE, "ASSISTANT_MLAB_MISSION_SUCCESS_DIALOGUE.txt"}, //Lilia
    {ASSISTANT_MLAB_APPLY_CHANGE, "ASSISTANT_MLAB_APPLY_CHANGE.txt"}, //Lilia
    {ASSISTANT_MLAB_APPLY_CHANGE_NO, "ASSISTANT_MLAB_APPLY_CHANGE_NO.txt"}, //Lilia
    {ASSISTANT_MLAB_ASK_DIAGNOSIS, "ASSISTANT_MLAB_ASK_DIAGNOSIS.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_COMPILE, "ASSISTANT_CFGEDIT_COMPILE.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_RUN, "ASSISTANT_CFGEDIT_RUN.txt"}, //Lilia    
    {ASSISTANT_CFGEDIT_TOP_LEVEL, "ASSISTANT_CFGEDIT_TOP_LEVEL.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_SUITABILITY, "ASSISTANT_CFGEDIT_SUITABILITY.txt"},
    {ASSISTANT_CFGEDIT_MISSION_RETRIEVED, "ASSISTANT_CFGEDIT_MISSION_RETRIEVED.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_REPAIR_SOLUTION_FOUND, "ASSISTANT_CFGEDIT_REPAIR_SOLUTION_FOUND.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_NO_REPAIR_SOLUTION_FOUND, "ASSISTANT_CFGEDIT_NO_REPAIR_SOLUTION_FOUND.txt"}, //Lilia
    {ASSISTANT_CFGEDIT_ASSEMBLAGE, "ASSISTANT_CFGEDIT_ASSEMBLAGE.txt"},
    {ASSISTANT_CFGEDIT_ROBOT_GROUP, "ASSISTANT_CFGEDIT_ROBOT_GROUP.txt"},
    {ASSISTANT_CFGEDIT_ROBOT, "ASSISTANT_CFGEDIT_ROBOT.txt"},
    {ASSISTANT_CFGEDIT_FSA_EMPTY, "ASSISTANT_CFGEDIT_FSA_EMPTY.txt"},
    {ASSISTANT_CFGEDIT_FSA, "ASSISTANT_CFGEDIT_FSA.txt"},
    {ASSISTANT_CFGEDIT_DEFAULT, "ASSISTANT_CFGEDIT_DEFAULT.txt"}
};

//---------------------------------------------------------------------
// Constants
//---------------------------------------------------------------------
const string AssistantDialog::DEFAULT_FONT_NAME_[] =
{
    "-*-clean-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-courier-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-fixed-bold-r-*-*-*-*-*-*-*-*-*-*",
    "-*-fixed-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*",
    "-*-lucidatypewriter-bold-r-*-*-14-*-*-*-*-*-*-*"
};
const string AssistantDialog::EMPTY_STRING_ = "";
const string AssistantDialog::RCTABLE_DISABLE_STRING_ = "DisableAssistantDialog";
const string AssistantDialog::RCTABLE_HIDE_STRING_ = "HideAssistantDialog";
const string AssistantDialog::RCTABLE_MESSAGE_DIR_STRING_ = "AssistantDialogMsgDir";
const string AssistantDialog::DIALOG_TITLE_ = "Assistant Dialog";
const string AssistantDialog::LABEL_HIDE_DIALOG_ = " Hide Dialog ";
const int AssistantDialog::NUM_DEFAULT_FONT_NAME_ = 6;
const int AssistantDialog::DEFAULT_DIALOG_HEIGHT_ = 300;
const int AssistantDialog::DEFAULT_DIALOG_WIDTH_ = 300;
const int AssistantDialog::DEFAULT_DIALOG_LEFTOFFSET_ = 50;
const int AssistantDialog::DEFAULT_DIALOG_BOTTOMOFFSET_ = 50;
const int AssistantDialog::DEFAULT_MESSAGE_WINDOW_HEIGHT_ = 200;
const int AssistantDialog::DEFAULT_MESSAGE_WINDOW_WIDTH_ = 300;

//---------------------------------------------------------------------
// Globals
//---------------------------------------------------------------------

//---------------------------------------------------------------------
// This function deletes the AssistantDialog instance. If there is an
// error message, it reports before it quits.
//---------------------------------------------------------------------
void quitAssistantDialog(char *msg)
{
    if (msg != NULL)
    {
        fprintf(stderr, "Error: %s\n", msg);
    }

    if (gAssistantDialog != NULL)
    {
        delete gAssistantDialog;
        gAssistantDialog = NULL;
    }
}

//-----------------------------------------------------------------------
// This callback function pops up the Assistant Dialog .
//-----------------------------------------------------------------------
void cbPopupAssistantDialog(Widget w, XtPointer client_data, XtPointer callback_data)
{
    gAssistantDialog->popupDialog();
}

//---------------------------------------------------------------------
// Constructor
//---------------------------------------------------------------------
AssistantDialog::AssistantDialog(
    Widget parentWidget,
    const symbol_table<rc_chain> &rc_table,
    const int initPopupOption)
{
    parentWidget_ = parentWidget;
    display_ = XtDisplay(parentWidget_);
    rc_table_ = rc_table;

    dialogCreated_ = false;
    dialogIsUp_ = false;

    messageDirectoryPath_ = getMessageDirectoryPath_();
    loadFonts_();
    
    if (check_bool_rc(rc_table, RCTABLE_DISABLE_STRING_.c_str()) == true)
    {
        // Use the assistant dialog unless specified not to.
        useAssistantDialog_ = false;
    }
    else
    {
        useAssistantDialog_ = true;

        switch (initPopupOption) {

        case ASSISTANT_INIT_POPUP_ENABLE:
            popupDialog_();
            break;

        case ASSISTANT_INIT_POPUP_RCVALUE:
            if (check_bool_rc(rc_table, RCTABLE_HIDE_STRING_.c_str()) == false)
            {
                // If not specified to hide, popup the dialog..
                popupDialog_();
            }
            break;

        default:
        case ASSISTANT_INIT_POPUP_DISABLE:
            break;
        }
    }
}

//---------------------------------------------------------------------
// Distructor
//---------------------------------------------------------------------
AssistantDialog::~AssistantDialog(void)
{
}

//-----------------------------------------------------------------------
// This function loads fonts.
//-----------------------------------------------------------------------
void AssistantDialog::loadFonts_(void)
{
    int i;
    XFontStruct *defaultFontStruct;

    // Prepare font.
    for (i = 0; i < NUM_DEFAULT_FONT_NAME_; i++)
    {
        defaultFontStruct = XLoadQueryFont(display_, DEFAULT_FONT_NAME_[i].c_str());

        if (defaultFontStruct != NULL)
        {
            break;
        }
    }

    if (defaultFontStruct == NULL)
    {
        quitAssistantDialog("Font cannot be loaded.");
    }

    defaultFont_ = defaultFontStruct->fid;
    defaultFonts_ = XmFontListCreate(defaultFontStruct, XmSTRING_DEFAULT_CHARSET);
}

//-----------------------------------------------------------------------
// This function loads fonts.
//-----------------------------------------------------------------------
void AssistantDialog::showMessage(int msgType, char *msg, ...)
{
    va_list args;
    char message[1024];

    if ((!useAssistantDialog_) || (!dialogIsUp_))
    {
        return;
    }

    va_start(args, msg);
    vsprintf(message, msg, args);
    va_end(args);

    switch (msgType) {

    case ASSISTANT_APPEND_MESSAGE:
        break;

    case ASSISTANT_NEW_MESSAGE:
    default:
        XmTextReplace(messageWindowWidget_, 0, messageTextPos_, NULL);
        messageTextPos_ = 0;
        break;
    }

    XmTextInsert(messageWindowWidget_, messageTextPos_, message);
    messageTextPos_ += strlen(message);
        
    XtVaSetValues(messageWindowWidget_, XmNcursorPosition, messageTextPos_, NULL);
    XmTextShowPosition(messageWindowWidget_, 0);

}

//-----------------------------------------------------------------------
// This function creates the assistant dialog.
//-----------------------------------------------------------------------
void AssistantDialog::createDialog_(void)
{
    XmString cancelString, msgString, titleString;
    //AssistantDialog_callbackData_t *callbackData;
    Arg wargs[16];
    Pixel whitepixel;
    int screenNumber;
    int n;

    msgString = XmStringCreateLtoR((char *)(DIALOG_TITLE_.c_str()), XmSTRING_DEFAULT_CHARSET);
    cancelString = XmStringCreateLocalized((char *)(LABEL_HIDE_DIALOG_.c_str()));
    titleString = XmStringCreateLocalized(" ");

    n = 0;
    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_MODELESS); n++;
    XtSetArg(wargs[n], XmNmessageString, msgString); n++; 
    XtSetArg(wargs[n], XmNcancelLabelString, cancelString); n++;
    XtSetArg(wargs[n], XmNdialogTitle, titleString); n++;
    XtSetArg(wargs[n], XmNdefaultPosition, false); n++;
    assistantDialog_ = XmCreateMessageDialog(parentWidget_, "", wargs, n);

    messageFrameWidget_ = XtVaCreateWidget
        ("assistantMenuFrameWidget",
         xmFormWidgetClass, assistantDialog_,
         XmNshadowType, XmSHADOW_ETCHED_OUT,
         XmNheight, DEFAULT_MESSAGE_WINDOW_HEIGHT_,
         XmNwidth, DEFAULT_MESSAGE_WINDOW_WIDTH_,
         NULL);

    screenNumber = DefaultScreen(display_);
    whitepixel = WhitePixel(display_, screenNumber);

    n = 0;
    XtSetArg(wargs[n], XmNscrollVertical, True); n++;
    XtSetArg(wargs[n], XmNscrollHorizontal, False); n++;
    XtSetArg(wargs[n], XmNeditMode, XmMULTI_LINE_EDIT); n++;
    XtSetArg(wargs[n], XmNeditable, False); n++;
    XtSetArg(wargs[n], XmNcursorPositionVisible, False); n++;
    XtSetArg(wargs[n], XmNwordWrap, True); n++;
    XtSetArg(wargs[n], XmNrows, 5); n++;
    XtSetArg(wargs[n], XmNbackground, whitepixel); n++;
    messageWindowWidget_ = XmCreateScrolledText(messageFrameWidget_, "messageWindowWidget_", wargs, n);

    XtVaSetValues(XtParent(messageWindowWidget_),
                  XmNleftAttachment, XmATTACH_FORM,
                  XmNtopAttachment, XmATTACH_FORM,
                  XmNrightAttachment, XmATTACH_FORM,
                  XmNbottomAttachment, XmATTACH_FORM,
                  NULL);

    messageTextPos_ = 0;

    XtUnmanageChild(XmMessageBoxGetChild(assistantDialog_, XmDIALOG_OK_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(assistantDialog_, XmDIALOG_HELP_BUTTON));

    XmStringFree(msgString);
    XmStringFree(cancelString);

    XtAddCallback(
        assistantDialog_,
        XmNcancelCallback,
        (XtCallbackProc)cbDialogHide_,
        this);

    dialogCreated_ = true;
}

//-----------------------------------------------------------------------
// This function pops up the assistant dialog.
//-----------------------------------------------------------------------
void AssistantDialog::popupDialog_(void)
{
    int displayWidth, displayHeight, posX, posY;
    int screenNumber;

    if (!useAssistantDialog_)
    {
        return;
    }

    if (!dialogCreated_)
    {
        createDialog_();
    }

    screenNumber = DefaultScreen(display_);
    displayWidth = DisplayWidth(display_, screenNumber);
    displayHeight = DisplayHeight(display_, screenNumber);

    posX = displayWidth - DEFAULT_DIALOG_LEFTOFFSET_ - DEFAULT_DIALOG_WIDTH_;
    posY = displayHeight - DEFAULT_DIALOG_BOTTOMOFFSET_ - DEFAULT_DIALOG_HEIGHT_;

    XtManageChild(messageFrameWidget_);
    XtManageChild(messageWindowWidget_);
    XtManageChild(assistantDialog_);

    XtVaSetValues(assistantDialog_,
                  XmNx, posX,
                  XmNy, posY,
                  NULL);

    dialogIsUp_ = true;
}

//-----------------------------------------------------------------------
// This function finds the path of the directory that stores the message
// files, defined in .cfgeditrc.
//-----------------------------------------------------------------------
string AssistantDialog::getMessageDirectoryPath_(void)
{
    string path;
    rc_chain *msgDirList = NULL;
    char *msgDir = NULL;

    // Get the directory from the .cfgeditrc file.
    msgDirList = (rc_chain *)rc_table_.get(RCTABLE_MESSAGE_DIR_STRING_.c_str());

    if (msgDirList != NULL)
    {
        msgDirList->first(&msgDir);
    }

    if (msgDir == NULL)
    {
        quitAssistantDialog("AssistantDialogMsgDir not specified in .cfgeditrc");
        return EMPTY_STRING_;
    }

    path = msgDir;

    // Append "/" if it does not end with it.
    if (msgDir[path.length()-1] != '/')
    {
        path += "/";
    }

    return path;
}

//-----------------------------------------------------------------------
// This function shows the message that is predefined.
//-----------------------------------------------------------------------
void AssistantDialog::showPredefinedMessage(int msgIndex)
{
    string filename, fullFilename;
    int dataSize;
    char *msg;
    bool hadError = true;

    if ((!useAssistantDialog_) || (!dialogIsUp_))
    {
        return;
    }

    fullFilename = messageDirectoryPath_;

    // Get the file name first.
    if (getPredefinedMessageFileName_(msgIndex, filename))
    {
        // Add directory;
        fullFilename += filename;

        // Load the message.
        if (loadFileContents_(fullFilename, &msg, &dataSize))
        {
            // Show it to the user.
            showMessage(ASSISTANT_NEW_MESSAGE, msg);
            delete [] msg;
            hadError = false;
            msg = NULL;
        }
    }
    
    if (hadError)
    {
        fprintf(stderr, "Warning: Assistant::showPredefinedMessage(). Unknown file.\n");
    }

    return;
}

//-----------------------------------------------------------------------
// This function loads the contents of the file as data.
//-----------------------------------------------------------------------
int AssistantDialog::loadFileContents_(string filename, char **data, int *dataSize)
{
    int length = 0;
    char *buf = NULL;
    ifstream inputStream(filename.c_str());
    
    if (inputStream.bad()) 
    {
        fprintf
            (stderr,
             "Warning: Assistant::loadFileContents_(). file %s not readable.\n",
             filename.c_str());
        return false;
    }

    inputStream.seekg (0, std::ios::end);
    length = inputStream.tellg();
    if (length < 1)
    {
        return false;
    }

    inputStream.seekg (0, std::ios::beg);

    buf = new char[length];
    inputStream.read (buf, length);
    buf[length-1] = '\0';

    if (buf == NULL)
    {
        return false;
    }

    *dataSize = length;
    *data = buf;

    return true;
}

//-----------------------------------------------------------------------
// This function finds the predefined message file name from the index.
//-----------------------------------------------------------------------
int AssistantDialog::getPredefinedMessageFileName_(int msgIndex, string &filename)
{
    int i;

    for (i = 0; i < ASSISTANT_NUM_PREDEFINED_MESSAGES; i++)
    {
        if (MESSAGE_FILE_[i].index == msgIndex) 
        {
            filename = MESSAGE_FILE_[i].fileName;
            return true;
        }
    }

    return false;
}

//-----------------------------------------------------------------------
// This function clears the message.
//-----------------------------------------------------------------------
void AssistantDialog::clearMessage(void)
{
    if ((!useAssistantDialog_) || (!dialogIsUp_))
    {
        return;
    }

    XmTextReplace(messageWindowWidget_, 0, messageTextPos_, NULL);
    messageTextPos_ = 0;
}

//-----------------------------------------------------------------------
// This callback function gets called when the "Cancel" button of the
// status window window was pressed.
//-----------------------------------------------------------------------
void AssistantDialog::cbDialogHide_(Widget w, XtPointer clientData, XtPointer callbackData)
{
    AssistantDialog *assistantDialogInstance = NULL;

    assistantDialogInstance = (AssistantDialog *)clientData;
    XtDestroyWidget(assistantDialogInstance->assistantDialog_);
    assistantDialogInstance->dialogCreated_ = false;
    assistantDialogInstance->dialogIsUp_ = false;
}

