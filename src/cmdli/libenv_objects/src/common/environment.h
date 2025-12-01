#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H
/**********************************************************************
 **                                                                  **
 **  environment.h                                                   **
 **                                                                  **
 **  The simulated robot environment                                 **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: environment.h,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

/**********************************************************************
* $Log: environment.h,v $
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
* Revision 1.11  2004/05/11 19:34:39  doug
* massive changes to integrate with USC and GaTech
*
* Revision 1.10  2004/03/10 14:37:57  doug
* add the guard back in for linux
*
* Revision 1.9  2004/03/08 14:52:28  doug
* cross compiles on visual C++
*
* Revision 1.8  2003/11/03 22:43:53  doug
* adding defComposite support
*
* Revision 1.7  2003/11/03 16:41:56  doug
* working on libcmdl
*
* Revision 1.6  2003/03/15 19:46:04  doug
* working on opcon
*
* Revision 1.5  2002/10/30 21:08:13  doug
* need to zero the width and height fields
*
* Revision 1.4  2002/09/23 21:42:14  doug
* *** empty log message ***
*
* Revision 1.3  2002/09/20 13:05:19  doug
* *** empty log message ***
*
* Revision 1.2  1999/03/08 23:35:49  doug
* moved to new library
*
* Revision 1.1  1998/06/16 18:10:12  doug
* Initial revision
*
**********************************************************************/

#include "mic.h"
#ifndef WIN32
#include "mutex.h"
#endif
#include "env_object.h"

/**********************************************************************/
namespace sara
{

class environment
{
   public:
      // constructor
      environment();

      // destructor
      ~environment() {};

      // Load the simulated environment
      // Returns 0 if success
      //        -1 if parse error, but some of the table is loaded
      //        -2 if unable to open file
      int load_environment(const std::string &filename, bool no_path_search, bool verbose, bool debug);

      /// Add an object to the environment
      bool addObject(class env_object *obj);

      /// Return the number of objects in the environment
      uint size() const {return objects.size();}

      /// Get a reference to the named object or return NULL if not found
      /// The user must not delete the returned pointer
      class env_object *getObject(const std::string &name) const;

      /// convience type
      typedef std::vector<class env_object *> env_objects; 

      /// The object container and its accessor guard
      env_objects objects;
#ifndef WIN32
      mutex objects_guard;
#endif

      /// The name of the scenario
      std::string scenario_name;

      /// The name of the site the overlay defines
      std::string site_name;

      /// Define the origin of the overlay in Meters and Degrees
      double origin_x, origin_y, origin_heading;

      /// Define the width and height of the overlay in Meters
      double width, height;

      /// lat/lon latitude and longitude passed in as decimal degrees are converted to utm coordinates
      static void convert_latlon_to_utm(double lat, double lon, double &utmx, double &utmy);

   private:
      /// decimal to dd.mmsssssssss 
      static double dec_to_dms(double dec);

      /// separate dd,mm,ss
      static void parsedeg(double dec_in,double *deg,double *min,double *sec);

      /// distance from meridian 
      static void meridiandist(double latrad, double *m);

      /// Track objects by name (at least the ones that are labeled)
      /// maps a name to an index into the "objects" container
      /// WARNING: THE MAPPING DOESN'T CURRENTLY GET UPDATED IF SOMEONE CHANGES THE LABEL AFTER INSERTION
      typedef std::map<std::string, int> objectsByName_T;
      objectsByName_T objectsByName;
};

}
/**********************************************************************/
#endif

