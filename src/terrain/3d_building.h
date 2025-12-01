/**********************************************************************
 **                                                                  **
 **                           3d_building.h                          **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 - 2006 Georgia Tech Research Corporation          **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: 3d_building.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef __BUILDING_3D__
#define __BUILDING_3D__

#include "gt_measure.h"

class Building_3d {
public:
  Building_3d(gt_Measure *walls, float height, int ysize, double hightpt);
  ~Building_3d(void);
  void Draw_Obstacle(float yoffset);

private:
  gt_Measure *Walls_3d;
  float Height;
  int Num_Walls;
  int Y_Size;
  int HighPt;
};

#endif /* __BUILDING_3D__ */

/**********************************************************************
 * $Log: 3d_building.h,v $
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
