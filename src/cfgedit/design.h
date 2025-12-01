/**********************************************************************
 **                                                                  **
 **                             design.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995 - 2005 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: design.h,v 1.1.1.1 2008/07/14 16:44:15 endo Exp $ */


#ifndef DESIGN_H
#define DESIGN_H

#include <Xm/Xm.h>
#include <string>

using std::string;

#define WINDOW_WIDTH    1100    
#define WINDOW_HEIGHT   800   

#define DRAWING_WIDTH   3000
//#define DRAWING_HEIGHT  ((int)(DRAWING_WIDTH * 0.77272727)) // 8.5 x 11 aspect ratio
#define DRAWING_HEIGHT  ((int)(DRAWING_WIDTH * 10))

extern bool  place_constant;
extern bool  copy_object;
extern bool  push_up_input;

extern Widget drawing_area;
extern Widget cur_page_label;
extern Widget cur_arch_label;
extern Widget cur_file_label;

extern Widget up_button;
extern Widget run_button;

extern char  *configuration_name;
extern Widget top_level;
extern bool verbose;

void reset_placement_list(void);                    
void reset_file_save(Boolean val);

void kill_node_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct *fcb);

void reset_binding_list(void);
void change_filename(char *filename);
void open_cdl_file(string filename);
void AddSystemGrabs(void);


extern XtTranslations mouse_translations;
extern Pixel blue_pixel;
extern Pixel insensitive_pixel;

// Table of architecture names and values
struct architecture_tbl
{
   char *name;
   unsigned long bit;
};

extern struct architecture_tbl architecture_table[];

#define ARCH_AuRA_BIT (1<<0)
#define ARCH_AuRA_urban_BIT  (1<<1)
#define ARCH_AuRA_naval_BIT  (1<<2)
#define ARCH_UGV_BIT  (1<<3)
extern unsigned long name_visibility;

#endif


/**********************************************************************
 * $Log: design.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:15  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:54  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/02/14 02:27:18  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.5  2006/01/30 02:47:27  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.4  2005/08/09 19:12:44  endo
 * Things improved for the August demo.
 *
 * Revision 1.3  2005/05/18 21:14:43  endo
 * AuRA.naval added.
 *
 * Revision 1.2  2005/02/07 22:25:27  endo
 * Mods for usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 22:59:32  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.52  1999/09/03 19:45:42  endo
 * *** empty log message ***
 *
 * Revision 1.51  1999/09/03 19:43:33  endo
 * ARCH_AuRA_urban_BIT added.
 *
 * Revision 1.50  1997/02/14 16:42:11  zchen
 * *** empty log message ***
 *
 * Revision 1.49  1996/05/02  22:58:26  doug
 * *** empty log message ***
 *
 * Revision 1.48  1996/02/29  01:48:49  doug
 * *** empty log message ***
 *
 * Revision 1.47  1996/02/25  01:13:14  doug
 * *** empty log message ***
 *
 * Revision 1.46  1996/02/16  00:07:18  doug
 * *** empty log message ***
 *
 * Revision 1.45  1996/02/06  18:16:42  doug
 * *** empty log message ***
 *
 * Revision 1.44  1996/02/04  17:11:48  doug
 * *** empty log message ***
 *
 * Revision 1.43  1996/02/01  04:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.42  1996/01/30  01:50:19  doug
 * *** empty log message ***
 *
 * Revision 1.41  1996/01/29  00:08:38  doug
 * *** empty log message ***
 *
 * Revision 1.40  1995/11/30  23:31:07  doug
 * *** empty log message ***
 *
 * Revision 1.39  1995/10/30  23:06:43  doug
 * *** empty log message ***
 *
 * Revision 1.38  1995/09/19  15:33:19  doug
 * The executive module is now working
 *
 * Revision 1.37  1995/09/07  14:22:26  doug
 * works
 *
 * Revision 1.36  1995/09/01  16:14:56  doug
 * Able to run double wander config
 *
 * Revision 1.35  1995/06/29  17:39:53  jmc
 * Added header and RCS id and log strings.
 **********************************************************************/
