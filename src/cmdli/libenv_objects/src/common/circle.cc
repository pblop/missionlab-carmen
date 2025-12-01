/**********************************************************************
 **                                                                  **
 **  circle.cc                                                       **
 **                                                                  **
 **  Implement a circle-like environmental object.                   **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: circle.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: circle.cc,v $
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
* Revision 1.5  2002/11/14 19:44:03  doug
* F16 works
*
* Revision 1.4  2002/11/05 17:05:46  doug
* builds again
*
* Revision 1.3  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.2  2002/09/20 22:34:44  doug
* *** empty log message ***
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "circle.h"
#include "circle_extra.h"
//#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
circle::circle()
{
}

/**********************************************************************/
// Constructor
circle::circle(const Vector &center, double radius) :
   loc(center),
   r(radius)
{
   // Create the users extra data record.
   extra = circle_extra::create(this);
}

/**********************************************************************/
// destructor
circle::~circle()
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
// Returns: A circle sensor reading.
bool 
circle::create_reading(SENSOR_READING *buf, const Vector &sourceLoc, double heading)
{
   buf->type = DISK_OBJECT;
   buf->SENSOR_READING_u.disk_reading.r = r;

   // Create a vector from the source to the center of the circle.
   Vector v(loc);
   v -= sourceLoc;

   // Rotate it to compensate for the heading.
   v.rotateZ_degrees(-1 * heading);

   // Return the egocentric vector.
   buf->SENSOR_READING_u.disk_reading.Vx = v.x;
   buf->SENSOR_READING_u.disk_reading.Vy = v.y;

   return true;
}

// **********************************************************************
/// move the object by the specified amount (meters).
void 
circle::move_by(const Vector &distance)
{
   loc += distance;
}

// **********************************************************************
/// move the object to the specified location (meters).
void 
circle::move_to(const Vector &location)
{
   loc = location;
}

// **********************************************************************
/// rotate the object to the specified attitude (degrees).
void 
circle::rotate_to(double roll, double pitch, double yaw)
{
   // I don't think that applies here.
}

// **********************************************************************
/// rotate the object by the specified amount (degrees).
void 
circle::rotate_by(double roll, double pitch, double yaw)
{
   // I don't think that applies here.
}

// **********************************************************************
// What is the distance from point x,y to the closest face of the object.
double 
circle::distance(const Vector &sensorLoc)
{
   Vector v = loc - sensorLoc;

   // Compute the distance.
   double dist = v.length() - r;

   // Handle special case where the sensor is inside the circle.
   if( dist <= 0 )
      return 0;

   // return distance from edge of circle to the source.
   return dist;
}

/**********************************************************************/
}
