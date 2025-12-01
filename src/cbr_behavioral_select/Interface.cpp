/**********************************************************************
 **                                                                  **
 **                         Interface.cpp                            **
 **                                                                  **
 ** Contains all the CBR algorithm interface function. These are     **
 ** the only functions that should be visible to the outside world.  **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: Interface.cpp,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

#include "Headers.h"
/***********************************************************************/


/***********global variables*******************************/
//CBR Controller
CCBRController *pCBR_Controller = NULL;
/**********************************************************/


#ifdef WINCOMPILER
bool GetSensorReadings(structRobotPos& RobotPos,
                       vector<structObstaclePos>& ObstaclesPosV, int &nNumofObstacles);
#endif


/**********************************************************************/
/*Retrieves, adapts and returns a suggestion for a case to use. The
**case is returned in the database. 
**The function also returns the error code.
**/
bool CBRSelectandApplyCase(PT GoalPos) {
  structRobotPos RobotPos;
  structObstaclePos ObstaclesPosArray[MAXNUMOBSTACLES];
  vector<structObstaclePos> ObstaclesPosV;
  int nNumofObstacles;
  
#if REAL_ROBOT == 1 //mk021015
  // for real robot: need to execute CBR only every second
  static bool bFirstTime = true;
  static double dLastTime;
  double dCurrentTime;
  timeval suCurrentTime;

  gettimeofday( &suCurrentTime, NULL );
  dCurrentTime = ( (double)suCurrentTime.tv_sec + (double)suCurrentTime.tv_usec * 1e-6 );
  
  if (bFirstTime) {
    dLastTime = dCurrentTime - 1.0; // make sure CBR is executed
    bFirstTime = false;
  }
  
  if ((dCurrentTime - dLastTime) >= 2.0) {
    // this should never happen...
    printf("mk:PROBLEM: MISSED ONE CBR CYCLE, WILL TRY TO DO EXTRA ONE\n");
  }

  if ((dCurrentTime - dLastTime) >= 1.0) {
    printf("mk:CBR needs to be executed, time %f\n", dCurrentTime);
    dLastTime += 1.0;

    // now execute CBR
#endif

#ifdef VERBOSE
  printf("Entering CBR Select and Apply Case function\n");
#endif

  //===check the state of the CBR Controller and initialize it if needed===
  if(pCBR_Controller == NULL) {
    //allocate
    pCBR_Controller = new CCBRController;

    //initialize
    if(pCBR_Controller->Initialize() != true) {
      fprintf(stderr, "Error during the initializion of CBRController\n");
      exit(1);
    } else {
#ifdef VERBOSE
      printf("Done Initializing the CBR Controller\n");
#endif
    }
  }
  //=====================================================================

  //=====read in the environment features================================
  if(!GetSensorReadings(RobotPos, ObstaclesPosV, nNumofObstacles)) {
    fprintf(stderr,"ERROR: Failed to parse the readings\n");
    exit(1);
  }
      
  //at this point all the obstacles and goal are given relative to the
  //robot position including its heading(that is all of them are rotated by its heading
  //so that the heading defines X axis and robot position defines the origin of the coord.
  //system))
  //convert some of the readings into the required format
  TranslateandLimitReadings(RobotPos, ObstaclesPosArray, ObstaclesPosV, nNumofObstacles, GoalPos);
  //=====================================================================

  //=====read in the current robot plan state and task ==================

  //Also change the state of the CBR Controller if it is a new plan state - TODO

  //=====================================================================

  //=====If a transition happened - do the reset of the CBR controller=====
  //==and any appropriate learning of the last case(or queue the learning)=
  if(pCBR_Controller->IsNewTask() == true) {
    if(pCBR_Controller->NewTaskReset() == false) {
      fprintf(stderr, "ERROR: Failed to Reset for the new task\n");
      exit(1);
    } else {
#ifdef VERBOSE
      printf("CBR Reset for the new task is done\n");
#endif
    }
  }
  //=====================================================================

  
  //====="Identify features" step========================================
  if(pCBR_Controller->IdentifyFeatures(RobotPos, ObstaclesPosArray, nNumofObstacles, GoalPos)
      == false) {
    fprintf(stderr, "ERROR: Failed to identify features\n");
    exit(1);
  }
  //=====================================================================


  //====="Select the case" step===============================
  if(pCBR_Controller->SelectandAdaptCase() == false) {
    fprintf(stderr, "ERROR: Failed to retrieve case\n");
    exit(1);
  }
  //=====================================================================

  //====="Apply the adapted case" step===============================
  if(pCBR_Controller->ApplyCurrentCase() == false) {
    fprintf(stderr, "ERROR: Failed to apply case\n");
    exit(1);
  }
  //=====================================================================

#if REAL_ROBOT == 1 //mk021015

    pCBR_Controller->SaveLibrary();

  } else {

#ifdef VERBOSE
    printf("mk:Nothing to do for CBR\n");
#endif
    
  }
  
#endif
  
  return true;
}

/***********************************************************************/




/**********************************************************************/
/*Destroys the CBR Unit
**/
bool CBRDestroy() {

  //save the CBR library
  pCBR_Controller->SaveLibrary();

  //delete the controller
  delete pCBR_Controller;

  return true;
}
/**********************************************************************/


/**********************************************************************
 * $Log: Interface.cpp,v $
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
