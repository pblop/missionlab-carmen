/*********************************************************************
 **                                                                  **
 **                    CPlan.cpp							         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Controls planning for teams of robots.                  **
 **	                             									 **		
 **********************************************************************/

#include "stdafx.h"

#include "CPlan.h"
#include "CMoveToElement.h"
#include "CReconBuildingElement.h"
#include "CCalculateMapCosts.h"
#include "CLoadMap.h"


#define OFFSET 1


CPlan::CPlan()  
{
    currentElement = NULL;
    iPlanStepIndex = 0;
    iPlanSize = 0;
}

CPlan::~CPlan()
{
    delete planContigencyMap;

    if( vPairsOfElements.size() != 0 )
    {
        for(unsigned int i=0; i<vPairsOfElements.size(); i++)
        {
            delete vPairsOfElements[i]->firstElement;
            delete vPairsOfElements[i]->secondElement;
        }
    }

    if( planSteps.size() != 0 )
    {
        for(unsigned int i=0; i<planSteps.size(); i++)
        {
            delete planSteps[i];
        }
    }
}

CReactivePlanningElement* CPlan::GetPlanStep(int i)
{
    return planSteps[i];
}

void CPlan::SetPlanData( int iNumberUsingPlan, int iID)
{
    try
	{
        SetThisRobotsIDNumber( iID ); 
	    SetNumberOfRobotsUsingThisPlan( iNumberUsingPlan );
	    currentSensoryData = new CFeatureVector( iNumberUsingPlan, iID );
	
	    list< int > mapping;
	
	    for(int i=1;i<( iNumberUsingPlan+1);i++)
	    {
	        mapping.push_back( i );
	    }
	    currentSensoryData->CreateMapping( mapping );
	}
	catch( ... ) //TO DO add exception class
	{}
}

void CPlan::SetNumberOfRobotsUsingThisPlan( int iNumberUsingPlan )
{
    iNumberOfRobots = iNumberUsingPlan;
    unsigned int i;
    for( i=0; i<planSteps.size(); i++)
    {
        planSteps[i]->SetNumberOfRobots( iNumberUsingPlan );
    }
}


int CPlan::UpdateFeatureVector( SensorData& sensorData )
{	
    try
	{
	    currentSensoryData->update( sensorData );
	}
	catch(...) //ADD exception class here
	{
	}
	return EXIT_SUCCESS;
}

void CPlan::GroupElementsIntoPairs()
{
    if( vPairsOfElements.size() != 0 )
        vPairsOfElements.clear();

	ElementPair* aPair;

    unsigned int i;
	for( i=0; i<planSteps.size();)
    {
        aPair = new ElementPair();
        aPair->firstElement = planSteps[i];
        aPair->secondElement = planSteps[i+1];
        vPairsOfElements.push_back( aPair );
        i = i+2;
    }
    iPlanSize = vPairsOfElements.size();
}

ElementPair* CPlan::RetrieveElementOfLeastDistance()
{
    double distance = GTIP_INFINITY;
    vector< ElementPair* >::iterator elementIter;
    vector< ElementPair* >::iterator deleteIter;
    ElementPair* rValue = NULL;
    CCoordinate cRobotPosition = GetFeatureVector()->GetMyCurrentLocation();

	for( elementIter = vPairsOfElements.begin(); elementIter != vPairsOfElements.end(); elementIter++ )
	{
        CCoordinate eGoal = (*elementIter)->firstElement->getElementGoal();
        int eMinusLocalX = cRobotPosition.getX() - eGoal.getX();
        int eMinusLocalY = cRobotPosition.getY() - eGoal.getY();
	
        double dDistanceValue =  sqrt( pow((double)eMinusLocalX, 2) + pow((double)eMinusLocalY, 2) );

        if(  dDistanceValue <  distance )
        { 
            distance = dDistanceValue;
            rValue = (*elementIter);
            deleteIter = elementIter;
        }
    }
	
    return rValue;
}

VectorConfidenceAdvice CPlan::GetAdvice()
{
    VectorConfidenceAdvice rValue;

    // Note: It is just returning an empty advise to avoid an unexpected
    // segfault during the demo.
    return rValue;
	
    /*
	if( iNumberOfRobots <2 || iNumberOfRobots > 4 )
	{
	  //cout << "Number of Robots: " << iNumberOfRobots << endl;
	  return rValue;
	}
    */

	rValue.SetConfidence( 1 );
    try
	{
		if( currentElement == NULL )      //retrieve the first step in the plan
		{ 		  
			if( planType == ORDERED )
			{
				currentElement = planSteps[iPlanStepIndex];
			}
			else if( planType == LEAST_COMMITMENT )
			{   
				this->GroupElementsIntoPairs();
				currentPair = RetrieveElementOfLeastDistance();
				currentElement = currentPair->firstElement;
			}
		}

		if( planType == ORDERED )
		{
			if( (CheckIfElementIsComplete() == true ) )
			{	
#ifdef MISSIONLAB_SIMULATION
			    sleep( 1 );
#endif
			     
			  iPlanStepIndex++;                
			   
				if( iPlanStepIndex == (signed)iPlanSize )
					return rValue;
				currentElement = planSteps[iPlanStepIndex];
				//cout << "Setting new Element: " <<currentElement->getElementNumber() << endl;
			}
			else if(CheckIfAtElementGoal() == true)
			{
			    return rValue;
			}
		}
		else if(planType ==  LEAST_COMMITMENT)
		{
			if( (CheckIfElementIsComplete() == true ) )
			{
				if( currentElement->getElementType() == "MOVE_TO_ELEMENT" )
				{	 
					currentElement = currentPair->secondElement;
				}
				else
				{ 
					if( vPairsOfElements.size() == 0 )
						return rValue;
					else
					{ 
						currentPair = RetrieveElementOfLeastDistance();
						currentElement = currentPair->firstElement;
					}
				}//end current element  
	   
			}
			else if(CheckIfAtElementGoal() == true)
			{
			    return rValue;
			}
		}
		rValue = currentElement->getAdviceVector(GetFeatureVector(), GetThisRobotsIDNumber() );
	}
    catch (...) //ADD Exception class here
	{
    }
    rValue.SetConfidence( 0 );

	//Make egoCentric 
	RotateEgoCentric( rValue, -1*currentSensoryData->GetHeading());

	return rValue;
}

bool CPlan::CheckIfNearLocation( CCoordinate currentLocation, CCoordinate goalLocation, double dAcceptableDistance )
{ 
    double dDistanceValue =  sqrt( pow((double)(goalLocation.getX() - currentLocation.getX()), 2) + pow((double)(goalLocation.getY() - currentLocation.getY()), 2) );
    if( dDistanceValue < dAcceptableDistance)
	  return true;
    else
        return false;
}

bool CPlan::CheckIfAtElementGoal() 
{ 
    vector< CRobotLocationCommunication> commsList = (GetFeatureVector())->getCommsList();
    if( CheckIfNearLocation(commsList[iRobotID].cLocation,currentElement->getElementGoal(),OFFSET) == false )
	    return false;  
	else
	    return true;
}

bool CPlan::CheckIfElementIsComplete()
{
    vector< CRobotLocationCommunication> commsList = (GetFeatureVector())->getCommsList();
    
    for(int i=1; i<(iNumberOfRobots+1);i++)
    {   
        if(bTethered == true)
        {
            if( i != 2 )    //need something here!!!
            {
                if( (CheckIfNearLocation(commsList[i].cLocation,currentElement->getElementGoal(),OFFSET) == false ) && 
				    ( currentElement->GetElementOverride() == false ) )																						    
				  return false; 
            }
        }
        else
        {
            if( ( CheckIfNearLocation(commsList[i].cLocation,currentElement->getElementGoal(),OFFSET) == false ) && 
			    ( currentElement-> GetElementOverride() == false ) )																						    
			  return false;  
        }
    }
    
    //cout << "Element Is complete " << iRobotID << endl;  
	
    return true;
}

void CPlan::editLineNumber( int iLineNumber )
{
    char buffer[255];
    int iValue;
    bool bValue;
    double xValue;
    double yValue;

    string bTeth;
    string pType;

    if( iLineNumber < iLineNumbersForEdit )
    {
        switch(iLineNumber)
        {
        case 1:
            cout << "Edit line: "<<iLineNumber<<" Value: " <<this->GetPlanID() <<endl;
            cout << "Enter new value: "<<endl;
            cin >> buffer;
            this->SetPlanID( (string)buffer);
            break;
        case 2:
            cout << "Edit line: "<<iLineNumber<<" Value: " <<this->GetMapFileName() <<endl;
            cout << "Enter new value: "<<endl;
            cin >> buffer;
            this->SetMapFileName( (string)buffer );
            break;
        case 3:
            if(this->GetPlanType() == ORDERED)
                pType = "ORDERED";
            else
                pType = "LEASE_COMMITMENT";
            cout << "Edit line: "<<iLineNumber<<" Value: " <<pType <<endl;
            cout << "Enter 1 for ORDERED 2 for LEAST_COMMITMENT: "<<endl;
            cin >> iValue;
            if( iValue == 1)
                this->SetPlanType( ORDERED );
            else
                this->SetPlanType( LEAST_COMMITMENT );
            break;
        case 4:
            if(this->GetTethered() == true)
                bTeth = "TRUE";
            else
                bTeth = "FALSE";
            cout << "Edit line: "<<iLineNumber<<" Value: " <<this->GetTethered() <<endl;
            cout << "Enter 1 for true 2 for false: "<<endl;
            cin >> bValue;
            this->SetTethered( bValue );
            break;
        case 5:
            cout << "Edit line: "<<iLineNumber<<" Value: <" << this->GetContigencyLocation().getX() <<"," << this->GetContigencyLocation().getY()<<">" << endl;
            cout << "Enter X value: "<<endl;
            cin >> xValue;
            cout << "Enter Y value: "<<endl;
            cin >> yValue;
            contingencyLoc.setXY( (int)xValue, (int)yValue );
            break;
        case 6:
            cout << "Line 5 is not editable" << endl;
            break;
        default:
            cout << "Error" << endl;
        }
    }
    else
    {
        int i;
        for(i=0; i< (signed)planSteps.size(); i++)
        {
            if( iLineNumber <= planSteps[i]->getLineNumbersForEdit() )
            {
                if( planSteps[i]->editLineNumbers( iLineNumber ) == 1)
                {
                    swap(planSteps[i], planSteps[planSteps[i]->getElementNumber()-1]);
                    return;
                }
                
            }
        }
    }
}


int CPlan::writePlanToConfigFile(string filename)
{
    ofstream fStream( filename.c_str() );

    if( fStream.is_open() == false )
    {
        cerr << "ERROR: File "<< filename << " could not be opened"<<endl; 
        return -1;
    }

    fStream << "#This file stores configuration data for a plan." << endl;
    fStream << "#The data is printed in a specific format to aid reconstruction of data objects."<< endl;
    fStream << endl;
    fStream << "BEGIN_PLAN" << endl;
    fStream << "#Plan ID" << endl;
    fStream << this->GetPlanID() << endl;
    fStream << "#Map File Name" << endl;
    fStream << this->GetMapFileName() << endl;
    fStream << "#Element Ordering" << endl;
    if( this->GetPlanType() == ORDERED )
        fStream << "ORDERED" << endl;
    else
        fStream << "LEAST-COMMITMENT" << endl;

    fStream << "#Tethered" << endl;
    if(this->GetTethered() == true)
        fStream << "TRUE" << endl;
    else
        fStream << "FALSE" << endl;

    fStream << "#Plan contigency location" << endl;
    fStream << "<" << this->GetContigencyLocation().getX() <<"," << this->GetContigencyLocation().getY()<<">" << endl;
    fStream << "#Number of Elements" << endl;
    fStream << (unsigned int) this->planSteps.size() << endl;
    fStream << "#########	Begin element specific information" << endl;

    unsigned int i;
    for(i=0; i< planSteps.size(); i++)
    {
        fStream << "#########	Begin element" << endl;
        fStream << "#Element "<< i+1 << " Type" << endl;
        planSteps[i]->writePlanToConfigFile( fStream, i+1);
    }

    return EXIT_SUCCESS;
}

void CPlan::print(bool bNumbered, int iOutput)
{
    int iItemNumber =1;

    cout << endl;
    cout << endl;
    cout << "Plan Information"<<endl;
    cout << endl;
    cout << "Item Number \t Item Name \t\t Item Value" << endl; 
    cout << "-----------------------------------------------------------------------" <<endl;
    
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Plan ID:\t\t" << this->GetPlanID() << endl;

    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Map File Name:\t\t" << this->GetMapFileName() << endl;

    string pType;
    if(this->GetPlanType() == ORDERED)
        pType = "ORDERED";
    else
        pType = "LEASE_COMMITMENT";
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Plan Type:\t\t" << pType << endl;

    string bTeth;
    if(this->GetTethered() == true)
        bTeth = "TRUE";
    else
        bTeth = "FALSE";
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Tethered:\t\t" << bTeth << endl;
    
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Contingency Location:\t\t" << "<" << this->GetContigencyLocation().getX() <<"," << this->GetContigencyLocation().getY()<<">" << endl;
    
    if( bNumbered == true )     {        cout << iItemNumber << "\t\t";        iItemNumber++;    }
    cout << "Number of Elements:\t" << this->GetNumberOfPlanSteps() << endl;

    iLineNumbersForEdit = iItemNumber;
    int i;
    for(i=0; i< (signed)planSteps.size(); i++)
    {
        cout << endl;
        cout << "Begin Element "<< i+1 << " Information" << endl;
        planSteps[i]->print(true, 1, iItemNumber);
    }
}

int CPlan::generateInternalizedPlanData()
{
    CLoadMap clm;
    vector<CCoordinate> vBoundary;      //empty, is this how I want to do this?

    planContigencyMap = new CMap();          //potential memory leak
    planContigencyMap->setMapID( this->GetPlanID() + "-CONTIN" );
    planContigencyMap->setGoal( this->GetContigencyLocation().getX(), this->GetContigencyLocation().getY() );
    if( clm.openOVL( mapFileName, planContigencyMap, vBoundary) != EXIT_SUCCESS)
	    return EXIT_FAILURE;

    CCalculateMapCosts calMapCostCompletion( planContigencyMap );
    if( calMapCostCompletion.performCalculations() != EXIT_SUCCESS )
        return EXIT_FAILURE;

    cout << endl;
    int i;
    for(i=0; i< (signed)planSteps.size(); i++)
    {
        if( planSteps[i]->getElementType() == "MOVETO" )
        {
            CMoveToElement* pMove = (CMoveToElement*)planSteps[i];
            pMove->SetContigencyLocation( planContigencyMap );
            if( pMove->generateInternalizedPlanData(this->GetMapFileName()) != EXIT_SUCCESS )
                return EXIT_FAILURE;
        }
        else if( planSteps[i]->getElementType() == "RECONBUILDING" )
        {
            planSteps[i]->generateInternalizedPlanData(this->GetMapFileName());
        }
    }

    return EXIT_SUCCESS;
}

int CPlan::writeInternalizedPlanData( string strFileReply)
{
    cout << endl;
    cout << "Writing Iplan.dat file" <<endl;
	cout << flush;
	ofstream dataFile( "Iplan.dat", ios::out | ios::binary );

	if( !dataFile ) {
		cerr << "File could not be opened\n";
		return EXIT_FAILURE;
	}
	
    dataFile << this->GetPlanID() << endl;
    dataFile << this->GetMapFileName() << endl;

    if( this->GetPlanType() == ORDERED )
        dataFile << "ORDERED" <<endl;
    else
        dataFile << "LEAST-COMMITMENT" << endl;

    if( this->GetTethered() == true )
        dataFile << "TRUE" << endl;
    else
        dataFile << "FALSE" << endl;

    dataFile << strFileReply << endl;
    dataFile << this->GetNumberOfPlanSteps() << endl;
    planContigencyMap->writeMapToFile( dataFile, strFileReply );

    int i;
    for( i=0; i<(signed)planSteps.size(); i++)
    {
        cout << ".";
        dataFile << "START_ELEMENT" <<endl;
        dataFile << planSteps[i]->getElementType() << endl;
        if( planSteps[i]->writeMaps(dataFile, strFileReply) != EXIT_SUCCESS )
            return EXIT_FAILURE;
    }
    dataFile << "END_FILE" <<endl;

	dataFile.close();
    return EXIT_SUCCESS;
}

int CPlan::LoadPlanFromDataFile()
{
    try
	{
	      cout << "Loading Plans for robot "<<iRobotID << " please wait" <<endl;
          CLoadMap clm;
          if( clm.openIPLANDAT( this ) == EXIT_SUCCESS )
	  {
             return EXIT_SUCCESS;
	  }
	}
	catch( ... )   //TO DO: add exception class
	{
	  cout << "FAILURE: Loading Iplan.dat file"<< endl;
	}

      
	return EXIT_FAILURE;
}

void CPlan::RotateEgoCentric(VectorConfidenceAdvice& vEgoCentric, double dHeading) 
{
    VectorConfidenceAdvice localCopy=vEgoCentric; 
    double cosHeading=cos(((dHeading)*M_PI/180.0));
    double sinHeading=sin(((dHeading)*M_PI/180.0));
    vEgoCentric.SetX( localCopy.GetX()*cosHeading - localCopy.GetY()*sinHeading );
    vEgoCentric.SetY( localCopy.GetX()*sinHeading + localCopy.GetY()*cosHeading );
}

/**********************************************************************
 * $Log: CPlan.cpp,v $
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
