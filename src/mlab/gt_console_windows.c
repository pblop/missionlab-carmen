/**********************************************************************
 **                                                                  **
 **                      gt_console_windows.c                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_console_windows.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>

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

#include "gt_console_windows.h"
#include "gt_console_db.h"
#include "gt_sim.h"
#include "draw.h"
#include "console.h"
#include "file_utils.h"
#include "version.h"

Widget alert_dialog;
Widget alert_text;
Widget alert_pane, alert_form1, alert_form2;
Widget alert_label, alert_widget;
Widget help_dialog;
Widget copyright_dialog;

XmTextPosition alert_cur_position = 0;

char *copyright_str =
#include "copyright"
;

//-----------------------------------------------------------------------
// Closes alert_dialog.
//-----------------------------------------------------------------------
void close_alert_dialog(Widget widget, XtPointer client_data, XtPointer callback_data)
{
   Widget shell = (Widget) client_data;
   XtPopdown(shell);

}

//-----------------------------------------------------------------------
// This routine actually makes the widget for the Alert Message.
//-----------------------------------------------------------------------
void create_alert_dialog(Widget parent)
{
   int    n = 0;
   //int    i;
   Arg    args[10];

   // Setup a dialog shell as a popup window.
   alert_dialog = XtVaCreatePopupShell("Alert Message",
      xmDialogShellWidgetClass, parent,
      XmNdeleteResponse, XmUNMAP,
      NULL);

   // Create a PanedWindow to manage the stuff in this
   // dialog.
   alert_pane = XtVaCreateWidget("pane",
      xmPanedWindowWidgetClass, alert_dialog,
      XmNsashHeight,        1,
      XmNsashWidth,         1,
      NULL);

   // Create a RowColumn in the form for label and text
   // Widget.
   alert_form1 = XtVaCreateWidget("form1", xmFormWidgetClass, alert_pane, NULL);

   XtVaSetValues(alert_form1,
      XmNwidth,                 500,
      NULL);

   // Create a label gadget
   alert_label = XtVaCreateManagedWidget("label", xmLabelGadgetClass, alert_form1,
      XmNlabelType,             XmPIXMAP,
      XmNleftAttachment,	XmATTACH_FORM,
      XmNtopAttachment,		XmATTACH_FORM,
      XmNbottomAttachment,	XmATTACH_FORM,
      NULL);

   // Prepare the text for display in the ScrolledText object.

   XtSetArg(args[n], XmNscrollVertical,             True);              n++;
   XtSetArg(args[n], XmNscrollHorizontal,           False);             n++;
   XtSetArg(args[n], XmNeditMode,                   XmMULTI_LINE_EDIT); n++;
   XtSetArg(args[n], XmNeditable,                   False);             n++;
   XtSetArg(args[n], XmNcursorPositionVisible,      False);             n++;
   XtSetArg(args[n], XmNwordWrap,                   True);              n++;
   XtSetArg(args[n], XmNrows,                       15);                n++;
   XtSetArg(args[n], XmNcolumns,                    66);                n++;
   alert_text = XmCreateScrolledText(alert_form1, "alert-text", args, n);

   XtVaSetValues (XtParent (alert_text),
      XmNleftAttachment,	XmATTACH_WIDGET,
      XmNleftWidget,            alert_label,
      XmNtopAttachment,		XmATTACH_FORM,
      XmNrightAttachment,	XmATTACH_FORM,
      XmNbottomAttachment,	XmATTACH_FORM,
      NULL);

   // Reset cursor
   alert_cur_position = 0;

   // Create another form to act as the action area for the dialog.
   alert_form2 = XtVaCreateWidget("form2", xmFormWidgetClass, alert_pane,
      XmNfractionBase,          5,
      NULL);

   alert_widget = XtVaCreateManagedWidget("Close",
      xmPushButtonGadgetClass,	alert_form2,
      XmNtopAttachment,		XmATTACH_FORM,
      XmNbottomAttachment,	XmATTACH_FORM,
      XmNleftAttachment,	XmATTACH_POSITION,
      XmNleftPosition,	        2,
      XmNrightAttachment,	XmATTACH_POSITION,
      XmNrightPosition,	        3,
      XmNshowAsDefault,		True,
      XmNdefaultButtonShadowThickness, 1,
      NULL);
   XtAddCallback(alert_widget, XmNactivateCallback, close_alert_dialog, alert_dialog);

}


//-----------------------------------------------------------------------
//  This routine creates the help dialog.
//-----------------------------------------------------------------------
void create_help_dialog(Widget parent)
{
    Display *display = NULL;
    Window window;
    Visual *myvis = NULL;
    XFontStruct *big_font_struct;
    Font big_font;
    XGCValues xgcv;
    GC erase_gc;
    GC draw_gc;
    Widget label;
    Pixmap about_pixmap;
    Dimension msg_height, msg_width;
    Dimension width, height;
    XmString about_console;
    XmString ok;
    XmString title;
    Arg wargs[2];
    int n = 0;
    int label_width;
    char *labelString = NULL;
    char about_console_str[10000];
    char *about_console_fmt = NULL;
    int bg, fg;
    int scr;

    ok = XSTRING("   OK   ");
    title = XSTRING("Help: About");

    about_console_fmt = "                version %s\n\
\n\
Compiled %s\n\
\n\
\n\
Director: \n\
\n\
Ronald C. Arkin \n\
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
Copyright (c) 1995 - 2006 \n\
ALL RIGHTS RESERVED.  \n\
See [Copyright] under the [Help] menu for \n\
details. \n";

    // create the basic dialog
    XtSetArg(wargs[n], XmNokLabelString, ok);   n++;
    XtSetArg(wargs[n], XmNdialogTitle, title);  n++;
    help_dialog = XmCreateMessageDialog(parent, "Help", wargs, n);
    XmStringFree(ok);

    // get rid of the cancel and help buttons
    XtUnmanageChild(XmMessageBoxGetChild(help_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(help_dialog, XmDIALOG_HELP_BUTTON));

    sprintf(
        about_console_str,
        about_console_fmt,
        version_str,
        compile_time());

    about_console = XSTRING(about_console_str);
    XmStringExtent(
        default_fonts,
        about_console,
        &msg_width,
        &msg_height);
    width = msg_width;
    height = msg_height + 30;

    // create a pixmap for the message label
    display = XtDisplay(help_dialog);
    window = RootWindowOfScreen(XtScreen(help_dialog));
    about_pixmap = XCreatePixmap(
        display,
        RootWindowOfScreen(XtScreen(help_dialog)),
        width,
        height,
        DefaultDepthOfScreen(XtScreen(help_dialog)));

    // create the GC's
    scr = DefaultScreen(display);
    myvis = DefaultVisual(display,scr);

    if (myvis->map_entries > 2)
    {
        XtVaGetValues(
            help_dialog,
            XmNforeground, &fg,
            XmNbackground, &bg,
            NULL);
    }
    else
    {
        fg = BlackPixel(display, scr);
        bg = WhitePixel(display, scr);
    }

    xgcv.foreground = bg;
    xgcv.background = bg;
    erase_gc = XtGetGC(help_dialog, GCForeground | GCBackground, &xgcv);
    xgcv.foreground = fg;
    xgcv.background = bg;
    draw_gc = XtGetGC(help_dialog, GCForeground | GCBackground, &xgcv);

    // clear the pixmap
    XFillRectangle(display, about_pixmap, erase_gc, 0, 0, width, height);

    // Create the big font
    big_font_struct = XLoadQueryFont(display, "-*-*-bold-i-*-*-24-*-*-*-*-*-*-*");

    if (big_font_struct == NULL)
    {
        big_font_struct = XLoadQueryFont(display, "-*-*-bold-i-*-*-24-*-*-*-*-*-*-*");
    }

    if (big_font_struct == NULL)
    {
        // Set up the default font and its structure
        big_font = default_font;
        XGCValues *theGCValues = (XGCValues *) malloc(sizeof(XGCValues));
        XGetGCValues(display, draw_gc, GCFont, theGCValues);
        XID this_font = theGCValues->font;
        free(theGCValues);
        big_font_struct = XQueryFont(display, this_font);
    }
    else
    {
        big_font = big_font_struct->fid;
    }

    // Write the nice (hopefully) big label
    labelString = "MissionLab";

    XSetFont(display, draw_gc, big_font);
    label_width = XTextWidth(big_font_struct, labelString, strlen(labelString));

    XDrawString(
        display,
        about_pixmap,
        draw_gc,
        (int)((width - label_width)/2),
        25,
        labelString,
        strlen(labelString));

    // Now draw the rest of the message
    XSetFont(display, draw_gc, default_font);
    XmStringDraw(
        display,
        about_pixmap,
        default_fonts,
        about_console,
        draw_gc,
        0,
        30,
        width,
        XmALIGNMENT_BEGINNING,
        XmSTRING_DIRECTION_L_TO_R,
        NULL);

    XmStringFree(about_console);

    // Now register the pixmap
    label = XmMessageBoxGetChild(help_dialog, XmDIALOG_MESSAGE_LABEL);
    XtVaSetValues(
        label,
        XmNlabelType, XmPIXMAP,
        XmNlabelPixmap, about_pixmap,
        NULL);

    // restore the system default font
    XSetFont(display, gGCs.black, default_font);
}



//-----------------------------------------------------------------------
// This routine creates the copyright dialog.
//-----------------------------------------------------------------------
void create_copyright_dialog(Widget parent)
{
   char *copyright_fmt = "MissionLab v%s\n\n%s";
   char full_copyright_str[3200];

   XmString title = XSTRING("Help: Copyright");
   XmString ok    = XSTRING("   OK   ");
   XmString copyright;
   Arg wargs[3];
   int n=0;

   // Initialize the copyright strings
   sprintf(full_copyright_str, copyright_fmt, version_str, copyright_str);
   copyright = XSTRING(full_copyright_str);

   // create the basic dialog
   XtSetArg(wargs[n], XmNdialogTitle, title);  n++;
   XtSetArg(wargs[n], XmNokLabelString, ok);   n++;
   XtSetArg(wargs[n], XmNmessageString, copyright);   n++;
   copyright_dialog = XmCreateMessageDialog(parent, "Copyright", wargs, n);
   XmStringFree(title);
   XmStringFree(ok);
   XmStringFree(copyright);

   // get rid of the cancel and help buttons
   XtUnmanageChild(XmMessageBoxGetChild(copyright_dialog,
					XmDIALOG_CANCEL_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(copyright_dialog,
					XmDIALOG_HELP_BUTTON));
}

/**********************************************************************
 * $Log: gt_console_windows.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/07/12 09:06:41  endo
 * Preparing for MissionLab 7.0 release.
 *
 * Revision 1.4  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.3  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.3  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.2  2003/04/06 09:06:42  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.1  2002/01/12 22:54:39  endo
 * Initial revision
 *
 **********************************************************************/
