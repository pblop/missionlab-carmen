/*********************************************************************
 **                                                                  **
 **                    CCalculateMapCosts.h                          **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2004 Georgia Tech Research Corporation         **
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

#ifndef CCALCULATEMAPCOSTS_H
#define CCALCULATEMAPCOSTS_H

#include "ProjectDefinitions.h"
#include "CMap.h"
#include "CCoordinate.h"


//using namespace std;

class CCalculateMapCosts
{	
public:
	CCalculateMapCosts(CMap*);
	CCalculateMapCosts(CMap*, CMap*);
	int performCalculations();
    int performProcessedCalculations( CCoordinate from, CCoordinate to);
    int performAlteredCalculationsLineV(int, int, int);
	int performAlteredCalculationsLineH(int, int, int);
	int growObstacle(CCoordinate, CCoordinate);

private:
	list< CCoordinate > getNeighbors(CCoordinate);
	bool goLeft( CCoordinate& );
	bool goRight( CCoordinate& );
	bool goUp( CCoordinate& );
	bool goDown( CCoordinate& );
	bool testValidCoordinate( CCoordinate );
	void removeDuplicateCoordinates( list< CCoordinate>& );

private:
	CMap* mapPointer;
	CMap* comPointer;
	CCoordinate mapGoal; 
};

#endif

/**********************************************************************
 * $Log: CCalculateMapCosts.h,v $
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
