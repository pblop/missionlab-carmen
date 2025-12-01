/**********************************************************************
 **                                                                  **
 **                          buildbitmaps.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: buildbitmaps.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */



/* Buildbitmaps from include files */

extern Pixmap pm_down;
extern Pixmap pm_fsa;
extern Pixmap pm_kill;
extern Pixmap pm_kill_connection;
extern Pixmap pm_text;
extern Pixmap pm_up;
extern Pixmap pm_bind;
extern Pixmap ispm_bind;
extern Pixmap pm_unbind;
extern Pixmap ispm_unbind;
extern Pixmap pm_group;
extern Pixmap pm_edit;
extern Pixmap pm_show_detail;
extern Pixmap pm_new_state;
extern Pixmap ispm_new_state;
extern Pixmap pm_new_trans;
extern Pixmap ispm_new_trans;
extern Pixmap pm_comment;
extern Pixmap pm_move;
extern Pixmap pm_linked;
extern Pixmap pm_blank;
extern Pixmap pm_output;
extern Pixmap ispm_output;

extern Pixmap pm_copy;
extern Pixmap ispm_copy;
extern Pixmap pm_cut;
extern Pixmap ispm_cut;
extern Pixmap pm_dup;
extern Pixmap ispm_dup;
extern Pixmap pm_paste;
extern Pixmap ispm_paste;
extern Pixmap pm_link;
extern Pixmap ispm_link;

extern Cursor arrow_cursor;
extern Cursor wait_cursor; 
extern Cursor kill_cursor;
extern Cursor detail_cursor;
extern Cursor trans_cursor;
extern Cursor connect_cursor;
extern Cursor hand_cursor;
extern Cursor pick_const_cursor;
extern Cursor cvt_input_cursor;

void BuildBitMaps( Pixel fg, Pixel bg, Pixel red_pixel);



/**********************************************************************
 * $Log: buildbitmaps.h,v $
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
 * Revision 1.19  1997/02/14  16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.18  1996/02/16  00:07:18  doug
 * *** empty log message ***
 *
 * Revision 1.17  1996/02/08  19:21:56  doug
 * *** empty log message ***
 *
 * Revision 1.16  1996/02/06  18:16:42  doug
 * *** empty log message ***
 *
 * Revision 1.15  1996/02/04  17:11:48  doug
 * *** empty log message ***
 *
 * Revision 1.14  1996/02/02  03:00:09  doug
 * getting closer
 *
 * Revision 1.13  1996/01/31  03:06:53  doug
 * *** empty log message ***
 *
 * Revision 1.12  1996/01/29  00:08:38  doug
 * *** empty log message ***
 *
 * Revision 1.11  1996/01/21  20:13:22  doug
 * *** empty log message ***
 *
 * Revision 1.10  1995/09/26  21:42:09  doug
 * added group pixmap
 *
 * Revision 1.9  1995/09/07  14:22:26  doug
 * works
 *
 * Revision 1.8  1995/08/01  21:47:45  doug
 * Add red color parm so kill button can use it
 *
 * Revision 1.7  1995/06/29  16:50:38  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
