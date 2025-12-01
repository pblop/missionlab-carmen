/**********************************************************************
 **                                                                  **
 **                            CMoveToElement.h                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

#include "stdafx.h"

#include "CMoveToElement.h"
#include "CMap.h"
#include "CCoordinate.h"
#include "CCalculateMapCosts.h"
#include "CLoadMap.h"

#define COMMUNICATIONS_CUTOFF_VALUE2 10

CMoveToElement::CMoveToElement():sElementType("MOVETO"),iLineNumbersForEdit(0)
{
    completionPoint = new CMap();  
    bElementOverride = false;
}

CMoveToElement::~CMoveToElement()
{
    delete completionPoint;
}

vector< CMap* > CMoveToElement::GetMaps() const
{
    vector< CMap* > rVector;

    rVector.push_back( completionPoint );
    rVector.push_back( contigencyPlan );
    
    return rVector;
}

VectorConfidenceAdvice CMoveToElement::getAdviceVector(CFeatureVector* cFeatureVector, int robot_id )
{
    vector< CRobotLocationCommunication> communicationList = cFeatureVector->getCommsList();
    CCoordinate cRobotPosition = cFeatureVector->GetMyCurrentLocation();

	double dGainCompletion = 1;
	double dGainContigency = 0;

    VectorConfidenceAdvice vOutput;
	
    for( int i=1; i < (iNumberOfRobots+1); i++)
    {
        if(  contigencyPlan != NULL )
        {
            if( (robot_id != i) && (communicationList[robot_id].dSignalStrength[i] < COMMUNICATIONS_CUTOFF_VALUE2) )
			{
			  //Nov 2004 Ft. Benning Demo change
#if 0
                dGainContigency = 1;
                dGainCompletion = 0;
                completionPoint = contigencyPlan;
#endif
                break;
            }
        }
    }
	
    if( contigencyPlan != NULL )
    {
        vOutput.SetX( dGainCompletion * completionPoint->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetX() + dGainContigency * contigencyPlan->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetX() );
        vOutput.SetY( dGainCompletion * completionPoint->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetY() + dGainContigency * contigencyPlan->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetY() );
    }
    else
    {   
        vOutput.SetX( dGainCompletion * completionPoint->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetX() );
        vOutput.SetY( dGainCompletion * completionPoint->getVector((double)cRobotPosition.getX(),(double)cRobotPosition.getY()).GetY() );
    }

    return vOutput;
}

CCoordinate CMoveToElement::getElementGoal() const
{
    return completionPoint->getGoal();//goal;
}

string CMoveToElement::getElementType() const
{
    return sElementType;
}

void CMoveToElement::setElementNumber( int iElNum)
{
    iElementNumber = iElNum;
}

int CMoveToElement::getElementNumber() const
{
    return iElementNumber;
}

int CMoveToElement::getLineNumbersForEdit() const
{
    return iLineNumbersForEdit;
}

int CMoveToElement::editLineNumbers( int iLineNumber )
{
//    char buffer[255];
    int iValue;
    int xValue;
    int yValue;

    string bTeth;
    string pType;

    int iField = iLineNumbersForEdit - iLineNumber;

    switch(iField)
    {
    case 2:
        cout << "Edit line: "<<iLineNumber<<" Value: " <<this->getElementNumber() <<endl;
        cout << "Enter new value: "<<endl;
        cin >> iValue;
        this->setElementNumber( iValue );
        return 1;  
    case 1:
        cout << "This line is not editable"<<endl;
        break;
    case 0:
        cout << "Edit line: "<<iLineNumber<<" Value: <" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;
        cout << "Enter X value: "<<endl;
        cin >> xValue;
        cout << "Enter Y value: "<<endl;
        cin >> yValue;
        goal.setXY( xValue, yValue );
        break;
    default:
        cout << "Error" << endl;
    }
    return 0;
}

void CMoveToElement::print(bool bNumbered, int iOutput, int& iItemNumber)
{
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Element Number:\t\t" << this->iElementNumber << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Element Type:\t\t" << this->sElementType << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Goal Location:\t\t" << "<" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;

    iLineNumbersForEdit = iItemNumber-1;
}

void CMoveToElement::writePlanToConfigFile(ofstream& fStream, int stepNumber)
{
    fStream << this->sElementType << endl;
    fStream << "#Plan step number" << endl;
    fStream << stepNumber << endl;
    fStream << "#Goal Position" << endl;
    fStream << "<" << this->goal.getX() <<"," << this->goal.getY()<<">" << endl;
}

int CMoveToElement::generateInternalizedPlanData(string mapFileName)
{
    CLoadMap clm;
    vector<CCoordinate> vBoundary;      //empty, is this how I want to do this?

    cout << "Calculating Internalized Plan: \tElement Number " << this->getElementNumber() << "\tType: "<< this->getElementType() <<endl; 
    cout << flush;

    completionPoint->setGoal( goal.getX(), goal.getY() );
    completionPoint->setMapID( "temp" );
    clm.openOVL( mapFileName, completionPoint, vBoundary);

    CCalculateMapCosts calMapCostCompletion( completionPoint );
    if( calMapCostCompletion.performCalculations() != EXIT_SUCCESS )
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}

int CMoveToElement::writeMaps(ofstream& fStream, string writeType)
{
    completionPoint->writeMapToFile( fStream, writeType ); //need return here
    return EXIT_SUCCESS;
}

int CMoveToElement::readMaps(ifstream& fStream, string writeType)
{
    CLoadMap clm;

    if( completionPoint == NULL )
        completionPoint = new CMap();

    clm.LoadMap( completionPoint, fStream, writeType );
    return EXIT_SUCCESS;
}

/**********************************************************************
 * $Log: CMoveToElement.cpp,v $
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
