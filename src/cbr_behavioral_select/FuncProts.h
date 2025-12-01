/**********************************************************************
 **                                                                  **
 **                          FuncProts.h                             **
 **                                                                  **
 ** Contains the declaration of other CBR related data types         **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: FuncProts.h,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#ifndef _CBR_FUNCPROTS_
#define _CBR_FUNCPROTS_



char* Float2Ascii(double fVal, int nPrec, char* strVal);
void Apply(char* strID, char* strValue);
structFeature ComputeGoalDistFeature(PT GoalPos);
double ComputeLength(PT ptStart, PT ptEnd=PT(0.0,0.0));
double ComputeAngleXAxis(PT pt1, PT pt2);
double ComputeAngleXAxis(PT pt2);
PT ComputeNormalVector(PT ptV);
void ComputeObstacleDensityApprox(structObstaclePos ObstaclesPosArray[],
                                  int nNumofObstacles, PT GoalPos, vector<structFeature> &CurrentFeaturesVector);
void Rotate(PT& ptP, double fAngle);
void ComputeObstaclePresenceandDistance(structObstaclePos ObstaclePos,
                                        double fRegStartAngle, double fRegEndAngle,
                                        double& fObsPres, double& fObsDist);
void SortbyDistance(structObstaclePos ObstacleArray[], int nNumofObstacles);
structFeature ComputeShortTermRelMotionFeature(structRobotPos RobotPos);
structFeature ComputeLongTermRelMotionFeature(structRobotPos RobotPos);
structFeature ComputeCaseTimeFeature(HISTORY *pCaseHistory);
CCBRCase* CreateCaseofType(CASE_TYPE nCaseType);
void TranslateandLimitReadings(structRobotPos& RobotPos, structObstaclePos ObstaclesPosArray[],
                               vector<structObstaclePos>& ObstaclesPosV, int& nNumofObstacles, PT& GoalPos);
void SortbyDistance(vector<structObstaclePos>& ObstaclesV);
void copyFC(double FCSourceVector[], double FCTargetVector[]);


#endif


/**********************************************************************
 * $Log: FuncProts.h,v $
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
