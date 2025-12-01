/**********************************************************************
 **                                                                  **
 **                            console.h                             **
 **                                                                  **
 **  Written by:  Jonathan M. Cameron                                **
 **                                                                  **
 **  Copyright 1995 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: console.h,v 1.2 2008/07/16 21:16:08 endo Exp $ */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdarg.h>
#include <X11/Intrinsic.h>
#include <Xm/Label.h>
#include <string>

#include "gt_map.h"
#include "load_rc.h"

using std::string;

void PumpEventQueue();

void EnablePauseControls( bool bEnable );
void ShowMovementFieldMenuItemEnabled( bool bEnabled );

void add_workproc( XtWorkProc proc, XtPointer client_data );

void get_proceed_clearance( int* flag, int timeout_given, int timeout, char *fmt, ... );

int popup_telop_interface(void);
int popup_start_proceed_dialog( int type );

void set_show_trails( int flag );
void set_circle_robots( int flag );
void set_show_repelling_obstacles( int flag );
void set_show_movement_vectors( int flag );
void set_debug_robots( int flag );
void set_debug_simulator( int flag );
void set_debug_scheduler( int flag );
void use_military_style(int flag);

void UpdateLMParamsFor( int iRobotID );
void DeleteLMParamDialog( Widget dialog );
void GetNextRobotForLMParamDialog( Widget dialog );
void ResetWorld( char* filename, int load_new_map );
void UpdateGPS (int iRobotID);

Pixel getPixelByColorName(string colorName);
GC getGCByColorName(string colorName, bool isXOR = false);

#define MIN_CYCLE_DURATION 0.1
#define MAX_CYCLE_DURATION 1.0

#define TYPE_A 'a'
#define TYPE_B 'b'

int set_cycle_duration( double dt );

void set_file_open_dialog_filename( char* filename );

void draw_scale_ruler();

void clear_simulation();

void clear_drawing_area();
void clear_map();

void warn_user( char* msg);
void warn_userf( const char* fmt, ... );
void warn_once( const char* fmt, ... );

void report_user( char* msg );
void report_userf( const char* fmt, ... );

void alert_user( char* msg );
void alert_userf( const char* fmt, ...);

extern class symbol_table<rc_chain> rc_table;

void scroll_drawing_area_to_bottom();

// some globals defined in console.c
extern Widget drawing_area;
extern Pixmap drawing_area_pixmap;

extern bool monochrome;

extern Colormap mlab_colormap;

extern Display* display;

extern Widget main_window;

extern XFontStruct* default_font_struct;
extern Font default_font;
extern XmFontList default_fonts;

#define FIRST_GRAY_COLORMAP_ENTRY 100
#define MAX_GRAY 100

extern unsigned long gray[];

void set_show_colormap( int flag );

extern unsigned int drawing_area_width_pixels;
extern unsigned int drawing_area_height_pixels;

extern double mission_area_width_meters;
extern double mission_area_height_meters;

void set_mission_area_size( double width, double height );   // in meters

extern Map_info* gMapInfo;

void set_busy_cursor();
void reset_cursor();

//Nadeem - callback for the virtual clock
void gStartClock(Widget widget, XtIntervalId* id);

enum T_console_error
{
	NONE,
	ROBOT_VERSION_MISMATCH
};

extern T_console_error console_error;
extern char* console_error_messages[];

#ifdef linux
#define nint(a) ((a)<0 ? -1 : 1) * (int)(fabs(a)+0.5)
#endif

enum OBJ_STYLES
{
    STYLE_FIXED, STYLE_MOVABLE, STYLE_CONTAINER
};

// If you add a new entry, make sure to add the corresponding entry
// in PREDEFINED_PIXEL_COLORNAMES below.
typedef struct ColorPixel {
    Pixel black;
    Pixel white;
    Pixel red;
    Pixel green;
    Pixel green4;
    Pixel blue;
    Pixel yellow;
    Pixel magenta;
    Pixel cyan;
    Pixel orange;
    Pixel gray80;
};
extern ColorPixel gColorPixel;

struct XColors_t {
    XColor black;
    XColor white;
};
extern XColors_t gXColors;

typedef struct GCTypes_t
{
    GC XOR;
    GC whiteXOR;
    GC redXOR;
    GC greenXOR;
    GC attractor;
    GC basket;
    GC robot;
    GC black;
    GC green;
    GC erase;
};
extern GCTypes_t gGCs;

extern int scroll_bar_v_val;
extern int scroll_bar_h_val;


#endif  // CONSOLE_H

/**********************************************************************
 * $Log: console.h,v $
 * Revision 1.2  2008/07/16 21:16:08  endo
 * Military style is now a toggle option.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.5  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.4  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.3  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.2  2006/09/26 19:29:57  nadeem
 * Added a mockup virtual clock display. Right now the clock display can be speeded up to 10 times
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.6  2006/07/11 10:07:49  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.5  2006/06/08 22:52:13  endo
 * CommBehavior from MARS 2020 migrated.
 *
 * Revision 1.4  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.3  2006/01/30 02:43:16  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:12  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.45  2003/04/02 23:57:41  endo
 * *** empty log message ***
 *
 * Revision 1.44  2003/04/02 22:05:54  zkira
 * Telop GUI modifications
 *
 * Revision 1.43  2002/10/31 21:36:37  ebeowulf
 * Added the code to reset the simulation environment.
 *
 * Revision 1.42  2002/07/02 20:42:52  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.41  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.40  2001/11/13 15:01:08  endo
 * The problem of mlab not being able to display a robot with a right color
 * was fixed by painting color XOR GC regions with a white XOR GC.
 *
 * Revision 1.39  2000/11/10 23:39:42  blee
 * added flags for learning momentum and saving of sensor readings
 *
 * Revision 1.38  2000/08/15 20:55:17  endo
 * popup_start_proceed_dialog added.
 *
 * Revision 1.37  2000/02/18 03:14:09  endo
 * alert_user, alert_userf added.
 *
 * Revision 1.36  1999/12/18 10:06:08  endo
 * report_user, report_userf added.
 *
 * Revision 1.35  1999/04/27 02:52:06  conrad
 * Added display to externals.
 *
 * Revision 1.34  1997/02/12  05:41:58  zchen
 * *** empty log message ***
 *
 * Revision 1.33  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.32  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.31  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.30  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.29  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.28  1995/07/06  21:03:41  jmc
 * Changed MAX_GRAY to 100 (to use X-11 graynn names).
 *
 * Revision 1.27  1995/06/14  16:39:23  jmc
 * Added function prototype for set_show_colormap().
 *
 * Revision 1.26  1995/06/13  18:18:45  jmc
 * Added the functions for displaying a busy cursor and resetting it.
 *
 * Revision 1.25  1995/06/09  21:35:12  jmc
 * Added "map" and grayscale declarations.  Cleaned up a little.
 *
 * Revision 1.24  1995/05/04  22:09:08  jmc
 * Moved scale related stuff to gt_scale.* and gt_show_trails to
 * draw.*.
 *
 * Revision 1.23  1995/05/04  20:00:46  jmc
 * Changed the definition for set_cycle_duration to return an int
 * (boolean).
 *
 * Revision 1.22  1995/05/02  21:28:33  jmc
 * Converted grid_* variables to mission_area_* and drawing_area_*
 * variables.  Added set_mission_area_size command.  Moved the
 * declaration of set_meters_per_pixel here (from gt_world.h).
 *
 * Revision 1.21  1995/04/28  17:29:12  jmc
 * Renamed pixmap variable to drawing_area_pixmap.
 *
 * Revision 1.20  1995/04/27  19:27:54  jmc
 * Added function prototype for clear_drawing_area() and clear_map().
 * Moved declarations for MIN/MAX_METERS_PER_PIXEL from gt_world.h to
 * here and converted them to lowercase doubles min_meters_per_pixel
 * and max_meters_per_pixel.  Converted GRID_HEIGHT/WIDTH_METERS
 * variable names to lower case.
 *
 * Revision 1.19  1995/04/26  13:35:44  jmc
 * Added function prototype for set_file_open_dialog_filename().
 *
 * Revision 1.18  1995/04/20  14:49:36  jmc
 * Corrected spelling.
 *
 * Revision 1.17  1995/04/19  18:47:42  jmc
 * Added function prototype for scroll_drawing_area_to_bottom().
 *
 * Revision 1.16  1995/04/10  17:14:04  jmc
 * Added ifndef CONSOLE_H protection to avoid double inclusion of
 * console.h.  Moved declarations for GCs and GRID_*, grid_*, and
 * meters_per_pixels to here from various locations (gt_sim.h, etc).
 *
 * Revision 1.15  1995/04/03  18:04:27  jmc
 * Added copyright notice.
 *
 * Revision 1.14  1995/02/02  22:52:49  jmc
 * Added function prototype for set_cycle_duration.  Moved
 * MIN/MAX_CYCLE_DURATION from console.c to here (they are needed in
 * gt_command_yac.y).
 *
 * Revision 1.13  1995/02/02  16:42:03  jmc
 * Added function prototype for set_show_movement_vectors.
 *
 * Revision 1.12  1995/01/30  15:45:20  doug
 * porting to linux
 *
 * Revision 1.11  1994/11/11  22:40:29  jmc
 * Changed simulation_clear to clear_simulation.
 *
 * Revision 1.10  1994/11/04  18:34:36  jmc
 * Added a function prototype for draw_scale_ruler.
 *
 * Revision 1.9  1994/11/04  15:26:10  jmc
 * Added function prototype for set_debug_scheduler.
 *
 * Revision 1.8  1994/11/03  18:46:01  jmc
 * Added function prototype for set_show_repelling_obstacles.
 *
 * Revision 1.7  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.6  1994/10/28  20:15:35  jmc
 * Added timeout_given and timeout to get_proceed_clearance function
 * prototype.
 *
 * Revision 1.5  1994/09/02  20:02:52  jmc
 * Added function prototype for add_workproc.
 *
 * Revision 1.4  1994/08/26  15:59:16  jmc
 * Added function prototypes for setting parameters.
 *
 * Revision 1.3  1994/08/23  16:01:29  jmc
 * Added the get_proceed_clearance function prototype.
 *
 * Revision 1.2  1994/08/04  14:48:37  jmc
 * Removed unnecessary extern's from function prototypes.
 *
 * Revision 1.1  1994/08/04  13:38:43  jmc
 * Initial revision
 **********************************************************************/
