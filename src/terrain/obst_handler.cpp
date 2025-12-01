/**********************************************************************
 **                                                                  **
 **                        obst_handler.cpp                          **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: obst_handler.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
//#include <GL/glut.h>
#include <GL/glu.h>
#include "3d_vis_wid.h"

ObstacleHandler::ObstacleHandler(TerrainMap *hm) {
  ObstacleList = new SLList();
  OH_HeightMap = hm;
}

// delete all the robots currently tracked
ObstacleHandler::~ObstacleHandler(void) {
  Obstacle_Cyl *temprec;

  ObstacleList->Reset_List();
  while (ObstacleList->Get_ListSize()) {
    temprec = (Obstacle_Cyl *)ObstacleList->Delete_Head();
    delete temprec;
  }
}

float ObstacleHandler::Place_Obstacle(gt_Measure *walldata, float height, double highpt) {
   int highpoint = -1;
   int ysize = OH_HeightMap->Get_YSize();

   Building_3d *tempobs;
   Obstacle_Record *temprec = new Obstacle_Record;

   ObstacleList->Reset_List();

   tempobs = new Building_3d(walldata, height, ysize, (int)highpt);

   temprec->Obstacle_Type = WALL_VIS;
   temprec->Obs_Rec = (void *)tempobs;

   ObstacleList->Insert_Rear((void *)temprec);
   return highpoint;
}

   
// add an Obstacle
float ObstacleHandler::Place_Obstacle(double x, double z, double height, double radius) {
  float highpoint = -1, lowpoint = 9999;
  int cnt, cnt2;
  float tempheight;
  Obstacle_Record *obsrec = new Obstacle_Record;
  
  Obstacle_Cyl *temprec;
  ObstacleList->Reset_List();

  // first we want to find the high an low point of the terrain on which the object resides
  // in order to eliminate the possibility of the terrain overdrawing the obstacle
  //    the height of the obstacle will be assumed to start at the highest point of the covered
  //    terrain and to make it look pretty it will also extend to the lowest point
  //    we assume the object is a square to simplify the calculations with sides of radius * 2 units

  for (cnt = (int)x - (int)radius; cnt <= (int)x + (int)radius; cnt++) {
    for (cnt2 = (int)z - (int)radius; cnt2 <= (int)z + (int)radius; cnt2++) {

      // skip terrain that isn't mapped
      if ((cnt2 >= 0) && (cnt >= 0)) {

	// find the high and low points
	tempheight = OH_HeightMap->Get_Height(cnt, cnt2);
	if (tempheight > highpoint) highpoint = tempheight;
	if (tempheight < lowpoint) lowpoint = tempheight;
      }
    }
  }
  
  temprec = new Obstacle_Cyl(x, lowpoint, z, height + (highpoint - lowpoint), radius);

  obsrec->Obstacle_Type = CYLINDER_VIS;
  obsrec->Obs_Rec = (void *)temprec;
  ObstacleList->Insert_Rear((void *)obsrec);
  return highpoint;
}	

void ObstacleHandler::Draw(void) {
  Obstacle_Cyl *temprec;
  Building_3d *temprec_wall;
  Obstacle_Record *temp_obsrec;

  // go through our robot list and draw all the robots
  ObstacleList->Reset_List();
  if (ObstacleList->Get_ListSize()) {
    do {
      temp_obsrec = (Obstacle_Record *)ObstacleList->Get_Data();
      if (temp_obsrec->Obstacle_Type == CYLINDER_VIS) {
	temprec = (Obstacle_Cyl *)temp_obsrec->Obs_Rec;
	temprec->Draw_Obstacle(temprec->Get_Y());
      }
      else if (temp_obsrec->Obstacle_Type == WALL_VIS) {
	temprec_wall = (Building_3d *)temp_obsrec->Obs_Rec;
        temprec_wall->Draw_Obstacle(0);
      }
    } while (ObstacleList->Next_Node());
  }
}

/**********************************************************************
 * $Log: obst_handler.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.3  2006/05/15 01:04:51  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.2  2005/12/07 05:30:44  endo
 * #include <GL/glut.h> commented.
 *
 *
 **********************************************************************/
