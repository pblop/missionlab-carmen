/*********************************************************************
 **                                                                  **
 **                    CLoadMap.h                                    **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class loads a map. This is still under             **
 **	construction.													 **   	
 **********************************************************************/


#ifndef CLOADMAP_H
#define CLOADMAP_H

#include "ProjectDefinitions.h"
class CMap;
class CCoordinate;
class CPlan;

class CLoadMap
{	 
public:
	CLoadMap();
	int parseOVLFile( string );
	int parseOPARFile( string );
	int openOVL( string, CMap*, vector< CCoordinate>);
	int openOPAR( string, CMap* );
	int openCMM( string, CMap* );
	int openIPLANDAT( list< CMap* >& );
    int openPCF( string strPCFname, CPlan* aPlan);
    int openIPLANDAT( CPlan* aPlan);
    int LoadMap( CMap* pMap, ifstream& fStream, string);

private:
	string loadFileToString( string );
    CCoordinate getCoordinate(string s1);

};
	
#endif

/**********************************************************************
 * $Log: CLoadMap.h,v $
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
