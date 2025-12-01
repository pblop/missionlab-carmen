/**********************************************************************
 **                                                                  **
 **  waves.cc                                                         **
 **                                                                  **
 **  Implement a waves-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: waves.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: waves.cc,v $
* Revision 1.1.1.1  2008/07/14 16:44:16  endo
* MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
*
* Revision 1.1.1.1  2006/07/20 17:17:46  endo
* MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
*
* Revision 1.1.1.1  2006/07/12 13:37:54  endo
* MissionLab 7.0
*
* Revision 1.1  2006/07/01 00:21:05  endo
* CMDLi from MARS 2020 migrated into AO-FNC repository.
*
* Revision 1.1.1.1  2006/06/29 20:42:07  endo
* cmdli local repository.
*
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.1  2002/09/20 22:34:44  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "waves.h"
#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
/*
waves::waves()
{
}
*/

/**********************************************************************/
// Constructor
/*
waves::waves(const Vectors &_verticies, const string &color, const string &label) :
   env_object(color, label),
   verticies(_verticies)
{
   // Create the users extra data record.
   extra = waves_extra::create(this);
   extra->changeLabel(label);
}
*/
/**********************************************************************/
// destructor
waves::~waves()
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
// Returns: A waves sensor reading.
bool 
waves::create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading)
{
/*
   buf->type = waves_OBJECT;
   buf->SENSOR_READING_u.waves_reading.id = id;
   buf->SENSOR_READING_u.waves_reading.color = const_cast<char *>(get_color().c_str());

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
   buf->SENSOR_READING_u.waves_reading.verticies.verticies_val = v;
   buf->SENSOR_READING_u.waves_reading.verticies.verticies_len = i;
*/
   return true;
}

/**********************************************************************/
// displace the object by x,y meters.
void 
waves::moveby(const Vector &displacement)
{
   for(uint i=0; i<verticies.size(); i++)
   {
      verticies[i] += displacement;
   }
}

/**********************************************************************/
// What is the distance from point x,y to the closest face of the object.
double 
waves::distance(const Vector &sensorLoc)
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
waves::center()
{
   // Use a rather stupid algorithm to determine the center of mass.
   // Just add the locations of the verticies and divide by the number of verticies.
   Vector center; // will be zero
   for(uint i=0; i<verticies.size(); i++)
   {
      center += verticies[i];
   }
   center /= verticies.size();

   return center;
}

/**********************************************************************/
// Change the color of an object
// This would be in the env_object base record, but we need access to extra.
void  
waves::set_color(const string &new_color)
{  
   // Pass it on up the inheritance chain.
   env_object::set_color(new_color);

   // Pass on the new color to our extra stuff.
   if( extra )
   {
      extra->set_color(new_color);
   }
}; 

/**********************************************************************/
// Change a label on the object
// This would be in the env_object base record, but we need access to extra.
void 
waves::changeLabel(const string &newLabel)
{
   // Pass on the new label to our extra stuff.
   if( extra )
   {
      extra->changeLabel(newLabel);
   }
}

/**********************************************************************/
}
