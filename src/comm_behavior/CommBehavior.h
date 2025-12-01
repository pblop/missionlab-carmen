/*********************************************************************
 **                                                                  **
 **                            CommBehavior.h                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002-2004 Georgia Tech Research Corporation           **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CommBehavior.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef COMM_BEHAVIOR_H
#define COMM_BEHAVIOR_H

// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the COMMBEHAVIOR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// COMMBEHAVIOR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef WINDOWS
#ifdef COMMBEHAVIOR_EXPORTS
#define COMMBEHAVIOR_API __declspec(dllexport)
#else
#define COMMBEHAVIOR_API __declspec(dllimport)
#endif
#else
#define COMMBEHAVIOR_API
#endif

// This class is exported from the GTechBehaviorDLL.dll
#include "CBehaviorCoordinator.h"
#include "VectorConfidenceAdvice.h"
#include "SensorData.h"

class CommBehavior {

public:
	CommBehavior(void);

	ERRNUM Initialize(int iNumberOfRobots, int iRobotID);
	int Load(int ldParam);
	VectorConfidenceAdvice Execute();
	int UpdateSensoryData(SensorData updParam);
	void Cleanup(void);
    void SetAdviseMethod(int adviseMethod);
    void SetSignalThresholdHigh(float threshold);
    void SetSignalThresholdLow(float threshold);

private:
	CBehaviorCoordinator* pBehaviorCoordinator;
};

// This class is exported from the VectorConfidenceAdviceUMWrap.dll
class CVectorConfidenceAdviceUMWrap {
public:
	CVectorConfidenceAdviceUMWrap(VectorConfidenceAdvice);
	double  GetX() const;
	double  GetY() const;
	double  GetConfidence() const;
	void    GetPolar(double& dMagnitude,double& dAngle);

private:
    VectorConfidenceAdvice vector;
};

// This class is exported from the VectorConfidenceAdviceUMWrap.dll
class SensorDataUMWrap {
public:
	void	SetSensorCommData(int id, int hops, double sig_strength, double x, double y, double z);
    void	SetSensorCommData(int id, int hops, double sig_strength, double lat, double lng);
    void	SetHeading(double heading);
	SensorData GetSensorData() const;

private:
	SensorData dataObject;
};

// Behavior
extern "C" COMMBEHAVIOR_API		CommBehavior *createCommBehavior(void);
extern "C" COMMBEHAVIOR_API		ERRNUM	Initialize(CommBehavior *, int iNumberOfRobots, int iRobotID);
extern "C" COMMBEHAVIOR_API		int		Load(CommBehavior *, int ldParam);
extern "C" COMMBEHAVIOR_API		CVectorConfidenceAdviceUMWrap* Execute(CommBehavior *);
extern "C" COMMBEHAVIOR_API		int		UpdateSensoryData(CommBehavior *, SensorDataUMWrap* );
extern "C" COMMBEHAVIOR_API		void	Cleanup(CommBehavior *);
extern "C" COMMBEHAVIOR_API		void    SetAdviseMethod(CommBehavior *hDLL, int adviseMethod);
extern "C" COMMBEHAVIOR_API 	void SetSignalThresholdLow(CommBehavior *hDLL, float signalThreshold);
extern "C" COMMBEHAVIOR_API 	void SetSignalThresholdHigh(CommBehavior *hDLL, float signalThreshold);

//data members
extern "C" COMMBEHAVIOR_API     double  GetX(CVectorConfidenceAdviceUMWrap *);
extern "C" COMMBEHAVIOR_API     double  GetY(CVectorConfidenceAdviceUMWrap *);
extern "C" COMMBEHAVIOR_API     double  GetConfidence(CVectorConfidenceAdviceUMWrap *);
extern "C" COMMBEHAVIOR_API     void    GetPolar(CVectorConfidenceAdviceUMWrap *,double dMagnitude,double dAngle);

//sensor updates
extern "C" COMMBEHAVIOR_API     SensorDataUMWrap*	createSensorDataUMWrap();
extern "C" COMMBEHAVIOR_API     void				SetSensorCommData(SensorDataUMWrap*, int id, int hops, double sig_strength, double heading, double x, double y, double z);
extern "C" COMMBEHAVIOR_API     void				SetSensorCommDataTwo(SensorDataUMWrap*, int id, int hops, double sig_strength, double heading, double lat, double lng);

#endif

/**********************************************************************
 * $Log: CommBehavior.h,v $
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
 * Revision 1.4  2004/09/15 06:27:30  endo
 * Functions related to SetAdviseMethod added.
 *
 **********************************************************************/

