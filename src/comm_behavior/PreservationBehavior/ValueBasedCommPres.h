/**********************************************************************
 **                                                                  **
 **                     ValueBasedCommPres.h					     **
 **																     **
 **                                                                  **
 **  Written by:  Matt Powers                                        **
 **                                                                  **
 **  Copyright 2002 - 2004 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This file encompasses the creation of a DLL for the     **
 **	internalized plan behavior.     								 **		
 **********************************************************************/

/* $Id: ValueBasedCommPres.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#ifndef VALUEBASEDCOMMPRES_H
#define VALUEBASEDCOMMPRES_H

//Include files
#include "VectorConfidenceAdvice.h"
#include "AbstractBehaviorInterface.h"
#include "vbcp.h"


// This class is exported from the InternalizedPlanDLL.dll
class ValueBasedCommPres:public AbstractBehaviorInterface {

	public:                             //interface
	virtual ERRNUM                  Initialize( InitializeParameters& );
	virtual ERRNUM                  Load( LoadParameters& );
	virtual VectorConfidenceAdvice  Execute(void);
	virtual ERRNUM                  UpdateSensoryData(SensorData&);
	virtual void                    Cleanup(void);
	
	/*  
 public:

  virtual ERRNUM Load();  
  virtual ERRNUM Initialize(int num_robots, int id_num);
  virtual VectorConfidenceAdvice Execute();
  virtual ERRNUM UpdateSensoryData(Comm_Sensor_Data* commdata);
  virtual void Cleanup();

*/
private:
	
	vbcp *behavior;
	
};

#endif

/**********************************************************************
 * $Log: ValueBasedCommPres.h,v $
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
