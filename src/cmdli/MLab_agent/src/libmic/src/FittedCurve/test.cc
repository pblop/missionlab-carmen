/**********************************************************************
 **                                                                  **
 **  test.cc                                                         **
 **                                                                  **
 **  Test harness for FittedCurve                                    **
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
* Revision 1.7  2003/03/10 16:31:34  doug
* FittedCurve is closer
*
* Revision 1.6  2003/03/03 13:44:23  doug
* seems to work
*
* Revision 1.5  2002/09/06 13:16:52  doug
* *** empty log message ***
*
* Revision 1.4  2002/08/29 17:06:10  doug
* *** empty log message ***
*
* Revision 1.3  2002/08/29 02:23:37  doug
* *** empty log message ***
*
* Revision 1.2  2002/08/27 18:58:09  doug
* *** empty log message ***
*
* Revision 1.1  2002/08/26 19:51:34  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "FittedCurve.h"
#include <math.h>

// *********************************************************************
int main()
{
   // Create a new FittedCurve
   FittedCurve ft;

   // call its test procedure.
//   ft.test();
//exit(0);
   // Test with up on left and down on right.
   int numSegments = 100;
   double segLength = 1;
   Vector p1(0.0, 0.0, 0.0);
   Vector h1(0.0, 1.0, 0.0);
   Vector p2(20.0, 0.0, 0.0);
   Vector h2(0.0, 1.0, 0.0);

   Vector heading = h1;
   Vector location = p1;

//cout << "firstPoint = " << p1 << endl;

   Vector firstPoint = ft.initCurve(numSegments, segLength, p1, p2, h1, h2);
//cout << "nextPoint = " << firstPoint << endl;
/*
   heading = firstPoint - location;
   location = firstPoint;

   double distance = firstPoint.length();
   for(int i=1; i<numSegments; i++)
   {
//cout << "computeNextPoint(location=" << location << ", heading=" << heading << endl;
      Vector nextPoint = ft.computeNextPoint(location, heading);

cout << "nextPoint = " << nextPoint << endl;

 Vector v(nextPoint - location);
 distance += v.length();

      heading = nextPoint - location;
      location = nextPoint;
   }
*/
   double desired = segLength * numSegments;
//   cout << "Final path: Length= " << distance << " desired=" << desired << " Error=" << fabs((distance-desired) / desired) * 100 << endl;
}

// *********************************************************************
