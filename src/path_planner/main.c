/**********************************************************************
 **                                                                  **
 **                               main.c                             **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:                                                     **
 **                                                                  **
 **  Copyright 2000, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: main.c,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

/******************************************************************************

				main 

******************************************************************************/
/* these 4 defines no longer used... */
                                     /* Nomad 150 */         /* Old Dennings */
#define ROBOT_RADIUS			0.30 /*in meters */  /* 1.4  in feet */
#define SAFETY_MARGIN   		0.35 /*in meters */  /* 0.5  in feet */
#define DEFAULT_CONVEX_SELECT_MODE 	3   /* most convex */
#define DEFAULT_CONCAVE_ANGLE_MODE 	1   /* 1 = most */

/*---------------------------------------------------------------------------*/
/* preliminaries                   				             */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "std.h"
#include "map_editor.h"
#include "map_display.h"
#include "map_bldr.h"
#include "nav.h"
#include "generateCDL.h"
#include "new_long_term_memory.h"
#include "cart_lib.h"

typedef struct plannerState {
  double robot_radius;     /* in meters */
  double safety_margin;    /* in meters */
  int   convex_select_mode; /* 3 is most convex */
  int   concave_angle_mode; /* 1 is most concave */
} plannerState_t;

typedef plannerState_t *plannerState_pt;

/* This function reads in planner parameters and 
 * packs the planner state structure 
 */

void load_planner_parameters(plannerState_pt PS){
  FILE *fp;
  char garb[60];
  char absolute[100];
  char *path = NULL;
  int len;
  
  path = getenv("MLAB_HOME");
  if(path){ /* Look for the parameter file in the appropriate directory */
    sprintf(absolute, "%s", path);
    len = strlen(absolute);
    if(len > 100) {
      printf("\nMLAB_HOME environment variable exceeds buffer length in path planner.\n");
      printf("Trying default values.\n");
      fp = NULL;
    }
    if(absolute[len-1] == '/') absolute[len-1] = '\0';
    strcat(absolute, "/src/path_planner/");
    strcat(absolute, PLANNER_PARAM_FILE);
    fp = fopen(absolute, "r");
  }
  else{ /* Try the current directory */
    fp = fopen(PLANNER_PARAM_FILE, "r");
  }
  if(!fp){   /* No luck -- go with hard-coded values. */
    printf("\nCouldn't open path planner parameter file.  Trying default values.\n");
    PS->robot_radius = ROBOT_RADIUS;
    PS->safety_margin = SAFETY_MARGIN;
    PS->convex_select_mode = 3;
    PS->concave_angle_mode = 1;
    return;
  }
  
  fscanf(fp, "%s %lf ", garb, &(PS->robot_radius));
  fscanf(fp, "%s %lf ", garb, &(PS->safety_margin));
  fscanf(fp, "%s %d ",  garb, &(PS->convex_select_mode));
  fscanf(fp, "%s %d ",  garb, &(PS->concave_angle_mode));

  fclose(fp);
}

/* eventually, these should come from the overlay file. */
double g_StartX = 119.0;
double g_StartY = 14.0;


int main(int argc, char *argv[])
{
   char map_file[120];
   int  answer = 0;
   double startx, starty, goalx, goaly;

   plannerState_t PState;
   plannerState_pt PS = &PState;

   struct path_node *path;

   int npoints = 0;
   float path_points_X[100];
   float path_points_Y[100];
 
   char outputFileName[255];
   
   if(argc == 1)
	 {
	   printf("Enter map file name >");
	   scanf("%s", map_file);
	   strcpy(outputFileName, "points.txt");
	 }
   else if(argc == 2)
	 {
	   strcpy(map_file, argv[1]);
	   strcpy(outputFileName, "points.txt");
	 }
   else if(argc == 3)
	 {
	   strcpy(map_file, argv[1]);
	   strcpy(outputFileName, argv[2]);
	 }
   else if(argc > 3)
	 {
	   fprintf(stderr, "\nToo many command line arguments!\n\n");
	   exit(1);
	 }

   /* RRB: added this so parameters could be modified at run-time */
   load_planner_parameters(PS);

   open_map_display();

   /* Call with arguments 2 instead of 1 for more detailed step by
      step window display of what the code does */
   load_map(map_file, 1, PS->robot_radius, PS->safety_margin);

   map_builder(1, PS->convex_select_mode, PS->concave_angle_mode);

   answer = 1;
   while(answer != 0)
     {
       printf("\nLeft click on start location\n");
       if(pick_xy(TRUE, &startx, &starty)) exit(1);

       printf("starting x,y : %5.2f\t%5.2f\n", startx, starty);

       printf("\nSelect goal location\n");
       if(pick_xy(TRUE, &goalx, &goaly)) exit(1);

       printf("goal x,y : %5.2f\t%5.2f\n", goalx, goaly);
   
       /* NOTE: safety_margin set to 0.0 */
       path = navigator(startx, starty, 0, goalx, goaly, 0, TRUE, PS->robot_radius*2.0, 0.0);

       /*=======================================================*/
       /* Prepare a list of path points and call a function     */
       /*  to generate a CDL file                               */
       /*=======================================================*/
       npoints = 0;
	  
       path_points_X[npoints]   = startx;
       path_points_Y[npoints++] = starty;

       //       if(1){
       printf("\nPath:\n===============\n");
       printf("%g %g  (start)\n", startx, starty);
       while((path->next_point) != NULL)
	 {
	   path_points_X[npoints]   = path->x;
	   path_points_Y[npoints++] = path->y;
	   
	   printf("%g %g\n", path->x, path->y);
	   path = path->next_point;
	 }
       printf("%g %g  (goal)\n", goalx, goaly);
       path_points_X[npoints]   = goalx;
       path_points_Y[npoints++] = goaly;
       //       }
       /* For each path segment print out the region that it traverses */

       /* generateCDLfile(path_points_X, path_points_Y, npoints); */
       generateTextFile(path_points_X, path_points_Y, npoints, outputFileName);

       if(mouse_read()) answer = 0;
       else answer = 1;

     }

   close_map_display();

   return 0;
}

/**********************************************************************
 * $Log: main.c,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:51  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:00  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/07/10 21:51:16  pulam
 * Fixed pathplanner
 *
 * Revision 1.2  2006/05/14 05:57:38  endo
 * gcc-3.4 upgrade
 *
 **********************************************************************/
