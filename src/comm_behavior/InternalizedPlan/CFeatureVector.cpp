/**********************************************************************
 **                                                                  **
 **                    CFeatureVector.cpp		   			         **
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
 **********************************************************************/

#include "stdafx.h"
#include "CFeatureVector.h"

CRobotLocationCommunication::CRobotLocationCommunication(int numberOfRobots):dSignalStrength(numberOfRobots+1,-5) {}


CFeatureVector::CFeatureVector(int numberOfRobots, int robot_id)	
{
	for( int i=0;i<(numberOfRobots+1);i++)
	{
		CRobotLocationCommunication robotPointer(numberOfRobots);
		robotPointer.iRobot_id = i;
		commsList.push_back( robotPointer );
	}
	
	iRobotID = robot_id;
	iNumberOfRobots = numberOfRobots;
	Heading=0;
}

int CFeatureVector::CreateMapping(list<int> ltOrderedIDS)//given an ordered list of robot ids
{
  if( IDMap.empty() != true)
  {
	IDMap.erase( IDMap.begin(), IDMap.end());
  }

  if( (signed)ltOrderedIDS.size() != iNumberOfRobots )
  {
	iNumberOfRobots = (int)ltOrderedIDS.size(); //not sure about this
  }

  for(int i=1;i<(iNumberOfRobots+1);i++)
  {
    
    IDMap.insert(Map::value_type( i, ltOrderedIDS.front() ));
    ltOrderedIDS.pop_front();
  }

  return 0;
}

void CFeatureVector::printVector()
{ 
    cout << "FeatureVector contents: "<< endl; 
	cout << '\t' << "Number of Robots: " << iNumberOfRobots << endl;
	cout << '\t' << "Robot ID (vector): "<< iRobotID << endl;
	cout << '\t' << "Robot ID (real): "  << IDMap[iRobotID] << endl;
	cout << '\t' << "Data:" << endl;
	vector< CRobotLocationCommunication>::iterator fVectorIter = commsList.begin();
    fVectorIter++;
	for( ; fVectorIter != commsList.end(); fVectorIter++ )
	{
	  cout << '\t' << '\t' << "ID: "<<(*fVectorIter).iRobot_id << endl;
	  cout << '\t' << '\t' << "Location: ("<<(*fVectorIter).cLocation.getX() << ","<<(*fVectorIter).cLocation.getY()<< ")"<<endl;
	  cout << '\t' << '\t' << "Signal: "<< endl;
	  if( (*fVectorIter).iRobot_id == iRobotID)
	  {
		for( int i=1;i<(iNumberOfRobots+1);i++)
		{
		  cout << '\t' << '\t' << '\t' <<(*fVectorIter).dSignalStrength[i] << endl;
		}
	  }
	  else
	  {
		cout << '\t' << '\t' << '\t' <<"No data for connections of "<< IDMap[(*fVectorIter).iRobot_id] << " robot"<<endl;
	  }
	}
    cout <<endl;
  
}

double CFeatureVector::GetHeading() const
{
  return Heading;
}

int CFeatureVector::update( SensorData  sensordata)
{   
    Comm_Sensor_Data localComData = sensordata.GetCommData();

	Heading = sensordata.GetHeading();
    
    GlobalUtilities g;
	
	for( int i = 1; i < (iNumberOfRobots+1); i++ )
	{
	  commsList[i].cLocation.setX( (int)g.round(localComData.readings[IDMap[i]-1].v.x) );
	  commsList[i].cLocation.setY( (int)g.round(localComData.readings[IDMap[i]-1].v.y) );

	  if(iRobotID == localComData.readings[IDMap[i]-1].id )
	  {
		currentLocation.setX( (int)g.round(localComData.readings[IDMap[i]-1].v.x) );
		currentLocation.setY( (int)g.round(localComData.readings[IDMap[i]-1].v.y) );
		
		for( int j=1; j < (iNumberOfRobots+1); j++)
		{
		  commsList[i].dSignalStrength[j]= localComData.readings[IDMap[j]-1].signal_strength;
		}
	  }
	}
   
    return 0;
}

void CFeatureVector::clear()
{
  	vector<  CRobotLocationCommunication >::iterator commsListBeginIter =  commsList.begin();
	vector<  CRobotLocationCommunication >::iterator commsListEndIter = commsList.end();
   
	commsList.erase(commsListBeginIter, commsListEndIter ); 
}

vector< CRobotLocationCommunication > CFeatureVector::getCommsList()
{
  return commsList;
}

void CFeatureVector::setCommsList(vector< CRobotLocationCommunication > stCommsList)
{
    commsList = stCommsList;
}

void CFeatureVector::setNumberOfRobots( int num)
{
    iNumberOfRobots = num;
}

int CFeatureVector::getNumberOfRobots() const
{
    return iNumberOfRobots;
}

bool CFeatureVector::fullCommunication()
{
  for( int i = 1; i < (iNumberOfRobots+1); i++ )
  {
	for( int j=1; j < (iNumberOfRobots+1); j++)
	{
      if( ( i!=j ) && (commsList[i].dSignalStrength[j] == 0))
	  {  
		  return false;
	  }
	}
  }
 
  return true;
}

CCoordinate CFeatureVector::GetMyCurrentLocation()
{
    return currentLocation;
}

/**********************************************************************
 * $Log: CFeatureVector.cpp,v $
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
