/**********************************************************************
 **                                                                  **
 **                     VectorConfidenceAdvice.h                     **
 **                                                                  **
 **                                                                  **
 **  Written by:  Alan Wagner                                        **
 **                                                                  **
 **  Copyright 2002 - 2006 Georgia Tech Research Corporation         **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 ** Purpose: This class is a standard return type for all behaviors. **
 **                                                                  **
 **                                                                  **
 **********************************************************************/


#ifndef VECTORCONFIDENCEADVICE_H
#define VECTORCONFIDENCEADVICE_H

#include <math.h>

class VectorConfidenceAdvice
{	 
public:
    VectorConfidenceAdvice()                                         {x=0; y=0; dConfidence=0;}
	VectorConfidenceAdvice(double iX,double iY)                      {x=iX; y=iY; dConfidence=0;}
	VectorConfidenceAdvice(double iX,double iY,double dConf)         {x=iX; y=iY; dConfidence=dConf;}
    
    double     GetX() const                                          {return x;}
    double     GetY() const                                          {return y;}
    void       SetX( double iX)                                      {x=iX;}
	void       SetY( double iY)                                      {y=iY;}
	void       SetXY( double iX, double iY)                          {x=iX; y=iY;}
    void       SetConfidence( double dConf )                         {dConfidence=dConf;}

    double     GetConfidence() const                                 {  if( dConfidence > 1)
                                                                            return 1;
                                                                        else if( dConfidence < 0 )
                                                                            return 0;
                                                                        else
                                                                            return dConfidence;
                                                                     }


	void       GetPolar( double& dMagnitude, double& dAngle )        {   dMagnitude = sqrt( x*x + y*y );
                                                                         if( x !=0 )
                                                                            dAngle = atan(y/x);
                                                                         else
                                                                            dAngle =0;
                                                                     }   

private:
	double x;
	double y;
	double dConfidence;                                     //confidence ranges from Max of 1 to Min 0
    
};

#endif

/**********************************************************************
 * $Log: VectorConfidenceAdvice.h,v $
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


