#ifndef CNP_EOD_TASK_H
#define CNP_EOD_TASK_H

#include "AbstractCNPTask.h"
#include "cnp_server_manager.h"
#include <string>
#include <list>

using std::string;
using std::list;

class CNPEODTask : public AbstractCNPTask {

 public:
  CNPEODTask(void)    {};

 public:
  const string    GetTaskName();
  double          CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints);
  
};

#endif
