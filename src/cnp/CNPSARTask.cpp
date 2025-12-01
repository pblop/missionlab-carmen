#include "CNPSARTask.h"

const string CNPSARTask::GetTaskName()               { return "SARTask"; }

double CNPSARTask::CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints) {

    double bid = 1;

    // determine if the robot has the sensor
    list<CNP_RobotConstraint>::iterator rbtCIter;
    for(rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end();rbtCIter++) {
	  if (rbtCIter->strConstraintName == "SAR") {
		  if ((*rbtCIter).strConstraintValue != "True") {
			bid = 0;
          }
		}
	}
	return bid;
}


