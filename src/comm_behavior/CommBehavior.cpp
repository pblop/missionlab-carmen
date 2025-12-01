/**********************************************************************
 **                                                                  **
 **                         CommBehavior.cpp                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CommBehavior.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

// Defines the entry point for the DLL application.

#include "stdafx.h"
#include "CommBehavior.h"

#ifdef WINDOWS
BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}
#endif

COMMBEHAVIOR_API CommBehavior *createCommBehavior(void)
{
	return new CommBehavior();
}

COMMBEHAVIOR_API ERRNUM Initialize(CommBehavior *hDLL,	int iNumberOrRobots, int iRobotID)
{
	return hDLL->Initialize(iNumberOrRobots, iRobotID);
}

COMMBEHAVIOR_API ERRNUM Load(CommBehavior *hDLL, int ldParam)
{
	return hDLL->Load(ldParam);
}

COMMBEHAVIOR_API ERRNUM UpdateSensoryData(CommBehavior *hDLL, SensorDataUMWrap* updParam)
{
	return hDLL->UpdateSensoryData( updParam->GetSensorData() );	//we don't mind passing copies of data
}

COMMBEHAVIOR_API CVectorConfidenceAdviceUMWrap* Execute(CommBehavior *hDLL)
{
	return new CVectorConfidenceAdviceUMWrap( hDLL->Execute() );
}

COMMBEHAVIOR_API void SetSignalThresholdLow(CommBehavior *hDLL, float signalThreshold)
{
	hDLL->SetSignalThresholdLow(signalThreshold);
}

COMMBEHAVIOR_API void SetSignalThresholdHigh(CommBehavior *hDLL, float signalThreshold)
{
	hDLL->SetSignalThresholdHigh(signalThreshold);
}

COMMBEHAVIOR_API void Cleanup(CommBehavior *hDLL)
{
	hDLL->Cleanup();
	return;
}

COMMBEHAVIOR_API void SetAdviseMethod(CommBehavior *hDLL, int adviseMethod)
{
	hDLL->SetAdviseMethod(adviseMethod);
	return;
}

CommBehavior::CommBehavior()
{
	pBehaviorCoordinator = new CBehaviorCoordinator();
	return;
}

int CommBehavior::Initialize(int iNumberOrRobots, int iRobotID )
{
	InitializeParameters ilParams;
	ilParams.iInitialNumberOfRobots = iNumberOrRobots;
	ilParams.iRobotID = iRobotID;

	return pBehaviorCoordinator->Initialize(ilParams);
}

int CommBehavior::Load(int ilParams)
{
	return pBehaviorCoordinator->Load(ilParams);
}

int CommBehavior::UpdateSensoryData(SensorData updParam)
{
	return pBehaviorCoordinator->UpdateSensoryData(updParam);
}

VectorConfidenceAdvice CommBehavior::Execute()
{
	return pBehaviorCoordinator->Execute();
}

void CommBehavior::SetSignalThresholdLow(float threshold)
{
	pBehaviorCoordinator->SetSignalThresholdLow(threshold);
}

void CommBehavior::SetSignalThresholdHigh(float threshold)
{
	pBehaviorCoordinator->SetSignalThresholdHigh(threshold);
}

void CommBehavior::Cleanup(void)
{
	pBehaviorCoordinator->Cleanup();
	return;
}

void CommBehavior::SetAdviseMethod(int adviseMethod)
{
	pBehaviorCoordinator->SetAdviseMethod(adviseMethod);
	return;
}

//Vector advice output...
COMMBEHAVIOR_API double GetX(CVectorConfidenceAdviceUMWrap* hDLL)
{
	return hDLL->GetX();
}

COMMBEHAVIOR_API double GetY(CVectorConfidenceAdviceUMWrap* hDLL)
{
	return hDLL->GetY();
}

COMMBEHAVIOR_API void GetPolar(CVectorConfidenceAdviceUMWrap* hDLL, double& dMagnitude, double& dAngle)
{
	hDLL->GetPolar(dMagnitude, dAngle);
	return;
}

COMMBEHAVIOR_API double GetConfidence(CVectorConfidenceAdviceUMWrap* hDLL)
{
	return hDLL->GetConfidence();
}

// This is the constructor of a class that has been exported.
// see VectorConfidenceAdviceUMWrap.h for the class definition
CVectorConfidenceAdviceUMWrap::CVectorConfidenceAdviceUMWrap(VectorConfidenceAdvice new_vector)
{
    vector = new_vector;
	return;
}

double CVectorConfidenceAdviceUMWrap::GetX() const
{
	return vector.GetX();
}

double CVectorConfidenceAdviceUMWrap::GetY() const
{
	return vector.GetY();
}

double CVectorConfidenceAdviceUMWrap::GetConfidence() const
{
    return vector.GetConfidence();
}

void CVectorConfidenceAdviceUMWrap::GetPolar( double& dMagnitude, double& dAngle )
{
    vector.GetPolar(dMagnitude, dAngle);
    return;
}

COMMBEHAVIOR_API SensorDataUMWrap* createSensorDataUMWrap()
{
	return new SensorDataUMWrap();
}

COMMBEHAVIOR_API void SetSensorCommData(SensorDataUMWrap* hSensor, int id, int hops, double sig_strength, double heading, double x, double y, double z)
{
	hSensor->SetSensorCommData( id, hops, sig_strength, x,  y, z);
    if( hops == 0 )
	{
        hSensor->SetHeading( heading );
	}
	return;
}
COMMBEHAVIOR_API void SetSensorCommDataTwo(SensorDataUMWrap* hSensor, int id, int hops, double sig_strength, double heading, double lat, double lng)
{
	hSensor->SetSensorCommData( id, hops, sig_strength, lat, lng);
	if( hops == 0 )
	{
        hSensor->SetHeading( heading );
	}
	return;
}

void SensorDataUMWrap::SetSensorCommData(int iRobotID, int hops, double dSignalStrength, double x, double y, double z)
{
	dataObject.UpdateCommData(iRobotID, hops, dSignalStrength, x, y, z);
	return;
}

void SensorDataUMWrap::SetSensorCommData(int iRobotID, int hops, double dSignalStrength, double lat, double lng)
{
	dataObject.UpdateCommData(iRobotID, hops, dSignalStrength, lat, lng);
    return;
}

void SensorDataUMWrap::SetHeading(double dHeading)
{
	dataObject.UpdateHeading(dHeading);
    return;
}
SensorData SensorDataUMWrap::GetSensorData() const
{
	return dataObject;
}

/**********************************************************************
 * $Log: CommBehavior.cpp,v $
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
 * Revision 1.5  2004/09/15 06:27:30  endo
 * Functions related to SetAdviseMethod added.
 *
 **********************************************************************/

