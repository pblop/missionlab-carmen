/**********************************************************************
 **                                                                  **
 **                            load_rc.cc                           **
 **                                                                  **
 **  Main entry point for the rc parser                             **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: load_rc.cc,v 1.1.1.1 2008/07/14 16:44:22 endo Exp $ */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
//#include <GetOpt.h>

#include "utilities.h"
#include "rc_defs.h"

const char *_rc_filename;

symbol_table<rc_chain> *_rc_table = NULL;


/*********************************************************************
*                                                                     *
*********************************************************************/

// Finds the file and returns the full pathname to it or NULL.
static char *
find_rc_file(const char *name)
{
   // Check current directory first.
   FILE *file = fopen(name,"r");
   if( file != NULL )
   {
      fclose(file);
      return strdup(name);
   }

   // Check user's home directory next
   char *home_dir;
   char *full_home_name = NULL;
   if( (home_dir = getenv("HOME")) != NULL )
   {
      full_home_name = strdupcat(strdup(home_dir),"/");
      full_home_name = strdupcat(full_home_name,name);
      if( (file = fopen(full_home_name,"r")) != NULL )
      {
         fclose(file);
         return full_home_name;
      }
   }

   // As last resort, follow the PATH directory list to see if is in the
   // directory this executable was ran from.
   char **dirs = extract_directories("PATH");
   if( dirs )
   {
      int i=0;
      char filename[2048];

      // Check all the directories
      while( dirs[i] )
      {
         sprintf(filename, "%s/%s",dirs[i], name);

         file = fopen(filename,"r");
         if( file != NULL )
         {
            fclose(file);

	    // Let's copy it to the user's home directory to speed
            // things up next time.
//            copyfile(filename, full_home_name);

            if( full_home_name )
               free(full_home_name);

            // But, we will return the name of the file we found in case
            // the copy didn't work for some reason.
            return strdup(filename);
         }

         i++;
      }
   }

   if( full_home_name )
      free(full_home_name);

   // Didn't find it
   return NULL;
}


/*********************************************************************
*                                                                     *
*********************************************************************/


// Returns 0 if success
//         1 if parse error, but some of the table is loaded
//         2 if unable to open file
int
load_rc(const char *filename, symbol_table<rc_chain> *table,
	bool no_path_search, bool verbose, bool debug)
{
   rc_debug = debug;

   /************** initialize the parser ********************/

   rc_had_error = false;
   _rc_filename = filename;
   _rc_table = table;

   const char *full_name = filename;

   if( !no_path_search )
   {
      full_name = find_rc_file(filename);
      if( full_name == NULL )
         return(2);
   }

   rc_in = fopen(full_name, "r");
   if( rc_in == NULL )
      return(2);

   if( verbose )
      cerr << "Loading resource file " << full_name << '\n';

/************** load the input file ********************/

   rc_parse();

/************** done ********************/

   fclose(rc_in);

   if( rc_had_error )
      return 1;

   return 0;
}


/*********************************************************************
*                                                                     *
*********************************************************************/


// Returns 0 if false
//         1 if true
//        -1 if not specified
//        -2 if value is not "true" or "false"
int
check_bool_rc(const symbol_table<rc_chain> &table, const char *name)
{
   const rc_chain *val = table.get(name);
   if( val == NULL)
      return -1;

   char *str;
   void *pos = val->first(&str);
   if( pos == NULL )
      return -2;

   if( strcasecmp(str,"true") == 0 )
      return 1;

   if( strcasecmp(str,"false") == 0 )
      return 0;

   return -2;
}

/*********************************************************************
*                                                                     *
*********************************************************************/


// Returns 1 if value is found
//         0 if value not found
//        -1 if keyword not specified
int
check_value_rc(const symbol_table<rc_chain> &table,
		const char *key,
		const char *value)
{
   const rc_chain *val = table.get(key);
   if( val == NULL)
      return -1;

   char *str;
   void *pos = val->first(&str);
   if( pos )
   {
      do
      {
         if( strcasecmp(str,value) == 0 )
            return 1;
      } while( (pos = val->next(&str,pos)) != NULL);
   }

   return 0;
}


/*********************************************************************
*                                                                     *
*********************************************************************/


// Returns
//        false if not specified
//        true if value is loaded
bool
check_dbl_rc(const symbol_table<rc_chain> &table, const char *name, double *d)
{
   const rc_chain *val = table.get(name);
   if( val == NULL)
      return false;

   char *str;
   void *pos = val->first(&str);
   if( pos == NULL )
      return false;

   *d = atof(str);

   return true;
}


///////////////////////////////////////////////////////////////////////
// $Log: load_rc.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:22  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:50  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:59  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:07  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.15  1999/12/16 22:45:04  endo
// rh-6.0 porting.
//
// Revision 1.14  1996/05/06 03:04:41  doug
// fixing compiler warnings
//
// Revision 1.13  1996/04/06  23:54:45  doug
// *** empty log message ***
//
// Revision 1.12  1996/03/06  23:40:22  doug
// *** empty log message ***
//
// Revision 1.11  1996/03/04  22:52:36  doug
// *** empty log message ***
//
// Revision 1.10  1996/03/04  22:30:45  doug
// *** empty log message ***
//
// Revision 1.9  1995/11/19  21:38:02  doug
// *** empty log message ***
//
// Revision 1.8  1995/11/19  21:13:30  doug
// added function to read booleans from table
//
// Revision 1.7  1995/10/30  21:35:17  doug
// *** empty log message ***
//
// Revision 1.6  1995/08/24  19:32:51  doug
// *** empty log message ***
//
// Revision 1.5  1995/08/24  17:06:08  doug
// don't print error message if can't open file, let caller do it.
//
// Revision 1.4  1995/08/24  16:02:44  doug
// *** empty log message ***
//
// Revision 1.3  1995/08/24  15:50:13  doug
// *** empty log message ***
//
// Revision 1.2  1995/08/24  15:45:24  doug
// *** empty log message ***
//
// Revision 1.1  1995/08/24  15:42:26  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
