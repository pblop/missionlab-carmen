/**********************************************************************
 **                                                                  **
 **  extract_directories.cc                                          **
 **                                                                  **
 **  pull the list of directories out of a text string               **
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1996-1998.  All Rights Reserved.                      **
 **  Mobile Intelligence Corporation                                 **
 **  Alto, Michigan, USA                                             **
 **                                                                  **
 **********************************************************************/

/* $Id: extract_directories.cc,v 1.1.1.1 2008/07/14 16:44:16 endo Exp $ */

///////////////////////////////////////////////////////////////////////
// $Log: extract_directories.cc,v $
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
// Revision 1.5  2004/05/11 19:34:29  doug
// massive changes to integrate with USC and GaTech
//
// Revision 1.4  1997/12/10 14:39:48  doug
// fixed so uses push_back.  The assignment was coredumping.
//
// Revision 1.3  1997/12/10 11:58:58  doug
// *** empty log message ***
//
// Revision 1.2  1997/12/09 15:58:48  doug
// *** empty log message ***
//
// Revision 1.1  1997/12/06 23:38:32  doug
// Initial revision
//
///////////////////////////////////////////////////////////////////////

#include <string.h>
#include "mic.h"
#include <stdio.h>
#include <stdlib.h>

namespace sara
{
/**********************************************************************/
// extract the list of colon seperated directories from an environment variable
strings
extract_directories(const char *name)
{
   strings dir_list;

   // Get the value associated with the environment variable
   char *val = getenv(name);
   if (val == NULL)
      return dir_list;

   // Get a copy of the environment variable so we can hack it.
   char *value = strdup(val);

   dir_list.push_back(".");

   char *pos = value;
   char *next;

   while ((next = strchr(pos, ':')) != NULL)
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
               dir_list.push_back(pos);
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

      dir_list.push_back(pos);
   }

   free(value);
   return dir_list;
}

/**********************************************************************/
}
