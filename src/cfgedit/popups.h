/**********************************************************************
 **                                                                  **
 **                             popups.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: popups.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */

#ifndef POPUPS_H
#define POPUPS_H

#include <time.h>
#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <vector>

using std::string;
using std::vector;

const int MAX_ARGS = 32;
const int MAX_CBs  = 16;
const XtPointer PASS_WIDGET  = (XtPointer)-31895;  // Weird string to match 

typedef Widget (*DIALOG_BLDR)(Widget, char *, ArgList, Cardinal);
typedef struct S_args {
      char *name;
      int64_t value;
   } S_args; 		// the arg array

typedef struct S_cbs {
      char *name;	// Name of callbacks
      XtCallbackProc proc; 	// fnc to call
      void *data;	// Client data
   } S_cbs;

typedef struct dialog_rec {
   DIALOG_BLDR func;		// Function creates and returns dialog widget
   S_args args[MAX_ARGS];	// the arg array
   S_cbs cbs[MAX_CBs];
} dialog_rec;

//extern void file_open_ok_cb(Widget w,caddr_t client_data, XmFileSelectionBoxCallbackStruct * fcb);
//extern void file_open_dialog();

void warn_user(char *msg);
void warn_userf(const char *fmt, ...);
void create_error_dialog(Widget parent);
void help_cb(Widget w, int item_num);
void create_help_dialog(Widget parent);
void open_dialog_box(Widget w, dialog_rec *rec, XmAnyCallbackStruct *cbs);
int wait_dialog_box(Widget w, dialog_rec *rec, XmAnyCallbackStruct *cbs);
int wait_SelectionBox(Widget w, dialog_rec *rec, char **result);

class PopUpWindows {

    typedef enum InitialDialogResult_t {
        IDB_CANCEL,
        IDB_NEW,
        IDB_LOAD
    };

    typedef enum OpenFileType_t {
        OPEN_FILE_CDL,
        OPEN_FILE_MEXP_DATA,
        OPEN_FILE_ACDLP2CIMXML,
        OPEN_FILE_ACDLP2CMDL,
        OPEN_FILE_ACDLP2CMDLI,
        OPEN_FILE_ACDLP2CDL,
        OPEN_FILE_CIMXML2ACDLP,
        OPEN_FILE_CMDL2ACDLP,
        OPEN_FILE_CMDLI2ACDLP,
        OPEN_FILE_CDL2ACDLP
    };

    typedef struct OpenFileDialogCallBackData_t {
        int openFileType;
    };

    typedef struct GeneralYesNoWindowCallbackData_t {
        bool result;
        PopUpWindows *popWinInstance;
    };

protected:
    XtAppContext PopWinAppContext_;
    Widget PopWinParent_;
    Widget InitialDialog_w_;
    Widget generalMessageWindow_;
    int PopWinIDBResult_;
    bool PopWinIDBDone_;
    bool generalMessageWindowIsUp_;
    bool generalYesNoWindowResult_;
    bool generalYesNoWindowDone_;

    static const struct timespec EVENT_WAITER_SLEEP_TIME_NSEC_;
    static const string EMPTY_STRING_;

    Widget createGeneralYesNoWindow_(string question);
    Widget createGeneralYesNoWindow_(const char *question);
    void createInitialDialog_(void);
    void popupInitialDialog_(void);
    void popdownGeneralMessageWindow_(void);
    void popupGeneralYesNoWindow_(Widget w);
    void sendGeneralYesNoWindowResult_(bool result);
    void sendGeneralToggleWindowResult_(int result);
    
    static void cbOpenFile_(
        Widget w,
        caddr_t clientData,
        XmFileSelectionBoxCallbackStruct *fileSelectData);
    static void cbGeneralMessageWindow_(
        Widget w,
        XtPointer client_data,
        XtPointer callback_data);

public:
    PopUpWindows(Widget parent, XtAppContext app);

    void runInitialDialog(void);
    void runOpenFileDialog(int openFileType);
    void sendInitialDialogResult(int result);
    void createAndPopupGeneralMessageWindow(string message);
    bool quitConfirm(Widget w);
    bool discardConfirm(Widget w);
    bool confirmUser(string phrase, bool useOKOnly);

    static void cbGeneralYesNoWindow_(
        Widget w,
        XtPointer client_data,
        XtPointer callback_data);
    static void cbRunOpenCDLFileDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunOpenMExpDataFileDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunACDLP2CIMXMLDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunACDLP2CMDLDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunACDLP2CMDLIDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunACDLP2CDLDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunCIMXML2ACDLPDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunCMDL2ACDLPDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunCMDLI2ACDLPDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
    static void cbRunCDL2ACDLPDialog(
        Widget w,
        XtPointer clientData,
        XtPointer callbackData);
};

void InitializeDialog_cb(Widget w, XtPointer client_data, XtPointer callback_data);

extern PopUpWindows *gPopWin;
extern bool gWarningUser;

#endif  


/**********************************************************************
 * $Log: popups.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/06/01 04:41:00  endo
 * LaunchWizard implemented.
 *
 * Revision 1.3  2006/08/02 21:37:33  endo
 * acdlp2cdl() implemented.
 *
 * Revision 1.2  2006/07/26 18:07:47  endo
 * ACDLPlus class added.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.2  2006/01/30 02:47:28  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.1.1.1  2005/02/06 22:59:33  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.14  2002/01/12 23:23:16  endo
 * Mission Expert functionality added.
 *
 * Revision 1.13  2000/10/16 19:33:40  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.12  2000/03/14 00:01:27  endo
 * The "Start Over" button was added to CfgEdit.
 *
 * Revision 1.11  1997/02/14 16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.10  1995/06/29  18:09:37  jmc
 * Added copyright message.
 *
 * Revision 1.9  1995/02/28  14:52:28  doug
 * *** empty log message ***
 *
 * Revision 1.7  1995/02/08  20:34:38  doug
 * *** empty log message ***
 *
 * Revision 1.6  1995/02/01  18:50:43  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/02/01  00:09:16  doug
 * *** empty log message ***
 *
 * Revision 1.5  1995/02/01  00:09:16  doug
 * *** empty log message ***
 *
 * Revision 1.4  1995/01/26  00:02:31  doug
 * *** empty log message ***
 *
 * Revision 1.3  1994/12/09  19:07:20  doug
 * *** empty log message ***
 *
 * Revision 1.3  1994/12/09  19:07:20  doug
 * *** empty log message ***
 *
 * Revision 1.2  1994/12/06  17:08:49  doug
 * *** empty log message ***
 *
 * Revision 1.2  1994/12/06  17:08:49  doug
 * *** empty log message ***
 *
 * Revision 1.1  1994/10/26  15:57:50  doug
 * Initial revision
 *
 **********************************************************************/
