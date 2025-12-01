/**********************************************************************
 **                                                                  **
 **                               pfilter.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Matthew Powers                                     **
 **                                                                  **
 **  Copyright 2004 - 2006, Georgia Tech Research Corporation        **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: pfilter.h,v 1.1.1.1 2006/07/12 13:37:57 endo Exp $ */

#ifndef PFILTER_H
#define PFILTER_H

#define X_INDEX 0
#define Y_INDEX 1
#define Z_INDEX 2
#define THETA_INDEX 3
//#include <iostream.h>
#include <stdio.h>
#include "../../PoseCalculatorTypes.h"

class Particle{
 public:
  Particle();
  ~Particle();
  double getX();
  double getY();
  double getZ();
  double getTheta();
  void setX(double new_x);
  void setY(double new_y);
  void setZ(double new_z);
  void setTheta(double new_theta);
 private:
  double x;
  double y;
  double z;
  double theta;
  
};

class PFilter{
public:
    PFilter();
    ~PFilter();
    vector<PoseCalcLocationInfo_t> lastLocData;
    vector<PoseCalcRotationInfo_t> lastRotData;  
    void setup(int sample_size, PoseCalcFusedPose_t fusedPose, 
               vector<PoseCalcLocationInfo_t>locData,
               vector<PoseCalcRotationInfo_t>rotData);

    void applyMotionModel(vector<PoseCalcLocationInfo_t>locData,
                          vector<PoseCalcRotationInfo_t>rotData);
  
    void applySensorModel(vector<PoseCalcLocationInfo_t>locData,
                          vector<PoseCalcRotationInfo_t>rotData);

    void addSamples(PoseCalcFusedPose_t fusedPose,
                    vector<PoseCalcLocationInfo_t>locData,
                    vector<PoseCalcRotationInfo_t>rotData);
  
    void resampleFilter();

    void calcFilterStats();
    void setFusedPose(PoseCalcFusedPose_t* fusedPose);
  
    Particle getMean();
  
    Particle getVar();
  
    Particle getParticle(int particle_index);
    void setParticle(int particle_index, 
                     Particle new_particle);
    void printStats();
    void dumpFilter(char* filename);
    int sample_size;
    double* getXs();
    double* getYs();
    double* getTs();
  
    double* getXs_pred();
    double* getYs_pred();
    double* getTs_pred();
 
private:
    bool useFrameResetting;
    bool initLastLoc;
    bool initLastRot;
    bool shouldResample;
	bool locShouldResample;
	bool rotShouldResample;
    double max(double a, double b);
    Particle initial_gyro;
    Particle initial_pos;
    Particle mean;
    Particle var;
    Particle * filter;
    Particle * pred_filter;
	Particle * theta_filter;
    Particle * theta_pred_filter;
    double last_odox;
    double last_odoy;
    double last_odot;
    double last_gyro;
    double * probabilities;
	double * theta_probabilities;
    double p_bar;
    double p_l;
    double p_s;
    static const double NU_;
    static const double ETA_L_;
    static const double ETA_S_;
};

/*class FilterInt{
 public:
  //LogReader * logreader;
  //Log_Rec* logrec;
  //locrec;

  PFilter filter;
  FilterInt();
  ~FilterInt();
  void advance();
  double* getXs();
  double* getYs();
  double* getTs();
 private:

 };*/
  

#endif

/**********************************************************************
# $Log: pfilter.h,v $
# Revision 1.1.1.1  2006/07/12 13:37:57  endo
# MissionLab 7.0
#
# Revision 1.1  2006/06/05 21:35:54  endo
# PoseCalculator from MARA 2020 migrated
#
#**********************************************************************/
