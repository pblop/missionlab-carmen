/**********************************************************************
 **                                                                  **
 **                         CNPInterceptTask.cpp                     **
 **                                                                  **
 **  Written by: Patrick Ulam                                        **
 **                                                                  **
 **  Copyright 2006 Georgia Tech Research Corporation                **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CNPInterceptTask.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "CNPInterceptTask.h"
#include "gt_robot_type.h"

const string CNPInterceptTask::GetTaskName()          { return "InterceptTask"; }

double CNPInterceptTask::CalculateBid(list<CNP_RobotConstraint> lstRobotConstraints) {
 
    double bid = 1;
    int i;
	
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

            /*
            if (rbtCIter->strConstraintValue == "UUV") {
                VehicleType = CNP_UUV;
            }
            else if (rbtCIter->strConstraintValue == "UAV") {
                VehicleType = CNP_UAV;
            }
            else if (rbtCIter->strConstraintValue == "UGV") {
                VehicleType = CNP_UGV;
            }
            else if (rbtCIter->strConstraintValue == "SLUAV") {
                VehicleType = CNP_SLUAV;
            }
            else if (rbtCIter->strConstraintValue == "USV") {
                VehicleType = CNP_USV;
            } 
            else {
                fprintf(stderr, "Error in bidding: Unknown robot type: %s\n", rbtCIter->strConstraintValue.c_str());
            }
            */
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
    
    
        // BEGIN MISSION TIME CONSTRAINTS
    
        // basically if the robot is available at the times needed in the task constraints it passes this set of constraints
        bool available_time = false;
        if ((*tskCIter).strConstraintName == "MISSION_OCCURANCE_TIME") {
            for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {
                if ((*rbtCIter).strConstraintName == "VEHICLE_AVAILABLE_TIME") {
                    if ((*rbtCIter).strConstraintValue == (*tskCIter).strConstraintValue) {
                        available_time = true;
                    }
                }
            }
        }
        if (!available_time) {
            bid = 0;
        }
        // ----- END MISSION TIME CONSTRAINTS


	 
        // BEGIN STEALTH CONSTRAINT
        if ((*tskCIter).strConstraintName == "MISSION_STEALTHINESS") {
            if ((*tskCIter).strConstraintValue == "STEALTHY") {
                for (rbtCIter=lstRobotConstraints.begin(); rbtCIter!=lstRobotConstraints.end(); rbtCIter++) {	
                    if ((*rbtCIter).strConstraintName == "STEALTHINESS") {
                        if ((*rbtCIter).strConstraintValue == "NOT_STEALTHY") {
                            bid = 0;
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



    
   

    // --------------------------------------------------------------------
    // Notation:
    // <xo, yo> = object (enemy) [known]
    // <xr, yr> = robot (self) [known]
    // <xc, yc> = collision point [unknown]
    // vo = speed of the object (enemy) [known]
    // tho = angle of the object (enemy) [unknown]
    // vr = speed of the robot (self) [known]
    // thr = desired angle of the robot (self) [unknown]
    // tc = time for the collision [unknown]
    //
    // Equations to solve (4 equations 4 unknown):
    // xc = vo*tc*cos(tho) + xo
    // yc = vo*tc*sin(tho) + yo
    // xc = vr*tc*cos(thr) + xr
    // yc = vr*tc*sin(thr) + yr
    //
    // The equation would be reduced to (1 unknown: thr):
    // (vo*sin(tho)-vr*sin(thr))*(xo-xr)=(vo*cos(tho)-vr*cos(thr))*(yo-yr)
    //
    // However, solving this is hard since it contains sin and cos.
    // So, here we check the possible angles for thr, and see which
    // angle makes the LHS and RHS be the closest.
    //
    // If that fails, choose a vector pointing towards the object
    // (i.e., MoveTo).
    //
    // --------------------------------------------------------------------

    Vector objVec;
    double xo, yo, xr, yr, vo, tho, vr, thr;
    double checkValue;
    double objDirDeg, objDirRad;
    double d;
    double besttime, maxdist;
    bool compBid = false;


    // if the constraints have been met then calculate the bid
    if (bid != 0) {
        // Get the robot position
        xr = myLocation.v.x;
        yr = myLocation.v.y;

        // Get the closest object (enemy)

        xo = targetLocation.v.x;
        yo = targetLocation.v.y;


        //	rotate_z(objVec, cur_pos.heading);
        objVec.x = xo - xr;
        objVec.y = yo - yr;


        objDirRad = atan2(objVec.y, objVec.x);
        objDirDeg = RADIANS_TO_DEGREES(objDirRad);


        // set vr based on type
        if (VehicleType == UAV) {
            vr = CNP_MAXVEL_UAV / 5.0f;
        }
        else if (VehicleType == UGV) {
            vr = CNP_MAXVEL_UGV / 5.0f;
        }
        else if (VehicleType == USV) {
            vr = CNP_MAXVEL_USV / 5.0f;
        }
        else if (VehicleType == UUV) {
            vr = CNP_MAXVEL_UUV / 5.0f;
        }

        // Get the velocities and angles
        vo = sqrt(pow(targetVelocity.v.x, 2) + pow(targetVelocity.v.y, 2));
        tho = atan2(targetVelocity.v.y, targetVelocity.v.x);

        compBid = false;

        // Using Law of Sines. Solve the following equation for thr:
        // (vo*tc)/(sin(thr - objDirRad)) = (vr*tc)/(sin(pi - tho + objDirRad))
    
        if (fabs(vr) > EPS_ZERO) {
            checkValue = (vo/vr)*sin(M_PI - tho + objDirRad);
      
            if ((checkValue >= -1.0) && (checkValue <= 1.0)) {
                thr = asin(checkValue) + objDirRad;
	
                // thr calculated. Now, solve tc based on:
                // (vr*tc)/(sin(pi - tho + objDirRad)) = d/(sin(tho - thr))
                //  where d is the initial distance between the robot and
                //  the enemy
	
                d = sqrt(pow(xo-xr,2) + pow(yo-yr,2));
                besttime = (d*sin(M_PI - tho + objDirRad))/(vr*sin(tho-thr));
	
                if (besttime > 0) {
                    compBid = true;
                }
            }
        }
    
   
        if (compBid) {
            switch (VehicleType) {
	
            case UAV:
                maxdist = CNP_FUEL_EFFICIENCY_UAV * 1000;
                break;
	
            case UGV:
                maxdist = CNP_FUEL_EFFICIENCY_UGV * 1000;
                break;
	
            case USV:
                maxdist = CNP_FUEL_EFFICIENCY_USV * 1000;
                break;
	
            case UUV:
                maxdist = CNP_FUEL_EFFICIENCY_UUV * 1000;
                break;
	
            default:
                fprintf(stderr, "Error: CNPInterceptTask.cpp. Unknown VehicleType: %d\a\n", VehicleType);
                maxdist = vr * besttime; // bid will be zero.
                break;
            }
         
            bid = (maxdist - vr * besttime) / maxdist;
        }
        else {
            bid = 0;
        }
    }
    
    return bid;
}

/**********************************************************************
 * $Log: CNPInterceptTask.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.9  2006/05/02 04:12:58  endo
 * TrackTask improved for the experiment.
 *
 **********************************************************************/
