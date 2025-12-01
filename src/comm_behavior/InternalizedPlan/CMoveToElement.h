/**********************************************************************
 **                                                                  **
 **                         CMoveToElement.h                         **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: Move to planning element... and a work in               **
 ** progress...                                                      **
 **                                                                  **
 **********************************************************************/

#ifndef CMOVETOELEMENT_H
#define CMOVETOELEMENT_H

#include "ProjectDefinitions.h"
#include "CReactivePlanningElement.h"
#include "CMap.h"
#include "CCoordinate.h"
#include "CFeatureVector.h"
#include "VectorConfidenceAdvice.h"

class CMoveToElement:public CReactivePlanningElement
{	 
public:
	CMoveToElement();
	CMoveToElement(CMap* contigecnyMap);

	virtual ~CMoveToElement();
	virtual VectorConfidenceAdvice getAdviceVector(CFeatureVector*, int);
    virtual	CCoordinate getElementGoal() const;
	virtual string getElementType() const;
    virtual void setElementGoal( CCoordinate eGoal )         {goal=eGoal;}
	virtual void setElementNumber( int );
	virtual int getElementNumber() const;
    virtual void print(bool bNumbered, int iOutput, int& iItemNumber);
    virtual int getLineNumbersForEdit() const;
    virtual int editLineNumbers( int lineNumber );
    virtual void writePlanToConfigFile(ofstream&, int);
    virtual int generateInternalizedPlanData( string );
    virtual int writeMaps(ofstream& fStream, string writeType);  
    virtual int readMaps(ifstream& fStream, string writeType);
    virtual vector< CMap* > GetMaps() const;
	virtual bool GetElementOverride() const                  { return bElementOverride;} 

    void    SetNumberOfRobots(int nm )          {iNumberOfRobots = nm;}    
    void    SetContigencyLocation(CMap* cmp)    {contigencyPlan=cmp;}

    CMap*   GetContingencyLocation()    const   {return contigencyPlan;}
    int     GetNumberOfRobots()         const   {return iNumberOfRobots;}


	

private:
	CMap* completionPoint;
	CMap* contigencyPlan;
	CCoordinate goal;
	bool bElementOverride;
	const string sElementType;
	int iNumberOfRobots;
	int iElementNumber;
    int iLineNumbersForEdit;
	CompletionType iCompletionType; 
};

#endif

/**********************************************************************
 * $Log: CMoveToElement.h,v $
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
