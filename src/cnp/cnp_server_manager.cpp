/**********************************************************************
 **                                                                  **
 **                      cnp_server_manager.cpp                      **
 **                                                                  **
 **  Written by:  Alan Richard Wagner                                **
 **                                                                  **
 **  Copyright 2005 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cnp_server_manager.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "cnp_server_manager.h"
#include "AbstractCNPTask.h"
#include "CNPTrackTask.h"
#include "CNPInterceptTask.h"
#include "CNPInspectTask.h"
#include "CNPObserveTask.h"
#include "CNPEODTask.h"
#include "CNPMADTask.h"
#include "CNPSARTask.h"
#include "CNPEOIRTask.h"
#include "CNPEOESMTask.h"
#include "CNPEOIRAndMADTask.h"
#include "CNPSARAndMADTask.h"
#include "CNPRobotConstraint.h" // ENDO - gcc 3.4

extern void CNPPrintf(const char *format, ...);

CNP_Server_Manager::CNP_Server_Manager(void) {}

AbstractCNPTask *CNP_Server_Manager::instantiateCNPTask_(string cnpTaskName)
{
    int i, taskType = -1;
    AbstractCNPTask* cnpTask = NULL;

    for (i = 0; i < NUM_CNP_TASK_TYPES; i++)
    {
        if (cnpTaskName == CNP_TASK_NAMES[i])
        {
            taskType = i;
            break;
        }
    }
    
    if (taskType < 0)
    {
        CNPPrintf("CNP_Server_Manager::instantiateCNPTask_().\n Unknown CNP Task.\n");
        return NULL;
    }
    
    switch (taskType) {

    case CNP_TASK_TYPE_TRACK:
        cnpTask = new CNPTrackTask();
        break;

    case CNP_TASK_TYPE_INTERCEPT:
        cnpTask = new CNPInterceptTask();
        break;

    case CNP_TASK_TYPE_INSPECT:
        cnpTask = new CNPInspectTask();
        break;

    case CNP_TASK_TYPE_OBSERVE:
        cnpTask = new CNPObserveTask();
        break;

    case CNP_TASK_TYPE_EOD:
        cnpTask = new CNPEODTask();
        break;

    case CNP_TASK_TYPE_MAD:
        cnpTask = new CNPMADTask();
        break;

    case CNP_TASK_TYPE_SAR:
        cnpTask = new CNPSARTask();
        break;

    case CNP_TASK_TYPE_EOIR:
        cnpTask = new CNPEOIRTask();
        break;

    case CNP_TASK_TYPE_EOESM:
        cnpTask = new CNPEOESMTask();
        break;

    case CNP_TASK_TYPE_EOIR_AND_MAD:
        cnpTask = new CNPEOIRAndMADTask();
        break;

    case CNP_TASK_TYPE_SAR_AND_MAD:
        cnpTask = new CNPSARAndMADTask();
        break;
    }

    return cnpTask;
}

CNP_OutputBundle CNP_Server_Manager::PremissionCNPBidding(CNP_InputBundle input) {

    AbstractCNPTask* newTask = NULL; 
    CNP_OutputBundle mission; 
    list<AbstractCNPTask*> currentTasks;
    const bool DEBUG = true;

    list<CNP_TaskInfo>::iterator          taskInfoIter; 
    list<AbstractCNPTask*>::iterator      taskIter;

    mission.status = CNP_OUTPUT_NORMAL;

    //Gets a task pointer for each task on the list...
    for(taskInfoIter=input.ListOfCNPTasks.begin();taskInfoIter!=input.ListOfCNPTasks.end();taskInfoIter++)
    {
        newTask = instantiateCNPTask_(taskInfoIter->name);

        /*
        if(strcmp((*taskInfoIter).name.c_str(),"TrackTask") == 0) {
            newTask = new CNPTrackTask();
        }
        else if(strcmp((*taskInfoIter).name.c_str(),"InterceptTask") == 0) {
            newTask = new CNPInterceptTask();
        }	
        else if(strcmp((*taskInfoIter).name.c_str(),"InspectTask") == 0) {
            newTask = new CNPInspectTask();
        }
        else if(strcmp((*taskInfoIter).name.c_str(),"ObserveTask") == 0) {
            newTask = new CNPObserveTask();
        }
        else if(strcmp((*taskInfoIter).name.c_str(),"EODTask") == 0) {
            newTask = new CNPEODTask();
        }
        else {
            CNPPrintf( "Could not locate task type!" );
            continue;
        }
        */

        if (newTask == NULL)
        {
            CNPPrintf("CNP_Server_Manager::PremissionCNPBidding().\n CNP Task could not be instantiated.\n");
            continue;
        }

        newTask->SetTaskID((*taskInfoIter).id);
        //check to see if this task has any constraints associated with it...
        list<CNP_TaskConstraint>::iterator              taskCnstIter;

        for(taskCnstIter=input.ListOfCNPTaskConstraints.begin();taskCnstIter!=input.ListOfCNPTaskConstraints.end();taskCnstIter++) {
            //check if the task name is the same as the task the constraint is tied to
            if(strcmp(newTask->GetTaskName().c_str(),(*taskCnstIter).taskInfo.name.c_str() ) == 0) {
                newTask->AddTaskConstraint( (*taskCnstIter) );
            }
        }
        currentTasks.push_back( newTask );
    }

    //Build a list of robot constraint objects
    list<CNP_RobotConstraint>::iterator      rbtIter;
    list<CNPRobotConstraint*>   currentRobots;
 
    input.ListOfCNPRobotConstraints.sort();
  
    CNPRobotConstraint* aRobot = NULL;
    for(rbtIter=input.ListOfCNPRobotConstraints.begin();rbtIter!=input.ListOfCNPRobotConstraints.end();rbtIter++) {
	
        if( aRobot == NULL ) {
            aRobot = new CNPRobotConstraint((*rbtIter).iRobotID );
            currentRobots.push_back( aRobot );
        }
        else if( (*rbtIter).iRobotID != aRobot->GetRobotID() ) {  
            aRobot = new CNPRobotConstraint((*rbtIter).iRobotID );
            currentRobots.push_back( aRobot );
        }
        aRobot->AddConstraint((*rbtIter) );
    }
 
    //At this point we have a list of task objects with each task constraint added to the object and
    //we also have a list of robot objects with constraints added to it. 
    //We must now iterate through all of the tasks and robots and collect bids for each task 
    //Assigning the task to the highest bidder
	if(currentTasks.size() < currentRobots.size()) {
        mission.status = CNP_OUTPUT_EXCESSIVE_ROBOT;
	}
	
    for(taskIter=currentTasks.begin();taskIter!=currentTasks.end();taskIter++) {
	  
        double dLargestBid = 0;
        int iRobotID = -1; 
        list<CNPRobotConstraint*>::iterator      curRbtIter;
        list<CNPRobotConstraint*>::iterator      selectedRbtIter;
        for(curRbtIter=currentRobots.begin();curRbtIter!=currentRobots.end();curRbtIter++) {

            double dCurrentBid = (*curRbtIter)->BidOnTask( (*taskIter) );

            //have each robot bid on a task
            if (DEBUG) {
                CNPPrintf( "TASKNAME: %s\n",(*taskIter)->GetTaskName().c_str());
                CNPPrintf( "BID: %f\n",dCurrentBid);
            }

            if( dCurrentBid > dLargestBid ) {
                iRobotID = (*curRbtIter)->GetRobotID();
                selectedRbtIter = curRbtIter;
            }
        } 
        if(iRobotID != -1) {
            mission.robotTaskMapping.insert( CNPRobotTaskMapping::value_type(iRobotID, (*taskIter)->GetTaskID()) );
            currentRobots.erase( selectedRbtIter ); 
        }
		else {
		    if(mission.status == CNP_OUTPUT_EXCESSIVE_ROBOT)
                mission.status = CNP_OUTPUT_EXCESSIVE_ROBOT_AND_INCOMPLETE_MAPPING;
			else
                mission.status = CNP_OUTPUT_INCOMPLETE_MAPPING;
		}
    }
    //COULD ADD GENERATE AND TEST HERE... If we need to..

    return mission;
}

/**********************************************************************
 * $Log: cnp_server_manager.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.2  2007/06/28 01:33:19  endo
 * Premission CNP prepared for 6/28/2007 demo.
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.14  2006/05/14 06:34:26  endo
 * gcc-3.4 upgrade.
 *
 * Revision 1.13  2005/08/12 21:49:01  endo
 * More improvements for August demo.
 *
 * Revision 1.12  2005/08/12 16:43:45  alanwags
 * Added error codes
 *
 * Revision 1.11  2005/08/09 19:12:15  endo
 * Things improved for the August demo.
 *
 * Revision 1.10  2005/07/22 20:01:47  alanwags
 * Fixes for cbr-cnp integration
 *
 * Revision 1.9  2005/07/18 21:02:07  alanwags
 * CBR-CNP Integration fixes
 *
 * Revision 1.8  2005/07/14 19:52:37  alanwags
 * Fixed a bug and added code for other tasks
 *
 * Revision 1.7  2005/07/06 18:11:16  alanwags
 * Added code to fix linking problems. Also added code for intercept and inspect tasks.
 *
 * Revision 1.6  2005/07/03 00:09:01  alanwags
 * Link error fixed
 *
 * Revision 1.5  2005/06/23 22:52:23  alanwags
 * Changes for bidding
 *
 * Revision 1.4  2005/06/22 20:01:12  endo
 * TaskInfo added.
 *
 **********************************************************************/
