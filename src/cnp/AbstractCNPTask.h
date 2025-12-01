#ifndef ABSTRACT_CNP_TASK_H
#define ABSTRACT_CNP_TASK_H

#include <string>
#include <list>
#include "gt_simulation.h"
#include "cnp_server_manager.h"

using std::string;
using std::list;


class AbstractCNPTask {

 public:
  virtual ~AbstractCNPTask() {};
  virtual double        CalculateBid(list< struct CNP_RobotConstraint>)=0;
  virtual const	string  GetTaskName()=0;
  
  void            AddTaskConstraint( struct CNP_TaskConstraint& tskCnst);//   {  lstTaskConstraints.push_back( tskCnst );  }
  void            SetTaskID(int ID);//                               { iCRBID = ID;  } 
  int             GetTaskID();//                                     { return iCRBID; }
  void            SetVehicleLocation( robot_position pos );//        { myLocation = pos; } 
  robot_position  GetVehicleLocation();//                            { return myLocation; }
  void            SetTargetLocation( robot_position pos );//         { targetLocation = pos; }
  robot_position  GetTargetLocation();//                             { return targetLocation; }
  void		  SetTargetVelocity( robot_position pos);
  robot_position  GetTargetVelocity();
  void SetVehicleType(int type);
  int GetVehicleType(void);
  
 protected:
  robot_position myLocation;
  robot_position targetLocation;
  robot_position targetVelocity;
  int VehicleType;
  int iCRBID;
  list< struct CNP_TaskConstraint > lstTaskConstraints;
  
};

#endif
