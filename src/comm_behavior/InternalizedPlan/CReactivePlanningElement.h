/*********************************************************************
 **                                                                  **
 **                     CReactivePlanningElement.h                   **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2004 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Abstract interface for all types of planning elements   **
 **                                                                  **
 **********************************************************************/

#ifndef CREACTIVE_PLANNING_ELEMENT_H
#define CREACTIVE_PLANNING_ELEMENT_H

#include "ProjectDefinitions.h"
#include "VectorConfidenceAdvice.h"

class CFeatureVector;
class CCoordinate;
class CMap;

class CReactivePlanningElement
{	 
public:                                                       //no need to implement here
    virtual VectorConfidenceAdvice getAdviceVector(CFeatureVector*,int)         =0;
    virtual	CCoordinate getElementGoal() const                                  =0;
	virtual string getElementType() const                                       =0;
	virtual vector< CMap* > GetMaps() const                                     =0;
	virtual int getElementNumber() const                                        =0;
    virtual int getLineNumbersForEdit() const                                   =0;
	virtual bool GetElementOverride() const                                     =0;  

    virtual int editLineNumbers( int lineNumber )                               =0;
    virtual void writePlanToConfigFile(ofstream& fStream, int stepNumber)       =0; 
    virtual int generateInternalizedPlanData( string )                          =0;
    virtual int writeMaps(ofstream& fStream, string writeType)                  =0;
    virtual int readMaps(ifstream& fStream, string writeType)                   =0;

    virtual void SetNumberOfRobots( int number )                                =0;
	virtual void setElementGoal( CCoordinate goal )                             =0;
	virtual void setElementNumber( int )                                        =0;
	virtual void print(bool, int, int&)                                         =0;
};

#endif

/**********************************************************************
 * $Log: CReactivePlanningElement.h,v $
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
