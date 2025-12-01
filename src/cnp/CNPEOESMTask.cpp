#include "CNPEOESMTask.h"

const string CNPEOESMTask::GetTaskName()               { return "EOESMTask"; }

double CNPEOESMTask::CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints) {

    double bid = 1;

    // determine if the robot has the sensor
    list<CNP_RobotConstraint>::iterator rbtCIter;
    for(rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end();rbtCIter++) {
	  if (rbtCIter->strConstraintName == "EO_ESM") {
		  if ((*rbtCIter).strConstraintValue != "True") {
			bid = 0;
          }
		}
	}
	return bid;
}


