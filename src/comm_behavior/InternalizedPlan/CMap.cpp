/*********************************************************************
 **                                                                  **
 **                     CMap.cpp                                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2004 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class represents an internal map. The array        **
 ** "mapCost" is of primal importance. This array stores the cost    **
 ** associated with travel to a coordinate.                          **
 ** constructors.                                                    **
 **                                                                  **
 **********************************************************************/

#include "stdafx.h"

#include "CMap.h"
#include "GlobalUtilities.h"


CMap::CMap()	
{
  xSize = -1;
  ySize = -1;
  resolution = -1;
  goal.setDistance( 0 );
  offset.setX( 0 );
  offset.setY( 0 );
  vectorMap = NULL;
}

CMap::~CMap()
{ 
    vBoundaryPoints.clear();
    int n=0;
    // deallocating the matrix
	for ( n=0;n<xSize;n++)
		delete []mapCost[n];

	delete []mapCost;

    if( vectorMap != NULL )
    {
        for (n=0;n<xSize;n++)
			delete []vectorMap[n];

	    delete []vectorMap;
    }
}

int CMap::allocateMapMemory()
{
    int n=0;
	int i=0;
	int j=0;
  
	if( (xSize < 1 ) || (ySize < 1))
		return -1;

	try
	{
	    mapCost = new double*[xSize];
       
		for (n=0;n<xSize;n++)
			mapCost[n]=new double[ySize];
	}
	catch(...)
	{
		for (n=0;n<xSize;n++)
			delete []mapCost[n];

	  delete []mapCost;
       
	  fprintf( stderr, "Could not allocation sufficient memory for the internal plan");
	  throw;
	}

    for(i=0;i<xSize;i++)
	{
		for(j=0;j<ySize;j++)
			mapCost[i][j]= GTIP_INFINITY;
	}

	return 0;
}


void CMap::printVectorMap()
{
	for(int j=(ySize-1);j>-1;j--)
	{
		for(int i=0;i<xSize;i++)
		{
			if ( (vectorMap[i][j].GetX() == OBSTACLEVALUE ) && (vectorMap[i][j].GetY() == OBSTACLEVALUE ) && (vectorMap[i][j].GetY() == OBSTACLEVALUE ) )
			        cout << setw(2) << "XX";
			else if ( (vectorMap[i][j].GetX() == GTIP_INFINITY ) && (vectorMap[i][j].GetY() == GTIP_INFINITY ) && (vectorMap[i][j].GetY() == GTIP_INFINITY ) )
			        cout << setw(2) << "99";
            else if ( (vectorMap[i][j].GetX() == 0 ) && (vectorMap[i][j].GetY() == 0 ) )
			        cout << setw(2) << "00";

            else if ( (vectorMap[i][j].GetX() == 1 ) && (vectorMap[i][j].GetY() == 0 ) )
			        cout << setw(2) << "->";
            else if ( (vectorMap[i][j].GetX() ==  1) && (vectorMap[i][j].GetY() == 1 ) )
			        cout << setw(2) << "/|";
            else if ( (vectorMap[i][j].GetX() == 0 ) && (vectorMap[i][j].GetY() == 1 ) )
			        cout << setw(2) << "^^";
            else if ( (vectorMap[i][j].GetX() == -1 ) && (vectorMap[i][j].GetY() == 1 ) )
			        cout << setw(2) << "|\\";

            else if ( (vectorMap[i][j].GetX() == -1 ) && (vectorMap[i][j].GetY() == 0 ) )
			        cout << setw(2) << "<-";            
            else if ( (vectorMap[i][j].GetX() ==  -1) && (vectorMap[i][j].GetY() == -1 ) )
			        cout << setw(2) << "|/";
            else if ( (vectorMap[i][j].GetX() == 0 ) && (vectorMap[i][j].GetY() == -1 ) )
			        cout << setw(2) << "vv";            
            else if ( (vectorMap[i][j].GetX() ==  1) && (vectorMap[i][j].GetY() == -1 ) )
			        cout << setw(2) << "\\|";


			cout << " "; 
		}
		cout << endl;
	}
}

void CMap::printMap()
{
	for(int j=(ySize-1);j>-1;j--)
	{
		for(int i=0;i<xSize;i++)
		{
			if ( getMapDataValue(i,j) == OBSTACLEVALUE )
			        cout << setw(5) <<  setprecision(4) << setfill('X') << "XXXX";
			else
			        cout << setw(5) <<  setprecision(4) << setfill('0') <<  getMapDataValue(i,j);

			cout << " "; 
		}
		cout << endl;
	}
}

void CMap::printGoal()
{
	for(int j=(ySize-1);j>-1;j--)
	{
		for(int i=0;i<ySize;i++)
		{
			if ( getMapDataValue(i,j) == 0 )
				cout << setw(5) <<  setprecision(4) << setfill('X') << "XXXX";
			else
				cout << setw(5) <<  setprecision(4) << setfill('0') << "    ";

			cout << " ";
		}
		cout << endl;
	}
}

void CMap::printObstacles()
{
	for(int j=(ySize-1);j>-1;j--)
	{
		for(int i=0;i<xSize;i++)
		{
			if ( getMapDataValue(i,j) == OBSTACLEVALUE )
				cout << setw(5) <<  setprecision(4) << setfill('X') << "XXXX";
			else
				cout << setw(5) <<  setprecision(4) << setfill('0') << "    ";

			cout << " ";
		}
		cout << endl;
	}
}

void CMap::printFilename()
{
    cout << getFilename() <<endl; 
}

//	This member function returns a unit vector to the minimal neighbor. 
//	When multiple neighbors have the same cost, the first one encountered is returned

VectorConfidenceAdvice CMap::getVector(double iDouble, double jDouble)
{	
	list<double> vectors;
	return (getMapAdvice(iDouble, jDouble, vectors)).front();
}

double CMap::getAngle(double iDouble, double jDouble)
{
	list<double> angle;
	getMapAdvice(iDouble, jDouble, angle);
	return angle.front();
}

list< VectorConfidenceAdvice > CMap::getAllVector(double iDouble, double jDouble)
{	
	list<double> angle;
	return getMapAdvice(iDouble, jDouble, angle);
}

list< double > CMap::getAllAngle(double iDouble, double jDouble)
{
	list<double> angle;
	getMapAdvice(iDouble, jDouble, angle);
	return angle;
}

list<VectorConfidenceAdvice> CMap::getMapAdvice(double iDouble, double jDouble, list<double> angle)
{
    GlobalUtilities g;

	iDouble = iDouble/resolution;
	jDouble = jDouble/resolution;

	int i =	(int)g.round(iDouble);
	int j = (int)g.round(jDouble);

	list<VectorConfidenceAdvice> rVector;
	VectorConfidenceAdvice aVector;

	double smallestValue=100000;

	if( (i>0) && (getMapDataValue(i-1,j) <= smallestValue) )
	{
        smallestValue = getMapDataValue(i-1, j );
        angle.push_front( 180 );
		aVector.SetX( -1 );
		aVector.SetY( 0 );
		rVector.push_front( aVector );
	}
	if( (i>0) && (j<ySize-1) && (getMapDataValue(i-1, j+1 ) <= smallestValue) )
	{       
		smallestValue = getMapDataValue(i-1, j+1 );
		angle.push_front( 135 );
		aVector.SetX( -1 );
		aVector.SetY( 1 );
		rVector.push_front( aVector );
	}
	if((j<ySize-1) && (getMapDataValue(i, j+1) <= smallestValue))
	{       
		smallestValue = getMapDataValue(i, j+1);
		angle.push_front( 90 );
		aVector.SetX( 0 );
		aVector.SetY( 1 );
		rVector.push_front( aVector );
	}
	if( (i<xSize-1) && (j<ySize-1) && (getMapDataValue(i+1, j+1) <= smallestValue))
	{ 
		smallestValue = getMapDataValue(i+1, j+1);
		angle.push_front( 45 );
		aVector.SetX( 1 );
		aVector.SetY( 1 );
		rVector.push_front( aVector );
	}
	if( (i<xSize-1) && (getMapDataValue(i+1, j) <= smallestValue))
	{ 
		smallestValue = getMapDataValue(i+1, j);
		angle.push_front( 0 );
		aVector.SetX( 1 );
		aVector.SetY( 0 );
		rVector.push_front( aVector );
	}
	if( (i<xSize-1) && (j>0) && (getMapDataValue(i+1, j-1) <= smallestValue))
	{ 
		smallestValue = getMapDataValue(i+1, j-1);
		angle.push_front( 315 );
		aVector.SetX( 1 );
		aVector.SetY( -1 );
		rVector.push_front( aVector );
	}
	if( (j>0) && (getMapDataValue(i, j-1) <= smallestValue))
	{       
		smallestValue = getMapDataValue(i, j-1);
		angle.push_front( 270 );
		aVector.SetX( 0 );
		aVector.SetY( -1 );
		rVector.push_front( aVector );
	}
	if( (i>0) && (j>0) && (getMapDataValue(i-1, j-1) <= smallestValue))
	{
		smallestValue = getMapDataValue(i-1, j-1);
		angle.push_front( 225 );
		aVector.SetX( -1 );
		aVector.SetY( -1 );
		rVector.push_front( aVector );
	}

	if( (smallestValue == -1) || (smallestValue == GTIP_INFINITY) || getMapDataValue(i, j) == 0)
	{
	    rVector.clear();
		aVector.SetX( 0 );
		aVector.SetY( 0 );
		rVector.push_front( aVector );
		return rVector;
	}
	else
		return rVector;
}

//set functions
int CMap::setSizeX( int sz )
{
	if( xSize == -1 )
	{
		if( resolution != -1) 
			xSize = (int)(sz/resolution);
		else
			xSize = sz;
		return 0;
	}
	else
		return -1;
}

int CMap::setSizeY( int sz )
{
	if( ySize == -1 )
	{
		if( resolution != -1) 
			ySize = (int)(sz/resolution);
		else
			ySize = sz;
		return 0;
	}
	else
		return -1;
}

int CMap::setResolution( double res, bool resetSize )
{
	if( resolution == -1 && res > 0 )
	{
		resolution = res;
		if( resetSize == true)
		{
			xSize = (int)(xSize/resolution);
			ySize = (int)(ySize/resolution);
		}
		return 0;
	}
	else
		return -1;
}

int CMap::setGoal(double i, double j)
{
    GlobalUtilities g;
    goal.setX((int)g.round(i));
    goal.setY((int)g.round(j));
    return 0;
}

int CMap::dangerousCopier( CMap* cFrom, CMap* cToo)
{
    //this is terribly written and NOT good coding style. 
    //I'll come back and fix it later though...

    cToo->setResolution( cFrom->getResolutionInMeters(), false);
    cToo->setSizeX( cFrom->getSizeX() );
    cToo->setSizeY( cFrom->getSizeY() );
    cToo->allocateMapMemory();
    cToo->addGoal( cFrom->getGoal().getX(), cFrom->getGoal().getY() );
    for( int i=0; i<cToo->getSizeX(); i++)
    {
        for( int j=0; j<cToo->getSizeY(); j++)
        {
            cToo->setMapDataValue(i,j,cFrom->getMapDataValue(i,j));
        }
    }
    return 1;
}

int CMap::convertToVectorMap()
{
    int n=0;
	int i=0;
	int j=0;
  
	if( (xSize < 1 ) || (ySize < 1))
		return -1;

	try
	{
	    vectorMap = new VectorConfidenceAdvice*[xSize];
       
		for (n=0;n<xSize;n++)
			vectorMap[n]=new VectorConfidenceAdvice[ySize];
	}
	catch(...)
	{
		for (n=0;n<xSize;n++)
			delete []vectorMap[n];

	    delete []vectorMap;
       
	  fprintf( stderr, "Could not allocation sufficient memory for the internal plan");
	  throw;
	}

    VectorConfidenceAdvice* zeroVector = new VectorConfidenceAdvice();

    for(i=0;i<xSize;i++)
	{
		for(j=0;j<ySize;j++)
			vectorMap[i][j] = *zeroVector;
	}

    VectorConfidenceAdvice* GTIP_INFINITY_VECTOR = new VectorConfidenceAdvice();
    GTIP_INFINITY_VECTOR->SetX( GTIP_INFINITY );
    GTIP_INFINITY_VECTOR->SetY( GTIP_INFINITY );
   
    VectorConfidenceAdvice* OBSTACLE_VECTOR = new VectorConfidenceAdvice();
    OBSTACLE_VECTOR->SetX( OBSTACLEVALUE );
    OBSTACLE_VECTOR->SetY( OBSTACLEVALUE );
    
    for(i=0;i<xSize;i++)
    {
        for(j=0;j<ySize;j++)
        {
            if( getMapDataValue(i,j) == GTIP_INFINITY )
                vectorMap[i][j] = *GTIP_INFINITY_VECTOR;
            else if (getMapDataValue(i,j) == OBSTACLEVALUE )
                vectorMap[i][j] = *OBSTACLE_VECTOR;
            else if (getMapDataValue(i,j) == 0 )
                vectorMap[i][j] = *zeroVector;
            else
            {
                VectorConfidenceAdvice v = getVector(i,j);
                vectorMap[i][j].SetX( v.GetX() );
                vectorMap[i][j].SetY( v.GetY() );
            }
        }
    }
    return 0;
}

int CMap::setMapVectorValue(int i, int j, VectorConfidenceAdvice vValue)
{
    if( vectorMap == NULL)
    {
        cout << "Error: Vector Map NULL" << endl;
        return -1;
    }
    else
    {
        vectorMap[i][j] = vValue;
    }
    return 0;
}

//get functions
double CMap::getSizeXInMeters() const
{
	return (double)xSize*resolution;
}

double CMap::getSizeYInMeters() const
{
	return (double)ySize*resolution;
}

int CMap::getSizeX() const
{
	return xSize;
}

int CMap::getSizeY() const
{
	return ySize;
}

double CMap::getResolutionInMeters() const
{
	return resolution;
}

void CMap::setMapID( string id )
{
	mapID = id;
}

string CMap::getMapID() const
{
	return mapID;
}

string CMap::getFilename() const
{
	int position = (int)mapID.find_first_of("-");
	return mapID.substr(0, position );
}

CCoordinate CMap::getGoal() const
{
	return goal;
}

void CMap::setBoundaryPoints( vector< CCoordinate > vBoundaryPts)
{
    vBoundaryPoints = vBoundaryPts;
}

vector< CCoordinate > CMap::getBoundaryPoints() const
{
    return vBoundaryPoints;
}

int CMap::addGoal( double x, double y )
{
  GlobalUtilities g;
    if( resolution != -1 )
    {
        x = x/resolution;
        y = y/resolution;

        int i = (int)g.round(x);
	    int j = (int)g.round(y);

        goal.setX(i);
        goal.setY(j);  

        return 0;
    }
    else
    {
        cout << "Error adding goal--negative resolution value" <<endl;
        return -1;
    }

}

int CMap::getMapType() const
{
	char* pEnd;
	int position = (int)mapID.find_first_of("-T");
	return (int)strtol(mapID.substr(position+2,1).c_str(), &pEnd,10 );	
}

int CMap::getMapLeftRightType()
{
	int position = (int)mapID.find_first_of("-T");
    string leftRightOrNiether = mapID.substr(position+3,1);
	if(leftRightOrNiether == "L" )
        return 1;
    else if(leftRightOrNiether == "R")
        return -1;
    else 
        return 0;
}

double CMap::getMapDataValue(int xPos, int yPos )
{ 
  if( (xPos >= xSize || xPos < 0 ) || (yPos >= ySize || yPos < 0) )
  {
    return -1;
  }
  else
    return mapCost[xPos][yPos];
}

void CMap::setMapDataValue(int xPos, int yPos, double value )
{
	mapCost[xPos][yPos] = value;
}

/*
	Member for adding obstacles to a map.
	The following functions add obstacle shapes to the plan. They're a little rough to follow. The general logic goes like 
	this: starting from a point round the point to get its approximate location on the plan. For a circle use the equation
	for a circle to determine all of the points on the plan that need to be marked as an obstacle. Fill in the circle as not 
	filling can cause bug. A wall uses the equation for a line to determine which points on the plan to mark. 
*/
 
void CMap::addObstacle(Shape obstacleShape, double firstPointX, double firstPointY, double secondPointX, double secondPointY)
{
    GlobalUtilities g;
    firstPointX = g.round(firstPointX);
    firstPointY = g.round(firstPointY);
    secondPointX = g.round(secondPointX);
    secondPointY = g.round(secondPointY);

	if( obstacleShape == CIRCLE )	
	{
	    double radius = secondPointX +.6;

		if( radius == 0 )
			return;
		else
		{ 
			// Use (x - x1)^2 + (y - y1)^2 = r^2
			//Given some x find the y value
			double rSquared = pow(radius,2);

			double i = firstPointX - radius;
			double y, yNeg;
			if (i <0 )
				i = 0;

			while( (i<=(firstPointX + radius)) && (i<xSize) )
			{
			  	double a = pow((i - firstPointX),2);
				if ( a > rSquared ) 
				{
				  a = rSquared;  
				}
				   
				y = sqrt( rSquared - a ) + firstPointY;

				//extreme -x and +x of a circle
				if((i != (firstPointX - radius)) && (i != (firstPointX + radius)) )
				{
					if( (firstPointY+radius) == y )
						yNeg = firstPointY-radius;
					else
						yNeg = (2 * firstPointY) - y;

					if( yNeg < 0 )
						yNeg = 0;

					makeAPointAnObstacle( i, yNeg );

					if( y-yNeg > 1 )
					{
					   for(double fill = yNeg; fill < y; fill++)
					   {
					//     fprintf(stderr, "FILL: %.01f y: %.01f yNeg: %.01f \n",fill,y,yNeg);
					       makeAPointAnObstacle(i,fill);
					   }
					}
				}

				if( y < ySize )
				{
					makeAPointAnObstacle( i, y );
				}
				
				if( radius < .5)
					i = i+ radius;
				else
					i = i +.5;
			}
		}
	}
	else if( obstacleShape == EXACTPOINT )
	{
		setMapDataValue( (int)firstPointX, (int)firstPointY, OBSTACLEVALUE );
	}
	else if( obstacleShape == WALL )
	{
		double i=0;
		double slope=0;
		double b;
		double incrementX = resolution;
		double incrementY = resolution;

		double deltaX = fabs( secondPointX - firstPointX );
		double deltaY = fabs( secondPointY - firstPointY );
		if( deltaX < .5 )	deltaX =0;
		if( deltaY < .5 )	deltaY =0;

		if( firstPointX > secondPointX )
			incrementX = -1*incrementX;

		if( firstPointY > secondPointY )
			incrementY = -1*incrementY;
	
		if( deltaX == 0 && deltaY == 0)
			makeAPointAnObstacle( firstPointX, firstPointY );
		else if( deltaX == 0)
		{
			if( firstPointY < secondPointY )
			{
				makeAPointAnObstacle( secondPointX, floor(firstPointY)+.1 );	//.1 is a temp hack
				for(i=firstPointY;i <= secondPointY;i=i+resolution)
					makeAPointAnObstacle( firstPointX, i );
			}
			else
			{
				makeAPointAnObstacle( secondPointX, floor(secondPointY)+.1 );
				for(i=secondPointY;i <= firstPointY;i=i+resolution)
					makeAPointAnObstacle( firstPointX, i );
			}
		}	
		else if( deltaY == 0)
		{
			if( firstPointX < secondPointX )
			{
				makeAPointAnObstacle( floor(firstPointX)+.1, secondPointY );	//.1 is a temp hack
				for(i=firstPointX;i <= secondPointX;i=i+resolution)
					makeAPointAnObstacle( i, firstPointY );
			}
			else
			{
				makeAPointAnObstacle( floor(secondPointX)+.1, secondPointY );
				for(i=secondPointX;i <= firstPointX ;i=i+resolution)
					makeAPointAnObstacle( i, firstPointY );
			}
		}
		//y = mx+b
		else 
		{	
			makeAPointAnObstacle( firstPointX, firstPointY );

			int firstPointYAsInt = (int)firstPointY;
			int secondPointYAsInt = (int)secondPointY;
			int firstPointXAsInt = (int)firstPointX;
			int secondPointXAsInt = (int)secondPointX;

			//y=mx+b
			slope = (secondPointY - firstPointY) / (secondPointX - firstPointX);
			b = firstPointY - slope * firstPointX;

			for(i=firstPointXAsInt; i!=(secondPointXAsInt+incrementX); )
			{
				makeAPointAnObstacle( i, (slope * i + b) );
				i = i + incrementX;
			}
			//
			for(i=firstPointYAsInt; i!=(secondPointYAsInt+incrementY); )
			{
				makeAPointAnObstacle( ((i-b)/slope), i );
				i = i + incrementY;
			}
		}
	}
}

void CMap::makeAPointAnObstacle(double x, double y)
{
    size_t i;
    vector<CCoordinate> rValue = convertAPointIntoIntoMapCoordinates(x,y);
	for( i=0;i<rValue.size();i++)
	  setMapDataValue(rValue[i].getX(),rValue[i].getY(),OBSTACLEVALUE);
    
  /*
	if( x <0 )	x = 0;
	if( y <0 )	y = 0;
	
	double fractionalPartX, fractionalPartY, intPartX, intPartY;
	fractionalPartX = modf((x/resolution), &intPartX);
	fractionalPartY = modf((y/resolution), &intPartY);

	if( fractionalPartX > .99 )
		intPartX += 1;
	
	if( fractionalPartY > .99 )
		intPartY += 1;

	int xAsInt = (int)intPartX;
	bool xEdge = false;
	if (xAsInt == x)
		xEdge = true;
	
	int yAsInt = (int)intPartY;
	bool yEdge = false;
	if( yAsInt == y )
		yEdge = true;

	if( xAsInt > xSize -1 )
		xAsInt = xSize -1;
	
	if( yAsInt > ySize -1 )
		yAsInt = ySize -1;

	mapCost[xAsInt][yAsInt] = OBSTACLEVALUE;	// point is in the middle

	if( (xEdge == true) && (xAsInt != 0))
		mapCost[xAsInt-1][yAsInt] = OBSTACLEVALUE;

	if( (yEdge == true) && (yAsInt != 0))
		mapCost[xAsInt][yAsInt-1] = OBSTACLEVALUE;

	if( (xEdge == true) && (yEdge == true) && (xAsInt != 0) && (yAsInt != 0))
		mapCost[xAsInt-1][yAsInt-1] = OBSTACLEVALUE;
  */
}

vector<CCoordinate> CMap::convertAPointIntoIntoMapCoordinates(double x, double y)
{
	//determine if x and y are int's. It so then make two discrete cells obstacle.
  vector< CCoordinate> rValue;

	if( x <0 )	x = 0;
	if( y <0 )	y = 0;
	
	double fractionalPartX, fractionalPartY, intPartX, intPartY;
	fractionalPartX = modf((x/resolution), &intPartX);
	fractionalPartY = modf((y/resolution), &intPartY);

	if( fractionalPartX > .99 )
		intPartX += 1;
	
	if( fractionalPartY > .99 )
		intPartY += 1;

	int xAsInt = (int)intPartX;
	bool xEdge = false;
	if (xAsInt == x)
		xEdge = true;
	
	int yAsInt = (int)intPartY;
	bool yEdge = false;
	if( yAsInt == y )
		yEdge = true;

	if( xAsInt > xSize -1 )
		xAsInt = xSize -1;
	
	if( yAsInt > ySize -1 )
		yAsInt = ySize -1;

	CCoordinate newCoord1(xAsInt,yAsInt );	    
	rValue.push_back(  newCoord1 );
 
	if( (xEdge == true) && (xAsInt != 0))
	{
	    CCoordinate newCoord(xAsInt-1,yAsInt );	    
	    rValue.push_back(  newCoord );
	}

	if( (yEdge == true) && (yAsInt != 0))
	{
	    CCoordinate newCoord(xAsInt, yAsInt-1);
	    rValue.push_back(  newCoord );
	}

	if( (xEdge == true) && (yEdge == true) && (xAsInt != 0) && (yAsInt != 0))
	{
	    CCoordinate newCoord(xAsInt-1, yAsInt-1);
		rValue.push_back(  newCoord );
	}

	return rValue;
}


void CMap::writeMapToFile( ofstream &dataFile, string strWriteType )
{
        dataFile << "BEGIN_MAP" << endl;
		dataFile << this->getMapID() << " " << endl;
		dataFile << this->getSizeX() << " " << endl;
		dataFile << this->getSizeY() << " " << endl;
		dataFile << this->getResolutionInMeters() << " " << endl; 

//		dataFile << "#Goals\n";

		CCoordinate goal = this->getGoal();
		
		dataFile << goal.getX() << " " << endl;
		dataFile << goal.getY() << " " << endl;
		 
        vector< CCoordinate > lBoundaryPoints = this->getBoundaryPoints();
        
        CCoordinate least; 
        CCoordinate most;

        least.setX( GTIP_INFINITY ); 
        least.setY( GTIP_INFINITY );
        most.setX( 0 );
        most.setY( 0 );

        int i;
		for( i=0; i<(signed)lBoundaryPoints.size(); i++ )
		{
            if( lBoundaryPoints[i].getX() < least.getX() )
                least.setX( lBoundaryPoints[i].getX() );

            if( lBoundaryPoints[i].getY() < least.getY() )
                least.setY( lBoundaryPoints[i].getY() );

            if( lBoundaryPoints[i].getX() > most.getX() )
                most.setX( lBoundaryPoints[i].getX() );

            if( lBoundaryPoints[i].getY() > most.getY() )
                most.setY( lBoundaryPoints[i].getY() );
        }

        if( least.getX() == GTIP_INFINITY ) 
            least.setX( 0 );

        if( least.getY() == GTIP_INFINITY ) 
            least.setY( 0 );

        if( most.getX() == 0 ) 
            most.setX( this->getSizeX() );

        if( most.getY() == 0 ) 
            most.setY( this->getSizeY() );

 //       dataFile << "#Boundaries 1) least-X 2) least-Y 3) most-X 4)most-Y"<<endl;
 //     dataFile <<least.getX()<<endl;
 //     dataFile <<least.getY()<<endl;
 //     dataFile <<most.getX()<<endl;
 //     dataFile <<most.getY()<<endl;

        dataFile << "#Data"<<endl;

        if( strWriteType == "FLOAT" )
        {
            float writebuf[220*220];       //MAP SIZE
            for(i=0; i<this->getSizeX() ; i++ )
		    {
			    for(int j=0; j<this->getSizeY(); j++)
			    {
                  //  unsigned char temp = (unsigned char)(this->getMapDataValue(i,j));
                    writebuf[i*220+j] = (float)(this->getMapDataValue(i,j));
			    }
		    }   
            dataFile.write((char*)writebuf,220*220*sizeof(float));  //MAP SIZE
            dataFile <<endl;
        }
        else
        {
            for(i=0; i<this->getSizeX(); i++ )
		    {
			    for(int j=0; j<this->getSizeY(); j++)
			    {

    				dataFile << this->getMapDataValue(i,j) << endl;
			    }
		    }
/*
            for(i=least.getX(); i<most.getX() ; i++ )
		    {
			    for(int j=least.getY(); j<most.getY(); j++)
			    {

    				dataFile << this->getMapDataValue(i,j) << " " << endl;
			    }
		    }
*/
        }

		dataFile << "END_MAP" <<endl;     
}


int CMap::IsObstacle( CCoordinate Position, int iSlack )
{
  if( Position.getX() > 0 && Position.getY() > 0 )
  {
     if( Position.getY() < this->getSizeX() && Position.getY() < this->getSizeY() )
	 {
	   int i,j;
	   for(i=-1*iSlack; i<iSlack; i++ )
	   {
		  for(j=-1*iSlack; j<iSlack; j++ )
		  {
              double rValue = getMapDataValue( Position.getX()+i, Position.getY()+j ); 
			  if( rValue  == OBSTACLEVALUE )
	              return 1;
		  }
       }
	   return 0;
	 }
  }
  return -1;
}

int CMap::IsInfinity( CCoordinate Position )
{
  if( Position.getX() > 0 && Position.getY() > 0 )
  {
     if( Position.getY() < this->getSizeX() && Position.getY() < this->getSizeY() )
	 {
	   
       double rValue = getMapDataValue( Position.getX(), Position.getY() ); 
	   if( rValue  == GTIP_INFINITY )
	      return 1;

	   return 0;
	 }
  }
  return -1;
}

vector< CCoordinate > CMap::GetPredictedLocations( int iLookAhead, CCoordinate currentPosition )
{
   vector< CCoordinate > rValue;
   int i;
   for( i=0;i<iLookAhead;i++)
   {
      CCoordinate nextCoord;
	  VectorConfidenceAdvice advice = getVector( (double)currentPosition.getX(), (double)currentPosition.getY());
	  nextCoord.setXY( (int)(currentPosition.getX()+ advice.GetX()), (int)(currentPosition.getY()+advice.GetY()) );
	  currentPosition = nextCoord;
	  rValue.push_back( nextCoord );
   }

   return rValue;
}
