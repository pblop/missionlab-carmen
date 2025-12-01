#ifndef CNP_EOIR_AND_MAD_TASK_H
#define CNP_EOIR_AND_MAD_TASK_H

#include "AbstractCNPTask.h"
#include "cnp_server_manager.h"
#include <string>
#include <list>

using std::string;
using std::list;

class CNPEOIRAndMADTask : public AbstractCNPTask {

 public:
  CNPEOIRAndMADTask(void) {};

 public:
  const string    GetTaskName();
  double          CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints);
  
};

#endif
