/*********************************************************************
 **                                                                  **
 **                     CCoordinate.cpp                              **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class represents a coordinate on an internal map.  **
 ** It consists of several setter and getter functions, operators,   **
 ** constructors.                                                    **
 **                                                                  **
 **********************************************************************/

#include "stdafx.h"

#include "CCoordinate.h"


//Constructors. By default set the distance to -1
CCoordinate::CCoordinate():x(0),y(0), distance(-1)	{}

CCoordinate::CCoordinate(int xPos, int yPos)
{
	x = xPos;
	y = yPos;
	distance = -1;
}

CCoordinate::CCoordinate(int xPos, int yPos, double distanceFromGoal)
{
	x = xPos;
	y = yPos;
	distance = distanceFromGoal;
}

// setting functions
void CCoordinate::setX( int newPos )    {    x = newPos;    }

void CCoordinate::setY( int newPos )    {    y = newPos;    }

void CCoordinate::setXY( int newPosX, int newPosY )    
{    
  x = newPosX;    
  y = newPosY;
}

void CCoordinate::setDistance( double dValue )  {    distance = dValue;   }

//operators
bool CCoordinate::operator== ( const CCoordinate & leftOp)
{
	if( (this->x == leftOp.x) && (this->y == leftOp.y) && (this->getDistance() == leftOp.distance) )
		return true;
	else
		return false;
}
bool CCoordinate::operator!= ( const CCoordinate & leftOp)
{
	if( (this->x != leftOp.x) || (this->y != leftOp.y) || (this->getDistance() == leftOp.distance))
		return true;
	else
		return false;
}
bool CCoordinate::operator<  ( const CCoordinate & leftOp)
{
	if( (this->distance < leftOp.distance) )
		return true;
	else 
		return false;
}
bool CCoordinate::operator>  ( const CCoordinate & leftOp)
{
	if( this->distance > leftOp.distance )
		return true;
	else 
		return false;
}
bool CCoordinate::operator<= ( const CCoordinate & leftOp)
{
	if( this->distance < leftOp.distance )
		return true;
	else 
		return false;
}
bool CCoordinate::operator>= ( const CCoordinate & leftOp)
{
	if( this->distance > leftOp.distance )
		return true;
	else 
		return false;
}

//getter functions
int CCoordinate::getX()	const	{	return x;	}

int CCoordinate::getY()	const	{	return y;	}

double CCoordinate::getDistance()	const	{	return distance;	}

/**********************************************************************
 * $Log: CCoordinate.cpp,v $
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
