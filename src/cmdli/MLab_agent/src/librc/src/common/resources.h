#ifndef RESOURCES_H
#define RESOURCES_H

/**********************************************************************
 **                                                                  **
 **  resources.h                                                     **
 **                                                                  **
 **  the resources object                                            **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: resources.h,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: resources.h,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.2  2006/07/11 10:09:54  endo
// Compiling error fixed.
//
// Revision 1.1  2006/07/01 00:34:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.13  2004/10/29 22:38:33  doug
// working on waittimeout
//
// Revision 1.12  2004/05/11 19:34:31  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.11  2003/09/16 16:59:17  doug
// added check_exists helper function
//
// Revision 1.10  2003/09/16 16:54:54  doug
// added set helper function
//
// Revision 1.9  2003/09/16 16:44:12  doug
// added get_string and set_string helper functions
//
// Revision 1.8  2002/12/23 20:21:37  doug
// added get/set int and set_double.  Also changed get_double to use reference instead of pointer
//
// Revision 1.7  2002/10/10 20:51:31  doug
// added set_boolean to allow updating the value of a flag
//
// Revision 1.6  2002/10/10 20:22:10  doug
// Added save capabilities to the resource library
//
// Revision 1.5  1997/12/10 12:14:51  doug
// *** empty log message ***
//
// Revision 1.4  1997/12/08 14:58:47  doug
// *** empty log message ***
//
// Revision 1.3  1997/12/01 19:05:06  doug
// added print
//
// Revision 1.2  1997/12/01 15:38:07  doug
// works
//
// Revision 1.1  1997/12/01 12:44:04  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include "mic.h"
#include <string>
#include <vector>
#include <map>

namespace sara
{
    // Define the type of our symbol table container
    typedef std::map<std::string, strings, std::less<std::string> > res_T;

class resources
{
   public:
      // Create an empty resource object
      resources();

      // Destroy a resource object
      ~resources();

      // restore a resource object from the specified file
      // Returns 0 if success
      //         1 if parse error, but some of the table is loaded
      //         2 if unable to open file
      int load(const string &filename, 
        	bool no_path_search = false,    // Just check local dir      
		bool verbose = false,           // verbose mode
		bool debug = false);            // debug yacc

      // write the current resource object to the specified file
      // the filename must be fully qualified.
      // the existing file is renamed with a ~ extension if backup_files is true.
      // Returns true if success
      //         false if unable to write the file
      bool save(const string &filename, bool backup_files);

      // Get the value of a boolean flag
      // Returns 0 if false
      //         1 if true
      //        -1 if not specified
      //        -2 if value is not "true" or "false"
      int check_bool(const string &name) const;

      // set the value of a boolean flag
      // Adds the resource if it does not exist
      void set_bool(const string &name, bool value);

      // Get the value of an integer resource
      //        false if not specified
      //        true if value is loaded
      bool get_int(const string &name, int &d) const;

      // set the value of an integer resource
      // Adds the resource if it does not exist
      void set_int(const string &name, int value);

      // Get the value of a symbol as a double
      // Returns 
      //        false if not specified
      //        true if value is loaded
      bool get_double(const string &name, double &d) const;

      // set the value of a double resource
      // Adds the resource if it does not exist
      void set_double(const string &name, double value);

      // Get the value of a symbol as a string
      // Returns 
      //        false if not specified
      //        true if value is loaded
      bool get_string(const string &name, string &value) const;

      // set the value of a string resource
      // Adds the resource if it does not exist
      void set_string(const string &name, const string &value);

      // Check the value of a symbol
      // Returns 1 if value is found
      //         0 if value not found
      //        -1 if keyword not specified
      int check_value(const string &key, const string &value) const;

      // Set the value of a resource 
      // Adds the resource if it does not exist
      void set(const string &key, const strings &values);                     

      // Get the value of a symbol 
      // Returns 1 if value is found
      //         0 if value not found 
      //        -1 if keyword not specified
      strings get(const string &key) const;

      // Check if a resource exists
      // Returns true if resource exists, false otherwise
      bool check_exists(const string &name) const;

      // Print the contents of the resource table
      friend ostream & operator<< (ostream & s, resources &d);

   private:

      // Container for the resources
      res_T res; 
};

/***********************************************************************/
}
#endif

