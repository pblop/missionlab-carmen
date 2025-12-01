#ifndef F16_H
#define F16_H
/**********************************************************************
 **                                                                  **
 **  F16.h                                                           **
 **                                                                  **
 **  Implement an F16-like robot object.                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: F16.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: F16.h,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:03  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.6  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.5  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.4  2002/12/23 22:23:05  doug
* added draw trails support
*
* Revision 1.3  2002/12/23 22:05:30  doug
* supporting show trails
*
* Revision 1.2  2002/11/14 19:44:03  doug
* F16 works
*
* Revision 1.1  2002/11/14 15:25:09  doug
* snapshot
*
**********************************************************************/

#include <deque>
#include "shape.h"

namespace sara
{
/**********************************************************************/
class F16 : public shape
{
public:
   /// Constructors
   F16();
//   F16(const Vector &location, double roll, double pitch, double yaw);

   /// destructor
   ~F16();

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

   /// rotate the sensor to the specified angle (degrees).
   void rotateSensor_to(double yaw);

   /// What is the distance from point x,y to the closest face of the object.
   double distance(const Vector &sensorLoc);

   /// What is the center location of the object?
   Vector center() {return loc;}

   /// grab updates to the sensor
   void setSensor(double yaw, bool on) {sensorYaw = yaw; sensorOn = on;}

   /// location of the F16
   Vector loc;

   /// attitude of the F16
   double roll;
   double pitch;
   double yaw;

   /// heading of the sensor, relative to the robot
   double sensorYaw;
   bool   sensorOn;

   /// Pointer to our parent record
   F16(class robot_link *parent);

   typedef Vectorq Trail_points;
   Trail_points *trail;
};

}
/**********************************************************************/
#endif

