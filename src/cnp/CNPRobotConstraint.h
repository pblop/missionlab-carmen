#ifndef CNP_ROBOT_CONSTRAINT_H
#define CNP_ROBOT_CONSTRAINT_H

#include <string>
#include <list>

#include "AbstractCNPTask.h"
#include "CNPConstraint.h"
#include "CNPConstraint.h"

using std::string;
using std::list;

class AbstractCNPTask;

/*
struct CNP_RobotConstraint {
  int iRobotID;
  string strConstraintName;
  CNP_ConstraintType constraintType; 
  string strConstraintValue;
  //We need this for sorting the list...
  bool operator<(const CNP_RobotConstraint & right) const         { return (this->iRobotID < right.iRobotID); }
};
*/

class CNPRobotConstraint {

 public:
  CNPRobotConstraint( int robotID )                              { iRobotID=robotID; } 
  ~CNPRobotConstraint(void)             {};

  double          BidOnTask(AbstractCNPTask* test);
  const	int       GetRobotID()                                   { return iRobotID; }              
  list<CNP_RobotConstraint>    GetConstraintList()               { return lstRobotConstraints; }
  void            AddConstraint(CNP_RobotConstraint ct);
 


 private:

  int iRobotID;
  list<CNP_RobotConstraint> lstRobotConstraints;
  
};

#endif
