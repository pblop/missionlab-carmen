#ifndef CNP_INSPECT_TASK_H
#define CNP_INSPECT_TASK_H

#include "AbstractCNPTask.h"
#include "cnp_server_manager.h"
#include <string>
#include <list>

using std::string;
using std::list;


class CNPInspectTask : public AbstractCNPTask {

 public:
  CNPInspectTask(void)  {};

 public:
  const string    GetTaskName();
  double          CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints);   

};

#endif
