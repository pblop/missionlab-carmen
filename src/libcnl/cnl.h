#ifndef CNL_H
#define CNL_H
/****************************************************************************
*                                                                           *
*                            cnl.h                                          *
*                                                                           *
*   cnl.h - define global stuff                                             *
*                                                                           *
*   Written by: Douglas C. MacKenzie                                        *
*                                                                           *
*                                                                           *
*   Copyright 1995 - 2005, Georgia Tech Research Corporation                *
*   Atlanta, Georgia  30332-0415                                            *
*   ALL RIGHTS RESERVED, See file COPYRIGHT for details.                    *
*                                                                           *
****************************************************************************/

/* $Id: cnl.h,v 1.1.1.1 2006/07/12 13:37:59 endo Exp $ */


#include <stdio.h>
#include <math.h>
#include <string>

#include "gt_message_types.h"
#include "gt_std.h"
#include "cfgedit_common.h"

using std::string;

const string CNL_EMPTY_STRING = "";

// Used by the object filtering code
// orange
#define Mines                   (1<<0)
#define Mine                    (1<<0)
#define Cans                    (1<<0)
#define Can                     (1<<0)
#define Biological_Agents       (1<<0)
#define Biological_Agent        (1<<0)
#define Biohazard               (1<<0)
#define Victim                  (1<<0)
#define Victims                 (1<<0)
#define Hostage                 (1<<0)
#define Hostages                (1<<0)

// red
#define Enemy_Robots            (1<<1)
#define Enemy_Robot             (1<<1)
#define Enemies                 (1<<1)
#define Enemy                   (1<<1)
#define Red_Marker              (1<<1)
#define Red_Flag                (1<<1)
#define Red_Flags               (1<<1)
#define Biohazard_Sign          (1<<1)
#define Red_Robot               (1<<1)

// purple
#define Flags                   (1<<2)
#define Flag                    (1<<2)
#define Possible_Victim         (1<<2)
#define Possible_Victims        (1<<2)

// green
#define EOD_Areas               (1<<3)
#define EOD_Area                (1<<3)
#define Baskets                 (1<<3)
#define Basket                  (1<<3)
#define Green_Marker            (1<<3)
#define Chemical_Agents         (1<<3)
#define Chemical_Agent          (1<<3)
#define Biohazard_Container     (1<<3)
#define Green_Robot             (1<<3)
#define Illegal_Weapons         (1<<3)

// black
#define Rocks                   (1<<4)
#define Rock                    (1<<4)
#define Safe_Material           (1<<4)

// DarkSeaGreen
#define Trees_and_Shrubs        (1<<5)
#define Tree_or_Shrub           (1<<5)

// white
#define Home_Base               (1<<6)
#define Start_Place             (1<<6)
#define StartPlace              (1<<6)

// brown
#define Unknown_Objects         (1<<7)
#define Unknown_Object          (1<<7)
#define Possible_Biohazard      (1<<7)

// lightblue
#define Hiding_Places           (1<<8)
#define Hiding_Place            (1<<8)
#define Radioactive_Materials   (1<<8)
#define Radioactive_Material    (1<<8)
#define Blue_Marker             (1<<8)
#define Possible_Hostage        (1<<8)
#define Possible_Hostages       (1<<8)

// yellow
// Note: Avoid using this color. Robot may confuse
// with other colors such as blue.
#define Yellow_Marker           (1<<9)
#define Banana                  (1<<9)
#define Yellow_Robot            (1<<9)

// blue
#define Friendly_Robot          (1<<10)
#define Friendly_Robots         (1<<10)
#define Blue_Robot              (1<<10)

#define Check_Theta_Only 0
#define Check_Psi_Only 1
#define Check_Both_Theta_And_Psi 2

#define Right                   0
#define Left                    1
#define Both                    2
#define Both_Left_and_Right     2
#define Facing                  3
#define Opposite                4

// Used by CSB
#define Comm_Recovery		0
#define Comm_Preserve		1
#define Internalized_Plan 	2
#define CSB_Average 		3
#define CSB_Greedy			4

// Used by Alert state
#define Send_Email              1
#define NO_Email                0
#define Attach_Image            1
#define NO_Image                0

// Used by ENTER_ROOM.cnl
#define ENTER_UNMARKED_ONLY     1
#define ENTER_ANY               2
// Used by ENTER_DOORWAY.cnl
#define THROUGH_UNMARKED_ONLY     1
#define THROUGH_ANY               2


// Used by DOORWAY_ULTRASOUND.cnl
#define DETECT_UNMARKED_ONLY    1
#define DETECT_ANY              2

// Used by the sigsense code
#define Danger			((int)'d')
#define Safe			((int)'s')
#define Abort			((int)'a')

// Used by COMPUTE_FORM_POSITION
#define NoFormation		"NO_FORMATION"
#define Sweep			"SWEEP"
#define Line			"LINE"
#define LineLeft		"LINE_LEFT"
#define LineRight		"LINE_RIGHT"
#define Diamond			"DIAMOND"
#define Wedge			"WEDGE"
#define WedgeLeft		"WEDGE_LEFT"
#define WedgeRight		"WEDGE_RIGHT"
#define Column			"COLUMN"

//Used by RESET_WORLD
#define USE_NEW_MAP             1
#define DEFAULT_MAP             0

// Used to set camera tracking mode
#define None                    0
#define Center                  1
#define Sweep_X                 2
#define Sweep_X_Center          2
#define Sweep_Y                 3
#define Sweep_Y_Center          3
#define Sweep_XY                4
#define Largest_Any_Color       5
#define Largest_A               6
#define Largest_B               7
#define Largest_C               8
#define Closest_Laser           9
#define Sweep_X_High            10
#define Sweep_X_Low             11
#define Sweep_Y_Left            12
#define Sweep_Y_Right           13

// Used by StartSubMission state
#define DEPLOY_BY_GOTO 0
#define DEPLOY_BY_LOCALIZE 1

// Used by SubMissionReady trigger
#define DEPLOY_FIRST 0
#define EXECUTE_IMMEDIATELY 1

// Used by NOTIFY_ROBOTS_OBJECT_LOCATION
const string NOTIFY_OBJECT_LOC_X_KEY = "NOTIFY_OBJECT_LOC_X_KEY";
const string NOTIFY_OBJECT_LOC_Y_KEY = "NOTIFY_OBJECT_LOC_Y_KEY";
const string NOTIFY_OBJECT_LOC_Z_KEY = "NOTIFY_OBJECT_LOC_Z_KEY";
const string NOTIFY_OBJECT_LOC_DONE_KEY = "NOTIFY_OBJECT_LOC_DONE_KEY";

// Used by CNP_SAVE_TARGET_LOCATION
const string CNP_TARGET_LOC_X_KEY = "CNP_TARGET_LOC_X_KEY";
const string CNP_TARGET_LOC_Y_KEY = "CNP_TARGET_LOC_Y_KEY";
const string CNP_TARGET_LOC_Z_KEY = "CNP_TARGET_LOC_Z_KEY";
const string CNP_TARGET_LOC_DONE_KEY = "CNP_TARGET_LOC_DONE_KEY";

// Used by CNP_SAVE_TARGET_VELOCITY
const string CNP_TARGET_VEL_X_KEY = "CNP_TARGET_VEL_X_KEY";
const string CNP_TARGET_VEL_Y_KEY = "CNP_TARGET_VEL_Y_KEY";
const string CNP_TARGET_VEL_Z_KEY = "CNP_TARGET_VEL_Z_KEY";
const string CNP_TARGET_VEL_DONE_KEY = "CNP_TARGET_VEL_DONE_KEY";

// Used by CNP_SAVE_TARGET_VEHICLE_TYPE
const string CNP_TARGET_VEHICLE_TYPE_KEY = "CNP_TARGET_VEHICLE_TYPE_KEY";

typedef struct ConstraintDatabaseKeyMap_t {
    string constraintName;
    string dataBaseKey;
};

const ConstraintDatabaseKeyMap_t CONSTRAINT_DATABASE_KEY_MAP[] =
{
    {"TARGET_LOCATION_X", CNP_TARGET_LOC_X_KEY},
    {"TARGET_LOCATION_Y", CNP_TARGET_LOC_Y_KEY},
    {"TARGET_LOCATION_Z", CNP_TARGET_LOC_Z_KEY},
    {"TARGET_VELOCITY_X", CNP_TARGET_VEL_X_KEY},
    {"TARGET_VELOCITY_Y", CNP_TARGET_VEL_Y_KEY},
    {"TARGET_VELOCITY_Z", CNP_TARGET_VEL_Z_KEY},
    {"TARGET_VEHICLE_TYPE", CNP_TARGET_VEHICLE_TYPE_KEY},
    {CNL_EMPTY_STRING, CNL_EMPTY_STRING}
};

// Used by CNP_TASK_COMPLETION_NOTIFIED trigger
typedef enum CNPTaskSelection_t {
    CHECK_WON_CNP_TASK,
    CHECK_SPECIFIC_CNP_TASK
};

typedef bool boolean;

#define MAX_ROBOT_READINGS 10
struct Robots
{
    int    cnt;
    int    id[ MAX_ROBOT_READINGS ];
    Vector v[ MAX_ROBOT_READINGS ];
};


#define MAX_UNIT_SIZE 64
struct gt_Unit_Members
{
    int unit_size;
    int unit[MAX_UNIT_SIZE];
};

struct CNLString_t
{
    char* val;
};

struct Reinforcer
{
    int state;
    int action;
    double value;
};

extern int robot_id;
extern int debug;
extern gt_Command *command;
extern char* gt_behavior_name[];
extern bool drawing_vector_field;

#endif



/**********************************************************************
 * $Log: cnl.h,v $
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.10  2006/05/02 04:17:18  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.9  2006/03/01 09:27:39  endo
 * Check-in for Type-I Intercept Experiment.
 *
 * Revision 1.8  2006/02/19 22:41:33  endo
 * CNP_SAVE_TARGET_LOCATION stuff.
 *
 * Revision 1.7  2006/02/19 17:54:53  endo
 * Experiment related modifications
 *
 * Revision 1.6  2006/01/10 06:16:04  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.5  2005/08/09 19:14:35  endo
 * Things improved for the August demo.
 *
 * Revision 1.4  2005/07/23 05:57:52  endo
 * 3D visualization improved.
 *
 * Revision 1.3  2005/03/23 07:36:54  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/08 00:07:28  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:01  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.43  2003/04/06 09:25:40  endo
 * Updated for CBR Wizard Prototype II.
 *
 * Revision 1.42  2002/10/31 21:15:08  ebeowulf
 * Added the RESET_WORLD behavior
 *
 * Revision 1.41  2002/07/02 20:28:16  blee
 * added MOVE_TO_GUARDED
 *
 * Revision 1.40  2001/12/22 15:56:36  endo
 * RH 7.1 porting.
 *
 * Revision 1.39  2001/09/24 21:30:18  ebeowulf
 * Added QLEARN operator, and Reinforcer type
 *
 * Revision 1.38  2000/11/20 19:34:26  sgorbiss
 * Add visualization of line-of-sight communication
 *
 * Revision 1.37  2000/09/19 09:57:50  endo
 * More colors and camera modes added.
 *
 * Revision 1.36  2000/07/13 19:05:17  conrad
 * added camera tracking
 *
 * Revision 1.35  2000/04/25 07:19:50  endo
 * blue added.
 *
 * Revision 1.34  2000/04/19 21:15:53  jdiaz
 * added some constants
 *
 * Revision 1.33  2000/04/16 15:41:35  endo
 * New items assigned to colors.
 *
 * Revision 1.32  2000/03/30 01:20:03  endo
 * New names added for the colors.
 *
 * Revision 1.31  2000/02/29 00:12:21  jdiaz
 * constants for enter room, leave room, and detect doorway added
 *
 * Revision 1.30  2000/02/18 02:26:41  endo
 * Some constants for Alert FSA was declared.
 *
 * Revision 1.29  1999/12/16 23:33:14  mjcramer
 * rh6 port
 *
 * Revision 1.25  1999/07/16 20:04:34  endo
 * Left & Right restored.
 *
 * Revision 1.24  1999/07/03 01:20:56  mjcramer
 * changed objects
 *
 * Revision 1.23  1999/06/07 04:02:25  jdiaz
 * added LEFT and RIGHT constants
 *
 * Revision 1.22  1996/06/21 22:48:40  doug
 * added cans and baskets as objects
 *
 * Revision 1.21  1996/06/17  22:58:23  doug
 * *** empty log message ***
 *
 * Revision 1.20  1996/05/14  23:17:31  doug
 * *** empty log message ***
 *
 * Revision 1.19  1996/02/26  05:01:53  doug
 * *** empty log message ***
 *
 * Revision 1.18  1996/02/21  15:44:22  doug
 * *** empty log message ***
 *
 * Revision 1.17  1996/02/08  19:22:52  doug
 * *** empty log message ***
 *
 * Revision 1.16  1995/09/08  14:33:25  doug
 * *** empty log message ***
 *
 * Revision 1.15  1995/06/28  21:17:59  jmc
 * Added RCS id and log strings.
 **********************************************************************/
