/**********************************************************************
 **                                                                  **
 **                            buildmenu.cc                          **
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

/* $Id: buildmenu.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */



/* build_option.c -- The final version of BuildMenu() is used to
 * build popup, option, pulldown -and- pullright menus.  Menus are
 * defined by declaring an array of MenuItem structures as usual.
 */
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/DrawingA.h>
#include <Xm/CascadeBG.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

#include <stdio.h>

#include "buildmenu.h"

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
MenuItem *items)
{
    Widget menu, cascade;
    int i;
    XmString str;

    if (menu_type == XmMENU_PULLDOWN || menu_type == XmMENU_OPTION)
        menu = XmCreatePulldownMenu(parent, "_pulldown", NULL, 0);
    else if (menu_type == XmMENU_POPUP)
        menu = XmCreatePopupMenu(parent, "_popup", NULL, 0);
    else {
        XtWarning("Invalid menu type passed to BuildMenu()");
        return NULL;
    }

    /* Pulldown menus require a cascade button to be made */
    if (menu_type == XmMENU_PULLDOWN) {
        str = XmStringCreateLocalized(menu_title);
        cascade = XtVaCreateManagedWidget(menu_title,
            xmCascadeButtonGadgetClass, parent,
            XmNsubMenuId,   menu,
            XmNlabelString, str,
            XmNmnemonic,    menu_mnemonic,
            NULL);
        XmStringFree(str);
    } else if (menu_type == XmMENU_OPTION) {
        /* Option menus are a special case, but not hard to handle */
        Arg args[2];
        str = XmStringCreateLocalized(menu_title);
        XtSetArg(args[0], XmNsubMenuId, menu);
        XtSetArg(args[1], XmNlabelString, str);
        /* This really isn't a cascade, but this is the widget handle
         * we're going to return at the end of the function.
         */
        cascade = XmCreateOptionMenu(parent, menu_title, args, 2);
        XmStringFree(str);
    }

    /* Now add the menu items */
    for (i = 0; items[i].label != NULL; i++) 
    {
        /* If subitems exist, create the pull-right menu by calling this
         * function recursively.  Since the function returns a cascade
         * button, the widget returned is used..
         */
        if (items[i].subitems)
	{
            if (menu_type == XmMENU_OPTION) 
	    {
                XtWarning("You can't have submenus from option menu items.");
                continue;
            } 
	    else
	    {
                items[i].widget = BuildMenu(menu, XmMENU_PULLDOWN,
                    items[i].label, items[i].mnemonic, items[i].subitems);
	    }
	}
        else
	{
/*
            items[i].widget = XtVaCreateManagedWidget(items[i].label,
                *items[i].cl, menu,
                NULL);
*/
            items[i].widget = XtCreateManagedWidget(items[i].label,
                *items[i].cl, menu, NULL, 0);
	}
	 
	bool is_toggle_button = 
                items[i].cl == &xmToggleButtonWidgetClass ||
                items[i].cl == &xmToggleButtonGadgetClass;

        /* Whether the item is a real item or a cascade button with a
         * menu, it can still have a mnemonic.
         */
        if (items[i].mnemonic)
           XtVaSetValues(items[i].widget, XmNmnemonic, items[i].mnemonic, NULL);

        // Make toggle buttons visible when off by default
        if (is_toggle_button)
           XtVaSetValues(items[i].widget, XmNvisibleWhenOff, true, NULL);


        /* any item can have an accelerator, except cascade menus. But,
         * we don't worry about that; we know better in our declarations.
         */
        if (items[i].accelerator) {
            str = XmStringCreateLocalized(items[i].accel_text);
            XtVaSetValues(items[i].widget,
                XmNaccelerator, items[i].accelerator,
                XmNacceleratorText, str,
                NULL);
            XmStringFree(str);
        }

        if (items[i].callback)
	{
            XtAddCallback(items[i].widget, 
		    is_toggle_button ? 
                    XmNvalueChangedCallback : /* ToggleButton class */
                    XmNactivateCallback,      /* PushButton class */
                items[i].callback, items[i].callback_data);
        }
    }

    /* for popup menus, just return the menu; pulldown menus, return
     * the cascade button; option menus, return the thing returned
     * from XmCreateOptionMenu().  This isn't a menu, or a cascade button!
     */
    return menu_type == XmMENU_POPUP? menu : cascade;
}



///////////////////////////////////////////////////////////////////////
// $Log: buildmenu.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.6  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.5  1996/05/02  22:58:26  doug
// *** empty log message ***
//
// Revision 1.4  1995/09/19  15:33:19  doug
// The executive module is now working
//
// Revision 1.3  1995/06/29  18:27:05  jmc
// Added copyright message.
//
// Revision 1.2  1995/06/29  16:58:14  jmc
// Added header and RCS id and log strings.
///////////////////////////////////////////////////////////////////////
