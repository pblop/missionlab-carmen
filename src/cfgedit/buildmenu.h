/**********************************************************************
 **                                                                  **
 **                            buildmenu.h                           **
 **                                                                  **
 **  Written by Dan Heller.  Copyright 1991, O'Reilly && Associates. **
 **  This file is freely distributable without licensing fees        **
 **  and is provided without guarantee or warrantee expressed or     **
 **  implied.  This file is -not- in the public domain.              **
 **                                                                  **
 **  Modfied by Douglas MacKenzie:                                   **
 **    Stripped out the buildmenu function                           **
 **    Modified it to save the widgets for the individual buttons    **
 **                                                                  **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: buildmenu.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef BUILDMENU_H
#define BUILDMENU_H


/* build_option.h -- The final version of BuildMenu() is used to
 * build popup, option, pulldown -and- pullright menus.  Menus are
 * defined by declaring an array of MenuItem structures as usual.
 */

struct MenuItem {
    char        *label;         // the label for the item
    WidgetClass *cl;            // pushbutton, label, separator...
    char         mnemonic;      // mnemonic; NULL if none
    char        *accelerator;   // accelerator; NULL if none
    char        *accel_text;    // to be converted to compound string
    XtCallbackProc callback;    // routine to call; NULL if none
    XtPointer    callback_data; // client_data for callback()
    MenuItem    *subitems;      // pullright menu items, if not NULL
    Widget	 widget;	// Loaded on creation

/*
    MenuItem()
    {
       label = NULL;
       cl = NULL;
       mnemonic = '\0';
       accelerator = NULL;
       accel_text = NULL;
       callback = NULL;
       callback_data = NULL;
       subitems = NULL;
    }
*/
};

/* Build popup, option and pulldown menus, depending on the menu_type.
 * It may be XmMENU_PULLDOWN, XmMENU_OPTION or  XmMENU_POPUP.  Pulldowns
 * return the CascadeButton that pops up the menu.  Popups return the menu.
 * Option menus are created, but the RowColumn that acts as the option
 * "area" is returned unmanaged. (The user must manage it.)
 * Pulldown menus are built from cascade buttons, so this function
 * also builds pullright menus.  The function also adds the right
 * callback for PushButton or ToggleButton menu items.
 */
Widget
BuildMenu(Widget parent, int menu_type, char *menu_title, char menu_mnemonic, 
MenuItem *items);

#endif



/**********************************************************************
 * $Log: buildmenu.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 22:59:32  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.6  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.5  1996/05/07  19:17:03  doug
 * fixing compile warnings
 *
 * Revision 1.4  1995/06/29  18:25:20  jmc
 * Added copyright message.
 *
 * Revision 1.3  1995/06/29  17:04:41  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
