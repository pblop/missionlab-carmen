/**********************************************************************
 **                                                                  **
 **                     ProjectDefinitions.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Project includes and useful structures                  **
 **                                                                  **
 **********************************************************************/

/* $Id: ProjectDefinitions.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef PROJECTDEFINITIONS_H
#define PROJECTDEFINITIONS_H

#include <list>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <string>
#include <vector>
#include <queue>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>

#include "stdlib.h"

using namespace std;

/*
#include "gt_message_types.h"
#include "gt_std.h"
#include "gt_simulation.h"
*/

struct stGoalString
{
	string goalPositionX;
	string goalPositionY;
};


#define GTIP_INFINITY 9999
#define OBSTACLEVALUE 9998
#define MISSIONLAB_SIMULATION

enum FeatureField	{ TIMEOUT, ROBOT_ID, COMMUNICATION };
enum Shape	{	CIRCLE, SQUARE, WALL, EXACTPOINT	};
enum Direction {	NONE, VERTICAL, HORIZONTAL	};
enum MapType { OBSTACLE_MAP, COMMUNICATION_MAP };
enum CompletionType { SOFT, HARD, INORDER };


#endif

/**********************************************************************
 * $Log: ProjectDefinitions.h,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
