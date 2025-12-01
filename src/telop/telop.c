/**********************************************************************
 **                                                                  **
 **                         telop.c                                  **
 **                                                                  **
 **  Written by:  Khaled S.  Ali                                     **
 **                                                                  **
 **  All the functions that mlab needs to know about to create and   **
 **  start the Telop interface.                                      **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: telop.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include <malloc.h>

#include "UxXt.h"
#include "gt_com.h"
#include "shared.h"
#include "telop.h"
#include "telop_window.h"
#include "status_window.h"
#include "personality_window.h"

Widget          telop_interface;
Widget          status_interface;
Widget 	        personality_interface;
XtAppContext    UxAppContext;
Widget          UxTopLevel;
Display         *UxDisplay;
int             UxScreen;

int gPersonalityWindow_numPersonalities = 0;
personality_type gPersonalityWindow_personalities[9];

int telop_num_robots = 0;
bool bTelopStart    = FALSE;
int *telop_robot = NULL;
char *telop_unit_name = NULL;

int  basic_up = 0;
int  personality_up = 0;

void read_personality_file(char * filename);


void gt_create_telop_interface(Widget top_level, XtAppContext app, char *
personality_filename)
{
    UxTopLevel = top_level;
    UxAppContext = app;
    UxDisplay = XtDisplay(UxTopLevel);
    UxScreen = XDefaultScreen(UxDisplay);

    telop_interface = create_main_telop_window(NULL);

    status_interface = create_main_status_window(NULL);

    read_personality_file(personality_filename);

    if (gPersonalityWindow_numPersonalities > 0)
    {
	personality_interface = create_personality_window(top_level);
    }

}



void  gt_popup_telop_interface(char *unit_name, int num_robots, int *robot_id)
{
   int i;
   char labelstring[50];

  if (!personality_up && !basic_up)
   {
    /* copy the unit information */
    telop_num_robots = num_robots;
    if (telop_robot != NULL) {
       free(telop_robot);
       telop_robot = NULL;
       }
    if (robot_id != NULL) {
       telop_robot = (int *)malloc(sizeof(int)*num_robots);
       for (i=0; i<num_robots; i++)
	  telop_robot[i] = robot_id[i];
       }
    if (telop_unit_name != NULL) {
       free(telop_unit_name);
       telop_unit_name = NULL;
       }
    if (unit_name) {
       telop_unit_name = (char *)malloc(strlen(unit_name)+1);
       strcpy(telop_unit_name, unit_name);
       }

    /* Generate the label */
    if (telop_unit_name == NULL)
       UxPutLabelString(unitname_labelGadget, "");
    else {
       sprintf(labelstring, "Teleoperating Unit '%s'", telop_unit_name);
       UxPutLabelString(unitname_labelGadget, labelstring);
       free(labelstring);
       }


     /* Tell all the robots to begin the TELOP command */
/*     if (telop_num_robots != 0)
      {
	for (i=0; i<telop_num_robots; i++)
         {
*/
	   /* Send the messages to each robot in the unit */
/*	   gt_update__FiPcT1(telop_robot[i], "teleop_complete", "0");
	 }
      }
*/
   }

   /* Finally, pop up the interface */
   UxPopupInterface(telop_interface, no_grab);

   UxPopupInterface(status_interface, no_grab);

   basic_up = 1;

   bTelopStart = TRUE;

}


void  gt_popup_telop_personality(char *unit_name, int num_robots, int *robot_id)
{
    int i;
    char labelstring[50];

    if (gPersonalityWindow_numPersonalities > 0)
    {
        if (!personality_up && !basic_up)
        {
            /* copy the unit information */
            telop_num_robots = num_robots;
            if (telop_robot != NULL) {
                free(telop_robot);
                telop_robot = NULL;
            }
            if (robot_id != NULL) {
                telop_robot = (int *)malloc(sizeof(int)*num_robots);
                for (i=0; i<num_robots; i++)
                    telop_robot[i] = robot_id[i];
            }
            if (telop_unit_name != NULL) {
                free(telop_unit_name);
                telop_unit_name = NULL;
            }
            if (unit_name) {
                telop_unit_name = (char *)malloc(strlen(unit_name)+1);
                strcpy(telop_unit_name, unit_name);
            }

            /* Generate the label */
            if (telop_unit_name == NULL)
                UxPutLabelString(unitname_labelGadget, "");
            else {
                sprintf(labelstring, "Teleoperating Unit '%s'", telop_unit_name);
                UxPutLabelString(unitname_labelGadget, labelstring);
                free(labelstring);
            }
        }

        UxPopupInterface(personality_interface, no_grab);
        personality_up = 1;
    }
    else
    {
        perror("Warning: No personalities are defined.");
        personality_up = 0;
    }
}



void  gt_end_teleoperation(int  window)
{
   int i;

   if (window == 1)
     basic_up = 0;
   else if (window == 2)
     personality_up = 0;


     if (telop_num_robots <= 0)
       /* No unit specified, so broadcast */
	 gt_update(BROADCAST_ROBOTID,"teleop_complete", "1");
     else
      {
       /* Tell all the robots to complete the TELOP command */
       for (i=0; i<telop_num_robots; i++)
        {
         /* Send the messages finish to each robot in the unit */
         gt_update(telop_robot[i], "teleop_complete", "1");
         }
      }

   if (!basic_up && !personality_up)
    {
	/* clear the globals */
	free(telop_unit_name);
	free(telop_robot);
	telop_num_robots = 0;
    }

   /* Finally, pop down the interface */
   UxPopdownInterface(telop_interface);

   UxPopdownInterface(status_interface);

   bTelopStart = FALSE;

}

void read_personality_file(char * filename)
{
    FILE  *fp;
    int  i, j;
    char  default_filename[150];

    /* DCM: June 23, 1995.
       Need to make sure that you got the file before you read from it.
    */
    if ( (filename == NULL) || (fp = fopen(filename, "r")) == NULL )
    {
        if (filename != NULL)
            perror("Warning: did not find personality file, using defaults");

        sprintf(default_filename, "%s/src/telop/personality_file", MLAB_HOME);

        if ( (default_filename==NULL) || (fp = fopen(default_filename, "r")) == NULL )
        {
            /* Use default values */
            gPersonalityWindow_numPersonalities = 2;

            strcpy(gPersonalityWindow_personalities[0].title, "Aggressiveness");
            gPersonalityWindow_personalities[0].num_params = 2;
            strcpy(gPersonalityWindow_personalities[0].params[0].key,
                   "navigation_move_to_goal_gain");
            gPersonalityWindow_personalities[0].params[0].base = 0.8;
            gPersonalityWindow_personalities[0].params[0].inc = 1;
            strcpy(gPersonalityWindow_personalities[0].params[1].key,
                   "navigation_avoid_obstacle_gain");
            gPersonalityWindow_personalities[0].params[1].base = 1.5;
            gPersonalityWindow_personalities[0].params[1].inc = 0;

            strcpy(gPersonalityWindow_personalities[1].title, "Wanderlust");
            gPersonalityWindow_personalities[1].num_params = 2;
            strcpy(gPersonalityWindow_personalities[1].params[0].key, "navigation_noise_gain");
            gPersonalityWindow_personalities[1].params[0].base = 0.1;
            gPersonalityWindow_personalities[1].params[0].inc = 1;
            strcpy(gPersonalityWindow_personalities[1].params[1].key,
                   "navigation_formation_gain");
            gPersonalityWindow_personalities[1].params[1].base = 1.0;
            gPersonalityWindow_personalities[1].params[1].inc = 0;
        }
    }

    if (fp != NULL)
    {
        fscanf(fp, "Number of sliders: %d\n", &gPersonalityWindow_numPersonalities);

        for (i=0; i<gPersonalityWindow_numPersonalities; i++)
        {
            fscanf(fp, "title: %s\n", gPersonalityWindow_personalities[i].title);
            fscanf(fp, "num_params: %d\n", &(gPersonalityWindow_personalities[i].num_params));
            for (j=0; j<gPersonalityWindow_personalities[i].num_params; j++)
            {
                fscanf(fp, "key: %s\n", gPersonalityWindow_personalities[i].params[j].key);
                fscanf(fp, "base: %lf\n", &(gPersonalityWindow_personalities[i].params[j].base));
                fscanf(fp, "inc: %d\n", &(gPersonalityWindow_personalities[i].params[j].inc));
            }
        }

        fclose(fp);
    }
}



/**********************************************************************
 * $Log: telop.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/07/07 04:21:20  endo
 * Personality Window debugged.
 *
 * Revision 1.1.1.1  2005/02/06 23:00:20  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.24  2003/04/02 21:27:18  zkira
 * Added code for status window GUI
 *
 * Revision 1.23  2002/07/18 17:05:08  endo
 * libtelop.a now compiles with g++.
 *
 * Revision 1.22  2000/02/29 22:03:54  saho
 * Updated include file telop_window.h instead of main_window.h
 *
 * Revision 1.21  1996/06/06 13:27:43  kali
 * *** empty log message ***
 *
 * Revision 1.20  1995/08/09  19:13:22  kali
 * personality sliders now adjust any number of parameters
 *
 * Revision 1.19  1995/07/06  19:03:21  kali
 * Added parameter to gt_create_telop_interface to specify personality file
 * name.
 *
 * Revision 1.18  1995/06/27  15:53:38  kali
 * Made changes to accomodate a seperate menu entry in the console window for
 * the personality window.
 *
 * Revision 1.17  1995/06/23  19:42:13  doug
 * Didn't check if got the personality file before it read from it.
 *
 * Revision 1.16  1995/06/20  20:45:32  kali
 * includes personality stuff again.
 *
 * Revision 1.15  1995/06/14  20:29:47  kali
 * *** empty log message ***
 *
 * Revision 1.11  1995/06/05  19:00:43  kali
 * made several changes for advanced teleautonomy
 *
 * Revision 1.10  1995/05/30  17:35:27  kali
 * Added stuff for personalities.
 *
 * Revision 1.9  1995/05/08  20:11:23  jmc
 * Added some cleanup to gt_end_teleoperation to clear the global
 * variables.
 *
 * Revision 1.8  1995/05/08  17:30:24  jmc
 * Fixed problem in gt_popup_telop_interface by allocating space for
 * labelstring.
 *
 * Revision 1.7  1995/05/08  14:50:07  kali
 * Added code to gt_popup_telop_interface to set the label correctly.
 *
 * Revision 1.6  1995/05/03  21:58:57  jmc
 * Implemented the gt_end_teleoperation() function.
 *
 * Revision 1.5  1995/05/03  19:12:05  jmc
 * Added definitions of several unit-related variables.  Added
 * arguments to interface functions to avoid using external globals.
 * Added code to the popup function to copy the specified unit to the
 * global variables.   Added RCS strings.
 **********************************************************************/
