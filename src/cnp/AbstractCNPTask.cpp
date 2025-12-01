#include "AbstractCNPTask.h"

void AbstractCNPTask::AddTaskConstraint( struct CNP_TaskConstraint& tskCnst)   {  
   
  if (tskCnst.strConstraintName == "TARGET_LOCATION_X") {
    targetLocation.v.x = atof(tskCnst.strConstraintValue.c_str());
  }
  else if (tskCnst.strConstraintName == "TARGET_LOCATION_Y") {
    targetLocation.v.y = atof(tskCnst.strConstraintValue.c_str());
  }
  else if (tskCnst.strConstraintName == "TARGET_LOCATION_Z") {
    targetLocation.v.z = atof(tskCnst.strConstraintValue.c_str());
  }
  else if (tskCnst.strConstraintName == "TARGET_VELOCITY_X") {
    targetVelocity.v.x = atof(tskCnst.strConstraintValue.c_str());
    //printf("tvx2: %f\n", targetVelocity.v.x);
  }
  else if (tskCnst.strConstraintName == "TARGET_VELOCITY_Y") {
   
    targetVelocity.v.y = atof(tskCnst.strConstraintValue.c_str());
    //printf("tvy: %f                    %s\n", targetVelocity.v.y, tskCnst.strConstraintValue.c_str() );
  }
  else if (tskCnst.strConstraintName == "TARGET_VELOCITY_Z") {
      //printf("tvz\n");
    targetVelocity.v.z = atof(tskCnst.strConstraintValue.c_str());
  }

  lstTaskConstraints.push_back( tskCnst );  
}

void AbstractCNPTask::SetTaskID(int ID) { 
  iCRBID = ID;  
} 

int AbstractCNPTask::GetTaskID() { 
  return iCRBID; 
}

void AbstractCNPTask::SetVehicleLocation( robot_position pos ) { 
  myLocation = pos; 
} 

robot_position  AbstractCNPTask::GetVehicleLocation() { 
  return myLocation; 
}

void AbstractCNPTask::SetTargetLocation( robot_position pos ) { 
  targetLocation = pos; 
}

robot_position  AbstractCNPTask::GetTargetLocation() { 
  return targetLocation; 
}

void AbstractCNPTask::SetTargetVelocity( robot_position pos ) {
  targetVelocity = pos;
}

robot_position  AbstractCNPTask::GetTargetVelocity() {
  return targetVelocity;
}

void AbstractCNPTask::SetVehicleType(int type) {
  VehicleType = type;
}

int AbstractCNPTask::GetVehicleType(void) {
  return VehicleType;
}

