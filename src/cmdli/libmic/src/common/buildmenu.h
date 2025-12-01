#ifndef BUILDMENU_H
#define BUILDMENU_H
/**********************************************************************
 **                                                                  **
 **                            buildmenu.h                           **
 **                                                                  **
 **  Written by Dan Heller.  Copyright 1991, O'Reilly && Associates. **
 **  This file is freely distributable without licensing fees        **
 **  and is provided without guarantee or warrantee expressed or     **
 **  implied.  This file is -not- in the public domain.              **
 **********************************************************************/

/* $Id: buildmenu.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */
/**********************************************************************
 * $Log: buildmenu.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:16  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:46  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/01 00:30:29  endo
 * CMDLi from MARS 2020 migrated into AO-FNC repository.
 *
 * Revision 1.1.1.1  2006/06/29 20:42:07  endo
 * cmdli local repository.
 *
 * Revision 1.3  2004/05/11 19:34:29  doug
 * massive changes to integrate with USC and GaTech
 *
 * Revision 1.2  1998/03/18 23:04:20  doug
 * adding edit engine.
 *
 * Revision 1.1  1998/03/18 19:41:48  doug
 * Initial revision
 *
 **********************************************************************/

/* build_option.h -- The final version of BuildMenu() is used to
 * build popup, option, pulldown -and- pullright menus.  Menus are
 * defined by declaring an array of MenuItem structures as usual.
 */

#include "mic.h"
namespace sara
{

typedef std::vector<class MenuItem *> MenuItems;

class MenuItem 
{
  public:
    char        *label;         // the label for the item
    WidgetClass *cl;            // pushbutton, label, separator...
    char         mnemonic;      // mnemonic; NULL if none
    char        *accelerator;   // accelerator; NULL if none
    char        *accel_text;    // to be converted to compound string
    XtCallbackProc callback;    // routine to call; NULL if none
    XtPointer    callback_data; // client_data for callback()
    MenuItems   *subitems;      // pullright menu items, if not NULL
    Widget	 widget;	// Loaded on creation

  MenuItem(char *label, WidgetClass *cl, char mnemonic, char *accelerator,
	   char *accel_text, XtCallbackProc callback, XtPointer callback_data,
	   MenuItems *subitems);
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
Widget BuildMenu(Widget parent, int menu_type, char *menu_title, 
                 char menu_mnemonic, MenuItems *items);

/**********************************************************************/
}
#endif
