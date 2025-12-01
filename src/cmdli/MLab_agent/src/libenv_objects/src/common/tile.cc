/**********************************************************************
 **                                                                  **
 **  tile.cc                                                         **
 **                                                                  **
 **  Implement a tile-like environmental object.                     **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: tile.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: tile.cc,v $
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
* Revision 1.3  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.2  2002/11/15 22:39:27  doug
* 2d images work
*
* Revision 1.1  2002/11/14 22:34:15  doug
* working on photo loading
*
**********************************************************************/

#include "mic.h"
#include "Vector.h"
#include "tile.h"
#include <math.h>

namespace sara
{
/**********************************************************************/
// Constructor
tile::tile()
{
   // Create the users extra data record.
   extra = tile_extra::create(this);
}

/**********************************************************************/
// Constructor
tile::tile(Vectors _verticies) :
   verticies(_verticies)
{
   // Create the users extra data record.
   extra = tile_extra::create(this);
}

/**********************************************************************/
// destructor
tile::~tile()
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
// Returns: A tile sensor reading.
bool 
tile::create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading)
{
   return false;
}

// *********************************************************************
/// move the object by the specified amount (meters).
void 
tile::move_by(const Vector &displacement)
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
tile::move_to(const Vector &location)
{
   // nothing for now.
}

// *********************************************************************
/// rotate the object by the specified amount (degrees).
void 
tile::rotate_by(double roll, double pitch, double yaw)
{
   // nothing for now.
}

// *********************************************************************
/// rotate the object to the specified attitude (degrees).
void 
tile::rotate_to(double roll, double pitch, double yaw)
{
   // nothing for now.
}

// *********************************************************************
// What is the distance from point x,y to the closest face of the object.
double 
tile::distance(const Vector &sensorLoc)
{
   return 0.0;
}

/**********************************************************************/
// What is the center location of the object?
Vector 
tile::center()
{
   return Vector(0,0,0);
}

/**********************************************************************/
}
