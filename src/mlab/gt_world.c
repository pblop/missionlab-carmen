/**********************************************************************
 **                                                                  **
 **                            gt_world.c                            **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, 1996, 1997 Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_world.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <malloc.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/timeb.h>

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>

#include <X11/Xutil.h>
#include <X11/cursorfont.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/PanedW.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Scale.h>
#include <Xm/Separator.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>

int ftime(struct timeb *tp);
}

#include "string_utils.h"
#include "gt_create_world.h"
#include "gt_sim.h"
#include "gt_scale.h"
#include "console.h"

#include "draw.h"

#include "3d_vis_wid.h"

/**********************************************************************
 **                                                                  **
 **                            constants                             **
 **                                                                  **
 **********************************************************************/

// Set to unequal 0 if you want to use the old setup of the random
// obstacle generator, where the default obstacle sizes depend on
// the screen resolution
const int USE_OLD_RANDOM_OBSTACLE_GENERATOR = 0;

const int MAX_NUM_OBST_COMPLAIN = 100;  // If more than this number of
                                        // obstacles are created, warn the user
const int MAX_COVERAGE_COMPLAIN =  20;  // If more than this coverage is
                                        // given, warn the user


/**********************************************************************
 **                                                                  **
 **                              macros                              **
 **                                                                  **
 **********************************************************************/

#define fix_x(c) (int)(c)
#define fix_y(c) (drawing_area_height_pixels - (int)(c))


#define RES_CONVERT( res_name, res_value) \
      XtVaTypedArg, (res_name), XmRString, (res_value), strlen(res_value) + 1
#define XSTRING(str) XmStringCreateLtoR(str, XmSTRING_DEFAULT_CHARSET)



/**********************************************************************
 **                                                                  **
 **                    local and global variables                    **
 **                                                                  **
 **********************************************************************/


double min_obs_r =  4.0;    // min size of obstacles in meters
double max_obs_r = 15.0;    // max size of obstacles in meters

static double MINIMUM_OBSTACLE_RADIUS =   1.0;  // minimum of min_obs_r or
                                                // max_obs_r in meters
static double MAXIMUM_OBSTACLE_RADIUS = 200.0;  // maximum of min_obs_r or
                                                // max_obs_r in meters


const double MIN_OBSTACLE_COVERAGE =  0.0;  // percent
const double MAX_OBSTACLE_COVERAGE = 30.0;  // percent

double obs_coverage = 3.0;  // obstacle coverage in percent


int debug = false;
bool paused = false;

double origin_x = 0.0;  // meters
double origin_y = 0.0;  // meters
double origin_heading = 0.0; // Degrees

static Widget obstacle_panel;
static Widget obstacle_panel_shell;
static Widget coverage_scale;
static Widget min_obs_r_scale;
static Widget max_obs_r_scale;
static Widget seed_textfield;

static Widget auto_randomize_button;
static int    auto_randomize = false;

int seed;
obs_t* obs = NULL;
int num_obs = 0;




/**********************************************************************
 **                                                                  **
 **                    implement global functions                    **
 **                                                                  **
 **********************************************************************/



/**********************************************************************/

int set_obstacle_coverage( double coverage )
{
    // make sure it is a legal value
    if ( coverage < MIN_OBSTACLE_COVERAGE )
    {
        warn_userf( "Coverage value, %1.8g%%, is less than\n"
                    "the minimum value of %1.8g%%", coverage, MIN_OBSTACLE_COVERAGE );
        return false;
    }
    if ( coverage > MAX_OBSTACLE_COVERAGE )
    {
        warn_userf( "Coverage value, %1.8g%%, is greater than\n"
                    "the maximum value of %1.8g%%", coverage, MAX_OBSTACLE_COVERAGE );
        return false;
    }

    obs_coverage = coverage;
    XmScaleSetValue( coverage_scale, nint( obs_coverage * 100.0 ) );
    return true;
}

/**********************************************************************/

int set_min_obstacle_radius( double radius )
{
    // Make sure the value is in range
    if ( radius < MINIMUM_OBSTACLE_RADIUS )
    {
        warn_userf( "Minimum obstacle radius value, %1.8g, is less than\n"
                    "the minimum value of %1.8g meters", radius, MINIMUM_OBSTACLE_RADIUS );
        return false;
    }
    if ( radius > MAXIMUM_OBSTACLE_RADIUS )
    {
        warn_userf( "Minimum obstacle radius value, %1.8g, is greater than\n"
                    "the maximum value of %1.8g meters", radius, MAXIMUM_OBSTACLE_RADIUS );
        return false;
    }

    min_obs_r = radius;
    XmScaleSetValue( min_obs_r_scale, nint( radius * 100.0 ) );
    return true;
}


/**********************************************************************/

int set_max_obstacle_radius( double radius )
{
    // Make sure the value is in range
    if ( radius < MINIMUM_OBSTACLE_RADIUS )
    {
        warn_userf( "Maximum obstacle radius value, %1.8g, is less than\n"
                    "the minimum value of %1.8g meters", radius, MINIMUM_OBSTACLE_RADIUS );
        return false;
    }
    if ( radius > MAXIMUM_OBSTACLE_RADIUS )
    {
        warn_userf( "Maximum obstacle radius value, %1.8g, is greater than\n"
                    "the maximum value of %1.8g meters", radius, MAXIMUM_OBSTACLE_RADIUS );
        return false;
    }

    max_obs_r = radius;
    XmScaleSetValue( max_obs_r_scale, nint( radius * 100.0 ) );
    return true;
}


/**********************************************************************/

void recompute_obstacle_radius_range()
{
    // Get the smallest mission area dimension
    double len = mission_area_width_meters;
    if ( mission_area_height_meters < len )
    {
        len = mission_area_height_meters;
    }

    if ( USE_OLD_RANDOM_OBSTACLE_GENERATOR == 0 )
    {
        // NEW METHOD: independent of screen resolution
   
        // Minimum size only depends on mission area, about 1/1000
        MINIMUM_OBSTACLE_RADIUS = len / 1000.0;

        // The maximum size should be equivalent to about 1/5 of the mission area
        MAXIMUM_OBSTACLE_RADIUS = len / 5.0;

        // Set default min/max values
        min_obs_r = 2.0;
        max_obs_r = 10.0;

        // and adjust them to the bounds if necessary
        max_obs_r = bound( max_obs_r, MINIMUM_OBSTACLE_RADIUS, MAXIMUM_OBSTACLE_RADIUS );
        min_obs_r = bound( min_obs_r, MINIMUM_OBSTACLE_RADIUS, MAXIMUM_OBSTACLE_RADIUS );
    }
    else
    {
        // OLD METHOD: min_obs_r, max_obs_r depend on screen resolution
   
        // The minimum size should equivalent to ~2 pixels across (zoomed in)
        MINIMUM_OBSTACLE_RADIUS = 2.0 * meters_per_pixel;

        // The maximum size should be equivalent to about 1/5 of the mission area
        MAXIMUM_OBSTACLE_RADIUS = len / 5.0;

        // Set the min/max values
        min_obs_r = 2.0 * MINIMUM_OBSTACLE_RADIUS;
        max_obs_r = ( 15.0 / 4.0 ) * min_obs_r;
        if ( max_obs_r > MAXIMUM_OBSTACLE_RADIUS )
        {
            max_obs_r = MAXIMUM_OBSTACLE_RADIUS;
        }
    }

    // Update the scales
    XtVaSetValues( min_obs_r_scale, 
                   XmNminimum, nint(MINIMUM_OBSTACLE_RADIUS * 100.0),
                   XmNmaximum, nint(MAXIMUM_OBSTACLE_RADIUS * 100.0),
                   XmNvalue, nint( min_obs_r * 100.0 ),
                   NULL );
    XtVaSetValues( max_obs_r_scale, 
                   XmNminimum, nint(MINIMUM_OBSTACLE_RADIUS * 100.0),
                   XmNmaximum, nint(MAXIMUM_OBSTACLE_RADIUS * 100.0),
                   XmNvalue, nint( max_obs_r * 100.0 ),
                   NULL );

    // Just to be sure
    XmScaleSetValue( min_obs_r_scale, nint( min_obs_r * 100.0 ) );
    XmScaleSetValue( max_obs_r_scale, nint( max_obs_r * 100.0 ) );
}

/**********************************************************************/

void clear_obstacles()
{
    if( obs != NULL )
    {
        for ( int i = 0; i < num_obs; i++ )
        {
            object_list.remove( obs[ i ].pobjShape );
        }

        free( obs );
    }

   obs = NULL;
   num_obs = 0;
}



/**********************************************************************/


int gt_add_obstacle( double x, double y, double radius,
		     double height)
{
  if (height == -1) 
    {
      height = (rand()%10) + 1;
    }

  return(create_obstacle( x, y, radius, height,
			  &obs, &num_obs ));
}


/**********************************************************************/
// returns true if the point x,y lies on top of any existing obstacle,
// otherwise, false.
int overlap_obstacle( double x, double y )
{
    for ( int i = 0; i < num_obs; i++ )
    {
        if ( Distance( obs[ i ].center.x, obs[ i ].center.y, x, y ) <= obs[ i ].radius )
        {
            return true;
        }
    }

    return false;
}



/**********************************************************************
 **                                                                  **
 **               functions for configuring the world                **
 **                                                                  **
 **********************************************************************/

extern double compute_pix_per_unit();

/**********************************************************************/

static void check_min_obs_value_cb()
{
    int min, max;

    XmScaleGetValue( min_obs_r_scale, &min );
    XmScaleGetValue( max_obs_r_scale, &max );

    if ( min > max )
    {
        XmScaleSetValue( min_obs_r_scale, max );
    }
}


/**********************************************************************/

static void check_max_obs_value_cb()
{
    int min, max;

    XmScaleGetValue( min_obs_r_scale, &min );
    XmScaleGetValue( max_obs_r_scale, &max );

    if ( max < min )
    {
        XmScaleSetValue( max_obs_r_scale, min );
    }
}



/**********************************************************************/
// Make sure that only numbers can be inserted into this text field
void numbers_only_cb( Widget w, caddr_t unused,
                      XmTextVerifyCallbackStruct* tvcs )
{
    if ( tvcs->reason != XmCR_MODIFYING_TEXT_VALUE )
    {
        return;
    }

    // examine the new data
    char* str = tvcs->text->ptr;
    int len = tvcs->text->length;
    int number = true;
    for ( int i = 0; i < len; i++ )
    {
        if ( !isdigit( str[ i ] ) )
        {
            number = false;
        }
    }

    // if it has non-digits, reject it
    if ( !number )
    {
        tvcs->doit = false;
    }
}

/**********************************************************************/

static void update_seed_display()
{
   char seed_str[ 40 ];
   sprintf( seed_str, "%d", seed );
   XmTextFieldSetString( seed_textfield, seed_str );
}

/**********************************************************************/
/* May want to delete this eventually
static int seed_from_textfield()
{
   char *str;
   int seed_value;

   str = XmTextFieldGetString(seed_textfield);
   sscanf(str, "%d", &seed_value);
   XtFree(str);

   return seed_value;
}
*/

/**********************************************************************/

static void update_seed_cb()
{
    // update the seed from seed_textfield
    char* str;

    str = XmTextFieldGetString( seed_textfield );

    // interpret the string from the seed_textfield
    if ( NO_STRING( str ) )
    {
        update_seed_display();
    }
    else
    {
        int new_seed;
        sscanf( str, "%d", &new_seed );
        XtFree( str );
        seed = new_seed;
    }
}

/**********************************************************************/

void gt_randomize_seed( int update_display )
{
    timeb tp;
    int p10 = 1;

    // figure out how many digits we want (1-10)
    ftime( &tp );
    int num_digits = ( tp.millitm % 10 ) + 1;

    // construct each digit randomly
    srand( tp.time );
    int new_seed = 0;
    for ( int i = 0; i < num_digits; i++, p10 *= 10 )
    {
        new_seed += ( rand() % 10 ) * p10;
    }

    // update the seed
    seed = new_seed;
    if ( update_display )
    {
        update_seed_display();
    }
}

/**********************************************************************/

static void randomize_seed_cb()
{
    gt_randomize_seed( true );
}

/**********************************************************************/

static void toggle_auto_randomize()
{
    if ( XmToggleButtonGetState( auto_randomize_button ) )
    {
        XmToggleButtonSetState( auto_randomize_button, True, True );
    }
    else
    {
        XmToggleButtonSetState( auto_randomize_button, False, True );
    }

    auto_randomize = XmToggleButtonGetState( auto_randomize_button );
}

/**********************************************************************/

void FillRegion( double dStartX,    double dStartY, 
                 double dWidth,     double dHeight, 
                 double dMinObsRad, double dMaxObsRad, 
                 double dCoverage,  int iSeed )
{
    fill_region( iSeed, dCoverage / 100.0, 
                 dStartX, dStartY, dWidth, dHeight, 
                 dMinObsRad, dMaxObsRad, 
                 obs, num_obs );
}

/**********************************************************************/

void ClearRegion( double dStartX, double dStartY, 
                  double dWidth,  double dHeight )
{
    if( obs != NULL )
    {
        int i;
        int iOrigNumObs = num_obs;

        double dStopX = dStartX + dWidth;
        double dStopY = dStartY + dHeight;

        // remove any obstacles within the region from the obstacle list and mark
        // them by setting their pobjShape member to NULL
        for ( i = 0; i < iOrigNumObs; i++ )
        {
            if ( ( obs[ i ].center.x >= dStartX ) && 
                 ( obs[ i ].center.x <= dStopX  ) && 
                 ( obs[ i ].center.y >= dStartY ) && 
                 ( obs[ i ].center.y <= dStopY  ) )
            {
                if ( obs[ i ].pobjShape != NULL )
                {
                    obs[ i ].pobjShape->erase();
                    object_list.remove( obs[ i ].pobjShape );
                    obs[ i ].pobjShape = NULL;
                }
                num_obs--;
            }
        }

        // we'll have to remember the current obs list to kill it later
        obs_t* temp = obs;
        obs = NULL;

        // if we have any obstacles left, relocate them to a new obs array
        if ( num_obs > 0 )
        {
            obs = (obs_t*) malloc( num_obs * sizeof( obs_t ) );

            int j = 0;

            // look at all the old obstacles
            for ( i = 0; i < iOrigNumObs; i++ )
            {
                // if it's not marked as killed, copy it to the new obs array
                if ( temp[ i ].pobjShape != NULL )
                {
                    obs[ j ] = temp[ i ];
                    j++;
                }
            }
        }

        free( temp ); // kill the old obs array
    }
}

/**********************************************************************/

void RemoveObsAt( double dX, double dY )
{
    if ( obs != NULL )
    {
        // look for the obstacke at coords ( dX, dY )
        for ( int i = 0; i < num_obs; i++ )
        {
            // see if the current obstacle is the one we're looking for
            if ( ( fabs( obs[ i ].center.x - dX ) < EPS_ZERO ) && 
                 ( fabs( obs[ i ].center.y - dY ) < EPS_ZERO ) )
            {
                // remove the obstacle from the object list
                if ( obs[ i ].pobjShape != NULL )
                {
                    obs[ i ].pobjShape->erase();
                    object_list.remove( obs[ i ].pobjShape );
                }

                // remove the obstacle from the obs array by shifting everything after
                // the removed object down by one in the array
                for ( int j = i + 1; j < num_obs; j++ )
                {
                    obs[ j - 1 ] = obs[ j ];
                }
                num_obs--;
            }
        }
    }
}

/**********************************************************************/
// create_obstacles: Use the current configuration to populate the world
void create_obstacles( int force )
{
  
  fprintf(stderr, "Creating obstacles\n");
  
  int   i;
  
  double old_obs_coverage = obs_coverage;
  double old_min_obs_r = min_obs_r;
  double old_max_obs_r = max_obs_r;
  int old_seed = seed;
  
  // get the latest values from the sliders
  XmScaleGetValue( coverage_scale, &i );
  obs_coverage = i / 100.0;
  XmScaleGetValue( min_obs_r_scale, &i );
  min_obs_r = i / 100.0;
  XmScaleGetValue( max_obs_r_scale, &i );
  max_obs_r = i / 100.0;
  
  update_seed_cb();
  
  // clear the screen
  clear_map();
  
  // only regen the obstacles if something changes so can zoom
  if ( force || ( ( old_obs_coverage != obs_coverage ) ||
		  ( old_min_obs_r    != min_obs_r    ) ||
		  ( old_max_obs_r    != max_obs_r    ) ||
		  ( old_seed         != seed         ) ) )
    {
      // get rid of any existing obstacles
      clear_obstacles();
      
      // get a new set of random obstacles
      fill_region( seed, (double)obs_coverage/100.0, 
		   0.0, 0.0, mission_area_width_meters, mission_area_height_meters, 
		   min_obs_r, max_obs_r, 
		   obs, num_obs );
      
      // clear regions   PAT!!!!
      //for (rectRegion_t* r = clear_regions; 
	//   r != NULL; 
	//   r = r->next)
	//{
	 // ClearRegion(r->lowerLeft.x, r->lowerLeft.y,
	//	      r->width, r->height);
	//}	          
      draw_world();
    }
  
    if (gt_show_3d_vis)
    {
        initialize_3d_vis();
    }
}



/**********************************************************************/
// config_apply_cb: called to allow modification of the world config
static void config_apply_cb( Widget w, Widget base )
{
    if ( auto_randomize )
    {
        gt_randomize_seed( true );
    }
    create_obstacles( true );
}

/**********************************************************************/
// config_ok_cb: called to allow modification of the world config
static void config_ok_cb( Widget w, Widget base )
{
    XtPopdown(obstacle_panel_shell);

    config_apply_cb( w, base );
}

/**********************************************************************/
// config_clear_cb: called to clear the world config
static void config_clear_cb( Widget w, Widget base )
{
    clear_map();
    clear_obstacles();

    draw_world();
}

/**********************************************************************/
// config_cancel_cb: called to hide the world configuration panel
static void config_cancel_cb( Widget w, Widget base )
{
    XtPopdown( obstacle_panel_shell );
}

/**********************************************************************/
// create_obstacle_panel: called to allow modification of the world config
void gt_create_obstacle_panel( Widget parent )
{
    Widget form1, form2, form3, widget;
    Widget seed_label, seed_update_button, seed_randomize_button;

    // create the popup
    obstacle_panel_shell = XtVaCreatePopupShell(
        "Obstacle Creation", xmDialogShellWidgetClass, parent,
        XmNdeleteResponse, XmDESTROY,
        NULL );

    obstacle_panel = XtVaCreateWidget(
        "obstacle-panel", xmPanedWindowWidgetClass, obstacle_panel_shell,
        XmNsashHeight,        1,
        XmNsashWidth,         1,
        NULL );

    form1 = XtVaCreateWidget( "config-form1", xmFormWidgetClass, obstacle_panel, NULL );

    coverage_scale = XtVaCreateManagedWidget(
        "coverage", xmScaleWidgetClass,   form1,
        RES_CONVERT(XmNtitleString, "Obstacle Coverage (%)"),
        XmNminimum,               nint(MIN_OBSTACLE_COVERAGE * 100.0),
        XmNmaximum,               nint(MAX_OBSTACLE_COVERAGE * 100.0),
        XmNvalue,                 nint(obs_coverage * 100.0),
        XmNshowValue,             True,
        XmNdecimalPoints,         2,
        XmNorientation,           XmHORIZONTAL,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL );

    min_obs_r_scale = XtVaCreateManagedWidget(
        "min_obs_r_scale", xmScaleWidgetClass, form1,
        RES_CONVERT(XmNtitleString, "Min obstacle radius (m)"),
        XmNminimum,               nint(MINIMUM_OBSTACLE_RADIUS * 100.0),
        XmNmaximum,               nint(MAXIMUM_OBSTACLE_RADIUS * 100.0),
        XmNvalue,                 nint(min_obs_r * 100.0),
        XmNshowValue,             True,
        XmNdecimalPoints,         2,
        XmNorientation,           XmHORIZONTAL,
        XmNtopAttachment,         XmATTACH_WIDGET,
        XmNtopWidget,             coverage_scale,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL );
    XtAddCallback( min_obs_r_scale, XmNvalueChangedCallback,
                   (XtCallbackProc) check_min_obs_value_cb, NULL );

    max_obs_r_scale = XtVaCreateManagedWidget(
        "max_obs_r_scale", xmScaleWidgetClass, form1,
        RES_CONVERT(XmNtitleString, "Max obstacle radius (m)"),
        XmNminimum,               nint(MINIMUM_OBSTACLE_RADIUS * 100.0),
        XmNmaximum,               nint(MAXIMUM_OBSTACLE_RADIUS * 100.0),
        XmNvalue,                 nint(max_obs_r * 100.0),
        XmNshowValue,             True,
        XmNdecimalPoints,         2,
        XmNorientation,           XmHORIZONTAL,
        XmNtopAttachment,         XmATTACH_WIDGET,
        XmNtopWidget,             min_obs_r_scale,
        XmNleftAttachment,        XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        NULL );
    XtAddCallback( max_obs_r_scale, XmNvalueChangedCallback,
                   (XtCallbackProc) check_max_obs_value_cb, NULL );

    XtManageChild( form1 );


    form2 = XtVaCreateWidget(
        "config-form2", xmFormWidgetClass, obstacle_panel,
        XmNmarginHeight,      4,
        XmNmarginWidth,       4,
        NULL );

    seed_label = XtVaCreateManagedWidget(
        " Seed:", xmLabelGadgetClass, form2,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNtopOffset,             5,
        XmNleftAttachment,        XmATTACH_FORM,
        NULL );

    auto_randomize_button = XtVaCreateManagedWidget(
        "auto-randomize", xmToggleButtonWidgetClass, form2,
        RES_CONVERT( XmNlabelString, "Auto Randomize " ),
        XmNset,                   (Boolean) auto_randomize,
        XmNindicatorType,         XmN_OF_MANY,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_FORM,
        XmNindicatorSize,         12,
        NULL );
    XtAddCallback( auto_randomize_button, XmNvalueChangedCallback,
                   (XtCallbackProc) toggle_auto_randomize, NULL );

    seed_randomize_button = XtVaCreateManagedWidget(
        "  Generate new seed  ", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_WIDGET,
        XmNrightWidget,           auto_randomize_button,
        XmNrightOffset,           20,
        NULL );
    XtAddCallback( seed_randomize_button, XmNactivateCallback,
                   (XtCallbackProc) randomize_seed_cb, NULL );

    seed_update_button = XtVaCreateManagedWidget(
        "  Accept  ", xmPushButtonGadgetClass, form2,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNrightAttachment,       XmATTACH_WIDGET,
        XmNrightWidget,           seed_randomize_button,
        XmNrightOffset,           20,
        NULL );
    XtAddCallback( seed_update_button, XmNactivateCallback,
                   (XtCallbackProc) update_seed_cb, NULL );

    seed_textfield = XtVaCreateManagedWidget(
        "seed_value", xmTextFieldWidgetClass, form2,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_WIDGET,
        XmNleftWidget,            seed_label,
        XmNrightAttachment,       XmATTACH_WIDGET,
        XmNrightWidget,           seed_update_button,
        NULL );
    XtAddCallback( seed_textfield, XmNactivateCallback,
                   (XtCallbackProc) update_seed_cb, NULL );
    XtAddCallback( seed_textfield, XmNmodifyVerifyCallback,
                   (XtCallbackProc) numbers_only_cb, NULL );

    XtManageChild( form2 );


    form3 = XtVaCreateWidget(
        "config-form3", xmFormWidgetClass, obstacle_panel,
        XmNfractionBase,  10,
        XmNskipAdjust,    True,
        NULL );

    widget = XtVaCreateManagedWidget(
        "  Ok  ",   xmPushButtonGadgetClass, form3,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_POSITION,
        XmNleftPosition,          1,
        XmNrightAttachment,       XmATTACH_POSITION,
        XmNrightPosition,         2,
        XmNshowAsDefault,         True,
        XmNdefaultButtonShadowThickness,1,
        NULL );
    XtAddCallback( widget, XmNactivateCallback,
                   (XtCallbackProc) config_ok_cb, NULL );

    widget = XtVaCreateManagedWidget(
        " Apply ", xmPushButtonGadgetClass, form3,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_POSITION,
        XmNleftPosition,          3,
        XmNrightAttachment,       XmATTACH_POSITION,
        XmNrightPosition,         4,
        XmNshowAsDefault,         False,
        XmNdefaultButtonShadowThickness,1,
        NULL );
    XtAddCallback( widget, XmNactivateCallback,
                   (XtCallbackProc) config_apply_cb, NULL );

    widget = XtVaCreateManagedWidget(
        " Cancel ", xmPushButtonGadgetClass, form3,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_POSITION,
        XmNleftPosition,          5,
        XmNrightAttachment,       XmATTACH_POSITION,
        XmNrightPosition,         6,
        XmNshowAsDefault,         False,
        XmNdefaultButtonShadowThickness,1,
        NULL );
    XtAddCallback( widget, XmNactivateCallback,
                   (XtCallbackProc) config_cancel_cb, NULL );

    widget = XtVaCreateManagedWidget(
        " Clear Obstacles ",   xmPushButtonGadgetClass, form3,
        XmNtopAttachment,         XmATTACH_FORM,
        XmNbottomAttachment,      XmATTACH_FORM,
        XmNleftAttachment,        XmATTACH_POSITION,
        XmNleftPosition,          7,
        XmNrightAttachment,       XmATTACH_POSITION,
        XmNrightPosition,         9,
        XmNshowAsDefault,         False,
        XmNdefaultButtonShadowThickness,1,
        NULL );
    XtAddCallback( widget, XmNactivateCallback,
                   (XtCallbackProc) config_clear_cb, NULL );

    XtManageChild( form3 );
}



/**********************************************************************/

void gt_popup_obstacle_panel()
{
    // Update the values in the sliders, etc
    XmScaleSetValue( coverage_scale,  nint( obs_coverage * 100.0 ) );
    XmScaleSetValue( min_obs_r_scale, nint( min_obs_r    * 100.0 ) );
    XmScaleSetValue( max_obs_r_scale, nint( max_obs_r    * 100.0 ) );
    update_seed_display();
    XmToggleButtonSetState( auto_randomize_button, auto_randomize, True );

    // pop it up
    XtManageChild( obstacle_panel );
    XtPopup( obstacle_panel_shell, XtGrabNone );
}


/**********************************************************************
 * $Log: gt_world.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/03/06 05:06:52  nadeem
 * Modified code to use Mercator projection to map between lat/lon and X/Y positions.
 *
 * Revision 1.3  2006/09/26 18:33:50  endo
 * ICARUS Wizard integrated with Lat/Lon.
 *
 * Revision 1.2  2006/09/20 18:34:48  nadeem
 * Added the code to convert latitude/longitude to X/Y and vice-versa. A lot of files had to be touched to make sure that the initialisation values can flow across from mlab to the robot executables as well as to ensure that the functions were available for use on both sides of the code.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.4  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.3  2005/08/22 22:12:50  endo
 * For Pax River demo.
 *
 * Revision 1.2  2005/03/23 07:36:56  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.87  2002/07/02 20:40:03  blee
 * changed "paused" to a bool
 *
 * Revision 1.86  2002/04/04 18:43:19  blee
 * Changed clear_obstacles() and create_obstacles().
 * Added FillRegion(), ClearRegion(), and RemoveObsAt().
 *
 * Revision 1.85  2001/10/13 22:06:36  kaess
 * Random obstacle generator now independent of screen resolution
 *
 * Revision 1.84  2000/04/25 09:11:43  jdiaz
 * added origin_heading
 *
 * Revision 1.83  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.82  1995/10/23  19:54:34  doug
 * *** empty log message ***
 *
 * Revision 1.81  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.80  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.79  1995/07/05  20:45:55  jmc
 * Removed select color specification so it will use the values
 * specified in the fallback resources.
 *
 * Revision 1.78  1995/06/30  18:12:01  jmc
 * Modified obstacle creation panel to allow bottom part to handle
 * resizes better.
 *
 * Revision 1.77  1995/06/13  20:55:39  jmc
 * Removed definition of NO_STRING macro and included string_utils.h
 * instead.   Minor cosmetic cleanup.
 *
 * Revision 1.76  1995/05/05  16:33:33  jmc
 * Removed some obsolete NOT_NEEDED code.
 *
 * Revision 1.75  1995/05/04  22:08:13  jmc
 * Moved all scale related stuff to gt_scale.*.
 *
 * Revision 1.74  1995/05/04  20:16:50  jmc
 * Various fixes to accomodate the different style of map scaling.  Renamed
 * magnify_robots to scale_robots.  Renamed user_robot_magnification to
 * robot_length_meters.  Renamed fixed_robot_magnification to
 * robot_length_pixels.  If scale_robots==TRUE, use robot_length_meters,
 * else use robot_length_pixels to compute the robot size.  Converted use
 * of ROBOT_WIDTH and ROBOT_LENGTH to ROBOT_WIDTH_LENGTH_RATIO.  Other
 * other related modifications.  Modified several functions to update
 * things appropriately.  Added functions recompute_obstacle_radius_range()
 * and recompute_robot_length() to recompute min/max obstacle size and
 * robot length.  Made most of the set_* functions return a boolean
 * indicating whether the the specified value was in range.  Moved several
 * #defines here since these constants are now only needed here (because
 * the set_* functions use them).  Made them const's instead of #defines.
 * Changed the number of decimals to 2 for all sliders.
 *
 * Revision 1.73  1995/05/03  16:39:11  jmc
 * Changed the number of digits after the decimal to three on the
 * meters_per_pixel scale.
 *
 * Revision 1.72  1995/05/02  21:37:19  jmc
 * Converted grid_* variables to mission_area_* and drawing_area_*
 * variables.  Removed the implementation of set_meters_per_pixel
 * (moved to console.c).  Implemented update_mpp_scale.  Deleted
 * rescale_world since the pixmap size is now fixed.  Modified the
 * zoom code to zoom by multiplying by the factor.
 *
 * Revision 1.71  1995/04/28  17:29:20  jmc
 * Renamed pixmap variable to drawing_area_pixmap.
 *
 * Revision 1.70  1995/04/27  19:47:24  jmc
 * Converted GRID_HEIGHT/WIDTH_METERS and MAX/MIN_METERS_PER_PIXEL
 * variables to lower case.  Minor tweaks to the function
 * gt_create_obstacle_panel().  Converted a number of occurances
 * of (int)(x+0.5) to nint(x).  Converted some direct clearing of
 * the drawing area to use clear_drawing_area() function.  Added
 * extra decimal place to meters_per_pixel scale.
 *
 * Revision 1.69  1995/04/20  18:28:25  jmc
 * Corrected spelling.
 *
 * Revision 1.68  1995/04/19  21:09:19  jmc
 * Converted obs_coverage to a double.  Converted sliders for
 * obs_coverage, min_obs_r, and max_obs_r to display a decimal from
 * improved precision.
 *
 * Revision 1.67  1995/04/14  17:29:35  jmc
 * Renamed clear_world() to clear_obstacles() and populate_world() to
 * create_obstacles().
 *
 * Revision 1.66  1995/04/14  14:26:49  jmc
 * Renamed configuration_panel functions to obstacle_panel functions.
 *
 * Revision 1.65  1995/04/12  18:03:57  jmc
 * Remove last STXmcu dependency.
 *
 * Revision 1.64  1995/04/10  17:15:45  jmc
 * Moved all drawing functions to draw.c.  Weeded out unnecessary
 * includes.
 *
 * Revision 1.63  1995/04/05  23:03:32  jmc
 * Implement the function draw_starting_point.
 *
 * Revision 1.62  1995/04/03  20:36:02  jmc
 * Added copyright notice.
 *
 * Revision 1.61  1995/04/03  14:09:32  jmc
 * Fix a bug in DrawText.  Wasn't malloc'ing some that that should
 * have been malloc'ed.
 *
 * Revision 1.60  1995/03/31  16:06:36  jmc
 * Minor hack to DrawText to make retrieval of this_font more X11R6
 * compliant.
 *
 * Revision 1.59  1995/03/30  20:31:14  doug
 * added stdlib
 *
 * Revision 1.58  1995/03/30  19:54:19  doug
 * need to include internals h file for font code under linux
 *
 * Revision 1.57  1995/03/08  23:44:46  jmc
 * Rewrote draw_axis so that it can handle axes with more than one
 * segment.
 *
 * Revision 1.56  1995/03/08  17:20:33  jmc
 * Modified several drawing routines to better position their labels.
 *
 * Revision 1.55  1995/03/07  22:50:01  jmc
 * Fixed a bug in draw_mission_name that kept the mission name from
 * being drawn at the top of screen when meters_per_pixel != 1.0.
 *
 * Revision 1.54  1995/03/03  21:05:29  jmc
 * Implemented new draw_mission_name function.  Modified DrawText
 * function to adjust the placement position to center the text and
 * put it right above the desired point.
 *
 * Revision 1.53  1995/02/28  16:47:17  doug
 * reflect y axis so positive is at top and 0 is at bottom left
 *
 * Revision 1.52  1995/02/14  22:07:58  jmc
 * Modifications to keep measure labels from being printed if
 * specified by the user.
 *
 * Revision 1.51  1995/02/13  20:25:21  jmc
 * Tweaks to make -Wall compile quietly.
 *
 * Revision 1.50  1995/02/06  14:45:49  doug
 * ifdef'd out size setting code in world popups.
 * It didn't work on the new motif edition.
 *
 * Revision 1.49  1995/02/03  16:47:34  jmc
 * Changed popup panels to set their slider/button values from the
 * global variables.  This way, they global variable is always the
 * "correct" value and obviates any corrective action to update the
 * global variables when the dialog is canceled.   This allowed me to
 * delete several saved_* variables and simplify things considerably.
 *
 * Revision 1.48  1995/01/27  19:19:16  jmc
 * Converted a number of XtVa[Set,Get]Values to XmScale[Set,Get]Value
 * function calls for all access and modification of the data in
 * Scale widgets.
 *
 * Revision 1.47  1995/01/26  20:35:35  jmc
 * Tweaked the "too many obstacles" warning message to make it a
 * little narrower to avoid cropping problems on some system.
 *
 * Revision 1.46  1995/01/11  21:17:49  doug
 * added new show vector option
 *
 * Revision 1.45  1994/11/29  17:31:06  doug
 * support small worlds
 *
 * Revision 1.44  1994/11/16  21:51:14  jmc
 * Made the ToggleButton indicators a little bigger and yellow.
 *
 * Revision 1.44  1994/11/16  21:51:14  jmc
 * Made the ToggleButton indicators a little bigger and yellow.
 *
 * Revision 1.43  1994/11/11  22:38:58  jmc
 * Added callbacks to prevent min_obs_r > max_obs_r and vice versa in
 * configuration menu.  Also changed min_obs_scale to min_obs_r_scale
 * and max_obs_scale to max_obs_r_scale.
 *
 * Revision 1.42  1994/11/11  21:25:27  jmc
 * Removed clear_robots from config_clear_cb.  This was really
 * screwing things up if called before any robots were created.  Not
 * sure why.  Fixed a syntax boo-boo from last edit.
 *
 * Revision 1.41  1994/11/11  19:12:17  jmc
 * Changed apply and ok callbacks for configuration panel for the
 * later to call the former (eliminate duplicate code).
 *
 * Revision 1.40  1994/11/11  16:59:19  jmc
 * Cleaned up minor problems (unused variables, etc) to get rid of
 * make -Wall problems.
 *
 * Revision 1.39  1994/11/11  16:32:49  jmc
 * Renamed world_clear to clear_world.
 *
 * Revision 1.38  1994/11/10  23:29:45  jmc
 * Added the "magnify robot" and "robot magnification" to the scale
 * window.  Added functions to set their values.  Modified scale ok
 * and apply callbacks for the former to call the latter.
 *
 * Revision 1.37  1994/11/09  21:19:23  doug
 * typeo
 *
 * Revision 1.36  1994/11/09  21:05:03  doug
 * support fixed and scaled robot sizes
 *
 * Revision 1.35  1994/11/09  16:38:03  jmc
 * Fixed problem in zooming functions.
 *
 * Revision 1.34  1994/11/08  23:11:58  jmc
 * Implemented zooming functions (zoom_in and zoom_out).
 *
 * Revision 1.33  1994/11/08  17:14:09  doug
 * drawimpact should use red_xorgc
 *
 * Revision 1.32  1994/11/08  16:01:38  doug
 * added drawimpact fnc
 *
 * Revision 1.31  1994/11/07  21:33:58  jmc
 * Implemented new function overlap_obstacle.
 *
 * Revision 1.30  1994/11/07  20:12:19  jmc
 * Added definition of origin_x and origin_y.  Moved here from
 * gt_overlay_yac.y since it is needed in a broader context.
 *
 * Revision 1.29  1994/11/07  17:23:56  jmc
 * Implemented the gt_add_obstacle function.
 *
 * Revision 1.28  1994/11/04  20:19:57  jmc
 * Added draw_scale_ruler to rescale_world.  Tweaked a couple minor
 * things.
 *
 * Revision 1.27  1994/11/03  23:47:22  jmc
 * Tweaked the buttons on the bottom of the configuration panel.
 *
 * Revision 1.26  1994/11/02  23:45:36  jmc
 * Modified gt_randomize_seed and made it global (so it can be called
 * in main).
 *
 * Revision 1.25  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.24  1994/11/02  16:58:44  jmc
 * Added random seed entry, regeneration, and randomization functions
 * to the world configuration panel.
 *
 * Revision 1.23  1994/11/01  16:49:07  jmc
 * Added warning messages about creating too many obstacles (>100) or
 * asking for excessively dense coverage (>20%).
 *
 * Revision 1.22  1994/10/31  19:49:59  jmc
 * Made a new panel for rescaling the world and moved that control
 * from the configure world panel.  Added functions for creating the
 * scaling panel, callbacks, etc.  Split rescaling world
 * functionality from populate_world function.  Converted
 * meter_per_pixel to meters_per_pixel.
 *
 * Revision 1.21  1994/10/28  18:35:29  jmc
 * Minor hacks to try to get the display/redisplay working.  It still
 * isn't right.
 *
 * Revision 1.20  1994/10/28  00:47:52  jmc
 * Removed the guts from apply_cb and made the populate_world
 * function to create the obstacles and redraw them as necessary.
 * Converted apply_cb and okay_cb to use it.  Added gt_draw_db at the
 * end of populate_world to redraw the overlay.  Added several set_*
 * functions for the world configuration parameters.  (Just setting
 * the globals is not adequate since the world configuration window
 * is not updated.)  Worked on gt_create_configuration_panel to make
 * sure the scales use the new constants defined in gt_world.h.
 *
 * Converted DrawPolygon to draw_polygon which uses the control
 * measure.  Similarly DrawPolyline->draw_polyline.  Added new draw_*
 * functions to draw each of the control measures.  Converted old
 * routines to draw some of the control measures (DrawAxis, etc) to
 * new draw_* routines and deleted the old ones.
 *
 * Revision 1.19  1994/10/27  18:58:58  jmc
 * Added constants for the legal values of the variables controlled
 * by sliders.  Converted ppf_scale to mpp_scale (meters per pixel).
 *
 * Revision 1.18  1994/10/26  23:18:15  doug
 * added slider bars and converted to meters
 *
 * Revision 1.17  1994/10/25  20:04:17  jmc
 * Added radius argument to DrawPolygon for the radius in the case
 * when a polyline isn't specified and a circle is drawn.  Note that
 * this moved the default radius to gt_overlay_yac.y and
 * gt_command.c.
 *
 * Revision 1.16  1994/10/18  21:55:57  jmc
 * Add prototypes for atan2 and floor.
 *
 * Revision 1.15  1994/10/11  20:12:50  doug
 * added tcxclose call
 *
 * Revision 1.14  1994/08/22  21:37:18  jmc
 * Removed the show_trails button and moved that functionality to the
 * Options menu (in console.c).
 *
 * Revision 1.13  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.12  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.11  1994/08/03  19:49:47  jmc
 * Cleaned up code format in a few spots (function prototypes, long
 * lines, etc).
 *
 * Revision 1.10  1994/07/27  19:17:42  jmc
 * Tweaked the initial values and slider maximums for configure
 * world.
 *
 * Revision 1.9  1994/07/25  21:26:05  jmc
 * Hardcoded default screen positions for configure_panel.
 *
 * Revision 1.8  1994/07/19  21:26:08  jmc
 * Removed unused variables for clean compiling.
 *
 * Revision 1.7  1994/07/19  15:58:59  jmc
 * Removed setting background to grey80; should do this in the
 * .Xdefaults file.
 *
 * Revision 1.6  1994/07/18  22:12:07  jmc
 * Changed layout of upper form to have sliders one over the other to
 * make longer sliders so the user can have more precision.  Added
 * [Clear] button.  Added [Apply] button to lower form and modified
 * [Ok] appropriately.  Converted all colors to grey80.
 *
 * Revision 1.5  1994/07/18  20:52:03  jmc
 * Hacked gt_create_configuration_panel to use local static variables
 * instead of temporary variables.  Converted [Ok] and [Cancel]
 * functions to popdown the panel instead of destroying it.  Made
 * callback functions static.  Added grey background color to entire
 * configuration panel.  Implemented function
 * gt_popup_configuration_panel.
 *
 * Revision 1.4  1994/07/18  19:21:43  jmc
 * Added extern "C" for system includes
 *
 * Revision 1.3  1994/07/14  20:36:52  jmc
 * Added Xt function prototypes to XtAddCallBack's
 *
 * Revision 1.2  1994/07/14  13:26:58  jmc
 * Added RCS automatic id and log strings
 **********************************************************************/
