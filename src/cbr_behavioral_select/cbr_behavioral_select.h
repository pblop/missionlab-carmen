/**********************************************************************
 **                                                                  **
 **                     cbr_behavioral_select.h                      **
 **                                                                  **
 ** Contains all the includes for the CBR Utility functions          **
 ** used in the Mlab and CBR                                         **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: cbr_behavioral_select.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include <vector>
#include <string.h>	//Old compilation error: several functions not defined

// ENDO
//using namespace std;
using std::vector;

/**Defines**/

#define FSALIST_KEY   "FSA_LIST"


struct structRobotPos;
struct structObstaclePos;

/*Function Prototypes*/
int GetCurrentStateandTask(char sState[], char sTask[]);
bool PrintReadings();
bool GetSensorReadings(structRobotPos& RobotPos,
                       vector<structObstaclePos>& ObstaclesPosVector, int &nNumofObstacles);

void UpdateMlab(char* strUpdateString);


/**********************************************************************
 * $Log: cbr_behavioral_select.h,v $
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
 * Revision 1.1  2003/04/05 23:51:36  kaess
 * Initial revision
 *
 **********************************************************************/
