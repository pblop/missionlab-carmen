/**********************************************************************
 **                                                                  **
 **                    find_file.cc                                  **  
 **                                                                  **
 **  Written by:  Douglas C. MacKenzie                               **
 **                                                                  **
 **  Copyright 1995, Georgia Tech Research Corporation               **
 **  Atlanta, Georgia  30332-0415                                    **
 **  ALL RIGHTS RESERVED, See file COPYRIGHT for details.            **
 **                                                                  **
 **********************************************************************/

/* $Id: find_file.cc,v 1.1.1.1 2008/07/14 16:44:25 endo Exp $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utilities.h"

/*-----------------------------------------------------------------------*/
// find the file by looking through the list of directories

char *
find_file(const char *name, const rc_chain *dirs)
{
   // Current directory is implicitly first.
   FILE *file = fopen(name,"r");
   if( file != NULL )
   {
      fclose(file);
      return strdup(name);
   }

   // Check all the directories
   if( dirs ) 
   {
      char *str;
      void *pos = dirs->first(&str);
      while( pos )
      {  
         char *fullname = strdupcat(strdup(str),"/",name);

         file = fopen(fullname,"r");
         if( file != NULL )
         {
	    fclose(file);
            return fullname;
         }

         pos = dirs->next(&str, pos);
      }
   }

   // Didn't find it
   return NULL;
}


/**********************************************************************
 * $Log: find_file.cc,v $
 * Revision 1.1.1.1  2008/07/14 16:44:25  endo
 * MAST Project (based on MissionLab-MINOS-20071018.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/20 17:17:52  endo
 * MINOS Project (based on MissionLab-7.0.20060712.tar.gz)
 *
 * Revision 1.1.1.1  2006/07/12 13:38:01  endo
 * MissionLab 7.0
 *
 * Revision 1.1.1.1  2005/02/06 23:00:22  endo
 * AO-FNC Project (based on mlab-6.0.01.06192003.tar.gz)
 *
// Revision 1.2  1995/11/12  21:35:20  doug
// accidentally freed the directory strings as it looked through them
//
// Revision 1.1  1995/10/31  19:24:09  doug
// Initial revision
//
 **********************************************************************/
