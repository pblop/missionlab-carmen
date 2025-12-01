#ifndef tomahawk_H
#define tomahawk_H
/**********************************************************************
 **                                                                  **
 **  tomahawk.h                                                      **
 **                                                                  **
 **  Implement an tomahawk-like robot object.                        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: tomahawk.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: tomahawk.h,v $
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
* Revision 1.2  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2003/06/16 19:48:46  doug
* Adding Tomahawk
*
**********************************************************************/

#include <deque>
#include "shape.h"

/**********************************************************************/
namespace sara
{
class tomahawk : public shape
{
public:
   /// Constructors
   tomahawk();
//   tomahawk(const Vector &location, double roll, double pitch, double yaw);

   /// destructor
   ~tomahawk();

   /// generate a sensor reading of the object relative to the robot's x,y,h.
   /// Note: source_X, and source_Y are in world coordinates.
   ///       The heading is in degrees CCW from Positive X.
   /// Returns true if buf was loaded.
   bool create_reading(SENSOR_READING *buf, const Vector &loc, double heading);

   /// Extend the visible trail to the specified location (meters).
   /// Prune the oldest segments to keep the trail no longer than 'max_length' 
   /// segments.  A max_length value of 0 deletes the trail.  A max_length 
   /// value of -1 leaves the length unlimited.
   void trail_to(const Vector &location, uint max_length);

   /// move the object by the specified amount (meters).
   void move_by(const Vector &distance);

   /// move the object to the specified location (meters).
   void move_to(const Vector &location);

   /// rotate the object by the specified amount (degrees).
   void rotate_by(double roll, double pitch, double yaw);

   /// rotate the object to the specified attitude (degrees).
   void rotate_to(double roll, double pitch, double yaw);

   /// What is the distance from point x,y to the closest face of the object.
   double distance(const Vector &sensorLoc);

   /// What is the center location of the object?
   Vector center() {return loc;}

   /// location of the tomahawk
   Vector loc;

   /// attitude of the tomahawk
   double roll;
   double pitch;
   double yaw;

   /// Pointer to our parent record
   tomahawk(class robot_link *parent);

   typedef Vectorq Trail_points;
   Trail_points *trail;
};
}
/**********************************************************************/
#endif

