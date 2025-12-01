/**********************************************************************
 **                                                                  **
 **                         CJboxInterface.cpp                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CJboxInterface.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "CJboxInterface.h"


// Constructor: Initilize the interface
CJboxInterface::CJboxInterface(int numrobo) 
{

  int cnt;
  
  ErrorFlag = false;
  Num_Robots = numrobo;
  Position = new Comm_Vector[MAX_JBOX_ROBOTS];
  if (!Position) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    ErrorFlag = true;
  }
  for (cnt = 0; cnt < MAX_JBOX_ROBOTS; cnt++) {
    Position[cnt].x = FLT_MAX;
  }
 

  Hop = new double[MAX_JBOX_ROBOTS];
  if (!Hop) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    ErrorFlag = true;
  }

  SignalStrength = new double[MAX_JBOX_ROBOTS];
  if (!SignalStrength) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    ErrorFlag = true;
  }

  Load_BaseInfo();

}

// deconstructor: free up the memory allocated for the interface
CJboxInterface::~CJboxInterface(void) {
  delete []Position;
  delete []Hop;
  delete []SignalStrength;
}

bool CJboxInterface::Get_ErrorFlag(void) {
  return ErrorFlag;
}

void CJboxInterface::Update_CommData(int id, int hops, float sig_strength, double lat, double lng) {
  Comm_Sensor_Data csd(1);

  csd.readings[0].id = id;
  csd.readings[0].v.x = (float)(((lng - Base_Long) *MPerLong) + Base_X);
  csd.readings[0].v.y = (float)(((lat - Base_Lat) * MPerLat) + Base_Y);
  csd.readings[0].v.z = 0;
  csd.readings[0].hops = hops;
  csd.readings[0].signal_strength = sig_strength;

  Update_CommData(&csd);
}

int CJboxInterface::Update_CommData(int id, int hops, float sig_strength, float x, float y, float z) {
  Comm_Sensor_Data csd(1);

  csd.readings[0].id = id;
  csd.readings[0].v.x = x;
  csd.readings[0].v.y = y;
  csd.readings[0].v.z = z;
  csd.readings[0].hops = hops;
  csd.readings[0].signal_strength = sig_strength;

  Update_CommData(&csd);
  return (int)Get_CommData().readings[0].v.y;
}
		


// update the jbox interface with data from simulation
void CJboxInterface::Update_CommData(Comm_Sensor_Data *commdata) {
  int cnt;


  // update all the data
  for (cnt = 0; cnt < commdata->num_readings; cnt++) {
    
    Position[commdata->readings[cnt].id].x = commdata->readings[cnt].v.x;
    Position[commdata->readings[cnt].id].y = commdata->readings[cnt].v.y;
    Position[commdata->readings[cnt].id].z = commdata->readings[cnt].v.z;
    Hop[commdata->readings[cnt].id] = commdata->readings[cnt].hops;
    SignalStrength[commdata->readings[cnt].id] = commdata->readings[cnt].signal_strength;
  }
}
    

// new robots were discovered to modify the data structure to accomadate them
void CJboxInterface::Add_Robot(int numrobo) {
  Comm_Vector *newvec;
  int cnt;
  double *newsig, *newhops;
  int *newuse;
  
  // keep track of which ids are used
  newuse = new int[numrobo];
  for (cnt = 0; cnt < Num_Robots; cnt++) {
    newuse[cnt] = Used[cnt];
  }

  // allocate enough memory for all the robots
  newvec = new Comm_Vector[numrobo];
  if (!newvec) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    exit(1);
  }

  newhops = new double[numrobo];
  if (!newhops) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    exit(1);
  }

  newsig = new double[numrobo];
  if (!newsig) {
    fprintf(stderr, "CCJboxInterface: Out of memory!\n");
    exit(1);
  }

  // fill in the new data structures with the old values
  for (cnt = 0; cnt < Num_Robots; cnt++) {
    newvec[cnt] = Position[cnt];
    newhops[cnt] = Hop[cnt];
    newsig[cnt] = SignalStrength[cnt];
  }

  // kill the old memory
  delete []Position;
  delete []Hop;
  delete []SignalStrength;
  delete []Used;

  // and assign the new structures to the proper names
  Position = newvec;
  Hop = newhops;
  SignalStrength = newsig;
  Used = newuse;

  Num_Robots = numrobo;

}

int CJboxInterface::Count_Used(void) {
  int cnt, numused = 0;

  for (cnt = 0; cnt < MAX_JBOX_ROBOTS; cnt++) {
    if (Position[cnt].x != FLT_MAX) {
      numused++;
    }
  }
  
  return numused;
}



// package the comm sensor data in the format we have been using in missionlab
Comm_Sensor_Data CJboxInterface::Get_CommData(void) {
  int cnt, numused, currobo = 0;

  numused = Count_Used();


  Comm_Sensor_Data csd(numused);

  // now load up the data structure
  for (cnt = 0; cnt < MAX_JBOX_ROBOTS; cnt++) {

    
    if (Position[cnt].x != FLT_MAX) {
      csd.readings[currobo].id = cnt;
      csd.readings[currobo].v.x = Position[cnt].x;
      csd.readings[currobo].v.y = Position[cnt].y;
      csd.readings[currobo].v.z = Position[cnt].z;
      csd.readings[currobo].hops = Hop[cnt];
      csd.readings[currobo].signal_strength = SignalStrength[cnt];
      currobo++;
    }
  }

  return csd;
}


void CJboxInterface::Load_BaseInfo(void) {
  FILE *basein;

  basein = fopen("base_gps.txt","r");
  
  if (!basein) {
	fprintf(stderr, "Error: base_gps.txt not found!\n");
	ErrorFlag = true;
  }
  else {

	float fBase_X, fBase_Y, fBase_Lat, fBase_Long, fMPerLat, fMPerLong;

    fscanf(basein, "%f, %f", &fBase_X, &fBase_Y);
    fscanf(basein, "%f, %f", &fBase_Lat, &fBase_Long);
    fscanf(basein, "%f, %f", &fMPerLat, &fMPerLong);
    fclose(basein);

	Base_X = (double)fBase_X;
	Base_Y = (double)fBase_Y;
	Base_Lat = (double)fBase_Lat;
	Base_Long = (double)fBase_Long;
	MPerLat = (double)fMPerLat;
	MPerLong = (double)fMPerLong;
	
  }
  
}

Comm_Vector CJboxInterface::Get_Position(void) {
  int cnt;
  Comm_Vector cv;
  
  cv.x = 0;
  cv.y = 0;
  cv.z = 0;
  
  for (cnt = 0; cnt < Num_Robots; cnt++) {
    
    if ((Position[cnt].x != FLT_MAX) && (Hop[cnt] == 0)) {
      return Position[cnt];
    }
  }
  return cv;
}

/**********************************************************************
 * $Log: CJboxInterface.cpp,v $
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
