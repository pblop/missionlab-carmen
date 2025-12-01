/**********************************************************************
 **                                                                  **
 **                           3d_vis_wid.h                           **
 **                                                                  **
 ** Written by: Patrick Daniel Ulam                                  **
 **                                                                  **
 ** Copyright 2005 Georgia Tech Research Corporation                 **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **                                                                  **
 **********************************************************************/

/* $Id: 3d_vis_wid.h,v 1.1.1.1 2008/07/14 16:44:24 endo Exp $ */

#ifndef VIS_3D_H
#define VIS_3D_H

#include <GL/glu.h>
#include <GL/glx.h>
#include <GL/gl.h>
//#include <GL/GLwMDrawA.h>
#include <Xm/Xm.h>
#include <Xm/MwmUtil.h>
#include "sllist.h"
#include "pmouse.h"
#include "3drobot.h"
#include "3d_obstacle.h"
#include "3d_building.h"
#include "terrainmap.h"
#include "robothandler.h"
#include "obst_handler.h"
#include "gt_measure.h"

#define MAP_SIZE 256
#define STEP_SIZE 5
#define HEIGHT_RATIO 1.0f

extern int *dblBuf_3dvis;
extern int *snglBuf_3dvis;

extern XVisualInfo *visualinfo_3dvis;


extern Display *display_3dvis;
extern bool doubleBuffer;
extern GLXContext glcontext_3dvis;

extern float Eye[3];
extern float Rotate[3];

extern TerrainMap *Terrain;
extern ObstacleHandler *Obs_Handler;
extern RobotHandler *Rob_Handler;
extern PMouse *Mouse_3d;

extern float ScaleValue;

extern bool gt_show_3d_vis;
extern unsigned char *CurHeightMap;
extern char *heightmap_name;
extern int heightmap_x;
extern int heightmap_y;
extern int *obshighpts;


typedef struct {
  double X;
  double Z;
  double Height;
  double Radius;
} Obs_3d_Rec;

typedef struct {
  int RobotNum;
  double X;
  double Y;
  double Z;
} Robot_3d_Rec;

typedef struct {
  gt_Measure *Walls;
  float Height;
  int HighPt;
} Wall_List_Element;

void drawGLScene(void);
void initGL(void);
void resizeGLScene(unsigned int width, unsigned int height);
void vis_3d_input(Widget w, XtPointer UxClientData, XtPointer UxCallbackArg);
void initialize_3d_vis(void);
bool load_heightmap(char *name, int x, int y, int sl, int scale);
int sim_get_elevation(int x, int y);
bool is_valid_terrain(int robot_id);
void init3DVisEye(void);


#endif

/**********************************************************************
 * $Log: 3d_vis_wid.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:24  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.5  2006/06/26 21:29:52  endo
 * Fix for Fedore Core 5.
 *
 * Revision 1.4  2006/01/10 06:12:43  endo
 * AO-FNC Type-I check-in.
 *
 * Revision 1.3  2005/12/07 05:30:44  endo
 * #include <GL/glut.h> commented.
 *
 * Revision 1.2  2005/07/23 06:02:05  endo
 * 3D visualization improved.
 *
 **********************************************************************/
