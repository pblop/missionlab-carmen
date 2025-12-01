/**********************************************************************
 **                                                                  **
 **                     CInternalizedPlan.cpp					     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006      Georgia Tech Research Corporation    **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This file encompasses the creation of a DLL for the     **
 **	internalized plan behavior.     								 **		
 **********************************************************************/

/* $Id: CInternalizedPlan.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include "stdafx.h"
#include "CInternalizedPlan.h"

/***********

Initialize the internalized plan behavior. The member InitializeParameters is not current used.

  *********/
ERRNUM CInternalizedPlan::Initialize(InitializeParameters& inParam)
{
	if( aPlan != NULL )
    { 
        aPlan = NULL;
    }

    aPlan = new CPlan(); 
    
	aPlan->SetPlanData( inParam.iInitialNumberOfRobots, inParam.iRobotID );
    return EXIT_SUCCESS;
}

/***********
Load the internalized plan behavior. This behavior can take a bit of time.
  *********/
ERRNUM CInternalizedPlan::Load( LoadParameters& refParameters)
{
    return aPlan->LoadPlanFromDataFile();
}

ERRNUM CInternalizedPlan::UpdateSensoryData( SensorData& refParameters)
{
    return aPlan->UpdateFeatureVector( refParameters );
}

VectorConfidenceAdvice CInternalizedPlan::Execute()
{
	return aPlan->GetAdvice();
}

void CInternalizedPlan::Cleanup(void)
{
    delete aPlan;
}

/**********************************************************************
 * $Log: CInternalizedPlan.cpp,v $
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
