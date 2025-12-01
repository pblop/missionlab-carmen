#include "CNPSARAndMADTask.h"

const string CNPSARAndMADTask::GetTaskName()               { return "SARAndMADTask"; }

double CNPSARAndMADTask::CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints) {

    double bid = 1;

    // determine if the robot has the sensor
    list<CNP_RobotConstraint>::iterator rbtCIter;
    for(rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end();rbtCIter++) {
	  if (rbtCIter->strConstraintName == "MAD") {
		  if ((*rbtCIter).strConstraintValue != "True") {
			bid = 0;
          }
		}

	  if (rbtCIter->strConstraintName == "SAR") {
		  if ((*rbtCIter).strConstraintValue != "True") {
			bid = 0;
          }
		}
	}
	return bid;
}


