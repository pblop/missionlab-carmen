#include "CNPRobotConstraint.h"

void CNPRobotConstraint::AddConstraint(CNP_RobotConstraint constraint)  
{ 
  lstRobotConstraints.push_back( constraint );  
}

double CNPRobotConstraint::BidOnTask(AbstractCNPTask* task) 
{ 
  return task->CalculateBid( lstRobotConstraints );
}
