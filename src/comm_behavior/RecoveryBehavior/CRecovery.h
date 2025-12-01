/**********************************************************************
 **                                                                  **
 **                    CRecovery.h			    				     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Patrick Ulam                                       **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: CRecovery.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef CRECOVERY_H
#define CRECOVERY_H


//Preprocessor defines

//Include files
#include "VectorConfidenceAdvice.h"
#include "AbstractBehaviorInterface.h"
#include "CRecoveryBehaviors.h"

// This class is exported from the InternalizedPlanDLL.dll
class CRecovery:public AbstractBehaviorInterface {

public:                             //interface
	virtual ERRNUM                  Load( LoadParameters& );
	virtual ERRNUM                  Initialize( InitializeParameters& );
	virtual VectorConfidenceAdvice  Execute(void);
	virtual ERRNUM                  UpdateSensoryData(SensorData&);
	virtual void                    Cleanup(void);
			void 					SetSignalThresholdLow(float signalThreshold);
			void 					SetSignalThresholdHigh(float signalThreshold);

private:
	Recovery_Behaviors *RBI;


};

#endif

/**********************************************************************
 * $Log: CRecovery.h,v $
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
