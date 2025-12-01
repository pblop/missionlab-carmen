/**********************************************************************
 **                                                                  **
 **  polygon.cc                                                      **
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

/* $Id: polygon.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: polygon.cc,v $
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
#include "polygon.h"
#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
polygon::polygon()
{
}

/**********************************************************************/
// Constructor
polygon::polygon(Vectors _verticies) :
   verticies(_verticies)
{
   // Create the users extra data record.
   extra = polygon_extra::create(this);
}

/**********************************************************************/
// destructor
polygon::~polygon()
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
// Returns: A polygon sensor reading.
bool 
polygon::create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading)
{
   buf->type = POLYGON_OBJECT;

   // Copy the object to the sensor buffer
   VECTOR *v = new VECTOR[verticies.size()];
   Vectors::iterator it;
   int i=0;
   for(it=verticies.begin(); it!=verticies.end(); ++it)
   {
      Vector t(*it);

      // Make relative to the sensor
      t -= sensorLoc;
      t.rotateZ_degrees(-1 * heading);

      v[i].x = t.x;
      v[i].y = t.y;
      v[i].z = t.z;
      i++;
   }
   buf->SENSOR_READING_u.polygon_reading.verticies.verticies_val = v;
   buf->SENSOR_READING_u.polygon_reading.verticies.verticies_len = i;

   return true;
}

// *********************************************************************
/// move the object by the specified amount (meters).
void 
polygon::move_by(const Vector &displacement)
{
   Vectors::iterator it;
   for(it=verticies.begin(); it!=verticies.end(); ++it)
   {
      *it += displacement;
   }
}

// *********************************************************************
/// move the object to the specified location (meters).
void 
polygon::move_to(const Vector &location)
{
   // nothing for now.
}

// *********************************************************************
/// rotate the object by the specified amount (degrees).
void 
polygon::rotate_by(double roll, double pitch, double yaw)
{
   // nothing for now.
}

// *********************************************************************
/// rotate the object to the specified attitude (degrees).
void 
polygon::rotate_to(double roll, double pitch, double yaw)
{
   // nothing for now.
}

// *********************************************************************
// What is the distance from point x,y to the closest face of the object.
double 
polygon::distance(const Vector &sensorLoc)
{
/*
   Vector minPoint;
   bool foundMin = false;
   Vectors::iterator it;
   for(it=verticies.begin(); it!=verticies.end(); ++it)
   {
      if( !foundMin || minPoint.
      *it += displacement;
   }

   // Compute the distance.
   double dist = v.length() - r;

   // Handle special case where the sensor is inside the disk.
   if( dist <= 0 )
      return 0;

   // return distance from edge of disk to the source.
   return dist;
*/
   return 0.0;
}

/**********************************************************************/
// What is the center location of the object?
Vector 
polygon::center()
{
   // Use a rather stupid algorithm to determine the center of mass.
   // Just add the locations of the verticies and divide by the number of verticies.
   Vector center;
   Vectors::iterator it;
   for(it=verticies.begin(); it!=verticies.end(); ++it)
   {
      center += *it;
   }
   center /= (int)(verticies.size());

   return center;
}

/**********************************************************************/
}
