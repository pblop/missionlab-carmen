/**********************************************************************
 **                                                                  **
 **                     CRecoveryBehaviors.h			    		 **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CRecoveryBehaviors.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef RECOVERY_H
#define RECOVERY_H

#include "comm_data.h"
#include "SensorData.h"
#include "CWaypointstack.h"


enum Comm_Stats {
	CONNECTED,
	BROKEN
};


class Recovery_Behaviors {
public:
	Recovery_Behaviors();
	~Recovery_Behaviors(void);
	double Get_Confidence(void);
	Comm_Vector Get_Movement(void);
	void UpdateSensorDataPointer(SensorData& pSensor);
	void SetSignalThresholdLow(float signalThreshold);
	void SetSignalThresholdHigh(float signalThreshold);

private:
	void Load_Configuration(void);

	double Len2d(Comm_Vector v);
	Comm_Vector Rotate_Z(Comm_Vector v, double theta);
	void Unit2d(Comm_Vector *v);

	Comm_Vector Get_NearestNeighbor(void);
	Comm_Vector Get_Retrotraverse(void);
	Comm_Vector Get_Probe(void);

	int Num_Robots;
	int Timeout;
	int Retro_Priority;
	int Near_Priority;
	int RetroNear_Priority;
	int Waypoint_Interval;
	int Waypoint_Stop;

	int Robot_Id;
	double Heading;
	Comm_Vector Position;
	double Confidence;
	Comm_Stats *Timeout_List;
	long *Tolerance_List;

	Comm_Sensor_Data QosData;
	int CurSequence;
	int Sequence_Time;
	long CurSequence_Time;
	int Team_Recovery;
	int Solo_Recovery;

	SensorData *SI;
	WaypointStack *Waypoint_Stack;

	float SIGSTR_THRESH_L;
	float SIGSTR_THRESH_H;
};


#endif

/**********************************************************************
 * $Log: CRecoveryBehaviors.h,v $
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
