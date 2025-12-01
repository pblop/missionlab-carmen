/**********************************************************************
 **                                                                  **
 **  photo.cc                                                      **
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

/* $Id: photo.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: photo.cc,v $
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
* Revision 1.6  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.5  2003/07/11 20:19:42  doug
* cleanup debug messages
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

#include "mic.h"
#include <fstream>
#include "Vector.h"
#include "photo.h"

namespace sara
{
/**********************************************************************/
// Constructor
photo::photo(shape *theShape) :
   env_object(theShape)
{
}

/**********************************************************************/
// Constructor
photo::photo(shape *theShape, const string &_filename) :
   env_object(theShape),
   filename( _filename),
   photoMetersPerPixel(4.0),
   photo_origin(0, 0, 0),
   photoLoaded(false),
   debug(false)
{
   // load the image file
   if( !load_photo(filename) )
   {
      stringstream out;
      out << "Image file '" << filename << "' not found!" << endl;
      ERROR(out.str().c_str());
   }
}

/**********************************************************************/
// Constructor
photo::photo(shape *theShape, const string &_filename, const Vector origin, const double _photoMetersPerPixel) :
   env_object(theShape),
   filename( _filename),
   photoMetersPerPixel(_photoMetersPerPixel),
   photo_origin(origin),
   photoLoaded(false),
   debug(false)
{
   // load the image file
   if( !load_photo(filename) )
   {
      cerr << "Image file '" << filename << "' not found!" << endl;
   }
}

/**********************************************************************/
// destructor
photo::~photo()
{
   if( photoLoaded )
   {
      delete [] raw_data;
      raw_data = NULL;
      photoLoaded = false;
   }
}

/**********************************************************************/
// generate a sensor reading of the object relative to the robot's x,y,h.
// Note: source_X, and source_Y are in world coordinates.
//       The heading is in degrees CCW from Positive X.
// Returns: A photo sensor reading.
bool 
photo::create_reading(SENSOR_READING *buf, const Vector &sensorLoc, double heading)
{
   return false;
}

/**********************************************************************/
// displace the object by x,y meters.
void 
photo::moveby(const Vector &displacement)
{
}

/**********************************************************************/
// What is the distance from point x,y to the closest face of the object.
double 
photo::distance(const Vector &sensorLoc)
{
   return 0.0;
}

/**********************************************************************/
// What is the center location of the object?
Vector 
photo::center()
{
   Vector center;
   return center;
}

/**********************************************************************/
// Change the color of an object
// This would be in the env_object base record, but we need access to extra.
void  
photo::set_color(const string &new_color)
{  
}; 

/**********************************************************************/
// Change a label on the object
// This would be in the env_object base record, but we need access to extra.
void 
photo::changeLabel(const string &newLabel)
{
}

// *********************************************************************
// internal function to read a number from a raw input stream.
static int readint(std::ifstream &infile)
{
   char buf[256];
   char ch;
   int  p=0;

   while( !infile.eof() && infile.read(&ch, 1) && infile.gcount() == 1 )
   {
      if( ch >= '0' && ch <= '9' )
      {
         // Found the start of the number.
	 buf[p++] = ch;

	 while( !infile.eof() && 
	         infile.read(&ch, 1) && 
		 infile.gcount() == 1 && 
		 isdigit(ch) &&
		 p < 255 )
         {
	    buf[p++] = ch;
         }
         buf[p] = '\0';
	 return atoi(buf);
      }
      else if( ch == '#' )
      {
         // Comment line, skip to end of line.
	 while( !infile.eof() && 
	         infile.read(&ch, 1) && 
		 infile.gcount() == 1 && 
		 ch != '\n');
      }
   }

   return 0;
}

// **********************************************************************
/// load the photo image file
bool
photo::load_photo(const string &filename)
{
   // Open the PGM photo file.
//std::ifstream infile(filename.c_str(), ios::binary);
   std::ifstream infile(filename.c_str());
   if( !infile )
   {
      WARN("Error opening photo file '%s'", filename.c_str());
      return false;
   }

   // Read the first line of the header.
   char buf[256];
   if( !infile.read(buf, 2) || infile.gcount() != 2 )
   {
      WARN("Error reading the photo file header: '%s'", filename.c_str());
      return false;
   }

   // Extract the file's data encoding: P2=ASCII, P5=binary.
   if( buf[0] != 'p' && buf[0] != 'P' )
   {
      WARN("Invalid photo file header: '%s'", filename.c_str());
      return false;
   }

   int bytes_per_pixel = 1;
   bool binary_data = true;
   if( buf[1] == '2' )
   {
      photo_format = LUMINANCE_8_DATA;
      bytes_per_pixel = 1;
      binary_data = false;
      if( debug )
         INFORM("Loading ASCII photofile: '%s'", filename.c_str());
   }
   else if( buf[1] == '5' )
   {
      photo_format = LUMINANCE_8_DATA;
      bytes_per_pixel = 1;
      binary_data = true;
      if( debug )
         INFORM("Loading greyscale photofile: '%s'", filename.c_str());
   }
   else if( buf[1] == '6' )
   {
      photo_format = RGB_24_DATA;
      bytes_per_pixel = 3;
      binary_data = true;
      if( debug )
         INFORM("Loading RGB photofile: '%s'", filename.c_str());
   }
   else
   {
      WARN("Invalid data type in photo file header: '%s'", filename.c_str());
      return false;
   }

   photo_width = readint(infile);
   if( photo_width < 100 || photo_width > 10000)
   {
      WARN("Unable to read width from photo file header (%u): '%s'", photo_width, filename.c_str());
      return false;
   }

   photo_height = readint(infile);
   if( photo_height < 100 || photo_height > 10000)
   {
      WARN("Unable to read height from photo file header (%u): '%s'", photo_height, filename.c_str());
      return false;
   }

   int photo_max_grey = readint(infile);
   if( photo_max_grey < 8 || photo_max_grey > 255)
   {
      WARN("Unable to read max_grey from photo file header (%u): '%s'  Expecting 8-255", photo_max_grey, filename.c_str());
      return false;
   }

   if( debug )
   {
      INFORM("Photo %s: width=%u pixels, height=%u pixels, max_grey=%u\n",
	       filename.c_str(), photo_width, photo_height, photo_max_grey);
   }
		         
   // allocate memory to hold the raw data.
   ulong photo_size = photo_width * photo_height * bytes_per_pixel;
   raw_data = new uchar[photo_size];

   // now read the photo data 
   if (binary_data) 
   {
      // read the binary data into our buffer.
      if( !infile.read((char *)raw_data, photo_size) )
      {
         WARN("Unable to read data from photo file: '%s'", filename.c_str());
         delete [] raw_data;
   	 return false;
      }

      if( infile.gcount() != (int)photo_size )
      {
         WARN("Only read %d bytes of %d in the photo file: '%s'", 
		filename.c_str(), infile.gcount(), photo_size);
         delete [] raw_data;
   	 return false;
      }

      if( debug )
      {
         INFORM("Photo %s: read %d bytes\n", filename.c_str(), photo_size);
      }
   }
   else
   {
      WARN("Haven't tested ASCII photo file reading: '%s'", filename.c_str());

      // Read it in ASCII format
      for (uint i=0; i<photo_size; i++)
      {
         if( !infile.get( ((char *)raw_data)[i] ) )
	 {
            WARN("Only read %d bytes of %d in the photo file: '%s'", filename.c_str(), i, photo_size);
            delete [] raw_data;
   	    return false;
         }
      }
   }

   // Make a check to see that we got everything.
   char scrap;
   infile.read(&scrap, 1);
   if( !infile.eof() )
   {
      WARN("Extra stuff at end of photo file: '%s'", filename.c_str());
   }

   // Close the photo file.
   infile.close();

   // mark we loaded it.
   photoLoaded = true;

   return true;
}

// *********************************************************************


}
