/*********************************************************************
 **                                                                  **
 **                    CRecovery.cpp							     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This file encompasses the creation of a DLL for the     **
 **	recovery behaviours			.     								 **
 **********************************************************************/

#include "stdafx.h"

#include "CRecovery.h"
#include "CRecoveryBehaviors.h"

/***********

Initialize the internalized plan behavior. The member InitializeParameters is not current used.

  *********/
ERRNUM CRecovery::Initialize(InitializeParameters& refParameters)
{
  if( RBI != NULL )
    {
      RBI = NULL;
    }

  RBI = new Recovery_Behaviors();
  return EXIT_SUCCESS;

}

/***********
Load the internalized plan behavior. This behavior can take a bit of time.
  *********/
ERRNUM CRecovery::Load( LoadParameters& refParameters)
{

  return EXIT_SUCCESS;
}

void CRecovery::Cleanup(void)
{
  delete RBI;
}

ERRNUM CRecovery::UpdateSensoryData( SensorData& refParameters)
{
  RBI->UpdateSensorDataPointer(refParameters);
  return EXIT_SUCCESS;
}

VectorConfidenceAdvice CRecovery::Execute()
{

  VectorConfidenceAdvice adv;

  Comm_Vector cv;

  cv = RBI->Get_Movement();

  adv.SetXY(cv.x, cv.y);
  adv.SetConfidence(RBI->Get_Confidence());

  return adv;

}

void CRecovery::SetSignalThresholdLow(float signalThreshold)
{
	RBI->SetSignalThresholdLow(signalThreshold);
}

void CRecovery::SetSignalThresholdHigh(float signalThreshold)
{
	RBI->SetSignalThresholdHigh(signalThreshold);
}




/**********************************************************************
 * $Log: CRecovery.cpp,v $
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
