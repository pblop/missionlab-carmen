/*********************************************************************
 **                                                                  **
 **                       CBehaviorCoordinator.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Interface class for behavioral coordination. This class **
 ** coordinates the output of several individual behaviors.          **
 **                                                                  **
 **********************************************************************/

/* $Id: CBehaviorCoordinator.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CBEHAVIORCOORDINATOR_H
#define CBEHAVIORCOORDINATOR_H


//Include files
#include "VectorConfidenceAdvice.h"
#include "SensorData.h"
#include "AbstractBehaviorInterface.h"
#include <vector>
#include <list>

using namespace std;

class CBehaviorCoordinator:public AbstractBehaviorInterface {

typedef enum BehaviorTypes_t {
    COMM_RECOVERY,
    COMM_PRESERVE,
    INTERNALIZED_PLAN
};

public:
	CBehaviorCoordinator();
	virtual ~CBehaviorCoordinator()             {};

public:									//interface
	virtual ERRNUM						Load( LoadParameters& );
	virtual ERRNUM						Initialize( InitializeParameters& );
	virtual VectorConfidenceAdvice		Execute(void);
	virtual ERRNUM						UpdateSensoryData( SensorData& );
	virtual void						Cleanup(void);
    void SetSignalThresholdHigh(float threshold);
    void SetSignalThresholdLow(float threshold);

    void SetAdviseMethod(int adviseMethod);


//add different methods for coordinating advice from several behaviors
private:
    VectorConfidenceAdvice				SelectHighestConfidence(list< VectorConfidenceAdvice > adviceVectors );
    VectorConfidenceAdvice				AverageBehaviorWeightConfidence(list< VectorConfidenceAdvice > adviceVectors );
    VectorConfidenceAdvice              SelectPickedBehaviorType(list< VectorConfidenceAdvice > adviceVectors, int behaviorType );

private:
	SensorData							currentSensoryData;
    vector<AbstractBehaviorInterface*>	m_hBehaviors;
    vector<int> m_hBehaviorTypes;
    int m_iAdviseMethod;
};

inline void CBehaviorCoordinator::SetAdviseMethod(int adviseMethod)
{
    m_iAdviseMethod = adviseMethod;
}

#endif

/**********************************************************************
 * $Log: CBehaviorCoordinator.h,v $
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
