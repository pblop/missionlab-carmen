/**********************************************************************
 **                                                                  **
 **                     CFeatureVector.h                             **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Messager class that shuttles sensory information to     **
 ** the planning behavior.                                           **
 **                                                                  **
 **********************************************************************/


#ifndef CFEATUREVECTOR_H
#define CFEATUREVECTOR_H


#include "ProjectDefinitions.h"
#include "CCoordinate.h"
#include "GlobalUtilities.h"
#include "SensorData.h"
#include <map>

class CMap;

typedef map<int, int, less< int > > Map;

class CRobotLocationCommunication			
{
public:
    CRobotLocationCommunication(int);
    int iRobot_id;
    CCoordinate cLocation;		
    vector< double > dSignalStrength;		//for each robot I need a list of the comms stats with all other robots
};

class CFeatureVector
{	 
public:
	CFeatureVector( int, int );
	void printVector();
	vector< CRobotLocationCommunication > getCommsList();
	void setCommsList(vector< CRobotLocationCommunication > );
    int CreateMapping( list<int> );
	int update(SensorData );
    void clear();
    void setNumberOfRobots( int );
    int getNumberOfRobots() const;
	bool fullCommunication();
    CCoordinate GetMyCurrentLocation(); 
	double GetHeading() const;

private:
	int iNumberOfRobots;
	int iRobotID;
	CCoordinate currentLocation;
    Map IDMap;
	double Heading;
	vector< CRobotLocationCommunication > commsList;	//the struct is not meant to hold a lot of data, or this member would be a pointer...
};

#endif 

/**********************************************************************
 * $Log: CFeatureVector.h,v $
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
