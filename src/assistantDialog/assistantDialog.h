/**********************************************************************
 **                                                                  **
 **                        assiatantDialog.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2003 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/
/* $Id: assistantDialog.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef ASSIATANTDIALOG_H
#define ASSIATANTDIALOG_H

#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

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

#include "load_rc.h"

using std::string;

typedef enum {
    ASSISTANT_NEW_MESSAGE,
    ASSISTANT_APPEND_MESSAGE
} AssistantDialog_messageTypes_t;

typedef enum {
    ASSISTANT_CFGEDIT_INITIAL_SELECTION,
    ASSISTANT_CFGEDIT_CONFIRM_MISSION_EXPERT,
    ASSISTANT_CFGEDIT_CONFIRM_NOSAVE,
    ASSISTANT_MLAB_SELECT_OVERLAY,
    ASSISTANT_MLAB_MISSION_DESIGN_INITIAL,
    // ECC -- Added start
    ASSISTANT_MLAB_MISSION_DESIGN_TASK,
    ASSISTANT_CFGEDIT_SUITABILITY,
    ASSISTANT_CFGEDIT_ASSEMBLAGE,
    ASSISTANT_CFGEDIT_ROBOT_GROUP,
    ASSISTANT_CFGEDIT_ROBOT,
    ASSISTANT_CFGEDIT_FSA_EMPTY,
    ASSISTANT_CFGEDIT_FSA,
    ASSISTANT_CFGEDIT_DEFAULT,
    // ECC -- Added end
    ASSISTANT_MLAB_MISSION_DESIGN_TASK_SELECT, //Lilia
    ASSISTANT_MLAB_MISSION_DESIGN_TASK_PROPERTIES, //Lilia
    ASSISTANT_MLAB_PLAYBACK, //Lilia
    ASSISTANT_MLAB_REPAIR_GUIDE, //Lilia
    ASSISTANT_MLAB_MISSION_SUCCESS_DIALOGUE, //Lilia
    ASSISTANT_MLAB_APPLY_CHANGE, //Lilia
    ASSISTANT_MLAB_APPLY_CHANGE_NO, //Lilia
    ASSISTANT_MLAB_ASK_DIAGNOSIS,//Lilia
    ASSISTANT_CFGEDIT_COMPILE, //Lilia
    ASSISTANT_CFGEDIT_RUN, //Lilia
    ASSISTANT_CFGEDIT_TOP_LEVEL, //Lilia
    ASSISTANT_CFGEDIT_MISSION_RETRIEVED, //Lilia
    ASSISTANT_CFGEDIT_REPAIR_SOLUTION_FOUND, //Lilia
    ASSISTANT_CFGEDIT_NO_REPAIR_SOLUTION_FOUND, //Lilia
    ASSISTANT_NUM_PREDEFINED_MESSAGES // Keep this at the end of the list
} AssistantDialog_predefinedMessagesIndex_t;

typedef enum {
    ASSISTANT_INIT_POPUP_DISABLE,
    ASSISTANT_INIT_POPUP_ENABLE,
    ASSISTANT_INIT_POPUP_RCVALUE
} AssistantDialog_initPopupOption_t;

class AssistantDialog {

    typedef struct {
        int index;
        string fileName;
    } AssistantDialog_predefinedMessageFile_t;

protected:
    Display *display_;
    Widget parentWidget_;
    Widget assistantDialog_;
    Widget messageFrameWidget_;
    Widget messageWindowWidget_;
    XmFontList fontList_;
    XmFontList defaultFonts_;
    Font defaultFont_;
    string messageDirectoryPath_;
    symbol_table <rc_chain> rc_table_;
    int messageTextPos_;
    bool useAssistantDialog_;
    bool dialogCreated_;
    bool dialogIsUp_;

    static const string DEFAULT_FONT_NAME_[];
    static const string EMPTY_STRING_;
    static const string RCTABLE_DISABLE_STRING_;
    static const string RCTABLE_HIDE_STRING_;
    static const string RCTABLE_MESSAGE_DIR_STRING_;
    static const string DIALOG_TITLE_;
    static const string LABEL_HIDE_DIALOG_;
    static const int NUM_DEFAULT_FONT_NAME_;
    static const int DEFAULT_MESSAGE_WINDOW_HEIGHT_;
    static const int DEFAULT_MESSAGE_WINDOW_WIDTH_;
    static const int DEFAULT_DIALOG_HEIGHT_;
    static const int DEFAULT_DIALOG_WIDTH_;
    static const int DEFAULT_DIALOG_LEFTOFFSET_;
    static const int DEFAULT_DIALOG_BOTTOMOFFSET_;
    static const AssistantDialog_predefinedMessageFile_t MESSAGE_FILE_[];

    void loadFonts_(void);
    void createDialog_(void);
    void popupDialog_(void);
    int getPredefinedMessageFileName_(int msgIndex, string &filename);
    int loadFileContents_(string filename, char **data, int *dataSize);
    string getMessageDirectoryPath_(void);

    static void cbDialogHide_(Widget w, XtPointer client_data, XtPointer callback_data);

public:
    AssistantDialog(void);
    AssistantDialog(
        Widget parentWidget,
        const symbol_table<rc_chain> &rc_table,
        const int initPopupOption);
    ~AssistantDialog(void);
    void showMessage(int msgType, char *msg, ...);
    void showPredefinedMessage(int msgIndex);
    void clearMessage(void);
    void setAssistantDialogUsage(bool use);
    void popupDialog(void);
    bool isDiabled(void);
    bool dialogIsUp(void);
};

inline void AssistantDialog::setAssistantDialogUsage(bool use)
{
    useAssistantDialog_ = use;
}

inline void AssistantDialog::popupDialog(void)
{
    popupDialog_();
}

inline bool AssistantDialog::isDiabled(void)
{
    return (!useAssistantDialog_);
}

inline bool AssistantDialog::dialogIsUp(void)
{
    return (dialogIsUp_);
}

extern AssistantDialog *gAssistantDialog;

void cbPopupAssistantDialog(Widget w, XtPointer client_data, XtPointer callback_data);


#endif
