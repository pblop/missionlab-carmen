/**********************************************************************
 **                                                                  **
 **                    CLoadMap.cpp                                  **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class loads a map. This is still under             **
 **	construction.													 **		
 **********************************************************************/

#include "stdafx.h"
#include "CLoadMap.h"
#include "ParseDefines.h"
#include "CMap.h"
#include "CPlan.h"

#define COMM_MAP_SIZE_X 135
#define COMM_MAP_SIZE_Y 100
#define COMMUNICATION_CUTOFF 15.0

CLoadMap::CLoadMap()
{}

int CLoadMap::parseOVLFile( string source )
{
  return 1;
}

int CLoadMap::parseOPARFile( string source )
{
  return 1;
}

int CLoadMap::openOPAR( string source, CMap* aMap )
{
	size_t position=0;
	string s1;
	size_t endOfNumber;

	s1 = loadFileToString( source );
	if( s1.size() == 0 )
		return -1;

//	aMap->setFilename( source );

	size_t eofCheck = s1.size()-1;
	while (s1.at(eofCheck) != '#' )
		eofCheck--;

	MOVE_TO_NEXT_NON_COMMENT	

//	End of Section 1	
	
	MOVE_TO_NEXT_NON_COMMENT

	FIND_END_OF_NUMBER

	string latitude = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string longitude = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string deltaPairs = s1.substr(position, endOfNumber - position);
	int pairs = atoi( deltaPairs.c_str() );

	MOVE_TO_NEXT_NON_COMMENT

	for(int i=0; i< pairs*2; i++)
	{
		FIND_END_OF_NUMBER
		string deltaX = s1.substr(position, endOfNumber - position);
		MOVE_TO_NEXT_LINE
	}

	MOVE_TO_NEXT_NON_COMMENT

//	End of Section 2

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_NUMBER

	string K1 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string K2 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_NON_COMMENT

//	End of Section 3

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_NUMBER

	string distanceCutoff = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_NUMBER

	string a1 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string b1 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string c1 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_NUMBER

	string a2 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string b2 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_LINE
	FIND_END_OF_NUMBER

	string c2 = s1.substr(position, endOfNumber - position);

	MOVE_TO_NEXT_NON_COMMENT
//	End of Section 4

	while( position != string::npos )
	{
		MOVE_TO_NEXT_NON_COMMENT

		if( position >= eofCheck )
			break;

		FIND_END_OF_NUMBER

		string obsType = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string height = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string obsLatitude = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string obsLongitude = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		deltaPairs = s1.substr(position, endOfNumber - position);
		pairs = atoi( deltaPairs.c_str() );

		MOVE_TO_NEXT_NON_COMMENT

		for(int i=0; i< pairs*2; i++)
		{
			FIND_END_OF_NUMBER
			string deltaX = s1.substr(position, endOfNumber - position);
			MOVE_TO_NEXT_LINE
		}	
		
		MOVE_TO_NEXT_NON_COMMENT

		if( position >= eofCheck )
			break;
	}
	
	return 0;
}

int CLoadMap::openOVL( string source, CMap* aMap, vector<CCoordinate> vFourCorners)
{
    size_t position=0, posA=0, posB=0;
    string s1,s2; 
    string obsValues[3];
    char * pEnd;
    size_t endOfNumber;

    s1 = loadFileToString( source );
    if( s1.size() == 0 )
        return -1;

    MOVE_TO_RESOLUTION_TAG
    FIND_END_OF_LINE

    string resolution = s1.substr(position, (endOfNumber-position));

    // Delete all the comments
    position = 0;
    while (true)
    {
        position  = s1.find("--", position);

        if (position == string::npos)
        {
            break;
        }

        FIND_END_OF_LINE

        s1.erase(position, (endOfNumber-position));
    }

    position = 0;

    MOVE_TO_MISSION_AREA_TAG
    FIND_END_OF_NUMBER

    string mapSizeX = s1.substr(position, (endOfNumber-position));

    position = endOfNumber+1;

    FIND_END_OF_LINE
	
    string mapSizeY = s1.substr(position, (endOfNumber-position));

    FIND_END_OF_LINE

    if( aMap->setSizeX( strtol(mapSizeX.c_str(), &pEnd, 10) ) != 0 )
        return -1;

    if( aMap->setSizeY( strtol(mapSizeY.c_str(), &pEnd, 10) ) != 0 )
        return -1;

    if( aMap->setResolution( strtod(resolution.c_str(), &pEnd),true ) != 0 )
    {
        cout << "Resolution Tag Error in overlay" << endl;
        return -1;
    }

    if( aMap->allocateMapMemory() != 0)
        return -1;

    if( vFourCorners.empty() != true)
    {
        aMap->addObstacle( WALL, (double)vFourCorners[0].getX(), (double)vFourCorners[0].getY(), (double)vFourCorners[1].getX(), (double)vFourCorners[1].getY());
        aMap->addObstacle( WALL, (double)vFourCorners[1].getX(), (double)vFourCorners[1].getY(), (double)vFourCorners[2].getX(), (double)vFourCorners[2].getY());
        aMap->addObstacle( WALL, (double)vFourCorners[2].getX(), (double)vFourCorners[2].getY(), (double)vFourCorners[3].getX(), (double)vFourCorners[3].getY());
        aMap->addObstacle( WALL, (double)vFourCorners[3].getX(), (double)vFourCorners[3].getY(), (double)vFourCorners[0].getX(), (double)vFourCorners[0].getY());
    }

	while( (position = s1.find(exactPoint, position)) != string::npos )				//iterate through the file
    {
	    position = position + 11;
        int endOfLine = (int)s1.find_first_of('\n', position);
        string stringWithoutTag = s1.substr(position, (endOfLine-position));		//get the first line
       
		CCoordinate temp = getCoordinate(stringWithoutTag);
        
		aMap->addObstacle( EXACTPOINT, (double)temp.getX(), (double)temp.getY(),0,0);
	} 

    while( (position = s1.find(obstacle, position)) != string::npos )				//iterate through the file
    {	
        position = position + 9;
        int endOfLine = (int)s1.find_first_of('\n', position);
        string stringWithoutTag = s1.substr(position, (endOfLine-position));		//get the first line
        size_t nextWhiteSpace;

        for(int i=0; i<3; i++)
        {
            nextWhiteSpace = stringWithoutTag.find_first_of(' ');
			if( nextWhiteSpace == string::npos)
			{
				obsValues[i] = stringWithoutTag.substr(0, stringWithoutTag.size());
				break;
			}
			else 
				obsValues[i] = stringWithoutTag.substr(0, nextWhiteSpace);

			stringWithoutTag = stringWithoutTag.substr(nextWhiteSpace+1, (stringWithoutTag.size() - nextWhiteSpace+1));
		}
		aMap->addObstacle( CIRCLE, strtod(obsValues[0].c_str(), &pEnd), strtod(obsValues[1].c_str(), &pEnd), strtod(obsValues[2].c_str(), &pEnd), 0.0);
	} 
	
	position = 0;
    while (true) //iterate through the file
    {	
        posA = s1.find(wall, position);
        posB = s1.find(wall, position);

        if ((posA == string::npos) && (posB == string::npos))
        {
            // No more "Wall" or "WALL" in the string.
            break;
        }

        position = (posA <= posB)? posA : posB;
		position = position + 6;
		position = s1.find("\"", position)+2;
		int endOfLine = (int)s1.find_first_of('\n', position);
		string stringWithoutTag = s1.substr(position, (endOfLine-position));		//get the first line
		double wallCoordX[256];
		double wallCoordY[256];
		size_t nextWhiteSpace;
		int count = 0;
		int coordNum = 0;
		int i=0;
		bool whileBreak = true;

		while ( stringWithoutTag.at(stringWithoutTag.size()-1) == ' ' )	
		{ 
			stringWithoutTag = stringWithoutTag.substr(0, stringWithoutTag.size()-1); 
		}

		while( stringWithoutTag.at(0) == ' ')	{	stringWithoutTag = stringWithoutTag.substr(1, stringWithoutTag.size());	}
		
		while(whileBreak)
		{
			string wallValues;
			nextWhiteSpace = stringWithoutTag.find_first_of(' ');
			if( nextWhiteSpace == string::npos)
			{
				whileBreak = false;
				nextWhiteSpace = stringWithoutTag.size();
			}
			 
			wallValues = stringWithoutTag.substr(0, nextWhiteSpace);
			coordNum++;
			
			if( coordNum%2 ==  1 )
				wallCoordX[count] = strtod(wallValues.c_str(), &pEnd);
			else
			{
				wallCoordY[count] = strtod(wallValues.c_str(), &pEnd);
				if( count > 0 )
					aMap->addObstacle( WALL, wallCoordX[count-1], wallCoordY[count-1], wallCoordX[count], wallCoordY[count]);

				count++;
				i=0;
			}

			if( whileBreak == true)
			{
				while( stringWithoutTag.at(nextWhiteSpace+1) == ' ')	{	nextWhiteSpace++;	}
				stringWithoutTag = stringWithoutTag.substr(nextWhiteSpace+1, (stringWithoutTag.size() - nextWhiteSpace+1));
			}
		}
	}		
    return 0;
}



int CLoadMap::openIPLANDAT( list< CMap* >& rList)
{
	size_t position=0;
	string s1;
	size_t endOfNumber;
	char* pEnd;

	s1 = loadFileToString( "Iplan.dat" );
	if( s1.size() == 0 )
		return -1;

	while( 1 )	//breaks when no BEGIN_PARALLEL tag exists
	{

        FIND_END_OF_LINE
        string temp = s1.substr(position, endOfNumber - position);
		if( s1.substr(position, endOfNumber - position) == "END_FILE" )
        {
            break;//cout << "Error on load" << endl;
            
        }

		MOVE_TO_BEGIN_MAP_TAG 

		CMap* newMap = new CMap();
		
		MOVE_TO_NEXT_NON_COMMENT
		FIND_END_OF_NUMBER

		string mapID = s1.substr(position, endOfNumber - position);
		newMap->setMapID( mapID );

		MOVE_TO_NEXT_NON_COMMENT
		FIND_END_OF_NUMBER

		string mapSizeX = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string mapSizeY = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string resolution = s1.substr(position, endOfNumber - position);

		int iMapSizeX = (int)strtol(mapSizeX.c_str(), &pEnd,10 );		
		int iMapSizeY = (int)strtol(mapSizeY.c_str(), &pEnd,10 );		

		if( newMap->setSizeX( iMapSizeX ) != 0 )
			return -1;

		if( newMap->setSizeY( iMapSizeY ) != 0 )
			return -1;

		if( newMap->setResolution( strtod(resolution.c_str(), &pEnd),false ) != 0 )
			return -1;

		if( newMap->allocateMapMemory() != 0)
			return -1;

		//set map size

		MOVE_TO_NEXT_NON_COMMENT
		FIND_END_OF_NUMBER

		string goalPositionX = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string goalPositionY = s1.substr(position, endOfNumber - position);

		int iGoalPositionX = strtol(goalPositionX.c_str(), &pEnd, 10);
		int iGoalPositionY = strtol(goalPositionY.c_str(), &pEnd, 10);

		newMap->addGoal( iGoalPositionX, iGoalPositionY );

		MOVE_TO_NEXT_NON_COMMENT
        FIND_END_OF_NUMBER

		string leastBoundaryX = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string leastBoundaryY = s1.substr(position, endOfNumber - position);

        MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string mostBoundaryX = s1.substr(position, endOfNumber - position);

        MOVE_TO_NEXT_LINE
		FIND_END_OF_NUMBER

		string mostBoundaryY = s1.substr(position, endOfNumber - position);
        
        int iLeastX = strtol(leastBoundaryX.c_str(), &pEnd, 10);
        int iLeastY = strtol(leastBoundaryY.c_str(), &pEnd, 10);
        int iMostX = strtol(mostBoundaryX.c_str(), &pEnd, 10);
        int iMostY = strtol(mostBoundaryY.c_str(), &pEnd, 10);

        MOVE_TO_NEXT_NON_COMMENT

		for(int i=iLeastX; i<iMostX; i++)
		{
			for(int j=iLeastY; j<iMostY; j++)
			{
				FIND_END_OF_NUMBER
				
				string costCell = s1.substr(position, endOfNumber - position);
				double iCostCell = strtod(costCell.c_str(), &pEnd );
				newMap->setMapDataValue( i, j, iCostCell);
				MOVE_TO_NEXT_LINE
			}
		}	
        FIND_END_OF_LINE
        temp = s1.substr(position, endOfNumber - position);
		if( s1.substr(position, endOfNumber - position) != "END_MAP" )
        {
            cout << "Error on load" << endl;
            
        }
    
        MOVE_TO_NEXT_LINE
        FIND_END_OF_LINE

		rList.push_back( newMap );
		s1 = s1.substr(position, s1.size() - position);
    
        position =0; endOfNumber =0;
                
	}

	return 0;
}

int CLoadMap::openIPLANDAT( CPlan* aPlan)
{
    char buf[255];
    string rValue;
    char* pEnd;

    ifstream fStream("Iplan.dat", ios::in ); 

    if( fStream.is_open() != true )
    {
        cerr << "ERROR: Could not open Iplan.dat file." << endl;
        return EXIT_FAILURE;
    }

    cout << "Skipping map Load " << endl;
   
    fStream >> rValue;
    aPlan->SetPlanID( rValue );
    fStream.getline(buf, 255, '\n' );
	
    fStream >> rValue;
    aPlan->SetMapFileName( rValue );
    fStream.getline(buf, 255, '\n' );
	
    fStream >> rValue;
    if( rValue == "ORDERED" )
        aPlan->SetPlanType( ORDERED );
    else
        aPlan->SetPlanType( LEAST_COMMITMENT );
    fStream.getline(buf, 255, '\n' );
	
    fStream >> rValue;
    if( rValue == "TRUE" )
        aPlan->SetTethered( true );
    else
        aPlan->SetTethered( false );
    fStream.getline(buf, 255, '\n' );

    fStream >> rValue;
    string strReadType = rValue;
    fStream.getline(buf, 255, '\n' );

    fStream >> rValue;
    int iNumberOfElements = strtol(rValue.c_str(), &pEnd, 10);
    fStream.getline(buf, 255, '\n' );

    CMap* pContingencyMap = new CMap();
    LoadMap( pContingencyMap, fStream, strReadType );
  
    int i;
    vector< CReactivePlanningElement* > planSteps(iNumberOfElements);
    for( i=0; i<iNumberOfElements; i++)
    {
        CReactivePlanningElement* newElement=NULL;
        cout << ".";
		cout << flush;
        fStream >> rValue;
        if( rValue != "START_ELEMENT" )
        {    
            cout << "ERROR: Element "<<i+1<< " failed at START_ELEMENT tag." <<endl;
            break;
        }
		
        fStream.getline(buf, 255, '\n' );
        fStream >> rValue;
        if(rValue == "MOVETO")
        {	
            newElement = new CMoveToElement();
            CMoveToElement* pElement = static_cast< CMoveToElement* >(newElement);
            pElement->SetContigencyLocation( pContingencyMap );
        }
        else if( rValue == "RECONBUILDING")
        {	
            newElement = new CReconBuildingElement();
            CReconBuildingElement* pElement = static_cast< CReconBuildingElement* >(newElement);
            vector< CCoordinate > vProg(2);

            fStream.getline(buf, 255, '\n');
            int i, iCount;
            for(iCount=0; iCount<2; iCount++)
            {
                for(i=0; i<(signed)vProg.size(); i++)
                {
                    fStream.getline(buf, 255, '\n');
                    string s1( buf );
                    vProg[i] = getCoordinate( s1 );
                }
                if( iCount == 0 )
                    pElement->setProgressBoxLeft( vProg );
                else
                    pElement->setProgressBoxRight( vProg );
            }
        }
        else
        {
            cout << "ERROR: Element "<<i+1<< " of unknown type." <<endl;
            break;
        }

        newElement->setElementNumber( i );		
        if( newElement->readMaps( fStream, strReadType) != EXIT_SUCCESS )
            return EXIT_FAILURE;
	  
		//This is a little flakey... would like to come up with something better.
        if( newElement->getElementType() == "RECONBUILDING")
            newElement->setElementGoal( (newElement->GetMaps()).front()->getGoal() );
 
        planSteps[i] = newElement;
    }
   
    aPlan->SetContigencyMap( pContingencyMap );
    aPlan->SetPlanElements( planSteps );
	
    fStream >> rValue;
    if( rValue != "END_FILE")
        cerr<< "ERROR: No end of file tag after loading all elements"<<endl;
	cout << endl;
    
	return EXIT_SUCCESS;
}

int CLoadMap::LoadMap( CMap* pMap, ifstream& fStream, string strReadType)
{
    char buf[255];
    string rValue;
    char* pEnd;
    
    fStream >> rValue;
    if( rValue != "BEGIN_MAP")
        return EXIT_FAILURE;
 
    fStream.getline(buf, 255, '\n' );

    fStream >>rValue;
    pMap->setMapID( rValue );
    fStream.getline(buf, 255, '\n' );

    fStream >>rValue;
    if( pMap->setSizeX( strtol(rValue.c_str(), &pEnd, 10) ) != 0 )
        return EXIT_FAILURE;
    fStream.getline(buf, 255, '\n' );

    fStream >>rValue;
    if( pMap->setSizeY( strtol(rValue.c_str(), &pEnd, 10) ) != 0 )
        return EXIT_FAILURE;
    fStream.getline(buf, 255, '\n' );

    fStream >>rValue;
    if( pMap->setResolution( strtod(rValue.c_str(), &pEnd ), false ) != 0 )
        return EXIT_FAILURE;
    fStream.getline(buf, 255, '\n' );

    if( pMap->allocateMapMemory() != 0)
			return EXIT_FAILURE;

    fStream >>rValue;
    double dGoalX = strtod(rValue.c_str(), &pEnd);
    fStream.getline(buf, 255, '\n' );
    fStream >>rValue;
    double dGoalY = strtod(rValue.c_str(), &pEnd);
    if( pMap->setGoal( dGoalX, dGoalY ) != 0 )
        return EXIT_FAILURE;
    fStream.getline(buf, 255, '\n' );

    fStream.getline(buf, 255, '\n' );

/*
    if( strReadType == "FLOAT" )
    {
        int i;
        float readBuffer[220*220];
        fStream.read((char*)readBuffer, pMap->getSizeX()*pMap->getSizeY()*sizeof(float));
        for(i=0;i<pMap->getSizeX();i++)
	    {
		    for(int j=0;j<pMap->getSizeY();j++) 
		    {
                float value = (float)((readBuffer[i*220+j]));
			    pMap->setMapDataValue(i,j,(double)value);
		    }
	    }
    }
    else
    {
*/
        for(int i=0; i<pMap->getSizeX(); i++)
		{
			for(int j=0; j<pMap->getSizeY(); j++)
			{
				string costCell;
                fStream >> costCell;
				double iCostCell = strtod(costCell.c_str(), &pEnd );
				pMap->setMapDataValue( i, j, iCostCell);
		
			}
		}	
//    }
    fStream.getline(buf, 255, '\n' );
    fStream.getline(buf, 255, '\n' );

    return EXIT_SUCCESS;
}

int CLoadMap::openCMM( string source, CMap* aMap)
{
	size_t position=0;
	string s1,s2;
	size_t endOfNumber;

	s1 = loadFileToString( source );
	if( s1.size() == 0 )
		return -1;
 
	if( aMap->setSizeX( COMM_MAP_SIZE_X ) != 0 )
		return -1;

	if( aMap->setSizeY( COMM_MAP_SIZE_Y ) != 0 )
		return -1;

	if( aMap->setResolution( 1, true ) != 0 )
		return -1;

	if( aMap->allocateMapMemory() != 0)
		return -1;

	char* pEnd;
	double max=0;
	while( true )	//temp... need to mark the end of the file
	{
		
		FIND_END_OF_NUMBER
		string mapPosX = s1.substr(position, (endOfNumber-position-1));
		MOVE_TO_BEGINNING_OF_NEXT_NUMBER

		FIND_END_OF_NUMBER
		string mapPosY = s1.substr(position, (endOfNumber-position-1));
		MOVE_TO_BEGINNING_OF_NEXT_NUMBER

		FIND_END_OF_LINE
		string commVal = s1.substr(position, (endOfNumber-position-1));

		int iMapPosX = (int)strtol(mapPosX.c_str(), &pEnd,10 );	
		int iMapPosY = (int)strtol(mapPosY.c_str(), &pEnd,10 );	
		double dCommValue = strtod(commVal.c_str(), &pEnd);

		aMap->setMapDataValue( iMapPosX, iMapPosY, dCommValue );
		if( dCommValue > max )
			max = dCommValue;
		if ( (iMapPosX == aMap->getSizeX()-1) && (iMapPosY == aMap->getSizeY()-1) )
			break;

		MOVE_TO_NEXT_LINE
	}

	for(int i=0;i<COMM_MAP_SIZE_X;i++)
	{
		for(int j=0;j<COMM_MAP_SIZE_Y;j++)
		{
			double value = max - aMap->getMapDataValue(i,j);
			
			if ( value > COMMUNICATION_CUTOFF )		//where do I get this number?
				value = OBSTACLEVALUE;
			else
				value = GTIP_INFINITY;
			
			aMap->setMapDataValue(i,j, value);
		}
	}

	return 0;
}

string CLoadMap::loadFileToString( string source)
{
	string s1;
	int length;
	char * buffer;

	ifstream dataFile( source.c_str(), ios::in );	//open the file

	if( !dataFile ) 
	{
		cerr << "Error opening file: "<< source <<endl;
	}
	else
	{
		dataFile.seekg(0, ios::end);	//determine the size of the file
		length = dataFile.tellg();
		dataFile.seekg(0,ios::beg);
		buffer = new char[length];		//create a character buffer to
		dataFile.read(buffer,length);	//store the files contents
		s1=buffer;						//revert copy char buffer to a string

		if( s1.at(s1.size()-1) != '\n')	//sometimes junk is 
		{								//inserted at the end. MS bug?
			s1 = s1.substr(0, s1.find_last_of("\n")+1 );
		}
	}

	dataFile.close();
	delete buffer;

	return s1;
}

CCoordinate CLoadMap::getCoordinate(string s1)
{
    CCoordinate result;
    char* pEnd;

    size_t comma = s1.find_first_of(",");
    string x = s1.substr(1, comma-1);
    string y = s1.substr(s1.find_first_of(",")+1, s1.size()-2-comma);

    result.setX( strtol(x.c_str(), &pEnd, 10) );
    result.setY( strtol(y.c_str(), &pEnd, 10) );

    return result;
}

int CLoadMap::openPCF( string strPCFname, CPlan* aPlan)
{
    size_t position=0;
	string s1;
	size_t endOfNumber;
	char* pEnd;

	s1 = loadFileToString( strPCFname );
	if( s1.size() == 0 )
		return EXIT_FAILURE;
    
    FIND_END_OF_LINE
	if( s1.substr(position, endOfNumber - position) == "END_FILE" )
        return EXIT_SUCCESS;

	MOVE_TO_BEGIN_PLAN_TAG 
	
	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE
    
    string temp = s1.substr(position, endOfNumber - position);
	string planID = s1.substr(position, endOfNumber - position);
	aPlan->SetPlanID( planID );

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE

	string mapFileName = s1.substr(position, endOfNumber - position);
	aPlan->SetMapFileName( mapFileName );

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE

	string strOrdering = s1.substr(position, endOfNumber - position);
	if( strOrdering == "LEAST-COMMITMENT" )
		aPlan->SetPlanType( LEAST_COMMITMENT );
	else
		aPlan->SetPlanType( ORDERED );

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE

	string strTethered = s1.substr(position, endOfNumber - position);
	if( strTethered == "YES" )
		aPlan->SetTethered( true );
	else
		aPlan->SetTethered( false );

	MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE

    aPlan->SetContigencyLocation( getCoordinate(s1.substr(position, endOfNumber - position) ) );

    MOVE_TO_NEXT_NON_COMMENT
	FIND_END_OF_LINE

    string strNumberOfElements = s1.substr(position, endOfNumber - position);
    int iNumberOfElements = (int)strtol(strNumberOfElements.c_str(), &pEnd,10 );

    vector< CReactivePlanningElement* > planSteps(iNumberOfElements);
   
    for( int i=0; i<iNumberOfElements; i++)
    {
		MOVE_TO_NEXT_NON_COMMENT
		FIND_END_OF_LINE
        
        string strElementType = s1.substr(position, endOfNumber - position);

		MOVE_TO_NEXT_NON_COMMENT
    	FIND_END_OF_LINE

		string strElementNumber = s1.substr(position, endOfNumber - position);	
		int iElementNumber = (int)strtol(strElementNumber.c_str(), &pEnd,10 );
	
        if(strElementType == "RECONBUILDING" )
        {
            CReconBuildingElement* someBuilding = new CReconBuildingElement();
			someBuilding->setElementNumber( iElementNumber );
            planSteps[iElementNumber-1] = someBuilding;
			
			MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

            someBuilding->setElementGoal( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

			someBuilding->setLeftFourRobotPointGoal( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

            someBuilding->setRightFourRobotPointGoal( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			//Boundary box that limits calculation of the map to a portion of the entire map
			vector< CCoordinate > boundaryBox(4);
            boundaryBox[0] = getCoordinate(s1.substr(position, endOfNumber - position) );

            MOVE_TO_NEXT_LINE
    		FIND_END_OF_LINE

            boundaryBox[1] = getCoordinate(s1.substr(position, endOfNumber - position) );

            MOVE_TO_NEXT_LINE
    		FIND_END_OF_LINE

            boundaryBox[2] = getCoordinate(s1.substr(position, endOfNumber - position) );

            MOVE_TO_NEXT_LINE
    		FIND_END_OF_LINE
            
            boundaryBox[3] = getCoordinate(s1.substr(position, endOfNumber - position) );
			someBuilding->setBoundaryBox( boundaryBox );
			
            //Box designating left progress point
            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			vector< CCoordinate > progressBoxLeft(2);
            progressBoxLeft[0] = getCoordinate(s1.substr(position, endOfNumber - position) );

            MOVE_TO_NEXT_LINE
    		FIND_END_OF_LINE

            progressBoxLeft[1] = getCoordinate(s1.substr(position, endOfNumber - position) );
			someBuilding->setProgressBoxLeft( progressBoxLeft );
            
			//Box designating right progress point
            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			vector< CCoordinate > progressBoxRight(2);
            progressBoxRight[0] = getCoordinate(s1.substr(position, endOfNumber - position) );

            MOVE_TO_NEXT_LINE
    		FIND_END_OF_LINE

            progressBoxRight[1] = getCoordinate(s1.substr(position, endOfNumber - position) );
			someBuilding->setProgressBoxRight( progressBoxRight );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

			someBuilding->setPreprocessStartLeft( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			
			someBuilding->setPreprocessEndLeft( getCoordinate(s1.substr(position, endOfNumber - position) ) );
            
            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			
			someBuilding->setPreprocessStartRight( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

			someBuilding->setPreprocessEndRight( getCoordinate(s1.substr(position, endOfNumber - position) ) );

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

            string growLeftStart = s1.substr(position, endOfNumber - position);
            if( growLeftStart != "NONE")
            {
                vector< CCoordinate > growLeft;
                growLeft.push_back( getCoordinate(s1.substr(position, endOfNumber - position) ) );
                
                MOVE_TO_NEXT_LINE
    		    FIND_END_OF_LINE 
                
                growLeft.push_back( getCoordinate(s1.substr(position, endOfNumber - position) ) );
                someBuilding->setGrowObstacleLeft( growLeft );
            }

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE

            string growRightStart = s1.substr(position, endOfNumber - position);
            if( growRightStart != "NONE")
            {
                vector< CCoordinate > growRight;
                growRight.push_back( getCoordinate(s1.substr(position, endOfNumber - position) ) );
               
                
                MOVE_TO_NEXT_LINE
    		    FIND_END_OF_LINE 
                
                growRight.push_back( getCoordinate(s1.substr(position, endOfNumber - position) ) );
                someBuilding->setGrowObstacleRight( growRight );
            }
        }
        else if(strElementType == "MOVETO")
        {
			CMoveToElement* someMovement = new CMoveToElement();
			someMovement->setElementNumber( iElementNumber );
            planSteps[iElementNumber-1] = someMovement;

            MOVE_TO_NEXT_NON_COMMENT
    		FIND_END_OF_LINE
			
			string temp = s1.substr(position, endOfNumber - position);
		
			someMovement->setElementGoal( getCoordinate(s1.substr(position, endOfNumber - position) ) );
        }
        else
            cout << "Element type not found!" <<endl;
    }

    aPlan->SetPlanElements( planSteps );

    return EXIT_SUCCESS;
}

/**********************************************************************
 * $Log: CLoadMap.cpp,v $
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
