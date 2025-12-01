/**********************************************************************
 **                                                                  **
 **                               pfilter.c                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Matthew Powers                                     **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: pfilter.c,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#include "pfilter.h"
#include "matt_lib.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "../../PoseCalculatorTypes.h"

#define PFILTER_CRAMPDEG(d,l,h) {while((d)<(l))(d)+=360.0; while((d)>=(h))(d)-=360.0;}
#define PFILTER_RAD2DEG(val) ((val)*180.0/M_PI)
#define PFILTER_DEG2RAD(val) ((val)*M_PI/180.0)

extern void ParticleFilterPrintf(const char *format, ...);

const double PFilter::NU_ = 2.0;
const double PFilter::ETA_L_ = 0.001;
const double PFilter::ETA_S_ = 0.1;

Particle::Particle(){
  x = 0;
  y = 0;
  z = 0;
  theta = 0;
}

Particle::~Particle(){
}

double Particle::getX(){
  return x;
}
double Particle::getY(){
  return y;
}
double Particle::getZ(){
  return z;
}
double Particle::getTheta(){
  return theta;
}

void Particle::setX(double new_x){
  x = new_x;
}
void Particle::setY(double new_y){
  y = new_y;
}
void Particle::setZ(double new_z){
  z = new_z;
}
void Particle::setTheta(double new_theta){
  theta = new_theta;
}


PFilter::PFilter(){
  
}

PFilter::~PFilter(){
  delete probabilities;
  delete filter;
  delete pred_filter;
  delete theta_filter;
  delete theta_pred_filter;
}

Particle PFilter::getParticle(int particle_index){
  return filter[particle_index];
}

void PFilter::setParticle(int particle_index, 
					 Particle new_particle){
  filter[particle_index] = new_particle;
}

//initializes everything
void PFilter::setup(int new_sample_size, PoseCalcFusedPose_t fusedPose, 
                    vector<PoseCalcLocationInfo_t> locData,
                    vector<PoseCalcRotationInfo_t> rotData){
    double rawGyro;
    useFrameResetting = false;
    p_bar = 0;
    p_l = 0;
    p_s = 0;
    sample_size = new_sample_size;
    
    probabilities = new double[sample_size];
    theta_probabilities = new double[sample_size];
    filter = new Particle[sample_size];
    pred_filter = new Particle[sample_size];
	theta_filter = new Particle[sample_size];
    theta_pred_filter = new Particle[sample_size];

//draw samples from the fused pose given
    for(int i = 0; i < sample_size; i++){

	  filter[i].setX(normal_sample(fusedPose.pose.loc.x, 
								   sqrt(fusedPose.variance.locVariance.x)));
	  filter[i].setY(normal_sample(fusedPose.pose.loc.y, 
								   sqrt(fusedPose.variance.locVariance.y)));
	  filter[i].setZ(normal_sample(fusedPose.pose.loc.z, 
								   sqrt(fusedPose.variance.locVariance.z)));
	  
	  theta_filter[i].setTheta(normal_sample(fusedPose.pose.rot.yaw, 
									   sqrt(fusedPose.variance.rotVariance.yaw)));  
  
    }
    //calculate the descriptive stats
    calcFilterStats();
  

  
    //initial_gyro reading, so we can use the gyro like a compass

  
    last_odox = 0;
    last_odoy = 0;
    last_odot = 0;
    
    //initialize the last odometry readings
    for(int j = 0; j < (int)locData.size(); j++){
        if((uint)locData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            last_odox = locData[j].rawLoc.x;
            last_odoy = locData[j].rawLoc.y;
        }   
    }     
    for(int j = 0; j < (int)rotData.size(); j++){
        if((uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            last_odot = rotData[j].rawRot.yaw;
        }
        if((uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_GYRO){
            rawGyro = rotData[j].rawRot.yaw;
        }
    }
    while(rawGyro > 360)
        rawGyro -= 360;
    while(rawGyro < 0)
        rawGyro += 360;
    double gyro_init = fusedPose.pose.rot.yaw - rawGyro;
    while(gyro_init > 360)
        gyro_init -= 360;
    while(gyro_init < 0)
        gyro_init += 360;
    initial_gyro.setTheta(gyro_init); 

}

//applies the motion model by updating the filter with odometry readings
void PFilter::applyMotionModel(vector<PoseCalcLocationInfo_t>locData,
						 vector<PoseCalcRotationInfo_t>rotData){
  double theta;
  double odo_x, odo_y;
  double odo_xs, odo_ys;
  double odo_dist;
  double odo_turn;
  double drive_angle;
  bool have_odometry = false;
  bool going_backwards = false;
  int rot_odo_index = -1;
  int loc_odo_index = -1;

  //Do any preliminary math on the locational odometry readings
  for(int j = 0; (uint)j < locData.size(); j++){
	/*
	  if(!locData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] &&
	  (uint)locData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){
	*/
	if((uint)locData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){

	  //if we got here, we know we have an odometry reading
	  have_odometry = true;
            
	  //the index for the odometry reading
	  loc_odo_index = j;

	  //if the last odo reading isn't initialized, do it.
	  if(last_odox == 0){
		last_odox = locData[j].rawLoc.x;
	  }
	  if(last_odoy == 0){
		last_odoy = locData[j].rawLoc.y;
	  }
	    
            
	  //get a guess at which direction we're driving in
	  drive_angle = atan2(locData[j].loc.y - mean.getY(), 
						  locData[j].loc.x - mean.getX());
	    
	  //guess if we're driving backwards
	  if((drive_angle - ((PI/180) * mean.getTheta()) >= PI/2 ||
		  drive_angle - ((PI/180) * mean.getTheta()) < -PI/2) &&
		 drive_angle - ((PI/180) * mean.getTheta()) <= PI &&
		 drive_angle - ((PI/180) * mean.getTheta()) > -PI){
        going_backwards = true;
	  }
	  else{
		going_backwards = false;
	  }
            
	  //figure out how far we've gone
	  odo_x = locData[j].rawLoc.x - last_odox;
	  odo_y = locData[j].rawLoc.y - last_odoy;
	  odo_dist = sqrt(pow(odo_x,2)+pow(odo_y,2));
	}	  
        
  }
                       
  //Do any preliminary math on the rotational odometry readings
  for(int j = 0; (uint)j < rotData.size(); j++){
	/*
	  if(!rotData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] && 
	  (uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){
	*/
	if((uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            
	  //if we got here, we know we have an odometry reading
	  have_odometry = true;

	  //the index for the odometry reading
	  rot_odo_index = j;

	  //how much we turned
	  odo_turn = rotData[j].rawRot.yaw - last_odot;
	    
	    
	  if(odo_turn > 180)
		odo_turn -= 360;
	  if(odo_turn <= -180)
		odo_turn += 360;

	} 
        
  }            
  for(int i = 0; i < sample_size; i++){//for each particle
	if(have_odometry){//have a reading?

	if(loc_odo_index>=0)
{	  
	if(useFrameResetting){//using the frame resetting technique?
		pred_filter[i].setX(normal_sample(locData[loc_odo_index].loc.x, 
										  sqrt(locData[loc_odo_index].variance.x)));
		pred_filter[i].setY(normal_sample(locData[loc_odo_index].loc.y,
										  sqrt(locData[loc_odo_index].variance.y)));
	  }
                    
	  else{//not using frame resetting
		/*if(going_backwards){//going backwards, so move backwards
		  odo_xs = odo_dist * cos(((PI/180) * filter[i].getTheta()) + PI);
		  odo_ys = odo_dist * sin(((PI/180) * filter[i].getTheta()) + PI);
		  }else{//not going backwards
		  odo_xs = odo_dist * cos((PI/180) * filter[i].getTheta());
		  odo_ys = odo_dist * sin((PI/180) * filter[i].getTheta());
		  }
		*/
		//This is to make the particle filter more like the kalman filter
		if(going_backwards){//going backwards, so move backwards
		  odo_xs = odo_dist * cos(((PI/180) * mean.getTheta()) + PI);
		  odo_ys = odo_dist * sin(((PI/180) * mean.getTheta()) + PI);
		}else{//not going backwards
		  odo_xs = odo_dist * cos((PI/180) * mean.getTheta());
		  odo_ys = odo_dist * sin((PI/180) * mean.getTheta());
		}
		
		pred_filter[i].setX(filter[i].getX() +
							normal_sample(odo_xs, 
										  sqrt(locData[loc_odo_index].variance.x)));
		pred_filter[i].setY(filter[i].getY() +
							normal_sample(odo_ys, 
										  sqrt(locData[loc_odo_index].variance.y)));	 
                            

	  }
}
else
{
	pred_filter[i].setX(filter[i].getX());
	  pred_filter[i].setY(filter[i].getY());   
}

if(rot_odo_index>=0)
{
	  if(useFrameResetting){//using frame resetting technique  THIS IS EVIL
		theta_pred_filter[i].setTheta(normal_sample(rotData[rot_odo_index].rot.yaw,
													sqrt(rotData[rot_odo_index].variance.yaw)));
	  }   
	  else{//not using frame resetting technique
		theta_pred_filter[i].setTheta(theta_filter[i].getTheta() + 
									  normal_sample(odo_turn,
													sqrt(rotData[rot_odo_index].variance.yaw)));
	  }
}
else
{
	  theta_pred_filter[i].setTheta(theta_filter[i].getTheta());  
}	  
	}
	else{//don't have an odo reading?  just copy the filter.
	  theta_pred_filter[i].setTheta(theta_filter[i].getTheta());  
	  pred_filter[i].setX(filter[i].getX());
	  pred_filter[i].setY(filter[i].getY());   
	}
	//make sure the theta is w/i bounds
	theta = theta_pred_filter[i].getTheta();
	PFILTER_CRAMPDEG(theta, 0.0, 360.0);
	theta_pred_filter[i].setTheta(theta);
    
  }
  //update the last odo readings
  if(have_odometry){
if(loc_odo_index>=0)
{
	last_odox = locData[loc_odo_index].rawLoc.x;
	last_odoy = locData[loc_odo_index].rawLoc.y;
}

if(rot_odo_index>=0)
{
	last_odot = rotData[rot_odo_index].rawRot.yaw;
}
  }
}

//evaluates the likelihood of particles based on sensor readings
void PFilter::applySensorModel(vector<PoseCalcLocationInfo_t>locData,
			       vector<PoseCalcRotationInfo_t>rotData){
    double temp_prob, prob_sum;
    double ang_prob, loc_prob;
    double loc_dist;
    double angle_diff, gyro_diff, gyro_reading;
    double highest_prob = 0;
	double theta_highest_prob = 0;

    prob_sum = 0;
    shouldResample = false;
    locShouldResample = false;
    rotShouldResample = false;

    //figure out if we have any location readings
    for(int j = 0; (uint)j < locData.size(); j++){
        /*
        if(!locData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] &&
           (uint)locData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
        */
        if((uint)locData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            locShouldResample = true; //if we have any location readings, we should resample the filter later.

        }
    }

    for(int j = 0; (uint)j < rotData.size(); j++){
        /*
        if(!rotData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] &&
           (uint)rotData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
        */
        if((uint)rotData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            rotShouldResample = true; //if we have any rotation readings we should resample the filter later.
            
            //adjust the gyro reading, so we can read it like a compass
            if((uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_GYRO){
                gyro_reading = rotData[j].rawRot.yaw + initial_gyro.getTheta();
                while(gyro_reading >= 360)
                    gyro_reading -= 360;
                while(gyro_reading < 0)
                    gyro_reading += 360;
            }
        }
    }
    //if we have any readings we should resample
    if(locShouldResample || rotShouldResample)
        shouldResample = true;
    

    for(int i = 0; i < sample_size; i++){
    
        temp_prob = 0;
        ang_prob = 0;
        loc_prob = 0;
        if(locShouldResample){//if we don't have any location readings, no point in doing this.
            for(int j = 0; (uint)j < locData.size(); j++){
                /*
                if(!locData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] &&
                   (uint)locData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
                */
                if((uint)locData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){

                    //distance from particle to reading
                    loc_dist = sqrt(pow(pred_filter[i].getX() - locData[j].loc.x, 2) +
                                    pow(pred_filter[i].getY() - locData[j].loc.y, 2));
                    
                    //likelihood of reading given particle
                    loc_prob += normal_pdf(loc_dist, 0, sqrt(locData[j].variance.x));
                }
            }
        }
    
        //if no readings, set liklihood to 1 so as to not screw up liklihood multiplication later
        else{
            loc_prob = 1;
        }

        if(rotShouldResample){//if we don't have any rotation readings, no point in doing this
            for(int j = 0; (uint)j < rotData.size(); j++){
                /*
                if(!rotData[j].extra.read[POSECALC_FUSER_TYPE_PARTICLE_FILTER] &&
                   (uint)rotData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
                */
                if((uint)rotData[j].sensorClass != POSECALC_SENSOR_CLASS_ROBOTSHAFT){
            
                    if((uint)rotData[j].sensorClass == POSECALC_SENSOR_CLASS_GYRO){
                        
                        if(useFrameResetting){//using frame resetting on the gyro?
                            ang_prob += angle_pdf(theta_pred_filter[i].getTheta(),
                                                   rotData[j].rot.yaw,
                                                   sqrt(rotData[j].variance.yaw));
				    
                        }
                        else{//not using frame resetting on the gyro?
                            ang_prob += angle_pdf(theta_pred_filter[i].getTheta(),
                                                   gyro_reading,
                                                   sqrt(rotData[j].variance.yaw));
                        }           
                    }
                    else{//anything but the gyro
                        ang_prob +=  angle_pdf(theta_pred_filter[i].getTheta(),
                                                rotData[j].rot.yaw,
                                                sqrt(rotData[j].variance.yaw));       
	  
                    }
                    
                }
            }   
        }
        else{//if no rotation readings, set likelihood to 1 so we can multiply
            loc_prob = 1;
        }
        //multiply likelihoods btw dimensions
        temp_prob = ang_prob * loc_prob;
		//        probabilities[i] = temp_prob;
		 probabilities[i] = loc_prob;
		 theta_probabilities[i] = ang_prob;
		 prob_sum += temp_prob;//keep a sum of the likelihoods
        //set the mean to the most likely particle
        //if we don't call calcFilterStats later this will be the returned value
        if(loc_prob > highest_prob){
            highest_prob = loc_prob;   
            mean = pred_filter[i];
        }
		if(ang_prob > theta_highest_prob){
            highest_prob = ang_prob;   
            mean.setTheta(theta_pred_filter[i].getTheta());
        }
    }
    
    //normalize the likelihoods
    for(int i = 0; i < sample_size; i++){
        if(prob_sum > .1){
            probabilities[i] = probabilities[i] / prob_sum;
        }
        else{
            probabilities[i] = (double)1 / (double)sample_size;
        }
    }
    
    //p_bar is a measure of how our sensors agree with the sample
    //used in addSamples to recover from errors
    p_bar = prob_sum;
}

double PFilter::max(double a, double b){
  if(a >= b)
    return a;
  else
    return b;
}


//adds samples to the set to recover from errors
//samples drawn from maxconf fusedPose
void PFilter::addSamples(PoseCalcFusedPose_t fusedPose, 
                         vector<PoseCalcLocationInfo_t>locData,
                         vector<PoseCalcRotationInfo_t>rotData){
  //calculate the lowpass and high pass averages of p_bar
  p_l = p_l + ETA_L_ * (p_bar - p_l);
  p_s = p_s + ETA_S_ * (p_bar - p_s);
  //the number of samples to add is a function of the ratio
  //of p_s/p_l
  int n = (int) (sample_size * max(0, 1 - NU_ * (p_s/p_l)));
  if(p_l/p_bar > 10)
	p_l = p_bar * 10;

  /*
    if(n > 0)
	ParticleFilterPrintf("adding %d samples, p_bar = %f p_l = %f p_s = %f\n", 
	n,p_bar,p_l,p_s);
  */  
  int index;
  //draw samples from fused pose:
  for(int i = 0; i < n; i++){
	index = (int) (unit_rand() * (sample_size - 1));
    
	filter[i].setX(normal_sample(fusedPose.pose.loc.x, 
								 sqrt(fusedPose.variance.locVariance.x)));
	filter[i].setY(normal_sample(fusedPose.pose.loc.y, 
								 sqrt(fusedPose.variance.locVariance.y)));
	filter[i].setZ(normal_sample(fusedPose.pose.loc.z, 
								 sqrt(fusedPose.variance.locVariance.z)));
            
        
	theta_filter[i].setTheta(normal_sample(fusedPose.pose.rot.yaw, 
										   sqrt(fusedPose.variance.rotVariance.yaw)));  
            
	if(theta_filter[i].getTheta() >= 360)
	  theta_filter[i].setTheta(filter[i].getTheta() - 360);
	else if(theta_filter[i].getTheta() < 0)
	  theta_filter[i].setTheta(filter[i].getTheta() + 360);
    
  }
}

//resamples the filter from pred_filter
void PFilter::resampleFilter(){
  double target_prob, prob_counter;
  int particle_counter;    
  for(int i = 0; i < sample_size; i++){//for each particle
	if(locShouldResample){//if we had any readings
	  particle_counter = 0;
	  target_prob = unit_rand(); //choose a number [0-1)
	  prob_counter = probabilities[particle_counter];
	  while(prob_counter < target_prob && 
			particle_counter < sample_size - 1){//count up the likelihoods to the target_prob
		particle_counter++; // this will be your index (likely particles get chosen more often)
		prob_counter += probabilities[particle_counter];
	  }
    
	  filter[i] = pred_filter[particle_counter];//copy the chosen particle
	}
	else{
	  filter[i] = pred_filter[i];//if no sensor readings, just copy the filter
	}
  }

  for(int i = 0; i < sample_size; i++){//for each particle
	if(rotShouldResample){//if we had any readings
	  particle_counter = 0;
	  target_prob = unit_rand(); //choose a number [0-1)
	  prob_counter = theta_probabilities[particle_counter];
	  while(prob_counter < target_prob && 
			particle_counter < sample_size - 1){//count up the likelihoods to the target_prob
		particle_counter++; // this will be your index (likely particles get chosen more often)
		prob_counter += theta_probabilities[particle_counter];
	  }
    
	  theta_filter[i] = theta_pred_filter[particle_counter];//copy the chosen particle
	}
	else{
	  theta_filter[i] = theta_pred_filter[i];//if no sensor readings, just copy the filter
	}
  }
}


//calculates the descriptive stats of the filter
void PFilter::calcFilterStats(){
  double x_counter, y_counter, z_counter;
  double thetax_counter, thetay_counter;
  double x_diff, y_diff, z_diff, theta_diff;
  double temp_diff;
  x_counter = 0;
  y_counter = 0;
  z_counter = 0;
  thetax_counter = 0;
  thetay_counter = 0;
  x_diff = 0;
  y_diff = 0;
  z_diff = 0;
  theta_diff = 0;
  for(int i = 0; i < sample_size; i++){
    x_counter += filter[i].getX();
    y_counter += filter[i].getY();
    z_counter += filter[i].getZ();
    thetax_counter += cos((PI/180) * theta_filter[i].getTheta());
    thetay_counter += sin((PI/180) * theta_filter[i].getTheta());
  }
  mean.setX( x_counter / sample_size);
  mean.setY( y_counter / sample_size);
  mean.setZ( z_counter / sample_size);
  if(thetay_counter > 0){
    mean.setTheta((180 / PI) * 
			   acos(thetax_counter / 
				   sqrt((thetax_counter * 
					    thetax_counter) +
					   (thetay_counter * 
					    thetay_counter))));
  }else{  
    mean.setTheta(360 - (180 / PI) * 
			   acos(thetax_counter / 
				   sqrt((thetax_counter * 
					    thetax_counter) +
					   (thetay_counter * 
					    thetay_counter))));
  }
  
  if(mean.getTheta() > 360){
    mean.setTheta(mean.getTheta() - 360);
  } 
  if(mean.getTheta() < 0){
    mean.setTheta(mean.getTheta() + 360);
  }
  for(int i = 0; i < sample_size; i++){
    x_diff += (filter[i].getX() - mean.getX()) * 
	 (filter[i].getX() - mean.getX());
    y_diff += (filter[i].getY() - mean.getY()) * 
	 (filter[i].getY() - mean.getY());
    z_diff += (filter[i].getZ() - mean.getZ()) * 
	 (filter[i].getZ() - mean.getZ());
    
    temp_diff = theta_filter[i].getTheta() - mean.getTheta();
    if(temp_diff < 180 && temp_diff > -180)
	 theta_diff += temp_diff * temp_diff;
    else if(temp_diff > 180){
	 temp_diff = 360 - theta_filter[i].getTheta() + mean.getTheta();
	 theta_diff += temp_diff * temp_diff;
    }
    else{
	 temp_diff = 360 - mean.getTheta() + theta_filter[i].getTheta();
	 theta_diff += (temp_diff + 180) * (temp_diff + 180);
    }
    

  }
  var.setX(sqrt(x_diff/sample_size));
  var.setY(sqrt(y_diff/sample_size));
  var.setZ(sqrt(z_diff/sample_size));
  var.setTheta(sqrt(theta_diff/sample_size));
}

//fills in the fused pose for the PoseCalculator
void PFilter::setFusedPose(PoseCalcFusedPose_t* fusedPose){
  fusedPose->pose.loc.x = mean.getX();
  fusedPose->pose.loc.y = mean.getY();
  fusedPose->pose.loc.z = mean.getZ();
  fusedPose->pose.rot.yaw = mean.getTheta();
  fusedPose->confidence.locConfidence.value = 1.0;
  fusedPose->confidence.rotConfidence.value = 1.0;
  fusedPose->variance.locVariance.x = var.getX();
  fusedPose->variance.locVariance.y = var.getY();
  fusedPose->variance.locVariance.z = var.getZ();
  fusedPose->variance.rotVariance.yaw = var.getTheta();
  fusedPose->extra.validLoc = true;
  fusedPose->extra.validRot = true;
}

Particle PFilter::getMean(){
  return mean;
}

Particle  PFilter::getVar(){
  return var;
}

void PFilter::printStats(){
  ParticleFilterPrintf("Mean xyt = %f,%f,%f; Var xyt = %f,%f,%f\n",
		       mean.getX(),mean.getY(),mean.getTheta(),
		       var.getX(),var.getY(),var.getTheta());
}

void PFilter::dumpFilter(char* filename){
  FILE *file;
  Particle temp_particle;
  file = fopen(filename, "w");
  double theta;
  for(int i = 0; i < sample_size; i++){
    temp_particle = filter[i];
	theta = theta_filter[i].getTheta();
    fprintf(file, "%f %f %f\n", temp_particle.getX(), 
		  temp_particle.getY(), theta);
  }
  fclose(file);
}

double* PFilter::getXs(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = filter[i];
	returnval[i] = temp_particle.getX();
  }
  return returnval;
}

double* PFilter::getYs(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = filter[i];
	returnval[i] = temp_particle.getY();
  }
  return returnval;
}

double* PFilter::getTs(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = theta_filter[i];
	returnval[i] = temp_particle.getTheta();
  }
  return returnval;
}

double* PFilter::getXs_pred(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = pred_filter[i];
	returnval[i] = temp_particle.getX();
  }
  return returnval;
}

double* PFilter::getYs_pred(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = pred_filter[i];
	returnval[i] = temp_particle.getY();
  }
  return returnval;
}

double* PFilter::getTs_pred(){
  double* returnval = new double[sample_size];
  Particle temp_particle;
  for(int i = 0; i < sample_size; i++){
	temp_particle = pred_filter[i];
	returnval[i] = temp_particle.getTheta();
  }
  return returnval;
}

/**********************************************************************
# $Log: pfilter.c,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
