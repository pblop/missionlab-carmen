/**********************************************************************
 **                                                                  **
 **                       simulation_server.c                        **
 **                                                                  **
 ** Written by: Douglas C. MacKenzie                                 **
 **                                                                  **
 ** Copyright 1995 - 2007 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: simulation_server.c,v 1.2 2008/07/16 21:16:09 endo Exp $ */

#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <string>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/Intrinsic.h>

#include "ipt/ipt.h"

#include "shape.hpp"
#include "robot.hpp"
#include "list.hpp"
#include "console.h"
#include "gt_simulation.h"
#include "gt_sim.h"
#include "gt_world.h"
#include "gt_create_world.h"
#include "gt_console_db.h"
#include "console_side_com.h"
#include "draw.h"
#include "gt_scale.h"
#include "lineutils.h"
#include "VectorField.h"
#include "gt_playback.h"
#include "3d_vis_wid.h"
#include "cnp_server_manager.h"
#include "mission_design.h"
#include "mlab_cnp.h"
#include "jbox_mlab.h"
#include "draw_military.h"

bool gt_draw_robot_military_style = true;

using std::string;

extern obs_t* obs;
extern int    num_obs;

obs_t* g_suBackupObs = NULL;
int    g_iBackupObsCount = 0;

// This is the list of line objects that will be used for
// detecting obstacles. This list is loaded from the overlay
// file and stored in the console database in gt_console_db.c
extern lineRec2d_t* gConsoleDBLineList;

// List of doorway objects as line segments. Loaded from the
// overlay and stored in the console database in gt_console_db.c
extern door_t* gConsoleDBDoorwayList;

// List of room objects, loaded from the overlay and stored in
// the console database.
extern room_t* gConsoleDBRoomList;

// List of hallway objects, loaded from the overlay and stored in
// the console database.
extern hallwayRec_t* gConsoleDBHallwayList;

int robot_debug_messages = false;
int debug_scheduler = false;
int report_state_messages = false; // If true, robot will report current state.
int log_robot_data = false;        // If true, data logging will be enabled.
int highlight_repelling_obstacles = false;
int show_movement_vectors = false;
int show_obstacles = false;
int erase_obstacles = true;
int laser_linear_mode = false;
int laser_normal_mode = true;
int laser_connected_mode = true;
int sonar_arc_mode = true;
int sonar_point_mode = false;
int LOS_mode = false;
int uf = false;
int ch = 0;
int show_colors = true;
int learning_momentum_enabled = true;
int save_learning_momentum_weights = false;
int save_sensor_readings = false;
double cycle_duration = 1.0;   // seconds per cycle

GTList<shape*> object_list;    // List of all features in the environment
int shape::last_shape_id = 0;  // See shape.hpp

ClVectorField g_objVectorField;

const string STRING_EXTRA_ROBOT = "extra_robot";

// ------------------------------------------------------------------
// Variables used for 3 dimensional representations and visualization
// ------------------------------------------------------------------
unsigned char *CurHeightMap = NULL;
char *heightmap_name = NULL;
int heightmap_x = 0;
int heightmap_y = 0;
TerrainMap *Terrain = NULL;
ObstacleHandler *Obs_Handler = NULL;
RobotHandler *Rob_Handler = NULL;
float ScaleValue = .15f;
float Eye[3];
float Rotate[3];
int *obshighpts = NULL;
extern SLList *Wall_List;

const string RCTABLE_3D_MODEL_DIR_STRING = "Vis3DModelDir";
const float UGV_OFFSET = 0; // In pixels
const float UXV_OFFSET = 0; // In pixels
const float UUV_OFFSET = -8; // In pixels
const float USV_OFFSET = 0; // In pixels
const float UAV_OFFSET = 64; // In pixels
//const int DEFAULT_MOBILITY_TYPE = UXV;
const int DEFAULT_MOBILITY_TYPE = UAV;
const int MAX_ROBOT_NUMBER = 10000;

T_robot_state* robots = NULL;
static T_robot_state* cur_robot_rec = NULL;

//------------------------------------------------------------------
void clear_robots()
{
    send_robots_suicide_msg();

    gt_delete(BROADCAST_ROBOTID, KEYPRESS);
    gt_delete(BROADCAST_ROBOTID, LEFT_MOUSE_CLICK);

    robots = NULL;

    clear_communications();

    erase_halos();
    erase_movement_vectors();
}

//------------------------------------------------------------------
static T_robot_state* find_robot_rec(int robot_id)
{
    T_robot_state* cur;
    int i;

    cur = robots;
    i = 0;
    while (cur)
    {
        if (cur->robot_id == robot_id)
        {
            break;
        }

        cur = cur->next;

        if (i++ > MAX_ROBOT_NUMBER)
        {
            fprintf(stderr, "find_robot_rec(%d): loop in robot state records\n", robot_id);
            cur->next = NULL;
            return NULL;
        }
    }

    return cur;
}

//------------------------------------------------------------------
static T_robot_state* find_robot_rec(shape* object_ptr)
{
    T_robot_state* cur;
    int i;

    cur = robots;
    i = 0;
    while (cur)
    {
        if (cur->object_ptr == object_ptr)
        {
            break;
        }

        cur = cur->next;

        if (i++ > 10000)
        {
            abort();
            return NULL;
        }
    }

    return cur;
}


//------------------------------------------------------------------
int find_first_robot_rec(void)
{
    int id;

    cur_robot_rec = robots;

    if (cur_robot_rec == NULL)
    {
        id = -1;
    }
    else
    {
        id = cur_robot_rec->robot_id;
    }

    return id;
}

//------------------------------------------------------------------
int find_next_robot_rec(void)
{
    int id;

    if (cur_robot_rec == NULL)
    {
        id = -1;
    }
    else
    {
        cur_robot_rec = cur_robot_rec->next;

        if (cur_robot_rec == NULL)
        {
            id = -1;
        }
        else
        {
            id = cur_robot_rec->robot_id;
        }
    }

    return id;
}

//------------------------------------------------------------------
int GetFirstRobotID()
{
    return find_first_robot_rec();
}

//------------------------------------------------------------------
int GetNextRobotID(int iID)
{
    int iCurrentID = find_first_robot_rec();

    // get the passed in ID
    while ((iCurrentID != iID) && (iCurrentID >= 0))
    {
        iCurrentID = find_next_robot_rec();
    }

    iCurrentID = find_next_robot_rec(); // get the ID after the passed in one

    // just in case the passed in ID wasn't found...
    if (iCurrentID < 0)
    {
        iCurrentID = find_first_robot_rec();
    }

    return iCurrentID;
}

//------------------------------------------------------------------
bool GetRobotLocation(int iID, Vector& suLocation)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        suLocation = psuRobot->loc;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool SetRobotLocation(int iID, const Vector& suLocation)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        psuRobot->loc = suLocation;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool SetDrawingVectorField(int iID, bool bDrawing)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        psuRobot->drawing_vector_field = bDrawing;
        EnablePauseControls(!bDrawing);
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool SetRunType(int iID, int newRunType, int &oldRunType)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        oldRunType = psuRobot->run_type;
        psuRobot->run_type = newRunType;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool SetRunType(int iID, int newRunType)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        psuRobot->run_type = newRunType;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool ResetStepTaken(int iID)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        psuRobot->step_taken = false;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------
bool WaitForRobotStep(int iID)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        while (!psuRobot->step_taken)
        {
            gt_simulation_loop();
        }
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------

bool ResetSensorsReceived(int iID)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        psuRobot->sensors_received = false;
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------

bool WaitForSensors(int iID)
{
    T_robot_state* psuRobot = find_robot_rec(iID);
    if (psuRobot != NULL)
    {
        while (!psuRobot->sensors_received)
        {
            gt_simulation_loop();
        }
    }

    return (psuRobot != NULL);
}

//------------------------------------------------------------------

void RestoreReadingsFromBackup()
{
    // destroy the temp obs
    if (obs != NULL)
    {
        delete [] obs;
    }

    // restore the obs from backup
    num_obs = g_iBackupObsCount;
    obs = g_suBackupObs;

    g_suBackupObs = NULL;
    g_iBackupObsCount = 0;
}

/****************************************************************************
*                                                                           *
* add_new_robot(int robot_id)                                               *
*                                                                           *
****************************************************************************/
//static T_robot_state* add_new_robot(int request_id)
T_robot_state* add_new_robot(int request_id)
{
    int robot_id;
    T_robot_state* cur = NULL;

    if (debug)
    {
        fprintf(stderr, "add_new_robot(%d)\n", request_id);
    }

    if (request_id == -1)
    {
        // don't care so give them the next unused one
        robot_id = 0;
        while (((cur = find_robot_rec(robot_id))) != NULL && cur->alive)
        {
            robot_id ++;
        }
    }
    else
    {
        robot_id = request_id;
        if (((cur = find_robot_rec(robot_id)) != NULL) && cur->alive)
        {
            fprintf(stderr, "add_new_robot(%d): id is already in use\n", robot_id);

            // already in use. too bad
            return NULL;
        }
    }

    if (cur == NULL)
    {
        cur = (T_robot_state*) calloc(sizeof(T_robot_state), 1);
        if (cur == NULL)
        {
            fprintf(stderr, "server: out of memory in new_robot\n");
            exit(100);
        }

        // set the robot id
        cur->robot_id = robot_id;

        // set the default mobility type
        cur->mobility_type = DEFAULT_MOBILITY_TYPE;
        cur->vis = true;

        // add to the chain
        cur->next = robots;
        robots = cur;
    }

    // Set defaults
    cur->robot_type = HOLONOMIC;
    cur->run_type = SIMULATION;
    cur->drawing_vector_field = false;

    cur->LOS_not = false;
    cur->pcount = 0;

    // Remember the initial CNP robot constraints.
    if ((gMlabCNP != NULL) && (gMMD != NULL))
    {
        gMlabCNP->saveInitCNPRobotConstraints(
            robot_id,
            gMMD->getCNPRobotConstraints(robot_id));
    }

    return cur;
}

//-----------------------------------------------------------------------
int start_robot(
    char *machine,
    char *file,
    int robot_id,
    int robot_debug,
    int report_state,
    double start_x,
    double start_y,
    char *color)
{
    T_robot_state* cur;
    rc_chain *rsh_user_name_ptr = NULL;
    rc_chain *laserfit_machine_name_ptr = NULL;
    char *prog = NULL;
    char  buf[1024];
    char *argv[1024];
    char *laserfit_machine_name = "columbus";
    char *rsh_user_name = NULL;
    int   p = 0;
    int   i;
    int pid;
    int use_laserfit_machine_name = false;
    int numAllowedRobotJBoxIDs = 0;
    vector<int> allowedRobotJBoxIDs;
    extern char ipt_home[];
    extern char console_ipt_port[];

    if (debug)
    {
        fprintf(
            stderr,
            "start_robot(%s, %s, %d, %d, %d, %.1f, %.1f)\n",
            machine,
            file,
            robot_id,
            robot_debug,
            report_state,
            start_x,
            start_y);
    }

    cur = add_new_robot(robot_id);
    if (cur == NULL)
    {
        fprintf(stderr, "Internal Error: Robot %d already exists\n", robot_id);
        return -1;
    }

    // Check to see if the name of the laserfit machine should be sent to robot.
    laserfit_machine_name_ptr = (rc_chain*) rc_table.get("LaserFitMachineName");
    if (laserfit_machine_name_ptr != NULL)
    {
        use_laserfit_machine_name = true;
        laserfit_machine_name_ptr->first(&laserfit_machine_name);
    }

    // Get the list of allowed robot JBoxes.
    rc_chain *allowedRobotJBoxList = (rc_chain *)rc_table.get("AllowedRobotJBoxIDs");
    if (allowedRobotJBoxList)
    {
        char *str;
        void *p = allowedRobotJBoxList->first(&str);

        while (p)
        {
            allowedRobotJBoxIDs.push_back(atoi(str));
            numAllowedRobotJBoxIDs++;
            p = allowedRobotJBoxList->next(&str, p);
        }
    }

    // Create a new object and add it to the list
    cur->object_ptr = new robot(start_x, start_y, 0.2, color);
    cur->col = strdup(color);
    object_list.append(cur->object_ptr);

    if (gPlayBackMode)
    {
        return 0;
    }


    // run a remote shell if machine name is specified
    if (machine && (machine[0] != '\0'))
    {
        rsh_user_name = getenv("USER");
        rsh_user_name_ptr = (rc_chain*)rc_table.get("RemoteShellUserName");

        if (rsh_user_name_ptr != NULL)
        {
            rsh_user_name_ptr->first(&rsh_user_name);
        }

        //strcat(buf2, file);
        prog = "rsh";
        argv[p++] = prog;
        argv[p++] = machine;
        argv[p++] = "-l";
        argv[p++] = rsh_user_name;
        argv[p++] = "-n";
        argv[p++] = file;
    }
    else
    {
    	prog = (char *)malloc(strlen(file)+3);
        strcpy(prog, "./");
        strcat(prog, file);
        argv[p++] = file;
    }

    argv[p++] = "-x";
    sprintf(buf, "%.1f",start_x);
    argv[p++] = strdup(buf);

    argv[p++] = "-y";
    sprintf(buf, "%.1f",start_y);
    argv[p++] = strdup(buf);

    argv[p++] = "-h";
    sprintf(buf, "%.1f",origin_heading);
    argv[p++] = strdup(buf);

    argv[p++] = "-i";
    sprintf(buf, "%d",robot_id);
    argv[p++] = strdup(buf);

    argv[p++] = "-c";
    argv[p++] = console_ipt_port;

    // The list of JBox IDs for the robots that are allowed to
    // team up with.
    if (numAllowedRobotJBoxIDs)
    {
        argv[p++] = "-J";
        sprintf(buf, "%d", allowedRobotJBoxIDs[0]);

        for (i = 1; i < numAllowedRobotJBoxIDs; i++)
        {
            sprintf(buf, "%s:%d", buf, allowedRobotJBoxIDs[i]);
        }
        argv[p++] = strdup(buf);
    }

    if (use_laserfit_machine_name)
    {
        argv[p++] = "-l";
        argv[p++] = laserfit_machine_name;
    }

    if (robot_debug)
    {
        argv[p++] = "-v";
    }

    // This will enable "Report Current State" upon the
    // robot's birth.
    if (report_state)
    {
        argv[p++] = "-S";
    }

    // This will enable "Log Robot Data" upon the
    // robot's birth.
    if (log_robot_data)
    {
        argv[p++] = "-L";
    }

    // This will cut off the robot feedback upon the
    // robot's birth.
    if (gStopRobotFeedback)
    {
        argv[p++] = "-f";
        argv[p++] = "0";
    }

    // Turn on runtime-CNP
    if (gMlabCNP != NULL)
    {
        argv[p++] = "-N";
    }

    // This will enable the "Save Learning Momentum Weights"
    // upon the robot's birth
    if (save_learning_momentum_weights)
    {
        argv[p++] = "-m";
    }

    // This will enable the "Save Sonar Readings" upon
    // the robot's birth
    if (save_sensor_readings)
    {
        argv[p++] = "-r";
    }

    argv[p++] = "-t";
    argv[p++] = ipt_home;

    if (debug_scheduler)
    {
        argv[p++] = "-cthread_debug";
        argv[p++] = "calls";
    }

    argv[p] = NULL;

    // ENDO - FC5
    // Since Fedora Core 5, the way longjmp() function is called has changed, and
    // because of the change, the robot executable (Cthreads) segfaults.
    // By setting the environmental variable LD_POINTER_GUARD to be 0, we work
    // around the problem. However, the real fix should be replacing Cthreads with
    // pthreads.
    //setenv("LD_POINTER_GUARD", "0", 1);

    printf("Attempting to execute");
    for (i = 0; i < p; i++)
    {
        printf(" %s", argv[i]);
    }
    printf("\n");

    if (debug || robot_debug)
    {
        fprintf(stderr, "execvp(%s ", prog);
        for (i = 0; i < p; i++)
        {
            fprintf(stderr, "%s%s", argv[i], (i < (p - 1)) ? " " : ")\n");
        }
    }

    if ((pid = fork()) == 0)
    {
        // in child
        if (execvp(prog, argv))
        {
            perror("Unable to exec the robot");
        }

        exit(2);
    }

    if (pid == -1)
    {
        perror("Unable to fork");
        return 1;
    }

    return 0;
}


//------------------------------------------------------------------

void erase_robot(T_robot_state* bot)
{
    // erase the robot
    if (bot->alive && bot->is_up)
    {
        if (bot->vis)
        {
            if (gt_draw_robot_military_style)
            {
                Draw_MilitaryStyle(-1, -1, bot->loc.x, bot->loc.y, bot->mobility_type, bot->object_ptr->gc(), false, bot->col);
            }
            else
            {
                DrawPath(0, 0, 0, bot->loc.x, bot->loc.y, bot->heading, bot->object_ptr->gc(), false);
            }
        }
        bot->is_up = false;
    }
}

//------------------------------------------------------------------

void draw_robot(T_robot_state* bot)
{
    if (gt_show_3d_vis == false)
    {
        // draw the robot
        if (bot->alive && !bot->is_up)
        {
            if (bot->vis) {
                if (gt_draw_robot_military_style) {
                    Draw_MilitaryStyle(-1, -1, bot->loc.x, bot->loc.y, bot->mobility_type, bot->object_ptr->gc(), false, bot->col);
                    bot->is_up = true;
                }
                else {

                    DrawPath(0, 0, 0, bot->loc.x, bot->loc.y, bot->heading, bot->object_ptr->gc(), false);
                    bot->is_up = true;
                }
            }
        }
    }
}

/****************************************************************************
*                                                                           *
* draw_robots								    *
*                                                                           *
****************************************************************************/

void draw_robots()
{
    T_robot_state* cur;

    cur = robots;
    while (cur)
    {
        draw_robot(cur);
        cur = cur->next;
    }
}


/****************************************************************************
*                                                                           *
* erase_robots								    *
*                                                                           *
****************************************************************************/

void erase_robots()
{
    T_robot_state* cur;

    erase_movement_vectors();

    cur = robots;
    while (cur)
    {
        erase_robot(cur);
        cur = cur->next;
    }
}

/****************************************************************************
*                                                                           *
* clear_objects()                                                           *
*                                                                           *
****************************************************************************/

void clear_objects()
{
    while (!object_list.isempty())
    {
        // Remove the object from the list
        delete object_list.get();
    }
}

/****************************************************************************
*                                                                           *
* delete_robot_rec(int robot_id)                                            *
*                                                                           *
****************************************************************************/

extern int auto_run;

void delete_robot_rec(int robot_id)
{
    T_robot_state* cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in delete_robot_rec\n", robot_id);
        return;
    }
    else
    {
        if (!gt_show_3d_vis)
        {
            erase_robot(cur);
        }

        // Remove the object from the list
        if (cur->object_ptr)
        {
            object_list.remove(cur->object_ptr);
            delete cur->object_ptr;
            cur->object_ptr = NULL;
        }
    }

    // handle first rec
    if (robots->robot_id == robot_id)
    {
        robots = robots->next;
    }
    else
    {
        // handle the rest
        cur = robots;
        while (cur->next)
        {
            if (cur->next->robot_id == robot_id)
            {
                cur->next = cur->next->next;
                break;
            }
            cur = cur->next;
        }
    }

    erase_halo(robot_id);
}

//------------------------------------------------------------------

IPConnection* sim_get_com_link(int robot_id)
{
    T_robot_state* cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        if (debug)
        {
            fprintf(stderr, "server: bad robot id %d in sim_get_com_link\n", robot_id);
        }
        return NULL;
    }

    if (cur->alive == false)
    {
        return NULL;
    }

    return cur->com_link;
}

//------------------------------------------------------------------

int sim_robot_birth(int request_id, gt_Point start_loc, float heading, IPConnection* com_link)
{
    int robot_id;
    T_robot_state* cur;

    if (debug)
    {
        fprintf(stderr, "sim_robot_birth(%d, <%.1f %.1f>)\n",
                 request_id, start_loc.x, start_loc.y);
    }

    robot_id = request_id;
    if ((cur = add_new_robot(robot_id)) == NULL)
    {
        // already in use. too bad
        fprintf(stderr, "In sim_robot_birth: add_new_robot(%d) failed\n", request_id);
        return -1;
    }

    cur->loc.x = start_loc.x;
    cur->loc.y = start_loc.y;
    cur->loc.z = 0;
    cur->lastLoc = cur->loc;
    cur->heading = heading;

    cur->steer_angle = 0;

    cur->com_link = com_link;

    cur->alive = true;

    // draw the robot
    if (!gt_show_3d_vis)
    {
        draw_robot(cur);
    }

    return robot_id;
}

//------------------------------------------------------------------
void sim_notify_config(int robot_id, int robot_type, int run_type)
{
    T_robot_state* cur;

    if (debug)
    {
        fprintf(stderr, "sim_notify_config(%d, %d %d)\n", robot_id, robot_type, run_type);
    }

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_notify_config\n", robot_id);
        return;
    }

    cur->robot_type = robot_type;
    cur->run_type = run_type;
}

//------------------------------------------------------------------

shape* id_to_object(int id)
{
    shape* p;
    void* pos;

    if ((pos = object_list.first(&p)))
    {
        do
        {
            if (p->id_ == id)
            {
                return p;
            }
        } while ((pos = object_list.next(&p, pos)));
    }

    return NULL;
}

//------------------------------------------------------------------

shape* object_robot_is_carrying(shape* robot)
{
    shape* p;
    void* pos;

    if ((pos = object_list.first(&p)))
    {
        do
        {
            if (p->bound_to_ == robot)
            {
                return p;
            }
        } while ((pos = object_list.next(&p, pos)));
    }

    return NULL;
}


//------------------------------------------------------------------


void sim_pickup(int robot_id, int object_id)
{
    T_robot_state *cur = NULL;
    const bool DEBUG_SIM_PICKUP = false;

    if (debug || DEBUG_SIM_PICKUP)
    {
        fprintf(stderr, "sim_pickup(%d, %d)\n", robot_id, object_id);
    }

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_pickup\n", robot_id);
        return;
    }

    shape* object = id_to_object(object_id);

    if (object)
    {
        if (debug || DEBUG_SIM_PICKUP)
        {
            fprintf(stderr, "   trying to pick up the object\n");
        }

        if (!gt_show_3d_vis)
        {
            erase_robot(cur);
        }

        if (object->is_robot())
        {
            T_robot_state* enemy;
            if ((enemy = find_robot_rec(object)) == NULL)
            {
                warn_once("Unable to disable robot");
                return;
            }

            // Remove the object from the list
            if (enemy->object_ptr)
            {
                object_list.remove(enemy->object_ptr);
            }

            send_robot_suicide_msg(enemy->robot_id);
        }
        else
        {
            object->bind(cur->object_ptr);

        }

        if (!gt_show_3d_vis)
        {
            draw_robot(cur);
        }
    }
    else
    {
        if (debug || DEBUG_SIM_PICKUP)
        {
            fprintf(stderr, "Bad object ID to sim_pickup: %d\n", object_id);
        }
    }
}

//-----------------------------------------------------------------------
void sim_probe(int robot_id, int object_id)
{
    T_robot_state* cur;

    if (debug)
    {
        fprintf(stderr, "sim_probe(%d, %d)\n", robot_id, object_id);
    }

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_probe\n", robot_id);
        return;
    }

    shape* object = id_to_object(object_id);
    if (debug)
    {
        cerr << "Object probed: " << object << '\n';
    }

    if (object)
    {
        double dist = object->distance(cur->loc.x, cur->loc.y);

        // ENDO
        //if (dist > MAX_PICKUP_DIST)
        if ((dist > MAX_PICKUP_DIST) || (object->is_robot()))
        {
            // ENDO
            //warn_once("The robot is not close enough to the object to probe it");

            // Safe
            gt_update(robot_id, KEYPRESS, "s");
        }
        else
        {
            // Put info where sigsense can find it.
            if (object->ismovable())
            {
                // Danger
                gt_update(robot_id, KEYPRESS, "d");
            }
            else
            {
                // Safe
                gt_update(robot_id, KEYPRESS, "s");
            }
        }
    }
    else
    {
        if (debug)
        {
            fprintf(stderr, "Bad object ID to sim_pickup: %d\n", object_id);
        }
    }

}

//-----------------------------------------------------------------------
// This function checks the vehicle type of the object if it is a robot.
//-----------------------------------------------------------------------
int sim_check_vehicle_type(int robotID, int objectID)
{
    T_robot_state *cur = NULL;
    shape *object = NULL;
    int i, vehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;

    object = id_to_object(objectID);

    if (object == NULL)
    {
        return vehicleType;
    }

    if (!(object->is_robot()))
    {
        return vehicleType;
    }

    // Now, search the robot.
    cur = robots;
    i = 0;
    while (cur)
    {
        if (cur->object_ptr == object)
        {
            // Found it.
            vehicleType = cur->mobility_type;
            break;
        }

        cur = cur->next;
        i++;

        if (i > MAX_ROBOT_NUMBER)
        {
            fprintf(stderr, "sim_check_vehicle_type(%d): loop in robot state records\n", robotID);
            cur->next = NULL;
            return vehicleType;
        }
    }

    // Special case: UXV
    if (vehicleType == UXV)
    {
        if (Terrain != NULL)
        {
            if (Terrain->Is_Water(cur->loc.x, cur->loc.y))
            {
                vehicleType = USV;
            }
            else
            {
                vehicleType = UGV;
            }
        }
        else
        {
            vehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;
        }
    }

    return vehicleType;
}

//-----------------------------------------------------------------------
// This function checks to see of the target object is trackable by this
// robot.
//-----------------------------------------------------------------------
bool sim_check_target_trackable(int robotID, int objectID)
{
    T_robot_state *cur = NULL;
    shape *object = NULL;
    Vector rloc, tloc;
    int i;
    int targetVehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;
    int myVehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;
    bool untrackable = true;

    // First, find my vehicle type.
    if ((cur = find_robot_rec(robotID)) == NULL)
    {
        fprintf(stderr, "Error: sim_check_target_trackable(): bad robot number %d\n", robotID);
        return untrackable;
    }

    myVehicleType = cur->mobility_type;
    rloc = cur->loc;
    tloc = rloc;

    // Make sure this robot is on a correct terrain.
    if (Terrain != NULL)
    {
        switch (myVehicleType) {

        case UGV:
            if (!(Terrain->Is_OutsideMap(rloc.x, rloc.y)))
            {
                if (Terrain->Is_Water(rloc.x, rloc.y))
                {
                    return true;
                }
            }
            break;

        case UUV:
        case USV:
            if (!(Terrain->Is_OutsideMap(rloc.x, rloc.y)))
            {
                if (!(Terrain->Is_Water(rloc.x, rloc.y)))
                {
                    return true;
                }
            }
            break;

        default:
            break;
        }
    }

    // Next, find the taget vehicle type.
    object = id_to_object(objectID);


    if (object == NULL)
    {
        return true;
    }

    if (!(object->is_robot()))
    {
        return true;
    }

    // Copy the target location.
    object->center(tloc.x, tloc.y);

    // Draw a line between this robot and the target, and see if there is
    // untrackable terrain on its way.
    if (Terrain != NULL)
    {
        switch (myVehicleType) {

        case UGV:
            if (Terrain->Does_CrossWater(rloc.x, rloc.y, tloc.x, tloc.y))
            {
                return true;
            }
            break;

        case UUV:
        case USV:
            if (Terrain->Does_CrossLand(rloc.x, rloc.y, tloc.x, tloc.y))
            {
                return true;
            }
            break;

        default:
            break;
        }
    }

    // Check the target vehicle type.
    cur = robots;
    i = 0;
    while (cur)
    {
        if (cur->object_ptr == object)
        {
            // Found it.
            targetVehicleType = cur->mobility_type;
            break;
        }

        cur = cur->next;
        i++;

        if (i > MAX_ROBOT_NUMBER)
        {
            fprintf(stderr, "sim_check_vehicle_type(%d): loop in robot state records\n", robotID);
            cur->next = NULL;
            return untrackable;
        }
    }

    // Special case: UXV
    if (targetVehicleType == UXV)
    {
        if (Terrain != NULL)
        {
            if (Terrain->Is_Water(cur->loc.x, cur->loc.y))
            {
                targetVehicleType = USV;
            }
            else
            {
                targetVehicleType = UGV;
            }
        }
        else
        {
            targetVehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;
        }
    }

    switch (targetVehicleType) {

    case UGV:
        switch (myVehicleType) {

        case UGV:
        case UAV:
        case SLUAVL:
            untrackable = false;
            break;

        default:
            untrackable = true;
            break;
        }
        break;

    case UUV:
        switch (myVehicleType) {

        case UUV:
            untrackable = false;
            break;

        default:
            untrackable = true;
            break;
        }
        break;

    case USV:
        switch (myVehicleType) {

        case UGV:
            untrackable = true;
            break;

        default:
            untrackable = false;
            break;
        }
        break;

    case UAV:
        switch (myVehicleType) {

        case UAV:
        case SLUAVL:
            untrackable = false;
            break;

        default:
            untrackable = true;
            break;
        }
        break;

    default:
        untrackable = true;
        break;
    }

    return untrackable;
}

/****************************************************************************
*                                                                           *
* sim_alert                                                                 *
*                                                                           *
* When it is called, it pops up the alert message.                          *
*                                                                           *
****************************************************************************/
void sim_alert(char* msg)
{
    if (debug)
    {
        fprintf(stderr, "sim_alert: msg =\n%s\n", msg);
    }

    //warn_userf(msg);
    alert_userf(msg);
}

//------------------------------------------------------------------

void sim_change_color(int robot_id, int object_id, char* new_color)
{
    T_robot_state* cur;

    if (debug)
    {
        fprintf(stderr, "sim_pickup(%d, %d)\n", robot_id, object_id);
    }

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_pickup\n", robot_id);
        return;
    }

    shape* object = id_to_object(object_id);

    if (object)
    {
        if (debug)
        {
            fprintf(stderr, "   trying to pick up the object\n");
        }

        if (!gt_show_3d_vis)
        {
            erase_robot(cur);
        }

        object->set_color(cur->object_ptr, new_color);

        if (!gt_show_3d_vis)
        {
            draw_robot(cur);
        }
    }
    else
    {
        if (debug)
        {
            fprintf(stderr, "Bad object ID to sim_pickup: %d\n", object_id);
        }
    }
}
//------------------------------------------------------------------
void
sim_change_robot_color(int robot_id, char* new_color){
    struct T_robot_state *cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_change_robot_color\n", robot_id);
        return;
    }

    if (!gt_show_3d_vis)
    {
        erase_robot(cur);
    }
    free(cur->col);
    cur->col = strdup(new_color);
    cur->object_ptr->change_color(new_color);

    if (!gt_show_3d_vis)
    {
        draw_robot(cur);
    }
}

//------------------------------------------------------------------

void sim_drop_in_basket(int robot_id, int object_id, int basket_id)
{
    T_robot_state* cur;

    if (debug)
    {
        fprintf(stderr, "sim_drop_in_basket(%d, %d, %d)\n",
                 robot_id, object_id, basket_id);
    }

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_drop_in_basket\n",
                 robot_id);
        return;
    }

    shape* object;
    if (object_id == -1)
    {
        object = object_robot_is_carrying(cur->object_ptr);
    }
    else
    {
        object = id_to_object(object_id);
    }

    if (basket_id == -1)
    {
        // Just plop it on the ground.
        object->unbind(cur->object_ptr);
    }
    else
    {
        // Put it in the basket
        shape* basket = id_to_object(basket_id);

        if (basket && object && (object->bound_to_ == cur->object_ptr))
        {
            if (debug)
            {
                fprintf(stderr, "   trying to put object in basket\n");
            }

            object->unbind(cur->object_ptr);

            // ENDO
            if (!(basket->is_robot()))
            {
                object->bind(basket);
            }
        }
    }
}

float sim_get_elevation(int robot_id) {
  T_robot_state* cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in move_robot\n", robot_id);
        return 0.0;
    }

    // if it is a uuv then the elevation is the sea floor
    if (cur->mobility_type == 2) {
       return Terrain->Get_Height(cur->loc.x, cur->loc.y);
    }

    // otherwise if the terrain is water then the elevation is sea level
    if (Terrain->Is_Water(cur->loc.x, cur->loc.y)) {
	return Terrain->Get_SeaLevel();
    }
    return Terrain->Get_Height(cur->loc.x, cur->loc.y);
}

// CNP related functions moved in mlab_cnp.cc
/*
CNP_RequestMess sim_cnp_got_a_task(int roboid) {
   CNP_OpenRequest *req;
   CNP_RequestMess mess;
   //   req = CNPServer->Check_ForTask(roboid);
   req = CNPServerManager->Check_ForTask(roboid);
   if (req == NULL) {
      mess.TaskID = -1;
      mess.Iteration = -1;
      mess.TaskDesc = strdup("None");
      mess.Restriction = strdup("None");
   }
   else{
      mess.TaskID = req->TaskID;
      mess.Iteration = req->Iteration;
      mess.TaskDesc = strdup(req->TaskType);
      mess.Restriction = strdup(req->Requirements);
      printf("task: %s\n", req->Requirements);
   }
   return mess;
}
*/

/*
void inject_track_task(int tt) {
  CNP_Request req;
  char dstr[80];

  req.TaskID = CNP_TaskID;
  CNP_TaskID++;
  req.Iteration = 1;
  if (tt == 1) {
    req.TaskType = strdup("TRACKRED");
  }
  if (tt == 2) {
    req.TaskType = strdup("TRACKYELLOW");
  }
  req.Requirements = strdup("Land");
  req.Exclusions = new SLList();
  //  if (CNPServer->Inject_Task(&req)) {
  if (CNPServerManager->Inject_Task(&req)) {
    printf("Task Injected");
    sprintf(dstr,"Task Injected: TaskID: %d %s\n", req.TaskID, req.TaskType);
    sim_alert(dstr);
  }
  else {
    printf("Task Injection Failed!\n");
  }
}
*/

/****************************************************************************
*                                                                           *
* simulacion NO_HOLONOMIC                                                   *
*                                                                           *
*  Calcula la distancia recorrida por un vehiculo no_holonomic a partir de  *
*  la velocidad en el eje X, Y, la orientación y el tiempo de ciclo         *
*                                                                           *
****************************************************************************/
Vector sim_no_holonomic(double velX, double velY, double heading, double tiempo)
{
    double x_dot , y_dot , phi_dot ;
	double phi, alfa;
	double velAvance=0;
    Vector v;
    Vector avance;
    const float DIST_EJES = 1.25;
    const float ALFA_MAX = 75; // Maximo angulo de giro de la rueda de la carretilla (grados)
    const float VEL_MAX = 5; // Maxima velocidad de la carretilla (m/s)

    v.x = velX;
    v.y = velY;
    v.z = 0;
    float speed = len_2d(v);
    float theta = RADIANS_TO_DEGREES(atan2(velY, velX));
    if(speed != 0  || theta != 0)
	{
		// Ecuaciones obtenidas del modelo cinematico directo del vehiculo tipo triciclo
		velAvance = sqrt(pow(speed,2)+ pow((theta*(PI/180)),2)*pow(DIST_EJES,2));
		alfa = asin(DIST_EJES*(theta*PI/180)/velAvance);

		phi = heading * (PI/180);

		// Comprobacion de alfa para evitar angulos fuera de limite. Se mantiene el signo del angulo
		if(fabs(alfa) > (ALFA_MAX*(PI/180)))
		{
			if(alfa < 0)
				alfa = ALFA_MAX * (PI/180) *-1;
			else
				alfa = ALFA_MAX * (PI/180);

			// Calculo de la nueva velocidad de avance basada en ALFA_MAX
			velAvance = (DIST_EJES*speed)/(sin(alfa));


		}

		// Control de la velocidad enviada al robot
        velAvance = fabs(velAvance);
		if(velAvance > VEL_MAX)
			velAvance = VEL_MAX;

		// Calculo de los nuevos incrementos
		y_dot = sin(phi) * cos(alfa) * velAvance;
		x_dot = cos(phi) * cos(alfa) * velAvance;
		phi_dot = (sin(alfa)/DIST_EJES) * velAvance;


		avance.x = x_dot * tiempo;
		avance.y = y_dot * tiempo;
		avance.z = phi_dot * tiempo ;

	}
    else{
        avance.x = 0.0;
		avance.y = 0.0;
		avance.z = 0.0;
    }
    return avance;
}

/****************************************************************************
*                                                                           *
* sim_move_robot                                                            *
*                                                                           *
*  Move the robot by a vector which is relative to the current heading      *
*                                                                           *
****************************************************************************/

void sim_move_robot(int robot_id, gt_Point rel_dist)
{
    T_robot_state* cur;
    Vector v, abs_dist, oldloc;
    double heading;
    static int vis3dcnt = 0;
    bool drawVis3DNow = false;
    bool water = false;
    //const double MAX_STEER_ANGLE = 25; 		// +- degrees
    //const double MAX_STEER_ANGLE_RATE = 25;	// +- degrees/sec
    //const double WHEEL_BASE = 3.302;   		// Meters
    //const int VIS3D_NUM_SKIPS = 6;
    const int VIS3D_NUM_SKIPS = 18;
    const bool RUN_LOS = true;
    Vector avance;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in move_robot\n", robot_id);
        return;
    }

    cur->step_taken = true;
    ShowMovementFieldMenuItemEnabled(true);

    // Create a vector rotated to world coordinates
    abs_dist.x = rel_dist.x;
    abs_dist.y = rel_dist.y;
    abs_dist.z = rel_dist.z;
    rotate_z(abs_dist, cur->heading);

    // if we should be drawing a vector field for the robot, then just do that
    if (cur->drawing_vector_field)
    {
        g_objVectorField.DrawVector(cur->loc, abs_dist);
        return;
    }
    g_objVectorField.EraseVectors();

    if (cur->run_type == SIMULATION)
    {

        switch (cur->robot_type) {
	case NO_HOLONOMIC:
            avance = sim_no_holonomic(rel_dist.x, rel_dist.y, cur->heading, cycle_duration);
            //printf( "Avance: new pos <%.1f %.1f > %.0f degrees\n",avance.x, avance.y, avance.z);
            v.x = cur->loc.x + avance.x;
            v.y = cur->loc.y + avance.y;
            v.z = 0;
            heading = cur->heading + RADIANS_TO_DEGREES(avance.z);
            oldloc = cur->loc;
        break;
        case HOLONOMIC:
        default:
            Vector delta = abs_dist;

            mult_3d(delta, cycle_duration);

            v.x = cur->loc.x + delta.x;
            v.y = cur->loc.y + delta.y;
            v.z = cur->loc.z + delta.z;

            // store the old location for possible heightmap issues
            oldloc = cur->loc;

            // fake a heading based on where we were and where we are now
            if (len_2d(abs_dist) < EPS_ZERO)
            {
                heading = cur->heading;
            }
            else
            {
                heading = RADIANS_TO_DEGREES(atan2(abs_dist.y, abs_dist.x));
            }

            break;
        }

        // bound to range 0..359
        deg_range_pm_360(heading);

        // keep the robot on the playing field
        v.x = bound(v.x, 0.5, (double) mission_area_width_meters  - 0.5);
        v.y = bound(v.y, 0.5, (double) mission_area_height_meters - 0.5);
    }

    // Erase old vector
    if (cur->is_up && show_movement_vectors)
    {
        if (gt_show_3d_vis && (Terrain != NULL))
        {
            // Don't draw the movement vector.
        }
        else
        {
            DrawVector(cur->loc, cur->old_v);
        }
    }

    if (cur->run_type == SIMULATION)
    {
        // show the path

        if (RUN_LOS && (!gt_show_3d_vis))
        {
            //the following lines are for the line-of-sight visualization
            //A robot is considered to be in 'line-of-sight' if it can see
            //a yellow robot (the robot at the entrance of the building)
            //or a robot that is in 'line-of-sight'
            //
            // This portion of the code needs to be move to somewhere as
            // it does not belong to sim_move_robot().

            if (uf)
            {
                T_robot_state* robotState;

                robotState = robots;
                while (robotState)
                {
                    robotState->LOS_not = true;
                    robotState = robotState->next;
                }
                uf = false;
            }
            //consider the robots that I can see

            RobotReadings rtn = sim_detect_robots(cur->robot_id, 100.0);
            cur->object_ptr->LOS = false;

            int ndet = rtn.num_readings;
            for (int j = 0; j < ndet; j++)
            {
                int det_id = rtn.readings[j].id;
                T_robot_state* cr;
                if ((cr = find_robot_rec(det_id)) != NULL)
                {
                    if (cr->object_ptr->LOS || !strcmp("yellow", cr->object_ptr->color_))
                    {
                        cur->object_ptr->LOS = true;
                    }
                }
                //if one of the robot that I can see is a yellow robot or a
                //robot in 'line-of-sight' I am in 'line-of-sight'

            }

            int LOS = cur->object_ptr->LOS;
            if ((cur->LOS_not && LOS_mode && LOS) || (!cur->pcount && LOS && LOS_mode))
            {
                ch = 1;
            }
            if ((cur->LOS_not && !LOS_mode && cur->pcount) || (cur->pcount && !LOS && LOS_mode))
            {
                ch = 2;
            }
            cur->LOS_not = false;
            cur->pcount = LOS;

            // this is for changing the color of the robots that are in
            // 'line-of-sight'

            if (ch)
            {
                if (ch ==1)
                {
                    DrawPath(
                        cur->loc.x,
                        cur->loc.y,
                        cur->heading,
                        0,
                        0,
                        0,
                        cur->object_ptr->gc(),
                        2 * (int) cur->is_up);

                    DrawPath(
                        0,
                        0,
                        0,
                        v.x,
                        v.y,
                        heading,
                        cur->object_ptr->LOSgc,
                        2 * (int) cur->is_up);

                }
                else
                {
                    DrawPath(
                        cur->loc.x,
                        cur->loc.y,
                        cur->heading,
                        0,
                        0,
                        0,
                        cur->object_ptr->LOSgc,
                        2 * (int) cur->is_up);

                    DrawPath(
                        0,
                        0,
                        0,
                        v.x,
                        v.y,
                        heading,
                        cur->object_ptr->gc(),
                        2 * (int) cur->is_up);
                }
                ch = 0;
            }
            else
            {
                if (LOS_mode && LOS)
                {
                    DrawPath(
                        cur->loc.x,
                        cur->loc.y,
                        cur->heading,
                        v.x,
                        v.y,
                        heading,
                        cur->object_ptr->LOSgc,
                        cur->is_up);
                }
                else
                {
                    if (cur->vis)
                    {
                        // PU Option to draw military style icons
                        if (gt_draw_robot_military_style) {
                            Draw_MilitaryStyle(cur->loc.x, cur->loc.y,  v.x, v.y, cur->mobility_type, cur->object_ptr->gc(), cur->is_up, cur->col);
                            cur->is_up = true;
                        }
                        else {
                            DrawPath(
                                cur->loc.x,
                                cur->loc.y,
                                cur->heading,
                                v.x,
                                v.y,
                                heading,
                                cur->object_ptr->gc(),
                                cur->is_up);
                        }
                    }
                }
            }
        }
        else
        {
            if ((cur->vis) && (!gt_show_3d_vis))
            {
                if (gt_draw_robot_military_style) {
                    Draw_MilitaryStyle(cur->loc.x, cur->loc.y, v.x, v.y, cur->mobility_type, cur->object_ptr->gc(), cur->is_up, cur->col);
                }
                else {
                    DrawPath(
                        cur->loc.x,
                        cur->loc.y,
                        cur->heading,
                        v.x,
                        v.y,
                        heading,
                        cur->object_ptr->gc(),
                        cur->is_up);
                }
            }
        }

        cur->is_up = true;

        // remember the new location
        cur->heading = heading;

        cur->lastLoc = cur->loc;
        cur->loc = v;
    }

    cur->object_ptr->moveto(v.x, v.y);

    // Draw new vector
    if (show_movement_vectors)
    {
        DrawVector(v, abs_dist);
        cur->v_up = true;
        cur->old_v = abs_dist;
    }


    if (gt_show_3d_vis && (Terrain != NULL))
    {
        Robot_3d_Rec temprec;
        float x, y, z;

        x = cur->loc.x;
        y = cur->loc.y;
        z = cur->loc.z;

        temprec.X = x;
        temprec.Z = heightmap_y - y;
        temprec.Y = z;

        water = Terrain->Is_Water(x, y);


        if (water)
        {
            temprec.Y = Terrain->Get_SeaLevel();

            switch (cur->mobility_type) {

            case UXV:
                temprec.Y += UXV_OFFSET;
                break;

            case UUV:
                temprec.Y += UUV_OFFSET;
                break;

            case USV:
                temprec.Y += + USV_OFFSET;
                break;

            case UAV:
            case SLUAVL:
                temprec.Y += UAV_OFFSET;
                break;
            }
        }
        else
        {
            temprec.Y = Terrain->Get_Height(x, y);

            switch (cur->mobility_type) {

            case UXV:
                temprec.Y += UXV_OFFSET;
                break;

            case UAV:
            case SLUAVL:
                temprec.Y += UAV_OFFSET;
                break;
            }
        }

        temprec.RobotNum = cur->robot_id;

        if (vis3dcnt++ > VIS3D_NUM_SKIPS)
        {
            drawVis3DNow = true;
            vis3dcnt = 0;
        }

        Rob_Handler->Place_Robot(
            temprec.RobotNum,
            temprec.X,
            temprec.Y,
            temprec.Z,
            cur->heading,
            cur->object_ptr->color(),
            cur->mobility_type,
            robot_length_meters,
            water,
            false, // cur->Uncertainty,
            true); // cur->vis);

        if (drawVis3DNow)
        {
            drawGLScene();
        }
    }

    if (debug)
    {
        fprintf(
            stderr,
            "sim_move_robot: new pos <%.1f %.1f %.1f> %.0f degrees\n",
            cur->loc.x,
            cur->loc.y,
            cur->loc.z,
            cur->heading);
    }
}

/****************************************************************************
*                                                                           *
* sim_spin_robot                                                            *
*                                                                           *
*  Spin the robot                                                           *
*                                                                           *
****************************************************************************/

void sim_spin_robot(int robot_id, double ang_vel)
{
    T_robot_state* cur;
    Vector         v;
    double         heading;
    //Vector         abs_dist;

    //double MAX_STEER_ANGLE = 25; 		// +- degrees
    //double MAX_STEER_ANGLE_RATE = 25;	// +- degrees/sec
    //double WHEEL_BASE = 3.302;   		// Meters


    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in spin_robot\n", robot_id);
        return;
    }

    // Create a vector rotated to world coordinates

    if (cur->run_type == SIMULATION)
    {
        switch (cur->robot_type) {

        case HOLONOMIC:
        default:
            double delta_w = ang_vel;
            delta_w *= cycle_duration;
            v.x = cur->loc.x;
            v.y = cur->loc.y;
            v.z = cur->loc.z;
            heading = cur->heading;
            heading += delta_w;
            break;
        }

        // bound to range 0..359
        deg_range_pm_360(heading);
    }

    // Erase old vector
    if (cur->is_up && show_movement_vectors)
    {
        DrawVector(cur->loc, cur->old_v);
    }


    if (cur->run_type == SIMULATION)
    {
        if (cur->vis) {
            if (gt_draw_robot_military_style) {
                Draw_MilitaryStyle(cur->loc.x, cur->loc.y, v.x, v.y, cur->mobility_type, cur->object_ptr->gc(), cur->is_up, cur->col);
            }
            else {
                DrawPath(cur->loc.x, cur->loc.y, cur->heading, v.x, v.y, heading,
                         cur->object_ptr->gc(), cur->is_up);
            }
        }
        cur->is_up = true;

        // remember the new location
        cur->heading = heading;
        cur->lastLoc = cur->loc;
        cur->loc = v;

    }
    cur->object_ptr->moveto(v.x, v.y);

    if (debug)
    {
        fprintf(stderr, "sim_spin_robot: new pos <%.1f %.1f> %.0f degrees cd=%f\n",
                cur->loc.x, cur->loc.y, cur->heading, cycle_duration);
    }
}

//------------------------------------------------------------------

void sim_robot_location(int robot_id, gt_Point position, double theta)
{
    sim_robot_location(robot_id, position, theta, NULL);
}

void sim_robot_location(int robot_id, gt_Point position, double theta, GC *gc)
{
    T_robot_state* cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in move_robot\n", robot_id);
        return;
    }

    if (cur->vis) {

        if (gt_draw_robot_military_style)
        {
            Draw_MilitaryStyle(
                cur->loc.x,
                cur->loc.y,
                position.x,
                position.y,
                cur->mobility_type,
                //cur->object_ptr->gc(),
                (gc == NULL)? cur->object_ptr->gc() : *gc,
                cur->is_up,
                cur->col);
        }
        else
        {
            // show the path
            DrawPath(
                cur->loc.x,
                cur->loc.y,
                cur->heading,
                position.x,
                position.y,
                theta,
                (gc == NULL)? cur->object_ptr->gc() : *gc,
                cur->is_up);
        }
    }
    cur->is_up = true;

    // remember the new location
    cur->loc.x = position.x;
    cur->loc.y = position.y;
    cur->heading = theta;
}

//------------------------------------------------------------------
void sim_set_mobility_type(int robotID, int mobilityType)
{
    T_robot_state* cur = NULL;
    const bool DEBUG_SIM_SET_MOBILITY_TYPE = false;

    if (debug || DEBUG_SIM_SET_MOBILITY_TYPE)
    {
        fprintf(
            stderr,
            "sim_set_mobility_type()[%d]: New Mobility Type = %s [%d]\n",
            robotID,
            ROBOT_MOBILITY_TYPE_NAMES[mobilityType].c_str(),
            mobilityType);
    }

    if ((cur = find_robot_rec(robotID)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_set_mobility_type()\n", robotID);
        return;
    }

    if (!gt_show_3d_vis)
    {
        erase_robot(cur);
    }

    if (mobilityType == SLUAVU)
    {
        cur->vis = false;
        DrawPath(0, 0, 0, cur->loc.x, cur->loc.y, cur->heading, cur->object_ptr->gc(), false);
    }
    else
    {
        cur->vis = true;
    }

    cur->mobility_type = mobilityType;
}

int sim_get_mobility(int robotID) {
  T_robot_state* cur = NULL;

  if ((cur = find_robot_rec(robotID)) == NULL) {
    fprintf(stderr, "server: bad robot number %d in sim_set_mobility_type()\n", robotID);
    return -1;
  }

  return cur->mobility_type;
}



robot_position sim_get_position(int robot_id)
{
    T_robot_state* cur;
    robot_position loc;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in get_xy\n", robot_id);
        loc.v.x = 0;
        loc.v.y = 0;
        loc.heading = 0;
        return loc;
    }
    else
    {
        loc.v = cur->loc;
        loc.heading = cur->heading;
    }

    if (debug)
    {
        fprintf(stderr, "sim_get_position(%d): <%.1f %.1f> %.0f degrees\n",
                 robot_id, loc.v.x, loc.v.y, loc.heading);
    }

    return loc;
}

//------------------------------------------------------------------

double sim_get_heading(int robot_id)
{
    T_robot_state* cur;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in get_xy\n", robot_id);
        return 0;
    }

    return cur->heading;
}

//------------------------------------------------------------------

int send_robot_command(gt_Command* cmd, gt_Robot type)
{
    T_robot_state* cur;
    char *value = NULL, *ptr = NULL;
    int robot_id = cmd->robot_id;
    int alive;
    int timeout;
    int mobilityType;
    static int seq = 0;
    bool isExtraRobot = false;

    // copy a unique sequence number to the command
    cmd->seq_num = seq++;

    // Should we startup a new robot executable?
    if (cmd->behavior == START)
    {
        cur = find_robot_rec(robot_id);

        // make sure is a unique robot ID
        if ((cur != NULL) && (cur->alive))
        {
            fprintf(stderr, "Request to start existing robot_id %d\n", robot_id);
            return GT_FAILURE;
        }

        // If they didn't specify a starting location, then spread them out
        if (((cmd->goal.x) == 0) && ((cmd->goal.y) ==0))
        {
            cmd->goal.x = 50 + cmd->robot_id * 10;
            cmd->goal.y = 50;
        }

        // exec the robot executable
        if (start_robot(
                gt_robot_info[type].host,
                gt_robot_info[type].executable,
                robot_id,
                robot_debug_messages,
                report_state_messages,
                cmd->goal.x,
                cmd->goal.y,
                gt_robot_info[type].color) == -1)
        {
            return GT_FAILURE;
        }

        // wait for the robot to signal its birth
        alive = false;
        timeout = 10;
        while (!alive && (timeout-- > 0) && (console_error == NONE))
        {
            // yield process time
            sleep(1);

            // run the com link
            gt_simulation_loop();

            // make sure the robot exists
            cur = find_robot_rec(robot_id);
            if (cur == NULL)
            {
                fprintf(stderr, "server: Internal Error starting robot %d in send_robot_command\n",
                         robot_id);
                return GT_FAILURE;
            }

            alive = cur->alive;
        }
        if (timeout <= 0)
        {
            fprintf(stderr, "Unable to start robot %d in send_robot_command\n", robot_id);
            return GT_FAILURE;
        }
        else if (console_error != NONE)
        {
            fprintf(stderr, "Unable to start robot %d in send_robot_command\n", robot_id);
            fprintf(stderr, "%s\n", console_error_messages[console_error]);
            return GT_FAILURE;
        }

        // send the start command
        new_command(robot_id, cmd);

        // Update the database for this robot
        if (debug)
        {
            fprintf(
                stderr,
                "Number of robot parameters = %d\n",
                gt_robot_info[type].num_params);
        }

        for (int i = 0; i < gt_robot_info[type].num_params; i++)
        {
            if (debug)
            {
                fprintf(
                    stderr,
                    "setting robot parm:  '%s' = '%s'\n",
                    gt_robot_info[type].param[i].key,
                    gt_robot_info[type].param[i].value);
            }

            sim_put_console_state(robot_id,
                                   gt_robot_info[type].param[i].key,
                                   gt_robot_info[type].param[i].value);
        }

        // Assign the mobility type if it is not an extra robot, and
        // CNP robotCosntraints are specified by gMMD.
        if (((cur = find_robot_rec(robot_id)) != NULL) && (gMMD != NULL))
        {
            value = sim_get_console_state(robot_id, (char *)(STRING_EXTRA_ROBOT.c_str()));

            if (value != NULL)
            {
                isExtraRobot = (bool)(strtol(value, &ptr, 10));

                if((ptr == value) || (ptr == NULL) || (*ptr != '\0'))
                {
                    // Bad entry
                    isExtraRobot = false;
                }


                if (!isExtraRobot)
                {
		  mobilityType = gMMD->getMobilityTypeFromRobotConstraints(robot_id);

                    if (mobilityType > -1)
                    {
		      cur->mobility_type = mobilityType;
                    }
                }
            }
        }

        // unpause the robot, if the console is not in paused state
        send_robot_pause_state(robot_id, paused);
    }
    else
    {
        // just want to send a new command, so make sure the robot exists
        if (robot_id != -1)
        {
            if ((cur = find_robot_rec(robot_id)) == NULL)
            {
                fprintf(stderr, "server: unknown robot_id %d in send_robot_command\n", robot_id);
                return GT_FAILURE;
            }
        }

        new_command(robot_id, cmd);
    }

    return GT_SUCCESS;
}

//------------------------------------------------------------------

char* sim_get_console_state(int robot_id, char* parm)
{
    char* rtn;

    rtn = gt_inquire(robot_id, parm);

    if (debug)
    {
        fprintf(stderr, "sim_get_console_state(%d,%s) -> '%s'\n",robot_id,parm,rtn);
    }

    return rtn;
}

//------------------------------------------------------------------

void sim_put_console_state(int robot_id, char* parm, char* val)
{
    // write the local cache
    gt_update(robot_id,parm,val);

    // forward it to the robot
    sim_put_state(robot_id, parm, val);
}

//------------------------------------------------------------------

void erase_halo(int robot_id)
{
    int i;

    for (i = 0; i < num_obs; i++)
    {
        if (obs[i].has_halo == robot_id)
        {
            // erase it
            obs[i].has_halo = -1;
            DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
        }

        if (obs[i].impacted == robot_id)
        {
            // erase it
            obs[i].impacted = -1;
            DrawImpact(obs[i].center.x, obs[i].center.y, obs[i].impact_radius);
        }
    }
}

//------------------------------------------------------------------

void erase_movement_vectors()
{
    T_robot_state* cur;
    int i = 0;

    cur = robots;
    while (cur)
    {
        if (cur->v_up)
        {
            DrawVector(cur->loc, cur->old_v);
            cur->v_up = false;
        }

        cur = cur->next;

        if (i++ > 10000)
        {
            fprintf(stderr, "erase_movement_vectors: loop in robot state records\n");
            cur->next = NULL;
            break;
        }
    }
}

//------------------------------------------------------------------

void erase_halos()
{
    int i;

    for (i = 0; i < num_obs; i++)
    {
        if (obs[i].has_halo != -1)
        {
            // erase it
            obs[i].has_halo = -1;
            DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
        }

        if (obs[i].impacted != -1)
        {
            // erase it
            obs[i].impacted = -1;
            DrawImpact(obs[i].center.x, obs[i].center.y, obs[i].impact_radius);
        }
    }
}
//------------------------------------------------------------------
// Function uses an odd/even test in two dimensions to determine if the robot
// is within any room
bool sim_detect_in_room(int robot_id, robot_position loc)
{
    T_robot_state*  cur;
    sim_obstacle_t* line_obs;
    sim_obstacle_t* t;
    point2d_t       rcenter;
    room_t*         tmp;
    int             num_line_obs;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_detect_in_room\n",
                 robot_id);
        return false;
    }
    rcenter.x = loc.v.x;
    rcenter.y = loc.v.y;

    // Loop over each room in the database and check if we are inside it
    for (tmp = gConsoleDBRoomList; tmp != NULL; tmp = tmp->next)
    {
        // Fire a ray from the robot's center a long distance to ensure it
        // has passed through the drawing. We can then use the odd\even test
        // to determine if the robot is within the room.

        line_obs = calculateRayLineIntersections(&rcenter, loc.heading,
                                                  tmp->walls, &num_line_obs);

        // Free all the intersection points, we are only interested in
        // the number of intersections
        while (line_obs != NULL)
        {
            t = line_obs;
            line_obs = line_obs->next;
            free(t);
        }

        // Odd number of intersections, inside the room
        if ((num_line_obs % 2) != 0)
        {
            return true;
        }
    }

    // Outside of all rooms!
    return false;
}
//------------------------------------------------------------------
// Function uses an odd/even test in two dimensions to determine if the robot
// is within any hallways, and returns the information of the all the hallways found
HallwayList sim_detect_hallway(int robot_id, robot_position loc)
{
    T_robot_state*       cur;
    point2d_t            rcenter;
    hallwayRec_t*        tmp;
    static hallwayRec_t* last_hway = NULL;
    int                  num_line_obs, count = 0;
    sim_obstacle_t*      line_obs;
    sim_obstacle_t*      t;
    HallwayList          retval;

    retval.num_hways = 0;
    retval.hways = NULL;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_detect_hallway\n",
                 robot_id);
        return retval;
    }
    rcenter.x = loc.v.x;
    rcenter.y = loc.v.y;

    // First check to see if we are still within the last hallway
    // This way the first hallway in the hallway list will be the
    // one we were inside in the last iteration
    if (last_hway != NULL)
    {
        line_obs = calculateRayLineIntersections(&rcenter, loc.heading,
                                                  last_hway->extents, &num_line_obs);
        // Free all the intersection points, we are only interested in
        // the number of intersections
        while (line_obs != NULL)
        {
            t = line_obs;
            line_obs = line_obs->next;
            free(t);
        }
        // Odd number of intersections, inside the hallway
        if ((num_line_obs % 2) != 0)
        {
            // Check to make sure we allocated enough space
            if ((count + 1) > retval.num_hways)
            {
                // Too little space so double it
                retval.num_hways = (retval.num_hways == 0) ? 1 : retval.num_hways * 2;
                retval.hways = (hallway_t*) realloc(retval.hways,
                                                     sizeof(hallway_t) * retval.num_hways);
                if (retval.hways == NULL)
                {
                    fprintf(stderr, "server: out of memory in sim_detect_hallway (robot %d)\n",
                             robot_id);
                    retval.num_hways = 0;
                    return retval;
                }
            }

            retval.hways[count].start = last_hway->start;
            retval.hways[count].end   = last_hway->end;
            retval.hways[count].width = last_hway->width;
            count++;
        }
    }

    // Loop over each hallway in the database and check if we are inside it
    for (tmp = gConsoleDBHallwayList; tmp != NULL; tmp = tmp->next)
    {
        if (tmp == last_hway)
        {
            continue;
        }

        // Simple bounding box check will not suffice. The hallway might be slanted.
        // Use odd/even test again
        line_obs = calculateRayLineIntersections(&rcenter, loc.heading,
                                                  tmp->extents, &num_line_obs);
        // Free all the intersection points, we are only interested in
        // the number of intersections

        while (line_obs != NULL)
        {
            t = line_obs;
            line_obs = line_obs->next;
            free(t);
        }

        // Odd number of intersections, inside the hallway
        if ((num_line_obs % 2) != 0)
        {
            // Check to make sure we allocated enough space
            if ((count + 1) > retval.num_hways)
            {
                // Too little space so double it
                retval.num_hways = (retval.num_hways == 0) ? 1 : retval.num_hways * 2;
                retval.hways = (hallway_t*) realloc(retval.hways,
                                                     sizeof(hallway_t) * retval.num_hways);

                if (retval.hways == NULL)
                {
                    fprintf(stderr, "server: out of memory in sim_detect_hallway (robot %d)\n",
                             robot_id);
                    retval.num_hways = 0;
                    return retval;
                }
            }

            retval.hways[count].start = tmp->start;
            retval.hways[count].end   = tmp->end;
            retval.hways[count].width = tmp->width;
            if (count == 0)
            {
                last_hway = tmp;
            }
            count++;
        }
    }
    if (count == retval.num_hways)
    {
        return retval;
    }

    // Too much space allocated so trim it off
    retval.num_hways = count;
    retval.hways = (hallway_t*) realloc(retval.hways, sizeof(hallway_t) * count);
    return retval;
}

//------------------------------------------------------------------
// Function returns a list of doorways that the robot is close to. The parameters
// sensor direction, field of view, and range control the way in which doorways
// are detected. Only doors that are within sensorRange of the center of the robot
// and within 1/2 sensorFoV degrees from the heading sensorDir are returned.
DoorwayList sim_detect_doorway(int robot_id, double sensorDir,
                                double sensorFoV, double sensorRange)
{
    T_robot_state*  cur;
    Vector          v;
    sim_obstacle_t* line_obs;
    point2d_t       rcenter, dcenter;
    int             num_line_obs, count = 0;
    double          cwDir, ccwDir, dist, ang;
    door_t*         tmp;
    DoorwayList     retval;

    retval.num_doors = 0;
    retval.doors = NULL;
    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_detect_doorway\n",
                 robot_id);
        return retval;
    }

    rcenter.x = cur->loc.x;
    rcenter.y = cur->loc.y;

    // Check for doors within range
    for (tmp = gConsoleDBDoorwayList; tmp != NULL; tmp = tmp->next)
    {
        // Set dcenter to the midpoint of the door */
        dcenter.x = (tmp->doorway->ls.endpt[0].x + tmp->doorway->ls.endpt[1].x) / 2.0;
        dcenter.y = (tmp->doorway->ls.endpt[0].y + tmp->doorway->ls.endpt[1].y) / 2.0;

        // Check to see of the center of the doorway is within range.
        if ((dist = cartesianDistance(&rcenter, &dcenter)) > sensorRange)
        {
            continue;
        }

        // Calculate the angle from the robot to the door
        ang = atan2((dcenter.y - rcenter.y), (dcenter.x - rcenter.x));
        ang = DEGREES_FROM_RADIANS(ang);
        deg_range_pm_360(ang);

        // Calculate the range of our field of view and normalize
        cwDir  = sensorDir - sensorFoV / 2.0;
        ccwDir = sensorDir + sensorFoV / 2.0;
        deg_range_pm_360(cwDir );
        deg_range_pm_360(ccwDir);

        // Test for the field of view extending through zero. If our FoV does
        // include zero, then we need to adjust the ccwDir and our test angle
        if ((cwDir + sensorFoV) > 360)
        {
            if (ang < ccwDir)
            {
                ang += 360;
            }
            ccwDir += 360;
        }
        // Now check to see if the door is within the field of view of the sensor
        if ((ang < cwDir) || (ang > ccwDir))
        {
            continue;
        }

        // We found a doorway intersect, might be occluded by walls. So
        // fire a single ray in the direction of the doorway at the wall objects
        line_obs = simulateSonarLineDetection(&rcenter, ang,
                                               gConsoleDBLineList, 1,
                                               sensorRange, &num_line_obs);
        if (line_obs != NULL)
        {
            // We collided with the doorway and a wall, determine which is closer
            v.x = line_obs->center[0] - rcenter.x;
            v.y = line_obs->center[1] - rcenter.y;
            free(line_obs);
            if (dist > len_2d(v)) // the wall is closer
            {
                continue;
            }
        }

        // We found a doorway! Add it to the list and draw a marker.
        count++;

        if (highlight_repelling_obstacles)
        {
            DrawImpact(dcenter.x, dcenter.y, 1.0);
        }

        // Check to make sure we allocated enough space
        if (count > retval.num_doors)
        {
            // Too little space so double it
            retval.num_doors = (retval.num_doors == 0) ? 1: retval.num_doors * 2;
            retval.doors = (gt_Doorway_t*) realloc(retval.doors,
                                                    sizeof(gt_Doorway_t) * retval.num_doors);
            if (retval.doors == NULL)
            {
                fprintf(stderr, "server: out of memory in sim_detect_doorway (robot %d)\n",
                         robot_id);
                retval.num_doors = 0;
                return retval;
            }
        }

        // Store the door away so it can be returned
        retval.doors[count - 1].x1 = tmp->doorway->ls.endpt[0].x;
        retval.doors[count - 1].y1 = tmp->doorway->ls.endpt[0].y;
        retval.doors[count - 1].x2 = tmp->doorway->ls.endpt[1].x;
        retval.doors[count - 1].y2 = tmp->doorway->ls.endpt[1].y;
        retval.doors[count - 1].distFromRobot = dist;
        retval.doors[count - 1].marked = tmp->marked;
    }
    if (count == retval.num_doors)
    {
        return retval;
    }

    // Too much space allocated so trim it off
    retval.num_doors = count;
    retval.doors = (gt_Doorway_t*) realloc(retval.doors, sizeof(gt_Doorway_t) * count);
    return retval;
}

//------------------------------------------------------------------

void sim_markunmark_nearest_doorway(int robot_id, int mark_status)
{
    DoorwayList doors = sim_detect_doorway(robot_id, 0, 360, 100);

    if (doors.num_doors == 0)
    {
        return;
    }
    double  bdist = doors.doors[0].distFromRobot;
    int best = 0;
    for (int i = 1; i < doors.num_doors; i++)
    {
        if (doors.doors[i].distFromRobot < bdist)
        {
            best = i;
            bdist = doors.doors[i].distFromRobot;
        }
    }
    for (door_t* tmp = gConsoleDBDoorwayList; tmp != NULL; tmp = tmp->next)
    {
        if ((tmp->doorway->ls.endpt[0].x == doors.doors[best].x1) &&
             (tmp->doorway->ls.endpt[0].y == doors.doors[best].y1) &&
             (tmp->doorway->ls.endpt[1].x == doors.doors[best].x2) &&
             (tmp->doorway->ls.endpt[1].y == doors.doors[best].y2))
        {
             tmp->marked = mark_status;
        }
    }
    free(doors.doors);
    return;
}

#define NUM_SIM_SONAR 16

/****************************************************************************
*                                                                           *
* sim_detect_obstacles                                                      *
*                                                                           *
****************************************************************************/

Obs_List sim_detect_obstacles(int robot_id, double sensor_range)
{
    T_robot_state*  cur;
    Vector          v;
    double          dist;
    int             num_within_range;
    int             i;
    int             pos;
    char*           val;
    double          safety_margin = 0;
    sim_obstacle_t* line_obs;
    sim_obstacle_t* zone_obs;
    sim_obstacle_t* tempobs;
    sim_obstacle_t* tmp;
    point2d_t       rcenter;
    int             num_line_obs, num_zone_obs;
    static sim_obstacle_t* last_line_obs = NULL;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in detect_obstacles\n", robot_id);
        Obs_List out(0);
        return out;
    }

    // count number of obstacles within range
    num_within_range = 0;
    for (i = 0; i < num_obs; i++)
    {
        v.x = obs[i].center.x - cur->loc.x;
        v.y = obs[i].center.y - cur->loc.y;
        v.z = 0;

        if (gt_show_3d_vis && (obshighpts != NULL))
        {
            v.z = (double)obs[i].height + obshighpts[i];
        }

        dist = len_2d(v);

        // check for obstacle elevation for ground robots
        if (gt_show_3d_vis)
        {
            if ((cur->loc.z == 0) && (v.z - sim_get_elevation((int)cur->loc.x, (int)cur->loc.y) > 0))
            {
                if (dist <= obs[i].radius + sensor_range)
                {
                    num_within_range++;
                }
            }
            else
            {
                if ((cur->loc.z != 0) && (dist <= obs[i].radius + sensor_range))
                {
                    num_within_range++;
                }
            }
        }
        else
        {
            if (dist <= obs[i].radius + sensor_range)
            {
                num_within_range++;
            }
        }
    }

    // Get the obstacles from line objects and their count
    rcenter.x = cur->loc.x;
    rcenter.y = cur->loc.y;
    line_obs = simulateSonarLineDetection(
        &rcenter,
        cur->heading,
        gConsoleDBLineList,
        NUM_SIM_SONAR,
        sensor_range,
        &num_line_obs);



    // PU Overlay Zone handling


    // UAV's can't go through no fly zones
    if (cur->mobility_type == UAV) {

      // add no-fly zones to obstacle list
      vector<NoFlyZone *>::iterator nfit = NoFlyZones.begin();
      while (nfit != NoFlyZones.end()) {

	// go to the end of the obstacle list
	tempobs = line_obs;
	if (tempobs != NULL) {
	  while (tempobs->next != NULL) {
	    tempobs = tempobs->next;
	  }
	}
	zone_obs = simulateSonarLineDetection( &rcenter,
					       cur->heading,
					       (*nfit)->Get_LineList(),
					       NUM_SIM_SONAR,
					       sensor_range,
					       &num_zone_obs);

	//	printf("%d: found %d obstacles from the no fly zone!\n", robot_id,num_zone_obs);
	if (tempobs == NULL) {
	  line_obs = zone_obs;
	}
	else {
	  tempobs->next = zone_obs;
	}
	num_line_obs += num_zone_obs;
	nfit++;
      }
    }


    // No one can go through weather zones
    // add weather zones to obstacle list
    vector<WeatherZone *>::iterator wzit = WeatherZones.begin();
    while (wzit != WeatherZones.end()) {
      // go to the end of the obstacle list
      tempobs = line_obs;
      if (tempobs != NULL) {
	while (tempobs->next != NULL) {
	  tempobs = tempobs->next;
	}
      }
      zone_obs = simulateSonarLineDetection( &rcenter,
					     cur->heading,
					     (*wzit)->Get_LineList(),
					     NUM_SIM_SONAR,
					     sensor_range,
					     &num_zone_obs);
      if (tempobs == NULL) {
	line_obs = zone_obs;
      }
      else {
	tempobs->next = zone_obs;
      }

      num_line_obs += num_zone_obs;
      wzit++;
    }


    // Erase and delete the old obstacles

    while (last_line_obs != NULL)
    {
        if (highlight_repelling_obstacles)
        {
            DrawImpact(last_line_obs->center[0], last_line_obs->center[1], 0.3);
        }

        tmp = last_line_obs;
        last_line_obs = last_line_obs->next;
        free(tmp);
    }

    // Draw the new obstacles
    for (tmp = line_obs; tmp != NULL; tmp = tmp->next)
    {
        if (highlight_repelling_obstacles)
        {
            DrawImpact(tmp->center[0], tmp->center[1], 0.3);
        }
    }

    // Store the new obstacles for next time
    last_line_obs = line_obs;

    // Add to the count the line obstacles
    num_within_range += num_line_obs;










    // allocate storage for them

    Obs_List out(num_within_range);

    // if want graphics, then get the safety margin to draw
    if (highlight_repelling_obstacles)
    {
        if ((val = gt_inquire(robot_id, SAFETY_MARGIN_MSG)) != NULL)
        {
            safety_margin = atof(val);
        }
    }

    // return the obstacles within range
    pos = 0;

    while (line_obs != NULL)
    {
        v.x = line_obs->center[0] - cur->loc.x;
        v.y = line_obs->center[1] - cur->loc.y;
        v.z = line_obs->center[2] - (cur->loc.z + sim_get_elevation((int)cur->loc.x, (int)cur->loc.y));

        rotate_z(v, -1 * cur->heading);
        out.obs[pos].x = v.x;
        out.obs[pos].y = v.y;
        out.obs[pos].z =  v.z;
        out.r[pos] = 0.1;
        pos++;
        line_obs = line_obs->next;
    }

    for (i = 0; i < num_obs; i++)
    {
        v.x = obs[i].center.x - cur->loc.x;
        v.y = obs[i].center.y - cur->loc.y;
        v.z = 0;
        if (gt_show_3d_vis)
        {
            v.z = obs[i].trueheight - (cur->loc.z + sim_get_elevation((int)cur->loc.x, (int)cur->loc.y));
        }
        dist = len_2d(v);

        // This code needs to be reviwed
        /*
        // check to make sure ground robot can see obstacle
        if ((cur->loc.z == 0) && (v.z  > 0))
        {
        if (dist <= (obs[i].radius + sensor_range))
        {
        // Make vector egocentric
        rotate_z(v, -1 * cur->heading);

        // Save the vector
        out.obs[pos].x = v.x;
        out.obs[pos].y = v.y;
        out.obs[pos].z = v.z;
        out.r[pos] = obs[i].radius;

        pos++;
        }
        }

        // handle arial obstacle detection
        else
        {
        if ((cur->loc.z != 0) && (dist <= (obs[i].radius + sensor_range))) {

        // Make vector egocentric
        rotate_z(v, -1 * cur->heading);

        // Save the vector
        out.obs[pos].x = v.x;
        out.obs[pos].y = v.y;
        out.obs[pos].z = v.z;
        out.r[pos] = obs[i].radius;

        pos++;
        }
        }
        */
        if (dist <= (obs[i].radius + sensor_range))
        {
            // Make vector egocentric
            rotate_z(v, -1 * cur->heading);

            // Save the vector
            out.obs[pos].x = v.x;
            out.obs[pos].y = v.y;
            out.obs[pos].z = v.z;
            out.r[pos] = obs[i].radius;
            pos++;
        }

        if (highlight_repelling_obstacles)
        {
            // draw the sphere of influence and impacts on the map

            if (dist <= (obs[i].radius + sensor_range))
            {
                if (obs[i].has_halo == -1)
                {
                    // draw the sphere of influence on the map
                    obs[i].has_halo = robot_id;
                    obs[i].halo_radius = obs[i].radius + sensor_range;

                    if (highlight_repelling_obstacles)
                    {
                        DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
                    }
                }
            }
            else if (obs[i].has_halo == robot_id)
            {
                // erase it, has gone out of range
                obs[i].has_halo = -1;
                DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
            }


            if (dist <= (obs[i].radius + safety_margin))
            {
                if (obs[i].impacted == -1)
                {
                    // draw the impact (virtual collision)
                    obs[i].impacted = robot_id;
                    obs[i].impact_radius = obs[i].radius + safety_margin;

                    if (highlight_repelling_obstacles)
                    {
                        DrawImpact(obs[i].center.x, obs[i].center.y, obs[i].impact_radius);
                    }
                }
            }
            else if (obs[i].impacted == robot_id)
            {
                // erase it, has gone out of range
                obs[i].impacted = -1;

                if (highlight_repelling_obstacles)
                {
                    DrawImpact(obs[i].center.x,obs[i].center.y,obs[i].impact_radius);
                }
            }
        }
    }
    return out;
}


  /*    T_robot_state*  cur;
    Vector          v;
    double          dist;
    int             num_within_range;
    int             i;
    int             pos;
    char*           val;
    double          safety_margin = 0;
    sim_obstacle_t* line_obs;
    sim_obstacle_t* tmp;
    point2d_t       rcenter;
    int             num_line_obs;
    static sim_obstacle_t* last_line_obs = NULL;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in detect_obstacles\n", robot_id);
        Obs_List out(0);
        return out;
    }

    // count number of obstacles within range
    num_within_range = 0;
    for (i = 0; i < num_obs; i++)
    {
        v.x = obs[i].center.x - cur->loc.x;
        v.y = obs[i].center.y - cur->loc.y;
	v.z = (double)obs[i].height + obshighpts[i];
        dist = len_2d(v);

        if (dist <= obs[i].radius + sensor_range)
        {
            num_within_range++;
        }
    }

    // Get the obstacles from line objects and their count
    rcenter.x = cur->loc.x;
    rcenter.y = cur->loc.y;
    line_obs = simulateSonarLineDetection(&rcenter, cur->heading,
                                           gConsoleDBLineList, NUM_SIM_SONAR,
                                           sensor_range, &num_line_obs);

    // Erase and delete the old obstacles

    while (last_line_obs != NULL)
    {
        if (highlight_repelling_obstacles)
        {
            DrawImpact(last_line_obs->center[0], last_line_obs->center[1], 0.3);
        }

        tmp = last_line_obs;
        last_line_obs = last_line_obs->next;
        free(tmp);
    }
    // Draw the new obstacles
    for (tmp = line_obs; tmp != NULL; tmp = tmp->next)
    {
        if (highlight_repelling_obstacles)
        {
            DrawImpact(tmp->center[0], tmp->center[1], 0.3);
        }
    }

    // Store the new obstacles for next time
    last_line_obs = line_obs;

    // Add to the count the line obstacles
    num_within_range += num_line_obs;

    // allocate storage for them
    Obs_List out(num_within_range);

    // if want graphics, then get the safety margin to draw
    if (highlight_repelling_obstacles)
    {
        if ((val = gt_inquire(robot_id, SAFETY_MARGIN_MSG)) != NULL)
        {
            safety_margin = atof(val);
        }
    }

    // return the obstacles within range
    pos = 0;

    while (line_obs != NULL)
    {
        v.x = line_obs->center[0] - cur->loc.x;
        v.y = line_obs->center[1] - cur->loc.y;
        rotate_z(v, -1 * cur->heading);
        out.obs[pos].x = v.x;
        out.obs[pos].y = v.y;
        out.r[pos] = 0.1;
        pos++;
        line_obs = line_obs->next;
    }
    for (i = 0; i < num_obs; i++)
    {
        v.x = obs[i].center.x - cur->loc.x;
        v.y = obs[i].center.y - cur->loc.y;

        dist = len_2d(v);

        if (dist <= (obs[i].radius + sensor_range))
        {
            // Make vector egocentric
            rotate_z(v, -1 * cur->heading);

            // Save the vector
            out.obs[pos].x = v.x;
            out.obs[pos].y = v.y;
            out.r[pos] = obs[i].radius;
            pos++;
        }

        if (highlight_repelling_obstacles)
        {
            // draw the sphere of influence and impacts on the map

            if (dist <= (obs[i].radius + sensor_range))
            {
                if (obs[i].has_halo == -1)
                {
                    // draw the sphere of influence on the map
                    obs[i].has_halo = robot_id;
                    obs[i].halo_radius = obs[i].radius + sensor_range;

                    if (highlight_repelling_obstacles)
                    {
                        DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
                    }
                }
            }
            else if (obs[i].has_halo == robot_id)
            {
                // erase it, has gone out of range
                obs[i].has_halo = -1;
                DrawHalo(obs[i].center.x, obs[i].center.y, obs[i].halo_radius);
            }


            if (dist <= (obs[i].radius + safety_margin))
            {
                if (obs[i].impacted == -1)
                {
                    // draw the impact (virtual collision)
                    obs[i].impacted = robot_id;
                    obs[i].impact_radius = obs[i].radius + safety_margin;

                    if (highlight_repelling_obstacles)
                    {
                        DrawImpact(obs[i].center.x, obs[i].center.y, obs[i].impact_radius);
                    }
                }
            }
            else if (obs[i].impacted == robot_id)
            {
                // erase it, has gone out of range
                obs[i].impacted = -1;

                if (highlight_repelling_obstacles)
                {
                    DrawImpact(obs[i].center.x,obs[i].center.y,obs[i].impact_radius);
                }
            }
        }
    }
   return out;
}
  */


//------------------------------------------------------------------

bool probably_robot(int my_id, Vector pos)
{
    T_robot_state* cur;
    int            i;
    const double MIN_PROB_DIST = 1.0; // Meters: center of robot to center of obs

    cur = robots;
    i = 0;
    while (cur)
    {
        if ((cur->robot_id != my_id) && cur->alive && (cur->run_type == REAL))
        {
            double dist = vector_length(pos.x - cur->loc.x, pos.y - cur->loc.y);
            if (dist < MIN_PROB_DIST)
            {
                return true;
            }
        }

        cur = cur->next;
    }

    return false;
}

/*
GC wch_lookup_color(const char* color)
{
    GC gc;

    XColor colorRec, ignore;
    Display* dpy = XtDisplay(main_window);
    int scr = DefaultScreen(dpy);
    Colormap cmap = DefaultColormap(dpy, scr);

    if (!XAllocNamedColor(dpy, cmap, color, &colorRec, &ignore))
    {
        warn_userf("Unable to allocate object color '%s': using black\nNote: on ly "
                    "color names existing in the file \"/usr/lib/X11/rgb.txt\" are legal.",
                    color);
        colorRec.pixel = BlackPixel(dpy, scr);
    }

    XGCValues xgcv;
    xgcv.foreground = colorRec.pixel;
    gc = XtGetGC(drawing_area, GCForeground, &xgcv);
    return gc;
}
*/

// sim_report_laser
void sim_report_laser(int robot_id, int num_readings, float* readings, float* angles)
{
    T_robot_state* cur;
    int i;
    Vector v;
    static GC whiteGC,blueGC;
    double readang;
    static float* lastreadings = NULL;
    static int firsttime = true;
    int x, y, num_points;
    XPoint* points = NULL;

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_report_laser\n", robot_id);
        return;
    }

    if (firsttime)
    {
        //whiteGC = wch_lookup_color("white");
        //blueGC = wch_lookup_color("midnight blue");
        whiteGC = getGCByColorName("white");
        blueGC = getGCByColorName("midnight blue");

        // These are never deallocated since they hang around for the life of
        // the program.
        if (num_readings > 0)
        {
            lastreadings = new float[num_readings];
        }
    }

    if (num_readings > 0)
    {
        points = new XPoint[num_readings];
    }

    num_points = 0;

    if (erase_obstacles && !firsttime)
    {
        if (laser_normal_mode)
        {
            for (i = 0; i < num_readings; i++)
            {
                if (readings[i] == lastreadings[i])
                {
                    continue;
                }
                readang = angles[i];
                v.x = lastreadings[i];
                v.y = 0;
                readang += cur->heading;
                rotate_z(v, readang);
                v.x += cur->loc.x;
                v.y += cur->loc.y;
                x = (int) (v.x / meters_per_pixel);
                y = (int) ((mission_area_height_meters - v.y) / meters_per_pixel);
                points[num_points + i].x = x;
                points[num_points + i].y = y;
            }
            if (laser_connected_mode)
            {
                XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                            whiteGC, &points[num_points], num_readings, CoordModeOrigin);
            }
            num_points += num_readings;
        }
        if (laser_linear_mode)
        {
            for (i = 0; i < num_readings; i++)
            {
                if (readings[i] == lastreadings[i])
                {
                    continue;
                }
                x = 10 + i;
                y = (int) ((mission_area_height_meters - (lastreadings[i] + 1)) / meters_per_pixel);
                points[num_points + i].x = x;
                points[num_points + i].y = y;
            }
            if (laser_connected_mode)
            {
                XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                            whiteGC, &points[num_points], num_readings, CoordModeOrigin);
            }
            num_points += num_readings;
        }
    }

    if (!laser_connected_mode)
    {
        XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                     whiteGC, points, num_points, CoordModeOrigin);
    }

    num_points = 0;
    if (laser_normal_mode)
    {
        for (i = 0; i < num_readings; i++)
        {
            readang = angles[i];
            v.x = readings[i];
            v.y = 0;
            readang += cur->heading;
            rotate_z(v, readang);
            v.x += cur->loc.x;
            v.y += cur->loc.y;
            x = (int) (v.x / meters_per_pixel);
            y = (int) ((mission_area_height_meters - v.y) / meters_per_pixel);
            points[num_points + i].x = x;
            points[num_points + i].y = y;
        }
        if (laser_connected_mode)
        {
            XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                        blueGC, &points[num_points], num_readings, CoordModeOrigin);
        }
        num_points += num_readings;
    }
    if (laser_linear_mode)
    {
        for (i = 0; i < num_readings; i++)
        {
            x = 10 + i;
            y = (int) ((mission_area_height_meters - (readings[i] + 1)) / meters_per_pixel);
            points[num_points + i].x = x;
            points[num_points + i].y = y;
        }
        if (laser_connected_mode)
        {
            XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                        blueGC, &points[num_points], num_readings, CoordModeOrigin);
        }
        num_points += num_readings;
    }

    if (!laser_connected_mode)
    {
        XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                     blueGC, points, num_points, CoordModeOrigin);
    }

    for (i = 0; i < num_readings; i++)
    {
        lastreadings[i] = readings[i];
    }

    firsttime = false;

    if (points != NULL)
    {
        delete [] points;
    }
}

/****************************************************************************
*                                                                           *
* sim_report_sensors                                                        *
*                                                                           *
****************************************************************************/

void sim_report_sensors(int robot_id, int cnt, SensorReading* reading)
{
#define SENSOR_SICKLMS200      0
#define SENSOR_SONAR           1
#define SENSOR_IR              2
#define SENSOR_XYT             3
#define SENSOR_PANTILT_CAMERA  4
#define SENSOR_COLOR_OBJECTS   5
#define SENSOR_ARM             6
#define SENSOR_VIDEO           7
#define SENSOR_CAMERA_FOCUS    8

    T_robot_state* cur;
    static int initialized = 0;
    static int lastCnt;
    static float lastHeading;
    static gt_Point lastLoc;
    static int frames = 0;

    static int num_laser_points = 0;
    static int num_laser_danger_points = 0;
    static int num_laser_linear_points = 0;
    static XPoint* laser_danger_points = NULL;
    static XPoint* laser_points = NULL;
    static XPoint* laser_linear_points = NULL;

    static int num_color_points = 0;
    static XPoint color_points[32];
    int color_point_colors[32];

    static int num_sonar_points = 0;
    static XPoint sonar_points[32];
    static XArc sonar_arcs[32];
    int x2,y2,d;
    static int erase_sonar_points = 0,erase_sonar_arcs = 0;

    int i,x,y;
    double angle;
    Vector v;

    static GC whiteGC;
    static GC colorGC[7];
    char color_names[7][20] = {
        "blue", // laser
        "orange",         // sonar, arm
        "black",        // ir
        "red","green","yellow", // color objects
        "purple",        // pan/tilt camera, camera_focus
    };

    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        fprintf(stderr, "server: bad robot number %d in sim_report_obstacles\n", robot_id);
        return;
    }

    frames++;
    if((frames%30)==0)
    {
        clear_map();
        draw_world();
    }

    if (!initialized)
    {
        int laser_count = 0;

        initialized = 1;

        // get the number of laser readings
        for (i = 0; i < cnt; i++)
        {
            if (reading[i].type == SENSOR_SICKLMS200)
            {
                laser_count++;
            }
        }
        // allocate space for laser reading data. These are never deallocated
        // since they stay around for the entire life of the program.
        if (laser_count > 0)
        {
            laser_danger_points = new XPoint[laser_count];
            laser_points        = new XPoint[laser_count];
            laser_linear_points = new XPoint[laser_count];
        }

        lastCnt = cnt;
        lastLoc.x = cur->loc.x;
        lastLoc.y = cur->loc.y;
        lastHeading = cur->heading;
        for (i = 0; i < 7; i++)
        {
            //colorGC[i] = wch_lookup_color(color_names[i]);
            colorGC[i] = getGCByColorName(color_names[i]);
        }
        //whiteGC = wch_lookup_color("white");
        whiteGC = getGCByColorName("white");
    }
    else if(laser_points == NULL)
    {
    	int laser_count = 0;

		// get the number of laser readings
		for (i = 0; i < cnt; i++)
		{
			if (reading[i].type == SENSOR_SICKLMS200)
			{
				laser_count++;
			}
		}

		if (laser_count > 0)
		{
			laser_danger_points = new XPoint[laser_count];
			laser_points        = new XPoint[laser_count];
			laser_linear_points = new XPoint[laser_count];
		}
    }

    /*if (erase_obstacles)
    {
        if (num_laser_points)
        {
            if (laser_normal_mode)
            {
                if (laser_connected_mode)
                {
                    XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                                whiteGC, laser_points, num_laser_points, CoordModeOrigin);
                    XDrawLines(XtDisplay(drawing_area), drawing_area_pixmap,
                                whiteGC, laser_points, num_laser_points, CoordModeOrigin);
                }
                else
                {
                    XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                                 whiteGC, laser_points, num_laser_points, CoordModeOrigin);
                    XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                                 whiteGC, laser_danger_points, num_laser_danger_points,
                                 CoordModeOrigin);

                    XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                                 whiteGC, laser_points, num_laser_points, CoordModeOrigin);
                    XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                                 whiteGC, laser_danger_points, num_laser_danger_points,
                                 CoordModeOrigin);
                }
            }
        }
        if (num_laser_linear_points)
        {
            if (laser_linear_mode)
            {
                XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                             whiteGC, laser_linear_points, num_laser_linear_points,
                             CoordModeOrigin);
                XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                             whiteGC, laser_linear_points, num_laser_linear_points,
                             CoordModeOrigin);
            }
        }
        if (num_color_points)
        {
            for (i = 0; i < num_color_points; i++)
            {
                XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area),
                          whiteGC, color_points[i].x, color_points[i].y,
                          10, 10, 0, 360 * 64);
                XFillArc(XtDisplay(drawing_area), drawing_area_pixmap,
                          whiteGC, color_points[i].x, color_points[i].y,
                          10, 10, 0, 360 * 64);
            }
        }
        if (num_sonar_points)
        {
            if (erase_sonar_arcs)
            {
                XDrawArcs(XtDisplay(drawing_area), XtWindow(drawing_area),
                           whiteGC, sonar_arcs, num_sonar_points);
                XDrawArcs(XtDisplay(drawing_area), drawing_area_pixmap,
                           whiteGC, sonar_arcs, num_sonar_points);
                erase_sonar_arcs = 0;
            }
            if (erase_sonar_points)
            {
                for (i = 0; i < num_sonar_points; i++)
                {
                    XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area),
                              whiteGC, sonar_points[i].x, sonar_points[i].y,
                              5, 5, 0, 360 * 64);
                    XFillArc(XtDisplay(drawing_area), drawing_area_pixmap,
                              whiteGC, sonar_points[i].x, sonar_points[i].y,
                              5, 5, 0, 360 * 64);
                }
                erase_sonar_points = 0;
            }
        }
    }*/

    num_laser_points = 0;
    num_laser_danger_points = 0;
    num_laser_linear_points = 0;
    num_color_points = 0;
    num_sonar_points = 0;

    for (i = 0; i < cnt; i++)
    {
        v.x = reading[i].reading;
        v.y = 0;
        angle = cur->heading + reading[i].angle;
        deg_range_pm_360(angle);
        rotate_z(v, angle);
        v.x += cur->loc.x;
        v.y += cur->loc.y;
        x = (int) (v.x / meters_per_pixel);
        y = (int) ((mission_area_height_meters - v.y) / meters_per_pixel);
        switch (reading[i].type)
        {
        case SENSOR_SICKLMS200:
            if (laser_connected_mode)
            {
                laser_points[num_laser_points].x = x;
                laser_points[num_laser_points].y = y;
                num_laser_points++;
            }
            else
            {
                if (reading[i].color)
                {
                    laser_danger_points[num_laser_danger_points].x = x;
                    laser_danger_points[num_laser_danger_points].y = y;
                    num_laser_danger_points++;
                }
                else
                {
                    laser_points[num_laser_points].x = x;
                    laser_points[num_laser_points].y = y;
                    num_laser_points++;
                }
            }
            if (laser_linear_mode)
            {
                laser_linear_points[num_laser_linear_points].x =
                    10 + num_laser_linear_points;
                laser_linear_points[num_laser_linear_points].y =
                    (int)((mission_area_height_meters-(reading[num_laser_linear_points].reading + 1)) / meters_per_pixel);
                num_laser_linear_points++;
            }
            break;
        case SENSOR_COLOR_OBJECTS:
            color_point_colors[num_color_points] = reading[i].color + 3;
            color_points[num_color_points].x = x;
            color_points[num_color_points].y = y;
            num_color_points++;
            break;
        case SENSOR_SONAR:
            sonar_points[num_sonar_points].x = x;
            sonar_points[num_sonar_points].y = y;
            if (sonar_arc_mode) {
                d = (int) (reading[i].reading / meters_per_pixel);
                x2 = (int) (cur->loc.x / meters_per_pixel);
                y2 = (int) ((mission_area_height_meters - cur->loc.y) / meters_per_pixel);
                sonar_arcs[num_sonar_points].width = (int) (d * 2);
                sonar_arcs[num_sonar_points].height = (int) (d * 2);
                sonar_arcs[num_sonar_points].angle1 = (short) ((angle - 15.0) * 64.0);
                sonar_arcs[num_sonar_points].angle2 = 30 * 64;
                sonar_arcs[num_sonar_points].x = x2 - d;
                sonar_arcs[num_sonar_points].y = y2 - d;
            } else {
                sonar_points[num_sonar_points].x = x;
                sonar_points[num_sonar_points].y = y;
            }
            num_sonar_points++;
            break;
        }
    }

    if (num_laser_points)
    {
	    if (laser_normal_mode)
        {
            if (laser_connected_mode)
            {
                XDrawLines(XtDisplay(drawing_area), XtWindow(drawing_area),
                            colorGC[0], laser_points, num_laser_points,CoordModeOrigin);
                XDrawLines(XtDisplay(drawing_area), drawing_area_pixmap,
                            colorGC[0], laser_points, num_laser_points,CoordModeOrigin);
            } else {
                XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                             colorGC[0], laser_points, num_laser_points,
                             CoordModeOrigin);
                XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                             colorGC[3], laser_danger_points, num_laser_danger_points,
                             CoordModeOrigin);

                XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                             colorGC[0], laser_points, num_laser_points,
                             CoordModeOrigin);
                XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                             colorGC[3], laser_danger_points, num_laser_danger_points,
                             CoordModeOrigin);
            }
	    }
    }
    if (num_laser_linear_points)
    {
        if (laser_linear_mode)
        {
            XDrawPoints(XtDisplay(drawing_area), XtWindow(drawing_area),
                         colorGC[0], laser_linear_points, num_laser_linear_points,
                         CoordModeOrigin);
            XDrawPoints(XtDisplay(drawing_area), drawing_area_pixmap,
                         colorGC[0], laser_linear_points, num_laser_linear_points,
                         CoordModeOrigin);
        }
    }

    if (num_color_points)
    {
        for (i = 0; i < num_color_points; i++)
        {
            XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area),
                      colorGC[color_point_colors[i]], color_points[i].x, color_points[i].y,
                      10, 10, 0, 360 * 64);
            XFillArc(XtDisplay(drawing_area), drawing_area_pixmap,
                      colorGC[color_point_colors[i]], color_points[i].x, color_points[i].y,
                      10, 10, 0, 360 * 64);
        }
    }

    if (num_sonar_points)
    {
        if (sonar_arc_mode)
        {
            XDrawArcs(XtDisplay(drawing_area), XtWindow(drawing_area),
                       colorGC[1], sonar_arcs, num_sonar_points);
            XDrawArcs(XtDisplay(drawing_area), drawing_area_pixmap,
                       colorGC[1], sonar_arcs, num_sonar_points);
            erase_sonar_arcs = 1;
        }
        if (sonar_point_mode)
        {
            for (i = 0; i < num_sonar_points; i++)
            {
                XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area),
                          colorGC[1], sonar_points[i].x, sonar_points[i].y,
                          5, 5, 0, 360 * 64);
                XFillArc(XtDisplay(drawing_area), drawing_area_pixmap,
                          colorGC[1], sonar_points[i].x, sonar_points[i].y,
                          5, 5, 0, 360 * 64);
                erase_sonar_points = 1;
            }
        }
    }

    if (cur->drawing_vector_field)
    {
        if (g_suBackupObs != NULL)
        {
            delete [] g_suBackupObs;
            g_suBackupObs = NULL;
            g_iBackupObsCount = 0;
        }

        if (num_obs > 0)
        {
            g_iBackupObsCount = num_obs;
            g_suBackupObs = obs;
        }

        num_obs = num_sonar_points + num_laser_points +
            num_laser_danger_points + num_laser_linear_points;
        if (num_obs > 0)
        {
            obs = new obs_t[num_obs];

            int iObsIndex = 0;
            for (i = 0; i < cnt; i++)
            {
                switch(reading[i].type)
                {
                case SENSOR_SONAR:
                case SENSOR_SICKLMS200:
                    v.x = reading[i].reading;
                    v.y = 0;
                    angle = cur->heading + reading[i].angle;
                    deg_range_pm_360(angle);
                    rotate_z(v, angle);
                    obs[iObsIndex].center.x = v.x + cur->loc.x;
                    obs[iObsIndex].center.y = v.y + cur->loc.y;
                    obs[iObsIndex].radius = 0.0;
                    iObsIndex++;
                    break;
                }
            }
        }

        cur->sensors_received = true;
    }
}


/****************************************************************************
*                                                                           *
* detect_robots                                                             *
*    construct readings of surrounding robots                               *
*                                                                           *
****************************************************************************/

RobotReadings sim_detect_robots(int robot_id, double max_dist) {

    T_robot_state* cur;
    T_robot_state* ptr;
    Vector v;
    int i;

    // Count number of robots within range so can allocate array
    int num = 0;
    if ((cur = find_robot_rec(robot_id)) != NULL)
    {
        ptr = robots;
        while (ptr)
        {
            // skip ourselves
            if (ptr->robot_id != robot_id)
            {
                v.x = ptr->loc.x - cur->loc.x;
                v.y = ptr->loc.y - cur->loc.y;

                if (len_2d(v) <= max_dist)
                {
                    num++;
                }
            }

            ptr = ptr->next;
        }

        // Allocate the array to hold the readings
        RobotReadings rtn(num);

        // this is for checking if a robot is hidden by a wall and therefore
        // it cannot be detected (similar to the sim_detect_object function)
        point2d_t rcenter, dcenter;
        double ang, dist;
        sim_obstacle_t* line_obs;
        int num_line_obs;
        rcenter.x = cur->loc.x;
        rcenter.y = cur->loc.y;

        // Load the return array
        ptr = robots;
        i = 0;
        while (ptr)
        {
            // skip ourselves
            if (ptr->robot_id != robot_id)
            {
                v.x = ptr->loc.x - cur->loc.x;
                v.y = ptr->loc.y - cur->loc.y;

                if (len_2d(v) <= max_dist)
                {

                    // this is for checking if a robot is hidden by a wall and therefore
                    // it cannot be detected (similar to the sim_detect_object function)
                    bool can_see = true;
                    dcenter.x = ptr->loc.x;
                    dcenter.y = ptr->loc.y;
                    ang = atan2((dcenter.y - rcenter.y), (dcenter.x - rcenter.x));
                    ang = DEGREES_FROM_RADIANS(ang);
                    deg_range_pm_360(ang);
                    line_obs = simulateSonarLineDetection(&rcenter, ang,
                                                           gConsoleDBLineList, 1,
                                                           max_dist, &num_line_obs);
                    if (line_obs != NULL)
                    {
                        // We collided with a wall, determine if it is closer than the object
                        Vector vwall;
                        vwall.x = line_obs->center[0] - rcenter.x;
                        vwall.y = line_obs->center[1] - rcenter.y;
                        free(line_obs);
                        dist = cartesianDistance(&rcenter, &dcenter);
                        if (dist > len_2d(vwall))
                        {
                            can_see = false;
                        }
                        else
                        {
                            can_see = true;
                        }
                    }
                    if (can_see)
                    {
                        // Make vector egocentric
                        rotate_z(v, -1 * cur->heading);

                        rtn.readings[i].v.x = v.x;
                        rtn.readings[i].v.y = v.y;
                        rtn.readings[i].id = ptr->robot_id;
                        i++;
                    }
                }
            }

            ptr = ptr->next;
        }

        rtn.num_readings = i;

        return rtn;

    }
    else
    {
        RobotReadings rtn;
        return rtn;
    }
}

/****************************************************************************
*                                                                           *
* sim_get_robot_pos                                                         *
*    construct readings of surrounding robots                               *
*                                                                           *
****************************************************************************/
RobotReadings sim_get_robot_pos(int robot_id, double max_dist)
{
    T_robot_state* cur;
    T_robot_state* ptr;
    Vector v;

    // Count number of robots within range so can allocate array
    int num = 0;
    if ((cur = find_robot_rec(robot_id)) != NULL)
    {
        ptr = robots;
        while (ptr)
        {
            // skip ourselves
            if (ptr->robot_id != robot_id)
            {
                v.x = ptr->loc.x - cur->loc.x;
                v.y = ptr->loc.y - cur->loc.y;

                if (len_2d(v) <= max_dist)
                    num++;
            }

            ptr = ptr->next;
        }

        // Allocate the array to hold the readings
        RobotReadings rtn(num);

        num=0;
        ptr = robots;
		while (ptr)
		{
			// skip ourselves
			if (ptr->robot_id != robot_id)
			{
				v.x = ptr->loc.x - cur->loc.x;
				v.y = ptr->loc.y - cur->loc.y;

				if (len_2d(v) <= max_dist)
				{
					rtn.readings[num].id = ptr->robot_id;
                    rtn.readings[num].v.x = ptr->loc.x;
                    rtn.readings[num].v.y = ptr->loc.y;
                    rtn.readings[num].v.z = 0;
                    rtn.readings[num].heading = ptr->heading;
					num++;
				}
			}

			ptr = ptr->next;
		}
		return rtn;
    }
    else
    {
    	RobotReadings rtn;
        return rtn;
    }
}

/****************************************************************************
*                                                                           *
* detect_objects                                                            *
*    construct readings of surrounding objects of all types                 *
*                                                                           *
****************************************************************************/

ObjectList sim_detect_objects(int robot_id, double max_dist)
{
    point2d_t rcenter, dcenter;
    double ang, dist;
    Vector v;
    sim_obstacle_t* line_obs;

    T_robot_state* cur;
    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        ObjectList rtn(0);
        return rtn;
    }

    // Construct a list of the objects within the max_distance
    GTList<shape*> objlist;
    void* pos;
    shape* p;
    rcenter.x = cur->loc.x;
    rcenter.y = cur->loc.y;

    if ((pos = object_list.first(&p)))
    {
        do
        {
            // If the object is not hidden, and is within sensor range,
            // and is not this robot, then return a reading to it.
            if (!p->is_hidden_ &&
                 p->distance(cur->loc.x, cur->loc.y) <= max_dist &&
                 p->id_ != cur->object_ptr->id_)
            {
                int num_line_obs;

                p->center(dcenter.x, dcenter.y);
                ang = atan2((dcenter.y - rcenter.y), (dcenter.x - rcenter.x));
                ang = DEGREES_FROM_RADIANS(ang);
                deg_range_pm_360(ang);

                // We found a object, might be occluded by walls. So
                // fire a single ray in the direction of the object at the wall objects
                line_obs = simulateSonarLineDetection(&rcenter, ang,
                                                       gConsoleDBLineList, 1,
                                                       max_dist, &num_line_obs);
                if (line_obs != NULL)
                {
                    // We collided with a wall, determine if it is closer than the object
                    v.x = line_obs->center[0] - rcenter.x;
                    v.y = line_obs->center[1] - rcenter.y;
                    free(line_obs);
                    dist = cartesianDistance(&rcenter, &dcenter);
                    if (dist > len_2d(v)) // the wall is closer
                    {
                        continue;
                    }
                }

                objlist.append(p);

            }
        } while ((pos = object_list.next(&p, pos)));
    }

    // Allocate the array for the return objects
    ObjectList rtn(objlist.len());

    // Load the array
    int i = 0;
    if ((pos = objlist.first(&p)))
    {
        do
        {
            rtn.objects[i++] = p->to_reading(cur->loc.x, cur->loc.y, cur->heading);
        } while ((i < rtn.count) && (pos = objlist.next(&p, pos)) != NULL);
    }

    return rtn;
}


/**********************************************************************/

void gt_init_simulation(char* ipt_home)
{
    gt_init_communication(ipt_home);
}

// send the 3d prog the obstacle info
void initialize_3d_vis(void)
{
    Obs_3d_Rec tempobs;
    Wall_List_Element *temprec;
    rc_chain *modelDirList = NULL;
    string modelPath = "./";
    char *modelDir = NULL;
    int cnt;

    // create the obstacle and robot handler
    if (Obs_Handler != NULL)
    {
        delete Obs_Handler;
        Obs_Handler=NULL;
    }

    Obs_Handler = new ObstacleHandler(Terrain);

    if (Rob_Handler != NULL)
    {
        delete Rob_Handler;
        Rob_Handler=NULL;
    }

    // Find the path to the 3D models
    modelDirList = (rc_chain *)rc_table.get(RCTABLE_3D_MODEL_DIR_STRING.c_str());

    if (modelDirList != NULL)
    {
        modelDirList->first(&modelDir);
    }

    if (modelDir != NULL)
    {
        modelPath = modelDir;
    }

    // Instantiate the handler
    Rob_Handler = new RobotHandler(modelPath);

    if (obshighpts != NULL)
    {
        delete []obshighpts;
    }

    obshighpts = new int[num_obs];

    // loop through the obstacles sending each one to the 3d vis program
    for (cnt = 0; cnt < num_obs; cnt++)
    {
        // throw it in a structure for readability
        tempobs.X = obs[cnt].center.x;
        // we need to offset because the origin for the vis software is top left
        tempobs.Z = heightmap_y - obs[cnt].center.y;
        tempobs.Height = obs[cnt].height;
        tempobs.Radius = obs[cnt].radius;
        // throw our obstacle into the obstcale handler
        obshighpts[cnt] = (int)(Obs_Handler->Place_Obstacle(
                                    tempobs.X,
                                    tempobs.Z,
                                    tempobs.Height,
                                    tempobs.Radius));

        obs[cnt].trueheight = obshighpts[cnt] + obs[cnt].height;

    }
    if (Wall_List != NULL)
    {
        Wall_List->Reset_List();

        do {
            temprec = (Wall_List_Element *)Wall_List->Get_Data();

            Obs_Handler->Place_Obstacle(
                temprec->Walls,
                temprec->Height,
                temprec->HighPt);
        } while (Wall_List->Next_Node());
    }
}

// 3d Vis   this function provides the elvation for a given map point;
int sim_get_elevation(int x, int y) {

  if (CurHeightMap == 0 || x < 0 || y < 0 || x > 255  || y > 255) return 0;
  return (int)(Terrain->Get_Height(x,y));
}


// function to load the height map named 'name' from a file
bool load_heightmap(char *name, int x, int y, int sl, int scl)
{
	//int cnt;
	//FILE *infile;
	//unsigned char *temphv;
	heightmap_name = name;
	heightmap_x = x;
	heightmap_y = y;

	if(Terrain != NULL)
		delete Terrain;

	Terrain = new TerrainMap(x, y, sl, scl);
	Terrain->Load_HeightMap(name);

	init3DVisEye();

	return true;
}

bool is_valid_terrain(int robot_id) {

    T_robot_state* cur;
    if ((cur = find_robot_rec(robot_id)) == NULL)
    {
        return false;
    }

    if (cur->mobility_type == UUV) {
      if ((Terrain->Is_Water(cur->loc.x, cur->loc.y)) && (cur->loc.z <= Terrain->Get_SeaLevel())) {
           return true;
      }
      else {
           return false;
      }
    }

    else if (cur->mobility_type == USV) {
      if ((Terrain->Is_Water(cur->loc.x, cur->loc.y)) && (cur->loc.z = Terrain->Get_SeaLevel())) {
           return true;
      }
      else {
         return false;
      }
    }

    return true;

}

/**********************************************************************
 * $Log: simulation_server.c,v $
 * Revision 1.2  2008/07/16 21:16:09  endo
 * Military style is now a toggle option.
 *
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.12  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.11  2007/08/24 22:22:34  endo
 * Draw_Neutral ghosting problem fixed.
 *
 * Revision 1.10  2007/08/17 20:36:35  endo
 * Fixing ghosting problem.
 *
 * Revision 1.9  2007/08/17 18:38:45  endo
 * Cleaning up military display.
 *
 * Revision 1.8  2007/08/16 19:21:43  endo
 * CVS log entry fixed.
 *
 *
 * Revision 1.3  2007/08/03 19:41:44  pulam
 * Added military icons for robots
 *
 * Revision 1.4  2007/08/05 03:12:41  pulam
 * Bugfix for drawing units after localization
 *
 * Revision 1.3  2007/08/03 19:41:44  pulam
 * Added military icons for robots
 *
 * Revision 1.2  2007/02/08 19:40:43  pulam
 * Added handling of zones (no-fly, weather)
 * Fixed up some of the icarus objectives to add turn limiting
 *
 * Revision 1.1.1.1  2006/07/20 17:17:50  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:59  endo
 * MissionLab 7.0
 *
 * Revision 1.30  2006/07/12 06:16:42  endo
 * Clean-up for MissionLab 7.0 release.
 *
 * Revision 1.29  2006/07/11 17:15:31  endo
 * JBoxMlab merged from MARS 2020.
 *
 * Revision 1.28  2006/07/11 06:39:47  endo
 * Cut-Off Feedback functionality merged from MARS 2020.
 *
 * Revision 1.27  2006/06/29 21:48:44  pulam
 * Modification for visibility and uncertainty
 *
 * Revision 1.26  2006/06/29 19:12:06  endo
 * Fix for FC5. LD_POINTER_GUARD flag is set, so that longjmp() called in Cthreads doesn't crash.
 *
 * Revision 1.25  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.24  2006/05/15 02:07:35  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.23  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.22  2006/05/06 04:25:14  endo
 * Bugs of TrackTask fixed.
 *
 * Revision 1.21  2006/05/02 04:19:59  endo
 * TrackTask improved for the experiment.
 *
 * Revision 1.20  2006/03/05 23:27:20  pulam
 * Addition of SL-UAV code and CNP code for calculating bids for intercept/inspect tasks
 *
 * Revision 1.19  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.18  2006/01/10 06:10:31  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.17  2005/08/22 22:14:10  endo
 * For Pax River demo.
 *
 * Revision 1.16  2005/08/09 19:14:00  endo
 * Things improved for the August demo.
 *
 * Revision 1.15  2005/08/01 19:12:20  endo
 * *** empty log message ***
 *
 * Revision 1.14  2005/07/31 03:40:43  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.13  2005/07/30 03:01:58  endo
 * 3DS moved to terrain/models.
 *
 * Revision 1.12  2005/07/30 02:17:48  endo
 * 3D visualization improved.
 *
 * Revision 1.11  2005/07/28 04:55:16  endo
 * 3D visualization improved more.
 *
 * Revision 1.10  2005/07/27 20:38:10  endo
 * 3D visualization improved.
 *
 * Revision 1.9  2005/07/23 05:58:20  endo
 * 3D visualization improved.
 *
 * Revision 1.8  2005/07/16 08:44:07  endo
 * CBR-CNP integration
 *
 * Revision 1.7  2005/06/07 23:35:09  alanwags
 * cnp revision for premission specification
 *
 * Revision 1.6  2005/05/18 21:09:48  endo
 * AuRA.naval added.
 *
 * Revision 1.5  2005/04/08 01:56:04  pulam
 * addition of cnp behaviors
 *
 * Revision 1.4  2005/03/27 01:19:37  pulam
 * addition of uxv behaviors
 *
 * Revision 1.3  2005/03/23 07:36:56  pulam
 * *** empty log message ***
 *
 * Revision 1.2  2005/02/07 23:12:35  endo
 * Mods from usability-2004
 *
 * Revision 1.1.1.1  2005/02/06 23:00:11  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.109  2003/06/19 20:35:15  endo
 * playback feature added.
 *
 * Revision 1.108  2002/10/31 20:05:29  ebeowulf
 * Added the ability to change the color of a robot.
 *
 * Revision 1.107  2002/10/08 16:55:49  blee
 * Remove a wait3() call since reaping is now done from a signal handler.
 *
 * Revision 1.106  2002/07/02 20:41:24  blee
 * made changes so mlab could draw vector fields
 *
 * Revision 1.105  2002/04/04 18:39:00  blee
 * Changed declaration of last_shape_id to make it a static member of shape.
 *
 * Revision 1.104  2002/01/12 23:00:58  endo
 * Mission Expert functionality added.
 *
 * Revision 1.103  2001/12/22 16:20:28  endo
 * RH 7.1 porting.
 *
 * Revision 1.102  2000/12/12 22:42:09  blee
 * Altered sim_report_laser() to accept angles and
 * dynamically allocate space.
 * Altered sim_report_sensors() to dynamically allocate
 * space for laser points.
 *
 * Revision 1.101  2000/11/20 19:33:54  sgorbiss
 * Add visualization of line-of-sight communication
 *
 * Revision 1.100  2000/11/10 23:38:08  blee
 * added learning_momentum_enabled. also added -r and -m options
 * to the robot executable's command line
 *
 * Revision 1.99  2000/08/15 20:56:59  endo
 * 'l' option to pass the laserfit machine name from .cfgeditrc added.
 *
 * Revision 1.98  2000/07/26 20:12:28  endo
 * The capability of MissionLab being able to FTP robot executable
 * was broken, and the mechanism of executing the robot executable
 * on a remote machine was hard coded to use /home/demo. This
 * modification fixes those.
 *
 * Revision 1.97  2000/07/02 01:16:42  conrad
 * *** empty log message ***
 *
 * Revision 1.96  2000/07/02 01:02:38  conrad
 * added sensor display
 *
 * Revision 1.95  2000/04/25 09:55:32  jdiaz
 * changes for birthed robot to take heading
 *
 * Revision 1.94  2000/04/13 22:33:33  endo
 * Checked in for Doug.
 * Fixes a compile problem with missing std.h
 *
 * Revision 1.93  2000/03/30 01:45:08  endo
 * sim_spin_robot added.
 *
 * Revision 1.92  2000/02/28 23:32:17  jdiaz
 * added mark/unmark door function and modified sim_detect_objects
 * to only return those objects not obscured by walls
 *
 * Revision 1.91  2000/02/19 01:40:39  endo
 * some debug statement disabled.
 *
 * Revision 1.90  2000/02/18 01:47:15  endo
 * Added sim_alert. Also, fixed the bug of the
 * simulation robot facing to East direction
 * when len_2d(abs_dist) < EPS_ZERO.
 *
 * Revision 1.89  2000/02/07 05:13:10  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.88  1999/12/18 09:43:47  endo
 * -S, -L for "report current state" and
 * "robot data logging" added.
 *
 * Revision 1.87  1999/12/16 22:48:35  mjcramer
 * RH6 port
 *
 * Revision 1.81  1999/07/03 21:55:36  dj
 * moved FTP and PPP setup to cfgedit
 *
 * Revision 1.80  1999/07/03 06:53:10  dj
 * re-enabled automatic FTP
 *
 * Revision 1.79  1999/07/03 01:09:25  mjcramer
 * Disabled automatic FTP and fixed rsh call
 *
 * Revision 1.78  1999/06/29 03:04:00  mjcramer
 * Added automatic ethernet detection, ftp downloading
 *
 * Revision 1.77  1999/04/27 02:23:11  conrad
 * Added instantaneous obstacle views.
 *
 * Revision 1.76  1997/02/12  05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.75  1997/01/15  21:30:03  zchen
 * add real 3d stuff
 *
 * Revision 1.74  1996/05/07  19:54:11  doug
 * fixing compiler warnings
 *
 * Revision 1.73  1996/03/18  19:12:41  doug
 * Fixed pickup robot so removes robot object from the list
 * immediently so won't get returned in subsequent sensor readings
 *
 * Revision 1.72  1996/03/13  01:52:58  doug
 * *** empty log message ***
 *
 * Revision 1.71  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.70  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.69  1996/03/04  22:51:58  doug
 * *** empty log message ***
 *
 * Revision 1.68  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.67  1996/03/01  00:47:09  doug
 * *** empty log message ***
 *
 * Revision 1.66  1996/02/29  01:53:18  doug
 * *** empty log message ***
 *
 * Revision 1.65  1996/02/27  05:00:04  doug
 * *** empty log message ***
 *
 * Revision 1.64  1996/02/22  00:56:22  doug
 * *** empty log message ***
 *
 * Revision 1.63  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.62  1995/11/12  22:08:27  doug
 * fixed so when robots die, the object record is removed
 *
 * Revision 1.61  1995/11/12  21:13:17  doug
 * fixed detect_objects robot reading code
 *
 * Revision 1.60  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.59  1995/10/23  20:17:30  doug
 * *** empty log message ***
 *
 * Revision 1.58  1995/10/23  19:54:34  doug
 * *** empty log message ***
 *
 * Revision 1.57  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.56  1995/10/16  21:33:52  doug
 * *** empty log message ***
 *
 * Revision 1.55  1995/10/12  20:13:34  doug
 * Added pickup function
 *
 * Revision 1.54  1995/10/11  20:35:47  doug
 * *** empty log message ***
 *
 * Revision 1.53  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.52  1995/08/31  15:59:10  kali
 * Made some changes to make movement vectors be shown on the screen
 * when running the real robots.
 *
 * Revision 1.51  1995/06/27  21:13:19  doug
 * *** empty log message ***
 *
 * Revision 1.50  1995/06/27  12:50:53  doug
 * *** empty log message ***
 *
 * Revision 1.49  1995/05/02  19:50:25  jmc
 * Changed work_work_*_meters to mission_area_*_meters.
 *
 * Revision 1.48  1995/05/02  18:54:35  jmc
 * Changed grid_*_meters to work_area_*_meters.
 *
 * Revision 1.47  1995/04/27  19:39:30  jmc
 * Converted GRID_HEIGHT/WIDTH_METERS variables to lower case.
 *
 * Revision 1.46  1995/04/26  22:02:40  doug
 * Converted from tcx to ipt.  Keep your fingers crossed!
 *
 * Revision 1.45  1995/04/20  18:36:43  jmc
 * Corrected spelling.
 *
 * Revision 1.44  1995/04/19  21:52:58  jmc
 * Fixed an insignificant problem that was pointed out by the latest
 * g++ compiler in -Wall mode.
 *
 * Revision 1.43  1995/04/10  15:53:39  jmc
 * Added include for draw.h.
 *
 * Revision 1.42  1995/04/03  20:39:30  jmc
 * Added copyright notice.
 *
 * Revision 1.41  1995/02/13  20:21:25  jmc
 * Tweaks to make -Wall compile quietly.
 *
 * Revision 1.40  1995/02/02  15:41:03  jmc
 * Changed show_movement_vector to show_movement_vectors.  (Made it
 * plural.)
 *
 * Revision 1.39  1995/02/02  15:28:41  jmc
 * Changed simulated_cycle_duration variable name to cycle_duration.
 *
 * Revision 1.38  1995/01/30  19:34:01  doug
 * detect obstacle code which made vectors egocentric needed to by heading * -1.
 *
 * Revision 1.37  1995/01/30  15:45:20  doug
 * porting to linux
 *
 * Revision 1.36  1995/01/17  18:40:00  doug
 * fixed UNICYCLE_HUMMER code in move_robot
 *
 * Revision 1.35  1995/01/13  17:22:47  doug
 * added UNICYCLE_HUMMER kinematics support
 * Added support for cycle_per_second control
 *
 * Revision 1.34  1995/01/11  21:18:06  doug
 * added new show vector option
 * limited steer rate in hummer movement code
 *
 * Revision 1.33  1995/01/10  20:59:56  doug
 * remove code to magnify HUMMER wheelbase along with display per Ron
 *
 * Revision 1.32  1995/01/10  19:42:47  doug
 * Make hummer wheel base scale with robot magnification.
 * Otherwise, it looks really weird on the screen
 *
 * Revision 1.31  1995/01/10  19:32:58  doug
 * handle hummer kinematics
 *
 * Revision 1.30  1994/12/06  16:30:39  doug
 * getting dennings to run
 *
 * Revision 1.29  1994/11/29  17:00:24  doug
 * Make I/O relative to current heading
 *
 * Revision 1.28  1994/11/21  17:02:40  doug
 * added debug print statements to put_state
 *
 * Revision 1.28  1994/11/21  17:02:40  doug
 * added debug print statements to put_state
 *
 * Revision 1.27  1994/11/18  00:47:59  doug
 * *** empty log message ***
 *
 * Revision 1.26  1994/11/17  23:44:52  doug
 * fixed init code when running denning
 *
 * Revision 1.25  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.25  1994/11/16  20:52:48  doug
 * combined get_xy and get_heading messages into get_location
 *
 * Revision 1.24  1994/11/15  16:15:55  doug
 * Fixed detect_robots to correctly count the size the return array
 * needs to be allocated instead of guessing based on the next_robot_id
 * which didn't work.
 *
 * Revision 1.23  1994/11/15  15:37:01  jmc
 * Working on next_robot_id bug.
 *
 * Revision 1.22  1994/11/14  21:39:16  doug
 * removed Jonathan's hack to clear_robots
 *
 * Revision 1.21  1994/11/11  23:03:52  jmc
 * Patched clear_robots to avoid crashing mlab.
 *
 * Revision 1.20  1994/11/08  17:51:25  doug
 * fixed erase_impact
 *
 * Revision 1.19  1994/11/08  16:02:15  doug
 * added impact drawing
 *
 * Revision 1.18  1994/11/04  03:34:32  doug
 * removed old style detect_obstacles
 *
 * Revision 1.17  1994/11/03  20:19:16  doug
 * added options button to turn on cthreads scheduler messages
 *
 * Revision 1.16  1994/11/03  18:37:51  jmc
 * Changed draw_halos to highlight_repelling_obstacles.
 *
 * Revision 1.15  1994/11/02  21:52:28  doug
 * Fixed avoid obstacle
 * added draw_halo option
 *
 * Revision 1.14  1994/11/01  23:40:47  doug
 * Name tcx socket with pid
 * handle robot error messages from bad version
 *
 * Revision 1.13  1994/10/26  23:18:15  doug
 * added slider bars and converted to meters
 *
 * Revision 1.12  1994/10/26  14:21:44  jmc
 * Modified clear_robots to reset the next_robot_id for
 * gt_command_yac.y.
 *
 * Revision 1.11  1994/10/25  15:39:16  jmc
 * Removed ROBOT_EXECUTABLE and MACHINE_NAME to use the robot info
 * database.  Modified send_robot_command to use the robot info when
 * execing robots and later to send them the parameters for that
 * robot definition.  Had to add a gt_Robot type parameter to this
 * function to know what type of robot is involved.  That information
 * is NOT in the gt_Command data structure.  Modified includes
 * appropriately.
 *
 * Revision 1.10  1994/10/19  22:09:06  doug
 * fixed machine name handling
 *
 * Revision 1.9  1994/10/19  21:54:09  doug
 * *** empty log message ***
 *
 * Revision 1.8  1994/10/18  20:55:29  doug
 * fixing prototypes
 *
 * Revision 1.7  1994/10/13  21:48:25  doug
 * Working on exit conditions for the robots
 *
 * Revision 1.6  1994/10/11  17:11:26  doug
 * added seq field to gt_command record
 *
 * Revision 1.5  1994/10/06  20:29:26  doug
 * fixing execute
 *
 * Revision 1.4  1994/10/04  22:20:50  doug
 * Getting execute to work
 *
 * Revision 1.3  1994/10/03  21:13:47  doug
 * Switched to tcx
 *
 * Revision 1.2  1994/09/28  20:45:42  doug
 * closer to working with tcx
 *
 * Revision 1.1  1994/09/28  19:43:02  doug
 * Initial revision
 *
 * Revision 1.18  1994/09/06  13:44:12  jmc
 * Removed some obsolete debugging code.
 *
 * Revision 1.17  1994/09/03  17:02:56  doug
 * Getting execution to work
 *
 * Revision 1.16  1994/08/30  16:10:38  doug
 * Up'd timeout to 5 minutes (Added the define to allow modifying)
 *
 * Revision 1.15  1994/08/22  16:26:42  doug
 * Added support for robot_debug_messages
 *
 * Revision 1.14  1994/08/19  17:08:30  doug
 * rpc is working
 *
 * Revision 1.13  1994/08/19  16:20:36  doug
 * fixing rpcgen code
 *
 * Revision 1.12  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.11  1994/08/18  18:40:27  doug
 * console links
 *
 * Revision 1.10  1994/08/17  22:57:45  doug
 * fixing rpc code
 *
 * Revision 1.9  1994/08/17  21:23:00  doug
 * moving to rpcgen
 *
 * Revision 1.8  1994/08/12  19:59:09  doug
 * added move_robot and get_xy calls
 *
 * Revision 1.7  1994/08/12  16:01:04  doug
 * moving it to use rpcgen
 *
 * Revision 1.6  1994/07/19  21:54:33  jmc
 * Removed obsolete bounding overwatch references.
 *
 * Revision 1.5  1994/07/15  18:52:47  jmc
 * Added function prototype for gethostname
 *
 * Revision 1.4  1994/07/15  18:16:37  jmc
 * Added cast to (struct sockaddr *) in line 707
 *
 * Revision 1.3  1994/07/14  14:04:36  jmc
 * Added several system function prototypes
 *
 * Revision 1.2  1994/07/12  19:27:51  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
