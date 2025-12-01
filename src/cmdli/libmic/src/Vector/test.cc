/**********************************************************************
 **                                                                  **
 **  test.cc                                                         **
 **                                                                  **
 **  Test harness for Vector                                         **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: test.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: test.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:55  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:30:29  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.1  2003/03/10 16:34:58  doug
* added test driver
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"

// *********************************************************************
int main()
{
   double sqrt2 = sqrt(2.0);

   // Test at 0
   Vector p1(1.0, 0.0, 0.0);
   Vector p2(1.0, 0.0, 0.0);
   double tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   double tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 0: " << tL << " should be 0: " << tR << endl;


   // Test at 45
   p1 = Vector(sqrt2, sqrt2, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 45: " << tL << " should be -45: " << tR << endl;

   // Test at 45
   p1 = Vector(100.0, 100.0, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 45: " << tL << " should be -45: " << tR << endl;


   // Test at 90
   p1 = Vector(77.0, -77.0, 0.0);
   p2 = Vector(-100.0, -100.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 90: " << tL << " should be -90: " << tR << endl;



   // Test at 90
   p1 = Vector(0.0, 1.0, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 90: " << tL << " should be -90: " << tR << endl;


   // Test at 135
   p1 = Vector(-1 * sqrt2, sqrt2, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 135: " << tL << " should be -135: " << tR << endl;


   // Test at 180
   p1 = Vector(-1.0, 0.0, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be 180: " << tL << " should be 180: " << tR << endl;


   // Test at 225
   p1 = Vector(-1 * sqrt2, -1 * sqrt2, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be -135: " << tL << " should be 135: " << tR << endl;


   // Test at 270
   p1 = Vector(0.0, -1.0, 0.0);
   p2 = Vector(1.0, 0.0, 0.0);
   tL = DEGREES_FROM_RADIANS(p1.dThetaZ(p2));
   tR = DEGREES_FROM_RADIANS(p2.dThetaZ(p1));
   cerr << "should be -90: " << tL << " should be 90: " << tR << endl;


}

// *********************************************************************
