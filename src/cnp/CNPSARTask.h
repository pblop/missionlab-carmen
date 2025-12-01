#ifndef CNP_SAR_TASK_H
#define CNP_SAR_TASK_H

#include "AbstractCNPTask.h"
#include "cnp_server_manager.h"
#include <string>
#include <list>

using std::string;
using std::list;

class CNPSARTask : public AbstractCNPTask {

 public:
  CNPSARTask(void) {};

 public:
  const string    GetTaskName();
  double          CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints);
  
};

#endif
