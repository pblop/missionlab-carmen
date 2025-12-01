#ifndef UGV_H
#define UGV_H
/**********************************************************************
 **                                                                  **
 **  UGV.h                                                           **
 **                                                                  **
 **  Implement an UGV-like robot object.                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: UGV.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: UGV.h,v $
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
* Revision 1.1  2004/08/18 22:52:04  doug
* got ugv demo filmed
*
**********************************************************************/

#include "shape.h"

namespace sara
{
/**********************************************************************/
class UGV : public shape
{
public:
   /// Constructors
   UGV();

   /// destructor
   ~UGV();

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

   /// location of the UGV
   Vector loc;

   /// attitude of the UGV
   double roll;
   double pitch;
   double yaw;

   /// heading of the sensor, relative to the robot
   double sensorYaw;
   bool   sensorOn;

   /// Pointer to our parent record
   UGV(class robot_link *parent);

   typedef Vectorq Trail_points;
   Trail_points *trail;
};

}
/**********************************************************************/
#endif

