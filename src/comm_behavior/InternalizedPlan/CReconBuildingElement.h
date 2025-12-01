/**********************************************************************
 **                                                                  **
 **                     CReconBuildingElement.h                      **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Reconnaisance planning element... and a work in         **
 ** progress...                                                      **
 **                                                                  **
 **********************************************************************/

#ifndef CRECON_BUILDING_ELEMENT_H
#define CRECON_BUILDING_ELEMENT_H

#include "ProjectDefinitions.h"
#include "CReactivePlanningElement.h"
#include "VectorConfidenceAdvice.h"

class CMap;
class CFeatureVector;
#include "CCoordinate.h"

enum COMM_STAT{ COMMS_OK, COMMS_BAD_LEFT, COMMS_BAD_RIGHT };
enum PHASE{ SETUP,	ATTEMPT, LEFTSWEEP, RIGHTSWEEP, CLEANUP, FINISH };

class CReconBuildingElement:public CReactivePlanningElement
{	 
public:
	CReconBuildingElement();
	CReconBuildingElement(vector< CCoordinate >, list< CMap*> , int, vector< CCoordinate >, vector< CCoordinate >, CCoordinate);
	virtual ~CReconBuildingElement();
	virtual VectorConfidenceAdvice getAdviceVector(CFeatureVector*, int);
	
    virtual void        print(bool bNumbered, int iOutput, int& iItemNumber);
    virtual int         editLineNumbers( int lineNumber );
    virtual void        writePlanToConfigFile(ofstream& fStream, int stepNumber);
    virtual int         generateInternalizedPlanData( string );
    virtual int         writeMaps(ofstream& fStream, string writeType);  
    virtual int         readMaps(ifstream& fStream, string writeType);
	virtual int         getLineNumbersForEdit() const                       {return iLineNumbersForEdit;}
    
	virtual void        setElementGoal( CCoordinate cGoal )					{goal = cGoal;}
	virtual void        setElementNumber( int iElNum)						{iElementNumber = iElNum;}
    virtual void        SetNumberOfRobots( int number )						{iNumberOfRobots=number;}       
	virtual int         getElementNumber()          const					{return iElementNumber;}
    virtual CCoordinate getElementGoal()			const					{return goal;}
	virtual string      getElementType()			const					{return sElementType;}
    virtual int         GetNumberOfRobots()         const                   {return iNumberOfRobots;}
    virtual vector< CMap* > GetMaps()				const;

	void setBoundaryBox( vector<CCoordinate> vBBox )						{vBoundaryBox = vBBox;}
	void setProgressBoxLeft( vector<CCoordinate> vPBoxLeft )				{vProgressBoxLeft = vPBoxLeft;}
	void setProgressBoxRight( vector<CCoordinate> vPBoxRight )				{vProgressBoxRight = vPBoxRight;}
	void setLeftFourRobotPointGoal( CCoordinate lfGoal );
	void setRightFourRobotPointGoal( CCoordinate rfGoal );
	void setPreprocessStartLeft( CCoordinate preStartLeft )					{preprocessStartLeft = preStartLeft;	}
	void setPreprocessEndLeft( CCoordinate preEndLeft )						{preprocessEndLeft = preEndLeft;}
	void setPreprocessStartRight( CCoordinate preStartRight )				{preprocessStartRight = preStartRight;}
	void setPreprocessEndRight( CCoordinate preEndRight )					{preprocessEndRight = preEndRight;}
	void setGrowObstacleLeft( vector<CCoordinate> obsGrowLeft )				{growObstacleLeft = obsGrowLeft;}	
	void setGrowObstacleRight( vector<CCoordinate> obsGrowRight )			{growObstacleRight = obsGrowRight;}

	vector<CCoordinate> getBoundaryBox() const								{return vBoundaryBox;}
	vector<CCoordinate> getProgressBoxLeft() const							{return vProgressBoxLeft;}
	vector<CCoordinate> getProgressBoxRight() const							{return vProgressBoxRight;}
    CCoordinate getPreprocessStartLeft() const								{return preprocessStartLeft;}
    CCoordinate getPreprocessEndLeft() const								{return preprocessEndLeft;}
    CCoordinate getPreprocessStartRight() const								{return preprocessStartRight;}
    CCoordinate getPreprocessEndRight() const								{return preprocessEndRight;}

	bool GetElementOverride() const											{return bElementOverride;}

private:

    bool IsWithinProgressBox(CCoordinate position, vector< CCoordinate > vProgressBox ) const;

private:
    CMap* CommunicationsMapLeft;
    CMap* CoverageMapLeft;
    CMap* CommunicationsMapRight;
    CMap* CoverageMapRight;
    CMap* leftFourRobotPoint;
    CMap* rightFourRobotPoint; 

	int iNumberOfRobots;
	vector<double> dGains;
    double dPreviousMultiplier;
    double dPreviousMultiplier2;

	int iGoToCommsContingencyPlan;
	int iPauseBetweenSteps;

    bool bCommsWentDown;
	int iProgressPoint;
	CCoordinate goal;
	string sElementType;
	bool bElementOverride;
	CCoordinate locationCounter;
	int iTimeout;
	
	vector<CCoordinate> vProgressBoxLeft;
    vector<CCoordinate> vProgressBoxRight;
	vector<CCoordinate> vBoundaryBox;

	CCoordinate preprocessStartLeft;
	CCoordinate preprocessEndLeft;
	CCoordinate preprocessStartRight;
	CCoordinate preprocessEndRight;

	vector<CCoordinate> growObstacleLeft;
	vector<CCoordinate> growObstacleRight;

	int iElementNumber;
    int iLineNumbersForEdit;
};

#endif

/**********************************************************************
 * $Log: CReconBuildingElement.h,v $
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
