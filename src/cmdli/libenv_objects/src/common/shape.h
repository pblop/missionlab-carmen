#ifndef shape_H
#define shape_H
/**********************************************************************
 **                                                                  **
 **  shape.h                                                         **
 **                                                                  **
 **  abstract base class for object shapes                           **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-2002.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Livonia, Michigan, USA                                          **
 **                                                                  **
 **********************************************************************/

/* $Id: shape.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: shape.h,v $
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
* Revision 1.7  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.6  2003/11/26 15:21:47  doug
* back from GaTech
*
* Revision 1.5  2002/12/23 22:05:30  doug
* supporting show trails
*
* Revision 1.4  2002/11/14 19:44:03  doug
* F16 works
*
* Revision 1.3  2002/11/05 21:57:48  doug
* gaining
*
* Revision 1.2  2002/11/04 22:34:34  doug
* snapshot
*
* Revision 1.1  2002/11/01 22:44:34  doug
* snapshot
**********************************************************************/

#include "mic.h"
#include "ipc_defs.h"
#include "Vector.h"
#include "env_object.h"
#include "shape_extra.h"

/**********************************************************************/
namespace sara
{
/// abstract base class for environmental shapes
class shape
{
public:
   /// Constructor
   shape();

   /// destructor
   virtual ~shape() {};

   /// Called to set the env_object parent pointer
   void setParent(class env_object *envParent) {parent = envParent;}

   /// generate a sensor reading of the object relative to the robot's x,y,h.
   /// Note: source_X, and source_Y are in world coordinates.
   ///       The heading is in degrees CCW from Positive X.
   /// Returns true if buf was loaded.
   virtual bool create_reading(SENSOR_READING *buf, const Vector &loc, double heading) = 0;

   /// move the object by the specified amount (meters).
   virtual void move_by(const Vector &distance) = 0;

   /// Extend the visible trail to the specified location (meters).
   /// Prune the oldest segments to keep the trail no longer than 'max_length' 
   /// segments.  A max_length value of 0 deletes the trail.  A max_length 
   /// value of -1 leaves the length unlimited.
   virtual void trail_to(const Vector &location, uint max_length) {};

   /// move the object to the specified location (meters).
   virtual void move_to(const Vector &location) = 0;

   /// rotate the object by the specified amount (degrees).
   virtual void rotate_by(double roll, double pitch, double yaw) = 0;

   /// rotate the object to the specified attitude (degrees).
   virtual void rotate_to(double roll, double pitch, double yaw) = 0;

   /// set sensor 
   virtual void setSensor(double yaw, bool on) {};

   /// What is the distance from point x,y to the closest face of the object.
   virtual double distance(const Vector &sensorLoc) = 0;

   /// What is the center location of the object?
   virtual Vector center() = 0;

   /// Pointer to implementation of the extra stuff.
   class shape_extra *extra;

   /// Pointer to our env_object record
   env_object *parent;
};
}
/**********************************************************************/
#endif

