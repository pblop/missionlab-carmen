/**********************************************************************
 **                                                                  **
 **                            console.c                             **
 **                                                                  **
 ** Written by:  Jonathan M. Cameron                                 **
 **                                                                  **
 ** Copyright 1995 - 2008 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: console.c,v 1.3 2008/07/16 21:16:08 endo Exp $ */

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include <signal.h>
#include <memory.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <getopt.h>
#include <string>
#include <algorithm>
#include <cctype>

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
#include <Xm/DialogS.h>
#include <Xm/DrawingA.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/ScrollBar.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

#ifndef NO_OPENGL
#include <GL/GLwDrawA.h>
#endif

extern "C" {
#include "compass.h"
}

#include "gt_simulation.h"
#include "gt_sim.h"
#include "gt_world.h"
#include "gt_scale.h"
#include "gt_command_panel.h"
#include "gt_load_command.h"
#include "gt_load_overlay.h"
#include "gt_console_windows.h"
#include "mission_design.h"
#include "gt_console_db.h"
#include "gt_map.h"
#include "draw.h"
#include "draw_military.h"
#include "file_utils.h"
#include "waypoint.h"
#include "lm_param_dialog.h"
#include "lmadjust.h"
#include "console.h"
#include "console_side_com.h"
#include "PickMapOverlay.h"
#include "utilities.h"
#include "cfgedit_common.h"
#include "EventLogging.h"
#include "version.h"
#include "VectorField.h"
#include "VectorFieldDlg.h"
#include "env_change_defs.h"
#include "FileOpenDlg.h"
#include "telop.h"
#include "sound_simulation.h"
#include "../telop/status_window.h"
#include "motivational_vector.h"
#include "gt_playback.h"
#include "x_utils.h"
#include "assistantDialog.h"
#include "3d_vis_wid.h"
#include "mlab_cnp.h"
#include "cnp_types.h"
#include "gt_cmdli_panel.h"
#include "jbox_mlab.h"
#include "jbox_display.h"
#include "convert_scale.h"
#include "mission_spec_wizard_types.h"
#include "bitmap_icon_displayer.h"

using std::string;
using std::transform;
using std::tolower;

//-----------------------------------------------------------------------
// Constatns
//-----------------------------------------------------------------------
#define MAXARGS 20
#define LINE_WIDTH 4
#define MAX_WORKPROCS 4
#define DEFAULT_MISSION_AREA_WIDTH_METERS  1000.0
#define DEFAULT_MISSION_AREA_HEIGHT_METERS 1000.0

const string EMPTY_STRING = "";
const long int WORKPROC_PERIOD_MSEC = 1;
const long int CMDLI_DISPLAY_UPDATE_PERIOD_MSEC = 1000;
const bool SCREEN_CAPTURE = false;

//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
typedef enum PickPoint_t
{
    PICK_POINT_OFF,
    PICK_POINT_SINGLE,
    PICK_POINT_SINGLE_GEOCOORD,
    PICK_POINT_MULTI,
    MLAB_MISSION_DESIGN
};

typedef struct SuLMViewNode
{
    SuLMViewDialog suDialog;
    struct SuLMViewNode* psuNext;
} SuLMViewNode;

//-----------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------
// X and Xm related
Display* display;
XtAppContext app;
static XtWorkProc workproc[MAX_WORKPROCS];
static GC sr_draw_gc;
static GC sr_erase_gc;
static char* default_font_name[] = {
    "-*-clean-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-courier-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-fixed-bold-r-*-*-*-*-*-*-*-*-*-*",
    "-*-fixed-bold-r-*-*-14-*-*-*-*-*-*-*",
    "-*-fixed-medium-r-*-*-14-*-*-*-*-*-*-*",
    "-*-lucidatypewriter-bold-r-*-*-14-*-*-*-*-*-*-*"};
static int num_default_font_names = 6;
XFontStruct* default_font_struct;
Font default_font;
XmFontList default_fonts;
static XmTextPosition wpr_position = 0;
static XmColorProc original_colorproc;
ColorPixel gColorPixel;
map<string, Pixel> pixelByColorNameList;
XColors_t gXColors;
GCTypes_t gGCs;
map<string, GC> gcByColorNameList;
map<string, GC> gcXORByColorNameList;

// Drawing area related
Pixmap drawing_area_pixmap; // grid_width x grid_height
Colormap mlab_colormap;
unsigned int drawing_area_width_pixels  = 1000;
unsigned int drawing_area_height_pixels = 1000;
int mlabDisplayWidth = 0;
int mlabDisplayHeight = 0;
double mission_area_width_meters  = DEFAULT_MISSION_AREA_WIDTH_METERS;
double mission_area_height_meters = DEFAULT_MISSION_AREA_HEIGHT_METERS;
unsigned long gray[MAX_GRAY + 1];
Map_info* gMapInfo = NULL;

// Widgets
Widget drawing_area;
Widget main_window;
Widget console_pause_button;
Widget console_refresh_button;
Widget console_feedback_button = NULL;
static Widget top_level;
static Widget drawing_area_scroll_bar;
static Widget drawing_area_scroll_bar_h;
static Widget time_scale_panel;
static Widget time_scale_panel_shell;
static Widget cycle_duration_scale;
static Widget virtual_clock_text;
static Widget virtual_clock_panel;
static Widget virtual_clock_panel_shell;
static Widget vclock_speed_scale;
static Widget error_dialog;
static Widget status_dialog;
static Widget status_text;
static Widget status_pane, status_form1, status_form2;
static Widget status_label, status_widget;
static Widget scale_ruler;
static Widget zoom_factor_label;
static Widget file_open_dialog;
static Widget overlay_open_dialog;
static Widget initial_copyright_dialog;
static Widget start_proceed_dialog;




// More widgets. These are for the buttons in menus the mlab console
// which users can toggle the optional features.
static Widget show_trails_button;
static Widget use_military_style_button;
static Widget circle_robots_button;
static Widget show_vectors_button;
static Widget show_movement_field;
static Widget show_obstacles_button;
static Widget erase_obstacles_button;
static Widget show_halos_button;
static Widget debug_button;
static Widget debug_robots_button;
static Widget debug_scheduler_button;
static Widget report_current_state_button;
static Widget log_robot_data_button;
static Widget laser_normal_button;
static Widget laser_linear_button;
static Widget laser_connected_button;
static Widget sonar_arc_button;
static Widget sonar_point_button;
static Widget lm_enabled_button;
static Widget save_lm_weights_button;
static Widget save_sensor_readings_button;
static Widget LOS_button;
static Widget show_3d_vis_button;

// Waypoints related
char* coordinates_filename = NULL;
char* waypoints_filename = NULL;
char* ovl_filename = NULL;
bool waypoint_design_mode = false;
point2d_t *waypts_ary = NULL;
int giNumWayPoints = 0;
int giWayptArySize = 0;
PickPoint_t Pick_Point_From_Overlay = PICK_POINT_OFF;

// MMD related
MlabMissionDesign* gMMD = NULL;
char* mlabMissionDesignIndexString;
bool gUseMlabMissionDesign = false;
string mexpRCFileName = EMPTY_STRING;

// PlayBack related
MlabPlayBack *gPlayBack = NULL;
int gPlayBackMode = MlabPlayBack::MPB_DISABLED;

// AssistantDialog related
AssistantDialog *gAssistantDialog = NULL;
int initPopupAssistantDialogOption = ASSISTANT_INIT_POPUP_DISABLE;
bool doubleBuffer;

#ifndef NO_OPENGL
GLXContext glcontext_3dvis;
Display *display_3dvis = NULL;
XVisualInfo *visualinfo_3dvis = NULL;
// 3d Visualization variable
int config_3dvis[] = {
    None, None, // Space for multisampling GLX attributes if supported.
    GLX_DOUBLEBUFFER, GLX_RGBA, GLX_DEPTH_SIZE, 12,
    GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1,
    None
};
int *dblBuf_3dvis = &config_3dvis[2];
int *snglBuf_3dvis = &config_3dvis[3];
#endif

Widget glxarea;
PMouse *Mouse_3d = NULL;
bool gt_show_3d_vis = false;
GLfloat LightAmbient[]= {0.75, 0.75, 0.75, 1.0};
GLfloat LightDiffuse[]= {0.5, 0.5, 0.5, 1.0};
//GLfloat LightPosition[]= {0.5, -1.0, 0.5, 1.0};
GLfloat LightPosition[]= {10000.0, 10000.0, 0.0, 1.0};
GLfloat LightSpecular[]= {1.0, 1.0, 1.0, 1.0};
GLfloat LightShininess[]= {100.0, 100.0, 100.0, 100.0};
GLfloat LightEmission[]= {1.0, 1.0, 1.0, 1.0};
const float VIS3D_INIT_ROT_X = 90.0;
const float VIS3D_INIT_ROT_Y = 0.0;
const float VIS3D_INIT_ROT_Z = 0.0;
GLdouble vis3d_fovy = 60.0;
GLdouble vis3d_aspect = 1.0;
GLdouble vis3d_zNear = 5.0;
GLdouble vis3d_zFar = 100.0;

// Contract Net Protocol (CNP) related
MlabCNP *gMlabCNP = NULL;
int cnpMode = CNP_MODE_DISABLED;
bool allowModCNPRobots = false;

// Vector field related
static ClVectorFieldDlg* pobjVectorFieldDlg = NULL;
extern ClVectorField g_objVectorField;

// Flags
bool gAutomaticExecution = false;
bool gStopRobotFeedback = false;
bool monochrome = false;
bool overlayOpenDialogIsUp = false;
bool auto_run = false;
bool ask_for_map = false;
bool use_ipt = true;
bool warning_user = false;
static bool show_colormap = false;
static bool private_colormap = false;
static bool show_command_panel_on_startup = false;
static int show_cmdli_panel_on_startup = false;
static bool show_telop_interface_on_startup = false;
static bool seen_initial_copyright = false;
static bool  pause_before_execution = false;

// File names
static char* startup_filename = NULL;
static char* personality_filename = NULL;
string g_strEnvChangeFilename = "";

// Proceed dialog constants
static int proceed_dialog_x = 500;
static int proceed_dialog_y = 200;
static int proceed_dialog_dx = 20;
static int proceed_dialog_dy = 20;
static int proceed_dialog_num = 0;

// CMDLi Panel
gt_cmdli_panel *the_cmdli_panel = NULL;

// JBox
JBoxMlab *gJBoxMlab = NULL;
JBoxDisplay *gJBoxDisplay = NULL;

// Others
symbol_table <rc_chain> rc_table(47);
SuLMViewNode* g_psuLMViewDialogList = NULL;
static char* last_warning = NULL;
int resetCount = 0;
T_console_error console_error = NONE;
char* console_error_messages[] =
{
    "None",
    "Robot com library version mismatch"
};
EventLogging* gEventLogging = NULL;
extern double cycle_duration;
//NADEEM - Placing the virtual clock variables until we figure out what to do with it
double virtual_clock_alpha = 1.0;
bool vClockAlive=true;
long g_base_time;
int expirationTimeInSec = -1;
BitmapIconDisplayer *gBitmapIconDisplayer = NULL;

// PU map scalling issues
int scroll_bar_v_val;
int scroll_bar_h_val;
static void Handle_MapScrolling(void);


//-----------------------------------------------------------------------
// forward declarations
//-----------------------------------------------------------------------
int execute_step_list_workproc(void);
void DeleteLMParamDialogHelper(Widget dialog, SuLMViewNode** ppsuCurrent);
static void redraw_map(void);

//-----------------------------------------------------------------------
// This function is called by signal
//-----------------------------------------------------------------------
void sig_catch(int sig)
{
    fprintf(stderr, "mlab: sig_catch - caught signal %d\n", sig);
    gEventLogging->log("mlab: sig_catch - caught signal %d", sig);

    quit_mlab();
}

//-----------------------------------------------------------------------
void child_reaper(int sig)
{
    int iStatus;
    while (waitpid(-1, &iStatus, WNOHANG) > 0)
    {
        // do nothing
    }
}

//-----------------------------------------------------------------------
// this processes pending events in the event queue
//-----------------------------------------------------------------------
void PumpEventQueue()
{
    if (display == NULL)
    {
        return;
    }

    while (XPending(XtDisplay(main_window)) > 0)
    {
        XEvent event;
        XNextEvent(XtDisplay(main_window), &event);
        XtDispatchEvent(&event);
    }
}


//-----------------------------------------------------------------------
void EnablePauseControls(bool bEnable)
{
    XtVaSetValues(console_pause_button, XmNsensitive, bEnable, NULL);
    EnableCommandPanelPauseButton(bEnable);
}


//-----------------------------------------------------------------------
void ShowMovementFieldMenuItemEnabled(bool bEnabled)
{
    XtVaSetValues(show_movement_field, XmNsensitive, bEnabled, NULL);
}

//-----------------------------------------------------------------------
void ResetWorld(char* filename, int load_new_file)
{
    gEventLogging->start("ResetWorld");

    if (executing_step_list)
    {
        clear_simulation();
    }

    if (filename == NULL)
    {
        return;
    }

    // Have to Load the robot, but use "interrupt_load_overlay" to
    //   prevent the map from being written over

    if(load_new_file == 1)
    {
        interrupt_load_overlay(filename);
    }

    if (gt_load_commands(startup_filename) != 0)
    {
        warn_userf("Commands NOT loaded successfully from\n file: %s", startup_filename);
        return;
    }

    // run automatically, now that the file is loaded
    gt_execute_step_list();

    last_warning = NULL;


    gEventLogging->end("PickMap");
    send_robots_pause_state(FALSE);
    sleep(5);
}


//-----------------------------------------------------------------------
void Pick_new_map(void)
{
    gEventLogging->start("PickMap");

    // if the Vector Field dialog is open, close it
    if (pobjVectorFieldDlg != NULL)
    {
        pobjVectorFieldDlg->SetStopDrawing(true);
        pobjVectorFieldDlg->Destroy();
    }
    ShowMovementFieldMenuItemEnabled(false);

    if (executing_step_list)
    {
        gt_abort_command_execution();
    }

    //************** Popup a  window to let the user choose a map overlay
    char* filename = PickMapOverlay();
    if (filename == NULL)
    {
        gEventLogging->cancel("PickMap");
        return;
    }

    delete_map(gMapInfo);
    clear_map();
    clear_obstacles();

    // Kill all the robots
    if (use_ipt)
    {
        clear_robots();
    }

    // Clear the objects (robots, attractors, etc.) from the global list
    clear_objects();

    // Clear just the map measures.
    gt_clear_db_measures();


    int err = load_new_map(filename);

    free(filename);
    if (err)
    {
        gEventLogging->status("PickMap \"Unable to load map file\"");
        gEventLogging->cancel("PickMap");
        return;
    }
    gEventLogging->status("PickMap \"%s\"", filename);

    // run automatically, now that the file is loaded
    if (auto_run)
    {
        gt_execute_step_list();
    }

    last_warning = NULL;

    gEventLogging->end("PickMap");
}

//-----------------------------------------------------------------------
static void create_colormap(Display* dpy)
{
    XColor color, exact;
    char gray_name[10];
    int num_colors_copied = 0;
    int i;

    // Install the color map and set up the colors
    int scr = DefaultScreen(dpy);
    Visual* vis = DefaultVisual(dpy, scr);
    Colormap default_colormap = DefaultColormap(dpy, scr);

    // Make sure we have a color system
    if (vis->map_entries <= 2)
    {
        printf("Unable to install colormap on monochome system\n");
        mlab_colormap = default_colormap;
        private_colormap = false;
        monochrome = true;
        return;
    }

    // First try and use the existing default colormap
    int use_default_colormap = true;
    for (i = 0; i <= MAX_GRAY; i++)
    {
        sprintf(gray_name, "gray%d", i);
        if (XAllocNamedColor(dpy, default_colormap,
                               gray_name, &exact, &color) == 0)
        {
            use_default_colormap = false;
            break;
        }
        gray[i] = color.pixel;
    }
    if (use_default_colormap)
    {
        printf("Using default colormap\n");
        mlab_colormap = default_colormap;
        private_colormap = false;
        return;
    }

    // Too bad.  Can't use the default colormap, so create a new one
    mlab_colormap = XCreateColormap(dpy, DefaultRootWindow(dpy),
                                     vis, AllocNone);

    // Get the first 100 colors from the default colormap
    int black_seen = false;

    for (i = 0; i < FIRST_GRAY_COLORMAP_ENTRY; i++)
    {
        color.pixel = i;
        XQueryColor(dpy, default_colormap, &color);
        if ((color.red == 0) && (color.green == 0) && (color.blue == 0))
        {
            if (black_seen)
            {
                break;
            }
            else
            {
                black_seen = true;
            }
        }

        if (XAllocColor(dpy, mlab_colormap, &color) == 0)
        {
            fprintf(stderr, "Error allocating color %d in mlab_colormap\n", i);
            return;
        }
        num_colors_copied++;
    }

    // See if we really need to make our own colormap
    if ((vis->map_entries - num_colors_copied - 55) > (MAX_GRAY + 1))
    {

        // There should be enough room in the existing colormap,
        // so use it instead of the one we've just created.
        // Note that we are requiring 55 free colors after installing our
        // grays.

        XFreeColormap(dpy, mlab_colormap);
        mlab_colormap = default_colormap;
    }

    // Set up the grayscale colors
    for (i = 0; i <= MAX_GRAY; i++)
    {
        sprintf(gray_name, "gray%d", i);
        color.pixel = FIRST_GRAY_COLORMAP_ENTRY + i;
        if (XAllocNamedColor(dpy, mlab_colormap,
                               gray_name, &exact, &color) == 0)
        {
            fprintf(stderr, "Error allocating %s (out of %d)\n",
                     gray_name, MAX_GRAY);
            if (mlab_colormap != default_colormap)
            {
                fprintf(stderr, "(Using private colormap, %d colors copied)\n",
                         num_colors_copied);
            }
            else
            {
                fprintf(stderr, "(Using default colormap)\n");
            }
            return;
        }

        gray[i] = color.pixel;
    }


    if (mlab_colormap != default_colormap)
    {
        private_colormap = true;
    }
}

//-----------------------------------------------------------------------
static void find_nearest_color(XColor* original)
{
    const int max_diff = 7710;    // Allows an error of approx 10
                                  // (out of 255) in each color
                                  // or 30 in one color.

    int i, red, green, blue, dr, dg, db, diff, best;
    unsigned long best_pixel;
    XColor color;

    red   = original->red;
    green = original->green;
    blue  = original->blue;

    // Look for the nearst match to this color below 100
    for (i = 0; i < FIRST_GRAY_COLORMAP_ENTRY; i++)
    {
        color.pixel = i;
        XQueryColor(display, mlab_colormap, &color);
        dr = color.red   - red;
        dg = color.green - green;
        db = color.blue  - blue;
        diff = abs(dr) + abs(dg) + abs(db);
        if ((i == 0) || (diff < best))
        {
            best = diff;
            best_pixel = i;
        }
    }

    // Copy in the new information (if it is close enough!)
    if (best <= max_diff)
    {
        color.pixel = best_pixel;
        XQueryColor(display, mlab_colormap, &color);
        original->pixel = best_pixel;
        original->red   = color.red;
        original->green = color.green;
        original->blue  = color.blue;
    }
}

//-----------------------------------------------------------------------
static void mlab_colorproc(
    XColor* bg,
    XColor* fg,
    XColor* sel,
    XColor* ts,
    XColor* bs)
{
    // Call the original colorproc and see what is requested
    original_colorproc(bg, fg, sel, ts, bs);

    // Check each for the nearest match in the first 100 colors
    find_nearest_color(fg );
    find_nearest_color(sel);
    find_nearest_color(ts );
    find_nearest_color(bs );

    // NOTE: I'd like to change bg here, but that leads to infinite
    // recalling of this routine.  So let it be.   It works fine
    // as long as bg is in the lower 100 colors.
}

//-----------------------------------------------------------------------
static void fix_window_colors(void)
{
    // If a we've installed a private colormap, we'd like to reuse the colors
    // as much as possible, so replace the Motif color calculation routine */

    if (private_colormap)
    {
        // First, save the original routine
        original_colorproc = XmGetColorCalculation();

        // Install the new colorproc
        XmSetColorCalculation(mlab_colorproc);
    }
}

//-----------------------------------------------------------------------
static void draw_colormap(void)
{
    // Show the colors
    XGCValues xgcv;
    GC gc;

    for (int i = 0; i < 256; i++)
    {
        xgcv.foreground = i;
        gc = XtGetGC(drawing_area, GCForeground, &xgcv);
        XFillRectangle(
            XtDisplay(drawing_area), XtWindow(drawing_area), gc,
            (int)(20 + 20 * (i % 10)),
            (int)(3 + 20 * ((int)(i / 10))),
            20, 20);
    }
}

//-----------------------------------------------------------------------
void set_show_colormap(int flag)
{
    if (show_colormap != flag)
    {
        show_colormap = flag;
        redraw_map();
        if (show_colormap)
        {
            draw_colormap();
        }
    }
}

//-----------------------------------------------------------------------
static void update_scale_ruler_zoom_factor()
{
    char label_str[20];
    XmString label;

    sprintf(label_str, "Zoom: %d%%", nint(zoom_factor * 100.0));
    label = XSTRING(label_str);
    XtVaSetValues(zoom_factor_label,
                   XmNlabelString, label,
                   NULL);
    XmStringFree(label);
}

//-----------------------------------------------------------------------
void draw_scale_ruler()
{
    Display* disp = XtDisplay(scale_ruler);
    Window   win  = XtWindow(scale_ruler);
    Dimension width, height;
    double range, min_size;
    int left, right, top, middle, bottom;
    char label[100];
    int i, x;

    // get the foreground and background
    XtVaGetValues(
        scale_ruler,
        XmNwidth, &width,
        XmNheight, &height,
        NULL);

    // erase the area first
    XFillRectangle(disp, win, sr_erase_gc,
                    0, 0, width, height);

    // compute what range to show
    min_size = mission_area_width_meters;
    if (mission_area_height_meters < min_size)
    {
        min_size = mission_area_height_meters;
    }
    range = pow(10.0, floor(log10(min_size) - 1.0));

    // compute the various positions
    left = 10;
    right = left + nint(range / meters_per_pixel);
    bottom = nint((double)(0.8 * height));
    top    = nint((double)(height / 4.0));
    middle = nint((top + bottom) / 2.0);

    // draw the main line of the scale ruler
    XDrawLine(disp, win, sr_draw_gc,
               left, bottom, right, bottom);

    // draw the ticks
    for (i = 0; i <= 10; i++)
    {
        x = left + nint(i * (right - left) / 10.0);
        XDrawLine(disp, win, sr_draw_gc,
                   x, middle, x, bottom);
    }
    for (i = 0; i <= 2; i++)
    {
        x = left + nint(i * (right - left) / 2.0);
        XDrawLine(disp, win, sr_draw_gc,
                   x, top, x, bottom);
    }

    // draw the labels on the ends of the scale
    XDrawString(disp, win, sr_draw_gc, 0,
                 nint((double) (height * 0.666)), "0", 1);
    sprintf(label, " %1.1f m   (Mission area is %1.1fm by %1.1fm)",
             range, mission_area_width_meters, mission_area_height_meters);
    XDrawString(disp, win, sr_draw_gc, right,
                 nint((double) (height * 0.666)), label, strlen(label));

    update_scale_ruler_zoom_factor();

    XmUpdateDisplay(main_window);
}

//-----------------------------------------------------------------------
static void draw_scale_ruler_cb(
    Widget w,
    XtPointer no_client_data,
    XmDrawingAreaCallbackStruct* callback_data)
{
    if ((callback_data->event == NULL) ||
         (callback_data->event->xexpose.count == 0))
    {
        draw_scale_ruler();
    }
}

//-----------------------------------------------------------------------
// Refreshes the screen
//-----------------------------------------------------------------------
static void run_refresh_button(void)
{
    clear_map();
    draw_world();
}


//-----------------------------------------------------------------------
// Handles scrolling the map
//-----------------------------------------------------------------------
static void Handle_MapScrolling(void)
{
  int slider_size, increment, page_increment, maximum;

  XtVaGetValues(drawing_area_scroll_bar,
		XmNsliderSize, &slider_size,
		XmNincrement, &increment,
		XmNpageIncrement, &page_increment,
		XmNmaximum, &maximum,
		NULL);
  scroll_bar_v_val = maximum - slider_size;
  XtVaGetValues(drawing_area_scroll_bar_h,
		XmNsliderSize, &slider_size,
		XmNincrement, &increment,
		XmNpageIncrement, &page_increment,
		XmNmaximum, &maximum,
		NULL);
  scroll_bar_h_val = maximum - slider_size;

  clear_map();
  draw_world();
}


//-----------------------------------------------------------------------
static void create_scale_ruler(Widget parent)
{
    Widget scale_label, zinb, zoutb;

    // create the "Scale:" label on the left side
    scale_label = XtVaCreateManagedWidget(
        " Scale: ", xmLabelGadgetClass, parent,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNleftAttachment,    XmATTACH_FORM,
        NULL);

    zinb = XtVaCreateManagedWidget(
        "zoom-in-button", xmArrowButtonGadgetClass, parent,
        XmNarrowDirection,    XmARROW_UP,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNrightAttachment,   XmATTACH_FORM,
        XmNrightOffset,       25,
        NULL);
    XtAddCallback(zinb, XmNactivateCallback,
                   (XtCallbackProc) zoom_in, NULL);

    zoutb  = XtVaCreateManagedWidget(
        "zoom-out-button", xmArrowButtonGadgetClass, parent,
        XmNarrowDirection,    XmARROW_DOWN,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       zinb,
        XmNrightOffset,       2,
        NULL);
    XtAddCallback(zoutb, XmNactivateCallback,
                   (XtCallbackProc) zoom_out, NULL);

    zoom_factor_label = XtVaCreateManagedWidget(
        " Zoom: 100% ", xmLabelGadgetClass, parent,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNtopOffset,         5,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNbottomOffset,      5,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       zoutb,
        XmNrightOffset,       5,
        NULL);

    XmString str = XmStringCreateLocalized("Pause");
    console_pause_button = XtVaCreateManagedWidget(
        "", xmPushButtonGadgetClass, parent,
        XmNlabelString, str,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       zoom_factor_label,
        XmNrightOffset,       10,
        NULL);
    XmStringFree(str);
    XtAddCallback(console_pause_button, XmNactivateCallback,
                   (XtCallbackProc) toggle_pause_execution_button, NULL);

    str = XmStringCreateLocalized("Refresh");
    console_refresh_button = XtVaCreateManagedWidget(
        "Refresh", xmPushButtonGadgetClass, parent,
        XmNlabelString, str,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNrightAttachment,   XmATTACH_WIDGET,
        XmNrightWidget,       console_pause_button,
        XmNrightOffset,       10,
        NULL);
    XmStringFree(str);
    XtAddCallback(console_refresh_button, XmNactivateCallback,
                   (XtCallbackProc) run_refresh_button, NULL);

    str = gStopRobotFeedback?
        XmStringCreateLocalized((char *)(FEEDBACK_BUTTON_LABEL_NONSTOP.c_str())) :
        XmStringCreateLocalized((char *)(FEEDBACK_BUTTON_LABEL_STOP.c_str()));
    console_feedback_button = XtVaCreateManagedWidget(
        "", xmPushButtonGadgetClass, parent,
        XmNlabelString, str,
        XmNtopAttachment, XmATTACH_FORM,
        XmNbottomAttachment, XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, console_refresh_button,
        XmNrightOffset, 10,
        NULL );
    XmStringFree(str);
    XtAddCallback(
        console_feedback_button,
        XmNactivateCallback, (XtCallbackProc)gt_toggle_feedback,
        NULL);

    // create the scale ruler and fill up the rest of the command bar
    scale_ruler = XtVaCreateManagedWidget(
        "scale-ruler", xmDrawingAreaWidgetClass, parent,
        XmNborderWidth,       0,
        XmNtopAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,  XmATTACH_FORM,
        XmNleftAttachment,    XmATTACH_WIDGET,
        XmNleftWidget,        scale_label,
        XmNrightAttachment,   XmATTACH_WIDGET,
        //XmNrightWidget,       console_refresh_button,
        XmNrightWidget,       console_feedback_button,
        XmNrightOffset,       10,
        NULL);
    XtAddCallback(scale_ruler, XmNexposeCallback,
                   (XtCallbackProc)draw_scale_ruler_cb, NULL);


    // get the default colors and construct the gc to clear the ruler
    XGCValues  xgcv;
    int        bg, fg;
    Display* dpy = XtDisplay(scale_ruler);
    int scr = DefaultScreen(dpy);
    Visual* myvis = DefaultVisual(dpy, scr);

    if (myvis->map_entries > 2)
    {
        XtVaGetValues(scale_ruler,
                       XmNforeground, &fg,
                       XmNbackground, &bg,
                       NULL);
    }
    else
    {
        fg = BlackPixel(dpy, scr);
        bg = WhitePixel(dpy, scr);
    }
    xgcv.foreground = bg;
    xgcv.background = bg;
    sr_erase_gc = XtGetGC(scale_ruler, GCForeground | GCBackground, &xgcv);
    xgcv.foreground = fg;
    xgcv.background = bg;
    sr_draw_gc = XtGetGC(scale_ruler, GCForeground | GCBackground, &xgcv);
}

//-----------------------------------------------------------------------
void warn_user(char* msg)
{
    XmString str;
    XmString old_msg, new_msg;

    gEventLogging->status("mlab warn_user() \"%s\"", msg);

    if (warning_user)
    {
        // Add this message to the message already being displayed

        // construct the new error string
        XtVaGetValues(error_dialog, XmNmessageString, &old_msg, NULL);
        str = XSTRING("\n\n");
        new_msg = XmStringConcat(old_msg, str);
        XmStringFree(str);
        XmStringFree(old_msg);
        str = XSTRING(msg);
        new_msg = XmStringConcat(new_msg, str);
        XmStringFree(str);

        // install the new message
        XtVaSetValues(error_dialog, XmNmessageString, new_msg, NULL);
        XmStringFree(new_msg);
    }
    else
    {
        XtVaSetValues(error_dialog, RES_CONVERT(XmNmessageString, msg), NULL);
        XtManageChild(error_dialog);

        warning_user = true;
    }
}

//-----------------------------------------------------------------------
void warn_userf(const char* fmt, ...)
{
    va_list args;
    char msg[512];

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    warn_user(msg);
}

//-----------------------------------------------------------------------
void warn_once(const char* fmt, ...)
{
    va_list args;
    char msg[2048];

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);

    // If this is a repeat of the last warning message, skip it.
    if ((last_warning == NULL) || strcmp(msg, last_warning))
    {
        if (last_warning)
        {
            free(last_warning);
        }
        last_warning = strdup(msg);

        warn_user(msg);
    }
}

//-----------------------------------------------------------------------
extern bool die_after_warning;
static void warn_user_ok_cb(void)
{
    XtUnmanageChild(error_dialog);
    warning_user = false;

    if (die_after_warning)
    {
        gEventLogging->end("mlab");
        gEventLogging->pause();

        if (use_ipt)
        {
            gt_close_communication();
        }
        exit(1);
    }
}

//-----------------------------------------------------------------------
static void create_error_dialog(Widget parent)
{
    Arg             args[MAXARGS];
    Cardinal        argcount = 0;
    XmString title = XSTRING("Error!");
    XmString ok    = XSTRING("     OK     ");
    static XtCallbackRec ok_cb_list[] = {
        { (XtCallbackProc) warn_user_ok_cb, NULL },
        { (XtCallbackProc) NULL, NULL }
    };

    XtSetArg(args[argcount], XmNdialogTitle, title);          argcount++;
    XtSetArg(args[argcount], XmNdialogStyle,
              XmDIALOG_FULL_APPLICATION_MODAL);                  argcount++;
    XtSetArg(args[argcount], XmNokLabelString, ok);           argcount++;
    XtSetArg(args[argcount], XmNokCallback, ok_cb_list);      argcount++;
    XtSetArg(args[argcount], XmNwidth, 550);                  argcount++;
    XtSetArg(args[argcount], XmNheight, 450);                 argcount++;
    XtSetArg(args[argcount], XmNresizePolicy, XmRESIZE_NONE); argcount++;
    error_dialog = XmCreateErrorDialog(parent, "error-dialog", args, argcount);

    // get rid of the unwanted buttons
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_HELP_BUTTON));

    XmStringFree(title);
    XmStringFree(ok);
    warning_user = false;
    last_warning = NULL;
}



//-----------------------------------------------------------------------
// alert_user
//  * When this routine is called, mlab pops up the alert window
//    showing the message in (char* msg).
//
// input
//  msg    = contens of message shown in the window.
//-----------------------------------------------------------------------
void alert_user(char* msg)
{
    char msg_bdy[4096];
    //const char* line = "\n-----------------------------------------------------------------\n";
    const char* line = "\n---------------------------------------------------------\n";

    sprintf(msg_bdy, "%s%s", msg, line);
    msg = msg_bdy;

    alert_cur_position = 0;

    XmTextSetInsertionPosition(alert_text, alert_cur_position);
    XmTextInsert(alert_text, alert_cur_position, msg);
    XmTextShowPosition(alert_text, alert_cur_position);

    XtManageChild(alert_text);
    XtManageChild(alert_form1);
    XtManageChild(alert_form2);
    XtManageChild(alert_pane);
    XtPopup(alert_dialog, XtGrabNone);

    // Ringing a bell in console.
    //    fprintf(stderr, "\a");

}

//-----------------------------------------------------------------------
// alert_userf
//  * formats the argument of the message used for alert_user(msg).
//-----------------------------------------------------------------------
void alert_userf(const char* fmt, ...)
{
    va_list args;
    char msg[512];

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    alert_user(msg);
}

//-----------------------------------------------------------------------
// report_user
//  * When this routine is called, mlab will bring up the scrolleable
//    text window, for report_current_state.
//
// input
//  msg    = contens of message shown in the window.
//-----------------------------------------------------------------------
void report_user(char* msg)
{
    XmTextInsert(status_text, wpr_position, msg);
    wpr_position += strlen(msg);
    XtVaSetValues(status_text, XmNcursorPosition, wpr_position, NULL);
    XmTextShowPosition(status_text, wpr_position);

    XtManageChild(status_text);
    XtManageChild(status_form1);
    XtManageChild(status_form2);
    XtManageChild(status_pane);
    XtPopup(status_dialog, XtGrabNone);

    XmToggleButtonSetState(report_current_state_button, True, True);
}

//-----------------------------------------------------------------------
// report_userf
//  * formats the argument of the message used for report_use(msg).
//-----------------------------------------------------------------------
void report_userf(const char* fmt, ...)
{
    va_list args;
    char msg[512];

    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    report_user(msg);
}

//-----------------------------------------------------------------------
// close_status_dialog
//  * Take down the scrollable text window for report_current state.
//-----------------------------------------------------------------------
static void close_status_dialog(Widget widget, XtPointer client_data, XtPointer call_data)
{
    Widget shell = (Widget) client_data;
    XtPopdown(shell);

    XmToggleButtonSetState(report_current_state_button, False, True);
}

//-----------------------------------------------------------------------
// create_status_dialog
//  * Creates Widgets that can display the scrollable text window for
//    report_current_state.
//-----------------------------------------------------------------------
static void create_status_dialog(Widget parent)
{
    int n = 0;
    Arg args[10];

    // Setup a dialog shell as a popup window.
    status_dialog = XtVaCreatePopupShell("Current Robot State",
                                         xmDialogShellWidgetClass, parent,
                                         XmNdeleteResponse, XmUNMAP,
                                         NULL);

    // Create a PanedWindow to manage the stuff in this dialog.
    status_pane = XtVaCreateWidget("pane",
                                   xmPanedWindowWidgetClass, status_dialog,
                                   XmNsashHeight,        1,
                                   XmNsashWidth,         1,
                                   NULL);

    // Create a RowColumn in the form for label and text Widget.
    status_form1 = XtVaCreateWidget("form1", xmFormWidgetClass, status_pane, NULL);

    XtVaSetValues(status_form1, XmNwidth,  500, NULL);

    // Create a label gadget
    status_label = XtVaCreateManagedWidget("label", xmLabelGadgetClass, status_form1,
                                           XmNlabelType,        XmPIXMAP,
                                           XmNleftAttachment,   XmATTACH_FORM,
                                           XmNtopAttachment,    XmATTACH_FORM,
                                           XmNbottomAttachment, XmATTACH_FORM,
                                           NULL);

    // Prepare the text for display in the ScrolledText object.

    XtSetArg(args[n], XmNscrollVertical,             True);              n++;
    XtSetArg(args[n], XmNscrollHorizontal,           False);             n++;
    XtSetArg(args[n], XmNeditMode,                   XmMULTI_LINE_EDIT); n++;
    XtSetArg(args[n], XmNeditable,                   False);             n++;
    XtSetArg(args[n], XmNcursorPositionVisible,      False);             n++;
    XtSetArg(args[n], XmNwordWrap,                   True);              n++;
    XtSetArg(args[n], XmNrows,                       5);                 n++;
    status_text = XmCreateScrolledText(status_form1, "status-text", args, n);

    XtVaSetValues (XtParent(status_text),
                   XmNleftAttachment,   XmATTACH_WIDGET,
                   XmNleftWidget,       status_label,
                   XmNtopAttachment,    XmATTACH_FORM,
                   XmNrightAttachment,  XmATTACH_FORM,
                   XmNbottomAttachment, XmATTACH_FORM,
                   NULL);

    // Reset cursor
    wpr_position = 0;

    // Create another form to act as the action area for the dialog.
    status_form2 = XtVaCreateWidget("form2", xmFormWidgetClass, status_pane,
                                    XmNfractionBase, 5,
                                    NULL);

    status_widget = XtVaCreateManagedWidget(
        "Close",
        xmPushButtonGadgetClass, status_form2,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 2,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                3,
        XmNshowAsDefault,                True,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(status_widget, XmNactivateCallback, close_status_dialog, status_dialog);
}

//-----------------------------------------------------------------------
// Redraws the map.
//-----------------------------------------------------------------------
static void redraw_map(void)
{
    if (gt_show_3d_vis)
    {
        drawGLScene();
    }
    else
    {
        XCopyArea(
            XtDisplay(drawing_area),
            drawing_area_pixmap,
            XtWindow(drawing_area),
            gGCs.black,
            0,
            0,
            (int)(mission_area_width_meters / meters_per_pixel),
            (int)(mission_area_height_meters / meters_per_pixel),
            0,
            0);
    }
}


//-----------------------------------------------------------------------
static void redraw_cb(
    Widget w,
    XtPointer no_client_data,
    XmDrawingAreaCallbackStruct* callback_data)
{
    if ((callback_data->event == NULL) ||
        (callback_data->event->xexpose.count == 0))
    {
        XtRemoveCallback(
            drawing_area,
            XmNexposeCallback,
            (XtCallbackProc)redraw_cb,
            NULL);

        XtRemoveCallback(
            drawing_area,
            XmNresizeCallback,
            (XtCallbackProc)redraw_cb,
            NULL);

        redraw_map();
        draw_world();

        if (show_colormap)
        {
            draw_colormap();
        }

        XFlush(XtDisplay(drawing_area));

        while (XtAppPending(app))
        {
            XtAppProcessEvent(app, XtIMAll);
        }

        XtAddCallback(
            drawing_area,
            XmNexposeCallback,
            (XtCallbackProc)redraw_cb,
            NULL);

        XtAddCallback(
            drawing_area,
            XmNresizeCallback,
            (XtCallbackProc)redraw_cb,
            NULL);

    }
}



//-----------------------------------------------------------------------
void add_workproc(XtWorkProc proc, XtPointer client_data)
{
    int i;

    // insert the work proc into the first empty slot
    for (i = 0; i < MAX_WORKPROCS; i++)
    {
        if (workproc[i] == NULL)
        {
            workproc[i] = proc;
            return;
        }
    }

    // No space left; complain!
    fprintf(stderr, "Out of room in the workproc array!\n");
}



//-----------------------------------------------------------------------
int main_workproc(void)
{
    int i;

    // The purpose of the procedure is to allow a set of work procedures
    // to be executed in round-robin style --- instead of the most recently
    // registered one being pre-eminent.

    // execute each work procedure
    for (i = 0; i < MAX_WORKPROCS; i++)
    {
        if (workproc[i] != NULL)
        {
            if (workproc[i](NULL))
            {
                workproc[i] = NULL;   // Remove this workproc
            }
        }
    }

    XtAppAddTimeOut(app, WORKPROC_PERIOD_MSEC, (XtTimerCallbackProc) main_workproc, NULL);

    // never be removed
    return false;
}

//-----------------------------------------------------------------------
int cmdli_workproc(void)
{
    bool done = the_cmdli_panel->update_display();

    if( !done )
    {
       // reset the timer
       XtAppAddTimeOut(app, CMDLI_DISPLAY_UPDATE_PERIOD_MSEC, (XtTimerCallbackProc)cmdli_workproc, NULL);
    }

    return false;
}

static int gt_popup_cmdli_panel(void)
{
    if (the_cmdli_panel == NULL)
    {
        the_cmdli_panel = new gt_cmdli_panel(main_window);
    }

   // init our timer
   XtAppAddTimeOut(app, CMDLI_DISPLAY_UPDATE_PERIOD_MSEC, (XtTimerCallbackProc)cmdli_workproc, NULL);

   the_cmdli_panel->popup_panel();

   return true;
}

//-----------------------------------------------------------------------
// This function will be called when the time to expire the mlab
// (expirationTimeInSec) was specified.
//-----------------------------------------------------------------------
int expireMlab(void)
{
    quit_mlab();

    return true;
}

//-----------------------------------------------------------------------
static void overlay_open_ok_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct* fcb)
{
    ovl_filename = NULL;

    XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET , &ovl_filename);
    gEventLogging->log("Select Overlay File = %s", ovl_filename);

    // unexpose the file selection dialog
    overlayOpenDialogIsUp = false;
    XtUnmanageChild(overlay_open_dialog);
    gEventLogging->end("Select Overlay File");

    // complain if commands are already executing
    if (executing_step_list)
    {
        warn_user(" Executing commands now!  Wait for completion\nor ABORT before loading files.");
        return;
    }

    // make sure we have a valid filename to read from
    if ((ovl_filename == NULL) || (strlen(ovl_filename) == 0))
    {
        warn_user("Overlay NOT loaded successfully: No filename entered yet!");
        if (ovl_filename)
        {
            XtFree(ovl_filename);
        }
        return;
    }
    if (!file_exists(ovl_filename))
    {
        warn_userf("Overlay NOT loaded successfully: \nfile '%s' does not exist!",
                    ovl_filename);
        if (ovl_filename)
        {
            XtFree(ovl_filename);
        }
        return;
    }


    clear_simulation();

    // load the overlay
    if (gt_load_overlay(ovl_filename) != 0)
    {
        warn_userf("Overlay NOT loaded successfully from\n file: %s",
                    ovl_filename);
    }

    // Update the filename in the command panel
    set_command_panel_filename(ovl_filename);

    if (gUseMlabMissionDesign)
    {
        if (!gMMD->windowsAlreadyCreated())
        {
            gMMD->createWindows();
        }
    }
}

//-----------------------------------------------------------------------
static void overlay_open_cancel_cb(
    Widget w,
    caddr_t client_data,
    caddr_t unused)
{
    gEventLogging->cancel("Select Overlay File");
    overlayOpenDialogIsUp = false;
    XtUnmanageChild(w);
}

//-----------------------------------------------------------------------
// This function will click on OK button for the Overlay dialog
// autimatically.
//-----------------------------------------------------------------------
static int clickOpenOverlayDialog(void)
{
    XmFileSelectionBoxCallbackStruct fcb;
    XmString value;

    while (!overlayOpenDialogIsUp)
    {
        if (XtAppPending(app))
        {
            XtAppProcessEvent(app, XtIMAll);
        }
    }

    XtVaGetValues(
        overlay_open_dialog,
        XmNtextString,
        &value,
        NULL);

    fcb.value = value;

    if ((XtHasCallbacks(overlay_open_dialog, XmNokCallback) == XtCallbackHasSome))
    {
        XtCallCallbacks(overlay_open_dialog, XmNokCallback, &fcb);
    }

    XmStringFree(value);

    return true;
}

//-----------------------------------------------------------------------
static void create_overlay_open_dialog(Widget parent, char* defaultOverlayName)
{
    Widget list_w, text_w;
    XmString titleXStr, filterXStr, directoryXStr, fileFullPathXStr;
    char fileFullPath[1024];
    char* directory;
    Arg wargs[5];
    int n = 0;

    // Create the dialog.
    titleXStr  = XSTRING("Select Overlay File");
    filterXStr = XSTRING("*.ovl");

    XtSetArg(wargs[n], XmNdialogTitle, titleXStr ); n++;
    XtSetArg(wargs[n], XmNpattern,     filterXStr); n++;

    gEventLogging->start("Select Overlay File");

    overlay_open_dialog = XmCreateFileSelectionDialog(
        parent,
        "Overlay Open Dialog",
        wargs,
        n);

    if (overlay_open_dialog == NULL)
    {
        warn_user("Error creating the file open dialog box\n");
        return;
    }

    XmStringFree(titleXStr );
    XmStringFree(filterXStr);

    // If the default overlay name was specified, select it as the current one.
    if (defaultOverlayName != NULL)
    {
        // Get the directory name.
        XtVaGetValues(overlay_open_dialog, XmNdirectory, &directoryXStr, NULL);

        if (XmStringGetLtoR(directoryXStr, XmSTRING_DEFAULT_CHARSET, &directory))
        {
            sprintf(fileFullPath, "%s%s", directory, defaultOverlayName);

            // Make sure the file exits.
            if (file_exists(fileFullPath))
            {
                // Display the file in the text field.
                text_w = XmFileSelectionBoxGetChild(overlay_open_dialog, XmDIALOG_TEXT);
                XtVaSetValues(text_w, XmNvalue, fileFullPath, NULL);

                // Highlight the file in the list.
                fileFullPathXStr = XSTRING(fileFullPath);
                list_w = XmFileSelectionBoxGetChild(overlay_open_dialog, XmDIALOG_LIST);
                XmListSelectItem(list_w, fileFullPathXStr, false);
                XmStringFree(fileFullPathXStr);
            }
        }
        XmStringFree(titleXStr);
    }

    XtUnmanageChild(XmFileSelectionBoxGetChild(overlay_open_dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmFileSelectionBoxGetChild(overlay_open_dialog, XmDIALOG_APPLY_BUTTON));

    overlayOpenDialogIsUp = true;

    // install the callbacks
    XtAddCallback(
        overlay_open_dialog,
        XmNokCallback,
        (XtCallbackProc)overlay_open_ok_cb,
        NULL);

    XtAddCallback(
        overlay_open_dialog,
        XmNcancelCallback,
        (XtCallbackProc)overlay_open_cancel_cb,
        NULL);

    gAssistantDialog->showPredefinedMessage(ASSISTANT_MLAB_SELECT_OVERLAY);

    if (gAutomaticExecution)
    {
        XtAppAddWorkProc(app, (XtWorkProc)clickOpenOverlayDialog, NULL);
    }
}

//-----------------------------------------------------------------------
void set_overlay_open_dialog_filename(char* filename)
{
    Widget dialog_text_field;
    char new_filename_str[256];
    char dir_str[256];
    char* new_dir;
    char* old_filename;
    XmString dir, new_filename, new_full_filename;

    // Combine the two filenames appropriately
    if (filename_has_directory(filename))
    {
        // We have a directory, use it to override the old one

        // Extract the directory and install it into the file open dialog
        extract_directory(filename, dir_str);
        dir = XSTRING(dir_str);
        XtVaSetValues(overlay_open_dialog, XmNdirectory, dir, NULL);
        XmStringFree(dir);

        // Retrieve the new directory from the file open dialog
        // (This gets the newly expanded version)
        XtVaGetValues(overlay_open_dialog, XmNdirectory, &dir, NULL);

        // Reintegrate the simple filename with the newly expanded directory
        remove_directory(filename, new_filename_str);
        new_filename = XSTRING(new_filename_str);
        new_full_filename = XmStringConcat(dir, new_filename);

        // Install the new full filename into the file open dialog
        XtVaSetValues(overlay_open_dialog, XmNtextString, new_full_filename, NULL);
        XmStringFree(new_filename);
        XmStringFree(new_full_filename);
    }
    else
    {
        // No directory in the new filename

        // Get the old filename
        dialog_text_field = XmFileSelectionBoxGetChild(overlay_open_dialog,
                                                        XmDIALOG_TEXT);
        old_filename = XmTextFieldGetString(dialog_text_field);

        // Check the directory in the old filename
        if (extract_directory(old_filename, dir_str) == NULL)
        {
            // No directory in the old file open dialog filename
            XmTextFieldSetString(dialog_text_field, filename);
        }
        else
        {
            // There is a directory in the old name

            // Revert the directory in the file dialog.  This makes the
            // file open dialog box reset itself to the default directory.
            dir = XSTRING("");
            XtVaSetValues(overlay_open_dialog, XmNdirectory, dir, NULL);
            XmStringFree(dir);

            // Get the newly refreshed directory
            XmStringContext context;
            XmStringCharSet charset;
            XmStringDirection direction;
            Boolean separator;
            XtVaGetValues(overlay_open_dialog, XmNdirectory, &dir, NULL);
            XmStringInitContext(&context, dir);
            XmStringGetNextSegment(context, &new_dir, &charset,
                                    &direction, &separator);
            XtFree(new_dir);
            XmStringFreeContext(context);

            // Construct the new complete filename
            strcpy(new_filename_str, new_dir);
            strcat(new_filename_str, filename);
            XmTextFieldSetString(dialog_text_field, new_filename_str);
        }

        XtFree(old_filename);
    }
}

//-----------------------------------------------------------------------
static void file_open_ok_cb(
    Widget w,
    caddr_t client_data,
    XmFileSelectionBoxCallbackStruct* fcb)
{
    char* filename;

    // unexpose the file selection dialog
    XtUnmanageChild(file_open_dialog);

    // complain if commands are already executing
    if (executing_step_list)
    {
        warn_user(" Executing commands now!  Wait for completion\n"
                   " or ABORT before loading files.");
        return;
    }

    // make sure we have a valid filename to read from
    XmStringGetLtoR(fcb->value, XmSTRING_DEFAULT_CHARSET , &filename);
    if ((filename == NULL) || (strlen(filename) == 0))
    {
        warn_user("Commands NOT loaded successfully: No filename entered yet!");
        if (filename)
        {
            XtFree(filename);
        }
        return;
    }
    if (!file_exists(filename))
    {
        warn_userf("Commands NOT loaded successfully: \nfile '%s' does not exist!", filename);
        if (filename)
        {
            XtFree(filename);
        }
        return;
    }

    // load the commands
    if (gt_load_commands(filename) != 0)
    {
        warn_userf("Commands NOT loaded successfully from\n file: %s", filename);
    }
    gt_rewind_step_list();

    // Update the filename in the command panel
    set_command_panel_filename(filename);
    XtFree(filename);
}



//-----------------------------------------------------------------------
static void file_open_cancel_cb(Widget w, caddr_t client_data, caddr_t unused)
{
    XtUnmanageChild(w);
}

//-----------------------------------------------------------------------
static void create_file_open_dialog(Widget parent)
{
    Arg wargs[2];
    int n = 0;

    XmString title = XSTRING("Select Command File");

    XtSetArg(wargs[n], XmNdialogTitle, title); n++;
    file_open_dialog = XmCreateFileSelectionDialog(parent, "select-file",
                                                    wargs, n);
    if (file_open_dialog == NULL)
    {
        warn_user("Error creating the file open dialog box\n");
        return;
    }
    XmStringFree(title);

    XtUnmanageChild(XmFileSelectionBoxGetChild(file_open_dialog, XmDIALOG_HELP_BUTTON));

    // install the callbacks
    XtAddCallback(file_open_dialog, XmNokCallback,     (XtCallbackProc) file_open_ok_cb,     NULL);
    XtAddCallback(file_open_dialog, XmNcancelCallback, (XtCallbackProc) file_open_cancel_cb, NULL);
}

//-----------------------------------------------------------------------
void set_file_open_dialog_filename(char* filename)
{
    Widget dialog_text_field;
    char new_filename_str[256];
    char dir_str[256];
    char* new_dir;
    char* old_filename;
    XmString dir, new_filename, new_full_filename;

    // Combine the two filenames appropriately
    if (filename_has_directory(filename))
    {
        // We have a directory, use it to override the old one

        // Extract the directory and install it into the file open dialog
        extract_directory(filename, dir_str);
        dir = XSTRING(dir_str);
        XtVaSetValues(file_open_dialog, XmNdirectory, dir, NULL);
        XmStringFree(dir);

        // Retrieve the new directory from the file open dialog
        // (This gets the newly expanded version)
        XtVaGetValues(file_open_dialog, XmNdirectory, &dir, NULL);

        // Reintegrate the simple filename with the newly expanded directory
        remove_directory(filename, new_filename_str);
        new_filename = XSTRING(new_filename_str);
        new_full_filename = XmStringConcat(dir, new_filename);

        // Install the new full filename into the file open dialog
        XtVaSetValues(file_open_dialog, XmNtextString, new_full_filename, NULL);
        XmStringFree(new_filename);
        XmStringFree(new_full_filename);
    }
    else
    {
        // No directory in the new filename

        // Get the old filename
        dialog_text_field = XmFileSelectionBoxGetChild(file_open_dialog,
                                                        XmDIALOG_TEXT);
        old_filename = XmTextFieldGetString(dialog_text_field);

        // Check the directory in the old filename
        if (extract_directory(old_filename, dir_str) == NULL)
        {
            // No directory in the old file open dialog filename
            XmTextFieldSetString(dialog_text_field, filename);
        }
        else
        {
            // There is a directory in the old name

            // Revert the directory in the file dialog.  This makes the
            // file open dialog box reset itself to the default directory.
            dir = XSTRING("");
            XtVaSetValues(file_open_dialog, XmNdirectory, dir, NULL);
            XmStringFree(dir);

            // Get the newly refreshed directory
            XmStringContext context;
            XmStringCharSet charset;
            XmStringDirection direction;
            Boolean separator;
            XtVaGetValues(file_open_dialog, XmNdirectory, &dir, NULL);
            XmStringInitContext(&context, dir);
            XmStringGetNextSegment(context, &new_dir, &charset,
                                    &direction, &separator);
            XmStringFreeContext(context);

            // Construct the new complete filename
            strcpy(new_filename_str, new_dir);
            strcat(new_filename_str, filename);
            XtFree(new_dir);
            XmTextFieldSetString(dialog_text_field, new_filename_str);
        }

        XtFree(old_filename);
    }
}

//-----------------------------------------------------------------------
struct Proceed_data
{
    Widget w;
    int* flag;
    int timed;
    XtIntervalId timer_id;
};

//-----------------------------------------------------------------------
static void proceed_dialog_ok_cb(Widget w, Proceed_data* data)
{
    if (data->timed)
    {
        XtRemoveTimeOut(data->timer_id);
    }

    XtUnmanageChild(data->w);
    XtDestroyWidget(data->w);
    *(data->flag) = true;

    free(data);
}


//-----------------------------------------------------------------------
static void proceed_dialog_cancel_cb(Widget w, Proceed_data* data)
{
    if (data->timed)
    {
        XtRemoveTimeOut(data->timer_id);
    }

    XtUnmanageChild(data->w);
    XtDestroyWidget(data->w);
    *(data->flag) = -1;

    free(data);
}

//-----------------------------------------------------------------------
static void proceed_dialog_timeout_cb(Proceed_data* data, XtIntervalId id)
{
    XtUnmanageChild(data->w);
    XtDestroyWidget(data->w);
    *(data->flag) = true;

    free(data);
}

//-----------------------------------------------------------------------
void get_proceed_clearance(int *flag, int timeout_given, int timeout, char* fmt, ...)
{
    // popup a "proceed" dialog box with the specified message.
    // When the user presses [Proceed], the flag is set to true.
    // When the user presses [Abort], the flag is set to -1.

    va_list args;
    time_t current_time;
    char msg[512];
    char timeout_str[100];
    int x, y;

    Widget dialog;
    Proceed_data* data;

    Arg             wargs[MAXARGS];
    Cardinal        nwargs = 0;
    XmString title  = XSTRING("Proceed?");
    XmString ok     = XSTRING("   PROCEED   ");
    XmString cancel = XSTRING(" ABORT ");


    // Check for a timeout
    if (timeout_given)
    {
        if (timeout == 0)
        {
            *flag = true;
            return;
        }
        else
        {
            current_time = time(0);
        }
    }

    // allocate the proceed data structure
    if ((data  = (Proceed_data*) malloc(sizeof(Proceed_data))) == NULL)
    {
        fprintf(stderr, "Aborting: Error creating proceed dialog box data, out of memory!\n");
        *flag = -1;
        return;
    }

    // figure out where to put the dialog box
    x = proceed_dialog_x + proceed_dialog_dx * (proceed_dialog_num % 4);
    y = proceed_dialog_y + proceed_dialog_dy * (proceed_dialog_num % 4);

    // create this proceed dialog box
    XtSetArg(wargs[nwargs], XmNdialogTitle, title);             nwargs++;
    XtSetArg(wargs[nwargs], XmNdialogStyle, XmDIALOG_MODELESS); nwargs++;
    XtSetArg(wargs[nwargs], XmNokLabelString, ok);              nwargs++;
    XtSetArg(wargs[nwargs], XmNcancelLabelString, cancel);      nwargs++;
    XtSetArg(wargs[nwargs], XmNdefaultPosition, false);         nwargs++;
    XtSetArg(wargs[nwargs], XmNx, x);                           nwargs++;
    XtSetArg(wargs[nwargs], XmNy, y);                           nwargs++;
    dialog = XmCreateMessageDialog(main_window, "proceed-dialog", wargs, nwargs);
    if (dialog == NULL)
    {
        fprintf(stderr, "Aborting: Error creating proceed dialog box, out of memory!\n");
        *flag = -1;
        return;
    }
    proceed_dialog_num++;
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
    XmStringFree(title);
    XmStringFree(ok);
    XmStringFree(cancel);

    // If there is a timeout, set up a timeout callback
    if (timeout_given)
    {
        time_t final_time;
        XtIntervalId id;

        // determine how long to wait to timeout
        if (timeout > 0)
        {
            // A final time was specified
            tm* time;
            final_time = timeout;
            time = localtime((time_t*) &final_time);
            strftime(timeout_str, 100, "  (Timeout at %m-%d-%y %T)", time);
        }
        else
        {
            // A timeout was specified
            final_time = current_time + abs(timeout);
            sprintf(timeout_str, "  (Timeout in %d seconds)", abs(timeout));
        }

        // set up the proceed data structure
        data->w = dialog;
        data->flag = flag;
        data->timed = true;
        id = XtAppAddTimeOut(
            app, (unsigned long) ((final_time - current_time)*1000),
            (void (*)(void*, long unsigned int*)) proceed_dialog_timeout_cb,
            data);
        data->timer_id = id;
    }
    else
    {
        // No timeout: set up the proceed data structure
        data->w = dialog;
        data->flag = flag;
        data->timed = false;
        data->timer_id = 0;
        timeout_str[0] = '\0';
    }

    // install the string
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    strcat(msg, timeout_str);
    XtVaSetValues(dialog, RES_CONVERT(XmNmessageString, msg), NULL);
    XtAddCallback(dialog, XmNokCallback, (XtCallbackProc) proceed_dialog_ok_cb, data);
    XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc) proceed_dialog_cancel_cb, data);

    // pop up the dialog box
    XtManageChild(dialog);
}

//-----------------------------------------------------------------------
// Note: This dialog is different from the proceed dialog above.
//-----------------------------------------------------------------------
static void start_proceed_ok_cb(Widget w, XtPointer client_data, XtPointer call_data)
{
    int type = (int) client_data;
    XtUnmanageChild(w);
    XtDestroyWidget(w);

    switch (type)
    {
    case TYPE_A:
        gt_execute_step_list();
        break;
    case TYPE_B:
        send_robots_proceed_mission();
        break;
    }
}

//-----------------------------------------------------------------------
static void show_start_proceed()
{
    popup_start_proceed_dialog(TYPE_A);
}

//-----------------------------------------------------------------------
int popup_start_proceed_dialog(int type)
{
    XmString title = XSTRING("Proceed");
    XmString ok    = XSTRING("   OK   ");
    XmString str;
    Arg wargs[4];
    int n = 0;

    switch (type)
    {
    case TYPE_A:
        str = XSTRING("Press OK to Proceed.");
        break;
    case TYPE_B:
        str = XSTRING("Robot passed the diagnosis test.\nPress OK to Proceed.");
        break;
    }

    // Create the initial copyright notice message dialog box
    XtSetArg(wargs[n], XmNdialogTitle, title);               n++;
    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_WORK_AREA);  n++;
    XtSetArg(wargs[n], XmNokLabelString, ok);                n++;
    XtSetArg(wargs[n], XmNmessageString, str);               n++;
    start_proceed_dialog = XmCreateMessageDialog(main_window, "start-proceed-dialog", wargs, n);
    XtAddCallback(start_proceed_dialog, XmNokCallback,
                   (XtCallbackProc) start_proceed_ok_cb, (XtPointer)type);
    XmStringFree(title);
    XmStringFree(ok);
    XmStringFree(str);

    // get rid of the cancel and help buttons
    XtUnmanageChild(XmMessageBoxGetChild(start_proceed_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(start_proceed_dialog, XmDIALOG_HELP_BUTTON  ));

    // Pop it up
    XtManageChild(start_proceed_dialog);

    return true;
}

//-----------------------------------------------------------------------
// called to allow modification of the time scale
//-----------------------------------------------------------------------
static void time_scale_apply_cb(Widget w, Widget base)
{
    int i;

    // save the current cycle duration
    XmScaleGetValue(cycle_duration_scale, &i);
    cycle_duration = (double) i / 100.0;
}


//-----------------------------------------------------------------------
// called to allow modification of time scale
//-----------------------------------------------------------------------
static void time_scale_ok_cb(Widget w, Widget base)
{
    XtPopdown(time_scale_panel_shell);
    time_scale_apply_cb(w, base);
}



//-----------------------------------------------------------------------
// time_scale_cancel_cb: called to hide the time_scale panel
//-----------------------------------------------------------------------
static void time_scale_cancel_cb(Widget w, Widget base)
{
    XtPopdown(time_scale_panel_shell);
}

//-----------------------------------------------------------------------
// called to allow modification of the time scale
//-----------------------------------------------------------------------
static void virtual_clock_kill_cb()
{
  vClockAlive = false;
}


//-----------------------------------------------------------------------
// called to allow modification of the time scale
//-----------------------------------------------------------------------
static void virtual_clock_apply_cb(Widget w, Widget base)
{
    int alpha;

    // save the current cycle duration
    XmScaleGetValue(vclock_speed_scale, &alpha);
    virtual_clock_alpha = (double) alpha;
}


//-----------------------------------------------------------------------
// called to allow modification of time scale
//-----------------------------------------------------------------------
static void virtual_clock_ok_cb(Widget w, Widget base)
{
    XtPopdown(virtual_clock_panel_shell);
    time_scale_apply_cb(w, base);
}


//-----------------------------------------------------------------------
// virtual_clock_cancel_cb: called to hide the virtual_clock panel
//-----------------------------------------------------------------------
static void virtual_clock_cancel_cb(Widget w, Widget base)
{
    XtPopdown(virtual_clock_panel_shell);
}

//-----------------------------------------------------------------------
// create_time_scale_panel: called to allow modification of the time scale
//-----------------------------------------------------------------------
void gt_create_time_scale_panel(Widget parent)
{
    Widget form1, form2, widget;

    // create the popup
    time_scale_panel_shell = XtVaCreatePopupShell(
        "Time Scale", xmDialogShellWidgetClass, parent,
        XmNdeleteResponse, XmDESTROY,
        NULL);

    time_scale_panel = XtVaCreateWidget(
        "time-scale-panel", xmPanedWindowWidgetClass, time_scale_panel_shell,
        XmNsashHeight,        1,
        XmNsashWidth,         1,
        NULL);

    form1 = XtVaCreateWidget(
        "time-scale-form1", xmFormWidgetClass, time_scale_panel,
        NULL);

    cycle_duration_scale = XtVaCreateManagedWidget(
        "cycle-duration-scale", xmScaleWidgetClass, form1,
        RES_CONVERT(XmNtitleString,"Cycle duration (seconds per cycle)"),
        XmNminimum,		nint(MIN_CYCLE_DURATION * 100.0),
        XmNmaximum,		nint(MAX_CYCLE_DURATION * 100.0),
        XmNvalue,		        nint(cycle_duration * 100.0),
        XmNdecimalPoints,		2,
        XmNshowValue,		True,
        XmNorientation,		XmHORIZONTAL,
        XmNtopAttachment,	        XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL);

    XtManageChild(form1);


    form2 = XtVaCreateWidget(
        "time-scale-form2", xmFormWidgetClass, time_scale_panel,
        XmNfractionBase, 7,
        XmNskipAdjust,   True,
        NULL);

    widget = XtVaCreateManagedWidget(
        "  Ok  ", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 1,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                2,
        XmNshowAsDefault,                True,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) time_scale_ok_cb, NULL);

    widget = XtVaCreateManagedWidget(
        " Apply ", xmPushButtonGadgetClass,	form2,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 3,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                4,
        XmNshowAsDefault,                False,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) time_scale_apply_cb, NULL);

    widget = XtVaCreateManagedWidget(
        " Cancel ", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 5,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                6,
        XmNshowAsDefault,                False,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) time_scale_cancel_cb, NULL);

    XtManageChild(form2);
}

//-----------------------------------------------------------------------
void gt_popup_time_scale_panel(void)
{
    // Update the slider first
   XmScaleSetValue(cycle_duration_scale, nint(cycle_duration * 100.0));

   XtManageChild(time_scale_panel);
   XtPopup(time_scale_panel_shell, XtGrabNone);
}

//-----------------------------------------------------------------------
// create_virtual_clock_panel: called to display and modify virtual time
//-----------------------------------------------------------------------
void gt_create_virtual_clock_panel(Widget parent)
{
    Widget form1, form2, form3, widget;
    time_t localTime, t;

    // create the popup
    virtual_clock_panel_shell = XtVaCreatePopupShell(
        "Virtual Clock", xmDialogShellWidgetClass, parent,
        XmNdeleteResponse, XmDESTROY,
        NULL);

    virtual_clock_panel = XtVaCreateWidget(
        "virtual-clock-panel", xmPanedWindowWidgetClass, virtual_clock_panel_shell,
        XmNsashHeight,        1,
        XmNsashWidth,         1,
        NULL);

    form1 = XtVaCreateWidget(
        "virtual-clock-form1", xmFormWidgetClass, virtual_clock_panel,
	XmNfractionBase, 5,
        NULL);

    // Get the current time to use as default string
    localTime = time(&t);

    virtual_clock_text = XtVaCreateManagedWidget(
	"virtual-clock-display", xmTextWidgetClass, form1,
	RES_CONVERT(XmNvalue, ctime(&localTime)),
	XmNautoShowCursorPosition,  False,
	XmNeditable,                False,
	XmNtopAttachment,	    XmATTACH_FORM,
	XmNleftAttachment,          XmATTACH_POSITION,
	XmNleftPosition,          1,
	XmNrightAttachment,         XmATTACH_POSITION,
	XmNrightPosition,         4,
	XmNposition, 50,
	NULL);

    // If somebody tries to kill the dialog, we should handle it
    XtAddCallback(virtual_clock_panel_shell, XmNdestroyCallback, (XtCallbackProc)virtual_clock_kill_cb ,0);

    // NADEEM - this is here temporarily. Need to figure out how the actual
    // timer will run
    g_base_time = (long) localTime;

    // Now get the clock started and running
    gStartClock((Widget) NULL, (XtIntervalId *) NULL);

    XtManageChild(form1);

    form2 = XtVaCreateWidget(
        "virtual-clock-form2", xmFormWidgetClass, virtual_clock_panel,
        NULL);

    vclock_speed_scale = XtVaCreateManagedWidget(
        "clock-speed-scale", xmScaleWidgetClass, form2,
        RES_CONVERT(XmNtitleString,"Clock Speed (ticks per second)"),
	// NADEEM - Might need to adjust these scales later
        XmNminimum,		nint(MIN_CYCLE_DURATION * 10),
        XmNmaximum,		nint(MAX_CYCLE_DURATION * 10),
        //XmNvalue,		        nint(virtual_clock_alpha * 100.0),
        XmNvalue,		        nint(virtual_clock_alpha),
        XmNdecimalPoints,		0,
        XmNshowValue,		True,
        XmNorientation,		XmHORIZONTAL,
	XmNtopAttachment,	        XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL);

    XtManageChild(form2);

    form3 = XtVaCreateWidget(
        "virtual-clock-form2", xmFormWidgetClass, virtual_clock_panel,
        XmNfractionBase, 7,
        XmNskipAdjust,   True,
        NULL);

    widget = XtVaCreateManagedWidget(
        "  Ok  ", xmPushButtonGadgetClass, form3,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 1,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                2,
        XmNshowAsDefault,                True,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) virtual_clock_ok_cb, NULL);

    widget = XtVaCreateManagedWidget(
        " Apply ", xmPushButtonGadgetClass,	form3,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 3,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                4,
        XmNshowAsDefault,                False,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) virtual_clock_apply_cb, NULL);

    widget = XtVaCreateManagedWidget(
        " Cancel ", xmPushButtonGadgetClass, form3,
        XmNtopAttachment,                XmATTACH_FORM,
        XmNbottomAttachment,             XmATTACH_FORM,
        XmNleftAttachment,               XmATTACH_POSITION,
        XmNleftPosition,                 5,
        XmNrightAttachment,              XmATTACH_POSITION,
        XmNrightPosition,                6,
        XmNshowAsDefault,                False,
        XmNdefaultButtonShadowThickness, 1,
        NULL);
    XtAddCallback(widget, XmNactivateCallback, (XtCallbackProc) virtual_clock_cancel_cb, NULL);

    XtManageChild(form3);
}

//-----------------------------------------------------------------------
void gt_popup_virtual_clock_panel(void)
{
    // Update the slider first
    //XmScaleSetValue(vclock_speed_scale, nint(virtual_clock_alpha * 100.0));
    XmScaleSetValue(vclock_speed_scale, nint(virtual_clock_alpha));

    XtManageChild(virtual_clock_panel);
    XtPopup(virtual_clock_panel_shell, XtGrabNone);
}

void
gStartClock(Widget widget, XtIntervalId *interval_id)
{
  if (!vClockAlive)
    return;

  XmTextSetString(virtual_clock_text,ctime((time_t*)&g_base_time));

  g_base_time++;

  (void) XtAppAddTimeOut(app, (long unsigned int)(1000.0/virtual_clock_alpha),
			 (XtTimerCallbackProc) gStartClock,
			 (XtPointer) NULL);
}

//-----------------------------------------------------------------------
int set_cycle_duration(double dt)
{
    if (dt < MIN_CYCLE_DURATION)
    {
        warn_userf("Cycle duration value, %1.8g, is less than\n"
                    "the minimum value of %1.8g seconds", dt, MIN_CYCLE_DURATION);
        return false;
    }
    if (dt > MAX_CYCLE_DURATION)
    {
        warn_userf("Cycle duration value, %1.8g, is greater than\n"
                    "the maximum value of %1.8g seconds", dt, MAX_CYCLE_DURATION);
        return false;
    }

    cycle_duration = dt;
    XmScaleSetValue(cycle_duration_scale, nint(cycle_duration * 100.0));
    return true;
}

//-----------------------------------------------------------------------
static void toggle_show_halos(void)
{
    if (XmToggleButtonGetState(show_halos_button))
    {
        XmToggleButtonSetState(show_halos_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(show_halos_button, False, True);
    }

    highlight_repelling_obstacles = XmToggleButtonGetState(show_halos_button);

    if(!highlight_repelling_obstacles)
    {
        erase_halos();
    }
}

//-----------------------------------------------------------------------
void set_show_repelling_obstacles(int flag)
{
    XmToggleButtonSetState(show_halos_button, flag, True);
    highlight_repelling_obstacles = flag;
}

//-----------------------------------------------------------------------
static void toggle_show_vectors(void)
{
    if (XmToggleButtonGetState(show_vectors_button))
    {
        XmToggleButtonSetState(show_vectors_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(show_vectors_button, False, True);
    }

    show_movement_vectors = XmToggleButtonGetState(show_vectors_button);

    if(!show_movement_vectors)
    {
        erase_movement_vectors();
    }
}

//-----------------------------------------------------------------------
static void show_movement_field_callback(Widget w, XtPointer client_data, XtPointer call_data)
{
    // if the dialog doesn't exist, create it
    if (pobjVectorFieldDlg == NULL)
    {
        if (g_objVectorField.GetRobotID() < 0)
        {
            g_objVectorField.SetRobotID(GetFirstRobotID());
        }
        pobjVectorFieldDlg = new ClVectorFieldDlg(main_window, &pobjVectorFieldDlg, &g_objVectorField);
        pobjVectorFieldDlg->Create();
    }
    // if the dialog already exists, raise it to the front
    else
    {
        pobjVectorFieldDlg->Raise();
    }
}

//-----------------------------------------------------------------------
static void toggle_show_obstacles(void)
{
    if (XmToggleButtonGetState(show_obstacles_button))
    {
        XmToggleButtonSetState(show_obstacles_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(show_obstacles_button, False, True);
    }

    show_obstacles = XmToggleButtonGetState(show_obstacles_button);
}

//-----------------------------------------------------------------------
static void toggle_erase_obstacles(void)
{
    if (XmToggleButtonGetState(erase_obstacles_button))
    {
        XmToggleButtonSetState(erase_obstacles_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(erase_obstacles_button, False, True);
    }

    erase_obstacles = XmToggleButtonGetState(erase_obstacles_button);
}

//-----------------------------------------------------------------------
void set_show_movement_vectors(int flag)
{
    XmToggleButtonSetState(show_vectors_button, flag, True);
    show_movement_vectors = flag;
}

//-----------------------------------------------------------------------
void set_learning_momentum_enabled(int flag)
{
    learning_momentum_enabled = flag;

    gt_update(
        BROADCAST_ROBOTID,
        szLM_ENABLED_LABEL,
        learning_momentum_enabled ? "1" : "0");
}

//-----------------------------------------------------------------------
static void toggle_lm_enabled(void)
{
    if (XmToggleButtonGetState(lm_enabled_button))
    {
        XmToggleButtonSetState(lm_enabled_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(lm_enabled_button, False, True);
    }

    set_learning_momentum_enabled(XmToggleButtonGetState(lm_enabled_button));
}

//-----------------------------------------------------------------------
void set_save_learning_momentum_weights(int flag)
{
    save_learning_momentum_weights = flag;
}

//-----------------------------------------------------------------------
static void toggle_save_learning_momentum_weights(void)
{
    if (XmToggleButtonGetState(save_lm_weights_button))
    {
        XmToggleButtonSetState(save_lm_weights_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(save_lm_weights_button, False, True);
    }

    set_save_learning_momentum_weights(XmToggleButtonGetState(save_lm_weights_button));
}

//-----------------------------------------------------------------------
void set_save_sensor_readings(int flag)
{
    save_sensor_readings = flag;
}

//-----------------------------------------------------------------------
static void toggle_save_sensor_readings(void)
{
    if (XmToggleButtonGetState(save_sensor_readings_button))
    {
        XmToggleButtonSetState(save_sensor_readings_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(save_sensor_readings_button, False, True);
    }

    set_save_sensor_readings(XmToggleButtonGetState(save_sensor_readings_button));
}

//-----------------------------------------------------------------------
static void toggle_lm_param_view(Widget w, XtPointer client_data, XtPointer clall_data)
{
    SuLMViewNode* psuCurrent = (SuLMViewNode*) malloc(sizeof(SuLMViewNode));

    if (psuCurrent != NULL)
    {
        psuCurrent->psuNext = g_psuLMViewDialogList;
        g_psuLMViewDialogList = psuCurrent;

        InitializeLMParamDialog(&(psuCurrent->suDialog));
        SetLMParamDialogRobotID(&(psuCurrent->suDialog), find_first_robot_rec());
        PopupLMParamDialog(&(psuCurrent->suDialog), main_window);
    }
    else
    {
        fprintf(stderr, "Error: out of memory\n");
    }
}

//-----------------------------------------------------------------------
void DeleteLMParamDialog(Widget dialog)
{
    DeleteLMParamDialogHelper(dialog, &g_psuLMViewDialogList);
}

//-----------------------------------------------------------------------
void DeleteLMParamDialogHelper(Widget dialog, SuLMViewNode** ppsuCurrent)
{
    // make sure we're not at the end of the list
    if ((*ppsuCurrent != NULL) && (dialog != NULL))
    {
        // if the current node has the dialog we want to delete, delete it
        if (GetLMParamDialog(&((*ppsuCurrent)->suDialog)) == dialog)
        {
            SuLMViewNode* psuTemp = *ppsuCurrent;
            *ppsuCurrent = (*ppsuCurrent)->psuNext;
            free(psuTemp);
        }
        // move to the next node in the list
        else
        {
            DeleteLMParamDialogHelper(dialog, &(*ppsuCurrent)->psuNext);
        }
    }
}

//-----------------------------------------------------------------------
void GetNextRobotForLMParamDialog(Widget dialog)
{
    int iNewRobotID;
    SuLMViewNode* psuCurrent = g_psuLMViewDialogList;

    if (dialog != NULL)
    {
        // get the desired dialog node
        while ((psuCurrent != NULL) &&
                (GetLMParamDialog(&(psuCurrent->suDialog)) != dialog))
        {
            psuCurrent = psuCurrent->psuNext;
        }

        // if the desired node was found, get the next robot ID
        if (psuCurrent != NULL)
        {
            iNewRobotID = find_first_robot_rec();

            while ((iNewRobotID >= 0) &&
                    (iNewRobotID != GetLMParamRobotID(&(psuCurrent->suDialog))))
            {
                iNewRobotID = find_next_robot_rec();
            }
            iNewRobotID = find_next_robot_rec();

            // if there was no "next", wrap around to the first
            if (iNewRobotID < 0)
            {
                iNewRobotID = find_first_robot_rec();
            }

            // if the ID changed, re-initialize the dialog
            if ((iNewRobotID >= 0) &&
                 (iNewRobotID != GetLMParamRobotID(&(psuCurrent->suDialog))))
            {
                InitializeLMParamDialog(&(psuCurrent->suDialog));
                SetLMParamDialogRobotID(&(psuCurrent->suDialog), iNewRobotID);
                UpdateLMParamDialogDisplay(&(psuCurrent->suDialog));
            }
        }
    }
}

//-----------------------------------------------------------------------
void UpdateLMParamsFor(int iRobotID)
{
    SuLMViewNode* psuCurrent = g_psuLMViewDialogList;

    SuLMParams suValues;
    char* szTemp;

    suValues.fGoalGain          = 0.0;
    suValues.fAvoidObstacleGain = 0.0;
    suValues.fObstacleSphere    = 0.0;
    suValues.fWanderGain        = 0.0;
    suValues.iWanderPersistence =   0;

    // try to get all of the gains
    if ((szTemp = gt_inquire(iRobotID, (char*) szMOVE_TO_GOAL_GAIN_LABEL)) != NULL)
    {
        suValues.fGoalGain = (float) atof(szTemp);
    }
    if ((szTemp = gt_inquire(iRobotID, (char*) szOBJECT_GAIN_LABEL)) != NULL)
    {
        suValues.fAvoidObstacleGain = (float) atof(szTemp);
    }
    if ((szTemp = gt_inquire(iRobotID, (char*) szSPHERE_OF_INFLUENCE_LABEL)) != NULL)
    {
        suValues.fObstacleSphere = (float) atof(szTemp);
    }
    if ((szTemp = gt_inquire(iRobotID, (char*) szWANDER_GAIN_LABEL)) != NULL)
    {
        suValues.fWanderGain = (float) atof(szTemp);
    }
    if ((szTemp = gt_inquire(iRobotID, (char*) szWANDER_PERSISTENCE_LABEL)) != NULL)
    {
        suValues.iWanderPersistence = atoi(szTemp);
    }

    // loop through all windows
    while (psuCurrent != NULL)
    {
        // if the current window matches the robot ID, update it
        if (GetLMParamRobotID(&(psuCurrent->suDialog)) == iRobotID)
        {
            UpdateLMParamDialogValue(&(psuCurrent->suDialog), suValues);
            UpdateLMParamDialogDisplay(&(psuCurrent->suDialog));
        }

        psuCurrent = psuCurrent->psuNext;
    }
}

//-----------------------------------------------------------------------
void UpdateGPS(int iRobotID)
{
    if (bTelopStart)
    {
        UpdateTargetDisplay();
    }
}

//-----------------------------------------------------------------------
static void toggle_show_trails(void)
{
    if (XmToggleButtonGetState(show_trails_button))
    {
        XmToggleButtonSetState(show_trails_button, true, true);
    }
    else
    {
        XmToggleButtonSetState(show_trails_button, false, true);
    }

    gt_show_trails = XmToggleButtonGetState(show_trails_button);
}

//-----------------------------------------------------------------------
static void toggle_use_military_style(void)
{
    if (XmToggleButtonGetState(use_military_style_button))
    {
        XmToggleButtonSetState(use_military_style_button, true, true);
    }
    else
    {
        XmToggleButtonSetState(use_military_style_button, false, true);
    }

    gt_draw_robot_military_style = XmToggleButtonGetState(use_military_style_button);
}

//-----------------------------------------------------------------------
static void toggle_3d_vis(void)
{
#ifndef NO_OPENGL
    // if we don't have a heightmap for the overlay, return
    if (heightmap_name == NULL)
    {
        return;
    }

    // initialize the 3d obs and such
    if (gt_show_3d_vis == false)
    {
        gt_show_3d_vis = true;
        initialize_3d_vis();
        fprintf(stderr, "3D visualization initialized.\n");
    }

    // delete the visualization objects and refresh
    else
    {
        gt_show_3d_vis = false;

        if (Obs_Handler != NULL)
        	delete Obs_Handler;

        if (Rob_Handler != NULL)
        	delete Rob_Handler;

        Obs_Handler = NULL;
        Rob_Handler = NULL;
        run_refresh_button();
    }
#endif
}

//-----------------------------------------------------------------------
void set_show_trails(int flag)
{
    XmToggleButtonSetState(show_trails_button, flag, true);
    gt_show_trails = flag;
}

//-----------------------------------------------------------------------
void use_military_style(int flag)
{
    XmToggleButtonSetState(use_military_style_button, flag, true);
    gt_draw_robot_military_style = flag;
}

//-----------------------------------------------------------------------
static void toggle_circle_robots(void)
{
    if (XmToggleButtonGetState(circle_robots_button))
    {
        XmToggleButtonSetState(circle_robots_button, true, true);
    }
    else
    {
        XmToggleButtonSetState(circle_robots_button, false, true);
    }

    gt_circle_robots = XmToggleButtonGetState(circle_robots_button);
}

//-----------------------------------------------------------------------
void set_circle_robots(int flag)
{
    XmToggleButtonSetState(circle_robots_button, flag, True);
    gt_circle_robots = flag;
}

//-----------------------------------------------------------------------
static void toggle_laser_normal(void)
{
    if (XmToggleButtonGetState(laser_normal_button))
    {
        XmToggleButtonSetState(laser_normal_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(laser_normal_button, False, True);
    }

    laser_normal_mode = XmToggleButtonGetState(laser_normal_button);
}

//-----------------------------------------------------------------------
static void toggle_laser_connected(void)
{
    if (XmToggleButtonGetState(laser_connected_button))
    {
        XmToggleButtonSetState(laser_connected_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(laser_connected_button, False, True);
    }

    laser_connected_mode = XmToggleButtonGetState(laser_connected_button);
}

//-----------------------------------------------------------------------
static void toggle_sonar_arc(void)
{
    if (XmToggleButtonGetState(sonar_arc_button))
    {
        XmToggleButtonSetState(sonar_arc_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(sonar_arc_button, False, True);
    }

    sonar_arc_mode = XmToggleButtonGetState(sonar_arc_button);
}

//-----------------------------------------------------------------------
static void toggle_sonar_point(void)
{
    if (XmToggleButtonGetState(sonar_point_button))
    {
        XmToggleButtonSetState(sonar_point_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(sonar_point_button, False, True);
    }

    sonar_point_mode = XmToggleButtonGetState(sonar_point_button);
}

//-----------------------------------------------------------------------
static void toggle_LOS(void)
{
    if (XmToggleButtonGetState(LOS_button))
    {
        XmToggleButtonSetState(LOS_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(LOS_button, False, True);
    }

    LOS_mode = XmToggleButtonGetState(LOS_button);
    uf = true;
}

//-----------------------------------------------------------------------
static void toggle_laser_linear(void)
{
    if (XmToggleButtonGetState(laser_linear_button))
    {
        XmToggleButtonSetState(laser_linear_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(laser_linear_button, False, True);
    }

    laser_linear_mode = XmToggleButtonGetState(laser_linear_button);
}

//-----------------------------------------------------------------------
static void toggle_debug_robots(void)
{
    if (XmToggleButtonGetState(debug_robots_button))
    {
        XmToggleButtonSetState(debug_robots_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(debug_robots_button, False, True);
    }

    robot_debug_messages = XmToggleButtonGetState(debug_robots_button);

    // send the state to the robots
    send_robots_debug_state(robot_debug_messages);
}

//-----------------------------------------------------------------------
void set_debug_robots(int flag)
{
    XmToggleButtonSetState(debug_robots_button, flag, True);
    robot_debug_messages = flag;

    // send the state to the robots
    send_robots_debug_state(robot_debug_messages);
}

//-----------------------------------------------------------------------
// Toggles the option for "Report Current State" feature.
//-----------------------------------------------------------------------
static void toggle_report_current_state()
{
    if (XmToggleButtonGetState(report_current_state_button))
    {
        XmToggleButtonSetState(report_current_state_button, True, True);
        report_userf(NULL);
    }
    else
    {
        XmToggleButtonSetState(report_current_state_button, False, True);
        XtPopdown(status_dialog);
    }
    report_state_messages = XmToggleButtonGetState(report_current_state_button);

    // send the state to the robots
    send_robots_report_state(report_state_messages);
}

//-----------------------------------------------------------------------
void set_report_current_state(int flag)
{
    XmToggleButtonSetState(report_current_state_button, flag, True);
    report_state_messages = flag;

    // send the state to the robots
    send_robots_report_state(report_state_messages);
}

//-----------------------------------------------------------------------
static void toggle_debug(void)
{
    if (XmToggleButtonGetState(debug_button))
    {
        XmToggleButtonSetState(debug_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(debug_button, False, True);
    }

    debug = XmToggleButtonGetState(debug_button);
}

//-----------------------------------------------------------------------
void set_debug_simulator(int flag)
{
    XmToggleButtonSetState(debug_button, flag, True);
    debug = flag;
}

//-----------------------------------------------------------------------
static void toggle_scheduler_debug(void)
{
    if (XmToggleButtonGetState(debug_scheduler_button))
    {
        XmToggleButtonSetState(debug_scheduler_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(debug_scheduler_button, False, True);
    }

    debug_scheduler = XmToggleButtonGetState(debug_scheduler_button);
}

//-----------------------------------------------------------------------
void set_debug_scheduler(int flag)
{
    XmToggleButtonSetState(debug_scheduler_button, flag, True);
    debug_scheduler = flag;
}

//-----------------------------------------------------------------------
// Toggles the option for "Log Robot Data" feature.
//-----------------------------------------------------------------------
static void toggle_log_robot_data(void)
{
    if (XmToggleButtonGetState(log_robot_data_button))
    {
        XmToggleButtonSetState(log_robot_data_button, True, True);
    }
    else
    {
        XmToggleButtonSetState(log_robot_data_button, False, True);
    }

    log_robot_data = XmToggleButtonGetState(log_robot_data_button);
}

//-----------------------------------------------------------------------
void set_log_robot_data(int flag)
{
    XmToggleButtonSetState(log_robot_data_button, flag, True);
    log_robot_data = flag;
}

//-----------------------------------------------------------------------
static void show_initial_copyright_ok_cb(void)
{
    XtUnmanageChild(initial_copyright_dialog);
    XtDestroyWidget(initial_copyright_dialog);
}

//-----------------------------------------------------------------------
static int show_initial_copyright(void)
{
    XmString title = XSTRING("Copyright Notice");
    XmString ok    = XSTRING("   OK   ");
    XmString copyright;
    Arg wargs[4];
    int n=0;

    char copyright_notice_str[3000];
    char* copyright_fmt =  "PLEASE READ THE FOLLOWING COPYRIGHT NOTICE:\n\
\n\
MissionLab v%s\n\
\n\
%s\n\
\n\
NOTE: To disable this notification in the future, invoke mlab with the \"-n\" option.";

    // Initialize the copyright strings
    sprintf(
        copyright_notice_str,
        copyright_fmt,
        version_str,
        copyright_str);
    copyright = XSTRING(copyright_notice_str);

    // Create the initial copyright notice message dialog box
    XtSetArg(wargs[n], XmNdialogTitle, title);                            n++;
    XtSetArg(wargs[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);  n++;
    XtSetArg(wargs[n], XmNokLabelString, ok);                             n++;
    XtSetArg(wargs[n], XmNmessageString, copyright);                      n++;
    initial_copyright_dialog =
        XmCreateMessageDialog(main_window, "initial-copyright-dialog", wargs, n);
    XtAddCallback(initial_copyright_dialog, XmNokCallback,
                   (XtCallbackProc) show_initial_copyright_ok_cb, NULL);
    XmStringFree(title);
    XmStringFree(ok);
    XmStringFree(copyright);

    // get rid of the cancel and help buttons
    XtUnmanageChild(XmMessageBoxGetChild(initial_copyright_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(initial_copyright_dialog, XmDIALOG_HELP_BUTTON));

    // Pop it up
    XtManageChild(initial_copyright_dialog);

    return true;
}

//-----------------------------------------------------------------------
// Quits mlab process.
//-----------------------------------------------------------------------
void quit_mlab(void)
{
    if (use_ipt)
    {
        clear_robots();
        gt_close_communication();
    }

    if (gJBoxMlab)
    {
        delete gJBoxMlab;
        gJBoxMlab = NULL;
    }

    if (gMMD)
    {
        delete gMMD;
        gMMD = NULL;
    }

    if (gPlayBack)
    {
        delete gPlayBack;
        gPlayBack = NULL;
    }

    if (gAssistantDialog)
    {
        delete gAssistantDialog;
        gAssistantDialog = NULL;
    }

    gEventLogging->end("mlab");
    gEventLogging->pause();

    exit(0);
}

//-----------------------------------------------------------------------
// Quits mlab process.
//-----------------------------------------------------------------------
void call_quit_mlab(void)
{
    XtAppAddWorkProc(app, (XtWorkProc)quit_mlab, NULL);
}

//-----------------------------------------------------------------------
void clear_drawing_area(void)
{
    Display* dpy = XtDisplay(drawing_area);
    Screen* scrn = XtScreen(drawing_area);

    // clear it
    XSetForeground(
        dpy,
        gGCs.black,
        WhitePixelOfScreen(scrn));


    XFillRectangle(
        dpy,
        drawing_area_pixmap,
        gGCs.black,
        0, 0,
        (int)(mission_area_width_meters / meters_per_pixel), (int)(mission_area_height_meters / meters_per_pixel));

    XSetForeground(
        dpy,
        gGCs.black,
        BlackPixelOfScreen(scrn));

    // And maybe copy the photo into it
    if (gMapInfo->photo_loaded)
    {
        draw_photo(gMapInfo);
    }
}

//-----------------------------------------------------------------------
// This function saves the pixel by its color name.
//-----------------------------------------------------------------------
void savePixelByColorName(Pixel pixel, string colorName)
{
    // Convert the name to the lower case.
    transform(
        colorName.begin(),
        colorName.end(),
        colorName.begin(),
        (int(*)(int))tolower);

    pixelByColorNameList[colorName] = pixel;
}

//-----------------------------------------------------------------------
// This function returns the predefined pixel by its color name.
//-----------------------------------------------------------------------
Pixel getPixelByColorName(string colorName)
{
    Display *display = NULL;
    Colormap cmap;
    XColor color, ignore;
    Pixel pixel;
    string key;
    map<string, Pixel>::iterator itr;
    int screenNum;

    key = colorName;

    // Convert the name to the lower case.
    transform(
        key.begin(),
        key.end(),
        key.begin(),
        (int(*)(int))tolower);

    itr = pixelByColorNameList.find(key);

    if (itr == pixelByColorNameList.end())
    {
        display = XtDisplay(drawing_area);
        screenNum = DefaultScreen(display);
        cmap = DefaultColormap(display, screenNum);

        if (XAllocNamedColor(display, cmap, (char *)(colorName.c_str()), &color, &ignore))
        {
            pixel = color.pixel;
            savePixelByColorName(pixel, key);
        }
        else
        {
            fprintf(
                stderr,
                "Error(mlab): getPixelByColorName() [%s]. 'Pixel for %s' could not be allocated.\n",
                __FILE__,
                colorName.c_str());

            pixel = BlackPixel(display, screenNum);
        }
    }
    else
    {
        pixel = itr->second;
    }

    return pixel;
}

//-----------------------------------------------------------------------
// This function saves the GC by its color name.
//-----------------------------------------------------------------------
void saveGCByColorName(GC gc, string colorName, bool isXOR)
{
    // Convert the name to the lower case.
    transform(
        colorName.begin(),
        colorName.end(),
        colorName.begin(),
        (int(*)(int))tolower);

    if (isXOR)
    {
        gcXORByColorNameList[colorName] = gc;
    }
    else
    {
        gcByColorNameList[colorName] = gc;
    }
}

//-----------------------------------------------------------------------
// This function returns the GC by its color name.
//-----------------------------------------------------------------------
GC getGCByColorName(string colorName, bool isXOR)
{
    GC gc;
    XGCValues xgcv;
    string key;
    map<string, GC> gclist;
    map<string, GC>::iterator itr;

    key = colorName;

    // Convert the name to the lower case.
    transform(
        key.begin(),
        key.end(),
        key.begin(),
        (int(*)(int))tolower);

    gclist = (isXOR)? gcXORByColorNameList : gcByColorNameList;

    itr = gclist.find(key);

    if (itr == gclist.end())
    {
        xgcv.foreground = getPixelByColorName(key);

        if (isXOR)
        {
            gc = XtGetGC(drawing_area, GCForeground | GCBackground, &xgcv);
            XSetFunction(display, gc, GXxor);
        }
        else
        {
            gc = XtGetGC(drawing_area, GCForeground, &xgcv);
        }

        saveGCByColorName(gc, colorName, isXOR);
    }
    else
    {
        gc = itr->second;
    }

    return gc;
}

//-----------------------------------------------------------------------
// This function initializes the global color pixels.
//-----------------------------------------------------------------------
void initialize_global_color_pixels(void)
{
    Display* dpy = XtDisplay(drawing_area);
    int scr = DefaultScreen(dpy);
    Colormap cmap = DefaultColormap(dpy, scr);
    XColor color, ignore;

    // Black
    gColorPixel.black = BlackPixel(dpy, scr);

    if(!XParseColor(dpy, cmap, "black", &(gXColors.black)))
    {
        fprintf(stderr, "Warning: XColor black not parsed.\n");
    }
    else
    {
        savePixelByColorName(gColorPixel.black, "black");
    }

    // White
    gColorPixel.white = WhitePixel(dpy, scr);

    if(!XParseColor(dpy, cmap, "white", &(gXColors.white)))
    {
        fprintf(stderr, "Warning: XColor white not parsed.\n");
    }
    else
    {
        savePixelByColorName(gColorPixel.white, "white");
    }

    // Gray80
    if (XAllocNamedColor(dpy, cmap, "gray80", &color, &ignore))
    {
        gColorPixel.gray80 = color.pixel;
        savePixelByColorName(gColorPixel.gray80, "gray80");
    }
    else
    {
        fprintf(stderr, "Warning: gray80 pixel not allocated.\n");
        gColorPixel.gray80 = gColorPixel.black;
    }

    // Red
    if (XAllocNamedColor(dpy, cmap, "red", &color, &ignore))
    {
        gColorPixel.red = color.pixel;
        savePixelByColorName(gColorPixel.red, "red");
    }
    else
    {
        fprintf(stderr, "Warning: red pixel not allocated.\n");
        gColorPixel.red = gColorPixel.gray80;
    }

    // Green
    if (XAllocNamedColor(dpy, cmap, "green", &color, &ignore))
    {
        gColorPixel.green = color.pixel;
        savePixelByColorName(gColorPixel.green, "green");
    }
    else
    {
        fprintf(stderr, "Warning: green pixel not allocated.\n");
        gColorPixel.green = gColorPixel.gray80;
    }

    // Green4
    if (XAllocNamedColor(dpy, cmap, "green4", &color, &ignore))
    {
        gColorPixel.green4 = color.pixel;
        savePixelByColorName(gColorPixel.green4, "green4");
    }
    else
    {
        fprintf(stderr, "Warning: green pixel not allocated.\n");
        gColorPixel.green = gColorPixel.gray80;
    }

    // Blue
    if (XAllocNamedColor(dpy, cmap, "blue", &color, &ignore))
    {
        gColorPixel.blue = color.pixel;
        savePixelByColorName(gColorPixel.blue, "blue");
    }
    else
    {
        fprintf(stderr, "Warning: blue pixel not allocated.\n");
        gColorPixel.blue = gColorPixel.gray80;
    }

    // Yellow
    if (XAllocNamedColor(dpy, cmap, "yellow", &color, &ignore))
    {
        gColorPixel.yellow = color.pixel;
        savePixelByColorName(gColorPixel.yellow, "yellow");
    }
    else
    {
        fprintf(stderr, "Warning: yellow pixel not allocated.\n");
        gColorPixel.yellow = gColorPixel.gray80;
    }

    // Magenta
    if (XAllocNamedColor(dpy, cmap, "magenta", &color, &ignore))
    {
        gColorPixel.magenta = color.pixel;
        savePixelByColorName(gColorPixel.magenta, "magenta");
    }
    else
    {
        fprintf(stderr, "Warning: magenta pixel not allocated.\n");
        gColorPixel.magenta = gColorPixel.gray80;
    }

    // Cyan
    if (XAllocNamedColor(dpy, cmap, "cyan", &color, &ignore))
    {
        gColorPixel.cyan = color.pixel;
        savePixelByColorName(gColorPixel.cyan, "cyan");
    }
    else
    {
        fprintf(stderr, "Warning: cyan pixel not allocated.\n");
        gColorPixel.cyan = gColorPixel.gray80;
    }

    // Orange
    if (XAllocNamedColor(dpy, cmap, "orange", &color, &ignore))
    {
        gColorPixel.orange = color.pixel;
        savePixelByColorName(gColorPixel.orange, "orange");
    }
    else
    {
        fprintf(stderr, "Warning: orange pixel not allocated.\n");
        gColorPixel.orange = gColorPixel.gray80;
    }
}

//-----------------------------------------------------------------------
void initialize_drawing_area(void)
{
    int fg, bg, impact_pixel, attractor_pixel, robot_pixel, basket_pixel, whitepixel;

    Display* dpy = XtDisplay(drawing_area);
    int scr = DefaultScreen(dpy);
    Visual* vis = DefaultVisual(dpy, scr);

    initialize_global_color_pixels();

    // Initialize them in black.
    impact_pixel = gColorPixel.black;
    attractor_pixel = gColorPixel.black;
    robot_pixel = gColorPixel.black;
    basket_pixel = gColorPixel.black;

    // figure out background and foreground for various types of systems
    if (vis->map_entries > 2)
    {
        fg = gColorPixel.white;
        whitepixel = gColorPixel.white;

        bg = gColorPixel.blue;
        robot_pixel = gColorPixel.blue;
        impact_pixel = gColorPixel.red;
        attractor_pixel = gColorPixel.orange;
        basket_pixel = gColorPixel.green;

        XtVaSetValues(
            drawing_area,
            XmNborderWidth, 1,
            XmNborderColor, gColorPixel.black,
            NULL);
    }
    else
    {
        // It's a monochrome display
        monochrome = true;

        XtVaGetValues(
            drawing_area,
            XmNforeground, &fg,
            XmNbackground, &bg,
            NULL);

        XtVaSetValues(
            drawing_area,
            XmNborderWidth, 1,
            XmNborderColor, fg,
            NULL);
    }

    // define a GC to xor
    gGCs.XOR = createGC(
        bg,
        fg,
        drawing_area,
        0,
        false,
        true);

    gGCs.whiteXOR = createGC(
        fg,
        bg,
        drawing_area,
        0,
        false,
        true);

    saveGCByColorName(gGCs.whiteXOR, "white", true);

    gGCs.redXOR = createGC(
        gColorPixel.red,
        fg,
        drawing_area,
        0,
        false,
        true);

    saveGCByColorName(gGCs.redXOR, "red", true);

    gGCs.greenXOR = createGC(
        gColorPixel.green,
        fg,
        drawing_area,
        0,
        false,
        true);

    saveGCByColorName(gGCs.greenXOR, "green", true);

    gGCs.attractor = createGC(
        attractor_pixel,
        fg,
        drawing_area,
        0,
        false,
        true); // XOR

    gGCs.basket = createGC(
        basket_pixel,
        fg,
        drawing_area,
        0,
        false,
        true); // XOR

    gGCs.robot = createGC(
        robot_pixel,
        fg,
        drawing_area,
        0,
        false,
        true); // XOR

    gGCs.black = createGC(
        fg,
        bg,
        drawing_area,
        0,
        false,
        false);

    saveGCByColorName(gGCs.black, "black", false);

    gGCs.green = createGC(
        gColorPixel.green,
        bg,
        drawing_area,
        0,
        false,
        false);

    saveGCByColorName(gGCs.green, "green", false);

    gGCs.erase = createGC(
        whitepixel,
        fg,
        drawing_area,
        LINE_WIDTH,
        false,
        false);

}

//-----------------------------------------------------------------------
// called to clear (ie, redraw) the map
//-----------------------------------------------------------------------
void clear_map(void)
{
    g_objVectorField.EraseVectors();
    erase_robots();
    clear_drawing_area();
    redraw_map();
}

//-----------------------------------------------------------------------
void scroll_drawing_area_to_bottom(void)
{
    int value, slider_size, increment, page_increment, maximum;
    XtVaGetValues(drawing_area_scroll_bar,
                   XmNsliderSize, &slider_size,
                   XmNincrement, &increment,
                   XmNpageIncrement, &page_increment,
                   XmNmaximum, &maximum,
                   NULL);
    value = maximum - slider_size;
    XmScrollBarSetValues(drawing_area_scroll_bar,
                          value, slider_size, increment, page_increment,
                          True);
}



//-----------------------------------------------------------------------
void set_mission_area_size(double width, double height)  // in meters
{
    double mpp;
    double width_pixels, height_pixels;
    double mission_area_aspect_ratio;

    // Scale meters_per_pixel so that work area fits into the drawing area
    width_pixels = drawing_area_width_pixels;
    height_pixels = drawing_area_height_pixels;
    mission_area_aspect_ratio = width / height;
    mpp = width / width_pixels;

    if (drawing_area_height_pixels * mission_area_aspect_ratio >= drawing_area_width_pixels)
    {
        // The work area width fits, but the work area height has to be shorter
        // than the drawing area height
        height_pixels = (double) drawing_area_width_pixels / mission_area_aspect_ratio;
        mpp = width / width_pixels;
    }
    else if (drawing_area_width_pixels / mission_area_aspect_ratio >= drawing_area_height_pixels)
    {
        // work area height fits, but the work area width has to be smaller
        // than the drawing area width
        width_pixels = (double) drawing_area_height_pixels * mission_area_aspect_ratio;
        mpp = height / height_pixels;
    }

    // set the values
    mission_area_width_meters = width;
    mission_area_height_meters = height;
    if (map_scale_factor == -1) {
      set_meters_per_pixel(mpp);
    }
    else {
      set_meters_per_pixel(map_scale_factor / pixelsPerMeter);  // NOTE: pixelsPerMeter is a constant
    }

    // Recompute a few length-related things
    recompute_obstacle_radius_range();
    recompute_robot_length();

    set_zoom_factor(1.0);


    // reset the 3d visualization info if needed
    if (heightmap_name == NULL) {
      load_heightmap("blank", (int)mission_area_width_meters, (int)mission_area_height_meters, 0, 1);
    }
    else {
      if (!strcmp(heightmap_name, "blank")) {
	load_heightmap("blank", (int)mission_area_width_meters, (int)mission_area_height_meters, 0, 1);
      }
    }
}

//-----------------------------------------------------------------------
// called when an item on the cmd menu is selected
//-----------------------------------------------------------------------
static void cmd_cb(Widget w, int item_num)
{
    // DCM: Redid this so still works when menu items come and go based on ifdefs
    int item = 0;
    if(item_num == item++)
    {
        gt_popup_command_panel();
    }
    else if(item_num == item++)
    {
        gt_popup_cmdli_panel();
    }
    else if(item_num == item++)
    {
        gt_popup_sound_simulation_interface();
    }
    else if(item_num == item++)
    {
        gt_popup_motivational_vector_interface();
    }
#ifdef TELOP
    else if(item_num == item++)
    {
        if (telop_num_robots == 0)
        {
            popup_telop_interface();
        }
        else
        {
            warn_userf("Error: Unable to activate teleoperation!\n"
                        "Already teleoperating unit '%s'", telop_unit_name);
        }
    }

    else if(item_num == item++)
    {
        if (telop_num_robots == 0)
        {
            gt_popup_telop_personality(NULL, 0, NULL);
        }
        else
        {
            warn_userf("Error: Unable to activate personlaity window!\n"
                        "Already teleoperating unit '%s'", telop_unit_name);
        }
    }
#endif // def TELOP
    else
    {
        // unknown item_num
    }
}

//-----------------------------------------------------------------------
static void create_command_menu(Widget menu_bar)
{
    XmString command     = XmStringCreateLocalized("Command Interface");
    XmString accel_label = XmStringCreateLocalized("^C");

    gt_create_command_panel(main_window);

    gt_create_sound_simulation_interface(top_level, app);
    gt_create_motivational_vector_interface(top_level, app);
    XmString sound = XmStringCreateLocalized("Sound Simulation");
    XmString motivational_vector = XmStringCreateLocalized("Motivational vector");

#ifdef TELOP
    gt_create_telop_interface(top_level, app, personality_filename);
    XmString telop = XmStringCreateLocalized("Telop Interface");
    XmString personality = XmStringCreateLocalized("Personality");
#endif

    XmString cmdli = XmStringCreateLocalized( "CMDLi Interface" );
    XmString cmdli_accel_label = XmStringCreateLocalized( "^I" );

    XmVaCreateSimplePulldownMenu(
        menu_bar, "cmd-menu", 2, (XtCallbackProc) cmd_cb,
        XmVaPUSHBUTTON, command, 'C', "Ctrl<Key>c", accel_label,
        XmVaPUSHBUTTON, cmdli, 'I', "Ctrl<Key>i", cmdli_accel_label,
        XmVaPUSHBUTTON, sound, 'S', NULL, NULL,
        XmVaPUSHBUTTON, motivational_vector, 'M', NULL, NULL,
#ifdef TELOP
        XmVaPUSHBUTTON, telop, 'T', NULL, NULL,
        XmVaPUSHBUTTON, personality, 'P', NULL, NULL,
#endif
        NULL);


    XmStringFree(accel_label);
    XmStringFree(command);
    XmStringFree(sound);
    XmStringFree(motivational_vector);

#ifdef TELOP
    XmStringFree(telop);
    XmStringFree(personality);
#endif
}

//-----------------------------------------------------------------------
void LoadEnvChangeFile(string strFileName)
{
    if (strFileName != "")
    {
        extern FILE* env_change_in;
        env_change_in = fopen(strFileName.c_str(), "r");

        if (env_change_in != NULL)
        {
            env_change_init();
            env_change_parse();
            fclose(env_change_in);
            env_change_in = NULL;
        }
        else
        {
            fprintf(stderr, "Error(mlab): In LoadEnvChangeFile(). Couldn't open \"%s\" for read\n", strFileName.c_str());
        }
    }
}

//-----------------------------------------------------------------------
void PromptAndLoadEnvChangeFile(void)
{
    ClFileOpenDlg dlg(main_window);

    if (dlg.Create())
    {
        // save the changes only if the user hit the "OK" button
        if (dlg.DoModal() == ClFileOpenDlg::EnRET_OK)
        {
            LoadEnvChangeFile(dlg.GetFileName());
        }
    }
}

//-----------------------------------------------------------------------
// called when an item on the configure menu is selected
//-----------------------------------------------------------------------
static void configure_cb(Widget w, int item_num)
{
    switch (item_num)
    {
    case 0:
        gt_popup_obstacle_panel();
        break;

    case 1:
        gt_popup_scale_panel();
        break;

    case 2:
        gt_popup_time_scale_panel();
        break;

    case 3:
      gt_popup_virtual_clock_panel();
        break;

    case 4:
        PromptAndLoadEnvChangeFile();
        break;

    case 5:
        if (executing_step_list)
        {
            warn_user("Warning: Cannot Clear All while executing.\n"
                       "Press [Abort] first.");
        }
        else
        {
            clear_simulation();
        }
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------
static void create_configure_menu(Widget menu_bar)
{
    XmString obstacle           = XmStringCreateLocalized("Obstacles");
    XmString scale              = XmStringCreateLocalized("Scale");
    XmString time_scale         = XmStringCreateLocalized("Time");
    XmString virtual_clock      = XmStringCreateLocalized("Virtual Clock");
    XmString change_environment = XmStringCreateLocalized("Change Environment");
    XmString clear              = XmStringCreateLocalized("Clear all");

    gt_create_obstacle_panel(main_window);
    gt_create_scale_panel(main_window);
    gt_create_time_scale_panel(main_window);
    gt_create_virtual_clock_panel(main_window);

    XmVaCreateSimplePulldownMenu(
        menu_bar, "configure-menu", 1, (XtCallbackProc)configure_cb,
        XmVaPUSHBUTTON, obstacle, 'O', NULL, NULL,
        XmVaPUSHBUTTON, scale, 'S', NULL, NULL,
        XmVaPUSHBUTTON, time_scale, 'T', NULL, NULL,
        XmVaPUSHBUTTON, virtual_clock, 'V', NULL, NULL,
        XmVaPUSHBUTTON, change_environment, 'E', NULL, NULL,
        XmVaPUSHBUTTON, clear, 'C', NULL, NULL,
        NULL);

    XmStringFree(obstacle);
    XmStringFree(scale);
    XmStringFree(time_scale);
    XmStringFree(virtual_clock);
    XmStringFree(change_environment);
    XmStringFree(clear);
}

//-----------------------------------------------------------------------
// called when an item on the file menu is selected
//-----------------------------------------------------------------------
static void file_cb(Widget w, int item_num)
{
    switch (item_num)
    {
    case 0:
        XtManageChild(file_open_dialog);
        break;

    case 1:
        create_overlay_open_dialog(main_window, NULL);
        XtManageChild(overlay_open_dialog);
        break;

    case 2:
        Pick_new_map();
        break;

    case 3:
        quit_mlab();

    default:
        break;
    }
}

//-----------------------------------------------------------------------
static void create_file_menu(Widget menu_bar)
{
    XmString open        = XmStringCreateLocalized("Open Mission");
    XmString overlay     = XmStringCreateLocalized("Load Overlay");
    XmString quit        = XmStringCreateLocalized("Quit");
    XmString restart     = XmStringCreateLocalized("Restart");
    XmString accel_label = XmStringCreateLocalized("^O");

    XmVaCreateSimplePulldownMenu(
        menu_bar, "file-menu", 0, (XtCallbackProc)file_cb,
        XmVaPUSHBUTTON, open, 'O', "Ctrl<Key>o", accel_label,
        XmVaPUSHBUTTON, overlay, 'L', NULL, NULL,
        XmVaPUSHBUTTON, restart, 'R', NULL, NULL,
        XmVaPUSHBUTTON, quit, 'Q', NULL, NULL,
        NULL);

    XmStringFree(open);
    XmStringFree(overlay);
    XmStringFree(quit);
    XmStringFree(accel_label);
}

//-----------------------------------------------------------------------
// called when an item on the compass menu is selected
//-----------------------------------------------------------------------
static void compass_cb(Widget w, int item_num)
{
    switch (item_num)
    {
    case 0:
        gt_popup_compass_interface();
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------
static void create_compass_menu(Widget menu_bar)
{
    XmString compass = XmStringCreateLocalized("Display Compass Heading");

    gt_create_compass_interface(top_level, app);
    XmVaCreateSimplePulldownMenu(menu_bar, "compass-menu", 4, (XtCallbackProc) compass_cb,
                                  XmVaPUSHBUTTON, compass, 'C', NULL, NULL,
                                  NULL);
    XmStringFree(compass);
}



//-----------------------------------------------------------------------
// called when an item on the help menu is selected
//-----------------------------------------------------------------------
static void help_cb(Widget w, int item_num)
{
    switch (item_num)
    {
    case 0:
        XtManageChild(help_dialog);
        break;

    case 1:
        XtManageChild(copyright_dialog);
        break;

    default:
        break;
    }
}

//-----------------------------------------------------------------------
static void create_help_menu(Widget menu_bar)
{
    XmString about     = XmStringCreateLocalized("About");
    XmString copyright = XmStringCreateLocalized("Copyright");

    // changed help-menu to 5 to allow compass menupull-down
    XmVaCreateSimplePulldownMenu(
        menu_bar, "help-menu", 5, (XtCallbackProc) help_cb,
        XmVaPUSHBUTTON, about, 'A', NULL, NULL,
        XmVaPUSHBUTTON, copyright, 'C', NULL, NULL,
        NULL);

    // Let the menu bar know the last button is the help button
    // THE "HELP" MENU MUST BE THE LAST ONE INSTALLED FOR THIS TO WORK!
    WidgetList buttons;
    Cardinal num_buttons;
    XtVaGetValues(menu_bar,
                   XmNchildren, &buttons,
                   XmNnumChildren, &num_buttons,
                   NULL);
    XtVaSetValues(menu_bar,
                   XmNmenuHelpWidget, (XtArgVal) buttons[num_buttons - 1],
                   NULL);

    XmStringFree(about);
    XmStringFree(copyright);
}

//-----------------------------------------------------------------------
static void create_options_menu(Widget menu_bar)
{

    Widget menu, log_menu_button, log_menu, debug_menu_button, debug_menu,
        lm_menu_button, lm_menu, lm_param_view;
    XmString accel, name;
    Widget laser_menu_button,laser_menu;

    // create the menu itself
    menu = XmVaCreateSimplePulldownMenu(menu_bar, "options-menu", 3, NULL,
                                         NULL);

    // add the menu items
    accel = XmStringCreateLocalized("^T");
    show_trails_button = XtVaCreateManagedWidget(
        "show-trails", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Show robot Trails"),
        XmNset,                   (Boolean) gt_show_trails,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'T',
        XmNaccelerator,           "Ctrl<Key>t",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(show_trails_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_show_trails, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^M");
    use_military_style_button = XtVaCreateManagedWidget(
        "use-military-style", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Use military style"),
        XmNset,                   (Boolean) gt_draw_robot_military_style,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'T',
        XmNaccelerator,           "Ctrl<Key>m",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(use_military_style_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_use_military_style, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^C");
    circle_robots_button = XtVaCreateManagedWidget(
        "circle-robots", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Circle robot position"),
        XmNset,                   (Boolean) gt_circle_robots,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'C',
        XmNaccelerator,           "Ctrl<Key>c",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(circle_robots_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_circle_robots, NULL);
    XmStringFree(accel);

  accel = XmStringCreateLocalized("^3");
    show_3d_vis_button = XtVaCreateManagedWidget(
	"show-3d-vis", xmToggleButtonWidgetClass, menu,
	RES_CONVERT(XmNlabelString, "Show 3d Visualization"),
	XmNset,			(Boolean) gt_show_3d_vis,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              '3',
        XmNaccelerator,           "Ctrl<Key>3",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);

    XtAddCallback(show_3d_vis_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_3d_vis, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^H");
    show_halos_button = XtVaCreateManagedWidget(
        "highlight-repelling-obstacles", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Highlight repelling obstacles"),
        XmNset,                   (Boolean) highlight_repelling_obstacles,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'H',
        XmNaccelerator,           "Ctrl<Key>h",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(show_halos_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_show_halos, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^V");
    show_vectors_button = XtVaCreateManagedWidget(
        "show-movement-vectors", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Show movement vectors"),
        XmNset,                   (Boolean) show_movement_vectors,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'v',
        XmNaccelerator,           "Ctrl<Key>v",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(show_vectors_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_show_vectors, NULL);
    XmStringFree(accel);

    show_movement_field = XtVaCreateManagedWidget(
        "show-movement-field", xmPushButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Show movement field"),
        NULL);
    ShowMovementFieldMenuItemEnabled(false);
    XtAddCallback(show_movement_field, XmNactivateCallback,
                   show_movement_field_callback, NULL);

    show_obstacles_button = XtVaCreateManagedWidget(
        "show-obstacles", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Show obstacles"),
        XmNset,                   (Boolean) show_obstacles,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(show_obstacles_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_show_obstacles, NULL);

    erase_obstacles_button = XtVaCreateManagedWidget(
        "erase-obstacles", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Erase obstacles"),
        XmNset,                   (Boolean) erase_obstacles,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(erase_obstacles_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_erase_obstacles, NULL);

    //--------------------------------------------------------------
    // Create the "Learning Momentum" sub-menu
    XtVaCreateManagedWidget(
        "options-sep3", xmSeparatorWidgetClass, menu, NULL);

    lm_menu_button = XmCreateCascadeButton(menu, "lm-menu-button",
                                            NULL, 0);

    lm_menu = XmCreatePulldownMenu(menu, "lm-menu", NULL, 0);

    // enable Learning Momentum menu item
    lm_enabled_button = XtVaCreateManagedWidget(
        "learning-momentum-enabled", xmToggleButtonWidgetClass, lm_menu,
        RES_CONVERT(XmNlabelString, "Enable"),
        XmNset,                   (Boolean) learning_momentum_enabled,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(lm_enabled_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_lm_enabled, NULL);

    // save Learning Momentum weights menu item
    save_lm_weights_button = XtVaCreateManagedWidget(
        "save-learning-momentum-weights", xmToggleButtonWidgetClass, lm_menu,
        RES_CONVERT(XmNlabelString, "Save Weights"),
        XmNset,                   (Boolean) save_learning_momentum_weights,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(save_lm_weights_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_save_learning_momentum_weights, NULL);

    // learning momentum parameter dialog menu item
    lm_param_view = XtVaCreateManagedWidget(
        "learning-momentum-param-view", xmPushButtonWidgetClass, lm_menu,
        RES_CONVERT(XmNlabelString, "View Parameters"),
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(lm_param_view, XmNactivateCallback,
                   (XtCallbackProc) toggle_lm_param_view, NULL);

    // learning momentum sub-menu item
    name = XmStringCreateLocalized("Learning Momentum");
    XtVaSetValues(lm_menu_button,
                   XmNlabelString, name,
                   XmNsubMenuId, lm_menu,
                   NULL);
    XtManageChild(lm_menu_button);
    XmStringFree(name);

    //--------------------------------------------------------------
    // Creating the button for "Log Robot Data" feature.
    XtVaCreateManagedWidget("options-sep2", xmSeparatorWidgetClass, menu, NULL);

    log_menu_button = XmCreateCascadeButton(menu, "log-menu-button",
                                             NULL, 0);

    log_menu = XmCreatePulldownMenu(menu, "log-menu", NULL, 0);

    log_robot_data_button = XtVaCreateManagedWidget(
        "log-robot-data", xmToggleButtonWidgetClass, log_menu,
        RES_CONVERT(XmNlabelString, "Log Robot Data"),
        XmNset,                   (Boolean) log_robot_data,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);

    XtAddCallback(log_robot_data_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_log_robot_data, NULL);

    // save sonar readings menu item
    save_sensor_readings_button = XtVaCreateManagedWidget(
        "save-sensor-readings", xmToggleButtonWidgetClass, log_menu,
        RES_CONVERT(XmNlabelString, "Save Sensor Readings"),
        XmNset,           (Boolean) save_sensor_readings,
        XmNindicatorType, XmN_OF_MANY,
        XmNindicatorSize, 12,
        NULL);
    XtAddCallback(save_sensor_readings_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_save_sensor_readings, NULL);

    name = XmStringCreateLocalized("Data Logging");
    XtVaSetValues(log_menu_button,
                   XmNlabelString, name,
                   XmNsubMenuId, log_menu,
                   NULL);

    XtManageChild(log_menu_button);
    XmStringFree(name);

    //--------------------------------------------------------------
    // Debug stuff
    XtVaCreateManagedWidget("options-sep", xmSeparatorWidgetClass, menu, NULL);

    debug_menu_button = XmCreateCascadeButton(menu, "debug-menu-button", NULL, 0);

    debug_menu = XmCreatePulldownMenu(menu, "debug-menu", NULL, 0);

    accel = XmStringCreateLocalized("^D");
    debug_button = XtVaCreateManagedWidget(
        "debug-simulator", xmToggleButtonWidgetClass, debug_menu,
        RES_CONVERT(XmNlabelString, "Debug simulator"),
        XmNset,                   (Boolean) debug,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'D',
        XmNaccelerator,           "Ctrl<Key>d",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(debug_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_debug, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^R");
    debug_robots_button = XtVaCreateManagedWidget(
        "debug-robots", xmToggleButtonWidgetClass, debug_menu,
        RES_CONVERT(XmNlabelString, "Debug robots"),
        XmNset,                   (Boolean) robot_debug_messages,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              'r',
        XmNaccelerator,           "Ctrl<Key>r",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(debug_robots_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_debug_robots, NULL);
    XmStringFree(accel);

    accel = XmStringCreateLocalized("^S");
    debug_scheduler_button = XtVaCreateManagedWidget(
        "debug-scheduler", xmToggleButtonWidgetClass, debug_menu,
        RES_CONVERT(XmNlabelString, "Debug robot scheduler"),
        XmNset,                   (Boolean) debug_scheduler,
        XmNindicatorType,         XmN_OF_MANY,
        XmNmnemonic,              's',
        XmNaccelerator,           "Ctrl<Key>s",
        XmNacceleratorText,       accel,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(debug_scheduler_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_scheduler_debug, NULL);
    XmStringFree(accel);

    //--------------------------------------------------------------
    // Creating the button for "Report Current State" feature.
    report_current_state_button = XtVaCreateManagedWidget(
        "report-current-state", xmToggleButtonWidgetClass, debug_menu,
        RES_CONVERT(XmNlabelString, "Report current state"),
        XmNset,                   (Boolean) report_state_messages,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(report_current_state_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_report_current_state, NULL);

    name = XmStringCreateLocalized("Debug");
    XtVaSetValues(debug_menu_button,
                   XmNlabelString, name,
                   XmNsubMenuId, debug_menu,
                   NULL);
    XtManageChild(debug_menu_button);
    XmStringFree(name);
    // Laser

    laser_menu_button = XmCreateCascadeButton(menu, "laser-menu-button", NULL, 0);
    laser_menu = XmCreatePulldownMenu(menu, "laser-menu", NULL, 0);

    laser_normal_button = XtVaCreateManagedWidget(
        "laser-nomal", xmToggleButtonWidgetClass, laser_menu,
        RES_CONVERT(XmNlabelString, "Laser Display Normal"),
        XmNset,                   (Boolean) laser_normal_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(laser_normal_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_laser_normal, NULL);

    laser_linear_button = XtVaCreateManagedWidget(
        "laser-linear", xmToggleButtonWidgetClass, laser_menu,
        RES_CONVERT(XmNlabelString, "Laser Display Linear"),
        XmNset,                   (Boolean) laser_linear_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(laser_linear_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_laser_linear, NULL);

    laser_connected_button = XtVaCreateManagedWidget(
        "laser-connected", xmToggleButtonWidgetClass, laser_menu,
        RES_CONVERT(XmNlabelString, "Laser Show connected"),
        XmNset,                   (Boolean) laser_connected_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(laser_connected_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_laser_connected, NULL);

    sonar_arc_button = XtVaCreateManagedWidget(
        "sonar_arc", xmToggleButtonWidgetClass, laser_menu,
        RES_CONVERT(XmNlabelString, "Show sonar as arc"),
        XmNset,                   (Boolean) sonar_arc_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(sonar_arc_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_sonar_arc, NULL);

    sonar_point_button = XtVaCreateManagedWidget(
        "sonar_point", xmToggleButtonWidgetClass, laser_menu,
        RES_CONVERT(XmNlabelString, "Show sonar as point"),
        XmNset,                   (Boolean) sonar_point_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(sonar_point_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_sonar_point, NULL);

    LOS_button = XtVaCreateManagedWidget(
        "Show Los", xmToggleButtonWidgetClass, menu,
        RES_CONVERT(XmNlabelString, "Show Line Of Sight"),
        XmNset,                   (Boolean) LOS_mode,
        XmNindicatorType,         XmN_OF_MANY,
        XmNindicatorSize,         12,
        NULL);
    XtAddCallback(LOS_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_LOS, NULL);

    name = XmStringCreateLocalized("Laser and sonar display");
    XtVaSetValues(laser_menu_button,
                  XmNlabelString, name,
                  XmNsubMenuId, laser_menu,
                  NULL);

    XtManageChild(laser_menu_button);
    XmStringFree(name);

}

//-----------------------------------------------------------------------
void clear_simulation(void)
{
    if (use_ipt)
    {
        gt_abort_command_execution();
    }
    delete_map(gMapInfo);
    clear_map();
    clear_obstacles();
    if (use_ipt)
    {
        clear_robots();
    }
    gt_clear_db();
    gt_clear_step_list();
    clear_objects();

    set_mission_area_size(DEFAULT_MISSION_AREA_WIDTH_METERS, DEFAULT_MISSION_AREA_HEIGHT_METERS);
}

//-----------------------------------------------------------------------
// returns the global coordinates and quits mlab
//-----------------------------------------------------------------------
void return_globalcoordinates(float x, float y)
{
    FILE *coordinateFile = NULL;
    string ovlFileName;
    char simpleOvlFilename[1024];

    if ((coordinateFile = fopen(coordinates_filename, "w+")) == NULL)
    {
        warn_userf("Can't write to coordinates file `%s'!\n", coordinates_filename);
    }
    else
    {
        fprintf(coordinateFile, "%.2f, %.2f\n", x, y);

        if (ovl_filename != NULL)
        {
            ovlFileName = ovl_filename;

            if (filename_has_directory(ovl_filename))
            {
                remove_directory(ovl_filename, simpleOvlFilename);
                ovlFileName = simpleOvlFilename;
            }

            fprintf(coordinateFile, "overlay %s\n", ovlFileName.c_str());
        }

        gEventLogging->log("returning global coordinate %.2f, %.2f", x, y);
        fclose(coordinateFile);
    }

    quit_mlab();
}

//-----------------------------------------------------------------------
// returns the geographic coordinates and quits mlab
//-----------------------------------------------------------------------
void return_geocoordinates(float x, float y)
{
    FILE *coordinateFile = NULL;
    string ovlFileName;
    double lat = 0, lon = 0;
    char simpleOvlFilename[1024];

    global2geographic(x, y, &lat, &lon);

    if ((coordinateFile = fopen(coordinates_filename, "w+")) == NULL)
    {
        warn_userf("Can't write to coordinates file `%s'!\n", coordinates_filename);
    }
    else
    {
        global2geographic(x, y, &lat, &lon);
        fprintf(coordinateFile, "%.6f, %.6f\n", lat, lon);

        if (ovl_filename != NULL)
        {
            ovlFileName = ovl_filename;

            if (filename_has_directory(ovl_filename))
            {
                remove_directory(ovl_filename, simpleOvlFilename);
                ovlFileName = simpleOvlFilename;
            }

            fprintf(coordinateFile, "overlay %s\n", ovlFileName.c_str());
        }

        gEventLogging->log("returning geographic coordinate %.6f, %.6f", lat, lon);

        fclose(coordinateFile);
    }

    quit_mlab();
}

//-----------------------------------------------------------------------
// called when button is clicked in drawing area or key pressed
//-----------------------------------------------------------------------
void cbKBInputDrawingArea(
    Widget w,
    XtPointer data,
    XmDrawingAreaCallbackStruct* cbs)
{
    XEvent* event = cbs->event;
    KeySym ks;
    XComposeStatus  cs;
    Mouse_Pos diffpos;
    point2d_t btn;
    double ratio, wPix, hPix, width, hDisp, dist;
    float angle, sealevel = 0;
    char buf[16], msg[80];
    int bstate = -1;
    int x, y, len;

    // we only want to do this for the 3d visualization stuff
    if (gt_show_3d_vis)
    {
        // find out the kind of button press
        if (event->xany.type == ButtonPress)
        {
            bstate = BUTTON_PRESS;
        }

        if (event->xany.type == ButtonRelease)
        {
            bstate = BUTTON_RELEASE;
            drawGLScene();
        }

        // find out which button
        if (bstate != -1)
        {
            switch (event->xbutton.button) {
            case Button1:
                Mouse_3d->Set_Button(LEFT_BUTTON,bstate);
                Mouse_3d->Set_XY(event->xbutton.x, event->xbutton.y);
                break;

            case Button2:
                Mouse_3d->Set_Button(MIDDLE_BUTTON,bstate);
                Mouse_3d->Set_XY(event->xbutton.x, event->xbutton.y);
                break;

            case Button3:
                Mouse_3d->Set_Button(RIGHT_BUTTON, bstate);
                Mouse_3d->Set_XY(event->xbutton.x, event->xbutton.y);
                break;

            default:
                break;
            }
        }

        // this is to handle motion events.. ie zooming, rotating etc
        if (event->type == MotionNotify)
        {
            x = event->xbutton.x;
            y = event->xbutton.y;
            diffpos = Mouse_3d->GetSet_DiffXY(x, y);

            if (Mouse_3d->Get_Button(RIGHT_BUTTON))
            {
                Eye[2] += ((float)diffpos.Y/2.0);
                Eye[2] = (Eye[2] >= 0)? Eye[2] : 0;
            }
            else if (Mouse_3d->Get_Button(LEFT_BUTTON))
            {
                Eye[0] += ((float)diffpos.X/2.0);
                Eye[1] -= ((float)diffpos.Y/2.0);
            }
            else if (Mouse_3d->Get_Button(MIDDLE_BUTTON))
            {
                // Pitch
                angle = (float)(diffpos.Y/10);
                angle = (angle <= 30.0)? angle : 30.0;
                angle = (angle >= -30.0)? angle : -30.0;

                if ((Rotate[0] + angle) > 90.0)
                {
                    angle = 90.0 - Rotate[0];
                    Rotate[0] = 90.0;
                }
                else if((Rotate[0] + angle) < 0.0)
                {
                    angle = 0 - Rotate[0];
                    Rotate[0] = 0.0;
                }
                else
                {
                    Rotate[0] += angle;
                }

                if (Terrain != NULL)
                {
                    sealevel = Terrain->Get_SeaLevel();
                }

                Eye[2] += (Eye[1]*sin(DEGREES_TO_RADIANS(angle)));
                Eye[2] -= (sealevel*(1.0 - cos(DEGREES_TO_RADIANS(angle))));

                Eye[1] -= Eye[1]*(1.0 - cos(DEGREES_TO_RADIANS(angle)));
                Eye[1] -= (sealevel*sin(DEGREES_TO_RADIANS(angle)));

                // Yaw
                angle = -(float)(diffpos.X/10);
                angle = (angle <= 30.0)? angle : 30.0;
                angle = (angle >= -30.0)? angle : -30.0;
                Rotate[1] += angle;

                width =  mission_area_width_meters;
                wPix = (double)mlabDisplayWidth;
                hPix = (double)mlabDisplayHeight;
                ratio = wPix/hPix;
                hDisp = width/ratio;

                dist = sqrt(pow(Eye[0],2) + pow((-Eye[1]+(width-hDisp)),2));

                Eye[0] -= dist*
                    (cos(DEGREES_TO_RADIANS(Rotate[1]-angle)) -
                     cos(DEGREES_TO_RADIANS(Rotate[1])));
                Eye[1] -= dist*
                    (sin(DEGREES_TO_RADIANS(Rotate[1]-angle)) -
                     sin(DEGREES_TO_RADIANS(Rotate[1])));
            }

            if (Eye[2] >= vis3d_zFar)
            {
                vis3d_zFar = Eye[2] * 2.0;
                gluPerspective(vis3d_fovy, vis3d_aspect, vis3d_zNear, vis3d_zFar);
            }
        }
    }
    else
    {
        if (cbs->reason == XmCR_INPUT)
        {
            if (event->xany.type == KeyPress)
            {
                len = XLookupString(&event->xkey, buf, 16, &ks, &cs);
                if (len == 1)
                {
                    buf[1] = '\0';
                    gt_update(BROADCAST_ROBOTID, KEYPRESS, buf);
                }
            }
            else if (event->xany.type == ButtonPress)
            {
                if (Pick_Point_From_Overlay)
                {
                    btn.x = (event->xbutton.x * meters_per_pixel) + origin_x;
                    btn.y = (drawing_area_height_pixels * meters_per_pixel)  -
                        (event->xbutton.y * meters_per_pixel) + origin_y;

                    switch (Pick_Point_From_Overlay) {

                    case PICK_POINT_SINGLE:
                        if (event->xbutton.button == 1)
                        {
                            return_globalcoordinates(btn.x, btn.y);
                        }
                        break;

                    case PICK_POINT_SINGLE_GEOCOORD:
                        if (event->xbutton.button == 1)
                        {
                            return_geocoordinates(btn.x, btn.y);
                        }
                        break;

                    case PICK_POINT_MULTI:
                        switch(event->xbutton.button) {

                        case 1:
                            add_waypoint(btn.x, btn.y);
                            break;
                        case 2:
                            delete_waypoint(btn.x, btn.y);
                            break;
                        case 3:
                            return_waypoints();
                            break;
                        }
                        break;

                    case MLAB_MISSION_DESIGN:
                        gMMD->mouseClick(btn.x, btn.y, event, meters_per_pixel);
                        break;

                    default:
                        // Do nothing.
                        break;
                    }
                }
                sprintf(
                    msg,"%.2f %.2f",
                    (double) event->xbutton.x * meters_per_pixel,
                    (double) event->xbutton.y * meters_per_pixel);
                gt_update(BROADCAST_ROBOTID, LEFT_MOUSE_CLICK, msg);
            }
            else if (event->xany.type == ButtonRelease)
            {
                if (Pick_Point_From_Overlay)
                {
                    btn.x = (event->xbutton.x * meters_per_pixel) + origin_x;
                    btn.y = (drawing_area_height_pixels * meters_per_pixel)  -
                        (event->xbutton.y * meters_per_pixel) + origin_y;

                    switch (Pick_Point_From_Overlay) {

                    case MLAB_MISSION_DESIGN:
                        gMMD->mouseRelease(btn.x, btn.y, event, meters_per_pixel);
                        break;

                    default:
                        // Do nothing.
                        break;
                    }
                }
            }
        }
    }
}

//-----------------------------------------------------------------------
static int popup_command_panel(void)
{
    gt_popup_command_panel();
    return true;
}

//-----------------------------------------------------------------------
// This function is a wrapper of gt_popup_telop_interface(), so that it
// can be called by XtAppAddWorkProc().
//-----------------------------------------------------------------------
int popup_telop_interface(void)
{
   int width, height;

#ifdef TELOP

   // Get resolution of screen
   height = XHeightOfScreen(XtScreen(top_level));
   width = XWidthOfScreen(XtScreen(top_level));

   // Resize the mlab window to fit new interface
   XtVaSetValues(
       top_level,
       XmNresizable, false,
       XmNx, 0,
       XmNy, 0,
       XmNwidth, (int) (888), // * ((double) width / 1280)),
       XmNheight, (int) (738), // * ((double) height / 1024)),
       NULL);

    gt_popup_telop_interface(NULL, 0, NULL);
#else
    warn_userf("Error(mlab): popup_telop_interface(). Unable to execute TELEOPERATION command!\nTeleoperation code is disabled.");
#endif

    return true;
}

//-----------------------------------------------------------------------
static int load_startup_file(void)
{
    // in any case, note the file
    set_command_panel_filename(startup_filename);
    set_file_open_dialog_filename(startup_filename);

    // Work procedure to load up the initial startup file
    if (gt_load_commands(startup_filename) != 0)
    {
        warn_userf("Commands NOT loaded successfully from\n file: %s",
                    startup_filename);
        return true;
    }

    // we loaded the file, so clean up a few things
    gt_rewind_step_list();

    // maybe run automatically, now that the file is loaded
    if (ask_for_map)
    {
        Pick_new_map();
    }
    else
    {
        // run the input file automatically
        if (auto_run)
        {
            gt_execute_step_list();
        }
    }

    // Next time, want to ask for map
    ask_for_map = true;

    return true;
}

//-----------------------------------------------------------------------
void set_busy_cursor(void)
{
    static Cursor watch = 0;

    if(!watch)
    {
        watch = XCreateFontCursor(XtDisplay(main_window), XC_watch);
    }

    XDefineCursor(XtDisplay(main_window), XtWindow(main_window), watch);
    XmUpdateDisplay(main_window);
}

//-----------------------------------------------------------------------
void reset_cursor(void)
{
    XUndefineCursor(XtDisplay(main_window), XtWindow(main_window));
    XmUpdateDisplay(main_window);
}

//-----------------------------------------------------------------------
// This handles mouse events sent from the background drawable.
//-----------------------------------------------------------------------
void da_mouse_cb(Widget w, XButtonEvent* event, String* args, int* num_args)
{
    const bool SHOW_MOUSE_POS = false;

    assert(*num_args == 1);
    int x = event->x;
    int y = event->y;

    if (args[0][0] == '1') // mouse button 1 down
    {
        if (SHOW_MOUSE_POS)
        {
            fprintf(stdout, "da_mouse_cb [%s]: X = %d, Y = %d\n", __FILE__, x, y);
        }
    }

}

//-----------------------------------------------------------------------
void load_new_overlay(char* defaultOverlayName)
{
    create_overlay_open_dialog(main_window, defaultOverlayName);
    XtManageChild(overlay_open_dialog);
}

//-----------------------------------------------------------------------
void resizeGLScene(unsigned int width, unsigned int height)
{
    if (height == 0)
    {
        height = 1; // no divide by 0 errors
    }

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (Eye[2] >= vis3d_zFar)
    {
        vis3d_zFar = Eye[2]*2.0;
    }

    gluPerspective(vis3d_fovy, vis3d_aspect, vis3d_zNear, vis3d_zFar);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gt_show_3d_vis)
    {
        drawGLScene();
    }
}

//-----------------------------------------------------------------------
void initGL(void)
{

    init3DVisEye();

	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		// Setup the ambient light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		// Setup the diffuse light
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpecular);	// Setup the specular light
	//glLightfv(GL_LIGHT1, GL_SHININESS, LightShininess);	// Setup the shininess
	glLightfv(GL_LIGHT1, GL_EMISSION, LightEmission);	// Setup the emission
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);	// Position The Light
	glEnable(GL_LIGHT1);								// Enable Light One
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glMatrixMode (GL_PROJECTION);    /* prepare for and then */
    glLoadIdentity ();               /* define the projection */

    if (Eye[2] >= vis3d_zFar)
    {
        vis3d_zFar = Eye[2]*2.0;
    }

	gluPerspective(vis3d_fovy, vis3d_aspect, vis3d_zNear, vis3d_zFar);
	glMatrixMode(GL_MODELVIEW);

}

//-----------------------------------------------------------------------
void init3DVisEye(void)
{
    float sealevel = 0;
    double ratio, wPix, hPix, w, h, hDisp;

    if (Terrain != NULL)
    {
        sealevel = Terrain->Get_SeaLevel();
    }

    w =  mission_area_width_meters;
    h = mission_area_height_meters;

    wPix = (double)mlabDisplayWidth;
    hPix = (double)mlabDisplayHeight;
    ratio = wPix/hPix;
    hDisp = w/ratio;

    Eye[0] = w/2.0;
    Eye[1] = -(h - (hDisp/2.0))+((w - hDisp)/2.0);
    Eye[2] = w + sealevel;

    Rotate[0] = VIS3D_INIT_ROT_X;
    Rotate[1] = VIS3D_INIT_ROT_Y;
    Rotate[2] = VIS3D_INIT_ROT_Z;
}

//-----------------------------------------------------------------------
void drawGLScene(void)
{
#ifndef NO_OPENGL
    bool drawn = false;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);     // Clear Screen And Depth Buffer
    glLoadIdentity();                                          // Reset The Current Modelview Matrix
    glTranslatef(-Eye[0], -Eye[1], -Eye[2]);
    glRotatef(Rotate[0], 1.0f, 0.0f, 0.0f);
    glRotatef(Rotate[1], 0.0f, 1.0f, 0.0f);
    glRotatef(Rotate[2], 0.0f, 0.0f, 1.0f);
    glPushMatrix();
    glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);    // Position The Light

    if (Terrain != NULL)
    {
        Terrain->Draw();
        drawn = true;
    }

    glPopMatrix();

    if (Rob_Handler != NULL)
    {
        Rob_Handler->Draw();
        drawn = true;
    }

    if (Obs_Handler != NULL)
    {
        Obs_Handler->Draw();
        drawn = true;
    }

    if (Terrain != NULL)
    {
        Terrain->Draw_Water();
        drawn = true;
    }

    glXSwapBuffers(display_3dvis, XtWindow(drawing_area));

    if (!drawn)
    {
        DrawText(
            mission_area_width_meters/2.0,
            mission_area_height_meters/2.0,
            gGCs.green,
            "Loading...");
    }
#endif
}

//-----------------------------------------------------------------------
// Print the mlab command line options.
//-----------------------------------------------------------------------
void print_usage(char* mlab_name)
{
    int i;

    printf("\n");
    printf("Usage: %s [X options] [Other options] [CMDL file] [-n]\n", mlab_name);
    printf("\n");
    printf("X options:\n");
    printf("  Normal X options (such as -geometry, etc.).\n");
    printf("\n");
    printf("Other options:\n");
    printf("  -3           Enable the 3D mode.\n");
    printf("  -A           Enable the assistant dialog.\n");
    printf("  -b           Enable the playback mode. Pick your own robot log data.\n");
    printf("  -B           Repair mission with the playback mode. Run with -M.\n");
    printf("  -c filename  Use a specified configuration file instead of .cfgeditrc.\n");
    printf("  -C           Show the command panel on start up.\n");
    printf("  -d           Enable debugging.\n");
    printf("  -e filename  Enable event logging. Use the specified file.\n");
    printf("  -h           Show usage.\n");
    printf("  -H height    Set the height of the main window.\n");
    printf("  -i filename  Display the CMDLi panel. Use the specified CMDLi file.\n");
    printf("  -I hostname  Use the specified address as the IPT host.\n");
    printf("  -l           Show color map.\n");
    printf("  -L           Log the robot data.\n");
    printf("  -m filename  Pick a coordinate of a single point on the map and write in the.\n");
    printf("               outfile.\n");
    printf("  -M string    Enable the Mission Design mode. The specified 'string' is used\n");
    printf("               to name output files.\n");
    printf("  -N cnpmode   Set the CNP mode. The input can be either a number or string:\n");

    for (i = 0; i < NUM_CNP_MODES; i++)
    {
        printf("                 %d or %s\n", i, CNP_MODE_STRING[i].c_str());
    }

    printf("  -p filename  Use the specified personality file.\n");
    printf("  -P           Pause before execution.\n");
    printf("  -r           Start the mission as soon as mlab is loaded.\n");
    printf("  -R           Pick the overlay of the mission manually.\n");
    printf("  -s number    Set the random number seed.\n");
    printf("  -S           Display the current state information during the execution.\n");
    //printf("  -t number    Time to expire mlab (in second).\n");
    printf("  -T           Show the telop interface on start up.\n");
    printf("  -w filename  Enable the waypoint mode and output the coordinates in the file.\n");
    printf("  -W width     Set the width of the main window.\n");
    printf("  -X           Show the fallback resources.\n");
    printf("\n");
    printf("  -n           Do not show the copyright dialog.\n");
    printf("\n");
}

//-----------------------------------------------------------------------
// Main function
//-----------------------------------------------------------------------
int main(int argc, char* argv[])
{
    Widget menu_bar, command_bar;
    string cnpModeString = EMPTY_STRING;
    string missionSpecWizardTypeString = EMPTY_STRING;
    string cmdli_filename = EMPTY_STRING;
    char *ipt_home = NULL;
    char mlab_title[100];
    char subs_font[120];
    char geo[50];
    char *defaultOverlayName = NULL;
    char *missionTimeString = NULL;
    const char* rc_filename = RC_FILENAME;
    float value;
    int i, c, h, s, w;
    int screen_num;
    int jboxID;
    int missionSpecWizardType = MISSION_SPEC_WIZARD_DISABLED;
    bool useJBoxMlab = false;
    bool enableMMDCBRClient = false;
    bool no_path_search = false;
    extern int optind;

    XInitThreads();

    gEventLogging = new EventLogging();

    // Define the fallback resources
    static String fallback_resources[] = {
        "MissionLab.geometry:",
        "MissionLab*Command Interface.geometry: +480+440",
        "MissionLab*sound_simulation_window.geometry: +70+550",
        "MissionLab*motivational_vector_window.geometry: +580+550",
#ifdef TELOP
        "MissionLab*personality_window.geometry: +580+550",
        "MissionLab*main_telop_window.geometry: +700+764",
        "MissionLab*main_status_window.geometry: +900+0",
#endif
        "MissionLab*Obstacle Creation.geometry: +5+580",
        "MissionLab*World Scale.geometry: +5+580",
        "MissionLab*Time Scale.geometry: +5+580",
        "MissionLab*Current Robot State.geometry: -550+100",
        //"MissionLab*Alert Message.geometry: +70+100",
        "MissionLab*Alert Message.geometry: -550-320",
        "MissionLab*selectColor: yellow",
        "MissionLab*.background: LightYellow3",
        "*glxarea*width: 400",
        "*glxarea*height: 300",
        NULL
    };

    // Define option table for XtVaAppInitialize().
    static XrmOptionDescRec optionTable [] = {
        {"-t", "t*", XrmoptionSkipArg, (caddr_t) NULL},
        {"-i", "i*", XrmoptionSkipArg, (caddr_t) NULL},
    };

    // initialize the array of workprocs
    for (i = 0; i < MAX_WORKPROCS; i++)
    {
        workproc[i] = NULL;
    }

    gt_randomize_seed(false);

    // Initialize the X display (and make sure that we're running under X)
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        fprintf(stderr, "\n    Error opening X display! \n");
        fprintf(stderr, "    (you may need to do 'setenv DISPLAY host:0.0')\n\n");
        exit(1);
    }

    // Try the list of candidate fonts
    for (i = 0; i < num_default_font_names; i++)
    {
        default_font_struct = XLoadQueryFont(display, default_font_name[i]);
        if (default_font_struct != NULL)
        {
            break;
        }
    }

    // If those fonts couldn't be found, let the user specify one
    if (default_font_struct == NULL)
    {
        printf("Error loading default font!\007\n");
        printf("MissionLab needs a fixed space 14pt bold font.\n");
        printf("(Try xfontsel to find a suitable one).\n");

        while (default_font_struct == NULL)
        {
            // Let the user enter a substitute font name
            printf("Please enter the name of an alternate font to load: ");
            fgets(subs_font, 100, stdin);
            default_font_struct = XLoadQueryFont(display, subs_font);

            if (default_font_struct == NULL)
            {
                printf("Unable to load font (%s)!\n", subs_font);
            }
        }
    }

    // Die if we didn't find a decent font
    if (default_font_struct == NULL)
    {
        printf("Aborting: Unable to find suitable fonts\n");
        exit(1);
    }
    else
    {
        default_font = default_font_struct->fid;
        default_fonts = XmFontListCreate(default_font_struct,
                                         XmSTRING_DEFAULT_CHARSET);
        if (default_fonts == NULL)
        {
            fprintf(stderr, "Error creating default font list!\n");
            exit(1);
        }
    }

    // Set up the colors
    create_colormap(display);
    fix_window_colors();

    // create the top-level shell widget and initialize the toolkit
    // (Note: Run this first to strip off X-related options)
    screen_num = DefaultScreen(display);

    mlabDisplayWidth = DisplayWidth(display, screen_num) - 30;
    mlabDisplayHeight = DisplayHeight(display, screen_num) - 50;

    if (!SCREEN_CAPTURE)
    {
        sprintf(
            geo,
            "MissionLab.geometry: %dx%d+10+0",
            mlabDisplayWidth,
            mlabDisplayHeight);
    }
    else
    {
        sprintf(geo, "MissionLab.geometry: 1200x800");
    }

    fallback_resources[0] = geo;

    sprintf(
        mlab_title,
        " MissionLab  v%s   (c) Georgia Institute of Technology",
        version_str);

    top_level = XtVaAppInitialize(
        &app,
        "MissionLab",
        optionTable,
        XtNumber(optionTable),
        &argc,
        (String *)argv,
        fallback_resources,
        XmNtitle, mlab_title,
        XmNdefaultFontList, default_fonts,
        XmNcolormap, mlab_colormap,
        NULL);

#ifndef NO_OPENGL
    // mot3d initialize the opengl stuff
    display_3dvis = XtDisplay(top_level);

    if (visualinfo_3dvis == NULL)
    {
        visualinfo_3dvis = glXChooseVisual(
            display_3dvis,
            DefaultScreen(display_3dvis),
            dblBuf_3dvis);

        if (visualinfo_3dvis == NULL)
        {
            visualinfo_3dvis = glXChooseVisual(
                display_3dvis,
                DefaultScreen(display_3dvis),
                snglBuf_3dvis);

            if (visualinfo_3dvis == NULL)
            {
                XtAppError(app, "No RGB visual with depth buffer.");
            }

            doubleBuffer = false;
        }
    }

    glcontext_3dvis = glXCreateContext(
        display_3dvis,
        visualinfo_3dvis,
        None,
        true);

    if (glcontext_3dvis == NULL)
    {
        XtAppError(app, "Could not create GL rendering context.");
    }
#endif

    // Let's make the drawing area according to the screen size, too.
    drawing_area_width_pixels  = DisplayWidth(display, screen_num) - 30;
    drawing_area_height_pixels = DisplayWidth(display, screen_num) - 30;

    // check for options

    while ((c = getopt(argc, argv, "3aAbBc:Cde:E:f:G:hH:i:I:J:lLm:M:nN:o:p:PrRs:St:TU:V:w:W:Xx:")) != EOF)
    {
        // NOTE: If you add any option with arguments (like T, H, and V in the
        // list above), include them with colons in the getopt() string
        // above and add them to the startup filename checking below by
        // adding another "or" clause of the form
        // (strchr(argv[i], 'H') != NULL)
        //
        // NOTE2: If you add any new option, make sure you explain the usage in
        // print_usage() above.
        //
        // NOTE3: If some option is not accepted, it is possible that option is
        // used by XtVaAppInitialize(). Add an entry in optionTable above to
        // prevent XtVaAppInitialize() using this option.


        switch (c) {

        case '3':
            gt_show_3d_vis = true;
            fprintf(stderr,"3D display enabled.\n");
            break;

        case 'a':
            gAutomaticExecution = true;
            break;

        case 'A':
            initPopupAssistantDialogOption = ASSISTANT_INIT_POPUP_ENABLE;
            break;

        case 'b':
            seen_initial_copyright = true;
            use_ipt = false;
            gPlayBackMode = MlabPlayBack::MPB_ENABLED;
            ask_for_map = true;
            report_state_messages = true;
            break;

        case 'B':
            seen_initial_copyright = true;
            use_ipt = false;
            gPlayBackMode = MlabPlayBack::MPB_ENABLED_COMM_CBRSERVER;
            break;

        case 'c':
            rc_filename = optarg;
            no_path_search = true;
            cerr << "Using configuration file " << rc_filename << '\n';
            break;

        case 'C':
            show_command_panel_on_startup = true;
            break;

        case 'd':
            debug = true;
            fprintf(stderr, "Console debugging enabled\n");
            break;

        case 'e':
            gEventLogging->setLogfileName(strdup(optarg));
            gEventLogging->resume(true);
            gEventLogging->start("mlab");
            break;

        case 'E':
            mexpRCFileName = optarg;
            break;

        case 'f':
            gStopRobotFeedback = !atoi(optarg);
            break;

        case 'G':
            use_ipt = false;
            Pick_Point_From_Overlay = PICK_POINT_SINGLE_GEOCOORD;
            coordinates_filename = strdup(optarg);
            break;

        case 'h':
            print_usage(argv[0]);
            exit(0);
            break;

        case 'H':
            if (sscanf(optarg, "%d", &h) == 0)
            {
                fprintf(stderr, "Error reading drawing area height in pixels (%s)\n", optarg);
            }
            else if (h <= 0)
            {
                fprintf(stderr, "Drawing area must be taller than 0!  (-H %s)\n", optarg);
            }
            else
            {
                drawing_area_height_pixels = h;
            }
            break;

        case 'i':
            seen_initial_copyright = true;
            show_cmdli_panel_on_startup = true;
            cmdli_filename = optarg;
            break;

        case 'I':
            ipt_home = strdup(optarg);
            break;

        case 'J':
            useJBoxMlab = true;
            jboxID = atoi(optarg);
            seen_initial_copyright = true;
            break;

        case 'l':
            show_colormap = true;
            break;

        case 'L':
            log_robot_data = true;
            fprintf(stderr, "Robot data logging enabled\n");
            break;

        case 'm':
            use_ipt = false;
            Pick_Point_From_Overlay = PICK_POINT_SINGLE;
            coordinates_filename = strdup(optarg);
            break;

        case 'M':
            seen_initial_copyright = true;
            gUseMlabMissionDesign = true;
            mlabMissionDesignIndexString = strdup(optarg);
            break;

        case 'n':
            seen_initial_copyright = true;
            break;

        case 'N':
            cnpModeString = optarg;

            // The argument can be either CNP_Mode_t number itself or CNP_MODE_STRING.
            if (isdigit(cnpModeString[0]))
            {
                cnpMode = atoi(cnpModeString.c_str());

                if ((cnpMode < CNP_MODE_DISABLED) ||
                    (cnpMode >= NUM_CNP_MODES))
                {
                    fprintf(
                        stderr,
                        "Warning: Command line argument for -N not recognized. [%d]\n",
                        cnpMode);
                    cnpMode = CNP_MODE_DISABLED;
                }
                else
                {
                    fprintf(
                        stderr,
                        "Enabling CNP: Mode(%d) [%s]\n",
                        cnpMode,
                        CNP_MODE_STRING[cnpMode].c_str());

                    seen_initial_copyright = true;
                }
            }
            else
            {
                for (i = 0; i < NUM_CNP_MODES; i++)
                {
                    if (cnpModeString == CNP_MODE_STRING[i])
                    {
                        if (i > CNP_MODE_DISABLED)
                        {
                            fprintf(
                                stderr,
                                "Enabling CNP:  Mode(%d) [%s]\n",
                                i,
                                CNP_MODE_STRING[i].c_str());

                            seen_initial_copyright = true;
                        }

                        cnpMode = i;
                        break;
                    }
                }

                if (i >= NUM_CNP_MODES)
                {
                    fprintf(
                        stderr,
                        "Warning: Command line argument for -N not recognized. [%s]\n",
                        cnpModeString.c_str());
                }
            }
            break;

        case 'o':
            defaultOverlayName = strdup(optarg);
            break;

        case 'p':
            personality_filename = strdup(optarg);
            break;

        case 'P':
            pause_before_execution = true;
            auto_run = false;
            break;

        case 'r':
            seen_initial_copyright = true;
            auto_run = true;
            break;

        case 'R':
            seen_initial_copyright = true;
            ask_for_map = true;
            auto_run = true;
            break;

        case 's':
            if (sscanf(optarg, "%d", &s) == 0)
            {
                fprintf(stderr, "Error reading seed! (%s)\n", optarg);
            }
            else
            {
                seed = s;
            }
            break;

        case 'S':
            report_state_messages = true;
            fprintf(stderr, "Report current state enabled\n");
            break;

        case 't':
            missionTimeString = strdup(optarg);
            break;

        case 'T':
            show_telop_interface_on_startup = true;
            break;

        case 'U':
            missionSpecWizardTypeString = optarg;

            // The argument can be either MissionSpecWizardTypes_t number itself
            // or MISSION_SPEC_WIZARD_NAMES[].
            if (isdigit(missionSpecWizardTypeString[0]))
            {
                missionSpecWizardType = atoi(missionSpecWizardTypeString.c_str());

                if ((missionSpecWizardType < MISSION_SPEC_WIZARD_DISABLED) ||
                    (missionSpecWizardType >= NUM_MISSION_SPEC_WIZARD_TYPES))
                {
                    fprintf(
                        stderr,
                        "Warning: Command line argument for -U not recognized. [%d]\n",
                        missionSpecWizardType);
                    missionSpecWizardType = MISSION_SPEC_WIZARD_DISABLED;
                }
                else
                {
                    /*
                      fprintf(
                      stderr,
                      "Enabling Mission-Specification Wizard: Mode(%d) [%s]\n",
                      missionSpecWizardType,
                      MISSION_SPEC_WIZARD_NAMES[missionSpecWizardType].c_str());
                    */
                    seen_initial_copyright = true;
                }
            }
            else
            {
                for (i = 0; i < NUM_MISSION_SPEC_WIZARD_TYPES; i++)
                {
                    if (missionSpecWizardTypeString == MISSION_SPEC_WIZARD_NAMES[i])
                    {
                        /*
                          fprintf(
                          stderr,
                          "Enabling Mission-Specification Wizard: Mode(%d) [%s]\n",
                          i,
                          MISSION_SPEC_WIZARD_NAMES[i].c_str());
                        */
                        seen_initial_copyright = true;

                        missionSpecWizardType = i;
                        break;
                    }
                }

                if ((i < MISSION_SPEC_WIZARD_DISABLED) ||
                    (i >= NUM_MISSION_SPEC_WIZARD_TYPES))
                {
                    fprintf(
                        stderr,
                        "Warning: Command line argument for -U not recognized. [%s]\n",
                        missionSpecWizardTypeString.c_str());
                }
            }
            break;

        case 'V':
            if (optarg != NULL)
            {
                g_strEnvChangeFilename = optarg;
            }
            break;

        case 'w':
            use_ipt = false;
            waypoint_design_mode = true;
            Pick_Point_From_Overlay = PICK_POINT_MULTI;
            waypoints_filename = strdup(optarg);
            break;

        case 'W':
            if (sscanf(optarg, "%d", &w) == 0)
            {
                fprintf(stderr, "Error reading drawing area width in pixels (%s)\n", optarg);            }
            else if (w <= 0)
            {
                fprintf(stderr, "Drawing area must be wider than 0!  (-W %s)\n", optarg);
            }
            else
            {
                drawing_area_width_pixels = w;
            }
            break;

        case 'x':
            value = atof(optarg);
            expirationTimeInSec = (int)value;
            break;

        case 'X':
            i = 0;
            printf("\nMissionLab fallback resources:\n\n");

            while (fallback_resources[i] != NULL)
            {
                printf("   %s\n", fallback_resources[i]);
                i++;
            }
            printf("\n");
            exit(0);
            break;

        default:
            fprintf(stderr, "\nUnrecognized input argument: %s\n", argv[optind - 1]);
            print_usage(argv[0]);
            exit(0);
            break;
        }
    }

    if(load_rc(rc_filename, &rc_table, no_path_search, false, false) == 2)
    {
        cerr << "Warning: Didn't find " << rc_filename << '\n';
    }

    // if there is a startup file specified, load it
    if (argc > optind)
    {
        startup_filename = argv[optind];

        // make sure the file exists
        if (!file_exists(startup_filename))
        {
            fprintf(stderr, "Error: File '%s' does not exist!\007\n", startup_filename);
            startup_filename = NULL;
        }
    }

    // init the globals
    gt_initialize_robot_info();
    if ((gMapInfo = create_map()) == NULL)
    {
        fprintf(stderr, "Error: Unable to create map info structure!\n");
    }

    // create the main window widget.
    main_window = XtVaCreateManagedWidget("MissionLab", xmMainWindowWidgetClass, top_level, NULL);

    // create the menu bar
    XmString file = XmStringCreateLocalized("File");
    XmString configure = XmStringCreateLocalized("Configure");
    XmString command = XmStringCreateLocalized("Command");
    XmString options = XmStringCreateLocalized("Options");
    XmString compass = XmStringCreateLocalized("Compass");
    XmString help = XmStringCreateLocalized("Help");

    menu_bar = XmVaCreateSimpleMenuBar(
        main_window, "Menubar",
        XmVaCASCADEBUTTON, file, 'F',
        XmVaCASCADEBUTTON, configure, 'n',
        XmVaCASCADEBUTTON, command, 'C',
        XmVaCASCADEBUTTON, options, 'O',
        XmVaCASCADEBUTTON, compass, 'P',
        XmVaCASCADEBUTTON, help, 'H',
        NULL);

    XmStringFree(file);
    XmStringFree(configure);
    XmStringFree(command);
    XmStringFree(compass);
    XmStringFree(options);
    XmStringFree(help);

    // Create the menus on the main menu bar
    create_file_menu(menu_bar);
    create_configure_menu(menu_bar);
    create_command_menu(menu_bar);
    create_options_menu(menu_bar);
    create_compass_menu(menu_bar);

    // create the "help" menu
    // NB: This must be the last menu item to work properly!
    create_help_menu(menu_bar);

    XtManageChild(menu_bar);

    // -------------------------------------------------------------
    // create a Form widget to be the work area.
    Widget work_area =  XtVaCreateManagedWidget(
        "", xmFormWidgetClass, main_window,
        XmNresizePolicy,  XmNONE,
        NULL);
    XtManageChild(work_area);

    // -------------------------------------------------------------
    // create the command bar
    command_bar =  XtVaCreateManagedWidget(
        "command-bar", xmFormWidgetClass, work_area,
        XmNtopAttachment,   XmATTACH_FORM,
        XmNleftAttachment,  XmATTACH_FORM,
        XmNrightAttachment, XmATTACH_FORM,
        NULL);
    create_scale_ruler(command_bar);

    XtManageChild(command_bar);

    // ----------------------------------------------------------------
    Widget scrolled_window = XtVaCreateManagedWidget(
        "", xmScrolledWindowWidgetClass, work_area,
        XmNtopAttachment,               XmATTACH_WIDGET,
        XmNtopWidget,                   command_bar,
        XmNleftAttachment,              XmATTACH_FORM,
        XmNrightAttachment,             XmATTACH_FORM,
        XmNbottomAttachment,            XmATTACH_FORM,
        XmNscrollBarDisplayPolicy,      XmAS_NEEDED,
        XmNscrollingPolicy,             XmAUTOMATIC,
        XmNvisualPolicy,                XmCONSTANT,
        NULL);

// 3D_VISUALIZATION
#ifndef NO_OPENGL
    drawing_area = XtVaCreateManagedWidget(
        "glxarea",
        glwDrawingAreaWidgetClass,
        scrolled_window,
        GLwNvisualInfo, visualinfo_3dvis,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNwidth,        drawing_area_width_pixels,
        XmNheight,       drawing_area_height_pixels,
        NULL);
#endif

#ifdef NO_OPENGL
    // create the drawing area
    drawing_area = XtVaCreateWidget(
        "drawing-area", xmDrawingAreaWidgetClass, scrolled_window,
        XmNresizePolicy, XmRESIZE_NONE,
        XmNwidth,        drawing_area_width_pixels,
        XmNheight,       drawing_area_height_pixels,
        NULL);
#endif

    // add a callback for mouse and keyboard input
    XtAddCallback(
        drawing_area, XmNinputCallback,
        (XtCallbackProc)cbKBInputDrawingArea,
        NULL);

    // add a callback for expose and resize
    XtAddCallback(
        drawing_area, XmNexposeCallback,
        (XtCallbackProc)redraw_cb, NULL);

    XtAddCallback(
        drawing_area, XmNresizeCallback,
        (XtCallbackProc)redraw_cb, NULL);

    XtManageChild(drawing_area);

    XtVaGetValues(
        scrolled_window,
        XmNverticalScrollBar, &drawing_area_scroll_bar,
        NULL);


    // Fix for scrolling
    XtVaGetValues(
        scrolled_window,
        XmNhorizontalScrollBar, &drawing_area_scroll_bar_h,
        NULL);


    XtAddCallback(drawing_area_scroll_bar, XmNvalueChangedCallback,
                  (XtCallbackProc)Handle_MapScrolling, NULL);
    XtAddCallback(drawing_area_scroll_bar_h, XmNvalueChangedCallback,
                  (XtCallbackProc)Handle_MapScrolling, NULL);

    initialize_drawing_area();

    // attach the menubar and the drawing_area area to the main window
    XmMainWindowSetAreas(main_window, menu_bar, NULL, NULL, NULL, work_area);

    // create the dialog boxes
    create_file_open_dialog(main_window);
    create_error_dialog(main_window);
    create_alert_dialog(main_window);
    create_status_dialog(main_window);
    create_save_ovl_as_dialog(main_window);
    create_help_dialog(main_window);
    create_copyright_dialog(main_window);

    // create the cmdli command interface
    if (show_cmdli_panel_on_startup)
    {
        the_cmdli_panel = new gt_cmdli_panel(main_window);

        if(cmdli_filename != EMPTY_STRING)
        {
            the_cmdli_panel->load_from_file(cmdli_filename);
        }
    }
    // Realize all widgets
    XtRealizeWidget(top_level);
#ifndef NO_OPENGL
    glXMakeCurrent(display_3dvis, XtWindow(drawing_area), glcontext_3dvis);
#endif

    gAssistantDialog = new AssistantDialog(
        main_window,
        rc_table,
        initPopupAssistantDialogOption);

    // create a pixmap the same size as the drawing area
    drawing_area_pixmap = XCreatePixmap(
        XtDisplay(drawing_area),
        RootWindowOfScreen(XtScreen(drawing_area)),
        drawing_area_width_pixels,
        drawing_area_height_pixels,
        DefaultDepthOfScreen(XtScreen(drawing_area)));

    // Initialize the mission area and related lengths
    //   (clears drawing area too)
    set_mission_area_size(
        DEFAULT_MISSION_AREA_WIDTH_METERS,
        DEFAULT_MISSION_AREA_HEIGHT_METERS);

    // add the main work proc
    // XtAppAddWorkProc(app, (XtWorkProc) main_workproc, NULL);
    XtAppAddTimeOut(
        app,
        WORKPROC_PERIOD_MSEC,
        (XtTimerCallbackProc)main_workproc,
        NULL);

    gBitmapIconDisplayer = new BitmapIconDisplayer(
        display,
        main_window,
        app,
        drawing_area,
        rc_table);

    if (gUseMlabMissionDesign)
    {
        enableMMDCBRClient = true;

        if (auto_run)
        {
            // Regular execution of the robot.
            use_ipt = true;
        }
        else
        {
            // Designing or playback the mission.
            use_ipt = false;

            Pick_Point_From_Overlay = MLAB_MISSION_DESIGN;
        }

        gMMD = new MlabMissionDesign(
            display,
            main_window,
            app,
            drawing_area,
            rc_table,
            mexpRCFileName,
            mlabMissionDesignIndexString,
            missionTimeString,
            enableMMDCBRClient,
            &defaultOverlayName,
            cnpMode);

        if (missionSpecWizardType != MISSION_SPEC_WIZARD_DISABLED)
        {
            gMMD->setMissionSpecWizardType(missionSpecWizardType);
        }
    }
    else if (cnpMode > CNP_MODE_DISABLED)
    {
        // Use MlabMissionDesign to load the robot constraints for
        // 3D display.
        gMMD = new MlabMissionDesign(
            display,
            main_window,
            app,
            drawing_area,
            rc_table,
            mexpRCFileName,
            NULL,
            missionTimeString,
            false,
            &defaultOverlayName,
            cnpMode);

        if (missionSpecWizardType != MISSION_SPEC_WIZARD_DISABLED)
        {
            gMMD->setMissionSpecWizardType(missionSpecWizardType);
        }
    }

    switch (cnpMode) {

    case CNP_MODE_RUNTIME:
    case CNP_MODE_PREMISSION_AND_RUNTIME:
        if (auto_run)
        {
            gMlabCNP = new MlabCNP(app);
        }
        break;
    }

    if (Pick_Point_From_Overlay)
    {
        if (gPlayBackMode != MlabPlayBack::MPB_ENABLED_COMM_CBRSERVER)
        {
            load_new_overlay(defaultOverlayName);
        }
    }

    // attach the simulation as a work procedure
    if (use_ipt)
    {
        gt_init_simulation(ipt_home);
        add_workproc((XtWorkProc) gt_simulation_loop, NULL);
    }

    // if we have a startup file, add a work procedure to load it
    if (startup_filename)
    {
        XtAppAddWorkProc(app, (XtWorkProc) load_startup_file, NULL);
    }

    // if we should display the command panel, add a workproc to do it
    if (show_command_panel_on_startup)
    {
        XtAppAddWorkProc(app, (XtWorkProc) popup_command_panel, NULL);
    }

    // if we should display the cmdli panel, add a workproc to do it
    if (show_cmdli_panel_on_startup)
    {
        XtAppAddWorkProc( app, (XtWorkProc) gt_popup_cmdli_panel, NULL );
    }

    // if we should display the telop interface, add a workproc to do it
    if (show_telop_interface_on_startup)
    {
        XtAppAddWorkProc(app, (XtWorkProc) popup_telop_interface, NULL);
    }

    if (!seen_initial_copyright)
    {
        XtAppAddWorkProc(app, (XtWorkProc)show_initial_copyright, NULL);
    }

    if (pause_before_execution)
    {
        XtAppAddWorkProc(app, (XtWorkProc)show_start_proceed, NULL);
    }

    if (expirationTimeInSec > 0)
    {
        XtAppAddTimeOut(
            app,
            expirationTimeInSec*1000,
            (XtTimerCallbackProc)expireMlab,
            NULL);
    }

    if (gPlayBackMode)
    {
        gPlayBack = new MlabPlayBack(
            app,
            display,
            main_window,
            drawing_area,
            meters_per_pixel,
            gPlayBackMode);

        XtSetSensitive(console_pause_button, false);
    }

    // JBox Console Visualization
    if (useJBoxMlab)
    {
        fprintf(stderr, "Using JBox.\n");
        gJBoxMlab = new JBoxMlab(jboxID);
        gJBoxDisplay = new JBoxDisplay(app, (startup_filename != NULL));
    }

    // Catch control-c and others to ensure that we kill the robots
    //signal(SIGINT,  sig_catch);
    //signal(SIGBUS,  sig_catch);
    //signal(SIGHUP,  sig_catch);
    //signal(SIGSEGV, sig_catch);
    //signal(SIGPIPE, sig_catch);
    //signal(SIGCHLD, child_reaper);

    if (report_state_messages)
    {
        report_userf("");
    }

    // 3D_VISUALIZATION STUFF
#ifndef NO_OPENGL
    // do some open gl and 3d init stuff
    initGL();
    resizeGLScene(drawing_area_width_pixels, drawing_area_height_pixels);
#endif

    Mouse_3d = new PMouse();

    if (gt_show_3d_vis)
    {
        initialize_3d_vis();
    }

    // start the main event-handling loop
    XtAppMainLoop(app);

    return 0;
}

/**********************************************************************
 * $Log: console.c,v $
 * Revision 1.3  2008/07/16 21:16:08  endo
 * Military style is now a toggle option.
 *
 * Revision 1.2  2008/07/15 22:38:53  endo
 * OpenCV, Gazebo and USARSim added.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.13  2007/09/18 22:36:59  endo
 * Mission time windows in ICARUS objectives can be now updated based on the environment message.
 *
 * Revision 1.12  2007/09/07 23:08:56  endo
 * The overlay name is now remembered when the coordinates are picked from an overlay.
 *
 * Revision 1.11  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.10  2007/08/24 22:22:33  endo
 * Draw_Neutral ghosting problem fixed.
 *
 * Revision 1.9  2007/08/10 04:41:05  pulam
 * Added scale fix and military unit drawing fix.
 *
 * Revision 1.8  2007/05/15 18:45:03  endo
 * BAMS Wizard implemented.
 *
 * Revision 1.7  2007/01/28 21:00:54  endo
 * MEXP_FEATURE_OPTION_POLYGONS added.
 *
 * Revision 1.6  2006/12/05 01:45:16  endo
 * return_geocoordinates() added and can be ivoked by -G option.
 *
 * Revision 1.5  2006/10/23 22:19:10  endo
 * ICARUS Wizard revised to incorporate CIM's latitude/longitude.
 *
 * Revision 1.4  2006/10/02 19:20:40  endo
 * ICARUS Loiter and Communication icons added.
 *
 * Revision 1.3  2006/09/26 19:29:57  nadeem
 * Added a mockup virtual clock display. Right now the clock display can be speeded up to 10 times
 *
 * Revision 1.2  2006/09/13 19:06:53  endo
 * ICARUS Wizard implemented.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.29  2006/07/12 06:16:42  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.28  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.27  2006/07/11 16:29:11  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.26  2006/07/11 12:55:47  endo
 * CMDLi Panel migrated from MARS 2020.
 *
 * Revision 1.25  2006/07/11 10:07:49  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.24  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.23  2006/07/10 17:46:29  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.22  2006/07/10 06:18:59  endo
 * Background color changed to LightYellow3
 *
 * Revision 1.21  2006/07/07 04:33:09  endo
 * Minor clean-up.
 *
 * Revision 1.20  2006/06/26 21:30:06  endo
 * Fix for Fedore Core 5.
 *
 * Revision 1.19  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.18  2006/05/15 01:50:10  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.17  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.16  2006/02/19 17:58:00  endo
 * Experiment related modifications
 *
 * Revision 1.15  2006/02/14 02:28:20  endo
 * gAutomaticExecution flag and its capability added.
 *
 * Revision 1.14  2006/01/30 02:43:16  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.13  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.12  2005/08/09 19:14:00  endo
 * Things improved for the August demo.
 *
 * Revision 1.11  2005/07/28 04:55:16  endo
 * 3D visualization improved more.
 *
 * Revision 1.10  2005/07/27 20:38:09  endo
 * 3D visualization improved.
 *
 * Revision 1.9  2005/07/23 05:58:19  endo
 * 3D visualization improved.
 *
 * Revision 1.8  2005/06/07 23:33:57  alanwags
 * CNP revision for premission specification
 *
 * Revision 1.7  2005/05/19 21:29:34  endo
 * The conflict between Mouse Inputs for CBR Wizard and 3D visualization fixed.
 *
 * Revision 1.6  2005/05/18 21:09:48  endo
 * AuRA.naval added.
 *
 * Revision 1.5  2005/04/08 01:56:03  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:37  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:54  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:12:34  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:09  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.231  2003/06/19 20:35:15  endo
 * playback feature added.
 *
 * Revision 1.230  2003/04/06 12:44:19  endo
 * gcc 3.1.1
 *
 * Revision 1.229  2003/04/02 21:47:19  zkira
 * Added telop status window GUI support
 *
 * Revision 1.228  2002/11/05 19:16:33  blee
 * - added g_strEnvChangeFilename
 * - in main()
 *   - got rid of strEnvChangeFilename
 *   - changed "case 'E':"
 *   - removed call to LoadEnvChangeFile()
 *
 * Revision 1.227  2002/10/31 21:36:37  ebeowulf
 * Added the code to reset the simulation environment.
 *
 * Revision 1.226  2002/10/08 16:57:09  blee
 * Added a signal handler for SIGCHLD to reap dead child processes.
 *
 * Revision 1.225  2002/08/22 14:35:15  endo
 * mlab can now accept the command line options -C and -T to popup
 * the Command Panel and Telop window, respectively.
 *
 * Revision 1.224  2002/07/18 17:13:29  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.223  2002/07/02 20:42:58  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.222  2002/04/04 18:44:19  blee
 * Added stuff to support changing the environment dynamically.
 *
 * Revision 1.221  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.220  2001/12/23 22:22:22  endo
 * Preparing for 5.0 release.
 *
 * Revision 1.219  2001/12/22 16:20:28  endo
 * RH 7.1 porting.
 *
 * Revision 1.218  2001/11/13 15:01:08  endo
 * The problem of mlab not being able to display a robot with a right color
 * was fixed by painting color XOR GC regions with a white XOR GC.
 *
 * Revision 1.217  2000/11/20 19:33:54  sgorbiss
 * Add visualization of line-of-sight communication
 *
 * Revision 1.216  2000/11/10 23:40:34  blee
 * added interface for learning momentum and the abiltiy to save
 * sensor readings
 *
 * Revision 1.215  2000/08/15 20:54:29  endo
 * popup_start_proceed_dialog added.
 *
 * Revision 1.214  2000/08/01 21:19:51  endo
 * Copyright and Contributers updated.
 *
 * Revision 1.213  2000/07/07 18:15:04  endo
 * create_save_ovl_as_dialog(main_window); added.
 * Waypoint functions moved to waypoint.c .
 *
 * Revision 1.212  2000/07/02 02:51:28  conrad
 * *** empty log message ***
 *
 * Revision 1.211  2000/07/02 02:47:32  conrad
 * added sonar display
 *
 *
 * Revision 1.209  2000/03/23 06:04:47  endo
 * In delete_waypoint():
 * gt_draw_cb() --> draw_world()
 *
 * Revision 1.208  2000/03/16 04:39:32  jdiaz
 * delete waypoints with middle mouse
 *
 * Revision 1.207  2000/03/15 17:02:24  sapan
 * Added logging calls
 *
 * Revision 1.206  2000/03/13 18:19:45  endo
 * This modification improves the way the alert window
 * pops up. It is now scrollable window, so that any
 * long message can be read. It now rings a bell when
 * it pops up, too.
 *
 * Revision 1.205  2000/02/29 21:00:25  saho
 * Added sound simulation and motivational vector menu items.
 *
 * Revision 1.204  2000/02/18 01:59:09  endo
 * alert_dialog added.
 *
 * Revision 1.203  2000/02/14 05:25:29  conrad
 * added pause before execution window and command line arg (-P).
 *
 * Revision 1.202  2000/02/02 23:42:54  jdiaz
 * waypoints support
 *
 * Revision 1.201  1999/12/18 09:34:42  endo
 * Data logging and Report current state capabilities
 * added.
 *
 * Revision 1.200  1999/12/18 00:01:11  endo
 * Display width restored.
 *
 * Revision 1.199  1999/12/16 22:47:48  mjcramer
 * RH6 port
 *
 * Revision 1.194  1999/08/30 21:42:44  jdiaz
 * changed the pick point from overlay to add the origin instead of subtract it
 *
 * Revision 1.193  1999/08/12 18:12:28  endo
 * *** empty log message ***
 *
 * Revision 1.192  1999/08/12 01:50:37  endo
 * *** empty log message ***
 *
 * Revision 1.191  1999/08/11 21:22:28  endo
 * "Contributers" added in the "About" window.
 *
 * Revision 1.190  1999/06/02 01:43:56  xu
 * Added "-m msgfile" command line option to write point-and-click coordinates into msgfile.
 * This option is usually used by a new feature in cfgedit: when you click middle button
 * on a "GoTo" behavior in cfgedit, it will call mlab with -m option so you can get the
 * target coordinates with a simple point-and-click on the overlay.
 *
 * Revision 1.187  1999/05/25 04:42:37  xu
 * Fixed a bug that's squeezing "Time Scale" window's size
 *
 * Revision 1.186  1999/04/27 03:12:01  conrad
 * Added refresh buttion.
 *
 * Revision 1.184  1999/03/09 22:47:07  endo
 * mlab window now fits on both large and small screens because
 * it detects the screen size.
 *
 * Revision 1.183  1999/02/22  18:11:31  xu
 * added a "load overlay" item in file menu
 *
 * Revision 1.21 1999/02/09 00:58:41  xu
 * Added file name filter "*.ovl" to the "Load Overlay" box.
 *
 * Revision 1.2  1999/02/03 00:58:41  xu
 * Added a "Load Overlay" in the File menu and it is actually working.
 *
 * Revision 1.1  1999/02/02 05:07:13  xu
 * Initial revision
 *
 * Revision 1.182  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.181  1997/01/15  21:30:16  zchen
 * add real 3d stuff
 *
 * Revision 1.180  1996/06/05  17:46:41  kali
 * changed location telop windows will pop up at
 *
 * Revision 1.179  1996/06/04  20:12:21  doug
 * Fixed -r and moved cfgedit to using a new -R
 *
 * Revision 1.178  1996/05/07  19:54:11  doug
 * fixing compiler warnings
 *
 * Revision 1.177  1996/04/02  14:17:54  doug
 * *** empty log message ***
 *
 * Revision 1.176  1996/03/13  01:52:58  doug
 * *** empty log message ***
 *
 * Revision 1.175  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.174  1996/03/05  22:56:09  doug
 * *** empty log message ***
 *
 * Revision 1.173  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.172  1996/03/01  00:47:09  doug
 * *** empty log message ***
 *
 * Revision 1.171  1996/02/29  01:53:18  doug
 * *** empty log message ***
 *
 * Revision 1.170  1996/02/28  03:54:51  doug
 * *** empty log message ***
 *
 * Revision 1.169  1996/02/27  05:00:04  doug
 * *** empty log message ***
 *
 * Revision 1.168  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.167  1995/11/08  16:51:20  doug
 * *** empty log message ***
 *
 * Revision 1.167  1995/11/08  16:51:20  doug
 * *** empty log message ***
 *
 * Revision 1.166  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.165  1995/10/23  19:54:34  doug
 * *** empty log message ***
 *
 * Revision 1.164  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.163  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.163  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.162  1995/08/24  21:19:20  doug
 * If executing with -r, then also skip copyright
 *
 * Revision 1.161  1995/07/18  14:26:56  jmc
 * Made initial copyright notice come up by default.
 *
 * Revision 1.160  1995/07/07  14:08:02  jmc
 * Removed unused variable.
 *
 * Revision 1.159  1995/07/06  23:37:58  jmc
 * Modified gray scale to 100 gray levels, and create them via X11
 * graynn named colors.  Attempt to allocate all the colors in the
 * default color map before even creating a new colormap.  Fixed a
 * bug in find_nearest_color().
 *
 * Revision 1.158  1995/07/06  20:35:25  jmc
 * Disabled some fallbacks if TELOP stuff is disabled.  Added -p to
 * usage message.
 *
 * Revision 1.157  1995/07/06  15:25:58  kali
 * added parameter to send personality file name to gt_create_telop_interface
 *
 * Revision 1.156  1995/07/05  21:04:46  jmc
 * Modified fallback resources to move personality window to a more
 * convenient location.
 *
 * Revision 1.155  1995/07/05  20:47:11  jmc
 * Moved select color specifications from widget creation to fallback
 * resources.
 *
 * Revision 1.154  1995/07/05  17:37:23  jmc
 * Added fallback resources to position the telop windows.
 *
 * Revision 1.153  1995/07/05  17:19:40  jmc
 * Added command line argument for personality filename.
 *
 * Revision 1.152  1995/06/30  18:58:48  jmc
 * Fixed minor oversight in the default font stuff.
 *
 * Revision 1.151  1995/06/30  18:41:59  jmc
 * Modified time scale panels to allow bottom part to handle resizes
 * better.  Redid finding big_font for the about dialog box to make
 * it more robust (hopefully).  Tweaked the setenv DISPLAY error
 * message.
 *
 * Revision 1.150  1995/06/28  16:17:30  jmc
 * Fixed color map problem.  Removed XSetWindowColormap since it
 * apparently should not be used until the widgets using it are
 * realized.  Other minor tweaks.
 *
 * Revision 1.149  1995/06/27  15:56:48  kali
 * Added code to gt_popup_telop_personality to create some necessary variables
 * which gt_popup_telop_interface used to create for both windows.
 *
 * Revision 1.148  1995/06/27  13:56:16  jmc
 * Numerous rearrangments and cleanup.  Added separate menu
 * entry for the telop personality control.
 *
 * Revision 1.147  1995/06/16  20:57:34  jmc
 * Fixed bug in color allocation scheme.  Improved error reporting
 * for the failure to allocate the appropriate number of grays.
 *
 * Revision 1.146  1995/06/16  15:02:20  jmc
 * Deleted unused variables and other minor fixes to eliminate
 * compiler complaints (with -Wall).  Minor tweaks to
 * create_colormap().
 *
 * Revision 1.145  1995/06/14  23:19:26  jmc
 * Modified color map creation.  If room exists in the default
 * colormap, use it instead of creating a new one.
 *
 * Revision 1.144  1995/06/14  17:12:53  jmc
 * Added the set_show_colormap() function to modify the show_colormap
 * flag.  Moved the colormap drawing stuff into a new function,
 * draw_colormap().  Rearranged code: moved all the colormap stuff to
 * be earlier in the function implementations.
 *
 * Revision 1.143  1995/06/13  21:06:35  jmc
 * Tweaked usage message.  Minor cosmetic adjustments.
 *
 * Revision 1.142  1995/06/13  19:49:34  jmc
 * Fixed draw_scale_ruler() to update the display.
 *
 * Revision 1.141  1995/06/13  18:31:42  jmc
 * Added set_busy_cursor() and reset_cursor() functions to show a
 * watch during busy computations.
 *
 * Revision 1.140  1995/06/12  23:35:23  jmc
 * Redid the expose callbacks for the drawing_area and the
 * scale_ruler to improve performance.  Only redraws when the event
 * count goes to zero.
 *
 * Revision 1.139  1995/06/09  21:40:23  jmc
 * Numerous changes to allow for a private colormap (necssary for the
 * photo grayscales).  Added option and modified redraw() to show the
 * colormap (-C).  Added "map" and grayscale definitions.  Removed
 * extra call to clear_drawing_area() in main() since the
 * set_mission_area_size command does this anyway.  Cleaned up main
 * (moved some code out into functions and reorganized).
 *
 * Revision 1.138  1995/05/22  14:44:31  jmc
 * Added a usage message and a -h command line help option.  Added
 * the -X option to print the fallback resources and quit.
 *
 * Revision 1.137  1995/05/16  15:43:52  jmc
 * Changed "System" menu name to "Debug" since all it contains (for
 * now) is debug options.
 *
 * Revision 1.136  1995/05/09  15:21:03  jmc
 * Added Khaled to the "about" author list.  Modified all copyright
 * messages so the include "MissionLab" and the version number.
 *
 * Revision 1.135  1995/05/08  20:13:08  jmc
 * Added some code to keep TELOP from being invoked when it was
 * already being used to control a unit.
 *
 * Revision 1.134  1995/05/08  19:39:40  jmc
 * Added the -I option to main() for the IPT server host.
 *
 * Revision 1.133  1995/05/08  14:43:04  jmc
 * Deleted some obsolete code.
 *
 * Revision 1.132  1995/05/06  21:48:15  jmc
 * Changed the [Zoom in] and [Zoom out] buttons to a new format to
 * take advantage of the new zoom factor: Zoom: 100% [v][^].
 * Did a few other modifications necessary to make this work smoothly.
 * Cleaned up a few odds and ends.
 *
 * Revision 1.131  1995/05/05  16:19:32  jmc
 * Fixed a problem with the new -W and -H options so the startup
 * filename processing would ignore them.  Added/improved comments.
 *
 * Revision 1.130  1995/05/04  22:07:37  jmc
 * Added include for gt_scale.h since all the scale related functions
 * moved to it.  Moved gt_show_trails to draw.*.  Moved all scale
 * related stuff (eg, meters_per_pixel) to gt_scale.*.
 *
 * Revision 1.129  1995/05/04  20:04:25  jmc
 * Various fixes to accomodate the different style of map scaling.
 * Added calls to set_mission_area_size to recompute several
 * length-related parameters (min/max obstacle size and robot
 * length).  Added a call to set_mission_area_size to main() to
 * initialize all of the length related parameters.  Changed the
 * cycle duration scale to use 2 decimal places.
 *
 * Revision 1.128  1995/05/03  21:17:10  jmc
 * Fixed a typo in a comment.
 *
 * Revision 1.127  1995/05/03  19:22:07  jmc
 * Added appropriate arguments to pop_telop_interface calls.
 * Modified call of create_telop_interface to pass the arguments that
 * it wanted.
 *
 * Revision 1.126  1995/05/02  21:30:49  jmc
 * Converted grid_* variables to mission_area_* and drawing_area_*
 * variables.  Added set_mission_area_size command.  Moved the
 * implementation of set_meters_per_pixel here (from gt_world.c).
 * Various changes to allow variable mission area size and fixed
 * pixmap size.
 *
 * Revision 1.125  1995/04/28  17:28:56  jmc
 * Renamed pixmap variable to drawing_area_pixmap.
 *
 * Revision 1.124  1995/04/27  19:49:50  jmc
 * Converted GRID_HEIGHT/WIDTH_METERS and MAX/MIN_METERS_PER_PIXEL
 * variables to lower case.  Converted a number of occurances of
 * (int)(x+0.5) to nint(x).  Implemented clear_drawing_area() and
 * converted some direct clearing of the drawing area to use it.
 *
 * Revision 1.123  1995/04/26  16:01:19  jmc
 * Added set_file_open_dialog_filename() function to update the
 * filename in the file open dialog box.
 *
 * Revision 1.122  1995/04/25  21:55:07  jmc
 * Modified some of the user warnings to put the filename on a
 * separate line since they can be very long.
 *
 * Revision 1.121  1995/04/24  20:03:52  jmc
 * Added a call to set_meters_per_pixel(1.0) in clear_simulation() to
 * reset the scale.
 *
 * Revision 1.120  1995/04/20  15:49:24  jmc
 * Corrected spelling.
 *
 * Revision 1.119  1995/04/19  22:08:14  jmc
 * Added default cases to several switch statements.
 *
 * Revision 1.118  1995/04/19  21:53:50  jmc
 * Fixed an insignificant problem that was pointed out by the latest
 * g++ compiler in -Wall mode.
 *
 * Revision 1.117  1995/04/19  19:51:58  jmc
 * Added the scroll_drawing_area_to_bottom function.  Added code to
 * grab the drawing_area_scroll_bar after it is created.  Reorganized
 * the variable declarations and functions.  Added comments here and
 * there.
 *
 * Revision 1.116  1995/04/14  17:30:18  jmc
 * Renamed clear_world() to clear_obstacles().
 *
 * Revision 1.115  1995/04/14  14:30:47  jmc
 * Renamed "configure/world" stuff: world_cb->configure_cb,
 * gt_create_configure_panel->gt_create_obstacle_panel,
 * gt_popup_configuration_panel->gt_popup_configure_pane.
 *
 * Revision 1.114  1995/04/12  19:12:18  jmc
 * Modified a couple things to use the file_exists function from (the
 * new) file_utils.*
 *
 * Revision 1.113  1995/04/10  17:11:38  jmc
 * Converted the variable METERS_PER_PIXEL to meters_per_pixel.
 * Moved the definition of grid_width_pixel and grid_height_pixel
 * from gt_world.c to here.
 *
 * Revision 1.112  1995/04/06  20:44:09  jmc
 * Deleted use of WINDOW_WIDTH and WINDOW_HEIGHT and move those
 * values into the fallback resources.  This seems to have fixed
 * problem of mlab being created too large.
 *
 * Revision 1.111  1995/04/04  21:08:37  kali
 * chmod g+w *
 *
 * Revision 1.110  1995/04/03  19:41:48  kali
 * *** empty log message ***
 *
 * Revision 1.109  1995/04/03  16:05:37  jmc
 * Added a copyright notice.  Made copyright_str a top level (but not
 * global) variable so that "copyright" doesn't need to be included
 * twice (to quiet "make depend" warnings).
 *
 * Revision 1.108  1995/04/03  13:30:05  kali
 * Added menu item under Command menu to start Telop interface.
 *
 * Revision 1.107  1995/03/31  14:56:44  doug
 * make sigcatch abort instead of exit
 *
 * Revision 1.106  1995/03/31  14:53:53  doug
 * add text to signal catching code
 *
 * Revision 1.105  1995/03/30  16:41:41  doug
 * fixed a couple warnings from the new GCC upgrade
 *
 * Revision 1.104  1995/03/07  23:02:06  jmc
 * Changed the order of popup_command_panel and load_startup_file in
 * the startup code to (hopefully) avoid the problem of the command
 * panel (occasionally) popping up microscopic when the "-c" option
 * is given and the filename is specified on the command line.
 * Changed a couple of C++ style comments to C style comments to
 * avoid problems with my indenter.
 *
 * Revision 1.103  1995/03/07  21:42:58  jmc
 * Modifications to allow the proceed dialog box to pop-up somewhere
 * other than the center of the MissionLab console window (on top of
 * everything).
 *
 * Revision 1.102  1995/02/13  21:07:41  jmc
 * Modifications to the names in the menu structure to make things
 * more logical.
 *
 * Revision 1.101  1995/02/12  21:40:12  doug
 * Redid the command bar code so it works on Motif 2.0
 *
 * Revision 1.100  1995/02/10  17:29:00  jmc
 * Changed the copyright dialog to use a separate file ("copyright")
 * which contains the copyright notification text as a string.  Added
 * the initial copyright notification via an exclusive dialog (which
 * uses the file "copyright" too).  Added command line options to
 * enable/disable display of the initial copyright notification.
 *
 * Revision 1.99  1995/02/03  16:59:26  jmc
 * Changed popup panels to set their slider/button values from the
 * global variables.  This way, the global variable is always the
 * "correct" value and obviates any corrective action to update the
 * global variables when the dialog is canceled.  This allowed me to
 * delete several saved_cycle_duration variable and simplify things
 * considerably.
 *
 * Revision 1.98  1995/02/02  22:23:58  jmc
 * Implemented the set_cycle_duration function.
 *
 * Revision 1.97  1995/02/02  20:01:57  jmc
 * Moved the debug option menu items down into a pulldown system
 * menu.
 *
 * Revision 1.96  1995/02/02  16:41:31  jmc
 * Changed show_movement_vector to show_movement_vectors.
 * Implemented set_show_movement_vector function.
 *
 * Revision 1.95  1995/02/02  15:28:58  jmc
 * Changed simulated_cycle_duration variable name to cycle_duration.
 *
 * Revision 1.94  1995/01/30  16:59:01  doug
 * oops, forgot a ,
 *
 * Revision 1.93  1995/01/30  16:20:14  doug
 * Added fallback resource of "*foreground: Black" to get black text in
 * widgets.  On Linux, it is white.
 *
 * Revision 1.92  1995/01/30  15:45:20  doug
 * porting to linux
 *
 * Revision 1.91  1995/01/13  18:00:18  jmc
 * Added a time scale to the "World" menu.  Uses/modifies the new
 * simulated_cycle_duration variable.
 *
 * Revision 1.90  1995/01/11  21:17:19  doug
 * added new show vector option
 *
 * Revision 1.89  1994/12/08  15:39:20  jmc
 * Updated the copyright message.
 *
 * Revision 1.88  1994/11/29  19:48:47  jmc
 * Added the copyright dialog box.
 *
 * Revision 1.87  1994/11/29  17:37:15  doug
 * added -S option to set the small world
 *
 * Revision 1.86  1994/11/29  17:31:06  doug
 * support small worlds
 *
 * Revision 1.85  1994/11/23  15:39:13  jmc
 * Changed mailing address to mlab@cc.
 *
 * Revision 1.84  1994/11/23  15:15:33  jmc
 * Added a list of fonts to try for the default font.
 *
 * Revision 1.83  1994/11/21  22:38:58  jmc
 * Added a little code to catch the problem of starting the program
 * without X running.  Minor hack to create_help_dialog.
 *
 * Revision 1.82  1994/11/21  18:51:44  jmc
 * Modified create_help_dialog to create GC's for erasing and drawing
 * the help dialog.
 *
 * Revision 1.81  1994/11/21  16:49:10  jmc
 * Added code to use a default font throughout.  Modified the help
 * dialog box to print "MissionLab" in large italic letters.  Moved
 * creation of the dialog boxes until after GC's have been created
 * since I need the GC's in the help dialog.  Changed the default
 * background color to LightSkyBlue3 to get better 3-D appearance.
 *
 * Revision 1.80  1994/11/17  21:42:38  jmc
 * Modifications to try and get the colors decent.  Work in progress.
 *
 * Revision 1.79  1994/11/15  21:51:12  jmc
 * Modified clear_simulation to abort execution first and then delete
 * things.
 *
 * Revision 1.78  1994/11/15  16:11:46  jmc
 * Added warning to Clear All if invoked while executing.
 *
 * Revision 1.77  1994/11/14  20:34:53  jmc
 * Modified fallback resources to place the command interface better,
 * by default (since it is now larger).
 *
 * Revision 1.76  1994/11/14  19:45:12  jmc
 * Changed -t option to -T option so X wouldn't eat it.  Added code
 * to warn user if the command file on the command line doesn't
 * exist.
 *
 * Revision 1.75  1994/11/11  22:40:38  jmc
 * Changed simulation_clear to clear_simulation.
 *
 * Revision 1.74  1994/11/11  16:59:42  jmc
 * Cleaned up minor problems (unused variables, etc) to get rid of
 * make -Wall problems.
 *
 * Revision 1.73  1994/11/11  16:33:24  jmc
 * Renamed world_clear to clear_world.
 *
 * Revision 1.72  1994/11/11  16:21:54  jmc
 * Tweaked okay label.
 *
 * Revision 1.71  1994/11/10  21:12:48  jmc
 * Finally got the error dialog box working.  Gave up resizing to
 * make it more reliable on the other window managers.  Looks a bit
 * dorky, but it seems to work well.
 *
 * Revision 1.70  1994/11/10  19:44:45  jmc
 * Modified the starting positions of the command panel, etc, in the
 * fallback resources.   Added option to specify a seed on the
 * command line.  In the process of trying to get the error dialog
 * box stuff working.
 *
 * Revision 1.69  1994/11/09  22:13:10  jmc
 * Modified proceed dialog box [okay] and [cancel] callbacks to fix a
 * bug.  Changed setup code and okay and cancel callbacks to delete
 * the timer so that the dialog box isn't deleted twice.
 *
 * Revision 1.68  1994/11/09  19:22:13  jmc
 * Fixed border of drawing area to make it a thin black line.
 * Added label indicating size of mission area to the right of the
 * scale ruler.
 *
 * Revision 1.67  1994/11/08  23:13:59  jmc
 * Modified the command bar to put pushbuttons for zooming in and out
 * on the right end.   Fiddled with color code to make colors work
 * for scale ruler and impact circles for BOTH monochrome and color.
 *
 * Revision 1.66  1994/11/08  18:19:27  jmc
 * Added GC's for the scale ruler to allow it to draw the using the
 * same colors as the rest of the menu bar and command bar.
 *
 * Revision 1.65  1994/11/08  17:51:25  doug
 * fixed red color
 *
 * Revision 1.64  1994/11/08  16:02:15  doug
 * added redxor gcc
 *
 * Revision 1.63  1994/11/07  23:05:41  jmc
 * Added accelerators for some of the main menu items.
 *
 * Revision 1.62  1994/11/04  20:23:24  jmc
 * Added the command_bar below the menu bar and added the scale ruler
 * on its left end.  Added various supporting functions:
 * draw_scale_ruler, create_scale_ruler.  Modified main
 * appropriately.
 *
 * Revision 1.61  1994/11/04  15:24:26  jmc
 * Added set_debug_scheduler function.
 *
 * Revision 1.60  1994/11/03  23:48:35  jmc
 * Tweaked configuration menu "Clear" -> "Clear all".
 *
 * Revision 1.59  1994/11/03  20:19:16  doug
 * added options button to turn on cthreads scheduler messages
 *
 * Revision 1.58  1994/11/03  18:39:37  jmc
 * Rearranged the options menu.  Changed draw_halos to
 * highlight_repelling_obstacles.  Added set_show_repelling_obstacles
 * function.
 *
 * Revision 1.57  1994/11/02  23:46:02  jmc
 * Modified to use gt_randomize_seed to create the initial seed.
 *
 * Revision 1.56  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.55  1994/11/02  00:10:14  jmc
 * Changed order in "World" menu.
 *
 * Revision 1.54  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.53  1994/11/01  22:00:10  jmc
 * Added file open selection dialog box and added it to the "File"
 * menu under "Open".  Moved "Clear" to "World" menu.
 *
 * Revision 1.52  1994/11/01  16:14:13  jmc
 * Fixed a minor bug to make the timeout on the OCCUPY UNTIL work.
 *
 * Revision 1.51  1994/10/31  19:53:02  jmc
 * Added a "Scale" to the "World" menu.  Converted meter_per_pixel to
 * meters_to_pixel.   Moved the initial position of the configuration
 * panel and the scale panel to the fallback resources.
 *
 * Revision 1.50  1994/10/31  16:00:40  jmc
 * Tweaked option parsing.  'd' and 'v' do the same thing, so
 * switched them to the same "case".
 *
 * Revision 1.49  1994/10/28  21:05:47  jmc
 * Changed gt_proceed_clearance to include timeout_given and timeout.
 * Added a timeout function (proceed_dialog_timeout_cb) for canceling
 * the proceed dialog box after the timeout (or final time).  Modified
 * gt_proceed_clearance to start up a timer using that function, when
 * appropriate.
 *
 * Revision 1.48  1994/10/28  00:49:49  jmc
 * Minor tweaks.
 *
 * Revision 1.47  1994/10/27  18:26:36  jmc
 * Changed "console" to "MissionLab".  The name of the executable
 * will be mlab.  Hacked fallback resources to get the "MissionLab"
 * on the title bar.
 *
 * Revision 1.46  1994/10/27  16:55:37  jmc
 * More tweaking with the error dialog box.
 *
 * Revision 1.45  1994/10/27  16:38:15  jmc
 * Changed int load_startup_file to save the filename in the text
 * field in the command panel whether or not it loads correctly.
 *
 * Revision 1.44  1994/10/27  16:28:42  jmc
 * Tweaked error dialog box display trying to fix some glitches.
 * Minor tweaks to some warning messages.
 *
 * Revision 1.43  1994/10/27  14:11:44  jmc
 * Changes by Doug to get the scrolling working.  Converted units
 * from feet to meters.
 *
 * Revision 1.42  1994/10/25  21:58:30  jmc
 * Added -c option to console command line that will popup the
 * command panel automatically.
 *
 * Revision 1.41  1994/10/25  14:25:55  jmc
 * Added gt_initialize_robot_info to the initialization.  Minor
 * changes to includes.  Weeded out small problems for -Wall.
 *
 * Revision 1.40  1994/10/18  22:33:20  jmc
 * Added gt_abort_command_execution to clear_simulation to make sure
 * things are reset properly when [Clear] is used.  Minor hack to
 * "About" information.
 *
 * Revision 1.39  1994/10/18  20:22:24  jmc
 * Added line to simulation_clear to clear the step list too.
 *
 * Revision 1.38  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.37  1994/10/13  18:23:58  jmc
 * Updated the "About" message.
 *
 * Revision 1.36  1994/10/11  20:12:50  doug
 * added tcxclose call
 *
 * Revision 1.35  1994/10/07  21:12:34  jmc
 * Moved XtVaAppInitialize up so that it would run before the
 * arguments are parsed since it needs to strip out the X-related
 * arguments first.
 *
 * Revision 1.34  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.33  1994/10/03  21:11:50  doug
 * switched to tcx
 *
 * Revision 1.32  1994/09/09  21:39:12  jmc
 * Converted main program resource name from "Simulation" to
 * "Console".   Added help dialog box and help menu with "about"
 * entry.  Moved fallback resource definition back into main().
 *
 * Revision 1.31  1994/09/08  14:43:45  jmc
 * Moved fallback resources to resources.c
 *
 * Revision 1.30  1994/09/06  14:11:59  jmc
 * Implemented main work procedure to allow round-robin execution of
 * all workprocs added through add_workproc.  Priority workprocs can
 * still be added via XtAppAddWorkProc.
 *
 * Revision 1.29  1994/09/03  14:52:09  jmc
 * Added X resource for initial position for command interface to the
 * fallback_resources.
 *
 * Revision 1.28  1994/09/02  20:03:45  jmc
 * Implemented add_workproc function.
 *
 * Revision 1.27  1994/08/29  18:09:48  jmc
 * Tweaked the options menu.
 *
 * Revision 1.26  1994/08/26  16:00:06  jmc
 * Implemented set_show_trails, set_debug_robots, and
 * set_debug_simulator functions to set parameter.
 *
 * Revision 1.25  1994/08/25  22:07:45  jmc
 * Minor hack to load_startup_file.  Changed startup_file to
 * startup_filename.
 *
 * Revision 1.24  1994/08/25  21:52:56  jmc
 * Added ability to specify a startup file on the command line.  If
 * present, it is automatically loaded on startup.  Also added a "-r"
 * option to automatically run the startup file (if present).
 *
 * Revision 1.23  1994/08/23  16:05:47  jmc
 * Added the get_proceed_clearance function to popup a proceed dialog
 * box.  This form of setting the callbacks works but the form using
 * a static callback_list failed.
 *
 * Revision 1.22  1994/08/22  22:17:18  jmc
 * Added the Options menu with its option toggle buttons for
 * show_trails, debug, and robot_debug_messages.  Moved show_trails
 * stuff to here from gt_world.c.
 *
 * Revision 1.21  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.20  1994/08/05  18:04:52  jmc
 * Yet another hack to try and get the colormap working correctly
 * with color and monochrome monitors.
 *
 * Revision 1.19  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.18  1994/08/03  21:29:54  jmc
 * Fixed the fg/bg color code to work on both color and monochrome
 * screens.  Tested on Sun SparcStation IPC and DECstation 5000/120.
 *
 * Revision 1.17  1994/08/03  19:45:43  jmc
 * Did several minor hacks to get the colors working again and
 * independent of the background specified in the fallback resources.
 *
 * Revision 1.16  1994/08/03  17:14:17  jmc
 * Added a void function prototype to warn_user_ok_cb instead of
 * defaulting to "int".
 *
 * Revision 1.15  1994/08/03  16:40:38  jmc
 * Tried to figure out why the dialog shell was not resizing itself
 * correctly.  Added minor hack to fix it.  Made OK button of error
 * dialog wider.
 *
 * Revision 1.14  1994/08/02  20:56:22  jmc
 * Added function warn_userf (with printf style arguments).
 *
 * Revision 1.13  1994/07/25  22:30:35  jmc
 * Modified warn_user to append cumulative messages onto the error
 * dialog message (this included adding an okay callback for the
 * error dialog box and a local variable to indicate whether the
 * dialog box is displayed.  Also added initial position for main
 * window to fallback resources.
 *
 * Revision 1.12  1994/07/22  21:58:24  jmc
 * Removed unnecessary XtCloseDisplay(XtDisplay(w)).
 *
 * Revision 1.11  1994/07/22  21:34:10  jmc
 * Removed some debugging code (for warn_user) that I forgot to remove before.
 *
 * Revision 1.10  1994/07/22  21:19:03  jmc
 * Added error dialog box to pop up warnings and errors.
 * Wrote function warn_user to pop up the dialog box.
 * Put header for warn_user in gt_sim.h.
 *
 * Revision 1.9  1994/07/19  16:12:38  jmc
 * Added fallback resources (primarily to set all backgrounds to gray90).
 *
 * Revision 1.8  1994/07/19  15:58:59  jmc
 * Removed setting background to gray80; should do this in the .Xdefaults file.
 *
 * Revision 1.7  1994/07/18  20:45:44  jmc
 * Added gt_create_configuration_panel.
 * Fixed menu callback for command interface (added cmd_cb).
 * Converted several callback functions to static.
 *
 * Revision 1.6  1994/07/18  19:52:16  jmc
 * Changed gt_configuration_popup to gt_popup_configuration_panel.
 *
 * Revision 1.5  1994/07/15  18:53:25  jmc
 * Added function prototype for gethostname
 *
 * Revision 1.4  1994/07/15  18:45:31  jmc
 * Added a function prototype for getopt
 * Cast several functions appropriately in XtAddCallBack's
 *
 * Revision 1.3  1994/07/12  19:14:25  jmc
 * Tweaked RCS automatic identification strings
 *
 * Revision 1.2  1994/07/12  18:07:29  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
