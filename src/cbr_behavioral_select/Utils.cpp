/**********************************************************************
 **                                                                  **
 **                           Utils.h                                **
 **                                                                  **
 ** Contains all the CBR algorithm utility functions.                **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Utils.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */


#define MEASURE_TIME


#include "Headers.h"

#include <algorithm>

#ifdef CREATE_DEBUG_FILES
extern FILE* fDebug1File;
#endif

//=========================Some geometric operations======================
//rotate the point ptP by the fAngle in radians counterclockwise
void Rotate(PT& ptP, double fAngle) {
  PT ptTemp = ptP;

  ptP.fX = ptTemp.fX*cos(fAngle) - ptTemp.fY*sin(fAngle);
  ptP.fY = ptTemp.fX*sin(fAngle) + ptTemp.fY*cos(fAngle);
}

//Compute the length of the vector between start and End points
//End point by default is set to 0,0, in which case
//the return value is the length of ptStart vector.
double ComputeLength(PT ptStart, PT ptEnd /*={0,0}*/) {
  return sqrt(pow((ptStart.fX-ptEnd.fX), 2.0) +
              pow((ptStart.fY-ptEnd.fY), 2.0));
}

//Computes the Angle between Vector{pt1, pt2} and X axis
//counterclockwise (starting from X axis)
double ComputeAngleXAxis(PT pt1, PT pt2) {
  double fAngle;
  //make the input a one-point vector
  PT pt3 = PT(pt2.fX-pt1.fX, pt2.fY-pt1.fY);

  //normalize it
  pt3 = ComputeNormalVector(pt3);

  //compute the angle itself
  fAngle = acos(pt3.fX);
  //adjust for the 3 and 4th quadrant angles
  if(pt3.fY < 0)
    fAngle = 2*PI - fAngle;

  return fAngle;
}


//Computes the Angle between one-point Vector{pt2} and X axis
//counterclockwise (starting from X axis)
double ComputeAngleXAxis(PT pt2) {
  return ComputeAngleXAxis(PT(0,0), pt2);
}


//Returns the normalized vector
PT ComputeNormalVector(PT ptV) {
  double fVLength = ComputeLength(ptV);

  return PT(ptV.fX/fVLength, ptV.fY/fVLength);
}
//===============end of geometric operations===============================







//=================Translate the readings into the right format===========
//The obstacle position are relative to the heading and position of the robot.
void TranslateandLimitReadings(structRobotPos& RobotPos, structObstaclePos ObstaclesPosArray[],
                               vector<structObstaclePos>& ObstaclesPosV, int& nNumofObstacles, PT& GoalPos) {
  vector<structObstaclePos> TempObstaclesPosV;
  unsigned int i;

#ifdef MEASURE_TIME
    //debug mk
    static bool bFirstTime = true;
    static timeval suFirstTime;
    static timeval suLastTime;
    static FILE* pfTimeOut = NULL;
    double dDiff;
    timeval suCurrentTime;
    gettimeofday( &suCurrentTime, NULL );
    if ( bFirstTime )
    {
        pfTimeOut = fopen( "time_func.log", "w" );
        suFirstTime = suCurrentTime;
        bFirstTime = false;
    }
    double dStart = ( ( (double)suCurrentTime.tv_sec + (double)suCurrentTime.tv_usec * 1e-6 ) -
                    ( (double)suFirstTime.tv_sec   + (double)suFirstTime.tv_usec    * 1e-6 ) );
    if ( pfTimeOut != NULL )
    {
        fprintf( pfTimeOut, "timeStart = %02.06f ", dStart );
        fflush( pfTimeOut );
    }
    suLastTime = suCurrentTime;
#endif

#ifdef VERBOSE
  printf("Entering Translate readings function\n");
  printf("The overall number of obstacles detected=%d\n", ObstaclesPosV.size());
#endif

  //clip the obstacles that are outside of the  range
  for(i = 0; i < ObstaclesPosV.size(); i++) {
    if(ComputeLength(ObstaclesPosV[i].ptC) <= SENSOR_THRESH) {
      TempObstaclesPosV.push_back(ObstaclesPosV[i]);
    }
  }

  //sort them now by distance if more than limit and copy the most nearest ones
  int nObsNum = TempObstaclesPosV.size();

#ifdef MEASURE_TIME
    //debug mk
    gettimeofday( &suCurrentTime, NULL );
    dDiff = ( ( (double)suCurrentTime.tv_sec + (double)suCurrentTime.tv_usec * 1e-6 ) -
                    ( (double)suLastTime.tv_sec   + (double)suLastTime.tv_usec    * 1e-6 ) );
    if ( pfTimeOut != NULL )
    {
        fprintf( pfTimeOut, "time3 = %02.06f nObsNum1=%i ", dDiff, nObsNum );
    }
#endif

  if(nObsNum > MAXNUMOBSTACLES) {
    SortbyDistance(TempObstaclesPosV);
    nObsNum  = MAXNUMOBSTACLES;
  }

#ifdef MEASURE_TIME
    //debug mk
    gettimeofday( &suCurrentTime, NULL );
    dDiff = ( ( (double)suCurrentTime.tv_sec + (double)suCurrentTime.tv_usec * 1e-6 ) -
                    ( (double)suLastTime.tv_sec   + (double)suLastTime.tv_usec    * 1e-6 ) );
    if ( pfTimeOut != NULL )
    {
        fprintf( pfTimeOut, "time4 = %02.06f nObsNum2=%i\n", dDiff, nObsNum );
        fflush( pfTimeOut );
    }
#endif
    
  //stright copy
  for(i = 0; i < (unsigned int)nObsNum; i++) {
    ObstaclesPosArray[i] = TempObstaclesPosV[i];
    //if real_robots, then make the obstacle radius equal to the robot's one
#if REAL_ROBOT==1
    ObstaclesPosArray[i].fRadius = ROBOT_RADIUS;
#endif

  }

  //set the number of obstacles
  nNumofObstacles = nObsNum;

  //translate the heading into radians
  RobotPos.fHeading = 2.0*PI*RobotPos.fHeading/360.0;

#ifdef VERBOSE
  printf("Robot's heading = %f\n", RobotPos.fHeading);
  printf("Goal Pos: %f %f \n", GoalPos.fX, GoalPos.fY);
  printf("Number of Obstacles = %d\n", nNumofObstacles);
  if(nNumofObstacles > 0)
    printf("First Obstacle: %f %f %f\n", ObstaclesPosArray[0].ptC.fX,
           ObstaclesPosArray[0].ptC.fY, ObstaclesPosArray[0].fRadius);
#endif
  
#ifdef CREATE_DEBUG_FILES
  fprintf(fDebug1File, "Robot's heading = %f\n", RobotPos.fHeading);
  fprintf(fDebug1File, "Goal Pos: %f %f \n", GoalPos.fX, GoalPos.fY);
  fprintf(fDebug1File, "Number of Obstacles = %d\n", nNumofObstacles);

#if REAL_ROBOT==1
  for(i = 0; i < nNumofObstacles; i++) {
    fprintf(fDebug1File, "Obs %d: %f %f %f\n",
            i, ObstaclesPosArray[i].ptC.fX,
            ObstaclesPosArray[i].ptC.fY,
            ObstaclesPosArray[i].fRadius);
  }
#endif
#endif



}


//=================Application of the output parameters===========
void Apply(char* strID, char* strValue) {
#ifndef WINCOMPILER
#ifdef VERBOSE
  printf("put_state: %s, %s\n", strID, strValue);
#endif
#endif

  put_state(strID, strValue);
}


//===============================================================================
//Float to ascii conversion
char* Float2Ascii(double fVal, int nPrec, char* strVal) {
  long int nVal, nTemp;
  char strTemp[100];
  int i, nIndex;
  bool bSign = false;

  if(fVal < 0 ) {
    bSign = true;
    fVal = -fVal;
  }

  //extract and write the integer part
  nVal = (int)fVal;
  for(i = 0, nTemp = nVal; i < 100; i++) {
    strTemp[i] = nTemp%10 + 48;
    nTemp = nTemp/10;
    if(nTemp == 0)
      break;
  }

  //insert the negative sign if required
  if(bSign) {
    nTemp = i+1;
    strVal[0] = '-';
  } else
    nTemp = i;

  while(i >= 0) {
    strVal[nTemp-i] = strTemp[i];
    i--;
  }
  nIndex = nTemp+1;

  //write point
  strVal[nIndex++] = '.';

  //extract and write floating part
  if(nPrec <= 0)
    nPrec = 1;
  fVal = fVal - nVal;
  nVal = (long int)(fVal*pow((double)10,(double)nPrec));
  for(i = 0, nTemp = nVal; i < nPrec; i++) {
    strTemp[i] = nTemp%10 + 48;
    nTemp = nTemp/10;
  }
  nTemp = --i;
  while(i >= 0) {
    strVal[nIndex + nTemp - i] = strTemp[i];
    i--;
  }
  nIndex += nTemp+1;
  strVal[nIndex] = '\0';

  return strVal;

}
//===============================================================================







// need to speed it up
// todo mk



#if 0


// old, slow versions

//----------------------------------------------------------------------
//This function sorts the array by the distance to the obstacles.
//The first is with the smallest distance
void SortbyDistance(structObstaclePos ObstacleArray[], int nNumofObstacles) {
  structObstaclePos TempObs;

  for(int i = nNumofObstacles-1; i > 0; i--) {
    for(int j = 0; j < i; j++) {
      if(ComputeLength(ObstacleArray[j].ptC) >
          ComputeLength(ObstacleArray[j+1].ptC)) {
        TempObs = ObstacleArray[j];
        ObstacleArray[j] = ObstacleArray[j+1];
        ObstacleArray[j+1] = TempObs;
      }
    }
  }
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//This function sorts the array by the distance to the obstacles.
//The first is with the smallest distance
void SortbyDistance(vector<structObstaclePos>& ObstaclesV) {
  structObstaclePos TempObs;

  for(int i = ObstaclesV.size()-1; i > 0; i--) {
    for(int j = 0; j < i; j++) {
      if(ComputeLength(ObstaclesV[j].ptC) >
          ComputeLength(ObstaclesV[j+1].ptC)) {
        TempObs = ObstaclesV[j];
        ObstaclesV[j] = ObstaclesV[j+1];
        ObstaclesV[j+1] = TempObs;
      }
    }
  }
}
//----------------------------------------------------------------------

#else

// fast version, mk


//----------------------------------------------------------------------
//This function sorts the array by the distance to the obstacles.
//The first is with the smallest distance
void SortbyDistance(structObstaclePos ObstacleArray[], int nNumofObstacles) {

  if (nNumofObstacles > 1) {
    for(int i=0; i<nNumofObstacles; i++) {
      ObstacleArray[i].len = ComputeLength(ObstacleArray[i].ptC);
    }

    // O(n log(n)) sorting
    sort(&ObstacleArray[0], &ObstacleArray[nNumofObstacles-1], lt_structObstaclePos());
  }
  
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//This function sorts the array by the distance to the obstacles.
//The first is with the smallest distance
void SortbyDistance(vector<structObstaclePos>& ObstaclesV) {

  int size = ObstaclesV.size();
  
  if (size > 1) {
    for(int i=0; i<size; i++) {
      ObstaclesV[i].len = ComputeLength(ObstaclesV[i].ptC);
    }

    // O(n log(n)) sorting
    sort(ObstaclesV.begin(), ObstaclesV.end(), lt_structObstaclePos());
  }
  
}
//----------------------------------------------------------------------

#endif






//----------------------------------------------------------------------
//This function copies from FCSourceVector into FCTargetVector
void copyFC(double FCSourceVector[], double FCTargetVector[]) {
  int i = 0;

  for(i = 0; i < NUMOFREGIONS; i++) {
    FCTargetVector[i] = FCSourceVector[i];
  }


}
//----------------------------------------------------------------------



/**********************************************************************
 * $Log: Utils.cpp,v $
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
