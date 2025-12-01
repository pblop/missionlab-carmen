/**********************************************************************
 **                                                                  **
 **                    CCalculateMapCosts.cpp                        **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class calculates the map costs associated with a   **
 ** map. This is where most of the work is done. It is based on the  **
 ** A* algorithim. The basic approach is the following: Get the      **
 ** minimium node is a queue, set the map with it's distance from the**
 ** goal, add all of it's neighbors to the queue, sort the queue.    **
 ** Repeat this procedure until there are no nodes left.			 **
 **********************************************************************/

#include "stdafx.h"

#include "CCalculateMapCosts.h"
#include "CCoordinate.h"


double resolution;
double sqrtOfResolution;
const int accuracy =1000;

//We perform the calculations everytime we instantiate
CCalculateMapCosts::CCalculateMapCosts(CMap* mPointer)	
{
	mapPointer = mPointer;
	comPointer = NULL;
	mapGoal = mapPointer->getGoal();
	resolution = mapPointer->getResolutionInMeters();

	double fractpart, intpart;
	fractpart = modf((sqrt(pow(resolution,2) + pow(resolution,2)))*accuracy, &intpart);
	if ( fractpart > .5 )
	{	
		intpart++;
	}
	sqrtOfResolution = intpart/accuracy;
}

CCalculateMapCosts::CCalculateMapCosts(CMap* mPointer, CMap* cPointer)	
{
	mapPointer = mPointer;
	comPointer = cPointer;
	mapGoal = mapPointer->getGoal();
	resolution = mapPointer->getResolutionInMeters();
	
	double fractpart, intpart;
	fractpart = modf((sqrt(pow(resolution,2) + pow(resolution,2)))*accuracy, &intpart);
	if ( fractpart > .5 )
	{	
		intpart++;
	}
	sqrtOfResolution = intpart/accuracy;
	performCalculations();
}

int CCalculateMapCosts::performAlteredCalculationsLineH(int iAlterLineFrom, int iAlterLineTo, int iAlterLineConst)
{
    int i=0;
    for( i=iAlterLineFrom; i <=iAlterLineTo; i++)
        mapPointer->setMapDataValue(i, iAlterLineConst,OBSTACLEVALUE);

    performCalculations();

    for( i=iAlterLineFrom; i <=iAlterLineTo; i++)
    {
        double value = mapPointer->getMapDataValue(i, iAlterLineConst-1);
        if( value == GTIP_INFINITY || value == OBSTACLEVALUE )   
            value = mapPointer->getMapDataValue(i-1, iAlterLineConst-1);
        if( value == GTIP_INFINITY || value == OBSTACLEVALUE ) 
            value = mapPointer->getMapDataValue(i+1, iAlterLineConst-1);

        mapPointer->setMapDataValue(i, iAlterLineConst, value+1);    
    }
    
    return 0;
}

int CCalculateMapCosts::performProcessedCalculations( CCoordinate fromCoord, CCoordinate toCoord)
{
    bool bVertical =false;
    if( fromCoord.getX() == toCoord.getX() )
        bVertical = true;
    
    int i=0;
    if( bVertical == true )
    {
        for( i=fromCoord.getY(); i <=toCoord.getY(); i++)
            mapPointer->setMapDataValue(fromCoord.getX(), i, OBSTACLEVALUE);
    }
    else
    {
        for( i=fromCoord.getX(); i <=toCoord.getX(); i++)
            mapPointer->setMapDataValue(i, fromCoord.getY(), OBSTACLEVALUE);
    }

    performCalculations();


    if( bVertical == true )
    {
        for( i=fromCoord.getY(); i <=toCoord.getY(); i++)
        {
            double value = mapPointer->getMapDataValue(fromCoord.getX()-1, i);
            if( value == GTIP_INFINITY || value == OBSTACLEVALUE )   
                value = mapPointer->getMapDataValue(fromCoord.getY()-1, i-1);
            if( value == GTIP_INFINITY || value == OBSTACLEVALUE ) 
                value = mapPointer->getMapDataValue(fromCoord.getY()-1, i+1);

            mapPointer->setMapDataValue(fromCoord.getX(), i, value+1);    
        }
    }
    else
    {
        for( i=fromCoord.getX(); i <=toCoord.getX(); i++)
        {
            double value = mapPointer->getMapDataValue(i, fromCoord.getY()-1);
            if( value == GTIP_INFINITY || value == OBSTACLEVALUE )   
                value = mapPointer->getMapDataValue(i-1, fromCoord.getY()-1);
            if( value == GTIP_INFINITY || value == OBSTACLEVALUE ) 
                value = mapPointer->getMapDataValue(i+1, fromCoord.getY()-1);

            mapPointer->setMapDataValue(i, fromCoord.getY(), value+1);    
        }
    }

	return EXIT_SUCCESS;

}
int CCalculateMapCosts::performAlteredCalculationsLineV(int iAlterLineFrom, int iAlterLineTo, int iAlterLineConst)
{
    int j=0;
    for( j=iAlterLineFrom; j <=iAlterLineTo; j++)
        mapPointer->setMapDataValue(iAlterLineConst, j, OBSTACLEVALUE);

    performCalculations();

    for( j=iAlterLineFrom; j <=iAlterLineTo; j++)
    {
        double value = mapPointer->getMapDataValue(iAlterLineConst-1, j);
        mapPointer->setMapDataValue(iAlterLineConst, j, value+1);    
    }
	return 0;
}

int CCalculateMapCosts::growObstacle(CCoordinate cFrom, CCoordinate cTo)
{
    bool bVertical =false;
    if( cFrom.getX() == cTo.getX() )
        bVertical = true;

    if( bVertical == true )
	{
	    for( int i = cFrom.getY(); i< cTo.getY(); i++)
		{
		    mapPointer->setMapDataValue(cFrom.getX(), i, OBSTACLEVALUE); 
		}
	}
    else
	{
	    for( int i = cFrom.getX(); i< cTo.getX(); i++)
		{
		    mapPointer->setMapDataValue(i, cFrom.getY(), OBSTACLEVALUE); 
		}
	}

    return EXIT_SUCCESS;
}

int CCalculateMapCosts::performCalculations()
{
	list< CCoordinate > nodeQueue;
	CCoordinate cheapestNode;

	nodeQueue.push_front( mapGoal );	//add our start node

	while( nodeQueue.size() != 0) 
	{
		//get the closest node
		cheapestNode = nodeQueue.front();
		nodeQueue.pop_front();
		//set the distance in the map
		mapPointer->setMapDataValue(cheapestNode.getX(), cheapestNode.getY(), cheapestNode.getDistance());
		//get it's neighbors
		list< CCoordinate> minNeighbors = getNeighbors(cheapestNode);
		//sort the neighbors
		minNeighbors.sort();
		nodeQueue.merge(minNeighbors);
	}
	return EXIT_SUCCESS;
}

//circumnavigate a node collecting all of the neighbors that are not obstacles 
//and are infinity
list< CCoordinate > CCalculateMapCosts::getNeighbors( CCoordinate position)
{
	int step=0;
	CCoordinate stepCoord = position;
	stepCoord.setDistance(GTIP_INFINITY);
	list< CCoordinate > returnList;

	while(step < 8)
	{
		switch( step )
		{
		case 0:
			if(goLeft(stepCoord)==true)
				break;
			else
				step++;
		case 1:
			if(goUp(stepCoord)==true)
				break;
			else
				step++;
		case 2:
			if(goRight(stepCoord)==true)
				break;
			else
				step++;
		case 3:
			if(goRight(stepCoord)==true)
				break;
			else
				step++;
		case 4:
			if(goDown(stepCoord)==true)
				break;
			else
				step++;
		case 5:
			if(goDown(stepCoord)==true)
				break;
			else
				step++;
		case 6:
			if(goLeft(stepCoord)==true)
				break;
			else
				step++;
		case 7:
			if(goLeft(stepCoord)==true)
				break;
			else
			{
				step++;
				break;
			}
		default:
			cout << "should never get here";
			break;
		}
		
		if( step > 7 ) break;		
		step++;
		//we only want nodes that are not obstacles and are infinity (all others will have already
		//been assigned a value on the map)
		if( (mapPointer->getMapDataValue(stepCoord.getX(), stepCoord.getY()) != OBSTACLEVALUE)) 
		{	
			double comCost=0;
			if( comPointer != NULL)
				comCost = comPointer->getMapDataValue(stepCoord.getX(),stepCoord.getY());

		   	if( step%2 == 0)
			{
			  if( mapPointer->getMapDataValue(stepCoord.getX(), stepCoord.getY()) > (position.getDistance() + sqrtOfResolution + comCost ) ) 
			  {	
					stepCoord.setDistance( position.getDistance() + sqrtOfResolution + comCost );
					mapPointer->setMapDataValue( stepCoord.getX(), stepCoord.getY(), (position.getDistance() + sqrtOfResolution + comCost) );
					returnList.push_front( stepCoord );	
			  }
			}
			else
			{
			  if( mapPointer->getMapDataValue(stepCoord.getX(), stepCoord.getY()) > (position.getDistance() + resolution + comCost)) 
			  {
					stepCoord.setDistance( position.getDistance() + resolution + comCost);
					mapPointer->setMapDataValue( stepCoord.getX(), stepCoord.getY(), (position.getDistance() + resolution + comCost ));
					returnList.push_front( stepCoord );	
			  }
			}
		}
	}
	return returnList;
}

//members to move around
bool CCalculateMapCosts::goLeft( CCoordinate& currentPosition)
{
	currentPosition.setX( currentPosition.getX() -1 );
	return testValidCoordinate(currentPosition);
}

bool CCalculateMapCosts::goRight( CCoordinate& currentPosition)
{
	currentPosition.setX( currentPosition.getX() +1 );
	return testValidCoordinate(currentPosition);
}

bool CCalculateMapCosts::goUp( CCoordinate& currentPosition)
{
	currentPosition.setY( currentPosition.getY() +1 );
	return testValidCoordinate(currentPosition);
}

bool CCalculateMapCosts::goDown( CCoordinate& currentPosition)
{
	currentPosition.setY( currentPosition.getY() -1 );
	return testValidCoordinate(currentPosition);
}

bool CCalculateMapCosts::testValidCoordinate( CCoordinate currentPosition)
{
	if( (currentPosition.getX() < 0) || (currentPosition.getX() >= mapPointer->getSizeX() ) ||
		(currentPosition.getY() < 0) || (currentPosition.getY() >= mapPointer->getSizeY() ))
	{
		return false;
	}
	else 
		return true;	
}

/**********************************************************************
 * $Log: CCalculateMapCosts.cpp,v $
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
