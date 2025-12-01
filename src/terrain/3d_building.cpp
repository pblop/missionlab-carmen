/**********************************************************************
 **                                                                  **
 **                          3d_building.cpp                         **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 - 2006 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: 3d_building.cpp,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#include <stdio.h>
#include "3d_building.h"
#include "3d_vis_wid.h"

Building_3d::Building_3d(gt_Measure *walldata, float height, int ysize, double highpt)
{
  Height = height;
  Num_Walls = walldata->data.num_points;
  Walls_3d = walldata;
  Y_Size = ysize;
  HighPt = (int)highpt;
}

Building_3d::~Building_3d(void) {
  delete []Walls_3d;
}

// draw the obstacles as a sequence of quads of height, Height, to represent walls.  need to add code for roof
void Building_3d::Draw_Obstacle(float yoffset) {
   int cnt;
  GLfloat mat_colors[4];
  mat_colors[0] = 0.6f;
  mat_colors[1] = 0.6f;
  mat_colors[2] = 0.6f;
  mat_colors[3] = 0.75f;
  PVector pv1, pv2, pv3;
                                                                                                           
  glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_colors);

  glBegin(GL_QUADS);
  for (cnt = 0; cnt < Num_Walls - 1; cnt++) {
    // bottom left

    glVertex3i((int)Walls_3d->data.point[cnt].x, (int)yoffset, Y_Size - (int)Walls_3d->data.point[cnt].y);

    // top left
    glVertex3i((int)Walls_3d->data.point[cnt].x, (int)yoffset + (int)Height + (int)HighPt,  Y_Size - (int)Walls_3d->data.point[cnt].y);

    // top right
    glVertex3i((int)Walls_3d->data.point[cnt + 1].x, (int)yoffset + (int)Height + (int)HighPt,  Y_Size - (int)Walls_3d->data.point[cnt + 1].y);

    // bottom left
    glVertex3i((int)Walls_3d->data.point[cnt + 1].x, (int)yoffset,  Y_Size - (int)Walls_3d->data.point[cnt + 1].y);

  }
  glEnd();
  
}

/**********************************************************************
 * $Log: 3d_building.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/01/10 06:12:43  endo
 * AO-FNC Type-I check-in.
 *
 **********************************************************************/
