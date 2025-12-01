/*********************************************************************
 **                                                                  **
 **                     CPlan.h                                       **
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


#ifndef CPLAN_H
#define CPLAN_H

#include "ProjectDefinitions.h"
#include "CMap.h"
#include "CFeatureVector.h"

#include "CReactivePlanningElement.h"
#include "CReconBuildingElement.h"
#include "CMoveToElement.h"
#include "VectorConfidenceAdvice.h"
#include "SensorData.h"

//maybe the construction process should happen here...

enum PlanType	{ ORDERED, LEAST_COMMITMENT };

struct ElementPair
{
    CReactivePlanningElement* firstElement;
    CReactivePlanningElement* secondElement;
};

class CPlan
{	 
public:
    CPlan();
    ~CPlan();

	CMap*       GetCurrentPlanStep(int);
    VectorConfidenceAdvice      GetAdvice(/*CFeatureVector*, int, CCoordinate*/);
    int         UpdateFeatureVector( SensorData& );


	bool        CheckIfElementIsComplete();
    bool        CheckIfAtElementGoal(); 
	ElementPair*    RetrieveElementOfLeastDistance();
    void        GroupElementsIntoPairs();
    bool        CheckIfNearLocation( CCoordinate currentLocation, CCoordinate goalLocation, double dAcceptableDistance );

    void        SetPlanID(string id)                    {planID=id;}        
    void        SetTethered(bool bT)                    {bTethered=bT;}
    void        SetMapFileName(string nm )              {mapFileName=nm;}
    void        SetContigencyMap(CMap* mp)              {planContigencyMap=mp;}
    void        SetPlanType(PlanType tp)                {planType=tp;}
    void        SetPlanElements( vector< CReactivePlanningElement* > el )   {planSteps=el;}
    void        SetContigencyLocation( CCoordinate cl)  {contingencyLoc=cl;}
    void        SetThisRobotsIDNumber( int id )         {iRobotID=id;} 
    void        SetFeatureVector( CFeatureVector* cf )  {currentSensoryData=cf;}
    void        SetNumberOfRobotsUsingThisPlan( int ir );
    void        SetPlanData( int iNumberUsingPlan, int iID);

    string      GetPlanID()             const       {return planID;}
	bool        GetTethered()           const       {return bTethered;}
	string      GetMapFileName()        const       {return mapFileName;}
    CMap*       GetContingencyMap()     const       {return planContigencyMap;}
    int         GetNumberOfPlanSteps()  const       {return (int)planSteps.size();}
    PlanType    GetPlanType()           const       {return planType;}
    int         GetNumberOfRobotsUsingPlan() const  {return iNumberOfRobots;}
    CCoordinate GetContigencyLocation() const       {return contingencyLoc;}
    int         GetThisRobotsIDNumber() const       {return iRobotID;}
    CFeatureVector*     GetFeatureVector() const    {return currentSensoryData;}
	CReactivePlanningElement* GetPlanStep(int i);
    
    //plan creation functions
    void    print(bool bNumbered, int iOutput);
    void    editLineNumber( int iLineNumber );
    int     writePlanToConfigFile(string filename);
    int     generateInternalizedPlanData();
    int     writeInternalizedPlanData( string );
    int     LoadPlanFromDataFile();

private:

	void RotateEgoCentric(VectorConfidenceAdvice& inVector,double heading); 
	
	vector< CReactivePlanningElement* >     planSteps;
	CReactivePlanningElement*               currentElement;
	vector< ElementPair* >                  vPairsOfElements;
	ElementPair*                            currentPair;
    CMap*                                   planContigencyMap;
    CFeatureVector*                         currentSensoryData; 

    int     iNumberOfRobots;
    int     iRobotID;
	
	int     iPlanStepIndex;
    size_t  iPlanSize;
	string  mapFileName;
	string  planID;
	bool    bTethered;
	PlanType planType; 
    int     iLineNumbersForEdit;
    CCoordinate contingencyLoc;
	int iTransitionPause;
    
};

#endif 
