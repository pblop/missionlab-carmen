/**********************************************************************
 **                                                                  **
 **  BUD.cc                                                          **
 **                                                                  **
 **  Implement a BUD-like environmental object.                      **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: BUD.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: BUD.cc,v $
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
* Revision 1.1  2004/10/27 19:58:24  doug
* Added new BUD robot type, including steered robot simulator in simserver
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "BUD.h"
#include "BUD_extra.h"

namespace sara
{
/**********************************************************************/
// Constructor
BUD::BUD() :
   roll(0),
   pitch(0),
   yaw(0),
   sensorYaw(0),
   sensorOn(false),
   trail(NULL)
{
   // Create the users extra data record.
   extra = BUD_extra::create(this);
}

/**********************************************************************/
// destructor
BUD::~BUD()
{
   // Delete the extra data
   if( extra )
   {
      // Delete the users extra data record.
      delete extra;
      extra = NULL;
   }
}

/**********************************************************************/
// generate a sensor reading of the object relative to the robot's x,y,h.
// Note: source_X, and source_Y are in world coordinates.
//       The heading is in degrees CCW from Positive X.
// Returns: A BUD sensor reading.
bool 
BUD::create_reading(SENSOR_READING *buf, const Vector &sourceLoc, double heading)
{
/*
   buf->type = DISK_OBJECT;
   buf->SENSOR_READING_u.disk_reading.r = r;

   // Create a vector from the source to the center of the BUD.
   Vector v(loc);
   v -= sourceLoc;

   // Rotate it to compensate for the heading.
   v.rotateZ_degrees(-1 * heading);

   // Return the egocentric vector.
   buf->SENSOR_READING_u.disk_reading.Vx = v.x;
   buf->SENSOR_READING_u.disk_reading.Vy = v.y;

   return true;
*/
   return false;
}

// **********************************************************************
/// move the object by the specified amount (meters).
void 
BUD::move_by(const Vector &distance)
{
   loc += distance;
}

// **********************************************************************
/// Extend the visible trail to the specified location (meters).
/// Prune the oldest segments to keep the trail no longer than 'max_length' 
/// segments.  A max_length value of 0 deletes the trail.  A max_length 
/// value of -1 leaves the length unlimited.
void 
BUD::trail_to(const Vector &location, uint max_length)
{
   // First case: max_length == 0 means delete the trail
   if( max_length == 0 )
   {
      // do we have a trail?
      if( trail )
      {
         delete trail;
	 trail = NULL;
      }
   }
   // Second case: add to the trail
   else
   {
      // do we have a trail?
      if( !trail )
      {
         trail = new Trail_points;
      }

      trail->push_back(location);
      if( max_length > 0 )
      {
         while( trail->size() > max_length )
	 {
            trail->erase( trail->begin() );
	 }
      }
   }
}

// **********************************************************************
/// move the object to the specified location (meters).
void 
BUD::move_to(const Vector &location)
{
   loc = location;
}

// **********************************************************************
/// rotate the object by the specified amount (degrees).
void 
BUD::rotate_by(double _roll, double _pitch, double _yaw)
{
   roll += _roll;
   pitch += _pitch;
   yaw += _yaw;
}

// **********************************************************************
/// rotate the object to the specified attitude (degrees).
void 
BUD::rotate_to(double _roll, double _pitch, double _yaw)
{
   roll = _roll;
   pitch = _pitch;
   yaw = _yaw;
}

// **********************************************************************
/// rotate the sensor to the specified angle (degrees).
void 
BUD::rotateSensor_to(double yaw)
{
   sensorYaw = yaw;
}

// **********************************************************************
// What is the distance from point x,y to the closest face of the object.
double 
BUD::distance(const Vector &sensorLoc)
{
   Vector v = loc - sensorLoc;

   // Compute the distance.
   double dist = v.length();

   // Handle special case where the sensor is inside the BUD.
   if( dist <= 0 )
      return 0;

   // return distance from edge of BUD to the source.
   return dist;
}

/**********************************************************************/
}
