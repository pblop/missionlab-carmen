/**********************************************************************
 **                                                                  **
 **                        cnp_server_manager.h                      **
 **                                                                  **
 **  Written by:  Alan Richard Wagner                                **
 **                                                                  **
 **  Copyright 2005 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: cnp_server_manager.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CNP_SERVER_MANAGER_H
#define CNP_SERVER_MANAGER_H

#include <list>
#include <string>
#include <map>
#include <math.h>
#include <string.h>	//Old compilation error (strcmp not defined)

// ENDO - gcc 3.4
//#include "AbstractCNPTask.h"
#include "CNPConstraint.h"
// ENDO - gcc 3.4
//#include "CNPRobotConstraint.h"
#include "cnp_server.h"
//#include "cnp_task_constraint.h"

using std::list;
using std::string;
using std::multimap;

class AbstractCNPTask;

typedef multimap<int,int> CNPRobotTaskMapping; // <Robot ID, Task ID>

// Make sure to match this enum with MExpCNPStatusType_t in
// mission_expert_types.h
typedef enum CNP_OutputStatus_t {
     CNP_OUTPUT_NORMAL, // Regular one-to-one mapping
     CNP_OUTPUT_EXCESSIVE_ROBOT, // Some robot did not get a job
     CNP_OUTPUT_INCOMPLETE_MAPPING, // Some job not assigned, lack of adequate robots
	 CNP_OUTPUT_EXCESSIVE_ROBOT_AND_INCOMPLETE_MAPPING,
     CNP_OUTPUT_UNEXPECTED_ERROR
};

// If you add a new entry, make sure to add the correponding
// entry in CNP_TASK_NAMES[] (below) as well.
typedef enum CNPTaskTypes_t
{
    CNP_TASK_TYPE_TRACK,
    CNP_TASK_TYPE_INTERCEPT,
    CNP_TASK_TYPE_INSPECT,
    CNP_TASK_TYPE_OBSERVE,
    CNP_TASK_TYPE_EOD,
    CNP_TASK_TYPE_MAD,
    CNP_TASK_TYPE_SAR,
    CNP_TASK_TYPE_EOIR,
    CNP_TASK_TYPE_EOESM,
    CNP_TASK_TYPE_EOIR_AND_MAD,
    CNP_TASK_TYPE_SAR_AND_MAD,
    NUM_CNP_TASK_TYPES // Leave this at the end of the list.
};

const string CNP_TASK_NAMES[NUM_CNP_TASK_TYPES] =
{
    "TrackTask", // CNP_TASK_TYPE_TRACK
    "InterceptTask", // CNP_TASK_TYPE_INTERCEPT
    "InspectTask", // CNP_TASK_TYPE_INSPECT
    "ObserveTask", // CNP_TASK_TYPE_OBSERVE
    "EODTask", // CNP_TASK_TYPE_EOD
    "MADTask", // CNP_TASK_TYPE_MAD
    "SARTask", // CNP_TASK_TYPE_SAR
    "EOIRTask", // CNP_TASK_TYPE_EOIR
    "EOESMTask", // CNP_TASK_TYPE_EOESM
    "EOIRAndMADTask", //CNP_TASK_TYPE_EOIR_AND_MAD
    "SARAndMADTask" //CNP_TASK_TYPE_SAR_AND_MAD
};

struct CNP_InputBundle {
  list<CNP_TaskInfo>              ListOfCNPTasks;
  list<CNP_TaskConstraint>        ListOfCNPTaskConstraints;
  list<CNP_RobotConstraint>       ListOfCNPRobotConstraints; 
};

struct CNP_OutputBundle {
  CNPRobotTaskMapping robotTaskMapping;
  int status;
  string errorMsg;
};

class CNP_Server_Manager {

protected:
    CNP_Server cnpServer; 
    list<AbstractCNPTask*> CNPTasks;
    
    AbstractCNPTask *instantiateCNPTask_(string cnpTaskName);

public:
    CNP_Server_Manager(void);
    ~CNP_Server_Manager(void)             {};

    //This is the input to CNP from CBR with callback in the form of a mapping of robot to task
    CNP_OutputBundle PremissionCNPBidding(CNP_InputBundle input);

    //This is the original realtime stuff...
    bool Inject_Task(CNP_Request &request)           { return cnpServer.Inject_Task( request ); }
    void Receive_Offer(CNP_Offer &offer)             { cnpServer.Receive_Offer( offer );        }
    void Cancel_Task(int taskid)                     { cnpServer.Cancel_Task( taskid );           }
    void Complete_Task(int taskid)                   { cnpServer.Complete_Task( taskid );         }
    void Cancel_Task(CNP_Cancel *tcan, int tid)      { cnpServer.Cancel_Task( tcan, tid);         }
    std::vector<CNP_OpenRequest> Check_ForTask(int robotid) { return cnpServer.Check_ForTask( robotid ); }
    CNP_Award Check_Expire(void)                    { return cnpServer.Check_Expire();           }
    CNP_Request *Is_TaskReinjection(int tid)        { return cnpServer.Is_ReneggedTask(tid); }
    void Remove_Task(int tid)                       { cnpServer.Remove_Task(tid); }
    list<CNP_TaskConstraint> Get_TaskConstraints(int tid) { return cnpServer.Get_TaskConstraints(tid); }
};

#endif
/**********************************************************************
 * $Log: cnp_server_manager.h,v $
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
 * Revision 1.18  2006/05/14 06:51:24  endo
 * gcc-3.4 upgrade
 *
 * Revision 1.17  2006/05/14 06:34:26  endo
 * gcc-3.4 upgrade.
 *
 * Revision 1.16  2006/04/28 22:50:42  pulam
 * Constraint checking update, Terrainmap disable for large maps, renegging overhaul
 *
 * Revision 1.15  2006/01/19 01:45:40  pulam
 * Cleaned up CNP code
 *
 * Revision 1.14  2006/01/12 20:33:08  pulam
 * cnp cleanup
 *
 * Revision 1.13  2005/08/12 21:49:01  endo
 * More improvements for August demo.
 *
 * Revision 1.12  2005/08/12 16:43:30  alanwags
 * Added Error codes
 *
 * Revision 1.11  2005/07/25 15:10:42  endo
 * CNP_Mode_t variable deleted.
 *
 * Revision 1.10  2005/07/25 15:01:01  endo
 * CNP_Mode_t
 *
 * Revision 1.9  2005/07/25 14:52:22  endo
 * cnp_server_manager.h
 *
 * Revision 1.8  2005/07/25 14:52:05  endo
 * cnp_server_manager.h
 *
 * Revision 1.7  2005/07/25 14:51:39  endo
 * cnp_server_manager.h
 *
 * Revision 1.6  2005/07/05 21:09:10  alanwags
 * Recent changes for constraint names
 *
 * Revision 1.5  2005/06/22 20:01:12  endo
 * TaskInfo added.
 *
 **********************************************************************/
