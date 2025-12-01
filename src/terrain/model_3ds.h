/**********************************************************************
 **                                                                  **
 **                            model_3ds.h                           **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2006 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: model_3ds.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef MODEL_3DS_H
#define MODEL_3DS_H


#include "pvector.h"
#include <stdlib.h>	//Old compilation error: exit and free not defined
#include <string.h>	//Old compilation error: strdup not defined

struct PolygonInd {
  int A;
  int B;
  int C;
};

enum Rot_Axis {
  X_AXIS,
  Y_AXIS,
  Z_AXIS
};
 


class Model_3ds {
 public:
  Model_3ds(char *filename, float scalex, float scaley, float scalez,
	    float rx, float ry, float rz, bool norminv, Rot_Axis ra);
  ~Model_3ds(void);
  void Draw_Model(float x, float y, float z, float theta);
  void Draw_Model(float x, float y, float z, float theta, float scx, float scy, float scz);


 private:
  void Load_Model(void);
  void Calculate_Normals(bool invert);

  float ScaleX;
  float ScaleY;
  float ScaleZ;
  float RotX;
  float RotY;
  float RotZ;
  Rot_Axis RotAx;

  int NumVerts;
  int NumPoly;

  PPoint *Vertex;
  PVector **Normal;
  PolygonInd *Polygon;

  char *Name;
};

#endif

/**********************************************************************
 * $Log: model_3ds.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/07/04 16:51:34  endo
 * Model_3ds added.
 *
 **********************************************************************/
