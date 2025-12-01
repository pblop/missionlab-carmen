/**********************************************************************
 **                                                                  **
 **                 extract_directories.cc                           **
 **                                                                  **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **********************************************************************/

/* $Id: extract_directories.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"

// extract the list of colon seperated directories from an environment variable
char **
extract_directories(const char *name)
{
   char **dir_list = NULL;
   int num_dir_list = 0;

   // Get a copy of the environment variable so we can hack it.
   char *cdl_path = getenv(name);
   if (cdl_path != NULL)
      cdl_path = strdup(cdl_path);

   // Count the number of entries we will be returning so can allocate the array
   // The min is 2: one for a "." and one for a null string trailer
   int num = 2;
   if (cdl_path != NULL)
   {
      char *pos = cdl_path;

      while ((pos = strchr(pos, ':')) != NULL)
      {
	 num++;
	 pos++;
      }

      // Add one to hold the last string if it doesn't end in a colon
      if( cdl_path[strlen(cdl_path)-1] != ':' ) 
         num++;
   }

   dir_list = new char *[num];

   dir_list[num_dir_list++] = ".";

   if (cdl_path != NULL)
   {
      char *pos = cdl_path;
      char *next;

      while ((next = strchr(pos, ':')) != NULL && num_dir_list < num - 1)
      {
	 *next = '\0';
	 int len = strlen(pos);

	 if (len > 0)
	 {
	    // Delete any trailing slashes
	    if (pos[len - 1] == '/')
	    {
	       pos[len - 1] = '\0';
	       len--;
	    }

	    if (len > 0)
	       dir_list[num_dir_list++] = strdup(pos);
	 }

	 pos = next;
	 pos++;			// Skip past the colon

      }

      // The last one probably doesn't have a colon on the end
      int len = strlen(pos);

      if (len > 0)
      {
	 // Delete any trailing slashes
	 if (pos[len - 1] == '/')
	 {
	    pos[len - 1] = '\0';
	    len--;
	 }

	 dir_list[num_dir_list++] = strdup(pos);
      }
   }
   dir_list[num_dir_list] = NULL;

   free(cdl_path);
   return dir_list;
}


///////////////////////////////////////////////////////////////////////
// $Log: extract_directories.cc,v $
// Revision 1.1.1.1  2008/07/14 16:44:25  endo
// MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
//
// Revision 1.1.1.1  2006/07/20 17:17:52  endo
// MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
//
// Revision 1.1.1.1  2006/07/12 13:38:01  endo
// MissionLab 7.0
//
// Revision 1.1.1.1  2005/02/06 23:00:22  endo
// AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
//
// Revision 1.3  1995/10/30  23:03:30  doug
// Fix so doesn't hose up the environment variable
//
// Revision 1.2  1995/10/30  21:35:30  doug
// *** empty log message ***
//
// Revision 1.1  1995/10/30  21:13:16  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////
