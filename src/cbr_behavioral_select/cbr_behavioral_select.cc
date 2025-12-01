/**********************************************************************
 **                                                                  **
 **                     cbr_behavioral_select.cc                     **
 **                                                                  **
 ** Contains all the CBR functions that are used between             **
 ** Mlab and CBR interface functions.                                **
 **                                                                  **
 ** By: Max Likhachev, Michael Kaess                                 **
 **                                                                  **
 ** Copyright 2003, Georgia Tech Research Corporation                **
 ** Atlanta, Georgia  30332-0415                                     **
 ** ALL RIGHTS RESERVED, See file COPYRIGHT for details.             **
 **********************************************************************/

/* $Id: cbr_behavioral_select.cc,v 1.1.1.1 2008/07/14 16:44:14 endo Exp $ */

//#include <vector.h>
#include <stdio.h>
#include <vector>

#include "gt_simulation.h"
#include "sensor_blackboard.h"

#include "Interface.h"
#include "cbr_behavioral_select.h"

using std::vector;

/***********************************************************************/


/*define here all the tasks that CBR can work for*/
#define CBRTASKARRAY_SIZE 2
char CBRTaskArray[CBRTASKARRAY_SIZE][100] =
  { "GoTo_CBR",
    "GoToOutdoor_CBR"
  };


/**********************************************************************/
/*The function returns the current state name(unique ID) in the sState
  argument, the task name in the sTask argument and 1 as a return value 
  if it determines that the current task is CBR applicable, 
  otherwise the function returns 0 as the return value  
*/
int GetCurrentStateandTask(char* sState, char* sTask) {
  char sFSAList[1000], sFSA[100], *sFSAListptr;
  char sTaskKey[100], sStateKey[100];
  int bTaskFound = 0;

  //Get the list of the current FSAs
  strcpy(sFSAList, get_state(FSALIST_KEY));

  //set the iterator through the FSAs
  sFSAListptr = sFSAList;

  //iterate through each FSA's task and find the one that CBR is used for
  while(!bTaskFound && strlen(sFSAListptr) != 0) {
    //get the first FSA Name
    sscanf(sFSAListptr, "%[^ ]", sFSA);

    //Form the task key
    strcpy(sTaskKey, sFSA);
    strcat(sTaskKey, "_task");

    //Get the current task for this FSA
    strcpy(sTask, get_state(sTaskKey));

#ifdef VERBOSE
    printf("Task Name=%s\n", sTask);
#endif
    
    //see if this task is one of those that can use CBR
    for(int i = 0; i < CBRTASKARRAY_SIZE; i++) {
      if(!strcmp(sTask, CBRTaskArray[i])) {
        //The task is found
        bTaskFound = 1;

        //Form the state key
        strcpy(sStateKey, sFSA);
        strcat(sStateKey, "_state");

        //Get the current task for this FSA
        strcpy(sState, get_state(sStateKey));

      }

    }

    //move the pointer to the FSAList to point to the next FSA
    sFSAListptr = strchr(sFSAListptr, ' ');
    sFSAListptr++;
  }

  return bTaskFound;


}


/***********************************************************************/




/**********************************************************************/
/*This function prints out the readings that are passed in.
It returns true if it succeeded to print, false otherwise  
*/
bool PrintReadings() {
#ifdef VERBOSE
  obs_reading Reading;

  //first print out the robot posistion and heading
  printf("The robot data:\n");
  printf("X = %f, Y = %f, Heading = %f\n",
         sensor_blackboard.x, sensor_blackboard.y, sensor_blackboard.theta);

  printf("The obstacle data:\n");
  printf("There are %d obstacles\n",
         sensor_blackboard.sensed_objects.size());
  if(!sensor_blackboard.sensed_objects.empty()) {
    for(unsigned int i = 0; i < sensor_blackboard.sensed_objects.size(); i++) {
      Reading = sensor_blackboard.sensed_objects[i];
      printf("Obstacle %d\n", i);
      printf("C.x = %f, C.y = %f, r = %f\n",
             Reading.center.x, Reading.center.y, Reading.r);
    }

  }
#endif
  
  return true;
}


/***********************************************************************/

/**********************************************************************/
/**Get Sensor readings**/
//read in the environment
bool GetSensorReadings(structRobotPos& RobotPos,
                       vector<structObstaclePos>& ObstaclesPosVector, int &nNumofObstacles) {
  obs_reading Reading;
  structObstaclePos ObsPos;

  RobotPos.ptC.fX = sensor_blackboard.x;
  RobotPos.ptC.fY = sensor_blackboard.y;
  RobotPos.fHeading = sensor_blackboard.theta;

  nNumofObstacles = sensor_blackboard.sensed_objects.size();


  for(unsigned int i = 0; i < sensor_blackboard.sensed_objects.size(); i++) {
    Reading = sensor_blackboard.sensed_objects[i];
    ObsPos.ptC.fX = Reading.center.x;
    ObsPos.ptC.fY = Reading.center.y;
    ObsPos.fRadius = Reading.r;
    ObstaclesPosVector.push_back(ObsPos);
  }


#if 0
  //debug mk
  static bool bFirstTime = true;
  static FILE* pfTimeOut = NULL;
  if ( bFirstTime )
    {
      pfTimeOut = fopen( "vector_size.log", "w" );
      bFirstTime = false;
    }
  if ( pfTimeOut != NULL )
    {
      fprintf( pfTimeOut, "blackboard size: %d\n", sensor_blackboard.sensed_objects.size());
      fflush( pfTimeOut );
    }
  //debug mk
#endif

  
  return true;
}
/**********************************************************************/

/**********************************************************************/
void UpdateMlab(char* strUpdateString) {
  // todo  exec_report_current_cbr_state(strUpdateString);
}
/**********************************************************************/



/**********************************************************************
 * $Log: cbr_behavioral_select.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:14  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:45  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:53  endo
 * MissionLab 7.0
 *
 * Revision 1.2  2006/05/14 07:50:26  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.1.1.1  2005/02/06 23:00:25  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
 * Revision 1.1  2003/04/05 23:51:36  kaess
 * Initial revision
 *
 **********************************************************************/
