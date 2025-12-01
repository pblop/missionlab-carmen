/**********************************************************************
 **                                                                  **
 **                      gt_console_windows.h                        **
 **                                                                  **
 **  Written by:  Yoichiro Endo                                      **
 **                                                                  **
 **  Copyright 2002 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_console_windows.h,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#ifndef GT_CONSOLE_WINDOWS_H
#define GT_CONSOLE_WINDOWS_H

#define RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)

extern Widget alert_dialog;
extern Widget alert_text;
extern Widget alert_pane, alert_form1, alert_form2;
extern Widget alert_label, alert_widget;
extern Widget help_dialog;
extern Widget copyright_dialog;

extern XmTextPosition alert_cur_position;

extern char *copyright_str;

void close_alert_dialog(Widget widget, XtPointer client_data, XtPointer callback_data);
void create_alert_dialog(Widget parent);
void create_help_dialog(Widget parent);
void create_copyright_dialog(Widget parent);

#endif

/**********************************************************************
 * $Log: gt_console_windows.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2002/01/12 22:54:39  endo
 * Initial revision
 *
 **********************************************************************/
