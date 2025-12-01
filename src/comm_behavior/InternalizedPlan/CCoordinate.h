/*********************************************************************
 **                                                                  **
 **                     CCoordinate.h                                **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class represents a coordinate on an internal map.  **
 ** It consists of several setter and getter functions, operators,   **
 ** constructors.                                                    **
 **                                                                  **
 **********************************************************************/


#ifndef CCOORDINATE_H
#define CCOORDINATE_H

#include "ProjectDefinitions.h"


class CCoordinate
{	 
public:
	CCoordinate();
	CCoordinate(int,int);
	CCoordinate(int,int,double);

public:
	int     getX() const;
	int     getY() const;
	void    setX( int );
	void    setY( int );
	void    setXY( int, int);
	double  getDistance() const;
	void    setDistance( double dValue );

	bool operator== ( const CCoordinate & );
	bool operator!= ( const CCoordinate & );
	bool operator<  ( const CCoordinate & );
	bool operator>  ( const CCoordinate & );
	bool operator<= ( const CCoordinate & );
	bool operator>= ( const CCoordinate & );

private:
	int x;
	int y;
	double distance;
};

#endif

/**********************************************************************
 * $Log: CCoordinate.h,v $
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
