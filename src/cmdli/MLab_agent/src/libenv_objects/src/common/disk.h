#ifndef disk_H
#define disk_H
/**********************************************************************
 **                                                                  **
 **  disk.h                                                          **
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

/* $Id: disk.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: disk.h,v $
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
* Revision 1.9  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.8  2002/11/01 21:49:41  doug
* working on it
*
* Revision 1.7  2002/09/20 22:34:44  doug
* *** empty log message ***
*
* Revision 1.1  2002/09/20 13:05:19  doug
* Initial revision
*
**********************************************************************/

#include "env_object.h"
#include "disk_extra.h"

/**********************************************************************/
namespace sara
{

class disk : public env_object
{
public:
   // Constructor
   disk();

   // Constructor
   disk(const Vector &center, double radius, uint red, uint green, uint blue);
   disk(const Vector &center, double radius, uint red, uint green, uint blue, const strings &label);

   // destructor
   ~disk();

   // generate a sensor reading of the object relative to the robot's x,y,h.
   // Note: source_X, and source_Y are in world coordinates.
   //       The heading is in degrees CCW from Positive X.
   // Returns true if buf was loaded.
   bool create_reading(SENSOR_READING *buf, const Vector &loc, double heading);

   // displace the object by x,y,z meters.
   void moveby(const Vector &displacement);

   // What is the distance from point x,y to the closest face of the object.
   double distance(const Vector &sensorLoc);

   // What is the center location of the object?
   Vector center();

   /// Pointer to implementation of the extra stuff.
   class disk_extra *extra;

   // Center of the disk
   Vector loc;

   // radius
   double r;

   // friend records.
   friend class opcon_extra_disk;
};

}
/**********************************************************************/
#endif

