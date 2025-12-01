#ifndef photo_H
#define photo_H
/**********************************************************************
 **                                                                  **
 **  photo.h                                                       **
 **                                                                  **
 **  Implement a photo-like environmental object.                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: photo.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: photo.h,v $
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
* Revision 1.4  2002/11/15 22:39:26  doug
* 2d images work
*
* Revision 1.3  2002/11/14 22:34:15  doug
* working on photo loading
*
* Revision 1.2  2002/10/02 15:51:48  doug
* moving to cvs
*
* Revision 1.1  2002/09/23 21:42:14  doug
* Initial revision
*
**********************************************************************/

#include "shape.h"
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class photo : public env_object
{
public:
   // photos are not movable.
   bool is_movable() {return false;}

   // Constructors
   photo(shape *theShape);
   photo(shape *theShape, const string &filename);
   photo(shape *theShape, const string &filename, const Vector origin, const double photoMetersPerPixel);

   // destructor
   ~photo();

   // generate a sensor reading of the object relative to the robot's x,y,h.
   // Note: source_X, and source_Y are in world coordinates.
   //       The heading is in degrees CCW from Positive X.
   // Returns true if buf was loaded.
   bool create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading);

   // move the object by a displacement
   void moveby(const Vector &offset);

   // Replace the photo with the new verticies
   void replaceVerticies(Vectors verticies);

   // What is the distance from a point to the closest face of the object.
   double distance(const Vector &loc);

   // Change a label on the object
   void changeLabel(const string &newLabel);

   // What is the center location of the object?
   Vector center();

   // Change the color of an object
   // This would be in the env_object base record, but we need access to extra.
   void set_color(const string &new_color);

   /// The filename
   string filename;

      /// the raw resolution of the image itself, in meters per pixel
      double photoMetersPerPixel;

      /// The orgin of the photo
      Vector photo_origin;

      /// Was the photo file successfully loaded?
      bool photoLoaded;

      /// Info from the photo file
      uint photo_width;
      uint photo_height;
      // LUMINANCE_8_DATA - 8 bit greyscale
      // RGB_24_DATA - 24 bit color, 1 byte per red, green, blue
      enum {LUMINANCE_8_DATA, RGB_24_DATA} photo_format;
      uchar *raw_data;

      /// Used to enable debug in the image manipulation code.
      bool debug;

   private:
      /// load the photo image file
      bool load_photo(const string &filename);
};

}
/**********************************************************************/
#endif

