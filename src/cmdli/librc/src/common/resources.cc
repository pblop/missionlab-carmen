/**********************************************************************
 **                                                                  **
 **  resources.cc                                                    **
 **                                                                  **
 **  The resource file object                                        **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: resources.cc,v 1.1.1.1 2008/07/14 16:44:17 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: resources.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:17  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:34:51  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.18  2004/11/12 21:59:51  doug
// renamed yacc/bision stuff so doesn't conflict with missionlab
//
// Revision 1.17  2004/10/29 22:38:33  doug
// working on waittimeout
//
// Revision 1.16  2004/05/11 19:34:31  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.15  2004/03/24 22:15:37  doug
// working on win32 port
//
// Revision 1.14  2003/09/16 16:59:17  doug
// added check_exists helper function
//
// Revision 1.13  2003/09/16 16:54:54  doug
// added set helper function
//
// Revision 1.12  2003/09/16 16:44:12  doug
// added get_string and set_string helper functions
//
// Revision 1.11  2003/07/12 01:59:41  doug
// Much work to get tomahawk mission to work.  Added task lead capability to opcon and -o flag to disable it in controller
//
// Revision 1.10  2002/12/23 20:21:37  doug
// added get/set int and set_double.  Also changed get_double to use reference instead of pointer
//
// Revision 1.9  2002/10/23 20:08:39  doug
// fix deprecated code
//
// Revision 1.8  2002/10/10 20:51:31  doug
// added set_boolean to allow updating the value of a flag
//
// Revision 1.7  2002/10/10 20:22:10  doug
// Added save capabilities to the resource library
//
// Revision 1.6  1998/02/10 14:37:41  doug
// fixed for Win32
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
// Revision 1.1  1997/12/01 12:42:55  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////


#include "mic.h"
#include <stdio.h>
#include <errno.h>
#include <fstream>
#ifdef WIN32
   #include <process.h>
#endif
#include "rc_parser.h"
#include "rc_scanner.h"
#include "resources.h"

namespace sara
{
/**********************************************************************/
// Create an empty resource object
resources::resources()
{
}

/**********************************************************************/
// Destroy a resource object
resources::~resources()
{
}

/**********************************************************************/
// restore a resource object from the specified file
// Returns 0 if success
//         1 if parse error, but some of the table is loaded
//         2 if unable to open file
int 
resources::load(const string &filename, 
	bool no_path_search, bool verbose, bool debug)
{
   /************** initialize the parser ********************/

   sara_rc_debug = debug;
   sara_rc_had_error = false;
   sara_rc_filename = &filename;
   sara_rc_table = &res;

   string fullname = filename;

   if( !no_path_search )
   {
      if( !findfile(filename, fullname) )
         return(2);
   }

   // The string data fnc is broken in the g++ library.
   // It doesn't put a null char on the string.
   char buf[2048];
   strcpy(buf,fullname.data());
   buf[fullname.length()] = '\0';
   sara_rc_in = fopen(buf,"r");
   if( sara_rc_in == NULL )
      return(2);

   if( verbose )
      INFORM("Loading resource file %s", buf);
//      INFORM("Loading resource file %s", fullname.data());

/************** load the input file ********************/

   sara_rc_parse();

/************** done ********************/

   fclose(sara_rc_in);

   if( sara_rc_had_error )
   {
      if( verbose )
         INFORM("\tDone, errors were encountered.");

      return 1;
   }

   if( verbose )
      INFORM("\tDone.");

   return 0;
}

// **********************************************************************
// set the value of a boolean flag
// Adds the resource if it does not exist
void 
resources::set_bool(const string &name, bool value)
{
   strings str;
   str.push_back( value ? "true" : "false" );
   res[name] = str;
}

// **********************************************************************
// Get the value of a boolean flag
// Returns 0 if false
//         1 if true
//        -1 if not specified
//        -2 if value is not "true" or "false"
int 
resources::check_bool(const string &name) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   if( it == res.end() )
      return -1;

   // Get the value list
   const strings &val( (*it).second );

   // error if not 1 value
   if( val.size() != 1 )
      return -2;

//   if( strcasecmp(val[0].data(),"true") == 0 )
   if( val[0] == "true" )
      return 1;

//   if( strcasecmp(val[0].data(),"false") == 0 )
   if( val[0] == "false" )
      return 0;

   return -2;
}

/**********************************************************************/
// Check the value of a symbol
// Returns 1 if value is found
//         0 if value not found
//        -1 if keyword not specified
int 
resources::check_value(const string &key, const string &value) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(key);

   // Was it there?
   if( it == res.end() )
      return -1;

   // Get the value list
   const strings &val( (*it).second );

   // See if the specified value was mentioned.
   for(strings::const_iterator sit = val.begin(); sit != val.end(); ++sit)
   {
//      if( strcasecmp((*sit).data(),value.data()) == 0 )
      if( (*sit) == value)
            return 1;
   }

   return 0;
}

// *********************************************************************
// Get the value of a symbol 
// Returns 1 if value is found
//         0 if value not found 
//        -1 if keyword not specified
strings 
resources::get(const string &name) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   if( it == res.end() )
   {
      strings empty;
      return empty;
   }

   // return the value list
   return (*it).second;
}

// *********************************************************************
// Set the value of a resource 
// Adds the resource if it does not exist
void 
resources::set(const string &name, const strings &value)
{
   res[name] = value;
}

// *********************************************************************
// Check if a resource exists
// Returns true if resource exists, false otherwise
bool 
resources::check_exists(const string &name) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   return it != res.end();
};

// **********************************************************************
// Get the value of a symbol as a double
// Returns 
//        false if not specified
//        true if value is loaded
bool 
resources::get_double(const string &name, double &d) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   if( it == res.end() )
      return false;

   // Get the value list
   const strings &val( (*it).second );

   // error if not 1 value
   if( val.size() != 1 )
      return false;

   // convert it to a double and return it
   d = atof( val[0].c_str() );

   return true;
}

// **********************************************************************
// set the value of a double resource
// Adds the resource if it does not exist
void 
resources::set_double(const string &name, double value)
{
   strings str;
   ostringstream ost;
   ost << value;
   str.push_back( ost.str() );
   res[name] = str;
}

// **********************************************************************
// Get the value of a symbol as a string
// Returns 
//        false if not specified
//        true if value is loaded
bool 
resources::get_string(const string &name, string &value) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   if( it == res.end() )
      return false;

   // Get the value list
   const strings &val( (*it).second );

   // error if not 1 value
   if( val.size() != 1 )
      return false;

   // return it
   value = val[0];

   return true;
}

// **********************************************************************
// set the value of a string resource
// Adds the resource if it does not exist
void 
resources::set_string(const string &name, const string &value)
{
   res[name] = value;
}

// **********************************************************************
// Get the value of an integer resource
//        false if not specified
//        true if value is loaded
bool 
resources::get_int(const string &name, int &d) const
{
   // Find the location of the key
   res_T::const_iterator it = res.find(name);

   // Was it there?
   if( it == res.end() )
      return false;

   // Get the value list
   const strings &val( (*it).second );

   // error if not 1 value
   if( val.size() != 1 )
      return false;

   // convert it to an int and return it.
   d = atoi( val[0].c_str() );

   return true;
}

// **********************************************************************
// set the value of an integer resource
// Adds the resource if it does not exist
void 
resources::set_int(const string &name, int value)
{
   strings str;
   ostringstream ost;
   ost << value;
   str.push_back( ost.str() );
   res[name] = str;
}

// **********************************************************************
// write the current resource object to the specified file
// the filename must be fully qualified.
// the existing file is renamed with a ~ extension if backup_files is true.
// Returns true if success
//         false if unable to write the file
bool 
resources::save(const string &filename, bool backup_files)
{
   // If want backups, check if the destination already exists.
   // If not, no need to make a backup.
   if( backup_files )
   {
      FILE *test = fopen(filename.c_str(),"r");
      if( test != NULL )
	 fclose(test);
      else
	 backup_files = false;
   }

   // write the new file to a temp name.
   string out_name;
   if( backup_files )
   {
      char buf[256];
      sprintf(buf,"%d", getpid());
      out_name = filename + buf;
   }
   else
   {
      out_name = filename;
   }

   // open the file
   ofstream out(out_name.c_str());
   if (!out)
   {
      cerr << "Unable to open output file " << out_name;
      return false;
   }

   out << "/*************************************************\n";
   out << "* This rc file " << filename << " was created by opcon\n";
   out << "**************************************************/\n\n";

   out << *this;

   out.close();

   if( backup_files )
   {
      // rename the original file to the same name with "~" on the end.
      string backupname = filename + '~';
      FILE *test = fopen(backupname.c_str(),"r");
      if( test != NULL )
      {
	 // A backup already exists, delete it.
         fclose(test);
	 unlink(backupname.c_str());
      }

      // rename the existing one to the backup
      if( rename(filename.c_str(), backupname.c_str()) )
      {
	 WARN("Unable to backup file %s as %s: %s", filename.c_str(), backupname.c_str(), strerror(errno));
	 unlink(filename.c_str());
      }

      if( rename(out_name.c_str(), filename.c_str()) )
      {
	 WARN("Unable to move temporary file %s to %s: %s", out_name.c_str(), filename.c_str(), strerror(errno));
      }
   }

   return true;
}

// **********************************************************************
// Print the contents of the resource table
ostream & operator<< (ostream & s, resources &d)
{
   // for each keyword
   for(res_T::iterator key=d.res.begin(); key != d.res.end(); ++key)  
   {
      // Print the keyword
      s << (*key).first << " = ";

      // for each value
      bool first(true);
      strings &rhs = (*key).second;
      for(strings::iterator val=rhs.begin(); val != rhs.end(); ++val)
      {
	 // Print the list of values, seperated by commas.
	 if( !first )
            s << ", ";
         first = false;

	 // does it contain unusual characters that need to be quoted?
	 bool quoteit = false;
	 for(uint i=0; i<val->size(); i++)
	 {
	    char ch = (*val)[i];
            if( !isalnum(ch) && ch != '_' && ch != '/' && ch != '\\' && ch != '-' && ch != '+' && ch != '.' )
	    {
	       quoteit = true;
	       break;
	    }
 
	 }

	 if( quoteit )
	 {
            s << '\"';
	 }

	 s << *val;

	 if( quoteit )
	 {
            s << '\"';
	 }
      }
      s << '\n';
   }

   return s;
}

/**********************************************************************/
}
