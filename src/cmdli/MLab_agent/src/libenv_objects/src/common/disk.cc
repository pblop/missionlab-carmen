/**********************************************************************
 **                                                                  **
 **  disk.cc                                                         **
 **                                                                  **
 **  Implement a disk-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: disk.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: disk.cc,v $
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
* Revision 1.12  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.11  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.10  2004/03/01 00:44:03  doug
* correctly set displaylabel so don't display ones ending in '*'
*
* Revision 1.9  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.8  2003/07/11 20:19:42  doug
* cleanup debug messages
*
* Revision 1.7  2002/11/01 21:49:41  doug
* working on it
*
* Revision 1.6  2002/09/20 22:34:44  doug
* *** empty log message ***
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
* Revision 1.4  1999/03/10 00:22:28  doug
* moved extra to here from env_object
*
* Revision 1.3  1998/06/16 18:09:43  doug
* *** empty log message ***
*
* Revision 1.2  1998/06/15 21:38:09  doug
* *** empty log message ***
*
* Revision 1.1  1998/06/15 21:11:39  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "disk.h"
#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
/*
disk::disk()
{
}
*/
/**********************************************************************/
// Constructor
/*
disk::disk(const Vector &initialLoc, double sr, uint red, uint green, uint blue) :
   env_object(uint red, uint green, uint blue),
   loc(initialLoc),
   r(sr) 
{
   // Create the users extra data record.
   extra = disk_extra::create(this);
}
*/
/**********************************************************************/
/*
disk::disk(const Vector &center, double radius, uint red, uint green, uint blue, const strings &name) :
   env_object(uint red, uint green, uint blue, name),
   loc(center),
   r(radius)
{
   // Create the users extra data record.
   extra = disk_extra::create(this);
}
*/
/**********************************************************************/
// destructor
disk::~disk()
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
// Returns: A disk sensor reading.
bool 
disk::create_reading(SENSOR_READING *buf, const Vector &sourceLoc, double heading)
{
/*
   buf->type = DISK_OBJECT;
   buf->SENSOR_READING_u.disk_reading.id = id;
   buf->SENSOR_READING_u.disk_reading.color = const_cast<char *>(get_color().c_str());
   buf->SENSOR_READING_u.disk_reading.r = r;

   // Create a vector from the source to the center of the disk.
   Vector v(loc);
   v -= sourceLoc;

   // Rotate it to compensate for the heading.
   v.rotateZ_degrees(-1 * heading);

   // Return the egocentric vector.
   buf->SENSOR_READING_u.disk_reading.Vx = v.x;
   buf->SENSOR_READING_u.disk_reading.Vy = v.y;
*/
	return true;
}

/**********************************************************************/
// displace the object by x,y meters.
void 
disk::moveby(const Vector &displacement)
{
//   loc += displacement;
}

/**********************************************************************/
// What is the distance from point x,y to the closest face of the object.
double 
disk::distance(const Vector &sensorLoc)
{
/*
   Vector v = loc - sensorLoc;

   // Compute the distance.
   double dist = v.length() - r;

   // Handle special case where the sensor is inside the disk.
   if( dist <= 0 )
      return 0;

   // return distance from edge of disk to the source.
   return dist;
*/
   ERROR("disk::distance - not implemented!");
   return 0.0;
}

/**********************************************************************/
// What is the center location of the object?
Vector 
disk::center()
{
   return loc;
}

/**********************************************************************/
// Change the color of an object
// This would be in the env_object base record, but we need access to extra.
/*
void  
disk::set_color(const string &new_color)
{  
   // Pass it on up the inheritance chain.
   env_object::set_color(new_color);

   // Pass on the new color to our extra stuff.
   if( extra )
   {
      extra->set_color(new_color);
   }
}; 
*/

/**********************************************************************/
// Change a label on the object
// This would be in the env_object base record, but we need access to extra.
/*
void 
disk::changeLabel(const string &newLabel)
{
   // Pass on the new label to our extra stuff.
   if( extra )
   {
      extra->changeLabel(newLabel);
   }
}
*/
/**********************************************************************/
}
