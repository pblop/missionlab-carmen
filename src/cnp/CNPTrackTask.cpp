#include "CNPTrackTask.h"

const string CNPTrackTask::GetTaskName()               { return "TrackTask"; }

double CNPTrackTask::CalculateBid(list< struct CNP_RobotConstraint> lstRobotConstraints)
{
 
    double bid = 1;
    int i;
    double discount = 1.0;
	
    // determine the robot type
    list<CNP_RobotConstraint>::iterator rbtCIter;
    for(rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end();rbtCIter++) {
        if (rbtCIter->strConstraintName == "VEHICLE_TYPES") {

            VehicleType = UNKNOWN_ROBOT_MOBILITY_TYPE;

            for (i = 0; i < NUM_ROBOT_MOBILITY_TYPES; i++)
            {
                if ((rbtCIter->strConstraintValue) == ROBOT_MOBILITY_TYPE_NAMES[i])
                {
                    VehicleType = i;
                    break;
                }
            }
        }
    }
  
  

    list< CNP_TaskConstraint >::iterator tskCIter;
    for (tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
    

        // ----- BEGIN MOBILITY CONSTRAINTS
        // check for vehicle mobility constraints
        if ((*tskCIter).strConstraintName == "ENVIRONMENT" ) {
      
            list<CNP_RobotConstraint>::iterator rbtCIter;
            for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {
                if ((*rbtCIter).strConstraintName == "VEHICLE_TYPES") {

                    // AIR can only be done by UAV
                    if ((*tskCIter).strConstraintValue == "AIR") {
                        if ((*rbtCIter).strConstraintValue != "UAV") {
                            bid = 0;
                        }
                    }

                    // UNDERWATER can only be done by UUV
                    else if ((*tskCIter).strConstraintValue == "UNDERWATER") {
                        if ((*rbtCIter).strConstraintValue != "UUV") {
                            bid = 0;
                        }
                    }

                    // SURFACE can be done by UAV, USV, UUV
                    else if ((*tskCIter).strConstraintValue == "SURFACE") {
                        if ((*rbtCIter).strConstraintValue == "UGV") {
                            bid = 0;
                        }
                    }
	  
                    // GROUND can be done by UAV and UGV
                    else if ((*tskCIter).strConstraintValue == "GROUND") {
                        if (((*rbtCIter).strConstraintValue != "UGV") && ((*rbtCIter).strConstraintValue != "UAV")) {
                            bid = 0;
                        }
                    }
                }
            }
        }
        // ---------------- END MOBILITY CONSTRAINTS

    
        // ----- BEGIN TARGET TYPE CONSTRAINTS
        if ((*tskCIter).strConstraintName == "TARGET_VEHICLE_TYPE" ) {
      
            list<CNP_RobotConstraint>::iterator rbtCIter;
            for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {
                if ((*rbtCIter).strConstraintName == "VEHICLE_TYPES") {

                    // UGV can only be done by UAV of UGV
                    if ((*tskCIter).strConstraintValue == "UGV") {
                        if (((*rbtCIter).strConstraintValue != "UAV") && ((*rbtCIter).strConstraintValue != "UGV")) {
                            bid = 0;            
                        }           
                    }   
                    // UaV can only be done by UAV
                    else if ((*tskCIter).strConstraintValue == "UAV") {
                        if ((*rbtCIter).strConstraintValue != "UAV") {
                            bid = 0;            
                        }               
                    }     
                    // USV can only be done by UAV or UUV or USV
                    else if ((*tskCIter).strConstraintValue == "USV") {
                        if (((*rbtCIter).strConstraintValue != "UAV") && ((*rbtCIter).strConstraintValue != "USV") && ((*rbtCIter).strConstraintValue != "UUV")) {
                            bid = 0;                
                        }               
                    }   
                    // UUV can only be done by UUV
                    else if ((*tskCIter).strConstraintValue == "UUV") {
                        if ((*rbtCIter).strConstraintValue != "UUV") {
                            bid = 0;            
                        }               
                    }   
                }
            }
       
        }


        // ---------------- END TARGET TYPE CONSTRAINTS

        


    
        // BEGIN MISSION TIME CONSTRAINTS
    
        // basically if the robot is available at the times needed in the task constraints it passes this set of constraints

        if ((*tskCIter).strConstraintName == "MISSION_OCCURANCE_TIME") {
        bool available_time = false;
            for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {
                if ((*rbtCIter).strConstraintName == "VEHICLE_AVAILABLE_TIME") {
                    if ((*rbtCIter).strConstraintValue == (*tskCIter).strConstraintValue) {
                        available_time = true;
                    }
                }
            }
            if (!available_time) {
                bid = 0;
            }
        }



        // ----- END MISSION TIME CONSTRAINTS


	 
        // BEGIN STEALTH CONSTRAINT
        if ((*tskCIter).strConstraintName == "MISSION_STEALTHINESS") {
            if ((*tskCIter).strConstraintValue == "STEALTHY") {
                for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {	
                    if ((*rbtCIter).strConstraintName == "STEALTHINESS") {
                        if ((*rbtCIter).strConstraintValue == "NOT_STEALTHY") {
                            discount *= 0.5;
                        }
                    }
                }
            }
        }
        // -- END STEALTH CONSTRAINT
    }      
      

    // ----- PU dont think we currently use this but we will keep the code here for future use
    /*	
      if(strcmp((*rbtCIter).strConstraintName.c_str(),"ABILITY_TO_MONITOR" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"MINIMAL" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"MONITORING_FREQUENCY" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"CONSTANTLY" ) == 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"ONCE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"MONITORING_FREQUENCY" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"ONCE" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }
      	
      if(strcmp((*rbtCIter).strConstraintName.c_str(),"ABILITY_TO_COMMUNICATE" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"MINIMAL" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"COMMUNICATION_FREQUENCY" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"CONSTANTLY" ) == 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"ONCE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"COMMUNICATION_FREQUENCY" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"ONCE" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }
  
      if(strcmp((*rbtCIter).strConstraintName.c_str(),"AVAILABLE_COMMLINK" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"CLOSE_PROXIMITY" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"COMMLINK_REQUIRED" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"CLOSE_TO_TARGET" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"FAR" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"COMMLINK_REQUIRED" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"FAR" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }

	
      if(strcmp((*rbtCIter).strConstraintName.c_str(),"COMM_SURFACE_TIME" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"MINIMAL" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"AVAILABLE_SURFACE_TIME" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"NIGHT_ONLY" ) == 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"NO_COMM" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"AVAILABLE_SURFACE_TIME" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"NO_COMM" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"NIGHT_ONLY" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"AVAILBLE_SURFACE_TIME" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"NIGHT_ONLY" ) != 0 ) {
      bid = 0;
      }
      }
      }
      } 
      }

      if(strcmp((*rbtCIter).strConstraintName.c_str(),"OPERATOR_ID" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"OBTAINABLE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"OPERATOR_ID_REQUIREMENT" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"IMMEDIATELY_REQUIRED" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"NOT_OBTAINABLE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"OPERATOR_ID_REQUIREMENT" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"NOT_REQUIRED" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }

      if(strcmp((*rbtCIter).strConstraintName.c_str(),"VEHICLE_TIME_VS_RISK_MANAGEMENT" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"TIME_SENSITIVE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"TIME_VS_RISK" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"TIME_SENSITIVE" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"RISK_SENSITIVE" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"TIME_VS_RISK" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"RISK_SENSITIVE" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"BALANCED" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"TIME_VS_RISK" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"BALANCED" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }

      if(strcmp((*rbtCIter).strConstraintName.c_str(),"VEHICLE_COLLISION_MANAGEMENT" ) == 0) {
      if(strcmp((*rbtCIter).strConstraintValue.c_str(),"UNCONCERNED" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"USV_COLLISION_RISK_MANAGEMENT" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"UNCONCERNED" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"WAIT" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"USV_COLLISION_RISK_MANAGEMENT" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"WAIT" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      else if(strcmp((*rbtCIter).strConstraintValue.c_str(),"SEEK_HELP" ) == 0) {
      list< CNP_TaskConstraint >::iterator tskCIter;
      for(tskCIter=lstTaskConstraints.begin();tskCIter!=lstTaskConstraints.end();tskCIter++) {
      if(strcmp((*tskCIter).strConstraintName.c_str(),"USV_COLLISION_RISK_MANAGEMENT" ) == 0) {
      if(strcmp((*tskCIter).strConstraintValue.c_str(),"SEEK_HELP" ) != 0) {
      bid = 0;
      }
      }
      }
      }
      }
	
      if(strcmp((*rbtCIter).strConstraintName.c_str(),"CURRENT_PORT_DISTANCE" ) == 0) {
      char * pEnd;
      double dPortDistance=0;
      dPortDistance = strtod((*rbtCIter).strConstraintValue.c_str(),&pEnd);
      if( dPortDistance > bid ) // not sure what this should be
      bid =0;
      else
      bid -= dPortDistance;
      }
    */


 	  //initial bid is based on distance to target
    if (bid != 0) {
        bid = 1000 - sqrt(pow(targetLocation.v.x - myLocation.v.x, 2) + pow(targetLocation.v.y - myLocation.v.y, 2));
        bid *= discount;
    }
 
	return bid;
}

