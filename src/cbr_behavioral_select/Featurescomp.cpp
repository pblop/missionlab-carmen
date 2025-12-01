/**********************************************************************
 **                                                                  **
 **                         Featurescomp.cpp                         **
 **                                                                  **
 ** Contains the computation of features functions                   **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Featurescomp.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include "Headers.h"


#ifdef CREATE_FEAT_DEBUG_FILE
extern FILE* fDebug1File;
#endif

//*********************************************************************

//---------------------------------------------------------------------
//The function computes the Distance to the goal feature and returns
//it
//Note: the Goal Position should be relative to the Robot Position
structFeature ComputeGoalDistFeature(PT GoalPos) {
  structFeature DistFeature;

  //compute the feature
  DistFeature.fDegofMembership = 1.0;
  DistFeature.fImportDegree = 1.0;
  DistFeature.fProb = 1.0;
  DistFeature.fValue = ComputeLength(GoalPos);
  strcpy(DistFeature.strName, GOALDISTFEATURE_NAME);
  DistFeature.nID = GOALDIST_FEATURE;

  return	DistFeature;
}
//---------------------------------------------------------------------



//---------------------------------------------------------------------
//This function computes the obstacle density discrete approximation
//as a function of angle. The circle around the robot is discretized
//into N non-uniform regions; For each region a feature is computed
//that consists of the region's density and the distance to the first
//obstacle in the region
//Note: the obstacle and goal positions should be relative to the
//robot position
void ComputeObstacleDensityApprox(structObstaclePos ObstaclesPosArray[],
                                  int nNumofObstacles, PT GoalPos, vector<structFeature> &CurrentFeaturesVector) {
  double fRegStart[NUMOFREGIONS];
  double fRegEnd[NUMOFREGIONS];
  double fRegDens[NUMOFREGIONS];
  double fRegDist[NUMOFREGIONS];
  double fTemp;
  structObstaclePos* TempObstacleArray = new structObstaclePos[nNumofObstacles];
  int i, nTemp;


  //compute all the regions
  //For now let's assume that the regions are uniformly distributed
  //TODO - apply non-uniform function
  //The width of each region
  fTemp = 2*PI/NUMOFREGIONS;
  for(i = 0; i < NUMOFREGIONS; i++) {
    fRegStart[i] = fTemp*i - fTemp/2;
    fRegEnd[i] = fRegStart[i] + fTemp;
    if(fRegStart[i] < 0)
      fRegStart[i] += 2*PI;
    fRegDens[i] = 0;
    fRegDist[i] = 0;
  }


  //rotate all the obstacles and the goal so that they all are relative to
  //the robot-goal line as X axis.
  //also adjust the radius of each obstacle by the safety margin
  double fRobotGoalAngle = ComputeAngleXAxis(GoalPos);
  for(i = 0; i < nNumofObstacles; i++) {
    TempObstacleArray[i] = ObstaclesPosArray[i];
    Rotate(TempObstacleArray[i].ptC, -fRobotGoalAngle);
    TempObstacleArray[i].fRadius += OBS_SAFETY_MARGIN;

  }
  Rotate(GoalPos, -fRobotGoalAngle);

  //sort the array of obstacles by their distance
  SortbyDistance(TempObstacleArray, nNumofObstacles);

  //Iterate through all the regions. For each obstacle compute its
  //degree of presence in the region and the distance to it.
  //Then iterate again through the obstacles computing
  //the most dense cluster in the region or the first(going from robot
  //outward) cluster with the density above a critical threshold.
  //This cluster's density is the density of the region.
  //The distance to the cluster is the region's distance
  double fRegLength = ComputeLength(GoalPos);
  double* fObsPres = new double [nNumofObstacles];
  double* fObsDist = new double [nNumofObstacles];
  for(int j = 0; j < NUMOFREGIONS; j++) {
#ifdef CREATE_FEAT_DEBUG_FILE
    fprintf(fDebug1File, "Computing %d region:\n", j);
#endif
    
    //compute the length of the region that we are interested in
    fRegLength = fRegLength*pow(REGLENGTHDECAY, (double)j);

#ifdef CREATE_FEAT_DEBUG_FILE
    fprintf(fDebug1File, "fRegLength=%f\n", fRegLength);
#endif
    
    //iterate through the obstacles
    for(i = 0; i < nNumofObstacles; i++) {
      //check if we are out of range of interest
      if(ComputeLength(TempObstacleArray[i].ptC) > fRegLength)
        break;

      //Compute the degree of obstacle's presence and
      //the distance to it within the given region
      ComputeObstaclePresenceandDistance(TempObstacleArray[i],
                                         fRegStart[j], fRegEnd[j], fObsPres[i], fObsDist[i]);
    }

    //get the number of obstacles that lie on the distance from the robot
    nTemp = i;

    //iterate through the obstacles in the region and
    //compute overall density
    fRegDens[j] = 0;
    fRegDist[j] = 0;
    for(i = 0; i < nTemp; i++) {
      double fLowBound = fObsDist[i] - TempObstacleArray[i].fRadius;
      double fHighBound = fObsDist[i] + TempObstacleArray[i].fRadius;
      double fDens = 0;
      double fDist = fObsDist[i];
      int nObsCount = 0;
      int k;

      //if the obstacle is not from this region then skip it
      if(IS_ZERO(fObsPres[i]))
        continue;

      //go down first
      for(k = i-1; k >= 0; k--) {

        //check whether the obstacle is within the bounds
        if(fObsDist[k] < fLowBound)
          break;

        //check that the obstacle is in this region
        if(IS_ZERO(fObsPres[k]))
          continue;

        //update the distance of the region
        fDist = fObsDist[k];

        //update the density for the region
        fDens += fObsPres[k];
        nObsCount++;
      }
      //go up
      for(k = i+1; k < nNumofObstacles; k++) {
        //check whether the obstacle is within the bounds
        if(fObsDist[k] > fHighBound)
          break;

        //check that the obstacle is in this region
        if(IS_ZERO(fObsPres[k]))
          continue;

        //update the density for the region
        fDens += fObsPres[k];
        nObsCount++;
      }
      //normalize the density(0-2)
      if(nObsCount > 0)
        fDens = fObsPres[i] + fDens/nObsCount;
      else
        fDens = fObsPres[i];

      //see if this is the most clustered so far
      if(fDens > fRegDens[j]) {
        fRegDens[j] = fDens;
        fRegDist[j] = fDist;
      }

      //check whether the density is above the critical
      //threshold - in which case we can stop computing it
      if(fRegDens[j] > REGDENSITYTHRESH)
        break;
    }//Num of obstacles in the region = nTemp
    //normalize the density
    fRegDens[j] = __min(1.0, fRegDens[j]);

    //store the feature into the vector of features
    structFeature TempFeature;
    char strTemp[200];

    //set the region's density feature
    TempFeature.fDegofMembership = 1.0;
    TempFeature.fImportDegree = 1.0;
    TempFeature.fProb = 1.0;
    TempFeature.fValue = fRegDens[j];
    sprintf(strTemp, "%s %d", REGDENSFEATURE_NAME, j);
    strcpy(TempFeature.strName, strTemp);
    TempFeature.nID = REG_FEATURE;
    CurrentFeaturesVector.push_back(TempFeature);

    //set the region's distance feature
    TempFeature.fDegofMembership = 1.0;
    TempFeature.fImportDegree = 1.0;
    TempFeature.fProb = 1.0;
    TempFeature.fValue = fRegDist[j];
    sprintf(strTemp, "%s %d", REGDISTFEATURE_NAME, j);
    strcpy(TempFeature.strName, strTemp);
    TempFeature.nID = REG_FEATURE;
    CurrentFeaturesVector.push_back(TempFeature);

  }//NUMREGIONS

  //cleanup
  delete [] TempObstacleArray ;
  delete [] fObsPres;
  delete [] fObsDist;
}
//---------------------------------------------------------------------

//----------------------------------------------------------------------
//Computes the Short Term Relative Motion and returns it in the
//feature structure
structFeature ComputeShortTermRelMotionFeature(structRobotPos RobotPos) {
  static PT LongWinRobotPos(RobotPos.ptC.fX-2,RobotPos.ptC.fY-2);
  static PT ShortWinRobotPos(RobotPos.ptC.fX,RobotPos.ptC.fY);
  double a;
  structFeature RetFeature;

  //recalculate the long and short window positions
  a = SHORTTERM_LONGWINDECAY;
  LongWinRobotPos.fX = a*LongWinRobotPos.fX +
                       (1-a)*RobotPos.ptC.fX;
  LongWinRobotPos.fY = a*LongWinRobotPos.fY +
                       (1-a)*RobotPos.ptC.fY;
  a = SHORTTERM_SHORTWINDECAY;
  ShortWinRobotPos.fX = a*ShortWinRobotPos.fX +
                        (1-a)*RobotPos.ptC.fX;
  ShortWinRobotPos.fY = a*ShortWinRobotPos.fY +
                        (1-a)*RobotPos.ptC.fY;

#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "In ShortTerm: LongX = %f, LongY = %f; ShortX = %f, ShortY = %f\n",
          LongWinRobotPos.fX, LongWinRobotPos.fY, ShortWinRobotPos.fX, ShortWinRobotPos.fY);
#endif


  //find the average velocity per cycle
  double fVel = ComputeLength(LongWinRobotPos, ShortWinRobotPos)/SHORTTERM_NORM;

  //normalize the velocity(we are not talking about the unit vector)
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "Robot Vel(Short) = %f\n", fVel);
#endif
  fVel = fVel/MAXVEL;
  fVel = __min(1.0, fVel);

  //set the feature structure
  RetFeature.fDegofMembership = 1.0;
  RetFeature.fImportDegree = 1.0;
  RetFeature.fProb = 1.0;
  RetFeature.fValue = fVel;
  strcpy(RetFeature.strName, SHORTTERMRELMOTFEATURE_NAME);
  RetFeature.nID = SHORTTERMRELMOT_FEATURE;

  return RetFeature;
}
//----------------------------------------------------------------------


//----------------------------------------------------------------------
//Computes the long Term Relative Motion and returns it in the
//feature structure
structFeature ComputeLongTermRelMotionFeature(structRobotPos RobotPos) {
  //TODO - possibly initialize with the Robot Start Position
  static PT LongWinRobotPos(RobotPos.ptC.fX-5,RobotPos.ptC.fY-5);
  static PT ShortWinRobotPos(RobotPos.ptC.fX,RobotPos.ptC.fY);
  double a;
  structFeature RetFeature;

#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "RobotPos(in Long Term Calc) = %f %f\n", RobotPos.ptC.fX,
          RobotPos.ptC.fY);
  fprintf(fDebug1File, "Old Long Term Pos: %f, %f\n", LongWinRobotPos.fX,
          LongWinRobotPos.fY);
#endif
  
  //recalculate the long and short window positions
  a = LONGTERM_LONGWINDECAY;

  LongWinRobotPos.fX = a*LongWinRobotPos.fX +
                       (1-a)*RobotPos.ptC.fX;
  LongWinRobotPos.fY = a*LongWinRobotPos.fY +
                       (1-a)*RobotPos.ptC.fY;
  a = LONGTERM_SHORTWINDECAY;
  ShortWinRobotPos.fX = a*ShortWinRobotPos.fX +
                        (1-a)*RobotPos.ptC.fX;
  ShortWinRobotPos.fY = a*ShortWinRobotPos.fY +
                        (1-a)*RobotPos.ptC.fY;

#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "In LongTerm: LongX = %f, LongY = %f; ShortX = %f, ShortY = %f\n",
          LongWinRobotPos.fX, LongWinRobotPos.fY, ShortWinRobotPos.fX, ShortWinRobotPos.fY);
#endif
  
  //find the average velocity per cycle
  double fVel = ComputeLength(LongWinRobotPos, ShortWinRobotPos)/LONGTERM_NORM;

  //normalize the velocity(we are not talking about the unit vector)
#ifdef CREATE_FEAT_DEBUG_FILE
  fprintf(fDebug1File, "Robot Vel(Long) = %f\n", fVel);
#endif
  fVel = fVel/MAXVEL;
  fVel = __min(1.0, fVel);

  //set the feature structure
  RetFeature.fDegofMembership = 1.0;
  RetFeature.fImportDegree = 1.0;
  RetFeature.fProb = 1.0;
  RetFeature.fValue = fVel;
  strcpy(RetFeature.strName, LONGTERMRELMOTFEATURE_NAME);
  RetFeature.nID = LONGTERMRELMOT_FEATURE;

  return RetFeature;
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//Computes and returns the feature describing how long the
//current case was applied
structFeature ComputeCaseTimeFeature(HISTORY *pCaseHistory) {
  structFeature RetFeature;

  //increment Time (in TIME_UNITS cycle units)
  pCaseHistory->nCount++;
  if(pCaseHistory->nCount >= TIME_UNITS) {
    pCaseHistory->nCount = 0;
    pCaseHistory->nTime++;
  }

  //set the retunr feature structure
  RetFeature.fDegofMembership = 1.0;
  RetFeature.fImportDegree = 1.0;
  RetFeature.fProb = 1.0;
  RetFeature.fValue = pCaseHistory->nTime;
  strcpy(RetFeature.strName, CASETIMEFEATURE_NAME);
  RetFeature.nID = CASETIME_FEATURE;

  return RetFeature;
}
//----------------------------------------------------------------------

//----------------------------------------------------------------------
//Computes the degree of presence of the obstacle in the region
//and the distance to the obstacle within the region
void ComputeObstaclePresenceandDistance(structObstaclePos ObstaclePos,
                                        double fRegStartAngle, double fRegEndAngle,
                                        double& fObsPres, double& fObsDist) {
  double fObsCentVectLen = ComputeLength(ObstaclePos.ptC);

  //first, check if the robot is within the radius of the obstacle
  if(fObsCentVectLen <= ObstaclePos.fRadius) {
    //the degree of presence is at maximum
    fObsPres = 1.0;
    fObsDist = 0.0;
    return;
  }

  //calculate the angle between the ObsCenter Vector and
  //the ObsTangent Vector
  double fDeltaAngle = asin(ObstaclePos.fRadius/fObsCentVectLen);

  //compute the angle between ObsCenter Vector and X Axis
  double fCenterAngle = ComputeAngleXAxis(ObstaclePos.ptC);

  //compute the angles that surround the obstacle
  double fEndAngle = fCenterAngle + fDeltaAngle;
  if(fEndAngle > 2*PI)
    fEndAngle -= 2*PI;
  double fStartAngle = fCenterAngle - fDeltaAngle;
  if(fStartAngle < 0)
    fStartAngle += 2*PI;

  //compute the overlap between the obstacle angle interval
  //and the region's angular interval
  //Note: fEndAngle and fStartAngle are within 0 to 2*PI
  //fRegEndAngle and fRegStartAngle are within 0 to 2*PI also
  double fIntEndAngle, fIntStartAngle;
  double fIntLength, fRegLength;
  if(fRegStartAngle >= fRegEndAngle) {
    //Region interval wraps around

    if(fStartAngle >= fEndAngle) {
      //Obstacle interval wraps around
      fIntStartAngle = __max(fStartAngle, fRegStartAngle);
      fIntEndAngle = __min(fEndAngle, fRegEndAngle);
      fIntLength = 2*PI-fIntStartAngle+fIntEndAngle;
      fRegLength = 2*PI-fRegStartAngle+fRegEndAngle;
      fObsPres = fIntLength/fRegLength;
    } else {
      //Obstacle interval is normal
      if(fStartAngle > fRegEndAngle) {
        //possibly overlap with upper part of Reg Int.
        fIntStartAngle = __max(fStartAngle, fRegStartAngle);
        fIntEndAngle = fEndAngle;
      } else {
        //possibly overlap with lower part of Reg. Interval
        fIntStartAngle = fStartAngle;
        fIntEndAngle = __min(fEndAngle, fRegEndAngle);
      }
      fIntLength = fIntEndAngle-fIntStartAngle;
      if(fIntLength < 0)
        fIntLength = 0;
      fRegLength = 2*PI-fRegStartAngle+fRegEndAngle;
      fObsPres = fIntLength/fRegLength;
    }
  } else {
    //Region interval is normal

    if(fStartAngle >= fEndAngle) {
      //Obstacle interval wraps around
      if(fRegStartAngle > fEndAngle) {
        //possibly overlap with upper part of Obs. Int.
        fIntStartAngle = __max(fStartAngle, fRegStartAngle);
        fIntEndAngle = fRegEndAngle;
      } else {
        //possibly overlap with lower part of Obs. Interval
        fIntStartAngle = fRegStartAngle;
        fIntEndAngle = __min(fEndAngle, fRegEndAngle);
      }
      fIntLength = fIntEndAngle-fIntStartAngle;
      if(fIntLength < 0)
        fIntLength = 0;
      fRegLength = fRegEndAngle-fRegStartAngle;
      fObsPres = fIntLength/fRegLength;
    } else {
      //Obstacle interval is normal
      fIntStartAngle = __max(fStartAngle, fRegStartAngle);
      fIntEndAngle = __min(fEndAngle, fRegEndAngle);
      fIntLength = fIntEndAngle-fIntStartAngle;
      if(fIntLength < 0)
        fIntLength = 0;
      fRegLength = fRegEndAngle-fRegStartAngle;
      fObsPres = fIntLength/fRegLength;
    }
  }

  //calculate the distance to the object
  //(we approximate for the sake of simplicity)
  fObsDist = fObsCentVectLen - ObstaclePos.fRadius;

}
//----------------------------------------------------------------------



/**********************************************************************
 * $Log: Featurescomp.cpp,v $
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
 * Revision 1.2  2003/04/06 15:08:25  endo
 * gcc 3.1.1
 *
 * Revision 1.1  2003/04/05 23:36:22  kaess
 * Initial revision
 *
 **********************************************************************/
