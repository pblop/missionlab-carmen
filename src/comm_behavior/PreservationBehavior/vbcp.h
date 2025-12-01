/**********************************************************************
 **                                                                  **
 **                             vbcp.h	        				     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Matt Powers                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This file encompasses the creation of a DLL for the     **
 **	internalized plan behavior.     								 **		
 **                                                                  **
 **********************************************************************/

/* $Id: vbcp.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef VBCP_H
#define VBCP_H

#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "lineutils.h"
#include "comm_data.h"

//#define PI 3.1415926

/*struct CommModelRobot 
{ 
  CommModelRobot(int _simulation_id,
		 double _x, double _y, double _z,
		 double _antenna_height = 1.0,
		 double _transmittedPower= 24.0,
		 double _transmitAntennaGain = 0.0,
		 double _receiveAntennaGain = 0.0)
    : sim_id(_simulation_id),
       x(_x),
       y(_y),
       z(_z),
       antenna_height(_antenna_height),
       transmittedPower(_transmittedPower),
       transmitAntennaGain(_transmitAntennaGain),
       receiveAntennaGain(_receiveAntennaGain)       
  {//nothing
  }
  
  int sim_id;
  double x,y,z; 
  
  double antenna_height;       //= 1;
  double transmittedPower;     //= 24.0; 
  double transmitAntennaGain;  //= 0.0;
  double receiveAntennaGain;   //= 0.0;
} CommModelRobot; 
*/
static const double comm_frequency = 2.4e9;
  static const double _surface_constant = 4;
  static const double quality = 1.0;
  static const double antenna_height = 1.0;
  static const double transmittedPower = 24.0;
  static const double transmitAntennaGain = 0.0;
  static const double receiveAntennaGain = 0.0;
  static const double _ReqEbNo = 8.0;
  static const double _BandExpansion = 0.0;
  static const double _rateMbps = 1.0;
  static const double _noiseFigure = 1.0;
class VectorM
{
 public:
  VectorM();
  VectorM(double _x, double _y);
  VectorM(double _x, double _y, double _z);
  ~VectorM();
  double getMagnitude();
  double getMagnitudeXY();
  double getTheta();
  double getX();
  double getY();
  double getZ();
  void setX(double _x);
  void setY(double _y);
  void setZ(double _z);
  void setXY(double _x, double _y);
  void setXYZ(double _x, double _y, double _z);
  void setMagnitude(double _mag);
  void setTheta(double _theta);
  void rotateTheta(double _delta);
  void addVector(VectorM* v);
  void subtractVector(VectorM* v);
  VectorM value();
 private:
  double x;
  double y;
  double z;
  
};

class NetworkState
{
 public:
  int numTotalRobots;
  int numLiveRobots;
  bool validReading;
  //Vector globalLoc;
  VectorM* GlobalLocs;
  double* signal_strengths;
  
};

class vbcp
{
 public:
  vbcp();
  vbcp(int _num_robots, int _id_num);
  ~vbcp();
  void NetStateUpdate(Comm_Sensor_Data* commdata, double heading);
  VectorM* getSuggestedVector();
  double predictSignalStrength(VectorM* sender, VectorM* reciever); 
  double calcValueAt(VectorM* globalLoc); 
  double getConfidence();
 private:
  double my_heading;
  int my_index;
  int* index2id;
  double receivedPower(VectorM* sender, VectorM* receiver);
  double getNoiseFloor(double rate, double noiseFigure);
  VectorM* robot_positions;
  double* signal_strengths;
  int id_num;
  int num_robots;
  int num_live_robots;
  double current_val;
  double look_ahead_dist;
  double signal_threshold;
  double confidence;
  
  bool haveLOS(VectorM* sender, VectorM* reciever);
  double predictPathLoss(VectorM* sender, VectorM* reciever);
  lineRec2d_t* walls;
  lineRec2d_t* readWalls();
};
#endif  

/**********************************************************************
 * $Log: vbcp.h,v $
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
