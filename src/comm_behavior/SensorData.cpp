/**********************************************************************
 **                                                                  **
 **                          SensorData.cpp                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: SensorData.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"

#include <stdio.h>
#include "SensorData.h"

SensorData::SensorData(void) {

  Error_Flag = false;
  Heading = 0;
  JBoxI = new CJboxInterface(100);
  if (!JBoxI) {
    Error_Flag = true;
  }
  if (JBoxI->Get_ErrorFlag()) {
    Error_Flag = true;
  }
}

SensorData::~SensorData(void) {
	///THIS COULD BE A PROBLEM--- CAN"T GET IT TO WORK WITH ROCI...
  //delete JBoxI;
}

bool SensorData::Get_ErrorFlag(void) {
  if (Error_Flag || JBoxI->Get_ErrorFlag()) {
    return true;
  }
  return false;
}


//These are the only two methods for updating sensor data
//arguements are restricted to primitive types
int SensorData::UpdateCommData(int id, int hops, double sig_strength, double x, double y, double z) {
	JBoxI->Update_CommData(id, hops, (float)sig_strength, (float)x, (float)y, (float)z);
	return EXIT_SUCCESS;
}

int SensorData::UpdateCommData(int id, int hops, double sig_strength, double lat, double lng) {
	JBoxI->Update_CommData(id, hops, (float)sig_strength, lat,lng);
	return EXIT_SUCCESS;
}

void SensorData::UpdateHeading( double heading )
{
	Heading = heading;
}

//used by missionlab
int SensorData::UpdateCommData(Comm_Sensor_Data *commdata) {
	JBoxI->Update_CommData(commdata);
	return EXIT_SUCCESS;
}

//These members are used by the behaviors to access relavent sensor data
double SensorData::GetHeading(void) {
  return Heading;
}

Comm_Sensor_Data SensorData::GetCommData(void) {
  return JBoxI->Get_CommData();
}

Comm_Vector SensorData::GetPosition(void) {
  return JBoxI->Get_Position();
}

/**********************************************************************
 * $Log: SensorData.cpp,v $
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
