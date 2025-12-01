/*********************************************************************
 **                                                                  **
 **                     CMap.h                                       **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class represents an internal map. The array        **
 ** "mapCost" is of primal importance. This array stores the cost    **
 ** associated with travel to a coordinate.                          **
 ** constructors.                                                    **
 **                                                                  **
 **********************************************************************/


#ifndef CMAP_H
#define CMAP_H

#include "ProjectDefinitions.h"
#include "CCoordinate.h"
#include "VectorConfidenceAdvice.h"

class CMap
{	 
public:
	CMap();
	~CMap();

	double getSizeXInMeters() const;
	double getSizeYInMeters() const;
	double getResolutionInMeters() const;
	int getSizeX() const;
	int getSizeY() const;
	int setSizeX(int);
	int setSizeY(int); 
	int setResolution(double, bool);

	double getMapDataValue(int,int);
	void setMapDataValue(int,int,double);

	int getMapType() const;
	void setMapID( string id );
	string getMapID() const;

	string getFilename() const;
	
	int addGoal( double, double );
    CCoordinate getGoal() const;
    int setGoal( double, double );
	
	VectorConfidenceAdvice getVector(double,double);
	double getAngle(double, double);

	list< VectorConfidenceAdvice > getAllVector(double,double);
	list< double > getAllAngle(double, double);

    int dangerousCopier( CMap*, CMap* );
    int convertToVectorMap();
	int setMapVectorValue(int, int, VectorConfidenceAdvice);
    
    int getMapLeftRightType();

    void setBoundaryPoints( vector< CCoordinate > );
    vector< CCoordinate > getBoundaryPoints() const;

    void printVectorMap();
	void printMap();
	void printObstacles();
	void printGoal();
	void printFilename();
	
	void makeAPointAnObstacle(double, double);
	void addObstacle(Shape, double, double, double, double);
	int allocateMapMemory();
    void writeMapToFile( ofstream &dataFile, string strWriteType );

	int IsObstacle( CCoordinate Position, int Slack );
	int IsInfinity( CCoordinate Position );
    vector< CCoordinate > GetPredictedLocations( int iLookAhead, CCoordinate currentPosition );

	vector<CCoordinate> convertAPointIntoIntoMapCoordinates(double x, double y);

private:
	list<VectorConfidenceAdvice> getMapAdvice( double, double, list<double> );


private:
	int xSize;
 	int ySize;
	CCoordinate offset;
	CCoordinate goal;
	double** mapCost;
    VectorConfidenceAdvice** vectorMap;
	string mapID;
	double resolution; //meters
    vector< CCoordinate > vBoundaryPoints;
};

#endif

/**********************************************************************
 * $Log: CMap.h,v $
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
