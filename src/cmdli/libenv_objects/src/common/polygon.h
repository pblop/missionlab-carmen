#ifndef polygon_H
#define polygon_H
/**********************************************************************
 **                                                                  **
 **  polygon.h                                                       **
 **                                                                  **
 **  Implement a polygon-like environmental object.                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: polygon.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: polygon.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:04  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.5  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.4  2002/11/14 19:44:03  doug
* F16 works
*
* Revision 1.3  2002/11/13 17:15:50  doug
* objectives work!
*
* Revision 1.2  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "shape.h"
#include "polygon_extra.h"

/**********************************************************************/
namespace sara
{

class polygon : public shape
{
public:
   // polygons are not movable.
   bool is_movable() {return false;}

   // Constructors
   polygon();
   polygon(Vectors verticies);

   // destructor
   ~polygon();

   // generate a sensor reading of the object relative to the robot's x,y,h.
   // Note: source_X, and source_Y are in world coordinates.
   //       The heading is in degrees CCW from Positive X.
   // Returns true if buf was loaded.
   bool create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading);

   /// move the object by the specified amount (meters).
   void move_by(const Vector &distance);

   /// move the object to the specified location (meters).
   void move_to(const Vector &location);

   /// rotate the object by the specified amount (degrees).
   void rotate_by(double roll, double pitch, double yaw);

   /// rotate the object to the specified attitude (degrees).
   void rotate_to(double roll, double pitch, double yaw);

   // Replace the polygon with the new verticies
   void replaceVerticies(Vectors verticies);

   // What is the distance from a point to the closest face of the object.
   double distance(const Vector &loc);

   // What is the center location of the object?
   Vector center();

   // Center of the polygon
   Vectors verticies;
};
}
/**********************************************************************/
#endif

