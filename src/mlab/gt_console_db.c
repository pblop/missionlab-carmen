/**********************************************************************
 **                                                                  **
 **                        gt_console_db.c                           **
 **                                                                  **
 **                                                                  **
 **  Written by: Jonathan M. Cameron                                 **
 **                                                                  **
 **  The philosophy behind this database is that the user provides a **
 **  block of arbitrary binary data (value). The database makes a    **
 **  copy of this block of data and stores it. The database manages  **
 **  its own memory and the user is to free their own data when      **
 **  appropriate.                                                    **
 **                                                                  **
 **  Copyright 1995 - 2007 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: gt_console_db.c,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h> 
#include <string> 
#include <vector>

#include "console.h"
#include "draw.h"

#define GT_CONSOLE_DB_C
#include "gt_console_db.h"

#include "gt_measure.h"
#include "gt_load_overlay.h"
#include "gt_simulation.h"
#include "attractor.hpp"
#include "flag.hpp"
#include "basket.hpp"
#include "obstacle.hpp"
#include "gt_sim.h"
#include "lineutils.h"
#include "3d_vis_wid.h"
#include "gt_message_types.h"

#include "overlay_zones.h"

using std::string;

//-----------------------------------------------------------------------
// Constants
//-----------------------------------------------------------------------
#define SymbolTableSize 2048 // Size for symbol table (Closed hashing)
#define DEFAULT_ASSEMBLY_AREA_DIAMETER 50.0
#define DEFAULT_ATTACK_POSITION_DIAMETER 50.0
#define DEFAULT_AXIS_WIDTH 100.0
#define DEFAULT_BATTLE_POSITION_DIAMETER 50.0
#define DEFAULT_GAP_WIDTH 40.0
#define DEFAULT_OBJECTIVE_DIAMETER 50.0
#define DEFAULT_PASSAGE_POINT_DIAMETER 10.0
#define DEFAULT_ATTRACTOR_DIAMETER 0.2	// meters
#define DEFAULT_BASKET_DIAMETER 0.4	// meters
#define DEFAULT_FLAG_DIAGONAL 0.4	// meters

const string STRING_DESIRED_ROBOT_ID_KEY = "desired_robot_id";
const string STRING_MEASURE_POINT = "PP";
const string STRING_START_PLACE_KEY = "StartPlace";
const string STRING_START_PLACE_STAR_KEY = "StartPlace*";
const int DEFAULT_DESIRED_ROBOT_ID = -1;

//-----------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------
#define add_string_to_db(key,str,func) \
   if (gt_update_db_entry(key, STRING, str, strlen(str)+1) == -1) { \
      warn_userf("%s: Error updating overlay database with %s.", key, str); \
      return GT_FAILURE; \
      }
#define null_out(var) \
  if (var) free(var); \
  var = NULL

//-----------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------
typedef struct {
    char *key;
    gt_db_Entry_type  type;
    void *value; // this is a pointer to anything
    unsigned  value_size; // number of bytes
} SymbolTableEntry;

//-----------------------------------------------------------------------
// Externs
//-----------------------------------------------------------------------
extern ObstacleHandler *Obs_Handler;

//-----------------------------------------------------------------------
// Variables
//
// NOTE: The first two entries in the gt_robot_info array, gt_robot_info[0]
//       and gt_robot_info[1] are predefined to UNIT and ROBOT, resepectively.
//       (See gt_initialize_robot_info()).  Information for robots that the
//       user defines is inserted in this array starting in position
//       gt_robot_info[2].
//-----------------------------------------------------------------------
int num_robot_types = 0;
gt_Robot_info gt_robot_info[MAX_NUM_ROBOT_TYPES];
lineRec2d_t *gConsoleDBLineList = NULL, *gConsoleDBLineListTail = NULL;
door_t *gConsoleDBDoorwayList = NULL, *gConsoleDBDoorwayListTail = NULL;
room_t *gConsoleDBRoomList = NULL, *gConsoleDBRoomListTail = NULL;
hallwayRec_t *gConsoleDBHallwayList = NULL, *gConsoleDBHallwayListTail = NULL;
SLList *Wall_List = NULL;

static SymbolTableEntry SymbolTable[SymbolTableSize];      




// PU added for zone handling;
vector<WeatherZone *> WeatherZones;
vector<NoFlyZone *> NoFlyZones;
vector<NoCommZone *> NoCommZones;




//-----------------------------------------------------------------------
// Forward Declaration
//-----------------------------------------------------------------------
static int add_subunit_members(gt_Unit *unit);
static void find_unit_members(gt_Unit *unit, gt_Unit_members *members);
static int num_robots_in_unit(gt_Unit *unit);
static void check_measure_draw_label(char *name, gt_Measure *measure);
static int add_lines_db(gt_Point_list *pt, double height);
static void add_doorway_db(gt_Point_list *pt);
static void add_room_db(gt_Point_list *pt);
static void add_hallway_db(gt_Point_list *centerline, double width, gt_Point_list *extents);
static int Hash(char *key);
static int HashForDelete(char *key);

//-----------------------------------------------------------------------
int gt_add_assembly_area(char *name, gt_Point_list *pt, float diameter)
{
   unsigned size;
   gt_Measure *measure;

   // check the diameter
   if (diameter < 0.0)
      diameter = DEFAULT_ASSEMBLY_AREA_DIAMETER;

   // create the measure
   measure = gt_new_multipoint_measure(ASSEMBLY_AREA, REGION, diameter, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create assembly area %s.", name);
      return GT_FAILURE;
      }

   // insert it into the data base
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database\n\
       with assembly area %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   // draw assembly area
   draw_assembly_area(name, measure);

   free(measure);

   return GT_SUCCESS;
}

//-----------------------------------------------------------------------
int gt_add_attack_position(char *name, gt_Point_list *pt, float diameter)
{
   unsigned size;
   gt_Measure *measure;

   // check the diameter
   if (diameter < 0.0)
      diameter = DEFAULT_ATTACK_POSITION_DIAMETER;

   // create the measure
   measure = gt_new_multipoint_measure(ATTACK_POSITION, REGION, diameter, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create attack position %s.", name);
      return GT_FAILURE;
      }

   // insert it into the data base
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with\n\
       attack point %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   // draw attack position
   draw_attack_position(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **            gt_add_axis             **
 ****************************************/

int gt_add_axis(char *name, gt_Point_list *pt, float width)
{
   unsigned size;
   gt_Measure *measure;

   /* check the diameter */
   if (width < 0.0)
      width = DEFAULT_AXIS_WIDTH;

   /* create the measure */
   measure = gt_new_multipoint_measure(AXIS, POLYLINE, width, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create axis %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with Axis %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw the axis */
   draw_axis(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **       gt_add_battle_position       **
 ****************************************/

int gt_add_battle_position(char *name, gt_Point_list *pt, float diameter)
{
   unsigned size;
   gt_Measure *measure;

   /* check the diameter */
   if (diameter < 0.0)
      diameter = DEFAULT_BATTLE_POSITION_DIAMETER;

   /* create the measure */
   measure = gt_new_multipoint_measure(BATTLE_POSITION, REGION, diameter, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create battle position %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with\n\
       battle position %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw battle position */
   draw_battle_position(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **          gt_add_boundary           **
 ****************************************/

int gt_add_boundary(char *name, gt_Point_list *pt)
{
   unsigned size;
   gt_Measure *measure;

   // PU  added handling for the different zone types
   //printf("adding: %s\n", name);
   if (strstr(name, "WeatherZone")) {
     WeatherZones.push_back(new WeatherZone(name, pt));
     //printf("made weather\n");
   }
   else if (strstr(name, "NoFlyZone")) {
     NoFlyZones.push_back(new NoFlyZone(name, pt));
     //printf("made nofly\n");
   }
   else if (strstr(name, "NoCommZone")) {
     NoCommZones.push_back(new NoCommZone(name, pt));
     //printf("made nocomm\n");
   }

   /* create the measure */
   measure = gt_new_multipoint_measure(BOUNDARY, POLYLINE, 0.0, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create boundary %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with\n\
       boundary %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw boundary */
   draw_boundary(name, measure);

   free(measure);

   return GT_SUCCESS;
}

/**********************************************************************
 **            gt_add_wall             **
 ****************************************/

int gt_add_wall(char *name, gt_Point_list *pt, double height)
{
    unsigned size;
    gt_Measure *measure;
    Wall_List_Element *temprec;
    int highpt;

    /* create the measure */
    measure = gt_new_multipoint_measure(WALL, POLYLINE, 0.0, pt, &size, height);
    if (measure == NULL) {
        warn_userf("Error: Unable to create wall %s.", name);
        return GT_FAILURE;
    }

    // 3d visualization
    if (Wall_List == NULL) {
        Wall_List = new SLList();
    }

    /* insert it into the data base */
    check_measure_draw_label(name, measure);
    if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
        warn_userf("Error: Unable to update overlay database with\nwall %s.", name);
        free(measure);
        return GT_FAILURE;
    }

    /* draw wall */
    draw_wall(name, measure); 
    highpt = add_lines_db(pt, height);
   
    temprec = new Wall_List_Element;
    temprec->Walls = measure;
    temprec->Height = height;
    temprec->HighPt = highpt;
    Wall_List->Insert_Rear((void *)temprec);   

    if (gt_show_3d_vis)
    {
        initialize_3d_vis();
    }

    return GT_SUCCESS;
}



/**********************************************************************
 **            gt_add_door             **
 ****************************************/

int gt_add_door(char *name, gt_Point_list *pt)
{
   unsigned size;
   gt_Measure *measure;

   /* create the measure */
   measure = gt_new_multipoint_measure(DOOR, POLYLINE, 0.0, pt, &size);
   if (measure == NULL) {
     warn_userf("Error: Unable to create door %s.", name);
      return GT_FAILURE;
   }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
     warn_userf("Error: Unable to update overlay database with\ndoor %s.", name);
     free(measure);
     return GT_FAILURE;
   }

   /* draw door */
   draw_door(name, measure);
   add_doorway_db(pt);
   free(measure);

   return GT_SUCCESS;
}

/**********************************************************************
 **            gt_add_room             **
 ****************************************/

int gt_add_room(char *name, gt_Point_list *pt)
{
   unsigned size;
   gt_Measure *measure;

   /* create the measure */
   measure = gt_new_multipoint_measure(ROOM, POLYLINE, 0.0, pt, &size);
   if (measure == NULL) {
     warn_userf("Error: Unable to create room %s.", name);
      return GT_FAILURE;
   }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
     warn_userf("Error: Unable to update overlay database with\nroom %s.", name);
     free(measure);
     return GT_FAILURE;
   }

   /* draw room */
   draw_room(name, measure);
   
   add_room_db(pt);
   free(measure);

   return GT_SUCCESS;
}

/**********************************************************************
 **            gt_add_hallway          **
 ****************************************/

int gt_add_hallway(char *name, gt_Point_list *centerline, double width, gt_Point_list *extents)
{
   unsigned size;
   gt_Measure *measure;

   /* create the measure */
   measure = gt_new_multipoint_measure(HALLWAY, POLYLINE, 0.0, extents, &size);
   if (measure == NULL) {
     warn_userf("Error: Unable to create hallway %s.", name);
      return GT_FAILURE;
   }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
     warn_userf("Error: Unable to update overlay database with\nhallway %s.", name);
     free(measure);
     return GT_FAILURE;
   }

   /* draw hallway */
   draw_hallway(name, measure);
   add_hallway_db(centerline, width, extents);
   free(measure);

   return GT_SUCCESS;
}

/**********************************************************************
 **            gt_add_gap              **
 ****************************************/

int gt_add_gap(char *name, gt_Point_list *pt, float width)
{
   unsigned size;
   gt_Measure *measure;

   /* check the diameter */
   if (width < 0.0)
      width = DEFAULT_GAP_WIDTH;

   /* create the measure */
   measure = gt_new_multipoint_measure(GAP, POLYLINE, width, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create gap %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with Gap %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw gap */
   draw_gap(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **            gt_add_ldlc             **
 ****************************************/

int gt_add_ldlc(char *name, gt_Point_list *pt)
{
   unsigned size;
   gt_Measure *measure;

   /* create the measure */
   measure = gt_new_multipoint_measure(LDLC, POLYLINE, 0.0, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create LD/LC %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with LD/LC %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw LD/LC */
   draw_LDLC(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **        gt_add_mission_name         **
 ****************************************/

int gt_add_mission_name(char *name)
{
   add_string_to_db("MISSION-NAME", name, "gt_add_mission_name");

   draw_mission_name(name);

   return GT_SUCCESS;
}


/**********************************************************************
 **       gt_add_new_robot_type        **
 ****************************************/

int gt_add_new_robot_type(
    char *name,
    char *executable,
    char *host,
    char *color,
    gt_Robot_parameters_list *params)
{
    /* It is up to the caller to guarantee that this is indeed a new
       type of robot.  Since this function is designed to be called
       by the parser, this should be a valid assumption. */

    gt_Robot_parameters_list *p;
    string bufString;
    char buf[1024];
    int npars = 0;
    int i;

    /* Install the new information */
    gt_robot_info[num_robot_types].name = strdup(name);
    gt_robot_info[num_robot_types].executable = strdup(executable);
    gt_robot_info[num_robot_types].desired_robot_id = DEFAULT_DESIRED_ROBOT_ID;

    if (host)
    {
        gt_robot_info[num_robot_types].host = strdup(host);
    }
    else
    {
        gt_robot_info[num_robot_types].host = NULL;
    }

    if (color)
    {
        gt_robot_info[num_robot_types].color = strdup(color);
    }
    else
    {
        gt_robot_info[num_robot_types].color = "blue";
    }

    /* See how many parameters there are */
    p = params;

    while (p)
    {
        npars++;
        p = p->next;
    }

    gt_robot_info[num_robot_types].num_params = npars;

    /* allocate an array for the parameters */
    if ((gt_robot_info[num_robot_types].param = 
         (gt_Robot_parameters *) malloc(npars*sizeof(gt_Robot_parameters))) == NULL)
    {
        fprintf(stderr, "Error in gt_add_new_robot_type: Out of memory!\n");
        return GT_FAILURE;
    }

    /* copy in the parameters */
    p = params;

    for ( i = 0; i < npars; i++)
    {
        gt_robot_info[num_robot_types].param[i].key = strdup(p->key);
        gt_robot_info[num_robot_types].param[i].value = strdup(p->value);

        // Check the Robot ID.
        bufString = p->key;

        if (bufString == STRING_DESIRED_ROBOT_ID_KEY)
        {
            sprintf(buf, "%s", p->value);
            gt_robot_info[num_robot_types].desired_robot_id = atoi(buf);
        }

        p = p->next;
    }

    num_robot_types++;

    return GT_SUCCESS;
}



/**********************************************************************
 **         gt_add_objective           **
 ****************************************/

int gt_add_objective(char *name, gt_Point_list *pt, float diameter)
{
   unsigned size;
   gt_Measure *measure;

   /* check the diameter */
   if (diameter < 0.0)
      diameter = DEFAULT_OBJECTIVE_DIAMETER;

   /* create the measure */
   measure = gt_new_multipoint_measure(OBJECTIVE, REGION, diameter, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create objective %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with\n\
       objective %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw objective */
   draw_objective(name, measure);

   free(measure);

   return GT_SUCCESS;
}



/**********************************************************************
 **          gt_add_overlay            **
 ****************************************/

int gt_add_overlay(char *filename)
{
   return gt_load_overlay(filename);
}

/**********************************************************************
 *  gt_load_waypoint_file                                             *
 *                                                                    *
 *  This function loads and parses the specified waypoint file.       *
 **********************************************************************/

int gt_load_waypoint_file(char *filename)
{

  FILE *file;

  // Check first to see if the waypoint file is readable.
  if ((file = fopen(filename, "r")) == NULL)
  {
    fprintf(stderr, "Warning: Unable to load waypoint file \"%s\".\n", filename);
    return GT_FAILURE;
  }
  else
    fclose(file);

  // Close the overlay file that was opened for parsing.
  fclose(odl_in);

  // Re-open the waypoint file, as a file to be parsed.
  odl_in = fopen(filename, "r");

  // Now, parse it.
  char full_filename[4096];
  sprintf(full_filename, "%s", filename);
  strcpy(odl_filename, full_filename);
  odl_parse();
  odl_file_reset();

  return GT_SUCCESS;
}


//-----------------------------------------------------------------------
int gt_add_passage_point(char *name, float x, float y, float diameter)
{
    gt_Measure *measure;

    // check the diameter
    if (diameter < 0.0)
    {
        diameter = DEFAULT_PASSAGE_POINT_DIAMETER;
    }

    // create the measure
    measure = gt_new_point_measure(PASSAGE_POINT, x, y, diameter);
    if (measure == NULL)
    {
        warn_userf("Error: Unable to create passage point %s.", name);
        return GT_FAILURE;
    }

    // insert it into the data base
    check_measure_draw_label(name, measure);
    if (gt_update_db_entry(name, MEASURE, measure, sizeof(gt_Measure)) == -1)
    {
        warn_userf("Error: Unable to update overlay database with\n\
       passage point %s.", name);
        free(measure);
        return GT_FAILURE;
    }

    // draw passage point
    draw_passage_point(name, measure);

    free(measure);

    return GT_SUCCESS;
}


/**********************************************************************
 **       gt_add_flag         **
 ****************************************/

int gt_add_flag(float x, float y, float diagonal)
{
   /* check the diagonal */
   if (diagonal < 0.0)
      diagonal = DEFAULT_FLAG_DIAGONAL;

   flag *f = new flag(x,y,diagonal/2,"purple");
   f->draw();

   // Add to list of all perceiveable features in the environment
   object_list.append(f);

   return GT_SUCCESS;
}

/**********************************************************************
 **       gt_add_attractor         **
 ****************************************/

int gt_add_attractor(float x, float y, float diameter)
{
   /* check the diameter */
   if (diameter < 0.0)
      diameter = DEFAULT_ATTRACTOR_DIAMETER;

   attractor *p = new attractor(x,y,diameter/2,"orange");
   p->draw();

   // Add to list of all perceiveable features in the environment
   object_list.append(p);

   return GT_SUCCESS;
}

/**********************************************************************
 **       gt_add_object         **
 ****************************************/

int gt_add_object(float x, float y, float diameter,
    const char *color, OBJ_STYLES style)
{
   /* check the diameter */
   if (diameter < 0.0)
      diameter = DEFAULT_ATTRACTOR_DIAMETER;

   shape *p;
   switch( style )
   {
      case STYLE_FIXED:
         p = new obstacle(x,y,diameter/2, 0.0, color);
	 break;

      case STYLE_MOVABLE:
         p = new attractor(x,y,diameter/2, color);
	 break;

      case STYLE_CONTAINER:
         p = new basket(x,y,diameter/2, color);
	 break;
   }

   // Draw it.
   p->draw();

   // Add to list of all perceiveable features in the environment
   object_list.append(p);

   return GT_SUCCESS;
}



/**********************************************************************
 **       gt_add_basket         **
 ****************************************/

int gt_add_basket(float x, float y, float diameter)
{
   /* check the diameter */
   if (diameter < 0.0)
      diameter = DEFAULT_BASKET_DIAMETER;

   basket *p = new basket(x,y,diameter/2,"green");
   p->draw();

   // Add to list of all perceiveable features in the environment
   object_list.append(p);

   return GT_SUCCESS;
}

/**********************************************************************
 **         gt_add_phase_line          **
 ****************************************/

int gt_add_phase_line(char *name, gt_Point_list *pt)
{
   unsigned size;
   gt_Measure *measure;

   /* create the measure */
   measure = gt_new_multipoint_measure(PHASE_LINE, POLYLINE, 0.0, pt, &size);
   if (measure == NULL) {
      warn_userf("Error: Unable to create phase line %s.", name);
      return GT_FAILURE;
      }

   /* insert it into the data base */
   check_measure_draw_label(name, measure);
   if (gt_update_db_entry(name, MEASURE, measure, size) == -1) {
      warn_userf("Error: Unable to update overlay database with\n\
       phase line %s.", name);
      free(measure);
      return GT_FAILURE;
      }

   /* draw phase line */
   draw_phase_line(name, measure); 

   free(measure);

   return GT_SUCCESS;
}


/**********************************************************************
 **          gt_add_scenario_label     **
 ****************************************/

int gt_add_scenario_label(char *label)
{
   add_string_to_db("SCENARIO",label,"gt_add_scenario_label");

   return GT_SUCCESS;
}




/**********************************************************************
 **         gt_add_site_name           **
 ****************************************/

int gt_add_site_name(char *name)
{
   add_string_to_db("SITE",name,"gt_add_site_name");

   return GT_SUCCESS;
}

//-----------------------------------------------------------------------
// This function returns the coordinate of the StartPlace
//-----------------------------------------------------------------------
int gtGetStartPlace(gt_Point *startPlace)
{
    //gt_Point sp;
    gt_Measure *measure;
    //gt_Unit_members *members;
    string measureType, measureKey;
    int i;

    memset(startPlace, 0x0, sizeof(gt_Point));

    for (i = 0; i < SymbolTableSize; i++)
    {
        if (SymbolTable[i].key)
        {
            if ((SymbolTable[i].type) == MEASURE)
            {
                measure = (gt_Measure *)SymbolTable[i].value;

                if ((measure->data_type) == POINT)
                {
                    measureType = gt_measure_type_name[measure->type];
                    measureKey = SymbolTable[i].key;
                    
                    if ((measureType == STRING_MEASURE_POINT) &&
                        ((measureKey == STRING_START_PLACE_KEY) ||
                         (measureKey == STRING_START_PLACE_STAR_KEY)))
                    {
                        if (measure->data.num_points > 0)
                        {
                            memcpy(
                                startPlace,
                                &(measure->data.point[0]),
                                sizeof(gt_Point));
                            return GT_SUCCESS;
                        }
                        else
                        {
                            return GT_FAILURE;
                        }
                    }
                }
            }
        }
    }

    return GT_FAILURE;
}

//-----------------------------------------------------------------------
// NOTE: This function is OBSOLETE as the starting point is replaced by
// "PP StartPlace <x> <y> <radius>"
//-----------------------------------------------------------------------
int gt_add_starting_point(char *name, float x, float y)
{
    //gt_Measure *measure;

    warn_userf("Syntax SP no longer supported. Use 'PP StartPlace' instead.");
    return GT_FAILURE;
    
    /*
    // create the measure
    measure = gt_new_point_measure(STARTING_POINT, x, y, 0.0);

    if (measure == NULL)
    {
        warn_userf("Error: Unable to create starting point %s.", name);
        return GT_FAILURE;
    }

    // insert it into the data base
    check_measure_draw_label(name, measure);

    if (gt_update_db_entry(name, MEASURE, measure, sizeof(gt_Measure)) == -1)
    {
        warn_userf(
            "Error: Unable to update overlay database with\nstarting point %s",
            name);
        free(measure);
        return GT_FAILURE;
    }

    // draw it
    draw_starting_point(name, measure);

    free(measure);

    return GT_SUCCESS;
    */
}



/**********************************************************************
 **           gt_add_unit              **
 ****************************************/

int gt_add_unit(gt_Unit *unit)
{
   /* Go through the tree and add a UNIT entry to the database for each
      node with a name.  The entry is simply a list of robots that should
      respond to that unit name. */

   /* add each subunit */
   return add_subunit_members(unit);
}



/**********************************************************************
 **      gt_initialize_robot_info      **
 ****************************************/

void gt_initialize_robot_info()
{
   int i, k;

   /* first, get rid of any existing excess definitions */
   for (i=2; i<num_robot_types; i++) {
      null_out(gt_robot_info[i].name);
      null_out(gt_robot_info[i].executable);
      null_out(gt_robot_info[i].host);
      null_out(gt_robot_info[i].color);
      for (k=0; k<gt_robot_info[i].num_params; k++) {
	 null_out(gt_robot_info[i].param[k].key);
	 null_out(gt_robot_info[i].param[k].value);
	 }
      null_out(gt_robot_info[i].param);
      }

   /* initialize the basic types (UNIT=0, ROBOT=1) */
   gt_robot_info[UNIT].name = strdup("UNIT");
   gt_robot_info[UNIT].executable = NULL;
   gt_robot_info[UNIT].num_params = 0;
   gt_robot_info[UNIT].param = NULL;

   gt_robot_info[ROBOT].name = strdup("ROBOT");
   gt_robot_info[ROBOT].executable = strdup("robot");
   gt_robot_info[ROBOT].color = strdup("blue");
   gt_robot_info[ROBOT].num_params = 0;
   gt_robot_info[ROBOT].param = NULL;

   num_robot_types = 2;
}


/**********************************************************************
 **        gt_print_robot_info         **
 ****************************************/

void gt_print_robot_info()
{
   int i,k;

   printf("CURRENT ROBOT INFO:\n");
   for (i=1; i<num_robot_types; i++) {
      printf("*** ROBOT \"%s\" EXE=\"%s\"",
	     gt_robot_info[i].name, gt_robot_info[i].executable);
      if (debug)
	 printf(" (type=%d)", i);
      if (gt_robot_info[i].host)
	 printf(" HOST=\"%s\"", gt_robot_info[i].host);
      if (gt_robot_info[i].num_params > 0) {
	 printf(" (");
	 for (k=0; k<gt_robot_info[i].num_params; k++) {
	    printf("%s=\"%s\"", gt_robot_info[i].param[k].key, gt_robot_info[i].param[k].value);
	    if (k+1 < gt_robot_info[i].num_params)
	       printf(", ");
	    }
	 printf(")");
	 }
      printf("\n");
      }
}


/**********************************************************************
 **    gt_new_robot_parameters_list    **
 ****************************************/

gt_Robot_parameters_list *gt_new_robot_parameters_list(
   char *key,
   char *value,
   gt_Robot_parameters_list *next)
{
   /* Create a new "robot_parameters_list" using the specified data.
      It is up to the user to "free" it. */
   gt_Robot_parameters_list *tmp =
      (gt_Robot_parameters_list *) malloc(sizeof(gt_Robot_parameters_list));

   if (tmp) {
      tmp->key = strdup(key);
      tmp->value = strdup(value);
      tmp->next = next;
      return tmp;
      }
   else
      return NULL;
}



/**********************************************************************
 **    gt_free_robot_parameters_list    **
 ****************************************/

void gt_free_robot_parameters_list(gt_Robot_parameters_list *params)
{
   gt_Robot_parameters_list *p, *next;

   /* zip through the list, freeing as we go */
   p = params;
   while (p) {
      next = p->next;
      free(p->key);
      free(p->value);
      free(p);
      p = next;
      }
}



/**********************************************************************
 **           gt_robot_type            **
 ****************************************/

gt_Robot gt_robot_type(char *name)
{
   int i;

   /* scan through the list of of robot types and try to find this one */
   /* NOTE: This function will not match "UNIT" since it is a special case */
   for (i=1; i<num_robot_types; i++) {
      if ((strlen(name) == strlen(gt_robot_info[i].name)) &&
	  (strcasecmp(name, gt_robot_info[i].name) == 0))
	 return i;
      }

   /* no match was found */
   return GT_FAILURE;
}

//-----------------------------------------------------------------------
// Adds the entry into the symbol tape if it isn't already there.
// If it is found, then update its value.
// Return 0 for GT_SUCCESS, -1 for GT_failure
//-----------------------------------------------------------------------
int gt_update_db_entry(
    char *key,
    gt_db_Entry_type type,
    void *value,
    unsigned value_size)
{
    int loc;

    // Choke if we have an error hashing
    if((loc = Hash(key)) < 0)
    {
        warn_userf("Error: Unable to hash key '%s'.", key);
        return GT_FAILURE;
    }

    // If it isn't already in the database, copy the key string
    if (SymbolTable[loc].key == NULL)
    {
        // Not in table so add string
        if( (SymbolTable[loc].key = (char *)malloc(strlen(key)+1)) == NULL)
        {
            fprintf(stderr, "Error: Unable to update overlay database;");
            fprintf(stderr, "       Unable to allocate memory for key '%s'.", key);
            return GT_FAILURE;
        }
        strcpy(SymbolTable[loc].key, key);
    }

    // Tell it what type of entry it is
    SymbolTable[loc].type = type;

    // If there is something there already, get rid of it
    if (SymbolTable[loc].value != NULL)
    {
        free(SymbolTable[loc].value);
        SymbolTable[loc].value = NULL;
    }

    // Install the new value
    if ((SymbolTable[loc].value = (void *)malloc(value_size)) == NULL)
    {
        fprintf(stderr, "Error: Unable to update overlay database;");
        fprintf(stderr, "       Unable to allocate memory for value of '%s'.", key);
        return GT_FAILURE;
    }
    memcpy(SymbolTable[loc].value,value,value_size);

    return GT_SUCCESS;
}

//-----------------------------------------------------------------------
int gt_delete_db_entry(char *key)
{
    int loc;

    // find it
    //if((loc = Hash(key)) < 0)
    if((loc = HashForDelete(key)) < 0)
    {
        warn_userf(
            "Error: Unable to delete overlay database entry %s.\nHash failed.",
            key);
        fprintf(
            stderr,
            "Error (mlab): gt_delete_db_entry(): key = %s, loc = %d.\n",
            key,
            loc);
        return GT_FAILURE;
    }

    // clear the key string
    if (SymbolTable[loc].key != NULL) 
    {
        free(SymbolTable[loc].key);
        SymbolTable[loc].key = NULL;
    }

    // clear the value
    if( SymbolTable[loc].value != NULL) 
    {
        free(SymbolTable[loc].value);
        SymbolTable[loc].value = NULL;
    }

    return GT_SUCCESS;
}


/**********************************************************************
 **           gt_get_db_entry            **
 ****************************************/

void *gt_get_db_entry(char *key,
		      gt_db_Entry_type type)
{
   int    loc;

   /* find the entry */
   loc = Hash(key);
   if( loc < 0 ) {
      warn_userf("Error: Unable to get overlay database entry %s; \n\
hash failed.", key);
      return NULL;
      }

   /* if nothing was found, let the user know */
   if (SymbolTable[loc].key == NULL)
      return NULL;

   /* make sure it is the correct type of entry */
   if (SymbolTable[loc].type != type) {
      warn_userf("Error: Retrieved value from overlay database has\n\
       the wrong type!  Asked for %s, got %s\n", gt_db_entry_type_name[type],
		 gt_db_entry_type_name[SymbolTable[loc].type]);
      return NULL;
      }

   /* and return its value */
   return SymbolTable[loc].value;
}



/**********************************************************************
 **         gt_get_db_measure          **
 ****************************************/

gt_Measure *gt_get_db_measure(char *name)
{
    gt_Measure *measure;
    int i, found, name_length, prefix_length;
    char newname[80];

    /* first, try retrieving the exact name */
    measure = (gt_Measure *)gt_get_db_entry(name, MEASURE);
    if (measure)
        return measure;

    /* if not GT_SUCCESSful, try to strip off the optional prefix */
    found = FALSE;
    name_length = strlen(name);
    for (i=1; i<GT_NUM_MEASURE_TYPES; i++) {

        /* If the prefix is longer than the name, forget this one! */
        prefix_length = strlen(gt_measure_type_name[i]);
        if (prefix_length > name_length)
            continue;

        /* See if the initial part matches this prefix */
        if ((strncasecmp(gt_measure_type_name[i], name, prefix_length) == 0) &&
            (name[prefix_length] == '-')) {
            found = TRUE;
            /* strip off the prefix (and dash) */
            strcpy(newname, &(name[prefix_length+1]));
            break;
        }
    }

    /* If we found a matching prefix, try the name without the prefix */
    if (found) {
        measure = (gt_Measure *)gt_get_db_entry(newname, MEASURE);
        if (measure)
            return measure;
    }
    else
        strcpy(newname, name);

    /* Try all lowercase */
    for (i = 1; i < (int)(strlen(newname)); i++)
        newname[i] = tolower(newname[i]);
    measure = (gt_Measure *)gt_get_db_entry(newname, MEASURE);
    if (measure)
        return measure;

    /* Try the capitalized version */
    newname[0] = toupper(newname[0]);
    measure = (gt_Measure *)gt_get_db_entry(newname, MEASURE);
    if (measure)
        return measure;

    /* Try all caps */
    for (i = 1; i < (int)(strlen(newname)); i++)
        newname[i] = toupper(newname[i]);
    measure = (gt_Measure *)gt_get_db_entry(newname, MEASURE);
    if (measure)
        return measure;

    /* give up */
    return NULL;
}

//-----------------------------------------------------------------------
void gt_clear_db_measures(void)
{
    int loc;
    lineRec2d_t *tmp;
    room_t *t;
    hallwayRec_t *h;
    door_t *d;
   
    // go through and zap each entry that is a map measure
    for (loc=0; loc < SymbolTableSize; loc++) 
    {
        // make sure it is the correct type of entry
        if (SymbolTable[loc].type == MEASURE) 
        {
            if (SymbolTable[loc].key != NULL) 
            {
                free(SymbolTable[loc].key);
                SymbolTable[loc].key = NULL;
            }

            if (SymbolTable[loc].value != NULL) 
            {
                free(SymbolTable[loc].value);
                SymbolTable[loc].value = NULL;
            }
   
            SymbolTable[loc].value_size = 0;
        }
    }

    // also "zap" the line lists
    while (gConsoleDBLineList != NULL)
    {
        tmp = gConsoleDBLineList;
        gConsoleDBLineList = gConsoleDBLineList->next;
        free(tmp);
    }

    while (gConsoleDBDoorwayList != NULL)
    {
        d = gConsoleDBDoorwayList;
        while (d->doorway != NULL)
        {
            tmp = d->doorway;
            d->doorway = d->doorway->next;
            free(tmp);
        }
        gConsoleDBDoorwayList = gConsoleDBDoorwayList->next;
        free(d);
    }

    while (gConsoleDBRoomList != NULL)
    {
        t = gConsoleDBRoomList;
        while (t->walls != NULL)
        {
            tmp = t->walls;
            t->walls = t->walls->next;
            free(tmp);
        }
        gConsoleDBRoomList = gConsoleDBRoomList->next;
        free(t);
    }

    while (gConsoleDBHallwayList != NULL)
    {
        h = gConsoleDBHallwayList;
        while (h->extents != NULL)
        {
            tmp = h->extents;
            h->extents = h->extents->next;
            free(tmp);
        }
        gConsoleDBHallwayList = gConsoleDBHallwayList->next;
        free(h);
    }
}

//-----------------------------------------------------------------------
void gt_clear_db(void)
{
    int loc;
    lineRec2d_t *tmp;
    room_t *t;
    hallwayRec_t *h;
    door_t *d;

    // go through and zap each and every entry
    for (loc=0; loc < SymbolTableSize; loc++)
    {
        if (SymbolTable[loc].key != NULL)
        {
            free(SymbolTable[loc].key);
            SymbolTable[loc].key = NULL;
        }
        if (SymbolTable[loc].value != NULL)
        {
            free(SymbolTable[loc].value);
            SymbolTable[loc].value = NULL;
        }
        SymbolTable[loc].value_size = 0;
    }

    // also "zap" the line lists
    while (gConsoleDBLineList != NULL)
    {
        tmp = gConsoleDBLineList;
        gConsoleDBLineList = gConsoleDBLineList->next;
        free(tmp);
    }

    while (gConsoleDBDoorwayList != NULL)
    {
        d = gConsoleDBDoorwayList;
        while (d->doorway != NULL)
        {
            tmp = d->doorway;
            d->doorway = d->doorway->next;
            free(tmp);
        }
        gConsoleDBDoorwayList = gConsoleDBDoorwayList->next;
        free(d);
    }
    while (gConsoleDBRoomList != NULL)
    {
        t = gConsoleDBRoomList;
        while (t->walls != NULL)
        {
            tmp = t->walls;
            t->walls = t->walls->next;
            free(tmp);
        }
        gConsoleDBRoomList = gConsoleDBRoomList->next;
        free(t);
    }
    while (gConsoleDBHallwayList != NULL)
    {
        h = gConsoleDBHallwayList;
        while (h->extents != NULL)
        {
            tmp = h->extents;
            h->extents = h->extents->next;
            free(tmp);
        }
        gConsoleDBHallwayList = gConsoleDBHallwayList->next;
        free(h);
    }
}

//-----------------------------------------------------------------------
// Draws all the control measures in the database
//-----------------------------------------------------------------------
void gt_draw_db(void)
{
    int entry;
    char *name;
    gt_Measure *measure;
   
    for (entry=0; entry<SymbolTableSize; entry++)
    {
        if (SymbolTable[entry].key)
        {
            name = SymbolTable[entry].key;

            switch (SymbolTable[entry].type) {

            case MEASURE: {

                measure = (gt_Measure *)SymbolTable[entry].value;

                // print the data for the measure
                switch (measure->type) {

                case BOUNDARY:
                    draw_boundary(name, measure);
                    break;

                case WALL:
                    draw_wall(name, measure);
                    break;

                case ROOM:
                    draw_room(name, measure);
                    break;

                case HALLWAY:
                    draw_hallway(name, measure);
                    break;

                case DOOR:
                    draw_door(name, measure);
                    break;

                case LDLC:
                    draw_LDLC(name, measure);
                    break;

                case ASSEMBLY_AREA:
                    draw_assembly_area(name, measure);
                    break;

                case PASSAGE_POINT:
                    draw_passage_point(name, measure);
                    break;

                case GAP:
                    draw_gap(name, measure);
                    break;

                case ATTACK_POSITION:
                    draw_attack_position(name, measure);
                    break;

                case AXIS:
                    draw_axis(name, measure);
                    break;

                case PHASE_LINE:
                    draw_phase_line(name, measure);
                    break;

                case BATTLE_POSITION:
                    draw_battle_position(name, measure);
                    break;

                case STARTING_POINT:
                    draw_starting_point(name, measure);
                    break;

                case OBJECTIVE:
                    draw_objective(name, measure);
                    break;

                default:
                    warn_userf(
                        "Error in gt_draw_db: Unrecognized type of\ncontrol measure! (type=%d)\n",
                        measure->type);
                    break;
                }
            }
                break;

            default:
                // ignore everything but control measures and MISSION-NAME
                if ((SymbolTable[entry].type == STRING) &&
                    (strcmp(name, "MISSION-NAME") == 0))
                    draw_mission_name((char *)SymbolTable[entry].value);
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------
void gt_print_db(void)
{
    int entry, i;
    gt_Measure *measure;
    gt_Unit_members *members;

    printf("CURRENT DATABASE:\n");
   
    for (entry=0; entry<SymbolTableSize; entry++)
    {
        if (SymbolTable[entry].key)
        {
            switch (SymbolTable[entry].type) {

            case STRING:
                printf("*** %s %s\n", SymbolTable[entry].key,  (char *)SymbolTable[entry].value);
                break;

            case MEASURE: {

                measure = (gt_Measure *)SymbolTable[entry].value;

                // print the name and type
                printf("*** %s %s", gt_measure_type_name[measure->type],
                       SymbolTable[entry].key);

                // print the data for the measure
                switch (measure->data_type) {

                case POINT:
                case POLYLINE:
                case REGION:
                    // print out the data for the measure
                    for (i = 0; i < (int)(measure->data.num_points); i++) {
                        printf(" %1.4g %1.4g", 
                               measure->data.point[i].x, measure->data.point[i].y);
                    }
                    printf("\n");
                    break;

                default:
                    printf(" Unrecognized data! (type=%d)\n", measure->data_type);
                    break;
                }
            }
                break;

            case UNIT_MEMBERS:
                members = (gt_Unit_members *)SymbolTable[entry].value;
                // print out the robots in this unit
                printf("*** UNIT <%s>", SymbolTable[entry].key);
                for (i=0; i<members->num_robots; i++)
                    printf(" %s-%d", gt_robot_info[members->robot[i].type].name,
                           members->robot[i].id);
                printf("\n");
                break;

            default:
                printf("*** Unrecognized type of entry, key=%s, type=%d\n",
                       SymbolTable[entry].key, SymbolTable[entry].type);
                break;
            }
        }
    }
}

//-----------------------------------------------------------------------
// This routine will return an integer index into the SymbolTable. The
// location that is returned must then be checked.  If the key is NULL then 
// the symbol is not in the table and should be inserted in this position.  If
// this is the symbol then it was found at this loc.  Return of -1 means error.
//-----------------------------------------------------------------------
static int Hash(char *str)
{
    int val = 0, loc = 0, start;

    while (str[loc] != 0)
    {
        val += str[loc++];
    }

    loc = (val * 3) % SymbolTableSize;
    start = loc;

    while (SymbolTable[loc].key != NULL &&
           (strcmp(SymbolTable[loc].key, str) != 0))
    {
        loc = (loc + 1) % SymbolTableSize;

        if (loc == start)
        {
            // Have gone through entire table and all locations are full!
            warn_userf("Error: Hash failed; overlay db symbol table is full.");
            return GT_FAILURE;
        }
    }

    return (loc);
}

//-----------------------------------------------------------------------
static int HashForDelete(char *str)
{
    int val = 0, loc = 0, start;
    bool doneflag = false;

    while (str[loc] != 0)
    {
        val += str[loc++];
    }

    loc = (val * 3) % SymbolTableSize;
    start = loc;

    while (!doneflag)
    {
        if (SymbolTable[loc].key != NULL)
        {
            if (strcmp(SymbolTable[loc].key, str))
            {
                loc = (loc + 1) % SymbolTableSize;

                if (loc == start)
                {
                    return GT_FAILURE;
                }
            }
            else
            {
                doneflag = true;
            }
        }
        else
        {
            loc = (loc + 1) % SymbolTableSize;

            if (loc == start)
            {
                return GT_FAILURE;
            }
        }   
    }   

    return (loc);
}

//-----------------------------------------------------------------------
static int AppendPointListToLineRecList(
    gt_Point_list *pt, 
    lineRec2d_t **head,
    lineRec2d_t **tail,
    double height = 1)
{
    lineRec2d_t *l;
    double pts[4];
    int highpoint = 0, temphighpt = 0, dummy_val;  
    gt_Point_list *pt2;

    if (pt == NULL) return 0;

    pt2 = pt;
    /* Add a new line for each line segment in the polyline */
    /* N points will generate N-1 connected line segments */
    while (pt2->next != NULL) {
        pts[0] = pt2->x;
        pts[1] = pt2->y;
        pt2 = pt2->next;
        pts[2] = pt2->x;
        pts[3] = pt2->y;

        // calculate the highpt
        if (Terrain != NULL)
        {
            if ((((int)pts[0] >= 0) && ((int)pts[0] < (Terrain->Get_XSize()))) &&
                (((int)pts[1] >= 0) && ((int)pts[1] < (Terrain->Get_YSize()))))
            {
                temphighpt = Terrain->Get_HighestPoint(
                    (int)pts[0],
                    (int)pts[1],
                    (int)pts[2],
                    (int)pts[3],
                    dummy_val,
                    dummy_val);

                if (temphighpt > highpoint)
                {
                    highpoint = temphighpt;
                }
            }
        }
    }    

    while (pt->next != NULL) {
        pts[0] = pt->x;
        pts[1] = pt->y;
        pt = pt->next;
        pts[2] = pt->x;
        pts[3] = pt->y;

        if (!(l = newLineRec(pts, height, highpoint))) return 0;

        if (*head == NULL) 
            *head = *tail = l;
        else {
            (*tail)->next = l;
            *tail = l;
        }
    }
    return highpoint;
}



/**********************************************************************
 **            add_lines_db            **
 ****************************************/
static int add_lines_db(gt_Point_list *pt, double height = 1)
{
  return AppendPointListToLineRecList(pt, &gConsoleDBLineList, &gConsoleDBLineListTail, height);
}

/**********************************************************************
 **          add_doorway_db            **
 ****************************************/
static void add_doorway_db(gt_Point_list *pt)
{
  lineRec2d_t *l;
  double pts[4];
  door_t *d;

  if (pt == NULL || pt->next == NULL) return;

  pts[0] = pt->x;
  pts[1] = pt->y;
  pt = pt->next;
  if (pt == NULL) {
    fprintf(stderr, "Malformed doorway in add_doorway_db()\n");
    return;
  }
  pts[2] = pt->x;
  pts[3] = pt->y;
  
  if (!(l = newLineRec(pts))) return;
  if (!(d = (door_t*)malloc(sizeof(door_t)))) return;
  d->next = NULL;
  d->doorway = l;
  d->marked = 0;
  if (gConsoleDBDoorwayList == NULL) 
    gConsoleDBDoorwayList = gConsoleDBDoorwayListTail = d;
  else {
    gConsoleDBDoorwayListTail->next = d;
    gConsoleDBDoorwayListTail = d;
  }
  return;
}
/**********************************************************************
 **            add_room_db             **
 ****************************************/
static void add_room_db(gt_Point_list *pt)
{
  room_t *r;
  lineRec2d_t *hd = NULL, *tl, *l;
  double pts[4];

  AppendPointListToLineRecList(pt, &hd, &tl);
  if (hd == NULL) return;
  /* Add a line from the second point in tail to 
     the firsst point in head to close the room */
  pts[0] = tl->ls.endpt[1].x;
  pts[1] = tl->ls.endpt[1].y;
  pts[2] = hd->ls.endpt[0].x;
  pts[3] = hd->ls.endpt[0].y;
  if (!(l = newLineRec(pts))) return;
  tl->next = l;
  tl = tl->next;

  /* Allocate a new room structure */
  if (!(r = (room_t*)malloc(sizeof(room_t)))) return;
  r->next = NULL;

  /* Add it to the list */
  r->walls = hd;
  if (debug) {
     printLineRec("Added room", hd);
  }

  if (gConsoleDBRoomList == NULL) 
    gConsoleDBRoomList = gConsoleDBRoomListTail = r;
  else {
    gConsoleDBRoomListTail->next = r;
    gConsoleDBRoomListTail = r;
  }
  return;
}
/**********************************************************************
 **            add_hallway_db          **
 ****************************************/
static void add_hallway_db(gt_Point_list *centerline, double width, gt_Point_list *extents)
{
  hallwayRec_t *h;
  lineRec2d_t *hd = NULL, *tl, *l;
  double pts[4];

  AppendPointListToLineRecList(extents, &hd, &tl);
  if (hd == NULL) return;
  /* Add a line from the second point in tail to 
     the firsst point in head to close the hallway extents */
  pts[0] = tl->ls.endpt[1].x;
  pts[1] = tl->ls.endpt[1].y;
  pts[2] = hd->ls.endpt[0].x;
  pts[3] = hd->ls.endpt[0].y;

  if (!(l = newLineRec(pts))) return;
  tl->next = l;
  tl = tl->next;

  /* Allocate a new hallwayRec structure */
  if (!(h = (hallwayRec_t*)malloc(sizeof(hallwayRec_t)))) return;
  h->next = NULL;

  h->extents = hd;
  h->start.x = centerline->x;
  h->start.y = centerline->y;
  centerline = centerline->next;
  if (centerline == NULL) {
    fprintf(stderr, "Malformed hallway in add_hallway_db()\n");
    return;
  }
  h->end.x = centerline->x;
  h->end.y = centerline->y;

  h->width = width;
  /* Add it to the list */
  
  if (gConsoleDBHallwayList == NULL) 
    gConsoleDBHallwayList = gConsoleDBHallwayListTail = h;
  else {
    gConsoleDBHallwayListTail->next = h;
    gConsoleDBHallwayListTail = h;
  }
  return;
}

/**********************************************************************
 **        add_subunit_members         **
 ****************************************/

static int add_subunit_members(gt_Unit *unit)
{
   /* Go through the tree under this unit and add a UNIT entry to the database
      for each node with a name.  The entry is simply a list of robots that
      should respond to that unit name. */

   gt_Unit *sibling;
   int num_robots, size;
   gt_Unit_members *members;

   /* make sure we have something to work with */
   if (unit == NULL)
      return GT_FAILURE;

   /* propagate this down the tree */
   if (unit->type == UNIT) {
      sibling = unit;
      while (sibling) {

	 /* if this unit has a name, add it and its members to the database */
	 if (sibling->name) {
	    num_robots = num_robots_in_unit(sibling);
	    size = sizeof(gt_Unit_members) +
	              (num_robots-1)*sizeof(gt_Unit_member_info);
	    members = (gt_Unit_members *)malloc(size);
	    members->num_robots = 0;
	    find_unit_members(sibling->children, members);

	    /* insert it into the data base */
	    if (gt_update_db_entry(sibling->name,UNIT_MEMBERS,members,size) == -1) {
	       warn_userf("Error: Unable to update the overlay database with \n\
       UNIT %s (in gt_add_subunit_members).", sibling->name);
	       return GT_FAILURE;
	       }
	    }
	 
	 add_subunit_members(sibling->children);
	 sibling = sibling->next;
	 }
      }
   else {
      sibling = unit;
      while (sibling) {
	 if (sibling->name) {
	    /* enroll this named robot! */
	    size = sizeof(gt_Unit_members);
	    members = (gt_Unit_members *)malloc(size);
	    members->num_robots = 1;
	    members->robot[0].id = sibling->id;
	    members->robot[0].type = sibling->type;

	    /* insert it into the data base */
	    if (gt_update_db_entry(sibling->name,UNIT_MEMBERS,members,size) == -1) {
	       warn_userf("Error: Unable to update overlay database with \n\
       ROBOT %s %s-%d (in gt_add_subunit_members).", sibling->name, 
		       gt_robot_info[sibling->type].name, sibling->id);
	       return GT_FAILURE;
	       }
	    }
	 sibling = sibling->next;
	 }
      }
   return GT_SUCCESS;
}


/**********************************************************************
 **         find_unit_members          **
 ****************************************/

static void find_unit_members(gt_Unit *unit,
			      gt_Unit_members *members)
{
   int i;
   gt_Unit *sibling;
   
   if (unit == NULL) {
      members = NULL;
      return;
      }

   if (unit->type == UNIT) {
      /* go further down the tree */
      sibling = unit;
      while (sibling) {
	 find_unit_members(sibling->children, members);
	 sibling = sibling->next;
	 }
      }
   else {
      /* add the robots in this list to the members */
      sibling = unit;
      while (sibling) {
	 i = members->num_robots;
	 members->robot[i].id = sibling->id;
	 members->robot[i].type = sibling->type;
	 members->num_robots++;
	 sibling = sibling->next;
	 }
      }
}



/**********************************************************************
 **         num_robots_in_unit         **
 ****************************************/

static int num_robots_in_unit(gt_Unit *unit)
{
   gt_Unit *sibling;
   int count = 0;

   if (unit == NULL)
      return 0;

   if (unit->type == UNIT) {
      sibling = unit;
      while (sibling) {
	 count += num_robots_in_unit(sibling->children);
	 sibling = sibling->next;
	 }
      }
   else {
      sibling = unit;
      while (sibling) {
	 count++;
	 sibling = sibling->next;
	 }
      }

   return count;
}



/**********************************************************************
 **      check_measure_draw_label      **
 ****************************************/

static void check_measure_draw_label(char *name, gt_Measure *measure)
/* Check for a trailing asterisk which indicates that the measures
   label shouldn't be drawn.  Remove it and note it if present. */
{
   if (name[strlen(name)-1] == '*') {
      name[strlen(name)-1] = '\0';
      measure->draw_label = FALSE;
      }
}

//-----------------------------------------------------------------------
int load_new_map(char *filename)
{
   return gt_load_overlay(filename);
}



/**********************************************************************
 * $Log: gt_console_db.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:22  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.4  2007/08/30 18:33:39  endo
 * BitmapIconDisplayer class added.
 *
 * Revision 1.3  2007/02/16 21:42:35  endo
 * Debugging statement commented.
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
 * Revision 1.8  2006/07/11 16:29:11  endo
 * CMDLi Panel merged from MARS 2020.
 *
 * Revision 1.7  2006/06/15 15:30:36  pulam
 * SPHIGS Removal
 *
 * Revision 1.6  2006/05/15 01:50:11  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.5  2006/01/30 02:43:17  endo
 * AO-FNC CBR-CNP Type-I check-in.
 *
 * Revision 1.4  2005/08/22 22:12:06  endo
 * For Pax River demo.
 *
 * Revision 1.3  2005/07/31 03:40:42  endo
 * Robot ID specified by CNP can be now recognized by mlab during execution.
 *
 * Revision 1.2  2005/03/23 07:36:55  pulam
 * *** empty log message ***
 *
 * Revision 1.1.1.1  2005/02/06 23:00:10  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.57  2000/10/16 19:39:40  endo
 * Modified due to the compiler upgrade.
 *
 * Revision 1.56  2000/07/07 18:19:45  endo
 * gt_load_waypoint_file() added.
 *
 * Revision 1.55  2000/02/28 23:39:17  jdiaz
 * changed doorways to be stored as a door_t instead of a lineRec2d_t
 *
 * Revision 1.54  2000/02/19 02:03:00  endo
 * Some printfs such as "Added doorway" and "Added
 * room" were put under "if (debug)" statements.
 *
 * Revision 1.53  2000/02/07 05:13:55  jdiaz
 * doorways walls hallways rooms
 *
 * Revision 1.52  1997/02/12 05:42:50  zchen
 * *** empty log message ***
 *
 * Revision 1.51  1997/01/15  21:28:50  zchen
 * add real 3d stuff
 *
 * Revision 1.50  1996/05/17  14:16:16  doug
 * finally got the compiler upgrade errors repaired
 *
 * Revision 1.49  1996/05/17  14:12:35  doug
 * *** empty log message ***
 *
 * Revision 1.48  1996/05/17  14:11:45  doug
 * *** empty log message ***
 *
 * Revision 1.47  1996/05/17  14:08:26  doug
 * fixing compile errors from the upgrade
 *
 * Revision 1.46  1996/03/13  01:52:58  doug
 * *** empty log message ***
 *
 * Revision 1.45  1996/03/09  01:08:43  doug
 * *** empty log message ***
 *
 * Revision 1.44  1996/03/06  23:39:33  doug
 * *** empty log message ***
 *
 * Revision 1.43  1996/03/04  00:04:30  doug
 * *** empty log message ***
 *
 * Revision 1.42  1996/02/29  01:53:18  doug
 * ..
 *
 * Revision 1.41  1996/02/08  19:24:07  doug
 * handle objects of any color
 *
 * Revision 1.40  1995/11/07  14:31:57  doug
 * added baskets and command to drop oranges into them
 *
 * Revision 1.39  1995/10/23  18:21:57  doug
 * Extend object stuff to support obstacles and also add 3d code
 *
 * Revision 1.38  1995/10/11  20:35:47  doug
 * *** empty log message ***
 *
 * Revision 1.37  1995/10/03  21:25:06  doug
 * add support for get_objects call
 *
 * Revision 1.36  1995/06/13  20:31:38  jmc
 * Removed unnecessary defines.
 *
 * Revision 1.35  1995/05/08  16:01:29  jmc
 * Fixed gt_initialize_robot_info() to free a couple things that were
 * overlooked.
 *
 * Revision 1.34  1995/04/25  20:15:58  jmc
 * Used symbolic name instead of constant in
 * gt_initialize_robot_info.  Improved a comment.
 *
 * Revision 1.33  1995/04/25  19:54:29  jmc
 * Added a comment.
 *
 * Revision 1.32  1995/04/20  18:09:23  jmc
 * Corrected spelling.
 *
 * Revision 1.31  1995/04/19  21:53:18  jmc
 * Fixed an insignificant problem that was pointed out by the latest
 * g++ compiler in -Wall mode.
 *
 * Revision 1.30  1995/04/14  21:27:35  jmc
 * Moved the defaults for the diameter/width of some control measures
 * into this file (from gt_overlay_yac.y).  Negative values trigger
 * using defaults.  Added diameter to the function
 * gt_add_assembly_area.
 *
 * Revision 1.29  1995/04/10  15:49:14  jmc
 * Changed include of gt_world.h to draw.h and rearranged includes to
 * prevent redefinition warnings.
 *
 * Revision 1.28  1995/04/05  22:26:27  jmc
 * Now that draw_starting_point is implemented, added actually
 * drawing the starting point to the functions gt_add_starting_point
 * and gt_draw_db.
 *
 * Revision 1.27  1995/04/03  20:16:20  jmc
 * Added copyright notice.
 *
 * Revision 1.26  1995/03/03  21:08:22  jmc
 * Removed the DrawText for gt_add_mission_name and made a call on
 * the new function draw_mission_name implemented in gt_world.c.
 * Also fixed the gt_draw_db so that the mission name is redrawn with
 * everything else.
 *
 * Revision 1.25  1995/02/28  16:47:17  doug
 * move overlay name to 775 Y instead of 25
 *
 * Revision 1.24  1995/02/14  22:15:35  jmc
 * Added function check_measure_draw_label to check for a trailing
 * asterisk to indicate that the measures label shouldn't be drawn.
 * Add a call to this function for all add_* routines which add
 * measures to the console database.
 *
 * Revision 1.23  1995/01/30  15:45:20  doug
 * porting to linux
 *
 * Revision 1.22  1995/01/26  21:27:25  jmc
 * Narrowed some error messages to avoid cropping problems on some
 * systems.
 *
 * Revision 1.21  1994/11/21  17:17:24  jmc
 * Added more debugging printout info to gt_print_robot_info.
 *
 * Revision 1.20  1994/10/31  16:35:36  jmc
 * Changed warn_userf message for out-of-memory conditions to
 * fprintf(stderr,...) commands since it is unlikely that warn_userf
 * would work with no memory.
 *
 * Revision 1.19  1994/10/31  15:55:50  jmc
 * Changed a printf to warn_userf.
 *
 * Revision 1.18  1994/10/28  01:03:18  jmc
 * Minor clean-ups to quiet -Wall.
 *
 * Revision 1.17  1994/10/28  00:36:28  jmc
 * Added function gt_draw_db to draw all the control measures in the
 * database using the draw_* functions.  Converted all the
 * add_*[measure] functions to use the new draw_* functions.
 *
 * Revision 1.16  1994/10/25  20:02:23  jmc
 * Converted gt_add_attack_position, gt_add_battle_position,
 * gt_add_objective, gt_add_passage_point to take a diameter argument
 * to be used when a polyline is not specified, just a single point.
 * It determines the size of circle drawn.
 *
 * Revision 1.15  1994/10/25  15:02:12  jmc
 * Added robot info database (gt_robot_info array) and functions to
 * interact with it: gt_add_new_robot_type, gt_initialize_robot_info,
 * gt_print_robot_info, gt_new_robot_parameters_list,
 * gt_free_robot_parameters_list, gt_robot_type.  Changed print
 * routines and references to the old gt_robot_name to use the new
 * database.
 *
 * Revision 1.14  1994/08/18  20:03:50  doug
 * moved gt_std.h to doug.h
 *
 * Revision 1.13  1994/08/18  19:26:52  doug
 * Converted SUCCESS/FAILURE to GT_SUCCESS/GT_FAILURE
 *
 * Revision 1.12  1994/08/04  18:49:39  jmc
 * Modified gt_add_axis and gt_add_gap to save the width.
 * Took out the defaults for AXIS_WIDTH and GAP_WIDTH and encoded
 * them in gt_overlay_yac.y.  Modified all calls to
 * gt_new_point_measure and gt_new_multipoint_measure to give a
 * width (zero in many cases).  Switched to warn_userf.
 *
 * Revision 1.11  1994/08/04  14:18:48  jmc
 * Modified includes because prototypes for warn_user... were
 * moved to console.h.
 *
 * Revision 1.10  1994/08/03  17:27:48  jmc
 * Changed find_unit_members to void since the return values were
 * never used.
 *
 * Revision 1.9  1994/08/03  16:59:30  jmc
 * Minor hack; changed a couple 'return 0' to 'return FAILURE' to
 * improve readability.
 *
 * Revision 1.8  1994/08/02  20:57:37  jmc
 * Added 'return GT_SUCCESS' to bottom of add_subunit_members to
 * indicate GT_SUCCESSful completion.
 *
 * Revision 1.7  1994/07/26  21:35:33  jmc
 * Modified gt_get_db_measure to be less case sensitive.  Now it
 * tries all lowercase, capitalized, and all uppercase.  Also fixed a
 * minor bug that let a name whose beginning matched a prefix get
 * stripped to nothing by adding a test for the hyphen necessary
 * after a prefix.
 *
 * Revision 1.6  1994/07/26  20:11:14  jmc
 * Numerous minor hacks to make all the gt_add* routines return a
 * FAILURE/SUCCESS value.
 *
 * Revision 1.5  1994/07/26  17:29:20  jmc
 * Converted error messages to use error dialog box.
 *
 * Revision 1.4  1994/07/19  21:14:33  jmc
 * Added #define GT_CONSOLE_DB_C to allow header file to compile strings into the
 * object file for this file and not make copies everywhere.
 *
 * Revision 1.3  1994/07/18  20:21:05  jmc
 * Put gt_world.h first in includes to avoid compiler complaints.
 *
 * Revision 1.2  1994/07/12  19:07:37  jmc
 * Added RCS automatic identification strings
 **********************************************************************/
