/**********************************************************************
 **                                                                  **
 **                    CRecoveryBehaviors.cpp			    		 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CRecoveryBehaviors.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"
#include <time.h>
#include <math.h>
#include "comm_data.h"
#include "CWaypointstack.h"
#include "CRecoveryBehaviors.h"


// constructor:  loads configuration data and sets up sensor interface variables
Recovery_Behaviors::Recovery_Behaviors(void) {
  int cnt;

  SIGSTR_THRESH_L = 64.0;
  SIGSTR_THRESH_H = 70.0;

  Load_Configuration();
  Waypoint_Stack = new WaypointStack(Waypoint_Interval);
  Confidence = 0;

  Timeout_List = new Comm_Stats[MAX_ROBOTS];
  for (cnt = 0; cnt < MAX_ROBOTS; cnt++) {
    Timeout_List[cnt] = CONNECTED;
  }

  Tolerance_List = new long[MAX_ROBOTS];
  for (cnt = 0; cnt < MAX_ROBOTS; cnt++) {
    Tolerance_List[cnt] = time(NULL);
  }

  CurSequence = 0;
  Sequence_Time = 0;
  Robot_Id = -1;

  Waypoint_Stop = 0;
}

void Recovery_Behaviors::SetSignalThresholdLow(float signalThreshold)
{
	SIGSTR_THRESH_L = signalThreshold;
	printf("Nuevo threshold bajo: %g\n", SIGSTR_THRESH_L);
}

void Recovery_Behaviors::SetSignalThresholdHigh(float signalThreshold)
{
	SIGSTR_THRESH_H = signalThreshold;
	printf("Nuevo threshold alto: %g\n", SIGSTR_THRESH_H);
}

// updates the sensor data
void Recovery_Behaviors::UpdateSensorDataPointer(SensorData& pSensor)
{
  Position = pSensor.GetPosition();
  Heading = pSensor.GetHeading();
  QosData = pSensor.GetCommData();
}

Recovery_Behaviors::~Recovery_Behaviors(void) {
  delete []Timeout_List;
  delete []Tolerance_List;
  delete Waypoint_Stack;
}

// loads configuration information from a text file within
// the current directory
void Recovery_Behaviors::Load_Configuration(void) {
  FILE *INFILE;

  INFILE = fopen("recov.parm", "r");
  if (!INFILE) {
    fprintf(stderr, "Recovery Configuration File (recov.parm) Not Found! - Using default values\n");
    Robot_Id = 1;
    Num_Robots = 1;
    Timeout = 30;
    Waypoint_Interval = 4;
    Retro_Priority = 1;
    Near_Priority = 0;
    Sequence_Time = 30;
    Team_Recovery = 0;
    Solo_Recovery = 1;
    return;
  }

  fscanf(INFILE, "%d,%d,%d,%d,%d,%d,%d,%d,%d", &Robot_Id, &Num_Robots, &Timeout, &Waypoint_Interval, &Retro_Priority, \
	 &Near_Priority, &Sequence_Time, &Team_Recovery, &Solo_Recovery);
  fclose(INFILE);
}


// returns the confidence level for a given behavior
double Recovery_Behaviors::Get_Confidence(void) {
  return Confidence;
}


// returns a vector representing the robots movement
Comm_Vector Recovery_Behaviors::Get_Movement(void) {

  int cnt;
  long curtime;
  int need_recov = 0;
  Comm_Vector output;
  output.x=0; output.y=0; output.z=0;
  int iamlost = 0;


  // grab our position information
  curtime = time(NULL);
  Waypoint_Stack->addPosition(Position);

  // grab our id in case it changed
  for (cnt = 0; cnt < QosData.num_readings; cnt++) {
    if (QosData.readings[cnt].hops == 0) {
      Robot_Id = QosData.readings[cnt].id;
    }
  }

  // calculate communications/timeout status
  for (cnt = 0; cnt < QosData.num_readings; cnt++) {

    //printf("Robot: %d    %f\n", Robot_Id, QosData.readings[cnt].signal_strength);
    // if we can't talk to this robot
    if (QosData.readings[cnt].signal_strength <= SIGSTR_THRESH_L && QosData.readings[cnt].id != Robot_Id) {

      Timeout_List[QosData.readings[cnt].id] = BROKEN;
      Waypoint_Stack->stop();
    }

    // else we can talk to this robot
    else if ((QosData.readings[cnt].hops > 0) &&
	     (QosData.readings[cnt].hops < 15) &&
	     (QosData.readings[cnt].signal_strength >= SIGSTR_THRESH_H)) {
      Timeout_List[QosData.readings[cnt].id] = CONNECTED;
      Waypoint_Stack->start();
    }
  }

  // check to see if recovery behaviors are needed
  need_recov = 0;
  for (cnt = 0; cnt < QosData.num_readings; cnt++) {
    if (Timeout_List[QosData.readings[cnt].id] == BROKEN) {
      need_recov++;
    }
  }
  if (need_recov == QosData.num_readings - 1) {
    iamlost = 1;
  }

  // we are doing solo recovery so as to provide highest mission completion rates as per empirical studies

  if (need_recov == 0) {
    CurSequence = 0;
    Confidence = 0;
    CurSequence_Time = (long)curtime;
    //printf("don't need!\n");
  }
  else {
      //printf("Need Recovery!\n");
    // confidence is calculated based upon the number of robots we can not talk to
    Confidence = (int)((float)need_recov / (float)(QosData.num_readings - 1));


    // calculate the sequence position
    if (curtime - CurSequence_Time > Sequence_Time) {
      CurSequence_Time = (long)curtime;
      CurSequence++;
      if (CurSequence == 2) {
	CurSequence = 0;
      }
    }
    CurSequence = 1;
    // and select the running behavior
    if(CurSequence == 1) {
      output = Get_Retrotraverse();
    }
    else {
      output = Get_NearestNeighbor();
    }
  }
  output.z = 0.0;
  //printf("output: %f %f\n", output.x, output.y);

  return output;
}




// move the robot towards the closes neighbor without communications connectivity
//    or the nearest neighbor
Comm_Vector Recovery_Behaviors::Get_NearestNeighbor(void) {
  Comm_Vector output, nearest, temp;
  double l1, l2;
  int i;

  output.x = 0;
  output.y = 0;
  output.z = 0;
  nearest.x = 0;
  nearest.y = 0;
  nearest.z = 0;
  temp.x = 0;
  temp.y = 0;
  temp.z = 0;

  nearest.x = 10000;
  nearest.y = 10000;

  // find the closest robot I can't talk to
  for (i = 0; i < QosData.num_readings; i++) {


    if (QosData.readings[i].id != Robot_Id  && QosData.readings[i].signal_strength <= SIGSTR_THRESH_H) {

      temp.x = QosData.readings[i].v.x - Position.x;
      temp.y = QosData.readings[i].v.y - Position.y;
      temp.z = 0;
      temp = Rotate_Z(temp, -1 * Heading);

      l1 = Len2d(temp);

      // if the distance between me and the lost robot is greater then 4m
      if (l1 > 4) {

	// and it is the nearest robot to me
	l2 = Len2d(nearest);
	if (l1 < l2) {
	  nearest = temp;
	}
      }
    }
  }

  // Otherwise we don't do anything
  if (nearest.x == 10000) {
    nearest.x = 0.0;
    nearest.y = 0.0;
    nearest.z = 0.0;
  }

  output = nearest;
  Unit2d(&output);
  output.z = 0.0;

  //printf("Nearest Output: %f %f\n", output.x, output.y);
  return output;
}


// this method returns the output vector for the retrotraverse behavior
Comm_Vector Recovery_Behaviors::Get_Retrotraverse(void) {
  static Comm_Vector curwaypoint;
  Comm_Vector output;

  // stop the waypoints
  Waypoint_Stack->stop();

  // grab a waypoint
  if (Waypoint_Stop == 0) {
    curwaypoint = Waypoint_Stack->peek();
    Waypoint_Stop = 1;
  }

  // calculate where to go
  output.x = curwaypoint.x - Position.x;
  output.y = curwaypoint.y - Position.y;
  output = Rotate_Z(output, -1 * Heading);

  // check if we need a new waypoint
  if (Len2d(output) < 1.0) {

    // if we got to the last waypoint flag it
    if (Waypoint_Stack->IsLast()) {
      output.x = 0;
      output.y = 0;
      output.z = 0;
      Confidence = 0;
    }

    // otherwise we don't move this cycle
    else {
      output.x = 0;
      output.y = 0;
      output.z = 0;

      // grab the next waypoint
      curwaypoint = Waypoint_Stack->next();
    }
  }

  // generate movement Comm_Vector to next waypoint
  else {
    Unit2d(&output);
  }
  return output;
}


// -----------------------------------------------------------------
// Utility Functions from MissionLab for Various Vector Operations
// -----------------------------------------------------------------

// Rotate around Z axis
Comm_Vector Recovery_Behaviors::Rotate_Z(Comm_Vector v, double theta) {
  double c, s;
  Comm_Vector temp;

  temp = v;

  c = cos(theta * 3.14159265358979323846 / 180.0);
  s = sin(theta * 3.14159265358979323846 / 180.0);
  v.x = temp.x * c - temp.y*s;
  v.y = temp.x * s + temp.y*c;

  v.z = 0.0;
  return v;
}

// Returns the length of the given vector
double Recovery_Behaviors::Len2d(Comm_Vector v) {
  return sqrt(v.x * v.x + v.y *v.y);
}

// Make the given vector a unit vector
void Recovery_Behaviors::Unit2d(Comm_Vector *v) {
  Comm_Vector v2;

  v2 = *v;
  if (Len2d(*v) < 0.001) {
    v->x = 0;
    v->y = 0;
    v->z = 0;
  }
  else {
    v->x /= Len2d(v2);
    v->y /= Len2d(v2);
    v->z = 0.0;
  }
}

/**********************************************************************
 * $Log: CRecoveryBehaviors.cpp,v $
 * Revision 1.1.1.1  2008/07/14 16:44:17  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:47  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:37:55  endo
 * MissionLab 7.0
 *
 * Revision 1.1  2006/06/08 14:47:42  endo
 * CommBehavior from MARS 2020 migrated.
 *
 **********************************************************************/
