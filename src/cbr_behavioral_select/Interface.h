/**********************************************************************
 **                                                                  **
 **                         Interface.h                              **
 **                                                                  **
 ** Contains all the includes for the CBR algorithm that should      **
 ** be visible to the outside world.                                 **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Interface.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBRINTERFACE_H__
#define _CBRINTERFACE_H__

/**Defines**/

/**Gains for each primitive behavior that are passed by Database*/
#define MOVETOGOAL_GAIN         "MoveToGoal_Gain"
#define NOISE_GAIN              "Noise_Gain"
#define NOISE_PERSISTANCE       "Noise_Persistence"
#define OBSTACLE_GAIN           "Obstacle_Gain"
#define OBSTACLE_SPHERE         "Obstacle_Sphere"
#define BIAS_VECTOR_X           "Bias_Vector_X"
#define BIAS_VECTOR_Y           "Bias_Vector_Y"
#define BIAS_VECTOR_GAIN        "Bias_Vector_Gain"

/**The default values for the above*/
#define MOVETOGOAL_GAIN_DEFAULT         1.0
#define NOISE_GAIN_DEFAULT              0.2
#define NOISE_PERSISTANCE_DEFAULT      10
#define OBSTACLE_GAIN_DEFAULT           1.0
#define OBSTACLE_SPHERE_DEFAULT         1.5
#define BIAS_VECTOR_X_DEFAULT           0.0
#define BIAS_VECTOR_Y_DEFAULT           0.0
#define BIAS_VECTOR_GAIN_DEFAULT        0.0

/**The lower and upper values for the above*/
#define MOVETOGOAL_GAIN_LOWER           0.1
#define NOISE_GAIN_LOWER                0.0
#define NOISE_PERSISTANCE_LOWER         5
#define OBSTACLE_GAIN_LOWER             0.2 //
#define OBSTACLE_SPHERE_LOWER           0.5
#define BIAS_VECTOR_X_LOWER             0.0
#define BIAS_VECTOR_Y_LOWER             0.0
#define BIAS_VECTOR_GAIN_LOWER          0.0

#define MOVETOGOAL_GAIN_UPPER           1.5
#define NOISE_GAIN_UPPER                1.5
#define NOISE_PERSISTANCE_UPPER        50
#define OBSTACLE_GAIN_UPPER             2.0
#define OBSTACLE_SPHERE_UPPER          15.0
#define BIAS_VECTOR_X_UPPER             0.0
#define BIAS_VECTOR_Y_UPPER             0.0
#define BIAS_VECTOR_GAIN_UPPER          0.0


/**Adaptability of the above**/
#define MOVETOGOAL_GAIN_ADAPT           true
#define NOISE_GAIN_ADAPT                true
#define NOISE_PERSISTANCE_ADAPT         true
#define OBSTACLE_GAIN_ADAPT             false
#define OBSTACLE_SPHERE_ADAPT           true
#define BIAS_VECTOR_X_ADAPT             false
#define BIAS_VECTOR_Y_ADAPT             false
#define BIAS_VECTOR_GAIN_ADAPT          false



//debugging option
#define DEBUG   1

//=====input readings data types========================================================
struct PT {
  double fX;
  double fY;
  PT() {
    fX = 0.0;
    fY = 0.0;
  };
  PT(double fNewX, double fNewY) {
    fX = fNewX;
    fY = fNewY;
  };
};

struct structRobotPos {
  PT  ptC;
  double fHeading;
  structRobotPos() {
    ptC = PT(0.0, 0.0);
    fHeading = 0.0;
  };
};

struct structObstaclePos {
  PT  ptC;
  double fRadius;
  double len;  //mk, for speedup in SortbyDistance(), CBRUtils.cpp
};
#if 1
struct lt_structObstaclePos {
  bool operator()(structObstaclePos op1, structObstaclePos op2) {
    return (op1).len < (op2).len;
  }
};
#endif
//=======================================================================================


#ifdef WINCOMPILER
#define put_state(x,y) printf("put_state: %s, %s\n", x, y)
#endif


/**Function Prototypes*/
bool CBRSelectandApplyCase(PT GoalPos);
bool CBRDestroy();
#endif



/**********************************************************************
 * $Log: Interface.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:26  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/05 23:36:22  kaess
 * Initial revision
 *
 **********************************************************************/
