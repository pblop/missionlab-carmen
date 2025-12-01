/**********************************************************************
 **                                                                  **
 **                          buildbitmaps.cc                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: buildbitmaps.cc,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


/* Buildbitmaps from include files */

#include <stdio.h>
#include <malloc.h>
#include <iostream>

#include <X11/Xlib.h>
#include <X11/cursorfont.h>
#include <Xm/Xm.h>

/******** Include the bitmaps ********/
#include "bitmaps/down.bit"
#include "bitmaps/kill.bit"
#include "bitmaps/kill_connection.bit"
#include "bitmaps/text.bit"
#include "bitmaps/up.bit"
#include "bitmaps/fsa.bit"
#include "bitmaps/bind.bit"
#include "bitmaps/unbind.bit"
#include "bitmaps/group.bit"
#include "bitmaps/edit.bit"
#include "bitmaps/show_detail.bit"
#include "bitmaps/new_state.bit"
#include "bitmaps/new_trans.bit"
#include "bitmaps/detail_cursor.bit"
#include "bitmaps/detail_cursor_mask.bit"
#include "bitmaps/trans_cursor.bit"
#include "bitmaps/trans_cursor_mask.bit"
#include "bitmaps/comment.bit"
#include "bitmaps/move.bit"
#include "bitmaps/linked.bit"
#include "bitmaps/blank.bit"
#include "bitmaps/connect_cursor.bit"
#include "bitmaps/connect_cursor_mask.bit"
#include "bitmaps/output.bit"
#include "bitmaps/cvt_input_cursor.bit"
#include "bitmaps/cvt_input_cursor_mask.bit"

#include "bitmaps/copy.bit"
#include "bitmaps/cut.bit"
#include "bitmaps/dup.bit"
#include "bitmaps/paste.bit"
#include "bitmaps/link.bit"

#include "design.h"
#include "popups.h"
#include "help.h"
#include "load_cdl.h"
#include "configuration.hpp"
#include "globals.h"
#include "buildbitmaps.h"

Pixmap pm_down;
Pixmap pm_fsa;
Pixmap pm_kill;
Pixmap pm_kill_connection;
Pixmap pm_text;
Pixmap pm_up;
Pixmap pm_bind;
Pixmap ispm_bind;
Pixmap pm_unbind;
Pixmap ispm_unbind;
Pixmap pm_group;
Pixmap pm_edit;
Pixmap pm_show_detail;
Pixmap pm_comment;  
Pixmap pm_move;
Pixmap pm_linked;
Pixmap pm_blank;

Pixmap pm_copy;
Pixmap ispm_copy;
Pixmap pm_cut;  
Pixmap ispm_cut;  
Pixmap pm_dup;  
Pixmap ispm_dup;  
Pixmap pm_paste;
Pixmap ispm_paste;
Pixmap pm_link;
Pixmap ispm_link;



Pixmap pm_new_state;
Pixmap ispm_new_state;
Pixmap pm_new_trans;
Pixmap ispm_new_trans;
Pixmap pm_output;
Pixmap ispm_output;

Cursor arrow_cursor;
Cursor wait_cursor;
Cursor kill_cursor;
Cursor detail_cursor;
Cursor trans_cursor;
Cursor connect_cursor;
Cursor hand_cursor;
Cursor pick_const_cursor;
Cursor cvt_input_cursor;

#define create_ispm(name) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	name##_bits, name##_width, name##_height, gCfgEditPixels.insensitive, bg, \
	DefaultDepthOfScreen(XtScreen(top_level)))

#define create_pm(name) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	name##_bits, name##_width, name##_height, fg, bg, \
	DefaultDepthOfScreen(XtScreen(top_level)))

#define create_colored_pm(name,fg,bg) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	name##_bits, name##_width, name##_height, fg, bg, \
	DefaultDepthOfScreen(XtScreen(top_level)))

#define cursor_pm(name) XCreatePixmapFromBitmapData(XtDisplay(top_level),\
	RootWindowOfScreen(XtScreen(top_level)),\
	name##_bits, name##_width, name##_height, 1, 0, 1)

#define create_cursor(name,mask) XCreatePixmapCursor(XtDisplay(top_level),\
		  cursor_pm(name), cursor_pm(mask), \
		  &black_color, &white_color, \
		  name##_x_hot, name##_y_hot)


void
BuildBitMaps( Pixel fg, Pixel bg, Pixel red_pixel)
{
   // Create pixmaps for the push buttons
   pm_down = create_pm(down);
   pm_comment = create_pm(comment);
   pm_move = create_pm(move);
   pm_fsa = create_pm(fsa);
   pm_kill = create_pm(kill);
   pm_text = create_pm(text);
   pm_up = create_pm(up);
   pm_group = create_pm(group);
   pm_edit = create_pm(edit);
   pm_show_detail = create_pm(show_detail);
   pm_blank = create_pm(blank);

   pm_copy = create_pm(copy);
   ispm_copy = create_ispm(copy);
   pm_cut = create_pm(cut);
   ispm_cut = create_ispm(cut);
   pm_dup = create_pm(dup);
   ispm_dup = create_ispm(dup);
   pm_paste = create_pm(paste);
   ispm_paste = create_ispm(paste);
   pm_link = create_pm(link);
   ispm_link = create_ispm(link);

   pm_new_state = create_pm(new_state);
   ispm_new_state = create_ispm(new_state);
   pm_new_trans = create_pm(new_trans);
   ispm_new_trans = create_ispm(new_trans);
   pm_bind = create_pm(bind);
   ispm_bind = create_ispm(bind);
   pm_unbind = create_pm(unbind);
   ispm_unbind = create_ispm(unbind);
   pm_output = create_pm(output);
   ispm_output = create_ispm(output);

   pm_linked = create_colored_pm(linked, gCfgEditPixels.blue, bg);
   pm_kill_connection = create_colored_pm(kill_connection,red_pixel,bg);


   // Create bitmap cursors 
   detail_cursor = create_cursor(detail_cursor, detail_cursor_mask);
   trans_cursor = create_cursor(trans_cursor, trans_cursor_mask);
   connect_cursor = create_cursor(connect_cursor, connect_cursor_mask);
   cvt_input_cursor = create_cursor(cvt_input_cursor, cvt_input_cursor_mask);

   // Allocate font cursors.
   arrow_cursor = XCreateFontCursor(XtDisplay(top_level), XC_left_ptr);
   wait_cursor = XCreateFontCursor(XtDisplay(top_level), XC_watch);
   kill_cursor = XCreateFontCursor(XtDisplay(top_level), XC_pirate);
   hand_cursor = XCreateFontCursor(XtDisplay(top_level), XC_hand2);
   pick_const_cursor = XCreateFontCursor(XtDisplay(top_level), XC_hand1);
}


///////////////////////////////////////////////////////////////////////
// $Log: buildbitmaps.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:15  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:45  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:54  endo
// MissionLab 7.0
//
// Revision 1.2  2005/02/07 22:25:25  endo
// Mods for usability-2004
//
// Revision 1.1.1.1  2005/02/06 22:59:32  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.21  2003/04/06 12:25:34  endo
// gcc 3.1.1
//
// Revision 1.20  1997/02/14  16:53:35  zchen
// *** empty log message ***
//
// Revision 1.19  1996/02/15  23:53:50  doug
// *** empty log message ***
//
// Revision 1.18  1996/02/08  19:21:56  doug
// *** empty log message ***
//
// Revision 1.17  1996/02/06  18:16:42  doug
// *** empty log message ***
//
// Revision 1.16  1996/02/04  17:11:48  doug
// *** empty log message ***
//
// Revision 1.15  1996/02/02  03:00:09  doug
// getting closer
//
// Revision 1.14  1996/01/31  03:06:53  doug
// *** empty log message ***
//
// Revision 1.13  1996/01/30  01:50:19  doug
// *** empty log message ***
//
// Revision 1.12  1996/01/29  00:07:50  doug
// *** empty log message ***
//
// Revision 1.11  1996/01/21  20:13:22  doug
// *** empty log message ***
//
///////////////////////////////////////////////////////////////////////
