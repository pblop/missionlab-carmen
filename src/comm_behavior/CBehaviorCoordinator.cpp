/*********************************************************************
 **                                                                  **
 **                     CBehaviorCoordinator.cpp                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002-2006 Georgia Tech Research Corporation           **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This file encompasses the creation of a DLL for the     **
 **	internalized plan behavior.     								 **
 **********************************************************************/

/* $Id: CBehaviorCoordinator.cpp,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

#include <list>
#include <vector>
#include <algorithm>
#include <numeric>

#include "stdafx.h"
#include "CBehaviorCoordinator.h"
#include "CInternalizedPlan.h"
#include "CRecovery.h"
#include "ValueBasedCommPres.h"
#include "CommBehaviorAdviseMethod.h"


// This is the constructor of a class that has been exported.
// see BehaviorCoordinatorDLL.h for the class definition

CBehaviorCoordinator::CBehaviorCoordinator(void) :
    m_iAdviseMethod(CSB_ADVISE_USE_COMM_RECOVERY)
{

    // Communication Recovery Behavior Suite
	m_hBehaviors.push_back( new CRecovery() );
	m_hBehaviorTypes.push_back(COMM_RECOVERY);

    //Planning Behavior Suite
//	m_hBehaviors.push_back( new CInternalizedPlan() );
//	m_hBehaviorTypes.push_back(INTERNALIZED_PLAN);

    //Communication Preservation Behavior Suite
//    m_hBehaviors.push_back( new ValueBasedCommPres() );
//	m_hBehaviorTypes.push_back(COMM_PRESERVE);

	return;
}


ERRNUM CBehaviorCoordinator::Initialize(InitializeParameters& refParameters)
{
    //Initialize all of the Behaviors
	ERRNUM errNum=0;
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
		errNum += m_hBehaviors[i]->Initialize( refParameters );
    }

    return errNum;
}


ERRNUM CBehaviorCoordinator::Load( LoadParameters& refParameters)
{
	ERRNUM errNum=0;
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
        errNum += m_hBehaviors[i]->Load( refParameters );
    }

    return errNum;
}

ERRNUM CBehaviorCoordinator::UpdateSensoryData( SensorData& refParameters)
{
	ERRNUM errNum=0;
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
		errNum += m_hBehaviors[i]->UpdateSensoryData( refParameters );
    }

    return errNum;
}

void CBehaviorCoordinator::Cleanup(void)
{
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
		m_hBehaviors[i]->Cleanup();
    }

    return;
}

void CBehaviorCoordinator::SetSignalThresholdLow(float threshold)
{
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
    	m_hBehaviors[i]->SetSignalThresholdLow(threshold);
    }
}

void CBehaviorCoordinator::SetSignalThresholdHigh(float threshold)
{
    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
    	m_hBehaviors[i]->SetSignalThresholdHigh(threshold);
    }}

VectorConfidenceAdvice CBehaviorCoordinator::Execute(void)
{
    VectorConfidenceAdvice rValue;
    list< VectorConfidenceAdvice > listOfVectors;

    for(int i=0; i<(signed)m_hBehaviors.size(); i++ )
    {
        listOfVectors.push_back( m_hBehaviors[i]->Execute() );
    }

    switch (m_iAdviseMethod) {

    case CSB_ADVISE_GREEDY:
        rValue = SelectHighestConfidence( listOfVectors );
        break;

    case CSB_ADVISE_AVERAGE:
        rValue = AverageBehaviorWeightConfidence( listOfVectors );
        break;

    case CSB_ADVISE_USE_COMM_RECOVERY:
        rValue = SelectPickedBehaviorType( listOfVectors, COMM_RECOVERY );
        break;

//    case CSB_ADVISE_USE_COMM_PRESERVE:
//        rValue = SelectPickedBehaviorType( listOfVectors, COMM_PRESERVE );
//        break;

//    case CSB_ADVISE_USE_INTERNALIZED_PLAN:
//        rValue = SelectPickedBehaviorType( listOfVectors, INTERNALIZED_PLAN );
//        break;
    }

    return rValue;
}

VectorConfidenceAdvice CBehaviorCoordinator::SelectHighestConfidence(list< VectorConfidenceAdvice > adviceVectors )
{
    double iLocalConfidence=-1;
    VectorConfidenceAdvice rValue;
    std::list< VectorConfidenceAdvice >::iterator adviceIterator;

    for(adviceIterator = adviceVectors.begin(); adviceIterator!= adviceVectors.end(); adviceIterator++)
    {
        if( (*adviceIterator).GetConfidence() > iLocalConfidence )
        {
            iLocalConfidence = (*adviceIterator).GetConfidence();
            rValue = (*adviceIterator);
        }
    }
    return rValue;
}

VectorConfidenceAdvice CBehaviorCoordinator::AverageBehaviorWeightConfidence(list< VectorConfidenceAdvice > adviceVectors )
{
    VectorConfidenceAdvice rValue;
    double sumX=0;
    double sumY=0;

    if( adviceVectors.size() == 0 )
        return rValue;

    std::list< VectorConfidenceAdvice >::iterator adviceIterator;
    for(adviceIterator = adviceVectors.begin(); adviceIterator!= adviceVectors.end(); adviceIterator++)
    {
        sumX += (*adviceIterator).GetConfidence()*(*adviceIterator).GetX();
        sumY += (*adviceIterator).GetConfidence()*(*adviceIterator).GetY();
    }

    rValue.SetXY( sumX/adviceVectors.size(), sumY/adviceVectors.size() );
    return rValue;
}

VectorConfidenceAdvice CBehaviorCoordinator::SelectPickedBehaviorType(list< VectorConfidenceAdvice > adviceVectors, int behaviorType )
{
    std::list< VectorConfidenceAdvice >::iterator adviceIterator;
    VectorConfidenceAdvice rValue;
    int index;

    // Note: Assuming that the order of the behaviors stored in m_hBehaviorTypes is
    // same as the ones stored in adviceVectors.

    index = 0;
    for(adviceIterator = adviceVectors.begin(); adviceIterator!= adviceVectors.end(); adviceIterator++)
    {
        if (m_hBehaviorTypes[index] == behaviorType)
        {
            rValue = (*adviceIterator);
            break;
        }

        index++;
    }

    return rValue;
}

/**********************************************************************
 * $Log: CBehaviorCoordinator.cpp,v $
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
 * Revision 1.5  2004/09/18 03:55:39  endo
 * *** empty log message ***
 *
 * Revision 1.4  2004/09/16 16:40:28  endo
 * *** empty log message ***
 *
 * Revision 1.3  2004/09/15 06:27:30  endo
 * Functions related to SetAdviseMethod added.
 *
 **********************************************************************/
