/**********************************************************************
 **                                                                  **
 **                            vbcp.cpp	        				     **
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

/* $Id: vbcp.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"

#include "vbcp.h"

VectorM::VectorM(){
  x = 0;
  y = 0;
  z = 0;
}

VectorM::VectorM(double _x, double _y){
  x = _x;
  y = _y;
  z = 0;
}

VectorM::VectorM(double _x, double _y, double _z){
  x = _x;
  y = _y;
  z = _z;
}

VectorM::~VectorM(){
}

double VectorM::getMagnitude(){
  return sqrt(x*x + y*y + z*z);
}

double VectorM::getMagnitudeXY(){
  return sqrt(x*x + y*y);
}

double VectorM::getTheta(){
  if(x > 0.001)
    return atan(y/x);
  else if(x < -0.001)
	return atan(y/x) + PI;
  else if(y < 0)
    return 3 * PI / 2;
  else if(y > 0)
    return PI / 2;
  else
    return 0;
}

double VectorM::getX(){
  return x;
}

double VectorM::getY(){
  return y;
}

double VectorM::getZ(){
  return z;
}

void VectorM::setX(double _x){
  x = _x;
}

void VectorM::setY(double _y){
  y = _y;
}

void VectorM::setZ(double _z){
  z = _z;
}

void VectorM::setXY(double _x, double _y){
  setX(_x);
  setY(_y);
}

void VectorM::setXYZ(double _x, double _y, double _z){
  setX(_x);
  setY(_y);
  setZ(_z);
}

void VectorM::setMagnitude(double _mag){
  double old_mag = getMagnitude();
  double scale = _mag / old_mag;
  setX(x * scale);
  setY(y * scale);
  setZ(z * scale);
}

void VectorM::setTheta(double _theta){
  double mag_xy = getMagnitudeXY();
  x = cos(_theta) * mag_xy;
  y = sin(_theta) * mag_xy;
}

void VectorM::rotateTheta(double _delta){
  double _theta = getTheta();
  double new_theta = _theta + _delta;
  if(new_theta > 2*PI){
    new_theta = new_theta - (2 * PI);
  }else if(new_theta < 0){
    new_theta = new_theta + (2 * PI);
  }
  setTheta(new_theta);
}

VectorM VectorM::value(){
  VectorM retval;
  retval.setXYZ(x,y,z);
  return retval;
}

vbcp::vbcp(){
  robot_positions = new VectorM[num_robots];
  signal_strengths = new double[num_robots];
  look_ahead_dist = 2.0;
  walls = readWalls();
  signal_threshold = 1.7;
  my_heading = 0;
}

vbcp::vbcp(int _num_robots, int _id_num){
  id_num = _id_num;
  num_robots = _num_robots;
  robot_positions = new VectorM[num_robots];
  signal_strengths = new double[num_robots];
  index2id = new int[num_robots];
  look_ahead_dist = 2.0;
  walls = readWalls();
  signal_threshold = 1.7;
  my_index = 0;
  my_heading = 0;
}

vbcp::~vbcp(){
  delete robot_positions;
  delete signal_strengths;
  delete walls;
}

void vbcp::NetStateUpdate(Comm_Sensor_Data* commdata, double heading){

  int i=0;
  num_live_robots = commdata->num_readings;
  my_heading = (heading / 180) * M_PI;
  
  for( i = 0; i < num_live_robots; i++){
    robot_positions[i] = VectorM(commdata->readings[i].v.x,
								commdata->readings[i].v.y);
  }

  for( i = 0; i < num_live_robots; i++){
    signal_strengths[i] = commdata->readings[i].signal_strength;
    index2id[i] = commdata->readings[i].id;
    if(index2id[i] == id_num){
      my_index = i;
    }
  }
  
  double highest_signal = -1;
  double second_signal = -1;
  double temp = 0;
  
  for(int i = 0; i < num_robots; i++){
    if(index2id[i] != id_num){
      
      temp = signal_strengths[i];//predictSignalStrength(globalLoc,&robot_positions[i]);
      if(temp > highest_signal){
	second_signal = highest_signal;
	highest_signal = temp;
      }
      else if(temp > second_signal){
		second_signal = temp;
      }
    }
  }
  if(second_signal < 0)
    second_signal = highest_signal;
  double sum = highest_signal + second_signal;
  double diff = highest_signal - second_signal;
  
  current_val = 10 / ((1 + exp(-6 * ((sum / 100) - 1.5))) * 
		      (1 + exp(10 * (diff/100 - .2))));
}

void VectorM::addVector(VectorM* v){
  setXYZ(x + v->getX(), y + v->getY(), z + v->getZ());
}

void VectorM::subtractVector(VectorM* v){
  setXYZ(x - v->getX(), y - v->getY(), z - v->getZ());
}

VectorM* vbcp::getSuggestedVector(){
  VectorM* ret_vect = new VectorM(0,0,0);
  VectorM* look_ahead = new VectorM(1,1,0);
  //  VectorM* global_look_ahead = new VectorM(1,1,0);
  
  look_ahead->setMagnitude(look_ahead_dist);
  double look_ahead_theta = 0;
  look_ahead->setTheta(look_ahead_theta);
  look_ahead->addVector(&robot_positions[my_index]);
  double look_ahead_score = 0;
  for(int i = 0; i < 4; i++){
	look_ahead_score = calcValueAt(look_ahead);	
	look_ahead->subtractVector(&robot_positions[my_index]);
	look_ahead->setMagnitude(look_ahead_score);	
	ret_vect->addVector(look_ahead);
	look_ahead->setMagnitude(look_ahead_dist);
	look_ahead_theta += 2*PI/4;
	look_ahead->setTheta(look_ahead_theta);
	look_ahead->addVector(&robot_positions[my_index]);
  }

  if(current_val > signal_threshold + 1){
	ret_vect->setMagnitude(1/(current_val - signal_threshold));
	confidence = ret_vect->getMagnitude();
  }
  else if (ret_vect->getMagnitude() < .01 ){
	ret_vect->setMagnitude(0);
	confidence = 0;
  }
  else{
	ret_vect->setMagnitude(1);
	confidence = 1;
  }
  delete look_ahead;
  ret_vect->rotateTheta(-my_heading);
  
  //fprintf(stderr, "ret_vect.x = %f, ret_vect.y = %f\n", ret_vect->getX(), ret_vect->getY());
  
  return ret_vect;
}

double vbcp::getConfidence(){
  return confidence;
}

double vbcp::calcValueAt(VectorM* globalLoc){
  double highest_signal = -1;
  double second_signal = -1;
  double temp = 0;

  for(int i = 0; i < num_robots; i++){
    if(index2id[i] != id_num){
	
	  temp = predictSignalStrength(globalLoc,&robot_positions[i]);
	  if(temp > highest_signal){
	    second_signal = highest_signal;
	    highest_signal = temp;
	  }
	  else if(temp > second_signal){
		second_signal = temp;
	  }
    }
  }
  if(second_signal < 0)
    second_signal = highest_signal;
  
  double sum = highest_signal + second_signal;
  double diff = highest_signal - second_signal;

  return 10 / ((1 + exp(-6 * ((sum / 100) - 1.5))) * 
			(1 + exp(10 * (diff/100 - .2))));
}

double vbcp::predictSignalStrength(VectorM* sender, VectorM* receiver){
  
  if(haveLOS(sender, receiver)){

	double signal=0;
    
	//    double x1 = sender->getX();
	//    double y1 = sender->getY();
	//    double z1 = sender->getZ() + antenna_height;
    
	//    double x2 = receiver->getX();
	//    double y2 = receiver->getY();
	//    double z2 = receiver->getZ() + antenna_height;
    
    
    double power_r = receivedPower(sender, receiver);
    
    double RequiredSNR = 
	 _ReqEbNo + _BandExpansion;
    
    double receiverSensitivity = 
	 getNoiseFloor(_rateMbps, _noiseFigure) + RequiredSNR;
    
    signal = power_r - receiverSensitivity; 
    
    return signal;
  }
  else{
    /*Assuming all walls are opaque, communication-wise.*/
    return 0;
  }
}


double vbcp::receivedPower(VectorM* sender,
				   VectorM* receiver)
{    
  double pathloss = predictPathLoss(sender, receiver);
  
  double receivedPower =  
    transmittedPower
    + transmitAntennaGain
    + receiveAntennaGain
    - pathloss;
  
  return receivedPower;
}


double vbcp::getNoiseFloor(double rate, double noiseFigure)
{
  if (rate == 1)   return (-113.97 + noiseFigure);
  if (rate == 2)   return (-110.96 + noiseFigure);
  if (rate == 5.5) return (-106.57 + noiseFigure);
  if (rate == 11)  return (-103.56 + noiseFigure);
  else return (-103.56 + noiseFigure);
}


lineRec2d_t* vbcp::readWalls(){
  lineRec2d_t* retval = new lineRec2d_t;
  lineRec2d_t* last;
  lineRec2d_t* curr;
  float x1,x2,y1,y2,h;
  //double *pts;
  //  int counter = 0;
  FILE* Infile = fopen("CommMap.map", "r");
  curr = retval;
  while(!feof(Infile)){
    fscanf(Infile, "%f %f %f %f %f\n", 
		 &x1, &y1, &x2, &y2, &h);
    setLineRec(x1,y1,x2,y2,curr);
    curr->height = h;
    curr->trueheight = h;
    curr->next = new lineRec2d_t;
    last = curr;
    curr = last->next;
  }
  fclose(Infile);
  return retval;
}


bool vbcp::haveLOS(VectorM* sender, VectorM* receiver){
  int num_walls_intersected;
  if(haveLOSwalls(sender->getX(), sender->getY(), sender->getZ(),
			   receiver->getX(), receiver->getY(), receiver->getZ(),
			   walls, &num_walls_intersected) == NULL)
    return true;
  else
    return false;
}

double vbcp::predictPathLoss(VectorM* sender, VectorM* receiver){
    
  double x1 = sender->getX();
  double y1 = sender->getY();
  double z1 = sender->getZ() + antenna_height;
  
  double x2 = receiver->getX();
  double y2 = receiver->getY();
  double z2 = receiver->getZ() + antenna_height;
  
  double pathloss = 0;
  //  double receivedPower = 0;  
  
  double ground_distance = sqrt(pow(x2 - x1, 2) 
						  + pow(y2 - y1, 2));
  
  double distance = sqrt(pow(x2 - x1, 2) 
					+ pow(y2 - y1, 2)
					+ pow(z2 - z1, 2));
  
  distance *=  quality;
  ground_distance *=  quality;
  
  double lambda = 3e8 / comm_frequency;
  
  /*FREE SPACE MODEL*/
  pathloss = -20 * log10(lambda / (4 * PI * distance)); 
  
  return(pathloss);
}

/**********************************************************************
 * $Log: vbcp.cpp,v $
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
