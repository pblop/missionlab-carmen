#ifndef waves_H
#define waves_H
/**********************************************************************
 **                                                                  **
 **  waves.h                                                         **
 **                                                                  **
 **  Implement a waves-like environmental object.                    **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: waves.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: waves.h,v $
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
* Revision 1.2  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.1  2002/09/20 22:34:44  doug
* Initial revision
*
**********************************************************************/

#include "env_object.h"
#include "waves_extra.h"

/**********************************************************************/
namespace sara
{

class waves : public env_object
{
public:
   // wavess are not movable.
   bool is_movable() {return false;}

   // Constructors
   waves();
   waves(const Vectors &vertices, const string &color, const string &name);

   // destructor
   ~waves();

   // generate a sensor reading of the object relative to the robot's x,y,h.
   // Note: source_X, and source_Y are in world coordinates.
   //       The heading is in degrees CCW from Positive X.
   // Returns true if buf was loaded.
   bool create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading);

   // move the object by a displacement
   void moveby(const Vector &offset);

   // Replace the waves with the new verticies
   void replaceVerticies(Vectors verticies);

   // What is the distance from a point to the closest face of the object.
   double distance(const Vector &loc);

   // Change a label on the object
   void changeLabel(const string &newLabel);

   // What is the center location of the object?
   Vector center();

   /// Pointer to implementation of the extra stuff.
   class waves_extra *extra;

   // Change the color of an object
   // This would be in the env_object base record, but we need access to extra.
   void set_color(const string &new_color);

   // the waves
   Vectors verticies;
};
}
/**********************************************************************/
#endif

