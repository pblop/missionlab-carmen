/**********************************************************************
 **                                                                  **
 **                    ValueBasedCommPres.cpp					     **
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

/* $Id: ValueBasedCommPres.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"

#include "ValueBasedCommPres.h"
#include "vbcp.h"


/***********

Initialize the internalized plan behavior. The member InitializeParameters is not current used.

  *********/

ERRNUM ValueBasedCommPres::Initialize(InitializeParameters& inParam)
{
	int num_robots = inParam.iInitialNumberOfRobots;
	int id_num = inParam.iRobotID;
	if( behavior != NULL )
    { 
        behavior = NULL;
    }
	behavior = new vbcp(num_robots, id_num);
    return EXIT_SUCCESS;
}



/***********
Load the internalized plan behavior. This behavior can take a bit of time.
  *********/

ERRNUM ValueBasedCommPres::Load(LoadParameters& ldParam)
{
  return EXIT_SUCCESS;
}


void ValueBasedCommPres::Cleanup(void)
{
    delete behavior;
}


ERRNUM ValueBasedCommPres::UpdateSensoryData(SensorData& sensorData)
{

	Comm_Sensor_Data commdata = sensorData.GetCommData();
	double heading = sensorData.GetHeading();
	behavior->NetStateUpdate(&commdata, heading);

	return EXIT_SUCCESS;

}


VectorConfidenceAdvice ValueBasedCommPres::Execute(void)
{

  VectorM vec = behavior->getSuggestedVector()->value();
  double conf = behavior->getConfidence();
  
  VectorConfidenceAdvice advice = VectorConfidenceAdvice(vec.getX(), vec.getY(), conf);
  
  return advice;
}

/**********************************************************************
 * $Log: ValueBasedCommPres.cpp,v $
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
