/**********************************************************************
 **                                                                  **
 **  findfile.cc                                                     **
 **                                                                  **
 **  find a file and return the full pathname to it                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: findfile.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: findfile.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:16  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:46  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:37:55  endo
// MissionLab 7.0
//
// Revision 1.1  2006/07/01 00:30:29  endo
// CMDLi from MARS 2020 migrated into AO-FNC repository.
//
// Revision 1.1.1.1  2006/06/29 20:42:07  endo
// cmdli local repository.
//
// Revision 1.6  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.5  2004/03/08 14:52:13  doug
// cross compiles on visual C++
//
// Revision 1.4  2002/10/23 20:00:53  doug
// remove deprecated warnings
//
// Revision 1.3  1997/12/10 11:58:58  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/09 15:58:48  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/01 17:59:00  doug
// Initial revision
//
//
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "mic.h"

using namespace std;

namespace sara
{
/**********************************************************************/

static const string DOT(".");

/**********************************************************************/
// Tries to find the file and return the full pathname to it.
bool 
findfile(const string &fname, const string &dirname, string &fullpath)
{
   // Check if the file exists in the directory, if so, return the full path.

   string fullname = dirname + SEPCHAR + fname;
   FILE *file = fopen(fullname.c_str(),"r");
   if( file != NULL )
   {        
      fclose(file);
      fullpath = fullname;
      return true;
   } 

   // Didn't find it
   return false;
}

/**********************************************************************/

// find the file by looking through the list of directories
bool 
findfile(const string &name, const strings &dirs, string &fullpath)
{
   // First, see if they gave us a full path
   FILE *file = fopen(name.c_str(),"r");
   if( file )
   {        
      fclose(file);
      fullpath = name;
      return true;
   } 

   // Check current directory first.
   if( findfile(name, DOT, fullpath) )
      return true;

   // Check all the directories
   for(strings::const_iterator it=dirs.begin(); it != dirs.end(); ++it)
   {
      string filename = (*it) + SEPCHAR + name;
      FILE *file = fopen(filename.c_str(),"r");
      if( file != NULL )
      {
         fclose(file);
         fullpath = filename;
         return true;
      }
   }

   // Didn't find it
   return false;
}


/**********************************************************************/
// Tries to find the file and return the full pathname to it.
bool 
findfile(const string &name, string &fullpath)
{
   // First, see if they gave us a full path
   FILE *file = fopen(name.c_str(),"r");
   if( file )
   {        
      fclose(file);
      fullpath = name;
      return true;
   } 

   // Check current directory next.
   if( findfile(name, DOT, fullpath) )
      return true;

   // Check user's home directory next
   char *home_dir;
   if( (home_dir = getenv("HOME")) != NULL )
   {
      if( findfile(name, home_dir, fullpath) )
         return true;
   }

   // As last resort, follow the PATH directory list to see if is in the
   // directory this executable was ran from.
   strings dirs = extract_directories("PATH");

   // Check all the directories
   for(strings::iterator it=dirs.begin(); it != dirs.end(); ++it)
   {
      string filename = (*it) + SEPCHAR + name;
      FILE *file = fopen(filename.c_str(),"r");
      if( file != NULL )
      {
         fclose(file);
         fullpath = filename;
         return true;
      }
   }

   // Didn't find it
   return false;
}

/**********************************************************************/
}
